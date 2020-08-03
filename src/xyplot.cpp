/*
XYPLOT.CPP

  Copyright (c) 1995--2020 Krishna Myneni
  <krishna.myneni@ccreweb.org>

  This software is provided under the terms of the
  GNU General Public License (GPL), v3.0 or later.

Written by:
        Krishna Myneni

Contributions by:
        John Kielkopf
        Bryan Frazar

System: Linux/X Windows/Motif
*/

#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;
#include "fbc.h"
#include "ForthCompiler.h"
#include "ForthVM.h"
#include "xyplot.h"
#include <Xm/XmStrDefs.h>

void ExitCB (Widget, void*, void*);
void ReDrawCB (Widget, void*, void*);
void FileMenuCB (Widget, void*, void*);
void EditMenuCB (Widget, void*, void*);
void PlotMenuCB (Widget, void*, void*);
void MathMenuCB (Widget, void*, void*);
void AboutCB (Widget, void*, void*);
void ForthCB (Widget, void*, void*);
void SaveOptionsCB (Widget, void*, void*);
void RadioToggledCB (Widget, void*, void*);
void InputCB (Widget, void*, void*);
void VerifyCB (Widget, void*, void*);
void FileOpenCB (Widget, void*, void*);

void CoordinatesEH (Widget, XtPointer, XEvent*, Boolean*);
void ButtonEH (Widget, XtPointer, XEvent*, Boolean*);
void KeysEH (Widget, XtPointer, XEvent*, Boolean*);
void StatusButtonEH (Widget, XtPointer, XEvent*, Boolean*);

extern int* GlobalSp;
extern byte* GlobalTp;
#define MAX_COLORS 32
extern COLORREF colors_rgb_table[];
extern char* color_names[];
extern "C" char* strupr(char*);

Widget TopLevel;

Widget RadioButtons[16];

extern Widget FileWidgets[];
extern Widget EditWidgets[];
extern Widget PlotWidgets[];
extern Widget MathWidgets[];
extern Widget HelpWidgets[];

int debug = False;                   // global variable for debug mode
volatile int InputData;
volatile int verify_answer;
XmString old_filter;

// Template array for Forth interface functions

IfcFuncTemplate IfcFuncList[] = {
	{ (const void*) get_color_map,     "FN_GET_COLOR_MAP"    },       
	{ (const void*) get_active_set,    "FN_GET_ACTIVE_SET"   },
	{ (const void*) get_operand_set,   "FN_GET_OPERAND_SET"  },
	{ (const void*) get_active_plot,   "FN_GET_ACTIVE_PLOT"  },
	{ (const void*) get_operand_plot,  "FN_GET_OPERAND_PLOT" },
	{ (const void*) set_active_plot,   "FN_SET_ACTIVE_PLOT"  },
	{ (const void*) set_operand_plot,  "FN_SET_OPERAND_PLOT" },
	{ (const void*) get_ds,            "FN_GET_DS"           },
	{ (const void*) set_ds_extrema,    "FN_SET_DS_EXTREMA"   },
	{ (const void*) make_ds,           "FN_MAKE_DS"          },
	{ (const void*) get_plot,          "FN_GET_PLOT"         },
	{ (const void*) drop_plot,         "FN_DROP_PLOT"        },
	{ (const void*) make_plot,         "FN_MAKE_PLOT"        },
	{ (const void*) set_plot_symbol,   "FN_SET_PLOT_SYMBOL"  },
	{ (const void*) set_plot_color,    "FN_SET_PLOT_COLOR"   },
	{ (const void*) set_plot_rgbcolor, "FN_SET_PLOT_RGBCOLOR"},
	{ (const void*) draw_plot,         "FN_DRAW_PLOT"        },
	{ (const void*) get_grid,          "FN_GET_GRID"         },
	{ (const void*) set_grid_tics,     "FN_SET_GRID_TICS"    },
	{ (const void*) set_grid_lines,	   "FN_SET_GRID_LINES"   },
	{ (const void*) get_window_title,  "FN_GET_WINDOW_TITLE" },
	{ (const void*) set_window_title,  "FN_SET_WINDOW_TITLE" },
	{ (const void*) clear_window,      "FN_CLEAR_WINDOW"     },
	{ (const void*) draw_window,       "FN_DRAW_WINDOW"      },
	{ (const void*) reset_window,      "FN_RESET_WINDOW"     },
	{ (const void*) get_window_limits, "FN_GET_WINDOW_LIMITS" },
	{ (const void*) set_window_limits, "FN_SET_WINDOW_LIMITS" },
	{ (const void*) add_menu_item,     "FN_ADD_MENU_ITEM"     },
	{ (const void*) make_menu,         "FN_MAKE_MENU"         },
	{ (const void*) make_submenu,      "FN_MAKE_SUBMENU"      },
	{ (const void*) set_background,    "FN_SET_BACKGROUND"    },
	{ (const void*) set_foreground,    "FN_SET_FOREGROUND"    }, 
	{ (const void*) radio_box,         "FN_RADIO_BOX"         },
	{ (const void*) message_box,  	   "FN_MESSAGE_BOX"       },
	{ (const void*) get_input,         "FN_GET_INPUT"         },
	{ (const void*) file_open_dialog,  "FN_FILE_OPEN_DIALOG"  },
	{ (const void*) file_save_dialog,  "FN_FILE_SAVE_DIALOG"  },
	{ (const void*) set_save_options,  "FN_SET_SAVE_OPTIONS"  }
};

vector<char*> ForthMenuCommandList;

CPlotWindow* pMainWnd;

