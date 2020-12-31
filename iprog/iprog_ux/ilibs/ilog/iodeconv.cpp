// iodeconv.cpp

#include "iodeconv.h"
#include "iarg.h"


#define SHR(a, b)       \
  (-1 >> 1 == -1        \
   ? (a) >> (b)         \
   : (a) / (1 << (b)) - ((a) % (1 << (b)) < 0))

////////////////////////////////////////////////////////////
// Static members
// ---------------------------------------------------------
char* gDateString::strDatePrintFormat=(char*)"%04d-%02u-%02u %02u:%02u:%02u";
const short gDateString::monthHashSize=251;
t_int8 gDateString::monthHashed[ 256 ];

////////////////////////////////////////////////////////////
static int tp_time_diff_leap (const struct tm *a, const struct tm *b, int& leapDays)
{
  /* Compute intervening leap days correctly even if year is negative.
     Take care to avoid int overflow in leap day calculations,
     but it's OK to assume that A and B are close to each other.  */
  int a4 = SHR(a->tm_year, 2) + SHR(TM_YEAR_BASE, 2) - ! (a->tm_year & 3);
  int b4 = SHR(b->tm_year, 2) + SHR(TM_YEAR_BASE, 2) - ! (b->tm_year & 3);
  int a100 = a4 / 25 - (a4 % 25 < 0);
  int b100 = b4 / 25 - (b4 % 25 < 0);
  int a400 = SHR(a100, 2);
  int b400 = SHR(b100, 2);

  int years = a->tm_year - b->tm_year;
  int days;

  leapDays = (a4 - b4) - (a100 - b100) + (a400 - b400);

  days = (365 * years + leapDays
              + (a->tm_yday - b->tm_yday));
  DBGPRINT("DBG: days: %d (%d - %d + ...), leapDays: %d\n", days, a->tm_yday, b->tm_yday, leapDays);
  return 60 * (60 * (24 * days + (a->tm_hour - b->tm_hour))
              + (a->tm_min - b->tm_min))
            + (a->tm_sec - b->tm_sec);
}


int tp_time_diff (const struct tm *a, const struct tm *b)
{
 int leapDays( 0 );
 ASSERTION(a,"a");
 ASSERTION(b,"b");
 return tp_time_diff_leap( a, b, leapDays );
}


int tp_to_calendar_date (const struct tm *pTM, gDateTime &toTime)
{
 if ( pTM==nil ) return -1;
 toTime.year = (t_int16)pTM->tm_year+1900;
 toTime.month = (t_uint8)pTM->tm_mon+1;
 toTime.day = (t_uint8)pTM->tm_mday;
 toTime.hour = (t_uint8)pTM->tm_hour;
 toTime.minu = (t_uint8)pTM->tm_min;
 toTime.sec =(t_uint8)pTM->tm_sec;
 toTime.wday = (t_uint8)pTM->tm_wday;
 toTime.yday = (t_uint16)pTM->tm_yday;
 toTime.isdst = (t_int8)pTM->tm_isdst;
 return (int)toTime.yday;
}


char* pt_diff_to_rfc822_plus_minus_mm (int diffSecs)
{
 static char rfcDiff[ 8 ];
 int uPositive( diffSecs >= 0 ? diffSecs : -diffSecs );
 int minutes( uPositive / 60 );
 int hours( minutes / 60 );
 snprintf(rfcDiff, 6, "%c%02d%02d", diffSecs>=0 ? '+' : '-', hours % 60, minutes % 60);
 rfcDiff[ 6 ] = rfcDiff[ 7 ] = 0;
 return rfcDiff;
}

////////////////////////////////////////////////////////////
// gIOLarge - Large storage handling
// ---------------------------------------------------------
gIOLarge::gIOLarge (eStorage aKind)
    : gStorage( aKind ),
      llVal( 0 )
{
}


gIOLarge::gIOLarge (int refKind, t_uint64 val)
    : gStorage( (eStorage)refKind ),
      llVal( val )
{
}


gIOLarge::~gIOLarge ()
{
}


