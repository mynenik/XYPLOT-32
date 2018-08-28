// CXyFile.cpp
//
// Copyright 1995--2018 Krishna Myneni
// <krishna.myneni@ccreweb.org>
//
// This software is provided under the terms of the
// GNU General Public License (GPL), v3.0 or later.
//

#include <string.h>
#include "consts.h"
#include "CXyFile.h"

//---------------------------------------------------------------

CXyFile::CXyFile (char* name, int open_mode)
{
    m_szName = new char [255];
    m_szHeader = new char [HEADER_LENGTH];
    m_szFirstLine = new char [MAX_LINE_LENGTH];
    m_pValues = new float [MAX_DATA_COLUMNS];

// Default output format


    m_pInFile = NULL;
    m_pOutFile = NULL;
    m_bReadData = 0;
    m_nFail = 0;
	strcpy (m_szName, name);

	if (open_mode)
	{
	  m_pOutFile = new ofstream (name);
	  if (m_pOutFile->fail()) m_nFail = 1;
	}
	else
	{
	  m_pInFile = new ifstream (name);
	  if (m_pInFile->fail()) m_nFail = 1;
	}

}
//---------------------------------------------------------------

CXyFile::~CXyFile()
{
    delete m_pInFile;
    delete m_pOutFile;
    delete [] m_pValues;
    delete [] m_szFirstLine;
    delete [] m_szHeader;
	delete [] m_szName;
}
//---------------------------------------------------------------

int CXyFile::Fail()
{
    // return fail indicator and clear

    int i = m_nFail;
    m_nFail = 0;
    return i;
}
//---------------------------------------------------------------

void CXyFile::ReadHeader()
{
   /*
      Reads the header of a data file.  The header must be enclosed by
      HEADER_BEGIN and HEADER_END.
   */

    char* p;
    char temp[MAX_LINE_LENGTH];

	*m_szHeader = '\0';

    // Ignore blank lines at beginning of file.

    do {
        m_pInFile->getline (m_szFirstLine, MAX_LINE_LENGTH);
    } while (BlankLine(m_szFirstLine) && (! m_pInFile->fail()));


    // Check whether first non-blank line is the beginning
    // of a comment

    p = strstr (m_szFirstLine, HEADER_BEGIN);

    if (p) 
      {
	do {
            m_pInFile->getline(m_szFirstLine, MAX_LINE_LENGTH);
            p = strstr (m_szFirstLine, HEADER_END);
            if (! p)
            {
                strcat (m_szHeader, m_szFirstLine);
                strcat (m_szHeader, "\n");
            }

        } while (!p && (!m_pInFile->fail()));

	m_pInFile->getline(m_szFirstLine, MAX_LINE_LENGTH);
      }
    else if (*m_szFirstLine == '#')
      {
	// Read unix style comment lines

	p = m_szFirstLine;

	do
	  {
	    ++p;;
	    if (*p == ' ') ++p;
	    strcat (m_szHeader, p);
	    strcat (m_szHeader, "\n");
	    m_pInFile->getline (m_szFirstLine, MAX_LINE_LENGTH);
	    p = m_szFirstLine;
	  } 
	while ((*p == '#') && (!m_pInFile->fail()));	  
      }
    else
      {
	return;
      }
      
    // Ignore blank lines after header

    while (BlankLine (m_szFirstLine) && (!m_pInFile->fail()))
      {
	m_pInFile->getline(m_szFirstLine, MAX_LINE_LENGTH);
      }    
}
//---------------------------------------------------------------

void CXyFile::WriteHeaderLines(char* prefix, char* eol)
{
    char *cp1, *cp2, line[256];

    cp1 = m_szHeader;
    cp2 = cp1;
    int nc;
    while (cp2) {
	cp2 = strchr(cp1, (int) '\012');  // search for LF
	if (cp2)
	{
	    nc = cp2 - cp1;
	    if (nc > 255) nc = 255;      // enforce max line length
	    strncpy(line, cp1, nc);      // extract line up to LF (not including LF)
	    if (nc)
	    {
		if (line[nc-1] == '\015') --nc;  // strip CR
	    } 
	    line[nc] = 0;
	    *m_pOutFile << prefix << ' ' << line << eol;
	    cp1 = cp2+1;
	}
	else
	{
	    if (*cp1) *m_pOutFile << prefix << ' ' << cp1 << eol;
	}
    }		    
}
//---------------------------------------------------------------

