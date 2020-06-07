// CPlotList.cpp
//
// Copyright (c) 1996--2018 Krishna Myneni
// <krishna.myneni@ccreweb.org>
//
// This software is provided under the terms of the 
// GNU General Public License (GPL), v3.0 or later.
//

#include "CPlotList.h"
#include <string.h>
#include <stdio.h>

#define MAX_COLORS 32

CPlotList::CPlotList()
{
    m_pHead = NULL;
    m_pActive = NULL;
    m_pOperand = NULL;

}
//---------------------------------------------------------------

CPlotList::~CPlotList()
{

    PlotListNode *node = m_pHead, *next_node;

    // Traverse the list and delete the plots and list nodes

    while (node)
    {
        next_node = node->Next;
        delete node->Plot;          // delete the plot
        delete node->Rgn;           // delete the display region
        delete node;                // delete the node
        node = next_node;
    }

}
//---------------------------------------------------------------

int CPlotList::Nplots ()
{
// Return the number of plots in the list

    int n = 0;
    PlotListNode* node = m_pHead;

    while (node)
    {
        ++n;
        node = node->Next;
    }

    return n;
}
//---------------------------------------------------------------

CPlot* CPlotList::operator[] (const int n)
{
  // Return a pointer to the n^th plot

  CPlot* p = NULL;
  int nP = Nplots();

  if ((nP > 0) && (n < nP))
    {
      PlotListNode* node = m_pHead;
      for (int i = 0; i < n; i++)
	node = node->Next;
      p = node->Plot;
    }
  return p;
}
//---------------------------------------------------------------

BOOL CPlotList::FindInList (CPlot* p)
{
// Search the list for plot p; return true if found, false otherwise

    PlotListNode* node = m_pHead;

    while (node)
    {
        if (node->Plot == p) return TRUE;
        node = node->Next;
    }
    return FALSE;
}
//---------------------------------------------------------------

CPlot* CPlotList::PlotOf (CDataset* d)
{
// Search the list for plot of data set d.
// Return a pointer to that plot; NULL pointer if not found

    PlotListNode* node = m_pHead;
    CPlot* p;

    while (node)
    {
        p = node->Plot;
        if (p->GetSet() == d) return p;
        node = node->Next;
    }

    return NULL;
}
//---------------------------------------------------------------

COLORREF CPlotList::NextColor()
{
// Return the first unused color in the color table

    PlotListNode* node = m_pHead;
    unsigned int c = 0;

    if (node)
    {
        for (int i = 0; i < MAX_COLORS; i++)
        {
            while (node)
            {
	      if (node->Plot->GetColor() == i) break;
              node = node->Next;
            }
            if (node == NULL)
            {
	      c = i;
              break;
            }
        }
    }

    return c;
}
//---------------------------------------------------------------

void CPlotList::AddPlot(CPlot* p)
{
    if (p == NULL) return;
    int dx = 15, i = 0;

    p->SetAttributes(PS_SOLID, 1, NextColor());

    PlotListNode* node = m_pHead;

    if (node)
    {
        // Traverse the list until last node is reached

        i++;
        while (node->Next) {node = node->Next; i++;}

        // Create a new node and link it to the list

        node->Next = new PlotListNode;
        node->Next->Prev = node;
        node = node->Next;
    }
    else
    {
        m_pHead = new PlotListNode;    // create the head node
        node = m_pHead;
        node->Prev = NULL;
        m_pOperand = p;
    }

    node->Plot = p;
// node->Rgn = new CRgn();
// node->Rgn->CreateEllipticRgn(i*dx, 0, (i+1)*dx, dx);
    node->Rgn = NULL;
    node->Next = NULL;
    m_pActive = p;
}
//---------------------------------------------------------------

