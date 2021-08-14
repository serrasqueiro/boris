#ifndef G_CNTP_HELPER_X_H
#define G_CNTP_HELPER_X_H

#include "ifile.h"

#ifdef iDOS_SPEC
;
#else
#include <signal.h>
#include <unistd.h>
#endif //~iDOS_SPEC

#include "iCntpConfig.h"
#include "iCIp.h"


#define STAMP_1970_JAN02	86400	// second day
#define STAMP_1980_JAN01	315532800

#define FMT_YYMMDD	"%04u-%02u-%02u"
#define FMT_HHMMSS	"%02u:%02u:%02u"
#define FMT_DTTM	FMT_YYMMDD " " FMT_HHMMSS

#define LIMITED(num,range) (range>0 ? (((num < 0 ? 0 : num) % range)) : num)

////////////////////////////////////////////////////////////

extern int my_bye (FILE* fRepErr, int signalId, int mask, gList& optActions) ;

extern int my_bailout (int signalId) ;

extern int my_start_daemon (int aPid, int mask) ;

extern int my_helper_signal_on (int signalId, bool doSignal) ;
extern int my_helper_signal_hup (bool doSignal) ;

extern int my_helper_signal_ignore (int signalId) ;

extern unsigned my_helper_alarm (unsigned seconds) ;

extern int my_helper_setuid (unsigned myUserId) ;

extern char* my_helper_ctime_current () ;

extern int my_client_check (gCNetSource& clientSource,
			    FILE* fRepErr,
			    gString& sMsg) ;

extern char* my_client_source (gCNetSource& aClient,
			       int codex,
			       int mask) ;

////////////////////////////////////////////////////////////
#endif //G_CNTP_HELPER_X_H

