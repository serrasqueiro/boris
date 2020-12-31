// matches.cpp
//
//	Simple regular expressions with files


#include <sys/stat.h>

#include "matches.h"
#include "crosslink.h"
#include "auxvpl.h"

#include "lib_ilog.h"


extern gSLog mylog;
extern struct stat lastStat;

////////////////////////////////////////////////////////////
static int hex_uchar (t_uchar uChr)
{
 if ( uChr>='0' && uChr<='9' ) {
     return (int)(uChr - '0');
 }
 if ( uChr>='a' && uChr<='f' ) {
     return (int)(uChr - 'a') + 10;
 }
 if ( uChr>='A' && uChr<='F' ) {
     return (int)(uChr - 'A') + 10;
 }
 return -1;
}


gString* new_basename (gString& sPath)
{
 gString* newName( new gString( ima_basename( sPath.Str() ) ) );
 unsigned pos;

 ASSERTION(newName,"newName");
 newName->UpString();

 pos = newName->FindBack( '.' );
 if ( pos ) {
     newName->Delete( pos );
 }
 return newName;
}


gList* new_matched_file (gString& name, int mask)
{
 int code( -1 );
 unsigned pos;
 char aBuf[ 4096 ];
 char* strName( name.Str() );
 char* strCurrentPath;
 char* strDir;
 gList* listed( new gList );
 gElem* ptrElem;

 ASSERTION(listed,"listed");
 memset( aBuf, 0x0, sizeof(aBuf) );
 strCurrentPath = gio_getcwd( aBuf, sizeof(aBuf)-6 );

 // This function leaks the string at global pool:
 strDir = ima_dirname( strName );
 ASSERTION(strDir,"strDir");

 if ( strDir[ 0 ]==0 ) {
     strcat( strCurrentPath, gSLASHSTR );
     strDir = strCurrentPath;
 }

 gDir aDir( strDir );

 for (ptrElem=aDir.StartPtr(); ptrElem; ptrElem=ptrElem->next) {
     gString myName( ptrElem->Str() );
     code = myName.Match( name )==true;
     if ( code==0 ) {
	 pos = myName.Find( name );
	 if ( pos ) {
	     code = (pos-1 + name.Length() == myName.Length());
	 }
     }
     DBGPRINT("code: %d pos=%u\t%s\n",code,pos,ptrElem->Str());
     if ( code ) {
	 listed->Add( myName );
     }
 }

 return listed;
}


gList* new_get_files_at_dir (const char* strPath, char* strExcludedExts, int mask)
{
 gList* obj;
 gDir entries( (char*)strPath );
 gElem* ptrElem;
 bool avoidDotFiles( strExcludedExts==nil || strncmp( strExcludedExts, ".;", 2 )==0 );
 bool addThis( false );
 bool addDirs( (mask & 1)!=0 );

 if ( entries.lastOpError ) return nil;

 obj = new gList;
 ASSERTION(obj,"obj");

 for (ptrElem=entries.StartPtr(); ptrElem; ptrElem=ptrElem->next) {
     gString sThere( ptrElem->me->Str() );
     if ( sThere[ 1 ]=='.' ) {
	 addThis = (avoidDotFiles==false);
     }
     else {
	 addThis = true;
     }
     if ( sThere.Length() && addThis ) {
	 if ( sThere[ sThere.Length() ]==gSLASHCHR ) {
	     addThis = addDirs;
	 }

	 if ( addThis ) {
	     gString* ptrThere( new gString( sThere ) );
	     obj->InsertOrderedUnique( ptrThere );
	 }
     }
 }
 return obj;
}


