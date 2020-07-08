// CPlotWindow.cpp
//
// Copyright 1996--2020 Krishna Myneni 
// <krishna.myneni@ccreweb.org>
//
// This software is provided under the terms of the 
// GNU Affero General Public License (AGPL), v3.0 or later.
//

#ifdef COPYRIGHT
const char* copyright=COPYRIGHT;
#else
const char* copyright="1999--2020"
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
extern int NumberParse (float*, char*);
extern int CompileAE (vector<byte>*, char* exp);
extern "C" char* strupr (char*);
extern int AddToHeader (char*, char*, bool);
extern int LoadForthFile (char*);

extern void FileMenuCB (Widget, void*, void*);
extern void EditMenuCB (Widget, void*, void*);
extern void PlotMenuCB (Widget, void*, void*);
extern void MathMenuCB (Widget, void*, void*);
extern void ForthCB (Widget, void*, void*);
extern void SaveOptionsCB (Widget, void*, void*);
extern void RadioToggledCB (Widget, void*, void*);

extern Widget TopLevel;
extern volatile int verify_answer;

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
  m_pFileName = new char[256];
  *m_pFileName = '\0';

  CxDC* pDc = new CxDC (DEV_X_WINDOW, 800, 640);

  // Setup the fonts

  FontEntries[0] = XmFontListEntryLoad (disp, "-*-courier-*-r-*--*-120-*", 
					XmFONT_IS_FONT, "Courier12pt");
  FontEntries[1] = XmFontListEntryLoad (disp, "-*-courier-bold-o-*--*-140-*", 
					XmFONT_IS_FONT, "CourierBold14pt");
  FontEntries[2] = XmFontListEntryLoad (disp, "-*-helvetica-*-r-*--*-120-*",
					XmFONT_IS_FONT, "Helvetica12pt");
  FontEntries[3] = XmFontListEntryLoad (disp, "-*-helvetica-*-r-*--*-140-*",
					XmFONT_IS_FONT, "Helvetica14pt");
  FontList = XmFontListAppendEntry (NULL, FontEntries[0]);
  for (ac = 1; ac < 4; ac++)
    FontList = XmFontListAppendEntry (FontList, FontEntries[ac]);

  for (ac = 0; ac < 4; ac++) XmFontListEntryFree (&FontEntries[ac]);

  // Create a form widget
  
  ac = 0;
  m_nForm = XtCreateManagedWidget("form", xmFormWidgetClass,
    TopLevel, al, ac);
  ac = 0;
  XtSetArg (al[ac], XmNheight, 640);
  ++ac;
  XtSetArg (al[ac], XmNwidth, 800);
  ++ac;
  XtSetValues (TopLevel, al, ac);


  m_pDb = NULL;
  m_pDi = NULL;

  // Create a menu bar and attach it to the form.
  ac = 0;
  XtSetArg(al[ac], XmNtopAttachment, XmATTACH_FORM); 
  ac++;
  XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM); 
  ac++;
  XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); 
  ac++;
  m_nMenuBar = XmCreateMenuBar(m_nForm, (char*) "Menu Bar", al, ac);
  XtManageChild(m_nMenuBar);

  // Create the Forth shell dialog

  ac = 0;
  XtSetArg(al[ac], XmNeditMode, XmMULTI_LINE_EDIT);
  ac++;
  XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM);
  ac++;
  XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_FORM);
  ac++;
  XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM);
  ac++;
  XtSetArg(al[ac], XmNheight, 100);
  ac++;

  m_nForthShell = XmCreateScrolledText(m_nForm, (char*) "Forth Shell",
    al, ac);
  XtVaSetValues(m_nForthShell, XtVaTypedArg, XmNbackground, XmRString, 
  		"White", 6, NULL);
  XtManageChild(m_nForthShell);

  // Create the status message label widget and attach it below the
  //   drawing area (plot window)

  ac = 0;
  XtSetArg(al[ac], XmNlabelString,
    XmStringCreate((char*) " ", XmSTRING_DEFAULT_CHARSET));
  ac++;
  XtSetArg(al[ac], XmNalignment, XmALIGNMENT_BEGINNING);
  ac++;
  XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM);
  ac++;
  XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM);
  ac++;
  XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_WIDGET);
  ac++;
  XtSetArg(al[ac], XmNbottomWidget, XtParent(m_nForthShell));
  ac++;

  m_nStatusBar = XtCreateManagedWidget("status_bar",
    xmLabelWidgetClass, m_nForm, al, ac);

  // Create coordinates label widget and attach it to the right of the
  //  status bar

  ac = 0;
  XtSetArg(al[ac], XmNlabelString,
 	   	 XmStringCreate((char*) " ",XmSTRING_DEFAULT_CHARSET)); 
  ac++;
  XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM);
  ac++;
  XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_WIDGET);
  ac++;
  XtSetArg(al[ac], XmNbottomWidget, XtParent(m_nForthShell));
  ac++;

  m_nCoordinates = XtCreateManagedWidget("coordinates",
    xmLabelWidgetClass, m_nForm, al, ac);

  // Create the drawing area and attach it below the menu bar
  // and above the status bar

  ac = 0;
  XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); 
  ac++;
  XtSetArg(al[ac], XmNtopWidget, m_nMenuBar); 
  ac++;
  XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM);
  ac++;
  XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM);
  ac++;
  XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_WIDGET);
  ac++;
  XtSetArg(al[ac], XmNbottomWidget, m_nStatusBar);
  ac++;
  XtSetArg(al[ac], XmNheight, 400);
  ac++;
  XtSetArg(al[ac], XmNwidth, 800);
  ac++;

  m_nPlotWindow = XtCreateManagedWidget ("Plot Window",
     xmDrawingAreaWidgetClass, m_nForm, al, ac);

  pDc->OpenDisplay(XtDisplay(m_nPlotWindow), XtWindow(m_nPlotWindow));
  m_pDc = pDc;


  // Make the menus

  for (int j = 0; j < 16; j++)
    {
      FileWidgets[j] = 0;
      EditWidgets[j] = 0;
      PlotWidgets[j] = 0;
      MathWidgets[j] = 0;
      HelpWidgets[j] = 0;
    }
  m_nFileMenu = MakeMenu (m_nMenuBar, (char*) "File", (char**) FileOptions, FileWidgets, 6);
  m_nEditMenu = MakeMenu (m_nMenuBar, (char*) "Edit", (char**) EditOptions, EditWidgets, 3);
  m_nPlotMenu = MakeMenu (m_nMenuBar, (char*) "Plot", (char**) PlotOptions, PlotWidgets, 7);
  m_nMathMenu = MakeMenu (m_nMenuBar, (char*) "Math", (char**) MathOptions, MathWidgets, 1);
  m_nHelpMenu = MakeMenu (m_nMenuBar, (char*) "Help", (char**) HelpOptions, HelpWidgets, 1);

  // Make the submenus

  //  m_nViewMenu = MakeMenu (PlotWidgets[ID_VIEW], "View", ViewOptions,
  //   ViewWidgets, 5); 

  // Create the file open dialog

  Widget temp;
  ac = 0;
  m_nFileOpenDialog = XmCreateFileSelectionDialog(TopLevel,
     (char*) "Open File", al, ac);
  temp = XmFileSelectionBoxGetChild(m_nFileOpenDialog,XmDIALOG_FILTER_TEXT);
  XtVaSetValues(temp, XtVaTypedArg, XmNbackground, XmRString, 
  		"White", 6, NULL);
  temp = XmFileSelectionBoxGetChild(m_nFileOpenDialog,XmDIALOG_TEXT);
  XtVaSetValues(temp, XtVaTypedArg, XmNbackground, XmRString, 
  		"White", 6, NULL);
  XtUnmanageChild(XmSelectionBoxGetChild(m_nFileOpenDialog,
    XmDIALOG_HELP_BUTTON));

  // Create the file save dialog

  ac = 0;
  m_nFileSaveDialog = XmCreateFileSelectionDialog(TopLevel,
     (char*) "Save File", al, ac);
  temp = XmFileSelectionBoxGetChild(m_nFileSaveDialog,XmDIALOG_FILTER_TEXT);
  XtVaSetValues(temp, XtVaTypedArg, XmNbackground, XmRString, 
  		"White", 6, NULL);
  temp = XmFileSelectionBoxGetChild(m_nFileSaveDialog,XmDIALOG_TEXT);
  XtVaSetValues(temp, XtVaTypedArg, XmNbackground, XmRString, 
  		"White", 6, NULL);
    
  XtUnmanageChild(XmSelectionBoxGetChild(m_nFileSaveDialog,
    XmDIALOG_HELP_BUTTON));

  // Create a dual purpose About/Message box

  m_nAboutBox = XmCreateInformationDialog(TopLevel, (char*) "About XYPLOT",
    NULL, 0);
  // XtVaSetValues(m_nAboutBox, XtVaTypedArg, XmNbackground, XmRString, 
  //		"LightGrey", 10, NULL);
  temp = XmMessageBoxGetChild (m_nAboutBox, XmDIALOG_CANCEL_BUTTON);
  XtUnmanageChild(temp);
  temp = XmMessageBoxGetChild (m_nAboutBox, XmDIALOG_HELP_BUTTON);
  XtUnmanageChild(temp);

  // Create a general purpose radio box

  m_nRadioBox = XmVaCreateSimpleRadioBox (TopLevel, "radio_box", 0,
		RadioToggledCB, XmVaRADIOBUTTON,
		XmStringCreateLocalized("one"),
		NULL, NULL, NULL, NULL);
  // XtManageChild(m_nRadioBox);

  // Create the input dialog

  ac = 0;
  XtSetArg(al[ac], XmNselectionLabelString, XmStringCreateLtoR
		("Enter input", "Courier12pt"));  
  ac++;
  XtSetArg (al[ac], XmNfontList, FontList);
  ac++;
  m_nInputDialog = XmCreatePromptDialog(TopLevel, "Input Dialog", al, ac);
  temp = XmSelectionBoxGetChild(m_nInputDialog, XmDIALOG_TEXT);
  XtVaSetValues(temp, XtVaTypedArg, XmNbackground, XmRString, 
  		"White", 6, NULL);
  XtVaSetValues(m_nInputDialog, XmNinitialFocus, temp, NULL);
  XtUnmanageChild(XmSelectionBoxGetChild(m_nInputDialog, 
    XmDIALOG_HELP_BUTTON));

  // Create the header view/edit dialog
  ac = 0;
  m_nHeaderDialog = XmCreateFormDialog(TopLevel, "Dataset Header", al, ac);

  ac = 0;
  XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_FORM); 
  ac++;
  XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM);
  ac++;
  XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM);
  ac++;
  m_nHeaderMessage = XmCreateMessageBox (m_nHeaderDialog, "header_message",
    al, ac);
  XtUnmanageChild(XmMessageBoxGetChild(m_nHeaderMessage, 
    XmDIALOG_HELP_BUTTON));
  XtManageChild(m_nHeaderMessage);

  ac = 0;
  XtSetArg(al[ac], XmNeditMode, XmMULTI_LINE_EDIT); 
  ac++;
  XtSetArg(al[ac], XmNtopAttachment, XmATTACH_FORM); 
  ac++;
  XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM);
  ac++;
  XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM);
  ac++;
  XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_WIDGET);
  ac++;
  XtSetArg(al[ac], XmNbottomWidget, m_nHeaderMessage);
  ac++;
  XtSetArg(al[ac], XmNheight, 200);
  ac++;
  XtSetArg(al[ac], XmNwidth, 400);
  ac++;
  XtSetArg(al[ac], XmNfontList, FontList);
  ac++;
  m_nHeaderText = XmCreateScrolledText(m_nHeaderDialog, "header_text",
    al, ac);
  XtManageChild(m_nHeaderText);

  // Create the Pick dialog
  ac = 0;
  m_nPickDialog = XmCreateSelectionDialog (TopLevel, "Pick Dataset",
    al, ac);
  temp = XmSelectionBoxGetChild(m_nPickDialog, XmDIALOG_TEXT);
  XtVaSetValues(temp, XtVaTypedArg, XmNbackground, XmRString, 
  		"White", 6, NULL);
  XtUnmanageChild(XmSelectionBoxGetChild(m_nPickDialog, 
    XmDIALOG_HELP_BUTTON));

  // Create the Symbol dialog
  ac = 0;
  m_nSymbolDialog = XmCreateSelectionDialog (TopLevel, "Symbols",
    al, ac);
  temp = XmSelectionBoxGetChild(m_nSymbolDialog, XmDIALOG_TEXT);
  XtVaSetValues(temp, XtVaTypedArg, XmNbackground, XmRString, 
  		"White", 6, NULL);
  XtUnmanageChild(XmSelectionBoxGetChild(m_nSymbolDialog, 
    XmDIALOG_HELP_BUTTON));
  XtUnmanageChild(XmSelectionBoxGetChild(m_nSymbolDialog,
    XmDIALOG_OK_BUTTON));
  temp = XmSelectionBoxGetChild(m_nSymbolDialog, XmDIALOG_LIST);
  for (int i = 0; i < 7; i++)
    {
      XmListAddItem(temp, XmStringCreate((char*) SymbolNames[i], 
	    XmSTRING_DEFAULT_CHARSET), 0);      
    }

  // Create the Color dialog
  ac = 0;
  m_nColorDialog = XmCreateSelectionDialog (TopLevel, "Colors",
    al, ac);
  temp = XmSelectionBoxGetChild(m_nColorDialog, XmDIALOG_TEXT);
  XtVaSetValues(temp, XtVaTypedArg, XmNbackground, XmRString, 
  		"White", 6, NULL);
  XtUnmanageChild(XmSelectionBoxGetChild(m_nColorDialog, 
    XmDIALOG_HELP_BUTTON));
  XtUnmanageChild(XmSelectionBoxGetChild(m_nColorDialog,
    XmDIALOG_OK_BUTTON));
  temp = XmSelectionBoxGetChild(m_nColorDialog, XmDIALOG_LIST);
  for (int i = 0; i < MAX_COLORS; i++)
    {
      XmListAddItem(temp, XmStringCreate((char*) color_names[i], 
	    XmSTRING_DEFAULT_CHARSET), 0);      
    }

  // Create the Save Options dialog
 
  ac = 0;
  m_nSaveOptionsDialog = XmCreateFormDialog(TopLevel, "Save Options", al, ac); 
  Widget rowcol, btn;
  rowcol = XtVaCreateManagedWidget ("rowcolumn", xmRowColumnWidgetClass, m_nSaveOptionsDialog, NULL);

  XmString one, two, three;
  XtVaCreateManagedWidget("Header Style", xmLabelWidgetClass, rowcol, NULL); 
  one = XmStringCreateLocalized("none");
  two = XmStringCreateLocalized("xyplot");
  three = XmStringCreateLocalized("line prefix");
  m_nSaveHeader = XmVaCreateSimpleRadioBox (rowcol, "Header Style", 
					 0,
					 SaveOptionsCB,
					 XmVaRADIOBUTTON, one, NULL, NULL, NULL,
					 XmVaRADIOBUTTON, two, NULL, NULL, NULL,
					 XmVaRADIOBUTTON, three, NULL, NULL, NULL,
					 NULL);
 
  XmStringFree(one);
  XmStringFree(two);
  XmStringFree(three);

  XtVaCreateManagedWidget ("Prefix", xmLabelWidgetClass, rowcol, NULL);

  m_nSavePrefix = XmCreateTextField(rowcol, "text", NULL, 0);
  XtVaSetValues(m_nSavePrefix, XmNeditable, False, NULL);

  XtVaCreateManagedWidget ("sep", xmSeparatorGadgetClass, rowcol, NULL);

  XtVaCreateManagedWidget ("Number Format", xmLabelWidgetClass, rowcol, NULL);
  one = XmStringCreateLocalized("exponential");
  two = XmStringCreateLocalized("floating point");
  three = XmStringCreateLocalized("integer");
  m_nSaveNumberFormat = XmVaCreateSimpleRadioBox (rowcol, "Number Format",
						  0,
						  SaveOptionsCB,
						  XmVaRADIOBUTTON, one, NULL, NULL, NULL,
						  XmVaRADIOBUTTON, two, NULL, NULL, NULL,
						  XmVaRADIOBUTTON, three, NULL, NULL, NULL,
						  NULL);
  XmStringFree(one);
  XmStringFree(two);
  XmStringFree(three);

  XtVaCreateManagedWidget ("sep", xmSeparatorGadgetClass, rowcol, NULL);

  XtVaCreateManagedWidget ("Column Delimiter", xmLabelWidgetClass, rowcol, NULL);
  one = XmStringCreateLocalized("space");
  two = XmStringCreateLocalized("tab");
  three = XmStringCreateLocalized("comma");
  m_nSaveColumnDelimiter = XmVaCreateSimpleRadioBox (rowcol, "Column Delimiter",
						  0,
						  SaveOptionsCB,
						  XmVaRADIOBUTTON, one, NULL, NULL, NULL,
						  XmVaRADIOBUTTON, two, NULL, NULL, NULL,
						  XmVaRADIOBUTTON, three, NULL, NULL, NULL,
						  NULL);

  XtVaCreateManagedWidget ("sep", xmSeparatorGadgetClass, rowcol, NULL);

  XtVaCreateManagedWidget ("End of Line", xmLabelWidgetClass, rowcol, NULL);
  one = XmStringCreateLocalized("Unix");
  two = XmStringCreateLocalized("DOS");
  m_nSaveEndOfLine = XmVaCreateSimpleRadioBox (rowcol, "EOL",
						  0,
						  SaveOptionsCB,
						  XmVaRADIOBUTTON, one, NULL, NULL, NULL,
						  XmVaRADIOBUTTON, two, NULL, NULL, NULL,
						  NULL);

  XtVaCreateManagedWidget ("sep", xmSeparatorGadgetClass, rowcol, NULL);

  one = XmStringCreateLocalized("Cancel");
  btn = XtVaCreateManagedWidget("button", xmPushButtonWidgetClass, rowcol, XmNlabelString, one, NULL);
  XtAddCallback (btn, XmNactivateCallback, SaveOptionsCB, (void*) PL_SAVE_OPTIONS_CANCEL);
  two = XmStringCreateLocalized("Done");
  btn = XtVaCreateManagedWidget("button", xmPushButtonWidgetClass, rowcol, XmNlabelString, two, NULL);
  XtAddCallback (btn, XmNactivateCallback, SaveOptionsCB, (void*) PL_SAVE_OPTIONS_DONE);

  XmStringFree(one);
  XmStringFree(two);
  XmStringFree(three);

  XtManageChild(m_nSaveHeader);
  XtManageChild(m_nSavePrefix);
  XtManageChild(m_nSaveNumberFormat);
  XtManageChild(m_nSaveColumnDelimiter);
  XtManageChild(m_nSaveEndOfLine);

  // Create the Grid dialog
 
  m_nGridDialog = XmCreateFormDialog(TopLevel, "Grid Options", NULL, 0);
  rowcol = XtVaCreateManagedWidget ("rowcolumn", xmRowColumnWidgetClass, m_nGridDialog, NULL);

  m_nXGridLines = XmCreateToggleButton(rowcol, "Draw X grid lines", NULL, 0);
  XtVaSetValues (m_nXGridLines, XmNset, True, NULL);
  m_nYGridLines = XmCreateToggleButton(rowcol, "Draw Y grid lines", NULL, 0);
  XtVaSetValues (m_nYGridLines, XmNset, True, NULL);
  m_nGridAxes = XmCreateToggleButton(rowcol, "Draw Axes", NULL, 0);
  XtVaSetValues (m_nGridAxes, XmNset, True, NULL);
  XtManageChild(m_nXGridLines);
  XtManageChild(m_nYGridLines);
  XtManageChild(m_nGridAxes);

  // Create the Verify dialog
  m_nVerifyDialog = XmCreateQuestionDialog (TopLevel, "Verify", NULL, 0);
  XtVaSetValues (m_nVerifyDialog, XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL, NULL);

  // Setup the plot colors

  m_pDc->SetColors((char**) color_names, MAX_COLORS);
  SetBackgroundColor("DarkGrey");
  SetForegroundColor("black");

  OnFileNew();
}
//---------------------------------------------------------------

