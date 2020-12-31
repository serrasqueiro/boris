// juicebox.cpp

#define thisProgramVersion "Version 2.0"
#define thisProgramCopyright "Prized Season & Sons"
#define thisProgramYear 2019
#define thisProgramCopy "This is free software (GPL)\n\
There is no warranty, not even for MERCHANTABILITY or\n\
FITNESS FOR A PARTICULAR PURPOSE."


#include <errno.h>

#include "lib_iobjs.h"
#include "lib_ilog.h"
#include "lib_ilambda.h"

#include "crosslink.h"
#include "matches.h"
#include "auxvpl.h"
#include "auxsfv.h"
#include "smart.h"


IMEDIA_DECLARE;

#define dprint(args...) printf(args)

#define ptm_release_pool gStorageControl::Self().DeletePool()

#define ptm_finit DBGPRINT("DBG: Objs undeleted: %d\n",gStorageControl::Self().NumObjs())


////////////////////////////////////////////////////////////
// Internal structs
////////////////////////////////////////////////////////////
struct sOptBox {
    sOptBox ()
	: level( 0 ),
	  debugLevel( 0 ),
	  maxDepth( maxAllowedDepth ),
	  areAll( false ),
	  doAppend( false ),
	  doCheck( false ),
	  doOnlyToMe( false ),
	  doOnlyPublic( false ),
	  permShow( 0 ),
	  zValue( 0 ) {
    }

    static const int maxAllowedDepth = 1000;

    int level;
    int debugLevel;
    int maxDepth;
    bool areAll;
    bool doAppend;
    bool doCheck;
    bool doOnlyToMe;
    bool doOnlyPublic;
    int permShow;  // 1: only to me; 2: to public; 0: normal
    int zValue;
    gString sValue;
    gList zReference;  // reference from sValue

    gString sBaseDir;
    gString sBase;  // base dir, simplified
    gString sLogFile;
    gString sConfigFile;
    gString sOutput, sOutExt;

    gString newLineCR;  // empty if not used (no '\r' before end of line)
    sSortBy sortBy;

    void SplitZValue (char* strSeparator) ;
};


////////////////////////////////////////////////////////////
// Globals
////////////////////////////////////////////////////////////
gSLog mylog;
t_uchar globalJclShortBuf[ 1024 ];
struct stat lastStat;

////////////////////////////////////////////////////////////
// Additional extern functions
////////////////////////////////////////////////////////////

extern int jcl_media_type_from_extension (gString& sExt) ;


// This one got here until this gets correctly done at libimedia library!
iEntry* jb_dir_normalized (bool complain, gDir& aDir)
{
 int error( 0 );
 unsigned pos;
 gElem* ptrIter;
 gList* pFailed( &aDir.errUnstatL );
 iEntry* pFound( nil );

 ASSERTION(pFailed,"pFailed");
#if 0
 printf("pFailed %d: ", pFailed->N()); pFailed->Show();
 if ( pFailed->N() ) {
     gString* p( (gString*)(pFailed->StartPtr()->me) );
     for (int ix=1; ix<=p->Length()+1; ix++) {
	 t_uchar chr( (*p)[ ix ] ); if ( chr<='~' ) printf("%c", chr<' ' ? '\n' : chr); else printf("[%02X]", chr);
     }
 }
#endif

 if ( complain ) {
     pFound = new iEntry;
 }

 if ( pFailed->N() ) {
     for (ptrIter=pFailed->StartPtr(); ptrIter; ptrIter=ptrIter->next) {
	 gString sName( ptrIter->Str() );
	 for ( ; ; ) {
	     pos = sName.Find( "a`" );
	     if ( pos ) {
		 sName.Delete( pos+1, pos+1 );
		 sName[ pos ] = 'à';
		 continue;
	     }
	     pos = sName.Find( "e\xB4" );  // Windows acute accent (´) is 0xB4
	     if ( pos ) {
		 sName.Delete( pos+1, pos+1 );
		 sName[ pos ] = 'é';
		 continue;
	     }
	     break;
	 }

	 gFileStat aStat( sName );
	 error = aStat.HasStat();
	 if ( error==0 ) {
	     // Remove leading path that is already assumed to be at aDir.sDirName:
	     int lenDir( (int)aDir.sDirName.Length() );
	     lenDir += lenDir>0;  // the slash also counts!
	     if ( lenDir < (int)sName.Length() ) {
		 gString sOnlyName( sName );
		 sOnlyName.Delete( 1, lenDir );
		 sName = sOnlyName;
	     }
	     // Finally add the adjusted name:
	     aDir.Add( sName );
	     DBGPRINT("DBG: Added to: %s {%s}\n", aDir.sDirName.Str(), aDir.EndPtr()->Str());
	 }
	 if ( pFound ) {
	     gString msg( sName.Length()+64, '\0' );
	     sprintf(msg.Str(), "%s: %s",
		     error ? "Unread entry" : "Corrected entry",
		     sName.Str());
	     pFound->Add( msg );
	 }
     }
 }
 return pFound;
}


////////////////////////////////////////////////////////////
// Helper functions
////////////////////////////////////////////////////////////
int print_help (const char* progStr)
{
 const char
     *msgHelp = "%s - %s\n\n\
Usage:\n\
        %s command [--help] [OPTION] [NAME ...]\n\
\n\
Commands are:\n\
   test         Just a test\n\
   ls           List audio files ('dir' lists everything)\n\
   dump         Dump playlist(s)\n\
   crosslink    Creates crosslink multimedia volumes\n\
   copy         Copies to a multimedia volume\n\
   generate     Build a playlist\n\
   generated    Build an almost random playlist\n\
   join-to      Join to playlist\n\
   compact      Compact multimedia directory\n\
   ren          Renames files\n\
   media-ren    Rename media files comprehensively\n\
   sfv-dump     Dump alternate CRC32 for files\n\
   sfv-check    Check SFV (alternate CRC32)\n\
\n\
Options are:\n\
   -h           This help (or --help / --version)\n\
   -v           Verbose (use twice, more verbose)\n\
   -a           All\n\
   --append     Append to output\n\
   -b X         Base dir\n\
   -c X         Use config file\n\
   -d N         Use debug level N\n\
   -f           Check that exists (or --file)\n\
   -l X         Log to file instead of stderr (or --log)\n\
   -o X         Output to file (or --output)\n\
   -z N         Use N (general purpose value)\n\
";

 printf(msgHelp,
	progStr,
	thisProgramVersion,
	progStr,
	progStr);
 return 0;
}


int print_version (const char* progStr)
{
 const char
     *msgVersion = "%s - %s\n\
\n\
Build \
005\
\n\
\n\
Written by Henrique Moreira.\n\
\n\
Copyright (C) %u %s.\n\
%s\n";

 printf(msgVersion,
	progStr,
	thisProgramVersion,
	thisProgramYear,thisProgramCopyright,
	thisProgramCopy);
 return 0;
}


int command_from_str (const char* cmdStr)
{
 int iter( 0 );
 const char* str;
 const cPair pairs[]={
	{ 1, "test" },
	{ 2, "ls" },
	{ 3, "dump" },
	{ 4, "crosslink" },
	{ 5, "copy" },
	{ 6, "compact" },
	{ 7, "generated" },
	{ 8, "ren" },
	{ 9, "del" },
	{ 10, "media-ren" },
	{ 11, "join-to" },
	{ 14, "generate" },
	{ 17, "dir" },
	{ 0, "-" },
	{ 19, "sfv-check" },
	{ 20, "sfv-dump" },
	{ 0, "-" },
	{ 0, "-" },
	{ 23, "smart-ren" },
	{ -1, nil },
	{ -1, nil },
	{ -1, nil }
 };

 if ( cmdStr==nil ) return -1;
 for ( ; (str = pairs[ iter ].str)!=nil; iter++) {
     if ( cmdStr[0]!=0 && strcmp( str, cmdStr )==0 ) return pairs[ iter ].value;
 }
 return -1;
}


int print_command_help (const char* progStr, const char* cmdStr)
{
 int cmdNr( command_from_str( cmdStr ) );

 const char* cmdHelp[]={
	nil,
	"\n\
\n\
A simple test\n\
",
	"[OPTIONS] [dir ...]\n\
\n\
List audio files.\n\
\n\
Options:\n\
  -f	Show only files that exist (those whose fstat is known)\n\
  -a	List all (recursive)\n\
\n\
  -z N	Format to show\n\
\n\
-z>=1000 shows all files, not only audio files.\n\
-z 0 or 1000 does not sort file list.\n\
\n\
See more options in 'dir' command help.\n\
",
	"[OPTIONS] [vpl_file|m3u_file ...]\n\
\n\
Dumps a playlist:\n\
	Comprehensively (default),\n\
	-z 1	in m3u format\n\
	-z 8	no duplicates\n\
	-z 9	ordered by occurrence\n\
\n\
When -o file.m3u is specified (m3u extension), output is done in DOS format.\n\
Note this command also handles m3u files as input.\n\
\n\
Other options:\n\
	-a	Show all entries, not only media files\n\
",
	"[-z 1] outdir [playlist_file ...]\n\
\n\
Where outdir is the output path where the crosslink multimedia volume\n\
is stored to.\n\
\n\
If -z 1 is used only dump is done, not the crosslink.\n\
\n\
Examples:\n\
	" gSLASHSTR "tmp" gSLASHSTR "musical mysongs.m3u other_list.vpl\n\
\n\
",
	"outdir [playlist_file ...]\n\
\n\
Where outdir is the output path to copy the multimedia volume.\n\
\n\
",  // <-- 5
	"dir [-z N]\n\
\n\
Compacts or renames files to be sequential.\n\
If -z 1000 is used, only suggestion is dumped, not the rename.\n\
If -z 999 is used, original directory order is to be used.\n\
Other values at -z, e.g. 3, allow starting sorting only at index 3.\n\
",  // <-- 6
	"[base-dir ...] [file ...]\n\
\n\
Generates an almost random playlist.\n\
\n\
Options:\n\
	-a	Do not randomize.\n\
	-z N	Only up to N entries.\n\
",  // <-- 7
	"[-z N] NAMES .\n\
or\n\
		[-z N] NAMES [file ...]\n\
\n\
Rename files at directory.\n\
NAMES suggest how files are renamed.\n\
%xx can be used as a character whose hex ASCII is xx.\n\
(Example: %20 is a space.)\n\
\n\
-z 1 only suggests how to rename, does not actually rename files.\n\
-a     is to be used exceptionally when '.' is selected as file\n\
       and user wants still to select all files to rename (not only\n\
       those who match the 'NAMES'.\n\
",  // <-- 8
	"[-z N] [file]\n\
\n\
Either stdin files listed are deleted, or the one entered at the arguments.\n\
\n\
-z 1 only displays files, not actually deletes them;\n\
-z 2 tries to match one file (if there are more than one, it bails out.)\n\
",  // <-- 9
	"[-z N] [path/file ...]\n\
\n\
Rules are applied from stdin for file(s) or directory provided;\n\
Current path is expected to contain media.\n\
\n\
-z 1 only displays renames, does not do anything else.\n\
\n\
NOTE: being tested!\n\
",  // <-- 10
	"[-z N] -o destination-file [source ...]\n\
\n\
Joins sources into playlist.\n\
If destination is '.' (dot), everything is appended to the existing playlist.\n\
",  // <-- 11
	"\
",  // <-- 12
	"\
",  // <-- 13
	"[base-dir ...] [file ...]\n\
\n\
Generates a playlist.\n\
\n\
Options:\n\
	-f	Check file exists\n\
	-a	Do not exclude duplicates\n\
	-z N	Only up to N entries.\n\
",  // <-- 14
	"\
",  // <-- 15
	"\
",  // <-- 16
	"[OPTIONS] [DIR ...]\n\
\n\
Lists dirs (-a recursively).\n\
\n\
Options are:\n\
  -f	Show only files that exist (those whose fstat is known)\n\
  -p 1  Show only files known to me (based on permissions)\n\
  -p 2  Show only files known to everyone\n\
  -p 3  Like '-p 1', but also check dirs\n\
  -a	List all (recursive)\n\
\n\
  -z N\n\
	1	aTime (access), or A/ a\n\
	2	mTime (modify), or M/ m\n\
	4	cTime (change), or C/ c\n\
	8	size, or S/ s\n\
	16	name, or N/ n\n\
Multiple sort not supported;\n\
but announcing e.g. M:100 forces max. number of files to be 100\n\
\n\
Notes about permissions:\n\
	read-only file in windows is considered only known to me.\n\
",  // <-- 17
	nil,
	"[OPTIONS] [SFV ...]\n\
\n\
Checks SFV file(s).\n\
\n\
Options are:\n\
  -f	Check whether files at SFV match dirs.\n\
  -a	Check whether all files exist.\n\
",  // <-- 19
	"[OPTIONS] [SFV ...]\n\
\n\
Dump CRC32, SFV file(s).\n\
",  // <-- 20
	"",  // 21
	"",  // 22
	"[OPTIONS] PATH EXPR1 by EXPR2\n\
\n\
Examples:\n\
	Substitute a (track) number and dot by the number and a blank:\n\
		juicebox smart-ren . %t. by \"%t \"\n\
",  // 23
	nil,
	nil,
	nil,
	nil
 };

 if ( cmdNr<0 ) return print_help( progStr );

 printf("%s %s\n\nUsage:\n\t%s %s %s",
	progStr,
	cmdStr,
	progStr, cmdStr,
	cmdHelp[ cmdNr ]);
 return 0;
}

