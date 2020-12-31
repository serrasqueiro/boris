// ipopauth.cpp

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <string.h>

#include "ipopauth.h"

#include "iconfig.h"
////////////////////////////////////////////////////////////
// Static members
// ---------------------------------------------------------
// <>
////////////////////////////////////////////////////////////
iPopUserPass::iPopUserPass (const char* strUser, const char* strPass)
    : gString( strUser ),
      pass( strPass ),
      passStatus( e_PassNone )
{
}


iPopUserPass::~iPopUserPass ()
{
}


void iPopUserPass::Reset ()
{
 SetEmpty();
 pass.SetEmpty();
}


char* iPopUserPass::PassBase65 ()
{
 gString64 base65( pass );

 // Re-enforce usage of Base65:
 base65.UseNow( gStringBase::e_Base65 );

 encodes.Set( base65.Encode64() );
 return encodes.Str();
}


int iPopUserPass::FromBase65 (const char* aStr, gString& sResult)
{
 // Returns 0 if 'aStr' can be converted back from Base65 to bin,
 // or 1 if cannot be decoded. -1 is aStr is nil.
 // 2 returned signals an empty Base65.

 gString64 base65( (char*)aStr );
 t_uchar* strPass;

 DBGPRINT("DBG: FromBase65(%s,sResult) <-start\n",aStr);
 passStatus = e_PassNone;
 if ( aStr==nil ) {
     sResult.SetEmpty();
     return -1;
 }

 // Re-enforce usage of Base65:
 base65.UseNow( gStringBase::e_Base65 );

 strPass = base65.Decode64();
 sResult.Set( strPass );
 if ( sResult.IsEmpty() ) {
     if ( base65.convertCode==0 )	// passStatus is e_PassNone!
	 return 2;
     passStatus = e_PassUndecoded;
     return 1;
 }
 passStatus = e_PassOk;
 return 0;
}


int iPopUserPass::thisSetStrLimited (const char* aStr,
				     int minSize,
				     int maxSize,
				     t_uint16 mask,
				     gString& sResult)
{
 bool blanksAllowed( (mask & 1)!=0 );
 bool limitedUserChars( (mask & 2)!=0 );
 bool doIgnoreNL( (mask & 4)!=0 );  // Chars after NL are discarded
 char chr;
 int countChars( 0 );
 int error( 0 );
 gString sCopy( aStr );
 char* ptrStr( sCopy.Str() );

 sResult.Reset();
 if ( aStr==nil ) return 1;
 if ( maxSize<0 ) return -1;

 // Note: input 'aStr' is changed!

 for ( ; (chr = *ptrStr)!=0; countChars++) {
     if ( chr=='\n' ) {
	 if ( doIgnoreNL ) {
	     *ptrStr = 0;
	     break;
	 }
     }
     else {
	 if ( chr==' ' ) {
	     if ( blanksAllowed==false ) return 4;
	 }
	 else {
	     if ( limitedUserChars ) {
		 if ( chr<' ' )
		     error |= 2;
	     }
	 }
     }
     if ( countChars>=maxSize ) {
	 *ptrStr = 0;
	 error = 8;
	 break;
     }
     ptrStr++;
 }
 DBGPRINT_MIN("thisSetStrLimited(%s,%d,%d,mask=%u,...), error=%d\n",
	      aStr,
	      minSize,
	      maxSize,
	      mask,
	      error);
 if ( countChars<minSize ) return 5;
 sResult = sCopy;
 return error;
}
////////////////////////////////////////////////////////////
iCredential::iCredential (const char* strUser, const char* strPass)
    : iPopUserPass( strUser, strPass ),
      credStatus( e_NotKnown )
{
 if ( strUser && strPass && strUser[0] && strPass[0] ) {
     credStatus = e_Valid;
 }
}


iCredential::~iCredential ()
{
}


char* iCredential::FindKeyword (const char* strKeyc, gList& list)
{
 // Finds the keyword, e.g. "pop_user=" within list

 unsigned pos( list.FindFirst( strKeyc, 1, e_FindExactPosition ) );
 int len( strlen( strKeyc ) );

 DBGPRINT("DBG: FindKeyword(%s,list), N=%u: pos=%u\n",
	  strKeyc,
	  list.N(),
	  pos);

 if ( pos==0 ) return nil;
 return list.CurrentPtr()->Str() + len;
}


int iCredential::CreateFile (const char* filename)
{
 mode_t defaultMode( S_IWUSR | S_IRUSR );
 mode_t appliedMode( defaultMode );
 int flags( O_CREAT | O_TRUNC | O_WRONLY );

 if ( filename==nil || filename[0]==0 ) return -1;
 return open( filename, flags, appliedMode );  // Returns the handle
}


int iCredential::FromFile (const char* strFile)
{
 int ioOp;
 char* strPassBase65;

 Reset();
 ioOp = thisReadCredentialsFromFile( strFile, contents );
 if ( ioOp!=0 && ioOp!=3 ) {
     credStatus = e_NotKnown;
     return ioOp;
 }

 Set( FindKeyword( "pop_user=", contents ) );
 strPassBase65 = FindKeyword( "pop_pass=", contents );
 gString sTrim( strPassBase65 );
 sTrim.Trim();
 FromBase65( sTrim.Str(), pass );

 if ( IsEmpty() || pass.IsEmpty() ) {
     credStatus = e_Invalid;
     return 1;
 }

 credStatus = e_Valid;
 // ioOp is 3 if permissions are too wide, or 0 if OK:
 return ioOp;
}


