// CPlotWindow.cpp
//
// Copyright 1996--2026 Krishna Myneni 
// <krishna.myneni@ccreweb.org>
//
// This software is provided under the terms of the 
// GNU Affero General Public License (AGPL), v3.0 or later.
//

#ifdef COPYRIGHT
const char* copyright=COPYRIGHT;
#else
const char* copyright="1999--2026"
#endif

#ifdef VERSION
const char* ver=VERSION;
#else
const char* ver="?";
#endif

#ifdef KFORTH_VER
const char* kfver=KFORTH_VER;
#else
const char* kfver="?";
#endif

const char* build=BUILD_DATE;

#include "consts.h"
#include <math.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <stdlib.h>
using namespace std;
#include "CPlotWindow.h"
#include "ColorTable.h"
#include "fbc.h"
#include "ForthCompiler.h"
#include "ForthVM.h"

#define PRINT_TEMP_FILE "xyplot-output.ps"

// Functions and data defined externally

extern void SortRect (CRect *);
extern int NumberParse (double*, char*);
extern int CompileAE (vector<byte>*, char* exp);
extern "C" char* strupr (char*);
extern int AddToHeader (char*, char*, bool);
extern int LoadForthFile (char*);
extern char sz_CurrentWorkingDir[];

extern void FileMenuCB (Widget, void*, void*);
extern void EditMenuCB (Widget, void*, void*);
extern void PlotMenuCB (Widget, void*, void*);
extern void MathMenuCB (Widget, void*, void*);
extern void ForthShellVerifyCB (Widget, void*, void*);
extern void ForthCB (Widget, void*, void*);
extern void RadioToggledCB (Widget, void*, void*);

extern Widget TopLevel;
extern volatile int verify_answer;
extern XtAppContext xapp;

// Global data

const char* SymbolNames[7] =
{
  "LINE", "DASHED", "POINT", "BIG_POINT", 
  "LINE_PLUS_POINT", "STICK", "HISTOGRAM"};

// Global menu data

const char* FileOptions[16] = 
  {"New", "Open", "Save", "Save Options", 
   "Print", "Exit", "", "",
   "", "", "", "",
   "", "", "", ""};
Widget FileWidgets[16];

const char* EditOptions[16] = 
  {"Header", "Delete", "Duplicate", "",
    "", "", "", "",
    "", "", "", "",
    "", "", "", ""};
Widget EditWidgets[16];

const char* PlotOptions[16] = 
  { ">View", "Extrema", "Pick", "Drop", 
    "Grid", "Color", "Symbol", "",
    "", "", "", "",
    "", "", "", "" };
Widget PlotWidgets[16];

const char* MathOptions[16] = 
  { 
    "Expression", "", "", "", 
    "", "", "", "",
    "", "", "", "",
    "", "", "", ""};
Widget MathWidgets[16];

const char* HelpOptions[16] = 
  {"About", "", "", "",
   "", "", "", "",
   "", "", "", "",
   "", "", "", "" };
Widget HelpWidgets[16];

// Widget ViewWidgets[5];
// char* ViewOptions[] = {"Cartesian", "Polar", "Aspect",
//  "Back", "Forward"};

// Fonts to be used globally

XmFontListEntry FontEntries[8];
XmFontList FontList;

//---------------------------------------------------------------