int main(int argc, char* argv[])
{

  TopLevel = XtInitialize(argv[0],"",NULL,0,&argc,argv);
  XtRealizeWidget(TopLevel);
  
  // Create the main window

  pMainWnd = new CPlotWindow(argc, argv);

  // Initialize the Forth environment

  OpenForth();       
  InitForthInterface();

  // Load initialization file (xyplot.4th in user's .xyplot directory)

  LoadInitializationFile();

  // Load any files specified on the command line

  vector<char*> FileList = GetStartupFileList(argc, argv);
  if (FileList.size())
    {
        vector<char*>::iterator i;

        for (i = FileList.begin(); i < FileList.end(); i++)
        {
            pMainWnd->LoadFile(*i);
        }
    }
  else
    pMainWnd->WriteConsoleMessage ("Ready!");


  // Setup callbacks and event handlers

  XtAddCallback (pMainWnd->m_nPlotWindow, XmNexposeCallback, 
    ReDrawCB, (void*) PL_REDRAW);
  XtAddCallback (pMainWnd->m_nPlotWindow, XmNresizeCallback,
    ReDrawCB, (void*) PL_RESIZE);
  XtAddEventHandler (pMainWnd->m_nPlotWindow, PointerMotionMask,
    False, CoordinatesEH, NULL);
  XtAddEventHandler (pMainWnd->m_nPlotWindow, ButtonPressMask,
    False, ButtonEH, NULL);
  XtAddEventHandler (pMainWnd->m_nPlotWindow, KeyPressMask,
    False, KeysEH, NULL);
  XtAddCallback (pMainWnd->m_nFileOpenDialog, XmNokCallback,
    FileMenuCB, (void*) PL_FILE_OPEN);
  XtAddCallback (pMainWnd->m_nFileOpenDialog, XmNcancelCallback,
    FileMenuCB, (void*) PL_FILE_CANCEL);
  XtAddCallback (pMainWnd->m_nFileSaveDialog, XmNokCallback,
    FileMenuCB, (void*) PL_FILE_SAVE);
  XtAddCallback (pMainWnd->m_nFileSaveDialog, XmNcancelCallback,
    FileMenuCB, (void*) PL_SAVE_CANCEL);
  XtAddEventHandler (pMainWnd->m_nStatusBar, ButtonPressMask,
    False, StatusButtonEH, NULL);
  XtAddCallback (HelpWidgets[ID_ABOUT], XmNactivateCallback,
    AboutCB, NULL);
  XtAddCallback (FileWidgets[ID_FILE_NEW],  XmNactivateCallback, 
    FileMenuCB, (void*) PL_NEW);
  XtAddCallback (FileWidgets[ID_FILE_OPEN], XmNactivateCallback,
    FileMenuCB, (void*) PL_OPEN);
  XtAddCallback (FileWidgets[ID_FILE_SAVE], XmNactivateCallback,
    FileMenuCB, (void*) PL_SAVE);
  XtAddCallback (FileWidgets[ID_SAVE_OPTIONS], XmNactivateCallback,
    SaveOptionsCB, (void*) PL_SAVE_OPTIONS);
  XtAddCallback (FileWidgets[ID_FILE_PRINT], XmNactivateCallback,
    FileMenuCB, (void*) PL_PRINT);
  XtAddCallback (FileWidgets[ID_APP_EXIT], XmNactivateCallback,
    ExitCB, NULL);
  XtAddCallback (EditWidgets[ID_EDIT_HDR], XmNactivateCallback,
    EditMenuCB, (void*) PL_HEADER);
  XtAddCallback (pMainWnd->m_nHeaderMessage, XmNokCallback,
    EditMenuCB, (void*) PL_HEADER_OK);
  XtAddCallback (pMainWnd->m_nHeaderMessage, XmNcancelCallback,
    EditMenuCB, (void*) PL_HEADER_CANCEL);
  XtAddCallback (EditWidgets[ID_EDIT_DEL], XmNactivateCallback,
    EditMenuCB, (void*) PL_DELETE);
  XtAddCallback (EditWidgets[ID_EDIT_DUP], XmNactivateCallback,
    EditMenuCB, (void*) PL_DUPLICATE);
  XtAddCallback (PlotWidgets[ID_PLOT_PICK], XmNactivateCallback,
    PlotMenuCB, (void*) PL_PICK);
  XtAddCallback (pMainWnd->m_nPickDialog, XmNokCallback,
    PlotMenuCB, (void*) PL_PICK_DATA);
  XtAddCallback (PlotWidgets[ID_PLOT_SYMBOL], XmNactivateCallback,
    PlotMenuCB, (void*) PL_SYMBOL);
  XtAddCallback (pMainWnd->m_nSymbolDialog, XmNapplyCallback,
    PlotMenuCB, (void*) PL_SET_SYMBOL);
  XtAddCallback (PlotWidgets[ID_PLOT_COLOR], XmNactivateCallback,
    PlotMenuCB, (void*) PL_COLOR);
  XtAddCallback (pMainWnd->m_nColorDialog, XmNapplyCallback,
    PlotMenuCB, (void*) PL_SET_COLOR);
  XtAddCallback (PlotWidgets[ID_PLOT_EXTREMA], XmNactivateCallback,
    PlotMenuCB, (void*) PL_EXTREMA_INPUT);
  XtAddCallback (PlotWidgets[ID_PLOT_DROP], XmNactivateCallback,
    PlotMenuCB, (void*) PL_DROP);
  XtAddCallback (PlotWidgets[ID_PLOT_GRID], XmNactivateCallback,
    PlotMenuCB, (void*) PL_GRID);
  XtAddCallback (pMainWnd->m_nGridXlines, XmNvalueChangedCallback,
    PlotMenuCB, (void*) PL_GRID_LINES);
  XtAddCallback (pMainWnd->m_nGridYlines, XmNvalueChangedCallback,
    PlotMenuCB, (void*) PL_GRID_LINES);
  XtAddCallback (pMainWnd->m_nGridXaxis, XmNvalueChangedCallback,
    PlotMenuCB, (void*) PL_GRID_AXES);
  XtAddCallback (pMainWnd->m_nGridYaxis, XmNvalueChangedCallback,
    PlotMenuCB, (void*) PL_GRID_AXES);
  XtAddCallback (MathWidgets[ID_MATH_EXPRESSION], XmNactivateCallback,
    MathMenuCB, (void*) PL_EXPRESSION_INPUT);
  XtAddCallback (pMainWnd->m_nVerifyDialog, XmNokCallback, 
    VerifyCB, (void*) &verify_answer);
  XtAddCallback (pMainWnd->m_nVerifyDialog, XmNcancelCallback,
    VerifyCB, (void*) &verify_answer);
  XtAddCallback (pMainWnd->m_nVerifyDialog, XmNhelpCallback,
    VerifyCB, (void*) &verify_answer);
  XtAddCallback (pMainWnd->m_nForthShell, XmNvalueChangedCallback,
    ForthCB, NULL);

  // Disable menu functions not yet implemented

  // XtSetSensitive (EditWidgets[ID_EDIT_COPY], False);

  XtMainLoop();

  return 0;
}
//------------------------------------------------------------------------

void ExitCB (Widget w, void* client_d, void* call_d)
{
  caddr_t client_data = (caddr_t) client_d;
  XmAnyCallbackStruct* call_data = (XmAnyCallbackStruct*) call_d;

  delete pMainWnd;
  CloseForth();

  vector<char*>* v = &ForthMenuCommandList;
  v->erase(v->begin(), v->end());

  exit(0);
}

void ReDrawCB (Widget w, void* client_d, void* call_d)
{
  caddr_t client_data = (caddr_t) client_d;
  XmAnyCallbackStruct* call_data = (XmAnyCallbackStruct*) call_d;

  int i = (int) client_data;
  switch (i)
    {
    case PL_REDRAW:
      pMainWnd->OnPaint();
      break;
    case PL_RESIZE:
      pMainWnd->OnResize();
      break;
    default:
      ;
    }
}

void VerifyCB (Widget w, void* client_d, void* call_d)
{
  int* answer = (int*) client_d;
  XmAnyCallbackStruct* call_data = (XmAnyCallbackStruct*) call_d;
  *answer = call_data->reason;
}
//--------------------------------------------------------------

void CoordinatesEH (Widget w, XtPointer client_data,
  XEvent* event, Boolean* continue_to_dispatch)
{
  CPoint p(event->xmotion.x, event->xmotion.y);
  pMainWnd->OnMouseMove(0, p);
}

void ButtonEH (Widget w, XtPointer client_data,
  XEvent* event, Boolean* continue_to_dispatch)
{
  CPoint p(event->xmotion.x, event->xmotion.y);
  pMainWnd->OnLButtonDown(0, p);
}

void KeysEH (Widget w, XtPointer client_data,
  XEvent* event, Boolean* continue_to_dispatch)
{
  KeySym key;
  XComposeStatus compose;
  char cl [64];

  if (event->type == KeyPress)
    {
      XLookupString (&event->xkey, cl, 64, &key, &compose);
      switch (key)
	{
	case 'D': case 'd':
	  pMainWnd->OnDrop();
	  break;
	case 'E': case 'e':
	  pMainWnd->OnExtremaInput();
	  break;
	case 'F': case 'f':
	  pMainWnd->OnFileOpen();
	  break;
	case 'G': case 'g':
	  pMainWnd->OnGridToggle();
	  break;
	case 'H': case 'h':
	  pMainWnd->OnHeader();
	  break;
	case 'I': case 'i':
	  pMainWnd->OnPick();
	  break;
	case 'R': case 'r':
	  pMainWnd->OnExpressionInput();;
	  break;
	case XK_Escape:
	  pMainWnd->OnDeleteView();
	  break;
	case XK_period:
	  pMainWnd->OnSymbolToggle();
	  break;
	case XK_F9:
	  pMainWnd->OnFileSave();
	  break;
	default:
	  if ((key > '0') && (key <= '9'))
	    {
	      // set the active plot
	      int nPlot = key - '0';
	      --nPlot;
	      pMainWnd->SelectPlot(nPlot, 1);
	    }
	  ;
	}
    }  
}
//-------------------------------------------------------------

void StatusButtonEH (Widget w, XtPointer client_data,
  XEvent* event, Boolean* continue_to_dispatch)
{
  // Handle button press in the plot list area

  CPoint p(event->xmotion.x, event->xmotion.y);
  XEvent report;
  Display* display = XtDisplay(pMainWnd->m_nStatusBar);

  // Wait for button to be released

  while (1)
    {
      XNextEvent(display, &report);
      if (report.type == ButtonRelease)
	{
	  // Determine the selected plot number and make it
	  //   the active plot or operand plot depending on the button 

	  int nPlot = (p.x/7 + 1)/3;

	  if (event->xbutton.button == 1)
	    pMainWnd->SelectPlot(nPlot, 1);
	  else
	    pMainWnd->SelectPlot(nPlot, 2);

	  // cout << event->xbutton.button << ' ' << ' ' << p.x << ' ' << nPlot << '\n';
	  break;
	}
      else
	{	
	   ;
	}      
    }
}

//--------------------------------------------------------------

void AboutCB (Widget w, void* client_d, void* call_d)
{
  caddr_t client_data = (caddr_t) client_d;
  XmAnyCallbackStruct* call_data = (XmAnyCallbackStruct*) call_d;

  pMainWnd->OnAbout();
}
//-------------------------------------------------------------

