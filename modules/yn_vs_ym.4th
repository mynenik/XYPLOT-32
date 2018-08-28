\ yn_vs_ym.4th
\
\ xyplot module for making an x,y dataset from the
\   y values of one set versus the y values from another set.
\
\ Copyright (c) 2001--2012 K. Myneni
\
\ This software is released under the terms of the GNU
\ Lesser General Public License (LGPL)
\ 
\ Revisions: 
\     01-14-2005  km  updated use of DatasetInfo structure
\     08-18-2007  km  updated to use FSL arrays
\     2009-10-29  km  updated data structure member names
\     2012-06-26  km  convert to unnamed module
\
\ Requires:
\
\	xyplot.4th
\	xutils.4th

Begin-Module

DatasetInfo ds1
DatasetInfo ds2
DatasetInfo ds3
FLOAT DMATRIX  ynm{{

0 value Npts

Public:

: yn_vs_ym ( -- )
    ?active dup 0< IF  drop EXIT  THEN
    ds1 get_ds		\ get active set info
    0 >= IF
	?operand dup 0< IF  drop EXIT  THEN
	ds2 get_ds	\ get operand set info
	0 >= IF
	    ds1 DatasetInfo->Npts @  ds2 DatasetInfo->Npts @  min to Npts
	    & ynm{{ Npts 2 }}malloc
	    
	    \ Create the new x, y pairs in the matrix ynm

	    Npts 0 DO
		I ds1 @xy  ynm{{ I 0 }} F! 
		I ds2 @xy  ynm{{ I 1 }} F!
		fdrop  fdrop
	    LOOP

	    \ Make the new dataset in xyplot

	    c" Yn_vs_Ym" 1+ ds3  DatasetInfo->Name !
	    c"  " 1+        ds3  DatasetInfo->Header !
	    256             ds3  DatasetInfo->Type !
	    Npts            ds3  DatasetInfo->Npts !
	    2               ds3  DatasetInfo->Size !
	    ynm{{           ds3  DatasetInfo->Data !

	    ds3 make_ds drop
		
	    & ynm{{ }}free
	THEN
    THEN	  
;


\ add xyplot math menu item

MN_MATH c" Y_m vs Y_n"	c" yn_vs_ym" add_menu_item

End-Module


