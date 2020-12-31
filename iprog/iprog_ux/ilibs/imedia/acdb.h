#ifndef iACDB_X_H
#define iACDB_X_H

#include "iunicode.h"


enum eCdbError {
    ece_NO_ERROR,           // no error so far
    ece_INPUT_TOO_BIG = 1,  // input too big
    ece_INPUT_TOO_SHORT,    // input is nearly empty
    ece_MISSING_EQ = 4,     // missing '=' in non-comment line
    ece_GARBLED_INPUT = 8
};


typedef t_uint32 t_cddb1;

typedef t_int16 t_mGenre;


struct sCdpTitle {
    sCdpTitle ()
	: nrTitle( -1 ),
	  millisecs( -1 ),
	  rawOther( nil ) {
    }

    ~sCdpTitle () {
    }

    t_int16 nrTitle;
    gString sName;
    t_int32 millisecs;
    gList* rawOther;
};


struct sCdpDisc {
    sCdpDisc ()
	: cddb1( 0x0 ),
	  length( -1 ),  // milliseconds
	  loaded( 0 ),  // stamp, when disc was loaded
	  isCompilation( false ),
	  discSet( 0 ),  // 1 of 3 (here discSet is 1)
	  discSetNumber( 0 ),  // 1 of 3 (here ...3)
	  year( 0 ),
	  genre1( -1 ),
	  genre2( -1 ),
	  rawNotes( nil ) {
    }

    ~sCdpDisc () {
	ReleaseExtras();
    }

    static const char* cdbKeys[];

    t_cddb1 cddb1;
    t_int32 length;  // milliseconds
    t_stamp loaded;
    bool isCompilation;
    t_int16 discSet, discSetNumber;
    t_uint16 year;
    t_mGenre genre1;
    t_mGenre genre2;

    gString dTitle;
    gString dArtist;
    gString dLabel;

    gString sLanguage;  // LANGUAGE=...
    gString sRegion;  // REGION=...

    gList tracksListed;
    gList* rawNotes;  // e.g. DNOTES=...

    // Methods
    void ReleaseExtras () {
	delete rawNotes; rawNotes = nil;
    }

    int WhichIndexFromKey (gString& sKey) {
	const char* strKey;
	for (int idx=1; (strKey = cdbKeys[ idx ])!=nil; idx++) {
	    if ( sKey.Match( (char*)strKey ) ) return idx;
	}
	return -1;
    }

    int WhichIndexFromLine (gString& sLine) {
	const char* strKey;
	for (int idx=1; (strKey = cdbKeys[ idx ])!=nil; idx++) {
	    if ( sLine.Find( (char*)strKey )==1 ) return idx;
	}
	return -1;
    }

    bool SetByRawString (gString& sRaw, int optPosEq=-1) ;
};


struct sCdpCdb {
    sCdpCdb ()
	: nTitles( -1 ),
	  ptrTitles( nil ),
	  anError( ece_NO_ERROR ),
	  rawCredits( nil ),
	  rawSegments( nil ),
	  rawExtra( nil ) {
    }

    ~sCdpCdb () {
	ReleaseAll();
    }

    sCdpDisc disc;

    int nTitles;
    gList titles;
    sCdpTitle* ptrTitles;  // ptrTitles[ 0 ] is generic

    eCdbError anError;

    gList* rawCredits;  // Strings like CRDNAME1, CRDROLE1, CRDTRK1
    gList* rawSegments;  // Strings like STITLE1, SSTTRK1, ...
    gList* rawExtra;  // Anything you like to add

    // Methods
    bool AddTitleFromRawName (gString& sRaw) ;

    bool EqLine (gString& sRaw, gString& sLeft, gString& sValue) ;

    void ReleaseAll () {
	nTitles = -1;
	delete[] ptrTitles;
	ptrTitles = nil;

	ReleaseRaw();
    }

    void ReleaseRaw () {
	delete rawCredits; rawCredits = nil;
	delete rawSegments; rawSegments = nil;
	delete rawExtra; rawExtra = nil;
    }
};

////////////////////////////////////////////////////////////

extern int buf_to_list (const t_uchar* uBuf, int bufSize, t_uchar newLine, gList& result) ;

extern int findback_non_digit (const t_uchar* uBuf, int& value) ;

extern int acdb_read (int fdIn, sCdpCdb& cdp) ;

extern int acdb_parse (gList& input, sCdpCdb& cdp) ;

////////////////////////////////////////////////////////////
#endif //iACDB_X_H

