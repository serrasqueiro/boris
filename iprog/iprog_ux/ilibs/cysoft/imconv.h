#ifndef CS_IMCONV_X_H
#define CS_IMCONV_X_H

#include "istring.h"

#include "icontrol.h"
////////////////////////////////////////////////////////////
class gMBase52 : public gString {
public:
    gMBase52 (const char* aStr=nil) ;

    gMBase52 (gMBase52& copy) ;

    virtual ~gMBase52 () ;

    // Public data-members
    static const char digibet[];

    // Get methods
    virtual bool IsOk () ;

    virtual bool ValidateSymbols () ;

    t_int8* GetAlpha2Digibet () {
        return alpha2digibet;
    }

    // Special methods
    virtual int ToInt () ;

    virtual int FromInt (int value) ;

    virtual char* FromText () {
	return FromTextStr( (char*)str );
    }

    virtual char* FromTextStr (const char* aStr) ;

    virtual char* ToText () {
	return ToTextStr( (char*)str );
    }

    virtual char* ToTextStr (const char* aStr) ;

    virtual char* ShowAlpha () ;

    virtual char* ShowDigibet () ;

    // Operators,valid
    gMBase52& operator= (gMBase52& copy) ;

    int operator!= (gMBase52& comp) ;

protected:
    static t_int8 hashibet[ 256 ];
    static t_int8 digi2alphabet[ 256 ];
    static t_int8 alpha2digibet[ 256 ];
    static const char symbols[ 54 ];
    static const char alphaSymbols[ 54 ];

    gString sUni;

    int thisConvertBase52ToInt (const t_uchar* uStr, int length, bool ignoreErrors, int& value) ;

    int thisConvertIntToBase52 (int value, gString& sResult) ;

private:
    static const int trigitMin;

    int thisRehashibet () ;
};
////////////////////////////////////////////////////////////
#endif //CS_IMCONV_X_H

