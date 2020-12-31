#ifndef iBASES_X_H
#define iBASES_X_H

// The preferred class is gString64!

#include "ibase.h"
#include "iunicode.h"

#define IM_ASSERTION(x,msg) ;
#define IM_ASSERTION_FALSE(args...) { fprintf(stderr,args); exit(5); }

#define IM_DBGPRINT DBGPRINT_MIN

#define I_ALPHA_CHRS	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
#define I_DIGIT_CHRS	"0123456789"
#define I_EXTRA_B64S	"+/"
#define I_BASE64_ALPHABET I_ALPHA_CHRS I_DIGIT_CHRS I_EXTRA_B64S


// Useful typedefs

typedef t_uint16 t_ucs4;


// Error codes, in increasing severity:

enum eRecodeError {
    ereNO_ERROR = 0,          /* no error so far */
    ereNOT_CANONICAL = 1,     /* input is not exact, but equivalent */
    ereAMBIGUOUS_OUTPUT = 2,  /* output will be misleading */
    ereUNTRANSLATABLE = 3,    /* input is getting lost, while valid */
    ereINVALID_INPUT = 4,     /* input is getting lost, but was invalid */
    ereSYSTEM_ERROR = 5,      /* system returned input/output failure */
    ereUSER_ERROR = 6,        /* library is being misused */
    ereINTERNAL_ERROR = 7,    /* programming botch in the library */
    ereMAXIMUM_ERROR = 8      /* impossible value (should be kept last) */
};


struct sTimeRef {
    // see also man (3) ftime

    time_t time;
    t_uint16 millitm;
    t_int16 timezone;
    t_int16 dstflag;
};


struct sUtfBox {
    sUtfBox ()
	: ucs2Error( ereNO_ERROR ),
	  lastUCS2( 0 ),
	  lastUCS4( 0 ),
	  strLast( nil ) {
    }

    ~sUtfBox () {
    }

    eRecodeError ucs2Error;
    t_unicode lastUCS2;
    t_uint32  lastUCS4;  // not used
    const t_uchar* strLast;

    void Reset () {
	ucs2Error = ereNO_ERROR;
	lastUCS2 = 0;
	lastUCS4 = 0;
	strLast = nil;
    }
};

////////////////////////////////////////////////////////////

extern int imb_absolute (int value) ;

extern int imb_strlen (const t_uchar* strIn) ;

extern t_uchar* imb_utf8_str_to_ISO8859 (const t_uchar* bufIn, int subOne) ;

extern t_uchar* imb_utf8_str_to_ucs2 (const t_uchar* bufIn, eRecodeError& returnError) ;

extern t_uchar* imb_utf8_str_to_ucs2_limit (const t_uchar* bufIn, t_uint32 maxMask, sUtfBox& box) ;

t_ucs4* imb_utf8_str_to_ucs4 (const t_uchar* bufIn, eRecodeError& returnError) ;
t_ucs4* imb_utf8_str_to_ucs4_limit (const t_uchar* bufIn, t_uint32 maxMask, sUtfBox& box) ;



extern eRecodeError imb_transform_utf8_to_ucs4 (int width,
						const t_uchar* bufIn,
						int maxBufOut,
						t_unicode* bufOut,
						int& outSize) ;



extern gString* imb_bin_to64 (int length,
			      t_uchar* binBuffer,
			      t_uint32& outLen) ;

extern gString* imb_auth_mime64 (const t_uchar* strUser,
				 const t_uchar* strPass) ;


extern int imb_dir_error (const char* strDir) ;   // returns 0 if yes

extern gList* imb_check_any_of (const char* strDir, gList& anyOf) ;

extern gList* imb_check_file (const char* strDir, const char* aFilename) ;

////////////////////////////////////////////////////////////
#endif //iBASES_X_H

