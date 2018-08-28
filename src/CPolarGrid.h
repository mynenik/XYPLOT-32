// CPolarGrid.h
//
// Header file for class CPolarGrid
//
// Copyright 1996--2018 Krishna Myneni
// <krishna.myneni@ccreweb.org>
//
// This software is provided under the terms of the
// GNU General Public License (GPL), v3.0 or later.
//

#ifndef __CPOLARGRID_H__
#define __CPOLARGRID_H__

#include "CGrid.h"
#define CDC CDeviceContext

class CPolarGrid : public CGrid {
public:
	CPolarGrid ();
	void Labels (CDC*);
	void Draw (CDC*);
};

#endif