iEntry* new_files_at_dir (const char* strPath, iEntry& extensions)
{
 int code;
 iEntry* obj;
 gDir entries( (char*)strPath );
 gElem* ptrElem;
 gElem* pSearch;
 bool avoidDotFiles( true );
 unsigned pos;
 char* strExt;

 if ( entries.lastOpError ) return nil;

 obj = new iEntry;
 ASSERTION(obj,"obj");

 for (ptrElem=entries.StartPtr(); ptrElem; ptrElem=ptrElem->next) {
     gString sThere( ptrElem->me->Str() );
     bool addThis( true );

     if ( sThere[ 1 ]=='.' ) {
	 addThis = (avoidDotFiles==false);
     }

     if ( sThere.Length() && addThis ) {
	 gString* ptrThere( new gString( sThere ) );
	 ASSERTION(ptrThere,"ptrThere");
     	 code = obj->InsertOrderedUnique( ptrThere );
	 if ( code==-1 ) {
	     delete ptrThere;
	 }
	 else {
	     pos = sThere.FindBack( '.' );
	     if ( pos>1 ) {
		 strExt = sThere.Str( pos-1 );
		 gString sDown( strExt );
		 sDown.DownString();

		 for (pSearch=extensions.StartPtr(); pSearch; pSearch=pSearch->next) {
		     if ( ((gString*)pSearch->me)->Match( sDown ) ) {
			 break;
		     }
		 }

		 if ( pSearch==nil ) {
		     extensions.Add( sDown );
		     pSearch = extensions.EndPtr();
		 }
		 pSearch->me->iValue++;
	     }
	 }
     }// added entry!
 }
 return obj;
}


gList* new_files_there (const char* strPath, gList& candidates, gString& usedExtension)
{
 gList* obj( new_get_files_at_dir( strPath, nil, 0 ) );

 usedExtension.SetEmpty();
 if ( obj ) {
     // usedExtension
 }

 return obj;
}

////////////////////////////////////////////////////////////
gList* new_rename_rules (gList& input, int mask, gString& returnMsg)
{
 bool seqProvided( false );
 int error( 0 );
 int mini( MAX_INT16_I ), maxi( -1 ), value( -1 );
 int thisVal( -1 );
 gList* obj( new gList );
 gElem* ptrElem( input.StartPtr() );
 char chr;

 ASSERTION(obj,"obj");

 for ( ; ptrElem; ptrElem=ptrElem->next) {
     gString* ptrStr( (gString*)ptrElem->me );
     ptrStr->Trim();
     if ( ptrStr->Length() ) {
	 obj->Add( *ptrStr );
	 obj->EndPtr()->me->iValue = value = atoi( ptrStr->Str() );
	 if ( value < mini ) mini = value;
	 if ( value > maxi ) maxi = value;
     }
 }

 if ( value>0 && maxi>0) {
     // Check again if there are holes in the values
     gInt* vals( new gInt[ maxi+1 ] );

     ASSERTION(vals,"vals");
     for (ptrElem=obj->StartPtr(); ptrElem; ptrElem=ptrElem->next) {
	 vals[ ptrElem->me->iValue ].Incr();
     }
     for (value=mini; value<=maxi; value++) {
	 thisVal = vals[ value ].GetInt();
	 if ( thisVal != 1 ) {
	     returnMsg.Set( (char*)(thisVal < 1 ? "Unsequenced numbers" : "Invalid sequence") );
	     break;
	 }
	 seqProvided = true;
     }
     delete[] vals;
 }

 error = returnMsg.Length()>0;
 if ( error==0 ) {
     // Two basic cases:
     //		- sequence number is provided,
     //		- ...or not.

     if ( seqProvided ) {
	 // Normalize input provided, thus excluding sequence number

	 for (ptrElem=obj->StartPtr(); ptrElem; ptrElem=ptrElem->next) {
	     gString* ptrStr( (gString*)ptrElem->me );
	     gString sCopy;

	     for (unsigned pos=0; (chr = ptrStr->Str()[ pos ])!=0; pos++) {
		 if ( chr<'0' || chr>'9' ) {
		     if ( chr<'A' ) pos++;
		     sCopy.Set( ptrStr->Str( pos ) );
		     break;
		 }
	     }
	     if ( sCopy.Length() ) {
		 sCopy.Trim();
		 thisVal = (int)sCopy.Length();
		 if ( thisVal < 2 ) {
		     MM_LOG( LOG_WARN, "Too short {%s}", ptrStr->Str());
		     if ( thisVal < 1 ) {
			 returnMsg.Set( "Name too short" );
		     }
		 }
		 *ptrStr = sCopy;
	     }
	 }// end FOR
     }
 }

 return obj;
}


