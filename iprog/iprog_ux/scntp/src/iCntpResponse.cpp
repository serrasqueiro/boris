// iCntpResponse.cpp

#include <string.h>

#include "ifile.h"

#include "iCntpDates.h"
#include "iCIO.h"

////////////////////////////////////////////////////////////
int cntp_VNTP_other_ClientReq (char* sBuf, sRawDtTm& rawDtTm)
{
 return -1;  // Handshaked-VNTP not finished
}
////////////////////////////////////////////////////////////
int cntp_VNTP_ClientReq (char* sBuf, t_int8& basicRequest, sRawDtTm& rawDtTm)
{
 static int error;
 static char threeLetBuf[ 8 ];
 static unsigned long aSec, mSec;
 int iClientMask = RAW_REFTIME_DEFAULT;

 // sBuf is as follow:
 //	Ab 0123456789 012 [-567]
 //		'Ab' is one upper-case letter A, followed by a second letter
 //		(a lower-case letter known as basic-request.)
 //		There is blank
 //		...then ten digits (known as client-sec-reference)
 //		...and three digits with indication of miliseconds
 //		(known as client-milis-ref).
 //		The optional integer value (min=-128, max=127) is known as
 //		the client-mask.

 basicRequest = -1;
 strncpy( threeLetBuf, sBuf, 2 );
 threeLetBuf[ 2 ] = 0;  // safety only

 switch ( *sBuf ) {
 case 'A':
     sBuf++;
     basicRequest = *(sBuf++);
     if ( basicRequest<'a' || basicRequest>'z' ) basicRequest = -2;
     if ( *(sBuf++) != ' ' ) return 2;
     error = sscanf( sBuf, "%lu %lu %d", &aSec, &mSec, &iClientMask );
     DBGPRINT("DBG: error=%d, aSec=%lu, mSec=%lu, mask=%d [%s]\n",error,aSec,mSec,iClientMask,sBuf);
     if ( error<2 ) return 2;  // basic parsing error
     if ( aSec==0 || mSec>=1000 ) return 3;  // out-of-range
     if ( iClientMask<-128 || iClientMask>127 ) return 3;
     rawDtTm.secs = (t_uint32)aSec;
     rawDtTm.milisecs = (t_uint16)mSec;
     rawDtTm.refTime = (t_int8)iClientMask;
     return 0;

 default:
     return cntp_VNTP_other_ClientReq( sBuf, rawDtTm );
 }

 return 1;
}

