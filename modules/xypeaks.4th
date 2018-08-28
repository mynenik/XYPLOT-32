\ xypeaks.4th
\
\ xyplot module for automated peak finding. The active dataset should be 
\   smoothed or filtered sufficiently to remove high frequency noise prior 
\   to calling the peak search function. Note that if you use low-pass
\   filtering on the data, it will shift the horizontal positions of the peaks.
\
\ Copyright (c) 2001--2012 Krishna Myneni
\ Provided under the GNU Lesser General Public License (LGPL)
\
\ Requires:
\	derivative.4th
\	polyfit.4th
\	peak_search.4th
\	xyplot.4th
\
\ Revisions:
\	3-30-2001  created
\	1-14-2005  updated use of DatasetInfo structure  km
\       9-15-2007  updated to use FSL style arrays  km
\       9-28-2007  revised to use new global constants for dataset type
\                    and plotting symbols; also set plot color for peaks plot  km
\       2009-10-29  updated data structure member names  km
\       2012-06-26  converted to unnamed module  km

Begin-Module

5 constant DEF_PEAK_WIDTH

DatasetInfo ds1
DatasetInfo dspks
PlotInfo    plpks

FLOAT DMATRIX input_data{{

: pks_free ( -- | free dynamic matrices)
    & pksin{{ }}free
    & input_data{{ }}free
;

Public:

: xypeaks ( -- )
    ?active dup 0< IF drop EXIT THEN
    ds1 get_ds  0< IF ." No valid dataset." EXIT THEN
    	    
    \ Copy dataset x,y values to the input_data matrix

    & input_data{{ ds1 DatasetInfo->Npts @ 2 }}malloc

    ds1 DatasetInfo->Npts @ 0 DO
	I ds1 @xy
	input_data{{ I 1 }} F!  input_data{{ I 0 }} F! 
    LOOP

    c" Enter the minimum peak width:" get_input
    IF  string>s  ELSE  DEF_PEAK_WIDTH  THEN

    input_data{{ ds1 DatasetInfo->Npts @ rot peak_search

    CASE
	1 OF  ." Error computing derivative of dataset!" pks_free EXIT  ENDOF
	2 OF  ." No peaks found!"  pks_free EXIT ENDOF
    ENDCASE

    \ Make a new dataset and a stick plot of the peaks

    \ Get info of current plot so we can draw the peaks plot in next color
    get_active_plot plpks  get_plot  drop  
    
    Npeaks 0> IF
	c" Peaks" 1+ dspks DatasetInfo->Name !
	c" Peaks" 1+ dspks DatasetInfo->Header !
	REAL_DOUBLE  dspks DatasetInfo->Type !
	Npeaks   dspks DatasetInfo->Npts !
	2        dspks DatasetInfo->Size !
	peaks{{  dspks DatasetInfo->Data !

	dspks make_ds
	  plpks PlotInfo->Set !
	0 plpks PlotInfo->Type !	\ plot type
	sym_STICK plpks PlotInfo->Symbol  ! \ plot symbol
	plpks PlotInfo->Color @ 1+ plpks PlotInfo->Color !
	plpks make_plot
	
    THEN

    pks_free
;
	       
MN_MATH c" Find Peaks"	c" xypeaks reset_window" add_menu_item

End-Module


