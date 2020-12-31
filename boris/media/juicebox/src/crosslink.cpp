// crosslink.cpp
//
//	Manages and creates crosslink multimedia volumes


#include "crosslink.h"
#include "matches.h"
#include "auxvpl.h"
#include "smart.h"

#include "lib_ilog.h"


#ifdef SHOW_APRINT
#define aprint(args...) printf(args);
#else
#define aprint(args...) ;
#endif

#define jclShorBuf	((char*)globalJclShortBuf)
#define jclShorBufSize	(sizeof( globalJclShortBuf ))

// Because on some jcl_read...() logs are disabled:
#define SP_LOG(level,args...) MM_LOG( level, args ); fprintf(stderr,args); fprintf(stderr,"\n");


extern gSLog mylog;
extern t_uchar globalJclShortBuf[ 1024 ];


int jcl_basic_proposed_alt_name (gString sIn, int numbersGenCode, gList* ptrItem, gString& sResult) ;

int jcl_conf_to_substed (const char* strConfFile, sCrossLinkConf& linkConf, sSubstedPaths& substed) ;

////////////////////////////////////////////////////////////
int sSubstedPaths::FindNonArtistDir (const char* strFullPath, int& length)
{
 unsigned pos;
 int refSearch;
 gElem* ptrElem( nonArtistDirs.StartPtr() );
 gString sPath( (char*)strFullPath );

 for ( ; ptrElem; ptrElem=ptrElem->next) {
     char* strNonArtistItem( ptrElem->me->Str() );
     gString sFind( strNonArtistItem );

     refSearch = ptrElem->me->iValue;  // the reference for search is >0, indicating the length to compare (without tail-wildcard)
     if ( refSearch<=0 ) {
	 if ( sFind.Find( '+' )==0 ) {
	     sFind.Add( "+" );
	 }
	 length = sFind.Length();
	 pos = sPath.Find( sFind );
	 if ( pos )
	     return (int)pos;
     }
     else {
	 pos = sPath.Find( sFind );
	 if ( pos ) {
	     gString sPart;
	     gString sLeft( sPath );
	     sPart.CopyFromTo( sPath, refSearch+1 );
	     length = (int)sPart.Find( '+' );
	     if ( length>0 ) {
		 length += refSearch;
		 sLeft.Delete( length );
		 DBGPRINT("DBG: FindNonArtistDir pos=%u {%s} length=%d, {%s}\n",
			  pos,
			  sPath.Str(),
			  length,
			  sPath.Str( length ));
		 MM_LOG( LOG_INFO, "Removed from name: {%s}", sLeft.Str() );
		 return pos;
	     }
	 }
     }
 }
 length = 0;
 return 0;
}


int sSubstedPaths::thisSplitSemicolonFromString (const char* str, int mask, gList& listed)
{
 gString sA( (char*)str );
 unsigned pos;
 int refSearch( 0 );
 sA.Trim();
 pos = sA.Find( ';' );

 if ( pos==1 ) {
     return thisSplitSemicolonFromString( str+1, mask, listed );
 }
 else {
     if ( pos ) {
	 sA[ pos ] = 0;
	 SplitSemicolonFromString( sA.Str(), listed );
	 return SplitSemicolonFromString( sA.Str( pos+1 ), listed );
     }
     else {
	 if ( sA.Length() ) {
	     if ( mask==0 ) {
		 if ( sA[ 1 ]=='*' ) {
		     MM_LOG( LOG_WARN, "Wrong leading wildcard, regexp won't be found: %s",sA.Str() );
		 }
		 else {
		     if ( sA[ sA.Length() ]=='*' ) {
			 sA.Delete( sA.Length() );
			 refSearch = (int)sA.Length();
		     }
		 }
	     }
	     listed.Add( sA );
	     listed.EndPtr()->me->iValue = refSearch;
	 }
     }
 }
 return (int)listed.N();
}

////////////////////////////////////////////////////////////
int sCrossLinkConf::InitOS ()
{
 gString sOutFile;
 char* strDump( getenv( "JUICEBOX_DUMP" ) );

 sOScommands[ e_OS_Cat ].Set( "cat" );
#ifdef iDOS_SPEC
 sOScommands[ e_OS_Copy ].Set( "copy" );
#else
 sOScommands[ e_OS_Copy ].Set( "cp" );
#endif
 sOScommands[ e_OS_ln ].Set( "ln -s" );

 if ( strDump ) {
     sOutFile.Set( strDump );
 }
 else {
     strDump = getenv( "TMP" );
     if ( strDump==nil ) strDump = getenv( "TEMP" );

     if ( strDump ) {
	 sOutFile.Set( strDump );
#ifdef iDOS_SPEC
	 sOutFile.Add( "juicebox.tmp" );
#endif
     }
     else {
	 sOutFile.Add( gSLASHSTR );
	 sOutFile.Add( "tmp" );
	 sOutFile.Add( gSLASHSTR );
	 sOutFile.Add( ".juicebox" );
     }
 }

 if ( sOutFile.Length() ) {
     sCommandToError.Set( " > " );

#ifdef iDOS_SPEC
     ;
#else
     sCommandToError.Set( " 2> " );
#endif
     sCommandToError.AddString( sOutFile );
 }

 DBGPRINT("DBG: InitOS() sCommandToError={%s}\n", sCommandToError.Str());
 return 0;
}


int sCrossLinkConf::SetVolumeType (eVolumeType aType)
{
 switch ( volumeType = aType ) {
 case e_CD:
     blockSize = 10;
     break;

 case e_DVD_4dot7:
 case e_DVD_dlayer:
     blockSize = 70;
     break;

 case e_Invalid:
 default:
     return -1;
 }
 return 0;
}

////////////////////////////////////////////////////////////
int sPlayCheck::CheckItem (int numbersGenCode, gList* ptrItem, gString& sName)
{
 // Returns 0 if all ok, or otherwise 'yet another code!'

 int code( 0 );
 int error;
 int size( ptrItem ? ptrItem->GetListInt( 3 ) : -1 );
 gString sProposed;
 gString* newName;

 // Note: without ptrItem you cannot 'guess' a better name.

 lastBasename = sName;

 if ( atNames ) {
     newName = new gString( sName );
     ASSERTION(newName,"newName");
     newName->iValue = size;
     code = atNames->InsertOrderedUnique( newName );
     error = code==-1;  // Repeated/ duplicated

     if ( error ) {
	 delete newName;
     }
     else {
	 // Try proposed name, if necessary

	 oprint("alt_name %s {%s},{%s}\n",
		numbersGenCode ? "generate#" : "no#",
		ptrItem->Str(2),
		newName->Str());
	 jcl_basic_proposed_alt_name( *newName, numbersGenCode, ptrItem, sProposed );

	 if ( sProposed.Length() ) {
	     MM_LOG( LOG_NOTICE, "Proposed <%s> instead of <%s>",
		     sProposed.Str(),
		     newName->Str() );

	     // Overwrite input provided:
	     sName = sProposed;
	 }
     }
     code = error!=0;
 }

 return code;
}

////////////////////////////////////////////////////////////
int jcl_absolute_path (const char* str)
{
 char chr( 0 );
 if ( str && (chr = str[ 0 ])!=0 ) {
     return chr=='/' || str[ 1 ]==':';
 }
 return 0;
}


int jcl_wipe_tail_cr_nl (char* strResult)
{
 // Removes tail CR ('\r') / newline ('\n')
 int idx, len;
 char chr;
 if ( strResult && (len = strlen( strResult ))>0 ) {
     for (idx=len-1; idx>=0; idx--) {
	 chr = strResult[ idx ];
	 if ( chr=='\r' || chr=='\n' ) {
	     strResult[ idx ] = 0;
	 }
	 else {
	     return idx;
	 }
     }
     return 0;
 }
 return -1;
}


int jcl_valid_seconds_m3u (const char* strExt, int skipColon)
{
 // e.g. #EXTINF:219,Inna

 int idx( 0 );
 char chr;
 const char* strNum( strExt );

 if ( skipColon ) {
     for ( ; (chr = strExt[ idx ])!=0; idx++) {
	 strNum++;
	 if ( chr==':' ) break;
     }
 }
 for (idx=0; (chr = strNum[ idx ])!=0; idx++) {
     if ( chr==',' ) break;
 }
 if ( chr==0 ) return 0;
 return atoi( strNum );
}


char* jcl_get_extension (gString& sIn, int mask, gString& sExt)
{
 unsigned pos( sIn.FindBack( '.' ) );

 if ( pos ) {
     sExt.CopyFromTo( sIn, pos );
     if ( (mask & 1)==0 ) {
	 sExt.DownString();
     }
 }
 else {
     sExt.Delete();
 }
 return sExt.Str();
}


char* jcl_extension (gString& sIn, bool downCase)
{
 unsigned len( sIn.Length() );
 gString* pNew;
 char* result;

 for (unsigned idx=len; idx>1; idx--) {
     t_uchar uChr( sIn[ idx ] );
     if ( uChr=='/' || uChr=='\\' ) break;
     if ( uChr=='.' ) {
	 result = sIn.Str( idx-1 );
	 if ( downCase ) {
	     pNew = new gString( result );
	     pNew->DownString();
	     gStorageControl::Self().Pool().AppendObject( pNew );
	     return pNew->Str();
	 }
	 return result;
     }
 }
 return sIn.Str( len );
}


ePlaylistType jcl_which_type (gString& sFile, gString& sExt)
{
 unsigned pos( sFile.FindBack( "." ) );

 if ( pos>1 ) {
     sExt.CopyFromTo( sFile, pos, 0 );
     if ( sExt.Find( '/' ) || sExt.Find( '\\' ) )
	 sExt.Delete();
     else
	 sExt.UpString();
 }
 else {
      sExt.Delete();
 }

 if ( sExt.Match( ".M3U" ) )
     return e_pl_M3U;

 if ( sExt.Match( ".VPL" ) )
     return e_pl_VPL;

 if ( sExt.Match( ".PLS" ) )
     return e_pl_PLS;

 return e_pl_Unknown;
}


