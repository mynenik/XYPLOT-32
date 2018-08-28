// CPlot.h
//
// Declar class CPlot
//
// Copyright 1995--2018 Krishna Myneni
// <krishna.myneni@ccreweb.org>
//
// This software is provided under the terms of the
// GNU General Public License (GPL), v3.0 or later.

#ifndef __CPLOT_H__
#define __CPLOT_H__

#include "CDataset.h"
#include "CPlotObject.h"

enum Symbol {
    sym_LINE,
    sym_DASHED,
    sym_POINT,
    sym_BIG_POINT,
    sym_LINE_PLUS_POINT,
    sym_STICK,
    sym_HISTOGRAM
};

class CPlot : public CPlotObject {
protected:
    CDataset* m_pSet;                   // ptr to data set
    Symbol m_nSymbol;                   // plotting symbol (e.g. line, point)
public:
	CPlot (CDataset*);
    CDataset* GetSet () {return m_pSet;}
    void SetSymbol (Symbol s) {m_nSymbol = s;}
    Symbol GetSymbol () {return m_nSymbol;}
};


#endif