CPlotWindow::CPlotWindow(int argc, char* argv[])
{
  Arg al[10];
  int ac;
  Display* disp = XtDisplay(TopLevel);
  m_pFileName = new char[512];
  *m_pFileName = '\0';
  m_pDb = NULL;
  m_pDi = NULL;

  CxDC* pDc = new CxDC (DEV_X_WINDOW, 800, 640);

  // Setup the fonts

  char s1[128], s2[128];  // generic strings for copying string constants 
  FontEntries[0] = XmFontListEntryLoad (disp,
		 strcpy(s1, "-*-courier-*-r-*--*-120-*"), 
		 XmFONT_IS_FONT, 
		 strcpy(s2, "Courier12pt"));
  FontEntries[1] = XmFontListEntryLoad (disp,
		 strcpy(s1, "-*-courier-bold-o-*--*-140-*"), 
		 XmFONT_IS_FONT, 
		 strcpy(s2, "CourierBold14pt"));
  FontEntries[2] = XmFontListEntryLoad (disp, 
		  strcpy(s1, "-*-helvetica-*-r-*--*-120-*"),
		  XmFONT_IS_FONT,
		  strcpy(s2, "Helvetica12pt"));
  FontEntries[3] = XmFontListEntryLoad (disp, 
		  strcpy(s1, "-*-helvetica-*-r-*--*-140-*"),
		  XmFONT_IS_FONT, 
		  strcpy(s2, "Helvetica14pt"));
  FontList = XmFontListAppendEntry (NULL, FontEntries[0]);
  for (ac = 1; ac < 4; ac++)
    FontList = XmFontListAppendEntry (FontList, FontEntries[ac]);

  for (ac = 0; ac < 4; ac++) XmFontListEntryFree (&FontEntries[ac]);

  XtVaSetValues( TopLevel,
	XmNfontList, FontList,
  	XmNheight, 640,
	XmNwidth, 800,
	NULL);

  // Create a form widget

  m_nForm = XtVaCreateManagedWidget( "form", 
	xmFormWidgetClass, TopLevel,
	XmNfontList, FontList,
       	NULL );

  // Create a menu bar and attach it to the form.
  m_nMenuBar = XmCreateMenuBar(m_nForm, strcpy(s1,"Menu Bar"), NULL, 0);
  XtVaSetValues( m_nMenuBar,
	XmNtopAttachment, XmATTACH_FORM,
	XmNrightAttachment, XmATTACH_FORM,
	XmNleftAttachment, XmATTACH_FORM,
	NULL);
  XtManageChild(m_nMenuBar);

  // Create the Forth command shell/console dialog

  ac = 0;
  XtSetArg(al[ac], XmNeditMode, XmMULTI_LINE_EDIT);
  ac++;
  XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM);
  ac++;
  XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_FORM);
  ac++;
  XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM);
  ac++;

  m_nForthShell = XmCreateScrolledText(m_nForm, strcpy(s1,"Forth Shell"),
    al, ac);
  XtVaSetValues( m_nForthShell,
	XmNfontList, FontList,
	XmNheight, 100,
	XtVaTypedArg, XmNbackground, XmRString, "White", 6,
       	NULL);
  XtManageChild(m_nForthShell);

  XmString xstr1;

  // Create the status message label widget and attach it below the
  //   drawing area (plot window)
  xstr1 = XmStringCreate(strcpy(s1, " "), XmSTRING_DEFAULT_CHARSET);

  m_nStatusBar = XtVaCreateManagedWidget("status_bar",
    xmLabelWidgetClass, m_nForm,
    XmNlabelString, xstr1,
    XmNalignment, XmALIGNMENT_BEGINNING,
    XmNleftAttachment, XmATTACH_FORM,
    XmNbottomAttachment, XmATTACH_WIDGET,
    XmNbottomWidget, XtParent(m_nForthShell),
//    XmNfontList, FontList,
    NULL);
  
  XmStringFree(xstr1);

  // Create coordinates label widget and attach it to the right of the
  //  status bar
  xstr1 = XmStringCreate(strcpy(s1, " "),XmSTRING_DEFAULT_CHARSET); 

  m_nCoordinates = XtVaCreateManagedWidget("coordinates",
    xmLabelWidgetClass, m_nForm,
    XmNlabelString, xstr1,
    XmNrightAttachment, XmATTACH_FORM,
    XmNbottomAttachment, XmATTACH_WIDGET,
    XmNbottomWidget, XtParent(m_nForthShell),
    XmNfontList, FontList,
    NULL);

  XmStringFree(xstr1);

  // Create the drawing area and attach it below the menu bar
  // and above the status bar

  m_nPlotWindow = XtVaCreateManagedWidget( "Plot Window",
	xmDrawingAreaWidgetClass, m_nForm,
	XmNtopAttachment, XmATTACH_WIDGET,
	XmNtopWidget, m_nMenuBar,
	XmNrightAttachment, XmATTACH_FORM,
	XmNleftAttachment, XmATTACH_FORM,
	XmNbottomAttachment, XmATTACH_WIDGET,
	XmNbottomWidget, m_nStatusBar,
	XmNfontList, FontList,
	XmNheight, 400,
	XmNwidth, 800,
	NULL);

  pDc->OpenDisplay(XtDisplay(m_nPlotWindow), XtWindow(m_nPlotWindow));
  m_pDc = pDc;

  // Setup the plot colors

  m_pDc->SetColors(colors_rgb_table, color_names, MAX_COLORS);
  SetBackgroundColor(strcpy(s1,"DarkGrey"));
  SetForegroundColor(strcpy(s1,"black"));


  // Make the menus

  for (int j = 0; j < 16; j++)
    {
      FileWidgets[j] = 0;
      EditWidgets[j] = 0;
      PlotWidgets[j] = 0;
      MathWidgets[j] = 0;
      HelpWidgets[j] = 0;
    }

  m_nFileMenu = MakeMenu (m_nMenuBar, strcpy(s1, "File"),
		 (char**) FileOptions, FileWidgets, 6);

  m_nEditMenu = MakeMenu (m_nMenuBar, strcpy(s1, "Edit"), 
		 (char**) EditOptions, EditWidgets, 3);

  m_nPlotMenu = MakeMenu (m_nMenuBar, strcpy(s1, "Plot"),
		 (char**) PlotOptions, PlotWidgets, 7);

  m_nMathMenu = MakeMenu (m_nMenuBar, strcpy(s1, "Math"),
		 (char**) MathOptions, MathWidgets, 1);

  m_nHelpMenu = MakeMenu (m_nMenuBar, strcpy(s1, "Help"),
		 (char**) HelpOptions, HelpWidgets, 1);

  // Make the submenus

  //  m_nViewMenu = MakeMenu (PlotWidgets[ID_VIEW], 
  //   strcpy(s1, "View"), ViewOptions,
  //   ViewWidgets, 5); 

  // Create the file open dialog

  Widget temp;

  strcpy(s1, "Open File");
  m_nFileOpenDialog = XmCreateFileSelectionDialog(TopLevel,
     s1, NULL, 0);
  xstr1 = XmStringCreateLocalized(s1);
  XtVaSetValues( m_nFileOpenDialog,
	XmNdialogTitle, xstr1,
	NULL);
  XmStringFree(xstr1);

  temp = XmFileSelectionBoxGetChild(m_nFileOpenDialog,XmDIALOG_FILTER_LABEL);
  XtVaSetValues( temp,
	XmNfontList, FontList,
       	NULL);

  temp = XmFileSelectionBoxGetChild(m_nFileOpenDialog,XmDIALOG_FILTER_TEXT);
  XtVaSetValues( temp,
	XmNfontList, FontList,
	XtVaTypedArg, XmNbackground, XmRString, "White", 6,
       	NULL);

  temp = XmFileSelectionBoxGetChild(m_nFileOpenDialog,XmDIALOG_LIST_LABEL);
  XtVaSetValues( temp,
	XmNfontList, FontList,
	NULL);

  temp = XmFileSelectionBoxGetChild(m_nFileOpenDialog,XmDIALOG_LIST);
  XtVaSetValues( temp,
	XmNfontList, FontList,
	NULL);

  temp = XmFileSelectionBoxGetChild(m_nFileOpenDialog,XmDIALOG_DIR_LIST_LABEL);
  XtVaSetValues( temp,
	XmNfontList, FontList,
	NULL);

  temp = XmFileSelectionBoxGetChild(m_nFileOpenDialog,XmDIALOG_DIR_LIST);
  XtVaSetValues( temp,
	XmNfontList, FontList,
	XmNwidth,400,
	NULL);

  temp = XmFileSelectionBoxGetChild(m_nFileOpenDialog,XmDIALOG_SELECTION_LABEL);
  XtVaSetValues( temp,
	XmNfontList, FontList,
	NULL);

  temp = XmFileSelectionBoxGetChild(m_nFileOpenDialog,XmDIALOG_TEXT);
  XtVaSetValues( temp,
	XmNfontList, FontList,
	XmNwidth, 400,
	XtVaTypedArg, XmNbackground, XmRString, "White", 6,
	NULL);

  temp = XmFileSelectionBoxGetChild(m_nFileOpenDialog,XmDIALOG_OK_BUTTON);
  XtVaSetValues( temp,
        XmNfontList, FontList,
        NULL);

  temp = XmFileSelectionBoxGetChild(m_nFileOpenDialog,XmDIALOG_OK_BUTTON);
  XtVaSetValues( temp,
        XmNfontList, FontList,
        NULL);
  temp = XmFileSelectionBoxGetChild(m_nFileOpenDialog,XmDIALOG_APPLY_BUTTON);
  XtVaSetValues( temp,
        XmNfontList, FontList,
        NULL);
  temp = XmFileSelectionBoxGetChild(m_nFileOpenDialog,XmDIALOG_CANCEL_BUTTON);
  XtVaSetValues( temp,
        XmNfontList, FontList,
        NULL);

  XtUnmanageChild(XmSelectionBoxGetChild(m_nFileOpenDialog,
    XmDIALOG_HELP_BUTTON));

  // Create the file save dialog

  strcpy(s1, "Save File");
  m_nFileSaveDialog = XmCreateFileSelectionDialog(TopLevel,
     s1, NULL, 0);
  xstr1 = XmStringCreateLocalized(s1);
  XtVaSetValues( m_nFileSaveDialog,
	XmNdialogTitle, xstr1,
	NULL);
  XmStringFree(xstr1);

  temp = XmFileSelectionBoxGetChild(m_nFileSaveDialog,XmDIALOG_FILTER_LABEL);
  XtVaSetValues( temp,
        XmNfontList, FontList,
        NULL);

  temp = XmFileSelectionBoxGetChild(m_nFileSaveDialog,XmDIALOG_FILTER_TEXT);
  XtVaSetValues( temp,
	XmNfontList, FontList,
	XtVaTypedArg, XmNbackground, XmRString, "White", 6,
       	NULL);

  temp = XmFileSelectionBoxGetChild(m_nFileSaveDialog,XmDIALOG_LIST_LABEL);
  XtVaSetValues( temp,
        XmNfontList, FontList,
        NULL);

  temp = XmFileSelectionBoxGetChild(m_nFileSaveDialog,XmDIALOG_LIST);
  XtVaSetValues( temp,
	XmNfontList, FontList,
	NULL);

  temp = XmFileSelectionBoxGetChild(m_nFileSaveDialog,XmDIALOG_DIR_LIST_LABEL);
  XtVaSetValues( temp,
        XmNfontList, FontList,
        NULL);

  temp = XmFileSelectionBoxGetChild(m_nFileSaveDialog,XmDIALOG_DIR_LIST);
  XtVaSetValues( temp,
	XmNfontList, FontList,
	NULL);

  temp = XmFileSelectionBoxGetChild(m_nFileSaveDialog,XmDIALOG_SELECTION_LABEL);
  XtVaSetValues( temp,
        XmNfontList, FontList,
        NULL);

  temp = XmFileSelectionBoxGetChild(m_nFileSaveDialog,XmDIALOG_TEXT);
  XtVaSetValues( temp,
	XmNfontList, FontList,
	XtVaTypedArg, XmNbackground, XmRString, "White", 6,
	NULL);

  temp = XmFileSelectionBoxGetChild(m_nFileSaveDialog,XmDIALOG_OK_BUTTON);
  XtVaSetValues( temp,
        XmNfontList, FontList,
        NULL);
  temp = XmFileSelectionBoxGetChild(m_nFileSaveDialog,XmDIALOG_APPLY_BUTTON);
  XtVaSetValues( temp,
        XmNfontList, FontList,
        NULL);
  temp = XmFileSelectionBoxGetChild(m_nFileSaveDialog,XmDIALOG_CANCEL_BUTTON);
  XtVaSetValues( temp,
        XmNfontList, FontList,
        NULL);

  XtUnmanageChild(XmSelectionBoxGetChild(m_nFileSaveDialog,
    XmDIALOG_HELP_BUTTON));

  // Create a dual purpose About/Message box

  strcpy(s1, "About XYPLOT");
  m_nAboutBox = XmCreateInformationDialog(TopLevel, s1, NULL, 0);
  xstr1 = XmStringCreateLocalized(s1);
  XtVaSetValues( m_nAboutBox,
	XmNdialogTitle, xstr1,
        XtVaTypedArg, XmNbackground, XmRString, "LightGrey", 10,
	NULL);
  XmStringFree(xstr1);

