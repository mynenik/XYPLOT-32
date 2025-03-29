// CPolarGrid.cpp
//
// Copyright 1996--2023 Krishna Myneni
// <krishna.myneni@ccreweb.org>
//
// This software is provided under the terms of the
// GNU General Public License (GPL), v3.0 or later.
//

#include "CPolarGrid.h"
#include <math.h>
extern char* LabelFormat (double, double, char);

//---------------------------------------------------------------

CPolarGrid::CPolarGrid()
{
	m_nStyle = PS_SOLID;
// m_nColor = RGB(64,64,64);
	m_nColor = 0;
	nXtics = 8;
	nYtics = 10;
}
//---------------------------------------------------------------

void CPolarGrid::Draw(CDC* pDc)
{
	// Draw the grid

	int x1, x2, y1, y2;
	int xL, yL;
	int k;
	double r1, r2, rmax;
	vector<double> x;
	CPoint p1, p2;
	CRect rect = m_pT->GetPhysical();
// CPen FramePen(m_nStyle, m_nWidth, m_nColor);
// CPen LinesPen(m_nStyle, m_nWidth, m_nColor);
// pDc->SelectObject (&FramePen);
// pDc->Rectangle(rect);

    x = m_pT->GetLogical();
    rmax = x[1];

    x[0] = 0.;
    x[1] = 0.;
    p1 = m_pT->Physical(x);

	// Draw Tic Marks

    // Draw polar grid

//    if (m_bGridLines)
//    {
// pDc->SelectObject(&LinesPen);

        // Radial grid

        for (k = nYtics; k > 0; k--)
        {
            r1 = k*rmax/nYtics;

            x[0] = M_PI;
            x[1] = r1;
            p2 = m_pT->Physical(x);
            x1 = p2.x;
            x[0] = 0.;
            x[1] = r1;
            p2 = m_pT->Physical(x);
            x2 = p2.x;
            x[0] = -M_PI/2.;
            x[1] = r1;
            p2 = m_pT->Physical(x);
            y1 = p2.y;
            x[0] = M_PI/2.;
            x[1] = r1;
            p2 = m_pT->Physical(x);
            y2 = p2.y;
// pDc->Ellipse (x1, y1, x2, y2);
        }

	    // Angular grid

	    for (k = 0; k < nXtics; ++k)
	    {
	        x[0] = k*2.*M_PI/nXtics;
	        x[1] = rmax;
	        p2 = m_pT->Physical(x);
            pDc->MoveTo (p1.x, p1.y);
            pDc->LineTo (p2.x, p2.y);
        }

//    }

// pDc->SelectObject (&FramePen);


}
//---------------------------------------------------------------


void CPolarGrid::Labels(CDC* pDc)
{
    return;
}
