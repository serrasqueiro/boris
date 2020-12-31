// listed.cpp


#include "listed.h"

#include "ioaux.h"


#define IMA_COMPRESSED_SUPPORT_EXTS "\
\
.mp3;\
.wma;\
.ogg;\
"


const char* sListingConf::defaultPreferredLists=".m3u;.pls;.vpl;m3u8";

const sFileTypeSupportDef sListingConf::defaultExtensionSupport[]={
	{ e_unknown, "Unknown", nil },
	{ e_textFile, "text/plain", ".txt;.txu" },
	{ e_otherTextFile, "text", "" },
	{ a_audioRaw, "audio", ".au" },
	{ e_audioCompressed, "audio/compressed", IMA_COMPRESSED_SUPPORT_EXTS },
	{ e_audioLossless, "audio/lossless", ".flac;.wv;.wav" },
	{ e_videoMpg2, "video/mpeg", ".mpg2" },
	{ e_videoVOB, "video/mpeg", ".vob" },
	{ e_videoBUP, "video/dvd", ".bup" },
	{ e_videoIFO, "video/dvd", ".ifo" },
	{ e_videoMpg4, "video/mpeg", ".mp4" },
	{ e_videoMpg4, "video/mpeg", ".mpg4" },
	{ e_videoWMV, "video/windows", ".wmv" },  // Windows Media Video
	{ e_videoOther, "video", "" },
	{ e_md5sum, "text/plain", ".md5sum" },
	{ e_md5suma, "text/plain", ".suma" },
	{ e_mp3check, "text/plain", ".mp3check.txt" },
	{ e_anyPlaylist, "playlist", "" },
	{ e_playList_m3u, "text/playlist", ".m3u" },
	{ e_playList_m3u8, "text/playlist", ".m3u8" },
	{ e_playList_vpl, "data/playlist", ".vpl" },
	{ e_imageUncompressed, "image/x-bmp", ".bmp" },
	{ e_imageCompressedJPG, "image/jpeg", ".jpg;.jpeg" },
	{ e_imageLossless, "image/gif", ".gif;.png" },
	{ e_isoGeneral, "application/x-iso9660", ".iso;.nrg" },
	{ e_fileUnsupported, "Unsupported", nil },
	{ e_fileUnsupported, nil, nil },
	{ e_fileUnsupported, nil, nil },
	{ e_fileUnsupported, nil, nil }
};

////////////////////////////////////////////////////////////
// sFileTypeSupport
////////////////////////////////////////////////////////////
int sFileTypeSupport::AddExtensions (const char* strExts)
{
 int count( 0 );
 gParam news( (char*)strExts, ";" );
 unsigned index( 1 ), nElems( news.N() );
 ASSERTION(strExts,"strExts");

 if ( extensions==nil ) {
     extensions = new gList;
 }
 ASSERTION(extensions,"extensions");

 supported |= IMA_BASIC_SUPPORT;

 for ( ; index<=nElems; index++) {
     gString sOne( news.Str( index ) );
     sOne.Trim();
     if ( sOne.Length() ) {
	 sOne.DownString();
	 extensions->Add( sOne );
	 supported |= IMA_NATIVE_SUPPORT;
	 count++;
     }
 }
 return count;
}

////////////////////////////////////////////////////////////
// sListingConf
////////////////////////////////////////////////////////////
void sListingConf::ToDefault ()
{
 sPreferredDirLists.Set( (char*)defaultPreferredLists );
 lastError = 0;
}


bool sListingConf::InitExtensions ()
{
 int idx( 1 );
 int extsIndex;
 const sFileTypeSupportDef* ptrSupport;
 const char* strExts;
 char* mime;
 char* userMime;

 for (int safe=1024; safe>=0; idx++, safe--) {
     ptrSupport = &defaultExtensionSupport[ idx ];
     if ( ptrSupport->fileType==e_fileUnsupported ) break;

     strExts = ptrSupport->strExts;
     extsIndex = (int)ptrSupport->fileType;
     ASSERTION(extsIndex>=0 && extsIndex<256,"extsIndex<256");

     mime = (char*)ptrSupport->mimeType;
     userMime = new char[ mime ? (strlen( mime )+1) : 8 ];
     gParam sDesc( mime, "/" );
     strcpy( userMime, sDesc.Str( 1 ) );

     supportedExtensions[ extsIndex ].idxDef = (t_uint16)idx;
     supportedExtensions[ extsIndex ].AddExtensions( ptrSupport->strExts );
     supportedExtensions[ extsIndex ].strExts = strExts;
     supportedExtensions[ extsIndex ].userMime = userMime;

     delete[] userMime;
 }
 return HashConfs();
}


