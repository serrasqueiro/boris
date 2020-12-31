// istorage.cpp

#include "istorage.h"
#include "ifile.h"

////////////////////////////////////////////////////////////
// Static members
// ---------------------------------------------------------
short gTop::nObjHistogram[MAX_INTSTGKIND];
int gTop::nObjs=0;
t_int32 gTop::maxDescriptionIndex=MAX_INT_VALUE;
t_desc_char* gTop::noDescriptionStr=(t_desc_char*)"";

gUCharBuffer gStorage::oneCharBuf( 20 );
gStorage::eStoreMethod gStorage::globalStoreMethod=gStorage::e_StgGlobalString;

////////////////////////////////////////////////////////////
// gTop - Generic control for objects
// ---------------------------------------------------------
gTop::gTop (t_uint16 intStgKind)
    : stgKind( intStgKind ),
      descriptionIndex( 0 ),
      ptrDescription( nil )
{
 nObjs++;
 ASSERTION(stgKind>=1 && stgKind<MAX_INTSTGKIND,"gTop::gTop");
 nObjHistogram[ stgKind ]++;
 memset(strIndex, 0x0, sizeof(strIndex));
}


gTop::~gTop ()
{
 nObjs--;
 DBGPRINT_MIN("DBG: gTop::~gTop: nObjs=%d, stgKind=%d\n",nObjs,stgKind);
 nObjHistogram[ stgKind % MAX_INTSTGKIND ]--;
 if ( ptrDescription ) {
     for (gElem* pDesc=gStorageControl::Self().DescriptionList().StartPtr(); pDesc; pDesc=pDesc->next) {
	 if ( pDesc->me->LRef()==this ) {
	     pDesc->me->iValue = -1;
	     break;
	 }
     }
 }
}


int gTop::StringHash (const char *aStr)
{
 static t_uchar thisChr;
 register t_uchar* p;
 register int x, iLength;

 if ( aStr==nil ) return -2;
 p = (t_uchar*)aStr;

 iLength = 0;
 x = (*p << 7);
 while ( (thisChr = *p)!=0 ) {
     x = (1000003*x) ^ thisChr;
     p++;
     iLength++;
 }
 x ^= iLength;
 if ( x==-1 ) return -2;
 return x;
}


int gTop::StringHash (const char *aStr, int iLength)
{
 register t_uchar* p;
 register int len, x;

 len = iLength;
 if ( len<=0 ) return 0;

 p = (t_uchar*)aStr;

 x = (*p << 7);
 while ( --len>=0 ) {
     x = (1000003*x) ^ *p++;
 }
 x ^= iLength;
 if ( x==-1 ) return -2;
 return x;
}

////////////////////////////////////////////////////////////
gUCharBuffer::gUCharBuffer (t_uint16 bufSize)
    : size( bufSize ),
      uBuf( nil )
{
 if ( size==0 ) size = GENUCHAR_USU_BUFSIZE;
 thisAllocate( size );
}


gUCharBuffer::~gUCharBuffer ()
{
 delete[] uBuf;
 uBuf = nil;
}


void gUCharBuffer::Copy (t_uchar* s)
{
 thisCopy( s, strlen((char*)s) );
}


void gUCharBuffer::Copy (char* s)
{
 thisCopy( (t_uchar*)s, strlen(s) );
}


void gUCharBuffer::Clear ()
{
 memset( uBuf, 0x0, (size_t)size );
}


void gUCharBuffer::thisAllocate (t_uint16 bufSize)
{
 delete[] uBuf;
 uBuf = new t_uchar[ size = bufSize ];
 ASSERTION(uBuf!=nil,"gUCharBuffer::thisAllocate");
 Clear();
}


void gUCharBuffer::thisCopy (t_uchar* s, t_uint16 len)
{
 if ( len+1>=size ) {
     // Buffer somehow short, rebuilding it
     thisAllocate( len*2 );
 }
 Clear();
 for (t_uint16 iter=0; iter<len; iter++) uBuf[iter] = s[iter];
}

////////////////////////////////////////////////////////////
// gStorage - Generic storage handling
// ---------------------------------------------------------
gStorage::gStorage (gStorage::eStorage aKind,
		    gStorage::eStoreMethod aMethod,
		    int aioMask)
    : gTop( (t_uint16)aKind ),
      iValue( 0 ),
      kind( aKind ),
      storeMethod( aMethod ),
      ioMask( aioMask ),
      lRef( nil )
{
}


gStorage::~gStorage ()
{
}


char* gStorage::Str (unsigned idx)
{
 return (char*)oneCharBuf.uBuf;
}


int gStorage::Compare (gStorage& comp)
{
 DBGPRINT_MIN("gStorage::Compare(%s) to me: {%s}, IsString()? %c\n",
	      comp.Str(),
	      Str(),
	      ISyORn( IsString() ));
 if ( IsString() ) {
     return CompareStr( comp.Str() );
 }
 return CompareInt( comp.iValue );
}


int gStorage::CompareInt (int v)
{
 if ( iValue==v )
     return 0;
 return iValue < v ? -1 : 1;
}


