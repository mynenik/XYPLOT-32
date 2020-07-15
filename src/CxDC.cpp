/*
CxDC.cpp

  X Windows device context class

  Copyright (c) 1997--2020 Krishna Myneni
  <krishna.myneni@ccreweb.org>

  This software is provided under the terms of the 
  GNU Affero General Public License (AGPL), v 3.0 or later.

*/

#include <string.h>
#include "CxDC.h"
#include <iostream>
using namespace std;

//---------------------------------------------------------------

CxDC::CxDC(int dev_type, int xres, int yres)
  :  CDeviceContext (dev_type, xres, yres)
{
  m_bInverted = true;
}
//---------------------------------------------------------------

void CxDC::OpenDisplay (char* name)
{
	char* display_name = NULL;
	char* window_name = name;
	XTextProperty windowName;
	XGCValues values;
	int revert;
	
	// Connect to X server

	if ((m_pDisplay = XOpenDisplay(display_name)) == NULL)
	{
	  return; /* PL_E_GDCANTOPEN */
	}
	XGetInputFocus (m_pDisplay, &m_nTermWin, &revert);
	m_nMode = DefaultScreen(m_pDisplay);
	m_nXres = DisplayWidth(m_pDisplay, m_nMode)/2;
	m_nYres = DisplayHeight(m_pDisplay, m_nMode)/3;
	
	// Create opaque window
	
	m_nWin = XCreateSimpleWindow(m_pDisplay, RootWindow(m_pDisplay,
	  m_nMode), 0, 0, m_nXres, m_nYres, 4, BlackPixel(m_pDisplay,
	  m_nMode), WhitePixel(m_pDisplay, m_nMode));
	
	XStringListToTextProperty (&window_name, 1, &windowName);
	XSetWMName (m_pDisplay, m_nWin, &windowName);
	
	// Select event types wanted
	
	XSelectInput (m_pDisplay, m_nWin, ExposureMask | KeyPressMask |
	  ButtonPressMask | ButtonReleaseMask | 
	  StructureNotifyMask | PointerMotionMask);
	  
	// Create graphics context for text and drawing
	
	m_nGC = XCreateGC (m_pDisplay, m_nWin, 0L, &values);
	
	// Display window
	
	XMapWindow (m_pDisplay, m_nWin);
	    
	// Use the pre-defined crosshair cursor
	    
	m_nCursor = XCreateFontCursor (m_pDisplay, XC_crosshair);
	XDefineCursor (m_pDisplay, m_nWin, m_nCursor);

	// Set the default line attributes

	XSetLineAttributes (m_pDisplay, m_nGC, 0, LineSolid, CapButt, JoinMiter);

	// Set up the default color map

	m_nMap = DefaultColormap (m_pDisplay, m_nMode);	    
}
//------------------------------------------------------------

void CxDC::OpenDisplay (Display* d, Window w)
{
  // Create a device context for a window already opened.
  
  XGCValues values;

  m_pDisplay = d;
  m_nWin = w;
  m_nMode = DefaultScreen(m_pDisplay);

  // Create graphics context for text and drawing
	
  m_nGC = XCreateGC (m_pDisplay, m_nWin, 0L, &values);
  
  // Obtain the text metrics

  XGetGCValues (m_pDisplay, XDefaultGC(m_pDisplay, DefaultScreen(m_pDisplay)), 
     GCFont, &values);
  // printf ("font = %d\n", values.font);
  XFontStruct* pFs = XQueryFont (m_pDisplay, values.font);
  // printf ("Queried the font\npFs = %x\n", pFs);
  if (pFs)
    {
      XCharStruct bounds = pFs->max_bounds;
      m_nTm.tmWidth = bounds.width;
      m_nTm.tmHeight = bounds.ascent + bounds.descent;
      m_nTm.tmAveCharWidth = bounds.width;;

  // printf ("bounds width = %d\n", bounds.width);
  // printf ("bounds ascent = %d, descent = %d\n", bounds.ascent, bounds.descent);
    }
  else
    {
      m_nTm.tmWidth = 8;
      m_nTm.tmHeight = 9;
      m_nTm.tmAveCharWidth = 8;
    }

  // Set up the default color map

  m_nMap = DefaultColormap (m_pDisplay, m_nMode);	
  
  // Use the pre-defined crosshair cursor
	    
  m_nCursor = XCreateFontCursor (m_pDisplay, XC_crosshair);
  XDefineCursor (m_pDisplay, m_nWin, m_nCursor);
  
}
//---------------------------------------------------

void CxDC::CloseDisplay ()
{
  XFreeGC (m_pDisplay, m_nGC);
  XCloseDisplay (m_pDisplay);
	
}
//---------------------------------------------------------------

void CxDC::ClearDisplay ()
{
  // Clear the display area

  XClearArea (m_pDisplay, m_nWin, 0, 0, m_nXres, m_nYres, True);
}
//---------------------------------------------------------------

void CxDC::SetColors(const char* color_list[], int nColors)
{
  // Setup color table for the device context colormap

  int nc = (nColors < 33) ? nColors : 32;
  m_nColors = nc;
  m_pColors = new unsigned [nc];
  m_pColorNames = new char* [nc];
      
  XColor c;
  Status s;

  for (int i = 0; i < m_nColors; i++)
    {
      m_pColorNames[i] = new char [16];
      strcpy (m_pColorNames[i], color_list[i]);

      s = XParseColor (m_pDisplay, m_nMap, color_list[i], &c);
      if (s)
	{
	  XAllocColor (m_pDisplay, m_nMap, &c);
	}
      else
	{
	  c.pixel = BlackPixel(m_pDisplay, m_nMode);
	}
      m_pColors[i] = c.pixel;
    } 
}
//--------------------------------------------------------------

