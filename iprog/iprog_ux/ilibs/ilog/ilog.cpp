// ilog.cpp

#include <string.h>
#include <stdarg.h>  // va_start...

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "ilog.h"

#include "imath.h"
#include "ifilestat.h"

////////////////////////////////////////////////////////////
const char* gCGenLog::base62Chars = "\
ABCDEFGHIJKLMNOPQRSTUVWXYZ\
abcdefghijklmnopqrstuvwxyz\
0123456789";

t_msg_kind gCGenLog::msgKinds[ 10 ]={
    "",
    "ERROR",  // 0
    "WARN",   // 1
    "NOTICE", // 2
    "INFO",   // 3
    "",  // Array idx# 5
    "",  // #6
    "",  // #7
    "",  // #8
    ""   // #9
};


////////////////////////////////////////////////////////////
// Aux functions
////////////////////////////////////////////////////////////
int log_month_from3letter (char monthC1, char monthC2, char monthC3, int auxChr)
{
 switch ( monthC3 ) {
 case 'n':
     switch ( monthC2 ) {
     case 'a':
	 return 1;
     default:
	 return 6;
     }

 case 'b':
     return 2;

 case 'y':
     return 5;

 case 'l':
     return 7;

 case 'g':
     return 8;

 case 'p':
     return 9;

 case 't':
     return 10;

 case 'v':
     return 11;

 case 'c':
     return 12;

 default:
     switch ( monthC2 ) {
     case 'a':
	 return 3;

     case 'p':
	 return 4;

     default:
	 break;
     }
 }
 return 0;
}


struct tm* log_a_current_year (int actualMonth, int& currentYear)
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


////////////////////////////////////////////////////////////
gCGenLog::gCGenLog ()
    : doShowKindOnLog( true ),
      rotateWithCompression( false ),
      identSeed( -1 ),
      rotatedFiles( 0 ),
      logMaxSize( 0 ),
      sRotateCompressCmd( (char*)ILG_ROTATED_COMPRESS_CMD ),
      fLog( nil ),
      fAltOutput( stdout )
{
 memset( scheme8, 0x0, sizeof(scheme8) );
}


gCGenLog::~gCGenLog ()
{
 Close();
}


FILE* gCGenLog::ValidStream ()
{
 ASSERTION(fLog,"fLog");
 return fLog;
}


char* gCGenLog::LogLevelDescription (int level)
{
 if ( level<-1 ) return nil;
 level++;  // msgKinds start at '-1' (NONE, i.e. empty)
 return msgKinds[ level % 10 ];
}


void gCGenLog::Reset ()
{
 sName.SetEmpty();
 Close();
}


void gCGenLog::ResetLog ()
{
 gControl::ResetLog();
 messages.Delete();
 if ( fLog ) fflush( fLog );
}


bool gCGenLog::Close ()
{
 DBGPRINT_MIN("DBG: gCGenLog::Close '%s' (#%u) %s\n",sName.Str(),messages.N(),fLog?"\0":" (already closed)");
 if ( fLog==nil ) return false;
 fclose( fLog );
 fLog = nil;
 return true;
}


bool gCGenLog::Reopen ()
{
 Close();

 if ( rotatedFiles==-1 ) return false;

 if ( rotatedFiles>0 && logMaxSize>0 ) {
     gFileStat aStatus( sName );
     RotateIfNecessary( sName, aStatus.statusL.Size() );
 }
 fLog = fopen( sName.Str(), "at" );
 return fLog!=nil;
}


bool gCGenLog::SetName (gString& sFileName)
{
 sName = sFileName;
 bool isOk = Reopen();
 return isOk;
}


char* gCGenLog::UniqueIdent (int width)
{
 const unsigned baseVal = (26+26+10);  // base-62
 unsigned value( IdentValue(width) );

 sIdent.SetEmpty();
 if ( value==0 ) return sIdent.Str();
 for (short idx=0; width>0; width--, idx++) {
     unsigned v( value%(idx ? baseVal : 26) );
     char chr( base62Chars[ v ] );
     value -= v;
     sIdent.Add( chr );
 }
 return sIdent.Str();
}


