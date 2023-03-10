\ xyarea.4th
\
\ K. Myneni and R. Taylor
\
\ Revised: 
\   2009-10-28  km
\   2020-04-24  km  revised for current version of XYPLOT;
\                   includes only trapezoid integration;
\                   Simpson's rule integration to be added.

Begin-Module

DatasetInfo dsa
fvariable xlim1
fvariable xlim2
fvariable y1
fvariable y2

Public:

: print_area ( rA -- )
	s" Area[ " 
	xlim1 f@ f>$ strcat
	s" , " strcat
	xlim2 f@ f>$ strcat
	s"  ] = " strcat
	2>r f>$ 2r> 2swap strcat
	type ;

\ Trapezoidal Area Calculation
\ (non-uniform abscissa spacing allowed)
: area_trap ( -- )
	?active dup 0 >= IF
	  dsa get_ds 0 >= IF
	    dsa index_limits 2dup <> IF
	      2dup
	      >r dsa @xy fdrop xlim1 f!
	      r> dsa @xy fdrop xlim2 f!
	      2>r
	      0e
	      2r> swap DO
	        I dsa @xy y1 f! I 1+ dsa @xy y2 f!
	        f- y1 f@ y2 f@ f+ f* 0.5e f* f+
	      LOOP
	      fnegate
	      print_area
	    ELSE
	      2drop
	      s" Less than two points within plot domain!" message_box
	    THEN
	  THEN
	ELSE drop
	THEN ;
	    
MN_MATH c" Area (Trapezoid)" c" area_trap draw_window" add_menu_item

End-Module

