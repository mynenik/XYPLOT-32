\ xysort.4th
\
\ Sort an x, y dataset by ascending x
\
\ Based on the FSL routine }SHELLSORT
\
\ K. Myneni, 2013-09-19

Begin-Module

DatasetInfo ds1

FLOAT DARRAY xs{
FLOAT DARRAY ys{

0 value np

0 ptr xss{
0 ptr yss{

\ Swap the floating point values at addresses a1 and a2
: swap_floats ( a1 a2 -- )
    2>r  r@ F@
    2r@ drop F@ r@ F!
    2r@ drop F!
    2r> 2drop ; 
  

Public:

: shellsort_xy ( np 'x 'y -- )
    TO yss{  TO xss{
    DUP
    BEGIN       ( nsize mspacing )
      2/ DUP    
    WHILE       ( n m )
      2DUP - 0
      DO        ( n m )
       0 I DO   ( n m )

\ compare Ith and (I+M)th elements 
              dup I +             ( n m l )
              xss{ over } F@        
              xss{ I }    F@      ( n m l Xl Xi )
              F>                \ reverse this to get descending sort

\ switch them if necessary
              IF  DROP LEAVE            ( n m )
              THEN                      ( n m l )
	      xss{ over } xss{ I } swap_floats
	      yss{ over } yss{ I } swap_floats
	      drop
              dup negate                ( n m -m )
            +LOOP
      LOOP
    REPEAT   2DROP
;


: xysort    ( -- )
    ?active dup 0 >= IF
	ds1 get_ds 0 >= IF
	    ds1 DatasetInfo->Npts @ to np
	    & xs{ np }malloc
	    & ys{ np }malloc
	    malloc-fail? IF ." Unable to allocate arrays." cr EXIT THEN
	    \ Copy dataset x,y values to the arrays
	    np 0 DO
	      I ds1 @xy  ys{ I } F!  xs{ I } F!
	    LOOP
	    \ Sort the arrays
            np xs{ ys{ shellsort_xy
	    \ Copy array data back to dataset
	    np 0 DO
	      xs{ I } F@ ys{ I } F@ I ds1 !xy
	    LOOP
	    \ Cleanup
	    & xs{ }free
	    & ys{ }free
        THEN
    ELSE  drop  THEN
;

MN_MATH c" Ascend Sort by X"	c" xysort reset_window" add_menu_item


End-Module

