\ ylog.4th
\
\ Take log base 10 of y values of a dataset
\
\ K. Myneni, 
\ Revisions:
\   8-14-2000, 10-29-2009, 06-26-2012
\

Begin-Module

DatasetInfo ds

Public:

: ylog
	?active dup 0 >=
	if
	  ds get_ds
	  0 >= if
	    ds DatasetInfo->Npts @ 0 do
	      i ds @xy fdup 0e f>	\ test for y > 0
	      if flog else fdrop 0e then
	      i ds !xy
	    loop
	    ?active set_ds_extrema
	  then
	else
	  drop
	then ;

\ add "Log(Y)" as an item in the math menu

MN_MATH c" Log(Y)" c" ylog reset_window" add_menu_item

End-Module

