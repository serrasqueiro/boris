#ifndef iLIB_ILAMBDA_X_H
#define iLIB_ILAMBDA_X_H

#define LIB_ILAMBDA_VERSION_MAJOR        1
#define LIB_ILAMBDA_VERSION_MINOR        2

////////////////////////////////////////////////////////////
#include "lib_iobjs.h"

#include "iLambdaNetPair.h"
#include "iJson.h"
#include "fcrc32.h"
#include "dba.h"
#include "suma.h"
#include "iID3vx.h"

////////////////////////////////////////////////////////////
struct sComOpt {
    sComOpt ()
	: level( 0 ),
	  debugLevel( 0 ),
	  isVerbose( false ),
	  isVeryVerbose( false ),
	  strsOriginalArgs( nil ) {
    }

    ~sComOpt () {
    }

    int level;
    int debugLevel;
    bool isVerbose, isVeryVerbose;
    char **strsOriginalArgs;

    gString sPidFile;

    int SetDebug (int forcedDebug) {
	if ( forcedDebug<=-1 ) {
	    level += (isVerbose==true) * 3;
	    level += (isVeryVerbose==true) * 6;
	}
	else {
	    level = forcedDebug;
	}
	return level;
    }
};

////////////////////////////////////////////////////////////
struct LambdaSortBy {
    LambdaSortBy ()
	: byCode( 0 ),
	  mainAscend( true ),
	  limit( MAX_INT_VALUE ) {
    }

    int byCode;
    bool mainAscend;
    int limit;
    gList codes;

    bool NonBasicSort () {
	return byCode > 0;
    }

    int SetCode (int code, bool ascending=true) {
	mainAscend = ascending;
	byCode |= code;
	codes.Add( (char*)(ascending ? ">" : "<") );
	codes.EndPtr()->me->iValue = byCode;
	return byCode;
    }

    void Copy (LambdaSortBy& copy) {
	byCode = copy.byCode;
	mainAscend = copy.mainAscend;
	limit = copy.limit;
	codes.CopyList( copy.codes );
    }
};

////////////////////////////////////////////////////////////

// Macros

#ifdef DEBUG_LOG
#define IL_LOG_LEVEL(logFile,level,args...) { \
		S_LOG_LEVEL(logFile,level,args); \
		fprintf(stderr,"DebugLog(%p%s), base level=%d, %d: ",logFile,logFile==stdout?"=stdout":"\0",lGlobLog.dbgLevel,level); \
		fprintf(stderr,args) ; \
		fprintf(stderr,"\n"); \
		}
#else
#define IL_LOG_LEVEL(logFile,level,args...) S_LOG_LEVEL(logFile,level,args)
#endif

#define IL_LOG_DEF(logFile,args...) S_LOG_LEVEL(logFile,LOG_INFO,args)


////////////////////////////////////////////////////////////
#endif //iLIB_LAMBDA_X_H

