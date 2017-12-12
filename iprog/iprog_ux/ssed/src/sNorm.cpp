// sNorm.cpp

// decking and mimicking gNorm if 'HAS_NORM' is defined.


#include "icalendar.h"
#include "ibases.h"

#include "sNorm.h"
#include "sDates.h"

#include <sys/time.h>
#include <sys/timeb.h>


struct sCmdCapa {
    const char* name;
    const char* desc;
    const int minArgs;
};

////////////////////////////////////////////////////////////
// Aux functions
////////////////////////////////////////////////////////////
int nrm_remove_backtill (t_uchar minChr, t_uchar* aStr)
{
 int len( imb_strlen( aStr ) );

 for ( ; len>0; ) {
     if ( aStr[ len-1 ]>=minChr ) break;
     len--;
     aStr[ len ] = 0;
 }
 return len;
}


int nrm_remove_newline (t_uchar* aStr)
{
 return nrm_remove_backtill( ' ', aStr );
}


int nrm_convert_time_from_str (const char* aStr, gDateTime& aUTC)
{
 int hh, mm, ss;
 int splits;
 gList splitted;
 gString sTime( (char*)aStr );
 char* strMM;
 char* strSS;

 sTime.SplitAnyChar( ':', splitted );
 splits = (int)splitted.N();
 hh = atoi( splitted.Str( 1 ) );
 mm = atoi( strMM = splitted.Str( 2 ) );
 ss = atoi( strSS = splitted.Str( 3 ) );
 DBGPRINT("DBG: splits: %d, hh=%d, strMM {%s}, strSS {%s}\n", splits, hh, strMM, strSS);
 if ( hh<0 || hh>24 ) return 1;
 if ( mm<0 || mm>59 ) return 1;
 if ( ss<0 || ss>61 ) return 1;  // leap-second year at 23:59:60, or even ...:61
 aUTC.hour = hh;
 aUTC.minu = mm;
 aUTC.sec = ss;
 if ( splits < 2 ) return -1;
 ASSERTION(strMM,"strMM");
 ASSERTION(strSS,"strSS");
 if ( strlen( strMM )!=0 && strlen( strMM )!=2 ) return 2;
 if ( strlen( strSS )!=0 && strlen( strSS )!=2 ) return 2;
 if ( strMM[ 0 ] < '0' || strMM[ 0 ] > '9' ) return 2;
 if ( strSS[ 0 ] && (strSS[ 0 ] < '0' || strSS[ 0 ] > '9') ) return 2;
 return 0;
}


int nrm_convert_dateofyear_from_str (const char* aStr, gDateTime& aUTC)
{
 int year, month, day;
 unsigned pos( 0 );
 gString sBuf( (char*)aStr );

 year = atoi( aStr );
 month = atoi( aStr + (pos = sBuf.Find( "-" )) );
 sBuf.Delete( 1, pos );
 day = atoi( sBuf.Str() + sBuf.Find( "-" ) );
 if ( year < 1900 ) return 1;

 aUTC.year = year;
 aUTC.month = month;
 aUTC.day = day;
 return 0;
}


int nrm_convert_dated_str (const char* strIn, gDateTime& aUTC, char* outBuf)
{
 gString sDate( (char*)strIn );
 gList splitted;
 int error( 0 );
 int timeError( 0 );
 unsigned pos;

 if ( strIn ) {
     sDate.SplitAnyChar( "\t ~^", splitted );

     if ( splitted.N() > 1 ) {
	 pos = splitted.FindFirst( ":", 1, e_FindFromPosition );
	 timeError = nrm_convert_time_from_str( splitted.Str( pos ), aUTC );
	 DBGPRINT("DBG: TIME: {%s}, timeError: %d\n",splitted.Str( pos ), timeError);
	 error += (timeError!=0);
	 pos = (pos<=1) ? 2 : 1;
	 error += nrm_convert_dateofyear_from_str( splitted.Str( pos ), aUTC )!=0;
	 DBGPRINT("DBG: splitted {%s}, pos: %u, timeError: %d, %02d:%02d:%02d\n", splitted.Str( pos ), pos, timeError, aUTC.hour, aUTC.minu, aUTC.sec);
     }
     else {
	 nrm_convert_dateofyear_from_str( sDate.Str(), aUTC );
     }
 }

 if ( error ) {
     outBuf[ 0 ] = 0;
     error = 2;
 }
 else {
     sprintf(outBuf, "%04u-%02u-%02u %02u:%02u:%02u",
	      aUTC.year, aUTC.month, aUTC.day,
	      aUTC.hour, aUTC.minu, aUTC.sec);

     error = aUTC.IsOk()==false;
 }
 DBGPRINT("DBG: strIn {%s}, error: %d, time is %02d:%02d:%02d\nDBG: \t%s\n", strIn, error, aUTC.hour, aUTC.minu, aUTC.sec, outBuf);
 return error;
}


