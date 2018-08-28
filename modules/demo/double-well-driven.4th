\ double-well-driven.4th
\
\  Double-Well Oscillator Driven with a Resonant Field
\  between its ground and first excited states.
\
\  Copyright (c) 2006 Krishna Myneni
\  Provided under the GNU General Public License
\
\  Revisions:
\    2010-01-21  km  updated for xyplot 2.4.x

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
  0 dspsi+ DatasetInfo->Type !
200 dspsi+ DatasetInfo->Npts !
  2 dspsi+ DatasetInfo->Size !
psi+_xydata dspsi+ DatasetInfo->Data !

c" First Excited State" 1+ dspsi- DatasetInfo->Name !
c" First Excited State of Double-Well Oscillator" 1+ dspsi- DatasetInfo->Header !
  0 dspsi- DatasetInfo->Type !
200 dspsi- DatasetInfo->Npts !
  2 dspsi- DatasetInfo->Size !
psi-_xydata dspsi- DatasetInfo->Data !


c" Driven State" 1+ dsanim DatasetInfo->Name ! \ name of new dataset
c" Evolution of State" 1+ dsanim DatasetInfo->Header !
0 dsanim DatasetInfo->Type !		\ real dataset
200 dsanim DatasetInfo->Npts !		\ 200 points in the set
2 dsanim DatasetInfo->Size !		\ dimension is 2 (x, y)
anim_xydata dsanim DatasetInfo->Data !	\ pointer to data


\ Fill out plot info structures on the new plots

0 plpsi+ PlotInfo->Type !
sym_LINE plpsi+ PlotInfo->Symbol !
0 plpsi+ PlotInfo->Color !

0 plpsi- PlotInfo->Type !
sym_LINE plpsi- PlotInfo->Symbol !
1 plpsi- PlotInfo->Color !

0 planim PlotInfo->Type !		\ plot type
sym_STICK planim PlotInfo->Symbol !	\ plot symbol
2 planim PlotInfo->Color !		\ plot color


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
	
	xtemp f@               psi+_xydata offset @ + sf!
	xtemp f@ psi+ fdup f*  psi+_xydata offset @ + SFLOAT + sf!

	xtemp f@               psi-_xydata offset @ + sf!
	xtemp f@ psi- fdup f*  psi-_xydata offset @ + SFLOAT + sf!
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

    fvariable temp
    fvariable t
    .3e fconstant dt
    -1e facos 2e f* 1e fswap f/ fconstant delta
    fvariable Omega                   \ Rabi rate
    delta 0.2e f* Omega f!
    
: evolve ( -- | modify the animation dataset )
    dsanim DatasetInfo->Npts @ 0 DO
	I dsanim @xy  fdrop
	2>r
	t f@ Omega f@ f* 2e f/ temp f!
	2r@ psi+
	2r@ psi-
	f* 2e f* delta t f@ f* fsin f*
	temp f@ fcos f*
	temp f@ fsin f*
	2r@ psi+ fdup f*
	temp f@ fcos fdup f* f*
	2r@ psi- fdup f*
	temp f@ fsin fdup f* f*
	f+ f+
	2r> fswap
	I dsanim !xy
    LOOP
    t f@ dt f+ t f!
;


: set_rabi ( -- | Set Rabi rate as a fraction of delta )
    c" What ratio do you want for Omega/delta?" get_input
    IF
	string>f delta f* Omega f!
    ELSE  drop 	THEN ;

variable update          \ update time in ms
100 update !

: set_update ( -- | Set time between plot updates )
    c" Time between updates (in milliseconds):" get_input
    IF
	string>s update !
    ELSE drop THEN ;

variable frame_counter
variable playback_delay
variable capture
FALSE capture !
create   window_id 16 allot

: capture-file-name ( n -- a u | return filename for capture file n)
    1000 +
    s" capture" rot s>string count 1 /string
    strcat s" .miff" strcat ; 

\ anim is the timer signal handler

variable psi+_plot#
variable psi-_plot#    
variable anim_plot#
    
: anim ( n -- | animate the active plot by modulating and translating it )
    drop              \ drop the signal number
    false draw_plot
    evolve
    psi+_plot# @ set_active_plot
    true draw_plot
    psi-_plot# @ set_active_plot
    true draw_plot
    anim_plot# @ set_active_plot
    true draw_plot
    ( draw_window)
    capture @ IF
	s" import -window " window_id count strcat
	s"  " strcat  frame_counter @ capture-file-name strcat
	strpck system drop
	1 frame_counter +!
    THEN
;

: start_anim ( -- | start the animation )
    ?active dsanim get_ds
    -4e 0e 4e 2e set_window_limits
    0e t f!
    0 frame_counter !
    capture @ IF
	c" Enter the window ID:" get_input
	IF window_id strcpy ELSE drop EXIT THEN
	c" Enter the playback delay between frames in 0.01 sec:" get_input
	IF string>s playback_delay ! ELSE drop EXIT THEN
    THEN
    
    ['] anim  SIGALRM  forth-signal  drop
    update @ dup set-timer    \ update waveform every 100 ms
;

: halt_anim ( -- | stop the animation )
    SIG_IGN  SIGALRM  forth-signal drop 
    ?active set_ds_extrema
    capture @ IF
	s" convert -delay " playback_delay @ s>string count strcat
	s"  *.miff capture.gif" strcat strpck system drop
	c" rm *.miff" system drop
	." Animation GIF created!"
    THEN
;

: toggle_capture ( -- )
    capture @ 0= dup capture !
    ." Capture is now "
    IF ." ON"  ELSE ." OFF" THEN ;

    
    0 0 set_grid_lines	\ turn off x and y axes grid lines

    init_wavefunctions
    dspsi+ make_ds plpsi+ PlotInfo->Set !
    plpsi+ make_plot
    get_active_plot psi+_plot# !
    dspsi- make_ds plpsi- PlotInfo->Set !
    plpsi- make_plot
    get_active_plot psi-_plot# !
    set_anim_data
    dsanim make_ds  planim PlotInfo->Set !	\ make a dataset in xyplot
    planim make_plot	\ make a plot in xyplot 
    get_active_plot anim_plot# !

    c" NH3 Animation 2" make_menu constant MN_NH3_2
    
    MN_NH3_2  c" Set Rabi Rate"        c" set_rabi"     add_menu_item
    MN_NH3_2  c" Set Update Time"      c" set_update"   add_menu_item    
    MN_NH3_2  c" Start Animation"      c" start_anim"   add_menu_item
    MN_NH3_2  c" Stop Animation"       c" halt_anim"    add_menu_item
    MN_NH3_2  c" Toggle Capture"       c" toggle_capture"  add_menu_item