bool gIOLarge::IsOk ()
{
 return gStorage::IsOk()==true && llVal!=0;
}


char* gIOLarge::Str ()
{
 t_uchar tBuf[ 50 ];
 ToString( tBuf );
 sLarge.Set( tBuf );
 return sLarge.Str();
}


gStorage* gIOLarge::NewObject ()
{
 ASSERTION_FALSE("gIOLarge::NewObject: redefine");
 return nil;
}


void gIOLarge::Reset ()
{
 gStorage::Reset();
 llVal = 0;
}


t_uchar* gIOLarge::ToString (t_uchar* uBuf)
{
 ASSERTION(uBuf!=nil,"uBuf!=nil");
 sprintf( (char*)uBuf, "%llu", llVal );
 return uBuf;
}


void gIOLarge::Show (bool doShowAll)
{
 printf("gIOLarge");
 gStorage::Show( doShowAll );
 printf("%s",doShowAll?"\n":"\0");
}

////////////////////////////////////////////////////////////
// gIOUDecimal - UDecimal : Decimal Conversion
// ---------------------------------------------------------
gIOUDecimal::gIOUDecimal (t_uint64 val)
    : gIOLarge( (int)e_ULongInt, val )
{
}


gIOUDecimal::~gIOUDecimal ()
{
}


char* gIOUDecimal::ToHexStr (bool showUpper, bool showPrefix)
{
 thisConvertToHexStr( llVal, showUpper, showPrefix, sLarge );
 return sLarge.Str();
}


int gIOUDecimal::thisConvertToHexStr (t_uint64 aLlVal, bool showUpper, bool showPrefix, gString& sResult)
{
 t_uint32 aVal( (aLlVal & (t_uint64)(MAX_UINT_UL<32)) >> 32 );
 gString sHigh;
 gString sLow;

 // Highest 4 octets converted
 if ( aVal>0 ) {
     thisConvertUInt32ToHex( aVal, showUpper, false, sHigh );
 }
 aVal = aLlVal & (t_uint64)(MAX_UINT_UL);
 // And now convert the lowest 4 octets
 thisConvertUInt32ToHex( aVal, showUpper, false, sLow );
 sResult.SetEmpty();
 if ( showPrefix ) sResult.Add( (char*)"0x" );
 sResult.AddString( sHigh );
 sResult.AddString( sLow );
 return 0;
}


int gIOUDecimal::thisConvertUInt32ToHex (t_uint32 aVal, bool showUpper, bool showPrefix, gString& sResult)
{
 char cFmt[ 20 ];
 char strResult[ 50 ];
 const char* leftFmt = "\0";  // or 08, or 8, or...none.
 unsigned long uVal = aVal;

 sprintf( cFmt, "%%%sl%c", leftFmt, showUpper ? 'X' : 'x' );
 sprintf( strResult, cFmt, uVal );
 sResult = strResult;
 return 0;
}

////////////////////////////////////////////////////////////
gIO2Powers::gIO2Powers (int val)
    : gInt( thisCalcLog2Power( val, sumMods ) )
{
}


gIO2Powers::~gIO2Powers ()
{
}


int gIO2Powers::thisCalcLog2Power (int val, int& mods)
{
 int thisMod, count( 0 );
 for (mods=0; val>=2; val>>=1) {
     thisMod = val & 1;
     mods += thisMod;
     count++;
 }
 return count;
}


int gIO2Powers::thisCalcPower2 (int base, int expnt)
{
 int result( 1 );
 if ( base<=0 ) return 0;
 for ( ; expnt>0; expnt--) {
     //val <<= 1;  // ...or:	val *= 2
     result *= base;
 }
 return result;
}

////////////////////////////////////////////////////////////
gEIntScale::gEIntScale (int value, int aScale)
    : gInt( value ),
      scale( aScale )
{
}


gEIntScale::gEIntScale (gEIntScale& value)
    : gInt( value.iValue ),
      scale( value.scale )
{
}


gEIntScale::~gEIntScale ()
{
}


bool gEIntScale::ConvertFromStr (char* str)
{
 return thisConvertFromStr( str, iValue, scale )==0;
}