////////////////////////////////////////////////////////////
// Aux functions
////////////////////////////////////////////////////////////
int ptm_prepare_custom_iso (int optTable, sIsoUni& data)
{
 int idx( 0 );
 char* myUcs8( data.customUcs8[ 3 ] );
 bool isInitialized( myUcs8[ 0 ]==0 );
 char hashed;
 t_uchar uChr;
 gUniCode* inUse( data.inUse );

 // ----> This function was copied almost 100% from boradb/src/processtext.cpp

 if ( isInitialized ) return -1;  // Nothing to do again

 ASSERTION(inUse,"inUse");
for ( ; idx<256; idx++) {
     if ( data.hashUcs16Eq[ idx ] ) {
         free( data.hashUcs16Eq[ idx ] );
     }
 }
 for (idx=0; idx<256; idx++) {
     data.hashUcs16Eq[ idx ] = (t_uchar*)calloc(idx < ' ' ? 2 : 4, sizeof(t_uchar));
 }

 for (idx=0; idx<' '; idx++) {
     myUcs8[ idx ] = 0;
 }

 for ( ; idx<127; idx++) {
     hashed = data.hashUcs8Custom[ idx ];
     myUcs8[ idx ] = hashed;
 }

 for ( ; idx<256; idx++) {
     hashed = data.hashUcs8Custom[ idx ];
     if ( hashed==-1 ) {
	 hashed = '~';
     }
     else {
	 uChr = (t_uchar)hashed;
	 hashed = inUse->hash256User[ gUniCode::e_Basic_Alpha26 ][ idx ];

	 if ( hashed==0 || hashed==-1 ) {
		 hashed = (char)idx;
	 }
	 else {
	     data.hashUcs16Eq[ idx ][ 0 ] = hashed;
	     data.hashUcs16Eq[ idx ][ 1 ] = 0;
	 }
     }
     myUcs8[ idx ] = hashed;
 }

 for (uChr='A'; uChr<='Z'; uChr++) {
     data.hashUcs16Eq[ uChr ][ 0 ] = uChr;
     ASSERTION(data.hashUcs16Eq[ uChr ][ 1 ]==0,"!iso (1)");
 }
 for (uChr='a'; uChr<='z'; uChr++) {
     data.hashUcs16Eq[ uChr ][ 0 ] = uChr;
     ASSERTION(data.hashUcs16Eq[ uChr ][ 1 ]==0,"!iso (2)");
 }
 for (uChr=47; uChr<='9'; uChr++) {
     idx = uChr==47 ? ' ' : (int)uChr;
     data.hashUcs16Eq[ idx ][ 0 ] = idx<'0' ? ' ' : idx;
     ASSERTION(data.hashUcs16Eq[ idx ][ 1 ]==0,"!iso (3)");
 }

 // German special chars, accepted:
 myUcs8[ 0xDF ] = 0xDF;		// 'LETTER SHARP S
 myUcs8[ 0xFF ] = 0;

 // also to the custom UCS8:
 data.hashUcs8Custom[ 0xDF ] = 0xDF;
 strcpy( (char*)data.hashUcs16Eq[ 0xDF ], "ss" );

 data.RefactorUcs16Eq();

 return 0;
}


void bora_iso_adjust ()
{
 ptm_prepare_custom_iso( 0, *ptrUniData );
}


int tracknr_indication (const char* str)
{
 int position( 0 );
 return jcl_tracknr_indication( str, position );
}


int media_type_from_path (const char* str)
{
 int audioType( -1 );
 int len;
 char* strExt;

 if ( str ) {
     for (len=strlen( str ); len>1; ) {
	 len--;
	 if ( str[ len ]=='.' ) {
	     strExt = (char*)str + len;
	     gString sExt( strExt );
	     audioType = jcl_media_type_from_extension( sExt );
	     return audioType;
	 }
     }
 }
 return audioType;
}


int name_to_subst_string (const char* strName, gString& result)
{
 char next, nextTwo;
 int count( 0 );
 int hex;

 if ( strName ) {
     for (char chr; (chr = *strName)!=0; strName++) {
	 next = *(strName + 1);
	 if ( chr=='%' && hex_char( next )>=0 ) {
	     nextTwo = *(strName + 2);
	     if ( hex_char( nextTwo )>=0 ) {
		 strName += 2;
		 hex = hex_char( next );
		 hex *= 16;
		 hex += hex_char( nextTwo );
		 chr = (char)hex;
	     }
	 }
	 result.Add( chr );
     }
 }
 DBGPRINT("DBG: name_to_subst_string {%s}\n",result.Str());
 return count;
}


int special_rename (const char* strPath, const char* str1, const char* str2)
{
 gString sName1( (char*)strPath );
 gString sName2;

 if ( strPath && strPath[ 0 ] ) {
     if ( sName1[ sName1.Length() ]!=gSLASHCHR )
	 sName1.Add( gSLASHSTR );
 }
 sName2 = sName1;
 sName1.Add( (char*)str1 );
 sName2.Add( (char*)str2 );
 return rename( sName1.Str(), sName2.Str() );
}


int random_listed (FILE* fOut, gList& listed, int nOutput, gList& indexes, gList& listedErrors, int& itemsShown)
{
 int count( 0 );
 int idx;
 int error( -1 );
 gString* myStr;
 const char* strChosen;
 char* strFile;
 gElem* ptrElem( listed.StartPtr() );
 bool incompatible( false );
 bool anyFileCheck( true );

 ASSERTION(fOut,"fOut");

 for ( ; ptrElem && anyFileCheck; ptrElem=ptrElem->next) {
     myStr = (gString*)ptrElem->me;
#ifdef iDOS_SPEC
     // If any slash (/) is found, we are here at Win32 systems, do not do any file check:
     incompatible = myStr->Find( '/' )>0;
     if ( incompatible )
	 anyFileCheck = false;
#else
     incompatible = myStr->Find( '\\' )>0;
     if ( incompatible ) {
	 anyFileCheck = false;
     }
#endif
 }

 for ( ; count<nOutput; count++) {
     gRandom any( indexes.N() );
     idx = any.GetInt()+1;
     if ( indexes.N()==0 ) break;
     DBGPRINT_MIN("count=%d < %d (#%u), idx=%d\n",
		  count, nOutput,
		  listed.N(),
		  idx);
     myStr = (gString*)listed.GetObjectPtr( indexes.GetInt( idx ) );
     strChosen = myStr->Str();
     fprintf(fOut,"%s",strChosen);
     indexes.Delete( idx, idx );
     itemsShown++;

     // Check if file exists
     gString sFilePath( myStr->Str( myStr->iValue ) );
     strFile = sFilePath.Str();
     jcl_wipe_tail_cr_nl( strFile );

     if ( anyFileCheck ) {
	 gFileStat aStat( strFile );
	 error = aStat.HasStat()==false;
	 if ( error ) {
	     listedErrors.Add( strFile );
	 }
     }
 }
 if ( anyFileCheck ) {
     if ( listedErrors.N() ) {
	 MM_LOG(LOG_WARN, "Not all files found (%u), %d",
		 listedErrors.N(),
		 count );
     }
 }
 if ( incompatible ) {
     MM_LOG(LOG_WARN, "Incompatible files format" );
 }
 return count;
}


int bulk_rename (gList& files, gList& substExpress, gList& news)
{
 gElem* ptrElem;
 gString* aName;
 gString* ptrSubst( (gString*)substExpress.StartPtr()->me );
 char* strExpress( ptrSubst->Str() );
 int doIt( 1 );
 int pos( -1 );
 int substLen( (int)ptrSubst->Length() );

 // files are the inputs;
 // news are the new names proposed;
 // substExpress the rules to rename.

 // Check if expression appears in all names:

 for (ptrElem=files.StartPtr(); ptrElem; ptrElem=ptrElem->next) {
     aName = (gString*)ptrElem->me;
     pos = (int)(aName->Find( strExpress ));
     if ( pos<=0 ) {
	 doIt = 0;
     }
     aName->iValue = pos;
 }

 DBGPRINT("{%s} doIt: %d\n",strExpress,doIt);
 if ( doIt ) {
     // Remove all strings found:
     for (ptrElem=files.StartPtr(); ptrElem; ptrElem=ptrElem->next) {
	 aName = (gString*)ptrElem->me;
	 gString sNew( *aName );
	 sNew.Delete( aName->iValue, aName->iValue + substLen -1 );
	 news.Add( sNew );
     }
     return 0;
 }

 news.CopyList( files );
 return 1;
}


int do_dir_ls (gString& sDir, int counter, FILE* fOut, FILE* fRepErr, sOptBox& opt, gList* ptrResult)
{
 static int complaints;

 int error( 0 );
 int anyError( 0 );
 unsigned uLen( 0 );
 unsigned pos;
 gElem* ptrIter;
 gList files;
 gList audios;
 gString sLastError;
 gString sPath( sDir );
 gString sDup;
 gString* myStr;
 const char* strNewLine( opt.newLineCR.Str() );

 bool isDot( false );
 bool unuse( false );

 const bool showAllFileTypes( opt.zValue>=1000 );
 const bool doSort( ((opt.zValue==0 || opt.zValue==1000))==false );
 const bool showMineOnly( opt.doOnlyToMe );
 const bool showPublicOnly( opt.doOnlyPublic );
 const bool checkExists( opt.doCheck==true || showMineOnly==true || showPublicOnly==true );
 const bool verbose( opt.level>=3 );

 sListingConf support;
 const char* strMime( nil );
 sFileTypeSupport* ptrSup;

 DBGPRINT("counter=%d, showAllFileTypes? %c, doSort? %c: %s\n",
	  counter,
	  ISyORn( showAllFileTypes ),
	  ISyORn( doSort ),
	  sDir.Str());

 if ( counter<0 ) {
     files.Add( sPath );
 }
 else {
     if ( counter>opt.maxDepth ) {
	 return 0;
     }
     if ( counter>=opt.maxAllowedDepth ) {
	 if ( complaints==0 ) {
	     complaints++;
	     fprintf(stderr, "Probably recursing too much... (within %d)\n", counter);
	 }
	 return 0;
     }

     uLen = sPath.Length();
     isDot = sPath.Match( "." );
     if ( isDot==false ) {
	 if ( uLen>0 && sPath[ uLen ]!=gSLASHCHR ) {
	     sPath.Add( gSLASHCHR );
	 }
	 sDup = sPath;
     }

     gDir aDir( sPath );
     iEntry* pCorrected( jb_dir_normalized( showAllFileTypes || verbose, aDir ) );

     if ( pCorrected ) {
	 for (gElem* ptr=pCorrected->StartPtr(); ptr; ptr=ptr->next) {
	     MM_LOG(LOG_WARN, "%s", ptr->Str());
	 }
	 delete pCorrected;
     }

     error = aDir.lastOpError;
     if ( error ) {
	 anyError = error;
	 sLastError.Set( strerror( errno ) );
	 fprintf(stderr,"Wrong dir: %s\n",sPath.Str());
     }

     bool isDir;

     for (ptrIter=aDir.StartPtr(); ptrIter; ptrIter=ptrIter->next) {
	 gString sName( sDup );
	 t_uint32 uxPerm( 0 );
	 bool doAdd( true );

	 sName.Add( ptrIter->Str() );
	 isDir = sName[ sName.Length() ]==gSLASHCHR;
	 if ( checkExists ) {
	     bool hasError( file_status_code( sName, isDir, uxPerm )!=0 );
	     doAdd = hasError==false;
	     if ( showMineOnly ) {
		 bool isPrivate( (uxPerm & 077)==0 );
		 if ( isDir && opt.permShow<=2 ) {
		     isPrivate = true;
		 }
		 doAdd = hasError==false && isPrivate;
	     }
	     else {
		 if ( showPublicOnly ) {
		     bool basicOk( (uxPerm & 07)!=0 );
		     if ( isDir ) {
			 basicOk = (uxPerm & (04 | 01))==(04 | 01);  // chmod a+rx ...dir
		     }
		     doAdd = hasError==false && basicOk;
		 }
	     }
	     DBGPRINT("#Show: %s octal=%o (mine-only? %c) {%s}\n", doAdd ? "(OK)" : "(-)", uxPerm, ISyORn( showMineOnly ), sName.Str());
	 }
	 if ( doAdd ) {
	     if ( isDir ) {
		 if ( opt.areAll ) {
		     do_dir_ls( sName, counter+1, fOut, fRepErr, opt, ptrResult );
		 }
	     }
	     else {
		 if ( doSort ) {
		     jcl_add_pname( sName, files );
		 }
		 else {
		     files.Add( sName );
		 }
	     }
	 }
     }
 }

 for (ptrIter=files.StartPtr(); ptrIter; ptrIter=ptrIter->next) {
     gString sDot;
     myStr = (gString*)ptrIter->me;

     unuse = showAllFileTypes==false;
     if ( unuse ) {
	 pos = myStr->FindBack( '.' );
	 if ( pos ) {
	     sDot.CopyFromTo( *myStr, pos );
	     sDot.DownString();
	     ptrSup = &support.SupportByExt( sDot.Str() );
	     strMime = support.defaultExtensionSupport[ ptrSup->idxDef ].mimeType;
	     unuse = strncmp( strMime, "audio", 5 )!=0;
	 }
     }
     if ( unuse ) {
	 myStr->Str()[ 0 ] = 0;
     }
 }

 ptrIter = files.StartPtr();

 for ( ; ptrIter; ptrIter=ptrIter->next) {
     myStr = (gString*)ptrIter->me;
     if ( myStr->Str()[ 0 ] ) {
	 if ( fOut ) {
	     /*
	     bool showMe( true );
	     if ( checkExists ) {
		 t_uint32 uxPerm;
		 showMe = file_status_code( *myStr, false, uxPerm )==0;
	     }
	     if ( showMe ) {
		 fprintf(fOut,"%s%s\n",myStr->Str(),strNewLine);
	     }
	     */
	     fprintf(fOut,"%s%s\n",myStr->Str(),strNewLine);
	 }
	 if ( ptrResult ) {
	     iString* newObj( new_stat_criteria( opt.sortBy, 1, *myStr ) );
	     if ( ptrResult->InsertOrderedUnique( newObj )==-1 ) {
		 delete newObj;
	     }
	     else {
		 if ( (int)(ptrResult->N()) > opt.sortBy.limit ) {
		     //printf("DBG: %u > %d, deleting: %s\n", ptrResult->N(), opt.sortBy.limit, ptrResult->Str( 1 )); ptrResult->Show();
		     ptrResult->Delete( 1, 1 );
		 }
	     }
	 }
     }
 }

 if ( anyError<=0 ) {
     MM_LOG(LOG_INFO, "ls %s%s",
	    sDir.Str(),
	    doSort ? " (sorted)" : "");
 }
 else {
     MM_LOG(LOG_ERROR, "ls %s%s %s",
	    sDir.Str(),
	    doSort ? " (sorted)" : "",
	    sLastError.Str());
 }
 return error;
}


extern int ipl_vpl_raw_into_parts (gList& entry, sVplPlaylist& playlist) ;

gElem* b_vpl_raw_to_parts (sVplPlaylist& playlist)
{
 gElem* ptrStart;
 gList* pList;
 const int nEntries( playlist.nEntries );

 for (int iter=1; iter<=nEntries; iter++) {
     pList = playlist.entries[ iter ];
     if ( pList ) {
	 ipl_vpl_raw_into_parts( *playlist.entries[ iter ], playlist );
	 ////printf("ITER: %d/%d: ", iter, nEntries); pList->Show();
     }
 }
 ptrStart = playlist.parts.StartPtr();

 for (gElem* ptrElem=ptrStart; ptrElem; ptrElem=ptrElem->next) {
     ASSERTION(ptrElem->me->Kind()==gStorage::e_List,"e_List");
     ////printf("PART: "); ptrElem->me->Show();
 }
 return ptrStart;
}


