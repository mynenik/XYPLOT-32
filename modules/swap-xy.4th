\ swap-xy.4th
\
\ Swap the x and y values of the active dataset
\
\ Copyright (c) 2011--2012 Krishna Myneni
\ Creative Consulting for Research and Education
\
\ Revisions:
\   2012-05-26  km  merge two forked versions
\   2012-06-26  km  convert to unnamed module

Begin-Module

DatasetInfo ds

Public:

: swap-xy ( -- )
	?active			\ get the active set number
	dup 0 >=		\ is it valid (non-negative)?
	if
	  ds get_ds	        \ ok, get info about the dataset
	  0 >=			\ did get_ds return an error?
	  if
	    ds DatasetInfo->Npts @	\ ok, obtain the number of points
	    0 do	
	      i ds @xy	        \ fetch the i^th x, y pair 
	      fswap
	      i ds !xy	        \ store the i^th x, y pair
	    loop
	    ?active
	    set_ds_extrema	\ reset the extrema for that dataset
	  then
	else
	  drop
	then ;

\ add this function to the math menu

MN_MATH c" Swap XY" c" swap-xy reset_window" add_menu_item

End-Module

