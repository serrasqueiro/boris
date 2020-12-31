// istring.cpp
//
// Now also with Hash()/Rehash() functions.

#include "istring.h"
#include "icontrol.h"
#include "ifile.h"

////////////////////////////////////////////////////////////
// Static members
// ---------------------------------------------------------
t_uchar gStringGeneric::myChrNul=0;
bool gStringGeneric::doDigConvertRelaxed=true;

////////////////////////////////////////////////////////////
// gStringGeneric - Generic string handling
// ---------------------------------------------------------
gStringGeneric::gStringGeneric (eStorage kind, const t_uchar* s)
    : gStorage( kind, e_StgDefault ),
      size( 0 ),
      str( nil )
{
 thisPreAllocate( (char*)s );
}


gStringGeneric::~gStringGeneric ()
{
 thisDelete();
}


char* gStringGeneric::Str (unsigned idx)
{
 static char strEmpty[ 2 ];
 //printf("Str(%u): %p {%s}\n", idx, str, str);
 if ( idx ) {
     if ( idx>size ) return strEmpty;
     return (char*)(str + idx);
 }
 return (char*)str;
}


t_uchar* gStringGeneric::UStr ()
{
 return str;
}


t_uchar* gStringGeneric::StrPlus (int index)
{
 if ( index>=0 && index<(int)size ) {
     return str + index;
 }
 return nil;
}


unsigned gStringGeneric::CountChars (t_uchar uChr, eBasicCaseSense senseKind)
{
 unsigned i( 0 ), n( size ), count( 0 );

 switch ( senseKind ) {
 case e_BasicDoCaseSense:
      for ( ; i<n; i++) {
	  count += (uChr==str[ i ]);
      }
      break;

 case e_BasicDoIgnoreCase:
 default:
     if ( uChr>='a' && uChr<='z' ) uChr -= 32;
      for (t_uchar myChr; i<n; i++) {
	  myChr = str[ i ];
	  if ( myChr>='a' && uChr<='z' ) myChr -= 32;
	  count += (uChr==myChr);
      }
 }
 return count;
}


int gStringGeneric::CompareStr (const char* aStr)
{
 DBGPRINT_MIN("gStringGeneric::Compare(%s) to me: {%s}\n",
	      aStr,
	      str);
 if ( str!=nil && str[0]!=0 ) {
     if ( aStr ) {
	 return strcmp( (char*)str, (char*)aStr );
     }
     // The compared string is nil, therefore aStr is lesser than str:
     return -1;
 }
 if ( aStr!=nil && aStr[0]!=0 ) {
     // The compared string is not-nil, therefore aStr is greater than str:
     return 1;
 }
 // Both str and the compared string are nil or empty: therefore equal:
 return 0;
}


t_uchar gStringGeneric::GetUChar (unsigned idx)
{
 if ( thisIndex(idx)==false ) return myChrNul;
 return str[idx-1];
}


void gStringGeneric::Reset ()
{
 gStorage::Reset();
 SetEmpty();
}


void gStringGeneric::SetEmpty ()
{
 thisDelete();
 thisPreAllocate( "\0" );
}


void gStringGeneric::Set (const char* s)
{
 if ( s ) {
     Set( (t_uchar*)s );
 }
 else {
     SetEmpty();
 }
}


void gStringGeneric::Set (char* s)
{
 if ( s ) {
     Set( (t_uchar*)s );
 }
 else {
     SetEmpty();
 }
}


void gStringGeneric::Set (t_uchar* s)
{
 ASSERTION(s!=nil,"gStringGeneric::Set");
 thisDelete();
 thisPreAllocate( (char*)s );
}


void gStringGeneric::Set (int v)
{
 thisDelete();
 Add( v );
}


void gStringGeneric::Set (unsigned v)
{
 thisDelete();
 Add( v );
}


unsigned gStringGeneric::Add (char c)
{
 char s[2];
 s[0] = c;
 s[1] = 0;
 return Add( s );
}


unsigned gStringGeneric::Add (t_uchar c)
{
 return Add( (char)c );
}


