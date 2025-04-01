// CDataset.cpp
//
// Copyright (c) 1995--2025 Krishna Myneni
// <krishna.myneni@ccreweb.org>
//
// This software is provided under the terms of the
// GNU General Public License (AGPL), v3.0 or later.
//

#include <string.h>
#include <math.h>
#include "CDataset.h"

CDataset::CDataset(int nSize, int nPoints, char* name, char* hdr) :
  vector<double> (nSize*nPoints)
{
  m_nDatumSize = nSize;
  
  // Initialize the extrema vector

  for (int i = 0; i < 2*m_nDatumSize; i++) m_vEx.push_back(0.0);

  m_nOrdering = 0;
  m_nEquallySpaced = 0;
  m_szHeader = new char [HEADER_LENGTH];
  m_szName = new char [NAME_LENGTH];
  if (hdr)
    strcpy (m_szHeader, hdr);
  else
    *m_szHeader = '\0';
  if (name)
    strcpy (m_szName, name);
  else
    *m_szName = '\0';

}
//---------------------------------------------------------------

CDataset::~CDataset()
{
	delete [] m_szName;
	delete [] m_szHeader;
}
//---------------------------------------------------------------

void CDataset::SetExtrema()
{
// Determine the extrema vector for the data.
// Ordering inside vector is {min0, max0, min1, max1, ...}
// where min0, max0 are the min and max of the first data
// element, and so on.

    int i, j, j2, nsize, ordering, equal_sp = 1;
    vector<double>::iterator p = begin(); 
    double last_x, delta;

    nsize = SizeOfDatum();

    last_x = *p;

    if (*(p+nsize) > last_x)
      ordering = 1;
    else if (*(p+nsize) < last_x)
      ordering = -1;
    else
      ordering = 0;

    delta = *(p+nsize) - last_x;

    for (i = 0; i < nsize; i++)
    {
        m_vEx[i << 1] = *(p + i);
        m_vEx[(i << 1) + 1] = *(p + i);
    }

    int n = NumberOfPoints();

    for (i = 0; i < n; i++)
    {
      if (ordering)
	{
	  if ((*p > last_x) && (ordering < 0))
	    ordering = 0;
	  else if ((*p < last_x) && (ordering > 0))
	    ordering = 0;
	  else
	    ;
	}
        
      if (equal_sp)
	{
	  if ((*p - last_x) != delta) equal_sp = 0;
	}
      last_x = *p;

      for (j = 0; j < nsize; j++)
	{
	  j2 = j << 1; // j2 = 2*j
          if (*p < m_vEx[j2]) m_vEx[j2] = *p;
          if (*p > m_vEx[j2+1]) m_vEx[j2+1] = *p;
          ++p;
        }
    }

    m_nOrdering = ordering;
    m_nEquallySpaced = equal_sp;
}
//---------------------------------------------------------------

vector<double>::iterator CDataset::Find(double x)
{
// Return pointer to the datum with first element value closest to x.

    vector<double>::iterator p = begin(), i;

    if (m_nOrdering)
    {
        // use intelligent search

        int jl = -1;
	int ju = NumberOfPoints();
        int jm = 0;
        int ascending = m_nOrdering > 0;

	while ((ju - jl) > 1)
	  {
            jm = (ju + jl)/2;

	    if (ascending == (x > *(p + jm*m_nDatumSize)))
	      jl = jm;
	    else
	      ju = jm;
	  }
        p += jm*m_nDatumSize;
    }
    else
    {
        // use brute force search

        double dev = fabs(*begin() - x), dev2;

        for (i = begin() + m_nDatumSize; i < end(); i += m_nDatumSize)
        {
            dev2 = fabs(*i - x);
            if (dev2 < dev)
            {
                dev = dev2;
                p = i;
            }
        }
    }

    return p;

}
//---------------------------------------------------------------

int CDataset::IsOverlapping (double x1, double x2)
{
// Determine whether domain of dataset overlaps the interval (x1, x2)

    if (x2 < x1)
    {
        double x = x1;
        x1 = x2;
        x2 = x;
    }
    return !((m_vEx[0] > x2) || (m_vEx[1] < x1));
}
//----------------------------------------------------------------

vector<vector<double>::iterator> CDataset::Limits (double x1, double x2, 
						  int& bOverlap)
{
// Return vector with iterators to start data and end data in interval (x1, x2)

    vector<vector<double>::iterator> flim;
    bOverlap = IsOverlapping (x1, x2);
    if (bOverlap)
    {
        vector<double>::iterator p1, p2, p;

	if (m_nOrdering)
	  {
	    p1 = Find(x1);
	    p2 = Find(x2);
	    if (p2 < p1)
	      {
		p = p1;
		p1 = p2;
		p2 = p;
	      }

	    // Ensure limits are inside of the interval
	    //   Find() only finds nearest point.

	    if ((*p1 < x1) && (p1 < (end() - m_nDatumSize)))
	      p1 += m_nDatumSize;
	    if ((*p2 > x2) && (p2 > begin())) p2 -= m_nDatumSize;

	  }
	else
	  {
	    p1 = begin();
	    p2 = p1 + (NumberOfPoints() - 1)*SizeOfDatum();
	  }
        flim.push_back(p1);
        flim.push_back(p2);
    }
    return flim;
}
//---------------------------------------------------------------