void FileMenuCB (Widget w, void* client_d, void* call_d)
{
  caddr_t client_data = (caddr_t) client_d;
  XmAnyCallbackStruct* call_data = (XmAnyCallbackStruct*) call_d;

  XmFileSelectionBoxCallbackStruct* sel;
  XmSelectionBoxCallbackStruct* sel2;
  XmToggleButtonCallbackStruct* sel3;
  CDatabase* pDb;
  char* filename;
  char* s;
  int i = (int) client_data;

  switch (i)
    {
    case PL_NEW:
      pMainWnd->OnFileNew();
      break;
    case PL_OPEN:
      pMainWnd->OnFileOpen();
      break;
    case PL_FILE_OPEN:
      XtUnmanageChild(pMainWnd->m_nFileOpenDialog);
      sel = (XmFileSelectionBoxCallbackStruct *) call_data;
      XmStringGetLtoR(sel->value, XmSTRING_DEFAULT_CHARSET, &filename);
      strcpy (pMainWnd->m_pFileName, filename);
      pMainWnd->LoadFile(filename);
      break;
    case PL_FILE_LOAD:
      XtUnmanageChild (pMainWnd->m_nInputDialog);
      sel2 = (XmSelectionBoxCallbackStruct *) call_data;
      XmStringGetLtoR(sel2->value, XmSTRING_DEFAULT_CHARSET, &s);
      if (*(pMainWnd->m_pFileName))
	pMainWnd->LoadDatasetFile(pMainWnd->m_pFileName, s);
      break;
    case PL_LOAD_CANCEL:
      XtUnmanageChild(pMainWnd->m_nInputDialog);
      break;
    case PL_FILE_CANCEL:
      XtUnmanageChild(pMainWnd->m_nFileOpenDialog);
      break;
    case PL_SAVE:
      pMainWnd->OnFileSave();
      break;
    case PL_FILE_SAVE:
      XtUnmanageChild (pMainWnd->m_nFileSaveDialog);
      sel = (XmFileSelectionBoxCallbackStruct *) call_data;
      XmStringGetLtoR(sel->value, XmSTRING_DEFAULT_CHARSET, &filename);
      pMainWnd->SaveFile(filename);
      break;
    case PL_SAVE_CANCEL:
      XtUnmanageChild (pMainWnd->m_nFileSaveDialog);
      break;

    case PL_PRINT:
      pMainWnd->OnPrint();
      break;
    default:
      break;
    }
}
//------------------------------------------------------------------

void SaveOptionsCB (Widget w, void* client_d, void* call_d)
{
    Widget parent = XtParent(w);
    CDatabase* pDb = pMainWnd->m_pDb;
    static SaveOptions o;
    char *prefix;

    int data = (int) client_d;

    if (data >= PL_SAVE_OPTIONS)
    {
	switch (data) {
	  case PL_SAVE_OPTIONS:
	      o = pDb->GetSaveOptions();
	      Widget w;
	      WidgetList wlist;
	      int i, nchildren, n;
	      XtVaGetValues(pMainWnd->m_nSaveHeader, XmNnumChildren, &nchildren, NULL);
	      XtVaGetValues(pMainWnd->m_nSaveHeader, XmNchildren, &wlist, NULL);
	      n = o.HeaderType;
	      for (i = 0; i < nchildren; i++) XmToggleButtonGadgetSetState(wlist[i], i==n, False);
	      XtVaGetValues(pMainWnd->m_nSaveNumberFormat, XmNnumChildren, &nchildren, XmNchildren, &wlist, NULL);
	      n = o.NumberFormat;
	      for (i = 0; i < nchildren; i++) XmToggleButtonGadgetSetState(wlist[i], i==n, False);
	      XtVaGetValues(pMainWnd->m_nSaveColumnDelimiter, XmNnumChildren, &nchildren, XmNchildren, &wlist, NULL);
	      n = o.Delimiter;
	      for (i = 0; i < nchildren; i++) XmToggleButtonGadgetSetState(wlist[i], i==n, False);
	      
	      XmTextSetString(pMainWnd->m_nSavePrefix, (char*) o.UserPrefix);
	      if (o.HeaderType == 2) XtVaSetValues (pMainWnd->m_nSavePrefix, XmNeditable, True, NULL);
 
	      XtManageChild(pMainWnd->m_nSaveOptionsDialog);
	      break;
	  case PL_SAVE_OPTIONS_CANCEL:
	      XtUnmanageChild(pMainWnd->m_nSaveOptionsDialog);
	      break;
	  case PL_SAVE_OPTIONS_DONE:
	      if (o.HeaderType == 2)
	      {
		  prefix = XmTextFieldGetString(pMainWnd->m_nSavePrefix);
		  strcpy( (char*) o.UserPrefix, prefix);
		  XtFree(prefix);
	      }
	      pDb->SetSaveOptions(o);
	      XtUnmanageChild(pMainWnd->m_nSaveOptionsDialog);
	      break;
	}
    }
    else
    {

	XmToggleButtonCallbackStruct *state = (XmToggleButtonCallbackStruct *) call_d;
	if (parent == pMainWnd->m_nSaveHeader)
	{
	    if (state->set) 
		{
		    o.HeaderType = data;
		    XtVaSetValues (pMainWnd->m_nSavePrefix, XmNeditable, o.HeaderType==2,  NULL);
		}
	}
	else if (parent == pMainWnd->m_nSaveNumberFormat)
	{
	    if (state->set)  o.NumberFormat = data;
	}
	else if (parent == pMainWnd->m_nSaveColumnDelimiter)
	{
	    if (state->set) o.Delimiter = data;
	}
	else if (parent == pMainWnd->m_nSaveEndOfLine)
	{
	    if (state->set) o.CrLf = data;
	}
	else
	    ;

    }
}
//------------------------------------------------------------------

void EditMenuCB (Widget w, void* client_d, void* call_d)
{
  caddr_t client_data = (caddr_t) client_d;
  XmAnyCallbackStruct* call_data = (XmAnyCallbackStruct*) call_d;

  int i = (int) client_data;

  switch (i)
    {
    case PL_HEADER:
      pMainWnd->OnHeader();
      break;
    case PL_HEADER_OK:
      XtUnmanageChild(pMainWnd->m_nHeaderDialog);
      pMainWnd->OnSetHeader(XmTextGetString(pMainWnd->m_nHeaderText));
      pMainWnd->OnSetDatasetName(XmTextGetString(pMainWnd->m_nDatasetName));
      break;
    case PL_HEADER_CANCEL:
      XtUnmanageChild(pMainWnd->m_nHeaderDialog);
      break;
    case PL_DELETE:
      pMainWnd->OnDelete();
      break;
    case PL_DUPLICATE:
      pMainWnd->OnDuplicate();
      break;
    default:
      break;
    }
}
//-------------------------------------------------------------

void PlotMenuCB (Widget w, void* client_d, void* call_d)
{
  caddr_t client_data = (caddr_t) client_d;
  XmAnyCallbackStruct* call_data = (XmAnyCallbackStruct*) call_d;

  char *s, *sXtics, *sYtics;
  XmSelectionBoxCallbackStruct *selection;
  XmToggleButtonCallbackStruct* sel3;
  int n, nXtics, nYtics;
  bool bXlines, bYlines, bXaxis, bYaxis;
  CPlotView* pView;

  int i = (int) client_data;
  switch(i)
    {
    case PL_EXTREMA_INPUT:
      pMainWnd->OnExtremaInput();
      break;
    case PL_EXTREMA:
      selection=(XmSelectionBoxCallbackStruct *) call_data;
      XmStringGetLtoR(selection->value, XmSTRING_DEFAULT_CHARSET, &s);
      XtRemoveCallback (pMainWnd->m_nInputDialog, XmNokCallback,
        PlotMenuCB, (void*) PL_EXTREMA);
      XtRemoveCallback (pMainWnd->m_nInputDialog, XmNcancelCallback,
	PlotMenuCB, (void*) PL_EXTREMA_CANCEL);
      XtUnmanageChild (pMainWnd->m_nInputDialog);
      pMainWnd->OnExtrema(s);
      break;
    case PL_EXTREMA_CANCEL:
      XtRemoveCallback (pMainWnd->m_nInputDialog, XmNokCallback,
        PlotMenuCB, (void*) PL_EXTREMA);
      XtRemoveCallback (pMainWnd->m_nInputDialog, XmNcancelCallback,
	PlotMenuCB, (void*) PL_EXTREMA_CANCEL);
      XtUnmanageChild (pMainWnd->m_nInputDialog);
      break;
    case PL_DROP:
      pMainWnd->OnDrop();
      break;
    case PL_PICK:
      pMainWnd->OnPick();
      break;
    case PL_PICK_DATA:
      pMainWnd->OnPickData();
      break;
    case PL_SYMBOL:
      pMainWnd->OnSymbol();
      break;
    case PL_SET_SYMBOL:
      pMainWnd->OnSetSymbol();
      break;
    case PL_COLOR:
      pMainWnd->OnColor();
      break;
    case PL_SET_COLOR:
      pMainWnd->OnSetColor();
      break;
    case PL_GRID:
      pMainWnd->OnGrid();
      break;
    case PL_GRID_LINES:
      bXlines = XmToggleButtonGetState(pMainWnd->m_nGridXlines);
      bYlines = XmToggleButtonGetState(pMainWnd->m_nGridYlines);
      pView = pMainWnd->m_pDi->GetCurrentView(); 
      pView->m_pGrid->SetLines(bXlines, bYlines);
      pMainWnd->Invalidate();
      break;
    case PL_GRID_AXES:
      bXaxis = XmToggleButtonGetState(pMainWnd->m_nGridXaxis);
      bYaxis = XmToggleButtonGetState(pMainWnd->m_nGridYaxis);
      pView = pMainWnd->m_pDi->GetCurrentView(); 
      pView->m_pGrid->SetAxes(bXaxis, bYaxis);
      pMainWnd->Invalidate();
      break;
    case PL_SET_GRID_TICS:
      sXtics = XmTextFieldGetString(pMainWnd->m_nGridXtics);
      sYtics = XmTextFieldGetString(pMainWnd->m_nGridYtics);
      pView = pMainWnd->m_pDi->GetCurrentView();
      pView->m_pGrid->GetTics(&nXtics, &nYtics);
      if (sXtics) {
	n = atoi(sXtics);
	if ((n > 0) && (n <= MAX_TICS)) nXtics = n;
      }
      if (sYtics) {
	n = atoi(sYtics);
	if ((n > 0) && (n <= MAX_TICS)) nYtics = n;
      }
      pView->m_pGrid->SetTics(nXtics, nYtics);
      pMainWnd->Invalidate();
      break;
    default:
      break;
    }
}
//----------------------------------------------------------------