unsigned gStringGeneric::Add (const char* s)
{
 return Add( (t_uchar*)s );
}


unsigned gStringGeneric::Add (t_uchar* s)
{
 unsigned len, totalLen;
 int copyCode;

 if ( s==nil ) return size;
 len = strlen( (char*)s );
 if ( size==0 ) {
     thisCopy( (char*)s, len );
     return len;
 }
 // Re-calculate existing string length
 size = strlen( (char*)str );
 totalLen = size+len;
 // Keep the old string
 gString tempStr( str );
 copyCode = thisCopy( tempStr.Str(), totalLen );
 if ( copyCode<0 ) {
     copyCode = thisCopy( tempStr.Str(), tempStr.Length() );
     ASSERTION(copyCode>=0,"copyCode>=0");
     return size;
 }
 strcat( (char*)str, (char*)s );
 return size;
}


unsigned gStringGeneric::Add (int v)
{
 gUCharBuffer s( 30 );
 sprintf( (char*)s.uBuf, "%d", v );
 Add( s.uBuf );
 return size;
}


unsigned gStringGeneric::Add (unsigned v)
{
 gUCharBuffer s( 30 );
 sprintf( (char*)s.uBuf, "%u", v );
 Add( s.uBuf );
 return size;
}


void gStringGeneric::UpString ()
{
 unsigned k( 0 );
 t_uchar chr;
 for ( ; (chr = str[ k ])!=0; k++) {
     if ( chr>='a' && chr<='z' ) {
	 chr = (t_uchar)(chr - 32);
	 str[ k ] = chr;
     }
 }
}


void gStringGeneric::DownString ()
{
 unsigned k( 0 );
 t_uchar chr;
 for ( ; (chr = str[ k ])!=0; k++) {
     if ( chr>='A' && chr<='Z' ) {
	 chr = (t_uchar)(chr + 32);
	 str[ k ] = chr;
     }
 }
}


t_uchar& gStringGeneric::CharAtIndex (int index)
{
 if ( thisIsValidIndex(index)==false ) return myChrNul;
 return str[ index-1 ];
}


t_uchar* gStringGeneric::ToString (const t_uchar* uBuf)
{
 if ( uBuf!=nil ) {
     strcpy( (char*)uBuf, (char*)str );
 }
 return str;
}


bool gStringGeneric::SaveGuts (FILE* f)
{
 if ( CanSave( f )==false ) return false;
 if ( str==nil ) return true;
 return fprintf(f,"%s",str)==1;
}


bool gStringGeneric::RestoreGuts (FILE* f)
{
 unsigned n=0;
 t_uchar uChr;
 gUCharBuffer sTemp;

 if ( CanRestore( f )==false ) return false;
 thisDelete();

 while ( fscanf(f,"%c",&uChr) ) {
     if ( uChr=='\n' || uChr==0 ) break;
     sTemp.uBuf[n++] = uChr;
     sTemp.uBuf[n] = 0;
     if ( n>=sTemp.size ) return false;
 }
 thisCopy( (char*)sTemp.uBuf, n );
 return true;
}


void gStringGeneric::thisPreAllocate (const char* s)
{
 if ( s ) {
     thisCopy( s, strlen( (char*)s ) );
 }
}


void gStringGeneric::thisDelete ()
{
 size = 0;
 if ( str!=nil ) {
     delete[] str;
 }
 str = nil;
}


int gStringGeneric::thisCopy (const char* s, unsigned len)
{
 thisDelete();
 if ( s==nil ) return -1;
 str = new t_uchar[ len+1 ];
 ASSERTION(str!=nil,"str!=nil");
 strcpy( (char*)str, s );
 size = len;
 return (int)size;
}


bool gStringGeneric::thisIndex (unsigned& idx)
{
 bool isOk = idx>=1 && idx<=size;

 if ( idx<1 ) idx = 1;
 if ( idx>size ) idx = size;
 isOk = idx>=1 && idx<=size;

 return isOk;
}

bool gStringGeneric::thisIsValidIndex (int index)
{
 bool isOk( index>0 );
 unsigned idx( (unsigned)index );

 if ( isOk==false ) return false;
 return idx>=1 && idx<=size;
}

