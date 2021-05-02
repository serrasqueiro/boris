// icalendar.cpp

#include <string.h>

#include "icalendar.h"

// Static members
int gDateTime::minimumYear=1900;

sPairExpl gDateTime::monthNames[ 13 ];

sPairExpl gDateTime::weekdayNames[ 7 ];

sPairExpl gDateTime::weekdayNames2Letter[ 7 ];

const t_uint8 gDateTime::tblCalDurMonth[13]={
    0,
    31,
    28,
    31,
    30,
    31,
    30,
    31,
    31,
    30,
    31,
    30,
    31
};

const sPairExpl gDateTime::defaultMonths[ 13 ]={
    { "\0", nil },
    { "Jan", "January" },
    { "Feb", "February" },
    { "Mar", "March" },
    { "Apr", "April" },
    { "May", "May" },
    { "Jun", "June" },
    { "Jul", "July" },
    { "Aug", "August" },
    { "Sep", "September" },
    { "Oct", "October" },
    { "Nov", "November" },
    { "Dec", "December" }
};

const sPairExpl gDateTime::weekdays3Letter[ 7 ]={
    { "Sun", "Sunday" },
    { "Mon", "Monday" },
    { "Tue", "Tuesday" },
    { "Wed", "Wednesday" },
    { "Thu", "Thursday" },
    { "Fri", "Friday" },
    { "Sat", "Saturday" }
};

const sPairExpl gDateTime::weekdays2Letter[ 7 ]={
    { "Su", nil },
    { "Mo", nil },
    { "Tu", nil },
    { "We", nil },
    { "Th", nil },
    { "Fr", nil },
    { "Sa", nil }
};
////////////////////////////////////////////////////////////
gDateTime::gDateTime (eKindTime aKind)
    : gStorage( e_StoreExtend ),
      lastOpError( 0 ),
      year( 0 ),
      month( 0 ),
      day( 0 ),
      hour( 0 ),
      minu( 0 ),
      sec( 0 ),
      wday( 0 ),
      yday( 0 ),
      isdst( -1 )
{
 thisCalInit();
 lastOpError = thisConvertNow( aKind );
}


gDateTime::gDateTime (t_stamp aStamp)
    : gStorage( e_StoreExtend ),
      lastOpError( 0 ),
      year( 0 ),
      month( 0 ),
      day( 0 ),
      hour( 0 ),
      minu( 0 ),
      sec( 0 ),
      wday( 0 ),
      yday( 0 ),
      isdst( -1 )
{
 thisCalInit();
 lastOpError = thisConvertFromStamp( aStamp );
}


gDateTime::gDateTime (int iYear, int iMonth, int iDay,
		      t_uint8 uHour, t_uint8 uMin, t_uint8 uSec)
    : gStorage( e_StoreExtend ),
      lastOpError( 0 ),
      year( iYear ),
      month( iMonth ),
      day( iDay ),
      hour( uHour ),
      minu( uMin ),
      sec( uSec ),
      wday( 0 ),
      yday( 0 ),
      isdst( 0 )
{
 thisCalInit();
 lastOpError = iMonth<1 || iDay<1;
 if ( lastOpError==0 ) lastOpError = thisCalCheck();
}


gDateTime::~gDateTime ()
{
}


bool gDateTime::IsOk ()
{
 // Note: gStorage::IsOk is always true...
 return lastOpError==0 && isdst!=-1 && thisCalCheck()==0;
}


void gDateTime::Reset ()
{
 lastOpError = 0;
 year = 0;
 month = 0;
 day = 0;
 hour = minu = sec = 0;
 wday = 0;
 yday = 0;
 isdst = -1;
}


int gDateTime::SetError (int opError)
{
 lastOpError = opError;
 return lastOpError;
}


bool gDateTime::FromStringTOD (char* strTOD, unsigned& pos, eTodFormat todFormat)
{
 static eTodFormat resultFormat;
 gDateTime now;
 int error;
 pos = 0;
 ASSERTION(strTOD,"strTOD");
 error = FromTOD( strTOD, strlen( strTOD ), now.year, pos, todFormat, resultFormat );
 DBGPRINT("DBG: FromStringTOD(%s, ...) error: %d, %04d-%02u-%02u %02u:%02u:%02u\n",
	  strTOD,
	  error,
	  year, month, day,
	  hour, minu, sec);
 return error==0;
}


