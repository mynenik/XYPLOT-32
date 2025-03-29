// CDatabase.cpp
//
// Copyright 1995--2024 Krishna Myneni
// <krishna.myneni@ccreweb.org>
//
// This software is provided under the terms of the
// GNU General Public License (GPL), v3.0 or later.
//
#include <stdlib.h>
#include <string.h>
#include "consts.h"
#include "CDatabase.h"

//---------------------------------------------------------------

CDatabase::CDatabase()
{
    Clear();

    m_nSave.HeaderType = 1;  // standard xyplot format
    m_nSave.Delimiter = 0;
    m_nSave.NumberFormat = 0;
    m_nSave.CrLf = 0;
    // default prefix for alternate header style is xmgrace comment
    strcpy(m_nSave.UserPrefix, "#");
}
//---------------------------------------------------------------
CDatabase::~CDatabase()
{
  Clear();  // delete the dataset list
}
//---------------------------------------------------------------

void CDatabase::RemoveSet(CDataset* d)
{
// Remove a data set entry from the database.
// NOTE: the actual data set object is not deleted!

  int n = m_nSets.size();
  if (d == NULL || n == 0) return;

  for (vector<CDataset*>::iterator i = m_nSets.begin(); i < m_nSets.end(); i++)
    {
      if (d == *i) m_nSets.erase(i);
    }
}
//---------------------------------------------------------------

int CDatabase::IndexOf (CDataset* d)
{
  // Return the vector index of the dataset d in the databse;
  // return -1 if dataset is not found in the database.

  int nSet = -1;
  if (d)
    {
      int i, n = Nsets();
      for (i = 0; i < n; i++)
	{
	  if (m_nSets[i] == d) break;
	}
      if (i < n) nSet = i;
    }
  return nSet;
}
//---------------------------------------------------------------

CDataset* CDatabase::FindInList (char* name)
{
// Return a pointer to the dataset with the specified name
// WARNING: This function returns the first data set in the
//          database with the matching name. If multiple data
//          sets with the same name exist, only the first
//          matching dataset is returned.

    int n = m_nSets.size();

    for (int i = 0; i < n; i++)
    {
        if (strcmp(m_nSets[i]->m_szName, name) == 0) return m_nSets[i];
    }

    return NULL;
}
//---------------------------------------------------------------

int CDatabase::MakeListOfNames (char** sp)
{
// Create list of names of all datasets in string array sp
//   Returns the number of datasets

    int n = m_nSets.size();

    for (int i = 0; i < n; i++)
        strcpy (sp[i], m_nSets[i]->m_szName);

    return n;

}
//---------------------------------------------------------------

void CDatabase::Clear()
{
// Delete the dataset vector (and data sets as well) and
//   reset to initial conditions.

    int n = m_nSets.size();

    if (n) {
      for (int i = 0; i < n; i++)  delete m_nSets[i];
      m_nSets.erase(m_nSets.begin(), m_nSets.end());
    }
}
//-----------------------------------------------------------------

int CDatabase::GetColumnCount (char* name)
{
  // Return the number of columns in a dataset file, -1
  //   if failed to open file.

    if (!strchr(name, '.')) strcat (name, ".dat");

    CXyFile f (name, 0);

    if (f.Fail())
    {
        return -1;
    }

    f.ReadHeader ();

    return f.Columns();
}
//-----------------------------------------------------------------