////////////////////////////////////////////////////////////
gString::gString ()
    : gStringGeneric( e_String, (t_uchar*)"\0" )
{
}


gString::gString (gString& copy)
    : gStringGeneric( e_String, (t_uchar*)(copy.Str()) )
{
 ////ASSERTION(str!=nil,"str!=nil");
}


gString::gString (const char* s)
    : gStringGeneric( e_String, (t_uchar*)s )
{
}


gString::gString (char* s)
    : gStringGeneric( e_String, (t_uchar*)s )
{
}


gString::gString (t_uchar* s)
    : gStringGeneric( e_String, s )
{
}


gString::gString (char c)
    : gStringGeneric( e_String, (t_uchar*)"\0" )
{
 Add( c );
}


gString::gString (unsigned nBytes, char c)
    : gStringGeneric( e_String, (t_uchar*)"\0" )
{
 if ( nBytes ) {
     thisCopy( "\0", nBytes );
     //for (i=0; i<nBytes; i++) str[i] = (t_uchar)c ==> too slow
     memset( str, (int)c, (size_t)nBytes );
 }
}


gString::~gString ()
{
}


bool gString::Match (gString& copy, bool doIgnoreCase)
{
 return thisMatch( Str(), copy.Str(), doIgnoreCase )==0;
}

bool gString::Match (const char* s, bool doIgnoreCase)
{
 return thisMatch( Str(), s, doIgnoreCase )==0;
}


unsigned gString::Find (gString& sSub, bool doIgnoreCase)
{
 return thisFind( Str(), sSub.Str(), 1, doIgnoreCase );
}


unsigned gString::Find (const char* s, bool doIgnoreCase)
{
 return thisFind( Str(), s, 1, doIgnoreCase );
}


unsigned gString::Find (char c, bool doIgnoreCase)
{
 char s[2];
 s[0] = c;
 s[1] = 0;
 if ( c==0 ) return 0;
 return thisFind( Str(), s, 1, doIgnoreCase );
}


unsigned gString::Find (gString& sSub, unsigned& nOcc, bool doIgnoreCase)
{
 return thisFindFwd( Str(), sSub.Str(), 1, doIgnoreCase, nOcc );
}


unsigned gString::Find (const char* s, unsigned& nOcc, bool doIgnoreCase)
{
 return thisFindFwd( Str(), s, 1, doIgnoreCase, nOcc );
}


unsigned gString::Find (char c, unsigned& nOcc, bool doIgnoreCase)
{
 char s[2];
 if ( c==0 ) return 0;
 s[0] = c;
 s[1] = 0;
 return thisFindFwd( Str(), s, 1, doIgnoreCase, nOcc );
}


unsigned gString::FindBack (gString& sSub, bool doIgnoreCase)
{
 unsigned nOcc;
 return thisFindBack( Str(), sSub.Str(), 1, doIgnoreCase, nOcc );
}


unsigned gString::FindBack (const char* s, bool doIgnoreCase)
{
 unsigned nOcc;
 return thisFindBack( Str(), s, 1, doIgnoreCase, nOcc );
}


unsigned gString::FindBack (char c, bool doIgnoreCase)
{
 unsigned nOcc;
 char s[2];
 if ( c==0 ) return 0;
 s[0] = c;
 s[1] = 0;
 return thisFindBack( Str(), s, 1, doIgnoreCase, nOcc );
}


unsigned gString::FindAnyChr (gString& b, bool doIgnoreCase)
{
 unsigned posAny;
 return thisFindAny( (char*)str, b.Str(), doIgnoreCase, posAny );
}


unsigned gString::FindAnyChr (const char* s, bool doIgnoreCase)
{
 unsigned posAny;
 return thisFindAny( (char*)str, s, doIgnoreCase, posAny );
}


unsigned gString::FindAnyChr (gString& b, bool doIgnoreCase, unsigned& posAny)
{
 return thisFindAny( (char*)str, b.Str(), doIgnoreCase, posAny );
}


