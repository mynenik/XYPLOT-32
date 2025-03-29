// CTransform.h
//
// Header file for class CTransform
//
// Copyright 1995--2023 Krishna Myneni
// <krishna.myneni@ccreweb.org>
//
// This software is provided under the terms of the
// GNU General Public License (GPL), v3.0 or later.
//
// Base class for coordinate transformations and some
//   transformation classes

#ifndef __CTRANSFORM_H__
#define __CTRANSFORM_H__

#include <vector>
using std::vector;

#include "CPoint.h"
#include "CRect.h"

class CTransform {
protected:
    CRect m_nPRect;             // rectangle for physical area
    vector<double> m_vEx;       // extrema
    vector<double> m_vDelta;
public:
	void SetPhysical (CRect rect) {m_nPRect = rect;}
	CRect GetPhysical (void) {return m_nPRect;}
	void SetLogical (vector<double>);
	vector<double> GetLogical () {return m_vEx;}
	virtual CPoint Physical (double*) = 0;
	// CPoint Physical (double*);
	CPoint Physical (vector<double> x) {return Physical( (double*) &x[0]);}
	virtual void Logical (CPoint, double*) = 0;
	vector<double> Logical (CPoint);
	vector<double> Logical (CRect);
};

class C2D_Transform : public CTransform {
public:
    CPoint Physical (double*);
    void Logical (CPoint, double*);
};

class C2Di_Transform : public CTransform {
 public:
  CPoint Physical (double*);
  void Logical (CPoint, double*);
};

class C2D_PolarTransform : public C2D_Transform {
public:
	CPoint Physical (double*);
	void Logical (CPoint, double*);
};

#endif