//  temp = XmMessageBoxGetChild( m_nAboutBox, XmDIALOG_TEXT);
//  XtVaSetValues( temp,
//	XmNfontList, FontList,
//	NULL);

  temp = XmMessageBoxGetChild (m_nAboutBox, XmDIALOG_CANCEL_BUTTON);
  XtUnmanageChild(temp);

  temp = XmMessageBoxGetChild (m_nAboutBox, XmDIALOG_HELP_BUTTON);
  XtUnmanageChild(temp);

  // Create a general purpose radio box

  m_nRadioBox = XmVaCreateSimpleRadioBox (TopLevel, 
	strcpy(s1, "radio_box"), 0,
	RadioToggledCB, XmVaRADIOBUTTON,
	XmStringCreateLocalized(strcpy(s2,"one")),
	NULL, NULL, NULL, NULL);

  // XtManageChild(m_nRadioBox);

  // Create the input dialog

  ac = 0;
  XtSetArg(al[ac], XmNselectionLabelString, 
	XmStringCreateLtoR(strcpy(s1, "Enter input"),
          strcpy(s2, "Courier12pt")));  
  ac++;
  XtSetArg (al[ac], XmNfontList, FontList);
  ac++;

  strcpy(s1, "Input Dialog");
  m_nInputDialog = XmCreatePromptDialog(TopLevel, s1, al, ac);
  xstr1 = XmStringCreateLocalized(s1);
  temp = XmSelectionBoxGetChild(m_nInputDialog, XmDIALOG_TEXT);
  XtVaSetValues( m_nInputDialog,
	XmNdialogTitle, xstr1,
	XmNinitialFocus, temp,
	NULL);
  XmStringFree(xstr1);

  XtVaSetValues( temp,
	XmNfontList, FontList, 
	XtVaTypedArg, XmNbackground, XmRString, "White", 6,
       	NULL);

  XtUnmanageChild(XmSelectionBoxGetChild(m_nInputDialog, 
    XmDIALOG_HELP_BUTTON));


  // Create the header view/edit dialog

  strcpy(s1, "Dataset Header");
  m_nHeaderDialog = XmCreateFormDialog(TopLevel, s1, NULL, 0);
  xstr1 = XmStringCreateLocalized(s1);
  XtVaSetValues( m_nHeaderDialog,
	XmNdialogTitle, xstr1,
	NULL);
  XmStringFree(xstr1);

  Widget label1 = XtVaCreateManagedWidget(
	"Dataset Name:", 
	xmLabelWidgetClass, m_nHeaderDialog,
	XmNtopAttachment, XmATTACH_FORM,
	XmNleftAttachment, XmATTACH_FORM,
	XmNfontList, FontList,
	NULL);

  XtManageChild(label1);

  m_nDatasetName = XmCreateTextField(m_nHeaderDialog, 
	strcpy(s1, "text"), NULL, 0);

  XtVaSetValues (m_nDatasetName, 
	XmNeditable, True, 
	XmNmaxLength, 256, 
	XmNvalue, "",
	XmNtopAttachment, XmATTACH_WIDGET,
	XmNtopWidget, label1,
	XmNleftAttachment, XmATTACH_FORM,
	XmNwidth, 400,
	XmNfontList, FontList,
	NULL );
  XtManageChild(m_nDatasetName);

  Widget label2 = XtVaCreateManagedWidget(
	"Dataset Header:", 
	xmLabelWidgetClass, m_nHeaderDialog,
	XmNtopAttachment, XmATTACH_WIDGET,
	XmNtopWidget, m_nDatasetName,
	XmNfontList, FontList,
	NULL );
  XtManageChild(label2);

  ac = 0;
  XtSetArg(al[ac], XmNeditMode, XmMULTI_LINE_EDIT); 
  ac++;
  XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); 
  ac++;
  XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM);
  ac++;
  XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM);
  ac++;
  XtSetArg(al[ac], XmNtopWidget, label2);
  ac++;
  XtSetArg(al[ac], XmNheight, 200);
  ac++;
  XtSetArg(al[ac], XmNwidth, 400);
  ac++;
  XtSetArg(al[ac], XmNfontList, FontList);
  ac++;

  m_nHeaderText = XmCreateScrolledText(m_nHeaderDialog,
    strcpy(s1, "header_text"), al, ac);

  XtManageChild(m_nHeaderText);

  m_nHeaderMessage = XmCreateMessageBox (m_nHeaderDialog, 
    strcpy(s1, "header_message"), NULL, 0);

  XtVaSetValues( m_nHeaderMessage,
	XmNtopAttachment, XmATTACH_WIDGET,
	XmNtopWidget, m_nHeaderText,
	XmNleftAttachment, XmATTACH_FORM,
	XmNrightAttachment, XmATTACH_FORM,
	XmNfontList, FontList,
	NULL);
  
  temp = XmMessageBoxGetChild(m_nHeaderMessage, XmDIALOG_OK_BUTTON);
  XtVaSetValues( temp, XmNfontList, FontList, NULL );
  temp = XmMessageBoxGetChild(m_nHeaderMessage, XmDIALOG_CANCEL_BUTTON);
  XtVaSetValues( temp, XmNfontList, FontList, NULL );

  XtUnmanageChild(XmMessageBoxGetChild(m_nHeaderMessage, 
    XmDIALOG_HELP_BUTTON));

  XtManageChild(m_nHeaderMessage);


  // Create the Pick dialog

  strcpy(s1, "Pick Dataset");
  m_nPickDialog = XmCreateSelectionDialog(TopLevel, s1, NULL, 0);
  xstr1 = XmStringCreateLocalized(s1);
  XtVaSetValues( m_nPickDialog,
	XmNdialogTitle, xstr1,
	NULL);
  XmStringFree(xstr1);

  strcpy(s1, "Datasets");
  xstr1 = XmStringCreateLocalized(s1);
  temp = XmSelectionBoxGetChild(m_nPickDialog, XmDIALOG_LIST_LABEL);
  XtVaSetValues( temp,
	XmNlabelString, xstr1,
	XmNfontList, FontList,
	NULL);
  XmStringFree(xstr1);

  temp = XmSelectionBoxGetChild(m_nPickDialog, XmDIALOG_SELECTION_LABEL);
  XtVaSetValues( temp,
	XmNfontList, FontList,
	NULL);

  temp = XmSelectionBoxGetChild(m_nPickDialog, XmDIALOG_LIST);
  XtVaSetValues( temp,
	XmNfontList, FontList, 
       	NULL);

  temp = XmSelectionBoxGetChild(m_nPickDialog, XmDIALOG_TEXT);
  XtVaSetValues( temp,
	XmNfontList, FontList, 
	XtVaTypedArg, XmNbackground, XmRString, "White", 6,
       	NULL);

  temp = XmSelectionBoxGetChild(m_nPickDialog, XmDIALOG_OK_BUTTON);
  XtVaSetValues( temp,
	XmNfontList, FontList,
	NULL);

  temp = XmSelectionBoxGetChild(m_nPickDialog, XmDIALOG_CANCEL_BUTTON);
  XtVaSetValues( temp,
	XmNfontList, FontList,
	NULL);

  XtUnmanageChild(XmSelectionBoxGetChild(m_nPickDialog,
    XmDIALOG_APPLY_BUTTON));
  XtUnmanageChild(XmSelectionBoxGetChild(m_nPickDialog, 
    XmDIALOG_HELP_BUTTON));

  // Create the Symbols dialog

  strcpy(s1, "Symbols");
  m_nSymbolDialog = XmCreateSelectionDialog (TopLevel, s1, NULL, 0);
  xstr1 = XmStringCreateLocalized(s1);
  XtVaSetValues( m_nSymbolDialog, XmNdialogTitle, xstr1, NULL);
  XmStringFree(xstr1);

  temp = XmSelectionBoxGetChild(m_nSymbolDialog, XmDIALOG_LIST_LABEL);
  strcpy(s1, "Plot Symbols");
  xstr1 = XmStringCreateLocalized(s1);
  XtVaSetValues( temp,
	XmNlabelString, xstr1,
	XmNfontList, FontList,
	NULL);
  XmStringFree(xstr1);
 
  temp = XmSelectionBoxGetChild(m_nSymbolDialog, XmDIALOG_LIST);
  XtVaSetValues( temp, XmNfontList, FontList, NULL);

  temp = XmSelectionBoxGetChild(m_nSymbolDialog, XmDIALOG_SELECTION_LABEL);
  XtVaSetValues( temp, XmNfontList, FontList, NULL);

  temp = XmSelectionBoxGetChild(m_nSymbolDialog, XmDIALOG_TEXT);
  XtVaSetValues( temp,
	XmNfontList, FontList, 
	XtVaTypedArg, XmNbackground, XmRString, "White", 6,
       	NULL);

  temp = XmSelectionBoxGetChild(m_nSymbolDialog, XmDIALOG_APPLY_BUTTON);
  XtVaSetValues( temp, XmNfontList, FontList, NULL );
  temp = XmSelectionBoxGetChild(m_nSymbolDialog, XmDIALOG_CANCEL_BUTTON);
  XtVaSetValues( temp, XmNfontList, FontList, NULL );

  XtUnmanageChild(XmSelectionBoxGetChild(m_nSymbolDialog, 
    XmDIALOG_HELP_BUTTON));
  XtUnmanageChild(XmSelectionBoxGetChild(m_nSymbolDialog,
    XmDIALOG_OK_BUTTON));

  temp = XmSelectionBoxGetChild(m_nSymbolDialog, XmDIALOG_LIST);
  for (int i = 0; i < 7; i++) {
      XmListAddItem(temp, XmStringCreate((char*) SymbolNames[i], 
	    XmSTRING_DEFAULT_CHARSET), 0);      
  }

  // Create the Colors dialog
  strcpy(s1, "Colors");
  m_nColorDialog = XmCreateSelectionDialog (TopLevel, s1, NULL, 0);
  xstr1 = XmStringCreateLocalized(s1);
  XtVaSetValues( m_nColorDialog, XmNdialogTitle, xstr1, NULL);
  XmStringFree(xstr1);

  temp = XmSelectionBoxGetChild(m_nColorDialog, XmDIALOG_LIST_LABEL);
  strcpy(s1, "Plot Colors");
  xstr1 = XmStringCreateLocalized(s1);
  XtVaSetValues( temp,
	XmNlabelString, xstr1,
	XmNfontList, FontList,
	NULL);
  XmStringFree(xstr1);

  temp = XmSelectionBoxGetChild(m_nColorDialog, XmDIALOG_LIST);
  XtVaSetValues( temp, XmNfontList, FontList, NULL);

  temp = XmSelectionBoxGetChild(m_nColorDialog, XmDIALOG_SELECTION_LABEL);
  XtVaSetValues( temp, XmNfontList, FontList, NULL);

  temp = XmSelectionBoxGetChild(m_nColorDialog, XmDIALOG_TEXT);
  XtVaSetValues(temp,
    XmNfontList, FontList, 
    XtVaTypedArg, XmNbackground, XmRString, "White", 6,
    NULL);

  temp = XmSelectionBoxGetChild(m_nColorDialog, XmDIALOG_APPLY_BUTTON);
  XtVaSetValues( temp, XmNfontList, FontList, NULL);
  temp = XmSelectionBoxGetChild(m_nColorDialog, XmDIALOG_CANCEL_BUTTON);
  XtVaSetValues( temp, XmNfontList, FontList, NULL );
  
  XtUnmanageChild(XmSelectionBoxGetChild(m_nColorDialog, 
    XmDIALOG_HELP_BUTTON));
  XtUnmanageChild(XmSelectionBoxGetChild(m_nColorDialog,
    XmDIALOG_OK_BUTTON));

  temp = XmSelectionBoxGetChild(m_nColorDialog, XmDIALOG_LIST);
  for (int i = 0; i < MAX_COLORS; i++) {
      XmListAddItem(temp, XmStringCreateLocalized((char*) color_names[i]), 0);   }

  // Create the Save Options dialog
 
  m_pSaveOptionsDialog = new CSaveOptionsDialog(TopLevel);

  // Create the Grid dialog

  Widget rowcol, label, btn;
  XmString one;
  
  strcpy( s1, "Grid Options");
  m_nGridDialog = XmCreateFormDialog(TopLevel, s1, NULL, 0);
  xstr1 = XmStringCreateLocalized(s1);
  XtVaSetValues( m_nGridDialog,
	XmNdialogTitle, xstr1,
	NULL);
  XmStringFree(xstr1);

  rowcol = XtVaCreateManagedWidget ("rowcolumn", 
	xmRowColumnWidgetClass, m_nGridDialog,
	XmNnumColumns, 2,
	NULL);

  label = XtVaCreateManagedWidget("Axes Lines",
	xmLabelWidgetClass, rowcol,
	XmNfontList, FontList,
	NULL);

  m_nGridXaxis = XmCreateToggleButton(rowcol, 
	strcpy(s1, "X Axis"), NULL, 0);

  XtVaSetValues (m_nGridXaxis, 
	XmNset, True,
	NULL);

  m_nGridYaxis = XmCreateToggleButton(rowcol, 
	strcpy(s1, "Y Axis"), NULL, 0);

  XtVaSetValues (m_nGridYaxis, 
	XmNset, True, 
	NULL);

  XtVaCreateManagedWidget ("sep",
        xmSeparatorGadgetClass, rowcol,
        NULL);

  XtVaCreateManagedWidget ("Grid Lines",
        xmLabelWidgetClass, rowcol,
        XmNfontList, FontList,
        NULL);

  strcpy(s1, "Horizontal");
  m_nGridYlines = XmCreateToggleButton(rowcol, s1, NULL, 0);

  XtVaSetValues (m_nGridYlines, 
	XmNset, True,
	NULL);

  strcpy(s1, "Vertical");
  m_nGridXlines = XmCreateToggleButton(rowcol, s1, NULL, 0);


  XtVaSetValues (m_nGridXlines, 
	XmNset, True, 
	NULL);

  XtVaCreateManagedWidget ("sep",
        xmSeparatorGadgetClass, rowcol,
        NULL);
 
  XtVaCreateManagedWidget ("# X Tics:", 
	xmLabelWidgetClass, rowcol,
	XmNfontList, FontList,	
	NULL);

  m_nGridXtics = XmCreateTextField(rowcol, 
	strcpy(s1, "text1"), NULL, 0);

  XtVaSetValues( m_nGridXtics, 
	XmNeditable, True, 
	XmNmaxLength, 4, 
	XmNvalue, "10",
	XmNwidth, 80,
       	XmNfontList, FontList,
	XtVaTypedArg, XmNbackground, XmRString, "White", 6,
	NULL );

  XtVaCreateManagedWidget ("# Y Tics:", 
	xmLabelWidgetClass, rowcol,
	XmNfontList, FontList,
	NULL);

  m_nGridYtics = XmCreateTextField(rowcol, 
	strcpy(s1, "text2"), NULL, 0);

  XtVaSetValues( m_nGridYtics, 
	XmNeditable, True, 
	XmNmaxLength, 4, 
	XmNvalue, "10", 
	XmNwidth, 80,
	XmNfontList, FontList,
	XtVaTypedArg, XmNbackground, XmRString, "White", 6,
	NULL );

  xstr1 = XmStringCreateLocalized(strcpy(s1, "Set Tics"));

  btn = XtVaCreateManagedWidget("button", 
	xmPushButtonWidgetClass, rowcol, 
	XmNlabelString, xstr1,
	XmNfontList, FontList,
	NULL);
  
  XtAddCallback(btn, XmNactivateCallback, PlotMenuCB, (void*) PL_SET_GRID_TICS);
  XmStringFree(xstr1);

  XtManageChild(m_nGridXlines);
  XtManageChild(m_nGridYlines);
  XtManageChild(m_nGridXaxis);
  XtManageChild(m_nGridYaxis);
  XtManageChild(m_nGridXtics);
  XtManageChild(m_nGridYtics);

  // Create the Verify dialog
  strcpy(s1, "Verify");
  m_nVerifyDialog = XmCreateQuestionDialog(TopLevel, s1, NULL, 0);
  xstr1 = XmStringCreateLocalized(s1);
  XtVaSetValues( m_nVerifyDialog,
	XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL, 
	XmNdialogTitle, xstr1,
	NULL);
  XmStringFree(xstr1);

  OnFileNew();
}
//---------------------------------------------------------------