unsigned gString::FindAnyChr (const char* s, bool doIgnoreCase, unsigned& posAny)
{
 return thisFindAny( (char*)str, s, doIgnoreCase, posAny );
}


unsigned gString::FindInStr (const char* aStr, const char* subStr, int offset)
{
 const char* ptr;
 if ( aStr==nil || subStr==nil ) return 0;
 ptr = strstr( aStr+offset, subStr );
 if ( ptr==nil ) return 0;
 return aStr+offset - ptr + 1;
}


unsigned gString::FindExcept (gString& sExcept, bool doIgnoreCase)
{
 // Finds any character except those in string 'sExcept'

 gString sTemp( sExcept );
 if ( doIgnoreCase ) sTemp.UpString();
 return thisFindExcept( Str(), sTemp.Str(), 1, doIgnoreCase );
}


unsigned gString::FindExcept (const char* s, bool doIgnoreCase)
{
 gString sTemp( s );
 if ( doIgnoreCase ) sTemp.UpString();
 return thisFindExcept( Str(), sTemp.Str(), 1, doIgnoreCase );
}


int gString::ConvertToUInt32 (const char* s,
			      t_uint32& vRes)
{
 // Return 0 on success
 unsigned posErr;
 return
     ConvertToUInt32( s,
		      10,
		      e_DigConvAny,
		      vRes,
		      posErr );
}


int gString::ConvertToInt32 (const char* s,
			     t_int32& vRes)
{
 // Return 0 on success
 unsigned posErr;
 return
     ConvertToInt32( s,
		     10,
		     e_DigConvAny,
		     vRes,
		     posErr );
}


int gString::ConvertToUInt32 (const char* s,
			      unsigned base,
			      eDigitConv caseSense,
			      t_uint32& vRes,
			      unsigned& posErr)
{
 // Return 0 on success
 unsigned i, len, kU, kD;
 t_uchar chr;
 static t_uchar convTblUp[17]="0123456789ABCDEF";
 static t_uchar convTblDn[17]="0123456789abcdef";
 t_uint64 uRes=0;
 bool doRelax( IsDigConvRelaxed() );

 posErr = 0;
 if ( s==nil || base>16 ) return -1;
 gString sVal( s );
 sVal.Trim();
 len = sVal.Length();
 if ( len==0 ) {
     if ( caseSense!=e_DigConvAnyEmpty0 ) return -1;
     vRes = 0;
     return 0;
 }
 uRes = 0;
 for (i=1; i<=len; i++) {
     chr = sVal[i];
     posErr = i;
     for (kU=0; kU<base; kU++)
	 if ( chr==convTblUp[kU] ) break;
     for (kD=0; kD<base; kD++)
	 if ( chr==convTblDn[kD] ) break;

     switch ( caseSense ) {
     case e_DigConvLower:
	 if ( kD>=base ) {
	     if ( doRelax ) return ReturnAndAssignUInt32( -1, uRes, vRes );
	     return -1;
	 }
	 uRes *= base;
	 uRes += (t_uint32)kD;
	 break;
     case e_DigConvUpper:
	 if ( kU>=base ) {
	     if ( doRelax ) return ReturnAndAssignUInt32( -1, uRes, vRes );
	     return -1;
	 }
	 uRes *= base;
	 uRes += (t_uint32)kU;
	 break;
     case e_DigConvAny:
     case e_DigConvAnyEmpty0:
	 if ( kU>=base && kD>=base ) {
	     if ( doRelax ) return ReturnAndAssignUInt32( -1, uRes, vRes );
	     return -1;
	 }
	 uRes *= base;
	 if ( kU<base )
	     uRes += (t_uint32)kU;
	 else
	     uRes += (t_uint32)kD;
	 break;
     default:
	 ASSERTION_FALSE("ConvertToUInt32(1)");
	 break;
     }
 }
 posErr = 0;
 ReturnAndAssignUInt32( 0, uRes, vRes );
 return 0;
}


