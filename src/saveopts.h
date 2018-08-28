// saveopts.h
//
// The save options structure
//
//

#ifndef __SAVEOPTS_H__
#define __SAVEOPTS_H__


struct SaveOptions
{
    int HeaderType;
    int Delimiter;
    int NumberFormat;
    int CrLf;
    char UserPrefix [16];
};

#endif
