#ifndef DBA_X_H
#define DBA_X_H

#include "lib_iobjs.h"


#define DBA_FIELD_STR_FORMAT	"ISO-8859-1"

#define DBA_FIELD_STR_SURFACE	DBA_FIELD_STR_FORMAT  // "s/ISO-8859-1"
#define DBA_FIELD_NUM_SURFACE	"n/16"  // network order, two octets
#define DBA_FIELD_HEX_SURFACE	"h/16"  // network order, "0x" plus 4 hex chars (6 bytes in total)

#if defined(BIG_ENDIAN) || defined(iDOS_SPEC)
#define DB_NUM_2_BYTE(x)  (((x) >> 8) | (((x) & 0xFF) << 8))
#define DB_NUM_4_BYTE(x)  ((DB_NUM_2_BYTE( (x) >> 16)) | (DB_NUM_2_BYTE((x) & 0xFFFF) << 16))
#else
#warning Motorola-like processor detected
#define DB_NUM_2_BYTE(x)  (x)
#define DB_NUM_4_BYTE(x)  (x)
#endif

////////////////////////////////////////////////////////////
struct dbiHeader {
    char head[ 4 ];  // "dbi'\0'
    char versionMajor[ 4 ];
    char versionMinor[ 2 ];
    char versionEnhPkg[ 2 ];
    char magicNumber[ 10 ];
    t_uint32 nrTables;
    char pad[ 2 ];
    t_uint16 num01FE;
};

////////////////////////////////////////////////////////////
class DBA : public gList {
public:
    DBA () ;

    virtual ~DBA () ;

    // Public data members
    gList configs;

    // Get methods

    virtual gString& ErrorMsg () {
	return sErrorMsg;
    }

    virtual int ValidError (int error=-1, bool forceError=true) ;

    virtual gList* Config (const char* strParam=nil) ;

    virtual int Elements () {
	return (t_uint32)iValue;
    }

    virtual gList& Tables () {
	return tables;
    }

    virtual gList& TableFiles () {
	return tableFiles;
    }

    virtual t_uint32 CRC32 () {
	return uCRC;
    }

    virtual gFileStat* DoStat (const char* fileOrDir) ;

    bool ValidateTableName (gString& s) ;

    bool ValidateTableName (gString& s, gString& result) ;

    // Set methods

    virtual gList* AddConfigLine (const char* strLine) ;

    gList* AddConfig (const char* strLeft, const char* strRight) ;

    int RemoveConfig (const char* strLeft) ;

    // Special methods

    int InitDB (const char* strDbFileName) ;

    int InitDB (gString& aDbFileName) ;

    int InitDB (gList& params) ;

    int CreateDB (gString& sPathName, bool forceRewrite=false) ;

    int CreateDB (gString& sPathName, gList& params, bool forceRewrite=false) ;

    int OpenDB (gString& sPathName) ;

    virtual bool CloseDB () ;

    int CreateTable (const char* strTableName) ;

    int CreateTable (gString& sTableName) ;

protected:
    gFileStat aStat;
    gString sErrorMsg;
    gList tables;
    gList tableFiles;

    void CommonInit () ;

    bool BuildName (gString& sPath, const char* strName, const char* strExt, gString& result) ;

    int thisUpdateDBI () ;

private:
    t_uint32 uCRC;
    int dbiHandle;

    gString sControlFile;
};

////////////////////////////////////////////////////////////

extern char* dba_simpler_path (gString& aStr, int mask) ;

extern gList* dba_fields_from_string (const char* strFields) ;

extern gList* split_chars (const char* str, const char* split) ;

////////////////////////////////////////////////////////////
#endif //DBA_X_H

