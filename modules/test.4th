\ test.4th
\
\ Functions for illustrating the xyplot-Forth interface
\
\ Copyright (c) 1999--2009 Krishna Myneni,
\ Creative Consulting for Research and Education
\
\ Revisions: 
\   2005-01-13  km
\   2009-10-28  km  updated data structure member names

DatasetInfo ds_info  \ allocate structure to hold dataset info


\ The following two words illustrate how to modify the data
\ in xyplot's active dataset. Both words will do the same thing: 
\ multiply all of the y values in the active dataset by -1. 
\ The word "test1" is easier to understand but less efficient 
\ since it relies on the words "@xy" and "!xy" to access data. 
\ The word "test2" is faster, working with the data
\ pointer directly, but its code is harder to follow.
\

: test1 ( -- )
	?active			\ get the active set number
	dup 0 >=		\ is it valid (non-negative)?
	if
	  ds_info get_ds	\ ok, get info about the dataset
	  0 >=			\ did get_ds return an error?
	  if
	    ds_info DatasetInfo->Npts @	\ ok, obtain the number of points
	    0 do	
	      i ds_info @xy	\ fetch the i^th x, y pair 
	      -1e f*  		\ multiply y by -1
	      i ds_info !xy	\ store the i^th x, y pair
	    loop
	    ?active
	    set_ds_extrema	\ reset the extrema for that dataset
	  then
	else
	  drop
	then ;

 
: test2 ( -- )
	?active			\ get the active set number
	dup 0 >= 		\ is it valid?
	if 			
	  ds_info get_ds 	\ ok, get info about the set
	  0 >= 			\ is it valid?
	  if
	    ds_info DatasetInfo->Data a@  \ fetch the pointer to the actual data
	    SFSIZE +		\ offset ptr by one float to point to first y
	    ds_info DatasetInfo->Size @	\ fetch number of values in a datum
	    SFSIZE * 		\ multiply by SFSIZE to get datum size in bytes
	    ds_info DatasetInfo->Npts @	\ fetch number of points
	    0 do			
	      over dup sf@	\ fetch the current y value (use sf@, not f@)
	      -1e f*		\ multiply by -1
	      rot sf!		\ store the y value (use sf!, not f!)
	      dup >r + r>	\ increment the pointer by datum size in bytes
	    loop	        
	    2drop		\ clean up the stack
	    ?active set_ds_extrema	\ reset the dataset extrema
	  then
	else
	  drop	
	then ;

	   
\ Remember to execute reset_win after executing test1 or test2
\ to make the plot display update!
 