int jcl_media_type_from_ext_aux (bool convToCapital, gString& sExt, int& audioType)
{
 // Type: 7=mp3; 9=ogg; 10=wma; 8=wav; 11=flac
 // See also imedia/inames.cpp

 // audioType <0 means lossless; audioType=0 means unknown;
 // audioType>0 is the named type, which is lossy (e.g. 7=mp3).

 int result( 0 );
 gString sExtension( sExt );

 audioType = 0;

 if ( convToCapital ) {
     sExtension.UpString();
 }

 if ( sExtension.Match( ".MP3" ) ) {
     audioType = 7;
 }
 else {
     if ( sExtension.Match( ".WMA" ) ) {
	 audioType = 10;
     }
     else {
	 if ( sExtension.Match( ".OGG" ) ) {
	     audioType = 9;
	 }
	 else {
	     	 if ( sExtension.Match( ".WAV" ) || sExtension.Match( ".WV" ) ) {
		     audioType = -8;
		 }
		 else {
		     if ( sExtension.Match( ".FLAC" ) ) {
			 audioType = -11;
		     }
		 }
	 }
     }
 }

 if ( audioType==0 ) return 0;

 if ( audioType<0 )
     result = -audioType;
 else
     result = audioType;
 return result;
}


int jcl_media_type_from_extension (gString& sExt)
{
 int audioType( 0 );
 return jcl_media_type_from_ext_aux( true, sExt, audioType );
}


t_uint32 jcl_usize_kb (t_uint32 size)
{
 t_uint32 mod( size % 1024 );
 if ( size==0 ) return 1;
 return size/1024 + (mod!=0);
}


t_uint32 jcl_usize_mb (t_uint32 sizeKb)
{
 return jcl_usize_kb( sizeKb );
}


t_uint32 jcl_usize_iso (t_uint32 octets, sCrossLinkConf* ptrOptConf)
{
 t_uint32 blockSize( ptrOptConf ? ptrOptConf->blockSize : 4096 );
 t_uint32 blocks( octets / blockSize );

 ASSERTION(blockSize,"blockSize");
 if ( octets==0 || (octets % blockSize)!=0 ) blocks++;
 return blocks * (blockSize / 1024);
}


t_uint32 jcl_iso_size_est (t_uint32 estSizeKb, t_uint32& estSectors, sCrossLinkConf* ptrOptConf)
{
 // Returns Mb estimation (for an input of 'estSizeKb')

 t_uint32 result( jcl_usize_kb( estSizeKb ) );
 estSectors = (estSizeKb / 2048) + ((estSizeKb % 2048)!=0) + (estSizeKb==0);
 return result;
}


char* jcl_track_shown_mmss (int secs)
{
 char* shortBuf( jclShorBuf );
 const unsigned maxBufSize( 16 );
 memset( shortBuf, 0x0, maxBufSize );

 if ( secs>=0 ) {
     snprintf( shortBuf, maxBufSize, "%u:%02u",
	       secs / 60,
	       secs % 60 );
 }
 return shortBuf;
}


void jcl_consolidate_to_slash (int toSlash, gString& sResult)
{
 // if toSlash=0, it converts all slashes to backslashes;
 // otherwise it's the opposite.

 if ( toSlash ) {
     sResult.ConvertChrTo( '\\', '/' );
 }
 else {
     sResult.ConvertChrTo( '/', '\\' );
 }
}


char* jcl_file_from_outfile (gString& sIn)
{
 unsigned aPos( sIn.Find( gSLASHCHR ) );
 if ( aPos==0 ) return nil;
 return sIn.Str( aPos-1 );
}


int jcl_dirname (gString& sFile, int mask, gString& sOutDir)
{
 int iter( (int)sFile.Length() );

 sOutDir.Delete();

 for ( ; iter>0; iter--) {
     if ( sFile[ iter ]=='/' || sFile[ iter ]=='\\' ) {
	 sOutDir.CopyFromTo( sFile, 1, iter );
	 return iter;
     }
 }
 return 0;
}


int jcl_is_fullpath (gString& sFile)
{
 return sFile[ 2 ]==':' || sFile[ 1 ]=='/' || sFile[ 1 ]=='/';
}


int jcl_is_numbered_item (const char* str)
{
 int iter( 0 ), maxIter( 3 );
 char digit( 0 );

 if ( str==nil ) return -1;

 for ( ; iter<maxIter; iter++) {
     digit = str[ iter ];
     if ( digit==0 ) return 0;  // Assume it is not numbered...too short string!
     if ( digit<'0' || digit>'9' )
	 return 0;
 }
 digit = str[ iter ];
 return digit==' ';
}


int jcl_tracknr_indication (const char* str, int& position)
{
 int iter( 0 );
 int tracknr( -1 );
 char chr;

 position = 0;

 for ( ; (chr = str[ iter ])!=0; iter++) {
     if ( strchr( "_-.,:+*", chr ) ) return -1;
     if ( chr>='0' && chr<='9' ) break;
 }
 if ( chr==0 ) return -1;

 position = iter;
 tracknr = atoi( str + iter );
 return tracknr;
}


int jcl_numbered_entries (const char* strPath, int mask, int& error)
{
 int result( 0 );
 int value( 1 );
 gDir entries( (char*)strPath );
 gElem* ptrElem( entries.StartPtr() );

 error = entries.lastOpError;
 if ( error ) return -1;

 for ( ; ptrElem; ptrElem=ptrElem->next) {
     if ( mask==0 ) {
	 value = jcl_is_numbered_item( ptrElem->Str() )==1;
	 DBGPRINT("DBG: IS NUMBERED? %d: %s\n",value,ptrElem->Str());
     }
     result += value;
 }
 return result;
}


int jcl_nicer_iso_string (gString& s, int useBlanksMask, gString& strictBase)
{
 int countCutChars( 0 );
 int posDot( -1 );
 int audioType( 0 );
 t_uchar uChr, nowChr;
 gList slashes;

 char* myUcs8;
 gUniCode* inUse;

 ASSERTION(ptrUniData,"ptrUniData");
 myUcs8 = ptrUniData->customUcs8[ 3 ];
 inUse = ptrUniData->inUse;

 // Note: strictBase is not deleted first.

 ASSERTION(inUse,"inUse");
 ASSERTION(myUcs8,"myUcs8");
#if 0
 int dbgIter( 0 ), matches;
 char strCheck[ 1024 ];

 FILE* fDbg( fopen( "check.txt", "rt" ) );
 if ( fDbg ) {
     fgets( strCheck, sizeof(strCheck)-1, fDbg );
     fclose( fDbg );
 }
 else {
     strcpy( strCheck, "Felicitá" );
 }
 for ( ; (uChr = (t_uchar)strCheck[ dbgIter ])!=0; dbgIter++) {
     nowChr = myUcs8[ uChr ];
     matches = uChr==nowChr;
     if ( matches ) {
	 matches = uniData.hashUcs16Eq[ uChr ][ 0 ]!=0;
	 if ( matches==0 ) {
	     printf("%c\t\tNot matching!\n",uChr);
	 }
	 else {
	     printf("%c\t\tmatching (%dd), %s\n",uChr,uChr,uniData.hashUcs16Eq[ uChr ]);
	 }
     }
     else {
	 printf("%c %c [%s]\tASCII %dd\n",
		uChr < ' ' ? '.' : uChr,
		nowChr < ' ' ? '!' : uChr,  // should not!
		uniData.hashUcs16Eq[ uChr ],
		(int)uChr);
     }
     strictBase.Add( uniData.hashUcs16Eq[ uChr ] );
 }
 printf("IN1: %s\nOUT: %s\n",
	strCheck,
	strictBase.Str());
 strictBase.Delete();
#endif // just verbose basic debug on uniData, if needed...

 for (int idx=1; (uChr = s[ idx ])!=0; idx++) {
     nowChr = uChr;
     if ( uChr<' ' ) nowChr = 0;
     if ( uChr>=127 && uChr<=0xA0 ) nowChr = 0;
     if ( uChr=='"' ) nowChr = 0;

     switch ( nowChr ) {
     case 0:
	 countCutChars++;
	 break;

     case ':':
     case '?':
     case '<':
     case '>':
	 break;

     case ' ':
     case '_':
	 if ( useBlanksMask )
	     nowChr = ' ';
	 else
	     nowChr = '_';
	 if ( strictBase[ strictBase.Length() ] != nowChr )
	     strictBase.Add( nowChr );
	 break;

     case '\\':
     case '/':
	 nowChr = '+';
	 slashes.Add( (int)strictBase.Length() );
	 strictBase.Add( nowChr );
	 break;

     case '.':
	 posDot = idx;
	 // No break here!

     default:
	 strictBase.Add( nowChr );
     }
 }

 if ( posDot<1 ) {
     MM_LOG( LOG_ERROR, "No dots on file: %s", s.Str() );
 }
 else {
     // Check if there are too many slashes in input...
     if ( slashes.N() > 2 ) {
	 strictBase.Delete( 1, slashes.GetListInt( slashes.N()-1 ) );
	 MM_LOG( LOG_INFO, "Reduced slashed path (%d): %s", slashes.N(), strictBase.Str() );
     }

     if ( posDot>=(int)s.Length() ) {
	 MM_LOG( LOG_ERROR, "Tailing dot on file: %s", s.Str() );
     }
     else {
	 posDot = strictBase.FindBack( '.' );

	 if ( posDot>1 && (useBlanksMask & 16)==0 ) {
	     // This is always like that unless someone uses '...|16' calling this function,
	     // to ignore media file extensions.

	     gString sExt( strictBase.Str( posDot-1 ) );
	     gString sUp( sExt );

	     strictBase.Delete( posDot );

	     sUp.UpString(); sExt = sUp;
	     jcl_media_type_from_ext_aux( false, sExt, audioType );
	     strictBase.AddString( sExt );

	     if ( audioType<=0 ) {
		 // <0 is lossless; not allowing

		 MM_LOG( LOG_ERROR, "Invalid audio media: %s (%s)%s",
			 s.Str(),
			 sExt.Str(),
			 audioType ? ", lossless!" : "\0" );
	     }
	 }
     }
 }
 return countCutChars;
}