int add_to_list (gElem* ptrIn, gList& out)
{
 int error( 0 );

 if ( ptrIn==nil ) return 0;
 for (gElem* pElem=ptrIn; pElem; pElem=pElem->next) {
     gStorage* newObj( pElem->me->NewObject() );
     ASSERTION(newObj,"newObj");
     if ( newObj->IsString() ) {
	 ((gString*)newObj)->Set( pElem->me->Str() );
     }
     else {
          if ( newObj->Kind()==gStorage::e_List ) {
		((gList*)newObj)->CopyList( *((gList*)pElem->me) );
	  }
	  else {
		error = -1;
	  }
     }
     out.AppendObject( newObj );
 }
 return error;  // well, not quite an error, but non-zero when unhandled!
}


gList* copy_names (gList& copy)
{
 gList* result( new gList );
 gElem* pElem;
 gString* myStr;

 ASSERTION(result,"result");

 for (pElem=copy.StartPtr(); pElem; pElem=pElem->next) {
     myStr = (gString*)pElem->me;
     result->Add( *myStr );
     result->EndPtr()->iValue = pElem->iValue;
     result->EndPtr()->me->iValue = pElem->me->iValue;
 }
 return result;
}


gList* join_lists (gList& L1, gList& L2)
{
 gList* result( new gList );
 ASSERTION(result,"result");
 add_to_list( L1.StartPtr(), *result );
 add_to_list( L2.StartPtr(), *result );
 return result;
}


char* does_match_extension (gString& s, const char* strExt)
{
 gString sExt( (char*)strExt );
 int pos( (int)s.Find( sExt, true ) );
 if ( pos==0 ) return nil;
 if ( pos + sExt.Length() >= s.Length() ) {
    return s.Str( pos );
 }
 return nil;
}

////////////////////////////////////////////////////////////
sSimilarMedia::sSimilarMedia ()
    : pList( nil )
{
}


sSimilarMedia::~sSimilarMedia ()
{
 delete pList;
}


void sSimilarMedia::AddNames (gList& copy)
{
 for (gElem* pElem=copy.StartPtr(); pElem; pElem=pElem->next) {
     names.Add( pElem->Str() );
     names.EndPtr()->me->iValue = pElem->me->iValue;
 }
}


bool sSimilarMedia::Analysis ()
{
 gElem* pElem;
 gElem* pFind;
 int upDiff;
 int diff;
 int idx( 0 );
 int secs, thisSec;
 short idxStat( 0 );
 gString* myStr;
 gString* newName;

 delete pList;
 pList = copy_names( names );

 // Cache items
 for (pElem=pList->StartPtr(); pElem; pElem=pElem->next) {
     idx++;
     myStr = (gString*)pElem->me;
     newName = new_basename( *myStr );
     if ( baseNames.InsertOrderedUnique( newName )==-1 ) {
	 fprintf(stderr, "Ignoring: %s (%s)\n", newName->Str(), myStr->Str());
	 delete newName;
     }
     else {
	 baseNames.CurrentPtr()->iValue = idx;
     }
 }

 // Search for similar times

 for (upDiff=0; upDiff<4; upDiff++) {
     for (pElem=pList->StartPtr(); pElem; pElem=pElem->next) {
	 secs = pElem->me->iValue;
	 if ( secs>0 ) {
	     for (pFind=pElem->next; pFind; pFind=pFind->next) {
		 thisSec = pFind->me->iValue;
		 if ( thisSec ) {
		     diff = abs( secs - thisSec );
		     if ( diff==upDiff ) {
			 stats[ idxStat ].Add( pElem->Str() );
			 stats[ idxStat ].EndPtr()->me->iValue = secs;
			 stats[ idxStat+1 ].Add( pFind->Str() );
			 stats[ idxStat+1 ].EndPtr()->me->iValue = thisSec;
		     }
		 }
	     }
	 }
     }//end FOR elements

     idxStat += 2;
 }//end upDiff

 return true;
}


