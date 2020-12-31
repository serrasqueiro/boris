#ifndef SET_X_H
#define SET_X_H

#include "lib_iobjs.h"


#define I_NUM_INVALID MIN_INT32BITS	// e.g. for non-numerical list cells

////////////////////////////////////////////////////////////
class iSet : public gList {
public:
    // Public enums
    enum eSetOrder {
	e_none,
	e_ascending = 1,
	e_descending = 2,
	e_invalid
    };

    // Constructors, etc.

    iSet () ;
    iSet (const char* strInit) ;
    iSet (iSet& copy) ;

    virtual ~iSet () ;

    // Get methods

    virtual eSetOrder Order () {
	return setOrder;
    }

    virtual bool AllowedRepeats () {
	return repeatedValues;
    }

    // Set methods

    gList& operator= (iSet& copy) ;
    iSet& operator= (const char* strInit) ;

    virtual bool SetOrder (eSetOrder requestedOrder=e_ascending) {
	setOrder = requestedOrder;
	return setOrder==e_ascending;
    }

    virtual void AllowRepeated (bool allowRepeat=true) {
	repeatedValues = allowRepeat;
    }

    // Redefined methods
    unsigned Add (int v) ;

    unsigned Add (unsigned v) ;

    unsigned Add (gString& copy) ;

    unsigned Add (const char* s) ;

    unsigned Add (t_uchar* s) ;

    virtual gList& AddFrom (gElem* pElem, bool doUpChar=false) ;
    virtual gList& CopyFrom (gElem* pFrom, bool doUpChar=false) ;

    virtual iSet* NewOrderedSet () {
	lastOpError = 0;
	return thisNewOrderedSet( *this, StartPtr(), setOrder );
    }

    // Public data-members
    int lastOpError;

protected:
    bool thisAddOne (iSet& pList, gStorage* cell, bool ordered=true) ;

    int thisAddFromStr (const char* strInit) ;
    int thisAddFromStrAs (const char* strInit, const char* separator, bool ordered=true) ;

    int thisAddAt (gElem* ptr, bool ordered=true) ;

    // Interesting methods
    gString* thisNewStrFromNumber (int v, const char* strFormat) ;

private:
    static eSetOrder defaultOrder;
    eSetOrder setOrder;
    bool repeatedValues;

    virtual iSet* thisNewOrderedSet (iSet& copy, gElem* pStart, eSetOrder byOrder) ;
};

////////////////////////////////////////////////////////////

#endif //SET_X_H

