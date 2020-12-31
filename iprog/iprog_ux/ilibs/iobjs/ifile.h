#ifndef iFILE_X_H
#define iFILE_X_H

#include <stdio.h>

#include "iobjs.h"
#include "icontrol.h"

// OS-SPECIFIC
#ifdef iDOS_SPEC
	;
#else
#include <unistd.h>
#include <sys/types.h>      // For off_t, ...
#endif //iDOS_SPEC (~)
// *end* OS-SPECIFIC


#ifdef iDOS_SPEC
#include <winsock2.h>
#else
#include <sys/socket.h>  //socket... (sys/types.h)
#include <netdb.h>       //gethostbyname...etc
#include <arpa/inet.h>   //inet_addr...
#endif //iDOS_SPEC (~)


#ifdef DEBUG_FIL
#define DEF_FIL_BUFSIZE 40
#else
#define DEF_FIL_BUFSIZE (4*4096)  // Normal line-buffer size
#endif

#define FL_FILE_TO_READ     2
#define FL_FILE_TO_WRITE    4

#define GX_SIGHUP    1
#define GX_SIGINT    2
#define GX_SIGQUIT   3
#define GX_SIGTRAP   5
#define GX_SIGKILL   9
#define GX_SIGUSR1  10
#define GX_SIGSEGV  11
#define GX_SIGUSR2  12
#define GX_SIGPIPE  13
#define GX_SIGALRM  14
#define GX_SIGTERM  15
#define GX_SIGCHLD  17


#if defined(linux) || defined(gDOS_LIB_CRT_DLL)
#define IX_GETPID() getpid()
#else
#define IX_GETPID() 0
#endif //linux...

#define LINE_HAS_CR "#"


#ifdef DEBUG_IO
#define ioprint(args...) printf(args)
#else
#define ioprint(args...) ;
#endif // ~DEBUG_IO

////////////////////////////////////////////////////////////
class gFileOut : public gControl {
public:
    gFileOut (FILE* aFile=nil, bool isModeDOS=false) ;

    virtual ~gFileOut () ;

    FILE* f;

    // Get methods
    virtual char* NewLine () {
	return sNL.Str();
    }

    virtual int MinimumStorageSize () {
	return 0;
    }
    virtual int MaximumStorageSize () {
	return -1;
    }

    // Set methods
    void SetModeDOS (bool isDOS) ;

    virtual void SetNL (gString& aNL) {
	sNL = aNL;
    }

protected:
    gString sNL;

private:
    // Operators,empty
    gFileOut (gFileOut& ) ; //empty
    gFileOut& operator= (gFileOut& ) ; //empty
};
////////////////////////////////////////////////////////////
class gFileControl : public gControl {
public:
    ~gFileControl () ;

    static gFileControl& Self () {
	return myself;
    }

    // Public data-members
    char tmpPath[ 300 ];
    char tmpPrefix[ 10 ];
    char tmpSuffix[ 10 ];
    t_uint16 userId;
    FILE* fOutput;
    FILE* fReport;
    int exitCodeOnSignal;
    gString sProgramName;

    // Public enums
    enum eSafeStream {
	e_SafeStream
    };

    // Get methods
    int CtrlGetTempPath (char* resPathStr, int maxLength=-1) ;

    t_uint32 CtrlGetPid () ;

    char* CtrlGetUserNameStr () {
	return strUName;
    }

    t_uint32 GetCurrentEpoch () ;

    gString& GetUniqueName (const char* s) ;

    virtual char* ErrorStr (int aErrorNo) ;

    virtual gList& TemporaryFiles () ;

    FILE* OutputStream () {
	return fOutput;
    }

    FILE* OutputStream (eSafeStream safeStream) {
	if ( fOutput ) return fOutput;
	return stdout;
    }

    FILE* ReportStream () {
	return fReport;
    }

    FILE* ReportStream (eSafeStream safeStream) {
	if ( fReport ) return fReport;
	return stderr;
    }

    // Set methods
    bool AddTempFile (gString& s) ;

    // Specific methods
    int RemoveTemp () ;
    int RemoveTemp (unsigned& nTemp) ;

    virtual int ReleaseAll () ;

    // I/O methods
    bool Write (int fHandle, t_uchar* uBuf, unsigned nBytes) ;

    // Utilities
    void GetCwd (gString& s) ; //Get current working directory

    int NanoSleep (t_uint32 uSec, t_uint32 nSec) ;

    int NanoSleep (t_uint32 nSec) {
	return NanoSleep( 0, nSec );
    }

    int SecSleep (t_uint32 aSec) ;

    int MiliSecSleep (t_uint32 aSec) ;

    int StreamClose (FILE** ptrFile, eSafeStream safeStream=e_SafeStream) ;

protected:
    gFileControl () ;

    char strUName[ 32 ];
    gList* tempL;  // List of temporary files

    int thisGetUniqueName (const char* s,
			   t_uint32& aStamp,
			   t_uint32& aRand,
			   gString& sRes) ;

private:
    static gFileControl myself;

    gString sErrorRef;

