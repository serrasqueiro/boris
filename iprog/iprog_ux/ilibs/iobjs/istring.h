#ifndef iSTRING_X_H
#define iSTRING_X_H

#include <string.h>

#include "istorage.h"

class gList;
////////////////////////////////////////////////////////////
enum eBasicCaseSense {
    e_BasicDoCaseSense = 0,
    e_BasicDoIgnoreCase = 1
};


enum eDigitConv {
    e_DigConvLower,
    e_DigConvUpper,
    e_DigConvAny,
    e_DigConvAnyEmpty0
};

////////////////////////////////////////////////////////////
class gStringGeneric : public gStorage {
public:
    virtual ~gStringGeneric () ;

    // Get methods
    virtual bool IsString () {
	return true;
    }

    virtual bool IsEmpty () {
	return size==0;
    }

    virtual unsigned Length () {
	return size;
    }

    virtual t_uchar& CharAtIndex (int index) ;

    virtual bool IsDigConvRelaxed () {
	return doDigConvertRelaxed;
    }

    virtual t_uchar GetUChar (unsigned idx) ;

    virtual char GetChar (unsigned idx) {
	return (char)GetUChar(idx);
    }

    virtual t_uchar LastUChar () {
	return CharAtIndex( (int)size );
    }

    virtual char LastChar () {
	return (char)LastUChar();
    }

    virtual char* Str (unsigned idx=0) ;

    virtual t_uchar* UStr () ;

    virtual t_uchar* StrPlus (int index) ;

    virtual int MinimumStorageSize () {
	return -1;
    }

    virtual int MaximumStorageSize () {
	return -1;
    }

    virtual unsigned CountChars (t_uchar uChr, eBasicCaseSense senseKind=e_BasicDoCaseSense) ;

    virtual int Hash () {
	// gStorage::Hash does not use 'size'; it's the same.
	if ( iValue==-1 || iValue==0 ) {
	    iValue = StringHash( (const char*)str, size );
	}
	return iValue;
    }

    virtual int CompareStr (const char* aStr) ;

    // Set methods
    virtual void Reset () ;
    virtual void SetEmpty () ;

    void Set (const char* s) ;
    void Set (char* s) ;
    void Set (t_uchar* s) ;
    void Set (int v) ;
    void Set (unsigned v) ;

    unsigned Add (char c) ;
    unsigned Add (t_uchar c) ;
    unsigned Add (const char* s) ;
    unsigned Add (t_uchar* s) ;
    unsigned Add (int v) ;
    unsigned Add (unsigned v) ;

    virtual void UpString () ;
    virtual void DownString () ;

    void SetConvertRelax (bool doRelax) {
	doDigConvertRelaxed = doRelax;
    }

    // Operators
    t_uchar& operator[] (int index) {
	return CharAtIndex( index );
    }

    t_uchar* operator+ (int index) {
	return StrPlus( index );
    }

    // Save/etc methods
    virtual t_uchar* ToString (const t_uchar* uBuf) ;
    virtual bool SaveGuts (FILE* f) ;
    virtual bool RestoreGuts (FILE* f) ;

protected:
    gStringGeneric (eStorage kind, const t_uchar* s) ;

    unsigned size;
    t_uchar* str;
    static t_uchar myChrNul;
    static bool doDigConvertRelaxed;

    // Protected methods
    void thisPreAllocate (const char* s) ;

    void thisDelete () ;
    int thisCopy (const char* s, unsigned len) ;

    bool thisIndex (unsigned& idx) ;
    bool thisIsValidIndex (int index) ;

    unsigned thisEvalLength () {
	if ( str ) {
	    return size = (unsigned)strlen( (char*)str );
	}
	size = 0;
	return 0;
    }

private:
    // Operators,empty
    gStringGeneric (gStringGeneric& ) ;
    gStringGeneric& operator= (gStringGeneric& ) ;
};
////////////////////////////////////////////////////////////
class gString : public gStringGeneric {
public:
    gString () ;
    gString (gString& copy) ;
    gString (const char* s) ;
    gString (char* s) ;
    gString (t_uchar* s) ;
    gString (char c) ;
    gString (unsigned nBytes, char c) ;

