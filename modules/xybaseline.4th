\ xybaseline.4th
\
\ Polynomial Baseline Subtraction
\
\ Copyright (c) 2005--2012 Krishna Myneni,
\ Creative Consulting for Research and Education
\
\ Provided under the GNU Lesser General Public License (LGPL)
\
\ Requires:
\   fsl-util
\   horner
\
\ Revisions:
\   2005-07-18  km
\   2009-10-29  km  updated for FSL arrays and data structure field names;
\                     use }Horner to compute polynomial
\   2012-06-26  km  convert to unnamed module

Begin-Module

DatasetInfo ds  \ active dataset info structure

8 constant MAX_BASELINE_PARAMS

MAX_BASELINE_PARAMS  FLOAT array baseline_params{
variable baseline_order

Public:

: sub-baseline ( -- | subtract baseline from active dataset )
    ?active DUP 0 >=  IF
	ds get_ds
	0 >= IF
	    ds DatasetInfo->Npts @ 0 DO
		I ds @xy
		fover baseline_params{ baseline_order @ }Horner F-
		I ds !xy
	    LOOP
	    ?active set_ds_extrema
	THEN
    ELSE  DROP  THEN 
;

: baseline ( -- | prompt user to enter baseline params )
    ?active 0 >=  IF
	c" Enter the polynomial coefficients of the baseline:" get_input
	IF
	    count parse_args dup 
	    IF dup MAX_BASELINE_PARAMS > ABORT" Too many parameters!"
		  1- dup baseline_order !
	          0 swap DO  baseline_params{ I } f! -1 +LOOP
		  sub-baseline
	    ELSE  drop  THEN  
	ELSE  drop  THEN
	THEN ;

\ add "baseline" as an item in the math menu

MN_MATH c" Baseline" c" baseline reset_window draw_window" add_menu_item

End-Module