gEIntScale& gEIntScale::operator= (gEIntScale& value)
{
 iValue = value.GetInt();
 scale = value.scale;
 return *this;
}


int gEIntScale::thisConvertFromStr (char* str, int& result, int aScale)
{
 t_int32 aInt=0, bInt=0;
 char* inStr;

 if ( str==nil ) return -1;
 gParam aParam( str, (char*)".", gParam::e_StopSplitOnFirst );
 unsigned n = aParam.N();
 inStr = aParam.Str( 1 );
 if ( gStorageControl::Self().ConvertToInt32( inStr, aInt ) ) return -1;
 if ( n>1 ) {
     if ( gStorageControl::Self().ConvertToInt32( aParam.Str( 2 ), bInt ) ) return -2;
     if ( str[0]=='-' ) bInt = -bInt;
 }
 result = aInt*aScale + bInt;
 return 0;
}
////////////////////////////////////////////////////////////
gDateString::gDateString (eDateFormat dateFormat)
    : gDateTime( e_Now ),
      kindFormat( dateFormat ),
      usedYear( year ),
      sDatePrintFormat( strDatePrintFormat )
{
}


gDateString::gDateString (char* str, eDateFormat dateFormat)
    : kindFormat( dateFormat ),
      usedYear( 0 ),
      sDatePrintFormat( strDatePrintFormat )
{
 thisConvertDtTmStrToDateTime( str, dateFormat );
}


gDateString::gDateString (gString& sDtTm, eDateFormat dateFormat)
    : kindFormat( dateFormat ),
      usedYear( 0 ),
      sDatePrintFormat( strDatePrintFormat )
{
 thisConvertDtTmStrToDateTime( sDtTm.Str(), dateFormat );
}


gDateString::gDateString (gString& sDate)
    : kindFormat( e_LogLinear ),
      usedYear( 0 ),
      sDatePrintFormat( strDatePrintFormat )
{
 gString sTime;
 thisConvertToDateTime( sDate, sTime, e_LogLinear, *this );
}


gDateString::gDateString (gString& sDate, gString& sTime, eDateFormat dateFormat)
    : kindFormat( dateFormat ),
      usedYear( year ),
      sDatePrintFormat( strDatePrintFormat )
{
 thisConvertToDateTime( sDate, sTime, dateFormat, *this );
}


gDateString::gDateString (gDateString& copy)
    : gDateTime( copy.year, copy.month, copy.day,
		 copy.hour, copy.minu, copy.sec ),
      kindFormat( copy.kindFormat ),
      usedYear( year ),
      sDatePrintFormat( strDatePrintFormat )
{
}


gDateString::gDateString (t_stamp aStamp)
    : gDateTime( aStamp ),
      kindFormat( e_LogLinear ),
      usedYear( year ),
      sDatePrintFormat( strDatePrintFormat )
{
}


gDateString::~gDateString ()
{
}


t_uchar* gDateString::ToString (t_uchar* uBuf)
{
 if ( uBuf==nil ) return nil;
 if ( sDatePrintFormat.IsEmpty() ||
      sDatePrintFormat.Find("%lu") ) {
     // If some '%lu' was specified, or date format is simply empty,
     // assume the time-stamp is to be shown.
     sprintf( (char*)uBuf, sDatePrintFormat.Str(), (unsigned long)GetTimeStamp() );
     return uBuf;
 }
 sprintf( (char*)uBuf, sDatePrintFormat.Str(),
	  year, month, day,
	  hour, minu, sec );
 return uBuf;
}


bool gDateString::SetDefaultDatePrintFormat (char* s)
{
 strDatePrintFormat = s ? s : (char*)"%lu";
 sDatePrintFormat.Set( strDatePrintFormat );
 return s!=nil;
}


bool gDateString::SetDatePrintFormat (char* s)
{
 // s=nil implies the default will be used
 if ( s==nil )
     sDatePrintFormat.Set( strDatePrintFormat );
 else
     sDatePrintFormat.Set( s );
 return true;
}


