// gdir.cpp

#include <string.h>
#include <errno.h>

#include "idir.h"
#include "ifilestat.h"
////////////////////////////////////////////////////////////
// Static members
// ---------------------------------------------------------
t_int8 gFileSysName::validChars256[ 258 ];

t_uchar gDirGeneric::slashChr=gSLASHCHR;

////////////////////////////////////////////////////////////
gFileSysName::gFileSysName (const char* s,
			    eFileSystemName aFSysName,
			    bool isADirectory)
    : gString( s ),
      fsysName( aFSysName ),
      isDirectory( isADirectory ),
      myStat( nil )
{
}


gFileSysName::gFileSysName (const char* s,
			    eFileSystemName aFSysName,
			    gFileStat& aStat)
    : gString( s ),
      fsysName( aFSysName ),
      isDirectory( aFSysName==e_FSysDir )
{
 myStat = new gFileStat;
 ASSERTION(myStat!=nil,"myStat!=nil");
 myStat->CopyStat( aStat );
}


gFileSysName::~gFileSysName ()
{
 delete myStat;
 myStat = nil;
}


bool gFileSysName::IsStrOk ()
{
 bool isStrict = isDirectory==false;
 unsigned i, n=Length();

 if ( IsOk()==false ) return false;
 for (i=0; i<n; i++) {
     if ( IsValidCharInName( str[i], isStrict )==false )
	 return false;
 }
 return true;
}


char* gFileSysName::Str (unsigned idx)
{
 ASSERTION(str!=nil,"str!=nil");
 return gString::Str( idx );
}


gFileStat& gFileSysName::GetStat ()
{
 ASSERTION(myStat!=nil,"myStat!=nil");
 return *myStat;
}


bool gFileSysName::IsValidChar (t_uchar uChr, int strictLevel)
{
 t_int8 validity( validChars256[ uChr ] );
 return validity>=1;
}


bool gFileSysName::IsValidCharInName (t_uchar uChr, bool isStrict)
{
 t_int8 validity( validChars256[ 0 ] );

 if ( validity==0 ) {
	InitializeValidChars256( validChars256, sizeof(validChars256), 0 );
 }
 ASSERTION(validChars256[0]==-1,"validChars256[0]==-1");
 validity = validChars256[ uChr ];
 return validity>=0;
}


bool gFileSysName::InitializeValidChars256 (t_int8* vChars, t_uint16 size, int mask)
{
 int iter;
 t_uchar uChr( 0xA8 );
 const char* basicValidChrs="\
abcdefghijklmnopqrstuvwxyz\
ABCDEFGHIJKLMNOPQRSTUVWXYZ\
0123456789\
_-\
 !@#$%&()={}'`´\
~^.:,;\
";

 // Note: mask is ignored.
 memset( vChars, -1, size );
 if ( size<=0 ) return false;

 for ( ; uChr>0; uChr++) {
	// Allow ISO-8859-1 chars
	vChars[ uChr ] = 1;
 }

 for (iter=0; (uChr = basicValidChrs[ iter ])!=0; iter++) {
     unsigned pos( FindChrInStr( basicValidChrs, uChr ) );
     if ( pos ) vChars[ uChr ] = 1 + (pos < 26+26+10);
 }
 return true;
}
////////////////////////////////////////////////////////////
gFileName::gFileName ()
    : gFileSysName( "\0", e_FSysNoName, false )
{
}


gFileName::gFileName (const char* s)
    : gFileSysName( s, e_FSysFile, false )
{
}


gFileName::gFileName (gString& s)
    : gFileSysName( s.Str(), e_FSysFile, false )
{
}


gFileName::gFileName (gString& s, gFileStat& aStat)
    : gFileSysName( s.Str(), e_FSysFile, aStat )
{
}


gFileName::~gFileName ()
{
}
////////////////////////////////////////////////////////////
gDirName::gDirName ()
    : gFileSysName( "\0", e_FSysNoName, true )
{
}


