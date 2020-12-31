#ifndef iCUE_X_H
#define iCUE_X_H

#include "entry.h"

////////////////////////////////////////////////////////////
class iCue : public iEntry {
public:
    iCue (gElem* pElem=nil) ;
    iCue (const char* aStr) ;

    virtual ~iCue () ;

    // Public data-members
    gString sDiscID;
    gString sPerformer;
    gString sTitle;
    gString sFile;

    gList ignored;

    // Set methods
    virtual int Parse (gList& lines) {
	return thisParseCue( lines );
    }

    // Special methods
    virtual gList* NewLine (gString& sLine) ;

protected:
    int thisParseCue (gList& lines) ;

    int thisAddIgnored (gString& sLine, int line=0) ;

private:
    // Operators, empty
    iCue (iCue& ) ;
    iCue& operator= (iCue& ) ;
};

////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
#endif //iCUE_X_H

