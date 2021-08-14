#ifndef CNTP_DATES_X_H
#define CNTP_DATES_X_H

#include "lib_iobjs.h"
#include "lib_icntp.h"

#ifndef MJD_1970D
#define MJD_1970D	40587
#define MJD_1970L	40587L
#endif

#define MJD_TO_UNIX_TIME(m) ( (long) (((long)(m) - MJD_1970L) * 86400L) )

////////////////////////////////////////////////////////////
enum e_LocalhostParse {
    e_ignore_127 = 127,
    e_substitute_by_localhost = 128,
    e_show_all_explicitly		// show 127.x.y.z and all other IPs
};

////////////////////////////////////////////////////////////
struct sCntpDateDST {
    sCntpDateDST ()
	: yearJulian( -1 ),
	  refTime( RAW_REFTIME_DEFAULT ) {
    }

    ~sCntpDateDST () ;

    t_int16 yearJulian;
    t_int8 refTime;  // -126: invalid; -1: year<0; 100: 1970-2035; 101: 2036-...

    // This struct intends to extend server capabilities for Civil and
    // political changes accross different countries.
};
////////////////////////////////////////////////////////////

extern char* new_parsed_peerstats (const char* strLine, int which, e_LocalhostParse localhostOpt, int& error) ;

extern char* new_parsed_loopstats (const char* strLine, int which, e_LocalhostParse localhostOpt, int& error) ;

////////////////////////////////////////////////////////////
#endif //CNTP_DATES_X_H

