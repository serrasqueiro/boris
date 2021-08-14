#ifndef RFC822_DATE_X_H
#define RFC822_DATE_X_H

#include <stdio.h>
#include <time.h>

#include "lib_iobjs.h"
#include "lib_ilog.h"

////////////////////////////////////////////////////////////

extern int ctime_trim_string (time_t now, t_uint16 bufSize, char* outBuffer) ;

extern int ctime_trim_sec_string (time_t now, t_uint16 bufSize, char* outBuffer, t_int16& second) ;

extern char* rfc822_date_string (gDateTime& here, int milisecs, bool showUTC, int timeDiff) ;

extern int dump_rfc822_date (FILE* fOut, gDateTime& here, int milisecs, bool showUTC, int timeDiff) ;

////////////////////////////////////////////////////////////
#endif //RFC822_DATE_X_H

