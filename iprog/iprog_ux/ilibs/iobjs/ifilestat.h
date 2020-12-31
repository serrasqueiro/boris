#ifndef iFILESTAT_X_H
#define iFILESTAT_X_H

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>       // For fstat, ...
#include <sys/types.h>      // For off_t, ...

#include "iobjs.h"
#include "icontrol.h"
#include "istring.h"

////////////////////////////////////////////////////////////
#ifndef S_IFMT
#define S_IFMT  00170000
#endif
#ifndef S_IFSOCK
#define S_IFSOCK 0140000
#endif
#ifndef S_IFLNK
#define S_IFLNK	 0120000
#endif
#ifndef S_IFREG
#define S_IFREG  0100000
#endif
#ifndef S_IFBLK
#define S_IFBLK  0060000
#endif
#ifndef S_IFDIR
#define S_IFDIR  0040000
#endif
#ifndef S_IFCHR
#define S_IFCHR  0020000
#endif
#ifndef S_IFIFO
#define S_IFIFO  0010000
#endif
#ifndef S_ISUID
#define S_ISUID  0004000
#endif
#ifndef S_ISGID
#define S_ISGID  0002000
#endif
#ifndef S_ISVTX
#define S_ISVTX  0001000
#endif

#define FILEM_IS_REGULAR(mode) (((mode)&S_IFIFO)==0)


////////////////////////////////////////////////////////////
typedef t_uint16 t_fs_perm;
typedef t_uint16 t_fs_dev;

////////////////////////////////////////////////////////////
struct sGenericStatus {
    t_fs_dev device;
    t_inode inode;
    t_uint32 mode;
    t_uint16 uid, gid;
    t_uint64 size;
    t_uint32 blocks;
    t_uint16 nLinks;
    time_t aTime, mTime, cTime;
    char* strName;  // allocated aside... if needed!
};


struct sFileStat {
    sFileStat ()
	: inode( 0 ),
	  mode( 0 ),
	  uid( 0xFFFF ),
	  gid( 0xFFFF ),
	  size( 0 ),
	  mSize( 0 ),
	  blocks( 0 ),
	  nLinks( 0 ),
	  aTime( 0 ),  // time of last access
	  mTime( 0 ),  // time of last modification
	  cTime( 0 ) { // time of last change, cTime change: e.g. owner change
    }

    t_inode inode;
    t_uint32 mode;
    t_uint16 uid, gid;
    t_uint32 size;
    t_uint32 mSize;  // size, in Mbytes; only if size>2^32
    t_uint32 blocks;
    t_uint16 nLinks;
    time_t aTime, mTime, cTime;

    void ToDefault () {
	inode = 0;
	mode = 0;
	size = blocks = 0;
	mSize = 0;
	nLinks = 0;
	uid = gid = 0xFFFF;
	aTime = mTime = cTime = 0;
    }

    bool IsValid () {
	return uid!=0xFFFF && gid!=0xFFFF;
    }

    bool IsDirectory () ;

    bool IsLink () ;

    t_fs_perm Permission () ;

    t_fs_perm AllPermission () ;

    long Size () ;

    t_uint32 USize () ;

    t_uint64 U64Size () ;

    // Set methods
    void Copy (sFileStat& copy) {
	inode = copy.inode;
	mode = copy.mode;
	uid = copy.uid;
	gid = copy.gid;
	size = copy.size;
	mSize = copy.mSize;
	blocks = copy.blocks;
	nLinks = copy.nLinks;
	aTime = copy.aTime;
	mTime = copy.mTime;
	cTime = copy.cTime;
    }

    bool SetSize (t_uint64 ullSize) ;

    int MapFromUxPerms (gString& sPerm) ;
};

////////////////////////////////////////////////////////////
class gFileStat : public gControl {
public:
    gFileStat (const char* fName=NULL) ;

    gFileStat (gString& sName) ;

    gFileStat (int fd) ;

    virtual ~gFileStat () ;

    // Public data-members
    int lastOpErrorL;
    gString name;
    sFileStat status, statusL;

    // Get methods
    virtual bool IsOk () {
	return HasStat()==true && Error()==0;
    }

    virtual bool HasStat () {
	return hasStat==true;
    }

    virtual int Error () {
	return lastOpError==0 ? lastOpErrorL : lastOpError;
    }

    bool IsDirectory () {
	return statusL.IsDirectory();
    }

    bool IsLink () {
	return statusL.IsLink();
    }

    // Set methods
    bool Update (int fd=-1) ;

    bool Update (const char* fName) ;

    gFileStat& CopyStat (gFileStat& copy) ;

protected:
    // Protected methods
    int thisStatName (const char* fName,
		      int fd,
		      sFileStat& st,
		      sFileStat& stL) ;

    int thisFileStat (int fd,
		      sFileStat& st,
		      sFileStat& stL) ;

private:
    bool hasStat;
    int fHandle;

    // Operators,empty
    gFileStat (gFileStat& ) ; //empty
    gFileStat& operator= (gFileStat& ) ; //empty
};
////////////////////////////////////////////////////////////

extern void ifs_os_status_to_gen (sFileStat& input, sGenericStatus& gen) ;

extern void ifs_os_gen_to_status (sGenericStatus& gen, sFileStat& result) ;

////////////////////////////////////////////////////////////
#endif //iFILESTAT_X_H

