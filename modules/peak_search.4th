\ peak_search.4th
\
\  Purpose: Find all peak positions and heights in data contained
\             in x, y arrays. The argument ipw is used to
\             discriminate against minor fluctuations in data that
\             may be detected as a peak. ipw should be set to the
\             number of points over which the peak should maintain
\             a smooth shape.
\
\ Krishna Myneni, 04-01-2001
\
\ Requires:
\
\	derivative.4th
\	polyfit.4th
\
\ Revisions:
\       2007-06-12  rewritten for use with FSL-style arrays and modules
\       2012-06-26  convert to unnamed module
\

Begin-Module

FLOAT DMATRIX pksin{{

32768 constant MAX_PEAKS
MAX_PEAKS 2 FLOAT MATRIX peaks{{

32 constant MAX_WIDTH

0 value Npeaks
0 value Npts
0 value MinPkWidth
0 value ips
0 value ipe
0 ptr input_m{{

variable ys1
variable ys2

fvariable xpeak
fvariable ypeak
fvariable xpk2
fvariable xoffs

3          FLOAT ARRAY  xp{
3          FLOAT ARRAY  yp{
MAX_WIDTH  FLOAT ARRAY  xpp{
MAX_WIDTH  FLOAT ARRAY  ypp{
3          FLOAT ARRAY  pcoeffs{

Public:

: peak_search ( 'mat np ipw -- ierr)

    to MinPkWidth  to Npts  to input_m{{

    & pksin{{ Npts 2 }}malloc

    \ Copy the input matrix into the pksin matrix

    input_m{{  pksin{{ Npts 2 }}fcopy 

    \ Compute first derivative of data.

    pksin{{ Npts  derivative  IF 1 EXIT THEN

    \  Search for X axis crossings.

    0 to Npeaks
    pksin{{ 0 1 }} F@ 0e F< ys1 !	\ is first value negative?

    Npts 1- 1 DO
	pksin{{ I 1 }} F@ 0e F< ys2 !	
	ys2 @ ys1 @ <> IF
	    \  X axis crossing found. Is it a peak or a valley?
	    
	    ys1 @ 0 >= IF
		\ Went from positive to negative so we found a peak.
	      

		\ Use linear interpolation to determine x value at crossing.

		pksin{{ I 0 }}    F@  pksin{{ I 1- 1 }} F@  F*
		pksin{{ I 1- 0 }} F@  pksin{{ I 1 }}    F@  F*
		F-
		pksin{{ I 1- 1 }} F@  pksin{{ I 1 }} F@ F- F/ xpeak F!

		\  Use parabolic fit to original data to determine peak height.

		pksin{{ I 1- 0 }} F@ fnegate xoffs F!	\ -x(i-1)

		0e                                xp{ 0 } F!
		pksin{{ I 0 }}    F@ xoffs F@ F+  xp{ 1 } F!
		pksin{{ I 1+ 0 }} F@ xoffs F@ F+  xp{ 2 } F!

		input_m{{ I 1- 1 }} F@  yp{ 0 } F!
		input_m{{ I 1 }}    F@  yp{ 1 } F!
		input_m{{ I 1+ 1 }} F@  yp{ 2 } F!

		xp{ yp{ pcoeffs{ 2 3 polfit fdrop

		xpeak F@ xoffs F@ F+ xpk2 F!
		xpk2 F@ fdup F* pcoeffs{ 2 } F@ F*
		xpk2 F@ pcoeffs{ 1 } F@ F* F+
		pcoeffs{ 0 } F@ F+  ypeak F!

		xpeak F@  peaks{{ Npeaks 0 }} F!	\ store peak position
		ypeak F@  peaks{{ Npeaks 1 }} F!	\ store peak height

		Npeaks 1+ dup MAX_PEAKS <= IF to Npeaks ELSE drop THEN

		\  Test for minimum width

		I MinPkWidth 2/ - to ips
		I MinPkWidth 2/ + to ipe 

		ips 0<  ipe Npts >=  OR
		IF 
		    Npeaks 1- to Npeaks     \ reject the peak
		ELSE
		    pksin{{ ips 0 }} F@ xoffs f!

		    ipe 1+ ips DO
			pksin{{ I 0 }} F@ xoffs F@ F-   xpp{ I ips - }  F!
			input_m{{ I 1 }} F@             ypp{ I ips - }  F!
		    LOOP

		    xpp{ ypp{ pcoeffs{ 2 ipe ips - 1+ polfit fdrop
	        
		    pcoeffs{ 2 } F@ 0e F>=  IF Npeaks 1- to Npeaks THEN  \ reject the peak
		THEN
	    THEN
	THEN
	ys2 @ ys1 !
    LOOP

    Npeaks 0> IF
	0		\ no error
    ELSE
	2		\ error: no peaks found
    THEN ;

End-Module