int gString::ConvertToInt32 (const char* s,
			     unsigned base,
			     eDigitConv caseSense,
			     t_int32& vRes,
			     unsigned& posErr)
{
 // Return 0 on success
 int error;
 t_uint32 vAux = 0;
 t_int32 mySign;
 gString sTrim;
 bool isSigned;

 if ( s==nil ) return -1;
 sTrim.Set( s );
 if ( caseSense==e_DigConvAny ) sTrim.Trim();
 char* str( sTrim.Str() );
 isSigned = str[0]=='-';
 mySign = isSigned ? -1 : 1;
 error = ConvertToUInt32( str+isSigned, base, caseSense, vAux, posErr );
 if ( error!=0 ) return error;
 // Check ranges
 if ( (long long)vAux >= MAX_LONG_L ) return 1;

 // All seem o.k., set the value for output: 'vRes'
 vRes = (t_int32)vAux * mySign;

 return 0;
}


int gString::ConvertBinToValue (const char* strBinValue, t_uint32& result)
{
 char chr;
 int idx( 0 );
 t_uint32 counter( 0 );

 // Returns 0 on success (even if input string is empty);
 // on overflow, or nil input string, it returns -1.
 // Otherwise returns the position 1..length the non-0-non-1 digit was found.
 result = 0;
 if ( strBinValue==nil ) return -1;

 for ( ; (chr = strBinValue[ idx ])!=0; ) {
     idx++;
     if ( idx>=32 ) return -1;
     counter <<= 1;
     switch ( chr ) {
     case '0':
	 break;
     case '1':
	 counter++;
	 break;
     default:
	 return idx;
     }
 }
 result = counter;
 return 0;
}


void gString::ConvertBinToStr (t_uint32 v,
			       t_int16 places,  // <=0 for no justify
			       gString& sRes)
{
 unsigned i, n;
 char c;
 gString sReverse;

 sRes.SetEmpty();

 for (n=0; v>0; n++) {
     c = (v & 1) ? '1' : '0';
     sReverse.Add( c );
     v >>= 1;
 }
 if ( n==0 ) {
     sReverse.Set( "0" );
     n++;
 }

 for (i=n; (int)i<places; i++) {
     sRes.Add( "0" );
 }
 for (i=n; i>0; i--) {
     sRes.Add( sReverse[i] );
 }
}


int gString::ReturnAndAssignUInt32 (int returnValue, t_uint64 value, t_uint32& vRes)
{
 vRes = 0;
 if ( value > MAX_U4B_ULL ) return -2;
 vRes = value;
 return returnValue;
}


unsigned gString::AddString (gString& a)
{
 if ( a.IsEmpty() ) return 0;
 return Add( a.Str() );
}


void gString::Copy (gString& copy)
{
 Set( copy.Str() );
}


gString& gString::CopyFromTo (gString& copy, unsigned startPos, unsigned endPos)
{
 unsigned i, k( 0 ), len( copy.Length() );

 SetEmpty();
 if ( startPos==0 ) startPos = 1;
 if ( endPos==0 ) endPos = len;
 if ( endPos>len ) endPos = len;
 if ( startPos>endPos ) return *this;

 thisCopy( "\0", 1+endPos-startPos );

 for (i=startPos; i<=endPos; i++) {
     str[ k++ ] = copy[ i ];
 }
 str[ k ] = 0;
 size = k;
 return *this;
}


unsigned gString::Delete (unsigned startPos, unsigned endPos)
{
 unsigned
     oldSize( size ),
     i, k( 0 ),
     p0( startPos==0 ? 1 : startPos ),
     p1( endPos==0 ? size : (gMIN(endPos,size)) );

 if ( startPos==0 && endPos==0 ) {
     SetEmpty();
     return oldSize;
 }

 if ( p0>size ) return 0;

 gString sTemp;
 for (i=1; i<p0; i++, k++) sTemp.Add( (char)str[ k ] );
 for (i=p1+1, k=p1; i<=size; i++, k++) sTemp.Add( (char)str[ k ] );

 Copy( sTemp );
 ASSERTION(oldSize>=size,"oldSize>=size");
 oldSize -= size;

 return oldSize;
}