int jcl_subst_string_at (gString& fromString, unsigned pos, gString& toString, gString& sResult)
{
 if ( pos==0 ) return 0;

 sResult.Delete( pos, pos+fromString.Length()-1 );
 sResult.Insert( toString, pos );
 return 0;
}


int jcl_add_path (gString& sIn, gString& sOut)
{
 unsigned posSlash( sIn.Find( '/' ) );
 unsigned posBkSlash( sIn.Find( '\\' ) );
 bool inputToAddHasSlash( posSlash>0 );
 gString sCopy;

 /* If output already has slashes, and input does not, it adds input with slashes instead.
    Same applies otherwise (backslashes.)
 */

 if ( posSlash && posBkSlash ) {
     J_DUMP( "Bogus conv sIn: %s\n",sIn.Str() );
     sOut.AddString( sIn );
     return -1;
 }

 posSlash = sOut.Find( '/' );
 posBkSlash = sOut.Find( '\\' );
 if ( posSlash && posBkSlash ) {
     J_DUMP( "Bogus conv sOut: %s\n",sOut.Str() );
     sOut.AddString( sIn );
     return -2;
 }

 sCopy = sIn;
 if ( posSlash ) {
     if ( inputToAddHasSlash==false ) {
	 J_DUMP( "conv backslash to slashes: %s\n",sCopy.Str() );
	 sCopy.ConvertChrTo( '\\', '/' );
     }
 }
 else {
     if ( inputToAddHasSlash ) {
	 J_DUMP( "conv slash to backslashes: %s\n",sCopy.Str() );
	 sCopy.ConvertChrTo( '/', '\\' );
     }
 }

 J_DUMP( "conv: %s+%s\n",
	 sOut.Str(),
	 sCopy.Str() );
 sOut.AddString( sCopy );
 return 0;
}


int jcl_subst_string_by (gString& fromString, gString& toString, int option, gString& sResult)
{
 unsigned pos( 0 );

 J_DUMP("jcl_subst_string_by(%s,%d,%s)\n",
	fromString.Str(),
	option,
	sResult.Str());

 switch ( option ) {
 case 0:
 default:
     pos = sResult.Find( fromString );
     if ( pos<=3 ) {
	 jcl_subst_string_at( fromString, pos, toString, sResult );
	 if ( pos>2 ) {
	     // e.g. 'F:\abc\def\' is to be subst´ed by /net/mac/
	     //	-> we have to substitute \abc\def\ by /net/mac/ and remove 'F:'

	     if ( sResult[ 2 ]==':' ) sResult.Delete( 1, 2 );
	 }
     }
     break;
 }

 J_DUMP("jcl_subst_string_by result (%s) pos=%u\n",
	sResult.Str(),
	pos);
 return 0;
}


int jcl_meta_from_item (sCrossLinkConf& linkConf, gList& item, gString* ptrMeta)
{
 sSubstedPaths* ptrSubsts( &linkConf.substed );
 gString sName( item.Str( 1 ) );
 gElem* ptrElem( ptrSubsts->sPrefixSubsts.StartPtr() );
 gList* ptrOneSubst;

 if ( ptrMeta ) {
     if ( ptrElem==nil ) {
	 // Leave as is:
	 ptrMeta->Set( sName.Str() );
	 return 0;
     }
 }

 if ( ptrMeta ) {
     for ( ; ptrElem; ptrElem=ptrElem->next) {
	 // Substitute strings

	 ptrOneSubst = (gList*)ptrElem->me;

	 gString sFrom( ptrOneSubst->Str( 1 ) );
	 gString sTo( ptrOneSubst->Str( 2 ) );

	 jcl_subst_string_by( sFrom, sTo, 0, sName );

	 ptrMeta->Set( sName.Str() );

	 DBGPRINT_MIN("DBG: subst:\t%s\n%s by %s\n\n",ptrMeta->Str(),sFrom.Str(),sTo.Str());
     }

     //// jcl_consolidate_to_slash( sName.Find( '/' )>0, sName );
 }
 return 0;
}


int jcl_basic_proposed_alt_name (gString sIn, int numbersGenCode, gList* ptrItem, gString& sResult)
{
 // sResult is empty if there is no better suggestion

 int posA( (int)sIn.Find( '/' ) ), posB( (int)sIn.Find( '\\' ) );
 int outer( 0 );
 int iter( 1 );
 int state( 0 ), beforeState;
 int diffs( -1 );
 int countDiffs( 0 );
 t_uchar uChr;
 t_uchar uOpen, uClose;
 const char* strOpenClose( "[]{}()" );

 gString newIn( sIn );

 ASSERTION(posA==0,"posA==0");
 ASSERTION(posB==0,"posB==0");
 ASSERTION(ptrItem,"ptrItem");

 for ( ; state==0; outer++) {
     gString sPossible;

     uOpen = strOpenClose[ outer ];
     if ( uOpen==0 ) break;

     uClose = strOpenClose[ ++outer ];

     for (iter=1; (uChr = newIn[ iter ])!=0; iter++) {
	 beforeState = state;
	 if ( uChr==uOpen ) {
	     if ( state==0 ) {
		 state = 1;
		 beforeState = 1;
	     }
	 }
	 else {
	     if ( uChr==uClose ) {
		 if ( state ) {
		     state = 0;
		 }
	     }
	 }
	 if ( beforeState==0 ) {
	     sPossible.Add( uChr );
	 }
     }

     diffs = sPossible.Length()>0 && sPossible.Match( newIn )==false;

     if ( diffs ) {
	 DBGPRINT_MIN("%s%s%s  %c%c\n",
		      countDiffs ? "\0" : newIn.Str(),
		      countDiffs ? "\0" : "\n",
		      sPossible.Str(),
		      uOpen, uClose);
	 countDiffs++;
	 newIn = sPossible;
     }
 }

 if ( countDiffs ) {
     newIn.Trim();
     for (diffs=999; (posA = newIn.Find( "  " ))>0; diffs--) {
	 newIn.Delete( posA, posA );
	 if ( diffs<0 ) break;
     }

     // e.g. "abc .MP3" ==> "abc.MP3"  (.MP3 has 4 chars.)
     for (diffs=999; (posA = newIn.Find( " ." ))>0; diffs--) {
	 if ( posA+4<(int)newIn.Length() ) break;
	 newIn.Delete( posA, posA );
	 if ( diffs<0 ) break;
     }

     // e.g. "Sale +02 Loca.MP3" ==> "Sale + Loca.MP3"
     for (iter=1; iter<=99; iter++) {
	 char bufNum[ 8 ];
	 sprintf( bufNum, "+%02d ", iter );
	 posA = newIn.Find( bufNum );
	 if ( posA>0 ) {
	     newIn.Delete( posA+1, posA+strlen( bufNum )-2 );
	 }
     }
     sResult = newIn;
 }

 aprint("proposed: %s\n[from: %s]\n\n",
	sResult.Str(),
	ptrItem->Str( 2 ));
 return countDiffs;
}


