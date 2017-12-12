// sUrlNames.cpp


#include <stdio.h>
#include <unistd.h>

#include "sUrlNames.h"

////////////////////////////////////////////////////////////
static int ssun_url_sane (int maxASCII, t_uchar* url)
{
 int idx( 0 ), to( 0 ), keep;
 int warns( 0 );
 t_uchar uChr, upChr( maxASCII < 0 ? 126 : maxASCII );

 // Sanitizes url

 for ( ; (uChr = url[ idx ])!=0; idx++) {
     if ( uChr > upChr ) {
	 uChr = '?';
	 warns++;
     }
     if ( uChr < ' ' ) {
	 uChr = '?';
	 warns++;
     }
     url[ idx ] = uChr;
 }
 keep = idx;

 for (idx=0; (uChr = url[ idx ])!=0; idx++) {
     if ( uChr==':' ) {
	 warns++;
	 url[ idx ] = '\x1';
     }
     else {
	 break;
     }
 }

 // copy-left all chars:
 for ( ; idx; ) {
     url[ to++ ] = url[ idx++ ];
     if ( to>=keep ) {
	 url[ to ] = 0;
	 break;
     }
 }

 return warns;
}


static bool ssun_url_hex (const char aChr)
{
 switch ( aChr ) {
 case '%':
 case '&':
 case ':':
 case '"':
 case '\'':
 case '(': case ')':
 case '{': case '}':
 case '[': case ']':
     return true;
 default:
     break;
 }
 return false;
}


static int ssun_url_name_chr_subst (char* ptrChr)
{
 switch ( (unsigned char)*ptrChr ) {
 case 0xB4:  // quote (0x00B4), apostrophe (')
     *ptrChr = '\'';
     return (int)'\'';

 default:
     break;
 }
 return 0;
}


int ssun_length (const char* aStr)
{
 if ( aStr ) {
     return strlen( aStr );
 }
 return 0;
}


char* ssun_url_builder_name (const char* aStr, int mask)
{
 char* newStr( ssun_url_builder_name_len( aStr, ssun_length(aStr), mask ) );
 ASSERTION(newStr,"ssun_url_builder_name (1)");
 return newStr;
}


char* ssun_url_builder_name_len (const char* aStr, int len, int mask)
{
 static char hexStr[ 8 ];
 static int iter;
 char aChr;
 char lastInvalid( '\0' ), lastBlank( '\0' );
 char* newStr;

 if ( aStr==nil ) return nil;

 newStr = (char*)calloc( len*4+1, sizeof(char) );
 if ( newStr==nil ) return nil;
 ASSERTION(newStr[ 0 ]==0,"?");

 switch ( mask ) {
 case -1:
 default:
     for (iter=0; iter<len; iter++) {
	 aChr = aStr[ iter ];
	 ssun_url_name_chr_subst( &aChr );

	 switch ( aChr ) {
	 case '\r':
	 case '\n':
	     hexStr[ 0 ] = aChr;
	     hexStr[ 1 ] = 0;
	     strcat( newStr, hexStr );
	     lastInvalid = lastBlank = 0;
	     break;

	 case '?':
	 case '+':
	 case '=':
	     lastInvalid = aChr;
	     continue;

	 case '_':
	 case ' ':
	     lastInvalid = 0;
	     aChr = '+';
	     // No break here!

	 default:
	     if ( ssun_url_hex( aChr ) || aChr<=0 ) {
		 sprintf(hexStr,"%%%02X",(unsigned)(unsigned char)aChr);
		 strcat( newStr, hexStr );
		 lastBlank = 0;
	     }
	     else {
		 hexStr[ 0 ] = aChr;
		 hexStr[ 1 ] = 0;
		 if ( lastInvalid ) {
		     if ( newStr[ 0 ]!=0 ) {
			 strcat( newStr, "+" );
		     }
		     lastInvalid = 0;
		 }

		 if ( aChr=='+' ) {
		     if ( lastBlank==0 ) {
			 strcat( newStr, hexStr );
			 lastBlank = '+';
		     }
		 }
		 else {
		     lastBlank = 0;
		     strcat( newStr, hexStr );
		 }
	     }
	     break;
	 }
     }
     break;
 }

 return newStr;
}