unsigned gString::Insert (gString& copy, unsigned startPos)
{
 unsigned p0( startPos==0 ? 1 : startPos );
 gString sTemp;
 gString sCopy( str );

 if ( p0>1 ) {
     sTemp = CopyFromTo( sCopy, 1, p0-1 );
 }
 sTemp.AddString( copy );
 sTemp += CopyFromTo( sCopy, p0 );
 *this = sTemp;
 return 0;
}


bool gString::Trim ()
{
 TrimLeft();
 TrimRight();
 return true;
}


bool gString::TrimLeft ()
{
 char* s( (char*)str );
 unsigned i( 0 );
 t_uchar chr;
 gString tempS;

 if ( str==nil ) return false;
 for ( ; (chr = s[ i ])!=0; i++) {
     if ( chr!=' ' && chr!='\t' ) {
	 break;
     }
 }
 if ( chr==0 ) return false;  // Nothing to trim
 tempS.Set( str + i );
 Copy( tempS );
 return true;
}


bool gString::TrimRight ()
{
 char* s( (char*)str );
 int n( (int)thisEvalLength() ), k( n ), i( n-1 );

 if ( str==nil ) return false;
 for ( ; i>=0; i--) {
     if ( s[ i ]!=' ' && s[ i ]!='\t' ) {
	 break;
     }
     s[ i ] = 0;
     k--;
 }
 gString tempS( s );
 Copy( tempS );
 return k<n;
}


unsigned gString::ConvertChrTo (char fromChr, char toChr)
{
 unsigned idx( 0 ), result( 0 );
 char aChr;
 if ( str==nil || fromChr==0 ) return 0;
 for ( ; (aChr = str[ idx ])!=0; idx++) {
     if ( aChr==fromChr ) {
	 str[ idx ] = toChr;
	 if ( result==0 ) {
	     result = idx+1;
	 }
     }
 }
 return result;
}


unsigned gString::ConvertAnyChrTo (const char* aStr, char toChr)
{
 unsigned result( 0 );
 char aChr;
 if ( aStr==nil ) return 0;
 for ( ; (aChr = *aStr)!=0; aStr++) {
     result += (ConvertChrTo( aChr, toChr ) != 0);
 }
 return result;
}


unsigned gString::ConvertAnyChrsTo (gString& s, char toChr)
{
 return ConvertAnyChrTo( s.Str(), toChr );
}


int gString::SplitAnyChar (char anyChr, gList& resultL)
{
 static char bufTmp[ 2 ];
 bufTmp[ 0 ] = anyChr;
 return SplitAnyChar( bufTmp, resultL );
}


int gString::SplitAnyChar (const char* anyChrStr, gList& resultL)
{
 // Returns the numbers of splits, or -1 on error
 if ( anyChrStr==nil ) return -1;
 gString sAnyChrStr( anyChrStr );

 return
     thisSplitAnyChar( sAnyChrStr,
		       e_BasicDoCaseSense,
		       false,  // do not stop on first match
		       false,  // do not include the splitting char
		       resultL );
}



char* gString::BaseName (const char* strOptSuffix)
{
 unsigned pos, len( Length() );
 gString* pNewString( new gString( str ) );

 if ( pNewString==nil ) return nil;

 pos = FindBack( gSLASHCHR );
 if ( pos==len ) {
     pNewString->Delete( pos, pos );
     pos = pNewString->FindBack( gSLASHCHR );
 }
 if ( pos ) {
     pNewString->Delete( 1, pos );
 }
 if ( strOptSuffix!=nil && strOptSuffix[0]!=0 ) {
     pos = pNewString->FindBack( strOptSuffix );
     if ( pos ) {
	 pNewString->Delete( pos );
     }
 }

 gStorageControl::Self().Pool().AppendObject( pNewString );
 return pNewString->Str();
}


char* gString::DirName ()
{
 unsigned pos;
 char* strBase( BaseName( nil ) );
 gString* pNewString;

 if ( strBase==nil ) return nil;

 pos = FindBack( strBase );

 if ( pos==0 ) {
     pNewString = new gString( "." );
 }
 else {
     pNewString = new gString( str );
     pNewString->Delete( pos );
     //if ( pNewString->IsEmpty() ) pNewString->Add( gSLASHCHR );
 }

 gStorageControl::Self().Pool().AppendObject( pNewString );
 return pNewString->Str();
}