int CDatabase::LoadDataset (CDataset** pd, char* name, 
  vector<int> columns, int dataset_type)
{
    int nPts, nCols, nSel, total_nPts = 0, Cols[8];
    char s[256];

    if (!strchr(name, '.')) strcat (name, ".dat");

    CXyFile f (name, 0);

    if (f.Fail())
    {
        return -1;
    }

    f.ReadHeader ();

    nCols = f.Columns();

    if (nCols < 1)
    {
        return -2;
    }

    if (columns.size() == 0)    // no columns specified
      {
	switch(nCols)
	  {
	  case 1:
            nSel = 1;
            Cols[0] = 0;
            break;
          case 2:
            nSel = 2;
            Cols[0] = 0;
            Cols[1] = 1;
            break;
          default:
            return -3;       // more than two columns and no columns specified
	  }
      }
    else
      {
	nSel = columns.size();
	if (nSel > nCols) return -4;  // Error in column specification 
	for (int j = 0; j < nSel; j++) {
	    if (columns[j] <= nCols)
	      Cols[j] = columns[j];
	    else
	      return -4;      // Error in column specification
	}
      }

    vector<double*> LoadBuffers;
    vector<double*>::iterator i;
    double* pData;

    do
    {
        pData = new double[MAX_POINTS*nSel];
        if (pData == NULL)
        {
            if (LoadBuffers.size())
            {
                for (i = LoadBuffers.begin(); i < LoadBuffers.end(); i++)
                {
                    delete [] *i;
                }
            }
            return -5;    // Error allocating memory
        }

        LoadBuffers.push_back(pData);
        nPts = f.ReadData (pData, nSel, Cols);
        if (nPts <= 0) break;

        total_nPts += nPts;

    } while (nPts == MAX_POINTS) ;

    if (LoadBuffers.size())
    {

        CDataset* d = MakeDataset(LoadBuffers, total_nPts, nSel, 
	  dataset_type, name, f.m_szHeader);
        if (d)
        {
            AddSet(d);
            *pd = d;
        }

        for (i = LoadBuffers.begin(); i < LoadBuffers.end(); i++)
        {
            delete [] *i;
        }
    }


    return total_nPts;
}
//---------------------------------------------------------------

int CDatabase::LoadWorkspace (CWorkspace41** ppWs, char* name)
{
  int i, ecode;
  CDataset* d;
  CWorkspace41* pWs;

  pWs = new CWorkspace41(); // allocate a 4.1 workspace
  *ppWs = pWs;

  ecode = pWs->ReadXsp(name);

  if (ecode == 0)
    {
      for (i = 0; i < pWs->db.nsets; i++)
        {
	  d = MakeDataset(pWs, i);
	  if (d) AddSet(d);
        }
    }

  return ecode;
}
//---------------------------------------------------------------

CDataset* CDatabase::MakeDataset (vector<double*> LoadBuffers, int nPts,
    int nCols, int nType, char* name, char* hdr)
{
  // Make new data set object

  CDataset* d;
  int nDim;

  switch (nType) {
    case 256:
      nDim = (nCols > 2) ? nCols : 2;
      d = new CReal(nDim, nPts, name, hdr);	// construct n-D real set
      break;
//	case 1:
//	    d = new CComplex(nCols, nPts, name, hdr); // construct n-D complex set
//	    break;
    default:
      d = NULL;
  }

  if (d)
    {
      vector<double*>::iterator i;
      double* pData;
      int nRem, nCopy, js;

      if (nCols > 1)
	{
	  nRem = nPts;
	  js = 0;
	  i = LoadBuffers.begin();

	  while (nRem > 0)
	    {
	      pData = *i;
	      if (nRem > MAX_POINTS) nCopy = MAX_POINTS; else nCopy = nRem;
	      d->CopyData(pData, js, js + nCopy - 1);
	      nRem -= nCopy;
	      js += nCopy;
	      ++i;
	    }
	}
      else
	{
	  // Generate running index x for one column data

	  double* x = new double [nPts];
	  if (!x) return NULL;

	  int j = 0;
	  for (j = 0; j < nPts; j++) x[j] = j + 1;

	  nRem = nPts;
	  js = 0;
	  i = LoadBuffers.begin();

	  while (nRem > 0)
	    {
	      pData = *i;
	      if (nRem > MAX_POINTS) nCopy = MAX_POINTS; else nCopy = nRem;
	      d->CopyFromXY (x + js, pData, js, js + nCopy - 1);
	      nRem -= nCopy;
	      js += nCopy;
	      ++i;
	    }
	  delete [] x;
	}

      d->SetExtrema();
    }

    return d;
}
//---------------------------------------------------------------