unsigned gCGenLog::IdentValue (int width)
{
 if ( width<0 ) return 0;
 unsigned range( width>=3 ? 14776336 : (1<<14) );
 // If width is 4: 62^4 is 14776336!
 gRandom iRand( range );
 DBGPRINT_MIN("DBG: RAND: %d (range=%u, seed=%d)\n",
	      iRand.GetInt(),
	      range,
	      identSeed);
 return (unsigned)iRand.GetInt();
}


bool gCGenLog::SetScheme (t_uint16 major, t_uint16 minor, t_uint16 sub, const char* strPrefix)
{
 char* aPrefix( strPrefix ? strdup( strPrefix ) : strdup( "Aa" ) );

 aPrefix[ 2 ] = 0;
 snprintf( scheme8, 9, "%s%02u%02u%02x",
	   aPrefix,
	   major % 100,
	   minor % 100,
	   sub % 0xFF );
 free( aPrefix );
 return true;
}


bool gCGenLog::SetRotation (int nrRotatedFiles, long maxLogSize, bool doCompression)
{
 bool isOk;

 rotatedFiles = nrRotatedFiles;
 logMaxSize = maxLogSize;
 rotateWithCompression = doCompression;
 isOk = rotatedFiles>=0 && logMaxSize>=1024L;
 ASSERTION(rotateWithCompression==false,"Unimplemented gzip on rotated files");
 return isOk;
}


bool gCGenLog::RemoveSimilarLogs (const char* strFile, gList* optExtensions)
{
 int count( 0 );

 ASSERTION(optExtensions==nil,"optExtensions unimplemented");
 if ( strFile==nil || strFile[ 0 ]<=' ' ) return false;

 unsigned maxLen( strlen( strFile )+16 );
 gString sFile( maxLen, '\0' );
 char* strTry( sFile.Str() );

 strcpy( strTry, strFile );
 count += (remove( strTry )==0);

 snprintf( strTry, maxLen-1, "%s.gz", strFile );
 count += (remove( strTry )==0);

 snprintf( strTry, maxLen-1, "%s.zip", strFile );
 count += (remove( strTry )==0);

 return count>0;
}


int gCGenLog::RotateIfNecessary (gString& sFile, long currentSize)
{
 int result( 0 );
 int iter( rotatedFiles ), currentIdx( iter-1 );
 char path[ sFile.Length()+16 ];
 char newPath[ sFile.Length()+16 ];

 if ( currentIdx<=0 ) return -2;

#if 1  // to debug comment following line
 if ( currentSize < logMaxSize ) return -1;  // Rotation not needed
#endif

 // Rotate a.8 to a.9, a.7 to a.8, ...
 // but first delete last file

 sprintf( path, "%s.%d", sFile.Str(), currentIdx );
 result = (int)RemoveSimilarLogs( path );

 DBGPRINT("DBG: Rotate logs, %sremove: %s (%ld > %ld)\n",
	  result ? "\0" : "TRIED TO ",
	  path,
	  currentSize,
	  logMaxSize);

 for ( ; currentIdx>1; currentIdx=iter) {
     iter = currentIdx-1;
     sprintf( path, "%s.%d", sFile.Str(), iter );
     sprintf( newPath, "%s.%d", sFile.Str(), currentIdx );
     result = (rename( path, newPath )==0);
     DBGPRINT_MIN("DBG: Rename %s to %s: %d\n",
		  path,
		  newPath,
		  result);
 }

 // Finally rename existing... e.g.	a.log into a.log.1
 result = (rename( sName.Str(), path )==0);

 DBGPRINT("DBG: RotateIfNecessary %s (%s): result=%d\n",
	  sName.Str(),
	  sFile.Str(),
	  result);
 return result;
}

