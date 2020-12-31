// gfile.cpp

#include <stdlib.h> // For: rand/mkstemp
#include <errno.h>
#include <string.h>
#include <time.h>

#include "ifile.h"
#include "lib_iobjs.h"

////////////////////////////////////////////////////////////
// Static members
gFileControl gFileControl::myself;
////////////////////////////////////////////////////////////
gFileOut::gFileOut (FILE* aFile, bool isModeDOS)
    : f( aFile )
{
 SetModeDOS( isModeDOS );
}

gFileOut::~gFileOut ()
{
 if ( f!=NULL && f!=stdout && f!=stdin && f!=stderr )
     fclose( f );
}

void gFileOut::SetModeDOS (bool isDOS)
{
 sNL.SetEmpty();
 if ( isDOS ) sNL.Add( '\r' );
 sNL.Add( '\n' );
}
////////////////////////////////////////////////////////////
gFileControl::gFileControl ()
    : userId( 0 ),
      fOutput( stdout ),
      fReport( stderr )
{
 tmpPath[0] = tmpPrefix[0] = 0;
 CtrlGetTempPath( tmpPath );
 strcpy(tmpPrefix,"tmp_");
#ifdef gGX_USR_SUPPORT
 userId = getuid();
#else
 userId = 0;
#endif
 char* strEnv( getenv("USER") );
 strncpy( strUName, strEnv==NULL ? (char*)"%" : strEnv, sizeof(strUName)-1 );
 strUName[ sizeof(strUName)-1 ] = 0;
 strcpy( tmpSuffix, ".tmp" );
 tempL = new gList;
}


gFileControl::~gFileControl ()
{
}


int gFileControl::CtrlGetTempPath (char* resPathStr, int maxLength)
{
 char* envStr;

 if ( resPathStr==nil ) return -1;

 resPathStr[ 0 ] = 0;
 if ( maxLength>0 ) {
     memset( resPathStr, 0x0, maxLength );
 }
#ifndef iDOS_SPEC
 envStr = getenv( "SystemDrive" );
 if ( envStr ) {
     strcpy(resPathStr,envStr);
 }
#endif //iDOS_SPEC

 strcat(resPathStr,gSLASHSTR);
 strcat(resPathStr,"tmp");
 envStr = getenv("TEMP");
 if ( envStr==nil || envStr[ 0 ]==0 ) {
     envStr = getenv("TMP");
 }

 if ( envStr && envStr[ 0 ] ) {
     if ( maxLength>0 ) {
	 strncpy(resPathStr,envStr,maxLength);
     }
     else {
	 strcpy(resPathStr,envStr);
     }
     return 0;
 }
 return 1;
}


t_uint32 gFileControl::CtrlGetPid ()
{
 return IX_GETPID();
}


t_uint32 gFileControl::GetCurrentEpoch ()
{
 static time_t aT;
 const time_t aTInvalid = ((time_t)-1);

 aT = time(NULL);
 if ( aT==aTInvalid ) {
     lastOpError = errno;
     return 0;
 }
 return (t_uint32)aT;
}


gString& gFileControl::GetUniqueName (const char* s)
{
 t_uint32 aStamp, aRand;
 thisGetUniqueName( s, aStamp, aRand, sName );
 return sName;
}


char* gFileControl::ErrorStr (int aErrorNo)
{
 sErrorRef.SetEmpty();
 if ( aErrorNo<=0 ) return NULL;
 // strerror() primitive would do as well...
 sErrorRef = strerror( aErrorNo );
 // Deprecated: return (char*)sys_errlist[aErrorNo]
 return sErrorRef.Str();
}


gList& gFileControl::TemporaryFiles ()
{
 ASSERTION(tempL,"tempL");
 return *tempL;
}


int gFileControl::RemoveTemp ()
{
 unsigned nTemp;
 return RemoveTemp( nTemp );
}


int gFileControl::RemoveTemp (unsigned& nTemp)
{
 unsigned i;
 int tempErr;

 ASSERTION(tempL!=nil,"tempL!=nil");
 nTemp = tempL->N();
 for (i=1; i<=nTemp; i++) {
     char* str( tempL->Str(i) );
     if ( (tempErr = remove( str ))!=0 ) {
	 lastOpError = tempErr;
     }
     if ( fReport ) {
	 fprintf(fReport,"Cleaning: %s file: %s\n",
		 tempErr==0?"Deleted":"Unable to delete",
		 str);
     }
 }
 delete tempL;
 tempL = new gList;
 return lastOpError;
}


