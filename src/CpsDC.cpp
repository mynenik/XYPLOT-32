/*
CpsDC.cpp

  The Postscript device context

  Copyright (c) 1999--2020 Krishna Myneni
  <krishna.myneni@ccreweb.org>

  This software is provided under the terms of the 
  GNU Affero General Public License (GPL), v 3.0 or later.

*/
#include <string.h>
#include "CpsDC.h"

CpsDC::CpsDC (int xres, int yres)
  :  CDeviceContext (DEV_PS, xres, yres)
{
  ;
}
//---------------------------------------------------------------

void CpsDC::OpenDisplay (char* fname)
{
  m_pFile = new ofstream;
  m_pFile->open(fname);
  (*m_pFile) << "%!PS-Adobe-2.0\n";
  (*m_pFile) << "%%EndComments\n";
  (*m_pFile) << "%%EndProlog\n";
  (*m_pFile) << "%%Page: 1  1  \n";
  (*m_pFile) << "/Times-Roman findfont 14 scalefont setfont\n";
  (*m_pFile) << "1 setlinewidth\n";

  // Set up metrics for text

  m_nTm.tmWidth = 7;
  m_nTm.tmHeight = 9;
  m_nTm.tmAveCharWidth = 7;

  m_bInverted = false;
}
//----------------------------------------------------------------

void CpsDC::CloseDisplay ()
{
  (*m_pFile) << "stroke\nshowpage\n";
  m_pFile->close();
}
//----------------------------------------------------------------

void CpsDC::ClearDisplay ()
{
  ;
}
//----------------------------------------------------------------

void CpsDC::SetColors (const char* color_list[], int nColors)
{
  int nc = (nColors <= MAX_PS_COLORS) ? nColors : MAX_PS_COLORS;
  m_nColors = nc;
  m_pColors = new unsigned [nc];
  m_pColorNames = new char* [nc];

  for (int i = 0; i < nc; i++)
    {
      float fRed = 0.;
      float fGreen = 0.;
      float fBlue = 0.;

      const char* cname = color_list[i];

      if (strcmp(cname, "red") == 0)
	{
	  fRed = 1.;
	}
      else if (strcmp(cname, "green") == 0)
	{
	  fGreen = 1.;
	}
      else if (strcmp(cname, "blue") == 0)
	{
	  fBlue = 1.;
	}
      else if (strcmp(cname, "dark green") == 0)
	{
	  fGreen = 1.;
	  fBlue = 0.2;
	}
      else if (strcmp(cname, "cyan") == 0)
	{
	  fGreen = 1.;
	  fBlue = 1.;
	}
      else if (strcmp(cname, "magenta") == 0)
	{
	  fRed = 1.;
	  fBlue = 1.;
	}
      else if (strcmp(cname, "brown") == 0)
	{
	  fRed = 1.;
	  fGreen = 0.2;
	}
      else if (strcmp(cname, "yellow") == 0)
	{
	  fRed = 1.;
	  fGreen = 1.;
	}
      else if (strcmp(cname, "grey") == 0)
	{
	  fBlue = 1.;
	  fGreen = .5;
	}
      else if (strcmp(cname, "white") == 0)
	{
	  fRed = 1.;
	  fGreen = 1.;
	  fBlue = 1.;
	}
      else
	;  // black if color name not known

      m_fRGB [i][0] = fRed;
      m_fRGB [i][1] = fGreen;
      m_fRGB [i][2] = fBlue;

      m_pColors[i] = i;
      m_pColorNames[i] = new char[16];
      strcpy (m_pColorNames[i], color_list[i]);

    }
}
//----------------------------------------------------------------

void CpsDC::SetColors (COLORREF rgb_table[], const char* color_list[], int nColors)
{
  int nc = (nColors <= MAX_PS_COLORS) ? nColors : MAX_PS_COLORS;
  m_nColors = nc;
  m_pColors = new unsigned [nc];
  m_pColorNames = new char* [nc];
  float fRed, fGreen, fBlue;
  COLORREF cr;
  unsigned short int r, g, b;

  for (int i = 0; i < nc; i++) {
    cr = rgb_table[i];
    r = cr & 0xff;
    g = (cr >> 8) & 0xff;
    b = (cr >> 16) & 0xff;
    fRed = ((float) r)/255.;
    fGreen = ((float) g)/255.;
    fBlue = ((float) b)/255.;
    m_fRGB [i][0] = fRed;
    m_fRGB [i][1] = fGreen;
    m_fRGB [i][2] = fBlue;

    m_pColors[i] = i;
    m_pColorNames[i] = new char[16];
    strcpy( m_pColorNames[i], color_list[i]);
  }
}


void CpsDC::SetForeground (unsigned c)
{
  if ((c >= 0) && (c < m_nColors))
    {
      m_nForeground = c;
      (*m_pFile) << "stroke\n";
      (*m_pFile) << m_fRGB [c][0] << ' ' << m_fRGB [c][1] <<
	' ' << m_fRGB [c][2] << " setrgbcolor\n";
    }
}
//-----------------------------------------------------------------

void CpsDC::SetBackground (unsigned c)
{
  ;
}
//-----------------------------------------------------------------

CRect CpsDC::GetClientRect ()
{
  return (CRect(0, 0, m_nXres, m_nYres));
}
//------------------------------------------------------------------
	
void CpsDC::SetClipRect (CRect r)
{
  ;
}
//------------------------------------------------------------------

void CpsDC::SetDrawingMode (int n)
{
  ;
}
//------------------------------------------------------------------

void CpsDC::Rectangle (CRect r)
{
  CDeviceContext::Rectangle(r);
  (*m_pFile) << "stroke\n";
}
//------------------------------------------------------------------

void CpsDC::MoveTo (int x, int y)
{
  (*m_pFile) << x << ' ' << y << " moveto\n";
}
//------------------------------------------------------------------

void CpsDC::LineTo (int x, int y)
{
  (*m_pFile) << x << ' ' << y << " lineto\n";
}
//------------------------------------------------------------------

void CpsDC::SetPixel (int x, int y)
{
  ;
}
//------------------------------------------------------------------

void CpsDC::TextOut (int x, int y, char* pLabel)
{
  MoveTo (x, y);
  (*m_pFile) << '(' << pLabel << ") show\n";
}
//-------------------------------------------------------------------

CPlotMessage* CpsDC::GetPlotMessage()
{
  return NULL;
}
