#ifndef SSED_NORM_X_H
#define SSED_NORM_X_H

#include "ilist.h"

////////////////////////////////////////////////////////////

#define NEWSTR(len) x_newstr(len,__FILE__,__LINE__)

#define MEMORY_FAULT(nonZero,len,strSourceFile,line) \
			if ( (nonZero)==0 ) { \
				fprintf(stderr,"%s:%d: Memory fault (%d)\n",strSourceFile,line,len); \
				ASSERTION_FALSE("Aborted"); \
			}
////////////////////////////////////////////////////////////

extern int norm_capa (const char* strOptCapa, gList& capaList) ;

extern char* x_newstr_from_list (gList& list, const char* strSepFmt, int mask) ;


#ifdef HAS_NORM

extern int norm_command (int cmdNr,
			 gList& capaList,
			 FILE* fIn,
			 FILE* fOut,
			 FILE* fRepErr,
			 gList& args,
			 int verboseLevel) ;

#endif //HAS_NORM

////////////////////////////////////////////////////////////
#endif //SSED_NORM_X_H

