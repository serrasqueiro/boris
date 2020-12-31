#ifndef I_MMDB_X_H
#define I_MMDB_X_H

#include "lib_iobjs.h"
#include "lib_imaudio.h"

#ifndef PRIME_MILLION_NP0
#define PRIME_MILLION_NP0  999983
#endif

#define MM_NAMED_PRIME PRIME_MILLION_NP0

#define MM_ERROR_STR(errorNumber) gFileControl::Self().ErrorStr( errorNumber )

#define MM_LOG(level,args...) S_LOG(mylog,mylog.Stream(),level,args)


#define MMDB_VERSION_MAJOR	0
#define MMDB_VERSION_MINOR	0
#define MMDB_COMPAT_SCHEME	0x0010

////////////////////////////////////////////////////////////
// mmdb structs
////////////////////////////////////////////////////////////
struct sAdbFactory {
    sAdbFactory ()
	: count( 0 ) {
    }

    ~sAdbFactory () {
    }

    int count;

    gList listedAll;
    gList listedArts;
    gList namedArts;
};


struct sMmDb {
    sMmDb ()
	: versionMajor( MMDB_VERSION_MAJOR ),
	  versionMinor( MMDB_VERSION_MINOR ),
	  scheme( MMDB_COMPAT_SCHEME ),
	  sArtstPref( "ART" ),
	  sTitlePref( "TIT" ),
	  hashDirLevels( 2 ),
	  runCode( -1 ),
	  listIn( nil ),
	  listOut( nil ) {
    }

    ~sMmDb () {
	CleanUp();
    }

    t_uint8 versionMajor, versionMinor;
    t_uint16 scheme;

    gString sBasePath;
    gString sArtstPref;
    gString sTitlePref;
    t_int16 hashDirLevels;
    int runCode;

    sAdbFactory factory;

    gList* listIn;
    gList* listOut;

    void CleanUp () {
	delete listIn; listIn = nil;
	delete listOut; listOut = nil;
    }
};

////////////////////////////////////////////////////////////
// mmdb functions
////////////////////////////////////////////////////////////

extern int adbb_create (const char* strPath, gList& listMsg) ;

extern int adbb_open (const char* strPath, sMmDb& mmdb) ;

extern int adbb_close (sMmDb& mmdb) ;

////////////////////////////////////////////////////////////
// Other functions
////////////////////////////////////////////////////////////

extern int mmdb_entry_trim (gString& sName, int mask) ;

extern sFileStat* mmdb_entry_stat (gString& sName, int& error) ;
extern int mmdb_entry_is_directory (gString& sName, bool& isDirectory) ;

extern int mmdb_list_errors (int errorCode, gList& list, FILE* fReport, FILE* fLog) ;

////////////////////////////////////////////////////////////
#endif //I_MMDB_X_H

