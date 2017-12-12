// sDates.cpp

#include <time.h>
#include <math.h>

#include "lib_iobjs.h"

#include "log.h"
#include "sDates.h"

////////////////////////////////////////////////////////////
struct tm* a_current_year (int actualMonth, int& currentYear)
{
 static time_t currentStamp;
 static struct tm* ptrTime;
 static int currentMonth;

 currentStamp = time( NULL );
 ptrTime = gmtime( &currentStamp );

 if ( currentYear<=0 || actualMonth<1 ) {
     currentYear = ptrTime->tm_year+1900;
     currentMonth = ptrTime->tm_mon+1;

     if ( actualMonth > currentMonth ) {
         currentYear--;
         ptrTime->tm_year = currentYear-1900;
     }
 }
 return ptrTime;
}


int match_month (gDateTime& dttm, const char* strMonth)
{
 for (int month=1; month<=12; month++) {
     if ( strncmp( strMonth, dttm.monthNames[ month ].abbrev, 3 )==0 )
	 return month;
 }
 return 0;
}


gList* ctime_date (t_stamp stamp)
{
 time_t aStamp( (time_t)stamp );
 gList* result( new gList );

 if ( stamp < 86400 ) {
     // First day (of 1970) is considered invalid
     result->Add( "-" );
 }
 else {
     const char* strTimezone( tzname[ 0 ] );

     gParam list( ctime( &aStamp ), "\n" );
     gParam nail( list.Str( 1 ), " " );
     gString* field( nil );
     gString* last;

     for (gElem* ptr=nail.StartPtr(); ptr; ) {
	 field = (gString*)ptr->me;
	 ptr = ptr->next;
	 if ( ptr ) {
	     result->Add( *field );
	 }
     }
     // Instead of just: result->CopyList( nail )
     // (because we want to preserve the last element to display the year)

     result->Add( (char*)strTimezone );
     last = (gString*)result->EndPtr()->me;
     last->Trim();

     if ( field ) {
	 result->Add( *field );  // year!
     }
 }
 return result;
}


gList* ctime_date_micro (struct timeval& tv, t_int16 format)
{
 time_t aStamp( (time_t)tv.tv_sec );
 t_uint32 uSec( (t_uint32)tv.tv_usec );
 gList* result( new gList );
 char uSecStr[ 32 ];

 uSecStr[ 0 ] = 0;

 if ( aStamp < 86400 ) {
     // First day (of 1970) is considered invalid
     result->Add( "-" );
 }
 else {
     const char* strTimezone( tzname[ 0 ] );

     gParam list( ctime( &aStamp ), "\n" );
     gParam nail( list.Str( 1 ), " " );
     gString* field( nil );
     gString* last;

     for (gElem* ptr=nail.StartPtr(); ptr; ) {
	 field = (gString*)ptr->me;
	 ptr = ptr->next;
	 if ( ptr ) {
	     bool timeField( ptr->next==nil );
	     if ( timeField && format>=3 ) {
		 if ( format==6 ) {
		     sprintf(uSecStr, ".%06lu", (unsigned long)uSec);
		 }
		 else {
		     sprintf(uSecStr, ".%03lu", (unsigned long)uSec / 1000UL);
		 }
	     }
	     field->Add( uSecStr );
	     result->Add( *field );
	 }
     }

     result->Add( (char*)strTimezone );
     last = (gString*)result->EndPtr()->me;
     last->Trim();

     if ( field ) {
	 result->Add( *field );  // year!
     }
 }
 return result;
}


gString* join_strings (gList& list, const char* strSep)
{
 gString* result( new gString );

 for (gElem* ptr=list.StartPtr(); ptr; ptr=ptr->next) {
     if ( result->Length() ) {
	 result->Add( (char*)strSep );
     }
     result->Add( ptr->Str() );
 }
 return result;
}


