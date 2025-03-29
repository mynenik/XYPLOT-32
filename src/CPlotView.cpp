// CPlotView.cpp
//
// Copyright 2020--2023 Krishna Myneni
//
// This software is provided under the terms of the
// GNU Affero General Public License (AGPL) v 3.0 or later.
//

#include <vector>
using namespace std;
#include "CTransform.h"
#include "CPlotObject.h"
#include "CGrid.h"
#include "CPolarGrid.h"
#include "CPlotView.h"

CPlotView::CPlotView( COORDINATE_SYSTEM cdns, vector<double> x )
{
    m_pCt = NULL;
    m_pGrid = NULL;

    switch (cdns)
    {
      case CARTESIAN:
        m_pCt   = new C2D_Transform();
        m_pGrid = new CCartesianGrid();
        break;
      case CARTESIAN_INVERTED:
        m_pCt   = new C2Di_Transform();
        m_pGrid = new CCartesianGrid();
        break;
      case POLAR:
        m_pCt   = new C2D_PolarTransform();
        m_pGrid = new CPolarGrid();
        break;
       default:
         ;
    }
    m_pGrid->SetTransform (m_pCt);
    SetExtrema (x);
}

CPlotView::~CPlotView()
{
    delete m_pGrid;
    delete m_pCt;
}

void CPlotView::SetExtrema(vector<double> x)
{
     m_pCt->SetLogical(x);
}

vector<double> CPlotView::GetExtrema()
{
    return( m_pCt->GetLogical() );
}