t_stamp gDateTime::GetTimeStamp ()
{
 t_stamp aStamp;
 SetError( thisConvertToStamp( *this, aStamp ) );
 return aStamp;
}


bool gDateTime::SetTimeStamp (t_stamp aStamp)
{
 return SetError( thisConvertFromStamp( aStamp ) )==0;
}


t_uint8 gDateTime::DaysOfMonth (int aYear, t_uint8 aMonth)
{
 t_uint8 nDays;
 if ( aMonth<1 || aMonth>12 ) return 0;
 nDays = tblCalDurMonth[aMonth];
 if ( aMonth==2 ) nDays += ((aYear%4)==0);
 return nDays;
}


gStorage* gDateTime::NewObject ()
{
 gDateTime* a = new gDateTime( GetTimeStamp() );
 return a;
}


t_uchar* gDateTime::ToString (const t_uchar* uBuf)
{
 if ( uBuf==nil ) return nil;
 sprintf( (char*)uBuf, "%04u-%02u-%02u %02u:%02u:%02u",
	  ((unsigned)year) % 10000, month, day,
	  hour, minu, sec );
 return (t_uchar*)uBuf;
}


bool gDateTime::SaveGuts (FILE* f)
{
 if ( CanSave( f )==false ) return false;
 return
     fprintf(f,"%04d/%02u/%02u,%02u:%02u:%02u",
	     year, month, day,
	     hour, minu, sec)>0;
}


bool gDateTime::RestoreGuts (FILE* f)
{
 t_stamp aStamp;
 int d1;
 unsigned d2, d3, h1, h2, h3;

 if ( CanRestore( f )==false ) return false;
 Reset();
 lastOpError =
     fscanf(f,"%d/%u/%u,%u:%u:%u",
	    &d1, &d2, &d3,
	    &h1, &h2, &h3);
 year = gRANGE0(d1,-65000,65000);
 month = gRANGE0(d2,1,12);
 day = gRANGE0(d3,1,31);
 hour = gRANGE0(h1,0,23);
 minu = gRANGE0(h2,0,59);
 sec = gRANGE0(h3,0,60);  // You an have a leap second sometimes (e.g. 23:59:60)
 if ( lastOpError!=6 ) return SetError( 1 );
 lastOpError = 0;
 return SetError( thisConvertToStamp( *this, aStamp ) )==0;
}


int gDateTime::thisCalCheck ()
{
 return thisDateTimeCheck( *this, minimumYear );
}


int gDateTime::thisDateTimeCheck (gDateTime& aDtTm, int minYear)
{
 if ( thisDateCheck( aDtTm.year, aDtTm.month, aDtTm.day, minYear )!=0 )
     return 1;
 if ( thisTimeCheck( aDtTm.hour, aDtTm.minu, aDtTm.sec )!=0 )
     return 2;
 return 0;
}


int gDateTime::thisDateCheck (int aYear, t_uint8 aMonth, t_uint8 aDay, int minYear)
{
 t_uint8 nDays;
 if ( aYear<minYear ) return 1;
 if ( aYear>65000 ) return 2;
 nDays = DaysOfMonth( aYear, aMonth );
 if ( nDays==0 ) return 3;
 if ( aDay<1 ) return 4;
 return aDay>nDays ? 5 : 0;
}


int gDateTime::thisTimeCheck (t_uint8 aHour, t_uint8 aMin, t_uint8 aSec)
{
 return
     (aHour<24 &&
      aMin<60 && aSec<=61)==false;
}


int gDateTime::thisConvertNow (eKindTime aKind)
{
 int result( thisConvertFromStamp( time(NULL), aKind==e_UTC ) );
 return result;
}


int gDateTime::thisConvertFromStamp (t_stamp aStamp, bool isUTC)
{
 struct tm* pTM;
 time_t tStamp = (time_t)aStamp;

 if ( isUTC )
     pTM = gmtime( &tStamp );
 else
     pTM = localtime( &tStamp );
 if ( pTM==nil ) return -1;
 year = (t_int16)pTM->tm_year+1900;
 month = (t_uint8)pTM->tm_mon+1;
 day = (t_uint8)pTM->tm_mday;
 hour = (t_uint8)pTM->tm_hour;
 minu = (t_uint8)pTM->tm_min;
 sec =(t_uint8)pTM->tm_sec;

 wday = (t_uint8)pTM->tm_wday;
 yday = (t_uint16)pTM->tm_yday;
 isdst = (t_int8)pTM->tm_isdst;
 return 0;
}


