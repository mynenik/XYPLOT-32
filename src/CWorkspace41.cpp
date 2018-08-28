// CWorkspace41.cpp
//
// Copyright 1995--2018 Krishna Myneni
// <krishna.myneni@ccreweb.org>
//
// This software is provided under the terms of the
// GNU General Public License (GPL), v3.0 or later.
//

#include <string.h>
#include <fstream>
using std::ifstream;
using std::ofstream;
#include "CWorkspace41.h"

//---------------------------------------------------------------

CWorkspace41::CWorkspace41()
{
	// Allocate space for an xyplot 4.1 workspace

    m_szHeader = new char [HEADER_LENGTH_41];
	x = new float[MAX_POINTS_41];
	y = new float[MAX_POINTS_41];
	if ((x == NULL) || (y == NULL)) return;
	pv = new Plot[MAX_PLOTS_41];
	ds = new Dataset[MAX_SETS_41];
}
//---------------------------------------------------------------

CWorkspace41::~CWorkspace41()
{
	delete [] x;
	delete [] y;
	delete [] pv;
	delete [] ds;
	delete [] m_szHeader;
}
//------------------------------------------------------------------

int CWorkspace41::ReadXsp (char* name)
{
	int j;
	char s[64];

	ifstream infile (name);
	if (infile.fail()) return (1);

	// Read identifier

	infile.read (s, 64);
	s[63] = '\0';
	strcpy (m_szHeader, s);

	if (! strstr (m_szHeader, "XYPLOT_WORKSPACE"))
	{
		return 2;   // not an xyplot workspace
	}

	// Read database info record

	infile.read ((char*) &db.nsets, 2);
	infile.read ((char*) &db.nvals, 2);
	infile.read ((char*) &db.hdr, HEADER_LENGTH_41);
	if (infile.fail()) return 3;   // file corrupt

	// Read display info record

	infile.read ((char*) &di.nplots, 2);
	infile.read ((char*) &di.plist, 2);
	infile.read ((char*) &di.sp1, 2);
	infile.read ((char*) &di.sp2, 2);
	infile.read ((char*) &di.scale, 2);
	infile.read ((char*) &di.xmin, 4);
	infile.read ((char*) &di.xmax, 4);
	infile.read ((char*) &di.ymin, 4);
	infile.read ((char*) &di.ymax, 4);
	infile.read ((char*) &di.csize, 2);
	infile.read ((char*) &di.cmode, 2);
	infile.read ((char*) &di.level, 2);
	infile.read ((char*) &di.grid, 2);
	infile.read ((char*) &di.mouse_button, 2);
	infile.read ((char*) &di.cx, 2);
	infile.read ((char*) &di.cy, 2);
	infile.read ((char*) &di.xsf, 4);
	infile.read ((char*) &di.ysf, 4);
	infile.read ((char*) &di.ylsf, 4);
	infile.read ((char*) &di.lymin, 4);

	// Read dataset information records

	for (j = 0; j < db.nsets; ++j)
	{
		infile.read ((char*) &ds[j].jsr, 2);
		infile.read ((char*) &ds[j].jer, 2);
		infile.read ((char*) &ds[j].jsi, 2);
		infile.read ((char*) &ds[j].jei, 2);
		infile.read ((char*) &ds[j].data_type, 2);
		infile.read ((char*) &ds[j].npts, 2);
		infile.read ((char*) &ds[j].xmin, 4);
		infile.read ((char*) &ds[j].xmax, 4);
		infile.read ((char*) &ds[j].ixmin, 4);
		infile.read ((char*) &ds[j].ixmax, 4);
		infile.read ((char*) &ds[j].ymin, 4);
		infile.read ((char*) &ds[j].ymax, 4);
		infile.read ((char*) &ds[j].iymin, 4);
		infile.read ((char*) &ds[j].iymax, 4);
		infile.read ((char*) &ds[j].fname, 25);
		infile.read ((char *) &ds[j].hdr, HEADER_LENGTH_41);
		if (infile.fail()) return 5;   // file corrupt
		ds[j].fname[24] = '\0';
		ds[j].hdr[HEADER_LENGTH_41 - 1] = '\0';
	}

	// Read plot information records

	for (j = 0; j < di.nplots; ++j)
	{
		infile.read ((char*) &pv[j].set, 2);
		infile.read ((char*) &pv[j].sym, 1);
		infile.read ((char*) &pv[j].col, 2);
		infile.read ((char*) &pv[j].vis, 2);
		if (infile.fail()) return 6;   // file corrupt
	}

	// Read x, y arrays

	infile.read ((char *) x, db.nvals*4);
	if (infile.fail()) return 7;       // file corrupt

	infile.read ((char *) y, 4*db.nvals);
	if (infile.fail()) return 8;       // file corrupt

	return 0;

}
//------------------------------------------------------------------

int CWorkspace41::WriteXsp (char* name)
{
  ofstream outfile(name);
  if (outfile.fail()) return 1;

  
  return 0;
}