void MathMenuCB (Widget w, void* client_d,
  void* call_d)
{
  caddr_t client_data = (caddr_t) client_d;
  XmAnyCallbackStruct* call_data = (XmAnyCallbackStruct*) call_d;

  char *s;
  XmSelectionBoxCallbackStruct *selection;

  int i = (int) client_data;
  switch(i)
    {
    case PL_EXPRESSION_INPUT:
      pMainWnd->OnExpressionInput();
      break;
    case PL_EXPRESSION:
      selection=(XmSelectionBoxCallbackStruct *) call_data;
      XmStringGetLtoR(selection->value, XmSTRING_DEFAULT_CHARSET, &s);
      XtRemoveCallback (pMainWnd->m_nInputDialog, XmNokCallback, 
	MathMenuCB, (void*) PL_EXPRESSION);
      XtRemoveCallback (pMainWnd->m_nInputDialog, XmNcancelCallback, 
	MathMenuCB, (void*) PL_EXPRESSION_CANCEL);  
      XtUnmanageChild (pMainWnd->m_nInputDialog);
      pMainWnd->OnExpression(s);
      break;
    case PL_EXPRESSION_CANCEL:
      XtRemoveCallback (pMainWnd->m_nInputDialog, XmNokCallback, 
	MathMenuCB, (void*) PL_EXPRESSION);
      XtRemoveCallback (pMainWnd->m_nInputDialog, XmNcancelCallback, 
	MathMenuCB, (void*) PL_EXPRESSION_CANCEL);  
      XtUnmanageChild (pMainWnd->m_nInputDialog);
      break;
    default:
      break;
    }
}
//---------------------------------------------------------------

void ForthCB (Widget wSrc, void* client_d,
  void* call_d)
{
  caddr_t client_data = (caddr_t) client_d;
  XmAnyCallbackStruct* call_data = (XmAnyCallbackStruct*) call_d;

  Widget w = pMainWnd->m_nForthShell;
  char s[256];
  int nError, nPos, nLen;
  *s = '\0';

  nPos = XmTextGetLastPosition(w);
  static bool ignore = False;

  if (ignore)
    {
      ignore = False;
      return;
    }

  char* fs = XmTextGetString(w); 

  if (client_data)
    {
      strcpy (s, (char*) client_data);
    }
  else
    {
      if (nPos)
	{
	  --nPos;
	  if (fs[nPos] == '\n')  // was Enter pressed?
	    {
	      char* cp = fs + nPos;
	      while (nPos)              // find the beginning of this line
		{
		  --cp; --nPos;
		  if (*cp == '\n') break;	      
		}
	      strcpy (s, cp);           // copy the last line and execute
	    }
	  else
	    {
	      return;
	    }
	}
      else
	return;
    }

  char out_s[256];
  long int nLine;
  stringstream ForthMessages;
  nError = ExecuteForthExpression (s, (ostringstream*) &ForthMessages, &nLine);
  ForthMessages.getline(out_s, 255, 0);

  if (nError)
  {
      if (nError & 0x100)
	  sprintf (s, "Compiler Error %d\n", nError & 0xFF);
      else
	  sprintf (s, "VM Error %d\n", nError);
  }
  else
      strcpy (s, " ok\n");

  strcat (out_s, s);
  nLen = strlen(fs);
  XtFree(fs);
  ignore = True;

  XmTextInsert (w, nLen, out_s);

  nPos = XmTextGetLastPosition (w);
  XmTextShowPosition(w, nPos);
  XmTextSetInsertionPosition(w, nPos);

}
//---------------------------------------------------------------

int BlankLine (char* s)
{
// Return TRUE if the line is blank

    char* p = s;
    while (*p != '\0')
    {
        if ((*p != ' ') && (*p != '\t') && (*p != '\n')) return 0;
        ++p;
    }

    return 1;
}
//---------------------------------------------------------------

int NumberParse (float* nums, char* s)
{
// Parse a string into array of floats; Return number of values

    const char d1 = 44; // comma
    const char d2 = 9;  // tab
    const char d3 = 32; // space
    char* p = s, *d, *ep = s + strlen(s);

    if (*s == '\0' || *s == '\r' || *s == 27) return 0;
	if (BlankLine(s)) return 0;

	int n = 0;

	do
	{
      d = strchr (p, d1);
      if (!d)
      {
        d = strchr (p, d2);
        if (!d)
        {
          d = strchr (p, d3);
        }
      }

	  if (!d)       // last delimited value
	  {
        nums [n] = atof (p);
        ++n;
        break;
	  }

      if (d == p)
      {
        if (*p == d1 || *p == d2) ++n;
        ++p;
      }
      else
      {
        nums [n] = atof (p);
        ++n;
        p = d + 1;
      }

	} while (p < ep) ;

    return n;

}
//---------------------------------------------------------------

void SortRect (CRect* prect)
{
	CPoint p1 = prect->TopLeft();
	CPoint p2 = prect->BottomRight();

	int i;

	if (p2.x < p1.x)
	{
		i = p1.x;
		p1.x = p2.x;
		p2.x = i;
	}
	if (p2.y < p1.y)
	{
		i = p1.y;
		p1.y = p2.y;
        p2.y = i;
	}
	prect->SetRect (p1.x, p1.y, p2.x, p2.y);
}
//---------------------------------------------------------------------

char* LabelFormat (float x1, float x2, char axis)
{
// Determine label output format for range of numbers between x1 and x2.

    static char format[255];

    float diff = fabs (x2 - x1);
    int j = (x1 < 0.) || (x2 < 0.);

	switch (axis)
	{
	  case 'Y':
        if ((diff < .001f) || (diff >= 100000.f))
          strcpy (format, "%9.1e");
        else if ((diff >= .001f) && (diff < 1.f))
          strcpy (format, "%6.4f");
        else if ((diff >= 1.f) && (diff < 10.f))
          strcpy (format, "%6.3f");
        else if ((diff >= 10.f) && (diff < 100.f))
          strcpy (format, "%6.2f");
        else if ((diff >= 100.f) && (diff < 1000.f))
          strcpy (format, "%6.1f");
        else
          strcpy (format, "%6.0f");
	    break;
	  case 'X':
        strcpy (format, DisplayFormat(x1, x2));
        break;
      default:
        ;
    }

    return format;

}
//----------------------------------------------------------------

char* DisplayFormat (float x1, float x2)
{
// Determine display format for range of numbers between x1 and x2.

    static char format[255];
    float diff = fabs ((x2 - x1));
    float a1 = fabs (x1);
    float a2 = fabs (x2);
    float vmax = (a2 > a1) ? a2 : a1;
    int field_width = 6;
    int precision = 4;

	if ((diff < .01f) || (diff >= 100000.f))
	  field_width += 6;
	else if ((diff >= .01f) && (diff < .1f))
	  ++precision;
	else if ((diff >= .1f) && (diff < 1.f))
	  ;
	else if ((diff >= 1.f) && (diff < 10.f))
	  --precision;
	else if ((diff >= 10.f) && (diff < 100.f))
	  precision -= 2;
	else if ((diff >= 100.f) && (diff < 1000.f))
	  precision -= 3;
	else
	  {
	    field_width = 8;
	    precision = 0;
	  }

    if (diff != 0.)
    {
        float a3 = vmax/diff;
        if (a3 >= 10. && a3 <= 1.e6)
        {
            int i = (int) (log(a3)/log(10.)) - 1;
            if ((x1 < 0.) || (x2 < 0.)) i++;
            if (i < 10) field_width += i;;
	}
    }
    
    if (field_width > 8)
      sprintf (format, "%c%2d.%1de", '%', field_width, precision);
    else
      sprintf (format, "%c%1d.%1df", '%', field_width, precision);

    return format;
}
//--------------------------------------------------------------

