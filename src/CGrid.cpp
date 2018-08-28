// CGrid.cpp
//
// Copyright 1995--2018 Krishna Myneni
// <krishna.myneni@ccreweb.org>
//
// This software is provided under the terms of the 
// GNU General Public License (GPL), v3.0 or later.
//

#include "CGrid.h"
#include <vector>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

bool CGridLines::m_bHorizontal = true;
bool CGridLines::m_bVertical = true;

extern char* LabelFormat (float, float, char);


CGridFrame::CGridFrame (int nXt, int nYt)
{
    if (nXt >= 0) nXtics = nXt;
    if (nYt >= 0) nYtics = nYt;
}
//---------------------------------------------------------------

void CGridFrame::Draw (CDC* pDc)
{
  CRect rect = m_pT->GetPhysical();
// CPen FramePen(m_nStyle, m_nWidth, m_nColor);
// CPen* pOldPen;
  int k;
  int ix1, ix2, iy1, iy2, ixL, iyL, ixT, iyT;

  // Draw the frame box

// pOldPen = pDc->SelectObject (&FramePen);
// pDc->Rectangle(rect);
// pDc->SetForeground(m_nColor);
	
  ix1 = rect.TopLeft().x;
  iy1 = rect.TopLeft().y;
  ix2 = rect.BottomRight().x;
  iy2 = rect.BottomRight().y;

  pDc->MoveTo(ix1,iy1);
  pDc->LineTo(ix2, iy1);
  pDc->LineTo(ix2, iy2);
  pDc->LineTo(ix1, iy2);
  pDc->LineTo(ix1, iy1);

  // Draw Tic Marks

  ixL = rect.Width()/50;
  iyL = rect.Height()/50;

// yL1 = log((double) iy2);
// yLd = log((double) iy1) - yL1;

  if (nXtics > 0)
    for (k = 0; k <= nXtics; ++k)
      nXt[k] = ix1 + k*rect.Width()/nXtics;

  if (nYtics > 0)
    for (k = 0; k <= nYtics; ++k)
      nYt[k] = iy1 + k*rect.Height()/nYtics;

  if (nXtics > 0)
    {
      for (k = 1; k < nXtics; ++k)
	{
	  pDc->MoveTo(nXt[k], iy1);
	  pDc->LineTo(nXt[k], iy1+iyL);
	  pDc->MoveTo(nXt[k], iy2);
	  pDc->LineTo(nXt[k], iy2-iyL);
        }
    }

  if (nYtics > 0)
    {
      for (k = 1; k < nYtics; ++k)
        {
       	    pDc->MoveTo(ix1, nYt[k]);
       	    pDc->LineTo(ix1+ixL, nYt[k]);
       	    pDc->MoveTo(ix2, nYt[k]);
       	    pDc->LineTo(ix2-ixL, nYt[k]);
	}
    }

// pDc->SelectObject(pOldPen);
}
//---------------------------------------------------------------

void CGridLines::Draw (CDC* pDc)
{
  int i, k, ixT, iyT, ix1, ix2, iy1, iy2;
  CRect rect = m_pT->GetPhysical();

  ix1 = rect.TopLeft().x;
  iy1 = rect.TopLeft().y;
  ix2 = rect.BottomRight().x;
  iy2 = rect.BottomRight().y;

  if (CGridLines::m_bVertical)
    {
      if (nXtics > 0)
	{
	  for (k = 1; k < nXtics; ++k)
	    {
	      ixT = nXt[k];
	      for (i = iy1; i < iy2; i++)
		if (i % 2) pDc->SetPixel (ixT, i);
	    }
	}
    }


  if (CGridLines::m_bHorizontal)
    {
      if (nYtics > 0)
	{
	  for (k = 1; k < nYtics; ++k)
	    {
	      iyT = nYt[k];
	      for (i = ix1; i < ix2; i++)
		if (i % 2) pDc->SetPixel(i, iyT);
	    }
	}
    }
}
//---------------------------------------------------------------

void CAxes::Draw (CDC* pDc)
{
  float ax[2] = {0., 0.};
  vector<float> x;
  CPoint p;
  CRect r = m_pT->GetPhysical();
  x = m_pT->GetLogical();
  p = m_pT->Physical(ax);

  if ((x[0] < ax[0]) && (x[1] > ax[0]))
    {
      pDc->MoveTo(p.x, r.TopLeft().y);
      pDc->LineTo(p.x, r.BottomRight().y);
    }
  if ((x[2] < ax[1]) && (x[3] > ax[1]))
    {
      pDc->MoveTo(r.TopLeft().x, p.y);
      pDc->LineTo(r.BottomRight().x, p.y);
    }
}

//---------------------------------------------------------------

