#ifndef SSED_SDATES_X_H
#define SSED_SDATES_X_H

#include <sys/time.h>

#include "istring.h"

////////////////////////////////////////////////////////////

extern gList* ctime_date (t_stamp stamp) ;
extern gList* ctime_date_micro (struct timeval& tv, t_int16 format) ;

extern gString* join_strings (gList& list, const char* strSep) ;

extern int show_datex (FILE* fIn, FILE* fOut, int dateType) ;

extern int sdate_x_conv (const char* aStr, int dateType, gString& sResult) ;

extern int sdate_shown_conv (FILE* fOut, const char* aStr, int length, gDateTime& dttm, int& year) ;


extern gList* time_list (FILE* fRepErr, gString& hhmmss, int year, t_int16 ms3or6) ;

extern int time_list_fix (FILE* fRepErr, gList& timed) ;

extern char* new_date_str (const t_stamp stamp, const char* strFormat, bool localTime) ;

////////////////////////////////////////////////////////////
#endif //SSED_SDATES_X_H