int nrm_convert_date_from_str (const char* strIn, int mask, char* outBuf)
{
 gDateTime aUTC;
 return nrm_convert_dated_str( strIn, aUTC, outBuf );
}


int x_norm_ftime (gDateTime* ptrDate, sTimeRef& timeRef)
{
 //	       int ftime(struct timeb *tp);
 //
 //	DESCRIPTION
 //	       Return  current  date and time in tp, which is declared as
 //	       following:
 //
 //	                 struct timeb {
 //	                      time_t   time;
 //	                      unsigned short millitm;
 //	                      short    timezone;
 //	                      short    dstflag;
 //	                 };

 int error( 0 );
 struct timeb bTime;

 memset( &bTime, 0, sizeof( struct timeb ) );

 if ( ptrDate==nil ) {
#ifdef DEBUG_FTIME
     error = ftime( &bTime );
#else
#ifdef iDOS_SPEC
     bTime.time = time( NULL );
#else
     struct timeval tv;
     error = gettimeofday( &tv, NULL );
     bTime.time = tv.tv_sec;
     bTime.millitm = tv.tv_usec/1000;  // from microseconds to milliseconds
#endif //~iDOS_SPEC
#endif //~DEBUG_FTIME
 }

 timeRef.time = bTime.time;
 timeRef.millitm = bTime.millitm;
 timeRef.timezone = bTime.timezone;
 timeRef.dstflag = bTime.dstflag;

 return error;
}


int nrm_dump_text_hash (FILE* fIn,
			FILE* fOut,
			const char* strFileAux,
			int verboseLevel,
			t_int16 mask)
{
 const int bufSize( 4096 );
 char buf[ bufSize ];
 bool noEOL( false );  // No '\n' at end of text file
 unsigned len;
 int countNoNewLines( 0 );
 int hash( -1 );
 gString s;

 if ( fIn==nil ) return -1;
 if ( fOut==nil ) return -2;

 for ( ; fgets( buf, bufSize, fIn ); ) {
     s.Add( buf );
     len = s.Length();

     if ( len ) {
	 noEOL = s[ len ]!='\n';
	 if ( noEOL ) {
	     countNoNewLines++;
	 }
	 else {
	     s.Delete( len, len );
	     len--;
	     if ( len ) {
		 if ( s[ len ]=='\r' ) {
		     s.Delete( len, len );
		     len--;
		 }
	     }
	     if ( mask==3 ) {
		 for (int iter=len; iter>0; iter--) {
		     if ( s[ iter ]=='\\' || s[ iter ]=='/' ) {
			  gString sCopy( s.Str( iter ) );
			  s = sCopy;
			  break;
		     }
		 }
	     }
	     hash = s.Hash();

	     if ( verboseLevel>1 ) {
		 fprintf(fOut,"%08x %s\n", hash, s.Str());
	     }
	     else {
		 fprintf(fOut,"%d\t%s\n", hash, s.Str());
	     }
	     s.Reset();
	 }
     }
     else {
	 noEOL = false;
     }
 } //end FOR

 if ( noEOL ) {
     if ( verboseLevel>1 ) {
	 fprintf(fOut,"%08x %s",s.Hash(),s.Str());
     }
     else {
	 fprintf(fOut,"%d\t%s",s.Hash(),s.Str());
     }
 }

 return countNoNewLines!=0;
}