void CPlotList::RemovePlot(CPlot* p)
{
// Remove the node for plot p from the doubly-linked-list
//   of plots. NOTE: The plot object itself is not deleted
//   by this routine, only its node. It is up to the calling
//   routine to explicitly delete the plot object if necessary.

    if (p == NULL) return;

    PlotListNode *node = m_pHead, *tempNode;
    CRgn *tempRgn1, *tempRgn2;

    while (node)
    {
        if (node->Plot == p)
        {
            // If plot to be removed is either the active or
            //   operand plots; make the previous or the next
            //   plot the active/operand.

            if (p == m_pActive)
            {
                if (node->Prev)
                    m_pActive = node->Prev->Plot;
                else if (node->Next)
                    m_pActive = node->Next->Plot;
                else
                    m_pActive = NULL;
            }

            if (p == m_pOperand)
            {
                if (node->Prev)
                    m_pOperand = node->Prev->Plot;
                else if (node->Next)
                    m_pOperand = node->Next->Plot;
                else
                    m_pOperand = NULL;
            }

            // splice the node out of the list

            if (node->Prev) node->Prev->Next = node->Next;
            if (node->Next) node->Next->Prev = node->Prev;

            if (node == m_pHead)
            {
                m_pHead = node->Next;
                if (m_pHead)
                    m_pHead->Prev = NULL;
                else
                    break;
            }

            tempNode = node;

            // slide all regions down the list

// tempRgn1 = node->Rgn;
//
// while (node->Next)
// {
// tempRgn2 = node->Next->Rgn;
// node->Next->Rgn = tempRgn1;
// tempRgn1 = tempRgn2;
// node = node->Next;
// }
//
// delete tempRgn1;
            delete tempNode;

            break;
        }
        node = node->Next;
    }

}
//---------------------------------------------------------------

BOOL CPlotList::SetActive (CPlot* p)
{
    if (! FindInList(p)) return FALSE;

    m_pActive = p;
    return TRUE;
}
//---------------------------------------------------------------

BOOL CPlotList::SetOperand (CPlot* p)
{
    if (! FindInList(p)) return FALSE;

    m_pOperand = p;
    return TRUE;
}
//---------------------------------------------------------------

CPlot* CPlotList::Selection (CPoint p)
{
    // Return pointer to plot if a node's region contains
    //   the point p

    PlotListNode* node = m_pHead;

    while (node)
    {
// if (node->Rgn->PtInRegion(p)) return node->Plot;
        node = node->Next;
    }

    return NULL;
}
//---------------------------------------------------------------

char* CPlotList::GetList ()
{
  // Return the current list of plots as a text string
  //   marked to show the active and operand plots

  static char text_plot_list [80];
  char* cp = text_plot_list, s[16];

  int n = Nplots();
  CPlot* p;

  *cp = '\0';

  for (int i = 0; i < n; i++)
    {
      p = (*this)[i];
      if (p == m_pActive)
	  sprintf(s, "[%1d]", i+1);
      else if (p == m_pOperand)
	sprintf(s, "(%1d)", i+1);
      else
	sprintf(s, " %1d ", i+1);

      strcat(cp, s);
    }

  return cp;
}
//---------------------------------------------------------------

void CPlotList::DisplayList (CDC* pDC)
{
    // Display the plot list as a series of colored circles

    PlotListNode* node = m_pHead;
/*
    CBrush *pBrush, *pOldBrush, *pEraseBrush;

    pEraseBrush = new CBrush (RGB(255,255,255));

    while (node)
    {
        pBrush = new CBrush (node->Plot->GetColor());
        pOldBrush = pDC->SelectObject(pBrush);

        pDC->FillRgn(node->Rgn, pEraseBrush);

        if (node->Plot == m_pActive)
            pDC->PaintRgn(node->Rgn);
        else if (node->Plot == m_pOperand)
            pDC->FrameRgn(node->Rgn, pBrush, 3, 3);
        else
            pDC->FrameRgn(node->Rgn, pBrush, 1, 1);


        pDC->SelectObject(pOldBrush);

        delete pBrush;

        node = node->Next;
    }

    delete pEraseBrush;
*/
}
//---------------------------------------------------------------

void CPlotList::Draw(CDC* pDC)
{
    PlotListNode* node = m_pHead;

    while (node)
    {
        node->Plot->Draw(pDC);
        node = node->Next;
    }
}
//---------------------------------------------------------------

vector<float> CPlotList::GetExtrema()
{
// Return extrema vector for the datasets which are plotted

	CDataset* d;
	vector<float> e(4), ex(4);
    int i;

    PlotListNode* node = m_pHead;
    CPlot* p;

    if (node)
    {
	    p = node->Plot;
	    d = p->GetSet();
	    ex = d->GetExtrema();

        node = node->Next;

	    while (node)
	    {

	        p = node->Plot;
	        d = p->GetSet();
		    e = d->GetExtrema();
		    if (e[0] < ex[0]) ex[0] = e[0];
		    if (e[1] > ex[1]) ex[1] = e[1];
		    if (e[2] < ex[2]) ex[2] = e[2];
		    if (e[3] > ex[3]) ex[3] = e[3];

		    node = node->Next;
	    }
	}
	else
	{
	    ex[0] = -1.;
	    ex[1] = 1.;
	    ex[2] = -1.;
	    ex[3] = 1.;
	}

    return ex;
}



