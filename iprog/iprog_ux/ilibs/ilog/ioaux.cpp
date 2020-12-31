// ioaux.cpp

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <errno.h>

#include <string.h>

#include <time.h>
#include <utime.h>  // utimbuf

#include "ioaux.h"

#include "ifile.h"

////////////////////////////////////////////////////////////
off_t gns_lseek (int fildes, off_t offset, int whence)
{
 ASSERTION(fildes!=-1,"fildes!=-1");
 return lseek( fildes, offset, whence );
}


int gns_open_readonly (const char* strFile)
{
 int result;
 if ( strFile==NULL || strFile[0]==0 ) return -1;
#ifdef iDOS_SPEC
 result = open( strFile, O_RDONLY | O_BINARY );
#else
 result = open( strFile, O_RDONLY );
#endif
 return result;
}


int gns_errno ()
{
 return errno;
}

////////////////////////////////////////////////////////////
// gio functions
////////////////////////////////////////////////////////////
int gio_strcmp (const char* str, const char* sub)
{
 if ( str==nil || sub==nil ) return -2;
 return strcmp( str, sub );
}


int gio_findstr (const char* str, const char* sub)
{
 static char* ptr;
 if ( str==nil || sub==nil ) return -1;
 ptr = strstr( (char*)str, (char*)sub );
 if ( ptr )
     return ptr-str;
 return -1;
}


int gio_findchr_back (const char* str, char toFind)
{
 if ( str==nil ) return -1;

 for (int iter=strlen( str ); iter>0; ) {
     iter--;
     if ( str[ iter ]==toFind ) return iter;
 }
 return -1;
}


char* gio_file_extension (const char* str, int mask)
{
 static char bufEmpty[ 8 ];
 int findDot( gio_findchr_back( str, '.' ) );
 if ( findDot<0 ) return bufEmpty;
 if ( mask ) {
     if ( findDot==0 )
	 return bufEmpty;
 }
 return (char*)str+findDot;
}


int gio_filepath (const char* strDir, char* strFile, gString& sFilepath)
{
 sFilepath.Set( (char*)strDir );
 sFilepath.Add( gSLASHCHR );
 sFilepath.Add( strFile );
 return (int)sFilepath.Length();
}


long gio_compare_tilldiff (char* strFile1, char* strFile2)
{
 int fd1, fd2;
 int mismatch( 0 );
 char buf1[ 1024 ];
 char buf2[ 1024 ];
 off_t size1;
 off_t size2;
 ssize_t didRead1( 0 );
 ssize_t didRead2( 0 );

 DBGPRINT("DBG: tilldiff {%s} vs {%s}\n",
	  strFile1,
	  strFile2);
 if ( strFile1==nil || strFile2==nil ) return -1L;

 fd1 = gns_open_readonly( strFile1 );
 if ( fd1==-1 ) return -2L;
 fd2 = gns_open_readonly( strFile2 );
 if ( fd2==-1 ) {
     close( fd1 );
     return -3L;
 }

 size1 = lseek( fd1, 0, SEEK_END );
 size2 = lseek( fd2, 0, SEEK_END );
 if ( size1 != size2 ) {
     close( fd1 );
     close( fd2 );
     return -4L;
 }

 size1 = lseek( fd1, 0, SEEK_SET );
 size2 = lseek( fd2, 0, SEEK_SET );

 if ( size1==0 && size2==0 ) {
     // Rewind should work, and its result is 0.
     do {
	 didRead1 = read( fd1, buf1, sizeof(buf1) );
	 didRead2 = read( fd2, buf2, sizeof(buf2) );
	 if ( didRead1<=0 || didRead2<=0 ) break;
	 if ( didRead1!=didRead2 ) break;
	 size1 += (long)didRead1;
	 mismatch = memcmp( buf1, buf2, didRead1 );
     } while ( mismatch==0 );
 }

 close( fd1 );
 close( fd2 );
 if ( mismatch ) return size1;
 if ( didRead1!=didRead2 ) return -4L;  // Sizes differ after all?!
 return 0L;
}