int b_vpl_from_raw (gList& textEntries, sVplPlaylist& playlist)
{
 int iter( 0 ), nEntries( 0 );
 int secs( -1 );
 gElem* ptrElem( textEntries.StartPtr() );
 char* str;

 DBGPRINT("DBG: ipl_vpl_from_raw (entries: %u, playlist=%d=0)\n",
	  textEntries.N(),
	  playlist.nEntries);
 playlist.Release();

 for ( ; ptrElem; ptrElem=ptrElem->next) {
     str = ptrElem->Str();

     if ( str[ 0 ]==0 ) continue;

     if ( str[ 0 ]=='#' ) {
	 if ( playlist.comment[ 0 ]==0 ) {
	     strncpy(playlist.comment, str, sizeof(playlist.comment)-1);
	 }
	 continue;
     }

     nEntries++;
 }

 playlist.entries = new gList*[ nEntries+1 ];
 ASSERTION(playlist.entries,"playlist.entries");

 for (ptrElem=textEntries.StartPtr(); ptrElem; ptrElem=ptrElem->next) {
     str = ptrElem->Str();
     if ( strncmp( str, "#EXTINF:", 8 )==0 ) {
	 secs = atoi( str+8 );
     }
     if ( str[ 0 ]==0 || str[ 0 ]=='#' ) continue;

     iter++;
     playlist.entries[ iter ] = new gList;
     ASSERTION(playlist.entries[ iter ],"Mem!");
     playlist.entries[ iter ]->iValue = secs;
     playlist.entries[ iter ]->Add( ptrElem->Str() );
     secs = -1;
 }

 playlist.nEntries = nEntries;
 DBGPRINT("DBG: b_vpl_from_raw with nEntries=%d\n",nEntries);
 return 0;
}


#ifdef OLD_NICED_CHARS
char* niced_chars (char* aStr, gUniCode::eHashBasic which, int& changed)
{
 char code;
 t_uchar uIdx;
 char* strResult( aStr );

 ASSERTION(aStr,"aStr");
 ASSERTION(which<gUniCode::e_Basic_Hash,"which?");
 ASSERTION(which>=0,"which??");
 changed = 0;

 for (char chr; (chr = *aStr)!=0; aStr++) {
     uIdx = (t_uchar)chr;
     code = uniData.inUse->hash256User[ which ][ uIdx ];
     //		uniData.hashUcs8Custom[ uIdx ]:		upper-lower change
     if ( code!=-1 ) {
	 *aStr = code;
     }
     changed += (code != chr);
 }
 return strResult;
}


char* nice_chars (char* aStr)
{
 int changed( -1 );
 char* strDup( strdup( aStr ) );
 char* strResult( niced_chars( aStr, gUniCode::e_Basic_Printable, changed ) );

 printf("\n\n[changed: %d]\t{%s} vs {%s}\n", changed, strDup, aStr);
 return strResult;
}

#else

char* nice_chars (char* aStr)
{
 char* strResult( imb_nice_words( 0, aStr ) );
 return strResult;
}

#endif //normal case: libimedia.so has the imb_nice_words() function to be used!

////////////////////////////////////////////////////////////
// Main functions
////////////////////////////////////////////////////////////
int do_test (int cmdNr, gList& arg, FILE* fOut, FILE* fRepErr, sOptBox& opt)
{
 FILE* fIn( stdin );
 gElem* ptrElem( arg.StartPtr() );
 gString* myStr;
 char miniBuf[ 2 ];

 bool remember( opt.level>=3 );

 ASSERTION(fOut,"fOut");
 miniBuf[ 1 ] = 0;

 imb_init_words( 0 );

 if ( ptrElem ) {
     myStr = (gString*)ptrElem->me;

     if ( myStr->Match( "." ) ) {
	 // Use stdin

	 gString sOriginal, sNew;

	 for ( ; fscanf(fIn, "%c", &(miniBuf[ 0 ]))==1; ) {
	     if ( miniBuf[ 0 ]=='\r' ) continue;

	     if ( remember ) sOriginal.Add( miniBuf );

	     nice_chars( miniBuf );  // convert into fine 8bit letter!

	     if ( remember ) sNew.Add( miniBuf );

	     fprintf(fOut, "%s", miniBuf);
	 }
	 if ( remember ) {
	     if ( sOriginal.Match( sNew )==false ) {
		 printf("\n-- Original input follows:\n%s", sOriginal.Str());
	     }
	 }
     }
     else {
	 for ( ; ptrElem; ptrElem=ptrElem->next) {
	     nice_chars( ptrElem->Str() );
	     fprintf(fOut, "%s\n", ptrElem->Str());
	 }
     }
 }
 else {
     ASSERTION(ptrUniData,"ptrUniData");
     imb_iso_show( fOut, *ptrUniData );

     printf("\
--\n\
iobjs\t%3d.%02d\n\
",
	    LIB_VERSION_ILIB_MAJOR, LIB_VERSION_ILIB_MINOR
	 );
 }

 return 0;
}


int do_ls (int nrFormat, gList& arg, FILE* fOut, FILE* fRepErr, sOptBox& opt)
{
 int error( 0 );
 int nIter( (int)arg.N() );
 int val;
 unsigned pos( 0 );
 gElem* ptrElem;
 gString* myStr;
 gString baseDir( opt.sBase );
 gString stdBaseDir( baseDir );
 gList dirs;
 bool isDir;

 const bool showAll( opt.areAll );
 const bool otherSort( opt.sortBy.NonBasicSort() );

 ASSERTION(fOut,"fOut");

 if ( baseDir.Length() && baseDir.Match( gSLASHSTR )==false && baseDir.Match( altSLASHSTR )==false ) {
     baseDir.Add( gSLASHCHR );
 }

 if ( opt.newLineCR.IsEmpty() ) {
     jcl_get_extension( opt.sOutput, 0, opt.sOutExt );
     if ( opt.sOutExt.Find( ".m3u" )==1 ) {
	 opt.newLineCR.Set( "\r" );
     }
 }

 if ( nIter==0 ) {
     gFileFetch input( nil, -1 );
     for (ptrElem=input.aL.StartPtr(); ptrElem; ptrElem=ptrElem->next) {
	 myStr = (gString*)ptrElem->me;
	 pos = myStr->FindBack( '\t' );
	 if ( pos ) {
	     // Ignore starting tab:
	     myStr->Delete( 1, pos );
	 }

	 jcl_add_pname( *myStr, dirs );
     }
 }
 else {
     for (gElem* pArg=arg.StartPtr(); pArg; pArg=pArg->next) {
	 jcl_add_pname( *((gString*)pArg->me), dirs );
     }
 }

 for (ptrElem=dirs.StartPtr();  ptrElem; ptrElem=ptrElem->next) {
     myStr = (gString*)ptrElem->me;
     pos = myStr->FindBack( '\t' );
     if ( pos ) {
	 // Ignore starting tab:
	 myStr->Delete( 1, pos );
     }

     for ( ; (pos = (((*myStr)[ 1 ]=='.' && ((*myStr)[ 2 ]==gSLASHCHR || (*myStr)[ 2 ]==altSLASHCHR))))!=0; ) {
	 myStr->Delete( 1, 2 );
     }

     // When mixing / and '\\'... we could force only one dir style

     if ( (*myStr)[ 1 ]==gSLASHCHR || (*myStr)[ 1 ]==altSLASHCHR ) {
	 myStr->Insert( stdBaseDir, 1 );
     }
     else {
	 myStr->Insert( baseDir, 1 );
     }
 }

 for (ptrElem=dirs.StartPtr();  ptrElem; ptrElem=ptrElem->next) {
     myStr = (gString*)ptrElem->me;
     gFileStat aStat( *myStr );
     isDir = aStat.IsDirectory();
     if ( isDir==false ) {
	 val = aStat.statusL.Size();
	 myStr->iValue = val;
	 if ( val==-1 ) {
	     error = 2;
	     MM_LOG(LOG_ERROR, "Not found: %s", myStr->Str());
	 }
     }
 }

 ptrElem = dirs.StartPtr();

 if ( otherSort ) {
     gList all;

     for ( ;  ptrElem; ptrElem=ptrElem->next) {
	 myStr = (gString*)ptrElem->me;
	 isDir = myStr->iValue==0;

	 if ( showAll==true || myStr->iValue!=-1 ) {
	     // Use 'all' to store results, and no output to fOut immediately:
	     do_dir_ls( *myStr, isDir ? 0 : -1, nil, fRepErr, opt, &all );
	 }
     }

     // Final dump
     const unsigned leadin0x01Pos( 2 );
     const unsigned width1( opt.level>=3 ? 0 : (20+1) );
     unsigned posIdx;

     for (ptrElem=all.StartPtr(); ptrElem; ptrElem=ptrElem->next) {
	 myStr = (gString*)ptrElem->me;
	 posIdx = ((*myStr)[ 2 ]==1 ? (leadin0x01Pos+width1) : 0);
	 if ( posIdx > myStr->Length() ) {
	     posIdx = 0;  // this won't happen, but we want to be tidy (in case 0x01 char appears, it won't hurt output too much, anyway)
	 }
	 fprintf(fOut, "%s\n", myStr->Str( posIdx ));
     }
 }
 else {
     for ( ;  ptrElem; ptrElem=ptrElem->next) {
	 myStr = (gString*)ptrElem->me;
	 isDir = myStr->iValue==0;

	 if ( myStr->iValue!=-1 ) {
	     do_dir_ls( *myStr, isDir ? 0 : -1, fOut, fRepErr, opt, nil );
	 }
     }
 }

 DBGPRINT("DBG: do_ls returns %d\n",error);
 return error;
}


