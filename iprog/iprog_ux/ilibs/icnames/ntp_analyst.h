#ifndef NYS_ANALYST_X_H
#define NYS_ANALYST_X_H

#include "lib_iobjs.h"


#define NTPSEC_FROM_1970JAN_EPOCH  2208988800UL

////////////////////////////////////////////////////////////
// 'nys' functions
////////////////////////////////////////////////////////////

extern long nys_DateToMjd (int y, int m, int day) ;
extern t_uint64 nys_SecondsSince1900 (int y, int m, int day) ;

////////////////////////////////////////////////////////////
#endif //NYS_ANALYST_X_H