long gio_compare_filesindir (char* strDir, char* strFile1, char* strFile2)
{
 gString sName1;
 gString sName2;
 gio_filepath( strDir, strFile1, sName1 );
 gio_filepath( strDir, strFile2, sName2 );
 return gio_compare_tilldiff( sName1.Str(), sName2.Str() );
}

////////////////////////////////////////////////////////////
char* gio_getcwd (char* buf, size_t size)
{
 char* str;
 ASSERTION(buf,"gio_getcwd (1)");
 ASSERTION(size,"gio_getcwd (2)");
 str = getcwd( buf, size );
 gStorageControl::Self().lastOpError = gns_errno();
 return str;
}

int gio_chdir (const char* strPath)
{
 int error( strPath==nil ), ioError( 0 );
 gStorageControl::Self().lastOpError = 0;
 if ( error ) return -1;
 if ( strPath[0]==0 ) return 0;
 error = chdir( strPath );
 if ( error ) {
     ioError = gns_errno();
 }
 gStorageControl::Self().lastOpError = ioError;
 return ioError;
}

char* gio_dirname (char* strFile)
{
 if ( strFile==nil ) return nil;

 gString s( strFile );
 int pos( (int)s.FindBack( gSLASHCHR ) );
 if ( s.IsEmpty() ) return strFile;

 strFile[ pos-- ] = 0;
 for ( ; pos>=0; pos--) {
     if ( strFile[ pos ]!=gSLASHCHR ) break;
     strFile[ pos ] = 0;
 }
 return strFile;
}

char* gio_current_dir (void)
{
 static char dirBuf[ 4096 ];
 gio_getcwd( dirBuf, sizeof(dirBuf) );
 return dirBuf;
}


char* gio_dirpath_join (gString& sDir, gString& sName, gString& sResult)
{
 sResult = sDir;
 if ( sResult.IsEmpty()==false )
     sResult.Add( gSLASHCHR );
 sResult.AddString( sName );
 return sResult.Str();
}


int gio_dir_trim (gString& sPath)
{
 // Trims dir name as necessary
 int count( 0 );
 unsigned lastLen( 0 ), len;

 // E.g. abc/./././ or abc/. or abc/ becomes just "abc" !

 for ( ; lastLen!=(len = sPath.Length()); ) {
     lastLen = len;
     if ( sPath[ len ]=='.' ) {
	 if ( len>1 ) {
	     if ( sPath[ len-1 ]==gSLASHCHR ) {
		 len--;
		 sPath.Delete( len );
		 len--;
		 count++;
	     }
	 }
	 else {
	     sPath.Delete();
	     count++;
	 }
     }
     if ( sPath[ len ]==gSLASHCHR ) {
	 sPath.Delete( len );
	 count++;
     }
 }
 return count;
}


int gio_dir_status (const char* strPath, int& error)
{
 error = -1;
 if ( strPath ) {
     if ( strPath[ 0 ] ) {
	 gFileStat dirStatus( (char*)strPath );
	 error = dirStatus.lastOpError;
	 if ( error ) return error;
	 return dirStatus.status.IsDirectory()==false;
     }
     return 0;  // Assume empty is ok
 }
 return -1;
}


int gio_mkdir (const char* strPath, mode_t mode, int mask)
{
 int error;
 gString sPath( (char*)strPath );

 gio_dir_trim( sPath );
 if ( sPath.IsEmpty() ) return -1;

 if ( mode==0 ) {
     if ( mask ) {
	 mode = mode & (mode_t)~mask;
     }
     else {
	 mode = 0750;
     }
 }

#ifdef iDOS_SPEC
 error = mkdir( sPath.Str() )!=0;
#else
 error = mkdir( sPath.Str(), mode )!=0;
#endif
 if ( error )
     return errno;
 return 0;
}

////////////////////////////////////////////////////////////
off_t gio_file_seek (int fildes, off_t offset, int whence)
{
 if ( fildes<0 ) return -1;
 off_t seeked( gns_lseek( fildes, offset, whence ) );
#ifdef DEBUG_MIN
 if ( whence==SEEK_SET ) return seeked;
 printf("DBG: enter seek value%s: %d: ",whence==SEEK_END?" (SEEK_END)":"\0",seeked);
 seeked=-2;
 while ( fscanf( stdin, "%ld", &seeked )!=1 && seeked<-1 ) ;
#endif //DEBUG_MIN
 return seeked;
}


