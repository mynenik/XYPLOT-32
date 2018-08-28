\ yln.4th
\
\ Take log base e of y values of a dataset
\
\ K. Myneni
\

Begin-Module

DatasetInfo ds

Public:

: yln
	?active dup 0 >=
	if
	  ds get_ds
	  0 >= if
	    ds DatasetInfo->Npts @ 0 do
	      i ds @xy fdup 0e f>	\ test for y > 0
	      if fln else fdrop 0e then
	      i ds !xy
	    loop
	    ?active set_ds_extrema
	  then
	else
	  drop
	then ;


\ add "Ln(Y)" as an item in the math menu

MN_MATH c" Ln(Y)" c" yln reset_window" add_menu_item

End-Module