int gFileControl::ReleaseAll ()
{
 if ( tempL==nil ) return -1;
 RemoveTemp();
 delete tempL;
 tempL = nil;
 return lastOpError;
}


bool gFileControl::AddTempFile (gString& s)
{
 ASSERTION(tempL!=nil,"tempL!=nil");
 if ( s.IsEmpty() ) return false;
 tempL->Add( s );
 return true;
}


bool gFileControl::Write (int fHandle, t_uchar* uBuf, unsigned nBytes)
{
 ssize_t bytesWritten;
 if ( fHandle==-1 ) return false;
 bytesWritten = write( fHandle, uBuf, (size_t)nBytes );
 if ( bytesWritten!=(ssize_t)nBytes ) {
     lastOpError = errno;
     return false;
 }
 lastOpError = 0;
 return true;
}


void gFileControl::GetCwd (gString& s)
{
 unsigned aSize;
 char* str;
 s.Reset();
 // Not using PATH_MAX, but supporting an arbitrarily long string
 for (aSize=5000; ; aSize*=2) {
     gUCharBuffer aBuf( aSize+2 );
     str = getcwd( aBuf.Str(), aSize );
     if ( str!=NULL ) {
	 s.Set( aBuf.Str() );
	 return;
     }
     lastOpError = errno;
     s.Reset();
     if ( lastOpError!=ERANGE ) return;
 }
}


int gFileControl::NanoSleep (t_uint32 uSec, t_uint32 nSec)
{
 ASSERTION(nSec<=999999999UL,"NanoSleep(1)");
#ifdef linux
 struct timespec tReq, tRem;

 SecSleep( uSec );

 tReq.tv_sec = 0;
 tReq.tv_nsec = (long)nSec;
 return nanosleep( &tReq, &tRem );
#else
 // Highly artificial Win32 solution (!!)
 static long lInt;
 DBGPRINT("@Win32: Faking nanosleep(%lu,%lu)\n",(unsigned long)uSec,(unsigned long)nSec);
 for (lInt=0; lInt>=0 && lInt<(long)uSec; ) {
     lInt = lInt + (long)((float)1.0);
 }
 return 0;
#endif //linux
}


int gFileControl::SecSleep (t_uint32 aSec)
{
 unsigned seconds( (unsigned)aSec );
 if ( aSec > MAX_UINT16_U ) {
	for ( ; aSec>1; aSec--) {
#ifdef iDOS_SPEC
		_sleep( 1000 );
#else
		sleep( 1 );
#endif
	}
 }
 else {
#ifdef iDOS_SPEC
	_sleep( seconds*1000 );
#else
	sleep( seconds );
#endif
 }
 return 0;
}


int gFileControl::MiliSecSleep (t_uint32 mSec)
{
 DBGPRINT_MIN("PACE(%u)\n",(unsigned)mSec);
#ifdef linux
 static t_uint32 aSec;
 if ( mSec>=1000 ) {
	aSec = mSec / 1000;
	mSec %= 1000;
	mSec *= 1000; // micro-seconds
	mSec *= 1000; // nano-seconds
	return NanoSleep( aSec, mSec );
 }
 return NanoSleep( 0, mSec*1000*1000 );
#else
 #ifdef iDOS_SPEC
	_sleep( mSec );
 #else
	Sleep( mSec );
 #endif
#endif  //~linux
 return 0;
}


int gFileControl::StreamClose (FILE** ptrFile, eSafeStream safeStream)
{
 FILE* aFile;
 if ( ptrFile==nil ) return -1;
 aFile = *ptrFile;
 if ( aFile ) {
     if ( aFile!=stdout && aFile!=stderr && aFile!=stdin ) {
	 fclose( aFile );
	 *ptrFile = nil;
	 return 0;
     }
     return fileno( aFile );
 }
 // Stream to close is NULL already!
 return -1;
}