void sSimilarMedia::Show ()
{
 gElem* pElem;
 gElem* pFind;
 int upDiff;
 short idxStat( 0 );

 for (upDiff=0; upDiff<4; upDiff++) {
     pFind = stats[ idxStat+1 ].StartPtr();

     if ( pFind ) {
	 printf("Diff: %ds --\n", upDiff);
	 for (pElem=stats[ idxStat ].StartPtr(); pElem; pElem=pElem->next) {
	     printf("%d +/- %ds :::\n%s\n%s\n\n",
		    pElem->me->iValue, upDiff,
		    pElem->Str(),
		    pFind->Str());
	     pFind = pFind->next;
	 }
     }

     idxStat += 2;
 }

 printf("---\n\nCACHE:\n");

 for (pElem=baseNames.StartPtr(); pElem; pElem=pElem->next) {
     printf("idx %d\t%s\n",
	    pElem->iValue,
	    pElem->Str());
 }
}


char* uri_to_ucs4 (gString& sURI, bool forceBackslash)
{
 t_uchar uChr;
 int idx( 1 ), len( sURI.Length() );
 int h1, h2;
 gString sCopy;

 // Returns null if something is weird

#ifdef iDOS_SPEC
 forceBackslash = true;
#endif

 for ( ; (uChr = sURI[ idx ])!=0 && idx<=len; idx++) {
     if ( forceBackslash && uChr=='/' ) {
	 uChr = '\\';
     }
     if ( uChr=='%' ) {
	 h1 = hex_uchar( sURI[ idx+1 ] );
	 h2 = hex_uchar( sURI[ idx+2 ] );
	 if ( h1>=0 && h2>=0 ) {
	     h1 <<= 4;
	     h1 |= h2;
	     idx += 2;
	     uChr = (t_uchar)h1;
	 }
     }
     if ( uChr<' ' || uChr==127 ) return nil;
     sCopy.Add( uChr );
 }

 sURI = sCopy;
 return sURI.Str();
}


char* str_slash (const char* str, char& resultChr)
{
 resultChr = 0;

 for (char chr; (chr = *str)!=0; str++) {
     if ( chr=='\\' || chr=='/' ) {
	 resultChr = chr;
	 return (char*)str;
     }
 }
 return nil;
}


char* str_simpler_path (char* result)
{
 t_uchar left( '\0' );
 t_uchar uChr;

 if ( result==nil ) return nil;

 for (int iter=strlen( result ); iter>0; ) {
     iter--;
     uChr = (t_uchar)result[ iter ];
     if ( uChr==127 || uChr<=' ' ) {
	 result[ iter ] = 0;
     }
     else {
	 if ( iter ) {
	     for ( ; result[ iter ]=='\\' || result[ iter ]=='/'; ) {
		 left = result[ iter ];
		 result[ iter ] = 0;
		 iter--;
		 if ( iter<=0 ) {
		     result[ iter ] = left;
		     result[ iter+1 ] = 0;
		     return result;
		 }
	     }
	 }
	 return result;
     }
 }
 return result;
}


gString* trim_filepath (const char* strInput)
{
 // Applies a right trim on the provide path ('strInput')
 const char* str( strInput ? strInput : "\0" );
 char chr;
 gString* result( new gString( str ) );
 ASSERTION(result,"result");
 for (int idx=0; (chr = str[ idx ])!=0; idx++) {
     if ( chr<' ' ) {
	 (*result)[ idx+1 ] = '\0';
     }
 }
 return result;
}


int sub_str_compare (const char* str, const char* subStr)
{
 // Returns 1 if subStr is "similarly" matching.

 // It does not distinguish slashes/ back-slashes

 if ( str==nil || subStr==nil ) return -1;
 if ( *subStr==0 ) return 0;

 for (char chr, sub; (chr = *str)!=0; str++, subStr++) {
     sub = *subStr;
     if ( sub==0 ) return 1;

     if ( sub=='\\' ) sub = '/';
     if ( chr=='\\' ) chr = '/';

     if ( chr!=sub ) return 0;
 }

 return 0;
}