int do_vpl_dump (int cmdNr, gList& arg, FILE* fOut, FILE* fRepErr, sOptBox& opt)
{
 int error( 0 );
 int countErrors( 0 );
 int iArg( 1 ), nArgs( (int)arg.N() ), maxArgs( nArgs ? nArgs : 1 );
 int audioType( -1 );
 int secs( 0 );
 int counts( 0 );
 unsigned pos( 0 );

 bool isVPL( false );
 bool doUnHTML( false );
 char* strLine( nil );

 gString baseDir( opt.sBase );
 gString sHead;
 gElem* ptrElem( nil );
 gElem* pIter( nil );
 gElem* thisPtr( nil );

 iEntry* entries( nil );
 iEntry* thisEntry( nil );
 iEntry lastOne;

 const char* strNewLine;
 const bool showAll( opt.areAll );
 const bool showAlways( opt.areAll );
 const int displayMask( opt.zValue );
 const bool outputM3U( (displayMask & 1)!=0 );
 const bool reportOnce( (displayMask & 8)!=0 );  // do not repeat [#EXTINF xyz\n]abc.mp3
 const bool reportByOccurrence( displayMask==9 );

 ASSERTION(fOut,"fOut");

 if ( opt.newLineCR.IsEmpty() ) {
     jcl_get_extension( opt.sOutput, 0, opt.sOutExt );

     bool hasExt( opt.sOutExt.Find( ".m3u", true )==1 );

     if ( hasExt || opt.sOutExt.Match( ".vpl", true )==1 ) {
	opt.newLineCR.Set( "\r" );
	if ( opt.doAppend==false ) {
	    if ( hasExt ) {
		sHead.Set( "#EXTM3U\r\n" );
	    }
	    else {
		sHead.Set( "#VUPlayer playlist\r\n" );
	    }
	}
     }
 }
 strNewLine = opt.newLineCR.Str();

 if ( fOut==nil ) {
     fOut = stdout;
 }

 entries = new iEntry;
 ASSERTION(entries,"mem");

 for ( ; iArg<=maxArgs; iArg++) {
     const char* strFile( arg.Str( iArg ) );
     gFileFetch input( (char*)strFile );

     DBGPRINT("DBG: vpl dump {%s}, N()=%u\n", strFile, input.aL.N());

     if ( input.IsOk()==false ) {
	 countErrors++;
	 perror( strFile );
	 continue;
     }

     for (ptrElem=input.aL.StartPtr(); ptrElem; ptrElem=ptrElem->next) {
	 gString sLeft( ptrElem->Str() );
	 iEntry* ptrVPL( nil );

	 sLeft.TrimRight();
	 if ( sLeft[ 1 ]=='#' ) {
	     if ( sLeft.Find( "#VUPlayer" )==1 ) {
		 isVPL = true;
	     }
	     else {
		 if ( sLeft.Find("#EXTINF:") ) {
		     unsigned posEnd( sLeft.FindBack( ".mp", true ) );

		     if ( posEnd+3>=sLeft.Length() ) {
			 sLeft.Delete( posEnd );
		     }
		     DBGPRINT("DBG: strLine {%s}\n", sLeft.Str());
		     lastOne.SetComment( sLeft.Str() );
		 }
	     }
	     continue;
	 }

	 pos = sLeft.Find( JCL_ASCII01 );
	 if ( pos ) {
	     ptrVPL = new_vpl_entry( sLeft, pos );
	     isVPL = ptrVPL!=nil;

	     if ( isVPL ) {
		 sLeft.Delete( pos );
		 lastOne.sComment.iValue = ptrVPL->iValue;
		 lastOne.SetComment( ptrVPL->sComment.Str() );

		 delete ptrVPL;
	     }
	 }

	 pos = sLeft.Find( '<' );
	 if ( pos==0 ) {
	     pos = sLeft.Find( "src=\"", true );
	     if ( pos ) {
		 pos += 4;
		 doUnHTML = true;
		 // when there is an HTML code like e.g. "&apos;" convert into "'"
		 // this is done at PhraseUnHTML(), see smart.cpp
	     }
	 }
	 if ( pos ) {
	     unsigned pQuote( 0 );
	     sLeft.Delete( 1, pos );

	     pos = sLeft.FindBack( '>' );
	     if ( pos ) {
		 sLeft.Delete( pos );
	     }
	     sLeft.Trim();

	     for (pos=sLeft.Length(); pos>1; pos--) {
		 if ( sLeft[ pos ]=='"' || sLeft[ pos ]=='/' ) {
		     sLeft.Delete( pos );
		     pQuote = pos;
		 }
		 else
		     break;
	     }

	     if ( pQuote ) {
		 pos = sLeft.Find( '"' );
		 if ( pos ) {
		     sLeft.Delete( 1, pos );
		 }
	     }
	     sLeft.Trim();
	     pQuote = sLeft.Find( '"' );
	     if ( pQuote ) {
		 gString sRem( sLeft.Str( pQuote ) );
		 if ( sRem.Find( "=\"" ) ) {
		     sLeft.Delete( pQuote );
		 }
		 sLeft.Trim();
	     }
	 }

	 if ( sLeft.IsEmpty() ) continue;

	 if ( sLeft[ sLeft.Length() ]==':' ) {
	     lastOne.SetComment();
	     if ( showAlways==false ) {
		 continue;
	     }
	 }

	 strLine = sLeft.Str();
	 gString sBase;  // tailing slash/ backslash if needed

	 if ( showAll || (audioType = media_type_from_path( sLeft.Str() ))>0 ) {
	     if ( baseDir.Length()>0 ) {
		 if ( sub_str_compare( strLine, baseDir.Str() )==1 ) {
		     strLine += baseDir.Length();
		     for ( ; (*strLine)=='\\'; ) {
			 strLine++;
		     }
		 }
		 else {
		     sBase = baseDir;
		     sBase.Add( slash_or_backslash( baseDir.Str() ) );
		 }
	     }

	     if ( strLine[ 0 ] ) {
		 iEntry* newMedia( new iEntry );
		 int commentLength( 0 );

		 secs = lastOne.sComment.iValue;
		 newMedia->display = lastOne.sComment;
		 if ( newMedia->display.Length() ) {
		     newMedia->display.Add( (char*)strNewLine );
		     newMedia->display.Add( '\n' );
		     commentLength = newMedia->display.Length();
		 }
		 newMedia->display.Add( sBase.Str() );
		 newMedia->display.Add( (char*)strLine );

		 newMedia->Add( (char*)strLine );
		 DBGPRINT("baseDir {%s}, {%s}, strLine {%s}\nnewMedia: %s\n\n",
			  baseDir.Str(),
			  sBase.Str(),
			  strLine,
			  newMedia->Str());

		 thisEntry = nil;
		 if ( reportOnce ) {
		     // This does not work:	if ( entries->FindFirst( newMedia->display.Str(), 1, e_FindExactPosition ) ) ...
		     // so, we adopt the following search:

		     thisEntry = search_entry_display( *entries, *newMedia );

		     if ( thisEntry ) {
			 // Matched newMedia (this entry) with an already existing one.
			 //
			 // Which one is bigger?
			 //
			 // We prefer the one containing #EXTINF

			 unsigned posThere( thisEntry->display.Length() );
			 unsigned posHere( newMedia->display.Length() );
			 DBGPRINT("DBG: {%s} posThere: %u, posHere: %u\n",
				  thisEntry->Str(),
				  posThere, posHere);

			 if ( posHere > posThere ) {
			     thisEntry->display.iValue = 0;
			     thisEntry = nil;  // append it again!
			 }
			 else {
			     // Add another hit to existing one:
			     thisEntry->display.iValue++;
			     delete newMedia;
			 }
		     }
		 }
		 if ( thisEntry==nil ) {
		     entries->AppendObject( newMedia );
		     thisPtr = entries->EndPtr();
		     ((iEntry*)thisPtr->me)->display.iValue = 1;  // New entry, count once

		     thisPtr->iValue = commentLength;
		     thisPtr->me->iValue = secs;
		 }

		 // Reset existing comment
		 lastOne.SetComment();
	     }
	 }
	 else {
	     entries->SetComment();
	     MM_LOG(LOG_WARN, "Skipped: %s%s", strLine, strNewLine);
	 }
     }//end FOR (lines in playlist)

     DBGPRINT("DBG: N()=%u, reportOnce? %c, sHead={%s}, output extension {%s}\t%s\n",
	      entries->N(),
	      ISyORn( reportOnce ),
	      sHead.Str(), opt.sOutExt.Str(),
	      strFile);

     if ( reportOnce==false ) {
	 fprintf(fOut, "%s", sHead.Str());

	 for (pIter=entries->StartPtr(); pIter; pIter=pIter->next) {
	     thisEntry = (iEntry*)pIter->me;
	     strLine = thisEntry->display.Str();

	     DBGPRINT("DBG: +%d\t{%s}\n\n",
		      pIter->iValue,
		      strLine);
	     fprintf(fOut, "%s%s\n",
		     strLine + (outputM3U ? 0 : (pIter->iValue)),
		     strNewLine);
	 }

	 delete entries; entries = nil;
	 entries = new iEntry;
	 ASSERTION(entries,"mem(2)");
     }
 }// end FOR (args)

 pIter = entries->StartPtr();

 if ( reportOnce ) {
     fprintf(fOut, "%s", sHead.Str());

     if ( reportByOccurrence ) {
	 iEntry* keepMax( nil );
	 int maxOcc( 0 );

	 for ( ; (pIter = entries->StartPtr())!=nil; ) {
	     for (maxOcc=0, keepMax=nil; pIter; pIter=pIter->next) {
		 thisEntry = (iEntry*)pIter->me;
		 counts = thisEntry->display.iValue;
		 if ( counts > maxOcc ) {
		     maxOcc = counts;
		     keepMax = thisEntry;
		 }
	     }

    	     if ( keepMax==nil ) break;

	     thisEntry = keepMax;
	     strLine = thisEntry->display.Str();

	     DBGPRINT("OCC: %d=%d ", maxOcc, thisEntry->display.iValue);
	     fprintf(fOut, "%s%s\n", strLine + (outputM3U ? 0 : (pIter->iValue)), strNewLine);

	     thisEntry->display.iValue = -1;
	 }
     }//end IF by occurrence
     else {
	 // report once, but not by occurrence:

	 for ( ; pIter; pIter=pIter->next) {
	     thisEntry = (iEntry*)pIter->me;
	     strLine = thisEntry->display.Str();
	     counts = thisEntry->display.iValue;

	     if ( counts > 1 ) {
		 oprint("count=%d, len#%d secs=%d\t%s\n",
			counts,
			pIter->iValue,
			pIter->me->iValue,
			strLine);
	     }

	     fprintf(fOut, "%s%s\n", strLine + (outputM3U ? 0 : (pIter->iValue)), strNewLine);
	 }
     }
 }

 delete entries;

 error = countErrors!=0;

 if ( lastOne.sComment.Length() ) {
     MM_LOG(LOG_WARN, "Unused comment: %s", lastOne.sComment.Str());
 }
 if ( doUnHTML ) {
     MM_LOG(LOG_INFO, "Do un-HTML");
 }

 DBGPRINT("DBG: countErrors: %d, error: %d\n",
	  countErrors,
	  error);
 return error;
}


int do_crosslink (const char* strOutDir,
		  unsigned startAt,
		  gList& playlists,
		  const char* strCommandName,
		  FILE* fOut,
		  FILE* fRepErr,
		  sOptBox& opt)
{
 int error;
 int itemsThere( 0 );
 int countErrors( 0 );
 unsigned idxArg( startAt ), nArgs( playlists.N() );
 const char* strFile;
 gFileStat checkDir( (char*)strOutDir );
 gList fileList;

 sCrossLinkConf linkConf( strOutDir );

 ASSERTION(fRepErr,"fRepErr");
 ASSERTION(fOut,"fOut");
 ASSERTION(strOutDir,"strOutDir");

 linkConf.showProgress = opt.level>3;  // Nada

 error = jcl_config( opt.sConfigFile.Str(), linkConf );
 if ( error ) {
     linkConf.substed.Release();
     fprintf(fRepErr,"Error-code %d reading conf; ignoring.\n",error);
 }

 error = checkDir.IsDirectory()==false;
 if ( error ) {
     if ( checkDir.HasStat()==false ) {
	 error = gio_mkdir( strOutDir, 0, 0 );
     }
 }

 if ( error ) {
     fprintf(fRepErr,"Not a valid output directory (%d): %s\n",
	     checkDir.lastOpError,
	     strOutDir);
     if ( strOutDir[ 0 ]==0 ) {
	 strOutDir = "<empty>";
     }
     MM_LOG(LOG_ERROR, "Invalid output directory: %s", strOutDir);
     return 2;
 }

 itemsThere = jcl_numbered_entries( strOutDir, 0, error );

 if ( itemsThere ) {
     if ( itemsThere>0 ) {
	 MM_LOG(LOG_WARN, "%d numbered item(s) already there: %s",
		 itemsThere,
		 strOutDir);
     }
     else {
	 MM_LOG(LOG_ERROR, "Invalid output directory: %s", strOutDir);
     }
 }

 for ( ; idxArg<=nArgs; idxArg++) {
     strFile = playlists.Str( idxArg );
     gString sPlay( (char*)strFile );
     gFileStat checkFile( sPlay.Str() );
     error = checkFile.HasStat()==false || checkFile.IsDirectory()==true;
     if ( error ) {
	 MM_LOG(LOG_ERROR, "Wrong playlist: %s",strFile);
	 fprintf(fRepErr,"Unable to open playlist (%d): %s\n",
		 checkFile.lastOpError,
		 strFile);
	 break;
     }
     fileList.Add( sPlay );
 }

 if ( error ) {
     fprintf(fRepErr,"At least one playlist is bogus: cowardly quitting...\n");
     return 4;
 }

 for (idxArg=1, nArgs=fileList.N(); idxArg<=nArgs; idxArg++) {
     sInPlaylist plays;
     gString sPlay( fileList.Str( idxArg ) );

     error = jcl_read_playlist( linkConf, sPlay, plays );
     if ( error ) {
	 fprintf(fRepErr,"Playlist read failed (%d): %s\n",
		 error,
		 sPlay.Str());
     }
     else {
	 if ( JCL_DEBUG_LEVEL>=9 ) {
	     printf("Playlist: %s, type: %02d, items: %d\n", sPlay.Str(), (int)plays.type, plays.items.N());
	 }

	 switch ( opt.zValue ) {
	 case 0:
	     error = jcl_build_crosslink( linkConf, plays );
	     break;

	 case 8:
	     error = jcl_build_copy( linkConf, plays );
	     break;

	 default:
	     break;
	 }
	 if ( error ) {
	     countErrors++;
	     fprintf(fRepErr,"%s failed for: %s\n",strCommandName,sPlay.Str());  // e.g. Crosslink failed...
	 }
     }

     if ( error ) {
	 MM_LOG(LOG_ERROR, "%s failed: %s",strCommandName,sPlay.Str());
     }
 }

 error = countErrors!=0;
 return error;
}


int do_join_to (gList& all, FILE* fOut, gString& sOutName, sOptBox& opt)
{
 int error( 0 );
 gString sOutFile( all.Str( 1 ) );
 gString sFirstLine;
 bool isVPL( false );
 bool thisIsVPL( false );
 bool reopened( false );
 gList* existing( nil );
 gString* strLine( nil );
 FILE* fIO( stdout );

 all.Delete( 1, 1 );

 // inputs are in the list: "all"
 if ( all.N()==0 ) return 0;

 existing = new gList;
 ASSERTION(existing,"Mem");

 if ( sOutFile.IsEmpty() ) {
    if ( fOut ) fIO = fOut;
    sOutFile = sOutName;
    isVPL = does_match_extension( sOutFile, ".vpl" )!=nil;
 }
 else {
    fIO = fopen( sOutFile.Str(), "r" );
    if ( fIO ) {
	fclose( fIO );
	gFileFetch fIn( sOutFile );
	gElem* whichStart( fIn.aL.StartPtr() );
	sFirstLine = fIn.aL.Str( 1 );
	isVPL = sFirstLine.Find( "#VUPlayer ", true )==1;
	if ( isVPL ) {
	    whichStart = whichStart->next;
	}
	add_to_list( whichStart, *existing );
    }
    else {
	isVPL = does_match_extension( sOutFile, ".vpl" )!=nil;
    }
    fIO = fopen( sOutFile.Str(), "a+" );
    reopened = fIO!=NULL;
    if ( fIO==NULL ) error = errno;
 }

 gList listed;
 const char* strCR="\r";

#ifdef iDOS_SPEC
 if ( fIO==stdout ) strCR = "\0";
#endif
 if ( isVPL || (opt.zValue & 2) ) strCR = "\0";

 if ( fIO ) {
    for (gElem* pInput=all.StartPtr(); pInput; pInput=pInput->next) {
	DBGPRINT("to: %s, %s%s\n",sOutFile.Str(), pInput->Str(), isVPL ? " [VPL]" : "");
	gFileFetch input( pInput->Str() );
	sFirstLine = input.aL.Str( 1 );
	if ( input.lastOpError ) {
	    MM_LOG(LOG_ERROR, "Invalid input: %s", pInput->Str());
	}
	else {
	    thisIsVPL = sFirstLine.Find( "#VUPlayer ", true )==1;
	    if ( thisIsVPL ) {
		input.aL.Delete( 1, 1 );
	    }
	    add_to_list( input.aL.StartPtr(), listed );
	}
    }

    for (gElem* show=listed.StartPtr(); show; show=show->next) {
        int skipIt( 0 );
	strLine = (gString*)show->me;
	DBGPRINT("%d\t%s\n",show->me->iValue,show->Str());
	strLine->TrimRight();
	if ( opt.zValue & 1 ) {
	    if ( strLine->IsEmpty() || strLine->Match( "#" ) ) skipIt = -1;
	}
	if ( isVPL ) {
	    if ( strLine->IsEmpty() ) skipIt = -1;
	}
	show->iValue = skipIt;
    }

    for (gElem* show=listed.StartPtr(); show; show=show->next) {
	if ( show->iValue!=-1 ) {
	    gString* strLine( (gString*)show->me );
	    if ( (*strLine)[ strLine->Length() ]==':' ) continue;
	    fprintf(fIO, "%s%s\n", strLine->Str(), strCR);
	}
    }

    if ( reopened ) {
	fclose( fIO );
    }
 }

 delete existing;
 return error;
}


