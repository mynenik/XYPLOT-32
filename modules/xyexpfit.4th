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

: Exp-func ( fx a -- fy )
    >r 
    r@ 1 } f@ f/ fnegate fexp
    r@ 0 } f@ f* 
    r> 2 } f@ f+ ;

FLOAT DARRAY x{
FLOAT DARRAY y{
FLOAT DARRAY yfit{
FLOAT DMATRIX fit_data{{

3 FLOAT array params{
3 FLOAT array increments{
3 FLOAT array sigma_par{

fvariable chi-sqr
variable npts

: alloc-mem ( u -- bfail )
    & x{ over }malloc
    & y{ over }malloc
    & yfit{ over }malloc
    & fit_data{{ over 2 }}malloc
    drop
    malloc-fail?
;

: free-mem ( -- )
    & fit_data{{ }}free
    & yfit{ }free
    & y{ }free
    & x{ }free
;

Public:

\ Make a fit to the active plot's data set to
\ the function and return a success (TRUE or FALSE) flag
: expfit ( -- flag )
    ?active dup 0 >= 
    IF
      ds1 get_ds
      0 >= IF
        ds1 DatasetInfo->Npts @ npts !
        npts @ alloc-mem IF
          ." Too many points. Unable to allocate memory!" cr
          false EXIT
        THEN
      THEN

      npts @ 0 ?DO
        I ds1 @xy  y{ I } f!  x{ I } f!
      LOOP

      x{ y{ yfit{ params{ increments{ sigma_par{ 3 npts @ ['] Exp-Func init-curvefit 

      5 0 DO  curfit chi-sqr f!  LOOP
      true
    ELSE drop false
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
    chi-sqr f@ fsqrt  \ approximate sigma(y_i's) as sqrt(chi-sqr)
    ." Fit to y = a*exp(-x/b) + c" cr
    ." a = " params{ 0 } f@ f$. ."  +/- " sigma_par{ 0 } f@ fover f* f$. cr
    ." b = " params{ 1 } f@ f$. ."  +/- " sigma_par{ 1 } f@ fover f* f$. cr
    ." c = " params{ 2 } f@ f$. ."  +/- " sigma_par{ 2 } f@ fover f* f$. cr
    fdrop
    ." Reduced Chi^2 = " chi-sqr f@ f$. cr ;

: xyexpfit ( -- )
    ?active 0 >= 
    IF
      c" Enter initial a, b, c for y=a*exp(-x/b) + c:" get_input
      IF
        count parse_csv 3 <> abort" Entered wrong number of parameters!"
        params{ 2 } f!  params{ 1 } f!  params{ 0 } f!
        c" Enter initial increments da, db, dc:" get_input
        IF
         count parse_csv 3 <> abort" Entered wrong number of params!"
         increments{ 2 } f!  increments{ 1 } f!  increments{ 0 } f!
        ELSE
          EXIT
        THEN
        ." Initial parameters and increments:" cr
        3 0 do params{ I } f@ f$. 2 spaces increments{ I } f@ f$. cr loop
        expfit IF
          .expfit-params
          make-expfit-ds
          free-mem
          make-expfit-plot
          reset_window
        then
      else
        drop
      then
    then ;

MN_MATH c" Exponential Fit"  c" xyexpfit" add_menu_item

End-Module

