\ pnm.4th
\ 
\ xyplot interface to 640x480 pnm files
\
\ Copyright (c) 2004--2009 Krishna Myneni
\
\ Revisions:
\   2005-01-14  km  updated use of DatasetInfo and PlotInfo structures 
\   2009-10-28  km  updated data structure member names
\
\ Requires:
\
\	strings.4th
\	files.4th
\

Begin-Module

DatasetInfo dsspec
PlotInfo    plspec

640 constant PNM_HORIZ_SIZE
480 constant PNM_VERT_SIZE

create pnm_line PNM_HORIZ_SIZE 2* sfloats allot

\ Fill out a dataset info structure on the new set

c" PNM line scan" 1+ dsspec DatasetInfo->Name ! \ name of new dataset
c"  " 1+ dsspec DatasetInfo->Header !
REAL_SINGLE dsspec DatasetInfo->Type !		\ real dataset
PNM_HORIZ_SIZE dsspec DatasetInfo->Npts !	\ points in the set
2 dsspec DatasetInfo->Size !			\ dimension is 2 (x, y)
pnm_line dsspec DatasetInfo->Data !		\ pointer to data

\ create a new data set in xyplot

dsspec make_ds
dup dsspec get_ds drop		\ get xyplot dataset info

\ Fill out a plot info structure on the new plot

  plspec PlotInfo->Set !	\ dataset number
0 plspec PlotInfo->Type !	\ plot type
1 plspec PlotInfo->Symbol !	\ plot symbol
1 plspec PlotInfo->Color !	\ plot color

\ create a new plot in xyplot

plspec make_plot


\ PNM file interface

variable pnm-id

Public:

: open-pnm-file	( ^name -- )
	count R/O open-file ABORT" Unable to open input file."
	pnm-id ! ;


create pnm_linebuf PNM_HORIZ_SIZE  PNM_VERT_SIZE max 3 * allot

: read_horizontal_line ( n -- flag | read current line data and put in buffer)
	\ n is the line number and goes from 1 to PNM_VERT_SIZE
	1- PNM_HORIZ_SIZE 3 * *			\ offset in bytes to line n
	s>d pnm-id @ reposition-file drop
	pnm_linebuf PNM_HORIZ_SIZE 3 * pnm-id @ read-file
	if drop false exit then drop 
	PNM_HORIZ_SIZE 0 do
	  i 1+ s>f				\ x value
	  pnm_linebuf i 3 * + c@ s>f	\ y value
	  i dsspec !xy
	loop 
	?active set_ds_extrema 
	true ;

: read_vertical_line ( n -- flag )
	\ n goes from 1 to PNM_HORIZ_SIZE
	1- 3 * 				\ offset to first pixel in line
	PNM_VERT_SIZE 0 do
	  dup
	  s>d pnm-id @ reposition-file drop
	  pnm_linebuf i + 1 pnm-id @ read-file
	  if 2drop false exit then drop
	  PNM_HORIZ_SIZE 3 * +
	loop
	drop
	PNM_VERT_SIZE 0 do
	  i 1+ s>f
	  pnm_linebuf i + c@ s>f
	  i dsspec !xy
	loop
	?active set_ds_extrema
	true ;

Private:

variable current_line
1 current_line !

Public:

: show_horizontal_lines ( -- | update line scan in xyplot )
	false false set_grid_lines
	draw_window
	current_line @ read_horizontal_line	\ get the current line
	drop 
	true draw_plot
	begin 
	  key? if
	    key case
	      [char] i  of  -1 current_line +!  endof
	      [char] k  of   1 current_line +!  endof
	      [char] I  of -10 current_line +!  endof
	      [char] K  of  10 current_line +!  endof
	      27        of   exit               endof
	    endcase
	    false draw_plot			\ erase previous plot
	    current_line @ 1 max PNM_VERT_SIZE min
	    current_line !
	    current_line @ read_horizontal_line
	    if true draw_plot else ." Unable to read line " then
	    current_line @ . bl emit
	    500 ms
	  then
	again ;


: show_vertical_lines ( -- | plot vertical line scan in xyplot )
	false false set_grid_lines
	draw_window
	current_line @ read_vertical_line
	drop
	true draw_plot
	begin
	  key? if
	    key case
	      [char] j  of  -1 current_line +!  endof
	      [char] l  of   1 current_line +!  endof
	      [char] J  of -10 current_line +!  endof
	      [char] L  of  10 current_line +!  endof
	      27        of   exit               endof
	    endcase
	    false draw_plot			\ erase previous plot
	    current_line @ 1 max PNM_HORIZ_SIZE min
	    current_line !
	    current_line @ read_vertical_line
	    if true draw_plot else ." Unable to read line " then
	    current_line @ . bl emit
	    500 ms
	  then
	again ;
	

: show_pnm_hor ( -- )
	c" Enter the .pnm filename:" get_input
	if
	  dup count type
	  open-pnm-file
	  PNM_VERT_SIZE 2/ current_line !
	  show_horizontal_lines
	  pnm-id @ close-file drop 
	else
	  drop
	then ;

: show_pnm_ver ( -- )
	c" Enter the .pnm filename:" get_input
	if
	  dup count type
	  open-pnm-file
	  PNM_HORIZ_SIZE 2/ current_line !
	  show_vertical_lines
	  pnm-id @ close-file drop 
	else
	  drop
	then ;

\ add item to xyplot DATA menu.

MN_DATA c" Show PNM Hor Lines" c" show_pnm_hor" add_menu_item
MN_DATA c" Show PNM Ver Lines" c" show_pnm_ver" add_menu_item

\ initialization

" white" set_background
" blue" set_plot_color
reset_window
false false set_grid_lines


End-Module