int do_compact (const char* strPath, gElem* ptrRest, FILE* fOut, FILE* fRepErr, sOptBox& opt)
{
 int error( 0 );
 int counter( 0 ), countZero( 0 ), countToRename( 0 );
 int countLinear( 0 );
 int value( -1 ), thisValue( 0 );
 int fromIdx( (int)(opt.zValue > 0 ? opt.zValue : 0) % 1000 );

 gDir aDir( (char*)strPath );
 const char* strName( nil );
 gFileSysName* ptrSys;
 gElem* ptrEntry;
 gElem* ptrElem;
 gString* myStr;
 sPairedList paired;

 t_uchar uChr;
 char smallBuf[ 64 ];
 bool dumpOnly( opt.zValue>=1000 );
 bool fromDirOrder( fromIdx==999 );

 ASSERTION(fOut,"fOut");
 ptrEntry = aDir.StartPtr();
 if ( ptrEntry==nil ) return 2;

 DBGPRINT_MIN("do_compact fromDirOrder? %c, fromIdx=%d\n",
	      ISyORn( fromDirOrder ),
	      fromIdx);

 for ( ; ptrEntry; ptrEntry=ptrEntry->next) {
     ptrSys = (gFileSysName*)ptrEntry->me;
     strName = ptrSys->Str();
     if ( ptrSys->IsDirectory()==false ) {
	 if ( ptrSys->HasStat() ) {
	     if ( fromDirOrder ) {
		 jcl_add_sort_paired( strName, false, ++countLinear, paired );
	     }
	     else {
		 value = (int)ptrSys->GetStat().statusL.Size();
		 if ( value<=0 ) {
		     MM_LOG(LOG_WARN, "Suspicious file: %s (%d)", strName, value);
		 }
		 else {
		     jcl_add_sort_paired( strName, true, value, paired );
		 }
	     }
	 }
	 else {
	     MM_LOG(LOG_WARN, "Cannot read: %s",strName);
	 }
     }
 }

 if ( fromDirOrder ) {
     counter = countLinear;
     fromIdx = 1;
 }
 else {
     for (ptrEntry=paired.b.StartPtr(); ptrEntry; ptrEntry=ptrEntry->next) {
	 strName = ptrEntry->Str();
	 value = ptrEntry->me->iValue;
	 countZero += (value==0);
	 value = tracknr_indication( strName );
	 if ( value>0 ) counter++;
	 ptrEntry->me->iValue = value;  // now value is the conversion to int of figures
     }
 }

#if 1
 for (gElem* ptrExt=paired.a.StartPtr(); ptrExt; ptrExt=ptrExt->next) {
     strName = ptrExt->Str();
     value = ptrExt->me->iValue;
     printf(" [%s] #%d;%s",
	    strName,
	    value,
	    ptrExt->next ? "" : "\n");
 }
 printf("\n");
 for (gElem* ptrExt=paired.b.StartPtr(); ptrExt; ptrExt=ptrExt->next) {
     strName = ptrExt->Str();
     value = ptrExt->me->iValue;
     printf("%d\t%s\n",
	    value,
	    strName);
 }
 printf("\n");
#endif

 if ( counter ) {
     int iter( 0 );
     int maxIn( -1 );
     gList names;
     gList entries;

     if ( countZero ) {
	 MM_LOG(LOG_WARN, "At least one empty file (%d)", countZero);
     }

     ptrEntry = paired.b.StartPtr();

     for ( ; ptrEntry; ptrEntry=ptrEntry->next) {
	 value = ptrEntry->me->iValue;
	 if ( value>0 ) {
	     myStr = new gString( ptrEntry->Str() );
	     ASSERTION(myStr,"myStr");
	     myStr->iValue = value;

	     if ( fromDirOrder ) {
		 names.AppendObject( myStr );
		 continue;
	     }

	     for (iter=1, maxIn=-1; myStr; iter++) {
		 if ( iter>(int)names.N() ) break;
		 strName = names.Str( iter );
		 ptrElem = names.CurrentPtr();
		 ASSERTION(ptrElem,"ptrElem");
		 thisValue = ptrElem->me->iValue;
		 if ( thisValue > maxIn ) maxIn = thisValue;

		 if ( value < thisValue ) {
		     names.InsertHere( myStr );
		     DBGPRINT("DBG: Here: myStr %d: %s\n",value,myStr->Str());
		     myStr = nil;  // so it is not inserted twice
		 }
	     }

	     DBGPRINT("%c - %d\t{%s}\n",
		      ISyORn( fromDirOrder ),
		      value,
		      myStr ? myStr->Str() : "?");

	     if ( myStr ) {
		 if ( value > maxIn ) {
		     names.AppendObject( myStr );
		 }
		 else {
		     names.InsertObject( myStr );
		 }
	     }
	 }
     }// end FOR: entries

     countLinear = 0;
     iter = 1;
     if ( fromIdx ) iter = fromIdx;

     for (ptrEntry=names.StartPtr(); ptrEntry; ptrEntry=ptrEntry->next) {
	 myStr = (gString*)ptrEntry->me;
	 value = myStr->iValue;
	 sprintf(smallBuf, "%03d", value);
	 int iNew( 0 );
	 int iPos( myStr->Find( smallBuf ) );
	 gString sValTry( *myStr );

	 countLinear++;

	 if ( sValTry[ 1 ]>='0' && sValTry[ 1 ]<='9' &&
	      sValTry[ 2 ]>='0' && sValTry[ 2 ]<='9' &&
	      sValTry[ 3 ]>='0' && sValTry[ 3 ]<='9' ) {
	     sValTry.Delete( 4 );
	 }
	 else {
	     sValTry.Delete();
	 }

	 if ( fromDirOrder && myStr->Length() ) {
	     gString sNew;

	     sprintf(smallBuf, "%03d", countLinear);
	     sNew.Add( smallBuf );

	     if ( sValTry.Length() ) {
		 sNew.Add( myStr->Str( sValTry.Length() ) );
	     }
	     else {
		 sNew.Add( " " );
		 sNew.AddString( *myStr );
	     }

	     entries.Add( sNew );
	     entries.EndPtr()->me->iValue = countLinear;
	     continue;
	 }

	 if ( iPos<=0 ) {
	     sprintf(smallBuf, "%d", value);
	     iPos = myStr->Find( smallBuf );
	 }

	 DBGPRINT("name #%d {%s} iPos=%d\n",
		  countLinear,
		  myStr->Str(),
		  iPos);
	 if ( iPos==1 ) {
	     for (iNew=iPos-1; iNew>=1; iNew--) {
		 uChr = (*myStr)[ iNew ];
		 if ( uChr<'0' || uChr>'9' ) break;
	     }

	     gString sLeft;
	     gString sRight( myStr->Str( iNew + strlen( smallBuf ) ) );
	     gString sKeep( sRight );

	     for ( ; sRight.Length(); ) {
		 t_uchar uFirst( sRight[ 1 ] );
		 if ( uFirst<'0' || uFirst>'9' ) break;
		 sRight.Delete( 1, 1 );
	     }

	     if ( sRight.IsEmpty() ) {
		 sRight = sKeep;
		 MM_LOG(LOG_WARN, "Uops (#%d): %s for {%s}",
			countLinear,
			sRight.Str(),
			myStr->Str());
	     }
	     else {
		 if ( sRight.Match( sKeep )==false ) {
		     MM_LOG(LOG_NOTICE, "Uops (#%d): %s|%s",
			    countLinear,
			    sRight.Str(),
			    sKeep.Str());
		 }
	     }

	     if ( iNew>=1 ) {
		 sLeft.CopyFromTo( *myStr, 1, iNew );
	     }
	     sprintf(smallBuf, "%03d", iter);

	     gString sNew( sLeft );
	     sNew.Add( smallBuf );
	     sNew.AddString( sRight );

	     entries.Add( sNew );
	     entries.EndPtr()->me->iValue = iter;
	     iter++;
	 }
     }

     if ( names.N()!=entries.N() ) {
	 MM_LOG(LOG_ERROR, "Mismatch: %u vs %u", names.N(), entries.N());
     }
     else {
	 for (iter=0, ptrEntry=entries.StartPtr(); ptrEntry; ptrEntry=ptrEntry->next) {
	     bool matches;
	     iter++;
	     myStr = (gString*)ptrEntry->me;
	     strName = names.Str( iter );
	     value = myStr->iValue;
	     matches = myStr->Match( (char*)strName );
	     if ( matches==false ) {
		 countToRename++;
		 myStr->iValue = 0;
	     }
	 }
     }

     if ( countToRename<=0 ) {
	 MM_LOG(LOG_NOTICE, "No renames to do, %u file(s)", counter);
     }
     else {
	 for (iter=0, ptrEntry=entries.StartPtr(); ptrEntry; ptrEntry=ptrEntry->next) {
	     bool matches;
	     iter++;
	     myStr = (gString*)ptrEntry->me;
	     strName = names.Str( iter );
	     value = myStr->iValue;
	     matches = value!=0;
	     if ( dumpOnly ) {
		 if ( matches ) {
		     fprintf(fOut,"%d\t%s\n\t[MATCH]\n",
			     iter,
			     myStr->Str());
		 }
		 else {
		     fprintf(fOut,"%d\t%s\n\t%s\n",
			     iter,
			     myStr->Str(),
			     strName);
		 }
	     }
	     else {
		 int doRename( special_rename( strPath, strName, myStr->Str() ) );
		 const char* strOk( doRename==0 ? "OK" : "ERROR" );
		 printf("%s:\t%s\n\t%s\n\n",
			strOk,
			strName,
			myStr->Str());
		 if ( opt.level>=3 ) {
		     MM_LOG(LOG_INFO, "Renamed to: %s", myStr->Str());
		 }
	     }
	 }// end FOR
     }
 }
 else {
     MM_LOG(LOG_WARN, "Nothing found to rename");
 }
 return error;
}


int do_rename (gList& arg,
	       FILE* fOut,
	       FILE* fRepErr,
	       int mask,
	       sOptBox& opt)
{
 int error( 0 );
 int countErrors( 0 );
 int itemCount( 0 );
 int iter( 1 ), nArgs( (int)arg.N() );
 int lastOpError( -1 );
 const bool showOnly( (mask & 1)!=0 );

 gElem* ptrElem( nil );
 gElem* ptrIter( nil );
 char* strFile( nil );
 const char* strOldName( nil );

 ASSERTION(fRepErr,"fRepErr");
 ASSERTION(fOut,"fOut");

 // Parse args
 if ( nArgs<1 ) {
     fprintf(fRepErr,"Command 'ren' requires at least one arg.\n");
     return 0;
 }

 const char* strName( arg.Str( 1 ) );
 gString sNames;
 gString sDir( opt.sBase );
 gString sSlashedBase( sDir );
 gList substExpress;
 gList inputs;
 gList files;
 gList news;
 gList errors;
 gFileStat aStat( sDir );
 bool doBulk( nArgs==2 && strcmp( arg.Str( 2 ), "." )==0 );

 name_to_subst_string( strName, sNames );
 substExpress.Add( sNames );

 if ( sDir.Length() ) {
     sSlashedBase.Add( gSLASHCHR );
 }

 if ( doBulk ) {
     gDir aDir( sDir.Length() ? sDir.Str() : arg.Str( 2 ) );
     gList candidate1;
     gList candidateAll;
     for (ptrIter=aDir.StartPtr(); ptrIter; ptrIter=ptrIter->next) {
	 gString* pStr( (gString*)ptrIter->me );
	 unsigned pos( pStr->Find( sNames ) );
	 if ( opt.debugLevel>0 ) {
	     printf("Candidate, pos=%u: %s\n", pos, pStr->Str());
	 }
	 if ( pos > 0 ) {
	     jcl_add_once( pStr->Str(), candidate1 );
	 }
	 jcl_add_once( pStr->Str(), candidateAll );
     }
     bool adoptCandidates( (candidate1.N() > 0) && opt.areAll==false );
     if ( opt.level>=3 ) {
	 printf("Num. files: %u, candidates: %u, %s\n",
		candidateAll.N(),
		candidate1.N(),
		adoptCandidates ? "using candidates (use '-a' to select all instead)" : "using all" );
     }
     if ( adoptCandidates ) {
	 inputs.CopyList( candidate1 );
     }
     else {
	 inputs.CopyList( candidateAll );
     }
 }
 else {
     inputs.CopyFrom( arg.StartPtr()->next );
 }

 for ( ; iter<=(int)inputs.N(); iter++) {
     gString sFile( sSlashedBase );
     sFile.Add( inputs.Str( iter ) );
     gFileStat aCheck( strFile = sFile.Str() );

     if ( aCheck.IsDirectory() ) {
	 fprintf(fRepErr,"Ignored dir: %s\n", strFile);
     }
     else {
	 if ( aCheck.HasStat() ) {
	     files.Add( strFile );
	 }
	 else {
	     lastOpError = aCheck.lastOpError;
	     errors.Add( strFile );
	 }
     }
 }

 error = errors.N()>0;
 if ( error ) {
     for (ptrElem=errors.StartPtr(); ptrElem; ptrElem=ptrElem->next) {
	 fprintf(fRepErr,"File not found: %s\n",ptrElem->Str());
     }
     MM_LOG(LOG_ERROR, "Invalid file%s (%d): %u",
	    errors.N()==1 ? "" : "s",
	    lastOpError,
	    errors.N());
     return 2;  // something like... not-found
 }

 DBGPRINT("DBG: iter=%d/%d, files#%u, errors#%u, strFile={%s}, substExpress#%u={%s}\n",
	  iter, nArgs,
	  files.N(),
	  errors.N(),
	  strFile,
	  substExpress.N(),
	  substExpress.Str( 1 ));

 if ( strFile ) {
     if ( opt.level > 3 ) {
	 printf("%s (...) ", files.Str( 1 ));
	 if ( sNames.Find( '{' ) || sNames.Find( '}' ) ) {
	     printf("substitute %s\n", sNames.Str());
	 }
	 else {
	     printf("substitute {%s}\n", sNames.Str());
	 }
     }

     error = bulk_rename( files, substExpress, news );

     for (gElem* ptr=files.StartPtr(); ptr; ptr=ptr->next) {
	 int foundPos( ptr->me->iValue );
	 if ( foundPos ) {
	     itemCount++;
	 }
     }

     if ( error ) {
	 if ( opt.level ) {
	     fprintf(stderr,"Pattern:\t%s\n",sNames.Str());

	     for (ptrIter=files.StartPtr(); ptrIter; ptrIter=ptrIter->next) {
		 int foundPos( ptrIter->me->iValue );

		 if ( opt.level>3 ) {
		     printf("%-3s  %s\n",
			    foundPos ? "F'" : "nf'",
			    ptrIter->Str());
		 }
		 else {
		     printf("  '  %s\n", ptrIter->Str());
		 }
	     }

	     if ( itemCount ) {
		 printf("  '  %u, missing: %d\n",
			files.N(),
			(int)files.N() - itemCount);
	     }
	     else {
		 printf("  '  None found!\n");
	     }
	 }

	 MM_LOG(LOG_ERROR, "Do not know how to rename%s",
		itemCount ? " (mixed matches)" : "");
	 return 1;
     }

     error = files.N() != news.N();
     if ( error ) {
	 MM_LOG(LOG_ERROR, "Internal error (%u vs %u)", files.N(), news.N());
     }
     else {
	 iter = 1;
	 for (ptrElem=news.StartPtr(); ptrElem; ptrElem=ptrElem->next, iter++) {
	     strFile = ptrElem->Str();
	     strOldName = files.Str( iter );
	     if ( opt.level>=3 ) {
		 fprintf(fRepErr,"%02d\t",iter);
	     }
	     fprintf(fOut,"%s\n",strFile);
	     fprintf(fOut,"%s%s\n\n",
		     opt.level>=3 ? "\t" : "",
		     strOldName);
	 }

	 ASSERTION(errors.N()==0,"!");

	 if ( showOnly ) return 0;

	 // Do the actual rename:

	 iter = 1;
	 for (ptrElem=news.StartPtr(); ptrElem; ptrElem=ptrElem->next, iter++) {
	     strFile = ptrElem->Str();
	     strOldName = files.Str( iter );
	     error = rename( strOldName, strFile )!=0;
	     if ( error ) {
		 error = errno;
		 countErrors++;
		 errors.Add( (char*)strOldName );
		 errors.EndPtr()->me->iValue = error;
	     }
	 }

	 if ( countErrors ) {
	     for (ptrElem=errors.StartPtr(); ptrElem; ptrElem=ptrElem->next) {
		 error = ptrElem->me->iValue;
		 MM_LOG(LOG_ERROR, "Rename not done (%d): %s",
			error,
			ptrElem->Str());
	     }
	     error = 4;
	 }
     }
 }
 else {
     MM_LOG(LOG_WARN, "No inputs");
 }
 return error;
}