int show_datex (FILE* fIn, FILE* fOut, int dateType)
{
 char buf[ 4096 ];
 const int maxBuf( (int)sizeof(buf)-1 );
 int error( 0 ), len, pos;
 int year( -1 );
 long line( 1 );
 gString sLeft, sRight;
 gDateTime dttm;

 FILE* fRepErr( stderr );

 ASSERTION(fIn,"fIn");
 ASSERTION(fOut,"fOut");

 for ( ; fgets( buf, maxBuf, fIn ); ) {
     len = strlen( buf );
     if ( len>=maxBuf ) {
	 fprintf(fRepErr,"Line too long: %ld\n",line);
     }
     else {
	 switch ( dateType ) {
	 case 0:
	 case -1:
	     if ( len>0 ) {
		 gString sLine( buf );
		 pos = (int)sLine.FindAnyChr( " \t" );

		 if ( pos>1 ) {
		     sLeft.CopyFromTo( sLine, 1, pos-1 );
		     sRight.CopyFromTo( sLine, pos );
		 }
		 else {
		     sLeft.Delete();
		     sRight.Set( buf );
		 }

		 error = sdate_x_conv( sLeft.Str(), dateType, sLine );
		 if ( error ) {
		     fprintf(fOut,"%s",buf);
		 }
		 else {
		     fprintf(fOut,"%s%s",
			     sLine.Str(),
			     sRight.Str());
		 }
	     }
	     break;

	 case 1:
	     year = -1;  // Recalculate always, based on current time
	     sdate_shown_conv( fOut, buf, len, dttm, year );
	     break;

	 default:
	     if ( dateType>1900 ) {
		 year = dateType;
		 sdate_shown_conv( fOut, buf, len, dttm, year );
	     }
	     break;
	 }// end Switch

	 line++;
     }// end IF (else...)
 }// end FOR (read lines)
 return 0;
}


int sdate_x_conv (const char* aStr, int dateType, gString& sResult)
{
 gParam param( (char*)aStr, "-" );
 int result( 0 );
 int a3( atoi( param.Str( 3 ) ) );
 int nrItems( (int)param.N() );

 sResult.SetEmpty();
 if ( aStr==nil ) return 0;

 switch ( dateType ) {
 case -1:
 case 0:
 case 1011:
     // Convert DD-MM-YYYY to YYYY-MM-DD
     if ( nrItems==3 ) {
	 // input="31-12-2010", output="2010-12-31"
	 if ( a3>=1000 && a3<=9999 ) {
	     sResult.Add( a3 );
	     sResult.Add( '-' );
	     sResult.Add( param.Str( 2 ) );
	     sResult.Add( '-' );
	     sResult.Add( param.Str( 1 ) );
	 }
	 else {
	     result = -1;
	 }
     }
     else {
	 result = 2;
	 sResult.Set( (char*)aStr );
     }
     break;

 case 1:
 default:
     ASSERTION_FALSE("sdate_x_conv dateType!");
 }

 return result;
}


int sdate_shown_conv (FILE* fOut, const char* aStr, int length, gDateTime& dttm, int& year)
{
 char strMonth[ 8 ];
 char strDay[ 8 ];
 int month( 0 );
 int day( 0 );
 const char* strRemain( nil );

 memset(strMonth, 0, 8 );
 memset(strDay, 0, 8 );
 strncpy( strMonth, aStr, 4 );
 if ( length>=8 ) {
     strncpy( strDay, aStr+3, 4 );
 }

 if ( strMonth[ 3 ]==' ' ) {
     month = match_month( dttm, strMonth );
 }
 DBGPRINT("DBG: strMonth={%s}, %d, strDay={%s}, %d\n",
	  strMonth, month,
	  strDay, day);
 if ( fOut ) {
     if ( strDay[ 0 ]==' ' && strDay[ 3 ]<=' ' ) {
	 day = atoi( strDay );
	 strRemain = aStr + 7;

	 if ( month > 0 ) {
	     // ptrTime = a_current_year( month, year );
	     if ( year>=0 ) {
		 fprintf(fOut,"%04d-%02d-%02d %s",
			 year, month, day,
			 strRemain);
	     }
	     else {
		 fprintf(fOut,"%s",aStr);
	     }
	 }
     }
     else {
	 fprintf(fOut,"%s",aStr);
     }
 }
 return 0;
}


