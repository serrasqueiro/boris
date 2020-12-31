#ifndef gIO_AUX_X_H
#define gIO_AUX_X_H

#include "ifilestat.h"

////////////////////////////////////////////////////////////

extern int gns_open_readonly (const char* strFile) ;
extern off_t gns_lseek (int fildes, off_t offset, int whence) ;

extern int gio_strcmp (const char* str, const char* sub)  ;
extern int gio_findstr (const char* str, const char* sub)  ;

extern char* gio_file_extension (const char* str, int mask)  ;
extern int gio_filepath (const char* strDir, char* strFile, gString& sFilepath)  ;

extern long gio_compare_tilldiff (char* strFile1, char* strFile2)  ;
extern long gio_compare_filesindir (char* strDir, char* strFile1, char* strFile2)  ;

extern char* gio_getcwd (char* buf, size_t size)  ;
extern int gio_chdir (const char* strPath)  ;
extern char* gio_dirname (char* strFile)  ;  // rewrites name if needed

extern char* gio_current_dir (void) ;
extern char* gio_dirpath_join (gString& sDir, gString& sName, gString& sResult) ;
extern int gio_dir_trim (gString& sPath) ;
extern int gio_dir_status (const char* strPath, int& error) ;

extern int gio_mkdir (const char* strPath, mode_t mode, int mask) ;

extern off_t gio_file_seek (int fildes, off_t offset, int whence)  ;
extern int gio_stable_file (int fd, FILE* fRepErr, int doProbe, off_t& seeked)  ;

extern t_stamp gio_current_time (void)  ;

extern int gio_file_touch (const char* strFilename, t_stamp aStamp, t_stamp mStamp)  ;
extern int gio_file_touch_io (const char* strFilename, t_stamp aStamp, t_stamp mStamp, int& errorCode)  ;
extern int gio_file_touch_check (const char* strFilename, t_stamp aStamp, t_stamp mStamp, int& errorCode)  ;
extern int gio_file_stamps (const char* strFilename, t_stamp& aStamp, t_stamp& mStamp)  ;

////////////////////////////////////////////////////////////

#endif //gIO_AUX_X_H

