// CPlotObject.cpp
//
// Copyright 1995--2020 Krishna Myneni
// <krishna.myneni@ccreweb.org>
//
// This software is provided under the terms of the
// GNU Affero General Public License (AGPL), v 3.0 or later.

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

void CPlotObject::SetAttributes (int PenStyle, int Width, unsigned long Color)
{
	m_nStyle = PenStyle;
	m_nWidth = Width;
	m_nColor = Color;
}
