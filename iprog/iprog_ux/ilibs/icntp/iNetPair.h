#ifndef iNETPAIR_X_H
#define iNETPAIR_X_H

#include "inet.h"

#ifdef iDOS_SPEC
#include <windows.h>
#endif //iDOS_SPEC

////////////////////////////////////////////////////////////
class gNetPair : public gIpAddr {
public:
    gNetPair () ;
    virtual ~gNetPair () ;

    // Get methods
    virtual bool IsHandledOk () {
	return handled>0;
    }

    virtual int Handled () {
	return handled;
    }

    virtual int LastIOValue () {
	return lastIOvalue;
    }

    // IO methods
    virtual int SocketRead (void *buf, size_t count) ;

    virtual int SocketWrite (void *buf, size_t count) ;

    // Set methods
    virtual void Reset () ;

    virtual int SetHandled (int aHandle) ;

    virtual int SetFlags (int flags) ;

protected:
    int handled;
    int ioFlags;  // Used for recv / send

    int thisCloseHandled () ;

    int thisShutDownSocket (int& fd) ;

    int thisSocketRead (int fd, void *buf, size_t count, int flags) ;

    int thisSocketWrite (int fd, void *buf, size_t count, int flags) ;

private:
    int lastIOvalue;

    // Operators,empty
    gNetPair (gNetPair& ) ; //empty
    gNetPair& operator= (gNetPair& ) ; //empty
};
////////////////////////////////////////////////////////////
#endif //iNETPAIR_X_H