void LoadInitializationFile ()
{
// Load the Forth file xyplot.4th, residing in the
// directory specified by the environment variable
// XYPLOT_DIR. If XYPLOT_DIR is not defined, the
// default directory is HOME/.xyplot. This file can
// load other forth files containing initialization scripts.

  char start_dir [512];
  char path[512];

  getcwd (start_dir, 511);

  if (getenv("XYPLOT_DIR"))
    {
      strcpy(path, getenv("XYPLOT_DIR"));
    }
  else
    {
      char* home_dir = getenv("HOME");
      strcpy (path, getenv("HOME"));
      strcat (path, "/.xyplot");
    }
  if (chdir(path) == 0)
    {
      LoadForthFile("xyplot.4th");
    }
  chdir(start_dir);

}    
//--------------------------------------------------------------

vector<char*> GetStartupFileList (int argc, char* argv[])
{
// Parse the command line into a vector of filenames.
//  The first element in the vector is the program pathname.

    vector<char*> filelist;

    int i = 1;

    while (i < argc)
      {
	if (*argv[i] != '-') filelist.push_back(argv[i]);
	i++;
      }

    return filelist;
}
//--------------------------------------------------------------

int AddToHeader (char* text, char* hdr, bool prefix)
{
  // Add text to the beginning of a header string
  // Return non-zero if length exceeded.

  int ecode = 0;
  int new_length = strlen(text) + strlen(hdr);
  if (prefix) new_length += strlen(ANNOTATION_PREFIX);

  if (new_length < HEADER_LENGTH)
    {
      char* temp_str = new char[HEADER_LENGTH];
      if (prefix) 
	{
	  strcpy (temp_str, ANNOTATION_PREFIX);
	  strcat (temp_str, text);
	}
      else
	{
	  strcpy (temp_str, text);
	}
      strcat (temp_str, hdr);
      strcpy (hdr, temp_str);
      delete [] temp_str;
    }
  else
    {
      ecode = -1;  // new header will exceed the HEADER_LENGTH
    }

  return ecode;
}
//-------------------------------------------------------------

int ExecuteForthExpression (char* s, ostringstream* pOutput, long int* pLine)
{
  // Return zero if no error; otherwise return the Forth error code.

  istringstream* pSS = new istringstream (s);
  long int *sp, nError;
  byte* tp;
  vector<byte> op;

  *pLine = 0;

  SetForthInputStream(*pSS);
  SetForthOutputStream(*pOutput);
  nError = ForthCompiler (&op, pLine);
  delete pSS;

  if (nError) return nError;

  if (op.size())
    {
      SetForthInputStream(cin);
      nError = ForthVM (&op, &sp, &tp);
    }

  return nError;
}
//-------------------------------------------------------------

void InitForthInterface ()
{
  char fs[256];
  const void* funcPtr;
  const char* constName;
  int i, nError;
  long int lnum;
  size_t nIFCfuncs = sizeof(IfcFuncList) / sizeof(IfcFuncList[0]);
  char out_s[256];
  stringstream ForthMessages;

  for (i = 0; i < nIFCfuncs; i++) {
	funcPtr = IfcFuncList[i].Function;
	constName = IfcFuncList[i].constantName;
  	sprintf (fs, "%lu constant %s\n", funcPtr, constName);
  	nError = ExecuteForthExpression(fs, (ostringstream*) &ForthMessages, &lnum);
  	if (nError) {
		ForthMessages.getline(out_s, 255, 0);
		pMainWnd->MessageBox(out_s);
		return;
	}
  }

  IfcFuncTemplate MenuList[5] = {
     { pMainWnd->m_nFileMenu, "MN_FILE" },
     { pMainWnd->m_nEditMenu, "MN_EDIT" },
     { pMainWnd->m_nPlotMenu, "MN_PLOT" },
     { pMainWnd->m_nMathMenu, "MN_MATH" },
     { pMainWnd->m_nHelpMenu, "MN_HELP" }
  };

  for (i = 0; i < 5; i++) {
	funcPtr = MenuList[i].Function;
	constName = MenuList[i].constantName;
  	sprintf (fs, "%lu constant %s\n", funcPtr, constName);
  	nError = ExecuteForthExpression(fs, (ostringstream*) &ForthMessages, &lnum);
  	if (nError) {
		ForthMessages.getline(out_s, 255, 0);
		pMainWnd->MessageBox(out_s);
		return;
	}
  }

}
//-----------------------------------------------------------------

int LoadForthFile(char* fname)
{
  char s[256], out_s[1024];
  int nError;
  long int lnum;

  strcpy (s, "include ");
  strcat (s, fname);

  stringstream ForthMessages;
  nError = ExecuteForthExpression (s, (ostringstream*) &ForthMessages, &lnum);

  if (nError)
    {
        ForthMessages.getline(out_s, 1023, 0);
        pMainWnd->MessageBox (out_s);
    }

  return nError;
}

//----------------------------------------------------------------
// WARNING:
//
//  DO NOT CALL the following functions from C, C++, or
//  assembly code. They are intended to be called from the
//  Forth environment only.
//
//----------------------------------------------------------------

int get_color_map ()
{
   // Fill in the xyplot color rgb values and names in the
   // data arrays
   // stack: ( acolorref acolornames max-name-len max-colors -- ncolors )
   ++GlobalSp; ++GlobalTp;
   int maxcolors = *GlobalSp;
   if (maxcolors <= 0) return 0;
   if (maxcolors > MAX_COLORS) maxcolors = MAX_COLORS;
   ++GlobalSp; ++GlobalTp;
   int max_name_len = *GlobalSp;
   ++GlobalSp; ++GlobalTp;
   if (*GlobalTp != OP_ADDR) return( -1 ); // invalid array
   char* pcolornames = (char*)(*GlobalSp);
   ++GlobalSp; ++GlobalTp;
   if (*GlobalTp != OP_ADDR) return( -1 ); // invalid array
   COLORREF* colorrefs = (COLORREF*)(*GlobalSp);
   
   *GlobalTp-- = OP_IVAL;
   int i, n;
   char *p = pcolornames;
   for (i = 0; i < maxcolors; i++) {
     *colorrefs = colors_rgb_table[i];
     ++colorrefs;
     char* name = color_names[i];
     n = strlen(name);
     if (n >= max_name_len) n = max_name_len-1;
     strncpy(p, name, n);
     *(p + n) = (char) 0;
     p += max_name_len;
   }
   *GlobalSp-- = maxcolors;
   return 0;
}
// --------------------------------------------------------------

int get_active_set ()
{
  // Return the active set index number on top of the Forth stack
  // Return a -1 if error.

  CDataset* d = pMainWnd->m_pDi->GetActiveSet();
  CDatabase* pDb = pMainWnd->m_pDb;

  int nSet = pDb->IndexOf(d);

  // Place the set number on top of the stack

  *GlobalSp = nSet;
  --GlobalSp;
  *GlobalTp = OP_IVAL;
  --GlobalTp;

  return 0;
}
//-----------------------------------------------------------------

int get_operand_set ()
{
  // Return the operand set index on top of the Forth stack; -1 if error

  CDataset* d = pMainWnd->m_pDi->GetOperandSet();
  CDatabase* pDb = pMainWnd->m_pDb;
  int nSet = pDb->IndexOf(d);
  *GlobalSp-- = nSet; *GlobalTp-- = OP_IVAL;
  return 0;
}
//-----------------------------------------------------------------

int get_active_plot ()
{
  // Return the active plot index on top of the stack; -1 if error

  CPlotDisplay* pDi = pMainWnd->m_pDi;
  CPlot* p = pDi->GetActivePlot();
  int nPlot = pDi->IndexOf(p);
  *GlobalSp-- = nPlot; *GlobalTp-- = OP_IVAL;
  return 0;
}
//-----------------------------------------------------------------

int get_operand_plot ()
{
  // Return the operand plot index on top of the stack; -1 if error

  CPlotDisplay* pDi = pMainWnd->m_pDi;
  CPlot* p = pDi->GetOperandPlot();
  int nPlot = pDi->IndexOf(p);
  *GlobalSp-- = nPlot; *GlobalTp-- = OP_IVAL;
  return 0;
}
//-----------------------------------------------------------------

int set_active_plot ()
{
  // Set the active plot to the index on top of the stack

  ++GlobalSp; ++GlobalTp;
  int nPlot = *GlobalSp;
  pMainWnd->SelectPlot(nPlot, 1);

  return 0;
}
//-----------------------------------------------------------------

