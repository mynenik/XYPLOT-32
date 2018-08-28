/*
CDeviceContext.cpp

  Copyright (c) 1998--2018 Krishna Myneni
  <krishna.myneni@ccreweb.org>

  This software is provided under the terms of the 
  GNU General Public License (GPL), v3.0 or later.

*/
#include <string.h>
#include "CDeviceContext.h"

CDeviceContext::CDeviceContext (int dev_type, int xres, int yres)
{
  m_nDevType = dev_type;
  m_nXres = xres;
  m_nYres = yres;
  m_bInverted = false;
  m_nColors = 0;
  m_nForeground = 0;
  m_pColors = NULL;
  m_pColorNames = NULL;
}
//---------------------------------------------------------------

CDeviceContext::~CDeviceContext ()
{
  delete [] m_pColors;

  for (int i = 0; i < m_nColors; i++)
    delete [] m_pColorNames[i];

  delete [] m_pColorNames;
}
//----------------------------------------------------------------

unsigned CDeviceContext::GetColor (char* name)
{
  // Search the color table for specified color and return
  //   its color value; in this case an index into m_pColors

  for (int i = 0; i < m_nColors; i++)
    {
      if (strcmp(m_pColorNames[i], name) == 0) return i;
    }
  return (m_nColors + 1);  // return invalid color number if not found
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
