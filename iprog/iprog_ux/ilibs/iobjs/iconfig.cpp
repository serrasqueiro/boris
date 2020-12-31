// iconfig.cpp

#include <string.h>

#include "iconfig.h"

////////////////////////////////////////////////////////////
gFileFetch::gFileFetch (int maxLines)
    : gFileText( NULL, false ),
      doEndNewLine( false ),
      maxNLines( maxLines ),
      isFetchBufferOk( true ),
      doResize( false ),
      doShowProgress( false ),
      fVRepErr( nil )
{
 // Do not use stdout as per gFile default!
 ASSERTION(f==stdout,"gFile default?");
 DBGPRINT("DBG: gFileFetch(maxLines=%d)\n", maxLines);
 f = nil;
}


gFileFetch::gFileFetch (gString& sFName, int maxLines)
    : gFileText( sFName.Str(), true ),
      doEndNewLine( false ),
      maxNLines( maxLines ),
      isFetchBufferOk( true ),
      doResize( IsDevice()==false ),
      doShowProgress( false ),
      fVRepErr( nil )
{
 bool isOk;
 aL.Shown().Set( sFName.Str() );
 DBGPRINT("DBG: gFileFetch(sFName={%s}, maxLines=%d)\n", sFName.Str(), maxLines);
 thisReadFile( isOk, aL );
}


gFileFetch::gFileFetch (const char* fName, int maxLines, bool aShowProgress)
    : gFileText( fName, true ),
      doEndNewLine( false ),
      maxNLines( maxLines ),
      isFetchBufferOk( true ),
      doResize( IsDevice()==false ),
      doShowProgress( aShowProgress ),
      fVRepErr( nil )
{
 bool isOk;
 if ( fName==NULL || fName[0]==0 ) {
     OpenDevice( e_fStdin );
     doShowProgress = false;
 }
 else {
     aL.Shown().Set( fName );
     if ( doShowProgress ) {
	 SetDeviceReport( e_fStderr );
     }
 }
 ioprint("DBG: gFileFetch(%s, %d, ...): seekPos=%d, isBufferOk?%d\n",
	 fName,
	 maxLines,
	 (int)seekPos,
	 (int)isBufferOk);

 thisReadFile( isOk, aL );
}


gFileFetch::gFileFetch (gString& sInput, bool aShowProgress)
    : gFileText( nil, false ),
      doEndNewLine( false ),
      maxNLines( -1 ),
      isFetchBufferOk( true ),
      doResize( false ),
      doShowProgress( aShowProgress ),
      fVRepErr( nil )
{
 if ( doShowProgress ) {
     SetDeviceReport( e_fStderr );
 }
 aL.Shown().Set( sInput.Str() );
 thisReadStringAsFile( sInput, aL );
}


gFileFetch::~gFileFetch ()
{
}


bool gFileFetch::SetFileReport (FILE* fRep)
{
 doShowProgress = fRep!=nil;
 fVRepErr = fRep;
 return doShowProgress;
}


bool gFileFetch::SetDeviceReport (eDeviceKind aDKind)
{
 fVRepErr = nil;
 switch ( aDKind ) {
 case e_fDevOther:
 case e_fStdin:
     return false;
 case e_fStdout:
     fVRepErr = stdout;
     break;
 case e_fStderr:
     fVRepErr = stderr;
     break;
 default:
     return false;
 }
 // doShowProgress is here updated as well...
 return SetFileReport( fVRepErr );
}


bool gFileFetch::Fetch (gString& sFName)
{
 bool isOk;
 int res;

 isOk = sFName.IsEmpty()==false && OpenToRead( sFName.Str() )==true;
 DBGPRINT_MIN("DBG: Fetch(%s) Ok? %c (lastOpError=%d) f=%p (%d)\n",
	      sFName.Str(),
	      ISyORn( isOk ),
	      lastOpError,
	      f,
	      f==stdin);
 if ( isOk==false ) return false;
 res = thisReadFile( isOk, aL );
 DBGPRINT_MIN("DBG: thisReadFile(): res=%d\n",res);
 return isOk && res!=2;
}


