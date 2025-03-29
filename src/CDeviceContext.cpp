/*
CDeviceContext.cpp

  Copyright (c) 1998--2023 Krishna Myneni
  <krishna.myneni@ccreweb.org>

  This software is provided under the terms of the 
  GNU Affero General Public License (AGPL), v 3.0 or later.

*/
#include <string.h>
#include <math.h>
#include "CDeviceContext.h"

CDeviceContext::CDeviceContext (int dev_type, int xres, int yres)
{
  m_nDevType = dev_type;
  m_nXres = xres;
  m_nYres = yres;
  m_bInverted = false;
  m_nColors = 0;
  m_nForeground = 0;
  m_pRGB = NULL;         // array of packed RGB values
  m_pColors = NULL;      // array of XColor pixel values
  m_pColorNames = NULL;  // array of color names
}
//---------------------------------------------------------------

CDeviceContext::~CDeviceContext ()
{
  delete [] m_pColors;

  for (int i = 0; i < m_nColors; i++)
    delete [] m_pColorNames[i];

  delete [] m_pColorNames;
  delete [] m_pRGB;
}
//----------------------------------------------------------------

unsigned long CDeviceContext::GetColor (char* name)
{
  // Search the color map for specified color name and return
  //   its index in the map.

  for (int i = 0; i < m_nColors; i++)
    {
      if (strcmp(m_pColorNames[i], name) == 0) return i;
    }
  return (m_nColors + 1);  // return invalid color number if not found
}
//---------------------------------------------------------------

unsigned long CDeviceContext::GetColor (COLORREF cr)
{
  // Search the color map for color rgb value nearest the specified 
  // rgb value and and return its index in the map.

  unsigned short int r1, g1, b1, r2, g2, b2, u, v, w;
  r1 = cr & 0xff;
  g1 = (cr >> 8) & 0xff;
  b1 = (cr >> 16) & 0xff;
  double dist = 255.0*sqrt(3.0);
  double new_dist;
  COLORREF map_cr;
  unsigned long imin = 0;

  for (int i = 0; i < m_nColors; i++) {
      map_cr = m_pRGB[i];
      r2 = map_cr & 0xff;
      g2 = (map_cr >> 8) & 0xff;
      b2 = (map_cr >> 16) & 0xff;
      u = r1-r2; v = g1-g2; w = b1-b2;
      new_dist = sqrt( (double) (u*u + v*v + w*w) );
      if (new_dist < dist) {
        dist = new_dist;
        imin = i;
      }
    }
  return (imin);
}
//---------------------------------------------------------------

void CDeviceContext::Rectangle (CRect rect)
{

	int ix1 = rect.TopLeft().x;
	int iy1 = rect.TopLeft().y;
	int ix2 = rect.BottomRight().x;
	int iy2 = rect.BottomRight().y;

	MoveTo(ix1,iy1);
	LineTo(ix2, iy1);
	LineTo(ix2, iy2);
	LineTo(ix1, iy2);
	LineTo(ix1, iy1);
}