void CxDC::SetColors(COLORREF rgb_table[], const char* color_list[], int nColors)
{
  // Setup color table for the device context colormap

  int nc = (nColors < 33) ? nColors : 32;
  m_nColors = nc;
  m_pColors = new unsigned [nc];
  m_pColorNames = new char* [nc];
  unsigned short int r, g, b;
  COLORREF cr;
  XColor c;

  for (int i = 0; i < m_nColors; i++)
    {
      cr = rgb_table[i];
      r = cr & 0xff;
      g = (cr >> 8) & 0xff;
      b = (cr >> 16) & 0xff;
      c.red = (r << 8);
      c.green = (g << 8);
      c.blue = (b << 8);
      if ( ! XAllocColor(m_pDisplay, m_nMap, &c) ) {
          c.pixel = BlackPixel(m_pDisplay, m_nMode);
	  // cout << "Failed to allocate color!" << endl;
      }
// cout << " r: " << c.red/256 << " g: " << c.green/256 << " b: " << c.blue/256 << endl;
      m_pColors[i] = c.pixel;
      m_pColorNames[i] = new char[16];
      strcpy (m_pColorNames[i], color_list[i]);
    }
}
//--------------------------------------------------------------

void CxDC::SetForeground (unsigned fg)
{
  // Set the foreground color

  if ((fg >= 0) && ( fg < m_nColors))
    {
      m_nForeground = fg;
      XSetForeground (m_pDisplay, m_nGC, m_pColors[fg]);
    }  
}
//---------------------------------------------------------------

void CxDC::SetBackground (unsigned bkg)
{
  // Set the background pixel value of the window

  if ((bkg >= 0) && (bkg < m_nColors))
    {
      XSetWindowBackground (m_pDisplay, m_nWin, m_pColors[bkg]);
    }
}
//--------------------------------------------------------------

CRect CxDC::GetClientRect ()
{
  // Obtain current size of window

  XWindowAttributes a;
  XGetWindowAttributes (m_pDisplay, m_nWin, &a);
  return (CRect(0, 0, a.width, a.height));
  
}
//---------------------------------------------------------------

void CxDC::SetClipRect (CRect r)
{
  // Set the clipping rectangle for output in this window

  XRectangle xr;

  xr.x = r.TopLeft().x;
  xr.y = r.TopLeft().y;
  xr.width = r.Width();
  xr.height = r.Height();

  XSetClipRectangles (m_pDisplay, m_nGC, 0, 0, &xr, 1, 0);
}		
//--------------------------------------------------------------

void CxDC::SetDrawingMode (int x)
{
  // set bitwise logic for drawing operations in the window

  XSetFunction (m_pDisplay, m_nGC, x);
}
//--------------------------------------------------------------

void CxDC::MoveTo (int x, int y)
{
  // set new position for virtual cursor
  
  m_nLastX = x; 
  m_nLastY = y;	
}
//--------------------------------------------------------------

void CxDC::LineTo (int x, int y)
{
// Draw a line from last position to new x, y
	

	XDrawLine (m_pDisplay, m_nWin, m_nGC, m_nLastX, m_nLastY, x, y);
	m_nLastX = x;
	m_nLastY = y;
}
//----------------------------------------------------------------

void CxDC::Rectangle (CRect rect)
{
// A more efficient routine than the one in the base class

	int x1 = rect.TopLeft().x;
	int y1 = rect.TopLeft().y;
	int x2 = rect.BottomRight().x;
	int y2 = rect.BottomRight().y;

	XSetForeground (m_pDisplay, m_nGC, m_pColors[m_nForeground]);
	XSetLineAttributes (m_pDisplay, m_nGC, 0, LineSolid, CapButt, JoinMiter);
	XDrawRectangle (m_pDisplay, m_nWin, m_nGC, x1, y1, x2-x1, y2-y1);
}
//--------------------------------------------------------------

void CxDC::TextOut (int x, int y, char* s)
{
  XDrawString (m_pDisplay, m_nWin, m_nGC, x, y, s, strlen(s));
}
//---------------------------------------------------------------

void CxDC::SetPixel (int x, int y)
{
  XFillRectangle (m_pDisplay, m_nWin, m_nGC, x, y, 1, 1);
}
//---------------------------------------------------------------

CPlotMessage* CxDC::GetPlotMessage()
{
  XNextEvent (m_pDisplay, &m_nReport);
	
  KeySym key;
  XComposeStatus compose;
  char cl [256];
  int* buf;

  CPlotMessage* pMsg = new CPlotMessage;
  pMsg->x = m_nReport.xmotion.x;
  pMsg->y = m_nReport.xmotion.y;
        
  switch (m_nReport.type) 
    {
      case Expose:
        pMsg->message = PL_REDRAW;
	break;
      case DestroyNotify:
        pMsg->message = PL_EXIT;
	break;
      case ConfigureNotify:
        m_nXres = m_nReport.xconfigure.width;
        m_nYres = m_nReport.xconfigure.height;
        pMsg->message = PL_RESIZE;
	break;
      case MotionNotify:
        pMsg->message = PL_CURSORMOVE;
	break;
      case ButtonPress:
        pMsg->message = PL_BUTTONPRESS;
	break;
      case KeyPress:
        pMsg->nParam = 1;
        pMsg->pBuf = new int [2];
        buf = (int *) pMsg->pBuf;
        XLookupString (&m_nReport.xkey, cl, 64, &key, &compose);
        *buf = (int) key;
	pMsg->message = PL_KEYPRESS;
	break;
      default:
        pMsg->message = PL_NONE;	
    }	    

  return pMsg;

// Note: It's up to the recipient  of the message to destroy the message

}
