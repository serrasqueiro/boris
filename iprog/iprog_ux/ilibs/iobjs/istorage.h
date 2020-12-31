#ifndef iSTORAGE_X_H
#define iSTORAGE_X_H

#include <stdio.h>
#include <string.h>

#include "iobjs.h"

#define nil 0
#define MAX_INTSTGKIND 1000
#ifdef DEBUG_BUF
#define GENUCHAR_USU_BUFSIZE 20    // for debug, only (a small buffer)
#else
#define GENUCHAR_USU_BUFSIZE 4096  // UCharBuffer...
#endif //DEBUG

#ifndef LOG_NONE
#define LOG_NONE -1
#endif
#ifndef LOG_ERROR
#define LOG_ERROR 0
#endif
#ifndef LOG_WARN
#define LOG_WARN 1
#endif
#ifndef LOG_NOTICE
#define LOG_NOTICE 2
#endif
#ifndef LOG_INFO
#define LOG_INFO 3
#endif
#ifndef LOG_LOGMAX
#define LOG_LOGMAX (LOG_INFO+1)
#endif
#ifndef LOG_NOCLASS
#define LOG_NOCLASS 99
#endif

////////////////////////////////////////////////////////////
class gTop {
public:
    virtual ~gTop () ;

    // Public data-members
    t_uint16 stgKind;
    static short nObjHistogram[ MAX_INTSTGKIND ];

    int GetNumObjects () {
	return nObjs;
    }

    virtual bool IsString () {
	return false;
    }

    virtual int GetInt () {
	return 0;
    }

    virtual bool HasDescription () {
	return descriptionIndex!=0;
    }

    virtual t_desc_char* Description () {
	if ( ptrDescription ) {
	    return ptrDescription;
	}
	return noDescriptionStr;
    }

    virtual t_uchar* DescriptionStr () {
	if ( ptrDescription ) {
	    return (t_uchar*)ptrDescription;
	}
	return (t_uchar*)noDescriptionStr;
    }

    virtual int HashSize () {
	return sizeof(int) * 8;  // 32bits for normal platforms
    }

    int StringHash (const char* aStr) ;

    int StringHash (const char* aStr, int iLength) ;

    // Set methods

    virtual void ResetDescription () {
	descriptionIndex = 0;
	ptrDescription = nil;
	memset(strIndex, 0x0, sizeof(strIndex));
    }

    void SetDescription (t_int32 index, const t_desc_char* description) {
	descriptionIndex = index;
	ptrDescription = (t_desc_char*)description;
	toDecimalString( index );
    }

    // Pure virtual
    virtual int Hash () = 0;  //PURE

    virtual int Rehash () = 0;  //PURE

protected:
    gTop (t_uint16 intStgKind) ;

    t_int32 descriptionIndex;
    t_desc_char* ptrDescription;

private:
    static int nObjs;
    static t_int32 maxDescriptionIndex;
    static t_desc_char* noDescriptionStr;
    char strIndex[ 12 ];

    void toDecimalString (t_int32 value) {
	sprintf(strIndex, "%d", value);
    }

    // Operators,empty
    gTop (gTop& ) ; //empty
    gTop& operator= (gTop& ) ; //empty
};

////////////////////////////////////////////////////////////
class gUCharBuffer {
public:
    gUCharBuffer (t_uint16 bufSize=0) ;

    ~gUCharBuffer () ;

    // Public data-members
    t_uint16 size;
    t_uchar* uBuf;

    // Get methods
    char* Str () {
	return (char*)uBuf;
    }

    // Set methods
    void Copy (t_uchar* s) ;
    void Copy (char* s) ;

    void Clear () ;

private:
    void thisAllocate (t_uint16 bufSize) ;
    void thisCopy (t_uchar* s, t_uint16 len) ;

    // Operators,empty
    gUCharBuffer (gUCharBuffer& ) ; //empty
    gUCharBuffer& operator= (gUCharBuffer& ) ; //empty
};