int do_media_ren (gList& arg,
		  FILE* fOut,
		  FILE* fRepErr,
		  int mask,
		  sOptBox& opt)
{
 int error( 0 );
 int nArgs( (int)arg.N() );
 //const bool showOnly( mask==1 );

 ASSERTION(fRepErr,"fRepErr");
 ASSERTION(fOut,"fOut");

 if ( nArgs==0 ) {
     fprintf(fRepErr,"Missing file or path.\n");
     return 1;
 }
 else {
     if ( nArgs==1 ) {
	 //thisCwd = arg.Match( "." )>0;
     }
     else {
	 MM_LOG(LOG_WARN, "Only implemented one path!");
     }
 }

 gFileFetch rules( nil, -1 );
 gString returnMsg;

 gList* toDo( new_rename_rules( rules.aL, 0, returnMsg ) );
 gList* grab( nil );
 ASSERTION(toDo,"mem!");
 error = returnMsg.Length()>0;
 if ( error ) {
     fprintf(fRepErr,"ERROR: %s\n",returnMsg.Str());
     MM_LOG(LOG_ERROR, "%s",returnMsg.Str());
 }
 else {
     gString usedExtension;
     grab = new_files_there( arg.Str( 1 ), *toDo, usedExtension );
     if ( grab ) {
	 grab->Show();
	 delete grab;
     }
     else {
	 error = errno;
     }

     DBGPRINT("usedExtension: {%s}\n", usedExtension.Str());
 }

 delete toDo;
 return error;
}


int do_smart_rename (gList& arg,
		     FILE* fOut,
		     FILE* fRepErr,
		     bool showOnly,
		     sOptBox& opt)
{
 int error( arg.N()<4 );
 unsigned idx( 2 ), posBy( arg.Match( "by" ) );
 gString sPath( arg.Str( 1 ) );
 char* str;

 ASSERTION(fRepErr,"fRepErr");

 if ( error ) {
     fprintf(fRepErr, "Need at least 4 args.\n");
     return 1;
 }

 error = posBy==0;
 if ( error ) {
     fprintf(fRepErr, "Missing 'by'.\n");
     return 1;
 }

 gString sFrom, sTo;
 Media nameBy;
 Media* dir;
 Media m;

 for ( ; idx<posBy; idx++) {
     str = arg.Str( idx );
     if ( str[ 0 ] ) {
	 if ( sFrom.Length() ) {
	     sFrom.Add( ' ' );
	 }
	 sFrom.Add( str );
     }
 }

 for (idx++; idx<=arg.N(); idx++) {
     str = arg.Str( idx );
     if ( str[ 0 ] ) {
	 if ( sTo.Length() ) {
	     sTo.Add( ' ' );
	 }
	 sTo.Add( str );
     }
 }

 dir = m.DirList( sPath.Str() );

 nameBy.Add( sFrom );
 nameBy.Add( sTo );
 nameBy.AppendObject( dir );

 if ( dir ) {
     printf("m.DirList {%s} (#%u)\n", dir->display.Str(), dir->N()); dir->Show(); printf("\n");
 }

 Media* listed( smart_rename( nameBy, 0 ) );

 printf("listed:\n"); listed->Show();
 delete listed;

 return 0;
}


int do_delete (gList& arg,
	       FILE* fOut,
	       FILE* fRepErr,
	       int mask,
	       sOptBox& opt)
{
 const bool showOnly( (mask & 1)!=0 );
 const bool tryMatch( mask==2 );
 int error( 0 );
 int countMissing( 0 );
 int iter( 1 ), maxIter( (int)arg.N() );
 int code( -1 );
 gString aName;
 gList toDel;
 gList final;
 gElem* ptrElem;

 ASSERTION(fOut,"fOut");

 for ( ; iter<=maxIter; iter++) {
     aName.Trim();
     if ( aName.Length() ) {
	 aName.Add( ' ' );
     }
     aName.Add( arg.Str( iter ) );
 }
 aName.Trim();

 if ( aName.Length() ) {
     toDel.Add( aName );
 }

 // Build list from input
 if ( maxIter<=0 ) {
     gFileFetch input( nil, -1 );
     toDel.CopyList( input.aL );
 }

 for (ptrElem=toDel.StartPtr(); ptrElem; ptrElem=ptrElem->next) {
     gString* ptrMe( (gString*)ptrElem->me );
     ptrMe->Trim();
     if ( ptrMe->Length() ) {
	 gFileStat aStat( ptrMe->Str() );
	 code = aStat.HasStat() && aStat.status.IsDirectory()==false;
	 if ( code==0 ) {
	     if ( tryMatch ) {
		 gList* matches( new_matched_file( *ptrMe, mask & 2 ) );
		 ASSERTION(matches,"nil");
		 code = (int)matches->N();
		 if ( code==1 ) {
		     final.Add( matches->Str() );
		 }
		 else {
		     MM_LOG(LOG_WARN, "%s: %s",
			    (code>1) ? "Ambiguous" : "No match",
			    ptrMe->Str());
		 }
		 delete matches;
	     }
	 }
	 else {
	     final.Add( *ptrMe );
	 }
	 if ( code==0 ) {
	     countMissing++;
	     MM_LOG(LOG_ERROR, "Error: %s", ptrMe->Str());
	 }
     }
 }

 if ( showOnly ) {
     for (ptrElem=final.StartPtr(); ptrElem; ptrElem=ptrElem->next) {
	 fprintf(fOut,"%s\n",ptrElem->Str());
     }
     return 0;
 }

 for (ptrElem=final.StartPtr(); ptrElem; ptrElem=ptrElem->next) {
     code = remove( ptrElem->Str() )!=0;
     if ( code ) {
	 MM_LOG(LOG_WARN, "Cannot remove (%d): %s",
		errno,
		ptrElem->Str());
	 error = 1;
     }
 }

 if ( countMissing ) {
     return 2;
 }
 return error;
}


int generate (gList& arg,
	      FILE* fOut,
	      FILE* fRepErr,
	      sOptBox& opt)
{
 int error( -1 );
 int level( mylog.dbgLevel );
 int iter( 1 ), nArgs( (int)arg.N() );
 int itemCount( 0 );
 int diffLen( 0 );
 char* strLine( nil );
 gList lines;
 gList output;
 gElem* ptrElem;
 gString* myStr( nil );
 gString bogusFirst;

 const bool unique( opt.areAll==false );
 const bool checkExists( (opt.doCheck & 1)!=0 );
 const bool dumpHeader( opt.doAppend==false );
 const int mask( 1 );

 sCrossLinkConf linkConf;
 const bool sortByOccurrence( linkConf.byOccurrence );

 ASSERTION(fRepErr,"fRepErr");
 ASSERTION(fOut,"fOut");

 linkConf.complainFileCheck = opt.level>3;
 linkConf.appendedOutput = opt.doAppend;

 if ( fOut==stdout ) {
     mylog.dbgLevel = 0;
 }

 error = jcl_config( opt.sConfigFile.Str(), linkConf );
 if ( error ) {
     linkConf.substed.Release();
     fprintf(fRepErr,"Error-code %d reading conf; ignoring.\n",error);
     return 1;
 }

 jcl_get_extension( opt.sOutput, 0, opt.sOutExt );
 ePlaylistType playType( jcl_which_type( opt.sOutput, opt.sOutExt ) );

 for (iter=1; iter<=nArgs; iter++) {
     sInPlaylist plays;
     gString sPlay( arg.Str( iter ) );

     if ( sPlay.IsEmpty() ) continue;

     error = jcl_anyread_playlist( linkConf, sPlay, plays );
     DBGPRINT("DBG: playlist %s: error %d, N()=%u\n",sPlay.Str(), error, plays.items.N());
     if ( error ) {
	 fprintf(fRepErr,"Playlist read failed (%d): %s\n",
		 error,
		 sPlay.Str());
     }
     else {
	 error = jcl_dump( linkConf, plays, lines );
	 DBGPRINT("DBG: jcl_generated {%s} error: %d, N()=%u\n", sPlay.Str(), error, lines.N());
	 MM_LOG(LOG_INFO, "%s: read %d line(s)", sPlay.Str(), lines.N());
     }
 }

 linkConf.maxCounter = opt.zValue;

 if ( nArgs==0 ) {
     sInPlaylist plays;
     gString sPlay;
     error = jcl_anyread_playlist( linkConf, sPlay, plays );
     DBGPRINT("DBG: playlist (stdin): %d\n", error);
     if ( error ) {
	 fprintf(fRepErr,"Playlist read failed (%d): %s\n",
		 error,
		 sPlay.Str());
     }
     else {
	 error = jcl_generated( linkConf, plays, mask, lines );
     }
 }

 for (ptrElem=lines.StartPtr(); ptrElem; ptrElem=ptrElem->next) {
     myStr = trim_filepath( ptrElem->Str() );
     if ( (*myStr)[ myStr->Length() ]==':' ) {
	 delete myStr;
	 continue;
     }
     strLine = myStr->Str();
     DBGPRINT("DBG: len of Str()=%u, len of myStr=%d\n", ptrElem->Length(), (int)strlen( strLine ));
     diffLen += (int)ptrElem->Length() - strlen( strLine );

     gString sNew( opt.sBaseDir.Length() + myStr->Length() + 16, '\0' );

     if ( strLine[ 0 ]=='#' ) {
     }
     else {
	 char slash;

	 if ( jcl_absolute_path( myStr->Str() )==0 ) {
	     sNew.AddString( opt.sBaseDir );
	 }
	 if ( str_slash( sNew.Str(), slash ) ) {
	     if ( sNew[ sNew.Length() ]!=slash ) {
		 sNew.Add( slash );
	     }
	 }
	 sNew.Add( strLine );

	 output.Add( sNew );
	 output.EndPtr()->me->iValue = ptrElem->me->iValue;
	 DBGPRINT("DBG: Read entry %u, base={%s}, {%s}\n", output.N(), opt.sBaseDir.Str(), sNew.Str());
     }
     delete myStr;
 }

 gList names;
 gList* pCopy( nil );
 int dupped, nDups( 0 ), nHits( -1 ), maxHits( 0 );
 int notExisting( 0 );

 // Remove duplicates if requested

 if ( unique ) {
     int idx( 0 );

     pCopy = copy_names( output );

     for (ptrElem=pCopy->StartPtr(); ptrElem; ptrElem=ptrElem->next) {
	 idx++;
	 if ( names.Match( ptrElem->Str() ) ) {
	     names.CurrentPtr()->iValue++;
	     nDups++;
	 }
	 else {
	     names.Add( ptrElem->Str() );
	     names.EndPtr()->iValue = 1;
	     names.EndPtr()->me->iValue = idx;
	 }
     }

     for (ptrElem=names.StartPtr(); ptrElem; ptrElem=ptrElem->next) {
	 dupped = ptrElem->iValue;
	 if ( dupped > maxHits ) {
	     maxHits = dupped;
	 }
     }

     ASSERTION(maxHits>=0,"maxHits>=0");
     if ( sortByOccurrence==false ) {
	 maxHits = 0;
     }

     gList hitsList[ maxHits+1 ];

     for (nHits=maxHits; nHits>0; nHits--) {
	 for (ptrElem=names.StartPtr(); ptrElem; ptrElem=ptrElem->next) {
	     dupped = ptrElem->iValue;
	     if ( dupped==nHits ) {
		 output.Str( ptrElem->me->iValue );  // place pointer at the first index found
		 hitsList[ nHits ].Add( ptrElem->Str() );
		 hitsList[ nHits ].EndPtr()->me->iValue = output.CurrentPtr()->me->iValue;
	     }
	 }
     }

     if ( sortByOccurrence==false ) {
	 for (ptrElem=names.StartPtr(); ptrElem; ptrElem=ptrElem->next) {
	     output.Str( ptrElem->me->iValue );  // place pointer at the first index found
	     hitsList[ nHits ].Add( ptrElem->Str() );
	     hitsList[ nHits ].EndPtr()->me->iValue = output.CurrentPtr()->me->iValue;
	 }
     }

     output.Delete();
     delete pCopy;

     pCopy = new gList;
     ASSERTION(pCopy,"pCopy");

     for (nHits=maxHits; nHits>=0; nHits--) {
	 ptrElem = hitsList[ nHits ].StartPtr();
	 if ( ptrElem!=nil && maxHits>1 ) {
	     MM_LOG(LOG_INFO, "Dups #%d %s%s",
		    nHits,
		    ptrElem->Str(),
		    hitsList[ nHits ].N() ? " [...]" : "");
	 }

	 for ( ; ptrElem; ptrElem=ptrElem->next) {
	     pCopy->Add( ptrElem->Str() );
	     pCopy->EndPtr()->me->iValue = ptrElem->me->iValue;
	 }
     }

     sSimilarMedia sim;
     sim.AddNames( *pCopy );
     if ( sim.Analysis() ) {
	 delete pCopy;

#ifdef DEBUG
	 sim.Show();
#endif //DEBUG

	 pCopy = copy_names( *sim.pList );
     }

     for (ptrElem=pCopy->StartPtr(); ptrElem; ptrElem=ptrElem->next) {
	 output.Add( ptrElem->Str() );
	 output.EndPtr()->me->iValue = ptrElem->me->iValue;
     }

     delete pCopy; pCopy = nil;
 }//end IF (unique)

#if 0
 int cnt( 0 );
 for (ptrElem=output.StartPtr(); ptrElem; ptrElem=ptrElem->next) {
     strLine = ptrElem->Str();
     DBGPRINT("DBG: line (type: %d) %d: {%s}\n", ptrElem->me->Kind(), ++cnt, strLine);
     printf("%d\t%d\t%s\n",
	    ptrElem->iValue,
	    ptrElem->me->iValue,
	    ptrElem->me->Str());
 }
#endif

 if ( dumpHeader && output.N() ) {
     switch ( playType ) {
     case e_pl_PLS:
	 fprintf(fOut, "[playlist]\r\n");
	 break;
     case e_pl_M3U:
	 fprintf(fOut, "#EXTM3U\r\n");
	 break;
     case e_pl_VPL:
	 fprintf(fOut, "#VUPlayer playlist\r\n");
	 break;
     case e_pl_raw:
     case e_pl_Unknown:
     default:
	 break;
     }
 }

 for (ptrElem=output.StartPtr(); ptrElem; ptrElem=ptrElem->next) {
     myStr = (gString*)ptrElem->me;
     myStr->Trim();
     strLine = myStr->Str();

     bool showItem( myStr->Length() && myStr->Find( '.' ) );
     if ( showItem ) {
	 if ( linkConf.maxCounter > 0 && itemCount >= linkConf.maxCounter ) {
	     fprintf(fRepErr, "Skipped, ignored: %s\n", strLine);
	     break;
	 }

	 DBGPRINT("Check file? %c, absolute? %c, {%s}\n",
		  ISyORn( checkExists ),
		  ISyORn( jcl_absolute_path( strLine )),
		  strLine);
	 if ( checkExists ) {
	     if ( jcl_absolute_path( strLine ) || jcl_absolute_path( opt.sBaseDir.Str() ) ) {
		 gFileStat aStat( strLine );
		 showItem = aStat.HasStat() && aStat.IsDirectory()==false;
		 if ( showItem==false ) {
		     if ( notExisting==0 ) {
			 bogusFirst.Set( strLine );
		     }
		     notExisting++;
		 }
	     }
	 }
     }
     if ( showItem ) {
	 itemCount++;
	 fprintf(fOut,"%s\r\n", strLine);
     }
 }//end FOR

 if ( opt.sLogFile.Length() || fOut!=stdout ) {
     mylog.dbgLevel = level;
 }

 if ( nDups ) {
     MM_LOG(LOG_WARN, "Skipped %d dup(s)", nDups);
 }
 if ( notExisting ) {
     MM_LOG(LOG_WARN, "Missing file(s): %d, first: %s", notExisting, bogusFirst.Str());
 }
 if ( diffLen ) {
     MM_LOG(LOG_NOTICE, "Trim on input, #lines: %d", diffLen / 2);
 }

 MM_LOG(LOG_INFO, "M3U items: %d", itemCount);

 DBGPRINT("DBG: generate returns %d, N()=%u\n", error, output.N());
 return error;
}


