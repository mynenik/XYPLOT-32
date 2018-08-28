\ xypolyfit.4th   
\                                                               
\  Polynomial fitting routine for xyplot                          
\  
\ Adapted from the routine polfit in P.R. Bevington,
\ "Data Reduction and Error Analysis for the Physical Sciences"
\
\ Requires:
\       fsl/extras/polyfit.4th
\
\ Revisions:
\              
\	1998-10-02  Adapted for LabForth KM             
\	1999-03-29  Adapted for kForth KM  
\	2000-03-15  Adapted for xyplot KM
\	2000-04-15  Added user query for order KM
\ 	2000-12-12  Indexing mods and use of fmatrix  KM
\       2005-01-13  use DatasetInfo structure creation word and update usage  KM
\       2007-06-02  use FSL arrays and matrices  KM
\       2007-06-05  use FSL modules lufact and dets for computing determinants  KM
\       2007-06-12  moved polfit to polfit2 in polyfit.4th; factored common
\                     code with array argument version of polfit KM
\       2009-10-29  updated data structure field names KM
\       2012-06-26  converted to unnamed module  km

Begin-Module

DatasetInfo  ds_pdata 
DatasetInfo  ds_pfit 
PlotInfo     pl_pfit

FLOAT DARRAY  poly_params{

0 value Norder

Public:

: xypoly ( n -- | fit a polynomial of order n to the active dataset )
    to Norder
    ?active dup 0 >= IF
	ds_pdata get_ds  0 >= IF
	    & poly_params{ Norder 1+ }malloc 
	    ds_pdata poly_params{ Norder polfit2
	    ." Reduced chi-square = " f. cr 
	    Nterms 0 DO  I . 2 spaces poly_params{ I } F@ f. cr  LOOP
	    
	    ds_pdata DatasetInfo->Npts @ 100 > IF
		  \ Compute fitted curve at same abscissas as data
	    ELSE
		  \ Computed fitted curve on a grid of 100 points
	    THEN

	    & poly_params{ }free
	THEN
    THEN ;


: xypolyfit ( -- | prompt user and fit the active data set )
    ?active 0 >= IF
	c" Enter the order of the fitting polynomial:" get_input
	IF  string>s xypoly  ELSE  drop  THEN
    THEN ;

\ add "Poly Fit" as an item in the math menu

MN_MATH c" Poly Fit" c" xypolyfit draw_window" add_menu_item

End-Module


