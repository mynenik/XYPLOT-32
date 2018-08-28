// CPlotDisplay.cpp
//
// Copyright 1996--2018 Krishna Myneni 
// <krishna.myneni@ccreweb.org>
//
// This software is provided under the terms of the
// GNU General Public License (GPL), v3.0 or later.
//

#include "CPlotDisplay.h"
#include <string.h>
extern char* DisplayFormat (float, float);

//---------------------------------------------------------------

CPlotDisplay::CPlotDisplay()
{
    m_fAspect = 1.;
    m_pPlotList = new CPlotList();
	vector<float> x(4);
	x[0] = -1.;
	x[1] = 1.;
	x[2] = -1.;
	x[3] = 1.;

    m_pGrid = NULL;
    CreateView (CARTESIAN, x);
}
//---------------------------------------------------------------

CPlotDisplay::~CPlotDisplay()
{
    delete m_pGrid;

    m_qCT.erase(m_qCT.begin(), m_qCT.end());

    delete m_pPlotList;

}
//---------------------------------------------------------------

void CPlotDisplay::SetExtrema(vector<float> x)
{
// Set extrema for the current view

    (**m_pCt).SetLogical(x);
    strcpy (m_szXform, DisplayFormat(x[0],x[1]));
    strcpy (m_szYform, DisplayFormat(x[2],x[3]));
}
//---------------------------------------------------------------

void CPlotDisplay::CreateView(COORDINATE_SYSTEM cdns, vector<float> x)
{

  CTransform* pCt;

  switch (cdns)
    {
    case CARTESIAN:
      pCt = new C2D_Transform();
      m_qCT.push_back(pCt);
      delete m_pGrid;
      m_pGrid = new CCartesianGrid();
      break;
    case CARTESIAN_INVERTED:
      pCt = new C2Di_Transform();
      m_qCT.push_back(pCt);
      delete m_pGrid;
      m_pGrid = new CCartesianGrid();
      break;
    case POLAR:
      pCt = new C2D_PolarTransform();
      m_qCT.push_back(pCt);
      delete m_pGrid;
      m_pGrid = new CPolarGrid();
      break;
    default:
      ;
    }

  m_pGrid->SetTransform (pCt);
  m_pCt = m_qCT.end() - 1;
  SetExtrema (x);

}
//---------------------------------------------------------------
/*
void CPlotDisplay::SetViewAngles(float theta, float phi)
{
	float angles[2] = {theta, phi};
	m_pCt->SetView(angles);
}
*/
//---------------------------------------------------------------

void CPlotDisplay::SetPlotRect (CRect wRect, CDC* pDC)
{
// Input rectangle is the entire client area to be used
//   for drawing. The plotting area rectangle will be
//   computed to acheive the current aspect ratio and
//   allow for x and y axis labels and other area surrounding
//   the plot.

        if (pDC)
        {
	  TEXTMETRIC tm;
	  pDC->GetTextMetrics (&tm);
	  wRect.TopLeft().x += 12*tm.tmAveCharWidth;
	  wRect.BottomRight().x -= 5*tm.tmAveCharWidth;

	 // Determine aspect corrected size

	  int y1;

	  if (pDC->m_bInverted)
	    {
	      wRect.BottomRight().y -= 6*tm.tmHeight;
	      y1 = (int)(wRect.BottomRight().y - wRect.Width()/m_fAspect);
	      if (y1 < 2*tm.tmHeight) y1 = 2*tm.tmHeight; 
	      wRect.TopLeft().y = y1;
	    }
	  else
	    {
	      wRect.TopLeft().y += 6*tm.tmHeight;
	      y1 = (int)(wRect.TopLeft().y + wRect.Width()/m_fAspect);
	      wRect.BottomRight().y = y1;
	    }
        }

    (**m_pCt).SetPhysical (wRect);
    m_pGrid->SetTransform (*m_pCt);
}
//---------------------------------------------------------------

float CPlotDisplay::GetAspect()
{
// Return aspect ratio of current display

    CRect wRect = (**m_pCt).GetPhysical();
    return (((float) wRect.Width())/wRect.Height());

}
//---------------------------------------------------------------

