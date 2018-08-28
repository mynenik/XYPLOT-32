\ double-osc1.4th
\
\  Double-Well Oscillator in a superposition state
\
\  Copyright (c) 2006--2007 Krishna Myneni
\  Provided under the GNU General Public License
\
\  Revisions:
\    2010-01-21  km  updated for xyplot 2.4.x
\  

DatasetInfo dspsi+
PlotInfo    plpsi+
DatasetInfo dspsi-
PlotInfo    plpsi-
DatasetInfo dsanim 
PlotInfo    planim

200 2* SFLOATS constant BUFSIZE

create psi+_xydata  BUFSIZE allot
create psi-_xydata  BUFSIZE allot
create anim_xydata  BUFSIZE allot

\ Fill out dataset info structures on the sets

c" Ground State" 1+ dspsi+ DatasetInfo->Name !
c" Ground State of Double-Well Oscillator" 1+ dspsi+ DatasetInfo->Header !
REAL_SINGLE dspsi+ DatasetInfo->Type !
200 dspsi+ DatasetInfo->Npts !
  2 dspsi+ DatasetInfo->Size !
psi+_xydata dspsi+ DatasetInfo->Data !

c" First Excited State" 1+ dspsi- DatasetInfo->Name !
c" First Excited State of Double-Well Oscillator" 1+ dspsi- DatasetInfo->Header !
REAL_SINGLE dspsi- DatasetInfo->Type !
200 dspsi- DatasetInfo->Npts !
  2 dspsi- DatasetInfo->Size !
psi-_xydata dspsi- DatasetInfo->Data !


c" Superposition State" 1+ dsanim DatasetInfo->Name ! \ name of new dataset
c" Evolution of Superposition State" 1+ dsanim DatasetInfo->Header !
REAL_SINGLE dsanim DatasetInfo->Type  !	\ real, single precision dataset
200 dsanim DatasetInfo->Npts !		\ 200 points in the set
2 dsanim DatasetInfo->Size  !		\ dimension is 2 (x, y)
anim_xydata dsanim DatasetInfo->Data  !	\ pointer to data


\ Fill out plot info structures on the new plots

0 plpsi+ PlotInfo->Type !
sym_LINE plpsi+ PlotInfo->Symbol !
0 plpsi+ PlotInfo->Color !

0 plpsi- PlotInfo->Type !
sym_LINE plpsi- PlotInfo->Symbol !
1 plpsi- PlotInfo->Color !

0 planim PlotInfo->Type  !		\ plot type
sym_STICK planim PlotInfo->Symbol !	\ plot symbol
2 planim PlotInfo->Color  !		\ plot color


\ Ground state wavefunction of a harmonic oscillator, with (m*omega)/(2*hbar) = 1

: psi_0 ( fx -- fpsi )
    fdup f* fnegate fexp ;

1e fconstant a

\ Ground state of double-well oscillator: psi+(x) = psi_0(x+a) + psi_0(x-a).

: psi+ ( fx -- fpsi )
    fdup a f+ psi_0 fswap a f- psi_0 f+ ;

\ First excited state of double-well oscillator: psi-(x) = psi_0(x+a) - psi_0(x-a).

: psi- ( fx -- fpsi )
    fdup a f+ psi_0 fswap a f- psi_0 fswap f- ;


  -5e fconstant XMIN
   5e fconstant XMAX
0.05e fconstant XSTEP

variable offset
fvariable xtemp

: init_wavefunctions ( -- )
    200 0 DO
	I 2* SFLOAT * offset !
	I s>f XSTEP f* XMIN f+ xtemp f!
	
	xtemp f@       psi+_xydata offset @ + sf!
	xtemp f@ psi+  psi+_xydata offset @ + SFLOAT + sf!

	xtemp f@       psi-_xydata offset @ + sf!
	xtemp f@ psi-  psi-_xydata offset @ + SFLOAT + sf!
    LOOP
;

    
: set_anim_data ( -- | initialize data buffer with 200 pts of y=x/10 )
	\ x goes from -5 to 5 in steps of 0.05 
	200 0 DO
	  -5e .05e i s>f f* f+
	  fdup
	  anim_xydata i 2* SFLOAT * + 
	  dup >r sf! 
	  0.05e f* r> SFLOAT + sf!
	LOOP ;
	   
fvariable t
.1e fconstant dt
-1e facos 2e f* 1e fswap f/ fconstant delta

: evolve ( -- | modify the animation dataset )
    dsanim DatasetInfo->Npts @ 0 DO
	I dsanim @xy  fdrop
	2>r
	2r@ psi+
	2r@ psi-
	f* 2e f* delta t f@ f* fcos f*
	2r@ psi+ fdup f*
	2r@ psi- fdup f* f+ f+
	2e f/
	2r> fswap
	I dsanim !xy
    LOOP
    t f@ dt f+ t f!
;


\ anim1 is the timer signal handler

: anim1 ( n -- | animate the active plot by modulating and translating it )
    drop              \ drop the signal number
\    false draw_plot
    evolve
\    true draw_plot
    draw_window
;

: start_anim1 ( -- | start the animation )
	?active dsanim get_ds
	-5e -2e 5e 2e set_window_limits
	0e t f!
	['] anim1  SIGALRM  forth-signal  drop
	50 50 set-timer    \ update waveform every 50 ms
;

: halt_anim1 ( -- | stop the animation )
	SIG_IGN  SIGALRM  forth-signal drop 
	?active set_ds_extrema ;

    
    0 0 set_grid_lines	\ turn off x and y axes grid lines

    init_wavefunctions
    dspsi+ make_ds plpsi+ PlotInfo->Set !
    plpsi+ make_plot
    dspsi- make_ds plpsi- PlotInfo->Set !
    plpsi- make_plot
    
    set_anim_data
    dsanim make_ds  planim PlotInfo->Set !	\ make a dataset in xyplot
    planim make_plot	\ make a plot in xyplot 

c" Double Well"  make_menu  constant MN_DBL_WELL
MN_DBL_WELL  c" Start Animation"  c" start_anim1"  add_menu_item
MN_DBL_WELL  c" Stop Animation"   c" halt_anim1"   add_menu_item

\ start_anim1			\ start the animation