CPlotWindow::~CPlotWindow()
{
  // Cleanup

  delete m_pDb;
  delete m_pDi;
  // need to free the font list entries
  m_pDc->CloseDisplay();
  delete m_pDc;
  delete [] m_pFileName;
}
//---------------------------------------------------------------

void CPlotWindow::OnPaint ()
{
  CRect r = m_pDc->GetClientRect();
  m_pDi->SetPlotRect (r, m_pDc);
  m_pDi->Draw(m_pDc);
  WriteStatusMessage(m_pDi->GetList());
}
//---------------------------------------------------------------

void CPlotWindow::OnPrint ()
{
  char s[256];
  CpsDC printDC (612, 792);
  printDC.OpenDisplay (strcpy(s,PRINT_TEMP_FILE));
  printDC.SetColors (color_names, MAX_COLORS);
  printDC.SetForeground (printDC.GetColor(strcpy(s,"black")));

  CRect r = printDC.GetClientRect();
  m_pDi->CreateView (CARTESIAN_INVERTED, m_pDi->GetExtrema());
  m_pDi->SetPlotRect (r, &printDC);
  m_pDi->Draw(&printDC);
  m_pDi->DeleteView();
  printDC.CloseDisplay();

  // Allow user to print or save to file from GhostView

  char print_string[256];
  strcpy (print_string, "gv ");
  strcat (print_string, PRINT_TEMP_FILE);
  strcat (print_string, " &");
  system (print_string);
}
//---------------------------------------------------------------

void CPlotWindow::OnResize ()
{
  CRect r = m_pDc->GetClientRect();
  m_pDc->SetClientRect(r);
  Invalidate();
}
//---------------------------------------------------------------

void CPlotWindow::OnReset ()
{
    m_pDi->ResetExtrema();
    m_pDi->GoHome();
    Invalidate ();
    if ( XtIsManaged(m_nHeaderDialog) ) OnHeader();
    if (XtIsManaged(m_nGridDialog)) OnGrid();

}
//--------------------------------------------------------------

void CPlotWindow::Invalidate()
{
    m_pDc->ClearDisplay();
}

