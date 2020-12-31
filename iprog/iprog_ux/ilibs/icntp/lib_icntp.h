#ifndef iLIB_CNTP_X_H
#define iLIB_CNTP_X_H

#define LIB_ICNTP_VERSION_MAJOR        1
#define LIB_ICNTP_VERSION_MINOR        0


#include "iCIp.h"
#include "iCntpRawDate.h"

#include "iCntproto.h"

#include "iNetPair.h"


// Macros

#ifdef DEBUG_LOG
#define CNTP_LOG_LEVEL(logFile,level,args...) { \
		S_LOG_LEVEL(logFile,level,args); \
		fprintf(stderr,"DebugLog(%p%s), base level=%d, %d: ",logFile,logFile==stdout?"=stdout":"\0",lGlobLog.dbgLevel,level); \
		fprintf(stderr,args) ; \
		fprintf(stderr,"\n"); \
		}
#else
#define CNTP_LOG_LEVEL(logFile,level,args...) S_LOG_LEVEL(logFile,level,args)
#endif

#define CNTP_LOG_DEF(logFile,args...) S_LOG_LEVEL(logFile,LOG_NOTICE,args)


#endif //iLIB_CNTP_X_H