CDataset* CDatabase::MakeDataset (CWorkspace41* ws, int n)
{
// Create corresponding data set for set n in
//   a workspace

    int jsr = ws->ds[n].jsr - 1;
    int npts = ws->ds[n].npts;
    char* name = ws->ds[n].fname;
    char* hdr = ws->ds[n].hdr;
    CDataset* d;

    switch (ws->ds[n].data_type) {
      case 0:
        d = new CReal (2, npts, name, hdr);
        break;
      case 1:
        d = new CComplex (2, npts, name, hdr);
        break;
      default:
        d = NULL;
    }

    if (d) {
      d->CopyFromXY (&ws->x[jsr], &ws->y[jsr], npts);
      d->SetExtrema();
    }

    return d;
}
//---------------------------------------------------------------

CDataset* CDatabase::MakeDataset (int* ds_info)
{
// Make a dataset from a dataset info structure (used by Forth)

    char* name = *((char**) ds_info);
    char* hdr = *((char**)(ds_info + 1));
    int ntype = *(ds_info + 2);
    int npts = *(ds_info + 3);
    int ncols = *(ds_info + 4);
    void* data = *((void**)(ds_info + 5));

    int i, nelem = npts*ncols;

    int data_type = ntype & 255;      // data_type can be real or complex
    int data_precision = ntype/256;   // precision can be single or double 

    double* temp_buf = new double [nelem];
    double* temp;
    if (temp_buf) temp = temp_buf; else  return NULL;

    vector<double*> LoadBuffers;
    double* pD;
    float*  pF;

    switch (data_precision) {
      case 0:
        // Single precision floating point n-D real dataset
        pF = (float*) data;
        for (i = 0; i < nelem; i++) {
          *temp = (double) (*pF);
          temp++; pF++;
        }
        break;
      case 1:
        // Double precision floating point n-D real dataset
        pD = (double*) data;
        for (i = 0; i < nelem; i++) *temp++ = *pD++;
        break;
      default:
        temp_buf = NULL;
    }

    temp = temp_buf;
    int nRem = npts;
    do {
      LoadBuffers.push_back(temp);
      nRem -= MAX_POINTS;
      temp += MAX_POINTS*ncols;
    }
    while (nRem > 0) ;

    CDataset* d = MakeDataset(LoadBuffers, npts, ncols, data_type, name, hdr);
    if (d) AddSet(d);

    return d;
}
//---------------------------------------------------------------

int CDatabase::SaveDataset (CDataset* d, vector<int> lim, char* name)
{
    CXyFile f (name, 1);
    f.SetSaveOptions (m_nSave);

    if (d)
    {
        f.WriteHeader(d->m_szHeader);
        int i1 = lim[0];
        int i2 = lim[1];
        int ncols = d->SizeOfDatum();
        double* p = (double*) &(*(d->begin())) + i1*ncols;
        f.WriteData(p, i2-i1+1, ncols);
    }
    else
    {
        return 0;
    }

    return 1;
}
//---------------------------------------------------------------

int CDatabase::Write (ofstream& ofile)
{
  // Write database info in binary form to the output file stream

  int intsize = sizeof(int);
  int nsets = Nsets();
  ofile.write ((char*) &nsets, intsize);
  
  // Write the datasets

  CDataset* d;
  int i, j;
  int set_type = 0;  // at present the set type is not defined
  int nLength, nPoints;

  for (i = 0; i < nsets; i++)
    {
      d = m_nSets[i];
      ofile.write ((char*) d->m_szName, NAME_LENGTH);
      ofile.write ((char*) d->m_szHeader, HEADER_LENGTH);
      ofile.write ((char*) &set_type, intsize);
      nPoints = d->NumberOfPoints();
      ofile.write ((char*) &nPoints, intsize);
      nLength = d->SizeOfDatum();
      ofile.write ((char*) &nLength, intsize);
    }

  return 0;
}







