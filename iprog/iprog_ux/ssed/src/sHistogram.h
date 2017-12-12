#ifndef SSED_HISTOGRAM_X_H
#define SSED_HISTOGRAM_X_H


#include "lib_iobjs.h"

////////////////////////////////////////////////////////////

extern int show_histogram (FILE* fIn,
			   FILE* fOut,
			   const char* strSep,
			   int zValue,
			   int charType) ;

extern int dump_buffer (FILE* fOut, int sigh, const t_uchar* buf, int size) ;


extern char* x_newstr_from_list (gList& list, const char* strSepFmt, int mask) ;


#ifdef HAS_NORM

extern int norm_command (int cmdNr,
			 gList& capaList,
			 FILE* fIn,
			 FILE* fOut,
			 FILE* fRepErr,
			 gList& args,
			 int option,
			 int verboseLevel) ;

#endif //HAS_NORM

////////////////////////////////////////////////////////////
#endif //SSED_NORM_X_H

