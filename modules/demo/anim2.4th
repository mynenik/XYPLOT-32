\ anim2.4th
\
\ Henon Map Animation for xyplot. Graphically track a ball of initial 
\   conditions under the transformation of a Henon Map.
\
\ Copyright (c) 2005--2009 Krishna Myneni
\ This software is provided under the GNU Lesser General Public License (LGPL)

( The Henon map is a two-dimensional map which can exhibit chaos.
  It is given by:

	x_n+1 = B*y_n - {x_n}^2 + A
	y_n+1 = x_n 

A and B are parameters, which for certain selections, give rise
to a chaotic sequence of values for x_n, y_n.

Here, we use A = 1.4, B = 0.3, which give chaos. The user
may experiment with other values of A and B as well. This
animation allows the user to observe how a set of points,
initially clustered very near to each other, evolves as
the map is applied. The "divergence of initial conditions"
and folding back of the points are characteristic properties
of chaos. In this example, the set of initial conditions
is a ball of points in the x-y plane, centered around a 
specified point. The display shows the evolution of the ball 
upon each iteration of the map. The ball of initial points
eventually disperses across the entire "attractor" for the
Henon map.

See also the kForth program  henon.4th.
)


fvariable A	1.4e A f!
fvariable B     0.3e B f!

: H ( x y -- xn yn | apply the Henon map to the input state)
    fover 2>r
    B f@ f*		\ B*y_n
    fswap fdup f* f- 	\ B*y_n - x_n^2
    A f@ f+ 2r> ;

fvariable theta
1e-3 fconstant ball_radius  ( chosen specifically for this map )

DatasetInfo dsball
PlotInfo plball
360 2 FLOAT MATRIX ellipse{{

\ Fill out a dataset info structure on the new set

c" Animation" 1+ dsball  DatasetInfo->Name !  \ name of new dataset
c" Evolution of Henon Map Initial Points" 1+ dsball  DatasetInfo->Header !
REAL_DOUBLE dsball  DatasetInfo->Type !	\ real dataset
360         dsball  DatasetInfo->Npts !	\ 360 points in the set
2           dsball  DatasetInfo->Size !	\ dimension is 2 (x, y)
ellipse{{   dsball  DatasetInfo->Data ! \ pointer to data


\ Fill out a plot info structure on the new plot

0         plball PlotInfo->Type !	\ plot type (x,y)
sym_POINT plball PlotInfo->Symbol !	\ plot symbol (point)
1         plball PlotInfo->Color  !	\ plot color

: init_ball ( x y -- | initialize the ball of initial conditions around point)
    dsball DatasetInfo->Npts @ 0 DO
	fover fover
	I s>f deg>rad theta f!
	fswap theta f@ fcos ball_radius f* f+	\ starting x
	fswap theta f@ fsin ball_radius f* f+	\ starting y
	I dsball !xy				\ store the evolved point
    LOOP 
    fdrop fdrop ;


\ evolve is the timer signal handler

: evolve ( n -- | apply Henon Map to each of the tracked points )
    drop              \ drop the signal number
    false draw_plot
    dsball DatasetInfo->Npts @ 0 DO
	I dsball @xy  H  I dsball !xy
    LOOP 
    true draw_plot ;

: start_henon_anim ( -- | start the animation )
    ?active dsball get_ds
    .2e .3e init_ball
    -2e -2e 2e 2e set_window_limits
    ['] evolve  SIGALRM  forth-signal drop
    500 500 set-timer  \ update points every 500 ms
;

: stop_henon_anim ( -- | stop the animation )
    SIG_IGN  SIGALRM  forth-signal  drop
    ?active set_ds_extrema ;

dsball make_ds  plball PlotInfo->Set  !
plball make_plot
0 0 set_grid_lines	

MN_FILE  c" Start Henon Animation"  c" start_henon_anim"  add_menu_item
MN_FILE  c" Stop Animation"         c" stop_henon_anim"   add_menu_item

start_henon_anim

