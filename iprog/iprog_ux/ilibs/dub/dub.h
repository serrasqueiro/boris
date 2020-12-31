#ifndef DUB_X_H
#define DUB_X_H


#include "idir.h"

////////////////////////////////////////////////////////////
struct sTriplet {
    sTriplet (t_uint32 changed=0)
	: stampA( 0 ),
	  stampM( 0 ),
	  stampC( changed ) {
    }
    t_uint32 stampA;  // access
    t_uint32 stampM;  // modify
    t_uint32 stampC;  // change
};

////////////////////////////////////////////////////////////
class mDir : public gDirGeneric {
public:
    mDir (char* dName=nil, bool ordered=true) ;

    mDir (gString dirName, bool ordered=true) ;

    virtual ~mDir () ;

    // Public data-members
    static const t_uint32 defaultSortMask;
    t_uint32 sortMask;
    gList byName;

    // Get methods
    virtual sTriplet& Triplet (int idx) {
	if ( timestamps && idx>=1 && idx<=(int)N() ) {
	    return timestamps[ idx ];
	}
	return emptyTriplet;
    }

    // Set methods
    virtual void AddDir (char* s) ;
    virtual void AddFile (char* s) ;

    virtual int GetDir (const char* strPath, bool ordered, const char* strGlobPositive, const char* strGlobNegative) ;

    // Special methods
    virtual void SortByName () {
	thisSortFromTo( *this, sortMask, byName );
    }

protected:
    DIR* apDir;
    sTriplet emptyTriplet;
    gString* names;
    sTriplet* timestamps;

    void thisSortFromTo (gList& from, t_uint32 mask, gList& to) ;

private:
    // Operators,empty
    mDir (mDir& ) ;
    mDir& operator= (mDir& ) ;

    void thisCleanUp () {
	lastOpError = 0;
	Delete();
	delete[] names;
	names = nil;
	delete[] timestamps;
	timestamps = nil;
    }
};

////////////////////////////////////////////////////////////


#endif //DUB_X_H

