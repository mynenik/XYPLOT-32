// colortable.h
//
// default colors for XYPLOT
//
// Copyright (c) 2007--2020 Krishna Myneni
// <krishna.myneni@ccreweb.org>
//
// This software is provided under the terms of the
// GNU Affero General Public License (AGPL), v3.0 or later.

#ifndef __COLORTABLE_H__
#define __COLORTABLE_H__

#define MAX_COLORS 32

// RGB values packed into type COLORREF
COLORREF colors_rgb_table[] = {
    RGB(0,0,255),    // blue
    RGB(255,0,0),    // red
    RGB(0,128,0),    // green
    RGB(64,224,208), // turquoise
    RGB(255,0,255),  // magenta
    RGB(160,82,45),  // sienna
    RGB(255,215,0),  // gold
    RGB(128,0,128),  // purple
    RGB(255,165,0),  // orange
    RGB(46,139,87),  // sea green
    RGB(112,128,144),  // slate gray
    RGB(255,127,80),   // coral
    RGB(0,0,0),        // black
    RGB(127,255,212),  // aqua marine
    RGB(70,130,180),   // steel blue
    RGB(218,112,214),  // orchid
    RGB(250,128,114),  // salmon
    RGB(25,25,112),    // midnight blue
    RGB(0,100,0),      // dark green
    RGB(0,255,255),    // cyan
    RGB(106,90,205),   // slate blue
    RGB(50,205,50),    // lime green
    RGB(255,255,0),    // yellow
    RGB(138,43,226),   // blue violet
    RGB(165,42,42),    // brown
    RGB(220,20,60),    // crimson
    RGB(0,255,127),    // spring green
    RGB(205,92,92),    // chestnut
    RGB(0,191,255),    // deep sky blue
    RGB(211,211,211),  // light gray
    RGB(245,222,179),  // wheat
    RGB(255,255,255)   // white
};

const char* color_names[] = 
  {"blue", "red", "forestgreen", "turquoise",
   "magenta", "sienna", "gold", "purple", 
   "orange", "seagreen", "slategray", "coral",
   "black", "aquamarine", "steelblue", "orchid",
   "salmon", "midnightblue", "darkgreen", "cyan",
   "slateblue", "limegreen", "yellow", "blueviolet",
   "brown", "crimson", "springgreen", "chestnut",
   "deepskyblue", "lightgray", "wheat", "white"};

#endif