////////////////////////////////////////////////////////////
gSLog::gSLog ()
    : programCall( nil ),
      programArgs( nil ),
      xPid( 0 ),
      pPid( 0 ),
      identIncr( -1 )
{
 logStamp.SetDatePrintFormat( (char*)"%04d-%02u-%02u %02u:%02u:%02u" );
 DBGPRINT_MIN("DBG: gSLog::gSLog fmt:%s\n",logStamp.Str());
}


gSLog::~gSLog ()
{
 DBGPRINT_MIN("DBG: gSLog::~gSLog\n");
}


int gSLog::Log (FILE* logFile, int level, const char* formatStr, ...)
{
 // Return 0 if no log applied.
 static time_t currentStamp;
 gString* newObj;

 DBGPRINT("DBG: %p, level: %d, dbgLevel: %d, %s; formatStr {%s}\n",
	  logFile,
	  level,
	  dbgLevel,
	  (level<=LOG_NONE || level>=dbgLevel) ? "skipped" : "log",
	  formatStr);

 if ( level<=LOG_NONE ) return 0;
 if ( level>=dbgLevel ) return 0;

 if ( level>=LOG_LOGMAX ) {
     level = LOG_LOGMAX-1;
 }

 ASSERTION(formatStr!=nil,"formatStr!=nil");
 memset( logBuf, 0x0, sizeof(logBuf) );
 currentStamp = time( NULL );

 // Or: va_start( ap, formatStr ); vfprintf( logFile, formatStr, ap ); va_end( ap );
 va_list ap;
 va_start( ap, formatStr );
 vsnprintf( logBuf, sizeof(logBuf)-1, formatStr, ap );
 va_end( ap );

 nErrors[ level ]++;
 messages.Add( logBuf );
 newObj = (gString*)messages.GetLastObjectPtr();
 newObj->iValue = (int)currentStamp;
 newObj->SetIoMask( level );

 identIncr = (currentStamp / 86400)%24;

 return 1;
}


int gSLog::DumpMessage (FILE* fOut, gString* strMsg, const char* strIdent, t_uint16 mask)
{
 static char* aStr;
 static int logLevel;
 const t_uchar* stampStr( (t_uchar*)"" );
 const bool showTOD( mask > 0 );

 if ( fOut==nil ) {
     DBGPRINT_LOG("DBG: DumpMessage fOut %d%s, %d%s\n",
		  (int)fOut, fOut==stdout ? " (stdout)" : "",
		  (int)fAltOutput, fAltOutput==stdout ? " (stdout)" : "");
     fOut = fAltOutput;
 }
 if ( fOut==nil ) return -1;

 if ( showTOD ) {
     stampStr = log_string_from_tod( strMsg->iValue );
 }
 aStr = strMsg->Str();

 DBGPRINT("DBG: %p%s, TOD {%s}, {%s}\n",
	  fOut,
	  fOut==stdout ? " (stdout)" : "",
	  stampStr, aStr);

 if ( doShowKindOnLog ) {
     fprintf(fOut,"%s%s%s%s[%s] %s\n",
	     stampStr, stampStr[ 0 ] ? " " : "",
	     strIdent,
	     strIdent[ 0 ] ? " " : "\0",
	     LogLevelDescription( logLevel = strMsg->GetIoMask() ),
	     aStr);
 }
 else {
     fprintf(fOut,"%s%s%s\n",
	     stampStr, stampStr[ 0 ] ? " " : "",
	     aStr);
 }
 return logLevel;
}

////////////////////////////////////////////////////////////
// File functions
////////////////////////////////////////////////////////////
int ilf_fileno (FILE* f)
{
 if ( f==nil ) return -1;
 return fileno( f );
}


int ilf_create (const char* strFilename, int mask, int& error)
{
 int fd;
 ASSERTION(mask==0,"mask==0");
 error = -1;
 if ( strFilename ) {
     fd = open( strFilename, O_CREAT | O_TRUNC | O_RDWR, S_IWUSR | S_IRUSR );
     if ( fd!=-1 ) {
	 error = 0;
	 return fd;
     }
     error = errno;
 }
 return -1;
}