    virtual ~gString () ;

    // Get methods
    bool Match (gString& copy, bool doIgnoreCase=false) ;
    bool Match (const char* s, bool doIgnoreCase=false) ;

    unsigned Find (gString& sSub, bool doIgnoreCase=false) ;
    unsigned Find (const char* s, bool doIgnoreCase=false) ;
    unsigned Find (char c, bool doIgnoreCase=false) ;

    unsigned Find (gString& sSub, unsigned& nOcc, bool doIgnoreCase=false) ;
    unsigned Find (const char* s, unsigned& nOcc, bool doIgnoreCase=false) ;
    unsigned Find (char c, unsigned& nOcc, bool doIgnoreCase=false) ;

    unsigned FindBack (gString& sSub, bool doIgnoreCase=false) ;
    unsigned FindBack (const char* s, bool doIgnoreCase=false) ;
    unsigned FindBack (char c, bool doIgnoreCase=false) ;

    unsigned FindAnyChr (gString& b, bool doIgnoreCase=false) ;
    unsigned FindAnyChr (const char* s, bool doIgnoreCase=false) ;
    unsigned FindAnyChr (gString& b, bool doIgnoreCase, unsigned& posAny) ;
    unsigned FindAnyChr (const char* s, bool doIgnoreCase, unsigned& posAny) ;

    unsigned FindExcept (gString& sExcept, bool doIgnoreCase=false) ;
    unsigned FindExcept (const char* s, bool doIgnoreCase=false) ;

    virtual unsigned FindChrInStr (const char* aStr, char subChr, int offset=0) {
        char buf[ 2 ];
        buf[ 0 ] = subChr;
        buf[ 1 ] = 0;
        return FindInStr( aStr, buf, offset );
    }

    virtual unsigned FindInStr (const char* aStr, const char* subStr, int offset=0) ;

    t_uint32 ConvertToUInt32 (const char* s) {
	t_uint32 result( 0 );
	ConvertToUInt32( s, result );
	return result;
    }

    int ConvertToUInt32 (const char* s,
			 t_uint32& vRes) ;

    t_uint32 ConvertToUInt32 (t_uint32 defaultValue=0) {
	ConvertToUInt32( Str(), defaultValue );
	return defaultValue;
    }

    t_int32 ConvertToInt32 (t_int32 defaultValue=0) {
	ConvertToInt32( Str(), defaultValue );
	return defaultValue;
    }

    int ConvertToInt32 (const char* s,
			t_int32& vRes) ;

    t_int32 ConvertToInt32 (const char* s) {
	t_int32 result( 0 );
	ConvertToInt32( s, result );
	return result;
    }

    int ConvertToUInt32 (const char* s,
			 unsigned base,
			 eDigitConv caseSense,
			 t_uint32& vRes,
			 unsigned& posErr) ;

    int ConvertToInt32 (const char* s,
			unsigned base,
			eDigitConv caseSense,
			t_int32& vRes,
			unsigned& posErr) ;

    int ConvertHexToUInt32 (const char* s,
			    eDigitConv caseSense,
			    t_uint32& vRes) {
	unsigned posErr;
	return ConvertToUInt32( s, 16, caseSense, vRes, posErr );
    }

    t_uint32 ConvertHexToUInt32 (const char* s, t_uint32 defaultValue=0) {
	ConvertHexToUInt32( s, e_DigConvAny, defaultValue );
	return defaultValue;
    }

    int ConvertBinToValue (const char* strBinValue, t_uint32& result) ;

    t_uint32 ConvertBinToValue (const char* strBinValue) {
	t_uint32 result( 0 );
	ConvertBinToValue( strBinValue, result );
	return result;
    }

