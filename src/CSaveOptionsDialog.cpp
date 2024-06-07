// CSaveOptionsDialog.cpp
//
// Copyright 1998--2024 Krishna Myneni
//
// Provided under the terms of the GNU Affero General Public License
// (AGPL) v 3.0 or later.

#include "saveopts.h"
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
#include "CSaveOptionsDialog.h"
#include "CPlotMessage.h"
extern void SaveOptionsCB (Widget, void*, void*);
//-------------------------------------------

CSaveOptionsDialog::CSaveOptionsDialog(Widget topLevel)
{
  // Create the Save Options dialog
  char s[64];

  Widget soDialog, rowCol, btn, hdrStyleLabel;

  m_nW = XmCreateFormDialog(topLevel,
    strcpy(s, "Save Options"), NULL, 0);

  rowCol = XtVaCreateManagedWidget("rowcolumn",
         xmRowColumnWidgetClass, m_nW,
         NULL);

  XmString one, two, three;
  hdrStyleLabel = XtVaCreateManagedWidget("Header Style",
        xmLabelWidgetClass, rowCol,
        // XmNtopWidget, rowCol,
        // XmNleftWidget, rowCol,
        NULL);
  one = XmStringCreateLocalized(strcpy(s,"none"));
  two = XmStringCreateLocalized(strcpy(s,"xyplot"));
  three = XmStringCreateLocalized(strcpy(s,"line prefix"));

  m_nSaveHeader = XmVaCreateSimpleRadioBox( rowCol,
        strcpy(s, "Header Style"),
        0,
        SaveOptionsCB,
        XmVaRADIOBUTTON, one, NULL, NULL, NULL,
        XmVaRADIOBUTTON, two, NULL, NULL, NULL,
        XmVaRADIOBUTTON, three, NULL, NULL, NULL,
        NULL);

  // XtVaSetValues(m_nSaveHeader,
  //      XmNtopWidget, hdrStyleLabel,
  //      XmNleftWidget, rowCol,
  //      NULL);

  XmStringFree(one);
  XmStringFree(two);
  XmStringFree(three);

  Widget prefixLabel;
  prefixLabel = XtVaCreateManagedWidget ("Prefix",
        xmLabelWidgetClass, rowCol,
        NULL);

  m_nSavePrefix = XmCreateTextField(rowCol, strcpy(s,"text"), NULL, 0);
  XtVaSetValues(m_nSavePrefix,
        XmNeditable, False,
        // XmNtopWidget, m_nSaveHeader,
        // XmNleftWidget, rowCol,
        NULL);

  XtVaCreateManagedWidget ("sep", xmSeparatorGadgetClass, rowCol, NULL);

  Widget numberFormatLabel;
  numberFormatLabel = XtVaCreateManagedWidget ("Number Format",
        xmLabelWidgetClass, rowCol,
        NULL);
  one = XmStringCreateLocalized(strcpy(s,"exponential"));
  two = XmStringCreateLocalized(strcpy(s, "floating point"));
  three = XmStringCreateLocalized(strcpy(s, "integer"));

  m_nSaveNumberFormat = XmVaCreateSimpleRadioBox (rowCol,
        strcpy(s, "Number Format"),
        0,
        SaveOptionsCB,
        XmVaRADIOBUTTON, one, NULL, NULL, NULL,
        XmVaRADIOBUTTON, two, NULL, NULL, NULL,
        XmVaRADIOBUTTON, three, NULL, NULL, NULL,
        NULL);
  XmStringFree(one);
  XmStringFree(two);
  XmStringFree(three);

  XtVaCreateManagedWidget ("sep", xmSeparatorGadgetClass, rowCol, NULL);
  XtVaCreateManagedWidget ("Column Delimiter", xmLabelWidgetClass, rowCol, NULL);
  one = XmStringCreateLocalized(strcpy(s,"space"));
  two = XmStringCreateLocalized(strcpy(s,"tab"));
  three = XmStringCreateLocalized(strcpy(s,"comma"));

  m_nSaveColumnDelimiter = XmVaCreateSimpleRadioBox( rowCol,
        strcpy(s, "Column Delimiter"),
        0,
        SaveOptionsCB,
        XmVaRADIOBUTTON, one, NULL, NULL, NULL,
        XmVaRADIOBUTTON, two, NULL, NULL, NULL,
        XmVaRADIOBUTTON, three, NULL, NULL, NULL,
        NULL);

  XtVaCreateManagedWidget ("sep", xmSeparatorGadgetClass, rowCol, NULL);

  XtVaCreateManagedWidget ("End of Line",
        xmLabelWidgetClass, rowCol,
        NULL);
  one = XmStringCreateLocalized(strcpy(s, "Unix"));
  two = XmStringCreateLocalized(strcpy(s, "DOS"));
  m_nSaveEndOfLine = XmVaCreateSimpleRadioBox (rowCol,
        strcpy(s,"EOL"),
        0,
        SaveOptionsCB,
        XmVaRADIOBUTTON, one, NULL, NULL, NULL,
        XmVaRADIOBUTTON, two, NULL, NULL, NULL,
        NULL);

  XtVaCreateManagedWidget ("sep", xmSeparatorGadgetClass, rowCol, NULL);

  one = XmStringCreateLocalized(strcpy(s, "Cancel"));

  btn = XtVaCreateManagedWidget("button",
        xmPushButtonWidgetClass, rowCol,
        XmNlabelString, one,
        NULL);

  XtAddCallback (btn, XmNactivateCallback, SaveOptionsCB,
                  (void*) PL_SAVE_OPTIONS_CANCEL);

  two = XmStringCreateLocalized(strcpy(s,"Done"));

  btn = XtVaCreateManagedWidget("button",
        xmPushButtonWidgetClass, rowCol,
        XmNlabelString, two,
        NULL);
  XtAddCallback (btn, XmNactivateCallback, SaveOptionsCB,
                  (void*) PL_SAVE_OPTIONS_DONE);

  XmStringFree(one);
  XmStringFree(two);
  XmStringFree(three);

  XtManageChild(m_nSaveHeader);
  XtManageChild(m_nSavePrefix);
  XtManageChild(m_nSaveNumberFormat);
  XtManageChild(m_nSaveColumnDelimiter);
  XtManageChild(m_nSaveEndOfLine);
}
//---------------------------------------------------------------
//
CSaveOptionsDialog::~CSaveOptionsDialog()
{
	;
}
//---------------------------------------------------------------