CPlotWindow::~CPlotWindow()
{
  // Cleanup

  delete m_pDb;
  delete m_pDi;
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
  CpsDC printDC (612, 792);
  printDC.OpenDisplay (PRINT_TEMP_FILE);
  printDC.SetColors ((char**) color_names, MAX_COLORS);
  printDC.SetForeground (printDC.GetColor("black"));

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
}
//--------------------------------------------------------------
void CPlotWindow::OnRefresh()
{
    Invalidate();
}

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

//  int i = 0;
//  while (buttons[i]) i++; // Find first non-zero entry

  XmString str = XmStringCreate (name, "Helvetica12pt");
  Widget b = XtVaCreateManagedWidget(name, xmPushButtonWidgetClass, wMenu,
				       XmNlabelString, str,
				       XmNfontList, FontList,
				       NULL);
  XtVaSetValues(b, XtVaTypedArg, XmNbackground, XmRString, 
		"LightGrey", 10, NULL);
  XmStringFree(str);  
  return b;
}
//---------------------------------------------------------------

Widget CPlotWindow::AddSubMenu (Widget wMenu, char* name)
{
  // Add a new submenu widget in wMenu, with specified name.
  // Return the pulldown menu widget, not the button.

  Widget sMenu = XmCreatePulldownMenu(wMenu, name, NULL, 0);
  XmString str = XmStringCreate (name, "Helvetica12pt");
  Widget b = XtVaCreateManagedWidget(name, xmCascadeButtonWidgetClass, wMenu,
				     XmNsubMenuId, sMenu,
				     XmNlabelString, str,
				     XmNfontList, FontList,
				     XmNmnemonic, 0,
				     NULL);
  XtVaSetValues(b, XtVaTypedArg, XmNbackground, XmRString, 
		"LightGrey", 10, NULL);
  XmStringFree(str);  
  return sMenu;
}
//---------------------------------------------------------------

