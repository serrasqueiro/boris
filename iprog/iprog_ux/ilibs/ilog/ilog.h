#ifndef gILOG_X_H
#define gILOG_X_H

#include "ifile.h"
#include "iodeconv.h"


#ifdef linux
#include <unistd.h>
#define GX_GETPID() getpid()
#define GX_GETPPID() getppid()
#else
#define GX_GETPID() 0
#define GX_GETPPID() 0
#endif //linux


#define ILG_LOG_MAX_SIZE_1M (1024L * 1024L)
#define ILG_LOG_MAX_SIZE ILG_LOG_MAX_SIZE_1M

#ifdef iDOS_SPEC
#define ILG_ROTATED_COMPRESS_CMD "zip -m "
#else
#define ILG_ROTATED_COMPRESS_CMD "gzip -9 "
#endif


#define ILF_APPEND_MASK		1	// Creates file if does not exist already

#define ILF_STM_DATETIME(p)	p->tm_year+1900, p->tm_mon+1, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec

typedef char t_msg_kind[ 16 ];

////////////////////////////////////////////////////////////
#ifdef DEBUG_LLOG
#define DBGPRINT_LLOG(args...) printf(args)
#else
#define DBGPRINT_LLOG(args...) ;
#endif //~DBGPRINT_LLOG

#define DBG_IDX_MIN (LOG_LOGMAX+1)

////////////////////////////////////////////////////////////
#define S_LOG(aLog,logFile,level,args...) { \
		aLog.Log( nil, level, args ); \
		; \
	}

#define S_LOG_LEVEL(logFile,level,args...) S_LOG(lGlobLog,logFile,level,args)

#define S_LOG_DEF(logFile,args...) S_LOG_LEVEL(logFile,LOG_INFO,args)


#ifdef DEBUG_LOG_KEY
#define DBG_PRESS_KEY(doit,key,msg) { \
		char zBuf[10]; \
		zBuf[ 0 ] = 0; \
		while ( doit!=0 && zBuf[0]!=key ) { \
			printf("[press %c:%s]\n",key,msg); \
			fgets(zBuf,9,stdin); \
		} \
	}
#else
#define DBG_PRESS_KEY(doit,key,msg) DBGPRINT_LLOG("DBG_LLOG: %s\n",msg)
#endif //~DEBUG_...

////////////////////////////////////////////////////////////
class gCGenLog : public gControl {
public:
    virtual ~gCGenLog () ;

    // Public data-members
    bool doShowKindOnLog;
    bool rotateWithCompression;  // not implemented here!
    int identSeed;
    int rotatedFiles;
    long logMaxSize;

    gString sPidFile;
    gString sRotateCompressCmd;

    static const char* base62Chars;

    static t_msg_kind msgKinds[ 10 ];

    // Get methods
    virtual bool IsOk () {
	return
	    gControl::IsOk() && fLog!=nil;
    }

    virtual char* Name () {
	return fLog==nil ? nil : sName.Str();
    }

    virtual gString& FileName () {
	return sName;
    }

    virtual bool EmptyStream () {
	return fLog==nil;
    }

    virtual FILE* Stream () {
	return fLog;
    }

    virtual FILE* ValidStream () ;

    virtual char* LogLevelDescription (int level) ;

    virtual unsigned NofMsgs () {
	return messages.N();
    }

    virtual gList& Messages () {
	return messages;
    }

    virtual char* SchemeStr () {
	return scheme8;
    }

    // Set methods
    virtual void Reset () ;
    virtual void ResetLog () ;

    virtual bool Close () ;
    virtual bool Reopen () ;

    virtual bool SetName (gString& sFileName) ;

    virtual bool SetScheme (t_uint16 major, t_uint16 minor, t_uint16 sub=0, const char* strPrefix=nil) ;

    virtual bool SetRotation (int nrRotatedFiles=10, long maxLogSize=ILG_LOG_MAX_SIZE, bool doCompression=false) ;

    virtual bool RemoveSimilarLogs (const char* strFile, gList* optExtensions=nil) ;

    virtual bool SetAltOuput (FILE* fOut) {
	fAltOutput = fOut;
	return fAltOutput!=nil;
    }

    // Special methods
    virtual char* UniqueIdent (int width) ;

    virtual int RotateIfNecessary (gString& sFile, long currentSize) ;

protected:
    gCGenLog () ;

    char scheme8[ 10 ];
    gString sIdent;
    FILE* fLog;
    FILE* fAltOutput;

    gList messages;

    virtual unsigned IdentValue (int width) ;

private:
    // Operators,empty
    gCGenLog (gCGenLog& ) ; //empty
    gCGenLog& operator= (gCGenLog& ) ; //empty
};

////////////////////////////////////////////////////////////
class gSLog : public gCGenLog {
public:
    gSLog () ;
    virtual ~gSLog () ;

    // Public data-members
    char* programCall;  // never allocated
    char** programArgs; // never allocated
    gString sNamedProgram;
    gString sHost;
    gString sHostnameToLog;
    unsigned xPid;
    unsigned pPid;
    t_int8 identIncr;
    gDateString logStamp;

    // Special methods
    int Log (FILE* logFile, int level, const char* formatStr, ...) ;

    int DumpMessage (FILE* fOut, gString* strMsg, const char* strIdent, t_uint16 mask=1) ;

private:
    gSLog (gSLog& ) ; //empty
    gSLog& operator= (gSLog& ) ; //empty
};

////////////////////////////////////////////////////////////
// Sin Log functions
////////////////////////////////////////////////////////////
extern int print_log_level (FILE* f,
			    gSLog& aLog,
			    int level) ;

extern int log_line_input (int currentYear,
			   int length,
			   const char* strMsg,
			   t_stamp& thisStamp) ;

extern int log_dump (FILE* fLog,
		     gSLog& aLog,
		     int hasFork,  // -1: no process ident!
		     unsigned thisPid) ;

// Main function log externals:
extern int log_file_flush (gSLog& aLog, int hasFork, unsigned thisPid) ;

// File funcions
extern int ilf_fileno (FILE* f) ;
extern int ilf_create (const char* strFilename, int mask, int& error) ;
extern int ilf_append (const char* strFilename, int mask, int& error) ;
extern int ilf_openrw (const char* strFilename, int mask, int& error) ;

extern int ilf_close (int& fd) ;

extern int ilf_fclose (FILE** ptrFile) ;


// Time functions

extern char* ilf_ctime (time_t stamp) ;
extern char* ilf_timenow () ;

// Log functions
extern t_uchar* log_string_from_tod (int iStamp) ;

extern int log_lockf_lock (int fd, int len) ;
extern int log_lockf_unlock (int fd, int len) ;

////////////////////////////////////////////////////////////
#endif //gILOG_X_H

