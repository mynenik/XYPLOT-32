/*
CpsDC.h

  Postscript device context

  Copyright (c) 1999--2018 Krishna Myneni
  <krishna.myneni@ccreweb.org>

  This software is provided under the terms of the
  GNU General Public License (GPL), v3.0 or later.

*/

#ifndef __CPSDC_H__
#define __CPSDC_H__

#define MAX_PS_COLORS 64

#include "CDeviceContext.h"

class CpsDC : public CDeviceContext
{
  float m_fRGB [MAX_PS_COLORS][3];
public:
	CpsDC (int, int);
	void OpenDisplay (char*);
	void CloseDisplay (void);
	void ClearDisplay ();
	void SetColors (char**, int);
	void SetForeground (unsigned);
	void SetBackground (unsigned);
	CRect GetClientRect ();
	void SetClipRect (CRect);
	void SetDrawingMode (int);
	void Rectangle (CRect);	// override default routine
	void MoveTo (int, int); // override default routine
	void LineTo (int, int);
	void SetPixel (int, int);
	void TextOut (int, int, char*);
	CPlotMessage* GetPlotMessage ();
};

#endif
