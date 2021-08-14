// iCntpDates.cpp

#include <stdlib.h>
#include <string.h>

#ifdef NO_ROUND
#else
#include <math.h>
#define simple_round(x) floor(x)
#endif

#include "iCntpDates.h"


////////////////////////////////////////////////////////////
t_stamp ntp_mjd_helper (const char* strMJD, const char* midnightSecs, t_uint16& ms)
{
 int secs( 0 );
 int mjd( 0 );
 const char* findDot( nil );

 // Returns 0 on error
 DBGPRINT("ntp_mjd_helper(%s,%s,ms) started\n",
	  strMJD,
	  midnightSecs);

 ms = 0;
 if ( strMJD ) {
     mjd = atoi( strMJD );
 }
 if ( midnightSecs ) {
     findDot = strchr( midnightSecs, '.' );
     if ( findDot ) {
	 secs = atoi( midnightSecs );
	 ms = atoi( findDot );
     }
     else {
	 // wrong format for peerstats...
	 return 1;
     }
 }
 DBGPRINT("mjd=%s=%d, secs: %d, ms: %d\n", strMJD, mjd, secs, ms);
 if ( mjd < MJD_1970D ) {
     return 0;
 }
 long d( MJD_TO_UNIX_TIME( mjd ) );
 d += secs;
 DBGPRINT("ctime %ld: %s", d, ctime( &d ));
 return (t_stamp)d;
}


char* new_parsed_peerstats (const char* strLine, int which, e_LocalhostParse localhostOpt, int& error)
{
 gString sCopy( (char*)strLine );
 unsigned uLen( sCopy.Length() );
 t_stamp when;
 t_uint16 ms;
 gString sIP;

 const int resolution( which );
 const char separator( '\t' );
 const bool doExcludeLocalLoop( localhostOpt==e_ignore_127 );

 error = -1;

 for (unsigned iter=1; iter<=sCopy.Length(); iter++) {
     if ( sCopy[ iter ]<' ' ) sCopy[ iter ] = ' ';
 }
 gParam entries( sCopy, " " );

 // Examples:
 // 56237 36700.973 10.152.138.3 965a -0.005596420 0.000561239 0.020581369 0.005577686

 when = ntp_mjd_helper( entries.Str( 1 ), entries.Str( 2 ), ms );
 error = when<=1;
 sIP = entries.Str( 3 );

 bool nakedIP( sIP.Find( '.' )==0 );
 bool matches127( sIP.Find( "127." )==1 );

 if ( matches127 ) {
     if ( doExcludeLocalLoop ) {
	 return nil;  // excluded
     }

     if ( localhostOpt==e_substitute_by_localhost ) {
	 sIP.Set( "localhost" );
     }
 }
 DBGPRINT("DBG: ntp_mjd_helper error: %d, when=%u {%s}\n",
	  error,
	  when,
	  strLine);

 if ( sIP.Length()<=0 || error ) {
     return nil;
 }

 error = nakedIP ? 2 : 0;

 gDateTime dttm( when );
 char* result( new char[ 256 ] );
 char* strOffset( entries.Str( 5 ) );
 double offset( atof( strOffset ) );
 float shown( (float)offset );

 if ( resolution ) {
     int divided = 10;
     if ( resolution > 1 ) {
	 divided *= 10;
     }

#ifdef NO_ROUND
     shown = (float)((int)(shown * divided) / divided);	// note: no round!
#else
     shown = (float)(simple_round( (shown * (float)divided) ) / (float)divided);
#endif

     // Quick & dirty print
     char fmt[ 16 ];
     snprintf(fmt, sizeof(fmt), "%%0.%df", resolution-1);
     snprintf(sCopy.Str(), uLen, fmt, shown);
     strOffset = sCopy.Str();
 }

 char prefixError[ 8 ] = "?";
 if ( error==0 ) {
     prefixError[ 0 ] = 0;
 }

 ASSERTION(result,"Mem");
 snprintf(result, 255, "%s%04u-%02u-%02u %02u:%02u:%02u%c%s%c%s",
	  prefixError,
	  dttm.year, dttm.month, dttm.day,
	  dttm.hour, dttm.minu, dttm.sec,
	  separator,
	  sIP.Str(),
	  separator,
	  strOffset);
 return result;
}


char* new_parsed_loopstats (const char* strLine, int which, e_LocalhostParse localhostOpt, int& error)
{
 gString sCopy( (char*)strLine );
 t_stamp when;
 t_uint16 ms;
 const char separator( '\t' );

 error = -1;

 for (unsigned iter=1; iter<=sCopy.Length(); iter++) {
     if ( sCopy[ iter ]<' ' ) sCopy[ iter ] = ' ';
 }
 gParam entries( sCopy, " " );

 //	Item	Units	Description
 //	50935	MJD	date
 //	75440.031	s	time past midnight
 //	0.000006019	s	clock offset
 //	13.778	PPM	frequency offset
 //	0.000351733	s	RMS jitter
 //	0.013380	PPM	RMS frequency jitter (aka wander)
 //	6	log2 s	clock discipline loop time constant

 // Examples:
 // 56237 35638.381 0.014288019 -49.763016 0.017379946 0.031061 10

 when = ntp_mjd_helper( entries.Str( 1 ), entries.Str( 2 ), ms );
 error = when <= 1;

 if ( error ) return nil;

 gDateTime dttm( when );
 char* result;

 result = new char[ 256 ];
 ASSERTION(result,"Mem");

 snprintf(result, 255, "%04u-%02u-%02u %02u:%02u:%02u%c%s",
	  dttm.year, dttm.month, dttm.day,
	  dttm.hour, dttm.minu, dttm.sec,
	  separator,
	  entries.Str( 3 ));
 return result;
}

////////////////////////////////////////////////////////////
sCntpDateDST::~sCntpDateDST ()
{
}

////////////////////////////////////////////////////////////