int jcl_build_item (sCrossLinkConf& linkConf, sInPlaylist& plays, gList& item, gString& outField)
{
 int error( 0 );
 int countCutChars( 0 );
 int code( -1 );
 int maxCuts( 9 );
 gFileStat aDir( linkConf.sBaseDir );
 gString based( item.Str( 5 ) );
 gString newBase;
 gString shortBase;
 t_uchar lastChr;
 char digs[ 32 ];
 bool generateNumbers( linkConf.listing==sCrossLinkConf::e_Numbered );
 int numbersGenCode( (int)generateNumbers );

 if ( outField.IsEmpty()==false ) return 0;

 gString sTarget;
 gString sAt;

 linkConf.freeCounter++;
 digs[ 0 ] = 0;
 if ( linkConf.digitsFormat[ 0 ] ) {
     char strFormat[ 16 ];
     // Has a blank after '%s' !
     snprintf(strFormat, sizeof(strFormat), "%s ", linkConf.digitsFormat);
     snprintf(digs, sizeof(digs), strFormat, linkConf.freeCounter);
 }

 sAt.AddString( linkConf.sBaseDir );
 lastChr = sAt[ sAt.Length() ];
 if ( lastChr=='/' || lastChr=='\\' ) {
 }
 else {
     if ( linkConf.sBaseDir.Find( '/' ) )
	 sAt.Add( '/' );
     else
	 sAt.Add( '\\' );
 }

 newBase = based.BaseName();
 if ( newBase.IsEmpty() ) return 3;

 gString sTitle( item.Str( 2 ) );
 aprint("name Base1: %s [item#2: %s%s]\n", newBase.Str(), sTitle.Str(), sTitle.IsEmpty() ? "<empty>" : "");
 // sTitle is empty whenever #EXTINF is not there

 // newBase is e.g. abc.mp3;
 // we prefer title whenever consistent (this is stored as sExtInf):
 if ( sTitle.Length() ) {
     int pos( newBase.FindBack( '.' ) );

     // someone preferred this ... (because it's not empty).
     // We DO care about the extension too.

     if ( pos>3 && (int)newBase.Length()==pos+3 ) {
	 gString sExtInfExtension( newBase.Str( pos-1 ) );
	 aprint("name Base2: %s; sTitle: %s; extension_there: %s\n", newBase.Str(), sTitle.Str(), sExtInfExtension.Str());

	 // If name is 'song_name.mp3' and sExtInfExtension.UpString()='.MP3',
	 // we do not want a fuzzy repetition of '.mp3.MP3'
	 int checkPos( sTitle.FindBack( sExtInfExtension, true ) );
	 if ( checkPos>=4 && sExtInfExtension.Length()==4 && checkPos+sExtInfExtension.Length() ) {
	     MM_LOG(LOG_INFO, "EXTINF of m3u has extension: %s", sTitle.Str());
	 }
	 else {
	     sExtInfExtension.UpString();
	     sTitle.AddString( sExtInfExtension );
	 }
     }
     newBase = sTitle;
 }

 aprint("name Base3: %s\n", newBase.Str());

 plays.checks.lastInputName = newBase;

 // Convert buggy ISO chars into something nicer

 countCutChars = jcl_nicer_iso_string( newBase, linkConf.useBlanksMask, shortBase );

 for ( ; shortBase[ 1 ]=='+' && shortBase.Length()>1; ) {
     shortBase.Delete( 1, 1 );
 }

 if ( countCutChars ) {
     MM_LOG( LOG_NOTICE, "Cut %d char(s) in input string <%s>",
	     countCutChars,
	     newBase.Str() );
 }

 for ( ; code; maxCuts--) {
     unsigned posInStr( linkConf.substed.FindNonArtistDir( shortBase.Str(), code ) );

     DBGPRINT("DBG: posInStr(%s), code=%d: %u\n",
	      shortBase.Str(),
	      code,
	      posInStr);

     if ( posInStr ) {
	 if ( posInStr==1 ) {
	     shortBase.Delete( 1, code );

	     MM_LOG( LOG_INFO, "Cut to: <%s>",shortBase.Str());
	 }
	 // ...otherwise, if found at middle of string, don't change.
     }

     // Do none or zero substitutions, no more:
     // -> in the case an artist has a bogus name, it will be only
     //    removed at the beginning of string.
     //
     // Uncomment to do just one :
     //		code = 0;

     if ( maxCuts<0 ) break;
 }

 code = plays.checks.CheckItem( numbersGenCode, &item, shortBase );
 error = code!=0;

 int existingSize( -1 ), thisSize( -1 );

 switch ( code ) {
 case 1:
     existingSize = plays.checks.atNames->CurrentPtr()->me->iValue;
     thisSize = item.GetListInt( 3 );

     if ( existingSize==thisSize ) {
	 MM_LOG( LOG_ERROR, "Repeated title: %d Kb: <%s>",
		 thisSize,
		 shortBase.Str() );
     }
     else {
	 MM_LOG( LOG_ERROR, "Repeated name: existing=%d, this=%d Kb: <%s>",
		 existingSize,
		 thisSize,
		 shortBase.Str() );
     }
     error = -1;
     break;
 case 0:
 default:
     break;
 }

 aprint("name sAt_2: %s|%s+%s (error=%d)\n",
	sAt.Str(),
	digs,
	shortBase.Str(),
	error);

 if ( generateNumbers ) {
     t_uchar third( shortBase[ 3 ] );

     sAt.Add( digs );

     // E.g. /path/123 +01 Geronimo.MP3
     //		we want to get rid of '01' in the title
     if ( third ) {
	 if ( shortBase[ 1 ]>='0' && shortBase[ 1 ]<='9' &&
	      shortBase[ 2 ]>='0' && shortBase[ 2 ]<='9' &&
	      (third==' ' || third=='.' || third=='-') ) {
	     shortBase.Delete( 1, 3 );
	     shortBase.TrimLeft();
	     third = shortBase[ 1 ];
	     if ( third=='.' || third=='-' ) {
		 shortBase.Delete( 1, 1 );
		 shortBase.TrimLeft();
	     }
	 }
     }
 }

 sAt.AddString( shortBase );
 jcl_consolidate_to_slash( sAt.Find( '/' )>0, sAt );

 sTarget = based;

 outField.AddString( sTarget );
 outField.Add( "\t" );
 outField.AddString( sAt );

 return error;
}


gList* jcl_new_item (sCrossLinkConf& linkConf,
		     gString& sDir,
		     gString& sItemPath,
		     int secs,
		     gString& sExtInf,
		     gString* ptrOptLine,
		     int& size)
{
 gList* ptrNew( new gList );
 gString sFullPath;
 gString sMeta;
 int knownStatus( 0 );

 aprint("\
Dir: %s\n\
Item: %s\n\
Info: %s.%s%s\n\
\n",
	sDir.Str(),
	sItemPath.Str(),
	sExtInf.Str(),
	ptrOptLine ? " " : "",
	ptrOptLine ? ptrOptLine->Str() : "");

 ASSERTION(ptrNew,"ptrNew");
 size = -1;

 if ( jcl_is_fullpath( sItemPath )==0 ) {
     sFullPath.AddString( sDir );
 }
 jcl_add_path( sItemPath, sFullPath );

 ptrNew->Add( sItemPath );
 ptrNew->EndPtr()->me->iValue = secs;
 ptrNew->Add( sExtInf );
 ptrNew->Add( size );
 ptrNew->Add( sFullPath );
 ptrNew->Add( sMeta );
 ptrNew->Add( knownStatus );
 ptrNew->Add( "" );  // free-1
 ptrNew->Add( "" );  // free-2
 ptrNew->Add( "" );  // free-3
 ptrNew->Add( "" );  // free-4
 ptrNew->Add( "" );  // built link-target

#if 0
 printf(":::\n");ptrNew->Show();printf("!\n\n");
#endif

 return ptrNew;
}

////////////////////////////////////////////////////////////
char* jcl_guess_config_file (const char* strConfFile)
{
 char* strResult( (char*)strConfFile );
 char* strTry;
 char* strAlt1( nil );
 const char* strJuiceConf( "juicebox.conf" );

 if ( (strTry = getenv( "SUBST" ))!=nil )
     return strTry;

 if ( strResult && strResult[ 0 ] )
     return strResult;

 strResult = jclShorBuf;

 strTry = getenv( "USERPROFILE" );

#ifdef iDOS_SPEC
 if ( strTry==nil ) {
     strTry = getenv( "HOMEDRIVE" );
     if ( strTry==nil ) {
	 strAlt1 = "C:";
     }
 }
#else
 if ( strTry==nil ) {
     strTry = getenv( "HOME" );
     if ( strTry==nil ) {
	 strAlt1 = (char*)"/etc";
     }
 }
#endif

 if ( strTry ) {
     snprintf( strResult, jclShorBufSize-1,
	       "%s" gSLASHSTR "%s",
	       strTry,
	       strJuiceConf );
     gFileStat testConf( strResult );
     if ( testConf.HasStat() )
	 return strResult;
 }

 strTry = strAlt1;

 if ( strTry ) {
     snprintf( strResult, jclShorBufSize-1,
	       "%s" gSLASHSTR "%s",
	       strTry,
	       strJuiceConf );
     gFileStat testConf( strResult );
     if ( testConf.HasStat() )
	 return strResult;
 }

 strResult[ 0 ] = 0;
 return strResult;
}


int jcl_config (const char* strConfFile, sCrossLinkConf& linkConf)
{
 const char* strTry( getenv( "DIGITS_FORMAT" ) );
 int error( jcl_conf_to_substed( strConfFile, linkConf, linkConf.substed ) );
 DBGPRINT("DBG: jcl_config returns %d\n",error);

 if ( strTry && strTry[ 0 ] ) {
     const char firstChr( strTry[ 0 ] );
     if ( firstChr!='%' ) {
	 if ( firstChr=='.' ) {
	     // Empty format
	     linkConf.digitsFormat[ 0 ] = 0;
	 }
	 else {
	     if ( (firstChr>='0' && firstChr<='9') || firstChr=='d' ) {
		 snprintf(linkConf.digitsFormat, sizeof(linkConf.digitsFormat)-1, "%%%s", strTry);
		 if ( firstChr>'0' && firstChr<'9' ) {
		     fprintf(stderr, "Leading blanks in DIGITS_FORMAT {%s}\n", linkConf.digitsFormat);
		 }
	     }
	     else {
		 fprintf(stderr, "Invalid DIGITS_FORMAT {%s}, ignored!\n", strTry);
	     }
	 }
     }
     else {
	 snprintf(linkConf.digitsFormat, sizeof(linkConf.digitsFormat)-1, "%s", strTry);
     }
 }
 return error;
}


