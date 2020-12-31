#ifndef CS_PHASH_X_H
#define CS_PHASH_X_H

#include "istring.h"

////////////////////////////////////////////////////////////
class PyStringObject : public gString {
public:
    PyStringObject (char* aStr=nil) ;

    PyStringObject (PyStringObject& copy) ;

    virtual ~PyStringObject () ;

    // Get methods
    virtual bool IsOk () ;

    virtual long ForceHash () ;

    virtual bool ValidateSymbols () ;

    // Special methods
    virtual int ToInt () ;

    long hashValue;

private:
    // Operators, empty
    PyStringObject& operator= (PyStringObject& ) ; 
};

////////////////////////////////////////////////////////////

extern long cys_str_hash (const char* str, int iLength) ;

extern long cys_string_hash (const char* str) ;

extern int cys_hash_reset (gString& s) ;

extern long cys_do_hash (gString& s) ;

////////////////////////////////////////////////////////////
#endif //CS_PHASH_X_H

