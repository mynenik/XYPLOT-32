// CWorkspace41.h
//
// Header file for class CWorkspace41
//
// Copyright 1995--2018 Krishna Myneni
// <krishna.myneni@ccreweb.org>
//
// This software is provided under the terms of the
// GNU General Public License (GPL), v3.0 or later.

#ifndef __CWORKSPACE41_H__
#define __CWORKSPACE41_H__

#include "xyp41.h"

class CWorkspace41 {
public:
	char* m_szHeader;
	struct Database db;
	struct Display41 di;
	struct Dataset* ds;
	struct Plot* pv;
	float * x;
	float * y;
	CWorkspace41 ();
	~CWorkspace41 ();
	int ReadXsp (char*);
	int WriteXsp (char*);
};

#endif