////////////////////////////////////////////////////////////
class gStorage : public gTop {
public:
    // Public enums
    enum eStorage {
	e_NoStorage,
	e_ResvdStore  = 1,
	e_UChar       = 2,
	e_SChar       = 3,
	e_UInt        = 4,
	e_SInt        = 5,
	e_ULongInt    = 6,
	e_SLongInt    = 7,
	e_ResvdStore8 = 8,
	e_ResvdStore9 = 9,
	e_Real        = 10,
	e_List        = 11,
	e_ResvdStore12= 12,
	e_ResvdStore13= 13,
	e_ResvdStore14= 14,
	e_ResvdStore15= 15,
	e_ResvdStore16= 16,
	e_ResvdStore17= 17,
	e_ResvdStore18= 18,
	e_ResvdStore19= 19,
	e_String      = 20,
	e_StrUnicode  = 21,
	e_ResvdStore22= 22,
	e_ResvdStore23= 23,
	e_ResvdStore24= 24,
	e_ResvdStore25= 25,
	e_ResvdStore26= 26,
	e_ResvdStore27= 27,
	e_ResvdStore28= 28,
	e_ResvdStore29= 29,
	e_ResvdStore30= 30,
	e_StoreExtend = 31,
	e_Control,
	e_UnusedStore
    };

    enum eStoreMethod {
	e_StgNoStore      = 0,
	e_StgGlobalString = 1,
	e_StgGlobalFlat   = 2,
	e_StgString       = 3,
	e_StgFlat         = 4,
	e_StgDefault      = 64
    };

    virtual ~gStorage () ;

    // Public data-members
    int iValue;

    // Get methods
    virtual bool IsOk () {
	return true;
    }

    eStorage Kind () {
	return kind;
    }

    virtual eStoreMethod GetStoreMethod () {
	return storeMethod;
    }

    virtual unsigned N () {
	return 1;
    }

    virtual int Hash () {
	if ( iValue==-1 || iValue==0 ) {
	    // Although iValue=0 is recalculated, probability is low;
	    // 0 or -1 indicates hash has not been done;
	    // Hash result is never -1.
	    Rehash();
	}
	return iValue;
    }

    virtual int Rehash () {
	return iValue = StringHash( Str() );
    }

    virtual int GetInt () {
	return iValue;
    }

    virtual int GetIoMask () {
	return ioMask;
    }

    virtual char* Str (unsigned idx=0) ;

    virtual unsigned Length () {
	char* aStr( Str() );
	return aStr ? (unsigned)strlen( aStr ) : 0;
    }

    virtual int IntLength (unsigned idx=0) {
	char* aStr( Str( idx ) );
	return aStr ? strlen( aStr ) : 0;
    }

    virtual int MinimumStorageSize () {   // >=0; -1 means it is a string-kind
	return 1;
    }

    virtual int MaximumStorageSize () {   // -1 means arbitrary size, like in strings...
#ifdef gUINT_IS_32bit
	return 10;  // ten octets for decimal representation of an unsigned int;
#else
	return 5;   // only five octets for representation of 2^16
#endif
    }

    virtual int Compare (gStorage& comp) ;

    virtual int CompareInt (int v) ;

    virtual int CompareStr (const char* aStr) {
	return 0;
    }

    virtual int CompareStrs (const char* aStr1, const char* aStr2) ;

    virtual gStorage* LRef () {
	return lRef;
    }

    virtual gStorage* ValidLRef () ;

    // Set methods

    virtual void Reset () {
	iValue = 0;
	ioMask = 0;
	lRef = nil;
    }

    int SetIoMask (int aioMask) {
	return ioMask = aioMask;
    }

    virtual void SetStoreMethod (eStoreMethod aMethod) ;

    virtual void SetLRef (gStorage* pStorage) {
	lRef = pStorage;
    }

    // Special methods
    virtual char* AllocateChars (unsigned nBytes, bool asserted=true) ;

    virtual char* DupChars (const char* s, bool asserted=true) ;

    virtual gStorage* NewObject () = 0; //PURE

    // Save/etc methods
    virtual t_uchar* ToString (const t_uchar* uBuf) = 0; //PURE

    virtual bool CanSave (FILE* f) ;
    virtual bool CanRestore (FILE* f) ;

    virtual bool EndGuts () ;
    virtual bool SaveGuts (FILE* f) = 0; //PURE
    virtual bool RestoreGuts (FILE* f) = 0; //PURE

    // Show methods
    virtual void Show (bool doShowAll=true) ;

protected:
    gStorage (eStorage aKind,
	      eStoreMethod aMethod=e_StgDefault,
	      int aioMask=0) ;

    static gUCharBuffer oneCharBuf;

    eStorage kind;
    eStoreMethod storeMethod;
    unsigned size;
    int ioMask;
    gStorage* lRef;

    bool thisSetStoreMethod (eStoreMethod aMethod,
			     eStoreMethod& resMethod) ;

private:
    static eStoreMethod globalStoreMethod;

