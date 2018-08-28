\ grace.4th
\
\ Generate a Grace project file for grace version 5.1.10
\   from within xyplot.
\
\ Grace is a program for generating publication quality scientific 
\ graphs. It is distributed under the GNU General Public License. 
\ For information on Grace, see
\
\	http://plasma-gate.weizmann.ac.il/Grace/
\
\ This file is Copyright (c) 2003--2012 Krishna Myneni
\ Provided under the GNU General Public License
\
\ Revisions:
\
\	2003-03-27  created  km
\       2005-02-03  added ability to read back in grace agr files;
\                   added ability to store and read dataset headers in agr files;
\                   fixed stack garbage in reader words.  km
\	2005-02-06  uses file_open_dialog interface in xyplot 2.1.x for
\                     selecting files.  km
\       2007-06-07  updated to use FSL matrices  km
\       2009-10-28  removed plotting symbol constants (already in xyplot.4th);
\                     revised data structure members; moved tdstring into
\                     xutils.4th  km
\       2012-06-26  converted to unnamed module  km

Begin-Module

   10  constant  MAXPLOTS
32768  constant  MAXGRACEPTS
16384  constant  MAXHDRSIZE

variable gr_fid

: rep(',") ( a u -- a u | replace every single quote with double quote in string)
	2dup 0 ?do dup c@ [char] ' = if [char] " over c! then 1+ loop drop ;
	  
: >grfile ( a u -- | write string to grace file )
	gr_fid @ write-line drop ;

create grfile_buf 256 allot
variable grace_line_count

: <grfile ( -- a u flag | read string from grace file )
	grfile_buf 256 gr_fid @ read-line drop grfile_buf -rot 
	1 grace_line_count +! ;

PlotInfo pl1
DatasetInfo ds1

create set_specifier 12 allot
s" @     s?    " set_specifier swap cmove 

create xyplot_set_specifier 14 allot
s" # @xyplot s?? " xyplot_set_specifier swap cmove

variable flag_symbol
variable flag_line
variable hdr_line_count
fvariable xmin
fvariable xmax
fvariable ymin
fvariable ymax

Public:

: write_grace_file ( -- )
	s" # Grace project file (xyplot generated)" >grfile
	s" # " tdstring strcat >grfile
	s" @version 50110" >grfile
	s" @page size 792, 612" >grfile
	s" @background color 0" >grfile
	s" @page background fill on" >grfile

	s" @map color 0 to (255, 255, 255), 'white'" 	rep(',")  >grfile
	s" @map color 1 to (0, 0, 0), 'black'"       	rep(',")  >grfile
	s" @map color 2 to (0, 0, 255), 'blue'"      	rep(',")  >grfile
	s" @map color 3 to (255, 0, 0), 'red'"	     	rep(',")  >grfile
	s" @map color 4 to (34, 139, 34), 'forestgreen'" rep(',")  >grfile
	s" @map color 5 to (64, 224, 208), 'turquoise'" rep(',")  >grfile
	s" @map color 6 to (255, 0, 255), 'magenta'"   	rep(',")  >grfile
	s" @map color 7 to (160, 82, 45), 'sienna'"    	rep(',")  >grfile
	s" @map color 8 to (255, 215, 0), 'gold'"	rep(',")  >grfile
	s" @map color 9 to (255, 165,0), 'orange'"  	rep(',")  >grfile
	s" @map color 10 to (0, 0, 0), 'black'"  	rep(',")  >grfile

	s" @g0 on" >grfile
	s" @g0 type XY" >grfile
	s" @with g0" >grfile
	
	get_window_limits ymax f! xmax f! ymin f! xmin f!

	xmin f@ 6 f>string count  s" @     world xmin " 2swap strcat >grfile
	xmax f@ 6 f>string count  s" @     world xmax " 2swap strcat >grfile
	ymin f@ 6 f>string count  s" @     world ymin " 2swap strcat >grfile
	ymax f@ 6 f>string count  s" @     world ymax " 2swap strcat >grfile

	s" @     xaxis on" >grfile
	s" @     xaxis tick on" >grfile 
	s" @     xaxis tick major " 
	xmax f@ xmin f@ f- 10e f/ 6 f>string count strcat >grfile
	s" @     xaxis ticklabel on" >grfile
	s" @     yaxis on" >grfile
	s" @     yaxis tick on" >grfile
	s" @     yaxis tick major " 
	ymax f@ ymin f@ f- 10e f/ 6 f>string count strcat >grfile
	s" @     yaxis ticklabel on" >grfile

	s" @     frame type 0" >grfile

\ Write info on all visible sets

	MAXPLOTS 0 DO
	  i pl1 get_plot 0< if leave then
	  pl1 PlotInfo->set @ ds1 get_ds drop
	  pl1 PlotInfo->set @ [char] 0 + set_specifier 7 + c!
	  pl1 PlotInfo->set @ s>d <# # # #> xyplot_set_specifier 11 + swap cmove

	  \ Write dataset name
	  set_specifier 9 s" comment '" strcat
	  ds1 DatasetInfo->Name a@ dup strlen strcat s" '" strcat rep(',") >grfile

	  \ Write dataset header
	  0 hdr_line_count !
	  ds1 DatasetInfo->header a@ dup strlen dup
	  IF
	    BEGIN
	      2dup 10 scan dup >r
	      2swap r> -  dup IF
	        1 hdr_line_count +!
	        xyplot_set_specifier 14 s" h000 " strcat 
		over 15 + >r hdr_line_count @ s>d <# # # # #> r> swap cmove    
		2swap strcat 255 min ( 2dup type cr) >grfile
	      ELSE 2drop THEN
	      dup IF 1 /string THEN dup 0=
	    UNTIL
	    2drop
	  ELSE 2drop THEN

	  \ Write dataset plot info
	  set_specifier 9 s" hidden false" strcat >grfile
	  set_specifier 9 s" type xy" strcat >grfile

	  set_specifier 9 s" symbol " strcat	  
	  pl1 PlotInfo->Symbol @ dup dup SYM_POINT = swap 
	                       SYM_LINE_PLUS_POINT = or swap
			       SYM_BIG_POINT = or dup flag_symbol ! 
	  abs u>string count strcat >grfile

	  flag_symbol @ IF
	    set_specifier 9 s" symbol size " strcat
	    pl1 PlotInfo->Symbol @ SYM_BIG_POINT = IF s" 0.6" ELSE s" 0.2" THEN strcat >grfile
	    set_specifier 9 s" symbol color " strcat 
	    pl1 PlotInfo->Color @ 2+ u>string count strcat >grfile
	    set_specifier 9 s" symbol fill color " strcat
	    pl1 PlotInfo->Color @ 2+ u>string count strcat >grfile
	    set_specifier 9 s" symbol pattern 1" strcat >grfile
	    set_specifier 9 s" symbol fill pattern 1" strcat >grfile
	  THEN

	  set_specifier 9 s" line type " strcat
	  pl1 PlotInfo->Symbol @ dup  SYM_LINE = swap SYM_LINE_PLUS_POINT = or dup flag_line ! 	  
	  abs u>string count strcat >grfile

	  flag_line @ IF
	    set_specifier 9 s" line color " strcat
	    pl1 PlotInfo->Color @ 2+ u>string count strcat >grfile
	    set_specifier 9 s" linewidth 1.0" strcat >grfile
	  THEN
	LOOP


\ Now write the visible set data

	MAXPLOTS 0 do
	  i pl1 get_plot 0< if leave then
	  pl1 PlotInfo->Set @ ds1 get_ds drop
	  s" @target G0.S" pl1 PlotInfo->Set @ u>string count strcat >grfile
	  s" @type xy" >grfile
	  ds1 DatasetInfo->Npts @ 0 ?do
	    i ds1 @xy 2>r 
	    8 f>string count s"  " strcat 2r> 8 f>string count strcat >grfile
	  loop
	  s" &" >grfile
	loop

\ Close the output file

	gr_fid @ close-file drop
;


: save_grace ( -- | prompt user for filename and write grace file )
	c" Enter the Grace (.agr) filename:" get_input
	if
	  count W/O create-file 0 < if  ." Unable to open output file!" 
	  else gr_fid ! write_grace_file then
	then ;

\ =================================================
\   Import graph from Grace agr file to xyplot
\ =================================================

Private:

variable grace_set
variable grace_set_npts

MAXGRACEPTS 2  FLOAT   MATRIX  grace_xydata{{
MAXPLOTS    5  INTEGER MATRIX  grace_pattrs{{         \ plot attributes

create  grace_set_names  MAXPLOTS 80 * allot  \ buffer to store set names
create  grace_color_map  MAXPLOTS 32 * allot  \ buffer to store color names
create  grace_headers    MAXHDRSIZE MAXPLOTS * allot  \ buffer to store headers

: parse_set_number ( a u -- a u n | parse a set number from the subline )
    over 2- c@ [char] 0 - ;
 
: parse_grace_name ( a u -- | parse and store a set name )
    parse_set_number >r  
    -trailing  9 /string 1- 79 min \ a2 u2  ( substring containing set name )
    r@ MAXPLOTS < IF
       r> 80 * grace_set_names + dup 80 erase 
       swap cmove
    ELSE  2drop  r> drop THEN 
;

: get_grace_set_name ( n -- a u | retrieve set name for set n)
    dup MAXPLOTS < IF    80 * grace_set_names + dup strlen 
                   ELSE  drop s" Unknown" 
                   THEN ;

: get_grace_header ( n -- a u | retrieve header string for set n)
    dup MAXPLOTS < IF    MAXHDRSIZE * grace_headers + dup strlen
                   ELSE  drop s" Grace Dataset"
                   THEN ; 


: parse_color_name ( a u -- | parse and store color name )
    -trailing 9 /string parse_token strpck string>s  >r
    s" ), " search IF
       4 /string 1- 31 min  \ a2 u2  (substring containing color name)
       r@ MAXPLOTS < IF
         r> 32 * grace_color_map + dup 32 erase
	 swap cmove
         EXIT
       THEN
    THEN
    2drop r> drop
;

: get_grace_color ( n -- a u | retrieve color name of color n from map)
    dup MAXPLOTS < IF
      32 * grace_color_map + dup strlen
    ELSE
      drop s" black"  \ default color for unknown
    THEN ;

        
: parse_grace_pattrs ( a u -- | parse and store plot attributes )
    parse_set_number >r
    2dup s" line type" search 
      IF 9 /string strpck string>s  grace_pattrs{{ r> 0 }} ! 2drop EXIT
      ELSE 2drop THEN
    2dup s" line color" search
      IF 10 /string strpck string>s grace_pattrs{{ r> 1 }} ! 2drop EXIT
      ELSE 2drop THEN
    2dup s" symbol color" search
      IF 12 /string strpck string>s grace_pattrs{{ r> 3 }} ! 2drop EXIT
      ELSE 2drop THEN
    2dup s" symbol size" search
      IF 11 /string strpck string>f 10e f* f>d drop
	  grace_pattrs{{ r> 4 }} ! 2drop EXIT
      ELSE 2drop THEN   
    2drop r> drop ;


variable hdr_set
variable hdr_line

: parse_grace_hdr_line ( a u -- | parse line of xyplot header data in grace file)
    9 /string over 2 strpck string>s  hdr_set !
    4 /string over 3 strpck string>s  hdr_line !    
    4 /string
    grace_headers hdr_set @ MAXHDRSIZE * +
    dup strlen dup IF + 10 over c! 1+ ELSE + THEN swap cmove
;

: parse_xyplot_extras ( a u -- | parse and store additional xyplot information)
    2dup s" @xyplot s" search IF parse_grace_hdr_line ELSE 2drop THEN
    2drop ;
      


: read_grace_set ( n -- err | read set n)
    grace_set !
    grace_set @  get_grace_set_name drop ds1 DatasetInfo->Name !
    grace_set @  get_grace_header   drop ds1 DatasetInfo->Header !
    REAL_DOUBLE ds1 DatasetInfo->Type  !	\ real dataset (double precision)
    2   ds1 DatasetInfo->Size  !		\ dimension is 2 (x, y)
    0   ds1 DatasetInfo->Npts  !
    grace_xydata{{ ds1 DatasetInfo->Data  !	\ pointer to data
    0 grace_set_npts !

    <grfile IF 
      2drop \ s" @type" search 0= IF 2drop 3 exit THEN 
    ELSE 2drop 4 EXIT 
    THEN
    BEGIN
      <grfile IF
        over c@ [char] & = IF
          2drop
	  grace_set_npts @ 0> IF
	    grace_set_npts @ ds1 DatasetInfo->Npts  !
	    ds1 make_ds drop                     \ make the dataset

	    \ make a corresponding plot also

	    \ determine xyplot plot attributes from parsed grace attributes

	    grace_pattrs{{ grace_set @ 0 }} @ 
	    IF 
	      SYM_LINE grace_pattrs{{ grace_set @ 4 }} @ IF
	        drop  SYM_LINE_PLUS_POINT   THEN
	    ELSE grace_pattrs{{ grace_set @ 4 }} @
	      5 < IF SYM_POINT ELSE SYM_BIG_POINT THEN
	    THEN   pl1 PlotInfo->Symbol   !  \ plot symbol
	  
	    grace_set @ pl1 PlotInfo->Set  !	\ dataset number
	    0           pl1 PlotInfo->Type !	\ plot type
	    pl1 make_plot

	    \ set the plot color

	    pl1 PlotInfo->Symbol @ dup SYM_POINT = swap SYM_BIG_POINT = or
	    IF  3  ELSE  1  THEN
	    grace_pattrs{{ grace_set @ rot }} @ 
	    get_grace_color strpck set_plot_color

	  THEN
	  0 EXIT
        ELSE
          parse_args 2 <> IF abort THEN  \ wrong number of values
	  grace_xydata{{ grace_set_npts @ 1 }} F!
	  grace_xydata{{ grace_set_npts @ 0 }} F!
	  1 grace_set_npts +!
        THEN
      ELSE
        2drop 2 EXIT   \ reached EOF before end of dataset
      THEN
      grace_set_npts @ MAXGRACEPTS >= IF 0 EXIT THEN
    AGAIN
;

 
Public:

: read_grace_file ( -- | assumes file has already been opened)
    0 grace_line_count !
    grace_pattrs{{ MAXPLOTS 5 * cells erase
    grace_set_names  MAXPLOTS 80 * erase
    grace_headers    MAXPLOTS MAXHDRSIZE * erase
    -1e xmin f! -1e ymin f! 1e xmax f! 1e ymax f!
    BEGIN
      <grfile IF  \ -- a u
        ( 2dup type cr)
	2dup s" # @xyplot" search IF parse_xyplot_extras 2drop  \ extra xyplot info lines
	ELSE 2drop over c@ [char] # = IF 2drop             \ skip comment line 
	  ELSE
	    2dup s" world xmin" search IF 10 /string strpck string>f xmin f! ELSE 2drop THEN
            2dup s" world xmax" search IF 10 /string strpck string>f xmax f! ELSE 2drop THEN
            2dup s" world ymin" search IF 10 /string strpck string>f ymin f! ELSE 2drop THEN
            2dup s" world ymax" search IF 10 /string strpck string>f ymax f! ELSE 2drop THEN
	    2dup s" map color"  search IF parse_color_name  ELSE 2drop THEN
	    2dup s" comment"    search IF parse_grace_name  ELSE 2drop THEN
	    2dup s" line type"  search IF parse_grace_pattrs ELSE 2drop THEN
	    2dup s" line color" search IF parse_grace_pattrs ELSE 2drop THEN
	    2dup s" symbol color" search IF parse_grace_pattrs ELSE 2drop THEN
	    2dup s" symbol size" search IF parse_grace_pattrs ELSE 2drop THEN
	    2dup s" @target G0.S" search IF 12 /string strpck string>s read_grace_set
	      dup IF ." read_grace_set error: " . gr_fid f@ close-file drop exit
	          ELSE drop THEN
	      ELSE 2drop THEN
	    2drop
	  THEN
	THEN
      ELSE
        2drop
        reset_window
        xmin f@ ymin f@ xmax f@ ymax f@ set_window_limits
        gr_fid @ close-file drop EXIT
      THEN
    AGAIN ;


: read_grace ( -- | prompt user for filename and read grace file )
	\ c" Enter the Grace (.agr) filename:" get_input
	c" *.agr" file_open_dialog
	IF
	  count R/O open-file 0 < IF drop
	    ." Unable to open output file!" 
	  ELSE gr_fid ! read_grace_file THEN
	THEN ;
 
\ add Grace file functions to the File Menu

MN_FILE  c" Save Grace File"   c" save_grace"  add_menu_item
MN_FILE  c" Read Grace File"   c" read_grace"  add_menu_item

End-Module

	