int int_log10 (int value)
{
 int result( 0 );
 do {
     result++;
     value /= 10;
 } while (value>0);
 return result;
}


int nearest_decimal (int value, int places)
{
 int L( int_log10( value ) );
 int result( value );
 int divide( 1 );

 if ( places<0 ) return value;

 if ( L > places ) {
     for ( ; L>places; L--) {
	 divide *= 10;
     }
#ifdef SKIP_MATH_ROUND
     result /= divide;
#else
     result = (int)((float)(rintf( (float)result / (float)divide )));
#endif
 }
 else {
     for ( ; L<places; L++) {
	 result *= 10;
     }
 }
 return result;
}


const char* time_plus_or_minus (const char* str, gList& param)
{
 int value( 0 );
 char chr( '\0' );
 const char* aStr( str );
 const char* result( aStr );

 if ( str ) {
     for ( ; (chr = *str)!=0; str++) {
	 if ( chr=='-' || chr=='+' ) {
	     value = atoi( str );
	     result = str;
	     break;
	 }
     }
     if ( chr==0 ) {
	 value = atoi( aStr );
     }
     param.Add( (char*)aStr );
 }
 param.iValue = value;
 return result;
}


gList* time_list (FILE* fRepErr, gString& hhmmss, int year, t_int16 ms3or6)
{
 // Time of day list from HH:MM:SS
 //
 // ms3or6 = 3 when showing milliseconds, or 6 for microseconds
 ;
 gParam dot( hhmmss, ".", gParam::e_StopSplitOnFirst );
 gParam timed( dot.Str( 1 ), ":" );
 gList* list( new gList );
 int iter( 0 ), n( 4 ), k( n-1 );
 int value[ n ];
 t_uint32 uValue;
 gString sFirst;

 char buf[ 64 ];

 ASSERTION(list,"list");

 if ( ms3or6<0 ) {
     ms3or6 = 0;
 }
 if ( year >= 0 ) {
     list->Add( year );
     list->Add( 0 );
     list->Add( 0 );
 }

 for (iter=0; iter<n; iter++) {
     value[ iter ] = 0;
 }

 for (n=timed.N(), iter=n; k>0; k--, iter--) {
     int thisValue( MIN_INT16_I );
     int bogus( 0 );
     if ( iter<=0 ) break;

     gString s( timed.Str( iter ) );
     if ( s.Length()!=2 && iter>1 ) {
	 bogus = iter;
	 LOG_ME(LOG_WARN, "Parameter too %s: %s", s.Length()>2 ? "long" : "short", s.Str());
     }
     if ( gStorageControl::Self().ConvertToUInt32( s.Str(), uValue )==0 ) {
	 thisValue = (int)uValue;
     }
     if ( iter==1 ) {
	 gList param;
	 sFirst.Set( (char*)time_plus_or_minus( s.Str(), param ) );
	 thisValue = param.iValue;
	 bogus = (thisValue==0 && sFirst.Match( "0" )==false);
	 if ( bogus ) {
	     LOG_ME(LOG_WARN, "Wrong parameter: %s", s.Str());
	 }
     }
     else {
	 if ( thisValue<=MIN_INT16_I ) {
	     bogus = iter;
	     LOG_ME(LOG_WARN, "Wrong time parameter: %s", s.Str());
	 }
     }

     if ( bogus ) {
	 list->iValue = bogus;
     }
     value[ k ] = thisValue;
 }

 gElem* pElem;
 int usedIndex( 0 );
 k++;
 for (iter=1; iter<=3; iter++) {
     int thisValue( value[ iter ] );

     if ( iter==k ) {
	 strcpy(buf, sFirst.Str());
	 usedIndex = 1;
     }
     else {
	 if ( thisValue<0 ) {
	     thisValue = 0;
	 }
	 sprintf(buf, "%02d", thisValue);
	 usedIndex += (usedIndex>0);
     }
     list->Add( buf );
     pElem = list->EndPtr();
     pElem->iValue = usedIndex;
     pElem->me->iValue = thisValue;
 }

 if ( dot.N() > 1 ) {
     char strDec[ 64 ];
     char* strMS( dot.Str( 2 ) );
     int thisValue( atoi( strMS ) );

     switch( ms3or6 ) {
     case 6:
	 snprintf(buf, sizeof(buf)-2, ".%06d", nearest_decimal( thisValue, 6 ));
	 break;
     case 0:
     case 3:
	 snprintf(buf, sizeof(buf)-2, ".%03d", nearest_decimal( thisValue, 3 ));
	 break;
     default:
	 sprintf(strDec, ".%%%02dd", ms3or6);
	 snprintf(buf, sizeof(buf)-2, strDec, nearest_decimal( thisValue, ms3or6 ));
	 break;
     }

     list->Add( buf );
     pElem = list->EndPtr();
     pElem->me->iValue = thisValue;
 }

 return list;
}