gDirName::gDirName (const char* s)
    : gFileSysName( "\0", e_FSysDir, true )
{
 SetDirName( s );
}


gDirName::gDirName (gString& s)
    : gFileSysName( "\0", e_FSysDir, true )
{
 SetDirName( s.Str() );
}


gDirName::gDirName (gString& s, gFileStat& aStat)
    : gFileSysName( s.Str(), e_FSysDir, aStat )
{
 SetDirName( s.Str() );
}


gDirName::~gDirName ()
{
}


char* gDirName::Str (unsigned idx)
{
 return dirStr.Str();
}


char* gDirName::Name ()
{
 ASSERTION(str!=nil,"str!=nil");
 return (char*)str;
}

void gDirName::SetDirName (const char* s)
{
 ASSERTION(s!=nil,"s!=nil");
 thisBuildDirStr( s );
}


t_uchar* gDirName::ToString (const t_uchar* uBuf)
{
 return dirStr.ToString( uBuf );
}


void gDirName::thisBuildDirStr (const char* s)
{
 gString sTemp( s );
 char* sPtr( sTemp.Str() );
 int pIdx, len = (int)sTemp.Length();

 for (pIdx=len; pIdx>0; ) {
     pIdx--;
     if ( sPtr[pIdx]==gSLASHCHR )
	 sPtr[pIdx] = 0;
     else
	 break;
 }
 Set( sPtr );
 dirStr.Set( sPtr );
 dirStr.Add( gSLASHCHR );
}

////////////////////////////////////////////////////////////
gDirStream::gDirStream (const char* dName)
    : gFile( gFile::e_Binary, dName, true ),
      pdir( nil ),
      dirName( dName ),
      dStat( dName ),
      isDirOpened( false )
{
#ifdef iDOS_SPEC
 lastOpError = 0;
#endif //iDOS_SPEC
 if ( lastOpError==0 ) doOpenDir( dName );
}


gDirStream::~gDirStream ()
{
 doCloseDir();
}


bool gDirStream::IsOpened ()
{
 return isDirOpened;
}


int gDirStream::doOpenDir (const char* dName)
{
 isDirOpened = false;
 lastOpError = dName==nil || dName[ 0 ]==0;
 if ( lastOpError )
     return 1;
 pdir = opendir( dName );
 isDirOpened = pdir!=NULL;
 DBGPRINT_MIN("doOpenDir(%s): isDirOpened?%d\n",dName,isDirOpened);
 if ( isDirOpened==false ) return lastOpError = errno;
 return 0;
}


int gDirStream::doCloseDir ()
{
 if ( isDirOpened ) {
     ASSERTION(pdir!=nil,"pdir!=nil");
     closedir( pdir );
     pdir = nil;
 }
 isDirOpened = false;
 return 0;
}
////////////////////////////////////////////////////////////
// gDirGeneric - Generic directory handling
// ---------------------------------------------------------
gDirGeneric::gDirGeneric (eStorage kind)
    : lastOpError( 0 ),
      slashStr( (char)slashChr )
{
}


gDirGeneric::~gDirGeneric ()
{
}


bool gDirGeneric::IsNameOk (const char* s)
{
 unsigned posBad;
 return thisNameOk( s, posBad );
}


bool gDirGeneric::AllNamesOk (int depthLevel)
{
 unsigned posBad;
 int notOkDepth;
 if ( depthLevel<0 ) return true;
 return thisAllNamesOk( depthLevel, posBad, notOkDepth );
}


gFileSysName* gDirGeneric::GetName (unsigned idx)
{
 gFileSysName* fSysPtr = (gFileSysName*)GetObjectPtr( idx );
 ASSERTION(fSysPtr!=nil,"fSysPtr!=nil");
 return fSysPtr;
}


t_uchar* gDirGeneric::ToString (const t_uchar* uBuf)
{
 return nil;
}