void CPlotWindow::OnBack ()
{
    m_pDi->GoBack();
    Invalidate();
}

void CPlotWindow::OnForward ()
{
    m_pDi->GoForward();
    Invalidate();
}

void CPlotWindow::OnDeleteView ()
{
    m_pDi->DeleteView();
    Invalidate();
}
//---------------------------------------------------------------

Widget CPlotWindow::AddMenuItem (Widget wMenu, char* name)
{
  // Add a new button widget in wMenu, with specified name.
  char s[32];
  XmString str = XmStringCreate(name, strcpy(s,"Helvetica12pt"));
  Widget b = XtVaCreateManagedWidget( name, 
	xmPushButtonWidgetClass, wMenu,
	XmNlabelString, str,
	XmNfontList, FontList,
	NULL);
  XtVaSetValues( b, 
	XtVaTypedArg, XmNbackground,
	XmRString, "LightGrey", 10, 
	NULL);
  XmStringFree(str);  
  return b;
}
//---------------------------------------------------------------

Widget CPlotWindow::AddSubMenu (Widget wMenu, char* name)
{
  // Add a new submenu widget in wMenu, with specified name.
  // Return the pulldown menu widget, not the button.
  char s[64];
  Widget sMenu = XmCreatePulldownMenu(wMenu, name, NULL, 0);
  XmString str = XmStringCreate (name, strcpy(s, "Helvetica12pt"));
  Widget b = XtVaCreateManagedWidget( name, 
	xmCascadeButtonWidgetClass, wMenu,
	XmNsubMenuId, sMenu,
	XmNlabelString, str,
	XmNfontList, FontList,
	XmNmnemonic, 0,
	NULL);
  XtVaSetValues( b,
	XtVaTypedArg, XmNbackground,
	XmRString, "LightGrey", 10,
	NULL);
  XmStringFree(str);  
  return sMenu;
}
//---------------------------------------------------------------

Widget CPlotWindow::MakeMenu (Widget wAttach, char* menu_name, 
  char* options[], Widget buttons[], int n)
{
  char submenu_name [32], s[32];
  Widget menu, cascade;
  Arg al[10];
  int i, ac = 0;

  menu = XmCreatePulldownMenu(wAttach, menu_name, al, ac);

  XtSetArg (al[ac], XmNsubMenuId, menu);  
  ac++;
  XmString str =  XmStringCreateLtoR(menu_name, strcpy(s, "Helvetica12pt"));
  XtSetArg(al[ac], XmNlabelString, str);  
  ac++;
  XtSetArg (al[ac], XmNfontList, FontList);
  ac++;
  if (strcmp(menu_name, "Help") == 0)
  { XtSetArg (al[ac], XmNalignment, XmALIGNMENT_END); ac++; }
  cascade = XmCreateCascadeButton (m_nMenuBar, menu_name, al, ac);

  XmStringFree(str);
  XtManageChild (cascade);

  // Create the menu button widgets

  for (i = 0; i < n; i++) {
      if (*options[i] == '>') {
	  // Create a submenu cascade button
	 
	    strcpy (submenu_name, options[i]+1);
	    buttons[i] = AddSubMenu (menu, submenu_name);
      }
      else {
	  buttons[i] = AddMenuItem (menu, options[i]);
      }
  }
  return menu ;
}
//---------------------------------------------------------------

void CPlotWindow::SetBackgroundColor (char* color_name)
{
  unsigned long bkg = GetColor(color_name);
  m_pDc->SetBackground(bkg);
  strcpy (m_szBackgroundColor, color_name);
  Invalidate();
}

void CPlotWindow::SetForegroundColor (char* color_name)
{
  unsigned long fg = GetColor(color_name);
  m_pDc->SetForeground(fg);
  strcpy (m_szForegroundColor, color_name);

  if (m_pDi) 
    {
      CPlotView* pView = m_pDi->GetCurrentView();
      CGrid* pGrid = pView->m_pGrid;
      if (pGrid) pGrid->SetColor(fg);
    }
  Invalidate();
}
//---------------------------------------------------------------

Symbol CPlotWindow::GetSymbol (char* symbol_name)
{
  // Return the plot symbol value associated with symbol_name
  // See the file CPlot.h

  strupr (symbol_name);
  Symbol s = sym_LINE;

  for (int i = 0; i < 7; i++)
    {
      if (strcmp(SymbolNames[i], symbol_name) == 0) 
	{
	  s = (Symbol) i;
	  break;
	}
    }
  return s;
}
//---------------------------------------------------------------

CPlotMessage* CPlotWindow::GetPlotMessage()
{
  return m_pDc->GetPlotMessage();
}
//---------------------------------------------------------------

void CPlotWindow::OnMouseMove(unsigned int mf, CPoint p)
{
    char cur_x [24], cur_y [24], cur_xy[48];
    vector<double> x;
    
    m_pDi->m_nMousePt = p;
    x = m_pDi->Logical(p);
    sprintf (cur_x, m_pDi->m_szXform, x[0]);
    sprintf (cur_y, m_pDi->m_szYform, x[1]);
    strcpy (cur_xy, cur_x);
    strcat (cur_xy, "  ");
    strcat (cur_xy, cur_y);

    int ac = 0;
    Arg al[2];

    XtSetArg(al[ac], XmNlabelString,
 	   	 XmStringCreate(cur_xy, XmSTRING_DEFAULT_CHARSET)); 
    ac++;
    XtSetValues(m_nCoordinates, al, ac);
}
//---------------------------------------------------------------

void CPlotWindow::OnLButtonDown(unsigned int mf, CPoint p)
{
    XEvent report;
    Display* display = XtDisplay(m_nPlotWindow);
    CRect rect, ExpRect;

    ExpRect.TopLeft() = p;
    ExpRect.BottomRight() = p;
    rect = ExpRect;

    m_pDc->SetForeground (-1);  // white 
    m_pDc->SetDrawingMode (GXxor);
    m_pDc->Rectangle (rect);

    while (1)
    {
      XNextEvent(display, &report);
      if (report.type == MotionNotify)
      {
	  m_pDc->Rectangle (rect);
	  p.x = report.xmotion.x;
	  p.y = report.xmotion.y;
	  OnMouseMove(0, p);
          ExpRect.BottomRight() = m_pDi->m_nMousePt;
          rect = ExpRect;
          SortRect (&rect);
          m_pDc->Rectangle (rect);
      }
      else if (report.type == ButtonRelease)
	{
	  m_pDc->SetDrawingMode (GXcopy);
	  break;
	}
      else
	{	
	   ;
	}      
    }

    SortRect (&ExpRect);

    if ((ExpRect.Width() > 9) && (ExpRect.Height() > 9))
      {
	vector<double> x = m_pDi->Logical(ExpRect);
        m_pDi->CreateView(CARTESIAN, x);
        Invalidate ();
     }
    
}
//---------------------------------------------------------------

void CPlotWindow::OnRButtonDown(unsigned int mf, CPoint p)
{

}
//---------------------------------------------------------------

void CPlotWindow::MessageBox (const char* msg)
{
// Display a message
  
  XmString Message;
  int msg_length = strlen(msg);
  char* s = new char[msg_length+1];
  strncpy(s, msg, msg_length);
  s[msg_length] = '\0';
  char font_str[32];

  Message = XmStringCreateLtoR(s, strcpy(font_str, "Courier12pt"));
  XtVaSetValues(m_nAboutBox,
	XmNmessageString, Message, 
	NULL);
  XtManageChild(m_nAboutBox);
  delete [] s;
  XmStringFree(Message);
}
//---------------------------------------------------------------

void CPlotWindow::SelectPlot (int plot_index, int select_type)
{
  // Select the specified plot as the active or operand plot
  //   The plot_index starts at zero.
  //   select_type = 0 for operand, 1 for active

  if (plot_index >= 0 && plot_index < m_pDi->Nplots())
    {
      CPlot* p = (*m_pDi)[plot_index];
      if (select_type == 1) {
	m_pDi->SetActivePlot(p);
	if ( XtIsManaged(m_nHeaderDialog) ) OnHeader();
      }
      else {
	m_pDi->SetOperandPlot(p);
      }
      WriteStatusMessage(m_pDi->GetList());
    }
  return ;
}
//---------------------------------------------------------------

void CPlotWindow::OnExtremaInput ()
{
  Arg al[4];
  char s1[32], s2[32];
  int ac = 0;
  XtSetArg( al[ac], XmNselectionLabelString, 
  XmStringCreateLtoR( strcpy(s1, "Enter xmin, ymin, xmax, ymax:"),
	 strcpy(s2, "CourierBold14pt")) ); 
  ac++;
  XtSetArg(al[ac], XmNfontList, FontList);
  ac++;
  XtSetArg(al[ac], XmNtextString, XmStringCreateLtoR(NULL,
    XmSTRING_DEFAULT_CHARSET));
  ac++;
  XtSetValues(m_nInputDialog, al, ac);
  XtAddCallback (m_nInputDialog, XmNokCallback, 
    PlotMenuCB, (void*) PL_EXTREMA);
  XtAddCallback (m_nInputDialog, XmNcancelCallback, 
    PlotMenuCB, (void*) PL_EXTREMA_CANCEL);
  Widget w = XmSelectionBoxGetChild(m_nInputDialog, XmDIALOG_TEXT);
  XtVaSetValues(m_nInputDialog, XmNinitialFocus, w, NULL);      
  XtManageChild (m_nInputDialog);
}
//----------------------------------------------------------------

