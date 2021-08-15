// rfc822date.cpp

#include <stdlib.h>
#include <time.h>

#include "rfc822date.h"


////////////////////////////////////////////////////////////
char* local_time(time_t now, t_uint16 bufSize, char* strResult)
{
 struct tm *tm = localtime(&now);
 strftime(strResult, bufSize, "%Y-%m-%d %H:%M:%S %z (%Z)", tm);
 return strResult;
}

int local_time_hhss(time_t now, t_uint16 bufSize, const char* prefix, char* strResult)
{
 const char* strPrefix( prefix ? prefix : "\0" );
 struct tm *tm = localtime(&now);
 if ( strPrefix ) {
     strftime(strResult, bufSize, "  %H:%M", tm);
 }
 else {
     strftime(strResult, bufSize, "%H:%M", tm);
 }
 return tm->tm_sec;
}


int ctime_string (time_t now, t_uint16 bufSize, char* outBuffer)
{
 bool isOk;

 ASSERTION(outBuffer,"outBuffer");
 snprintf(outBuffer, bufSize, "%s", ctime( &now ));
 isOk = outBuffer[ 0 ]>='A' && outBuffer[ 0 ]<='Z';
 return isOk==false;
}


int ctime_trim_string (time_t now, t_uint16 bufSize, char* outBuffer)
{
 t_int16 second( 0 );
 return ctime_trim_sec_string( now, bufSize, outBuffer, second );
}


int ctime_trim_sec_string (time_t now, t_uint16 bufSize, char* outBuffer, t_int16& second)
{
 char chr;
 const bool hour_and_minute( bufSize == 0 );
 int iter( 0 ), colonIndex( -1 );
 int error( 0 );

 if ( hour_and_minute ) {
     second = local_time_hhss(now, 80, "  ", outBuffer);
 }
 else {
     error = ctime_string( now, bufSize, outBuffer );
 }

 if ( error ) return error;

 for ( ; (chr = outBuffer[ iter ])>=' '; iter++) {
     if ( chr<=0 || chr>=127 ) {
	 // ASCII must be <= 126d
	 outBuffer[ iter ] = 0;
	 return -1;
     }
     if ( chr==':' ) {
	 colonIndex = iter;
     }
 }
 if ( outBuffer[ iter ]<' ' ) {
     outBuffer[ iter ] = 0;
 }
 if ( hour_and_minute ) {
     return 0;
 }
 second = atoi( outBuffer+colonIndex+1 );
 return 0;
}


char* rfc822_date_string (gDateTime& here, int milisecs, bool showUTC, int timeDiff)
{
 static char out[ 60 ];

 // Returns yday (the day of year)
 // could also be:	gString sDoWeek( here.weekdayNames[ weekDay ].abbrev );  // e.g. "Sat"
 gString sMonth( here.monthNames[ here.month % 13 ].abbrev );  // e.g. "Dec"
 gString sUTC;  // e.g. "+0100"

 if ( showUTC ) {
    sUTC.Set( pt_diff_to_rfc822_plus_minus_mm( timeDiff ) );
 }

 // RFC-822 string (e.g. date -R)
 if ( milisecs < 0 ) {
     snprintf(out, sizeof(out)-1, "%-3s, %2d %-3s %04d %02u:%02u:%02u %-5s",
	      here.WeekdayAbbrev(),
	      here.day,
	      sMonth.Str(),
	      here.year,
	      here.hour, here.minu, here.sec,
	      sUTC.Str());
 }
 else {
     snprintf(out, sizeof(out)-1, "%-3s, %2d %-3s %04d %02u:%02u:%02u.%03d %-5s",
	      here.WeekdayAbbrev(),
	      here.day,
	      sMonth.Str(),
	      here.year,
	      here.hour, here.minu, here.sec,
	      milisecs,
	      sUTC.Str());
 }
 return out;
}


int dump_rfc822_date (FILE* fOut, gDateTime& here, int milisecs, bool showUTC, int timeDiff)
{
 if ( fOut ) {
     fprintf(fOut, "%s", rfc822_date_string( here, milisecs, showUTC, timeDiff ));
 }
 return here.yday;
}

////////////////////////////////////////////////////////////

