\ lpf.4th
\
\ Apply a low-pass RC filter to a data set
\
\
\  v_in(t)--------/\/\/\/\------*-------------- v_out(t)
\                     R         |
\				|
\			    ---------  C
\			    ---------  
\				|
\				|
\				V (Ground)
\
\
\ Copyright (c) 2002  Krishna Myneni, Provided under the GNU
\   General Public License.
\
\ Revisions:
\   2012-06-02  km  make this an unnamed module
\
\ Notes:
\
\ 1. The calculation is based on integrating the differential equation:
\
\	d v_out / dt  = (v_in - v_out)/RC
\

Begin-Module

fvariable RC		1e-4 RC f!	\ default time constant of the filter
fvariable xlast
fvariable ylast

DatasetInfo ds

Public:

: lpf ( -- | apply a low pass filter to a data set )
	\ The input data is replaced with the output data
	\ First output value assumed to be same as first input.

	?active dup 0 >=
	if
	  ds get_ds
	  0 >= if
	    0 ds @xy  ylast f! xlast f!

	    ds DatasetInfo->Npts @ 1- 1 do
	      i ds @xy ylast f@ f- RC f@ f/ fswap xlast f@ f- f*    
	      ylast f@ f+ ylast f!
	      i ds @xy fdrop 
	      fdup ylast f@ i ds !xy
	      xlast f!
	    loop 
	    ?active set_ds_extrema
	  then
	else
	  drop
	then ;

: xylpf ( -- | prompt user and filter the active data set )
	?active 0 >= if
	  c" Enter the time constant:" get_input
	  if   string>f RC f! lpf
	  else drop
	  then
	then ;

\ add "Low-Pass Filter" as an item in the math menu

MN_MATH c" Low-Pass Filter" c" xylpf draw_window" add_menu_item

End-Module

