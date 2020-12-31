#ifndef MDIR_X_H
#define MDIR_X_H


#include "entry.h"

////////////////////////////////////////////////////////////
class Media : public iEntry {
public:
    Media () ;

    Media (const char* aStr) ;

    virtual ~Media () ;

    // Set methods
    virtual void NormalizedName (const char* strName, gString& result) ;

    // Special methods
    virtual Media* DirList (const char* dName) ;

private:
    // Operators, empty
    Media (Media& ) ;
    Media& operator= (Media& ) ;
};

////////////////////////////////////////////////////////////

extern iEntry* dir_normalized (bool complain, gDir& aDir) ;

////////////////////////////////////////////////////////////
#endif //MDIR_X_H