gString& gString::operator= (gString& copy)
{
 Copy( copy );
 return *this;
}


gString& gString::operator= (char* s)
{
 Set( s );
 return *this;
}


gString& gString::operator= (char c)
{
 char s[2];
 s[0] = c;
 s[1] = 0;
 Set( s );
 return *this;
}


gString& gString::operator= (int v)
{
 char s[40];
 snprintf(s,40,"%d",v);
 Set( s );
 return *this;
}


gString& gString::operator+ (gString& copy)
{
 char* s( copy.Str() );
 Add( s );
 return *this;
}


gString& gString::operator+= (gString& copy)
{
 AddString( copy );
 return *this;
}


gStorage* gString::NewObject ()
{
 gString* a( new gString( str ) );
 return a;
}


t_uchar* gString::ToString (const t_uchar* uBuf)
{
 return gStringGeneric::ToString( uBuf );
}


bool gString::SaveGuts (FILE* f)
{
 return gStringGeneric::SaveGuts( f );
}


bool gString::RestoreGuts (FILE* f)
{
 return gStringGeneric::RestoreGuts( f );
}


void gString::Show (bool doShowAll)
{
 iprint("%s%s%s",
	doShowAll?"\"":"\0",
	str==NULL?"\0":(char*)str,
	doShowAll?"\"":"\0");
}

////////////////////////////////////////////////////////////
// gString Protected methods
////////////////////////////////////////////////////////////
int gString::thisMatch (const char* s1, const char* s2, bool doIgnoreCase)
{
 if ( s1==nil || s2==nil ) return -2;
 if ( doIgnoreCase==false ) {
     return strcmp(s1,s2);
 }
 gString uStr1( s1 );
 gString uStr2( s2 );
 uStr1.UpString();
 uStr2.UpString();
 return strcmp( uStr1.Str(), uStr2.Str() );
}


unsigned gString::thisFind (const char* s,
			    const char* sub,
			    unsigned startPos,
			    bool doIgnoreCase)
{
 const char* ptr;

 if ( s==nil || sub==nil ) return 0;
 ASSERTION(startPos>=1,"startPos>=1");

 ptr = strstr( s+startPos-1, sub );

 if ( doIgnoreCase==false ) {
     if ( ptr!=0 ) return ptr-s+startPos;
     return 0;
 }

 gString uStr( s );
 s = uStr.Str();  //Important for using ptr-s+startPos formula!
 gString uSub( sub );
 uStr.UpString();
 uSub.UpString();

 ptr = strstr( s, uSub.Str() );
 if ( ptr!=0 ) return ptr-s+startPos;
 return 0;
}


unsigned gString::thisFindAny (const char* s,
			       const char* strAny,
			       bool doIgnoreCase,
			       unsigned& posAny)
{
 t_uchar uChr, fChr;
 unsigned i, k, uLen, aLen;
 char* uStr;
 char* aStr;

 posAny = 0;
 if ( s==nil || strAny==nil ) return 0;
 gString sAll( s );
 gString sAny( strAny );
 if ( doIgnoreCase ) {
     sAll.UpString();
     sAny.UpString();
 }
 uLen = sAll.Length();
 uStr = sAll.Str();
 aLen = sAny.Length();
 aStr = sAny.Str();
 for (k=0; k<uLen; k++) {
     uChr = (t_uchar)uStr[k];
     for (i=0; i<aLen; ) {
	 fChr = (t_uchar)aStr[i];
	 i++;
	 if ( uChr==fChr ) {
	     posAny = i;
	     return k+1;
	 }
     }
 }
 return 0;
}


