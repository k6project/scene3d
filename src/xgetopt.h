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

#ifdef __cplusplus
#define XGETOPT_API extern "C"
#else // __cplusplus
#define XGETOPT_API extern
#endif // __cplusplus

#include <tchar.h>

XGETOPT_API int optind, opterr;
XGETOPT_API TCHAR *optarg;

XGETOPT_API int getopt(int argc, TCHAR *argv[], TCHAR *optstring);

#endif // XGETOPT_H

#endif // _MSC_VER