bool sListingConf::HashConfs ()
{
 int extsIndex( 0 );
 int oneExt, nExts;
 gList* ptrExts;
 gStorage* myObj;

 hashedExtensions.Delete();

 for ( ; extsIndex<256; extsIndex++) {
     ptrExts = supportedExtensions[ extsIndex ].extensions;
     if ( ptrExts ) {
	 for (oneExt=1, nExts=(int)ptrExts->N(); oneExt<=nExts; oneExt++) {
	     hashedExtensions.Add( ptrExts->Str( oneExt ) );
	     myObj = hashedExtensions.EndPtr()->me;
	     myObj->iValue = extsIndex;
	     DBGPRINT_MIN("DBG: Extension %d\t%s\n",
			  extsIndex,
			  hashedExtensions.EndPtr()->Str());
	 }
     }
 }
 return true;
}


sFileTypeSupport& sListingConf::SupportByExt (const char* strExt, t_uint8& supported)
{
 int index;
 unsigned posHash( hashedExtensions.Match( (char*)strExt ) );

 if ( posHash ) {
     index = hashedExtensions.CurrentPtr()->me->iValue;
     ASSERTION(index>=0 && index<256,"index!");
     which = (unsigned)index;
 }
 else {
     which = 0;
 }
 supported = supportedExtensions[ which ].supported;
 DBGPRINT_MIN("DBG: supportedExtensions[ which=%u ].supported=%u, strExt=%s\n",
	      which,
	      supported,
	      strExt);
 return supportedExtensions[ which ];
}


////////////////////////////////////////////////////////////
// listed media functions
////////////////////////////////////////////////////////////
int ima_new_unique_name (gFileSysName* ptrSys, t_uint32 uMask, gList& result)
{
 gString* newObj;
 sFileStat* ptrStat;
 t_stamp stamp;
 bool isDuplicate;
 int code( -2 );

 // On duplicate it returns -1;
 // or -2 if cell element has been used before, already;
 // or >=1, the number of elements in list now.

 ASSERTION(ptrSys,"ptrSys");
 newObj = new gString( ptrSys->Str() );
 ASSERTION(newObj,"newObj");
 isDuplicate = result.InsertOrderedUnique( newObj )==-1;
 if ( isDuplicate ) {
     delete newObj;
     return -1;
 }

 if ( result.CurrentPtr()->iValue==0 ) {
     // the return code is -2 if element is being reused!
     code = (int)result.N();
 }

 ptrStat = &ptrSys->GetStat().status;
 stamp = ptrStat->mTime;
 result.CurrentPtr()->iValue = stamp;
 newObj->iValue = (int)(ptrStat->USize());

 return code;
}


char* ima_basename (const char* strName)
{
 char chr( 0 );
 int idx, len;

 if ( strName==nil ) return nil;
 len = strlen( strName );
 idx = len-1;

 for (idx--; idx>=0; idx--) {
     chr = strName[ idx ];
     if ( chr=='\\' || chr=='/' ) {
	 return (char*)(strName+idx+1);
     }
 }
 return (char*)strName;
}


char* ima_dirname (const char* strName)
{
 // Similar to gString::DirName, except it allows both '\' or '/'

 gString* pNewString;
 gString sName( (char*)strName );
 char chr;
 int idx, len;

 if ( strName==nil ) return nil;

 // Check if the last chrs are slashes
 sName.TrimRight();
 idx = len = (int)sName.Length();
 chr = sName[ len ];
 if ( chr=='\\' || chr=='/' ) return (char*)strName;

 pNewString = new gString;
 ASSERTION(pNewString,"dirname");

 for ( ; idx>=1; idx--) {
     chr = sName[ idx ];
     if ( chr=='\\' || chr=='/' ) {
	 pNewString->CopyFromTo( sName, 1, idx );
	 break;
     }
 }
 gStorageControl::Self().Pool().AppendObject( pNewString );
 return pNewString->Str();
}


int ima_file_extension (const char* strName, int dotFiles, gString& sExt)
{
 if ( dotFiles ) {
     gString sName( (char*)strName );
     gString sBase( sName.BaseName() ); // Basename...
     if ( sBase[ 1 ]=='.' ) {
	 sExt = sBase;
     }
     else {
	 dotFiles = -1;
     }
 }
 if ( dotFiles<=0 ) {
     sExt.Set( gio_file_extension( strName, 0 ) );
 }
 sExt.DownString();
 return sExt.IsEmpty();
}


