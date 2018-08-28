\ xymap.4th
\
\ xyplot module for creating 1-D maps on x or y values of a dataset. 
\  Maps are useful for analyzing chaotic sequences. For example,
\  a dataset can contain the sequence of peaks in a chaotic waveform, 
\  where the x values are the positions, and the y values are the peak
\  heights. A "return-map" on the peak heights or heights usually shows
\  a simple structure for low-dimensional chaotic systems. The peak 
\  searching module for xyplot (xypeaks.4th) may be used to 
\  extract peak information from waveform data.
\
\ Copyright (c) 2000--2012 Krishna Myneni
\ Provided under the GNU Lesser General Public License (LGPL)
\
\ Required files:
\	xyplot.4th
\
\ Revisions:
\      04-01-2000   created
\      01-15-2005   updated use of DatasetInfo structure km
\      08-18-2007   updated to use FSL matrices; placed menu items
\                     in a Maps submenu of the Math menu  km
\      2009-10-29   updated data structure member names  km
\      2012-06-26   converted to unnamed module  km

Begin-Module

DatasetInfo ds1
DatasetInfo dsmap
FLOAT DMATRIX  map_data{{

variable map_type
0 value Npts

Public:

\ map type = 
\	0  y_n+1 vs y_n
\ 	1  x_n+1 vs x_n
\	2  dy_n+1 vs dy_n
\	3  dx_n+1 vs dx_n
\	4  dy_n+1 vs dx_n

: map ( ntype -- | create a map )
    map_type !
    ?active dup 0< IF drop EXIT THEN
    ds1 get_ds
    0 >= IF

	map_type @
	CASE
	    0 OF
		ds1 DatasetInfo->Npts @ 1- to Npts
		& map_data{{ Npts 2 }}malloc
		Npts 0 DO
		    I    ds1 @xy fswap fdrop  map_data{{ I 0 }} F!
		    I 1+ ds1 @xy fswap fdrop  map_data{{ I 1 }} F!
		LOOP
		c" Map of y_n+1 vs y_n"
	    ENDOF
	        
	    1 OF
		ds1 DatasetInfo->Npts @ 1- to Npts
		& map_data{{ Npts 2 }}malloc
		Npts 0 DO
		    I    ds1 @xy fdrop  map_data{{ I 0 }} F!
		    I 1+ ds1 @xy fdrop  map_data{{ I 1 }} F!
		LOOP
		c" Map of x_n+1 vs x_n"
	    ENDOF

	    2 OF
		ds1 DatasetInfo->Npts @ 2- to Npts
		& map_data{{ Npts 2 }}malloc
		Npts 0 DO
		    I 1+ ds1 @xy fswap fdrop  I ds1 @xy fswap fdrop F-
		    map_data{{ I 0 }} F!
		    I 2+ ds1 @xy fswap fdrop  I 1+ ds1 @xy fswap fdrop F-
		    map_data{{ I 1 }} F!
		LOOP
		c" Map of dy_n+1 vs dy_n"
	    ENDOF

	    3 OF
		ds1 DatasetInfo->Npts @ 2- to Npts
		& map_data{{ Npts 2 }}malloc
		Npts 0 DO
		    I 1+ ds1 @xy fdrop I    ds1 @xy fdrop F-  map_data{{ I 0 }} F!
		    I 2+ ds1 @xy fdrop I 1+ ds1 @xy fdrop F-  map_data{{ I 1 }} F!
		LOOP
		c" Map of dx_n+1 vs dx_n"
	    ENDOF

	    4 OF
		ds1 DatasetInfo->Npts @ 2- to Npts
		& map_data{{ Npts 2 }}malloc
		Npts 0 DO
		    I 1+ ds1 @xy fdrop I ds1 @xy fdrop F-  map_data{{ I 0 }} F!
		    I 2+ ds1 @xy fswap fdrop I 1+ ds1 @xy fswap fdrop F-
		    map_data{{ I 1 }} F!
		LOOP
		c" Map of dy_n+1 vs dx_n"
	    ENDOF
	    0 to Npts  c"  "
	ENDCASE

	Npts 0= IF drop ." Unrecognized map type." EXIT THEN

	\ Create the map dataset in xyplot

	1+         dsmap  DatasetInfo->Header !
	c" Map" 1+ dsmap  DatasetInfo->Name !
	REAL_DOUBLE dsmap DatasetInfo->Type !	\ double precision fp type
	Npts       dsmap  DatasetInfo->Npts !
	2          dsmap  DatasetInfo->Size !
	map_data{{ dsmap  DatasetInfo->Data !

	dsmap make_ds drop
	    
	& map_data{{ }}free
    THEN
;	    
	    
MN_MATH  c" Maps" make_submenu  CONSTANT  MN_MAPS

MN_MAPS  c" y_n+1 vs y_n"    c" 0 map"  add_menu_item
MN_MAPS  c" x_n+1 vs x_n"    c" 1 map"  add_menu_item
MN_MAPS  c" dy_n+1 vs dy_n"  c" 2 map"  add_menu_item
MN_MAPS  c" dx_n+1 vs dx_n"  c" 3 map"  add_menu_item
MN_MAPS  c" dy_n+1 vs dx_n"  c" 4 map"  add_menu_item

End-Module


