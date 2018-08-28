// CDataset.cpp
//
// Copyright (c) 1995--2018 Krishna Myneni
// <krishna.myneni@ccreweb.org>
//
// This software is provided under the terms of the
// GNU General Public License (GPL), v3.0 or later.
//

#include <string.h>
#include <math.h>
#include "CDataset.h"

CDataset::CDataset(int nLength, int nPoints, char* name, char* hdr) :
  vector<float> (nLength*nPoints)
{
  m_nDatumLength = nLength;
  
  // Initialize the extrema vector

  for (int i = 0; i < 2*nLength; i++) m_vEx.push_back(0.F);

  m_nOrdering = 0;
  m_nEquallySpaced = 0;
  m_szHeader = new char [HEADER_LENGTH];
  m_szName = new char [NAME_LENGTH];
  strcpy (m_szHeader, hdr);
  strcpy (m_szName, name);

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
    vector<float>::iterator p = begin(); 
    float last_x, delta;

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

vector<float>::iterator CDataset::Find(float x)
{
// Return pointer to the datum with first element value closest to x.

    vector<float>::iterator p = begin(), i;

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

	    if (ascending == (x > *(p + jm*m_nDatumLength)))
	      jl = jm;
	    else
	      ju = jm;
	  }
        p += jm*m_nDatumLength;
    }
    else
    {
        // use brute force search

        float dev = fabs(*begin() - x), dev2;

        for (i = begin() + m_nDatumLength; i < end(); i += m_nDatumLength)
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

int CDataset::IsOverlapping (float x1, float x2)
{
// Determine whether domain of dataset overlaps the interval (x1, x2)

    if (x2 < x1)
    {
        float x = x1;
        x1 = x2;
        x2 = x;
    }
    return !((m_vEx[0] > x2) || (m_vEx[1] < x1));
}
//----------------------------------------------------------------

vector<vector<float>::iterator> CDataset::Limits (float x1, float x2, 
						  int& bOverlap)
{
// Return vector with iterators to start data and end data in interval (x1, x2)

    vector<vector<float>::iterator> flim;
    bOverlap = IsOverlapping (x1, x2);
    if (bOverlap)
    {
        vector<float>::iterator p1, p2, p;

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

	    if ((*p1 < x1) && (p1 < (end() - m_nDatumLength)))
	      p1 += m_nDatumLength;
	    if ((*p2 > x2) && (p2 > begin())) p2 -= m_nDatumLength;

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

vector<int> CDataset::IndexLimits (float x1, float x2, int& bOverlap)
{
// Return vector with index range of data points which
//   occur in the interval (x1, x2).
//   NOTE: First point is at index 0.

    vector<int> lim;

    bOverlap = IsOverlapping (x1, x2);
    if (bOverlap)
    {
        vector<vector<float>::iterator> flim = Limits(x1, x2, bOverlap);
        int i1 = (flim[0] - begin())/m_nDatumLength;
        int i2 = (flim[1] - begin())/m_nDatumLength;
        lim.push_back(i1);
        lim.push_back(i2);
    }

    return lim;
}
//---------------------------------------------------------------

void CDataset::CopyData (float* a, int npts)
{
    vector<float>::iterator p = begin();
    int n = npts*m_nDatumLength;
    for (int i = 0; i < n; i++) *p++ = *a++;
    // memcpy ((float*) &(*this)[0], a, npts*m_nDatumLength*sizeof(float));
}

void CDataset::CopyData (float* x, int js, int je)
{
// Copy data from buffer given start and stop indices

    vector<float>::iterator p = begin();
    p += js*m_nDatumLength;

    int n = (je - js + 1)*m_nDatumLength;
    for (int i = 0; i < n; i++) *p++ = *x++;
    // memcpy (p, x, (je - js + 1)*m_nDatumLength*sizeof(float));
}

void CDataset::CopyData (vector<float>::iterator src, int npts)
{
// Copy data from the iterator for another dataset.

    vector<float>::iterator p = begin();
    int n = npts*m_nDatumLength;
    for (int i = 0; i < n; i++) *p++ = *src++;
}

void CDataset::CopyFromXY (float* x, float* y, int npts)
{
    vector<float>::iterator p = begin();

    for (int i = 0; i < npts; i++)
    {
	*p++ = *x++;
	for (int j = 1; j < m_nDatumLength; j++) *p++ = *y++;
    }
}

void CDataset::CopyFromXY (float* x, float* y, int js, int je)
{
    vector<float>::iterator p = begin();
    p += js*m_nDatumLength;

    for (int i = js; i <= je; i++)
    {
        *p++ = *x++;
        for (int j = 1; j < m_nDatumLength; j++) *p++ = *y++;
    }
}

void CDataset::CopyToBuffer (float* a)
{
    vector<float>::iterator p = begin();
    int n = size()*m_nDatumLength;
    for (int i = 0; i < n; i++) *a++ = *p++;

    // memcpy (a, (float*) &(*this)[0], size()*m_nDatumLength*sizeof(float));
}

//---------------------------------------------------------------


CReal::CReal(int ndim, int npts, char* name, char* hdr)
	: CDataset(ndim, npts, name, hdr)
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

CComplex::CComplex(int ndim, int npts, char* name, char* hdr)
	: CDataset(ndim*2, npts, name, hdr)
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