Widget CPlotWindow::MakeMenu (Widget wAttach, char* menu_name, 
  char* options[], Widget buttons[], int n)
{
  char submenu_name [32];
  Widget menu, cascade;
  Arg al[10];
  int i, ac = 0;

  menu = XmCreatePulldownMenu (wAttach, menu_name, al, ac);

  XtSetArg (al[ac], XmNsubMenuId, menu);  
  ac++;
  XmString str =  XmStringCreateLtoR(menu_name, "Helvetica12pt");
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

  for (i = 0; i < n; i++)
    {
      if (*options[i] == '>')
	{
	  // Create a submenu cascade button
	 
	    strcpy (submenu_name, options[i]+1);
	    buttons[i] = AddSubMenu (menu, submenu_name);
	}
      else
	{
	  buttons[i] = AddMenuItem (menu, options[i]);
	}
    }
  return menu ;

}
//---------------------------------------------------------------

void CPlotWindow::SetBackgroundColor (char* color_name)
{
  unsigned bkg = m_pDc->GetColor(color_name);
  m_pDc->SetBackground(bkg);
  strcpy (m_szBackgroundColor, color_name);
  Invalidate();
}

void CPlotWindow::SetForegroundColor (char* color_name)
{
  unsigned fg = m_pDc->GetColor(color_name);
  m_pDc->SetForeground(fg);
  strcpy (m_szForegroundColor, color_name);

  if (m_pDi) 
    {
      CGrid* pGrid = m_pDi->m_pGrid;
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
    vector<float> x;
    
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
	vector<float> x = m_pDi->Logical(ExpRect);
        m_pDi->CreateView(CARTESIAN, x);
        Invalidate ();
     }
    
}
//---------------------------------------------------------------

