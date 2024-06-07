// CSaveOptionsDialog.h
//
// Header file for class CSaveOptionsDialog
//
// Copyright 1998--2024 Krishna Myneni
//
// Provided under the terms of the GNU Affero Public License
// (AGPL) v 3.0 or later.

#ifndef __CSAVEOPTIONSDIALOG_H__
#define __CSAVEOPTIONSDIALOG_H__
class CSaveOptionsDialog {
    // CButton* m_pButtons[11];
    SaveOptions m_nSO;
public:
    Widget m_nW;  // main widget
    Widget m_nSaveHeader;
    Widget m_nSavePrefix;
    Widget m_nSaveNumberFormat;
    Widget m_nSaveColumnDelimiter;
    Widget m_nSaveEndOfLine;

    CSaveOptionsDialog (Widget);
    ~CSaveOptionsDialog();

    void SetOptions(SaveOptions);
    SaveOptions GetOptions() {return m_nSO;};

    // callback helper
    void OnOptions (Widget, int, int);
};

#endif
