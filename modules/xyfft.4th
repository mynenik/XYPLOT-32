\ xyfft.4th
\
\ FFT module for xyplot
\
\ Copyright (c) 2000--2009 Krishna Myneni
\ Provided under the GNU Lesser General Public License (LGPL)
\
\ Requires:
\   complex.4th
\   four1.4th
\
\ Revisions:
\
\   2004-07-26  km; added Real(FT) and Imag(FT) functions
\   2004-07-27  km; added Real(FT^-1)
\   2005-01-13  km; updated use of DatasetInfo structure
\   2005-08-03  km; create FFT submenu for functions
\   2009-10-29  km; updated data structure member names
\
\ Notes:
\
\  1. Data should be uniformly spaced.
\

DatasetInfo dsfft 
FLOAT DMATRIX  fftdata{

0 value Npts

\ determine nearest power of 2 >= n and set the fftdata{ array size accordingly
: allocate_fft_array ( n -- )
    1
    BEGIN
	2dup 1 swap lshift >
    WHILE  1+
    REPEAT
    nip 1 swap lshift  \ ensure power of 2
    TO Npts
;



fvariable dx
fvariable df
fvariable dt

: real-ft ( -- | take Real[FT] )
    ?active dup 0< IF drop EXIT THEN
    dsfft get_ds 0< IF EXIT THEN

    \ Determine the size of the fft array and allocate it
		
    dsfft DatasetInfo->Npts @ allocate_fft_array	
    \ fftdata fmat_zero		\ zero the array		

    \ Determine frequency scale

    0 dsfft @xy fdrop  1 dsfft @xy fdrop  F- fabs  dx F!
    1e Npts 2* s>f F/  dx F@ F/ df F!
	     
    \ Transfer data from xyplot dataset to the fftdata buffer

    dsfft DatasetInfo->Npts @ 0 DO
	I dsfft @xy  0e fftdata{ I 2* } z!  fdrop
    LOOP
	    
    Npts 1 fftdata{ }four1  \ perform the Fast Fourier Transform

    \ Replace the data with appropriate x and Real(FT) values

    Npts 0 DO
	I s>f df F@ F* fftdata{ I 2* } F@  fftdata{ I 2* } z!
    LOOP
	    
    \ make a new xyplot dataset

    c" Real(FT)" 1+ ds1 DatasetInfo->Name !
    c"  " 1+ ds1 DatasetInfo->Header !
    256  ds1 DatasetInfo->Type !
    Npts ds1 DatasetInfo->Npts !
    2 ds1 DatasetInfo->Size !
    fftdata{  ds1 DatasetInfo->Data !		 	      
    ds1 make_ds drop
;

0 [IF]
: imag-ft ( -- | take Imag[FT] )
	?active dup 0 >=
	if
	  dsfft get_ds
	  0 >= if

		\ Set the size of the fft data buffer
		
		dsfft DatasetInfo->Npts @ adjust_fft_size	\ ensure power of 2
		fftdata fmat_zero		\ zero the buffer		

		\ Determine frequency scale

		0 dsfft @xy fdrop
	        1 dsfft @xy fdrop
		f- fabs fft_dx f!
		1e fftdata mat_size@ drop s>f f/ fft_dx f@ f/
		fft_df f!
	     
		\ Transfer data from xyplot dataset to the fftdata buffer

		dsfft DatasetInfo->Npts @ 0 do
		  i dsfft @xy
		  i 2* 1+ 1 fftdata fmat!
		  0e i 1+ 2* 1 fftdata fmat! \ set imaginary part to zero
		  fdrop
		loop
	    
		fftdata fft	\ perform the Fast Fourier Transform

		\ Replace the data with appropriate x and Imag(FT) values

		fftdata mat_size@ drop 1+ 1 do
		  i s>f fft_df f@ f* 		\ determine x value
		  i 1 fftdata fmat!
		2 +loop
	    
		\ make a new xyplot dataset

		c" Imag(FT)" 1+ ds1 DatasetInfo->Name !
		c"  " 1+ ds1 DatasetInfo->Header !
		256 ds1 DatasetInfo->Type !
		fftdata mat_size@ drop ds1 DatasetInfo->Npts !
		2 ds1 DatasetInfo->Size !
		fftdata cell+ cell+ ds1 DatasetInfo->Data !		 	      
		ds1 make_ds drop
 	  then
	else
	  drop
	then ;

: real-ft^-1 ( -- | take Real[FT^-1] )
	?active dup 0 >=
	if
	  dsfft get_ds
	  0 >= if

		\ Set the size of the fft data buffer
		
		dsfft DatasetInfo->Npts @ adjust_fft_size	\ ensure power of 2
		fftdata fmat_zero		\ zero the buffer		

		\ Determine time scale

		0 dsfft @xy fdrop
		fabs 2e f* 1e fswap f/ fft_dt f!
	     
		\ Transfer data from xyplot dataset to the fftdata buffer

		dsfft DatasetInfo->Npts @ 0 do
		  i dsfft @xy
		  i 2* 1+ 1 fftdata fmat!
		  0e i 1+ 2* 1 fftdata fmat! \ set imaginary part to zero
		  fdrop
		loop
	    
		fftdata inv_fft	\ perform the Fast Fourier Transform

		\ Replace the data with appropriate x and Re(FT^-1) values

		fftdata mat_size@ drop 1+ 1 do
		  i 1 fftdata fmat@
		  i 1+ 1 fftdata fmat!		\ store y value
		  i 1- s>f fft_dt f@ f* 	\ determine x value
		  i 1 fftdata fmat!
		2 +loop
	    
		\ make a new xyplot dataset

		c" Real(FT^-1)" 1+ ds1 DatasetInfo->Name !
		c"  " 1+ ds1 DatasetInfo->Header !
		256 ds1 DatasetInfo->Type !
		fftdata mat_size@ drop ds1 DatasetInfo->Npts !
		2 ds1 DatasetInfo->Size !
		fftdata cell+ cell+ ds1 DatasetInfo->Data !		 	      
		ds1 make_ds drop
	  then
	else
	  drop
	then ;
[THEN]

:  power ( -- | take fft of data and obtain its power spectrum )
    ?active dup 0 >= IF
	dsfft get_ds 0 >= IF

	    \ Set the size of the fft data buffer
		
	    dsfft DatasetInfo->Npts @ adjust_fft_size	\ ensure power of 2
	    fftdata fmat_zero		\ zero the buffer		

	    \ Determine frequency scale

	    0 dsfft @xy fdrop
	    1 dsfft @xy fdrop
	    F- fabs fft_dx F!
	    1e fftdata mat_size@ drop s>f f/ fft_dx f@ f/
	    fft_df F!
	     
	    \ Transfer data from xyplot dataset to the fftdata buffer

	    dsfft DatasetInfo->Npts @ 0 DO
		I dsfft @xy  0e fftdata{ i 2* } z! fdrop
	    LOOP
	    
	    fftdata{ }four1

		\ Replace the data with appropriate x and power values

		fftdata mat_size@ drop 1+ 1 do
		  i 1 fftdata fmat@ fdup f*
		  i 1+ 1 fftdata fmat@ fdup f* f+
		  i 1+ 1 fftdata fmat!		\ store y value
		  i s>f fft_df f@ f* 		\ determine x value
		  i 1 fftdata fmat!
		2 +loop
	    
		\ make a new xyplot dataset containing the power spectrum

		c" PowerSpectrum" 1+ ds1 DatasetInfo->Name !
		c"  " 1+ ds1 DatasetInfo->Header !
		256 ds1 DatasetInfo->Type !
		fftdata mat_size@ drop ds1 DatasetInfo->Npts !
		2 ds1 DatasetInfo->Size !
		fftdata cell+ cell+ ds1 DatasetInfo->Data !		 	      
		ds1 make_ds drop
	  then
	else
	  drop
	then ;	

MN_MATH c" FFT"  make_submenu  constant  MN_FFT

MN_FFT  c" Real(FT)"        c" real-ft"    add_menu_item
MN_FFT  c" Imag(FT)"        c" imag-ft"    add_menu_item
MN_FFT  c" Real(FT^-1)"     c" real-ft^-1" add_menu_item
MN_FFT  c" Power Spectrum"  c" power"      add_menu_item

