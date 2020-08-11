\ template.4th
\
\  Provide a Math Menu function to generate a dataset and plot
\  of the function y=x. Such a set may be operated upon, using
\  "Expression" from the Math Menu to generate plots of simple
\  functions.
\
\  Copyright (c) 2007--2020 Krishna Myneni
\  Provided under the GNU Lesser General Public License (LGPL)
\

Begin-Module

DatasetInfo ds
PlotInfo    pl

FLOAT DMATRIX templ{{ 

\ Fill out a dataset info structure on the template dataset

c" Template" 1+ ds DatasetInfo->Name ! \ name of new dataset
c" Y=X" 1+      ds DatasetInfo->Header !
REAL_DOUBLE ds DatasetInfo->Type  !	\ real dataset
2 ds DatasetInfo->Size  !		\ dimension is 2 (x, y)


\ Fill out a plot info structure on the new plot

0 pl PlotInfo->Type  !		\ plot type
sym_LINE pl PlotInfo->Symbol !	\ plot symbol

	   
fvariable fx1
fvariable fx2
fvariable fxdel

0 value Npts

Public:

\ Get template parameters from the user, generate the function, and
\   make a dataset and plot.
: xy_template ( -- )
    c" Enter x1, x2, and delta x:" get_input
    0= IF drop EXIT THEN

    \ Sanity checks on the user's input parameters
    
    count [char] , bl replace-char parse_args
    3 <> IF
	s" Incorrect number of arguments for Template." message_box
	abort
    THEN
    fxdel f! fx2 f! fx1 f!

    fxdel f@ F0= IF
	s" Delta x cannot be zero." message_box
	abort
    THEN

    fx1 f@ fx2 f@ f> IF fx1 f@ fx2 f@ fx1 f! fx2 f! THEN
    fxdel f@ fabs fxdel f!
    
    fx2 f@ fx1 f@ f- fxdel f@ f/ fround>s 1+ to Npts
  
    & templ{{ Npts 2 }}malloc

    fx1 f@
    Npts 0 DO  fdup templ{{ I 0 }} F!  fdup templ{{ I 1 }} F! fxdel f@ f+ LOOP
    fdrop

    Npts ds DatasetInfo->Npts !
    templ{{ ds DatasetInfo->Data !
 
    ds make_ds         \ make the dataset ( set number returned )
    dup pl PlotInfo->Set !  pl PlotInfo->Color ! 
    pl make_plot	\ make the corresponding plot  

    \ cleanup
    & templ{{ }}free
;


MN_MATH  c" Template"  c" xy_template reset_window"  add_menu_item

End-Module

