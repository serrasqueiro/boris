// gfilestat.cpp

#include <errno.h>
#include <string.h>  // memset,...
#include "ifilestat.h"




////////////////////////////////////////////////////////////
bool sFileStat::IsDirectory ()
{
 return S_ISDIR( mode );
}


bool sFileStat::IsLink ()
{
#ifdef linux
 return S_ISLNK( mode );
#else
 return false;
#endif //linux
}


long sFileStat::Size ()
{
 if ( IsDirectory() ) return 0L;
 return
     uid==0xFFFF ? -1L : (long)size;
}


t_uint32 sFileStat::USize ()
{
 long v = Size();
 return v<0 ? 0 : (t_uint32)v;
}


t_uint64 sFileStat::U64Size ()
{
 t_uint64 uSize( (t_uint64)size );
 if ( mSize ) {
     uSize += (mSize * 1024ULL * 1024ULL);
 }
 return uSize;
}


t_fs_perm sFileStat::Permission ()
{
 return mode & 0x1FF; // max mask ox777
}


t_fs_perm sFileStat::AllPermission ()
{
 // ox7777 (0xFFF) gives sticky-bit ('t', ox1000) and suid ('s', ox6000)
 return mode & 0xFFF;
}


bool sFileStat::SetSize (t_uint64 ullSize)
{
 if ( ullSize > (t_uint64)MAX_DUINT32 ) {
     mSize = ullSize / 1024ULL / 1024ULL;
     size = (t_uint32)(ullSize - mSize * 1024ULL * 1024ULL);
 }
 else {
     mSize = 0;
     size = (t_uint32)ullSize;
 }
 return true;
}


int sFileStat::MapFromUxPerms (gString& sPerm)
{
 int result( 0 );
 int ix( 2 ), ugoIdx( 1 ), len( (int)sPerm.Length() );
 t_uchar ux1( sPerm[ 1 ] ), uxLast;
 t_uint32 perm( 0 );
 t_uint32 sticky( 001000 );
 gString sUgo[ 4 ];

 mode = 0;
 nLinks = 0;

 switch ( ux1 ) {
 case '-':
     break;
 case 'h':
     nLinks = 1;
     break;
 case 'd':
 case 't':
     mode = S_IFDIR;
     break;
 default:
     break;
 }

 for ( ; ugoIdx<=3 && ix<len; ugoIdx++) {
     // User/Group/Others
     sUgo[ ugoIdx ].CopyFromTo( sPerm, ix, ix+2 );
     DBGPRINT("DBG: ugoIdx=%d, {%s}\n",ugoIdx,sUgo[ ugoIdx ].Str());
     ix += 3;
 }

 for (ugoIdx=3, ix=0; ugoIdx>=1; ugoIdx--) {
     // Inversed: i.e. others, then group, then user:
     if ( sUgo[ ugoIdx ][ 1 ]=='r' ) perm |= (004 << ix);
     if ( sUgo[ ugoIdx ][ 2 ]=='w' ) perm |= (002 << ix);
     switch ( uxLast = sUgo[ ugoIdx ][ 3 ] ) {
     case 's':
     case 't':
	 // Sticky:
	 perm |= sticky;
	 // No break here!
     case 'x':
	 perm |= (001 << ix);
	 break;
     case 'S':
	 // Sticky:
	 perm |= sticky;  // upper-'S' mean non-executable, whilst 's' mean sticky-executable.
	 break;
     default:
	 result++;
	 break;
     }
     sticky <<= 1;
     ix += 3;
 }
 mode |= perm;
 DBGPRINT("DBG: '%c,%s,%s,%s: (perm=ox%4o) ox%4o\n",
	  ux1,
	  sUgo[ 1 ].Str(),
	  sUgo[ 2 ].Str(),
	  sUgo[ 3 ].Str(),
	  perm, mode);
 return result;
}
////////////////////////////////////////////////////////////
// gFileStat - Generic file status
// ---------------------------------------------------------
gFileStat::gFileStat (const char* fName)
    : lastOpErrorL( 0 ),
      name( fName==NULL ? (char*)"\0" : fName ),
      hasStat( false ),
      fHandle( -1 )
{
 if ( name.IsEmpty()==false ) {
     thisStatName( fName, -1, status, statusL );
 }
}


gFileStat::gFileStat (gString& sName)
    : lastOpErrorL( 0 ),
      name( sName ),
      hasStat( false ),
      fHandle( -1 )
{
 if ( name.IsEmpty()==false ) {
     thisStatName( name.Str(), -1, status, statusL );
 }
}


gFileStat::~gFileStat ()
{
}


bool gFileStat::Update (int fd)
{
 fHandle = fd;
 if ( name.IsEmpty() ) {
     if ( fd==-1 ) return false;
     return thisFileStat(fd,status,statusL)==0;
 }
 return Update( name.Str() );
}


bool gFileStat::Update (const char* fName)
{
 ASSERTION(fName!=0 && fName[0]!=0,"gFileStat::Update");
 name.Set( fName );
 return thisStatName( fName, fHandle, status, statusL )==0;
}