int time_list_fix (FILE* fRepErr, gList& timed)
{
 int error( 0 );
 int anyNonZero( 0 );
 int value;
 int add( 0 );
 //bool hasTOD( timed.N()>=5 )
 gElem* pMinute( nil );
 gElem* pSec( timed.StartPtr() );
 gElem* pNext( nil );
 gElem* prev( nil );

 // Either timed.N = 3 (HH:MM:SS), or 4 (HH:MM:SS.mmm / HH:MM:SS.nnnnnn, respectively with milliseconds/ microseconds)
 // or timed.N = 6 (YYYY-MM-DD HH:MM:SS) or 7 (with milliseconds/ microseconds).

 for ( ; pSec; pSec=pNext) {
     pNext = pSec->next;
     if ( pNext==nil ) break;
     if ( pNext->iValue > 0 ) {
	 anyNonZero++;
     }
     if ( anyNonZero>0 && pNext->iValue==0 ) break;
 }

 if ( pSec ) {
     pNext = pSec->prev;
     if ( pNext ) {
	 pMinute = pNext;
     }
 }

 if ( pMinute ) {
     prev = pMinute->prev;
 }

 DBGPRINT("DBG: PRE_ pMinute %s, pSec %s\n", pMinute ? pMinute->Str() : "@", pSec ? pSec->Str() : "@");

 if ( pSec && (value = pSec->me->iValue)>60 ) {
     add = value / 60;
     value %= 60;
     pSec->me->iValue = value;
     if ( add ) {
	 error |= 16;
	 if ( pMinute ) {
	     pMinute->me->iValue += add;
	 }
	 else {
	     error |= 64;
	 }
     }
 }

 if ( pMinute && prev && (value = pMinute->me->iValue)>60 ) {
     add = value / 60;
     value %= 60;
     pMinute->me->iValue = value;
     if ( add ) {
	 error |= 32;
	 prev->me->iValue += add;
     }
 }

 return error;
}


char* new_date_str (const t_stamp stamp, const char* strFormat, bool localTime)
{
 const time_t aTime( stamp );
 const char* aFormat( strFormat ? strFormat : "%04u-%02u-%02u %02u:%02u:%02u" );
 gString* pNew( new gString( 42, '\0' ) );
 struct tm* pTM;

 pTM = localTime ? localtime( &aTime ) : gmtime( &aTime );

 snprintf(pNew->Str(), 42-1, aFormat,
	  pTM->tm_year+1970, (pTM->tm_mon+1), (pTM->tm_mday),
	  (pTM->tm_hour)%24, (pTM->tm_min), (pTM->tm_sec));

 gStorageControl::Self().Pool().AppendObject( pNew );
 return pNew->Str();
}

////////////////////////////////////////////////////////////