    void ConvertBinToStr (t_uint32 v,
			  t_int16 places,  // <=0 for no justify
			  gString& sRes) ;

    void ConvertBinToStr (t_uint32 v,
			  gString& sRes) {
	ConvertBinToStr( v, 0, sRes );
    }

    void ConvertBinToStr (t_uint32 v, t_int16 places=0) {
	ConvertBinToStr( v, places, *this );
    }

    int ReturnAndAssignUInt32 (int returnValue,
			       t_uint64 value,
			       t_uint32& vRes) ;

    // Set methods
    virtual unsigned AddString (gString& a) ;

    virtual void Copy (gString& copy) ;

    virtual gString& CopyFromTo (gString& copy, unsigned startPos=0, unsigned endPos=0) ;
    virtual gString& CopyFromToStr (const char* s, unsigned startPos=0, unsigned endPos=0) {
	gString copy( (char*)s );
	return CopyFromTo( copy, startPos, endPos );
    }

    virtual unsigned Delete (unsigned startPos=0, unsigned endPos=0) ;

    virtual unsigned Insert (gString& copy, unsigned startPos=0) ;

    virtual unsigned InsertStr (const char* s, unsigned startPos=0) {
	gString copy( (char*)s );
	return Insert( copy, startPos );
    }

    virtual bool Trim () ;
    virtual bool TrimLeft () ;
    virtual bool TrimRight () ;

    virtual unsigned ConvertChrTo (char fromChr, char toChr) ;

    virtual unsigned ConvertAnyChrTo (const char* aStr, char toChr) ;

    virtual unsigned ConvertAnyChrsTo (gString& s, char toChr) ;

    int SplitAnyChar (char anyChr, gList& resultL) ;
    int SplitAnyChar (const char* anyChrStr, gList& resultL) ;

    virtual char* BaseName (const char* strOptSuffix=nil) ;

    virtual char* DirName () ;

    // Operators
    gString& operator= (gString& copy) ;
    gString& operator= (char* s) ;
    gString& operator= (char c) ;
    gString& operator= (int v) ;

    gString& operator+ (gString& copy) ;

    gString& operator+= (gString& copy) ;

    // Save/etc methods
    virtual gStorage* NewObject () ;
    virtual t_uchar* ToString (const t_uchar* uBuf) ;
    virtual bool SaveGuts (FILE* f) ;
    virtual bool RestoreGuts (FILE* f) ;

    // Show methods
    virtual void Show (bool doShowAll=true) ;

protected:
    // Other methods
    int thisMatch (const char* s1,
		   const char* s2,
		   bool doIgnoreCase) ;

    unsigned thisFind (const char* s,
		       const char* sub,
		       unsigned startPos,
		       bool doIgnoreCase) ;

    unsigned thisFindAny (const char* s,
			  const char* strAny,
			  bool doIgnoreCase,
			  unsigned& posAny) ;

    unsigned thisFindFwd (const char* s,
			  const char* sub,
			  unsigned startPos,
			  bool doIgnoreCase,
			  unsigned& nOcc) ;

    unsigned thisFindFwdOcc (const char* s,
			     const char* sub,
			     unsigned& nOcc) ;

    unsigned thisFindBack (const char* s,
			   const char* sub,
			   unsigned startPos,
			   bool doIgnoreCase,
			   unsigned& nOcc) ;

    unsigned thisFindBackOcc (const char* s,
			      const char* sub,
			      unsigned& nOcc) ;

    unsigned thisFindExcept (const char* s,
			     const char* exceptStr,
			     unsigned startPos,
			     bool doIgnoreCase) ;

    int thisSplitAnyChar (gString& sSepStr,
			  eBasicCaseSense senseKind,
			  bool doStopOnFirst,
			  bool doIncludeChar,
			  gList& resultL);

private:
    // Operators,empty
    gString (gStringGeneric& ) ; //empty
};

////////////////////////////////////////////////////////////
#endif //iSTRING_X_H