int gFileControl::thisGetUniqueName (const char* s,
				     t_uint32& aStamp,
				     t_uint32& aRand,
				     gString& sRes)
{
 static t_uint32 aCnt;
 gString sTemp( tmpPath ), sEpoch( 50, '\0' );
 sTemp.Add( gSLASHCHR );
 sTemp.Add( tmpPrefix );
 if ( s!=nil ) sTemp.Add( s );
 aStamp = GetCurrentEpoch();
 aRand = ++aCnt;
 sprintf( sEpoch.Str(), "%04x-%04x_%03X-%lu%s",
	  (unsigned)userId,
	  (unsigned)CtrlGetPid(),
	  (unsigned)(aRand%0xFFF),
	  (unsigned long)aStamp,
	  tmpSuffix );
 sTemp.AddString( sEpoch );
 sName = sTemp;
 return 0;
}
////////////////////////////////////////////////////////////
// gFile - Generic file handling
// ---------------------------------------------------------
gFile::gFile (eFileKind aFKind, const char* fName, bool doOpenToRead, bool isTmpFile)
    : lastOpError( 0 ),
      f( NULL ),
      fKind( aFKind ),
      dKind( e_fDevOther ),
      fMode( doOpenToRead ? FL_FILE_TO_READ : 0 )
{
 lastErrorMsg[0] = 0;
 if ( fName==nil || fName[0]==0 ) {
     DBGPRINT("DBG: gFile::gFile isTmpFile? %c, doOpenToRead? %c\n",
	      ISyORn( isTmpFile ),
	      ISyORn( doOpenToRead ));
     if ( isTmpFile==false ) OpenDevice( doOpenToRead ? e_fStdin : e_fStdout );
 }
 else {
     if ( doOpenToRead==true ) {
	 f = fopen(fName,fKind==e_Text?"rt":"rb");
	 if ( f==nil ) lastOpError = errno;
     }
     DBGPRINT("DBG: gFile::gFile f=%p, e_Text? %c, lastOpError=%d\n",
	      f,
	      ISyORn( fKind==e_Text ),
	      lastOpError);
 }
}


gFile::~gFile ()
{
 Close();
}


bool gFile::ReadData (void* buf, t_uint16 bufSize)
{
 t_uint16 nBytes;
 return ReadBuffer( buf, bufSize, nBytes );
}


bool gFile::ReadBuffer (void* buf, t_uint16 bufSize, t_uint16& nBytes)
{
 ;
 // Reads one chunk of data of 'bufSize' bytes
 ;
 size_t nReadItems;

 ASSERTION(buf!=NULL,"buf!=NULL");
 nBytes = 0;
 lastOpError = -1;
 if ( f==nil ) return false;
 //size_t fread (void *ptr, size_t size, size_t nmemb, FILE *stream)
 nReadItems = fread(buf,bufSize,1,f);
 if ( nReadItems==0 ) {
     lastOpError = errno;
     return false;
 }
 nBytes = (t_uint16)bufSize;
 lastOpError = 0;
 return true;
}


bool gFile::Read (void* buf, t_uint16 bufSize, t_uint16& nBytes)
{
 // Try to read 'bufSize' bytes into 'buf'.
 ASSERTION(buf!=NULL,"buf!=NULL");
 if ( f==nil ) return false;
 return thisRead(fileno(f),buf,bufSize,nBytes);
}


char* gFile::LastErrorStr ()
{
 lastErrorMsg[0] = 0;
 if ( lastOpError==0 ) return lastErrorMsg;
 strcpy( lastErrorMsg, "Unknown internal error" );
 if ( lastOpError<0 ) return lastErrorMsg;
 strcpy( lastErrorMsg, ErrorStr( lastOpError ) );
 return lastErrorMsg;
}


char* gFile::ErrorStr (int aErrorNo)
{
 return gFileControl::Self().ErrorStr( aErrorNo );
}


bool gFile::OpenDevice (eDeviceKind aDKind)
{
 Close();
 switch ( aDKind ) {
 case e_fDevOther:
     return false;
 case e_fStdin:
     fMode = FL_FILE_TO_READ;
     f = stdin;
     break;
 case e_fStdout:
 case e_fStderr:
     fMode = FL_FILE_TO_WRITE;
     f = aDKind==e_fStdout ? stdout : stderr;
     break;
 default:
     ASSERTION_FALSE("OpenDevice(1)");
     break;
 }
 dKind = aDKind;
 return true;
}


bool gFile::OpenToRead (const char* fName)
{
 lastErrorMsg[0] = 0;
 lastOpError = 0;
 if ( IsOpened() ) return false;

 fMode = FL_FILE_TO_READ;
 if ( fName!=NULL && fName[0]!=0 ) {
     f = fopen(fName,fKind==e_Text?"rt":"rb");
     if ( f==nil ) lastOpError = errno;
 }
 return lastOpError==0;
}