void CXyFile::WriteHeader (char* hdr)
{
// Write a header to the file


  char szPrefix[16], szEOL[4];
  
  strcpy (szPrefix, m_nSave.UserPrefix);
  if (m_nSave.CrLf) strcpy (szEOL, "\015\012"); else strcpy (szEOL, "\012");

  strcpy (m_szHeader, hdr);

  switch (m_nSave.HeaderType)
    {
    case 0:
      // No header
      break;

    case 1:
      // Write header in standard xyplot format

      *m_pOutFile << HEADER_BEGIN << szEOL;
      // *m_pOutFile << m_szHeader;
      WriteHeaderLines("", szEOL);
      *m_pOutFile << HEADER_END << szEOL;
      break;

    case 2:
      // Write header with prefix symbol on each line
	WriteHeaderLines(szPrefix, szEOL);
      break;

    default:
      // no output
      ;

    }
}
//---------------------------------------------------------------

int CXyFile::Columns()
{
// Return the number of data columns in the file
//   A prior call to ReadHeader should have been made

    m_nCols = NumberParse (m_pValues, m_szFirstLine);
    return m_nCols;
}
//---------------------------------------------------------------
int CXyFile::ReadData(float* data, int nSel, int ColumnSelections[])
{
/*
Reads data from the data file.

Arguments:

    ColumnSelections is an array containing the column numbers
        which are selected to be read from the file.
    nSel is the number of selections

Return the number of points read from the file.
*/

	float* d = data;
    int nPts = 0, i, *cp;
    char s [MAX_LINE_LENGTH];

    if (nSel <= 0) return 0;

    // Read the requested data from the file.

    if (m_bReadData == 0)
    {
        // Parse the first line of data

        NumberParse (m_pValues, m_szFirstLine);

        cp = ColumnSelections;
        i = 0;
        while (i < nSel)
        {
            *d++ = m_pValues[*cp++];
            ++i;
        }

	    ++nPts;
    }

    while ((! m_pInFile->fail()) && (nPts < MAX_POINTS))
    {
        m_pInFile->getline (s, MAX_LINE_LENGTH);
        if (NumberParse (m_pValues, s) < m_nCols) break;  // stop reading if ncols changes

        cp = ColumnSelections;
        i = 0;
        while (i < nSel)
        {
            *d++ = m_pValues[*cp++];
            ++i;
        }
        ++nPts;
    }

    m_bReadData = 1;
    return nPts;
}
//---------------------------------------------------------------

int CXyFile::WriteData(float* data, int nrows, int ncols)
{
// Write data to file

    float* d = data;
    char temp [32], szDelimiter[4], szFormat[16], szEOL[4];
    int i, j;

    if (m_nSave.CrLf) strcpy (szEOL, "\015\012"); else strcpy (szEOL, "\012");

    switch (m_nSave.Delimiter)
    {
        case 1:
            strcpy (szDelimiter, "\t");
            break;
        case 2:
            strcpy (szDelimiter, ",");
            break;
        default:
            strcpy (szDelimiter, " ");
            break;
    }

    switch (m_nSave.NumberFormat)
    {
        case 1:
            strcpy (szFormat, "%f");
            break;
        case 2:
            strcpy (szFormat, "%d");
            break;
        default:
            strcpy (szFormat, "%14e");
            break;
    }

	if (nrows > 0)
	{
		for (i = 0; i < nrows; ++i)
		{
		    for (j = 0; j < ncols; ++j)
		    {
   			    sprintf (temp, szFormat, *d++);
			    *m_pOutFile << temp;
			    if (j < (ncols-1)) *m_pOutFile << szDelimiter;
			}
			*m_pOutFile << szEOL;
		}
	}
    return 0;

}
//---------------------------------------------------------------


