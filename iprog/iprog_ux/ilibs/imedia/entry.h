#ifndef iENTRY_X_H
#define iENTRY_X_H

#include "ibase.h"


#define str_compare(a,b) ((a) ? strcmp((a),(b)) : -2)

////////////////////////////////////////////////////////////
class iString : public gString {
public:
    iString (char* aStr=nil) ;

    iString (iString& copy) ;

    iString (unsigned nBytes, char c) ;

    virtual ~iString () ;

    // Public data-members

    gString sOrigin;

    // Get methods

    virtual int GetOrder () {
	return equalFactor;
    }

    virtual int CompareStr (char* aStr) {
	if ( str!=nil && str[0]!=0 ) {
	    if ( aStr ) {
		return strcmp( (char*)str, aStr ) * equalFactor;
	    }
	    // The compared string is nil, therefore aStr is lesser than str:
	    return -1 * equalFactor;
	}
	if ( aStr!=nil && aStr[0]!=0 ) {
	    // The compared string is not-nil, therefore aStr is greater than str:
	    return 1 * equalFactor;
	}
	// Both str and the compared string are nil or empty: therefore equal:
	return 0;
    }

    // Set methods

    virtual bool SetOrder (int normalIsOne=1) {
	// Return true on success
	equalFactor = normalIsOne;
	return equalFactor==-1 || equalFactor==1;
    }

protected:
    int equalFactor;

private:
    // Operators, empty
    iString& operator= (iString& ) ;
};

////////////////////////////////////////////////////////////
class iEntry : public gList {
public:
    iEntry (gElem* pElem=nil) ;
    iEntry (const char* aStr) ;

    virtual ~iEntry () ;

    // Public data-members

    int lastOpError;
    gString sComment;
    gString display;  // e.g. "#EXTINF:123,Artist - Title\r\nK:\pathDir\name.mp3\r\n"

    // Set methods

    virtual int SetComment (const char* strComment=nil) ;

    virtual int AddAt (gElem* pElem) {
	return thisCopyFromElement( pElem );
    }

protected:
    int thisAdd (char* aStr, int value=0) ;

    int thisCopyFromElement (gElem* pElem) ;

private:
    // Operators, empty
    iEntry (iEntry& ) ;
    iEntry& operator= (iEntry& ) ;
};

////////////////////////////////////////////////////////////
#endif //iENTRY_X_H

