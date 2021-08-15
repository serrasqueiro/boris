// iCntpConfig.cpp

#include <stdio.h>

#include "iCntpConfig.h"

////////////////////////////////////////////////////////////

// Other globals

int iGlobDay=-1;
int iGlobDenyAll=0;
gString sGlobClient;
gString sGlobTod( 256, '\0' );  // Time-of-day (tod), to be used in logs
gSLog lGlobLog;
FILE* fGlobOut = nil;

////////////////////////////////////////////////////////////
int sOptCntp::BuildRemoteHost ()
{
 int iPort;
 gParam remoteHost( sRemoteHost, ":" );
 char* strPort;

 if ( sRemoteHost.IsEmpty() ) return -1;

 sRemoteHostOnly = remoteHost.Str( 1 );
 strPort = remoteHost.Str( 2 );
 DBGPRINT_MIN("DBG: BuildRemoteHost: '%s':'%s'\n",
	      sRemoteHostOnly.Str(),
	      strPort);
 if ( strPort[ 0 ] ) {
     iPort = atoi( strPort );
     remotePort = (t_gPort)iPort;
     return iPort<=0;
 }
 // ...otherwise do not touch remotePort
 return 0;
}

////////////////////////////////////////////////////////////
int flush_log_a ()
{
 DBGPRINT_LOG("DBG: flush_log_a\n");
 return log_file_flush( lGlobLog, 0, GX_GETPID() );
}


char* tod_date_cntpas (gDateTime& now, t_int16 minutesAdjust, char* strResult, size_t size)
{
 const bool hour_and_minute( size==0 );
 if ( size == 0 ) {
     size = 255;
 }
 if ( strResult==nil ) {
     strResult = sGlobTod.Str();
 }
 iGlobDay = now.day;  // Just keeping the day
 if ( hour_and_minute ) {
     snprintf( strResult, size, "%02u:%02u", now.hour, now.minu );
 }
 else {
     snprintf( strResult, size, "%04u-%02u-%02u %02u:%02u:%02u",
	       now.year, now.month, now.day,
	       now.hour, now.minu, now.sec );
 }
 return strResult;
}


char* tod_date_cntpas_m (gDateTime& now, t_uint16 miliseconds, t_int16 minutesAdjust, char* strResult)
{
 const size_t size( 255 );
 if ( strResult==nil ) {
     strResult = sGlobTod.Str();
 }
 iGlobDay = now.day;  // Just keeping the day
 snprintf( strResult, size, "%04u-%02u-%02u %02u:%02u:%02u.%03u",
           now.year, now.month, now.day,
           now.hour, now.minu, now.sec, miliseconds%1000 );
 return strResult;
}


char* tod_date ()
{
 gDateTime now( gDateTime::e_Now );
 return tod_date_cntpas( now, 0, nil, 255 );
}


char* tod_date_ext (int opt_secs)
{
 gDateTime now( gDateTime::e_Now );
 return tod_date_cntpas( now, 0, nil, opt_secs == 60 ? 0 : 255 );
}


char* tod_date_miliseconds (t_uint16 miliseconds)
{
 gDateTime now( gDateTime::e_Now );
 return tod_date_cntpas_m( now, miliseconds, 0, nil );
}


int opt_to_VerboseLevel (sOptCntp& opt)
{
 return opt.isVerbose ? (opt.isVeryVerbose ? 9 : 3) : 0;
}
////////////////////////////////////////////////////////////

