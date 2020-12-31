// iCntpRawResp.cpp

#include <string.h>

#include "iCntpRawResp.h"

////////////////////////////////////////////////////////////
int sRawDtTmResponse::TrimServerMsg ()
{
 int count=0, len=strlen( serverMsg );
 for ( ; len>0; ) {
     char c( serverMsg[ --len ] );
     if ( c>' ' ) break;
     serverMsg[ len ] = 0;
     count++;
 }
 return count;
}

int sRawDtTmResponse::ParseServerResponse (char* str)
{
 long aSec, aMil=-1;

 if ( str!=nil )
     snprintf( serverMsg, sizeof(serverMsg)-1, str );
 serverMsg[ sizeof(serverMsg)-1 ] = 0;
 if ( serverMsg[0]==0 )
     Reset();
 if ( serverMsg[2]!=' ' )
     return 1;  // Invalid server message
 strncpy( sTwoChr, serverMsg, 2 ); sTwoChr[ 2 ] = 0;
 if ( sscanf( serverMsg+3, "%ld %ld", &aSec, &aMil )!=2 ||
      aSec<=0 ||
      aMil<0 || aMil>=1000 )
     return 2;  // Server message has invalid fields

 serverTime.secs = (t_uint32)aSec;
 serverTime.milisecs = (t_uint16)aMil;
 DBGPRINT("DBG: serverMsg='%s': %ld.%03ld\n",
	  serverMsg,
	  aSec,
	  aMil);

 return 0;
}
////////////////////////////////////////////////////////////

