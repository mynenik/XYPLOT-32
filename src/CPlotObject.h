// CPlotObject.h
//
// Header file for class CPlotObject
//
// Copyright (c) 1995-2020 Krishna Myneni
// <krishna.myneni@ccreweb.org>
//
// This software is provided under the terms of the
// GNU Affero General Public License (AGPL), v 3.0 or later.

#ifndef __CPLOTOBJECT_H__
#define __CPLOTOBJECT_H__

#include "CTransform.h"
#include "CxDC.h"
#include "wincompat.h"

#define PS_SOLID 0
#define PS_DOT 1

// abstract base class for plotting objects

class CPlotObject
{
protected:
	int m_nStyle;
	int m_nWidth;
	unsigned m_nColor;
	static CTransform* m_pT;      // transform shared by all plot objects
public:
	CPlotObject (void);
	void SetTransform (CTransform* pCt) {m_pT = pCt;}
	void SetAttributes (int, int, unsigned);
	void SetColor (unsigned cr) { m_nColor = cr;}
	unsigned GetColor (void) {return m_nColor;}
	int GetLineStyle () {return m_nStyle;}
	int GetLineWidth () {return m_nWidth;}
	virtual void Draw (CDeviceContext*) = 0;
//	virtual void Write ();
};


#endif
