/*
  xyp41.h

 Header file for xyplot 4.1 compatibility

 Copyright (c) 1990-2018, Krishna Myneni,
   <krishna.myneni@ccreweb.org>

 This software is provided under the terms of the
 GNU General Public License (GPL), v3.0 or later.
*/

/*--------------------------------------------------------------------*/
/*              CONFIGURABLE CONSTANTS				      */
/*--------------------------------------------------------------------*/

#define MAX_POINTS_41  16300	   
#define MAX_SETS_41  10          	// number of allowed data sets
#define MAX_PLOTS_41  9               	// number of plots
#define HEADER_LENGTH_41  4096        	// length of header text

/*--------------------------------------------------------------------*/
/*              XYPLOT DATA STRUCTURES				      */
/*--------------------------------------------------------------------*/

// Define structure to manage data sets

struct Dataset {
	short int jsr;
	short int jer;
	short int jsi;
	short int jei;
	short int data_type;
	short int npts;
	float xmin;
	float xmax;
	float ixmin;
	float ixmax;
	float ymin;
	float ymax;
	float iymin;
	float iymax;
	char fname [25];
	char hdr [HEADER_LENGTH_41];
};



//  Define structure to keep track of data base 

struct Database {
	short int nsets;
	short int nvals;
	char hdr [HEADER_LENGTH_41];
};


//  Define structure to hold plot information

struct Plot {
	short int set;
	char sym;
	short int col;
	short int vis;
};



// Define data structure to save display information

struct Display41 {
	short int nplots;
	short int plist;
	short int sp1;
	short int sp2;
	short int scale;
	float xmin;
	float xmax;
	float ymin;
	float ymax;
	short int csize;
	short int cmode;
	short int level;
	short int grid;
	short int mouse_button;
	short int cx;
	short int cy;
	float xsf;
	float ysf;
	float ylsf;
	float lymin;
};

	
// end of xyp41.h





