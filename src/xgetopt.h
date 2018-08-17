// XGetopt.h  Version 1.2
//
// Author:  Hans Dietrich
//          hdietrich2@hotmail.com
//
// This software is released into the public domain.
// You are free to use it in any way you like.
//
// This software is provided "as is" with no expressed
// or implied warranty.  I accept no liability for any
// damage or loss of business that this software may cause.
//
///////////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER

#ifndef XGETOPT_H
#define XGETOPT_H

#include "macros.h"

#include <tchar.h>

C_API int optind, opterr;
C_API TCHAR *optarg;

C_API int getopt(int argc, TCHAR *argv[], TCHAR *optstring);

#endif // XGETOPT_H

#endif // _MSC_VER
