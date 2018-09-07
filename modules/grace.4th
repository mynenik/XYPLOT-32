\ grace.4th
\
\ Generate a Grace project file for grace version 5.1.23
\   from within xyplot.
\
\ Grace is a program for generating publication quality scientific 
\ graphs. It is distributed under the GNU General Public License. 
\ For information on Grace, see
\
\	http://plasma-gate.weizmann.ac.il/Grace/
\
\ This file is Copyright (c) 2003--2018 Krishna Myneni
\ Provided under the terms of the GNU General Public License
\
\ Please report bugs to  <krishna.myneni@ccreweb.org>
\
\ Revisions:
\
\  2003-03-27  created  km
\  2005-02-03  added ability to read back in grace agr files;
\              added ability to store and read dataset headers in agr files;
\              fixed stack garbage in reader words.  km
\  2005-02-06  uses file_open_dialog interface in xyplot 2.1.x for
\              selecting files.  km
\  2007-06-07  updated to use FSL matrices  km
\  2009-10-28  removed plotting symbol constants (already in xyplot.4th);
\              revised data structure members; moved tdstring into
\              xutils.4th  km
\  2012-06-26  converted to unnamed module  km
\  2018-08-29  fixed handling of "default" statements for read_grace_file;
\              renamed SAVE_GRACE to EXPORT_GRACE, and READ_GRACE to
\              IMPORT_GRACE, and changed Menu function labels accordingly  km
\  2018-09-06  completed fix of parsing words for import of .agr files:
\              PARSE_SET_NUMBER  PARSE_GRACE_PATTRS; added PARSE_WORLD_COORDS ;
\              factored words IMPORT_GRACE_SET and IMPORT_GRACE_PLOT  km
\  2018-09-07  further factored code; modified writing all xyplot datasets
\              to grace file, not just those with corresponding plots;
\              implemented line attributes and plot visibility; updated
\              MAXGRACEPTS to 65536
Begin-Module

Private:
   20  constant  MAXSETS
   20  constant  MAXPLOTS
65536  constant  MAXGRACEPTS
16384  constant  MAXHDRSIZE

variable gr_fid

