/*
CPoint.h

A simple point class to mimic the MFC class
*/

#ifndef __CPOINT_H__
#define __CPOINT_H__

class CPoint
{
public:
	int x;
	int y;
	CPoint () { x = 0; y = 0;}
	CPoint (int ix, int iy) {x = ix; y = iy;}
	CPoint& operator=(const CPoint& p) {x = p.x; y = p.y; return *this;}
};

#endif
	