////////////////////////////////////////////////////////////
// norm commands
////////////////////////////////////////////////////////////
int norm_date (FILE* fOut, FILE* fRepErr,
	       gList& argsIn,
	       int option,
	       int verboseLevel)
{
 int error( 0 ), countErrors( 0 );
 unsigned i( 1 ), n( argsIn.N() );
 time_t reqTime( (time_t)0 );
 const int mask( 0 );
 const char* strDate( nil );
 char smallBuf[ 128 ];

 gList args;

 smallBuf[ 127 ] = 0;
 strDate = argsIn.Str( 1 );
 DBGPRINT("DBG: norm_date #%u, option: %d\n", args.N(), option);

 // If only two args provided, assume it's a year and time:
 if ( n==2 && strDate[ 0 ]>='0' && (reqTime = atoi( strDate )) <= 9999 && strchr( argsIn.Str( 2 ), ':' )!=nil ) {
     gString sCopy( argsIn.Str( 1 ) );
     sCopy.Add( ' ' );
     sCopy.Add( argsIn.Str( 2 ) );
     args.Add( sCopy );
     n = 1;
 }
 else {
     args.CopyList( argsIn );
 }

 if ( n ) {
     for ( ; i<=n; i++) {
	 smallBuf[ 0 ] = 0;
	 strDate = args.Str( i );

	 if ( option ) {
	     gDateTime aUTC;
	     error = nrm_convert_dated_str( strDate, aUTC, smallBuf );
	     reqTime = aUTC.GetTimeStamp();
	     snprintf( smallBuf, 127, ctime( &reqTime ) );
	     nrm_remove_newline( (t_uchar*)smallBuf );
	     DBGPRINT("DBG: smallBuf {%s}, stamp %lu\n", smallBuf, (unsigned long)reqTime);
	 }
	 else {
	     reqTime = 0;
	     error = nrm_convert_date_from_str( strDate, mask, smallBuf );
	 }
	 if ( error ) {
	     countErrors++;
	 }
	 else {
	     if ( verboseLevel >= 3 && reqTime ) {
		 fprintf(fOut,"%s\t%lu\n", smallBuf, (unsigned long)reqTime);
	     }
	     else {
		 fprintf(fOut,"%s\n", smallBuf);
	     }
	 }
     }
 }
 else {
     // No args: show the current date

     if ( option ) {
	 reqTime = time( NULL );

	 if ( verboseLevel ) {
	     struct tm* ptrTM;
	     ptrTM = localtime( &reqTime );

#ifdef iDOS_SPEC
	     strftime( smallBuf, 127, "%Y-%m-%d %H:%M:%S %Z", ptrTM );   // BUG at Dev-C++, instead of "WEST"/"WET", it shows "GMT standard time"
#else
	     strftime( smallBuf, 127, "%Y-%m-%d %H:%M:%S %z", ptrTM );
#endif

#ifdef DEBUG
	     char strZone[ 64 ];
	     sTimeRef timeRef;
	     x_norm_ftime( nil, timeRef );
	     snprintf( strZone, 63, "_ Millisecs: %03u %lu @%d DST=%d",
		       timeRef.millitm,
		       timeRef.time,
		       timeRef.timezone,
		       timeRef.dstflag);
	     strncat( smallBuf, strZone, 63 );
#endif //DEBUG
	 }
	 else {
	     snprintf( smallBuf, 127, ctime( &reqTime ) );
	     nrm_remove_newline( (t_uchar*)smallBuf );
	 }
     }
     else {
	 countErrors = nrm_convert_date_from_str( nil, mask, smallBuf )!=0;
     }
     fprintf(fOut,"%s\n",smallBuf);
 }
 DBGPRINT("DBG: verboseLevel: %d, countErrors: %d\n", verboseLevel, countErrors);
 return countErrors;
}


int norm_text_hash (FILE* fOut,
		    FILE* fRepErr,
		    gList& args,
		    int option,
		    int verboseLevel)
{
 int error;
 int countErrors( 0 );
 FILE* fIn( stdin );
 unsigned iter( 1 ), n( args.N() ), nIter( n ? n : 1 );
 const t_int16 mask( (t_int16)option );
 char* strFile;

 DBGPRINT("DBG: norm_text_hash fOut=0x%p fRepErr=0x%p, option: %d\n", fOut, fRepErr, option);

 for ( ; iter<=nIter; iter++) {
     strFile = args.Str( iter );
     if ( n ) {
	 fIn = fopen( strFile, "rt" );
	 if ( fIn==nil ) {
	     if ( fRepErr ) {
		 fprintf(fRepErr, "%s: cannot read file\n", strFile);
	     }
	 }
     }

     error = nrm_dump_text_hash( fIn, fOut, strFile, verboseLevel, mask );

     if ( fIn && fIn!=stdin ) fclose( fIn );
     countErrors += (error!=0);
 }

 return countErrors!=0;
}


int norm_stat (FILE* fOut,
	       FILE* fRepErr,
	       gList& args,
	       int option,
	       int verboseLevel)
{
  //// const t_int16 mask( (t_int16)option );
 const bool showUnits( verboseLevel >= 3 );
 const char* strLine( verboseLevel ? "\n" : " " );
 const char* strModified;
 const bool localTime( option==0 );
 int error;
 int countErrors( 0 );
 char* strFile;
 gElem* pElem( args.StartPtr() );

 DBGPRINT("DBG: norm_stat fOut=0x%p fRepErr=0x%p, option: %d, verboseLevel: %d\n", fOut, fRepErr, option, verboseLevel);
 ASSERTION(fOut,"fOut");

 for ( ; pElem; pElem=pElem->next) {
     strFile = pElem->Str();
     if ( strFile[ 0 ] ) {
	 gFileStat aStat( strFile );

	 error = aStat.HasStat()==false;

	 if ( error ) {
	     if ( fRepErr ) {
		 perror( strFile );
	     }
	     countErrors++;
	 }
	 else {
	     strModified = new_date_str( aStat.status.mTime, nil, localTime );

	     fprintf(fOut, "%s:", strFile);
	     fprintf(fOut, "%s%llu%s",
		     strLine, aStat.status.U64Size(),
		     showUnits ? " byte(s)" : "");
	     if ( verboseLevel ) {
		 fprintf(fOut, "%s%s%s%s%s%s%s%s%s",
			 strLine, new_date_str( aStat.status.aTime, nil, localTime ), showUnits ? " (accessed)" : "",
			 strLine, strModified, showUnits ? " (modified)" : "",
			 strLine, new_date_str( aStat.status.cTime, nil, localTime ), showUnits ? " (changed)" : "");
	     }
	     else {
		 fprintf(fOut, "%s%s", strLine, strModified);
	     }
	     fprintf(fOut, "\n");
	 }
     }
 }

 return countErrors!=0;
}