bool gDateString::SetLocalTime (t_stamp aStamp)
{
 int convError;
 if ( aStamp==0 ) {
     aStamp = GetTimeStamp();
 }
 convError = thisConvertFromStampUTCorLocal( aStamp, e_ReferenceLocal );
 return convError==0;
}


t_stamp gDateString::GetTimeStamp ()
{
 t_stamp aStamp;
 SetError( thisConvertToStamp( *this, aStamp ) );
 // This method is repeated because needed a corrected 'daylight' for gobj...
 return aStamp;
}


gDateString& gDateString::operator= (gDateString& copy)
{
 year = copy.year;
 month = copy.month;
 day = copy.day;
 hour = copy.hour;
 minu = copy.minu;
 sec = copy.sec;
 wday = copy.wday;
 yday = copy.yday;
 isdst = copy.isdst;
 return *this;
}


int gDateString::thisConvertToDateTime (gString& sDate,
					gString& sTime,
					eDateFormat dateFormat,
					gDateString& my)
{
 // Return 0 if all ok
 int error( thisConvertDtTmToDateTime( sDate, sTime, dateFormat, my ) );
 DBGPRINT_MIN("DBG: thisConvertToDateTime({%s|%s}, error=%d)\n",
	      sDate.Str(),
	      sTime.Str(),
	      error);
 if ( error==0 ) return my.GetTimeStamp()==0;
 Reset();
 return my.SetError( 2 );
}


int gDateString::thisConvertDtTmToDateTime (gString& sDate,
					    gString& sTime,
					    eDateFormat dateFormat,
					    gDateString& my)
{
 // Return 0 if all ok
 t_uint32 value;
 gList lDate, lTime;
 gDateString aDtTm( nil );
 gString aDate( sDate );
 gString aTime( sTime );

 if ( dateFormat==e_LogMaster ) {
     unsigned len( aTime.Length() );
     t_uchar chr1( aDate[ 1 ] );
     t_uchar chr2( aTime[ len ] );
     if ( chr1=='[' && chr2==']' ) {
	 aDate.Delete( 1, 1 );
	 aTime.Delete( len, len );
     }
 }
 aDate.SplitAnyChar( '-', lDate );
 aTime.SplitAnyChar( ':', lTime );

 DBGPRINT_MIN("DBG: lDate='%s,%s,%s', lTime='%s,%s,%s'\n",
	      lDate.Str(1),
	      lDate.Str(2),
	      lDate.Str(3),
	      lTime.Str(1),
	      lTime.Str(2),
	      lTime.Str(3));

 if ( gStorageControl::Self().ConvertToUInt32( lDate.Str(1), value ) ) return -1;
 aDtTm.year = value;
 if ( gStorageControl::Self().ConvertToUInt32( lDate.Str(2), value ) ) return -1;
 aDtTm.month = value;
 if ( gStorageControl::Self().ConvertToUInt32( lDate.Str(3), value ) ) return -1;
 aDtTm.day = value;

 if ( lTime.N() ) {
     if ( gStorageControl::Self().ConvertToUInt32( lTime.Str(1), value ) ) return -1;
     aDtTm.hour = value;
     if ( gStorageControl::Self().ConvertToUInt32( lTime.Str(2), value ) ) return -1;
     aDtTm.minu = value;
     gString s( lTime.Str(3) );
     if ( s.IsEmpty()==false ) {
	 if ( gStorageControl::Self().ConvertToUInt32( s.Str(), value ) ) return -1;
	 aDtTm.sec = value;
     }
 }
 my = aDtTm;
 return 0;
}


