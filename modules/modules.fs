\ Modules library, version 0.5.2

\ License: LGPL

\ Copyright (c) 2011--2012 Krishna Myneni and David N. Williams
\ This file is automatically generated using LyX and noweb.
\ Changes should be made to the original file, modular-forth.lyx

create ver-buf 16 allot
: ver$ ( "version" -- ca u)
        bl parse 16 min 2dup ver-buf swap move
        nip ver-buf swap ;
: make-$ ( ca1 u "name" -- )
        create dup 1+ allot? 2dup ! 1+ swap move
        does> ( a -- ca2 u ) count ;

ver$ 0.5.2

make-$ MODULES-VERSION

[UNDEFINED] drops [IF]
  : drops ( +n -- ) 0 ?DO drop LOOP ;
[THEN]

[UNDEFINED] order-drops [IF]
  : order-drops  ( +n -- ) ( o: wid_n ... wid_1 -- )
      0 ?DO previous LOOP ;
[THEN]

[UNDEFINED] order-depth [IF]
  : order-depth  ( -- n )  get-order dup >r drops r> ;
[THEN]

[UNDEFINED] >order [IF]
  : >order  ( wid -- order: wid )
      >r get-order r> swap 1+ set-order ;
[THEN]

[UNDEFINED] order> [IF]
  : order>  ( o: wid -- ) ( -- wid )
      get-order swap >r 1- set-order r> ;
[THEN]

[UNDEFINED] order@ [IF]  \ not used here
  : order@  ( o: wid -- ) ( -- wid )
      get-order over >r drops r> ;
[THEN]

VARIABLE initial-defs
VARIABLE initial-order-depth

: save-SO-state ( -- )
        get-current  initial-defs !
        order-depth  initial-order-depth !
;

: restore-SO-state ( -- )
        initial-defs a@ set-current
        order-depth initial-order-depth @ - order-drops
;
wordlist  constant  Private-Words
wordlist  constant  MODULES-WORDLIST
MODULES-WORDLIST >order

: MODULES ( -- )  get-order nip MODULES-WORDLIST swap set-order ;
VARIABLE named-module   0 named-module !

: make-module ( "name" -- )
        CREATE  \ here 0 , 0 , 0 , named-module !
	0 0 0 3 cells allot? dup >r 2! r@ 2 cells + ! r> named-module !
        DOES>  ( o: wid' -- wid )  a@ >r get-order nip r> swap set-order ;

: MODULE:  ( "name" -- )
        save-SO-state
        MODULES-WORDLIST set-current
        make-module
    
        initial-defs a@ set-current
;
   
VARIABLE private-defs  0 private-defs !
VARIABLE public-defs   0 public-defs  !

: BEGIN-MODULE  ( -- ) ( o: -- public private )
    named-module @ if
      wordlist    \ new private wordlist
      wordlist    \ new public wordlist
      2dup named-module a@ 2!
    else
      save-SO-state
      Private-Words
      get-current
    then
    2dup ( wid_private wid_public -- )  public-defs !  private-defs !

    2dup  >order  >order
    drop set-current    \ default section is private
;
: not-in-module ( -- ) ."  Not in Module Body!" cr ABORT ;
: safe-set-current ( wid -- ) 
        ?dup IF set-current ELSE not-in-module THEN ;
: PRIVATE:  ( -- )  private-defs a@  safe-set-current ;
: PUBLIC:   ( -- )  public-defs  a@  safe-set-current ;
: END-MODULE  ( o: <extras> -- )  
    restore-SO-state
        0 named-module !
        0 private-defs !
        0 public-defs  ! ;
: show-modules ( -- )  \ Display all loaded modules
        MODULES-WORDLIST >order  words  PREVIOUS ;
: >public  ( xtmodule -- wid )  >body a@ ;
: >private ( xtmodule -- wid )  >body cell+ a@ ;
create member_name 128 allot

: !member-name ( c-addr1 -- c-addr2 u)
        dup c@ 127 min 1+ member_name swap move ;

: member-find ( "module-name" "word-name" -- 0 | xt 1 | xt -1 )
        bl word find 0= ABORT" Unknown Module!"
        also execute order> >r ( "word_name" )
        bl word !member-name
        member_name count r> search-wordlist ;

: member-not-found ( -- )
        member_name count type
        ."  : Member Not Found!"  cr ABORT ;

: module-do-ref ( ... xt 1 | xt -1 -- ...)
        -1 = state @ and IF  compile, ELSE execute THEN ;
: m' ( "module-name" "word-name" -- xt )
        member-find 0= IF member-not-found THEN ;

: [m'] ( "module-name" "word-name" -- )  m' postpone literal ; immediate

: [m] ( ... "module-name"  "word-name" -- ? )
        member-find
        ?dup IF  module-do-ref  ELSE  member-not-found  THEN
; immediate

true [IF] : ∋ postpone [m] ; immediate [THEN]
: [this] ( "word-name" -- )
     bl word !member-name
         private-defs a@ 0=  public-defs a@ 0= or IF not-in-module THEN
         member_name count private-defs a@ search-wordlist
         ?dup IF  
                  module-do-ref
         ELSE
           member_name count public-defs a@ search-wordlist
           ?dup IF  module-do-ref  ELSE  member-not-found  THEN
         THEN ; immediate

true [IF]  : ∃ postpone [this] ; immediate  [THEN]
: [member] ( "module" "name" -- flag )
    member-find if drop true else false then ; immediate