int ilf_append (const char* strFilename, int mask, int& error)
{
 int fd;
 error = -1;
 if ( strFilename ) {
     fd = open( strFilename, O_APPEND | O_WRONLY, S_IWUSR | S_IRUSR );
     if ( fd!=-1 ) {
	 error = 0;
	 return fd;
     }
     if ( mask & ILF_APPEND_MASK ) {
	 fd = ilf_openrw( strFilename, mask, error );
	 return fd;
     }
 }
 return -1;
}


int ilf_openrw (const char* strFilename, int mask, int& error)
{
 int fd;
 error = -1;
 if ( strFilename ) {
     fd = open( strFilename, O_RDWR, S_IWUSR | S_IRUSR );
     if ( fd!=-1 ) {
	 error = 0;
	 return fd;
     }
     error = errno;
     if ( mask!=0 && error==2 ) {
	 fd = ilf_create( strFilename, 0, error );
	 if ( fd!=-1 ) return fd;
     }
 }
 return -1;
}


int ilf_close (int& fd)
{
 if ( fd==-1 ) return -1;
 close( fd );
 fd = -1;
 return 0;
}


int ilf_fclose (FILE** ptrFile)
{
 FILE* aFile;

 ASSERTION(ptrFile,"ptrFile");
 aFile = *ptrFile;
 if ( aFile==stdout || aFile==stderr || aFile==stdin )
     return -1;
 if ( aFile ) {
     fclose( aFile );
     *ptrFile = nil;
     return 1;
 }
 return 0;  // did nothing
}

////////////////////////////////////////////////////////////
// Time functions
////////////////////////////////////////////////////////////
char* ilf_ctime (time_t stamp)
{
 return ctime( &stamp );
}


char* ilf_timenow ()
{
 return ilf_ctime( time(NULL) );
}

////////////////////////////////////////////////////////////
// Log functions
////////////////////////////////////////////////////////////
t_uchar* log_string_from_tod (int iStamp)
{
 static t_uchar strDate[ 64 ];
 gDateString dStamp( iStamp );
 return dStamp.ToString( strDate );
}


int log_line_input (int currentYear, int length, const char* strMsg, t_stamp& thisStamp)
{
 // Returns <=0 on error!

 static time_t currentStamp;
 static struct tm* ptrTime;
 static int month;
 int thisFormat( -1 );

 //		Feb 26 11:56:59
 // OR
 //		12345678901234567890
 //		2011-02-26 11:56:59
 // formats allowed:
 //	5. English Unix (Month, day)
 //	16. TOD (year, month, day, and 24h-time)

 if ( strMsg==nil ) return -1;
 if ( length<=15 ) return -2;  // Log too short

 thisStamp = 0;

 if ( strMsg[ 6 ]==' ' ) {
     thisFormat = 5;

     month = log_month_from3letter( strMsg[ 0 ], strMsg[ 1 ], strMsg[ 2 ], 0 );
     if ( month<1 ) return -1;

     ptrTime = log_a_current_year( month, currentYear );

     ptrTime->tm_mon = month-1;
     ptrTime->tm_mday = atoi( strMsg+4 );
     strMsg += 7;
 }
 else {
     if ( length>19 && strMsg[ 10 ]<=' ' && strMsg[ 19 ]<=' ' ) {
	 thisFormat = 16;
     }

     ptrTime = localtime( &currentStamp );

     ptrTime->tm_year = atoi( strMsg )-1900;
     month = atoi( strMsg+5 );
     if ( month<1 ) return -1;
     ptrTime->tm_mon = month-1;
     ptrTime->tm_mday = atoi( strMsg+8 );
     strMsg += 11;
 }

 ptrTime->tm_hour = atoi( strMsg );
 ptrTime->tm_min = atoi( strMsg+3 );
 ptrTime->tm_sec = atoi( strMsg+6 );

 thisStamp = mktime( ptrTime );

 DBGPRINT("thisFormat=%d, thisStamp=%ld, currentYear=%d, length=%d, %s\n",
	  thisFormat,
	  (unsigned long)thisStamp,
	  currentYear,
	  length,
	  strMsg);
 return thisFormat;
}


