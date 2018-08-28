\ anim3.4th
\
\  Copyright (c) 2005--2009 Krishna Myneni,
\  Creative Consulting for Research and Education
\
\  This software is provided under the GNU Lesser General Public License (LGPL)
\
\  Revisions:
\
\     2005-01-21  created  km
\     2007-08-19  modified to use FSL matrices km
\     2007-09-28  use global constants for dataset type and plot symbol  km
\     2009-10-29  updated data structure field names  km

DatasetInfo dscube 
PlotInfo    plcube 

 8 3 FLOAT MATRIX  cube_data{{      \ 3D cube data
 8 2 FLOAT MATRIX  cube_xydata{{    \ Tranformed x,y data for cube
16 2 FLOAT MATRIX  cube_plotdata{{  \ directed line plot data for drawing cube

\ Fill out a dataset info structure on the new set

c" CubeAnimation" 1+       dscube  DatasetInfo->Name !   \ name of new dataset
c" 3D Rotation of Cube" 1+ dscube  DatasetInfo->Header !
REAL_DOUBLE                dscube  DatasetInfo->Type  !  \ real dataset (double precision)
 16                        dscube  DatasetInfo->Npts  !  \ 10 points in the set
2                          dscube  DatasetInfo->Size  !  \ dimension is 2 (x, y)
cube_plotdata{{            dscube  DatasetInfo->Data  !  \ pointer to data

\ Fill out a plot info structure on the new plot

0        plcube PlotInfo->Type  !	\ plot type
sym_LINE plcube PlotInfo->Symbol !	\ plot symbol
2        plcube PlotInfo->Color !	\ plot color

\ Current view angles

fvariable theta       \ elevation angle in degrees
fvariable phi         \ azimuthal angle in degrees

fvariable sintheta
fvariable costheta
fvariable sinphi
fvariable cosphi

3 3 FLOAT MATRIX R-Matrix{{   \ rotation matrix

: >R-Matrix ( ftheta fphi -- | compute rotation matrix given angles )
    deg>rad  fsincos  cosphi   F!  sinphi   F!
    deg>rad  fsincos  costheta F!  sintheta F!

    costheta F@ cosphi F@ F*   costheta F@ sinphi F@ F*   sintheta F@ fnegate
    sinphi F@ fnegate          cosphi F@                  0e
    sintheta F@ cosphi F@ F*   sintheta F@ sinphi F@ F*   costheta F@

    3 3 R-Matrix{{ }}fput ; 

: Transform-Coords ( -- | apply rotation matrix to cube )
    8 0 DO
	R-Matrix{{ 0 0 }} F@  cube_data{{ I 0 }} F@ F*
	R-Matrix{{ 0 1 }} F@  cube_data{{ I 1 }} F@ F* F+
	R-Matrix{{ 0 2 }} F@  cube_data{{ I 2 }} F@ F* F+  \ x'
        cube_xydata{{ I 0 }} F!
	R-Matrix{{ 1 0 }} F@  cube_data{{ I 0 }} F@ F*
	R-Matrix{{ 1 1 }} F@  cube_data{{ I 1 }} F@ F* F+  
	R-Matrix{{ 1 2 }} F@  cube_data{{ I 2 }} F@ F* F+  \ y'
	cube_xydata{{ I 1 }} F!
    LOOP ; 

: cubexy>plotxy ( n m -- | copy point n from cube_xydata to 
    point m of cube_plotdata )
    >R >R
    cube_xydata{{ R@ 0 }} F@  cube_xydata{{ R> 1 }} F@
    cube_plotdata{{ R@ 1 }} F! cube_plotdata{{ R> 0 }} F!
;
	  

: xydata>linedata ( -- | generate a directed line plot for the cube)
    4 0 DO  I I cubexy>plotxy  LOOP  0 4  cubexy>plotxy  \ Top square
    8 4 DO  I I 1+ cubexy>plotxy  LOOP  4 9  cubexy>plotxy
    5 10  cubexy>plotxy
    1 11  cubexy>plotxy
    2 12  cubexy>plotxy
    6 13  cubexy>plotxy
    7 14  cubexy>plotxy
    3 15  cubexy>plotxy
;

: project_cube ( -- | rotate and project cube onto x,y plane)
    theta f@ phi f@ >R-Matrix   \ compute rotation matrix
    Transform-Coords            \ transform cube coordinates
    xydata>linedata             \ make 2D line drawing
;


: rotate_cube ( -- )
    project_cube
    dscube DatasetInfo->Npts @ 0 DO
	cube_plotdata{{ I 0 }} F@
	cube_plotdata{{ I 1 }} F@
	I dscube !xy
    LOOP
;


\ anim3 is the timer signal handler

: anim3 ( n -- )
    drop              \ drop the signal number
    false draw_plot
    rotate_cube
    true draw_plot
    theta f@ 1e f+ theta f!  \ increment by 1 degrees
    phi   f@ 1e f+ phi   f!  \ "
;

: start_anim3 ( -- | start the animation )
    ?active dscube get_ds
    -2e -2e 2e 2e set_window_limits
    ['] anim3  SIGALRM  forth-signal  drop
    100 100 set-timer    \ update drawing every 100 ms
;

: stop_anim3 ( -- | stop the animation )
    SIG_IGN  SIGALRM  forth-signal drop 
    ?active set_ds_extrema ;


\ initialize the 8 vertices of the unit length cube
\
\  x     y      z
 -0.5e  -0.5e   0.5e
  0.5e  -0.5e   0.5e
  0.5e   0.5e   0.5e
 -0.5e   0.5e   0.5e
 -0.5e  -0.5e  -0.5e
  0.5e  -0.5e  -0.5e
  0.5e   0.5e  -0.5e
 -0.5e   0.5e  -0.5e

8 3 cube_data{{ }}fput
\ Set initial view angles

0e theta f!
0e phi   f!

\ Compute the x, y plot data
project_cube
dscube make_ds  0 plcube PlotInfo->Set  !  \ make a dataset in xyplot
plcube make_plot	\ make a plot in xyplot 
0 0 set_grid_lines	\ turn off x and y axes grid lines
-2e -2e 2e 2e set_window_limits

MN_FILE  c" Start Cube"   c" start_anim3"  add_menu_item
MN_FILE  c" Stop  Cube"   c" stop_anim3"   add_menu_item

start_anim3			\ start the animation

