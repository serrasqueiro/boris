#ifndef iLANGED_X_H
#define iLANGED_X_H


#include "lib_iobjs.h"

#include "lib_imedia.h"


#define LANGED_ASCII8BIT_ACCUTE_ACCENT	0xB4	// Unicode 00B4;ACUTE ACCENT
#define LANGED_ASCII8BIT_MIDDLE_DOT	0xB7	// Unicode 00B7;MIDDLE DOT


#define LANGED_ISO_CHARSET_DEFAULT	885901	// iso-8859-1, v is 885900+1
#define LANGED_MAIN_CHARSET(v)	(v/100)		//	--> v/1000 is the major charset, v%1000 is the table variant
#define LANGED_CHARSET_TVAR(v)	(v%100)


////////////////////////////////////////////////////////////
struct sLangParam {
#pragma pack(1)
    // (4+4+16+40) 64 bytes total
    t_int32 code;
    const char abbrev[ 4 ];  // e.g. 'en'; see also lang+scripts.txt
    char lang[ 16 ];   // e.g. 'en-US'
    char name[ 40 ];   // e.g. english
};


struct sUniHint {
#pragma pack(1)
    // (4+2+32+2+24) 64 bytes in total 
    t_unicode code;  // e.g. 0xC0 (A') 0xE1 (a'), LATIN CAPITAL|SMALL LETTER A WITH ACUTE
    char twoLetter2[ 2 ];  // 2char (not null terminated)
    char strDesc[ 32 ];  // last chr must be null (0x0)
    t_int16 shortHist;  // for a small histogram
    t_int8 langs[ 24 ];  // 0: uninitialized / unused
};


struct sUniPreHint {
    t_unicode code;
    char* strTwoLetter;
    char* strDesc;
};

////////////////////////////////////////////////////////////
struct sLanged {
    sLanged ()
	: defaultLangCode( 0 ),
	  currentLangCode( 0 ),
	  charset( LANGED_ISO_CHARSET_DEFAULT ),
	  myUniData( nil ),
	  maxUcs16EqLen( -1 ),
	  cached( nil ) {
    }

    ~sLanged () {
	Release();
    }

    t_int32 defaultLangCode;
    t_int32 currentLangCode;

    t_unitable charset;

    gString sAbbrev;  // 'en', 'pt', 'es', 'fr', 'ge'
    gString sLanguage;  // 'en-US', or 'en-GB'; 'pt-PT', or 'pt-BR'

    sIsoUni* myUniData;  // Never allocated

    int maxUcs16EqLen;

    gList* cached;

    // Methods
    int Init () ;

    void Release () ;
};

////////////////////////////////////////////////////////////
// lang functions
////////////////////////////////////////////////////////////

extern sLanged mainLang;


extern int langed_init () ;

extern int langed_finit () ;

extern char* langed_charset (gString& charsetName, gString& charsetSuggested, t_unitable& charset) ;

extern int langed_split_words (const t_uchar* buf, int mask, gList& listed) ;

////////////////////////////////////////////////////////////
#endif //iLANGED_X_H