bool gFile::Overwrite (const char* fName)
{
 Close();
 fMode = FL_FILE_TO_WRITE;
 if ( fName==nil || fName[0]==0 ) return false;
 dKind = e_fDevOther;
 f = fopen(fName,fKind==e_Text?"wt":"wb");
 if ( f==nil ) lastOpError = errno;
 return f!=NULL;
}


bool gFile::Close ()
{
 bool wasOpened( f!=NULL );
 lastOpError = 0;
 DBGPRINT("DBG: gFile::Close wasOpened? %c, closing(%p)? %c\n",
	  ISyORn( wasOpened ),
	  f,
	  ISyORn( f!=NULL && dKind==e_fDevOther ));
 if ( f!=NULL && dKind==e_fDevOther ) fclose(f);
 f = NULL;
 return wasOpened;
}


bool gFile::thisRead (int fd, void* buf, t_uint16 bufSize, t_uint16& nBytes)
{
 // Tries to read one chunk of data of 'bufSize' bytes; if read less than
 // that, still returns true but returns 'nBytes' that were effectively read.

 // From:
 //   #include <unistd.h>
 //   ssize_t read(int fd, void *buf, size_t count);
 ssize_t readResult;

 lastOpError = -1;
 if ( fd==-1 ) return false;
 lastOpError = 0;
 readResult = read(fd,buf,bufSize);
 nBytes = 0;
 if ( readResult<0 ) {
     lastOpError = errno;
     return false;
 }
 nBytes = (t_uint16)readResult;
 return true;
}
////////////////////////////////////////////////////////////
gFileStream::gFileStream (const char* fName, bool doOpenToRead)
    : gFile( gFile::e_Text, fName, doOpenToRead ),
      isOpOk( true ),
      isFileChanged( false ),
      isBufferOk( true ),
      seekPos( 0 ),
      seekEnd( 0 ),
      bufferSize( DEF_FIL_BUFSIZE ),
      buffer( NULL )
{
 int fd=-1;
 isOpOk = f!=NULL && (fd = fileno(f))!=-1;
 if ( isOpOk ) {
     isOpOk = thisGetStreamSize(fd)>=0;
 }
 thisAllocateBuffer( bufferSize );

 ioprint("gFileStream(%s) constructor: seekPos=%d\n",
	 fName,
	 (int)seekPos);
}


gFileStream::gFileStream (gFile::eFileKind aFKind, const char* fName, bool doOpenToRead, bool isTmpFile)
    : gFile( aFKind, fName, doOpenToRead, isTmpFile ),
      isOpOk( true ),
      isFileChanged( false ),
      isBufferOk( true ),
      seekPos( 0 ),
      seekEnd( 0 ),
      bufferSize( DEF_FIL_BUFSIZE ),
      buffer( NULL )
{
 int fd=-1;
 isOpOk = f!=NULL && (fd = fileno(f))!=-1;
 if ( isOpOk ) {
     isOpOk = thisGetStreamSize(fd)>=0;
 }
 thisAllocateBuffer( bufferSize );

 ioprint("gFileStream(aFKind=%d) constructor: seekPos=%d\n",
	 (int)aFKind,
	 (int)seekPos);
}


gFileStream::~gFileStream ()
{
 delete[] buffer;
 buffer = NULL;
}


char* gFileStream::Buffer ()
{
 ASSERTION(buffer!=NULL,"buffer!=nil");
 return (char*)buffer;
}


t_uchar* gFileStream::UBuffer ()
{
 ASSERTION(buffer!=NULL,"buffer!=nil");
 return buffer;
}


bool gFileStream::Overwrite (const char* fName)
{
 isOpOk = gFile::Overwrite( fName );
 if ( isOpOk==false ) return false;
 isFileChanged = true;
 isBufferOk = true;
 seekPos = seekEnd = 0;
 return true;
}


bool gFileStream::ReadBuffer (void* buf, t_uint16 bufSize, t_uint16& nBytes)
{
 t_uint16 anBytes=0;
 isOpOk = gFile::ReadBuffer(buf,bufSize,anBytes);
 seekPos += anBytes;
 return isOpOk;
}


bool gFileStream::Read (void* buf, t_uint16 bufSize, t_uint16& nBytes)
{
 if ( f==nil ) return false;
 isOpOk = gFile::Read(buf,bufSize,nBytes);
 seekPos += nBytes;
 return isOpOk;
}


