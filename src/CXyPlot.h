// CXyPlot.h
//
// Header file for class CXyPlot
//
// Copyright 1995--2018 Krishna Myneni
// <krishna.myneni@ccreweb.org>
//
// This software is provided under the terms of the
// GNU General Public License (GPL), v3.0 or later.

#ifndef __CXYPLOT_H__
#define __CXYPLOT_H__

#include "CPlot.h"
#define CDC CDeviceContext

class CXyPlot : public CPlot {
	int m_nPointWidthScale;
public:
	CXyPlot (CDataset*);
	void Draw (CDC*);
	void DrawPointPlot (CDC*, int);
	void DrawLinePlot (CDC*);
	void DrawStickPlot (CDC*);
	void DrawHistogram (CDC*);
};

#endif
