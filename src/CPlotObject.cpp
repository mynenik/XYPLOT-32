// CPlotObject.cpp
//
// Copyright 1995--2018 Krishna Myneni
// <krishna.myneni@ccreweb.org>
//
// This software is provided under the terms of the
// GNU General Public License (GPL), v3.0 or later.

#include "CPlotObject.h"

CTransform* CPlotObject::m_pT = NULL;

//---------------------------------------------------------------

CPlotObject::CPlotObject()
{
	m_nStyle = PS_SOLID;
	m_nWidth = 1;
	m_nColor = 0;
}
//---------------------------------------------------------------

void CPlotObject::SetAttributes (int PenStyle, int Width, COLORREF Color)
{
	m_nStyle = PenStyle;
	m_nWidth = Width;
	m_nColor = Color;
}