gFileStat& gFileStat::CopyStat (gFileStat& copy)
{
 lastOpError = copy.lastOpError;
 lastOpErrorL = copy.lastOpErrorL;
 name.Copy( copy.name );
 status.Copy( copy.status );
 statusL.Copy( copy.statusL );
 hasStat = copy.hasStat;
 fHandle = -1;
 return *this;
}


int gFileStat::thisStatName (const char* fName,
			     int fd,
			     sFileStat& st,
			     sFileStat& stL)
{
 int res, resL;
 struct stat zsStat;
 struct stat zsStatL;

 lastOpError = lastOpErrorL = 0;
 st.ToDefault();
 stL.ToDefault();

 hasStat = false;
 if ( fd!=-1 ) return thisFileStat(fd,st,stL);

 // zsStat must be zeroed, because 'stat' will
 // not overwrite the contents for symbolic links!!!
 memset( &zsStat, 0x0, sizeof(zsStat) );
 // Optionally, zeros on statL as well.
 memset( &zsStatL, 0x0, sizeof(zsStatL) );

 ASSERTION(fName!=NULL && fName[0]!=0,"gFileStat::thisStatName");
 res = stat( fName, &zsStat );
 if ( res==-1 ) {
     lastOpError = errno;
 }
#ifdef linux
 resL = lstat( fName, &zsStatL );
#else
 resL = stat( fName, &zsStatL );
#endif //linux
 if ( resL==-1 ) {
     lastOpErrorL = errno;
     return res;
 }
 DBGPRINT_MIN("stat(%s)=%d:%d (err=%d:%d)\n",fName,res,resL,lastOpError,lastOpErrorL);

 st.inode = (t_inode)/*(ino_t)*/zsStat.st_ino;
 st.mode = zsStat.st_mode;
 st.uid = zsStat.st_uid;
 st.gid = zsStat.st_gid;
 if ( res==0 ) st.size = zsStat.st_size;
#ifdef linux
 st.blocks = (t_uint32)zsStat.st_blocks;  // unsigned long<
#else
 st.blocks = 1;
#endif //linux
 st.nLinks = zsStat.st_nlink;
 st.aTime = zsStat.st_atime;
 st.mTime = zsStat.st_mtime;
 st.cTime = zsStat.st_ctime;

 stL.inode = (t_uint64)zsStatL.st_ino;
 stL.mode = zsStatL.st_mode;
 stL.uid = zsStatL.st_uid;
 stL.gid = zsStatL.st_gid;
 stL.size = zsStatL.st_size;
#ifdef linux
 stL.blocks = zsStatL.st_blocks;
#else
 st.blocks = 1;
#endif //linux
 st.nLinks = zsStat.st_nlink;  //Irrelevant, I think...
 stL.aTime = zsStatL.st_atime;
 stL.mTime = zsStatL.st_mtime;
 stL.cTime = zsStatL.st_ctime;

 hasStat = true;
 return 0;
}


int gFileStat::thisFileStat (int fd,
			     sFileStat& st,
			     sFileStat& stL)
{
 int res;
 struct stat zsStat;

 st.ToDefault();
 stL.ToDefault();
 lastOpError = lastOpErrorL = 0;
 hasStat = false;

 if ( fd==-1 ) return 0;
 res = fstat(fd,&zsStat);
 if ( res==-1 ) {
     lastOpError = errno;
     return -1;
 }

 st.mode = zsStat.st_mode;
 st.uid = zsStat.st_uid;
 st.gid = zsStat.st_gid;
 st.size = zsStat.st_size;
 st.aTime = zsStat.st_atime;
 st.mTime = zsStat.st_mtime;
 st.cTime = zsStat.st_ctime;

 hasStat = true;
 return 0;
}

////////////////////////////////////////////////////////////
void ifs_os_status_to_gen (sFileStat& input, sGenericStatus& gen)
{
 gen.inode = input.inode;
 gen.mode = input.mode;
 gen.uid = input.uid;
 gen.gid = input.gid;
 gen.size = input.U64Size();
 gen.blocks = input.blocks;
 gen.nLinks = input.nLinks;
 gen.aTime = input.aTime;
 gen.mTime = input.mTime;
 gen.cTime = input.cTime;
}


void ifs_os_gen_to_status (sGenericStatus& gen, sFileStat& result)
{
 result.inode = gen.inode;
 result.mode = gen.mode;
 result.uid = gen.uid;
 result.gid = gen.gid;
 if ( gen.size > MAX_DUINT32 ) {
     result.size = gen.size % (1024ULL * 1024ULL);
     result.mSize = gen.size / 1024ULL / 1024ULL;
 }
 else {
     result.size = (t_uint32)gen.size;
 }
 result.blocks = gen.blocks;
 result.nLinks = gen.nLinks;
 result.aTime = gen.aTime;
 result.mTime = gen.mTime;
 result.cTime = gen.cTime;
}

////////////////////////////////////////////////////////////