bool gFileStream::Rewind ()
{
 int error;
 if ( IsOpened()==false || IsDevice()==true ) return false;
 error = fseek( f, 0L, SEEK_SET );
 lastOpError = errno;
 if ( error!=0 ) return false;
 seekPos = 0;
 return true;
}


int gFileStream::thisAllocateBuffer (t_uint16 aBufferSize)
{
 // Returns 0 on success
 ASSERTION(aBufferSize>0,"aBufferSize>0");
 if ( buffer!=NULL ) {
     delete[] buffer;
 }
 buffer = new t_uchar[aBufferSize];
 ASSERTION(buffer!=nil,"buffer!=nil");
 return buffer==nil;
}


int gFileStream::thisGetStreamSize (int fd)
{
 off_t seekResult;

 if ( fd==-1 ) return -1;
 seekResult = lseek(fd,(off_t)0,SEEK_END);
 lastOpError = errno;
 if ( seekResult==(off_t)-1 ) return -1;
 if ( seekEnd!=seekResult ) {
     isFileChanged = true;
 }
 seekEnd = seekResult;
 seekResult = lseek(fd,0,SEEK_SET);
 lastOpError = errno;
 seekPos = 0;
 return isFileChanged==true;
}

////////////////////////////////////////////////////////////
gFileText::gFileText (const char* fName, bool doOpenToRead)
    : gFileStream( gFile::e_Text, fName, doOpenToRead)
{
 ioprint("gFileText(%s) constructor, doOpenToRead?%c: seekPos=%d\n",
	 fName,
	 ISyORn( doOpenToRead ),
	 (int)seekPos);
}


gFileText::~gFileText ()
{
}


bool gFileText::ReadLine ()
{
 bool isOk;
 bool hasNewLine;
 return ReadLine( isOk, hasNewLine );
}


bool gFileText::ReadLine (bool& isOk)
{
 bool hasNewLine;
 return ReadLine( isOk, hasNewLine );
}


bool gFileText::ReadLine (bool& isOk, bool& hasNewLine)
{
 // Returns true on success;
 // isOk is true if buffer is big enough for the line
 // that is being read.
 static unsigned readLineLen;

 lastOpError = 0;

 // Clean just the first position of buffer,
 // since it is a text-file read-out.
 buffer[0] = 0;
 isOk = IsOpened();
 if ( isOk==false ) {
     lastOpError = -1;
     return false;
 }

 ioprint("DBG: readLineLen at 1: %d, bufferSize=%d, seekPos=%d, isBufferOk?%d\n",
	 readLineLen,
	 bufferSize,
	 (int)seekPos,
	 (int)isBufferOk);

 isOk = false;
 if ( fgets((char*)buffer,bufferSize,f)==NULL ) return false;

 readLineLen = (unsigned)strlen( (char*)buffer );
 seekPos += (off_t)readLineLen;
 isBufferOk = readLineLen+1 < bufferSize;
 // At least one character is present when 'fgets' succeeds.

 ioprint("DBG: readLineLen at 2: %d, bufferSize=%d, seekPos=%d, isBufferOk?%d\n",
	 readLineLen,
	 bufferSize,
	 (int)seekPos,
	 (int)isBufferOk);

 if ( readLineLen==0 ) return false;
 readLineLen--;
 hasNewLine = buffer[readLineLen]=='\n';
 if ( hasNewLine ) {
     buffer[readLineLen] = 0;
     if ( readLineLen>0 && buffer[--readLineLen]=='\r' ) {
	 buffer[readLineLen] = 0;
	 strcpy(lastErrorMsg, LINE_HAS_CR);
     }
 }

 isOk = true;
 return true;
}
////////////////////////////////////////////////////////////
gFileTemp::gFileTemp (const char* strTemplate, gFile::eFileKind aFKind)
    : gFileStream( aFKind, nil, false, true ),
      fHandle( -1 ),
      tempMethod( e_NameStd ),
      isHandledFStream( false )
{
 ;
 // This contructor is designed to be used with
 //    <unistd.h>
 //    ssize_t write(int fd, const void *buf, size_t count)

 char* str;
 unsigned pos;

 gString sTemplate( strTemplate );
 gString sName( gFileControl::Self().tmpPath );

 pos = sTemplate.Find("XXXXXX");
 if ( pos>0 && pos+6-1==sTemplate.Length() ) {
     str = sTemplate.Str();
 }
 else {
     sName.Add( gSLASHCHR );
     sName.Add( gFileControl::Self().tmpPrefix );
     sName.Add( "XXXXXX" );
     str = sName.Str();
 }

#if defined(linux) || defined(gDOS_LIB_XIO)
 fHandle = mkstemp( str );
 // Note str gets overwritten by mkstemp
 thisOverwrite( str );
#else
 char* pTemplate = mktemp( str );
 ASSERTION(pTemplate!=nil,"pTemplate!=nil");
 str = pTemplate;
 f = fopen( str, "wb" );
 ASSERTION(f!=nil,"f!=nil");
 sTempName = str;
 if ( (isHandledFStream = f!=nil)==true ) fHandle = fileno( f );
 DBGPRINT("DBG: TEMP PATH: <%s>, fHandle=%d\n",str,fHandle);
#endif //linux...
}


