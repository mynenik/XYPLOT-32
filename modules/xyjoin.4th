\ xyjoin.4th
\
\ XYPLOT module to join two data sets into a new single set.
\
\ Copyright (c) 2021 Krishna Myneni
\
\ This software is released under the terms of the GNU
\ Affero General Public License (AGPL).

Begin-Module

DatasetInfo ds1
DatasetInfo ds2
DatasetInfo ds3

PlotInfo pl3

variable nelem

variable newNpts
variable newType
variable newSize
variable newElements

0 ptr pJoinedData
0 ptr pSrc
0 ptr pDest

Public:

: join_datasets ( -- )
    ds1 ds2 get_sel_setinfo2 0= IF
      ?active ?operand = IF
         s" Active and operand sets are the same!" message_box
         EXIT
      THEN
   ELSE
     s" Error: Failed to retrieve data sets info!" message_box
     EXIT
   THEN

   0 newSize !
   ds1 DatasetInfo->Size @ ds2 DatasetInfo->Size @
   2dup max 0= IF
     2drop
     s" Error: Datum size is zero!" message_box
     EXIT
   THEN 
   <> IF
     s" Error: Cannot join sets -- datum sizes not equal!" message_box
     EXIT
   ELSE
     ds1 DatasetInfo->Size @ newSize !
   THEN

\ Determine combined data type for the two sets.   
   -1 newType !
   ds1 DatasetInfo->Type @
   CASE
     REAL_DOUBLE OF  REAL_DOUBLE newType !  ENDOF
     REAL_SINGLE OF
       ds2 DatasetInfo->Type @
       CASE
         REAL_DOUBLE OF  REAL_DOUBLE newType !  ENDOF
         REAL_SINGLE OF  REAL_SINGLE newType !  ENDOF
       ENDCASE
     ENDOF
   ENDCASE
   newType @ -1 = IF
     s" Error: Unsupported data type(s)!" message_box
     EXIT
   THEN

   ds1 DatasetInfo->Npts @ ds2 DatasetInfo->Npts @ + newNpts !

   \ Allocate memory for joined data
   newNpts @ newSize @ * newElements !
   newType @
   CASE
     REAL_SINGLE OF  newElements @ SFLOATS  ENDOF
     REAL_DOUBLE OF  newElements @ DFLOATS  ENDOF
   ENDCASE
   allocate IF    \ memory allocation failed
     drop 
     s" Error: Insufficient memory to join data sets!" message_box
     EXIT
   ELSE
     to pJoinedData
   THEN
 
   \ Copy data from both sets into new memory block
   pJoinedData to pDest
   ds1 DatasetInfo->Data a@ to pSrc
   ds1 DatasetInfo->Npts @ ds1 DatasetInfo->Size @ * nelem !
   ds1 DatasetInfo->Type @
   CASE
     REAL_SINGLE OF
       newType @ REAL_SINGLE = IF
         pSrc pDest nelem @ sfloats move 
         pDest nelem @ sfloats + to pDest
       ELSE
         pSrc pDest nelem @ Copy_SF_to_DF   \ single -> double
         pDest nelem @ dfloats + to pDest
       THEN
       ENDOF
     REAL_DOUBLE OF
       psrc pDest nelem @ dfloats move
       pDest nelem @ dfloats + to pDest
     ENDOF
   ENDCASE
   ds2 DatasetInfo->Data a@ to pSrc
   ds2 DatasetInfo->Npts @ ds2 DatasetInfo->Size @ * nelem !
   ds2 DatasetInfo->Type @
   CASE
     REAL_SINGLE OF
       newType @ REAL_SINGLE = IF
         pSrc pDest nelem @ sfloats move
       ELSE
         pSrc pDest nelem @ Copy_SF_to_DF
       THEN
       ENDOF
     REAL_DOUBLE OF
       pSrc pDest nelem @ dfloats move
     ENDOF
   ENDCASE

   \ Fill in data set info structure to add joined set to database
   c" Joined" 1+ ds3 DatasetInfo->Name !
   s" Join: "
   ds1 DatasetInfo->Name a@ dup strlen strcat
   s"  + " strcat
   ds2 DatasetInfo->Name a@ dup strlen
   strcat strpck 1+
             ds3 DatasetInfo->Header !
   newType @ ds3 DatasetInfo->Type !   
   newNpts @ ds3 DatasetInfo->Npts !
   newSize @ ds3 DatasetInfo->Size !
   pJoinedData ds3 DatasetInfo->Data !

   ds3 make_ds 
   pJoinedData free drop  \ Free the memory.
   dup 0< IF
     drop s" Error making joined set!" message_box
     EXIT
   THEN   \ -- n

   \ Fill in plot info structure to make new plot
   pl3          PlotInfo->Set !
   0 pl3        PlotInfo->Type !
   sym_Line pl3 PlotInfo->Symbol !
   2 pl3        PlotInfo->Color !

   pl3 make_plot
;

MN_EDIT c" Join Sets"  c" join_datasets draw_window" add_menu_item

End-Module