char* slash_or_backslash (const char* str)
{
 static char smallBuf[ 4 ];

 smallBuf[ 0 ] = 0;
 if ( str && str[ 0 ] ) {
     if ( strstr( str, "/" ) ) {
	 strcpy( smallBuf, "/" );
     }
     else {
	 strcpy( smallBuf, "\\" );
     }
 }
 return smallBuf;
}

////////////////////////////////////////////////////////////
iString* new_stat_criteria (sSortBy& criteria, t_uint16 type, gString& sFilePath)
{
 int error;
 iString* newObj( new iString( sFilePath.Length() + 64, '\0' ) );
 char by[ 60 ];
 t_uint32 stamp( 0 );
 off_t size( 0 );

 int byCode( criteria.byCode & 0x0FFF );
 bool isSymLink( false );

 ASSERTION(newObj,"newObj");
 by[ 0 ] = 0;

#ifdef iDOS_SPEC
 error = stat( sFilePath.Str(), &lastStat )!=0;
#else
 // link status returns current timestamp if linked file does not exist
 // so, we use:	lstat( sFilePath.Str(), &lastStat ),
 // although we do not check the destination file, we signal it with '0' (zero)
 // so that the order will not include it at the top when sorting ascently.

 error = lstat( sFilePath.Str(), &lastStat )!=0;
 isSymLink = error==0 && S_ISLNK( lastStat.st_mode )!=0;
#endif

 size = lastStat.st_size;

 switch ( byCode ) {
 case 1:
     stamp = lastStat.st_atime;
     if ( type ) {
	 gDateTime dttm( stamp );
	 sprintf(by, "A" JCL_ASCII01S "%04u-%02u-%02u %02u:%02u:%02u.",
		 dttm.year, dttm.month, dttm.day,
		 dttm.hour, dttm.minu, dttm.sec);
     }
     else {
	 sprintf(by, "A" JCL_ASCII01S "%020lu", (unsigned long)stamp);
     }
     break;

 case 2:
     stamp = lastStat.st_mtime;
     if ( type ) {
	 gDateTime dttm( stamp );
	 sprintf(by, "M" JCL_ASCII01S "%04u-%02u-%02u %02u:%02u:%02u.",
		 dttm.year, dttm.month, dttm.day,
		 dttm.hour, dttm.minu, dttm.sec);
     }
     else {
	 sprintf(by, "M" JCL_ASCII01S "%020lu", (unsigned long)stamp);
     }
     break;

 case 4:
     stamp = lastStat.st_ctime;
     if ( type ) {
	 gDateTime dttm( stamp );
	 sprintf(by, "C" JCL_ASCII01S "%04u-%02u-%02u %02u:%02u:%02u.",
		 dttm.year, dttm.month, dttm.day,
		 dttm.hour, dttm.minu, dttm.sec);
     }
     else {
	 sprintf(by, "C" JCL_ASCII01S "%020lu", (unsigned long)stamp);
     }
     break;

 case 8:
     sprintf(by, "s" JCL_ASCII01S "%20lu", (unsigned long)size);
     break;

 default:
     break;
 }

 if ( isSymLink && by[ 0 ] ) {
     by[ 0 ] = '0';  // zero ASCII is less than 'A' or 'a', so, it will be the first sorted, or the last one.
 }

 DBGPRINT("DBG: error:%d symlink?%c %o (mode octal: %o) {%s} %s\n",
	  error,
	  ISyORn( isSymLink ),
	  lastStat.st_mode & S_IFLNK,
	  (unsigned)lastStat.st_mode,
	  by,
	  sFilePath.Str());

 sprintf(newObj->Str(), "%s%s%s",
	 by,
	 by[ 0 ] ? " " : "",
	 sFilePath.Str());

 if ( criteria.mainAscend==false ) {
     newObj->SetOrder( -1 );
 }

 return newObj;
}

////////////////////////////////////////////////////////////

