#ifndef iCONTROL_X_H
#define iCONTROL_X_H

#include "ilist.h"

////////////////////////////////////////////////////////////
class gControl : public gStorage {
public:
    gControl (eStorage aKind=e_Control) ;

    virtual ~gControl () {
    }

    // Public data-members
    int lastOpError;
    int dbgLevel;
    gString sName;
    static int nErrors[ LOG_LOGMAX ];
    static gList lLog[ LOG_LOGMAX ];

    // Public enums
    enum eDescriptionStoreType {
	e_none = 0,
	e_ASCII_only,
	e_ASCII_and_HTML
    };

    // Get methods
    virtual bool IsOk () {
	return lastOpError==0;
    }

    virtual char* GetErrorStr () {
	return sStrError;
    }

    virtual t_uint16 GetRandom (t_uint16 maxRange) ;

    virtual int MinimumStorageSize () {
	return 0;
    }
    virtual int MaximumStorageSize () {
	return 0;
    }

    int ConvertToUInt32 (const char* s,
			 t_uint32& vRes) {
	return sName.ConvertToUInt32( s, vRes );
    }

    t_uint32 ConvertToUInt32 (t_uint32 defaultValue=0) {
	return sName.ConvertToUInt32( defaultValue );
    }

    int ConvertToInt32 (const char* s, t_int32& vRes) {
	return sName.ConvertToInt32( s, vRes );
    }

    t_int32 ConvertToInt32 (const char* s) {
	return sName.ConvertToInt32( s );
    }

    int ConvertToUInt32 (const char* s,
			 unsigned base,
			 eDigitConv caseSense,
			 t_uint32& vRes,
			 unsigned& posErr) {
	return ConvertToUInt32( s, base, caseSense, vRes, posErr );
    }

    int ConvertToInt32 (const char* s,
			unsigned base,
			eDigitConv caseSense,
			t_int32& vRes,
			unsigned& posErr) {
	return sName.ConvertToInt32( s, base, caseSense, vRes, posErr );
    }

    int ConvertHexToUInt32 (const char* s,
			    eDigitConv caseSense,
			    t_uint32& vRes) {
	return sName.ConvertHexToUInt32( s, caseSense, vRes );
    }

    t_uint32 ConvertHexToUInt32 (const char* s, t_uint32 defaultValue=0) {
	return sName.ConvertHexToUInt32( s, defaultValue );
    }

    // Set methods
    virtual void Reset () ;

    virtual void ResetLog () ;

    virtual int SetDefaultDbgLevel (int aDbgLevel) {
	return dbgLevelDefault = aDbgLevel;
    }

    virtual int SetError (int opError) ;

    // Special methods
    int Log (FILE* dbgFile, int level, const char* formatStr, ...) ;
    virtual int ClearLogMem (int level) ;
    virtual int ClearLogMemAll () ;
    virtual int ClearAllLogs () ;

    // Save/etc methods
    virtual gStorage* NewObject () ;

    virtual t_uchar* ToString (const t_uchar* uBuf) {
	return nil;
    }

    virtual bool SaveGuts (FILE* f) {
	return CanSave( f );
    }
    virtual bool RestoreGuts (FILE* f) {
	return CanRestore( f );
    }

    // Show methods
    virtual void Show (bool doShowAll=true) ;

protected:
    char sStrError[200];
    static int dbgLevelDefault;
    static char logBuf[4096];

private:
    // Operators,empty
    gControl (gControl& ) ; //empty
    gControl& operator= (gControl& ) ; //empty
};

////////////////////////////////////////////////////////////
class gStorageControl : public gControl {
public:
    virtual ~gStorageControl () ;

    static gStorageControl& Self () {
        return myself;
    }

    // Get methods
    virtual int NumObjs () ;

    virtual gList& Pool () {
	return pool;
    }

    virtual gList& DescriptionList () {
	return descriptions;
    }

    // Set methods

    virtual bool StartAll () ;  // initializes Windows wsock32, or just Init in Linux

    virtual bool Init () ;

    virtual bool End () ;

    virtual void Reset () ;

    virtual bool DeletePool () ;

    void StaticAlloc (const char* msgStr, int incrNrStaticStgObjs) ;

    bool RegisterDescription (gStorage& object, const t_desc_char* description) ;

    bool RegisterDescriptionStr (gStorage& object, const char* description) ;

    void TidyDescriptions () ;

    // Show methods

    virtual void Show (bool doShowAll=true) ;

protected:
    gStorageControl () ;

    gList pool;
    gList descriptions;

private:
    static int rStaticStgObjs;
    static int rStaticInitNrObjs;
    static eDescriptionStoreType descStoreType;

    static gStorageControl myself;

    void DeleteDescriptions () {
	descriptions.Delete();
    }

    // Operators,empty
    gStorageControl (gStorageControl& ) ; //empty
    gStorageControl& operator= (gStorageControl& ) ; //empty
};
////////////////////////////////////////////////////////////
#endif //iCONTROL_X_H

