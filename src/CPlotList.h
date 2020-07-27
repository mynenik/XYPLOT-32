// CPlotList.h
//
// Copyright (c) 1996-2020 Krishna Myneni
// <krishna.myneni@ccreweb.org>
//
// This software is provided under the terms of the 
// GNU Affero General Public License (AGPL), v 3.0 or later.
//

#ifndef __CPLOTLIST_H__
#define __CPLOTLIST_H__

#include "CPlot.h"

struct PlotListNode             // node structure for linked list of plots
{
    CPlot* Plot;
    CRgn* Rgn;
    PlotListNode* Prev;
    PlotListNode* Next;
};

class CPlotList
{
    PlotListNode* m_pHead;        // ptr to start of a linked list of plots
    CPlot* m_pActive;
    CPlot* m_pOperand;
    BOOL FindInList (CPlot*);
public:
    CPlotList ();
    ~CPlotList ();
    CPlot* Active() {return m_pActive;}
    CPlot* Operand() {return m_pOperand;}
    CPlot* PlotOf (CDataset*) ;
    CPlot* Selection (CPoint);
    int Nplots ();
    unsigned long NextColor();
    void AddPlot (CPlot*);
    void RemovePlot (CPlot*);
    BOOL SetActive(CPlot*);
    BOOL SetOperand(CPlot*);
    char* GetList();
    void DisplayList (CDC*);
    void Draw (CDC*);
    vector<float> GetExtrema ();
    CPlot* operator[] (const int);
//    BOOL Write (CFile*);
};

#endif