int cntp_ClientResp (sRawDtTm& clientTime,
		     sRawDtTm& current,
		     t_int8& basicRequest,
		     char* sBuf)
{
 // Returns 0 if time on client is to be advanced; -1 if it is to be skewed, or 1 on other errors.
 // sBuf is filled-up: for performance, no checking is made on the size of the buffer

 bool isTimedSkew;
 float secClient = clientTime.GetSecs();
 float secSystem = current.GetSecs();
 float delta = secClient - secSystem;

 DBGPRINT_MIN("DBG: delta is: %0.3f\n",delta);

 if ( basicRequest=='b' ) {
     sprintf( sBuf, "Br %08lu %03u",
	      (unsigned long)current.secs,
	      (unsigned)current.milisecs );
     // 'Br' stands for '*B*asic *r*esponse'
     return 0;
 }

 // We usually expect the client to have a time lower than ours...
 if ( delta<0 ) {
     sprintf( sBuf, "Bq %08lu %03u %0.3f",
	      (unsigned long)current.secs,
	      (unsigned)current.milisecs,
	      delta );
     // 'Bq' stands for '*B*asic ac*q*uire'
     return 0;
 }

 // ...otherwise we respond with 1/10th of the time-clock skew,
 // maximum skew of 55 seconds, and:
 // as long as the time does not 'leap'.
 // For instance: if our client clock is 03:00:33 (ca. 3pm) we will
 // not skew more than 32 seconds.
 // In this case we indicate 'Bt' (*B*asic *t*imed skew),
 // instead of the normal skew indication: 'Bs' (*B*asic *s*kew).

 sprintf( sBuf, "Bt 0 0" );

 // Note this part is not time critical: we can take our time to
 // calculate the best skew.
 gDateTime calNow( clientTime.secs );
 float hisSec( (float)(calNow.sec+1) );

 delta /= 10;
 if ( (isTimedSkew = (delta >= hisSec))==true ) delta = hisSec-2.0;
 if ( delta < 0.0 ) delta = 0.0;
 if ( delta > 55.0 ) delta = 55.0;  // Academic check

 t_stamp newProposedStamp = (t_stamp)clientTime.secs;
 if ( newProposedStamp <= (t_stamp)delta ) return 1;
 newProposedStamp -= (t_uint32)delta;

 sprintf( sBuf, "B%c %lu 0",
	  isTimedSkew ? 't' : 's',
	  (unsigned long)newProposedStamp );
 DBGPRINT("DBG: client-skew (timed?%c): hisSec=%0.1f, delta=%0.1f\n",
	  ISyORn(isTimedSkew),
	  hisSec,
	  delta);

 return -1;  // just indication of skew, not an error!
}
////////////////////////////////////////////////////////////
int cntp_BasicVNTP_HandleClient (int handle)
{
 static const ssize_t stdMessageResp = 3+10+1+3;
 static char sBuf[ 256 ];
 static sRawDtTm current;
 ssize_t readBytes;
 int code;
 int errors=0;
 char chr;
 t_int8 basicRequest;

 ASSERTION(handle>=0,"handle>=0");
 readBytes = cxio_read( handle, sBuf, 250 );
 if ( readBytes<=2 || readBytes>128 ) return 1;  // short or long read

 // Strip tail characters, including blanks
 for (; readBytes>0; ) {
	chr = sBuf[ --readBytes ];
	if ( chr>' ' ) break;
	errors += chr<'\n';
	sBuf[ readBytes ] = 0;
 }
 for (; readBytes>0; readBytes--) {
     if ( sBuf[ readBytes ]<' ' ) sBuf[ readBytes ] = ' ';
     // Note: since sBuf is of type char, unreadable characters
     // like 127d are transformed into blanks.
 }
 //for (readBytes=0; readBytes<(ssize_t)strlen(sBuf); readBytes++) printf("%02d:\t%3ud\t%c\n",readBytes,(unsigned)(t_uchar)sBuf[readBytes],sBuf[readBytes]);

 sRawDtTm rawDtTm;
 code = cntp_VNTP_ClientReq( sBuf, basicRequest, rawDtTm );

#ifdef DEBUG
 gDateTime cal;  // ...( rawDtTm.secs );
 int convError = rawDtTm.ConvertToDate( cal );
 DBGPRINT("DBG: cntp_ClientReq('%c') returned %d rawDtTm=[%04u-%02u-%02u %02u:%02u:%02u.%03u]:conv=%d\n",
	  basicRequest < ' ' ? '?' : basicRequest,
	  code,
	  cal.year,
	  cal.month,
	  cal.day,
	  cal.hour,
	  cal.minu,
	  cal.sec,
	  rawDtTm.milisecs,
	  convError);
#endif //DEBUG

 if ( code ) return code;

 // Fills-up current system time-of-day
 cntp_GetTimeOfDay( current );

 code = cntp_ClientResp( rawDtTm, current, basicRequest, sBuf );
 DBGPRINT("DBG: cntp_ClientResp returned %d [%s]\n",
	  code,
	  sBuf);
 ASSERTION(code<=1,"code<=1");
 strcat( sBuf, "\n" );

#ifdef DEBUG_MIN
 for (ssize_t iDbg=0; iDbg<=stdMessageResp; iDbg++) {
     printf("MSG#%02d: %c\t%3dd\n",
	    iDbg,
	    sBuf[iDbg] < ' ' ? '?' : sBuf[iDbg],
	    (int)(unsigned char)sBuf[iDbg]);
 }
#endif //DEBUG_...

 code = cxio_write( handle, sBuf, stdMessageResp+1 );
 return code<stdMessageResp;  // Return 1 on error
}
////////////////////////////////////////////////////////////