CGrid::CGrid()
{
  m_nStyle = PS_DOT;
  m_nFontPointSize = 100;
  m_szFontName = new char[32];
  strcpy (m_szFontName, "Courier New");

  m_pFrame = NULL;
  m_pLines = NULL;
  m_pAxes = NULL;

  m_pFrame = new CGridFrame (10, 10);
  m_pFrame->SetAttributes (PS_SOLID, 1, m_nColor);

  SetLines(CGridLines::m_bHorizontal, CGridLines::m_bVertical);
  SetAxes(True);
}
//---------------------------------------------------------------
CGrid::~CGrid()
{
  delete [] m_szFontName;
  delete m_pFrame;
  delete m_pLines;
  delete m_pAxes;
}
//---------------------------------------------------------------

void CGrid::SetTics (int xtics, int ytics)
{
  delete m_pFrame;

  if ((xtics > 0) && (ytics > 0))
    {
      m_pFrame = new CGridFrame(xtics, ytics);
    }
  else
    {
      m_pFrame = NULL;
    }
}

void CGrid::GetTics (int* pXtics, int* pYtics)
{
    *pXtics = nXtics;
    *pYtics = nYtics;
}

void CGrid::SetLines (bool bx, bool by)
{
  delete m_pLines;
  CGridLines::m_bVertical = bx;
  CGridLines::m_bHorizontal = by;

  if (bx || by)
    {
      m_pLines = new CGridLines ();
      m_pLines->SetAttributes (PS_DOT, 1, m_nColor);
    }
  else
    {
      m_pLines = NULL;
    }
}

void CGrid::SetAxes (bool b)
{
  delete m_pAxes;

  if (b)
    {
      m_pAxes = new CAxes ();
      m_pAxes->SetAttributes (PS_SOLID, 1, m_nColor);
    }
  else
    {
      m_pAxes = NULL;
    }
}
//---------------------------------------------------------------

void CCartesianGrid::Draw(CDC* pDc)
{
  pDc->SetForeground(m_nColor);

  // Draw the frame and tics

  if (m_pFrame) m_pFrame->Draw(pDc);

  // Draw the grid lines

  if (m_pLines) m_pLines->Draw(pDc);

  // Draw the axes

  if (m_pAxes) m_pAxes->Draw(pDc);

}
//---------------------------------------------------------------

void CCartesianGrid::Labels(CDC* pDc)
{
  int k, ix, iy;
  vector<float> x;
  float xt, yt;
  char xl[20], yl[20];
  char xform[16], yform[16];

// CFont font, *pOldFont;
//
//    if (! font.CreatePointFont(m_nFontPointSize, m_szFontName, pDc))
//	{
//	    AfxMessageBox("Font not available!");
//	}
//    pOldFont = pDc->SelectObject(&font);
//
  TEXTMETRIC tm;
  pDc->GetTextMetrics (&tm);

  CRect rect = m_pT->GetPhysical();
  x = m_pT->GetLogical();

  float xmin = x[0];
  float xmax = x[1];
  float ymin = x[2];
  float ymax = x[3];
  float xdel = (xmax - xmin)/(float) nXtics;
  float ydel = (ymax - ymin)/(float) nYtics;

  strcpy (xform, LabelFormat (xmin, xmax, 'X'));
  strcpy (yform, LabelFormat (ymin, ymax, 'Y'));
  int xl_width = atoi(&xform[1]);
  int yl_width = atoi(&yform[1]);
  if (xl_width < 6) xl_width = 6;
  if (yl_width < 6) yl_width = 6;

  int xl_pixel_width = xl_width*tm.tmAveCharWidth;
  int xkstep = 2;

  while (nXtics*xl_pixel_width/xkstep > rect.Width())
        xkstep *= 2;

  if (xkstep > nXtics) xkstep = nXtics;

  xkstep = (rect.Width()/((float) xl_width * tm.tmAveCharWidth) > 4.5) ? 2 : 5;

  iy = pDc->m_bInverted ? rect.BottomRight().y + 2*tm.tmHeight :
    rect.TopLeft().y - 2*tm.tmHeight;

  for (k = 0; k <= nXtics; k += xkstep)
    {
      xt = xmin + k*xdel;
      memset (xl, 32, 20);
      sprintf (xl, xform, xt);
      xl_pixel_width = strlen(xl)*tm.tmAveCharWidth;
      pDc->TextOut(nXt[k] - xl_pixel_width/2, iy, xl);
    }

  int ykstep = 2;

  while (nYtics*tm.tmHeight/ykstep > rect.Height()/2)
    ykstep *= 2;
    
  if (ykstep > nYtics) ykstep = nYtics;

  ykstep = (rect.Height()/(float) tm.tmHeight > 4.5) ? 2 : 5;

  iy = pDc->m_bInverted ? tm.tmHeight/2 : -tm.tmHeight/2;

  for (k = 0; k <= nYtics; k += ykstep)
    {
      yt = pDc->m_bInverted ? ymax - k*ydel : ymin + k*ydel;      
      memset (yl, 32, 20);
      sprintf (yl, yform, yt);
      pDc->TextOut(rect.TopLeft().x - yl_width*tm.tmAveCharWidth,
		   nYt[k] + iy, yl);
    }

//	pDc->SelectObject(pOldFont);
}