int jcl_conf_to_substed (const char* strConfFile, sCrossLinkConf& linkConf, sSubstedPaths& substed)
{
 const char* configFile( jcl_guess_config_file( strConfFile ) );
 const char* rValue( nil );
 int nSubsts( 0 );

 if ( configFile && configFile[ 0 ] ) {
     gFileFetch conf( (char*)configFile );
     gElem* ptrElem( conf.aL.StartPtr() );

     if ( conf.IsOpened()==false ) {
	 MM_LOG( LOG_ERROR, "Unable to use config file: %s", configFile );
	 return 2;
     }

     MM_LOG( LOG_INFO, "Reading config file: %s", configFile );

     for (int idxLine=1; ptrElem; ) {
	 gString sLine( ptrElem->Str() );
	 bool didIt( false );

	 sLine.Trim();
	 if ( sLine.Length() && sLine[ 1 ]!='#' ) {
	     if ( sLine.Find( "prefix_subst=" )==1 ) {
		 rValue = sLine.Str( strlen( "prefix_subst=" ) );

		 // -> parse format like:
		 //			\images\;/net/mach/huge/;../music

		 gParam lAndR( (char*)rValue, ";" );
		 gString sLeft( lAndR.Str( 1 ) );
		 gString sRight( lAndR.Str( 2 ) );

		 sLeft.Trim();
		 sRight.Trim();

		 gList* newTriple( new gList );
		 newTriple->Add( sLeft );
		 newTriple->Add( sRight );
		 newTriple->Add( ".." );
		 substed.sPrefixSubsts.AppendObject( newTriple );
		 didIt = true;
	     }

	     if ( sLine.Find( "non_artist=" )==1 ) {
		 sLine.Delete( 1, strlen( "non_artist=" ) );
		 sLine.Trim();
		 substed.AddNonArtistDir( sLine.Str() );

		 for (ptrElem=ptrElem->next; ptrElem; ptrElem=ptrElem->next, idxLine++) {
		     if ( ((gString*)ptrElem->me)->Find( '=' ) ) break;
		     substed.AddNonArtistDir( ptrElem->Str() );
		 }
		 continue;
	     }

	     if ( sLine.Find( "used_title=" )==1 ) {
		 sLine.Delete( 1, strlen( "used_title=" ) );
		 sLine.Trim();
		 didIt = true;
		 if ( sLine.Match( "TitleFirst", true ) ) {
		     // This is the DEFAULT!
		 }
		 else {
		     if ( sLine.Match( "UnusedAlways", true ) ) {
			 linkConf.usedNameFirst = sCrossLinkConf::e_UnusedAlways;
		     }
		     else {
			 if ( sLine.Match( "FileFirst", true ) ) {
			     linkConf.usedNameFirst = sCrossLinkConf::e_FileFirst;
			 }
			 else {
			     didIt = false;
			     MM_LOG( LOG_WARN, "Invalid r-value for used_title");
			 }
		     }
		 }
	     }

	     if ( didIt==false ) {
		 MM_LOG( LOG_WARN, "Config, line %d: not understood.\n",idxLine);
	     }
	 }
	 ptrElem=ptrElem->next;
	 idxLine++;
     }//end FOR
 }

 if ( configFile==nil ) return 0;

#if 0
 // Removed these hard-coded substitutes:
 strSemicolon = "mp3 snip;" \
		"mp3 room;" \
		"mp3 new;" \
		"mp3 ancient;" \
		"mp3;" \
		"bau;" \
		"Various;" \
		"Disc1of2;" \
		"Disc2of2;" \
		"320kbps;" \
		"CD;" \
		"CD 1;" \
		"CD 2;" \
		"CD 3;" \
		"CD 4;" \
		"CD 5;" \
		"CD 6;" \
		"CD1;" \
		"CD2;" \
		"CD3;" \
		"CD4;" \
		"CD5;" \
		"CD6;" \
		"" \
     ;
 // semicolon separated list with strings (paths) that
 // should NOT be considered as artists!
 if ( strSemiColon ) {
     // Never reaches here:
     substed.NonArtistDirsFromString( strSemicolon );
 }

 if ( configFile[ 0 ]==0 ) {
 gList* newTriple;
 newTriple = new gList;
 newTriple->Add( "\\huge\\images\\music\\" );
 newTriple->Add( "/net/luisa/huge/images/music/" );
 newTriple->Add( "../music" );
 substed.sPrefixSubsts.AppendObject( newTriple );

 newTriple = new gList;
 newTriple->Add( "\\huge\\images\\music_copy\\wav\\" );
 newTriple->Add( "/net/luisa/huge/images/music_copy/wav/" );
 newTriple->Add( "../music_copy" );
 substed.sPrefixSubsts.AppendObject( newTriple );
 }// hard-coded substitutes
#endif  // Sample above, NOT used!

 nSubsts = (int)substed.sPrefixSubsts.N();

 MM_LOG( LOG_NOTICE, "Considering %u non-artist paths: %s; ...",
	 substed.nonArtistDirs.N(),
	 substed.nonArtistDirs.Str( 1 ) );
 if ( nSubsts ) {
     MM_LOG( LOG_NOTICE, "Considering %d substs: (%s%s)",
	     nSubsts,
	     substed.sPrefixSubsts.StartPtr()->me->Str( 1 ),
	     nSubsts>1 ? ", ..." : "\0" );
 }
 else {
     MM_LOG( LOG_NOTICE, "Not considering substs" );
 }

#if 0
 substed.sPrefixSubsts.Show(); printf("<-- sPrefixSubsts\n\n");

 substed.nonArtistDirs.Show(); printf("<-- nonArtistDirs\n\n");
#endif

 return 0;
}



int jcl_add_pname (gString& myStr, gList& result)
{
 const int mask( 0 );
 gString sNew( myStr );

 for ( ; (sNew[ 1 ]=='.' && (sNew[ 2 ]==gSLASHCHR || sNew[ 2 ]==altSLASHCHR)); ) {
     sNew.Delete( 1, 2 );
 }
 return jcl_add_once_mask( sNew.Str(), 0, mask, result );
}


int jcl_add_once (const char* aStr, gList& result)
{
 const int mask( 0 );
 return jcl_add_once_mask( aStr, 0, mask, result );
}


int jcl_add_once_mask  (const char* aStr, int value, int mask, gList& result)
{
 if ( aStr ) {
     char* pNew( str_simpler_path( strdup( aStr ) ) );
     gString* newStr( new gString( pNew ) );
     ASSERTION(newStr,"newStr");
     ASSERTION(pNew,"pNew");
     free( pNew );
     if ( result.InsertOrderedUnique( newStr )==-1 ) {
	 delete newStr;
	 return -1;
     }
     newStr->iValue = value;
 }
 return (int)result.N();
}


int jcl_trim_line (gString& sLine, int mask)
{
 int iter;
 t_uchar chr;

 if ( mask<0 ) {
     sLine.TrimLeft();
 }

 for (iter=(int)sLine.Length(); iter>=1 && ((chr = sLine[ iter ])>1 && chr<=' '); ) {
     sLine[ iter ] = 0;
     iter--;
 }
 return iter;
}


