// CXyPlot.cpp
//
// Copyright 1995--2023 Krishna Myneni
// <krishna.myneni@ccreweb.org>
//
// This software is provided under the terms of the
// GNU General Public License (GPL), v3.0 or later.

#include "CXyPlot.h"
#define DEF_POINT_WIDTH 3

CPlot::CPlot(CDataset* d)
{
  m_pSet = d;
  m_nSymbol = sym_LINE;
}

//---------------------------------------------------------------

CXyPlot::CXyPlot(CDataset* ds) : CPlot(ds)
{
  m_nPointWidthScale = 200;
}
//---------------------------------------------------------------

void CXyPlot::Draw(CDC* pDc)
{
// CPen pen(m_nStyle, m_nWidth, m_nColor);

  pDc->SetForeground(m_nColor);

  switch (m_nSymbol)
    {
    case sym_POINT:
      DrawPointPlot (pDc, DEF_POINT_WIDTH);
      break;

    case sym_BIG_POINT:
      DrawPointPlot (pDc, 2*DEF_POINT_WIDTH + 1);
      break;

    case sym_LINE_PLUS_POINT:
      DrawPointPlot (pDc, DEF_POINT_WIDTH);
      DrawLinePlot (pDc);
      break;

    case sym_STICK:
      DrawStickPlot (pDc);
      break;

    case sym_HISTOGRAM:
      DrawHistogram (pDc);
      break;

    default:
      DrawLinePlot (pDc);
    }

}

//---------------------------------------------------------------

void CXyPlot::DrawPointPlot (CDC* pDc, int pt_width)
{
    int j, k, l, npts, pt_width2, bOverlap;
    int x1, y1, x2, y2;
    CPoint p;

    npts = m_pSet->NumberOfPoints();
    if (! npts) return;

// pt_width = pDc->GetDeviceCaps(HORZRES)/m_nPointWidthScale;

    if (pt_width % 2 == 0) ++pt_width;
    pt_width2 = pt_width/2;
    if (pt_width2 == 0) pt_width2 = 1;

    CRect plot_rect = m_pT->GetPhysical();
    x1 = plot_rect.TopLeft().x;
    y1 = plot_rect.TopLeft().y;
    x2 = plot_rect.BottomRight().x;
    y2 = plot_rect.BottomRight().y;
    vector<double> e = m_pT->GetLogical();
    vector<vector<double>::iterator> lim = m_pSet->Limits(e[0], e[1], bOverlap);

    if (bOverlap)
      {

        vector<double>::iterator data = lim[0];
        int step = m_pSet->SizeOfDatum();
	if (m_pSet->begin() < data) data -= step;
	if (lim[1] < (m_pSet->end() - step)) lim[1] += step;

        while (data <= lim[1])
	  {
	    p = m_pT->Physical ((double*) &data[0]);
            if ((p.x <= x2) && (p.x >= x1) && (p.y <= y2) && (p.y >= y1))
	      {
		for (k = -pt_width2; k <= pt_width2; k++)
		  for (l = -pt_width2; l <= pt_width2; l++)
		    pDc->SetPixel(p.x + k, p.y +l);
	      }
	    data += step;
	  }
      }
}
//---------------------------------------------------------------

void CXyPlot::DrawLinePlot (CDC* pDc)
{
    BOOL b_last, b_current;
    CPoint p, last_p;
    int bOverlap;

    int npts = m_pSet->NumberOfPoints();
    if (! npts) return;

    CRect plot_rect = m_pT->GetPhysical();
    vector<double> e = m_pT->GetLogical();
    vector<vector<double>::iterator> lim = m_pSet->Limits(e[0], e[1], bOverlap);

    if (bOverlap)
    {
        vector<double>::iterator vis_data_start = lim[0];
        vector<double>::iterator vis_data_end = lim[1];

        int step = m_pSet->SizeOfDatum();
        vector<double>::iterator data = vis_data_start;

        // Start at points just outside interval so line will be
        //   continuous to edge of plot area.

        if (m_pSet->begin() < data) data -= step;
        if (vis_data_end < (m_pSet->end() - step)) vis_data_end += step;

        p = m_pT->Physical ((double*) &data[0]);
        pDc->MoveTo (p.x, p.y);

        data += step;

        while (data <= vis_data_end) {
          last_p = p;
          p = m_pT->Physical ((double*) &data[0]);

          if (p.x != last_p.x || p.y != last_p.y) {
            b_last = plot_rect.PtInRect(last_p);
            b_current = plot_rect.PtInRect(p);

            // if (b_last || b_current) {
            if (! b_last) pDc->MoveTo(last_p.x, last_p.y);
            pDc->LineTo (p.x, p.y);
            // }
          }
          data += step;
	}
    }
}
//---------------------------------------------------------------

void CXyPlot::DrawStickPlot (CDC* pDc)
{
    CPoint p1, p2;
    double x[2];
    int bOverlap;

    int npts = m_pSet->NumberOfPoints();
    if (! npts) return;

    vector<double> e = m_pT->GetLogical();
    vector<vector<double>::iterator> lim = m_pSet->Limits(e[0], e[1], bOverlap);

    if (bOverlap) {
      vector<double>::iterator data = lim[0];
      int step = m_pSet->SizeOfDatum();

      while (data <= lim[1]) {
	x[0] = *data;
	x[1] = 0.;
	p1 = m_pT->Physical (x);
	p2 = m_pT->Physical ((double*) &data[0]);
	pDc->MoveTo (p1.x, p1.y);
	pDc->LineTo (p2.x, p2.y);
	data += step;
      }
    }
}
//---------------------------------------------------------------

void CXyPlot::DrawHistogram (CDC* pDc)
{
    int bOverlap;
    int npts = m_pSet->NumberOfPoints();
    if (! npts) return;
    vector<double> e = m_pT->GetLogical();
    vector<vector<double>::iterator> lim = m_pSet->Limits(e[0], e[1], bOverlap);

    if (bOverlap) {
      CPoint p;
      double x[2], xc;
      vector<double>::iterator data = lim[0];
      int step = m_pSet->SizeOfDatum();
        
      if (m_pSet->begin() < data) data -= step;
      if (lim[1] < (m_pSet->end() - step)) lim[1] += step;

      double bw2 = (*(data+step) - *data)/2.;

      x[0] = *data - bw2; x[1] = *(data + 1);

      p = m_pT->Physical (x);
      pDc->MoveTo (p.x, p.y);
     
      while (true) {
        xc = *data;
	x[0] = xc + bw2;
	p = m_pT->Physical (x);
	pDc->LineTo (p.x, p.y);  // horizontal portion
	data += step;
	if (data > lim[1]) break;
	x[1] = *(data + 1);
	p = m_pT->Physical (x);
	pDc->LineTo (p.x, p.y);  // vertical portion
      }
    }
}
// --------------------------------------------------------------