int char_is_alpha (t_uchar uChr)
{
 t_uchar altChr( '\0' );
 return char_is_alpha_eq( uChr, 0, altChr );
}


int char_is_alpha_eq (t_uchar uChr, int up, t_uchar& altChr)
{
 if ( uChr>='a' && uChr<='z' ) {
     altChr = uChr - (32 * (up==1));
     return 1;
 }
 if ( uChr>='A' && uChr<='Z' ) {
     altChr = uChr + (32 * (up==-1));
     return 1;
 }
 altChr = 0;
 return 0;
}


gList* urlx_checked (gList* ptrURLX, bool checkURL)
{
 // #warning TODO: checkURL, may add additional info

 ASSERTION(ptrURLX,"ptrURLX");
 DBGPRINT("DBG: urlx_checked (%c) #%u\n",
	  ISyORn( checkURL ),
	  ptrURLX->N());
 DBGPRINT("\t%s\t%s:%s%s\n\n",
	  ptrURLX->StartPtr()->next->me->Str(),
	  ptrURLX->StartPtr()->Str(),
	  ptrURLX->StartPtr()->next->me->Str(),
	  ptrURLX->EndPtr()->Str( 3 ));
 return ptrURLX;
}


gList* new_urlx_from_buffer (t_uchar* buf, int lenBuf)
{
 gList* urlx;
 int status( 0 );
 int idxBuf( 0 );
 short state( 1 ), idx( 0 );
 t_uchar uChr, lastChr( '\0' );
 t_uchar altChr( '\0' );
 gString sProto;
 gString sMean;
 t_uchar uri[ 4096 ];
 const short maxUriSize( sizeof(uri) );

 // States:
 //	1.	initial state
 //	2.	':' found, but protocol string too short
 //	4.	http: found, or other protocol (state 4...36)
 //	5.	https: also ok, see above
 //	100.	slash ('/') after protocol
 //	101.	any other letter/ digit following protocol
 //	500.	URI

 memset( uri, 0x0, sizeof(uri) );

 ASSERTION(buf,"buf");

 urlx = new gList;
 ASSERTION(urlx, "urlx");

 for ( ; idxBuf<lenBuf; idxBuf++) {
     uChr = buf[ idxBuf ];
     if ( uChr==0 || uChr=='\r' || uChr=='\n' ) {
	 lastChr = 0;
	 continue;
     }

     if ( uChr=='\t' ) {
	 uChr = ' ';
     }

     DBGPRINT_MIN("state: %d,\tnow: %c  <%s>\n",
		  state,
		  uChr < ' ' || uChr>=127 ? '?' : uChr,
		  uri);

     switch ( state ) {
     case 0:  // same as 1, means re-started...
     case 1:
	 if ( uChr==':' ) {
	     state = 2;
	     if ( sProto.Length() >= 3 ) {
		 if ( sProto.Find( "http" ) ) {
		     state = 4;
		     if ( sProto.Match( "https" ) ) {
			 state = 5;
		     }
		 }
		 else {
		     state = (((t_uint32)sProto.Hash()) % 31) + 6;
		 }
		 if ( sProto.Length() > 9 ) {
		     sProto.Delete( 9 );
		     sProto.Add( "..." );
		 }
		 urlx->Add( sProto );
		 urlx->EndPtr()->me->iValue = (int)state;
	     }
	 }
	 else {
	     char_is_alpha_eq( uChr, -1, altChr );
	     if ( altChr ) {
		 sProto.Add( altChr );
	     }
	     else {
		 sProto.SetEmpty();
	     }
	 }
	 lastChr = uChr;
	 break;

     case 2:
	 char_is_alpha_eq( uChr, -1, altChr );
	 if ( altChr ) {
	     state = 1;
	     sProto.Add( altChr );
	 }
	 lastChr = '\0';
	 break;

     case 100:
     case 101:
	 if ( uChr=='/' && sMean.Length() < 6 ) {
	     if ( lastChr ) {
		 sMean.Add( uChr );
		 lastChr = 0;
	     }
	 }
	 else {
	     ASSERTION(idx<=9,"idx?");
	     uri[ idx++ ] = uChr;
	     lastChr = uChr;
	     state = 500;
	 }
	 break;

     case 500:
	 if ( uChr=='/' && lastChr=='/' ) {
	 }
	 else {
	     uri[ idx++ ] = uChr;
	     if ( idx >= maxUriSize ) break;
	     lastChr = uChr;
	 }
	 break;

     case 4:
     default:
	 DBGPRINT_MIN("DBG: state: %d, sProto={%s} sMean={%s}\n",
		      state,
		      sProto.Str(),
		      sMean.Str());
	 if ( uChr=='/' ) {
	     sMean.Add( uChr );
	     state = 100;
	 }
	 else {
	     if ( uChr==' ' ) {
		 // Unexpected blank, let's restart!
		 urlx->Delete();
		 state = 0;
		 lastChr = 0;
		 sProto.SetEmpty();
	     }
	     else {
		 ASSERTION(idx<=9,"idx?");
		 uri[ idx++ ] = uChr;
		 lastChr = uChr;
		 state = 101;
	     }
	 }
	 break;
     }
 }// end FOR

 if ( sMean.Match( "/" ) ) {
     sMean.Set( "//" );
     status = 1;
 }
 if ( sMean.Match( "//" )==false ) {
    status = 2;
 }
 urlx->Add( sMean );
 urlx->EndPtr()->me->iValue = status;

 status = ssun_url_sane( -1, uri );

 if ( idx >= maxUriSize ) {
     // Buffer overflow
     urlx->iValue = -1;
     urlx->Add( "" );
 }
 else {
     urlx->Add( uri );
     urlx->EndPtr()->me->iValue = status;
 }

 DBGPRINT("DBG: state: %d, N# %u {%s}\n",
	  state,
	  urlx->N(),
	  uri);

 return urlx;
}


