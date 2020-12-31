#ifndef G_CNTP_RAW_DATE_X_H
#define G_CNTP_RAW_DATE_X_H

#include "icalendar.h"

#define RAW_REFTIME_INVALID -126
#define RAW_REFTIME_DEFAULT 100  // 'epoch-time'

////////////////////////////////////////////////////////////
struct sRawDtTm {
    sRawDtTm ()
	: secs( 0 ),
	  milisecs( 0 ),
	  timeZone( 0 ),
	  refTime( RAW_REFTIME_DEFAULT ) {
    }

    t_uint32 secs;
    t_uint16 milisecs;
    t_int8 timeZone;
    t_int8 refTime;  // -126: invalid; -1: year<0; 100: 1970-2035; 101: 2036-...

    // Get methods
    float GetSecs () {
	return (float)secs + (float)milisecs/1000.0;
    }

    // Set methods
    void Reset () {
	secs = 0;
	milisecs = 0;
	timeZone = 0;
	refTime = RAW_REFTIME_INVALID;
    }

    void MarkInvalid (bool isInvalid=true) {
	if ( isInvalid )
	    refTime = RAW_REFTIME_INVALID;
	else
	    refTime = RAW_REFTIME_DEFAULT;
    }

    int SetTime (t_uint32 aSec, t_uint16 aMilisec=0, t_int8 zone=0, t_int8 aRefTime=0) {
	// Return 0 if is tentatively ok
	secs = aSec;
	milisecs = aMilisec;
	timeZone = zone;
	refTime = aRefTime;
	return refTime==RAW_REFTIME_INVALID;
    }

    // Special methods
    int ConvertToDate (gDateTime& cal) ;
};
////////////////////////////////////////////////////////////

extern int cntp_GetTimeOfDay (sRawDtTm& current) ;

extern int cntp_GetTimeOfDay (sRawDtTm& current) ;

////////////////////////////////////////////////////////////
#endif //G_CNTP_RAW_DATE_X_H