int gFileFetch::thisReadFile (bool& isOk, gList& zL)
{
 int error( 0 );
 t_uint32 nBytes;

 DBGPRINT("DBG: thisReadFile(), IsOpened()? %c\n", ISyORn( IsOpened() ));
 if ( IsOpened()==false ) return 2;

 error = thisReadAll( isOk, isFetchBufferOk, zL );
 if ( error!=0 ) return error;
 if ( doResize==false ) return error;

 ASSERTION(IsDevice()==false,"IsDevice()==false");
 // Check if there was a buffer overun
 if ( isFetchBufferOk ) return 0;
 // Empty list meanwhile used: zL
 zL.Delete();
 if ( Rewind()==false ) return 1;
 thisReadFileThrough( zL, nBytes );
 error = nBytes<Size() ? 4 : 0;
 //if ( error!=0 ) fprintf(stderr,"DBG::: nBytes=%ld, size()=%ld\n",(long)nBytes,(long)Size())
 isFetchBufferOk = error==0;
 return error;
}

int gFileFetch::thisReadAll (bool& isOk, bool& isBufOk, gList& zL)
{
 int iCount( 0 );
 int len;
 bool hasCR;

 isBufOk = true;
 lastErrorMsg[ 0 ] = 0;

 while ( ReadLine( isOk, doEndNewLine ) ) {
     iCount++;
     if ( maxNLines>=0 && iCount>maxNLines ) {
	 doEndNewLine = true;
	 return lastOpError = -9;
     }

     zL.Add( Buffer() );

     hasCR = lastErrorMsg[ 0 ]=='#';
     len = strlen( Buffer() );
     zL.EndPtr()->iValue = len + 1 + (int)hasCR;
     zL.EndPtr()->me->iValue = len;

     ioprint("has CR? %c ('%s')\n",
	     ISyORn( hasCR ),
	     lastErrorMsg);

     if ( isBufferOk==false ) {
	 isBufOk = false;
     }
 }
 isOk = true;
 return 0;
}


int gFileFetch::thisReadFileThrough (gList& zL, t_uint32& nBytes)
{
 int handle;
 int len( 0 ), binLength( 0 );
 gString s;
 t_uint32 aSize( Size() );
 t_uchar c;

 ASSERTION(f,"f");
 handle = fileno( f );
 DBGPRINT("DBG: thisReadFileThrough() handle: %d\n", handle);

 nBytes = 0;
 while ( read(handle, &c, 1)==1 ) {
     nBytes++;
     binLength++;
     if ( c=='\r' ) continue;
     if ( c=='\n' ) {
	 zL.Add( s );
	 zL.EndPtr()->iValue = binLength;
	 zL.EndPtr()->me->iValue = len;
	 s.SetEmpty();
	 if ( doShowProgress ) {
	     fprintf(fVRepErr,"%ld of %ld\r",(long)nBytes,(long)aSize);
	     fflush( fVRepErr );
	 }
	 len = binLength = 0;
     }
     else {
	 s.Add( c );
	 len++;
     }
 }
 if ( doShowProgress ) {
     fprintf(fVRepErr,".%20s\n"," ");
 }
 if ( s.IsEmpty() ) return 0;
 zL.Add( s );
 return 0;
}


int gFileFetch::thisReadStringAsFile (gString& sInput, gList& zL)
{
 unsigned i, len=sInput.Length();
 char* str;  //for quickness
 char chr;

 gString sLine;
 for (i=0, str=sInput.Str(); i<len; i++) {
     chr = str[i];
     if ( chr=='\r' ) continue;
     if ( chr=='\n' ) {
	 zL.Add( sLine );
	 sLine.SetEmpty();
     }
     else {
	 sLine.Add( chr );
     }
 }
 doEndNewLine = sLine.IsEmpty();
 if ( doEndNewLine==false ) {
     zL.Add( sLine );
 }
 return 0;
}
////////////////////////////////////////////////////////////