int log_dump_mask (FILE* fLog,
		   gSLog& aLog,
		   int hasFork,
		   unsigned thisPid,
		   t_int16 mask)
{
 char identBuf[ 64 ];
 bool isStd( fLog==nil || fLog==stdout || fLog==stderr );
 FILE* fOut( fLog );
 int ioError( -9 );
 int fd( -1 );
 unsigned nMsgs( aLog.NofMsgs() );
 gString* pStr;

 // mask:
 //	0: no TOD (time-of-day)
 //	1: TOD

 if ( nMsgs==0 ) return 0;

 DBGPRINT("DBG: log_dump(handle=%d, aLog, hasFork=%d, thisPid=%d)\n",
	  fLog ? fileno( fLog ) : -1,
	  hasFork,
	  thisPid);

 if ( isStd==false ) {
    fd = fileno( fOut );
    ioError = log_lockf_lock( fd, 0 );
 }

 identBuf[ 0 ] = 0;

 if ( thisPid ) {
     gRandom iRandSeed;
     if ( hasFork ) {
	 iRandSeed.GarbleSeed( thisPid+(unsigned)hasFork );  // without this, forked procs would have same ident!
     }
     char* uniqIdent( aLog.UniqueIdent( 3 ) );

     char chr1( aLog.identIncr<0 ? 'Z' : aLog.base62Chars[ ((unsigned)aLog.identIncr)%26 ] );
     char chr4( aLog.base62Chars[ thisPid%62 ] );
     // The character above (chr1) is the first ident char:
     // the GMT hour, in 'A-...' (alphabetical uppercase) notation

     if ( hasFork!=-1 ) {
     snprintf( identBuf, sizeof(identBuf), "%c%c%s%s%u",
	       chr1,
	       chr4,
	       uniqIdent,
	       uniqIdent[ 0 ] ? "-" : "\0",
	       thisPid );
     }
 }
 DBG_PRESS_KEY(thisPid>0,'c',"Locking-log");

 for (gElem* ptrElem=aLog.Messages().StartPtr();
      ptrElem;
      ptrElem=ptrElem->next) {

     pStr = (gString*)ptrElem->me;
     DBGPRINT_MIN("%d, %d\t{%s}\n", ptrElem->iValue, ptrElem->me->iValue, pStr->Str());

     aLog.DumpMessage( fOut, pStr, identBuf, mask );
 }

 DBG_PRESS_KEY(thisPid>0,'u',"Unlocking-log");
 if ( ioError==0 ) {
     ioError = log_lockf_unlock( fd, 0 );
 }

 return ioError;
}


int log_dump (FILE* fLog,
	      gSLog& aLog,
	      int hasFork,
	      unsigned thisPid)
{
 const t_int16 mask( fLog!=0 );

 DBGPRINT("DBG: log_dump(%p - handle=%d, ..., thisPid=%u), mask: %u\n",
	  fLog,
	  fLog ? fileno( fLog ) : -1,
	  thisPid,
	  mask);

 return log_dump_mask( fLog, aLog, hasFork, thisPid, mask );
}


int log_file_flush (gSLog& aLog, int hasFork, unsigned thisPid)
{
 int result;
 FILE* fLog;

 aLog.xPid = thisPid;
 aLog.Reopen();
 fLog = aLog.Stream();

 result = log_dump( fLog, aLog, hasFork, thisPid );
 aLog.ResetLog();
 return result;
}

////////////////////////////////////////////////////////////
int log_lockf_lock (int fd, int len)
{
#ifdef iDOS_SPEC
 return 0;
#else
 return lockf( fd, F_LOCK, (off_t)len );
#endif //~iDOS_SPEC
}

int log_lockf_unlock (int fd, int len)
{
#ifdef iDOS_SPEC
 return 0;
#else
 return lockf( fd, F_ULOCK, (off_t)len );
#endif //~iDOS_SPEC
}
////////////////////////////////////////////////////////////