int set_operand_plot ()
{
  // Set the operand plot to the index on top of the stack

  ++GlobalSp; ++GlobalTp;
  int nPlot = *GlobalSp;
  pMainWnd->SelectPlot(nPlot, 2);

  return 0;
}
//-----------------------------------------------------------------

int get_ds ()
{
  // Return information about the specified dataset

  ++GlobalSp; ++GlobalTp;
  if (*GlobalTp == OP_ADDR)
    {
      // obtain the pointer to the dataset info structure
      unsigned int* ds_info = *((unsigned int**)GlobalSp);

      ++GlobalSp;
      int nSet = *GlobalSp;
      CDatabase* pDb = pMainWnd->m_pDb;
      int n = pDb->Nsets();

      if (nSet >= 0 && nSet < n)
	{
	  // Fill in the dataset info structure
	  CDataset* d = (*pDb)[nSet];
	  *ds_info++ = (unsigned int) d->m_szName;
	  *ds_info++ = (unsigned int) d->m_szHeader;
	  *ds_info++ = 0;  // set zero for the dataset type
	  *ds_info++ = d->NumberOfPoints();
	  *ds_info++ = d->SizeOfDatum();
	  *ds_info = (unsigned int) &(*d)[0]; // ->begin();
	  *GlobalSp = 0;
	}
      else
	{
	  *GlobalSp = -1;
	}
      --GlobalSp;
    }
  else
    {
      pMainWnd->MessageBox("Invalid parameter for get_ds");
    }
  return 0;
}
//-----------------------------------------------------------------

int set_ds_extrema ()
{
  // Reset extrema for specified dataset

  ++GlobalSp; ++GlobalTp;
  int i = *GlobalSp;
  if (i >= 0 && i < pMainWnd->m_pDb->Nsets())
    {
      CDataset* d = (*(pMainWnd->m_pDb))[i];
      if (d) d->SetExtrema();
    }
  return 0;
}
//-----------------------------------------------------------------

int make_ds ()
{
  // Make a dataset from the dataset info structure passed
  // on top of the Forth stack.

  char s[256];

  ++GlobalSp; ++GlobalTp;
  if (*GlobalTp == OP_ADDR)
    {
      int* ds_info = *((int**) GlobalSp);
      char* name = *((char**) ds_info);
      CDataset* d = pMainWnd->m_pDb->MakeDataset(ds_info);
      if (d)
	{
	  sprintf (s, "Created new dataset %s", name);
	  pMainWnd->WriteConsoleMessage(s);
          int n = pMainWnd->m_pDb->Nsets();
          *GlobalSp-- = n-1; 
	}
      else
	{
	  sprintf (s, "Cannot create dataset %s", name);
	  pMainWnd->MessageBox(s);
	  *GlobalSp-- = -1;
	}
    }
  else
    {
      pMainWnd->MessageBox("Invalid parameter for make_ds");
      *GlobalSp-- = -1;
    }
  *GlobalTp-- = OP_IVAL;
  return 0;
}
//-----------------------------------------------------------------

int get_plot ()
{
  // Return information about the specified plot

  ++GlobalSp; ++GlobalTp;
  if (*GlobalTp == OP_ADDR)
    {
      int* pl_info = *((int**)GlobalSp);
      ++GlobalSp;
      int nPlot = *GlobalSp;
      CPlotDisplay* pDi = pMainWnd->m_pDi;
      CDatabase* pDb = pMainWnd->m_pDb;
      int n = pDi->Nplots();

      if (nPlot >= 0 && nPlot < n)
	{
	  // Fill in the plot info structure
	  CPlot* p = (*pDi)[nPlot];
	  if (p)
	    {
	      CDataset* d = p->GetSet();
	      int nSet = pDb->IndexOf(d);
	      *pl_info++ = nSet;  // dataset index number
	      *pl_info++ = 0;     // set zero for the plot type
	      *pl_info++ = p->GetSymbol();  // plot symbol
	      *pl_info = p->GetColor();     // plot color (index in color map)
	      *GlobalSp = 0;
	    }
	  else
	    *GlobalSp = -1;
	}
      else
	{
	  *GlobalSp = -1;
	}
      --GlobalSp;
    }
  else
    {
      pMainWnd->MessageBox("Invalid parameter for get_plot");
    }
  return 0;
} 
//-----------------------------------------------------------------

int drop_plot ()
{
   // Drop the active plot
   pMainWnd->OnDrop();
   return 0;
}
//-----------------------------------------------------------------

int make_plot ()
{
  // Make a new plot

  ++GlobalSp; ++GlobalTp;
  if (*GlobalTp == OP_ADDR)
    {
      CDatabase* pDb = pMainWnd->m_pDb;
      int* pl_info = *((int**) GlobalSp);
      int nSet = *pl_info++;
      if ((nSet >= 0) && (nSet < pDb->Nsets()))
	  {
	    int nType = *pl_info++;
	    CDataset* d = (*pDb)[nSet];
	    if (d)
	      {
		CPlot* p = pMainWnd->m_pDi->MakePlot(d, nType);
		if (p)
		  {
		    Symbol nSym = (Symbol) *pl_info++;
		    p->SetSymbol(nSym);
		    unsigned c = *pl_info;  // index into color map
		    p->SetColor(c);
		  }
	      }
	  }
    }
  else
    {
      pMainWnd->MessageBox("Invalid parameter for make_plot");
    }
  return 0;
}
//------------------------------------------------------------------

int set_plot_symbol ()
{
  ++GlobalSp; ++GlobalTp;
  if (*GlobalTp == OP_ADDR)
    {
      char* symbol_name = *((char**)GlobalSp);
      ++symbol_name;
      Symbol nSym = pMainWnd->GetSymbol(symbol_name);
      pMainWnd->m_pDi->GetActivePlot()->SetSymbol(nSym);
      pMainWnd->Invalidate();
    }
  else
    {
      pMainWnd->MessageBox("Invalid parameter for set_plot_symbol");
    }
  return 0;
}
//------------------------------------------------------------------

int set_plot_color ()
{
  ++GlobalSp; ++GlobalTp;
  if (*GlobalTp == OP_ADDR)
    {
      char* color_name = *((char**)GlobalSp);
      ++color_name;
      unsigned int c = pMainWnd->GetColor(color_name);
      pMainWnd->m_pDi->GetActivePlot()->SetColor(c);
      pMainWnd->Invalidate();
    }
  else
    {
      pMainWnd->MessageBox("Invalid parameter for set_plot_color");
    }
  return 0;
}
//------------------------------------------------------------------

int set_plot_rgbcolor ()
{
  // stack: ( COLORREF -- )
  ++GlobalSp; ++GlobalTp;
  COLORREF cr = *((unsigned long *) GlobalSp);
  unsigned c = pMainWnd->GetColor(cr);
  pMainWnd->m_pDi->GetActivePlot()->SetColor(c);
  pMainWnd->Invalidate();
  return 0;
}
//------------------------------------------------------------------

int draw_plot ()
{
  ++GlobalSp; ++GlobalTp;
  int vis = *GlobalSp;
  CPlot* p = pMainWnd->m_pDi->GetActivePlot();
  unsigned c = p->GetColor();
  if (! vis)
    {
      unsigned bkg = pMainWnd->GetBackgroundColor();
      p->SetColor(bkg);
    }
  CDeviceContext* pDc = pMainWnd->GetDC();
  p->Draw(pDc);
  XFlush(XtDisplay(pMainWnd->m_nPlotWindow));
  XSync(XtDisplay(pMainWnd->m_nPlotWindow), False);
  p->SetColor(c);
  return 0;
}
//------------------------------------------------------------------

int get_grid ()
{
   // stack: ( -- nXtics nYtics bXGridLines bYGridLines bXaxis bYaxis )
   int nx, ny;
   bool bXlines, bYlines, bXaxis, bYaxis;
   CPlotView* pView = pMainWnd->m_pDi->GetCurrentView();
   pView->m_pGrid->GetTics(&nx, &ny);
   *GlobalSp-- = nx; *GlobalTp-- = OP_IVAL;
   *GlobalSp-- = ny; *GlobalTp-- = OP_IVAL;
   pView->m_pGrid->GetLines(&bXlines, &bYlines);
   *GlobalSp-- = bXlines; *GlobalTp-- = OP_IVAL;
   *GlobalSp-- = bYlines; *GlobalTp-- = OP_IVAL;
   pView->m_pGrid->GetAxes(&bXaxis, &bYaxis);
   *GlobalSp-- = (int) bXaxis; *GlobalTp-- = OP_IVAL;
   *GlobalSp-- = (int) bYaxis; *GlobalTp-- = OP_IVAL;
   return 0;
}

