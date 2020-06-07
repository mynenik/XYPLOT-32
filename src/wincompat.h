/*
wincompat.h

Windows MFC compatibility definitions

*/

#ifndef __WINCOMPAT_H__
#define __WINCOMPAT_H__

#define CDC CDeviceContext
#define BOOL bool
#define COLORREF unsigned int
#define BYTE unsigned char
#define CRgn void

#define RGB(r, g, b) ( ((b)<<16) | ((g)<<8) | (r) )

#define TRUE -1
#define FALSE 0
#define IDCANCEL -1
#endif
