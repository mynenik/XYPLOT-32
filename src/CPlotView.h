// CPlotView.h
//
// Header file for class CPlotView
//
// Copyright 2020--2023 Krishna Myneni
//
// This software is provided under the terms of the
// GNU Affero General Public License (AGPL) v 3.0 or later.
//
#ifndef __CPLOTVIEW_H__
#define __CPLOTVIEW_H__

enum COORDINATE_SYSTEM {CARTESIAN, CARTESIAN_INVERTED, POLAR};

class CPlotView {
public:
	CTransform* m_pCt;
	CGrid* m_pGrid;

	CPlotView (COORDINATE_SYSTEM, vector<double>);
	~CPlotView ();
	void SetExtrema (vector<double>);
	vector<double> GetExtrema ();
};

#endif