int jcl_read_playlist (sCrossLinkConf& linkConf, gString& sFile, sInPlaylist& plays)
{
 int error;
 int iter( 0 );
 int secs( -1 );
 int ignoreChars( 0 );
 int nErroredItems( 0 );
 unsigned pos( 0 );
 unsigned estSizeKb( 0 );  // estimated size (in Kb)
 t_uint32 estSectors( 0 );

 bool itemAllOk( true );
 bool isPlsStyle;
 bool isPlsInput( false );

 gFileFetch input( sFile );
 gString sExtInf;
 gString sDir;
 gString sExt;
 gElem* ptrElem( input.aL.StartPtr() );
 gList* ptrItem( nil );
 gElem* itemList;
 gString* ptrMeta;
 gInt* ptrStatus( nil );

 const char* strBasicName( nil );

 error = input.IsOpened()==false;
 DBGPRINT("Read: %s: %d line(s), error: %d\n",
	  sFile.Str(),
	  input.aL.N(),
	  error);
 if ( error ) return 2;

 plays.type = jcl_which_type( sFile, sExt );
 jcl_dirname( sFile, 0, sDir );

 isPlsInput = sExt.Match( ".PLS" );
 DBGPRINT("DBG: jcl_read_playlist plays.type=%d, PLS? %c sExt: {%s}\n",
	  plays.type,
	  ISyORn( isPlsInput ),
	  sExt.Str());

 for ( ; ptrElem; ptrElem=ptrElem->next) {
     gString sNew;
     gString sLine( ptrElem->Str() );
     gString fullFilePath;
     gString relativeLink;
     int size( -1 );
     unsigned posEq( 0 );

     jcl_trim_line( sLine, 0 );
     if ( sLine.IsEmpty() ) continue;

     posEq = sLine.Find( "=" );

     switch ( plays.type ) {
     case e_pl_PLS:  // Note: this is just a basic PLS support
	 DBGPRINT("DBG: PLS {%s}\n", sLine.Str());
	 if ( sLine[ 1 ]=='[' ) continue;  // informative, usually "[playlist]"
	 if ( posEq==0 ) continue;

	 // e.g.	"File1=abc.mp3", but ignoring e.g. "Title1=Singer / Song"
	 if ( sLine.Find("File", true)!=1 ) continue;

	 // No break here

     case e_pl_M3U:
	 if ( sLine.Find( "#EXTINF:" )==1 ) {
	     unsigned posComma;
	     ignoreChars = strlen( "#EXTINF:" );
	     if ( linkConf.usedNameFirst==sCrossLinkConf::e_TitleFirst ) {
		 sExtInf.CopyFromTo( sLine, ignoreChars+1, 0 );
	     }
	     secs = atoi( sExtInf.Str() );
	     posComma = sExtInf.Find( ',' );

	     DBGPRINT("DBG: sExtInf posComma=%d {%s}\n", posComma, sExtInf.Str());
	     if ( posComma>0 && posComma<5 && sExtInf.Length() > posComma + 5) {
		 sExtInf.Delete( 1, posComma );
	     }
	 }
	 else {
	     if ( sLine[ 1 ]=='#' ) continue;

	     isPlsStyle = sLine.Find( "File" )==1 && (posEq > 4 && posEq < 12);
	     if ( isPlsStyle ) {
		 sNew.CopyFromTo( sLine, posEq+1 );
		 if ( isPlsInput==false ) {
		     MM_LOG(LOG_WARN, "Accepting suspicious PLS entry: %s", sNew.Str());
		 }
	     }
	     else {
		 sNew = sLine;
	     }
	 }
	 break;

     case e_pl_VPL:
	 pos = sLine.Find( JCL_ASCII01 );
	 if ( pos ) {
	     const char* strThis( nil );
	     sNew.CopyFromTo( sLine, 1, pos-1 );

	     // In VPL, first line statement is the filename, separated by ASCII 0x01;
	     // We fill in sExtInf with reasonable info we have from either "NAME=" or "ATST=";
	     // "ALBM=" (the album) is often not correctly filled-in, and not interesting.

	     sLine.Delete( 1, pos );
	     // sExtInf is filled-in always (unless you change usedNameFirst config at .h!

	     if ( linkConf.usedNameFirst==sCrossLinkConf::e_TitleFirst ) {

		 gParam elems( sLine, JCL_ASCII01S );
		 gList relevant;
		 int trackNr( -1 );
		 for (pos=1; pos<=elems.N(); pos++) {
		     strThis = elems.Str( pos );
		     if ( strncmp( strThis, "NAME=", 5 )==0 ) {
			 relevant.Add( (char*)(strThis+5) );
			 break;
		     }
		 }
		 if ( relevant.N()==0 ) relevant.Add( "" );
		 for (pos=1; pos<=elems.N(); pos++) {
		     strThis = elems.Str( pos );
		     if ( strncmp( strThis, "ATST=", 5 )==0 ) {
			 relevant.Add( (char*)(strThis+5) );
			 break;
		     }
		 }
		 if ( elems.FindFirst( "TRKN=", 1, e_FindExactPosition ) ) {
		     const char* strTrackNumber( elems.CurrentPtr()->Str() );
		     if ( strTrackNumber[ 0 ] ) {
			 relevant.Add( (char*)(strTrackNumber + 5) );
			 relevant.EndPtr()->me->iValue = atoi( strTrackNumber + 5 );
		     }
		 }
		 else {
		     relevant.Add( 0 );
		 }

		 if ( relevant.N()==1 ) relevant.Add( "" );
		 ASSERTION(relevant.N()>0,"N()?");

		 for (pos=1; pos<=2; pos++) {
		     if ( relevant.GetObjectPtr( pos )->Kind()==gStorage::e_String ) {
			 jcl_trim_line( *((gString*)relevant.GetObjectPtr( pos )), -1 );
		     }
		 }
		 pos = 1;
		 if ( relevant.GetObjectPtr( pos )->Kind()==gStorage::e_String && relevant.Str( pos )[ 0 ] ) {
		     unsigned blankAt( 0 );
		     sExtInf.Set( relevant.Str( pos ) );
		     trackNr = atoi( sExtInf.Str() );
		     if ( trackNr!=0 && trackNr==relevant.EndPtr()->me->iValue ) {
			 // Track number may be e.g. "16.Song name",
			 // and we want, at the end: "16.Song name" !
			 blankAt = sExtInf.Find( relevant.Str( 3 ) );
			 DBGPRINT("DBG: blankAt=%u, Str(3)=%s, sExtInf={%s}\n",
				  blankAt,
				  relevant.Str( 3 ),
				  sExtInf.Str());
			 if ( blankAt ) {
			     sExtInf.Delete( 1, strlen( relevant.Str( 3 ) ) );
			 }
			 else {
			     blankAt = sExtInf.Find( ' ' );
			     if ( blankAt ) {
				 // Remove track# within name itself:
				 sExtInf.Delete( 1, blankAt );
				 sExtInf.Trim();
			     }
			 }
			 if ( sExtInf[ 1 ]=='.' ) {
			     sExtInf.Delete( 1, 1 );
			 }
		     }
		     if ( blankAt ) {
			 MM_LOG( LOG_NOTICE, "Removed track# from title: %s", sExtInf.Str());
		     }
		 }
		 else {
		     sExtInf.Set( relevant.Str( 2 ) );
		 }
		 sExtInf.Trim();
	     }// end IF usedNameFirst Title (default)
	 }
	 pos = sLine.Find( JCL_ASCII01S "TIME=" );
	 if ( pos ) {
	     secs = atoi( sLine.Str( pos + 5 ) );
	 }
	 jcl_trim_line( sNew, 0 );
	 break;

     case e_pl_raw:
	 break;

     case e_pl_Unknown:
	 if ( posEq ) {
	     if ( NameFromWPL( sLine, 0, sNew )==0 ) {
		 MM_LOG( LOG_INFO, "WPL entry: %s", sNew.Str() );
	     }
	 }
	 break;

     default:
	 break;
     }

     if ( sNew.Length() ) {
	 // Fields are:
	 //	path-name (e.g. H:\path\a.mp3, or just b.mp3)
	 //	extended-info (e.g. Julio Iglesias - ¡Uno!)
	 //	size, in octets
	 //	full-path (e.g. H:\path\a.mp3, or H:\path\b.mp3)
	 //	meta-path
	 //	known-status (0: ok; other: error)
	 //	free-1
	 //	free-2
	 //	free-3
	 //	free-4
	 //	further built link-target (initially empty)

	 gList* ptrNew( jcl_new_item( linkConf,
				      sDir,
				      sNew,
				      secs,
				      sExtInf,
				      nil,
				      size ) );
	 ASSERTION(ptrNew,"ptrNew");
	 plays.items.AppendObject( ptrNew );

	 DBGPRINT("DBG: Added %s; %s; secs=%d\n",
		  sNew.Str(),
		  sExtInf.Str(),
		  secs);

	 if ( secs>0 ) {
	     linkConf.totalSeconds += secs;
	 }

	 secs = -1;
	 sExtInf.Delete();
     }
 }// end FOR (iterating playlist lines)

 DBGPRINT("DBG: type: %d; items: %d\n",
	  plays.type,
	  plays.items.N());

 // Build meta

 for (ptrElem=plays.items.StartPtr(); ptrElem; ptrElem=ptrElem->next) {
     gString* ptrHint( nil );

     ptrItem = (gList*)ptrElem->me;
     ptrStatus = (gInt*)(ptrItem->GetObjectPtr( 6 ));
     ptrMeta = (gString*)(ptrItem->GetObjectPtr( 5 ));
     ASSERTION(ptrMeta,"ptrMeta");

     jcl_meta_from_item( linkConf, *((gList*)ptrElem->me), ptrMeta );
     aprint("Meta: %s {%s}\n",ptrElem->Str(),ptrMeta->Str());

     gFileStat aStat( *ptrMeta );
     error = aStat.HasStat()==false;
     ptrStatus->SetInt( error );
     itemAllOk = false;

     if ( error ) {
	 if ( linkConf.complainFileCheck ) {
	     MM_LOG(  LOG_ERROR, "Check failed (%d) %s",
		      aStat.lastOpError,
		      ptrMeta->Str() );
	 }
     }
     else {
	 // Usually here 'free-1' is empty;
	 // if so, go ahead and build it!

	 unsigned uSize( jcl_usize_kb( aStat.status.USize() ) );

	 ((gInt*)ptrItem->GetObjectPtr( 3 ))->SetInt( (int)uSize );

	 ptrHint = (gString*)ptrItem->GetObjectPtr( 7 );
	 ASSERTION(ptrHint,"ptrHint");

	 error = jcl_build_item( linkConf, plays, *ptrItem, *ptrHint );
	 aprint("name hint_: %s (error=%d)\n", ptrHint->Str(), error);

	 if ( error ) {
	     if ( error>0 ) {
		 MM_LOG( LOG_ERROR, "Build item free failed (%d): '%s'",
			 error,
			 ptrItem->Str( 1 ) );
	     }
	 }
	 else {
	     itemAllOk = true;
	     estSizeKb += uSize;

	     strBasicName = ((gString*)ptrItem->GetObjectPtr( 7 ))->Str();

	     MM_LOG( LOG_INFO, "Check OK: %9u Kb %s",
		     uSize,
		     ptrMeta->Str() );

	     if ( (linkConf.freeCounter % 100)==0 ) {
		 MM_LOG( LOG_INFO, "Processing (%d), now <%s>",linkConf.freeCounter,strBasicName);
	     }

	     oprint("L\t%s\n",strBasicName);
	 }
     }

     nErroredItems += (itemAllOk==false);
 }// end FOR play items

 char strMinutes[ 256 ];
 sprintf(strMinutes, " (%d\")", linkConf.ApproxMinutes());
 if ( linkConf.ApproxMinutes()==0 ) {
     strMinutes[ 0 ] = 0;
 }

 MM_LOG( LOG_INFO, "Processed (%d), %d item%s errored, ~%u Mb (%u Kb), secs: %d%s",
	 linkConf.freeCounter,
	 nErroredItems, nErroredItems==1 ? "s" : "",
	 jcl_iso_size_est( estSizeKb, estSectors, &linkConf ),
	 estSizeKb,
	 linkConf.totalSeconds,
	 strMinutes );

 if ( JCL_DEBUG_LEVEL>9 ) {
#if 0
     for (ptrElem=plays.items.StartPtr(); ptrElem; ptrElem=ptrElem->next) {
	 ptrItem = (gList*)ptrElem->me;
	 ptrItem->Show();
     }
#endif
 }
 else {
     for (ptrElem=plays.items.StartPtr(); ptrElem; ptrElem=ptrElem->next) {
	 ptrItem = (gList*)ptrElem->me;
	 iter++;
	 itemList = ptrItem->StartPtr();

	 secs = itemList->me->iValue;

	 oprint( "\
p %d	%s\n\
e	%s\n\
s	%d Kb, %d s, %s\n\
f	%s\n\
m	%s\n\
\n\
",
		 iter,
		 itemList->Str(),
		 itemList->next->Str(),
		 itemList->next->next->me->GetInt(), secs, jcl_track_shown_mmss( secs ),
		 itemList->next->next->next->Str(),
		 itemList->next->next->next->next->Str());
     }
 }

 DBGPRINT("! secs=%d\n\n",ptrItem->StartPtr()->me->iValue);
 return 0;
}


int jcl_anyread_playlist (sCrossLinkConf& linkConf, gString& sFile, sInPlaylist& plays)
{
 int error( 0 );
 int playType;
 gString sExt;

 plays.type = jcl_which_type( sFile, sExt );
 playType = (int)plays.type;
 DBGPRINT("DBG: playType: %d, playlist: %s\n", playType, sFile.Str());

 if ( playType ) {
     DBGPRINT("jcl_anyread_playlist (error=%d) %s, sExt={%s}: type=%d\n",
	      error,
	      sFile.Str(),
	      sExt.Str(),
	      plays.type);
     error = jcl_read_playlist( linkConf, sFile, plays );
 }
 else {
     // Use stdin if sFile is empty!
     gFileFetch input( sFile.IsEmpty() ? nil : sFile.Str(), -1 );
     input.SetFileReport( stderr );
     input.Fetch( sFile );
     error = input.IsOpened()==false;
     DBGPRINT("DBG: input.IsOpened? %c, input.aL.N()=%u\n",
	      ISyORn( input.IsOpened() ),
	      input.aL.N());
     if ( error ) return 2;
     error = jcl_read_rawlist( linkConf, input.aL, plays );
 }
 DBGPRINT("jcl_anyread_playlist error: %d, items.N()=%u\n",
	  error,
	  plays.items.N());

#if 0
 for (gElem* ptrElem=plays.items.StartPtr(); ptrElem; ptrElem=ptrElem->next) {
     char* strName( ptrElem->Str() );
     gFileStat aStat( strName );
     bool fileExists( aStat.HasStat() );
     printf("OK? %c %s\n", ISyORn( fileExists ), strName);
 }
#endif
 return error;
}


