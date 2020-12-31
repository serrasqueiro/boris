#ifndef ISOCONV_X_H
#define ISOCONV_X_H

#include "iunicode.h"


#define IMB_SANE(optStr) ASSERTION(ptrUniData->inUse,optStr); \
				ASSERTION(ptrUniData->hashUcs8Custom[ 0 ]!=-128,optStr);

#define IMB_ISO8859_1_STR_EURO_SIGN		"€"  // 128d

#define IMB_ISO8859_1_CHR_ACUTE_ACCENT		0xB4
#define IMB_ISO8859_1_CLOSE_QUOTE		0x92	//Windows-1252 ASCII 0x92
#define IMB_ISO8859_1_CHR_MIDDLE_DOT		0xB7

#ifdef IMB_ALLOW_DISPLAY_8BIT
#define IMB_DUMP_NEUTRAL IMB_ISO8859_1_CHR_MIDDLE_DOT
#else
#define IMB_DUMP_NEUTRAL '.'
#endif


#ifdef DEBUG_UNI
#define printu(args...) printf(args)
#else
#define printu(args...) ;
#endif

////////////////////////////////////////////////////////////
struct sIsoUni {
    sIsoUni (t_unitable aIsoTable=885901) ;

    ~sIsoUni () {
	Release();
    }

    t_unitable isoTable;
    t_uchar strUcs16Buf[ 4 ];
    gUniCode* uniCode;
    gUniCode* inUse;  // never allocated, just a pointer for the currently 'in use' data
    gList* buffers;

    char hashUcs8Printable[ 256 ];
    char* hashUcs8Custom;  // never allocated
    char customUcs8[ 4 ][ 256 ];

    // UCS16 hashes (for ASCII 0...255):
    t_uchar* hashUcs16Eq[ 256 ];
    t_uchar* hashUcs16Strs[ 256 ];

    gList isoForCountry;

    bool Release () ;

    bool Is8859 () {
	return isoTable>=885901 && isoTable<=885999;
    }

    bool IsIsoLatin () {
	t_uint16 isoVariant;
	return IsIsoLatin( isoVariant );
    }

    bool IsIsoLatin (t_uint16& isoVariant) {
	isoVariant = isoTable%100;
	return Is8859()==true && isoVariant==1;
    }

    int RefactorUcs16Eq () ;
};

////////////////////////////////////////////////////////////

extern sIsoUni* ptrUniData;

extern int imb_iso_init (const char* strPath, sIsoUni* newUniData) ;

extern int imb_iso_release () ;

extern int imb_iso_show (FILE* fOut, sIsoUni& data) ;


extern t_uchar* imb_iso_str (const t_uchar* strIn, int mask, int maxBufSize, t_uchar* strBuf) ;

extern t_uchar* imb_iso_str_custom (const t_uchar* strIn, int mask, const char* strHash, int maxBufSize, t_uchar* strBuf) ;

extern t_uchar* imb_iso_bin (const t_uchar* strIn, int size, int mask, int maxBufSize, t_uchar* strBuf) ;

extern t_uchar* imb_iso_bin_custom (const t_uchar* strIn, int size, int mask, const char* strHash, int maxBufSize, t_uchar* strBuf) ;


extern void imb_init_words (t_uint16 which) ;

extern char* imb_nice_words (t_uint16 which, char* strResult) ;


////////////////////////////////////////////////////////////
#endif //ISOCONV_X_H

