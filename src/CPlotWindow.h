/*
CPlotWindow.h

  Header file for the CPlotWindow class

  Copyright (c) 1996--2024 Krishna Myneni
  <krishna.myneni@ccreweb.org>

  This software is provided under the terms of the 
  GNU Affero General Public License (AGPL), v 3.0 or later.
*/

#ifndef __CPLOTWINDOW_H__
#define __CPLOTWINDOW_H__

#include "CDatabase.h"
#include "CPlotDisplay.h"
#include "CxDC.h"
#include "CpsDC.h"
#include <Xm/Xm.h>
#include <Xm/Label.h>
#include <Xm/Form.h>
#include <Xm/RowColumn.h>
#include <Xm/CascadeB.h>
#include <Xm/PushB.h>
#include <Xm/MessageB.h>
#include <Xm/SelectioB.h>
#include <Xm/FileSB.h>
#include <Xm/DrawingA.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/List.h>
#include <Xm/ToggleB.h>
#include <Xm/ToggleBG.h>
#include <Xm/SeparatoG.h>
#include <Xm/DialogS.h>

// #include "CInputDialog.h"
// #include "CPickDataDialog.h"
// #include "CGridDialog.h"
// #include "CHeaderDialog.h"
// #include "CVerifyDialog.h"
#include "CSaveOptionsDialog.h"
#include "CXyPlot.h"
#include "CXyFile.h"
#include "resource.h"

class CPlotWindow {
	CDeviceContext* m_pDc;
	char m_szBackgroundColor[16];
	char m_szForegroundColor[16];
public:
	CSaveOptionsDialog* m_pSaveOptionsDialog;
	Widget m_nPlotWindow;
	Widget m_nForm;
	Widget m_nMenuBar;
	Widget m_nStatusBar;
	Widget m_nCoordinates;
	Widget m_nForthShell;
	Widget m_nFileMenu;
	Widget m_nEditMenu;
	Widget m_nDataMenu;
	Widget m_nPlotMenu;
	Widget m_nViewMenu;
	Widget m_nSymbolMenu;
	Widget m_nMathMenu;
	Widget m_nHelpMenu;
	Widget m_nFileOpenDialog;
	Widget m_nFileSaveDialog;
	Widget m_nAboutBox;
	Widget m_nRadioBox;
	Widget m_nInputDialog;
	Widget m_nHeaderDialog;
	Widget m_nDatasetName;
	Widget m_nHeaderText;
	Widget m_nHeaderMessage;
	Widget m_nPickDialog;
	Widget m_nGridDialog;
	Widget m_nGridXaxis;
	Widget m_nGridYaxis;
	Widget m_nGridXlines;
	Widget m_nGridYlines;
	Widget m_nGridXtics;
	Widget m_nGridYtics;
	Widget m_nSymbolDialog;
	Widget m_nColorDialog;
	Widget m_nVerifyDialog;

        CPlotWindow(int argc, char* argv[]);
        ~CPlotWindow();
	Widget AddMenuItem (Widget, char*);
	Widget AddSubMenu (Widget, char*);
	Widget MakeMenu (Widget, char*, char**, Widget*, int);
	CDeviceContext* GetDC () {return m_pDc;}
	void SetBackgroundColor (char*);
	void SetForegroundColor (char*);
	unsigned long GetBackgroundColor () { return GetColor(m_szBackgroundColor); }
	unsigned long GetForegroundColor () { return GetColor(m_szForegroundColor); }
	unsigned long GetColor (char* szName) {return m_pDc->GetColor(szName);}
	unsigned long GetColor (COLORREF cr) { return m_pDc->GetColor(cr); }
	int GetWindowText (char*, int);
	void SetWindowText (char*);
	CPlotMessage* GetPlotMessage();

        void OnPaint ();
	void OnPrint ();
        void OnMouseMove (unsigned int, CPoint);
        void OnLButtonDown (unsigned int, CPoint);
        void OnLButtonUp (unsigned int, CPoint);
        void OnRButtonDown (unsigned int, CPoint);
	void SelectPlot (int, int);
	void OnResize ();
        bool OnFileNew ();
	bool OnFileOpen ();
        bool OnFileSave ();
	void LoadDatasetFile (char*, char*);
	bool LoadFile (char*);
	bool SaveFile (char*);
	bool SaveWorkspace (char*);
        void OnCopyBitmap ();
        void OnCopyMetafile ();
        void OnHeader ();
	void OnSetHeader (char*);
	void OnSetDatasetName (char*);
        void OnDelete ();
        void OnDuplicate ();
        void OnView2DCartesian ();
        void OnView2DPolar ();
// void OnView3DCartesian ();
        void OnAspect ();
// void OnRotate ();
        void OnRefresh ();
        void OnBack ();
        void OnForward ();
        void OnDeleteView ();
        void OnGrid ();
	void OnGridToggle();
        void OnPick ();
	void OnPickData ();
        void OnDrop ();
        void OnColor ();
	void OnSetColor ();
	void OnSymbol ();
	void OnSetSymbol ();
        void OnSymbolLine ();
        void OnSymbolPoint ();
        void OnSymbolLinePt ();
        void OnSymbolStick ();
	void OnSymbolToggle ();
	void OnExpressionInput ();
        void OnExpression (char*);
        void OnAbout ();
        void OnReset ();
	void OnExtremaInput ();
        void OnExtrema (char*);
	void Invalidate ();
	void WriteStatusMessage (char*);
	void WriteConsoleMessage (const char*);
	void MessageBox (const char*);

        int GetInput (char*, char*);
        void SelectColumns (char*, int);
	Symbol GetSymbol (char* name);
	void Clear() {m_pDc->ClearDisplay();}
	vector<int> ParseColumns (char*);
        bool SaveMetafile (char*);

        // member variables

	char* m_pFileName;      // current file opened 
        CDatabase *m_pDb;       // pointer to database object
        CPlotDisplay *m_pDi;    // pointer to display object
};

#endif