int set_grid_tics ()
{
  ++GlobalSp; ++GlobalTp;
  int ny = *GlobalSp++; ++GlobalTp;
  int nx = *GlobalSp;
  CPlotView* pView = pMainWnd->m_pDi->GetCurrentView();
  pView->m_pGrid->SetTics(nx, ny);
  pMainWnd->Invalidate();
  return 0;
}

int set_grid_lines ()
{
  ++GlobalSp; ++GlobalTp;
  bool by = *GlobalSp++; ++GlobalTp;
  bool bx = *GlobalSp;
  CPlotView* pView = pMainWnd->m_pDi->GetCurrentView();
  pView->m_pGrid->SetLines(bx, by);
  pMainWnd->Invalidate();
  return 0;
}

//------------------------------------------------------------------

int get_window_title ()
{
  // stack: ( abuf nmax -- abuf nret )
  char* buf;
  ++GlobalSp; ++GlobalTp;
  int nmax = *((unsigned long *)GlobalSp);
  ++GlobalSp; ++GlobalTp;
  if (*GlobalTp == OP_ADDR) {
    char* buf = *((char**)GlobalSp);
    int nret = pMainWnd->GetWindowText(buf, nmax);
    nret = min(nmax-1, nret);
    buf[nret] = (char) 0;
    --GlobalSp; --GlobalTp;
    *GlobalSp-- = nret; --GlobalTp;
  }
  else {
    --GlobalSp; --GlobalTp;
    *GlobalSp-- = 0; --GlobalTp;
    pMainWnd->MessageBox("get_window_title buffer address error!");
  }
  return 0;
}

int set_window_title ()
{
  // stack: ( c-addr u -- )
  ++GlobalSp; ++GlobalTp;
  int ulen = *GlobalSp;
  ++GlobalSp; ++GlobalTp;
  if (*GlobalTp == OP_ADDR) {
    char* buf = *((char**)GlobalSp);
    buf[ulen] = (char) 0;
    pMainWnd->SetWindowText(buf);
  }
  else {
    pMainWnd->MessageBox("set_window_title buffer address error!");
  }
  return 0;
}

int clear_window ()
{
  // Clear the plot window

  pMainWnd->Clear();
  return 0;
}

int reset_window ()
{
  // Reset the plot window

  pMainWnd->OnReset();
  return 0;
}

int draw_window ()
{
  // Force drawing of the plot window

  pMainWnd->Invalidate();
  pMainWnd->OnPaint();

  return 0;
}
//-----------------------------------------------------------------

int get_window_limits ()
{

  // Return x1, y1, x2, y2 on top of the stack

  vector<float> e = pMainWnd->m_pDi->GetExtrema();

  --GlobalSp; --GlobalTp;
  *((double*)GlobalSp) = (double) e[0];
  *GlobalTp = OP_FVAL;
  GlobalSp -= 2; GlobalTp -= 2;
  *((double*)GlobalSp) = (double) e[2];
  *GlobalTp = OP_FVAL;
  GlobalSp -= 2; GlobalTp -= 2;
  *((double*)GlobalSp) = (double) e[1];
  *GlobalTp = OP_FVAL;
  GlobalSp -= 2; GlobalTp -= 2;
  *((double*)GlobalSp) = (double) e[3];
  *GlobalTp = OP_FVAL;
  --GlobalSp; --GlobalTp;

  return 0;
}
//-----------------------------------------------------------------

int set_window_limits ()
{
  // Set the plot window limits

  vector<float> e(4);
  ++GlobalSp;
  e[3] = *((double*)GlobalSp);
  GlobalSp += 2;
  e[1] = *((double*)GlobalSp);
  GlobalSp += 2;
  e[2] = *((double*)GlobalSp);
  GlobalSp += 2;
  e[0] = *((double*)GlobalSp);
  ++GlobalSp;
  GlobalTp += 8;

  pMainWnd->m_pDi->CreateView(CARTESIAN, e);
  pMainWnd->Invalidate();

  return 0;
}
//---------------------------------------------------------------

int add_menu_item ()
{
  // Add item to the specified menu.
  // Stack: ( menu  ^item_name  ^forth_command -- )
  //   menu is the menu widget id (e.g. MN_MATH)
  //   item_name is the address of a Forth counted string
  //   forth_command is the address of a Forth counted string

  char name[256];
  char* emsg1 = "add_menu_item: Invalid parameter";
  unsigned char *cp;
  Widget menu, new_item;

  ++GlobalSp; ++GlobalTp;
  if (*GlobalTp != OP_ADDR)
    {
      pMainWnd->MessageBox(emsg1);
      return 0;
    }
  cp = *((unsigned char**) GlobalSp);
  int nCount = (int) *cp;

  char* fc = new char[256];  // allocate a new command string
                              
  strncpy (fc, (const char*) cp+1, nCount);
  fc[nCount] = '\0';
  ++GlobalSp; ++GlobalTp;
  if (*GlobalTp != OP_ADDR)
    {
      pMainWnd->MessageBox(emsg1);
      return 0;
    }
  cp = *((unsigned char**) GlobalSp);
  nCount = (int) *cp;
  strncpy (name, (const char*) cp+1, nCount);
  name[nCount] = '\0';
  ++GlobalSp; ++GlobalTp;
  menu = (Widget) *GlobalSp;
  
  new_item = pMainWnd->AddMenuItem (menu, name);
  if (new_item)
  {
      ForthMenuCommandList.push_back(fc);
      XtAddCallback (new_item, XmNactivateCallback, ForthCB, fc);
  }

  return 0;
}
//---------------------------------------------------------------

int make_menu ()
{
    // Create a new menu.
    // Stack: ( ^menu_name -- menu_id )

    char name[256];
    ++GlobalSp; ++GlobalTp;
    if (*GlobalTp != OP_ADDR) { pMainWnd->MessageBox("make_menu: Invalid parameter"); return 0;}
    unsigned char *cp = *((unsigned char**) GlobalSp);
    int nCount = (int) *cp;

    Widget menu_id = pMainWnd->MakeMenu (pMainWnd->m_nMenuBar, (char*) cp+1, NULL, NULL, 0);
    *GlobalTp-- = OP_IVAL; *GlobalSp-- = (int) menu_id;
 
    return 0;
}
//---------------------------------------------------------------

int make_submenu ()
{
    // Create a new menu.
    // Stack: ( menu_id ^menu_name -- submenu_id )

    char name[256];
    ++GlobalSp; ++GlobalTp;
    if (*GlobalTp != OP_ADDR) { pMainWnd->MessageBox("make_submenu: Invalid parameter"); return 0;}
    unsigned char *cp = *((unsigned char**) GlobalSp);
    int nCount = (int) *cp;
    ++GlobalSp; ++GlobalTp;
    Widget menu_id = (Widget) *GlobalSp;
    // XmString title = XmStringCreateLocalized ((char*)cp+1);
    Widget PullDown = pMainWnd->AddSubMenu(menu_id, (char*) cp+1);
    // XmCreatePulldownMenu (menu_id, "  ", NULL, 0);
    // Widget cascade = XtVaCreateManagedWidget((char*)cp+1, 
    //						xmCascadeButtonWidgetClass, menu_id, 
    //						XmNsubMenuId, PullDown,
    //						XmNlabelString, title,
    //						XmNmnemonic, 0,
    //						NULL);
    // XmStringFree(title);

    *GlobalTp-- = OP_IVAL; *GlobalSp-- = (int) PullDown;
 
    return 0;
}
//---------------------------------------------------------------

int set_background ()
{
  // Stack: ( addr -- | counted string contains color name )

  char color_name[256];

  ++GlobalSp; ++GlobalTp;
  if (*GlobalTp == OP_ADDR)
    {
      char* name = *((char**) GlobalSp);
      unsigned nLen = (unsigned) *name;
      ++name;
      strncpy (color_name, name, nLen);
      color_name[nLen] = '\0';
      pMainWnd->SetBackgroundColor(color_name);
    }
  else
    {
      pMainWnd->MessageBox("Invalid parameter for set_background");
    }

  return 0;
}
//----------------------------------------------------------------

int set_foreground ()
{
  // Stack: ( addr -- | counted string contains color name )

  char color_name[256];

  ++GlobalSp; ++GlobalTp;
  if (*GlobalTp == OP_ADDR)
    {
      char* name = *((char**) GlobalSp);
      unsigned nLen = (unsigned) *name;
      ++name;
      strncpy (color_name, name, nLen);
      color_name[nLen] = '\0';
      pMainWnd->SetForegroundColor(color_name);
    }
  else
    {
      pMainWnd->MessageBox("Invalid parameter for set_foreground");
    }

  return 0;
}
//----------------------------------------------------------------