void CPlotWindow::OnExtrema (char* s)
{
  double x[6];
  vector<double> e = m_pDi->GetExtrema();
  x[0] = e[0];
  x[1] = e[2];
  x[2] = e[1];
  x[3] = e[3];

  int n = NumberParse (x, s);

  e[0] = x[0];
  e[1] = x[2];
  e[2] = x[1];
  e[3] = x[3];

  m_pDi->CreateView(CARTESIAN, e);
    
  Invalidate();

}
//---------------------------------------------------------------

void CPlotWindow::WriteStatusMessage(char* msg)
{
  // Write a message to the status bar. This
  //   routine is generally used by the CPlotList object.
  char s[32];
  XmString Message = XmStringCreate(msg, strcpy(s, "Helvetica14pt"));
  XtVaSetValues(m_nStatusBar,
	XmNlabelString, Message,
	NULL);
  XmStringFree(Message);
  return;
}
//---------------------------------------------------------------

void CPlotWindow::WriteConsoleMessage(const char* msg)
{
  // Write a message to the Forth console area;

  const int MaxMsgLength = 1024;
  char s[MaxMsgLength+4];
  int msgLen = strlen(msg);
  if (msgLen > MaxMsgLength) {
    strncpy(s, msg, MaxMsgLength);
    s[MaxMsgLength] = '\0';
  }
  else {
    strcpy(s, msg);
  }
  strcat(s, "\n ");

  Widget w = m_nForthShell;
  char* fs = XmTextGetString(w);
  int nLen = strlen(fs);
  XtFree(fs);
  XmTextInsert (w, nLen, s);
  int nPos = XmTextGetLastPosition (w);
  XmTextShowPosition(w, nPos);
  XmTextSetInsertionPosition(w, nPos);
}
//---------------------------------------------------------------

void CPlotWindow::OnCopyBitmap ()
{
	;
}
//------------------------------------------------------------------

void CPlotWindow::OnCopyMetafile ()
{
	;
}
//---------------------------------------------------------------

void CPlotWindow::OnSymbol()
{
  XtManageChild(m_nSymbolDialog);
}

void CPlotWindow::OnSetSymbol()
{
  Widget slist = XmSelectionBoxGetChild(m_nSymbolDialog,
    XmDIALOG_LIST);

  int* selections = new int[16];
  int nsel, i;

  if (XmListGetSelectedPos(slist, &selections, &nsel))
    {
      i = selections[0];
      char sym_name [32];
      strcpy (sym_name, SymbolNames[i-1]);
      Symbol nSym = GetSymbol(sym_name);
      CPlot* p = m_pDi->GetActivePlot();
      if (p) 
	{ 
	  p->SetSymbol(nSym);
	  Invalidate();
	}
    }
  delete [] selections;  
}

void CPlotWindow::OnSymbolLine ()
{
    m_pDi->GetActivePlot()->SetSymbol (sym_LINE);
    Invalidate();
}

void CPlotWindow::OnSymbolPoint ()
{
    m_pDi->GetActivePlot()->SetSymbol (sym_POINT);
    Invalidate();
}

void CPlotWindow::OnSymbolLinePt ()
{
    m_pDi->GetActivePlot()->SetSymbol (sym_LINE_PLUS_POINT);
    Invalidate();
}

void CPlotWindow::OnSymbolStick ()
{
    m_pDi->GetActivePlot()->SetSymbol (sym_STICK);
    Invalidate();
}

void CPlotWindow::OnSymbolToggle ()
{
    if (m_pDi->GetActivePlot()->GetSymbol() == sym_LINE)
        m_pDi->GetActivePlot()->SetSymbol (sym_POINT);
    else
        m_pDi->GetActivePlot()->SetSymbol (sym_LINE);

    Invalidate();
}
//---------------------------------------------------------------

void CPlotWindow::OnExpressionInput ()
{
  Arg al[4];
  char s1[32], s2[32];
  int ac = 0;
  XtSetArg(al[ac], XmNselectionLabelString, 
    XmStringCreateLtoR(strcpy(s1, "Enter the math expression:"), 
    strcpy(s2, "Courier12pt")));  
  ac++;
  XtSetArg(al[ac], XmNtextString, XmStringCreateLtoR(NULL,
    XmSTRING_DEFAULT_CHARSET));
  ac++;
  XtSetValues(m_nInputDialog, al, ac);
  XtAddCallback (m_nInputDialog, XmNokCallback, 
    MathMenuCB, (void*) PL_EXPRESSION);
  XtAddCallback (m_nInputDialog, XmNcancelCallback, 
    MathMenuCB, (void*) PL_EXPRESSION_CANCEL);
  Widget w = XmSelectionBoxGetChild(m_nInputDialog, XmDIALOG_TEXT);
  XtVaSetValues(m_nInputDialog, XmNinitialFocus, w, NULL);      
  XtManageChild (m_nInputDialog);
}
//---------------------------------------------------------------

void CPlotWindow::OnExpression(char* exp)
{
  char s[256], emsg[1024];
  *s = 0;
  int i;
  long int *StackPtr;
  BYTE* TypePtr;
  
  if (*exp)
    {
      vector<BYTE> opcodes, prefix;
      strupr(exp);

      int ecode = CompileAE (&opcodes, exp);

      //OutputForthByteCode (&opcodes);
      //MessageBox(emsg);
 
      if (ecode) {
	  sprintf (emsg, "%s %d", "Expression Compiler Error:", ecode);
	  MessageBox (emsg);
      }
      else {
            // Perform the expression operations

	  CDataset* d = m_pDi->GetActiveSet();
	  int npts = d->NumberOfPoints();
	  int nsize = d->SizeOfDatum();
	  double *p = &(*(d->begin()));
	  BYTE* bp = (BYTE*) &p;
	  prefix.push_back(OP_ADDR);
	  for (i = 0; i < sizeof(void*); i++)
	    prefix.push_back(*(bp + i));
	  bp = (BYTE*) &nsize;
	  prefix.push_back(OP_IVAL);
	  for (i = 0; i < sizeof(int); i++)
	    prefix.push_back(*(bp + i));
	  bp = (BYTE*) &npts;
	  prefix.push_back(OP_IVAL);
	  for (i = 0; i < sizeof(int); i++)
	    prefix.push_back(*(bp + i));
// prefix.push_back(OP_DOTS);
	  prefix.push_back(OP_RET);

	  // SetForthInputStream (cin);
          stringstream ForthMessages;
	  SetForthOutputStream (ForthMessages);
	  ecode = ForthVM (&prefix, &StackPtr, &TypePtr);
	  if (ecode)
	    {
	      sprintf (emsg, "%s %d", "Execution Error:", ecode);
	      MessageBox (emsg);
	    }
	  else
	    {
	      ecode = ForthVM (&opcodes, &StackPtr, &TypePtr);
	      if (ecode)
		{
		  sprintf (emsg, "%s %d", "Execution Error:", ecode);
		  MessageBox (emsg);
	        }
	      else
		{
		  // Revise the dataset extrema and header

		  d->SetExtrema();
		  sprintf (emsg, "%s\n", exp);
		  // if (AddToHeader (emsg, d->m_szHeader, True))
		  if (!d->AppendToHeader(emsg))
		    MessageBox ("Header not updated.\nLength exceeded.");
		  
		  m_pDi->ResetExtrema();
		  WriteConsoleMessage (exp);
		  Invalidate();
		}
	    }
	}
    }
}
//--------------------------------------------------------------

void CPlotWindow::OnGrid()
{
  // Allow user to setup grid style
  CPlotView* pView = m_pDi->GetCurrentView();
  CGrid* pGrid = pView->m_pGrid;
  int nX, nY;
  bool bVer, bHor, bXaxis, bYaxis;
  pGrid->GetTics(&nX, &nY);
  pGrid->GetLines(&bVer, &bHor);
  pGrid->GetAxes(&bXaxis, &bYaxis); 
  XmToggleButtonSetState(m_nGridXlines, bVer, False);
  XmToggleButtonSetState(m_nGridYlines, bHor, False);
  XmToggleButtonSetState(m_nGridXaxis, bXaxis, False);
  XmToggleButtonSetState(m_nGridYaxis, bYaxis, False);
  char sXtics[8], sYtics[8];
  sprintf(sXtics, "%4d", nX);
  sprintf(sYtics, "%4d", nY);
  XtVaSetValues(m_nGridXtics, XmNvalue, sXtics, NULL );
  XtVaSetValues(m_nGridYtics, XmNvalue, sYtics, NULL );
  XtManageChild (m_nGridDialog);
}

