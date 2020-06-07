/*
xyplot.h

  Header file for XYPLOT (C++/Linux/X Windows version)

  Copyright (c) 1995--2020 Krishna Myneni 
  <krishna.myneni@ccreweb.org>

  This software is provided under the terms of the 
  GNU Affero General Public License (GPL), v3.0 or later.

*/

#include "CPlotWindow.h"

#define ANNOTATION_PREFIX "==XYPLOT==> "

// Function prototypes:

int BlankLine (char*);
int NumberParse (float*, char*);
void SortRect (CRect*);
char* LabelFormat (float, float, char);
char* DisplayFormat (float x1, float x2);
int AddToHeader (char*, char*, bool);
void LoadInitializationFile();
vector<char*> GetStartupFileList (int, char**);
void InitForthInterface();
int LoadForthFile(char*);
int ExecuteForthExpression (char*, ostringstream*, int*);

struct IfcFuncTemplate
{
    const void* Function;
    const char* constantName;
};

// The following functions provide the interface between
// the C++ functions and the Forth environment. They
// should not be called by C, C++, or assembler routines.
//
int get_color_map();
int get_active_set();
int get_operand_set();
int get_active_plot();
int get_operand_plot();
int set_active_plot();
int set_operand_plot();
int get_ds();
int set_ds_extrema();
int make_ds();
int get_plot();
int drop_plot();
int make_plot();
int set_plot_symbol();
int set_plot_color();
int draw_plot();
int set_grid_tics();
int set_grid_lines();
int clear_window();
int draw_window();
int reset_window();
int get_window_limits();
int set_window_limits();
int add_menu_item();
int make_menu();
int make_submenu();
int set_background();
int set_foreground();
int radio_box();
int message_box();
int get_input();
int file_open_dialog();
int set_save_options();