gFileTemp::gFileTemp (gFile::eFileKind aFKind)
    : gFileStream( aFKind, nil, false, true ),
      fHandle( -1 ),
      tempMethod( e_NamePre ),
      isHandledFStream( false )
{
 gString sName( gFileControl::Self().GetUniqueName( gFileControl::Self().CtrlGetUserNameStr() ) );
 if ( thisOverwrite( sName.Str() )==0 ) {
     fHandle = fileno( f );
 }
}


gFileTemp::~gFileTemp ()
{
 char* strNameTemp;
 if ( IsOpened() ) {
     switch ( tempMethod ) {
     case e_NameStd:
     case e_NamePre:
	 if ( sTempName.IsEmpty()==false ) {
	     if ( isHandledFStream ) Close();
	     strNameTemp = sTempName.Str();
	     lastOpError = remove( strNameTemp )!=0;
	     if ( lastOpError!=0 ) fprintf(stderr,"Unable to delete: %s\n",strNameTemp);
	     gFileControl::Self().TemporaryFiles().DeleteString( sTempName );
	 }
	 break;
     default:
	 ASSERTION_FALSE("gFileTemp::~gFileTemp");
	 break;
     }
 }
}


bool gFileTemp::IsOpened ()
{
 switch ( tempMethod ) {
 case e_NameStd:
     return fHandle!=-1;
 case e_NamePre:
     return gFileStream::IsOpened();
 default:
     return false;
 }
}


bool gFileTemp::Rewind ()
{
 off_t seekResult;

 DBGPRINT("DBG: :::gFileTemp::Rewind(<%s>,method=%d,fHandle=%d)\n",sTempName.Str(),tempMethod,fHandle);

 switch ( tempMethod ) {

 case e_NameStd:
     if ( fHandle==-1 ) return false;
#ifdef iDOS_SPEC
     // Limitation: file was re-opened...
     ASSERTION(f!=nil,"f!=nil");
     fclose( f );
     fHandle = -1;
     f = fopen( sTempName.Str(), "rb" );
     seekResult = f==nil ? -1 : 0;
     isHandledFStream = f!=nil;
     if ( isHandledFStream ) fHandle = fileno( f );
#else
     seekResult = lseek( fHandle, 0L, SEEK_SET );
#endif //iDOS_SPEC
     return seekResult!=(off_t)-1;

 case e_NamePre:
     ASSERTION(fHandle!=-1,"Rewind(2)");
     if ( IsOpened()==false ) return false;
     ASSERTION(f!=nil,"Rewind(2)");
     fclose( f );
     DBGPRINT("DBG: reopening '%s'\n",sTempName.Str());
     f = fopen(sTempName.Str(),"rb");
     fHandle = -1;
     if ( f==nil ) return false;
     fHandle = fileno( f );
     break;
 default:
     ASSERTION_FALSE("Rewind(5)");
     break;
 }//end CASE
 return true;
}


int gFileTemp::thisOverwrite (const char* fName)
{
 ASSERTION(fName!=nil,"fName!=nil");
 switch ( tempMethod ) {
 case e_NameStd:
     ASSERTION(fHandle!=-1,"fHandle!=-1");
     break;
 case e_NamePre:
     if ( Overwrite( fName )==false ) return -1;
     break;
 default:
     ASSERTION_FALSE("thisOverwrite");
 }

 sTempName.Set( fName );
 gFileControl::Self().AddTempFile( sTempName );
 return lastOpError;
}
////////////////////////////////////////////////////////////

