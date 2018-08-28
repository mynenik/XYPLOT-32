\ xyderiv.4th
\
\ xyplot module for derivative computation
\
\ Copyright (c) 2001--2012 Krishna Myneni, Creative Consulting
\   for Research and Education.
\
\ Provided under the GNU Lesser General Public License (LGPL)
\
\ Requires:
\	derivative.4th
\
\ Revisions:
\       2001        created
\       2005-01-14  updated for new DatasetInfo data structure  km
\       2007-06-02  updated to use FSL-style arrays  km
\       2009-10-28  updated data structure member names  km
\       2010-01-27  automatically generate and draw new plot of derivative  km
\       2012-06-26  convert to unnamed module  km

Begin-Module

DatasetInfo ds1
DatasetInfo dsder
PlotInfo    plder

FLOAT DMATRIX deriv{{
0 value np

Public:

: xyderiv ( -- )
    ?active dup 0 >= IF
	ds1 get_ds 0 >= IF
	    ds1 DatasetInfo->Npts @ to np
	    & deriv{{ np 2 }}malloc
	    malloc-fail? IF ." Unable to alloc derivative matrix." cr EXIT THEN
	    
	    \ Copy dataset x,y values to the derivative matrix

	    np 0 DO
		I ds1 @xy  deriv{{ I 1 }} F!  deriv{{ I 0 }} F!
	    LOOP

	    deriv{{  np derivative  IF
	      ." Error computing derivative" EXIT
	    THEN

	    \ Make a new dataset

	    c" Derivative" 1+ dsder DatasetInfo->Name !
	    c" Derivative" 1+ dsder DatasetInfo->Header !
	    REAL_DOUBLE dsder DatasetInfo->Type !	\ double precision fp type
	    np  dsder DatasetInfo->Npts !
	    2   dsder DatasetInfo->Size !
	    deriv{{ dsder DatasetInfo->Data !

            get_active_plot plder get_plot drop

            \ Make a new plot if dataset creation succeeds
	    dsder make_ds dup 0> IF
 	      plder PlotInfo->Set !
	      0 plder PlotInfo->Type !
	      sym_LINE plder PlotInfo->Symbol !
              plder PlotInfo->Color @ 1+ plder PlotInfo->Color !
	      plder make_plot
	    ELSE
	      drop  ." Error creating derivative plot" 
	    THEN
	    & deriv{{ }}free
	THEN
    ELSE  drop  THEN ;
	       
MN_MATH c" Derivative"	c" xyderiv reset_window" add_menu_item

End-Module

