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
\ This file is Copyright (c) 2003--2020 Krishna Myneni
\ Provided under the GNU Affero General Public License
\
\ Please report bugs to  <krishna.myneni@ccreweb.org>
\
Begin-Module

Public:

   32  constant  MAXSETS
   32  constant  MAXPLOTS
65536  constant  MAXGRACEPTS
16384  constant  MAXHDRSIZE

fvariable xmin
fvariable xmax
fvariable ymin
fvariable ymax

: rep(',") ( a u -- a u | replace every single quote with double quote in string)
	2dup 0 ?do dup c@ [char] ' = if [char] " over c! then 1+ loop drop ;

: rep(comma,space) ( a u -- a u | replace every comma with a space in string)
	2dup 0 ?do dup c@ [char] , = if bl over c! then 1+ loop drop ;

\ Number <=> string conversions, enforcing conversion in base 10
\ (uses words from strings.4th ).
: u>$ ( u1 -- a u2 ) u>string count  ;
: $>s ( a u -- n )   strpck string>s ;
: f>$ ( r -- a u )   f>string count  ;
: $>f ( a u -- r )   strpck string>f ;

: parse_csv ( a u -- r1 ... rn n )
    rep(comma,space) parse_args ;

: dos2unix-line ( a u -- a u2 )
    2dup 1- + c@ 13 = IF 1- THEN ;

variable gr_fid
variable grace_line_count
	  
: >grfile ( a u -- | write string to grace file )
    gr_fid @ write-line drop ;

create grfile_buf 256 allot

: <grfile ( -- a u b | read string from grace file )
    grfile_buf 256 erase
    grfile_buf 256 gr_fid @ read-line drop \ -- u b
    >r grfile_buf swap dos2unix-line r>
    \ grfile_buf -rot
    1 grace_line_count +! ;


MAXPLOTS PlotInfo% %size ARRAY PlotList{
variable nplots  \ number of plots in plot list (set by get_plot_list)

MAXPLOTS INTEGER ARRAY PlotsForSet{
variable nplots_for_set

Public:

\ Obtains an array of PlotInfo structures corresponding to
\ the current XYPLOT plot list. The number of plots in the
\ plot list is stored in NPLOTS; however, the user can
\ also determine the last valid array element by checking
\ the element's data set number to see if it is >= 0.

: get_plot_list ( -- )
    0 nplots !
    \ Initialize PlotList{ array with invalid set number
    MAXPLOTS 0 DO -1 PlotList{ I } PlotInfo->Set ! LOOP

    MAXPLOTS 0 DO
      I PlotList{ I } get_plot 0< IF leave THEN
      1 nplots +!
    LOOP
;

\ For data set n, make a list of the plot numbers for the set,
\ and return the number of plots in the list. GET_PLOT_LIST
\ must be called prior to using this.
: plots_for_set ( n -- u )
    MAXPLOTS 0 DO 0 PlotsForSet{ I } ! LOOP
    0 nplots_for_set !
    nplots @ 0 ?DO
      dup PlotList{ I } PlotInfo->Set @ = IF
	I PlotsForSet{ nplots_for_set @ } ! 
        1 nplots_for_set +! 
      THEN
    LOOP
    drop
    nplots_for_set @
;

\ Color mapping for Grace file output
32 constant MAX_XYCOLORS        \ must be at least 16
16 constant MAX_GRCOLORS
16 constant MAX_COLORNAME_LEN

MAX_GRCOLORS INTEGER            ARRAY gr_rgb{
MAX_GRCOLORS MAX_COLORNAME_LEN  ARRAY gr_colors{

: RGB>COLORREF ( ur ug ub -- ucolorref | pack RGB values into colorref value)
    16 lshift >r 8 lshift or r> or ;

: COLORREF>RGB ( ucolorref -- ur ug ub | unpack RGB color value)
    dup 16 rshift 255 and >r
    dup  8 rshift 255 and >r
    255 and r> r> ;

MAX_XYCOLORS INTEGER ARRAY xy_rgb{
MAX_XYCOLORS MAX_COLORNAME_LEN ARRAY xy_colors{

: default_xy_colormap ( -- )
    s" blue"           0   0 255  
    s" red"          255   0   0 
    s" green"          0 128   0  
    s" turquoise"     64 224 208  
    s" magenta"      255   0 255 
    s" sienna"       160  82  45 
    s" gold"         255 215   0 
    s" purple"       128   0 128
    s" orange"       255 165   0 
    s" seagreen"      46 139  87
    s" slategray"    112 128 144
    s" coral"        255 127  80
    s" black"          0   0   0
    s" aquamarine"   127 255 212
    s" steelblue"     70 130 180
    s" orchid"       218 112 214
    s" salmon"       250 128 114
    s" midnightblue"  25  25 112
    s" darkgreen"      0 100   0
    s" cyan"           0 255 255
    s" slateblue"    106  90 205
    s" limegreen"     50 205  50
    s" yellow"       255 255   0
    s" blueviolet"   138  43 226
    s" brown"        165  42  42
    s" crimson"      220  20  60
    s" springgreen"    0 255 127
    s" chestnut"     205  92  92
    s" deepskyblue"    0 191 255
    s" lightgray"    211 211 211
    s" wheat"        245 222 179
    s" white"        255 255 255
    0 MAX_XYCOLORS 1-  DO 
      RGB>COLORREF xy_rgb{ I } !  
      xy_colors{ I } swap cmove 
    -1 +LOOP
;

: default_gr_colormap ( -- )
    s" white"        255 255 255
    s" black"          0   0   0
    s" red"          255   0   0
    s" green"          0 255   0
    s" blue"           0   0 255
    s" yellow"       255 255   0
    s" brown"        188 143 143
    s" turquoise"     64 224 208  
    s" magenta"      255   0 255 
    s" sienna"       160  82  45 
    s" gold"         255 215   0 
    s" purple"       128   0 128
    s" orange"       255 165   0 
    s" seagreen"      46 139  87
    s" slategray"    112 128 144
    s" coral"        255 127  80
    0 MAX_GRCOLORS 1-  DO 
      RGB>COLORREF gr_rgb{ I } !  
      gr_colors{ I } swap cmove 
    -1 +LOOP
;


\ Setup XYPLOT and Grace output color maps
: setup_colormaps ( -- )
    gr_colors{ MAX_GRCOLORS MAX_COLORNAME_LEN * erase
    default_gr_colormap

    \ Get XYPLOT's current color map
    xy_rgb{ xy_colors{ MAX_COLORNAME_LEN MAX_XYCOLORS get_color_map
    MAX_XYCOLORS < IF 
      default_xy_colormap  \ Use a default color map on error
    THEN 

    \ For each of the plots in the PlotList, look up the xyplot
    \   COLORREF values and names, and place those in the grace
    \   output colormap.
    \ (For now, simply copy the first 14 colors into the grace output map)
    MAX_GRCOLORS 2 DO
      xy_rgb{ I 2- } @ gr_rgb{ I } !
      xy_colors{ I 2- } gr_colors{ I } MAX_COLORNAME_LEN cmove
    LOOP
;

setup_colormaps

variable rgb1
variable rgb2

: color-distance ( ucolorref1 ucolorref2 -- rdist )
    rgb2 ! rgb1 !
    rgb1 @ COLORREF>RGB 2drop
    rgb2 @ COLORREF>RGB 2drop
    - dup *
    rgb1 @ COLORREF>RGB drop nip
    rgb2 @ COLORREF>RGB drop nip
    - dup * +
    rgb1 @ COLORREF>RGB nip nip
    rgb2 @ COLORREF>RGB nip nip
    - dup * +
    s>f fsqrt
;

variable idx_nearest
fvariable distance

\ Return nearest index of rgbcolor in the xyplot colormap
: nearest_xyplot_color ( ucolorref -- idx )
    255 dup * 3 * s>f fsqrt distance f!
    0 idx_nearest !
    MAX_XYCOLORS 0 DO
      dup xy_rgb{ I } @ color-distance
      fdup distance f@ f< IF distance f! I idx_nearest ! ELSE  fdrop  THEN
    LOOP
    drop
    idx_nearest @ ;

: write_grace_pageinfo ( -- )
    s" @page size 792, 612"      >grfile
    s" @background color 0"      >grfile
    s" @page background fill on" >grfile
;
   
: write_grace_colormap ( -- )
    MAX_GRCOLORS 0 DO
      s" @map color " I u>$ strcat s"  to (" strcat
      gr_rgb{ I } @ COLORREF>RGB >r >r
      u>$ strcat s" ," strcat
      r> u>$ strcat s" ," strcat
      r> u>$ strcat s" ), '" strcat
      gr_colors{ I } dup strlen strcat
      s" '" strcat rep(',") >grfile
    LOOP ;

: write_xyplot_window ( -- )
    get_window_limits ymax f! xmax f! ymin f! xmin f!
    s" @    world xmin " xmin f@ 6 f>$  strcat >grfile
    s" @    world xmax " xmax f@ 6 f>$  strcat >grfile
    s" @    world ymin " ymin f@ 6 f>$  strcat >grfile
    s" @    world ymax " ymax f@ 6 f>$  strcat >grfile
;

\ XYPLOT exported Grace files will contain a single graph, G0.
: write_xyplot_graph ( -- )
    s" @g0 on"           >grfile
    s" @g0 type XY"      >grfile
    s" @with g0"         >grfile
    s" @g0 hidden false" >grfile
;

: write_xyplot_axes ( -- )
    s" @    xaxis on"      >grfile
    s" @    xaxis tick on" >grfile 
    s" @    xaxis tick major " 
    xmax f@ xmin f@ f- 10e f/ 
    6 f>$ strcat >grfile
    s" @    xaxis ticklabel on" >grfile
    s" @    yaxis on"      >grfile
    s" @    yaxis tick on" >grfile
    s" @    yaxis tick major " 
    ymax f@ ymin f@ f- 10e f/ 
    6 f>$ strcat >grfile
    s" @    yaxis ticklabel on" >grfile

    s" @    frame type 0"  >grfile
;

Private:

create gr_set_label  12 allot
create xy_set_label  14 allot

s" @    s??    "   gr_set_label swap cmove 
s" # @xyplot s?? " xy_set_label swap cmove

Public:

: $gr_set gr_set_label  9 ;
: $xy_set xy_set_label 14 ;

: make_set_labels ( u -- )
    dup 0 MAXSETS 1+ within IF
      s>d <# # # #>
    ELSE
      drop s" ??"
    THEN 
    2dup
    gr_set_label  6 + swap cmove
    xy_set_label 11 + swap cmove
;


Public:

\ map XYPLOT plot symbol to grace symbol flag
: xysym_to_grsym ( nsymbol -- 0 | 1 )
    dup  sym_POINT = 
    over sym_LINE_PLUS_POINT = or
    over sym_BIG_POINT = or
    nip abs
;

\ map XYPLOT plot symbol to grace linetype
: xysym_to_grltype ( nsymbol -- nlinetype )
    CASE
      sym_LINE            OF  1  ENDOF
      sym_LINE_PLUS_POINT OF  1  ENDOF
      sym_DASHED          OF  1  ENDOF
      sym_HISTOGRAM       OF  2  ENDOF
      sym_STICK           OF  1  ENDOF
      \ other: linetype = 0
      >r 0 r>
    ENDCASE
;

\ Write the Grace set's symbol properties
\ ucolor is an rgb value
: write_symbol_info ( nsymbol ucolor -- )
    nearest_xyplot_color
    $gr_set s" symbol size " strcat
    2over drop sym_BIG_POINT = IF 
      s" 0.6" ELSE s" 0.2" THEN  strcat >grfile

    $gr_set s" symbol color " strcat 
    2over nip u>$ strcat >grfile
    $gr_set s" symbol fill color " strcat
    2over nip u>$ strcat >grfile

    $gr_set s" symbol pattern 1"      strcat >grfile
    $gr_set s" symbol fill pattern 1" strcat >grfile
    2drop
;

\ Write the Grace set's line properties
: write_line_info ( nsymbol ucolor -- )
    nearest_xyplot_color
    $gr_set s" linestyle " strcat
    2over drop sym_DASHED = IF 3 ELSE 1 THEN
    u>$ strcat >grfile

    $gr_set s" linewidth 1.0" strcat >grfile

    $gr_set s" line color " strcat
    2over nip u>$ strcat >grfile
    2drop
;

Private:

variable  gr_symbol
variable  gr_linetype
variable  hdr_line_count
PlotInfo    pl1
DatasetInfo ds1

Public:


\ Write info about all xyplot data sets and their corresponding plot
\ attributes. A data set without a corresponding plot will have its
\ grace "symbol" and "line type" attributes set to "0".
\ In xyplot, there may be multiple plots associated with a data set.
\ However, Grace permits only one plot for a set, so we simply write
\ the plot attributes for the first plot of the set.
: write_datasets_info ( -- )
    get_plot_list

    MAXSETS 0 DO
      I ds1 get_ds 0< IF leave THEN  \ dataset exists in xyplot?
      I make_set_labels

      \ Write dataset name
      $gr_set s" comment '" strcat
      ds1 DatasetInfo->Name a@ dup strlen strcat 
      s" '" strcat rep(',") >grfile

      \ Write dataset header
      0 hdr_line_count !
      ds1 DatasetInfo->header a@ 
      dup strlen dup IF
        BEGIN
	  2dup 10 scan dup >r
	  2swap r> -  dup IF
	    1 hdr_line_count +!
	    $xy_set s" h000 " strcat 
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
      I plots_for_set 0> IF
        PlotsForSet{ 0 } @ pl1 get_plot 0< IF
          ." Error obtaining plot information for set " I . cr
          gr_fid @ close-file drop
          abort
        THEN

        $gr_set s" hidden false" strcat >grfile
        $gr_set s" type xy"      strcat >grfile

	pl1 PlotInfo->Symbol @ dup 
	xysym_to_grsym   gr_symbol ! 
        xysym_to_grltype gr_linetype !

        $gr_set s" symbol " strcat	  
        gr_symbol @ u>$ strcat >grfile

        gr_symbol @ IF
          pl1 PlotInfo->Symbol @  pl1 PlotInfo->Color @  
          write_symbol_info
        THEN

	$gr_set s" line type " strcat	  
	gr_linetype @ u>$ strcat >grfile

	gr_linetype @ IF
          pl1 PlotInfo->Symbol @  pl1 PlotInfo->Color @  
          write_line_info
	THEN

      ELSE
        $gr_set s" hidden true" strcat >grfile
        $gr_set s" type xy"     strcat >grfile
        $gr_set s" symbol 0"    strcat >grfile
        $gr_set s" line type 0" strcat >grfile
      THEN
    LOOP
;

\ Write all xyplot data sets to grace file, whether or not they have
\ corresponding plots.
: write_datasets_xydata ( -- )
    MAXSETS 0 DO
      I ds1 get_ds 0< IF leave THEN
      s" @target G0.S" I u>$ strcat >grfile
      s" @type xy" >grfile
      ds1 DatasetInfo->Npts @ 0 ?DO
	I ds1 @xy 2>r 
	8 f>$ s"  " strcat 2r>
	8 f>$ strcat >grfile
      LOOP
      s" &" >grfile
    LOOP
;

: write_grace_file ( -- )
    s" # Grace project file (xyplot generated)" >grfile
    s" # " tdstring strcat >grfile
    s" @version 50114"     >grfile

    write_grace_pageinfo
    write_grace_colormap
    write_xyplot_graph
    write_xyplot_window
    write_xyplot_axes
    write_datasets_info
    write_datasets_xydata
;

: export_grace ( -- | prompt user for filename and export grace file )
    c" Enter the Grace (.agr) filename:" get_input
    IF
      count 2dup ." Exporting Grace file, " type cr 
      W/O create-file 0 < IF  
        ." Unable to create output file!" cr
      ELSE 
        gr_fid ! write_grace_file
        gr_fid @ close-file drop
        ." Grace file successfully written." cr 
      THEN
    THEN 
;

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
create  grace_headers    MAXHDRSIZE MAXSETS * allot  \ buffer to store headers

: set_default_world_coords ( -- )
    -1e xmin f! -1e ymin f! 1e xmax f! 1e ymax f! ;

Public:

: parse_world_coords ( a u -- | parse world coordinates )
    ( 2dup type cr )
    7 /string
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
    2over [char] s scan 1 /string drop 2 $>s ;

\ Parse and store a set name
: parse_grace_name ( a1 u1 a2 u2 -- a1 u1 )
    parse_set_number >r  
    -trailing  9 /string 1- 79 min \ a2 u2  ( substring containing set name )
    r@ MAXSETS < IF
       r> 80 * grace_set_names + dup 80 erase 
       swap cmove
    ELSE  2drop  r> drop THEN 
;

: get_grace_set_name ( n -- a u | retrieve name for set n)
    dup MAXSETS < IF    80 * grace_set_names + dup strlen 
                  ELSE  drop s" Unknown" 
                  THEN ;

: get_grace_header ( n -- a u | retrieve header for set n)
    dup MAXSETS < IF    MAXHDRSIZE * grace_headers + dup strlen
                  ELSE  drop s" Grace Dataset"
                  THEN ; 

variable color_idx

: parse_colormap_color ( a u -- | parse and store rgb value and color name )
    -trailing 10 /string parse_token $>s color_idx !
    s" (" search IF
      1 /string bl skip
      0 0 2swap >number  [char] , skip bl skip
      0 0 2swap >number  [char] , skip bl skip
      0 0 2swap >number  2>r
      d>s >r d>s >r d>s r> r>  RGB>COLORREF
      gr_rgb{ color_idx @ MAX_GRCOLORS 1- min } ! \ store colorref
      2r> s" ), " search IF
        4 /string 1- MAX_COLORNAME_LEN min  \ a2 u2  ( color name substring )
        gr_colors{ color_idx @ MAX_GRCOLORS 1- min } 
        dup MAX_COLORNAME_LEN erase swap cmove
      ELSE 2drop
      THEN
    ELSE 2drop
    THEN
;

\ For diagnostics
: show_grace_colormap ( -- )
    MAX_GRCOLORS 0 do
      gr_rgb{ I } @ 8 .r 2 spaces
      gr_colors{ I } dup strlen type cr
    LOOP
;

: get_grace_color ( n -- a u | retrieve color name of color n from map)
    dup MAX_GRCOLORS < IF
      gr_colors{ swap } dup strlen
    ELSE
      drop s" black"  \ default color for unknown
    THEN ;

\ Parse grace set's hidden attribute: true = not visible
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
      10 /string $>s 
      grace_pattrs{{ grace_set @ ATTR_LINETYPE }} !
      2drop EXIT
    ELSE 2drop THEN

    2dup s" line linestyle" search IF
      15 /string $>s
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
      11 /string $>s 
      grace_pattrs{{ grace_set @ ATTR_LINECOLOR }} !
      2drop EXIT
    ELSE 2drop THEN
    
    2drop ;

\ Parse grace set's symbol attributes
: parse_symbol_attrs ( a1 u1 a2 u2 -- a1 u1 )
    parse_set_number grace_set !
    -trailing
    2dup s" symbol color" search IF
      13 /string $>s 
      grace_pattrs{{ grace_set @ ATTR_SYMBOLCOLOR }} ! 
      2drop EXIT
    ELSE 2drop THEN
    
    2dup s" symbol size" search IF
      12 /string bl skip >float IF
        10e f* f>d drop 
        grace_pattrs{{ grace_set @ ATTR_SYMBOLSIZE }} ! 
      THEN 
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
    9 /string over 2 $>s  hdr_set !
    4 /string over 3 $>s  hdr_line !    
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

variable xyp_symbol

\ Map grace plot attributes for current grace set to xyplot plot symbol and
\ plot type. Return true for success OR false with no attributes if grace
\ set is hidden
: map_grace_attr ( -- nsymbol color ntype true | false )

    grace_pattrs{{ grace_set @ ATTR_HIDDEN }} @ 
    IF false EXIT THEN  \ set is hidden

    \ grace plotting symbol/line type --> xyplot symbol

    sym_LINE xyp_symbol !  \ defaults to line
    grace_pattrs{{ grace_set @ ATTR_SYMBOL }} @ IF
      grace_pattrs{{ grace_set @ ATTR_LINETYPE }} @ IF
        sym_LINE_PLUS_POINT xyp_symbol !
      ELSE
        grace_pattrs{{ grace_set @ ATTR_SYMBOLSIZE }} @ 11 < IF 
          sym_POINT xyp_symbol !
        ELSE 
          sym_BIG_POINT xyp_symbol !
        THEN
      THEN
    ELSE
      grace_pattrs{{ grace_set @ ATTR_LINETYPE }} @
      CASE
        1 OF sym_LINE       xyp_symbol ! ENDOF
        2 OF sym_HISTOGRAM  xyp_symbol ! ENDOF
        3 OF sym_HISTOGRAM  xyp_symbol ! ENDOF
      ENDCASE

      grace_pattrs{{ grace_set @ ATTR_LINESTYLE }} @
      CASE
        3 of sym_DASHED xyp_symbol ! endof
      ENDCASE
    THEN

    xyp_symbol @

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
      \ gr_rgb{ swap } @ nearest_xyplot_color  \ index in xyp colormap
      \ xy_rgb{ swap } @ pl1 PlotInfo->Color  !
      swap
      pl1 PlotInfo->Symbol !
      pl1 make_plot
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
      2dup s" @map color"   search IF parse_colormap_color   ELSE 2drop THEN
      2dup s"   world"      search IF parse_world_coords ELSE 2drop THEN
      2dup s" hidden "      search IF parse_visibility   ELSE 2drop THEN
      2dup s" comment"      search IF parse_grace_name   ELSE 2drop THEN
      2dup s"  line "       search IF parse_line_attrs   ELSE 2drop THEN
      2dup s"  symbol "     search IF parse_symbol_attrs ELSE 2drop THEN
      2dup s" @target G0.S" search IF
        12 /string $>s
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

create grace_title 64 allot

: import_grace ( -- | prompt user for filename and import grace file )
	grace_title 64 erase
	c" *.agr" file_open_dialog
	IF
	  count
	  2dup 63 min grace_title swap cmove 
          2dup ." Importing Grace file, " type cr 
	  R/O open-file 0< IF drop
	    ." Unable to open input file!" 
	  ELSE
	    gr_fid ! read_grace_file
	  THEN
	ELSE drop
	THEN ;
 
\ add Grace file functions to the File Menu

MN_FILE  c" Export Grace File"   c" export_grace"  add_menu_item
MN_FILE  c" Import Grace File"   c" import_grace"  add_menu_item

End-Module