int jcl_read_rawlist (sCrossLinkConf& linkConf, gList& lines, sInPlaylist& plays)
{
 gElem* ptrElem( lines.StartPtr() );
 unsigned pos;
 unsigned idxInfo( 1 ), maxIdx( lines.N() );
 int value;
 int size( -1 );
 int countUnsupported( 0 );
 sListingConf support;
 const char* strMime( nil );
 sFileTypeSupport* ptrSup;

 for ( ; ptrElem; ptrElem=ptrElem->next, idxInfo++) {
     gString sLine( ptrElem->Str() );
     char lastChr( sLine[ sLine.Length() ] );

     // Ignore dirs:
     if ( lastChr=='/' || lastChr=='\\' ) continue;

     if ( sLine[ 1 ]!='#' ) {
	 gString sDir;
	 gString sExtInf;
	 gString sDot;
	 t_uchar fixChr( '\0' );

	 for (pos=sLine.Length(); pos>0; pos--) {
	     fixChr = sLine[ pos ];
	     if ( fixChr==JCL_ASCII01 || fixChr==27 || fixChr==127 ) {
		 sLine.Delete( pos );
		 pos = sLine.Length();
	     }
	 }

	 pos = sLine.Find("<location>", true);  // xspf playlists
	 if ( pos ) {
	     sLine.Delete( 1, pos + strlen( "<location>" ) - 1 );
	     pos = sLine.Find( "file:///", true );
	     if ( pos ) {
		 sLine.Delete( 1, pos + strlen("file:///") - 1 );
		 sLine.Trim();
	     }
	     pos = sLine.FindBack( "</" );
	     if ( pos > 4 ) {
		 sLine.Delete( pos );
	     }
	     sLine.Trim();
	     uri_to_ucs4( sLine, false );
	 }

#ifdef SUPPORT_RAW_WPL
	 // This won't work with 'list_musco.txu' raw files
         gString sNew;
	 if ( NameFromWPL( sLine, ---, sNew )!=-1 ) {
	     sLine = sNew;
	 }
#else
	 NameFromRawWPL( sLine );
#endif

	 pos = sLine.FindBack( '.' );
	 if ( pos ) {
	     sDot.CopyFromTo( sLine, pos );
	     sDot.DownString();

	     ptrSup = &support.SupportByExt( sDot.Str() );
	     strMime = support.defaultExtensionSupport[ ptrSup->idxDef ].mimeType;

	     value = atoi( sLine.Str() );
	     size = value;
	     pos = sLine.FindBack( '\t' );
	     if ( pos ) {
		 sLine.Delete( 1, pos );
		 sLine.Trim();
	     }

	     DBGPRINT("DBG: %d byte(s), %s\n",
		      size,
		      sLine.Str());
	     DBGPRINT("DBG:	idxDef=%d, supported=%d, strExts={%s}, MIME <%s>\n",
		      ptrSup->idxDef,
		      ptrSup->supported,
		      ptrSup->strExts,
		      support.defaultExtensionSupport[ ptrSup->idxDef ].mimeType);

	     if ( strMime && strncmp( strMime, "audio", 5 ) ) {
		 if ( sLine.Find( "<" ) || sLine.Find( ">" ) ) continue;

		 countUnsupported++;
		 if ( ptrSup->idxDef ) {
		     if ( countUnsupported >= 10 ) {
			 if ( countUnsupported <= 10 ) {
			     SP_LOG( LOG_NOTICE, "Ignored, not supported (and many more...) <%s> %s",
				     strMime,
				     sLine.Str() );
			 }
		     }
		     else {
			     SP_LOG( LOG_NOTICE, "Ignored, not supported: <%s> %s",
				     strMime,
				     sLine.Str() );
		     }
		 }
		 else {
		     if ( countUnsupported <= 10 ) {
			 SP_LOG( LOG_WARN, "Ignored, unknown extension: %s",sLine.Str() );
		     }
		 }
		 continue;
	     }

	     gList* ptrNew( jcl_new_item( linkConf,
					  sDir,
					  sLine,
					  0,
					  sExtInf,
					  nil,
					  size ) );
	     ASSERTION(ptrNew,"ptrNew");
	     plays.items.AppendObject( ptrNew );

	     // Show progress...
	     if ( linkConf.showProgress ) {
		 if ( (idxInfo%100)==0 ) {
		     fprintf(stderr,"\rRead progress: %u / %u ",
			     idxInfo,
			     maxIdx);
		 }
	     }
	 }
	 else {
	     MM_LOG(LOG_ERROR, "No extension: %s", sLine.Str());
	 }
     }//end IF
 }//end FOR each line...

 if ( linkConf.showProgress ) {
     fprintf(stderr,"\rRead progress: 100%%%20s\n"," ");
 }

 return 0;
}


int jcl_build_stuff (sCrossLinkConf& linkConf, int idxCommand, sInPlaylist& plays)
{
 int error( 0 );
 int size;
 int countErrors( 0 );
 bool executed( false );

 t_uint32 thisBlockKb, countBlockKb( 0 );
 t_uint32 netKb( 0 );
 t_uint32 estSectors( 0 );
 t_uint32 ruledSectors( 330000 );  // estimated CD number of sectors
 float percentInUse( 0.0 );

 gElem* ptrElem( plays.items.StartPtr() );
 gString* ptrNames;
 gString sCommandToError( linkConf.dumpCommandsToErrorFile ? linkConf.sCommandToError.Str() : (char*)"\0" );

 FILE* fIn( nil );

 char lineBuf[ 1024 ];

 ASSERTION(idxCommand<sCrossLinkConf::e_OS_unused,"idxCommand<e_OS_unused");
 sCrossLinkConf::eOScommand osCommand( idxCommand<0 ? sCrossLinkConf::e_OS_ln : (sCrossLinkConf::eOScommand)idxCommand );

 /*
	according 'man mkisofs':

	For a 650Mb CD, the allocation block is 10Kb, for a  4.7Gb
	DVD it will be about 70Kb.

	The  maximum  number  of  files  in an HFS volume is about
	65500 - although the real limit will be somewhat less than this.

	<<<

	The minimum allocation is thereby 100 Kb.
 */

#if 0
  for (gElem* ptrDebug=plays.items.StartPtr(); ptrDebug; ptrDebug=ptrDebug->next) {
      static int songCount;
      int debugCount( 1 );
      songCount++;
      gElem* ptrItem( ((gList*)ptrDebug->me)->StartPtr() );
      for (gElem* ptrThis=ptrItem;  ptrThis; ptrThis=ptrThis->next, debugCount++) {
	  printf("    %03d.%02d : %s\n", songCount, debugCount, ptrThis->Str());
      }
  }
#endif

 for ( ; ptrElem; ptrElem=ptrElem->next) {
     gString sCmd( linkConf.sOScommands[ osCommand ] );
     gList* ptrItem( (gList*)ptrElem->me );

     ptrNames = (gString*)(ptrItem->GetObjectPtr( 7 ));

     sCmd.Trim();
     if ( sCmd.IsEmpty() ) continue;

     gParam sPair( *ptrNames, "\t" );
     gString sLeft( sPair.Str( 1 ) );
     gString sRight( sPair.Str( 2 ) );

     if ( sPair.N()!=2 ) continue;

     if ( sLeft.IsEmpty() || sRight.IsEmpty() ) continue;

     jcl_consolidate_to_slash( sLeft.Find( '/' )>0, sLeft );
     jcl_consolidate_to_slash( sRight.Find( '/' )>0, sRight );

     sCmd.Add( " " );
     sCmd.Add( '"' );
     sCmd.AddString( sLeft );
     sCmd.Add( '"' );

     sCmd.Add( " " );

     sCmd.Add( '"' );
     sCmd.AddString( sRight );
     sCmd.Add( '"' );

     sCmd.AddString( sCommandToError );

     DBGPRINT_MIN("DBG: sCommandToError={%s}, osCommand=%d\n",sCommandToError.Str(),(int)osCommand);
     printf("\n%s\n",sCmd.Str());

     lineBuf[ 0 ] = 0;

     // Because on Linux we can safely read stderr output: " >2 ...file" !
     fIn = popen( sCmd.Str(), "r" );
     executed = fIn!=nil;
     if ( fIn )  {
	 fgets( lineBuf, sizeof(lineBuf)-1, fIn );
	 fclose( fIn );
     }

     if ( executed ) {
	 gString sErrFile( jcl_file_from_outfile( sCommandToError ) );
	 if ( sErrFile.Length() ) {
	     lineBuf[ 0 ] = 0;
	     fIn = fopen( sErrFile.Str(), "rt" );
	     if ( fIn ) {
		 fgets( lineBuf, sizeof(lineBuf)-1, fIn );
		 fclose( fIn );
	     }
	     error = lineBuf[ 0 ]!=0;
	     if ( error ) {
		 unsigned pos;
                 gString sError( lineBuf );
		 sError.Delete( sError.Find( '\n' ) );
		 pos = sError.FindBack( ':' );
		 error = pos>1;
		 if ( error ) {
		     sError.Delete( 1, error );
		     sError.Trim();
		     MM_LOG( LOG_ERROR, "File error: %s (%s)",
			     sRight.Str(),
			     sError.Str() );
		     countErrors++;
		 }
	     }
	 }
     }
 }

 if ( plays.checks.atNames ) {
     printf("\n\n--\t#%u\n",plays.checks.atNames->N());

     for (ptrElem=plays.checks.atNames->StartPtr(); ptrElem; ptrElem=ptrElem->next) {
	 size = ptrElem->me->iValue;
	 if ( size<0 )
	     thisBlockKb = 0;
	 else
	     thisBlockKb = (t_uint32)size;
	 printf("n\t%9u Kb  %s\n",
		thisBlockKb,
		ptrElem->Str());

	 netKb += thisBlockKb;
	 countBlockKb += jcl_usize_iso( thisBlockKb*1024, &linkConf );
     }
 }

 unsigned mbUsed( jcl_iso_size_est( countBlockKb, estSectors, &linkConf ) );

 printf("-- total estimated size: %u Mb (net: %u Mb)\n",
	mbUsed,
	jcl_usize_mb( netKb ));

 J_DUMP("Kb: ISO %u Kb, neto size %u Kb, sectors: %u\n",
	countBlockKb,
	netKb,
	estSectors);

 if ( estSectors > ruledSectors ) {
     if ( netKb > 360000 ) {
	 MM_LOG( LOG_WARN, "Exceeded size, ISO %u Kb, neto size %u Kb, does not fit into 700Mb CD",
		 countBlockKb,
		 netKb);
	 ruledSectors = 360000;
     }
     else {
	 MM_LOG( LOG_NOTICE, "Exceeded size, ISO %u Kb, neto size %u Kb, it does not fit into a 650Mb CD",
		 countBlockKb,
		 netKb);
     }
 }
 else {
     percentInUse = (float)estSectors * 100.0 / (float)ruledSectors;
     if ( countBlockKb ) {
	 MM_LOG( LOG_INFO, "ISO %u Kb, neto size %u Kb, done volume (%s%0.3f%%)",
		 countBlockKb,
		 netKb,
		 "~",
		 percentInUse);
     }
 }

 error = countErrors!=0;
 return error;
}