    // Operators,empty
    gFileControl (gFileControl& ) ; //empty
    gFileControl& operator= (gFileControl& ) ; //empty
};
////////////////////////////////////////////////////////////
class gFile {
public:
    // Public enums
    enum eFileKind {
	e_Text,
	e_Binary
    };

    enum eDeviceKind {
	e_fDevOther,
	e_fStdin,
	e_fStdout,
	e_fStderr
    };

    gFile (eFileKind aFKind, const char* fName, bool doOpenToRead, bool isTmpFile=false) ;
    virtual ~gFile () ;

    // Public data-members
    int lastOpError;

    // Get methods
    virtual bool IsOk () {
	return lastOpError==0 && IsOpened();
    }

    eFileKind FileKind () {
	return fKind;
    }

    eDeviceKind Device () {
	return dKind;
    }

    virtual bool IsDevice () {
	return dKind!=e_fDevOther;
    }

    virtual bool IsOpened () {
	return f!=NULL;
    }

    FILE* Stream () {
	return f;
    }

    t_uint16 Mode () {
	return fMode;
    }

    char* LastErrorStr () ;

    char* ErrorStr (int aErrorNo) ;

    // IO-methods
    virtual bool OpenDevice (eDeviceKind aDKind) ;

    virtual bool OpenToRead (const char* fName) ;

    virtual bool Overwrite (const char* fName) ;

    virtual bool Close () ;

    bool ReadData (void* buf, t_uint16 bufSize) ;

    virtual bool ReadBuffer (void* buf, t_uint16 bufSize, t_uint16& nBytes) ;

    virtual bool Read (void* buf, t_uint16 bufSize, t_uint16& nBytes) ;

protected:
    FILE* f;
    eFileKind fKind;
    eDeviceKind dKind;
    t_uint16 fMode;
    char lastErrorMsg[1024];

    bool thisRead (int fd, void* buf, t_uint16 bufSize, t_uint16& nBytes) ;

private:
    // Operators,empty
    gFile (gFile& ) ; //empty
    gFile& operator= (gFile& ) ; //empty
};
////////////////////////////////////////////////////////////
class gFileStream : public gFile {
public:
    gFileStream (const char* fName, bool doOpenToRead) ;

    gFileStream (gFile::eFileKind aFKind, const char* fName, bool doOpenToRead=true, bool isTmpFile=false) ;

    virtual ~gFileStream () ;

    // Get methods
    virtual bool IsBufferOk () {
	return isBufferOk;
    }

    virtual t_uint16 BufferSize () {
	return bufferSize;
    }

    virtual t_uint32 SeekPos () {
	return (t_uint32)seekPos;
    }

    virtual t_uint32 Size () {
	return (t_uint32)seekEnd;
    }

    virtual char* Buffer () ;
    virtual t_uchar* UBuffer () ;

    // IO-methods
    virtual bool Overwrite (const char* fName) ;

    virtual bool ReadBuffer (void* buf, t_uint16 bufSize, t_uint16& nBytes) ;
    virtual bool Read (void* buf, t_uint16 bufSize, t_uint16& nBytes) ;

    virtual bool Rewind () ;

protected:
    bool isOpOk, isFileChanged;
    bool isBufferOk;
    off_t seekPos, seekEnd;
    t_uint16 bufferSize;
    t_uchar* buffer;

    int thisAllocateBuffer (t_uint16 aBufferSize) ;

private:
    int thisGetStreamSize (int fd) ;

    // Operators,empty
    gFileStream (gFileStream& ) ; //empty
    gFileStream& operator= (gFileStream& ) ; //empty
};
////////////////////////////////////////////////////////////
class gFileText : public gFileStream {
public:
    gFileText (const char* fName, bool doOpenToRead=true) ;

    virtual ~gFileText () ;

    // IO-methods
    bool ReadLine () ;

    bool ReadLine (bool& isOk) ;

    bool ReadLine (bool& isOk, bool& hasNewLine) ;

protected:
    bool isOpOk, isFileChanged;
    off_t seekPos, seekEnd;

private:
    // Operators,empty
    gFileText (gFileText& ) ; //empty
    gFileText& operator= (gFileText& ) ; //empty
};

////////////////////////////////////////////////////////////
class gFileTemp : public gFileStream {
public:
    // Public enums
    enum eTempMethod {
	e_NameStd,   // Via mkstemp
	e_NamePre    // Big string based on 'pid',...
    };

    gFileTemp (const char* strTemplate, gFile::eFileKind aFKind=gFile::e_Binary) ;

    gFileTemp (gFile::eFileKind aFKind) ;

    virtual ~gFileTemp () ;

    // Public data-members
    int fHandle;
    gString sTempName;

    // Get methods
    virtual bool IsOpened () ;

    // Other methods
    virtual bool Rewind () ;

protected:
    eTempMethod tempMethod;
    bool isHandledFStream;

    int thisOverwrite (const char* fName) ;

private:
    // Operators,empty
    gFileTemp (gFileTemp& ) ; //empty
    gFileTemp& operator= (gFileTemp& ) ; //empty
};
////////////////////////////////////////////////////////////
#endif //iFILE_X_H