int gDateTime::thisConvertToStamp (gDateTime& aDtTm, t_stamp& aStamp)
{
 // Converts aDtTm into a timestamp (aStamp)
 time_t tStamp;
 struct tm aTM;

 // Within gobj lib, stamp 0 is invalid!, see below
 aStamp = 0;  // Default result already here, since mktime may fail, obviously!
 if ( thisConvertTo_libc_tm( aDtTm, &aTM )!=0 )
     return 4;  // Invalid date
 tStamp = mktime( &aTM );
 DBGPRINT_MIN("{[%d,%d,%d],dst?%d|%ld}",aTM.tm_year,aTM.tm_mon,aTM.tm_mday,aTM.tm_isdst,(long)tStamp);
 DBGPRINT_MIN("{tzname(%d)=%s:%s|%s}",daylight,tzname[daylight],tzname[0],tzname[1]);
 if ( tStamp==(time_t)-1 ) {
     // Cannot be represented as calendar time (a.k.a epoch time)
     return 8;
 }
 aStamp = (t_stamp)tStamp;
 // All ok, so returns 0
 return 0;
}


int gDateTime::thisConvertTo_libc_tm (gDateTime& aDtTm, struct tm* pTM)
{
 ASSERTION(pTM!=nil,"pTM!=nil");
 memset( pTM, 0x0, sizeof(struct tm) );
 // memset: not needed, but useful if tm is changed,
 // or if some checks fail
 if ( thisDateTimeCheck( aDtTm, 1900 )!=0 )
     return 1;
 // (Note: parameter 1900 depends clearly on libc functions!)
 // Here we are sure year>=1900, and month>=1, ...etc
 pTM->tm_year = aDtTm.year-1900;
 pTM->tm_mon = aDtTm.month-1;
 pTM->tm_mday = aDtTm.day;
 pTM->tm_hour = aDtTm.hour;
 pTM->tm_min = aDtTm.minu;
 pTM->tm_sec = aDtTm.sec;
 pTM->tm_wday = aDtTm.wday;
 pTM->tm_yday = (int)aDtTm.yday;
 pTM->tm_isdst = aDtTm.isdst;
 DBGPRINT_MIN("{tm_isdst=%d}",aDtTm.isdst);
 return aDtTm.isdst==-1 ? -1 : 0;
}


void gDateTime::thisCalInit ()
{
 int iter;

 DBGPRINT("DBG: thisCalInit: monthNames[ 1 ]={%s}\n",monthNames[ 1 ].abbrev);
 if ( monthNames[ 1 ].abbrev[ 0 ]==0 || monthNames[ 1 ].name==nil ) {
     // Use default
     for (iter=1; iter<=12; iter++) {
	 strncpy( monthNames[ iter ].abbrev, defaultMonths[ iter ].abbrev, 4 );
	 monthNames[ iter ].name = defaultMonths[ iter ].name;
     }
 }

 if ( weekdayNames[ 0 ].abbrev[ 0 ]==0 || weekdayNames[ 0 ].name==nil ) {
     for (iter=0; iter<7; iter++) {
	 memcpy( weekdayNames[ iter ].abbrev, weekdays3Letter[ iter ].abbrev, 4 );
	 weekdayNames[ iter ].name = weekdays3Letter[ iter ].name;

	 memcpy( weekdayNames2Letter[ iter ].abbrev, weekdays2Letter[ iter ].abbrev, 4 );
	 weekdayNames2Letter[ iter ].abbrev[ 2 ] = 0;  // Two chars max.!
	 weekdayNames2Letter[ iter ].name = weekdayNames2Letter[ iter ].abbrev;
	 DBGPRINT_MIN("DBG: thisCalInit: monthNames[ 1 ]={%s}, weekdayNames[ %d ]={%s} end\n",monthNames[ 1 ].abbrev,iter,weekdayNames[ iter ].abbrev);
     }
 }
}


