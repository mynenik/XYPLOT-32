\ Routines for trapezoidal integration of a real function or
\ complex function of a real variable.

Defer Integrand

1000000 constant NDIV
fvariable xfinal
fvariable xdel
fvariable rsum
fvariable rylast


\ Integrate a real integrand from x1 to x2 using
\ trapezoid rule

: trap_integrate ( F: x1 x2 -- f )
	0e rsum f!
	fover fover fswap f- NDIV s>f f/ xdel F!
	xfinal F!
	fdup Integrand rylast f!
	BEGIN
	  xdel f@ F+
	  fdup Integrand 
	  fdup rylast f@ f+ 
	  rsum f@ f+ rsum f!
	  rylast f!
	  fdup xfinal f@ 
	  xdel f@ f0> IF f>= ELSE f<= THEN
	UNTIL
	fdrop
	rsum f@
	0.5e f* xdel f@ f* 
;

zvariable zsum
zvariable zylast 

\ Integrate a complex integrand from x1 to x2 using
\ trapezoid rule

: ztrap_integrate ( F: x1 x2 -- z )
	z=0 zsum z!
	fover fover fswap f- NDIV s>f f/ xdel F!
	xfinal F!
	fdup Integrand zylast z!
	BEGIN
	  xdel f@ F+
	  fdup Integrand 
	  zdup zylast z@ z+ 
	  zsum z@ z+ zsum z!
	  zylast z!
	  fdup xfinal f@ f>=
	UNTIL
	fdrop
	zsum z@
	0.5e z*f xdel f@ z*f 
;



