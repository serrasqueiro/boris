#ifndef iCIP_X_H
#define iCIP_X_H

#include "inet.h"

////////////////////////////////////////////////////////////
class gCNetSource : public gIpAddr {
public:
    gCNetSource (bool hasSearch=false) ;
    virtual ~gCNetSource () ;

    // Public data-members
    int handleIn;
    t_int16 sourceGuessId;  // -1: unknown; -2: denied; 0: root
    gString sourceGuessName;

    // Get methods
    virtual bool IsOk () {
	return
	    handleIn>=0 &&
	    GetNetworkAddress()!=0;
    }

    virtual t_uchar* String () ;

    virtual char* BestString () {
	return sBestString.Str();
    }

    // Set methods
    virtual bool Close () ;

    // Specific methods
    virtual int BestGuessIdFromIP (gIpAddr& ip) ;

private:
    gString sBestString;

    // Operators,empty
    gCNetSource (gCNetSource& ) ; //empty
    gCNetSource& operator= (gCNetSource& ) ; //empty
};
////////////////////////////////////////////////////////////
#endif //iCIP_X_H