gList* ima_ordered_dir (gDir& aDir)
{
 gList* entries( new gList );
 gList* ptrFolders( new gList );
 gList* ptrFiles( new gList );
 gElem* ptrElem( aDir.StartPtr() );
 gFileSysName* ptrSys;
 int code( -1 );
 bool isDir;

 ASSERTION(entries && ptrFolders && ptrFiles,"entries");

 for ( ; ptrElem; ptrElem=ptrElem->next) {
     ptrSys = (gFileSysName*)ptrElem->me;
     isDir = ptrSys->IsDirectory();

     // Skip soft or hard-links
#ifdef iDOS_SPEC
     ;
#else
     if ( isDir==false ) {
	 code = (ptrSys->GetStat().status.nLinks > 1);
	 //// TODO: code |= (ptrSys->GetStat().statusL.mode & S_IFLNK);
	 if ( code ) {
	     DBGPRINT("DBG: Skipped 0x%x, links: %d, %s\n",
		      code,
		      ptrSys->GetStat().status.nLinks,
		      ptrSys->Str());
	     continue;
	 }
     }
#endif
     if ( isDir ) {
	 code = ima_new_unique_name( ptrSys, 1, *ptrFolders );
     }
     else {
	 code = ima_new_unique_name( ptrSys, 2, *ptrFiles );
     }
     if ( code==-2 ) {
	 fprintf(stderr,"Internal error: %s\n",ptrSys->Str());
     }
 }

 entries->AppendObject( ptrFolders );
 entries->AppendObject( ptrFiles );

#ifdef DEBUG_MIN
 // Dump dirs and files, only for debug:
 for (ptrElem=ptrFolders->StartPtr(); ptrElem; ptrElem=ptrElem->next) {
     printf("DBG: D: stamp: %d, %s\n",
	    ptrElem->iValue,
	    ptrElem->Str());
 }
 for (ptrElem=ptrFiles->StartPtr(); ptrElem; ptrElem=ptrElem->next) {
     printf("DBG: F: stamp: %d, size: %lu\t%s\n",
	    ptrElem->iValue,
	    (unsigned long)ptrElem->me->iValue,
	    ptrElem->Str());
 }
#endif //DEBUG_MIN
 return entries;
}


gList* ima_open_dir (const char* strPath, gFileStat* ptrStat, sListingConf& aConf)
{
 int error;
 gList* entries;
 gDir aDir( (char*)strPath );

 error = aDir.lastOpError;
 if ( error ) {
     aConf.lastError = error;
     return nil;
 }

 entries = ima_ordered_dir( aDir );
 return entries;
}


gList* ima_basic_open (int fdIn, gFileStat* ptrStat, sListingConf& aConf)
{
 gList* mediaDesc( nil );
 const char* strName;
 gString sExt;
 gString* myStr;
 gString* mySim;
 gList* tagRefs;
 bool noExtension;
 t_uint8 supported( 0 );

 ASSERTION(fdIn!=-1,"fdIn!=-1");
 ASSERTION(ptrStat,"ptrStat");

 strName = ptrStat->name.Str();
 noExtension = ima_file_extension( strName, 1, sExt )!=0;
 if ( noExtension ) return nil;  // No extension returns nothing (?)

#if 0
 for (int idx=0; idx<256; idx++) {
     t_uint16 idxDef( aConf.supportedExtensions[ idx ].idxDef );
     const char* mimeType( aConf.defaultExtensionSupport[ idxDef ].mimeType );
     t_uint8 support( aConf.supportedExtensions[ idx ].supported );

     if ( aConf.supportedExtensions[ idx ].extensions ) {
	 printf("DBG: idx %3d: support=%u\t%d  [%s]  {%s}  ",
		idx,
		support,
		idxDef,
		mimeType,
		aConf.defaultExtensionSupport[ idxDef ].strExts);
	 aConf.supportedExtensions[ idx ].extensions->Show();
     }
 }
#endif

 aConf.SupportByExt( sExt.Str(), supported );
 DBGPRINT_MIN("DBG: strName={%s}, sExt={%s}, supported=%u\n",
	      strName,
	      sExt.Str(),
	      supported);

 mediaDesc = new gList;
 ASSERTION(mediaDesc,"mediaDesc");

 // Convention is:
 //	[1]  basename, schema (129, ...or other)
 //	[2]  basename without extension
 //	[3]  full filename, iValue=SIZE
 //	[4]  dirname
 //	[5]  extension string, iValue=support
 //	[6]  list of Tag-refs

 myStr = new gString( ima_basename( strName ) );
 ASSERTION(myStr,"myStr");
 myStr->iValue = IMA_SCHEMA;
 mediaDesc->AppendObject( myStr );

 mySim = new gString( myStr->BaseName( sExt.Str() ) );
 ASSERTION(mySim,"mySim");
 mediaDesc->AppendObject( mySim );

 mediaDesc->Add( (char*)strName );
 mediaDesc->EndPtr()->me->iValue = (int)ptrStat->status.Size();

 mediaDesc->Add( ima_dirname( strName ) );

 myStr = new gString( sExt );
 ASSERTION(myStr,"myStr");
 myStr->iValue = (int)supported;
 mediaDesc->AppendObject( myStr );

 tagRefs = new gList;
 ASSERTION(tagRefs,"tagRefs");
 mediaDesc->AppendObject( tagRefs );

 // Clean-up pool
 gStorageControl::Self().DeletePool();

 return mediaDesc;
}

////////////////////////////////////////////////////////////

