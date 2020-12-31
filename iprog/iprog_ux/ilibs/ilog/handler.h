#ifndef gHANDLER_X_H
#define gHANDLER_X_H

#include "lib_iobjs.h"

////////////////////////////////////////////////////////////

extern int ilf_flush_log (int handleIn, int handleOut, int dateIdx, gDateTime& aDate, t_uint16 mask, char* lineBuf, int& idx) ;

extern int ilf_dump_apache_logs (int handleIn, int handleOut, t_uint16 mask, gDateTime& aDate, gString& lastLine) ;

////////////////////////////////////////////////////////////

#endif //gHANDLER_X_H

