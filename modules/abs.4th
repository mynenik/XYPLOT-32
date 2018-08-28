\ abs.4th
\
\ Take absolute value of y values of the active dataset
\
\ Copyright (c) 2000--2012 Krishna Myneni
\ 
\ Revisions:
\    2000-06-21  KM
\    2009-10-29  km  updated data structure field names
\    2012-06-26  km  convert to unnamed module; update names

Begin-Module

DatasetInfo ds

Public:

: abs_ds ( -- )
	?active			\ get the active set number
	dup 0 >=		\ is it valid (non-negative)?
	if
	  ds get_ds	        \ ok, get info about the dataset
	  0 >=			\ did get_ds return an error?
	  if
	    ds DatasetInfo->Npts @	\ ok, obtain the number of points
	    0 do	
	      i ds @xy		\ fetch the i^th x, y pair 
	      fabs  		\ take absolute value
	      i ds !xy		\ store the i^th x, y pair
	    loop
	    ?active
	    set_ds_extrema	\ reset the extrema for that dataset
	  then
	else
	  drop
	then ;

\ add this function to the math menu

MN_MATH c" Magnitude" c" abs_ds reset_window" add_menu_item

End-Module