gElem* find_value (int value, gElem* pElem)
{
 for ( ; pElem; pElem=pElem->next) {
     if ( pElem->me->iValue==value ) return pElem;
 }
 return nil;
}


gList* new_urlx_from_file (int inputHandle, bool checkURL)
{
 int idxBuf( 0 );
 t_uchar uBuf[ 4224 ];
 t_uchar uChr;
 gList* urlx;

 if ( inputHandle==-1 ) return nil;

 for ( ; read( inputHandle, &uChr, 1 )==1; idxBuf++) {
     if ( idxBuf >= 4096 ) break;
     uBuf[ idxBuf ] = uChr;
 }
 uBuf[ idxBuf ] = 0;

 urlx = new_urlx_from_buffer( uBuf, idxBuf );

 return urlx_checked( urlx, checkURL );
}


gList* new_urlx_from_buffered_list (FILE* fOut, gString& sTempFile, gList& args, int acceptGarbage, bool checkURL)
{
 const char* strEnv( getenv( "SSED_ARGS" ) );
 const char* strNewFile( nil );
 int idx( 0 );

 gString reusedFile;
 gString* p;
 gList* urlx( nil );
 gElem* pElem( args.StartPtr() );

 if ( strEnv && strEnv[ 0 ]>' ' && strEnv[ 0 ]<='~' ) {
     strNewFile = strEnv;
 }
 if ( fOut && fOut!=stdout ) {
     reusedFile = sTempFile;
 }
 else {
     if ( strNewFile ) {
	 fOut = fopen( strNewFile, "wb" );
	 reusedFile = (char*)strNewFile;
     }
     else {
	 fprintf(stderr, "Missing env var: SSED_ARGS\n");
     }
 }

 if ( reusedFile.IsEmpty() ) return nil;

 ASSERTION(fOut,"fOut");

 for ( ; pElem; pElem=pElem->next) {
     idx++;
     p = (gString*)pElem->me;
     fprintf(fOut, "%s%s",
	     idx > 1 ? " " : "",
	     p->Str());
 }
 fprintf(fOut, "\n");
 fflush( fOut );

 FILE* fIn( fopen( reusedFile.Str(), "r" ) );
 if ( fIn ) {
     urlx = new_urlx_from_file( fileno( fIn ), checkURL );
     fclose( fIn );
 }

 return urlx;
}


