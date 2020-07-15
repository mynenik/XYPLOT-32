/*
CPlotMessage.h

  Copyright (c) 1990-2020 Krishna Myneni
  <krishna.myneni@ccreweb.org>

  This software is provided under the terms of the 
  GNU Affero General Public License (GPL), v 3.0 or later.

*/


#ifndef __CPLOTMESSAGE_H__
#define __CPLOTMESSAGE_H__

#define PL_NONE		0
#define PL_ERR		1
#define PL_EXIT		2
#define PL_RESET	3
#define PL_HELP		4
#define PL_MENU		5

#define PL_CURSORMOVE	16
#define PL_BUTTONPRESS	17
#define PL_KEYPRESS	18
#define PL_MOUSETOGGLE	19
#define PL_DATACURSOR	20
#define PL_RESIZE	21
#define PL_REDRAW	22

#define PL_NEW		32
#define PL_OPEN		33
#define PL_FILE_OPEN    34
#define PL_FILE_CANCEL  35
#define PL_FILE_LOAD    36
#define PL_LOAD_CANCEL  37
#define PL_SAVE	        38
#define PL_FILE_SAVE	39
#define PL_SAVE_CANCEL	40
#define PL_SAVETEX	41
#define PL_SAVEPS	42
#define PL_SAVE_OPTIONS	43
#define PL_SAVE_OPTIONS_HEADER  44
#define PL_SAVE_OPTIONS_CANCEL 45
#define PL_SAVE_OPTIONS_DONE   46
#define PL_VIEWFILE	47
#define PL_HEADER	48
#define PL_HEADER_OK    49
#define PL_HEADER_CANCEL 50
#define PL_PRINT        51


#define PL_BASE		64
#define PL_EXTREMA	65
#define PL_EXTREMA_INPUT 66
#define PL_EXTREMA_CANCEL 67
#define PL_ESCAPE	68

#define PL_DROP		80
#define PL_PICK		81
#define PL_PICK_DATA    82
#define PL_ASPECT	83
#define PL_SCALE	84

#define PL_PALETTE	96
#define PL_GRID		97
#define PL_GRID_LINES   98
#define PL_POINT	99
#define PL_LINE		100
#define PL_TOGGLESYMBOL 101
#define PL_SYMBOL       102
#define PL_SET_SYMBOL   103
#define PL_COLOR        104
#define PL_SET_COLOR    105
#define PL_GRID_AXES    106
#define PL_SET_GRID_TICS 107

#define PL_DELETE	128
#define PL_DUPLICATE	129
#define PL_JOIN		130
#define PL_PRUNE	131

#define PL_ZERO		160
#define PL_AREA		161
#define PL_DERIV	162
#define PL_INTEGRATE	163
#define PL_BIN		164
#define PL_POLY		165
#define PL_SORT		166
#define PL_SWAPXY	167
#define PL_MAG		168
#define PL_PHASE	169
#define PL_CONJ		170
#define PL_REALY	171
#define PL_IMAGY	172
#define PL_CMPLY	173
#define PL_EXPRESSION	174
#define PL_EXPRESSION_INPUT 175
#define PL_EXPRESSION_CANCEL 176
#define PL_ARITHMETIC	177
#define PL_SMOOTH	178
#define PL_FFT		179
#define PL_TEMPLATE	180


class CPlotMessage {
public:
	int wid;	// window identifier
	int x;		// pointer x
	int y;		// pointer y 
	int message;	// message id
	int nParam;	// number of parameters
	int dLen;	// buffer length in bytes
	void* pBuf;	// ptr to parameter buffer
	CPlotMessage () {pBuf = NULL; nParam = 0; dLen = 0;}
	~CPlotMessage () { ;}
};

#endif