: rep(',") ( a u -- a u | replace every single quote with double quote in string)
	2dup 0 ?do dup c@ [char] ' = if [char] " over c! then 1+ loop drop ;

: rep(comma,space) ( a u -- a u | replace every comma with a space in string)
	2dup 0 ?do dup c@ [char] , = if bl over c! then 1+ loop drop ;

: parse_csv ( a u -- r1 ... rn n )
    rep(comma,space) parse_args ;
	  
: >grfile ( a u -- | write string to grace file )
	gr_fid @ write-line drop ;

create grfile_buf 256 allot
variable grace_line_count

: <grfile ( -- a u b | read string from grace file )
	grfile_buf 256 erase
	grfile_buf 256 gr_fid @ read-line drop grfile_buf -rot
	1 grace_line_count +! ;

PlotInfo pl1
DatasetInfo ds1

create set_specifier 12 allot
s" @     s?    " set_specifier swap cmove 

create xyplot_set_specifier 14 allot
s" # @xyplot s?? " xyplot_set_specifier swap cmove

variable  flag_symbol
variable  flag_line
variable  hdr_line_count
fvariable xmin
fvariable xmax
fvariable ymin
fvariable ymax

Public:

: write_grace_colormap ( -- )
    s" @map color 0 to (255, 255, 255), 'white'" 	rep(',")  >grfile
    s" @map color 1 to (0, 0, 0), 'black'"       	rep(',")  >grfile
    s" @map color 2 to (0, 0, 255), 'blue'"      	rep(',")  >grfile
    s" @map color 3 to (255, 0, 0), 'red'"	     	rep(',")  >grfile
    s" @map color 4 to (34, 139, 34), 'forestgreen'"    rep(',")  >grfile
    s" @map color 5 to (64, 224, 208), 'turquoise'"     rep(',")  >grfile
    s" @map color 6 to (255, 0, 255), 'magenta'"   	rep(',")  >grfile
    s" @map color 7 to (160, 82, 45), 'sienna'"    	rep(',")  >grfile
    s" @map color 8 to (255, 215, 0), 'gold'"	        rep(',")  >grfile
    s" @map color 9 to (255, 165,0), 'orange'"  	rep(',")  >grfile
    s" @map color 10 to (0, 0, 0), 'black'"  	        rep(',")  >grfile
;

: write_grace_window ( -- )
    xmin f@ 6 f>string count  s" @     world xmin " 2swap strcat >grfile
    xmax f@ 6 f>string count  s" @     world xmax " 2swap strcat >grfile
    ymin f@ 6 f>string count  s" @     world ymin " 2swap strcat >grfile
    ymax f@ 6 f>string count  s" @     world ymax " 2swap strcat >grfile
;

: write_grace_graph ( -- )
    s" @g0 on"           >grfile
    s" @g0 type XY"      >grfile
    s" @with g0"         >grfile
    s" @g0 hidden false" >grfile
;

\ Write info about all xyplot data sets and their corresponding plot
\ attributes. A data set without a corresponding plot will have its
\ grace "symbol" and "line type" attributes set to "0".
: write_datasets_info ( -- )
    MAXSETS 0 DO
      I ds1 get_ds 0< IF leave THEN
      I [char] 0 + set_specifier 7 + c!
      I s>d <# # # #> xyplot_set_specifier 11 + swap cmove

      \ Write dataset name
      set_specifier 9 s" comment '" strcat
      ds1 DatasetInfo->Name a@ dup strlen strcat s" '" strcat rep(',") >grfile

      \ Write dataset header
      0 hdr_line_count !
      ds1 DatasetInfo->header a@ 
      dup strlen dup IF
        BEGIN
	  2dup 10 scan dup >r
	  2swap r> -  dup IF
	    1 hdr_line_count +!
	    xyplot_set_specifier 14 s" h000 " strcat 
            over 15 + >r hdr_line_count @ s>d <# # # # #> r> swap cmove    
            2swap strcat 255 min ( 2dup type cr) >grfile
          ELSE 2drop 
          THEN
	  dup IF 1 /string THEN
          dup 0=
	UNTIL
	2drop
      ELSE 2drop 
      THEN

      
      \ Write dataset plot info
      I pl1 get_plot 0 >= IF
        set_specifier 9 s" hidden false" strcat >grfile
        set_specifier 9 s" type xy" strcat >grfile
	set_specifier 9 s" symbol " strcat	  
	pl1 PlotInfo->Symbol @ 
        dup dup sym_POINT = swap 
	sym_LINE_PLUS_POINT = or swap
	sym_BIG_POINT = or dup flag_symbol ! 
	abs u>string count strcat >grfile

	flag_symbol @ IF
	  set_specifier 9 s" symbol size " strcat
	  pl1 PlotInfo->Symbol @ sym_BIG_POINT = IF s" 0.6" ELSE s" 0.2" THEN
          strcat >grfile
	  set_specifier 9 s" symbol color " strcat 
	  pl1 PlotInfo->Color @ 2+ u>string count strcat >grfile
	  set_specifier 9 s" symbol fill color " strcat
	  pl1 PlotInfo->Color @ 2+ u>string count strcat >grfile
	  set_specifier 9 s" symbol pattern 1" strcat >grfile
	  set_specifier 9 s" symbol fill pattern 1" strcat >grfile
	THEN

	set_specifier 9 s" line type " strcat
	pl1 PlotInfo->Symbol @ 
        dup  sym_LINE = swap sym_LINE_PLUS_POINT = or dup flag_line ! 	  
	abs u>string count strcat >grfile

	flag_line @ IF
	  set_specifier 9 s" linestyle " strcat
	  pl1 PlotInfo->Symbol @
          sym_DASHED = IF 3 ELSE 1 THEN u>string count strcat >grfile
	  set_specifier 9 s" linewidth 1.0" strcat >grfile
	  set_specifier 9 s" line color " strcat
	  pl1 PlotInfo->Color @ 2+ u>string count strcat >grfile
	THEN
      ELSE
        set_specifier 9 s" hidden true" strcat >grfile
        set_specifier 9 s" type xy"     strcat >grfile
        set_specifier 9 s" symbol 0"    strcat >grfile
        set_specifier 9 s" line type 0" strcat >grfile
      THEN
    LOOP
;

\ Write all xyplot data sets to grace file, whether or not they have
\ corresponding plots.
: write_datasets_xydata ( -- )
    MAXSETS 0 DO
      I ds1 get_ds 0< IF leave THEN
      s" @target G0.S" I u>string count strcat >grfile
      s" @type xy" >grfile
      ds1 DatasetInfo->Npts @ 0 ?DO
	I ds1 @xy 2>r 
	8 f>string count s"  " strcat 2r>
	8 f>string count strcat >grfile
      LOOP
      s" &" >grfile
    LOOP
;

: write_grace_file ( -- )
	s" # Grace project file (xyplot generated)" >grfile
	s" # " tdstring strcat >grfile
	s" @version 50114" >grfile
	s" @page size 792, 612" >grfile
	s" @background color 0" >grfile
	s" @page background fill on" >grfile

	write_grace_colormap
	write_grace_graph

	get_window_limits ymax f! xmax f! ymin f! xmin f!
	write_grace_window

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

	write_datasets_info
	write_datasets_xydata

\ Close the output file

	gr_fid @ close-file drop
;


: export_grace ( -- | prompt user for filename and export grace file )
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

\ Number of plot attributes to use for Grace datasets
8 constant NATTRIBUTES

\ Row indices for grace_pattrs{{
0 constant ATTR_HIDDEN
1 constant ATTR_SYMBOL
2 constant ATTR_SYMBOLSIZE
3 constant ATTR_SYMBOLCOLOR
4 constant ATTR_LINETYPE
5 constant ATTR_LINESTYLE
6 constant ATTR_LINEWIDTH
7 constant ATTR_LINECOLOR

MAXGRACEPTS 2            FLOAT   MATRIX  grace_xydata{{
MAXPLOTS    NATTRIBUTES  INTEGER MATRIX  grace_pattrs{{   \ plot attributes

create  grace_set_names  MAXSETS  80 * allot  \ buffer to store set names
create  grace_color_map  MAXPLOTS 32 * allot  \ buffer to store color names
create  grace_headers    MAXHDRSIZE MAXSETS * allot  \ buffer to store headers

: set_default_world_coords ( -- )
    -1e xmin f! -1e ymin f! 1e xmax f! 1e ymax f! ;

Public:

: parse_world_coords ( a u -- | parse world coordinates )
    10 /string
    2dup s" xmin" search IF
      5 /string bl skip >float IF xmin f! THEN 2drop EXIT
    ELSE 2drop THEN

    2dup s" xmax" search IF
      5 /string bl skip >float IF xmax f! THEN 2drop EXIT
    ELSE 2drop THEN

    2dup s" ymin" search IF
      5 /string bl skip >float IF ymin f! THEN 2drop EXIT
    ELSE 2drop THEN

    2dup s" ymax" search IF
      5 /string bl skip >float IF ymax f! THEN 2drop EXIT
    ELSE 2drop THEN
      
    parse_csv
    dup 4 <> if  0 ?do fdrop loop EXIT  then
    drop
    ymax f! xmax f! ymin f! xmin f!
;

\ Parse a set number from original line in file
: parse_set_number ( a1 u1 a2 u2 -- a1 u1 a2 u2 n )
    2over [char] s scan 1 /string drop 2 strpck string>s ;

\ Parse and store a set name
: parse_grace_name ( a1 u1 a2 u2 -- a1 u1 )
    parse_set_number >r  
    -trailing  9 /string 1- 79 min \ a2 u2  ( substring containing set name )
    r@ MAXSETS < IF
       r> 80 * grace_set_names + dup 80 erase 
       swap cmove
    ELSE  2drop  r> drop THEN 
;

: get_grace_set_name ( n -- a u | retrieve set name for set n)
    dup MAXSETS < IF    80 * grace_set_names + dup strlen 
                  ELSE  drop s" Unknown" 
                  THEN ;

: get_grace_header ( n -- a u | retrieve header string for set n)
    dup MAXSETS < IF    MAXHDRSIZE * grace_headers + dup strlen
                  ELSE  drop s" Grace Dataset"
                  THEN ; 

: parse_color_name ( a u -- | parse and store color name )
    -trailing 10 /string parse_token strpck string>s >r
    s" ), " search IF
       4 /string 1- 31 min  \ a2 u2  (substring containing color name)
       r@ MAXPLOTS < IF
         r> 32 * grace_color_map + 
         dup 32 erase
	 swap cmove
       THEN
    ELSE 2drop r> drop
    THEN
;

: get_grace_color ( n -- a u | retrieve color name of color n from map)
    dup MAXPLOTS < IF
      32 * grace_color_map + dup strlen
    ELSE
      drop s" black"  \ default color for unknown
    THEN ;

\ Parse grace set's hidden attribute: true = not visible, false = visible
: parse_visibility ( a1 u1 a2 u2 -- a1 u1 )
    2over s" @g" search IF 2drop 2drop EXIT THEN
    2drop
    parse_set_number grace_set !
    s" true" search
    grace_pattrs{{ grace_set @ ATTR_HIDDEN }} !
    2drop
;

\ Parse grace set's line attributes
: parse_line_attrs ( a1 u1 a2 u2 -- a1 u1 )
    parse_set_number grace_set !
    -trailing
    2dup s" line type" search IF
      10 /string strpck string>s 
      grace_pattrs{{ grace_set @ ATTR_LINETYPE }} !
      2drop EXIT
    ELSE 2drop THEN

    2dup s" line linestyle" search IF
      15 /string strpck string>s
      grace_pattrs{{ grace_set @ ATTR_LINESTYLE }} !
      2drop EXIT
    ELSE 2drop THEN

    2dup s" line linewidth" search IF
      15 /string bl skip >float if
        10e f* f>d drop
        grace_pattrs{{ grace_set @ ATTR_LINEWIDTH }} !
      then
      2drop EXIT
    ELSE 2drop THEN
    
    2dup s" line color" search IF
      11 /string strpck string>s 
      grace_pattrs{{ grace_set @ ATTR_LINECOLOR }} !
      2drop EXIT
    ELSE 2drop THEN
    
    2drop ;

\ Parse grace set's symbol attributes
: parse_symbol_attrs ( a1 u1 a2 u2 -- a1 u1 )
    parse_set_number grace_set !
    -trailing
    2dup s" symbol color" search IF
      13 /string strpck string>s 
      grace_pattrs{{ grace_set @ ATTR_SYMBOLCOLOR }} ! 
      2drop EXIT
    ELSE 2drop THEN
    
    2dup s" symbol size" search IF
      12 /string bl skip >float if
        10e f* f>d drop 
        grace_pattrs{{ grace_set @ ATTR_SYMBOLSIZE }} ! 
      then 
      2drop EXIT
    ELSE 2drop THEN

    2dup 7 /string bl skip 
    dup 1 = IF
      drop c@ [char] 0 - 
      grace_pattrs{{ grace_set @ ATTR_SYMBOL }} ! 
      2drop EXIT
    ELSE 2drop THEN
     
    2drop
;

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
      

\ n has following meaning:
\   n >= 0  xyplot set number
\   n < 0  error
: import_grace_set ( -- n )
    grace_set_npts @ 0> IF
      grace_set_npts @ ds1 DatasetInfo->Npts  !
      ds1 make_ds                     \ make the dataset
    ELSE
      -1
    THEN
;

\ Map grace plot attributes for current grace set to xyplot plot symbol and
\ plot type. Return true for success OR false with no attributes if grace
\ set is hidden
: map_grace_attr ( -- nsymbol color ntype true | false )

    grace_pattrs{{ grace_set @ ATTR_HIDDEN }} @ 
    IF false EXIT THEN  \ set is hidden

    \ grace plotting symbol/line type --> xyplot symbol

    grace_pattrs{{ grace_set @ ATTR_SYMBOL }} @ IF
      grace_pattrs{{ grace_set @ ATTR_LINETYPE }} @ IF
        sym_LINE_PLUS_POINT
      ELSE
        grace_pattrs{{ grace_set @ ATTR_SYMBOLSIZE }} @ 11 < IF 
          sym_POINT 
        ELSE 
          sym_BIG_POINT 
        THEN
      THEN
    ELSE
      grace_pattrs{{ grace_set @ ATTR_LINESTYLE }} @
      CASE
        0 of sym_LINE   endof
        1 of sym_LINE   endof
        2 of sym_LINE   endof
        3 of sym_DASHED endof
        4 of sym_LINE   endof
        5 of sym_LINE   endof
        6 of sym_LINE   endof
      ENDCASE
    THEN

    \ grace symbol/line color

    grace_pattrs{{ grace_set @ ATTR_SYMBOL }} @ IF
      ATTR_SYMBOLCOLOR  
    ELSE  
      ATTR_LINECOLOR  
    THEN
    grace_pattrs{{ grace_set @ rot }} @

    \ xyplot XY plot type
    0     
    true  \ plot is visible
;

: import_grace_plot ( n -- | make plot for xyplot set n)
    pl1 PlotInfo->Set !

    map_grace_attr IF
      pl1 PlotInfo->Type   !
      ( pl1 PlotInfo->Color  ! ) swap
      pl1 PlotInfo->Symbol !
      pl1 make_plot
      \ Workaround for mismatch of color maps between xyplot and Grace
      get_grace_color strpck set_plot_color
    THEN
;

\ err has the following meaning:
\   0  no error
\  -1  reached EOF before end of data set
\  -2  exceeded maximum number of points in a grace data set
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
        over c@ [char] & = IF  \ found end of set
          2drop
	  import_grace_set dup 0 >= IF
            import_grace_plot 
          ELSE  drop 
          THEN
	  0 EXIT
        ELSE
          parse_args 2 <> IF abort THEN  \ wrong number of values
	  grace_xydata{{ grace_set_npts @ 1 }} F!
	  grace_xydata{{ grace_set_npts @ 0 }} F!
	  1 grace_set_npts +!
        THEN
      ELSE
        2drop -1 EXIT   \ reached EOF before end of dataset
      THEN
      grace_set_npts @ MAXGRACEPTS >= IF -2 EXIT THEN
    AGAIN
;

 
Public:

: ?grace-comment-line ( a u -- b )
     over c@ [char] # = IF 		\ skip comment line except for @xyplot
       s" # @xyplot"  search IF parse_xyplot_extras ELSE 2drop THEN
       true
     ELSE 2drop false
     THEN
;

: parse_grace_line ( a u -- )
    2dup ?grace-comment-line 0= IF
      2dup s" @default"     search IF 2drop 2drop EXIT   ELSE 2drop THEN
      2dup s" @    line "   search IF 2drop 2drop EXIT   ELSE 2drop THEN
      2dup s" @map color"   search IF parse_color_name   ELSE 2drop THEN
      2dup s" @    world"   search IF parse_world_coords ELSE 2drop THEN
      2dup s" hidden "      search IF parse_visibility   ELSE 2drop THEN
      2dup s" comment"      search IF parse_grace_name   ELSE 2drop THEN
      2dup s"  line "       search IF parse_line_attrs   ELSE 2drop THEN
      2dup s"  symbol "     search IF parse_symbol_attrs ELSE 2drop THEN
      2dup s" @target G0.S" search IF
        12 /string strpck string>s
        read_grace_set IF
          ." read_grace_set error " cr
          gr_fid f@ close-file drop 
          abort 
        THEN
      ELSE 2drop THEN
    THEN
    2drop
;

: read_grace_file ( -- | assumes file has already been opened)
    0 grace_line_count !
    0 grace_set !
    grace_pattrs{{   MAXPLOTS NATTRIBUTES * cells erase
    grace_set_names  MAXSETS 80 * erase
    grace_headers    MAXSETS MAXHDRSIZE * erase
    set_default_world_coords

    BEGIN
      <grfile IF  \ -- a u
        parse_grace_line
      ELSE
        2drop
        reset_window
        xmin f@ ymin f@ xmax f@ ymax f@ set_window_limits
        gr_fid @ close-file drop EXIT
      THEN
    AGAIN ;


: import_grace ( -- | prompt user for filename and import grace file )
	c" *.agr" file_open_dialog 
	IF
	  count R/O open-file 0< IF drop
	    ." Unable to open output file!" 
	  ELSE
	    gr_fid ! read_grace_file
	  THEN
	ELSE drop
	THEN ;
 
\ add Grace file functions to the File Menu

MN_FILE  c" Export Grace File"   c" export_grace"  add_menu_item
MN_FILE  c" Import Grace File"   c" import_grace"  add_menu_item

End-Module

	

