// CTransform.cpp
//
// Copyright 1995--2024 Krishna Myneni
// <krishna.myneni@ccreweb.org>
//
// This software is provided under the terms of the
// GNU General Public License (GPL), v3.0 or later.
//

#include "CTransform.h"
#include <math.h>

//---------------------------------------------------------------

void CTransform::SetLogical(vector<double> e)
{
    m_vEx = e;
    m_vDelta.erase(m_vDelta.begin(), m_vDelta.end());

    for (int i = 0; i < m_vEx.size(); i += 2)
        m_vDelta.push_back(m_vEx[i+1] - m_vEx[i]);

}
//---------------------------------------------------------------

CPoint CTransform::Physical (double* x)
{
    double xx[2] = {x[0], x[1]};
    return Physical (xx);
}
//---------------------------------------------------------------

vector<double> CTransform::Logical (CPoint p)
{
    // Return logical coordinates of p in a vector

    vector<double> vx(2);
    double x[2];

    Logical (p, x);
    vx[0] = x[0];
    vx[1] = x[1];

    return vx;
}
//---------------------------------------------------------------

CPoint C2D_Transform::Physical (double* x)
{
    // Compute physical coordinates of point

    CPoint p;

    p.x = (int)(m_nPRect.TopLeft().x +
     (x[0] - m_vEx[0])*m_nPRect.Width()/m_vDelta[0]);
    p.y = (int)(m_nPRect.BottomRight().y -
     (x[1] - m_vEx[2])*m_nPRect.Height()/m_vDelta[1]);

    return p;

}
//----------------------------------------------------------------

void C2D_Transform::Logical (CPoint p, double* x)
{
    // Compute logical coordinates of point p

    x[0] = m_vEx[0] + (double)(p.x - m_nPRect.TopLeft().x)*
      m_vDelta[0]/m_nPRect.Width();
    x[1] = m_vEx[2] + (double)(m_nPRect.BottomRight().y - p.y)*
      m_vDelta[1]/m_nPRect.Height();
}
//---------------------------------------------------------------

CPoint C2Di_Transform::Physical (double* x)
{
    // Compute physical coordinates of point

    CPoint p;

    p.x = (int)(m_nPRect.TopLeft().x +
     (x[0] - m_vEx[0])*m_nPRect.Width()/m_vDelta[0]);
    p.y = (int)(m_nPRect.TopLeft().y + 
     (x[1] - m_vEx[2])*m_nPRect.Height()/m_vDelta[1]);
    return p;
}
//----------------------------------------------------------------

void C2Di_Transform::Logical (CPoint p, double* x)
{
  // Compute logical coordinates of point p

  x[0] = m_vEx[0] + (double)(p.x - m_nPRect.TopLeft().x)*
    m_vDelta[0]/m_nPRect.Width();
  x[1] = m_vEx[2] + (double)(p.y - m_nPRect.BottomRight().y)*
    m_vDelta[1]/m_nPRect.Height();
}
//---------------------------------------------------------------

vector<double> CTransform::Logical (CRect rect)
{
// Compute and return logical coordinates of rectangle as a vector
//   order is xmin, xmax, ymin, ymax

    double br[2], tl[2];
    vector<double> x(4);

    Logical (rect.TopLeft(), tl);	// xmin, ymax
    Logical (rect.BottomRight(), br);	// xmax, ymin
    x[0] = tl[0];
    x[1] = br[0];
    x[2] = br[1];
    x[3] = tl[1];

    return x;
}
//---------------------------------------------------------------

CPoint C2D_PolarTransform::Physical (double* x)
{
    // Compute physical coordinates of point

    CPoint p;

    p.x = (int)(m_nPRect.TopLeft().x +
     (x[1]*cos(x[0]) - m_vEx[0])*m_nPRect.Width()/m_vDelta[0]);

    p.y = (int)(m_nPRect.BottomRight().y -
     (x[1]*sin(x[0]) - m_vEx[2])*m_nPRect.Height()/m_vDelta[1]);

    return p;
}
//---------------------------------------------------------------

void C2D_PolarTransform::Logical (CPoint p, double* x)
{
    // Compute logical coordinates of point p

    double xLogical, yLogical;

    xLogical = m_vDelta[0]*(p.x - m_nPRect.TopLeft().x)/m_nPRect.Width()
        + m_vEx[0];
    yLogical = -m_vDelta[0]*(p.y - m_nPRect.BottomRight().y)/m_nPRect.Height()
        + m_vEx[2];
	x[0] = atan2(yLogical, xLogical);
	x[1] = sqrt(xLogical*xLogical + yLogical*yLogical);
}
//---------------------------------------------------------------

