\ arithmetic.4th
\
\ xyplot module for dataset arithmetic functions
\
\ (c) 1999--2021 Krishna Myneni
\
\ This software is released under the terms of the GNU
\ Lesser General Public License (LGPL)
\ 
\ Requires:
\
\	xyplot.4th
\	xutils.4th

Begin-Module

DatasetInfo ds3

FLOAT DMATRIX result{{
0 value Npts

: make_result ( ^name  -- n | make dataset resulting from an arithmetic operation )
	
    \ Set up the info structure for a new dataset

    dup 1+      ds3  DatasetInfo->Name !
    count s"  of " strcat
    ds1 DatasetInfo->Name a@ dup strlen strcat
    s"  and " strcat
    ds2 DatasetInfo->Name a@ dup strlen strcat
    strpck 1+   ds3 DatasetInfo->Header !

    REAL_DOUBLE ds3  DatasetInfo->Type !
    Npts        ds3  DatasetInfo->Npts !
    2           ds3  DatasetInfo->Size !
    result{{    ds3  DatasetInfo->Data !
		 	      
    ds3 make_ds

    & result{{ }}free
;

PlotInfo pl3
: plot_result ( n -- | make plot of result using new dataset number)
    pl3  PlotInfo->Set !
    0 pl3 PlotInfo->Type !
    sym_LINE pl3 PlotInfo->Symbol !
    2 pl3 PlotInfo->Color !

    pl3 make_plot
;

: ?interpolate ( rx1 idx -- flag | return TRUE if interpolation is needed )
    ds2 @xy fdrop f<> ;

Private:

fvariable xs
fvariable x1
fvariable x2
fvariable y1
fvariable y2

Public:

fvariable slope
variable ordering2  \ true indicates ds2 is in ascending order; false otherwise
true ordering2 !

: interpolate_y2 ( fx1 idx -- fy2i | linear interpolation )
    >r xs f!
    r@ ds2 @xy y1 f! x1 f!

    r@ 0= IF
      r@ 1+
    ELSE
      r@ ds2 DatasetInfo->Npts @ 1- = IF
        r@ 1-
      ELSE
        r@
        ordering2 @ IF
          x1 f@ xs f@ f< IF  1+  ELSE  1-  THEN
        ELSE
          x1 f@ xs f@ f< IF  1-  ELSE  1+  THEN
        THEN
      THEN
    THEN
    ds2 @xy y2 f! x2 f!
    r> drop
    x1 f@ x2 f@ f> IF
      x1 f@ x2 f@  x1 f! x2 f!
      y1 f@ y2 f@  y1 f! y2 f!
    THEN
    y2 f@ y1 f@ f- x2 f@ x1 f@ f- f/ slope f!
    y2 f@ x2 f@ xs f@ f- slope f@ f* f-
;


0 value ridx
0 ptr   ar_operator 

: do_arithmetic ( xt -- flag | loop over index range n1 to n2 and apply operator )
    to ar_operator
    0 to ridx
    index_range
    2dup <>
    IF
	0 ds2 @xy fdrop  1 ds2 @xy fdrop f< ordering2 !
	swap
	2dup - to Npts
	& result{{ Npts 2 }}malloc      \ allocate the result matrix
	DO
	    I ds1 @xy
	    fover 
	    fdup ds2 findx    \ find index of nearest point in ds2
	    dup >r
	    ?interpolate 0= IF
	      r> ds2 @xy
	      fswap fdrop
	    ELSE
	      fover r> interpolate_y2
	    THEN
	    ar_operator execute
	    result{{ ridx 1 }} F!  result{{ ridx 0 }} F!
	    ridx 1+ to ridx
	LOOP  true
    ELSE
	2drop false
    THEN ;

Public:
 
: add_ds ( -- | add the operand set and the active set )
	['] f+ do_arithmetic  IF c" Sum" make_result plot_result THEN ;

: sub_ds ( -- | add the operand set and the active set )
	['] f- do_arithmetic  IF c" Difference" make_result plot_result THEN ;
     	        
: mul_ds ( -- | add the operand set and the active set )
	['] f* do_arithmetic  IF c" Product" make_result plot_result THEN ;
   
: div_ds ( -- | add the operand set and the active set )
	['] f/ do_arithmetic  IF c" Quotient" make_result plot_result THEN ;
   

\ add arithmetic functions to menu/submenu of Math Menu
[UNDEFINED] _WIN32_ [IF]
MN_MATH  c" Arithmetic"  make_submenu  CONSTANT  MN_ARITHMETIC
MN_ARITHMETIC
[ELSE]
MN_MATH
[THEN]

dup  c" Add" 		c" add_ds reset_window"  add_menu_item
dup  c" Subtract"	c" sub_ds reset_window"  add_menu_item
dup  c" Multiply"	c" mul_ds reset_window"  add_menu_item
dup  c" Divide"		c" div_ds reset_window"  add_menu_item
drop

End-Module