unsigned gString::thisFindFwd (const char* s,
			       const char* sub,
			       unsigned startPos,
			       bool doIgnoreCase,
			       unsigned& nOcc)
{
 if ( s==nil || sub==nil ) return 0;
 ASSERTION(startPos==1,"startPos==1");

 gString uStr( s );
 gString uSub( sub );
 if ( doIgnoreCase ) {
     uStr.UpString();
     uSub.UpString();
 }
 return thisFindFwdOcc( uStr.Str(), uSub.Str(), nOcc );
}


unsigned gString::thisFindFwdOcc (const char* s,
				  const char* sub,
				  unsigned& nOcc)
{
 unsigned i, k;
 unsigned posFirst=0;
 unsigned pos=0, aPos, endPos=Length();

 nOcc = 0;
 for (i=1, k=0; i<=endPos; i++) {
     aPos = thisFind( s+k, sub, 1, false );
     if ( aPos>0 ) {
	 pos = k + aPos;
	 if ( posFirst==0 ) posFirst = pos;
         nOcc++;
         k = pos;
         i = k+1;
     }
 }
 return posFirst;
}


unsigned gString::thisFindBack (const char* s,
				const char* sub,
				unsigned startPos,
				bool doIgnoreCase,
				unsigned& nOcc)
{
 if ( s==nil || sub==nil ) return 0;
 ASSERTION(startPos==1,"startPos==1");

 gString uStr( s );
 gString uSub( sub );
 if ( doIgnoreCase ) {
     uStr.UpString();
     uSub.UpString();
 }
 return thisFindBackOcc( uStr.Str(), uSub.Str(), nOcc );
}


unsigned gString::thisFindBackOcc (const char* s,
				   const char* sub,
				   unsigned& nOcc)
{
 unsigned i, k;
 unsigned pos=0, aPos, endPos=Length();

 nOcc = 0;
 for (i=1, k=0; i<=endPos; i++) {
     aPos = thisFind( s+k, sub, 1, false );
     if ( aPos==0 ) return pos;
     pos = k + aPos;
     nOcc++;
     k = pos;
     i = k;
 }
 return pos;
}


unsigned gString::thisFindExcept (const char* s,
				  const char* exceptStr,
				  unsigned startPos,
				  bool doIgnoreCase)
{
 // Finds any character except those in 'exceptStr'
 // Returns 0 if only characters in 'exceptStr' were found
 bool didFound;
 unsigned i=0, k, excLen=strlen(exceptStr);
 char c;

 gString sUp( s );
 if ( doIgnoreCase ) sUp.UpString();
 char* sAll = sUp.Str();

 ASSERTION(startPos==1,"startPos==1");

 for (i=startPos-1; (c = sAll[i])!=0; ) {
     didFound = false;
     for (k=0; k<excLen && didFound==false; k++) {
	 didFound = c==exceptStr[k];
     }
     i++;
     if ( didFound==false ) return i;
 }
 return 0;
}

int gString::thisSplitAnyChar (gString& sSepStr,
			       eBasicCaseSense senseKind,
			       bool doStopOnFirst,
			       bool doIncludeChar,
			       gList& resultL)
{
 // Return the number of elements added to 'resultL'.

 // Example: ("henrique-campos@moreira","-@",...,false,false,L)
 // The output is: L={'henrique','campos','moreira'}

 unsigned iter( 0 ), n( size );
 t_uchar uChr;
 gString sTemp;
 int count( 0 );

 for ( ; iter<n; iter++) {
     // The algorithm is linear: iterating through the input string,
     // checking whether '@' is found (in the example above),
     // then just adding to the result list.
     uChr = str[ iter ];
     if ( sSepStr.Find( uChr ) ) {
	 count++;
	 if ( sTemp.IsEmpty()==false ) resultL.Add( sTemp );
	 sTemp.SetEmpty();
	 if ( doIncludeChar ) {
	     gString sChr( uChr );
	     resultL.Add( sChr );
	 }
	 if ( doStopOnFirst ) return count;
     }
     else {
	 sTemp.Add( uChr );
     }
 }// end FOR

 // Just adding the remaining buffer, if any.
 if ( sTemp.IsEmpty() ) return count;
 resultL.Add( sTemp );

 return ++count;
}

////////////////////////////////////////////////////////////

