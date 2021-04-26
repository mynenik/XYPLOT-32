\ xutils.4th
\
\ xyplot Forth interface utilities
\
\ Copyright (c) 1999-2021 Krishna Myneni
\
\ This software is provided under the terms of the GNU
\ Lesser General Public License (LGPL)
\
\ Glossary:
\
\   GET_SEL_SETINFO2 ( ds1addr ds2addr -- nerror )
\     Retrieve selected data sets information into two dataset
\     info structures: the active dataset info is copied to 
\     ds1addr and the operand dataset info is copied to ds2addr.
\     Return zero if successful, or a nonzero error code.
\
\   DS_EXTREMA   ( dsaddr -- ) 
\     Set variables fxmin, fxmax, fymin, fymax given DatasetInfo 
\     structure address.
\
\   FINDX        ( rx dsaddr -- n )
\     Return index of data point with x value closest to rx.
\
\   INDEX_LIMITS ( dsaddr -- n1 n2 ) 
\     Return indices of data points with abscissas nearest to the
\     window domain limits: xmin, xmax.
\
\   OVERLAP      ( dsaddr1 dsaddr2 -- flag )
\     Find the overlap interval for two data sets. Return a true 
\     flag if the two sets have overlap in x. The overlap interval
\     is stored in the fvariables fx1 and fx2.
\
\   INDEX_RANGE  ( -- n1 n2 )
\     Return the indices of data points in the active data set which
\     correspond to its overlap interval with the operand data set.
\
\   COPY_SF_TO_DF ( asrc adest u -- )
\     Copy u single floats from asrc to u double floats at adest.
\
\ Revisions:
\   2009-10-28  km  revised data structure members; moved TDSTRING
\                   from grace.4th to this module.
\   2020-07-09  km  moved TDSTRING to utils.4th
\   2021-04-24  km  implemented INDEX_LIMITS
\   2021-04-26  km  added COPY_SF_TO_DF and GET_SEL_SETINFO2

DatasetInfo ds1
DatasetInfo ds2

\ Retrieve data set info for active and operand sets into
\ dataset info structures, ds1addr and ds2addr, respectively.
\ Return zero if successful, or non-zero error code.
: get_sel_setinfo2 ( ds1addr ds2addr -- nerror )
    >r ?active dup 0 >= IF
       swap get_ds drop
    ELSE
      2drop r> drop -1 EXIT
    THEN
    r> 
    ?operand dup 0 >= IF
       swap get_ds drop
    ELSE
      2drop -2 EXIT
    THEN
    0
;

fvariable fxmin
fvariable fxmax
fvariable fymin
fvariable fymax

: ds_extrema ( dsaddr1 -- | determine the dataset extrema )
	\ results are stored in the variables fxmin, fxmax, fymin, fymax
	dup 0 swap @xy fdup fymax f! fymin f! fdup fxmax f! fxmin f!
	dup DatasetInfo->Npts @ 1 do
	  i over @xy
	  fdup fymin f@ fmin fymin f!
	  fymax f@ fmax fymax f!
	  fdup fxmin f@ fmin fxmin f!
	  fxmax f@ fmax fxmax f!
	loop
	drop ;


variable dstemp
variable npts
variable ordering
fvariable fdel1
variable jtemp

\ Find point with closest x to fx within a dataset and
\ return its zero-based index in the set.
: findx ( fx dsaddr -- n )
	\ dsaddr is the address of a dataset info structure
	dup dstemp ! DatasetInfo->Npts @ npts !	
	0 dstemp a@ @xy fdrop
	npts @ 1- dstemp a@ @xy fdrop
	f< ordering !
	npts @ 0
	begin
	  2dup - 1 >
	while
	  2dup + 2/ -rot
	  >r >r 
	  dup dstemp a@ @xy fdrop rot 
	  >r fover f<= ordering @ =
	  r> swap r> r> rot
	  if drop swap else nip then 
	repeat
	nip dup jtemp !
	dstemp a@ @xy fdrop fover f- fabs fdel1 f!
	jtemp @ npts @ 1- < if
	  jtemp @ 1+ dstemp a@ @xy fdrop f- fabs
	  fdel1 f@ f< if 1 jtemp +! then
	else
	  fdrop
	then
	jtemp @ ;  

\ Return indices from a dataset for x-values spanning the window domain.
\ The returned indices are ordered such that n1 <= n2; it is the
\ responsibility of calling word to check for condition n1 = n2.
: index_limits ( dsaddr -- n1 n2 )
    >r get_window_limits 
    fdrop fxmax f! fdrop fdup fxmin f!
    r@ findx
    fxmax f@ r> findx
    2dup > IF swap THEN ;

fvariable fx1
fvariable fx2

: overlap ( dsaddr1 dsaddr2 -- flag | determine the overlap of two datasets )
	\ store the overlap region in fx1 and fx2
	\ flag is true if the regions overlap
	ds_extrema fxmin f@ rot fxmax f@ rot
	ds_extrema fxmax f@ fmin fx2 f!
	fxmin f@ fmax fx1 f!
	fx1 f@ fx2 f@ f<= ;

: index_range ( -- n1 n2 | return starting and ending indices of overlap region in active set )
	?active
	dup 0 >= if
	  ds1 get_ds drop
	  ?operand
	  dup 0 >= if
	    ds2 get_ds drop
	    ds1 ds2 overlap		\ determine if sets overlap
	    if
	      fx1 f@ ds1 findx		\ find starting and ending indices of
	      fx2 f@ ds1 findx		\ overlap region in active dataset
	      1+
	      2dup > if swap then	\ swap if descending order
	      2dup = if 1+ then		\ adjust to prevent infinite loop
	      exit
	    then
	  then
	  drop
	then
	drop
	0 0 ;
	
\ Copy u single floats from asrc to u double floats at adest
: Copy_SF_To_DF ( asrc adest u -- )
    0 ?DO 2dup >r sf@ r> df! dfloat+ >r sfloat+ r> LOOP
    2drop ;

