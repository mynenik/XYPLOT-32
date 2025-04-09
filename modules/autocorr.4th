\ autocorr.4th
\
\ Auto-correlation function for xyplot
\
\ Copyright (c) 2000--2012 Krishna Myneni
\ Provided under the GNU Lesser General Public License (LGPL)
\
\ Revisions:
\    2000-03-06  created  km
\    2005-01-14  updated use of DatasetInfo structure  km
\    2009-10-29  updated data structure field names  km
\    2012-06-26  convert to unnamed module  km

Begin-Module

DatasetInfo ds1    \ active dataset info structure; ds1 should exist
DatasetInfo ds_acorr
create acorrbuf 32768 dfloats allot

variable np
variable npcorr
fvariable fcorrsum
fvariable fcorrnorm

Public:

: autocorrelation ( -- | compute the numerical autocorrelation function )

	\ assume dataset is ordered by increasing abscissas

	?active dup 0 >=
	if
	  ds1 get_ds
	  0 >= if
	    ds1 DatasetInfo->Npts @ np !

	    \ Determine normalization constant

	    0e
	    np @ 0 do
	      i ds1 @xy fswap fdrop fsquare f+
	    loop
	    fcorrnorm f!

	    \ Compute the normalized autocorrelation function
 
	    np @ 2* 1- npcorr !	\ there are 2N-1 pts in the autocorr func.	    
	    
	    npcorr @ 0 do
	      0e		\ running integral value
	      np @ 0 do
	        i j - np @ + 1-
	        dup dup 0< swap np @ 1- > or 
	        if 
	          drop 
	        else 
	          ds1 @xy fswap fdrop
	          i ds1 @xy fswap fdrop
	          f* f+
	        then
	      loop
	      fcorrnorm f@ f/	\ normalize the integral 
	      fcorrsum f!
	      i np @ - 1+ s>f
	      acorrbuf i 2* sfloats + sf!
	      fcorrsum f@
	      acorrbuf i 2* sfloats + sfloat+ sf!
	    loop
	          
	    \ setup the autocorrelation data structure

	    c" autocorrelation" 1+ ds_acorr DatasetInfo->Name !
	    c"  " 1+ ds_acorr DatasetInfo->Header !
	    ds1 DatasetInfo->Type @ ds_acorr DatasetInfo->Type !
	    npcorr @ ds_acorr DatasetInfo->Npts !
	    2 ds_acorr DatasetInfo->Size !
	    acorrbuf ds_acorr DatasetInfo->Data !

	    ds_acorr make_ds drop
		 
	  then
	else
	  drop
	then ;

\ add "AutoCorrelation" as an item in the math menu

MN_MATH  c" AutoCorrelation"  c" autocorrelation draw_window" add_menu_item

End-Module


