#ifndef iDIR_X_H
#define iDIR_X_H

#include <dirent.h>

#include "ilist.h"
#include "ifile.h"
#include "ifilestat.h"

////////////////////////////////////////////////////////////
enum eFileSystemName {
    e_FSysNoName,
    e_FSysFile,
    e_FSysDir
};

////////////////////////////////////////////////////////////
class gFileSysName : public gString {
public:
    virtual ~gFileSysName () ;

    // Get methods
    virtual bool IsOk () {
	return fsysName!=e_FSysNoName;
    }

    virtual bool IsStrOk () ;

    virtual bool IsDirectory () {
	return isDirectory;
    }

    virtual char* Str (unsigned idx=0) ;

    virtual char* Name () {
	return Str();
    }

    virtual bool HasStat () {
	return myStat!=nil;
    }

    virtual gFileStat& GetStat () ;

    virtual gFileStat* GetStatPtr () {
	return myStat;
    }

    virtual bool IsValidCharInName (t_uchar uChr, bool isStrict=true) ;

    virtual bool IsValidChar (t_uchar uChr, int strictLevel) ;

    // Set methods
    virtual bool InitializeValidChars256 (t_int8* vChars, t_uint16 size, int mask=0) ;

    // Public data-members
    eFileSystemName fsysName;
    static t_int8 validChars256[ 258 ];

protected:
    gFileSysName (const char* s,
		  eFileSystemName aFSysName,
		  bool isADirectory) ;

    gFileSysName (const char* s,
		  eFileSystemName aFSysName,
		  gFileStat& aStat) ;

    bool isDirectory;
    gFileStat* myStat;

private:
    // Operators,etc
    gFileSysName (gFileSysName& ) ; //empty
    gFileSysName& operator= (gFileSysName& ) ; //empty
};

////////////////////////////////////////////////////////////
class gFileName : public gFileSysName {
public:
    gFileName () ;
    gFileName (const char* s) ;
    gFileName (gString& s) ;
    gFileName (gString& s, gFileStat& aStat) ;
    virtual ~gFileName () ;

private:
    // Operators,etc
    gFileName (gFileName& ) ; //empty
    gFileName& operator= (gFileName& ) ; //empty
};

////////////////////////////////////////////////////////////
class gDirName : public gFileSysName {
public:
    gDirName () ;
    gDirName (const char* s) ;
    gDirName (gString& s) ;
    gDirName (gString& s, gFileStat& aStat) ;
    virtual ~gDirName () ;

    // Get methods
    virtual char* Str (unsigned idx=0) ;
    virtual char* Name () ;

    // Set methods
    virtual void SetDirName (const char* s) ;

    // Save/etc methods
    virtual t_uchar* ToString (const t_uchar* uBuf) ;

protected:
    gString dirStr;

    // Protected methods
    void thisBuildDirStr (const char* s) ;

private:
    // Operators,etc
    gDirName (gDirName& ) ; //empty
    gDirName& operator= (gDirName& ) ; //empty
};

////////////////////////////////////////////////////////////
class gDirStream : public gFile {
public:
    gDirStream (const char* dName=nil) ;
    virtual ~gDirStream () ;

    // Public data-members
    DIR* pdir;
    gString dirName;  // Original directory name
    gFileStat dStat;

    // Get methods
    virtual bool IsOpened () ;
    virtual char* Name () {
	return dirName.Str();
    }

protected:
    bool isDirOpened;

    int doOpenDir (const char* dName) ;
    int doCloseDir () ;

private:
    // Operators,etc
    gDirStream (gDirStream& ) ; //empty
    gDirStream& operator= (gDirStream& ) ; //empty
};

////////////////////////////////////////////////////////////
class gDirGeneric : public gList {
public:
    virtual ~gDirGeneric () ;

    // Public data-members
    int lastOpError;
    gDirName sDirName;
    static t_uchar slashChr;

    // Get methods
    virtual bool IsOk () {
	return lastOpError==0;
    }
    virtual bool IsNameOk (const char* s) ;
    virtual bool AllNamesOk (int depthLevel) ;

    virtual gFileSysName* GetName (unsigned idx) ;

    // Set methods
    // .

    // Save/etc methods
    virtual t_uchar* ToString (const t_uchar* uBuf) ;
    virtual bool SaveGuts (FILE* f) ;
    virtual bool RestoreGuts (FILE* f) ;

protected:
    gDirGeneric (eStorage kind) ;

    gString slashStr;

    // Protected methods
    bool thisNameOk (const char* s,
		     unsigned& posBad) ;

    bool thisAllNamesOk (unsigned depthLevel,
			 unsigned& posBad,
			 int& notOkDepth) ;

private:
    // Operators,empty
    gDirGeneric (gDirGeneric& ) ;
    gDirGeneric& operator= (gDirGeneric& ) ;
};

////////////////////////////////////////////////////////////
class gDir : public gDirGeneric {
public:
    gDir (const char* dName=nil) ;
    gDir (gString& dName) ;
    virtual ~gDir () ;

    // Public data-members
    gDirStream dirStream;
    gList errUnstatL;

    // Get methods
    virtual gFileStat* GetStat (unsigned idx) ;
    virtual bool GetNameDir (unsigned idx, gString& resName) ;
    virtual bool GetFullNameDir (unsigned idx, gString& resName, gString& resFullName) ;

    // Set methods
    virtual void AddDir (const char* s) ;
    virtual void AddFile (const char* s) ;

protected:
    int dirCount;

    gDir (const char* dName, bool doReadDir) ;

    virtual int ScanDir () ;

    virtual int AddStatName (eFileSystemName aFSysName, const char* s, gFileStat& aStat) ;

    int thisScanDir () {
	lastOpError = 1;
	return ScanDir();
    }

    int thisAddSystemName (gFileSysName* aPtr) ;

private:
    // Operators,empty
    gDir (gDir& ) ;
    gDir& operator= (gDir& ) ;
};
////////////////////////////////////////////////////////////
#endif //iDIR_X_H

