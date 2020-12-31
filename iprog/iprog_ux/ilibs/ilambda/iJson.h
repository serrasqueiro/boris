#ifndef IJSON_X_H
#define IJSON_X_H

#include "ilist.h"

////////////////////////////////////////////////////////////
class iJSON : public gList {
public:
    iJSON (char* strName=nil) ;

    virtual ~iJSON () ;

    enum eState {
	e_Idle,
	e_Start,
	e_Finit
    };

    // Set methods
    virtual void Reset () ;

    virtual void Start () ;

    virtual int AddChar (t_uchar aChr) ;

    virtual int AddLine (const char* aStr) ;

    virtual int AddError (int aErrNo, const char* strMsg=nil) ;
    virtual int AddWarn (int aErrNo, const char* strMsg=nil) ;

    // Special methods
    virtual int Flush () ;

    // Public data-members
    gList hints;

private:
    int state;
    eState finito;
    int y, x;
    t_uchar lastChr;

    gString sKey;
    gString sValue;

    // Operators,empty
    iJSON (iJSON& copy) ;  //empty
    iJSON& operator= (iJSON& ) ;  //empty
};

////////////////////////////////////////////////////////////

extern iJSON* ijson_get_from_handle (int fd) ;

////////////////////////////////////////////////////////////
#endif //IJSON_X_H