int gStorage::CompareStrs (const char* aStr1, const char* aStr2)
{
 if ( aStr1!=nil && aStr1[ 0 ]!=0 ) {
     if ( aStr2 )
	 return strcmp( aStr1, aStr2 );
     // See also gString...::CompareStr
     return -1;
 }
 if ( aStr2!=nil && aStr2[ 0 ]!=0 )
     return 1;
 return 0;
}


gStorage* gStorage::ValidLRef ()
{
 ASSERTION(lRef, "lRef");
 return lRef;
}


char* gStorage::AllocateChars (unsigned nBytes, bool asserted)
{
 char* strResult( new char[ nBytes+1 ] );
 if ( asserted ) {
     ASSERTION(strResult,"strResult");
 }
 return strResult;
}


char* gStorage::DupChars (const char* s, bool asserted)
{
  unsigned len( s ? strlen( s ) : 1 );
  char* strResult( AllocateChars( len, asserted ) );
  if ( strResult ) {
      if ( s )
	  strcpy( strResult, s );
      else
	  strResult[ 0 ] = 0;
  }
  return strResult;
}


void gStorage::SetStoreMethod (gStorage::eStoreMethod aMethod)
{
 eStoreMethod resMethod( e_StgDefault );
 bool isOk( thisSetStoreMethod( aMethod, storeMethod ) );
 ASSERTION(isOk,"gStorage::SetStoreMethod");
 storeMethod = resMethod;
}


bool gStorage::EndGuts ()
{
 // Return true if this object is 'active'
 return false;
}


void gStorage::Show (bool doShowAll)
{
 if ( kind>=e_UnusedStore ) {
     iprint("[unknown]");
 }
}


bool gStorage::thisSetStoreMethod (gStorage::eStoreMethod aMethod,
				   gStorage::eStoreMethod& resMethod)
{
 resMethod = aMethod;

 switch ( aMethod ) {
 case gStorage::e_StgNoStore:
     break;
 case gStorage::e_StgGlobalString:
     globalStoreMethod = aMethod;
     break;
 case gStorage::e_StgGlobalFlat:
     globalStoreMethod = aMethod;
     break;
 case gStorage::e_StgString:
 case gStorage::e_StgFlat:
 case gStorage::e_StgDefault:
     break;
 default:
     return false;
 }
 return true;
}


bool gStorage::CanSave (FILE* f)
{
 return CanRestore( f );
}


bool gStorage::CanRestore (FILE* f)
{
 if ( globalStoreMethod==e_StgNoStore ) return false;
 return f!=NULL;
}
////////////////////////////////////////////////////////////
void gUChar::Reset ()
{
 gStorage::Reset();
 c = 0;
}


gStorage* gUChar::NewObject ()
{
 gUChar* a( new gUChar( c ) );
 return a;
}


t_uchar* gUChar::ToString (const t_uchar* uBuf)
{
 if ( uBuf==nil ) return nil;
 sprintf( (char*)uBuf, "%c", c );
 return (t_uchar*)uBuf;
}


bool gUChar::SaveGuts (FILE* f)
{
 if ( CanSave( f )==false ) return false;
 return fprintf( f,"%c", c );
}


bool gUChar::RestoreGuts (FILE* f)
{
 if ( CanRestore( f )==false ) return false;
 return fscanf( f, "%c", &c )>=1;
}
////////////////////////////////////////////////////////////
void gInt::Reset ()
{
 gStorage::Reset();
 iValue = 0;
}


gStorage* gInt::NewObject ()
{
 gInt* a( new gInt( iValue ) );
 return a;
}


t_uchar* gInt::ToString (const t_uchar* uBuf)
{
 if ( uBuf==nil ) return nil;
 sprintf( (char*)uBuf, "%d", iValue );
 return (t_uchar*)uBuf;
}


bool gInt::SaveGuts (FILE* f)
{
 if ( CanSave( f )==false ) return false;
 return fprintf( f, "%d", iValue );
}


bool gInt::RestoreGuts (FILE* f)
{
 if ( CanRestore( f )==false ) return false;
 return fscanf( f, "%d", &iValue )>=1;
}


void gInt::Show (bool doShowAll)
{
 iprint("%s%d",doShowAll?"'":"\0",iValue);
}
////////////////////////////////////////////////////////////
gUInt::gUInt (unsigned v)
    : gInt( (int)v )
{
}


gUInt::~gUInt ()
{
}
////////////////////////////////////////////////////////////
void gReal::Reset ()
{
 gStorage::Reset();
 c = 0.0;
}


gStorage* gReal::NewObject ()
{
 gReal* a( new gReal( (float)c ) );
 return a;
}


t_uchar* gReal::ToString (const t_uchar* uBuf)
{
 if ( uBuf==nil ) return nil;
 sprintf( (char*)uBuf, "%f", c);
 return (t_uchar*)uBuf;
}


bool gReal::SaveGuts (FILE* f)
{
 if ( CanSave( f )==false ) return false;
 return fprintf( f, "%f", c );
}


bool gReal::RestoreGuts (FILE* f)
{
 bool isOk;
 float val;
 if ( CanRestore( f )==false ) return false;
 isOk = fscanf( f, "%f", &val );
 if ( isOk ) c = val;
 return isOk;
}


void gReal::Show (bool doShowAll)
{
 iprint("%s%f",doShowAll?"`":"\0",c);
}
////////////////////////////////////////////////////////////

