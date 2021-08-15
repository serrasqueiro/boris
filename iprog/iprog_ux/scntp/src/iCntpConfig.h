#ifndef G_CNTP_CONFIG_X_H
#define G_CNTP_CONFIG_X_H

#include "iarg.h"
#include "ifile.h"

#include "lib_ilog.h"
#include "lib_icntp.h"

// Useful macros

#define MY_LOG(level,args...) CNTP_LOG_LEVEL(fGlobOut,level,args)

#define MY_DEF_LOG(args...) CNTP_LOG_DEF(fGlobOut,args)

#define SLEEP_SEC(nSecs)	gFileControl::Self().SecSleep( nSecs )

#define SLEEP_SEC_LOG(nSecs,args...) { \
					MY_LOG(LOG_NOTICE,args); \
					SLEEP_SEC(nSecs); \
				     }

////////////////////////////////////////////////////////////

extern int iGlobDay;
extern int iGlobDenyAll;
extern gString sGlobClient;
extern FILE* fGlobOut;
extern gSLog lGlobLog;

////////////////////////////////////////////////////////////
struct sOptPxyStat {
    sOptPxyStat ()
	: secs( 0 ),
	  stamp0( 0 ),
	  stamp1( 0 ),
	  countCli( 0 ),
	  countSrv( 0 ),
	  bytesCli( 0 ),
	  bytesSrv( 0 )
	{
    }

    long secs;
    t_stamp stamp0, stamp1;
    t_uint16 countCli;
    t_uint16 countSrv;
    t_uint32 bytesCli;
    t_uint32 bytesSrv;
};

struct sOptCntp {
    sOptCntp ()
	: isVerbose( false ),
	  isVeryVerbose( false ),
	  doShowMillis( false ),
	  doShowStats( false ),
	  doTestOnly( false ),
	  anyPort( false ),
	  myPort( DEF_CNTP_PORT ),
	  remotePort( myPort ),
	  nAccepts( 0 ),
	  nStalls( 0 ),
	  fStat( nil ) {
    }
    ~sOptCntp () {
    }
    bool isVerbose, isVeryVerbose;
    bool doShowMillis;
    bool doShowStats;
    bool doTestOnly;
    bool anyPort;
    t_gPort myPort;
    t_gPort remotePort;
    gString sRemoteHost;

    // ...-opt-ins
    gString sLogFile;
    gString sRemoteHostOnly;
    unsigned nAccepts, nStalls;
    // Stats, etc
    FILE* fStat;

    int BuildRemoteHost () ;
};
////////////////////////////////////////////////////////////

extern int flush_log_a () ;

extern char* tod_date () ;

extern char* tod_date_ext (int opt_secs) ;

extern char* tod_date_miliseconds (t_uint16 miliseconds) ;

extern int opt_to_VerboseLevel (sOptCntp& opt) ;

extern char* tod_date_cntpas (gDateTime& now, t_int16 minutesAdjust, char* strResult, size_t size) ;

////////////////////////////////////////////////////////////
#endif //G_CNTP_CONFIG_X_H