void CPlotWindow::OnRButtonDown(unsigned int mf, CPoint p)
{

}
//---------------------------------------------------------------

void CPlotWindow::MessageBox (char* s)
{
// Display a message
  
  Arg a;
  XtSetArg(a, XmNmessageString, XmStringCreateLtoR(s, (char*)"Courier12pt"));
  XtSetValues (m_nAboutBox, &a, 1);
  XtManageChild(m_nAboutBox);

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

      if (select_type == 1)
	m_pDi->SetActivePlot(p);
      else
	m_pDi->SetOperandPlot(p);

      WriteStatusMessage(m_pDi->GetList());
    }
  return ;
}
//---------------------------------------------------------------

void CPlotWindow::OnExtremaInput ()
{
  Arg al[4];
  int ac = 0;
  XtSetArg(al[ac], XmNselectionLabelString, 
  XmStringCreateLtoR("Enter xmin, ymin, xmax, ymax:", "CourierBold14pt")); 
		     //    XmSTRING_DEFAULT_CHARSET));  
  ac++;
  XtSetArg(al[ac], XmNfontList, FontList);
  ac++;
  XtSetArg(al[ac], XmNtextString, XmStringCreateLtoR("",
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
  float x[6];
  vector<float> e = m_pDi->GetExtrema();
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

  Arg al[2];
  // XtSetArg(al[0], XmNlabelString,
  //	   XmStringCreate(msg, XmSTRING_DEFAULT_CHARSET));

  XtSetArg(al[0], XmNlabelString, XmStringCreate(msg, "Helvetica14pt"));
  XtSetValues (m_nStatusBar, al, 1);
  return;
}
//---------------------------------------------------------------

void CPlotWindow::WriteConsoleMessage(char* msg)
{
  // Write a message to the Forth console area;

  char s[256];
  strcpy(s, msg);
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

void CPlotWindow::OnTemplate()
{
    char tmpl[256], prompt[256];

    strcpy (prompt, "Enter x1, x2, dx:");
    if (GetInput(prompt, tmpl) == 0) return;

    float x[6], temp;
    vector<float> e = m_pDi->GetExtrema();
    x[0] = e[0];
    x[1] = e[1];
    x[2] = (x[1] - x[0])/100.f;

    char s[255];
    strcpy (s, tmpl);

    NumberParse (x, s);
    if (x[0] > x[1])
    {
        temp = x[1];
        x[1] = x[0];
        x[0] = temp;
    }

    int npts;

    if (x[2] == 0.)
    {
        npts = 1;
    }
    else
    {
        npts = 1 + (int)((x[1] - x[0])/x[2]) ;
    }

    CReal* d = new CReal (2, npts, "Template", "\0");

    if (d == NULL)
    {
// AfxMessageBox ("Unable to create template data set.");
        return;
    }

    float* data = &(*(d->begin()));
    temp = x[0];
    for (int i = 0; i < npts; i++)
    {
        *data++ = temp;
        *data++ = temp;
        temp += x[2];
    }
    d->SetExtrema();

    m_pDb->AddSet(d);
    m_pDi->MakePlot (d, 0);

    OnReset();

}
//---------------------------------------------------------------

void CPlotWindow::OnExpressionInput ()
{
  Arg al[4];

  int ac = 0;
  XtSetArg(al[ac], XmNselectionLabelString, 
    XmStringCreateLtoR("Enter the math expression:", 
    "Courier12pt"));  
  ac++;
  XtSetArg(al[ac], XmNtextString, XmStringCreateLtoR("",
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
  int i, *StackPtr;
  BYTE* TypePtr;
  ostringstream* pSS = NULL;
  
  if (*exp)
    {
      vector<BYTE> opcodes, prefix;
      pSS = new ostringstream(emsg);
      SetForthOutputStream (*pSS);
      strupr(exp);

      int ecode = CompileAE (&opcodes, exp);

      //OutputForthByteCode (&opcodes);
      //MessageBox(emsg);
 
      if (ecode)
	{
	  sprintf (emsg, "%s %d", "Expression Compiler Error:", ecode);
	  MessageBox (emsg);
	}
      else
	{
            // Perform the expression operations

	  CDataset* d = m_pDi->GetActiveSet();
	  int npts = d->NumberOfPoints();
	  int nsize = d->SizeOfDatum();
	  float *p = &(*(d->begin()));
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
	  SetForthOutputStream (*pSS);
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

void CPlotWindow::OnArithmetic()
{
  ;
}
//--------------------------------------------------------------

void CPlotWindow::OnSwap()
{
  ;
}
//---------------------------------------------------------------

void CPlotWindow::OnSmooth()
{
  ;
}
//---------------------------------------------------------------

void CPlotWindow::OnGrid()
{
  // Allow user to setup grid style
  XmToggleButtonSetState(m_nXGridLines, CGridLines::m_bVertical, False);
  XmToggleButtonSetState(m_nYGridLines, CGridLines::m_bHorizontal, False);
  XtManageChild (m_nGridDialog);
}

void CPlotWindow::OnGridToggle()
{
  (m_pDi->m_pGrid->m_pLines) ? m_pDi->m_pGrid->SetLines(false,false) :
    m_pDi->m_pGrid->SetLines(true, true);

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
  if (nsets == 0)
    {
      MessageBox ("There are no datasets in the database.");
    }
  else
    {
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
      unsigned c = GetColor(color_name);
      CPlot* p = m_pDi->GetActivePlot();
      if (p) 
	{
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
      char s[256];;
      Arg al[5];
      int ac = 0;
      // strcpy (s, d->m_szName);
      // XtSetArg(al[ac], XmNselectionLabelString, 
      //	XmStringCreateLtoR(s, XmSTRING_DEFAULT_CHARSET));  
      // ac++;

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

BOOL CPlotWindow::OnFileNew ()
{
	delete m_pDi;
	delete m_pDb;

	m_pDb = new CDatabase();	// create database and display
	m_pDi = new CPlotDisplay();

	WriteConsoleMessage("Workspace cleared.");

	OnReset();
	return TRUE;
}
//---------------------------------------------------------------

bool CPlotWindow::OnFileOpen ()
{
  XtManageChild(m_nFileOpenDialog); return True; 
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
  strcpy (s, "XYPLOT "); strcat (s, ver); strcat (s, " for Linux/X Windows");
  strcat (s, "  (kForth "); strcat (s, kfver);
  strcat (s, ")  Build: "); strcat (s, build); strcat(s, "\n"); 
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
  XtSetArg(al[ac], XmNtextString, XmStringCreateLtoR("",
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

  float x[8];
  int i, nSel = 0;
  char s[64];
  vector<int> Cols;

  if (*cs)
  {
    strcpy (s, cs);
    nSel = NumberParse(x, s);
    for (i = 0; i < nSel; i++) Cols.push_back((int) x[i] - 1);
  }

  return Cols;

}
//---------------------------------------------------------------

void CPlotWindow::LoadDatasetFile (char* fname, char* col_spec)
{
  // Load a dataset file into the database, with the columns
  //   to load specified in the col_spec

  char s[128];
  CDataset* d;

  int ecode = m_pDb->LoadDataset(&d, fname, ParseColumns(col_spec), 0);
  if (ecode <= 0)
    {
      MessageBox ("Error reading file.");
    }
  else
    {
      sprintf (s, "%d points read from %s", ecode, fname);
      WriteConsoleMessage(s);
      m_pDi->MakePlot(d, 0);
      OnReset();
    }
  *m_pFileName = '\0';
}
  
//---------------------------------------------------------------

bool CPlotWindow::LoadFile(char* fname)
{
    int i, ecode, ns;
    char s[128];

    if (*fname)
    {
        CDataset* d;

	strcpy (m_pFileName, fname);   // save filename for callbacks in case of a command-line specified file

       	if (strstr(fname, ".xsp"))
       	{
       	    CWorkspace41* pWs;
            ecode = m_pDb->LoadWorkspace (&pWs, fname);
            if (ecode)
            {
                sprintf (s, "XSP FILE CORRUPT");
                MessageBox (s);
            }
            else
            {
                for (i = 0; i < pWs->di.nplots; i++)
                {
                    ns = pWs->pv[i].set - 1;  // array origin is now 0
                    d = m_pDb->FindInList(pWs->ds[ns].fname);
                    if (d) m_pDi->MakePlot(pWs, d, i);
                }
                OnReset();
   	        delete pWs;     // delete the workspace (we're done with it)

            }

        }
        else if (strstr(fname, ".4th"))
        {
          // Load a Forth source file into the built-in Forth environment

	  ecode = LoadForthFile (fname);
        }
        else
        {
	  int nCols = m_pDb->GetColumnCount(fname);
          if (nCols < 0)
	    {
	      MessageBox ("Cannot open input file.");	    
	    }
	  else if (nCols == 0)
	    {
	      MessageBox ("Cannot find first line of data in file.");
	    }
	  else if (nCols <= 2)
	    {
	      LoadDatasetFile (fname, "");
	    }
	  else
	    {
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
    char s[256];
    XmString prompt, no;

    CDataset* d = m_pDi->GetActiveSet();
    if (d)
    {
      if (strstr(fname, ".xsp"))
	{
	  // Create an xyplot workspace file

	  success = SaveWorkspace(fname);
	}
      else
	{
	  // Save the active data set

	  vector<float> e = m_pDi->GetExtrema();
	  int bOverlap;
	  vector<int> lim = d->IndexLimits(e[0], e[1], bOverlap);
	  if (bOverlap)
	    {
	      sprintf (s, "Source file is %s\n", d->m_szName);
	      AddToHeader (s, d->m_szHeader, True);
	      // d->PrependToHeader(s);

	      if ((lim[0] > 0) || (lim[1] < (d->NumberOfPoints()-1)))
		{
		  /*
		    CVerifyDialog::m_szPromptString =
                    "Save only data in window domain?";
		    CVerifyDialog vd;
		    int result = vd.DoModal();
		  */
		    verify_answer = 0;
		    prompt = XmStringCreateLocalized("Save only data in window domain?");
		    no = XmStringCreateLocalized("Save All");
		    XtVaSetValues(m_nVerifyDialog, 
				  XmNmessageString, prompt, 
				  XmNhelpLabelString, no,
				  NULL);
		    XmStringFree(prompt); XmStringFree(no);
		    XtManageChild(m_nVerifyDialog);
		    XtPopup (XtParent(m_nVerifyDialog), XtGrabNone);
		    while (verify_answer == 0) XtProcessEvent (XtIMAll);
		    XtPopdown (XtParent(m_nVerifyDialog));
		    
		  if (verify_answer == XmCR_HELP)
		  {
                    lim[0] = 0;
                    lim[1] = d->NumberOfPoints()-1;
		  }

		  if (verify_answer == XmCR_CANCEL)
		  {
                    WriteConsoleMessage ("Save cancelled.");
		    return success;
		  }
		}
	      
	      
	  int fd;
	  if (fd = open(fname, O_RDONLY) != -1)
	  {
	      close(fd);
	      verify_answer = 0;
	      prompt = XmStringCreateLocalized("File already exists. Overwrite?");
	      no = XmStringCreateLocalized("No");
	      XtVaSetValues(m_nVerifyDialog, XmNmessageString, prompt, XmNhelpLabelString, no, NULL);
	      XtManageChild(m_nVerifyDialog);
	      XtPopup (XtParent(m_nVerifyDialog), XtGrabNone);
	      while (verify_answer == 0) XtProcessEvent (XtIMAll);
	      XtPopdown (XtParent(m_nVerifyDialog));
	      
	      if (verify_answer == XmCR_OK) 
		  success = m_pDb->SaveDataset (d, lim, fname);
	      else
	      {
		  WriteConsoleMessage ("Save cancelled.");
		  return success;
	      }
	  }
	  else
	      success = m_pDb->SaveDataset (d, lim, fname);

	      if (success)
		{
		  strcpy (d->m_szName, fname);
		  sprintf (s, "%d points written to %s", lim[1]-lim[0]+1,
			   fname);
		  WriteConsoleMessage (s);
		}
	      else
		{
		  MessageBox ("Error writing output file.");
		}
	    }
	  else
	    {
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