    // Operators,empty
    gStorage (gStorage& ) ; //empty
    gStorage& operator= (gStorage& ) ; //empty
};

////////////////////////////////////////////////////////////
class gUChar : public gStorage {
public:
    gUChar (t_uchar v='\0')
	: gStorage( e_UChar ),
	  c( v ) {
    }

    virtual ~gUChar () {
    }

    // Get methods
    virtual t_uchar GetUChar () {
	return c;
    }

    virtual int MaximumStorageSize () {
	return 1;
    }

    // Set methods
    virtual void Reset () ;
    virtual bool SetUChar (t_uchar v) {
	c = v;
	return true;
    }

    virtual gStorage* NewObject () ;

    // Save/etc methods
    virtual t_uchar* ToString (const t_uchar* uBuf) ;
    virtual bool SaveGuts (FILE* f) ;
    virtual bool RestoreGuts (FILE* f) ;

protected:
    t_uchar c;

private:
    // Operators,empty
    gUChar (gUChar& ) ; //empty
    gUChar& operator= (gUChar& ) ; //empty
};

////////////////////////////////////////////////////////////
class gInt : public gStorage {
public:
    gInt (int v=0)
	: gStorage( e_SInt ) {
	iValue = v;
    }

    virtual ~gInt () {
    }

    // Get methods
    virtual int GetInt () {
	return iValue;
    }

    int GetMinInt () {
#ifdef gUINT_IS_32bit
	return (int)MIN_DLINT32;
#else
	return -32768;
#endif
    }

    int GetMaxInt () {
#ifdef gUINT_IS_32bit
	return (int)MAX_DLINT32;
#else
	return MAX_INT16_I;
#endif
    }

    // Set methods
    virtual void Reset () ;

    virtual bool SetInt (int v) {
	iValue = v;
	return true;
    }

    virtual int Decr (int v=1) {
	iValue -= v;
	return iValue;
    }

    virtual int Incr (int v=1) {
	iValue += v;
	return iValue;
    }

    virtual gStorage* NewObject () ;

    // Save/etc methods
    virtual t_uchar* ToString (const t_uchar* uBuf) ;

    virtual bool SaveGuts (FILE* f) ;
    virtual bool RestoreGuts (FILE* f) ;

    gInt& operator= (int v) {
	iValue = v;
	return *this;
    }

    // Show methods
    virtual void Show (bool doShowAll=true) ;

private:
    // Operators,empty
    gInt (gInt& ) ; //empty
    gInt& operator= (gInt& ) ; //empty
};

////////////////////////////////////////////////////////////
class gUInt : public gInt {
public:
    gUInt (unsigned v=0) ;

    virtual ~gUInt () ;

    // Get methods
    virtual unsigned GetUInt () {
	return (unsigned)iValue;
    }

    // Set methods
    virtual unsigned SetUInt (unsigned v) {
	iValue = (int)v;
	return v;
    }

    gUInt& operator= (unsigned v) {
	iValue = (int)v;
	return *this;
    }

private:
    // Operators,empty
    gUInt (gUInt& ) ; //empty
    gUInt& operator= (gUInt& ) ; //empty
};

////////////////////////////////////////////////////////////
class gReal : public gStorage {
public:
    gReal (float v)
	: gStorage( e_Real ),
	  c( (double)v ) {
    }
    gReal (unsigned v=0)
	: gStorage( e_Real ),
	  c( (double)v ) {
    }

    virtual ~gReal () {
    }

    // Get methods
    virtual double MyReal () {
	return c;
    }

    virtual int GetInt () {
	return (int)c;
    }

    virtual int MaximumStorageSize () {
	return oneCharBuf.size;
    }

    // Set methods
    virtual void Reset () ;

    virtual bool SetInt (int v) {
	c = (double)v;
	return true;
    }

    virtual double SetReal (double value) {
	c = value;
	return c;
    }

    virtual gStorage* NewObject () ;

    // Save/etc methods
    virtual t_uchar* ToString (const t_uchar* uBuf) ;

    virtual bool SaveGuts (FILE* f) ;
    virtual bool RestoreGuts (FILE* f) ;

    // Show methods
    virtual void Show (bool doShowAll=true) ;

protected:
    double c;

private:
    // Operators,empty
    gReal (gReal& ) ; //empty
    gReal& operator= (gReal& ) ; //empty
};
////////////////////////////////////////////////////////////
#endif //iSTORAGE_X_H

