\ anim1.4th
\
\  Simple animation demo of wavepacket propagation for xyplot.
\  This module demonstrates creating a new dataset, creating a 
\  plot associated with the set, and animation of the plot. 
\  The initial wavepacket moves to the left and, while doing so, 
\  becomes attenuated and broadened. A short optical pulse 
\  propagating in a dispersive and absorptive medium may show 
\  this kind of behavior.
\
\  Copyright (c) 1999--2009 Krishna Myneni
\  Provided under the GNU Lesser General Public License (LGPL)
\
\  Revisions:
\
\    2005-01-19  use signal handler to perform animation.  km
\    2005-01-20  updated for better efficiency of PROPAGATE1; drop
\                 the signal number passed to the handler.  km
\    2007-06-02  use FSL-style matrices  km
\    2007-09-28  use global constants for dataset type and plot symbol  km
\    2009-10-29  updated data structure field names  km

DatasetInfo dsanim 
PlotInfo    planim 
200 2 SFLOAT MATRIX anim{{ 
0 ptr xyplot_anim_buf

\ Fill out a dataset info structure on the new set

c" Animation" 1+ dsanim DatasetInfo->Name ! \ name of new dataset
c" Example of creating and animating a dataset" 1+ dsanim DatasetInfo->Header !
REAL_SINGLE dsanim DatasetInfo->Type  !	\ real single precision dataset
200 dsanim DatasetInfo->Npts  !		\ 200 points in the set
2 dsanim DatasetInfo->Size   !		\ dimension is 2 (x, y)
anim{{ dsanim DatasetInfo->Data !	\ pointer to data


\ Fill out a plot info structure on the new plot
	
0        planim PlotInfo->Type  !	\ plot type
sym_LINE planim PlotInfo->Symbol !	\ plot symbol
2        planim PlotInfo->Color  !	\ plot color

	   
fvariable phase
fvariable deltax
fvariable atten
fvariable width

: initialize1 ( -- | set initial params and data for animation waveform )
    0e phase  F!
    0e deltax F!
    1e atten  F!
    1e width  F!
    \ x goes from -10 to 10 in steps of 0.1 
    200 0 DO
	-10e .1e i s>f F* F+
	fdup    anim{{ I 0 }} SF! 
	0.1e F* anim{{ I 1 }} SF!
    LOOP
;


: propagate1 ( -- | modify the animation dataset )
    dsanim DatasetInfo->Npts @ 0 DO
	anim{{ I 0 }} SF@ deltax F@ F+ 
	fdup fdup F* fnegate width F@ F/ fexp fswap
	10e F* phase F@ F+ fcos F*
	atten F@ F*
	anim{{ I 1 }} SF!
    LOOP
    
    anim{{  xyplot_anim_buf  200 2 * SFLOATS move
    
    \ ?active set_ds_extrema
    deltax F@ 0.03e F+  deltax F!
    atten  F@ 0.99e F*  atten  F!
    width  F@ 0.04e F+  width  F!
    phase  F@ 0.20e F+  phase  F!
;


\ anim1 is the timer signal handler

: anim1 ( n -- | animate the active plot by modulating and translating it )
    drop              \ drop the signal number
    false draw_plot
    propagate1
    true draw_plot
;

: start_anim1 ( -- | start the animation )
    ?active dsanim get_ds
    dsanim DatasetInfo->Data a@ to xyplot_anim_buf
    -10e -1e 10e 1e set_window_limits
    initialize1
    ['] anim1  SIGALRM  forth-signal  drop
    100 100 set-timer    \ update waveform every 100 ms
;

: halt_anim1 ( -- | stop the animation )
    SIG_IGN  SIGALRM  forth-signal drop 
    ?active set_ds_extrema ;

initialize1
dsanim make_ds  planim PlotInfo->Set !	\ make a dataset in xyplot
planim make_plot	\ make a plot in xyplot 
0 0 set_grid_lines	\ turn off x and y axes grid lines

MN_FILE  c" Start Animation"  c" start_anim1"  add_menu_item
MN_FILE  c" Stop Animation"   c" halt_anim1"   add_menu_item

start_anim1			\ start the animation

