/*
CxDC.h

  X Windows device context class

  Copyright (c) 1997--2018 Krishna Myneni
  <krishna.myneni@ccreweb.org>

  This software is provided under the terms of the 
  GNU General Public License (GPL), v3.0 or later.

*/

#ifndef __CXDC_H__
#define __CXDC_H__

extern "C"
{
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
}

#include "CDeviceContext.h"

class CxDC : public CDeviceContext
{
	Display* m_pDisplay;
	Window m_nWin;
	Window m_nTermWin;
	GC m_nGC;		// X graphics context
	Colormap m_nMap;
	Cursor m_nCursor;
	XEvent m_nReport;
public:
	CxDC (int, int, int);
	void OpenDisplay (char*);
	void OpenDisplay (Display*, Window w);
	void CloseDisplay (void);
	void ClearDisplay ();
	void SetColors (char**, int);
	unsigned GetColor (char*);
	void SetForeground (unsigned);
	void SetBackground (unsigned);
	CRect GetClientRect ();
	void SetClipRect (CRect);
	void SetDrawingMode (int);
	void Rectangle (CRect);	// override default routine
	void MoveTo (int, int);
	void LineTo (int, int);
	void SetPixel (int, int);
	void TextOut (int, int, char*);
	CPlotMessage* GetPlotMessage ();
};

#endif