////////////////////////////////////////////////////////////
int norm_capa (const char* strOptCapa, gList& capaList)
{

 capaList.Delete();

#ifdef HAS_NORM
 int cmdNr( 0 );
 int iter( 1 );
 gString sMsg;
 const char* strName( nil );
 const sCmdCapa cmds[]={
     { nil, nil, -1 },
     { "du", "Disk Usage", 0 },			// 1
     { "dd", "Create / extract file", 2 },	// 2
     { "bdate", "Basic date", 0 },		// 3
     { "texth", "Text hash", 0 },		// 4
     { "stat", "File status", 0 },		// 5
     { nil, nil, -1 }
 };

 DBGPRINT("norm_capa(%s,capaList), cmdNr: %d\n",
	  strOptCapa,
	  cmdNr);

 for ( ; (strName = cmds[ iter ].name)!=nil; iter++) {
     if ( strOptCapa ) {
	 if ( strcmp( strName, strOptCapa )==0 ) {
	     cmdNr = iter;
	     capaList.Add( (char*)strName );
	     capaList.Add( (char*)cmds[ iter ].desc );
	     capaList.Add( sMsg );
	     capaList.Add( cmds[ iter ].minArgs );
	     return cmdNr;
	 }
     }
     else {
	 gString sCmd( (char*)strName );
	 sCmd.Add( "\t" );
	 sCmd.Add( (char*)cmds[ iter ].desc );
	 if ( cmds[ iter ].minArgs > 0 ) {
	     sCmd.Add( " ARG(s)" );
	 }
	 capaList.Add( sCmd );
     }
 }

 return cmdNr;

#else

 return -1;

#endif //~HAS_NORM
}


int norm_command (int cmdNr,
		  gList& capaList,
		  FILE* fIn,
		  FILE* fOut,
		  FILE* fRepErr,
		  gList& args,
		  int option,
		  int verboseLevel)
{
 int error( 0 );

 switch ( cmdNr ) {
 case 1:  // du
     // No break here!

 case 2:  // dd
     fprintf(stderr,"STILL unimplemented.\n");
     break;

 case 3:  // bdate (a basic date)
     error = norm_date( fOut, fRepErr, args, option, verboseLevel );
     if ( fRepErr ) {
	 if ( error ) {
	     fprintf(fRepErr, "%d error(s) in date(s)\n",error);
	 }
     }
     break;

 case 4:  // texth
     error = norm_text_hash( fOut, fRepErr, args, option, verboseLevel );
     break;

 case 5:  // stat
     error = norm_stat( fOut, fRepErr, args, option, verboseLevel );
     break;

 default:
     ASSERTION_FALSE("what?");
 }

 return error;
}

////////////////////////////////////////////////////////////
char* x_newstr (int length, const char* strSourceFile, int line)
{
 char* newStr;
 int len( length<=0 ? 0 : length );
 newStr = (char*)calloc( len+1, sizeof(char) );
 MEMORY_FAULT(newStr,len,strSourceFile,line);
 return newStr;
}


char* x_newstr_from_list (gList& list, const char* strSepFmt, int mask)
{
 char* newStr;
 char* strFmt;
 char* strTemp;
 gElem* ptrElem( list.StartPtr() );
 unsigned totalLen( 1 ), sepLen;

 if ( strSepFmt==nil ) {
     strSepFmt = "%s\n";
 }
 strFmt = NEWSTR( strlen( strSepFmt )+6 );
 sprintf(strFmt, "%%s%s", strSepFmt );

 sepLen = (unsigned)strlen( strSepFmt );

 for ( ; ptrElem; ptrElem=ptrElem->next) {
     totalLen += sepLen+2;
     totalLen += ptrElem->me->Length();
 }

 newStr = NEWSTR( totalLen );

 for (ptrElem=list.StartPtr(); ptrElem; ptrElem=ptrElem->next) {
     const char* strThis( ptrElem->Str() );
     strTemp = strdup( newStr );
     sprintf(newStr, strFmt, strTemp, strThis);
     free( strTemp );
 }

 free( strFmt );
 return newStr;
}

////////////////////////////////////////////////////////////