int gio_stable_file (int fd, FILE* fRepErr, int doProbe, off_t& seeked)
{
 int retry=1, maxRetries=10;
 off_t seekedCheck;
 ssize_t did_read;
 char blindBuf[ 128 ];
 unsigned secSleep;

 ASSERTION(fd>=0,"fd>=0");
 seeked = gio_file_seek( fd, 0, SEEK_END );
 if ( seeked<0 ) return 1;

 if ( gio_file_seek( fd, 0, SEEK_SET )<0 ) return 1;

 for (secSleep=5;
      retry<=maxRetries && (seekedCheck = gio_file_seek( fd, 0L, SEEK_END ))!=seeked;
      retry++, secSleep+=10) {
     if ( gio_file_seek( fd, seeked, SEEK_SET )!=seeked ) {
	 if ( fRepErr )
	     fprintf(fRepErr,"File shrunk (below %lu bytes)?\n",seeked);
	 return 4;  // Severe error, flie shrunk?
     }
     if ( doProbe==1 ) {
	 if ( fRepErr )
	     fprintf(fRepErr,"File size differs, retry (%d/%d), sleeping %u seconds\n",
		     retry,
		     maxRetries,
		     secSleep);
	 gFileControl::Self().SecSleep( (t_uint32)secSleep );
     }
 }

 did_read = read( fd, blindBuf, sizeof(blindBuf) );
 if ( did_read>0 ) return 3;  // Unable to know end-point of file

 // Rewind!
 if ( gio_file_seek( fd, 0, SEEK_SET )<0 ) return 1;

 return 0;
}

////////////////////////////////////////////////////////////
t_stamp gio_current_time (void)
{
 time_t epoch( time(NULL) );
 return (t_stamp)epoch;
}


int gio_file_touch (const char* strFilename, t_stamp aStamp, t_stamp mStamp)
{
 int errorCode;
 return gio_file_touch_io( strFilename, aStamp, mStamp, errorCode );
}


int gio_file_touch_io (const char* strFilename, t_stamp aStamp, t_stamp mStamp, int& errorCode)
{
 int error;
 struct utimbuf aTimBuf;

 if ( strFilename==nil || strFilename[0]==0 ) return -1;

 aTimBuf.actime = aStamp;
 aTimBuf.modtime = mStamp;

 error = utime( strFilename, &aTimBuf );
 if ( error ) {
     errorCode = gns_errno();
 }
 else {
     errorCode = 0;
 }
 return error;
}


int gio_file_touch_check (const char* strFilename, t_stamp aStamp, t_stamp mStamp, int& errorCode)
{
 t_stamp checkAstamp( 0 );
 t_stamp checkMstamp( 0 );
 int error( gio_file_touch_io( strFilename, aStamp, mStamp, errorCode ) );
 int checker;
 if ( error || errorCode ) return error;
 checker = gio_file_stamps( strFilename, checkAstamp, checkMstamp );
 if ( checker ) return -2;
 checker |= (checkAstamp!=aStamp)*32;
 checker |= (checkMstamp!=mStamp)*64;
 return checker;
}


int gio_file_stamps (const char* strFilename, t_stamp& aStamp, t_stamp& mStamp)
{
 if ( strFilename==nil || strFilename[0]==0 ) return -1;

 gFileStat fstat( (char*)strFilename );
 aStamp = mStamp = 0;

 if ( fstat.IsOk() ) {
     aStamp = fstat.status.aTime;
     mStamp = fstat.status.mTime;
     return 0;
 }
 return 2;
}

////////////////////////////////////////////////////////////
/*
int gio_similar_paths (const char* path1, const char* path2, const char* strBasePath)
{
#warning TODO
 // see also acopy.cpp: simplify_path()
 return 0;
}
*/

////////////////////////////////////////////////////////////

