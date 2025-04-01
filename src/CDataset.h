// CDataset.h
//
// Abstract base class CDataset
//
// Copyright (c) 1995--2025 Krishna Myneni
// <krishna.myneni@ccreweb.org>
//
// This software is provided under the terms of the
// GNU Affero General Public License (AGPL), v3.0 or later.
//

#ifndef __CDATASET_H__
#define __CDATASET_H__

#include<vector>
using std::vector;

// Data set types
const int REAL_SINGLE = 0;    // real, single-precision float
const int REAL_DOUBLE = 256;  // real, double-precision float

const int HEADER_LENGTH = 4096;
const int NAME_LENGTH = 256;

class CDataset : public vector<double>
{
protected:
	vector<double> m_vEx;  // the extrema vector
	int m_nDatumSize;
	int m_nOrdering;
	int m_nEquallySpaced;
public:
	char* m_szName;
	char* m_szHeader;
	CDataset (int, int, char*, char*);
	~CDataset ();
	int NumberOfPoints() {return size()/m_nDatumSize;}
	int SizeOfDatum() {return m_nDatumSize;}
	void SetExtrema();
	vector<double> GetExtrema() {return m_vEx;}
	void CopyData (double*, int);
	void CopyData (vector<double>::iterator, int);
	void CopyData (double*, int, int);
	void CopyFromXY (double*, double*, int);
	void CopyFromXY (float*, float*, int);
	void CopyFromXY (double*, double*, int, int);
	void CopyToBuffer (double*);
	bool AppendToHeader (char*);
	vector<double>::iterator Find(double);
	int IsOverlapping (double, double);
	vector<vector<double>::iterator> Limits (double, double, int&);
	vector<int> IndexLimits (double, double, int&);
	virtual CDataset* Duplicate() = 0;
};

class CReal : public CDataset
{
public:
    CReal (int, int, char*, char*);
    CDataset* Duplicate();
};

class CComplex : public CDataset {
public:
    CComplex (int, int, char*, char*);
    CDataset* Duplicate();
};

#endif
