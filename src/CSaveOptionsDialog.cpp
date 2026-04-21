// CSaveOptionsDialog.cpp
//
// Copyright 1998--2026 Krishna Myneni
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

extern XmFontList FontList;
extern void SaveOptionsCB (Widget, void*, void*);

//-------------------------------------------

CSaveOptionsDialog::CSaveOptionsDialog(Widget topLevel)
{
  // Create the Save Options dialog
  char s[64];
  XmString xstr;

  Widget soDialog, rowCol, btn, hdrStyleLabel;

  strcpy(s, "Save Options");
  m_nW = XmCreateFormDialog(topLevel, s, NULL, 0);
  xstr = XmStringCreateLocalized(s);
  XtVaSetValues( m_nW,
	XmNdialogTitle, xstr,
	NULL);
  XmStringFree(xstr);

  rowCol = XtVaCreateManagedWidget("rowcolumn",
         xmRowColumnWidgetClass, m_nW,
         NULL);

  XmString one, two, three;
  hdrStyleLabel = XtVaCreateManagedWidget("Header Style",
        xmLabelWidgetClass, rowCol,
	XmNfontList, FontList,
        NULL);

  one = XmStringCreateLocalized(strcpy(s,"none"));
  two = XmStringCreateLocalized(strcpy(s,"xyplot"));
  three = XmStringCreateLocalized(strcpy(s,"line prefix"));

  strcpy(s, "Header Style"),
  m_nSaveHeader = XmVaCreateSimpleRadioBox( rowCol,
        s,
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
	XmNfontList, FontList,
        NULL);

  strcpy(s, "text");
  m_nSavePrefix = XmCreateTextField(rowCol, s, NULL, 0);
  XtVaSetValues(m_nSavePrefix,
        XmNeditable, False,
	XmNfontList, FontList,
	XtVaTypedArg, XmNbackground, XmRString, "White", 6,
        NULL);

  XtVaCreateManagedWidget ("sep", 
	xmSeparatorGadgetClass, rowCol,
	NULL);

  Widget numberFormatLabel;
  numberFormatLabel = XtVaCreateManagedWidget ("Number Format",
        xmLabelWidgetClass, rowCol,
	XmNfontList, FontList,
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

  XtVaCreateManagedWidget ("sep",
	xmSeparatorGadgetClass, rowCol,
	NULL);

  XtVaCreateManagedWidget ("Column Delimiter",
	xmLabelWidgetClass, rowCol,
	XmNfontList, FontList,
	NULL);

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

  XmStringFree(one);
  XmStringFree(two);
  XmStringFree(three);

  XtVaCreateManagedWidget ("sep",
	xmSeparatorGadgetClass, rowCol,
	NULL);

  XtVaCreateManagedWidget ("End of Line",
        xmLabelWidgetClass, rowCol,
	XmNfontList, FontList,
        NULL);

  one = XmStringCreateLocalized(strcpy(s, "Unix"));
  two = XmStringCreateLocalized(strcpy(s, "DOS"));
  
  m_nSaveEndOfLine = XmVaCreateSimpleRadioBox (rowCol,
        strcpy(s,"EOL"),
        0,
        SaveOptionsCB,
        XmVaRADIOBUTTON, one, NULL, NULL, NULL,
        XmVaRADIOBUTTON, two, NULL, NULL, NULL,
	XmNfontList, FontList,
        NULL);

  XmStringFree(one);
  XmStringFree(two);

  XtVaCreateManagedWidget ("sep",
	xmSeparatorGadgetClass, rowCol,
	NULL);

  one = XmStringCreateLocalized(strcpy(s, "Apply"));

  btn = XtVaCreateManagedWidget("button",
        xmPushButtonWidgetClass, rowCol,
        XmNlabelString, one,
        NULL);

  XtVaSetValues( btn,
	XmNfontList, FontList,
	NULL);

  XtAddCallback (btn, XmNactivateCallback, SaveOptionsCB,
                  (void*) PL_SAVE_OPTIONS_APPLY);

  two = XmStringCreateLocalized(strcpy(s,"Close"));

  btn = XtVaCreateManagedWidget("button",
        xmPushButtonWidgetClass, rowCol,
        XmNlabelString, two,
        NULL);

  XtAddCallback (btn, XmNactivateCallback, SaveOptionsCB,
                  (void*) PL_SAVE_OPTIONS_CLOSE);

  XtVaSetValues( btn,
        XmNfontList, FontList,
        NULL);

  XmStringFree(one);
  XmStringFree(two);

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

void CSaveOptionsDialog::UpdatePrefixOption()
{
    // copy prefix string from text widget to m_nSO
    char* prefix = XmTextFieldGetString(m_nSavePrefix);
    int prefixLen = strlen(prefix);
    prefixLen = (prefixLen > 15) ? 15 : prefixLen;
    strncpy((char*) m_nSO.UserPrefix, prefix, prefixLen);
    m_nSO.UserPrefix[prefixLen] = '\0';
}