int iCredential::ToFile (const char* strFile, bool reuse)
{
 int ioOp;
 FILE* fOut;
 unsigned iter( 1 ), nLines;
 bool wroteUser( false );
 bool wrotePass( false );
 short countIter;

 if ( strFile==nil || strFile[0]==0 ) return -1;

 contents.Reset();
 if ( reuse ) {
     ioOp = thisReadCredentialsFromFile( strFile, contents );
     if ( ioOp==3 ) {
	 // File needs to be re-created, it has wide permissions to group/others!
	 remove( strFile );
     }
 }
 ioOp = CreateFile( strFile );
 if ( ioOp==-1 ) return -1;

 close( ioOp );
 fOut = fopen( strFile, "wt" );
 if ( fOut==nil ) return -1;

 for (nLines=contents.N(); iter<=nLines; iter++) {
     gString line( contents[ iter ] );
     countIter = 0;
     if ( line.Find( "pop_user=" )==1 ) {
	 if ( wroteUser ) {
	     fprintf(fOut,"# [DUPLICATE]: %s\n",line.Str());
	 }
	 else {
	     fprintf(fOut,"pop_user=%s\n",Str());
	 }
	 wroteUser = true;
	 countIter++;
     }
     if ( line.Find( "pop_pass=" )==1 ) {
	 if ( wrotePass ) {
	     fprintf(fOut,"# [DUPLICATE]: %s\n",line.Str());
	 }
	 else {
	     fprintf(fOut,"pop_pass=%s\n",PassBase65());
	 }
	 wrotePass = true;
	 countIter++;
     }
     if ( countIter==0 ) {
	 fprintf(fOut,"%s\n",line.Str());
     }
 }

 if ( wroteUser==false ) {
     fprintf(fOut,"pop_user=%s\n",Str());
 }
 if ( wrotePass==false ) {
     fprintf(fOut,"pop_pass=%s\n",PassBase65());
 }

 fclose( fOut );
 return 0;
}


int iCredential::thisReadCredentialsFromFile (const char* strFile, gList& outList)
{
 // Returns 2 if file is not readable, or 3 if it is too wide open (group/others),
 // or 0 if OK.

 int result( 2 );
 t_uint32 mode;
 bool isFromStdin( strFile==nil || strFile[0]==0 );
 gFileStat statFile( strFile );

 outList.Reset();

 if ( isFromStdin || statFile.IsOk() ) {
     gFileFetch input( strFile );
     outList.CopyList( input.aL );

     mode = statFile.status.mode & 00777;
     result = ((mode & 0077)!=0)*3;  // Others, or group, can read, etc.!
 }
 DBGPRINT("DBG: thisReadCredentialsFromFile(%s,outList), N=%u, returns %d\n",
	  strFile,
	  outList.N(),
	  result);
 return result;
}
////////////////////////////////////////////////////////////
int ipcl_strcmp (const char* str1, const char* str2)
{
 if ( str1 ) {
     if ( str2 ) return strcmp( str1, str2 );
     return str1[ 0 ]!=0;
 }
 if ( str2 )
     return str2[ 0 ] ? -1 : 0;
 return 0;  // Both str1 and str2 are nil
}


int ipcl_strcmp_case (const char* str1, const char* str2)
{
 if ( str1 ) {
     if ( str2 ) return strcasecmp( str1, str2 );
     return str1[ 0 ]!=0;
 }
 if ( str2 )
     return str2[ 0 ] ? -1 : 0;
 return 0;  // Both str1 and str2 are nil
}


int ipcl_strncmp (const char* str1, const char* str2, size_t nChars)
{
 if ( str1 ) {
     if ( str2 ) return strncmp( str1, str2, nChars );
     return str1[ 0 ]!=0;
 }
 if ( str2 )
     return str2[ 0 ] ? -1 : 0;
 return 0;  // Both str1 and str2 are nil
}


int ipcl_strncmp_case (const char* str1, const char* str2, size_t nChars)
{
 if ( str1 ) {
     if ( str2 ) return strncasecmp( str1, str2, nChars );
     return str1[ 0 ]!=0;
 }
 if ( str2 )
     return str2[ 0 ] ? -1 : 0;
 return 0;  // Both str1 and str2 are nil
}


int ipcl_str_to_int (const char* strValue, int minValue, int maxValue, int errorValue)
{
 t_int32 vRes( 0 );
 int error( gStorageControl::Self().ConvertToInt32( (char*)strValue, vRes ) );
 if ( error )
     return errorValue;
 if ( vRes > (t_int32)maxValue || vRes < (t_int32)minValue )
     return errorValue;
 return (int)vRes;
}


int ipcl_ip_from_hostaddr (const char* strIpOrHost, gIpAddr& ip)
{
 // Returns 1 on error
 if ( strIpOrHost==nil || strIpOrHost[ 0 ]==0 ) return -1;
 ip.GetHostByName( (char*)strIpOrHost );
 return ip.GetHostAddress()==0;
}


int ipcl_host_from_ipaddr (gIpAddr& ip, gString& sHost)
{
 int error;
 if ( ip.IsOk() && ip.GetHostAddress() ) {
     ip.GetHostByAddr( sHost );
     error = (int)sHost.IsEmpty();
     return error;
 }
 else {
     sHost.Delete();
 }
 return 1;
}
////////////////////////////////////////////////////////////

