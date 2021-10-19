\ abs.4th
\
\ Take absolute value of y values of the active dataset
\
\ Copyright (c) 2000--2020 Krishna Myneni
\
Begin-Module

DatasetInfo ds

Public:

: abs_ds ( -- )
	?active			\ get the active set number
	dup 0 >= IF		\ is it valid (non-negative)?
	  ds get_ds		\ ok, get info about the dataset
	  0 >= IF		\ did get_ds return an error?
	    ds DatasetInfo->Npts @
	    0 DO
	      I ds @xy  fabs  I ds !xy
	    LOOP
	    ?active
	    set_ds_extrema	\ reset extrema for dataset
	  THEN
	ELSE
	  drop
	THEN ;

\ add this function to the math menu

MN_MATH c" Magnitude" c" abs_ds reset_window" add_menu_item

End-Module