int radio_box ()
{
    // stack: ^label1 ^label2 ... n -- m 


    // ++GlobalSp; ++GlobalTp;
    // int nRadioButtons = *GlobalSp++; ++GlobalTp;
    // if (nRadioButtons <= 0) return 0;

 
    //   radio_box = XmCreateRadioBox (TopLevel, "radio_box", NULL, 0);
    // cout << " nRadioButtons = " << nRadioButtons << '\n';
    // int i;
    // char *label, s[64];
    // for (i = 0; i < nRadioButtons; i++)
    // {
//	label = *((char**) GlobalSp++) + 1; ++GlobalTp;
//	cout << label << '\n';
//	strcpy (s, label);
	// radio_buttons[i] = XtVaCreateManagedWidget(s, xmToggleButtonGadgetClass, radio_box, NULL);
	// XtAddCallback (radio_buttons[i], XmNvalueChangedCallback, RadioToggledCB, (void*) (i+1));
//    }

    XtManageChild(pMainWnd->m_nRadioBox);
    InputData = FALSE;
    XtPopup(XtParent(pMainWnd->m_nRadioBox), XtGrabNone);

    while ((!InputData) || XtPending()) 
      XtProcessEvent(XtIMAll);
    
    int sel = 0;
    *GlobalSp-- = sel; *GlobalTp-- = OP_IVAL;
    return 0;
}
// -----------------------------------------------------------------

int message_box ()
{
    // stack: a u --
    ++GlobalSp; ++GlobalTp;
    int nc = *GlobalSp;
    ++GlobalSp; ++GlobalTp;
    if (*GlobalTp == OP_ADDR)
    {
	char message[4096];
	if (nc > 4095) nc = 4095;
	strncpy(message, *((char**) GlobalSp), nc);
	message[nc] = 0;
	pMainWnd->MessageBox(message);
    }
    return 0;
}
//----------------------------------------------------------------

int get_input ()
{
  ++GlobalSp; ++GlobalTp;
  if (*GlobalTp == OP_ADDR)
    {
      char* prompt = *((char**) GlobalSp);
      ++prompt;
      Arg al[4];
      int ac = 0;
      XtSetArg(al[ac], XmNselectionLabelString, 
	       XmStringCreateLtoR(prompt, 
				  XmSTRING_DEFAULT_CHARSET));  
      ac++;
      XtSetArg(al[ac], XmNtextString, XmStringCreateLtoR("",
							 XmSTRING_DEFAULT_CHARSET));
      ac++;
      XtSetValues(pMainWnd->m_nInputDialog, al, ac);

      XtAddCallback (pMainWnd->m_nInputDialog, XmNokCallback,
      	     InputCB, (void*) TRUE);
      XtAddCallback (pMainWnd->m_nInputDialog, XmNcancelCallback,
      	     InputCB, (void*) FALSE);
      XtManageChild (pMainWnd->m_nInputDialog);
      XtPopup(XtParent(pMainWnd->m_nInputDialog), XtGrabNone);

      InputData = FALSE;

      while ((!InputData) || XtPending()) 
	XtProcessEvent(XtIMAll);
    }
  else
    {
      pMainWnd->MessageBox("Invalid parameter for get_input");
    }

  return 0;
}
//----------------------------------------------------------------

int file_open_dialog ()
{
    ++GlobalSp; ++GlobalTp;
    if (*GlobalTp == OP_ADDR)
    {
      // Get the current filter string and save it
      XtVaGetValues(pMainWnd->m_nFileOpenDialog, 
		    XmNpattern, &old_filter,
		    NULL);
      char* filter = *((char**) GlobalSp);
      ++filter;
      Arg al[4];
      int ac = 0;
      XtSetArg(al[ac], XmNpattern, 
	       XmStringCreateLtoR(filter, XmSTRING_DEFAULT_CHARSET));  
      ac++;

      XtSetValues(pMainWnd->m_nFileOpenDialog, al, ac);

      XtRemoveCallback (pMainWnd->m_nFileOpenDialog, XmNokCallback,
          FileMenuCB, (void*) PL_FILE_OPEN);
      XtRemoveCallback (pMainWnd->m_nFileOpenDialog, XmNcancelCallback,
          FileMenuCB, (void*) PL_FILE_CANCEL);

      XtAddCallback (pMainWnd->m_nFileOpenDialog, XmNokCallback,
      	     FileOpenCB, (void*) TRUE);
      XtAddCallback (pMainWnd->m_nFileOpenDialog, XmNcancelCallback,
      	     FileOpenCB, (void*) FALSE);
      XtManageChild (pMainWnd->m_nFileOpenDialog);
      XtPopup(XtParent(pMainWnd->m_nFileOpenDialog), XtGrabNone);

      InputData = FALSE;

      while ((!InputData) || XtPending()) 
	XtProcessEvent(XtIMAll);
    }
  else
    {
      pMainWnd->MessageBox("Invalid filter string for file_open_dialog");
    }
    return 0;
}
//------------------------------------------------------------------

int file_save_dialog ()
{
    // This is a stub to be filled in later.
    // stack: ( ^filter -- ^filename flag )
    *GlobalSp-- = 0; *GlobalTp-- = OP_IVAL;
    return 0;
}
//----------------------------------------------------------------

int set_save_options ()
{
    ++GlobalSp; ++GlobalTp;
    SaveOptions *p = (SaveOptions*)(*GlobalSp);
    pMainWnd->m_pDb->SetSaveOptions(*p);
    return 0;
}
//------------------------------------------------------------------

void RadioToggledCB (Widget w, void* client_d, void* call_d)
{
  int which = (int) client_d;
  XtUnmanageChild (pMainWnd->m_nRadioBox);
  InputData = TRUE;
}
//-------------------------------------------------------------------

//
// The following callback is for use by get_input only

void InputCB (Widget w, void* client_d, void* call_d)
{
  caddr_t client_data = (caddr_t) client_d;
  XmAnyCallbackStruct* call_data = (XmAnyCallbackStruct*) call_d;

  static char rs[64];  // counted response string

  int i = (int) client_data, n;
  switch (i)
    {
    case TRUE:
      // Ok button has been pressed

      XmSelectionBoxCallbackStruct *selection;
      char* s;
      selection=(XmSelectionBoxCallbackStruct *) call_data;
      XmStringGetLtoR(selection->value, XmSTRING_DEFAULT_CHARSET, &s);
      n = strlen(s);
      strcpy (rs + 1, s);
      *rs = n;
      break;

    case FALSE:
      // Cancel button has been pressed
      *rs = 0;
      break;
    default:
      ;
    }
  *GlobalSp-- = (int) rs;
  *GlobalTp-- = OP_ADDR;
  *GlobalSp-- = i;
  *GlobalTp-- = OP_IVAL;

  XtRemoveCallback (pMainWnd->m_nInputDialog, XmNokCallback,
        InputCB, (void*) TRUE);
  XtRemoveCallback (pMainWnd->m_nInputDialog, XmNcancelCallback,
	InputCB, (void*) FALSE);
  XtUnmanageChild (pMainWnd->m_nInputDialog);
  InputData = TRUE;
}
//---------------------------------------------------------------
//
// The following callback is for use by file_open_dialog only

void FileOpenCB (Widget w, void* client_d, void* call_d)
{

    caddr_t client_data = (caddr_t) client_d;
    XmAnyCallbackStruct* call_data = (XmAnyCallbackStruct*) call_d;
    static char cfilename [256];
    
    int i = (int) client_data, n;
    switch (i)
    {
	case TRUE:
	    XmFileSelectionBoxCallbackStruct* sel;
	    sel = (XmFileSelectionBoxCallbackStruct *) call_data;
	    char* s;
	    XmStringGetLtoR(sel->value, XmSTRING_DEFAULT_CHARSET, &s);
	    n = strlen(s);
	    strcpy (cfilename + 1, s);
	    *cfilename = n;
	    break;
	case FALSE:
	    *cfilename = 0;
	    break;
	default:
	    ;
    }
    *GlobalSp-- = (int) cfilename; *GlobalTp-- = OP_ADDR;
    *GlobalSp-- = i;  *GlobalTp-- = OP_IVAL;
    XtRemoveCallback (pMainWnd->m_nFileOpenDialog, XmNokCallback,
        FileOpenCB, (void*) TRUE);
    XtRemoveCallback (pMainWnd->m_nFileOpenDialog, XmNcancelCallback,
	FileOpenCB, (void*) FALSE);
 
    // Restore original file filter
    Arg al[1];
    XtSetArg(al[0], XmNpattern, old_filter);  
    XtSetValues(pMainWnd->m_nFileOpenDialog, al, 1);
 
    XtAddCallback (pMainWnd->m_nFileOpenDialog, XmNokCallback,
        FileMenuCB, (void*) PL_FILE_OPEN);
    XtAddCallback (pMainWnd->m_nFileOpenDialog, XmNcancelCallback,
        FileMenuCB, (void*) PL_FILE_CANCEL);

    XtUnmanageChild (pMainWnd->m_nFileOpenDialog);
    InputData = TRUE;
}

