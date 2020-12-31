// iCntpRawDate.cpp

#include <sys/time.h>

#ifdef iDOS_SPEC
#include <sys/timeb.h>
#endif //iDOS_SPEC


#include "iCntpRawDate.h"

////////////////////////////////////////////////////////////
int sRawDtTm::ConvertToDate (gDateTime& cal)
{
 // Returns 0 if convertion worked
 bool isOk = cal.SetTimeStamp( secs );
 return isOk==false;
}
////////////////////////////////////////////////////////////
int cntp_GetTimeOfDay (sRawDtTm& current)
{
 long usec( 0L );
 time_t curtime;

#ifdef iDOS_SPEC
 struct timeb tb;
 ftime( &tb );
 curtime = (long)tb.time;
#else
 struct timeval tv;
 struct timezone tz;

 current.Reset();
 if ( gettimeofday( &tv, &tz ) ) return -1;
 curtime = tv.tv_sec;
 usec = tv.tv_usec;
#endif //~iDOS_SPEC

 // this function returns always zero here!
 return current.SetTime( curtime, usec/1000 );
}

float cntp_GetTimeSecs ()
{
 sRawDtTm current;
 cntp_GetTimeOfDay( current );
 return current.GetSecs();
}
////////////////////////////////////////////////////////////

