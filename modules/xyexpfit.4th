\ xyexpfit.4th
\
\ Fit data to the function:
\
\	y = a*exp(-x/b) + c
\
\ Parameter array ordering is:
\
\	0   a
\	1   b
\	2   c
\
\ Requires:
\   fsl/extras/curvefit.4th
\

Begin-Module

: noname ( fx a -- fy )
    >r 
    r@ 1 } f@ f/ fnegate fexp
    r@ 0 } f@ f* 
    r> 2 } f@ f+ ;
' noname IS functn

MAX_POINTS FLOAT array x{
MAX_POINTS FLOAT array y{
MAX_POINTS 2 FLOAT MATRIX fit_data{{

3 FLOAT array params{
3 FLOAT array increments{
fvariable chi-sqr
variable npts

Public:

\ Make a fit to the active plot's data set to
\ the function and return a success (TRUE or FALSE) flag
: expfit ( -- flag )
    ?active dup 0 >= 
    IF
      ds1 get_ds
      0 >= IF
        ds1 DatasetInfo->Npts @ dup npts !
        MAX_POINTS > IF
          ." Too many points in data set. Resize MAX_POINTS." cr
          EXIT
        THEN
       THEN

       npts @ 0 ?DO
         I ds1 @xy  y{ I } f!  x{ I } f!
       LOOP

       5 0 DO
         x{ y{ params{ increments{ 3 npts @ curfit chi-sqr f!
       LOOP
       true
     ELSE false
     THEN ;


\ Make a dataset with the fit parameters;
\ Return set number of the fit dataset.
: make-expfit-ds ( -- n )
    npts @ 0 ?DO
      x{ I }    f@  fit_data{{ I 0 }} f!
      yfit{ I } f@  fit_data{{ I 1 }} f!
    LOOP
	    
    c"  " 1+       ds2 DatasetInfo->Header !
    c" Exp Fit" 1+ ds2 DatasetInfo->Name   !
    REAL_DOUBLE    ds2 DatasetInfo->Type   !
    npts @         ds2 DatasetInfo->Npts   !
    2              ds2 DatasetInfo->Size   !
    fit_data{{     ds2 DatasetInfo->Data   !

    ds2 make_ds ;

PlotInfo pl3

: make-expfit-plot ( n -- | make plot of result using new dataset number)
    pl3    PlotInfo->Set !
    0 pl3  PlotInfo->Type !
    sym_LINE pl3 PlotInfo->Symbol !
    2 pl3  PlotInfo->Color !

    pl3 make_plot
;

: .expfit-params ( -- )
    ." Reduced Chi^2 = " chi-sqr f@ f. cr
    ." a = " params{ 0 } f@ f. cr
    ." b = " params{ 1 } f@ f. cr
    ." c = " params{ 2 } f@ f. cr ;

: xyexpfit ( -- )
    ?active 0 >= 
    if
      c" Enter initial a, b, c for y=a*exp(-x/b) + c:" get_input
      if
        count parse_csv 3 <> abort" Entered wrong number of parameters!"
        params{ 2 } f!  params{ 1 } f!  params{ 0 } f!
        c" Enter initial increments da, db, dc:" get_input
        if
         count parse_csv 3 <> abort" Entered wrong number of params!"
         increments{ 2 } f!  increments{ 1 } f!  increments{ 0 } f!
        else
          exit
        then
        ." Initial parameters and increments:" cr
        3 0 do params{ I } f@ f. 2 spaces increments{ I } f@ f. cr loop
        expfit if
          .expfit-params
          make-expfit-ds
          make-expfit-plot
          reset_window
        then
      else
        drop
      then
    then ;

MN_MATH c" Exponential Fit"  c" xyexpfit" add_menu_item

End-Module

