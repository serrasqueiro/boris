#ifndef iBASE_X_H
#define iBASE_X_H

#include <stdlib.h>

#include "lib_iobjs.h"

#define BS_ASSERTION(x,msg) ;
#define BS_ASSERTION_FALSE(msg) { fprintf(stderr,"Unasserted: %s\n",msg); exit(5); }

#if defined(DEBUG_MIN)
#define DBGPRINT_BAS(args...) printf(args)
#else
#define DBGPRINT_BAS(args...) ;
#endif

////////////////////////////////////////////////////////////
class gStringBase : public gString {
public:
    // Public enums
    enum eBase {
	e_Base64 = 64,
	e_Base65 = 65
    };

    enum eB64status {
	decode64_success,
	decode64_failure = 1,
	decode64_internal_error = 2
    };

    // Public data-members
    t_int16 convertCode;

    virtual ~gStringBase () ;

    // Get methods
    virtual gString& LastString () {
	return sResult;
    }

    // Set methods
    virtual bool UseNow (eBase newBase) ;

protected:
    gStringBase () ;
    gStringBase (char* aStr) ;

    t_uchar padChr;
    gString sResult;

    static t_uchar base64[ 128 ];
    static t_int16 convFromBase64[ 256 ];
    static const t_int16 sizeOfFastIdx;

    int thisInitBase64 (t_int16 size16bit) ;

private:
    // Operators,empty
    gStringBase (gStringBase& ) ; //empty
    gStringBase& operator= (gStringBase& ) ; //empty
};

////////////////////////////////////////////////////////////
class gString64 : public gStringBase {
public:
    gString64 () ;
    gString64 (char* aStr) ;
    gString64 (gString& s) ;

    gString64 (eBase base, char* aStr=nil) ;

    virtual ~gString64 () ;

    // Get methods
    // .

    // Set methods
    virtual bool UseNow (eBase newBase) ;

    // Special methods
    virtual char* Encode64 () ;

    virtual t_uchar* Decode64 () ;

    virtual char* EncodeBinTo64 (unsigned srcLen, t_uchar* binBuffer, t_uint32& outLen) ;

    virtual t_uchar* Decode64ToBin (unsigned bufLen, t_uchar* binBuffer, t_uint32& outLen) ;

protected:
    eB64status myBase64Decode (t_uchar* src,  // Note: trailing '=' are cut.
			       t_uint32 srcLen,
			       t_uchar* dest,
			       t_uint32& dstLen,
			       t_uint32& outLen) ;

private:
    void myBase64Encode_3to4 (const t_uchar* src, t_uchar* dest) ;
    void myBase64Encode_2to4 (const t_uchar* src, t_uchar* dest) ;
    void myBase64Encode_1to4 (const t_uchar* src, t_uchar* dest) ;

    eB64status myBase64Decode_4to3 (const t_uchar* src, t_uchar* dest) ;
    eB64status myBase64Decode_3to2 (const t_uchar* src, t_uchar* dest) ;
    eB64status myBase64Decode_2to1 (const t_uchar* src, t_uchar* dest) ;

    gString64 (gString64& ) ; //empty
    gString64& operator= (gString64& ) ; //empty
};
////////////////////////////////////////////////////////////

// Basic Base64 functions:
extern const t_uchar* ibase_base64 () ;
extern const t_uchar ibase_base64_item (t_uint8 item) ;

extern void ibase_Base64Encode_3to4 (const t_uchar* src, t_uchar* dest) ;
extern void ibase_Base64Encode_2to4 (const t_uchar* src, t_uchar* dest) ;
extern void ibase_Base64Encode_1to4 (const t_uchar* src, t_uchar* dest) ;

extern bool ibase_Prime (unsigned value) ;

extern unsigned ibase_NextPrime (unsigned value) ;

////////////////////////////////////////////////////////////
#endif //iBASE_X_H

