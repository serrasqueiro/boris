#ifndef iUNICODE_X_H
#define iUNICODE_X_H

#include "icontrol.h"
#include "ilist.h"
////////////////////////////////////////////////////////////
// Unicode defs
// ---------------------------------------------------------
typedef t_uint16 t_unicode;

typedef t_int32 t_unitable;

#define UNI_HASH_MINUS1(c)	((c==-1) ? '#' : c)

////////////////////////////////////////////////////////////
// Basic Unicode element
// ---------------------------------------------------------
////////////////////////////////////////////////////////////
struct sUniElement {
    t_unicode code;
    char* strDescription;
    char twoLetter[ 3 ];
    t_int8 zero;
    char type;
    char* strCompat;
    char* a1;
    char* a2;
    char* a3;
    char* strNormal;  // 'N' for Normal charset
    char* strAltDescription;
    char* strNote;
    t_unicode upperEq;
    t_unicode lowerEq;
    t_unicode upperAlt;

    sUniElement (t_unicode aCode=0)
	: code( aCode ),
	  strDescription( nil ),
	  zero( 0 ),
	  type( 'L' ),
	  strCompat( nil ),
	  a1( nil ),
	  a2( nil ),
	  a3( nil ),
	  strNormal( nil ),
	  strAltDescription( nil ),
	  strNote( nil ),
	  upperEq( 0 ),
	  lowerEq( 0 ),
	  upperAlt( 0 ) {
	twoLetter[ 0 ] = twoLetter[ 1 ] = twoLetter[ 2 ] = 0;
    }

    ~sUniElement () {
	Release();
    }

    void Release () {
	delete[] strDescription;
	delete[] strCompat;
	delete[] a1;
	delete[] a2;
	delete[] a3;
	delete[] strNormal;
	delete[] strAltDescription;
	delete[] strNote;
    }
};
////////////////////////////////////////////////////////////
// Functions to convert Unicode elements, etc.
// ---------------------------------------------------------

////////////////////////////////////////////////////////////
class gUniCode : public gControl {
public:
    gUniCode (const char* strTableName=nil, t_unitable iTable=0) ;

    virtual ~gUniCode () ;

    // Public enums
    enum eHashBasic {
	e_Basic_Alpha26 = 0,  // A-Z and a-z
	e_Basic_Alpha = 1,  // A-Z, a-z, and also special accented chars (e.g. "A'" will be marked as 'u' (uppercase)
	e_Basic_DigiAlpha = 2,  // A-Z, a-z, special accented chars and digits
	e_Basic_Printable = 3,
	e_Basic_Hash
    };

    // Public data data-members
    static const t_unitable defaultUniTable;
    static const char* defaultUniLettersISO8859_1;

    gString tableName;
    gList uniListed;  // The semi-colon separated list

    gList hash256BasicSkipInfo[ e_Basic_Hash ];

    char hash256Basic[ e_Basic_Hash ][ 256 ];
    t_uint8 hash256User[ e_Basic_Hash ][ 256 ];

    // Get methods
    virtual t_unitable UniTable () {
	return uniTable;
    }

    // Set methods
    virtual void Reset () ;

    virtual int Build () ;

    virtual int BuildFromList (gList& uniTableStrings, t_unitable iTable) ;

protected:
    t_unitable uniTable;
    int defaultAllocationMask;
    sUniElement* ptrUniElems;

    int thisListFromStr (const char* strLines, gList& uniList) ;

    int thisUniElementFromStr (const char* aStr, int allocateMask, sUniElement& uniElem) ;

private:
    // Operators,empty
    gUniCode (gUniCode& ) ; //empty
    gUniCode& operator= (gUniCode& ) ; //empty
};
////////////////////////////////////////////////////////////
#endif //iUNICODE_X_H

