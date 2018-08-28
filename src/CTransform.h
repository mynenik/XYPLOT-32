// CTransform.h
//
// Header file for class CTransform
//
// Copyright 1995--2018 Krishna Myneni
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
    vector<float> m_vEx;        // extrema
    vector<float> m_vDelta;
public:
	void SetPhysical (CRect rect) {m_nPRect = rect;}
	CRect GetPhysical (void) {return m_nPRect;}
	void SetLogical (vector<float>);
	vector<float> GetLogical () {return m_vEx;}
	virtual CPoint Physical (float*) = 0;
	CPoint Physical (double*);
	CPoint Physical (vector<float> x) {return Physical( (float*) &x[0]);}
	virtual void Logical (CPoint, float*) = 0;
	vector<float> Logical (CPoint);
	vector<float> Logical (CRect);
};

class C2D_Transform : public CTransform {
public:
    CPoint Physical (float*);
    void Logical (CPoint, float*);
};

class C2Di_Transform : public CTransform {
 public:
  CPoint Physical (float*);
  void Logical (CPoint, float*);
};

class C2D_PolarTransform : public C2D_Transform {
public:
	CPoint Physical (float*);
	void Logical (CPoint, float*);
};

#endif




