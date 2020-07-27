/*
CDeviceContext.h

  The abstract base class for a graphics device context.

  Copyright (c) 1997--2020 Krishna Myneni
  <krishna.myneni@ccreweb.org>

  This software is provided under the terms of the
  GNU Affero General Public License (AGPL), v 3.0 or later.

*/

#ifndef __CDEVICECONTEXT_H__
#define __CDEVICECONTEXT_H__

#include "wincompat.h"
#include <fstream>
#include "CRect.h"
#include "CPlotMessage.h"

using std::ifstream;
using std::ofstream;

struct TEXTMETRIC
{
  int tmWidth;
  int tmHeight;
  int tmAveCharWidth;
};

enum DeviceType
{
	DEV_NULL,
	DEV_VGA,
	DEV_X_WINDOW,
	DEV_PS,
	DEV_LATEX
};

//  Define type for containing output device information 

class CDeviceContext {
protected:
 	int m_nDevType;         // type of device (0=NULL)
        int m_nMode;            // device mode 
        char* m_szDevName;
        ofstream* m_pFile;	// file stream ptr
        int m_nRows;            // number of rows
        int m_nCols;            // number of columns
        int m_nColors;          // number of colors
        int m_nXres;            // resolution in x
        int m_nYres;            // resolution in y
	TEXTMETRIC m_nTm;       // text metric structure
	unsigned m_nForeground; // foreground color
        int m_nMono;            // monochrome/color flag
        int m_nLastX;
        int m_nLastY;
	// char* m_pColorNames[];   // color names
	char** m_pColorNames;
	// COLORREF m_pRGB[];       // packed RGB values
	COLORREF* m_pRGB;
	// unsigned m_pColors[];    // XColor pixel values
	unsigned* m_pColors;
public:
	bool m_bInverted;       // y coordinate inverted?

	CDeviceContext (int dev_type, int xres, int yres);
	~CDeviceContext ();
	int GetMode () {return m_nMode;}
	void Rectangle (CRect rect);
	void SetClientRect (CRect r) {m_nXres = r.Width(); m_nYres = r.Height();}
	unsigned long GetColor (char*);
	unsigned long GetColor (COLORREF);
	unsigned long GetForeground () {return m_nForeground;}

	virtual CRect GetClientRect () = 0;
	virtual void SetClipRect (CRect) = 0;
	virtual void OpenDisplay (char*) = 0;
	virtual void CloseDisplay () = 0;
	virtual void ClearDisplay () = 0;
	virtual void SetColors (const char* [], int) = 0;
	virtual void SetColors (COLORREF [], const char* [], int ) = 0;
	virtual void SetForeground (unsigned) = 0;
	virtual void SetBackground (unsigned) = 0;
	virtual void SetDrawingMode (int) = 0;
	virtual void SetPixel (int x, int y) = 0;
	virtual void TextOut (int x, int y, char*) = 0;
	virtual CPlotMessage* GetPlotMessage() = 0;	        
	virtual void MoveTo (int x, int y) = 0; 
	virtual void LineTo (int x, int y) = 0;
	void GetTextMetrics (TEXTMETRIC* p) {*p = m_nTm;}
};


#endif