int gDateString::thisConvertDtTmStrToDateTime (char* str, eDateFormat dateFormat)
{
 int result( 0 );
 char* strField;
 t_int8 monthAuto( -1 ), monthNow( -1 );
 int dayAuto( -1 );

 DBGPRINT_MIN("DBG: thisConvertDtTmStrToDateTime('%s',%d)\n",
	      str,
	      (int)dateFormat);
 if ( str==nil ) return SetError( -1 );

 gString basic( str );
 gList resultL;
 gString sDate, sTime;

 basic.SplitAnyChar( ' ', resultL );

 unsigned i( 1 ), n( resultL.N() ), keepIdx( 0 );
 unsigned count, keepCount( 0 );
 unsigned timeCount, keepTimeCount( 0 );

 sRemaining.SetEmpty();
 thisHashMonthNamesEn();

 strField = resultL.Str( 1 );
 if ( usedYear==0 ) {
     gDateTime now( e_Now );
     usedYear = now.year;
     monthNow = (t_int8)now.month;
 }
 if ( usedYear>0 && (monthAuto = thisMonthAbbrevByStr( strField ))>=1 ) {
     // Try to guess, e.g. "Apr 30", as 2010-04-30
     dayAuto = atoi( resultL.Str( 2 ) );
     keepIdx = keepCount = 3;
 }

 if ( dayAuto<1 ) {
     for ( ; i<=n; i++) {
	 gString field( resultL.Str( i ) );

	 count = field.CountChars( '-' );
	 if ( field[ 1 ]==':' || field[ field.Length() ]==':' )
	     timeCount = 0;
	 else
	     timeCount = field.CountChars( ':' );

	 if ( count==2 && keepCount==0 ) {
	     sDate = field;
	     keepCount = count;
	     keepIdx = i;
	 }
	 if ( timeCount>=1 && keepTimeCount==0 && keepCount>0 ) {
	     sTime = field;
	     keepTimeCount = count;
	     keepIdx = i;
	 }
     }
 }
 else {
     sTime.Set( resultL.Str( keepIdx ) );
     sDate.Add( (int)(usedYear) - (monthAuto > monthNow) );
     sDate.Add( '-' );
     sDate.Add( (int)monthAuto );
     sDate.Add( '-' );
     sDate.Add( (int)dayAuto );
     DBGPRINT("DBG: thisConvertDtTmStrToDateTime using English abbrev, year: %u {%s,%s}\n",
	      usedYear,
	      sDate.Str(),
	      sTime.Str());
 }

 if ( keepCount ) {
     for (i=keepIdx+1; i<=n; i++) {
	 sRemaining.Add( resultL.Str( i ) );
	 if ( i<n ) sRemaining.Add( ' ' );
     }
 }

 result = thisConvertToDateTime( sDate, sTime, dateFormat, *this );
 DBGPRINT("DBG: result=%d, keepCount=%u, n=%u, sDate='%s', sTime='%s'\n",
	  result,
	  keepCount,
	  n,
	  sDate.Str(),
	  sTime.Str());
 return result;
}


int gDateString::thisConvertToStamp (gDateTime& aDtTm, t_stamp& aStamp)
{
 // Converts aDtTm into a timestamp (aStamp)
 time_t tStamp;
 struct tm aTM;

 DBGPRINT_MIN("DBG: gDateString::thisConvertToStamp({...date...}{%02u:%02u:%02u})\n",
	      aDtTm.hour,
	      aDtTm.minu,
	      aDtTm.sec);
 aStamp = 0;
 if ( thisConvertTo_libc_tm( aDtTm, &aTM )!=0 ) return 4;  // Invalid date
 tStamp = mktime( &aTM );
 DBGPRINT_MIN("DBG: {gDateString:[%d,%d,%d],dst?%d|%ld}\n",aTM.tm_year,aTM.tm_mon,aTM.tm_mday,aTM.tm_isdst,(long)tStamp);
 DBGPRINT_MIN("DBG: {tzname(%d)=%s:%s|%s}\n",daylight,tzname[daylight!=0],tzname[0],tzname[1]);
 if ( tStamp==(time_t)-1 ) {
     // Cannot be represented as calendar time (a.k.a epoch time)
     return 8;
 }
//// fprintf(stderr,"{%ld|daylight=%d|%ld}\n",(long)tStamp,daylight,(long)(tStamp + 60*60));
//// tStamp += daylight * 60*60;  // add-on due to inpaired (?) libc
 aStamp = (t_stamp)tStamp;
 // All ok, so returns 0
 return 0;
}