bool gDirGeneric::SaveGuts (FILE* f)
{
 bool isOk=true;
 unsigned i, n=N();
 char* s;

 if ( CanSave( f )==false ) return false;

 // Only stores one depth level
 isOk = AllNamesOk( 0 );
 if ( isOk==false ) return false;
 for (i=1; i<=n; i++) {
     gStorage* aObj = GetObjectPtr( i );
     s = ((gString*)aObj)->Str();
     isOk = fprintf( f, "%s\n", s )==1;
 }
 return isOk;
}


bool gDirGeneric::RestoreGuts (FILE* f)
{
 gUCharBuffer sTemp;
 char* s;

 if ( CanRestore( f )==false ) return false;

 while ( fgets( (char*)sTemp.uBuf, sTemp.size, f )!=nil ) {
     s = sTemp.Str();
     if ( IsNameOk( s )==false ) return false;
     Add( s );
 }
 return true;
}


bool gDirGeneric::thisNameOk (const char* s, unsigned& posBad)
{
 unsigned pos=0, countSlash=0;
 char c='\0';
 t_uchar chr=0;

 ASSERTION(s!=nil,"s!=nil");
 posBad = 0;
 while ( (c = s[pos++])!=0 ) {
     chr = (t_uchar)c;
     posBad = pos;
     if ( chr<' ' ) return false;
     countSlash += chr==slashChr;
 }
 if ( chr==0 ) return false;
 posBad = 1;
 if ( strcmp(s,".")==0 || strcmp(s,"..")==0 ) return false;
 // If it is a file-name, no slash,
 // otherwise must be a directory (e.g. 'etc/') with slash at end.
 if ( countSlash>1 ) return false;
 if ( countSlash>0 && chr!=slashChr ) return false;
 if ( slashStr.Match( s ) ) return false;
 posBad = 0;
 return true;
}


bool gDirGeneric::thisAllNamesOk (unsigned depthLevel,
				  unsigned& posBad,
				  int& notOkDepth)
{
 bool isOk;
 char* s;

 posBad = 0;
 notOkDepth = 0;
 ASSERTION(depthLevel>=0,"depthLevel>=0");
 ASSERTION(depthLevel==0,"TODO: depthLevel not 0...");
 unsigned i, n=N();

 for (i=1; i<=n; i++) {
     gStorage* aObj = GetObjectPtr( i );
     s = ((gString*)aObj)->Str();
     isOk = IsNameOk( s );
     if ( isOk==false ) {
	 posBad = i;
	 return false;
     }
 }
 return true;
}
////////////////////////////////////////////////////////////
gDir::gDir (const char* dName)
    : gDirGeneric( e_StoreExtend ),
      dirStream( dName ),
      dirCount( -1 )
{
 lastOpError = dirStream.lastOpError;

 if ( dName!=nil ) {
     sDirName.SetDirName( dName );
 }
 if ( dirStream.IsOpened() ) {
     thisDelete();
     thisScanDir();
 }
}


gDir::gDir (gString& dName)
    : gDirGeneric( e_StoreExtend ),
      dirStream( dName.Str() ),
      dirCount( -1 )
{
 lastOpError = dirStream.lastOpError;
 if ( dName.IsEmpty()==false ) sDirName.SetDirName( dName.Str() );
 if ( dirStream.IsOpened() ) {
     thisDelete();
     thisScanDir();
 }
}


gDir::gDir (const char* dName, bool doReadDir)
    : gDirGeneric( e_StoreExtend ),
      dirStream( dName ),
      dirCount( -1 )
{
 lastOpError = dirStream.lastOpError;
 ASSERTION(dName,"dName");
 sDirName.SetDirName( dName );
 if ( doReadDir && dirStream.IsOpened() ) {
     thisDelete();
     thisScanDir();
 }
}


gDir::~gDir ()
{
}