CPlot* CPlotDisplay::MakePlot(CDataset* ds, int plot_type)
{
    CPlot* p = NULL;

    switch (plot_type)
      {
      case 0:
	p = new CXyPlot(ds);
	break;
      default:
	;
      }

    // Add to plot list

    if (p) m_pPlotList->AddPlot (p);
    return p;
}
//---------------------------------------------------------------

CPlot* CPlotDisplay::MakePlot (CWorkspace41* ws, CDataset* ds, int n)
{
// Create corresponding plot for plot n in a 4.1 workspace

    CPlot* p = NULL;

    p = new CXyPlot (ds);
    switch (ws->pv[n].sym)
    {
        case 'p':
          p->SetSymbol(sym_POINT);
          break;
        case 'l':
          p->SetSymbol(sym_LINE);
          break;
        case '+':
          p->SetSymbol(sym_LINE_PLUS_POINT);
          break;
        case 'S':
          p->SetSymbol(sym_STICK);
          break;
        default:
          ;
    }

    // Add to plot list

    m_pPlotList->AddPlot (p);

    return p;
}
//---------------------------------------------------------------

int CPlotDisplay::IndexOf (CPlot* p)
{
  // Return the vector index of the plot p in the display;
  // return -1 if plot is not found in the display.

  int nPlot = -1;
  if (p)
    {
      int i, n = Nplots();
      
      for (i = 0; i < n; i++)
	{
	  if ((*this)[i] == p) break;
	}
      if (i < n) nPlot = i;
    }
  return nPlot;
}
//----------------------------------------------------------------

void CPlotDisplay::DeletePlot ()
{
// Delete the active plot in the plot list

    CPlot* p = m_pPlotList->Active();

    m_pPlotList->RemovePlot(p);
    delete p;
}
//---------------------------------------------------------------

void CPlotDisplay::DeletePlotsOf (CDataset* d)
{
// Delete all plots which are associated with dataset d

    CPlot* p;

    while (p = m_pPlotList->PlotOf(d))
    {
        m_pPlotList->RemovePlot(p);

        delete p;
    }
}
//---------------------------------------------------------------

void CPlotDisplay::Draw(CDC *pDC)
{

  m_pGrid->SetColor(pDC->GetColor("black"));  // set color of grid
  m_pGrid->Draw(pDC);
  m_pGrid->Labels(pDC);

  CRect rect = (**m_pCt).GetPhysical();
  pDC->SetClipRect(rect);  // enable clipping for plot area
  m_pPlotList->Draw(pDC);
  pDC->SetClipRect (pDC->GetClientRect()); // disable clipping
}
//---------------------------------------------------------------

void CPlotDisplay::ResetExtrema()
{
// Set display extrema to encompass all data sets for
//   which there are plots

  vector<float> x = m_pPlotList->GetExtrema();
  (**m_qCT.begin()).SetLogical(x);
  strcpy (m_szXform, DisplayFormat(x[0],x[1]));
  strcpy (m_szYform, DisplayFormat(x[2],x[3]));
}
//---------------------------------------------------------------

void CPlotDisplay::GoBack()
{
// Back up to the previous coordinate transformation in
//   a circular fashion

    if (m_pCt > m_qCT.begin())
        --m_pCt;
    else
        m_pCt = m_qCT.end() - 1;
}
//---------------------------------------------------------------

void CPlotDisplay::GoForward()
{
// Move to the next coordinate transformation in the queue,
//   in a circular fashion

    if (m_pCt < (m_qCT.end() - 1))
        ++m_pCt;
    else
        m_pCt = m_qCT.begin();
}
//---------------------------------------------------------------

void CPlotDisplay::DeleteView ()
{
// Delete the current transformation and go to previous one,
//   unless it is the first transformation.

    if (m_pCt > m_qCT.begin())
    {
        --m_pCt;
        m_qCT.erase (m_pCt + 1, m_pCt + 2);
    }
}
//---------------------------------------------------------------

CDataset* CPlotDisplay::GetActiveSet()
{
    CPlot* p = m_pPlotList->Active();
    if (p)
        return p->GetSet();
    else
        return NULL;
}

CDataset* CPlotDisplay::GetOperandSet()
{
    CPlot* p = m_pPlotList->Operand();
    if (p)
        return p->GetSet();
    else
        return NULL;
}
//---------------------------------------------------------------

int CPlotDisplay::Write (ofstream& ofile)
{
  // Write the plot display information to a file stream

  return 0;
}



