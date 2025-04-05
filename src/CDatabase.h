// CDatabase.h
//
// class CDatabase
//
// Copyright 1995-2023 Krishna Myneni
// <krishna.myneni@ccreweb.org>
//
// This software is provided under the terms of the 
// GNU General Public License (GPL), v3.0 or later.
//

#ifndef __CDATABASE_H__
#define __CDATABASE_H__

#include "CWorkspace41.h"
#include "CDataset.h"
#include "CXyFile.h"
#include "saveopts.h"
#include <fstream>
#include <vector>
using std::vector;

#define byte unsigned char

class CDatabase
{
  int m_bMem;
  vector<CDataset*> m_nSets;
  SaveOptions m_nSave;
public:
  CDatabase ();
  ~CDatabase (void);
  int Nsets() {return m_nSets.size();}
  void AddSet (CDataset* d) {m_nSets.push_back(d);}
  void RemoveSet (CDataset*);
  int IndexOf (CDataset*);
  CDataset* FindInList (char*);
  int MakeListOfNames (char**);
  void Clear();           // clear the database
  CDataset* MakeDataset (vector<double*>, int, int, int, char*, char*);
  CDataset* MakeDataset (CWorkspace41*, int);
  CDataset* MakeDataset (int*);
  int GetColumnCount (char*);
  int LoadDataset (CDataset**, char*, vector<int>, int);
  int LoadWorkspace (CWorkspace41**, char*);
  int SaveDataset (CDataset*, vector<int>, char*);
  SaveOptions GetSaveOptions () {return m_nSave;}
  void SetSaveOptions (SaveOptions s) {m_nSave = s;}
  CDataset* operator[] (const int i) {return m_nSets[i];}
  int Write (ofstream&);
};

#endif


