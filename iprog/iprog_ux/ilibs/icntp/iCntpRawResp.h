#ifndef iCNTP_RAW_RESP_X_H
#define iCNTP_RAW_RESP_X_H

#include "iCntpRawDate.h"

////////////////////////////////////////////////////////////
struct sRawDtTmResponse {
    sRawDtTmResponse ()
	: lastSecUpDiff( MIN_INT16_I ) {
	sTwoChr[ 0 ] = 0;
    }

    ~sRawDtTmResponse () {
    }

    t_int32 lastSecUpDiff;
    char sTwoChr[ 4 ];
    char serverMsg[ 128 ];
    sRawDtTm serverTime;

    // Get methods
    bool NoComparison () {
	return lastSecUpDiff<=MIN_INT16_I;
    }

    // Set methods
    void Reset () {
	sTwoChr[ 0 ] = 0;
	serverTime.Reset();
    }

    int TrimServerMsg () ;

    // Special methods
    int ParseServerResponse (char* str) ;
};
////////////////////////////////////////////////////////////
#endif //iCNTP_RAW_RESP_X_H