int gDateTime::FromTOD (char* strTOD, int len, int optYear, unsigned& pos, eTodFormat todFormat, eTodFormat& resultFormat)
{
 const int idxDateBlank( 10 ), idxTimeNoSecs( 16 ), idxTimeBlank( 19 );
 const int idxJanBlank( 6 );  // e.g. "Jan  1"
 char* strTime;

 resultFormat = e_YYYYMMDD_HHMMSS;

 if ( todFormat==e_MonthDay_opt_HHMMSS ) {
     return -2;  // Unsupported
 }

 DBGPRINT("DBG: FromTOD {%s} len: %d idx: %d. ASCII #%d: %dd, ASCII #%d: %dd.\n", strTOD, len, idxJanBlank, idxDateBlank, strTOD[ idxDateBlank ], idxTimeBlank, strTOD[ idxTimeBlank ]);
 if ( len<=idxJanBlank ) return -1;

 if ( strTOD[ idxJanBlank ]==' ' ) {
     month = 0;
     for (short idx=1; idx<=12; idx++) {
	 if ( strncmp( strTOD, monthNames[ idx ].abbrev, 3 )==0 ) {
	     month = (t_uint8)idx;
	     break;
	 }
     }
     if ( month ) {
	 strTime = strTOD + idxJanBlank + 1;
	 if ( strTime[ 8 ]==' ' ) {
	     resultFormat = e_MonthDay_HHMMSS;
	     year = optYear;
	     day = atoi( strTOD+3 );

	     strTime[ 8 ] = 0;
	     hour = atoi( strTime );
	     minu = atoi( strTime+3 );
	     sec = atoi( strTime+6 );

	     pos = idxJanBlank + 1 + 8 +1;
	     return 0;
	 }
	 else {
	     return -1;
	 }
     }
     return 2;  // Month abbreviation not found
 }

 sec = 0;
 if ( todFormat==e_YYYYMMDD_opt_HHMM_wORwoSS ) {
     if ( strTOD[ idxDateBlank ]==' ' &&  (strTOD[ idxTimeNoSecs ]==' ' || strTOD[ idxTimeNoSecs ]==0) ) {
	year = atoi( strTOD );
	month = atoi( strTOD+5 );
	day = atoi( strTOD+8 );
	strTOD[ idxDateBlank ] = 0;
	strTime = strTOD + idxDateBlank + 1;
	hour = atoi( strTime );
	minu = atoi( strTime+3 );
	DBGPRINT("DBG: todFormat: %d, len=%d (%d), idxTimeBlank=%d, %dd %dd\n{%s}\n{%s}, strTime={%s}\n\n",
		todFormat,
		len, strlen( strTOD ),
		idxTimeBlank,
		strTOD[ idxDateBlank ], strTOD[ idxTimeNoSecs ],
		strTOD,
		strTOD + idxTimeNoSecs,
		strTime);
	return 0;
     }
 }

 if ( len >= idxTimeBlank ) {
     if ( strTOD[ idxDateBlank ]==' ' &&
	  (strTOD[ idxTimeBlank ]==' ' || strTOD[ idxTimeBlank ]==0) ) {
	 strTOD[ idxDateBlank ] = 0;
	 strTOD[ idxTimeBlank ] = 0;
	 strTime = strTOD + idxDateBlank + 1;
	 pos = idxTimeBlank+1;
	 year = atoi( strTOD );
	 month = atoi( strTOD+5 );
	 day = atoi( strTOD+8 );
	 hour = atoi( strTime );
	 minu = atoi( strTime+3 );
	 sec = atoi( strTime+6 );
	 return 0;
     }
 }

 return -1;
}

////////////////////////////////////////////////////////////
// gTimeStamp - Stamp class
// ---------------------------------------------------------
gTimeStamp::gTimeStamp (t_stamp aStamp)
    : gUInt( aStamp ),
      iValid( aStamp!=0 )  //iValid=1 means valid!
{
}


gTimeStamp::gTimeStamp (gDateTime& aDtTm)
    : iValid( aDtTm.IsOk()==true )
{
 SetStamp( aDtTm.GetTimeStamp() );
}


gTimeStamp::~gTimeStamp ()
{
}


bool gTimeStamp::IsOk ()
{
 return (bool)(iValid = iValue!=0);
}


bool gTimeStamp::SetStamp (t_stamp aStamp)
{
 return SetUInt( (unsigned)aStamp )!=0;
}


unsigned gTimeStamp::SetUInt (unsigned v)
{
 gUInt::SetUInt( v );
 IsOk();
 return v;
}

////////////////////////////////////////////////////////////

