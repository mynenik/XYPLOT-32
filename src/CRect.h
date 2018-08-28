/*
CRect.h

A simple rectangle class
*/

#ifndef __CRECT_H__
#define __CRECT_H__

#include "CPoint.h"

class CRect
{
  CPoint p1;
  CPoint p2;
public:
	CRect () {p1.x = 0; p2.x = 0; p1.y = 0; p2.y = 0;}
	CRect (int ix1, int iy1, int ix2, int iy2) {p1.x = ix1; p1.y = iy1; p2.x = ix2; p2.y = iy2;}
	CRect (CPoint pt1, CPoint pt2) {p1 = pt1; p2 = pt2;}
	CRect& operator= (const CRect& r) {p1 = r.p1; p2 = r.p2; return *this;}
	int Width() { return (p2.x - p1.x);};
	int Height() {return (p2.y - p1.y);}
	CPoint& TopLeft() {return p1;}
	CPoint& BottomRight() {return p2;}
	bool PtInRect (CPoint p) { return ((p.x >= p1.x) && (p.x <= p2.x) &&
		(p.y >= p1.y) && (p.y <= p2.y));}
	void SetRect (int ix1, int iy1, int ix2, int iy2) {p1.x = ix1; p1.y = iy1; p2.x = ix2; p2.y = iy2;}
};

#endif
