\ xyhistogram.4th
\
\ Bin series of y values from the active dataset to create a histogram
\
\ Copyright (c) 2001--2012 Krishna Myneni
\ This software is provided under the terms of the 
\ GNU Lesser General Public License (LGPL)
\
\ Revisions:
\	2001-07-27  modified histogram.4th for use with xyplot KM
\	2001-09-24  modified find_min_max to take argument  KM
\       2005-01-13  changed F>S to FROUND>S; updated use of DatasetInfo structure  KM
\       2007-06-02  use FSL arrays and matrices now. KM
\       2009-10-29  updated data structure member names.  KM
\       2011-02-12  automatically replace plot of data with its histogram  KM
\       2012-06-26  convert to unnamed module  KM

Begin-Module

FLOAT DMATRIX hist{{

fvariable hmax 0e hmax f!
fvariable hmin 0e hmin f!

0 ptr dsaddr

: find_min_max ( addr -- | determine min and max of y values of dataset)
    to dsaddr
    0 dsaddr @xy fdup hmin F! hmax F! fdrop
    dsaddr DatasetInfo->Npts @ 1 DO
	I dsaddr @xy   
	fdup hmin F@ F< IF  fdup hmin F!  THEN
	fdup hmax F@ F> IF  fdup hmax F!  THEN
	fdrop fdrop
    LOOP ;

: ?bins ( fwidth -- n | compute number of bins required )
    \ hmin and hmax must be set with prior execution of find_min_max
    hmax F@ hmin F@ F- fover F/ fround>s 1+ ;

fvariable bin_width
0 value Nbins
DatasetInfo ds1
DatasetInfo dshist
PlotInfo    plhist

Public:
	
: histogram ( f -- | create a histogram of the active set's y values )
    \ f is the bin width for the histogram
    bin_width F!
    ?active dup 0 >= IF
	ds1 get_ds 0 >= IF
	    ds1 find_min_max	\ determine min and max of y values
	    bin_width F@ ?bins to Nbins  \ no. of histogram bins needed

	    & hist{{ Nbins 2 }}malloc
	    malloc-fail? IF ." Unable to allocate histogram matrix." cr EXIT  THEN

	    \ Set up abscissas and initial zero counts for histogram

	    bin_width F@ 2e F/ hmin F@ F+

	    Nbins 0 DO
		fdup hist{{ I 0 }} F!  0e hist{{ I 1 }} F!
		bin_width F@ F+
	    LOOP  fdrop
	      	
	    \ Create the histogram

	    ds1 DatasetInfo->Npts @ 0 DO
		I ds1 @xy  hmin F@ F-
		bin_width F@ F/ fround>s >r  \ bin index for current y val
		hist{{ r@ 1 }} F@ 1e F+ hist{{ r> 1 }} F!  \ increment bin count
		fdrop
	    LOOP

	    \ Create dataset in xyplot

	    c"  " 1+ dshist DatasetInfo->Header !
	    c" Histogram" 1+ dshist DatasetInfo->Name !
	    REAL_DOUBLE dshist DatasetInfo->Type !       \ double precision fp type
	    Nbins  dshist DatasetInfo->Npts !
	    2      dshist DatasetInfo->Size !
	    hist{{ dshist DatasetInfo->Data !

	    dshist make_ds

            \ Replace plot of data with histogram if dataset creation succeeds
	    dup 0> IF
	      get_active_plot plhist get_plot drop
 	      plhist PlotInfo->Set !
	      0 plhist PlotInfo->Type !
	      sym_HISTOGRAM plhist PlotInfo->Symbol !
	      drop_plot
	      plhist make_plot
	    ELSE
	      drop  ." Error creating histogram plot" 
	    THEN
	    & hist{{ }}free
	    
	THEN
    ELSE  drop  THEN ;
	  

: xyhistogram ( -- | prompt user to enter bin width and create histogram )
    ?active 0 >= IF
	c" Enter the bin width (for y values):" get_input
	IF  string>f histogram  ELSE  drop  THEN
    THEN ;


MN_MATH c" Histogram" c" xyhistogram reset_window" add_menu_item

End-Module