void CPlotWindow::OnGridToggle()
{
  CPlotView* pView = m_pDi->GetCurrentView();
  bool bHor, bVer;
  pView->m_pGrid->GetLines(&bVer, &bHor);
  if (bHor && bVer)
    pView->m_pGrid->SetLines(false, false);
  else
    pView->m_pGrid->SetLines(true, true);

  Invalidate();
}
//--------------------------------------------------------------

int CPlotWindow::GetInput (char* prompt, char* response)
{
  int i = 0;
  /*
  CInputDialog getinp(prompt);
  i = getinp.DoModal();
  strcpy (response, getinp.GetResponse());

  return i; */
  return 0;
}
//---------------------------------------------------------------

int CPlotWindow::Verify (char* msgLabel, char* okLabel, 
		char* cancelLabel, char* helpLabel)
{
// Display a verification dialog with a message string,
// msgLabel, and button labels okLabel, cancelLabel,
// and szHelpLabel for the OK, CANCEL, and HELP buttons.
// Empty button label strings will unmanage the
// corresponding buttons.
// Return the corresponding button code:
//
    XmString xstr;
    Widget w = m_nVerifyDialog;
    Widget btn;

    XtVaSetValues(w, 
	XmNdefaultButtonType, XmDIALOG_CANCEL_BUTTON, NULL);

    if (strlen(msgLabel)) {
      xstr = XmStringCreateLocalized(msgLabel);
      XtVaSetValues( w, 
	XmNmessageString, xstr,
	NULL);
      XmStringFree(xstr);
    }

    btn = XmMessageBoxGetChild(w, XmDIALOG_OK_BUTTON);
    if (strlen(okLabel)) {
      xstr = XmStringCreateLocalized(okLabel);
      XtVaSetValues( btn,
	XmNlabelString, xstr,
	XmNfontList, FontList,
	NULL);
      XmStringFree(xstr);
      XtManageChild(btn);
    }
    else {
      XtUnmanageChild(btn);
    }

    btn = XmMessageBoxGetChild(w, XmDIALOG_CANCEL_BUTTON);
    if (strlen(cancelLabel)) {
      xstr = XmStringCreateLocalized(cancelLabel);
      XtVaSetValues( btn,
	XmNlabelString, xstr,
	XmNfontList, FontList,
	NULL);
      XmStringFree(xstr);
      XtManageChild(btn);
    }
    else {
      XtUnmanageChild(btn);
    }

    btn = XmMessageBoxGetChild(w, XmDIALOG_HELP_BUTTON);
    if (strlen(helpLabel)) {
      xstr = XmStringCreateLocalized(helpLabel);
      XtVaSetValues( btn,
	XmNlabelString, xstr,
	XmNfontList, FontList,
	NULL);
      XmStringFree(xstr);
      XtManageChild(btn);
    }
    else {
      XtUnmanageChild(btn);
    }

    verify_answer = 0;

    XtManageChild(w);
    XtPopup(XtParent(w), XtGrabNone);
    while (verify_answer == 0) XtAppProcessEvent(xapp,XtIMAll);
    XtPopdown(XtParent(w));

    return verify_answer;
}
//---------------------------------------------------------------

void CPlotWindow::OnDelete()
{
    CDataset* d = m_pDi->GetActiveSet();
    int i;
    char s[256];

    if (d)
    {
        // Remove the dataset from the database

        m_pDb->RemoveSet(d);

        // Delete all plots of set d

        m_pDi->DeletePlotsOf(d);

        sprintf (s, "Dataset %s deleted from workspace", d->m_szName);
	WriteConsoleMessage(s);

        // Delete the actual data set

        delete d;

        // Refresh the display

        m_pDi->ResetExtrema();
        Invalidate();
    }
}
//---------------------------------------------------------------

void CPlotWindow::OnDuplicate()
{
    // Duplicate the active data set and make a new plot

    CDataset* d = m_pDi->GetActiveSet();
    if (d)
    {
        CDataset* d2 = d->Duplicate();
        if (d2)
        {
            m_pDb->AddSet(d2);
            m_pDi->MakePlot(d2, 0);
            Invalidate();
        }
        else
        {
	  MessageBox ("Dataset Duplication Failed!");
        }
    }
}
//---------------------------------------------------------------

void CPlotWindow::OnPick()
{
  int nsets = m_pDb->Nsets();
  if (nsets == 0) {
      MessageBox ("There are no datasets in the database.");
  }
  else {
      char** sp = new char* [nsets];
      int i;

      for (i = 0; i < nsets; i++) sp[i] = new char [256];

      m_pDb->MakeListOfNames(sp);

      Widget pick_list = XmSelectionBoxGetChild(m_nPickDialog, 
	XmDIALOG_LIST);

      XmListDeleteAllItems (pick_list);

      for (i = 0; i < nsets; i++)
	{
	  XmListAddItem(pick_list, XmStringCreate(sp[i], 
	    XmSTRING_DEFAULT_CHARSET), 0);
	}

      XtManageChild(m_nPickDialog);
    }
}
//---------------------------------------------------------------

void CPlotWindow::OnPickData()
{

  Widget pick_list = XmSelectionBoxGetChild(m_nPickDialog,
    XmDIALOG_LIST);

  int* selections = new int[16];
  int nsel, i;

  if (XmListGetSelectedPos(pick_list, &selections, &nsel))
    {
      i = selections[0];
      CDataset* d =  (*m_pDb)[i-1];
      if (d)
	{
	  m_pDi->MakePlot(d, 0);
	  OnReset();
	}
    }
  delete [] selections;

  XtUnmanageChild(m_nPickDialog);
}
//----------------------------------------------------------------

void CPlotWindow::OnDrop()
{
    m_pDi->DeletePlot();
    m_pDi->ResetExtrema();
    Invalidate();
    if ( XtIsManaged(m_nHeaderDialog) ) OnHeader();
}
//---------------------------------------------------------------

void CPlotWindow::OnColor()
{
  XtManageChild(m_nColorDialog);
}

void CPlotWindow::OnSetColor()
{
  Widget slist = XmSelectionBoxGetChild(m_nColorDialog,
    XmDIALOG_LIST);

  int* selections = new int[MAX_COLORS];
  int nsel, i;

  if (XmListGetSelectedPos(slist, &selections, &nsel))
    {
      i = selections[0];
      char color_name [32];
      strcpy (color_name, color_names[i-1]);
      unsigned c = GetColor(color_name); // index into color map
      CPlot* p = m_pDi->GetActivePlot();
      if (p) { 
	p->SetColor(c);
	Invalidate();
      }
    }
  delete [] selections;  
}
//---------------------------------------------------------------


void CPlotWindow::OnView2DCartesian()
{
//
	OnReset();
}
//---------------------------------------------------------------

void CPlotWindow::OnView2DPolar()
{
  ;
}
//---------------------------------------------------------------
/*
void CPlotWindow::OnView3DCartesian()
{
	m_pDi->Set3D (CARTESIAN);
	OnReset();
}
*/
//---------------------------------------------------------------

void CPlotWindow::OnAspect()
{
  ;
}
//---------------------------------------------------------------
/*
void CPlotWindow::OnRotate()
{

}
*/
//---------------------------------------------------------------

void CPlotWindow::OnHeader ()
{
  CDataset* d = m_pDi->GetActiveSet();
  if (d)
    {
      XmTextSetString(m_nDatasetName, d->m_szName);
      XtVaSetValues(m_nDatasetName, XtVaTypedArg, XmNbackground,
		    XmRString, "White", 6, NULL);
      XmTextSetString(m_nHeaderText, d->m_szHeader);      
      XtVaSetValues(m_nHeaderText, XtVaTypedArg, XmNbackground, XmRString, 
		    "White", 6, NULL);
      XtManageChild (m_nHeaderDialog);     

    }
}
//---------------------------------------------------------------

void CPlotWindow::OnSetHeader (char* s)
{
  CDataset* d = m_pDi->GetActiveSet();
  if (d)
    {
      strcpy (d->m_szHeader, s);
    }
}
//---------------------------------------------------------------

void CPlotWindow::OnSetDatasetName (char* s)
{
  CDataset* d = m_pDi->GetActiveSet();
  if (d)
    {
      strcpy (d->m_szName, s);
    }
}
//---------------------------------------------------------------