vector<int> CDataset::IndexLimits (double x1, double x2, int& bOverlap)
{
// Return vector with index range of data points which
//   occur in the interval (x1, x2).
//   NOTE: First point is at index 0.

    vector<int> lim;

    bOverlap = IsOverlapping (x1, x2);
    if (bOverlap)
    {
        vector<vector<double>::iterator> flim = Limits(x1, x2, bOverlap);
        int i1 = (flim[0] - begin())/m_nDatumSize;
        int i2 = (flim[1] - begin())/m_nDatumSize;
        lim.push_back(i1);
        lim.push_back(i2);
    }

    return lim;
}
//---------------------------------------------------------------

void CDataset::CopyData (double* a, int npts)
{
    vector<double>::iterator p = begin();
    int n = npts*m_nDatumSize;
    for (int i = 0; i < n; i++) *p++ = *a++;
}

void CDataset::CopyData (double* x, int js, int je)
{
// Copy data from buffer given start and stop indices

    vector<double>::iterator p = begin();
    p += js*m_nDatumSize;

    int n = (je - js + 1)*m_nDatumSize;
    for (int i = 0; i < n; i++) *p++ = *x++;
}

void CDataset::CopyData (vector<double>::iterator src, int npts)
{
// Copy data from the iterator for another dataset.

    vector<double>::iterator p = begin();
    int n = npts*m_nDatumSize;
    for (int i = 0; i < n; i++) *p++ = *src++;
}

void CDataset::CopyFromXY (double* x, double* y, int npts)
{
    vector<double>::iterator p = begin();

    for (int i = 0; i < npts; i++)
    {
	*p++ = *x++;
	for (int j = 1; j < m_nDatumSize; j++) *p++ = *y++;
    }
}

// Copy from float arrays needed to support DOS xyplot workspaces
void CDataset::CopyFromXY (float* x, float* y, int npts)
{
    vector<double>::iterator p = begin();

    for (int i = 0; i < npts; i++)
    {
        *p = (double) (*x); p++; x++;
        for (int j = 1; j < m_nDatumSize; j++) {
          *p = (double) (*y); p++; x++;
        }
    }
}

void CDataset::CopyFromXY (double* x, double* y, int js, int je)
{
    vector<double>::iterator p = begin();
    p += js*m_nDatumSize;

    for (int i = js; i <= je; i++)
    {
        *p++ = *x++;
        for (int j = 1; j < m_nDatumSize; j++) *p++ = *y++;
    }
}

void CDataset::CopyToBuffer (double* a)
{
    vector<double>::iterator p = begin();
    int n = size()*m_nDatumSize;
    for (int i = 0; i < n; i++) *a++ = *p++;
}

//---------------------------------------------------------------

bool CDataset::AppendToHeader (char* szNewText)
{
    // Note: If a new line needs to be appended, it must be included
    // at the end szNewText.
    int len1 = strlen(m_szHeader);
    int len2 = strlen(szNewText);
    char* p = NULL;
    if ((len1+len2) < HEADER_LENGTH) {
      p = strcat(m_szHeader, szNewText);
    }
    return (p != NULL);
}
//---------------------------------------------------------------

CReal::CReal(int nsize, int npts, char* name, char* hdr)
	: CDataset(nsize, npts, name, hdr)
{
    ;
}
//---------------------------------------------------------------


CDataset* CReal::Duplicate()
{
// Create a new real dataset, duplicate its contents
//  and return a ptr to it

    CReal* ds = new CReal (SizeOfDatum(), NumberOfPoints(), m_szName, m_szHeader);
    ds->CopyData (begin(), NumberOfPoints());
    ds->m_vEx = m_vEx;
    return ((CDataset*) ds);
}
//---------------------------------------------------------------

CComplex::CComplex(int nsize, int npts, char* name, char* hdr)
	: CDataset(nsize*2, npts, name, hdr)
{
    ;
}
//---------------------------------------------------------------

CDataset* CComplex::Duplicate()
{
// Create a new complex dataset, duplicate its contents
//  and return a ptr to it

    CComplex* ds = new CComplex (SizeOfDatum()/2, NumberOfPoints(), 
      m_szName, m_szHeader);
    ds->CopyData (begin(), NumberOfPoints());
    ds->m_vEx = m_vEx;
    return ((CDataset*) ds);
}




