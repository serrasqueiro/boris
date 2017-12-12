#ifndef XLOG_X_H
#define XLOG_X_H

#include "lib_ilog.h"


extern gSLog lGlobLog;

////////////////////////////////////////////////////////////

#define RD_DO_LOG(level,args...) lGlobLog.Log( lGlobLog.Stream(), level, args )

#define LOG_ME(level,args...) RD_DO_LOG(level,args)

#define LOG_ERR(args...) { LOG_ME(LOG_ERROR, args); if ( fRepErr==stderr ) { fprintf(fRepErr,args); fprintf(fRepErr,"\n"); } }

#define LOG_DBG(args...) { if ( lGlobLog.dbgLevel >= 9 ) { printf("%s:%d: ", __FILE__, __LINE__); printf(args); printf("\n"); } }

#define LOG_FLUSH() log_file_flush( lGlobLog, 0, 0 )

#define LOG_FLUSH_STD() log_file_flush_std( lGlobLog, 0, 0 )

////////////////////////////////////////////////////////////
#endif //XLOG_X_H