int do_generated (gList& arg,
		  FILE* fOut,
		  FILE* fRepErr,
		  sOptBox& opt)
{
 int error( -1 );
 int secs( 0 ), totalSecs( 0 );
 int level( mylog.dbgLevel );
 int iter( 1 ), nIter( (int)arg.N() ), nLists( nIter );
 int nrItems( 0 ), itemCount( 0 );
 int itemsShown( 0 );
 unsigned pos( 0 );
 char* strLine( nil );
 gList files;
 gList lines;
 gList output, indexes, listedErrors;
 gElem* ptrElem;
 bool unknownSecs( false );
 bool anyProgress( false );

 const int mask( opt.areAll==false );

 sCrossLinkConf linkConf;

 ASSERTION(fRepErr,"fRepErr");
 ASSERTION(fOut,"fOut");

 linkConf.showProgress = opt.level>3;
 linkConf.complainFileCheck = opt.level>3;
 linkConf.appendedOutput = opt.doAppend;

 if ( fOut==stdout ) {
     mylog.dbgLevel = 0;
 }

 error = jcl_config( opt.sConfigFile.Str(), linkConf );
 if ( error ) {
     linkConf.substed.Release();
     fprintf(fRepErr,"Error-code %d reading conf; ignoring.\n",error);
     return 1;
 }

 for ( ; iter<=nIter; iter++) {
     char* strItem( arg.Str( iter ) );
     gFileStat check( strItem );

     error = check.HasStat()==false;
     if ( error ) {
	 fprintf(fRepErr,"Invalid item (%d): %s\n",
		 check.lastOpError,
		 strItem);
	 return 2;
     }
     if ( check.IsDirectory() ) {
	 fprintf(stderr,"Dir is not yet implemented: %s\n",strItem);
     }
     else {
	 files.Add( strItem );
     }
 }

 if ( error==-1 ) {
     // Use stdin
     nLists = 0;
 }

 for (iter=1, nIter=(int)files.N(); iter<=nIter; iter++) {
     sInPlaylist plays;
     gString sPlay( files.Str( iter ) );

     if ( sPlay.IsEmpty() ) continue;

     error = jcl_anyread_playlist( linkConf, sPlay, plays );
     DBGPRINT("DBG: playlist %s: %d\n",sPlay.Str(), error);
     if ( error ) {
	 fprintf(fRepErr,"Playlist read failed (%d): %s\n",
		 error,
		 sPlay.Str());
     }
     else {
	 error = jcl_generated( linkConf, plays, mask, lines );
     }
     itemCount += (int)plays.items.N();
 }

#ifdef OLD_MAX
 linkConf.maxCounter = opt.zValue;
#endif

 if ( nLists==0 ) {
     sInPlaylist plays;
     gString sPlay;
     error = jcl_anyread_playlist( linkConf, sPlay, plays );
     DBGPRINT("DBG: playlist (stdin): %d\n", error);
     if ( error ) {
	 fprintf(fRepErr,"Playlist read failed (%d): %s\n",
		 error,
		 sPlay.Str());
     }
     else {
	 error = jcl_generated( linkConf, plays, mask, lines );
     }
     itemCount += (int)plays.items.N();
 }

 for (ptrElem=lines.StartPtr(); ptrElem; ptrElem=ptrElem->next) {
     gString* myStr( (gString*)ptrElem->me );
     strLine = myStr->Str();
     secs = myStr->iValue;

     if ( (*myStr)[ myStr->Length() ]==':' ) continue;

     if ( strLine[ 0 ] ) {
	 if ( linkConf.maxCounter > 0 && (int)output.N() >= linkConf.maxCounter ) {
	     fprintf(fRepErr, "Skipped, ignored: %s\n", strLine);
	     break;
	 }

	 gString sNew( opt.sBaseDir.Length() + myStr->Length() + 16, '\0' );

	 if ( strLine[ 0 ]=='#' ) {
	     pos = myStr->Find( '\n' );
	     if ( pos>0 && jcl_absolute_path( myStr->Str( pos ) )==0 ) {
		 sNew.CopyFromTo( *myStr, 1, pos );
		 sNew.AddString( opt.sBaseDir );
		 sNew.Add( strLine + pos );
	     }
	     else {
		 sNew.Add( strLine );
	     }
	 }
	 else {
	     if ( jcl_absolute_path( myStr->Str() )==0 ) {
		 sNew.AddString( opt.sBaseDir );
	     }
	     sNew.Add( strLine );
	 }

	 output.Add( sNew );
	 if ( sNew[ sNew.Length() ]=='\n' ) {
	     sNew.Delete( sNew.Length() );
	 }
	 pos = sNew.FindBack( '\n' );
	 output.EndPtr()->me->iValue = (int)pos;
	 indexes.Add( (int)output.N() );

	 totalSecs += secs;
	 if ( secs==0 ) {
	     unknownSecs = true;
	 }

	 if ( linkConf.showProgress ) {
	     static t_uint16 progress;
	     if ( ((++progress) % 100)==0 ) {
		 anyProgress = true;
		 if ( sNew.Length() > 40 ) {
		     sNew.InsertStr( "[...]", 10 );
		     sNew.Delete( 15, sNew.Length() - 9 );
		 }
		 fprintf(fRepErr, "\rReading (%u), last: %-42s ", output.N(), sNew.Str());
	     }
	 }//end IF show progress
     }
 }//end FOR

 if ( opt.sLogFile.Length() || fOut!=stdout ) {
     mylog.dbgLevel = level;
 }

 if ( anyProgress ) {
     fprintf(fRepErr, "\r\n\n");
 }

 if ( mask ) {
     gRandom any;
#if DEBUG
     ;
#else
     any.GarbleSeed();
#endif
     nrItems = (opt.zValue ? opt.zValue : (int)output.N());
     random_listed( fOut, output, nrItems, indexes, listedErrors, itemsShown );
 }
 else {
     for (ptrElem=output.StartPtr(); ptrElem; ptrElem=ptrElem->next) {
	 fprintf(fOut,"%s",ptrElem->Str());
	 itemsShown++;
     }
 }

 for (ptrElem=listedErrors.StartPtr(); ptrElem; ptrElem=ptrElem->next) {
     error = 1;
     if ( opt.level>3 ) {
	 MM_LOG(LOG_WARN, "File not found: %s", ptrElem->Str());
     }
 }

 if ( unknownSecs ) {
     fprintf(fRepErr,"Listed%s %d (of %d)\n",
	     mask ? " (random)" : "",
	     itemsShown,
	     itemCount);
 }
 else {
     fprintf(fRepErr,"Listed%s %d (of %d): %d sec(s)\n",
	     mask ? " (random)" : "",
	     itemsShown,
	     itemCount,
	     totalSecs);
 }
 DBGPRINT("DBG: generated returns %d\n",error);
 return error;
}


int do_sfv_check (gList& arg,
		  FILE* fOut,
		  FILE* fRepErr,
		  sOptBox& opt)
{
 int error( 0 );
 int code;
 int missing( 0 );
 gElem* ptrElem( arg.StartPtr() );
 iEntry* sfvLines;
 const char* strFile( nil );
 const char* strPath( nil );
 const bool verbose( opt.level>=3 );
 const bool filesMatch( opt.doCheck );
 const bool checkAll( opt.areAll );

 ASSERTION(fOut, "fOut");

 if ( filesMatch ) {
     for ( ; ptrElem; ptrElem=ptrElem->next) {
	 iEntry* there( nil );
	 iEntry extensions;
	 iEntry builtSFV;
	 gElem* pLine;
	 gList valid;
	 gList invalid;

	 strFile = ptrElem->Str();
	 sfvLines = sfv_entries_from_file( strFile );

	 if ( sfvLines ) {
	     strPath = sfvLines->sComment.Str();
	     there = new_files_at_dir( strPath, extensions );
	     DBGPRINT("DBG: {%s} path: %s; there: 0x%p\n", strFile, sfvLines->sComment.Str(), there);
	 }

	 // Example:
	 //	bla.sfv contains
	 //		a.mp3 AB123456
	 //		b.flac 1234567A
	 //	there (the corresponding dir) has:
	 //		a.mp3, b.flac, c.mp3
	 // In this case not all *.mp3 files match, it's ambiguous.

	 if ( there ) {
	     for (pLine=sfvLines->StartPtr(); pLine; pLine=pLine->next) {
		 gString sName( pLine->Str() );
		 char* strExt( jcl_extension( sName, true ) );

		 if ( strExt[ 0 ] ) {
		     if ( extensions.Match( strExt ) ) {
			 extensions.CurrentPtr()->iValue++;
			 DBGPRINT("0x%08X {%s}\n", pLine->me->iValue, sName.Str());

			 iEntry* newEntry( new iEntry );
			 newEntry->Add( strExt );
			 newEntry->iValue = pLine->me->iValue;
			 builtSFV.AppendObject( newEntry );
		     }
		     else {
			 extensions.iValue++;
			 DBGPRINT("DBG: mismatch extension {%s} (#%d)\n", strExt, extensions.iValue);
			 MM_LOG(LOG_ERROR, "Extension '%s' not found", strExt);
		     }
		 }
	     }

	     if ( extensions.iValue==0 ) {
		 for (pLine=extensions.StartPtr(); pLine; pLine=pLine->next) {
		     DBGPRINT("DBG: EXT %d %d {%s}\n",
			      pLine->iValue,
			      pLine->me->iValue,
			      pLine->Str());
		     if ( pLine->iValue ) {
			 if ( pLine->iValue == pLine->me->iValue ) {
			     valid.Add( pLine->Str() );
			 }
			 else {
			     invalid.Add( pLine->Str() );
			 }
		     }
		 }

		 if ( invalid.N() ) {
		     printf("Mismatch count for extension(s): "); invalid.Show();
		 }
		 else {
		     for (pLine=there->StartPtr(); pLine; pLine=pLine->next) {
			 char* strExt( jcl_extension( *((gString*)pLine->me), true ) );
			 if ( valid.Match( strExt ) ) {
			     DBGPRINT("DBG: bingo {%s}\n", pLine->Str());

			     for (gElem* fetch=builtSFV.StartPtr(); fetch; fetch=fetch->next) {
				 iEntry* pEntry( (iEntry*)fetch->me );
				 if ( fetch->iValue==0 && strcmp(pEntry->Str(), strExt)==0 ) {
				     fetch->iValue = 1;
				     ((gString*)pEntry->StartPtr()->me)->Set( pLine->Str() );
				     break;
				 }
			     }
			 }
			 else {
			     pLine->iValue = 1+(strcmp( strExt, ".sfv" )!=0);  // 2: file not there; 1: SFV
			 }
		     }

		     for (pLine=builtSFV.StartPtr(); pLine; pLine=pLine->next) {
			 DBGPRINT("built mark: %d, 0x%08X %s\n", pLine->iValue, pLine->me->iValue, pLine->Str());
			 if ( pLine->iValue!=1 ) {
			     MM_LOG(LOG_WARN, "Unsolved entry: %s", pLine->Str());  // internal error?!
			 }
		     }

		     code = sfv_check( builtSFV, strPath, fOut, fRepErr, missing );
		     if ( code ) {
			 MM_LOG(LOG_ERROR, "%d (%u) error(s)%s at: %s",
				code, builtSFV.N(),
				missing ? ", missing file(s)" : "",
				strFile);
			 error = 1;
		     }
		 }
	     }
	 }//end IF 'there'

	 if ( checkAll ) {
	     missing = 0;
	     for (pLine=there->StartPtr(); pLine; pLine=pLine->next) {
		 bool notChecked( pLine->iValue >= 2 );

		 if ( notChecked ) {
		     missing++;
		 }
		 DBGPRINT("There: %d, %d {%s}\n", pLine->iValue, pLine->me->iValue, pLine->Str());
	     }

	     if ( missing ) {
		 error |= 4;
		 MM_LOG(LOG_ERROR, "Not all files checked (%d)", missing);
	     }
	 }

	 delete there;
	 delete sfvLines;
     }//end FOR (each arg)
 }

 for ( ; ptrElem; ptrElem=ptrElem->next) {
     strFile = ptrElem->Str();
     sfvLines = sfv_entries_from_file( strFile );

     if ( sfvLines ) {
	 code = sfv_check( *sfvLines, nil, verbose ? fOut : nil, fRepErr, missing );
	 if ( code ) {
	     MM_LOG(LOG_ERROR, "%d (%u) error(s)%s at: %s",
		    code, sfvLines->N(),
		    missing ? ", missing file(s)" : "",
		    strFile);
	     error = 1;
	 }
     }
     else {
	 error = 2;
	 MM_LOG(LOG_ERROR, "Cannot read: %s", strFile);
     }

     delete sfvLines;
 }

 return error;
}