gFileStat* gDir::GetStat (unsigned idx)
{
 gFileSysName* fSysPtr = GetName( idx );
 // fSysPtr is always non-nil
 gFileStat* statPtr = fSysPtr->GetStatPtr();
 ASSERTION(statPtr!=nil,"statPtr!=nil");
 return statPtr;
}


bool gDir::GetNameDir (unsigned idx, gString& resName)
{
 gString resFullName;
 return GetFullNameDir( idx, resName, resFullName );
}


bool gDir::GetFullNameDir (unsigned idx, gString& resName, gString& resFullName)
{
 gFileSysName* fSysPtr = GetName( idx );
 resName.Set( fSysPtr->Str() );
 resFullName = sDirName.Str();
 resFullName.AddString( resName );
 return fSysPtr->GetStatPtr()->IsDirectory();
}


void gDir::AddDir (const char* s)
{
 ASSERTION(s!=nil && s[0]!=0,"AddDir(1)");
 gString sTemp( s );
 gDirName* sName = new gDirName( sTemp );
 thisAddSystemName( sName );
}


void gDir::AddFile (const char* s)
{
 ASSERTION(s!=nil && s[0]!=0,"AddFile(1)");
 gString sTemp( s );
 gFileName* sName = new gFileName( sTemp );
 thisAddSystemName( sName );
}


int gDir::ScanDir ()
{
 bool isOk, isDir;
 struct dirent tDirEnt;
 struct dirent* dp;
 char* s;
 int counter=0;
 DIR* apDir;

 apDir = dirStream.pdir;
 if ( dirStream.dirName.IsEmpty() ) return -1;
 if ( apDir==nil ) return -1;

 // SYS-CALL: int getdents(unsigned int fd, struct dirent *dirp, unsigned int count)
 // Used: ==> struct dirent *readdir(DIR *dir)

 for (lastOpError=0; ; counter++) {
     gFileStat aStat;
     dp = readdir( apDir );
     if ( counter==0 && dp==NULL ) lastOpError = errno;
     if ( dp==NULL ) break;
     memset( &tDirEnt, 0, sizeof(tDirEnt) );
     memcpy( &tDirEnt, dp, sizeof(tDirEnt) );
     // d_reclen is the size, d_name the name itself
     s = (char*)tDirEnt.d_name;
     if ( strcmp(s,".")==0 || strcmp(s,"..")==0 ) continue;

     gString sTemp( sDirName.Name() );
     sTemp.Add( gSLASHCHR );
     sTemp.Add( s );
     aStat.Update( sTemp.Str() );
     isOk = aStat.HasStat();
     DBGPRINT_MIN("DBG: gdir: '%s', isOk?%c: %s\n",
		  s,
		  ISyORn( isOk ),
		  sTemp.Str());
     if ( isOk==false ) {
	 errUnstatL.Add( sTemp );
	 continue;
     }
     isDir = aStat.statusL.IsDirectory();
     AddStatName( isDir?e_FSysDir:e_FSysFile, s, aStat );
 }
 DBGPRINT_MIN("dirStream(%s):%d\n",dirStream.Name(),lastOpError);
 return lastOpError;
}


int gDir::AddStatName (eFileSystemName aFSysName, const char* s, gFileStat& aStat)
{
 gFileSysName* aPtr=nil;

 ASSERTION(s!=nil && s[0]!=0,"AddDir(1)");
 gString sTemp( s );

 switch ( aFSysName ) {
 case e_FSysFile:
     aPtr = new gFileName( sTemp, aStat );
     break;
 case e_FSysDir:
     aPtr = new gDirName( sTemp, aStat );
 case e_FSysNoName:
 default:
     break;
 }
 return thisAddSystemName( aPtr );
}


int gDir::thisAddSystemName (gFileSysName* aPtr)
{
 if ( aPtr==nil ) return -1;
 return thisAppend( aPtr )==false;
}
////////////////////////////////////////////////////////////