int gDateString::thisConvertFromStampUTCorLocal (t_stamp aStamp, eLocalOrUTC localOrUTC)
{
 struct tm* pTM( nil );
 time_t tStamp( (time_t)aStamp );

 switch ( localOrUTC ) {
 case e_ReferenceUTC:
     pTM = gmtime( &tStamp );
     break;
 case e_ReferenceLocal:
     pTM = localtime( &tStamp );
     break;
 case e_ReferenceAuto:
     ASSERTION_FALSE("e_ReferenceAuto not ready yet");
     return -1;
 case e_NoReference:
     ASSERTION_FALSE("e_NoReference");
     return -1;
 case e_InvalidReference:
     return -1;
 }

 if ( pTM==nil ) return -1;
 year = (t_int16)pTM->tm_year+1900;
 month = (t_uint8)pTM->tm_mon+1;
 day = (t_uint8)pTM->tm_mday;
 hour = (t_uint8)pTM->tm_hour;
 minu = (t_uint8)pTM->tm_min;
 sec =(t_uint8)pTM->tm_sec;
 ;
 wday = (t_uint8)pTM->tm_wday;
 yday = (t_uint16)pTM->tm_yday;
 isdst = (t_int8)pTM->tm_isdst;
 return 0;
}


t_uint16 gDateString::thisMonthHash (const char* monthAbbrev)
{
 t_uint16 month;
 if ( monthAbbrev && monthAbbrev[ 0 ] && monthAbbrev[ 1 ] && monthAbbrev[ 2 ] ) {
     month = (t_uint16)(monthAbbrev[ 0 ]-'A') % 32;
     return
	 (month*25*25 + (monthAbbrev[ 1 ]-'a') + (monthAbbrev[ 2 ]-'a')) % monthHashSize;
 }
 return monthHashSize;
}


int gDateString::thisHashMonthNamesEn ()
{
 int iter( 1 );
 t_uint16 hash;

 if ( monthHashed[ 255 ] ) return 0;
 monthHashed[ 255 ] = 12;

 for ( ; iter<=12; iter++) {
     hash = thisMonthHash( defaultMonths[ iter ].abbrev );
     ASSERTION(monthHashed[ hash ]==0,"monthHashed");
     monthHashed[ hash ] = iter;
 }
 return 12;
}

////////////////////////////////////////////////////////////
gList* ils_inversed_log_names_from_list (gList& in)
{
 gList* newObj( new gList );
 gList stack;
 gElem* ptrElem( in.StartPtr() );
 gElem* back;
 gString* ptrThis;
 gString sLast;
 bool similar;
 unsigned len;

 // Order is inversed for similar log-files.
 // e.g. 'entropa dump /var/log/kernel*' shows first kernel.1.gz, and only after kernel.

 ASSERTION(newObj,"Mem!");

 for ( ; ptrElem; ptrElem=ptrElem->next) {
     ptrThis = (gString*)ptrElem->me;
     len = ptrThis->Length();
     if ( len<=0 ) continue;
     if ( len > 2 ) {
	 similar = strncmp( sLast.Str(), ptrThis->Str(), len-2 )==0;
	 if ( similar==false) {
	     if ( len>5 && strcmp( ptrThis->Str( len-3 ), ".gz" )==0 ) {
		 similar = strncmp( sLast.Str(), ptrThis->Str(), len-5 )==0;
	     }
	 }
	 DBGPRINT("DBG: len=%d, similar? %c {%s}, %s\n",
		  len,
		  ISyORn( similar ),
		  sLast.Str(),
		  ptrThis->Str());
	 if ( sLast.IsEmpty() || similar ) {
	 }
	 else {
	     for (back=stack.EndPtr(); back; back=back->prev) {
		 newObj->Add( back->Str() );
	     }
	     stack.Delete();
	 }
     }
     sLast = *ptrThis;
     stack.Add( sLast );
 }
 for (back=stack.EndPtr(); back; back=back->prev) {
     newObj->Add( back->Str() );
 }
 return newObj;
}


gList* ils_inversed_log_names_from_str (const char* str, int mask)
{
 gParam listed( (char*)str, " " );
 return ils_inversed_log_names_from_list( listed );
}

////////////////////////////////////////////////////////////