int do_sfv_dump (gList& arg,
		 FILE* fOut,
		 FILE* fRepErr,
		 sOptBox& opt)
{
 int error( 0 );
 int thisError;
 gElem* pElem;
 bool newLineCR( false );

 jcl_get_extension( opt.sOutput, 0, opt.sOutExt );
 newLineCR = opt.sOutExt.Match( ".sfv" );

 for (pElem=arg.StartPtr(); pElem; pElem=pElem->next) {
     iEntry entries;
     char* str( pElem->Str() );
     gFileStat aStat( str );

     if (aStat.IsDirectory() ) {
	 MM_LOG(LOG_INFO, "Currently SFV dump skips dirs: %s", str);
     }
     else {
	 if ( aStat.HasStat() ) {
	     entries.Add( str );
	     thisError = sfv_dump( entries, nil, newLineCR, fOut, fRepErr );
	     if ( thisError ) {
		 error = 1;
	     }
	     DBGPRINT("DBG: thisError {%s}, newLineCR? %c\n",
		      str,
		      ISyORn( newLineCR ));
	 }
	 else {
	     MM_LOG(LOG_NOTICE, "Skipped: %s", str);
	 }
     }
 }

 return error;
}

////////////////////////////////////////////////////////////
int go (int argc, char** argv, char** envp)
{
 int error;
 int forcedDebug( -1 );
 gArg arg( argv, envp );
 sOptBox opt;
 char* str;
 int cmdNr;
 unsigned n;
 bool isVerbose( false );
 bool isVeryVerbose( false );
 gList* myArgs( nil );

 FILE* fOut( stdout );
 FILE* fRepErr( nil );

 arg.AddParams( 1, "-vv|--verbose -h|--help --version\
 -a|--all\
 --append\
 -b:%s|--base-dir:%s\
 -c:%s|--config:%s\
 -d:%d|--debug:%d\
 -f|--file\
 -p:%d|--perm:%d\
 -l:%s|--log:%s\
 -m:%d|--max:%d\
 -o:%s|--output:%s\
 -z:%s|--value:%s\
" );

 // Parse command line
 error = arg.FlushParams();
 n = arg.N();
 arg.FindOptionOccurr( "verbose", isVerbose, isVeryVerbose );

 gString firstCmd( arg.Str(1) );
 char* cmdStr( firstCmd.Str() );
 arg.Delete( 1, 1 );

 if ( arg.FindOption("version") ) {
     print_version( arg.Program() );
     return 0;
 }

 if ( arg.FindOption('h') || error!=0 || n==0 ) {
     if ( n>0 && cmdStr[0]!='-' ) {
	 print_command_help( arg.Program(), cmdStr );
     }
     else {
	 print_help( str = arg.Program() );
	 //printf("For command help, use: %s command --help\n",str);
     }
     return 0;
 }

 if ( isVerbose ) {
     fRepErr = stderr;
 }

 opt.areAll = arg.FindOption('a');
 opt.doCheck = arg.FindOption('f');

 if ( arg.FindOption("perm", opt.sValue) ) {
     int pValue( atoi( opt.sValue.Str() ) );
     opt.permShow = pValue;
     if ( pValue<0 or pValue>3 ) {
	 return 0;
     }
     opt.doOnlyToMe = (pValue & 1)!=0;
     opt.doOnlyPublic = pValue==2;
 }
 if ( arg.FindOption("value",opt.sValue) ) {
     opt.zValue = atoi( opt.sValue.Str() );
 }

 arg.FindOption("max",opt.maxDepth);

 if ( arg.FindOption("debug",opt.debugLevel) ) {
     if ( opt.level<0 || opt.level>9 ) {
	 fprintf(stderr,"Invalid debug level (0...9): %d\n",opt.level);
     }
     forcedDebug = mylog.dbgLevel = opt.debugLevel;
 }

 if ( arg.FindOption('l',opt.sLogFile) ) {
     int fdLog( -1 );
     mmdb_entry_trim( opt.sLogFile, 0 );
     if ( gio_dir_status( opt.sLogFile.Str(), error ) ) {
	 // No directory with such name, can be a new file
	 fdLog = ilf_append( opt.sLogFile.Str(), ILF_APPEND_MASK, error );
	 if ( fdLog==-1 ) {
	     fprintf(stderr,"Cannot use log file (%d): %s\n",error,opt.sLogFile.Str());
	     return 1;
	 }
	 ilf_close( fdLog );

	 mylog.SetName( opt.sLogFile );
	 fRepErr = mylog.Stream();
     }
     else {
	 if ( fRepErr ) {
	     fprintf(fRepErr,"Invalid log (%d): %s\n",
		     error,
		     opt.sLogFile.Str());
	 }
	 return 2;
     }
 }

 if ( arg.FindOption('b',opt.sBaseDir) ) {
     gString sNew( opt.sBaseDir.Str() );
     opt.sBase.Set( str_simpler_path( sNew.Str() ) );
 }

 arg.FindOption('c',opt.sConfigFile);

 opt.doAppend = arg.FindOption("append");
 if ( arg.FindOption('o',opt.sOutput) ) {
     if ( opt.sOutput.Length() && opt.sOutput.Match( "." )==false ) {
	 fOut = fopen( opt.sOutput.Str(), opt.doAppend ? "ab" : "wb" );
	 if ( fOut==nil ) {
	     fprintf(stderr,"Invalid output: %s\n",opt.sOutput.Str());
	     return 1;
	 }
     }
 }

 // Finalized command parsing, adjusting
 if ( forcedDebug<=-1 ) {
     opt.level += (isVerbose==true) * 3;
     opt.level += (isVeryVerbose==true) * 6;
 }
 else {
     opt.level = forcedDebug;
 }

 cmdNr = command_from_str( cmdStr );

 mylog.SetScheme( MMDB_VERSION_MAJOR,
		  MMDB_VERSION_MINOR,
		  MMDB_COMPAT_SCHEME,
		  "A-" );
 MM_LOG(LOG_INFO, "mmdb lib scheme: %s (cmd: %d)", mylog.SchemeStr(), cmdNr);

 switch ( cmdNr ) {
 case 1:  // test
     error = do_test( 1, arg, fOut, fRepErr, opt );
     break;

 case 17:  // dir
     opt.zValue += 1000;
     // no break here

 case 2:  // ls
     if ( opt.maxDepth > opt.maxAllowedDepth ) {
	 fprintf(stderr, "Warn: depth too high: %d\n", opt.maxDepth);
     }

     opt.SplitZValue( (char*)":" );

     if ( opt.zReference.N() > 0 ) {
	 const char* strRef( opt.zReference.Str( 1 ) );

	 switch ( strRef[ 0 ] ) {
	 case 0:
	     break;

	 case 'A':
	     opt.sortBy.SetCode( 1 );
	     break;
	 case 'a':
	     opt.sortBy.SetCode( 1, false );
	     break;

	 case 'M':
	     opt.sortBy.SetCode( 2 );
	     break;
	 case 'm':
	     opt.sortBy.SetCode( 2, false );
	     break;

	 case 'C':
	     opt.sortBy.SetCode( 4 );
	     break;
	 case 'c':
	     opt.sortBy.SetCode( 4, false );
	     break;

	 case 'S':
	     opt.sortBy.SetCode( 8 );
	     break;
	 case 's':
	     opt.sortBy.SetCode( 8, false );
	     break;

	 case 'N':
	     opt.sortBy.SetCode( 16 );
	     break;
	 case 'n':
	     opt.sortBy.SetCode( 16, false );
	     break;

	 default:
	     MM_LOG(LOG_WARN, "Unknown -z option: %s\n", strRef);
	     break;
	 }

	 if ( opt.zReference.N() >= 2 ) {
	     opt.sortBy.limit = opt.zReference.EndPtr()->me->iValue;
	 }
     }
     else {
	 opt.sortBy.SetCode( opt.zValue % 1000 );
     }

     //printf("opt.zValue: %d, sortBy.byCode: %d ", opt.zValue, opt.sortBy.byCode); opt.zReference.Show();

     error = do_ls( opt.zValue, arg, fOut, fRepErr, opt );
     break;

 case 3:  // dump (formerly vpl-dump)
     error = do_vpl_dump( -1, arg, fOut, fRepErr, opt );
     break;

 case 4:  // crosslink
     if ( fRepErr==nil ) fRepErr = stderr;
     error = do_crosslink( arg.Str( 1 ), 2, arg, "Crosslink", fOut, fRepErr, opt );
     break;

 case 5:  // copy
     if ( fRepErr==nil ) fRepErr = stderr;
     if ( opt.zValue ) {
	 fprintf(stderr,"copy command ignores -z option\n");
     }
     opt.zValue = 8;
     error = do_crosslink( arg.Str( 1 ), 2, arg, "Copy", fOut, fRepErr, opt );
     break;

 case 6:  // compact
     if ( fRepErr==nil ) fRepErr = stderr;
     if ( arg.N()>=1 && arg.Str( 1 )[ 0 ] ) {
	 error = do_compact( arg.Str( 1 ), arg.StartPtr()->next, fOut, fRepErr, opt );
     }
     else {
	 if ( arg.N() )
	     fprintf(stderr,"Consider '.' for current directory\n");
	 else
	     fprintf(stderr,"Few args.\n");
     }
     break;

 case 14:  // generate
     if ( fRepErr==nil ) fRepErr = stderr;
     error = generate( arg, fOut, fRepErr, opt );
     break;

 case 7:  // generated
     if ( fRepErr==nil ) fRepErr = stderr;
     error = do_generated( arg, fOut, fRepErr, opt );
     break;

 case 8:  // ren
     if ( fRepErr==nil ) fRepErr = stderr;
     if ( opt.zValue & 1 ) {
	 fprintf(fRepErr,"ren: display only\n");
     }
     error = do_rename( arg, fOut, fRepErr, opt.zValue, opt );
     if ( isVeryVerbose ) {
	 fprintf(stderr,"rename returned %d\n",error);
     }
     break;

 case 9:  // del
     if ( opt.zValue & 1 ) {
	 if ( fRepErr ) {
	     fprintf(fRepErr,"del: display only\n");
	 }
     }
     error = do_delete( arg, fOut, fRepErr, opt.zValue, opt );
     if ( isVeryVerbose ) {
	 fprintf(stderr,"delete returned %d\n",error);
     }
     break;

 case 10:  // media-ren
     if ( opt.zValue & 1 ) {
	 if ( fRepErr ) {
	     fprintf(fRepErr,"media-ren: display only\n");
	 }
     }
     error = do_media_ren( arg, fOut, stderr, opt.zValue, opt );
     if ( isVeryVerbose ) {
	 fprintf(stderr,"media-ren returned %d\n",error);
     }
     break;

 case 23:  // smart-ren
     error = do_smart_rename( arg, fOut, stderr, (opt.zValue & 1)!=0, opt );
     break;

 case 11:  // join-to
     myArgs = new gList;
     ASSERTION(myArgs,"myArgs");
     if ( opt.sOutput.Match( "." ) ) {
	 // output is the same as first argument
	 error = arg.N()<1;
	 if ( error ) {
	      fprintf(stderr, "Use at least one arg!\n");
	 }
	 else {
	      myArgs->CopyList( arg );
	 }
     }
     else {
	 myArgs->Add( "" );
	 add_to_list( arg.StartPtr(), *myArgs );
     }
     if ( error==0 ) {
	error = do_join_to( *myArgs, fOut, opt.sOutput, opt );
     }
     delete myArgs;
     break;


 case 19:  // sfv-check
     error = do_sfv_check( arg, fOut, fRepErr, opt );
     break;

 case 20:  // sfv-dump
     error = do_sfv_dump( arg, fOut, fRepErr, opt );
     break;

 default:
     error = 1;
     fprintf(stderr,"Invalid command: %s\n",cmdStr);
     break;
 }

 if ( fOut && fOut!=stdout ) fclose( fOut );
 DBGPRINT("DBG: go %s returned %d\n",
	  argv[ 1 ],
	  error);
 return error;
}

////////////////////////////////////////////////////////////
void sOptBox::SplitZValue (char* strSeparator)
{
 gElem* pElem;

 gParam copy( sValue, strSeparator );
 if ( sValue.ConvertToInt32( -1 )==-1 ) {
     for (pElem=copy.StartPtr(); pElem; pElem=pElem->next) {
	 zReference.Add( pElem->Str() );
	 zReference.EndPtr()->me->iValue = atoi( pElem->Str() );
     }
 }
}

////////////////////////////////////////////////////////////
int main (int argc, char* argv[], char* envp[])
{
 int result;

 gINIT;

 IMEDIA_INIT;
 bora_iso_adjust();

 result = go( argc, argv, envp );

 MM_LOG(LOG_INFO, "Ended (%d)", result);
 log_file_flush( mylog, -1, GX_GETPID() );
 mylog.Close();

 imb_iso_release();
 ptm_release_pool;
 ptm_finit;

 DBGPRINT_MIN("DBG: Objs undeleted: %d\n",gStorageControl::Self().NumObjs());
 gEND;

 DBGPRINT("DBG: result: %d\n",result);
 return result;
}