int jcl_build_crosslink (sCrossLinkConf& linkConf, sInPlaylist& plays)
{
 int idxCommand( (int)sCrossLinkConf::e_OS_ln );
 int error( jcl_build_stuff( linkConf, idxCommand, plays ) );

 DBGPRINT("DBG: jcl_build_crosslink returned %d\n",error);
 return error;
}


int jcl_build_copy (sCrossLinkConf& linkConf, sInPlaylist& plays)
{
 int idxCommand( (int)sCrossLinkConf::e_OS_Copy );
 int error( jcl_build_stuff( linkConf, idxCommand, plays ) );

 DBGPRINT("DBG: jcl_build_copy returned %d\n",error);
 return error;
}


int jcl_dump (sCrossLinkConf& linkConf, sInPlaylist& plays, gList& outlist)
{
 int iter( (int)outlist.N() );
 int maxElems( linkConf.maxCounter ? linkConf.maxCounter : MAX_INT16_I );
 int secs( -1 );
 gElem* ptrElem( plays.items.StartPtr() );
 gList* ptrItem;
 gString* ptrName;

 for ( ; ptrElem; ptrElem=ptrElem->next) {
     if ( iter >= maxElems ) break;
     ptrItem = (gList*)ptrElem->me;

     ptrName = (gString*)ptrItem->GetObjectPtr( 1 );
     secs = ptrName->iValue;

     if ( ptrName->Length() ) {
	 iter++;
	 outlist.Add( *ptrName );
	 outlist.CurrentPtr()->me->iValue = secs;
     }
 }//end FOR
 return 0;
}


int jcl_generated (sCrossLinkConf& linkConf, sInPlaylist& plays, int mask, gList& outlist)
{
 int iter( (int)outlist.N() );
 int maxElems( linkConf.maxCounter ? linkConf.maxCounter : MAX_INT16_I );
 int secs( -1 );
 unsigned idxInfo( 0 ), maxIdx( plays.items.N() );
 gElem* ptrElem( plays.items.StartPtr() );
 gList* ptrItem;
 gString* ptrName;
 char newLine[ 4 ];
 char oneLine[ 4096 ];
 char* strExt;

 strcpy( newLine, "\r\n" );

 for ( ; ptrElem; ptrElem=ptrElem->next) {
     idxInfo++;
     if ( iter >= maxElems ) break;
     ptrItem = (gList*)ptrElem->me;

     ptrName = (gString*)ptrItem->GetObjectPtr( 1 );
     secs = ptrName->iValue;
     strExt = ptrItem->Str( 2 );
     if ( secs>0 ) {
	 if ( jcl_valid_seconds_m3u( strExt, 0 ) ) {
	     snprintf(oneLine,
		      sizeof( oneLine )-1,
		      "#EXTINF:%s%s%s%s",
		      strExt,
		      newLine,
		      ptrName->Str(),
		      newLine);
	 }
	 else {
	     snprintf(oneLine,
		      sizeof( oneLine )-1,
		      "#EXTINF:%d,%s%s%s%s",
		      secs,
		      strExt,
		      newLine,
		      ptrName->Str(),
		      newLine);
	 }
     }
     else {
	 snprintf(oneLine,
		  sizeof( oneLine )-1,
		  "%s%s",
		  ptrName->Str(),
		  newLine);
     }

     if ( mask ) {
	 // Do sort:
	 gString* newObj( new gString( oneLine ) );
	 ASSERTION(newObj,"newObj");
	 if ( outlist.InsertOrderedUnique( newObj )==-1 ) {
	     delete newObj;
	 }
	 else {
	     iter++;
	     outlist.CurrentPtr()->me->iValue = secs;
	 }
     }
     else {
	 // No sort:
	 iter++;
	 outlist.Add( oneLine );
	 outlist.EndPtr()->me->iValue = secs;
     }

     if ( linkConf.showProgress ) {
	 if ( (idxInfo%100)==0 ) {
	     fprintf(stderr,"\rList progress: %u / %u ",
		     idxInfo,
		     maxIdx);
	 }
     }
 }

 if ( linkConf.showProgress ) {
     fprintf(stderr,"\rList progress: 100%%%20s\n"," ");
 }
 return 0;
}


gString* jcl_add_ordered (const char* strName, int value, gList& listed)
{
 int code;
 gString* ptrNew( new gString( (char*)strName ) );
 ASSERTION(ptrNew,"ptrNew");
 ptrNew->iValue = value;
 code = listed.InsertOrderedUnique( ptrNew );
 if ( code<0 ) {
     delete ptrNew;
     return nil;
 }
 return ptrNew;
}


int jcl_add_sort_paired (const char* strFile, bool doSort, int value, sPairedList& paired)
{
 int code( 0 );
 unsigned pos;
 gString* ptrNew( nil );
 gString sExt;

 if ( strFile==nil || strFile[ 0 ]==0 ) return -2;

 if ( doSort ) {
     ptrNew = jcl_add_ordered( strFile, value, paired.b );
     code = (ptrNew==nil);
     if ( code ) return -1;  // Already exists
 }
 else {
     ptrNew = new gString( (char*)strFile );
     ASSERTION(ptrNew,"ptrNew");
     paired.b.AppendObject( ptrNew );
     paired.b.EndPtr()->me->iValue = value;
 }

 pos = ptrNew->FindBack( '.' );
 if ( pos ) {
     sExt.Set( ptrNew->Str( pos-1 ) );
 }
 if ( sExt.Length() ) {
     sExt.DownString();
     code = jcl_add_ordered( sExt.Str(), 1, paired.a )==nil;
     if ( code ) {
	 // Increase number of occurrences of this extension:
	 (paired.a.CurrentPtr()->me->iValue)++;
     }
 }
 return 0;
}


int jcl_add_paired (const char* strFile, int value, sPairedList& paired)
{
 int code;
 unsigned pos;
 gString* ptrNew;
 gString sExt;

 if ( strFile==nil || strFile[ 0 ]==0 ) return -2;
 ptrNew = jcl_add_ordered( strFile, value, paired.b );
 code = (ptrNew==nil);
 if ( code ) return -1;  // Already exists

 pos = ptrNew->FindBack( '.' );
 if ( pos ) {
     sExt.Set( ptrNew->Str( pos-1 ) );
 }
 if ( sExt.Length() ) {
     sExt.DownString();
     code = jcl_add_ordered( sExt.Str(), 1, paired.a )==nil;
     if ( code ) {
	 // Increase number of occurrences of this extension:
	 (paired.a.CurrentPtr()->me->iValue)++;
     }
 }
 return 0;
}
////////////////////////////////////////////////////////////
int pure_file_status_code (gString& aFilePath, const bool isDir, t_uint32& uxPerm)
{
 int okStat;
 gFileStat aStat( aFilePath );

 uxPerm = isDir ? 0755 : 0644;

#ifdef linux
 okStat = aStat.HasStat() && aStat.statusL.blocks>0;
 if ( okStat ) {
     uxPerm = aStat.status.mode & 01777;
 }
#else
 okStat = aStat.HasStat();
#endif

 return okStat==0;
}
////////////////////////////////////////////////////////////
int file_status_code (gString& aFilePath, const bool isDir, t_uint32& uxPerm)
{
 int okStat;
 gFileStat aStat( aFilePath );

 uxPerm = isDir ? 0755 : 0644;

#ifdef linux
 okStat = aStat.HasStat() && aStat.statusL.blocks>0;
 if ( okStat ) {
     uxPerm = aStat.status.mode & 01777;
 }
#else
 okStat = aStat.HasStat();
 if ( okStat==true ) {
     t_uint32 perm( aStat.status.mode & 0777 );
     // emulate that a read-only file, in Windows, is private
     if ( (perm & 0600)==0600 ) {
     }
     else {
     	uxPerm = isDir ? 0700 : 0400;
     }
 }
#endif

 return okStat==0;
}
////////////////////////////////////////////////////////////
int basic_file_status (gString& aFilePath)
{
 t_uint32 uxPerm;
 return file_status_code( aFilePath, true, uxPerm );
}
////////////////////////////////////////////////////////////