void CSaveOptionsDialog::SetOptions(SaveOptions o)
{
    WidgetList wlist;
    int i, nchildren, n;

    // Set current fields to show options o

    XtVaGetValues(m_nSaveHeader, 
      XmNnumChildren, &nchildren,
      XmNchildren, &wlist, NULL);

    n = o.HeaderType;
    for (i = 0; i < nchildren; i++) 
      XmToggleButtonGadgetSetState(wlist[i], i==n, False);

    XtVaGetValues(m_nSaveNumberFormat,
      XmNnumChildren, &nchildren,
      XmNchildren, &wlist, NULL);

    n = o.NumberFormat;
    for (i = 0; i < nchildren; i++)
      XmToggleButtonGadgetSetState(wlist[i], i==n, False);

    XtVaGetValues(m_nSaveColumnDelimiter,
      XmNnumChildren, &nchildren, 
      XmNchildren, &wlist, NULL);

    n = o.Delimiter;
    for (i = 0; i < nchildren; i++)
      XmToggleButtonGadgetSetState(wlist[i], i==n, False);

    XmTextSetString(m_nSavePrefix, (char*) o.UserPrefix);
    if (o.HeaderType == 2) 
      XtVaSetValues(m_nSavePrefix,
        XmNeditable, True,
	NULL);

    m_nSO = o;
}
//---------------------------------------------------------------

void CSaveOptionsDialog::OnOptions(Widget parent, int set, int data)
{
    if (parent == m_nSaveHeader) {
      if (set) {
        m_nSO.HeaderType = data;
	bool editable = (data == 2);
        XtVaSetValues (parent,
            XmNeditable, editable,
    	    NULL);
      }
    }
    else if (parent == m_nSaveNumberFormat) {
       if (set)  m_nSO.NumberFormat = data;
    }
    else if (parent == m_nSaveColumnDelimiter) {
       if (set) m_nSO.Delimiter = data;
    }
    else if (parent == m_nSaveEndOfLine) {
      if (set) m_nSO.CrLf = data;
    }
    else
      ; 
}
//---------------------------------------------------------------