bool CPlotWindow::OnFileNew ()
{
	delete m_pDi;
	delete m_pDb;

	m_pDb = new CDatabase();	// create database and display
	m_pDi = new CPlotDisplay();

	// Close popped up dialogs for which info is no longer valid
	if (XtIsManaged(m_nHeaderDialog)) XtUnmanageChild(m_nHeaderDialog);
	if (XtIsManaged(m_nPickDialog)) XtUnmanageChild(m_nPickDialog);
	OnReset();
	WriteConsoleMessage("Workspace cleared.");

	return True;
}
//---------------------------------------------------------------

bool CPlotWindow::OnFileOpen ()
{
  char s[8192];
  getcwd(s, 4096);
  strcpy(sz_CurrentWorkingDir, s);
  strcat (s, "/*.dat");
  XmString cwd =  XmStringCreateLocalized(s);
  XmFileSelectionDoSearch(m_nFileOpenDialog, cwd); 
  XtManageChild(m_nFileOpenDialog);
  XmStringFree(cwd);
  return True; 
}
//--------------------------------------------------------------

bool CPlotWindow::OnFileSave ()
{
  XtManageChild(m_nFileSaveDialog); return True;
}
//---------------------------------------------------------------

void CPlotWindow::OnAbout()
{
  char s[512];
  sprintf(s, "XYPLOT %s for Linux/X Windows  (kForth %s)\nBuild: %s\n",
		  ver, kfver, build);
  strcat (s, "\nCopyright (c) "); strcat (s, copyright);
  strcat (s, " Krishna Myneni and John Kielkopf\n");
  strcat (s, "e-mail: krishna.myneni@ccreweb.org\n\n");
  strcat (s, "Provided under the GNU General Public License\n\n");
  MessageBox(s);
}
//--------------------------------------------------------------

void CPlotWindow::SelectColumns (char* fname, int nCols)
{
  // Open up a dialog box to allow user to specify columns to load

  char s[128];
  Arg al[4];
  int ac = 0;
  XtSetArg(al[ac], XmNtextString, XmStringCreateLtoR(NULL,
    XmSTRING_DEFAULT_CHARSET));
  ac++;
  sprintf (s, "File contains %d columns.\n", nCols);
  strcat (s, "Select columns to load:");
  XtSetArg(al[ac], XmNselectionLabelString, 
    XmStringCreateLtoR(s, XmSTRING_DEFAULT_CHARSET));  
  ac++;
  //  XtSetArg(al[ac], XmNinitialFocus, XmDIALOG_TEXT);
  // ac++;
  XtSetValues(m_nInputDialog, al, ac);
  XtAddCallback (m_nInputDialog, XmNokCallback, 
    FileMenuCB, (void*) PL_FILE_LOAD);
  XtAddCallback (m_nInputDialog, XmNcancelCallback, 
    FileMenuCB, (void*) PL_LOAD_CANCEL);
  Widget w = XmSelectionBoxGetChild(m_nInputDialog, XmDIALOG_TEXT);
  XtVaSetValues(m_nInputDialog, XmNinitialFocus, w, NULL);      
  XtManageChild (m_nInputDialog);
}
//---------------------------------------------------------------

vector<int> CPlotWindow::ParseColumns (char* cs)
{
// Parse a column specifier string into a vector of column numbers

  double x[8];
  int i, nSel = 0;
  char s[64];
  vector<int> Cols;

  if (cs) {
    if (*cs) {
      strcpy (s, cs);
      nSel = NumberParse(x, s);
      for (i = 0; i < nSel; i++) Cols.push_back((int) x[i] - 1);
    }
  }
  return Cols;
}
//---------------------------------------------------------------

void CPlotWindow::LoadDatasetFile (char* fname, char* col_spec)
{
  // Load a dataset file into the database, with the columns
  //   to load specified in the col_spec

  char s[512];
  CDataset* d;

  int ecode = 0;
  ecode = m_pDb->LoadDataset(&d, fname, ParseColumns(col_spec), REAL_DOUBLE);
  if (ecode <= 0)
    {
      MessageBox ("Error reading file.");
    }
  else
    {
      sprintf (s, "%d points read from %s", ecode, fname);
      WriteConsoleMessage(s);
      m_pDi->MakePlot(d, sym_LINE);
      OnReset();
    }
  *m_pFileName = '\0';
}
  
//---------------------------------------------------------------

bool CPlotWindow::LoadFile(char* fname)
{
    int i, ecode, ns;
    char s[128];

    if (*fname) {
        CDataset* d;

	strcpy (m_pFileName, fname);   // save filename for callbacks in case of a command-line specified file

       	if (strstr(fname, ".xsp")) {
       	    CWorkspace41* pWs;
            ecode = m_pDb->LoadWorkspace (&pWs, fname);
            if (ecode) {
                sprintf (s, "XSP FILE CORRUPT");
                MessageBox (s);
            }
            else {
                for (i = 0; i < pWs->di.nplots; i++) {
                    ns = pWs->pv[i].set - 1;  // array origin is now 0
                    d = m_pDb->FindInList(pWs->ds[ns].fname);
                    if (d) m_pDi->MakePlot(pWs, d, i);
                }
                OnReset();
   	        delete pWs;     // delete the workspace (we're done with it)

            }
        }
        else if (strstr(fname, ".4th")) {
          // Load a Forth source file into the built-in Forth environment
	  ecode = LoadForthFile (fname);
        }
        else {
	  int nCols = m_pDb->GetColumnCount(fname);
          if (nCols < 0) {
	      MessageBox ("Cannot open input file.");	    
	  }
	  else if (nCols == 0) {
	      MessageBox ("Cannot find first line of data in file.");
	  }
	  else if (nCols <= 2) {
	      LoadDatasetFile (fname, NULL);
	  }
	  else {
	      SelectColumns (fname, nCols);
	  }
        }
    }
    return FALSE;
}
//---------------------------------------------------------------

bool CPlotWindow::SaveFile (char* fname)
{
    bool success = False;
    char s[512]; 	      

    CDataset* d = m_pDi->GetActiveSet();
    if (d) {
      if (strstr(fname, ".xsp")) {
	  // Create an xyplot workspace file
	  success = SaveWorkspace(fname);
      }
      else {
	// Save the active data set
	vector<double> e = m_pDi->GetExtrema();
	int bOverlap;
	vector<int> lim = d->IndexLimits(e[0], e[1], bOverlap);
	if (bOverlap) {
	  sprintf (s, "Source file is %s\n", d->m_szName);
	  AddToHeader (s, d->m_szHeader, True);

	  char s1[64], s2[16], s3[16], s4[16];
	  int v;
	  if ((lim[0] > 0) || (lim[1] < (d->NumberOfPoints()-1))) {
	    strcpy(s1, "Save only data in window domain?");
	    strcpy(s2, "Yes"); strcpy(s3, "Cancel"); strcpy(s4, "Save All");
	    v = Verify((char*) s1, (char*) s2, (char*) s3, (char*) s4);    
	    if (verify_answer == XmCR_HELP) {
	      lim[0] = 0;
	      lim[1] = d->NumberOfPoints()-1;
	    }
	    if (v == XmCR_CANCEL) {
	      WriteConsoleMessage ("Save cancelled.");
	      return success;
	    }
	  } // if ((lim[0] > 0 ...
	      
	  int fd;
	  if (fd = open(fname, O_RDONLY) != -1) {
	    close(fd);
	    strcpy(s1, "File already exists. Overwrite?");
	    strcpy(s2, "Yes"); strcpy(s3, "No"); s4[0] = '\0';
	    v = Verify((char*) s1, (char*) s2, (char*) s3, (char*) s4); 
	    if (v == XmCR_OK) 
	      success = m_pDb->SaveDataset (d, lim, fname);
	    else {
	      WriteConsoleMessage ("Save cancelled.");
	      return success;
	    }
	  } // if (fd = open ...
	  else
	    success = m_pDb->SaveDataset (d, lim, fname);

	  if (success) {
	    strcpy (d->m_szName, fname);
	    sprintf (s, "%d points written to %s", lim[1]-lim[0]+1, fname);
	    WriteConsoleMessage (s);
	  }
	  else {
	    MessageBox ("Error writing output file.");
	  }
	} // if (bOverlap) ...
	else {
	  MessageBox("Active dataset not visible in plot window.");
	}
      }
    }
    return success;
}
//----------------------------------------------------------------

bool CPlotWindow::SaveWorkspace (char* fname)
{
  // Save an xyplot workspace file

  bool success = False;

  ofstream outfile (fname);
  if (!outfile.fail())
    {
      char hdr[64];

      // Write the header

      strcpy (hdr, "XYPLOT_WORKSPACE_VER10X");
      outfile.write(hdr, 64);
      
      // Write database info

      m_pDb->Write(outfile);

      // Write plot display info
      
      m_pDi->Write(outfile);

      success = True;
    }
  return success;
}
//-----------------------------------------------------------------

int CPlotWindow::GetWindowText (char* buf, int nmax)
{
    return 0;
}

void CPlotWindow::SetWindowText (char* pStr)
{

}