gList* new_urlx_from_list (FILE* fOut, gString& sTempFile, gList& args, int acceptGarbage, bool checkURL)
{
#if 1
 return new_urlx_from_buffered_list( fOut, sTempFile, args, acceptGarbage, checkURL );
#else
#warning Do it simpler than this!
#endif
}


int gen_unescape (gList& input, gList& output, int mask)
{
 int error( 0 );
 int status( -1 );
 unsigned value;
 gElem* pElem( input.StartPtr() );
 char chr, next;
 char upChar, nextUp;

 const bool unescape7BitOnly( mask==10 );

 for ( ; pElem; pElem=pElem->next) {
     gString sOut;
     gString sArg( pElem->Str() );
     unsigned iter( 1 ), n( sArg.Length() );

     switch ( mask ) {
     case 1:  // escapes when needed
     case 2:  // escapes always
	 break;

     case 10:  // unescape valid 7bit ASCII
     default:
	 for ( ; iter<=n; iter++) {
	     chr = sArg[ iter ];
	     next = sArg[ iter+1 ];

	     if ( chr=='%' ) {
		 if ( next=='%' ) {
		     status = -1;
		     sOut.Add( chr );
		     iter++;
		 }
		 else {
		     status = 0;
		 }
	     }
	     else {
		 if ( status<0 ) {
		     sOut.Add( chr );
		 }
		 else {
		     upChar = ((chr>='a' && chr<='z') ? (chr-'0'+10-1) : chr);
		     nextUp = ((next>='a' && next<='z') ? (next-'0'+10-1) : next);
		     if ( upChar>='0' && upChar<('0'+16) && nextUp>='0' && nextUp<'0'+16 ) {
			 value = (upChar - '0') * 16 + (nextUp - '0');
			 iter++;

			 if ( unescape7BitOnly && ( (value>=' ' && value<127) || (value=='\t' || value=='\n') )==false ) {
			     error = 2;
			     break;
			 }
			 else {
			     sOut.Add( (char)value );
			 }
		     }
		     else {
			 error = 1;
			 sOut.Add( '%' );
			 sOut.Add( chr );
		     }
		     status = -1;
		 }
	     }
	 }
	 break;
     }

     output.Add( sOut );
 }//end FOR

 return error;
}

////////////////////////////////////////////////////////////
void html_dump_char (FILE* fOut, sHtmlized& htmlized, bool flushBuffer, t_uchar aChr)
{
 gString* myHint;
 bool exclude( false );
 int atPos;

 wholeLine[ lineIndex ] = 0;

 if ( fOut ) {
     if ( aChr ) {
	 lineIndex++;
	 if ( lineIndex+1 < 16 * 1024 ) {
	     wholeLine[ lineIndex-1 ] = aChr;
	     wholeLine[ lineIndex ] = 0;
	 }
     }

     if ( flushBuffer ) {
	 if ( lineIndex > 0 ) {
	     // Check whether it's excluded

	     for (gElem* hint=htmlized.fastExcluded.StartPtr(); exclude==false && hint; hint=hint->next) {
		 myHint = (gString*)hint->me;
		 atPos = myHint->iValue;
		 if ( lineIndex > (myHint->Length()) ) {
		     if ( atPos==1 && lineIndex >= (int)(myHint->Length()) ) {
			 exclude = strncmp( myHint->Str( atPos-1 ), (char*)wholeLine, myHint->Length() )==0;
		     }
		     else {
			 if ( atPos==-1 ) {
			     exclude = (strstr( (char*)wholeLine, myHint->Str() )!=nil);
			 }
		     }
		 }
	     }

	     if ( exclude ) {
		 htmlized.countExcluded++;
	     }
	     else {
		 fprintf(fOut, "%s", wholeLine);
	     }
	 }
	 wholeLine[ lineIndex = 0 ] = 0;
     }
 }
}


void html_dump_string (FILE* fOut, gString& s)
{
 DBGPRINT("fOut=0x%p, s #%u {%s}\n", fOut, s.Length(), s.Str());

 if ( fOut ) {
     char* aStr( s.Str() );

     for (unsigned iter=0, n=s.Length(); iter<n; iter++) {
	 html_dump_char( fOut, basicData, false, aStr[ iter ] );
     }
 }
}

////////////////////////////////////////////////////////////

