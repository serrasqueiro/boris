#ifndef ILAMBDA_NETPAIR_X_H
#define ILAMBDA_NETPAIR_X_H

#include "inet.h"

#ifdef iDOS_SPEC
#include <windows.h>

typedef unsigned int __socklen_t;
typedef __socklen_t socklen_t;
#endif //iDOS_SPEC


#ifndef UNIVERSAL_APPEND
#ifdef iDOS_SPEC
#define UNIVERSAL_APPEND (O_WRONLY | O_APPEND)
#else
#define UNIVERSAL_APPEND (O_WRONLY | O_APPEND)
#endif
#endif


////////////////////////////////////////////////////////////
class iNetPair : public gIpAddr {
public:
    iNetPair () ;
    virtual ~iNetPair () ;

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
    iNetPair (iNetPair& ) ; //empty
    iNetPair& operator= (iNetPair& ) ; //empty
};

////////////////////////////////////////////////////////////
class gXGenericNetServer : public gList {
public:
    // Public data-members
    int verboseLevel;

    virtual ~gXGenericNetServer () ;

    // Get methods
    virtual int Handle (int idx=0) {
	if ( idx<=0 ) return handleServe;
	return idx < 8 ? handles[ idx ].GetInt() : -1;
    }

    virtual char* StrIP (int idx=0) {
	if ( idx<=0 ) return nil;
	return Str( idx );
    }

    virtual t_gPort BindPort (int idx=0) {
	if ( idx<=0 ) return bindPort;
	return GetInt( idx );
    }

    virtual bool IsOk () {
	return handleServe!=-1 && bindPort>0;
    }

    virtual gList& Messages () {
	return listMsgs;
    }

protected:
    gXGenericNetServer (t_gPort aPort) ;
    int handleServe;
    t_gPort bindPort;
    gInt handles[ 8 ];
    gList listMsgs;

    int thisOpenSocketTcp (t_gPort aPort, int protocol, bool isVerbose) ;

    int thisBind (t_gPort aPort, int backlog, bool isVerbose) ;

private:
    // Operators,empty
    gXGenericNetServer (gXGenericNetServer& ) ; //empty
    gXGenericNetServer& operator= (gXGenericNetServer& ) ; //empty
};

////////////////////////////////////////////////////////////
class gAltNetServer : public gXGenericNetServer {
public:
    gAltNetServer (t_gPort aPort, int backlog=1) ;

    virtual ~gAltNetServer () ;

    // Get methods
    virtual iNetPair& ClientIp () ;

    // Set methods
    virtual void Reset () ;

    int Accept () ;

    virtual int BindAll () ;

    virtual int BindByList (gList& bindList) ;

protected:
    struct sockaddr_in clientAddressData;
    int clientAddressDataLength;
    int backlogOne;
    iNetPair ipClient;

    int thisBindByList (gList& bindList, bool toAll, t_gPort aPort, int backlog) ;

private:
    int thisInitClientData () ;

    // Operators,empty
    gAltNetServer (gAltNetServer& ) ; //empty
    gAltNetServer& operator= (gAltNetServer& ) ; //empty
};

////////////////////////////////////////////////////////////
class Addressee : public gList {
public:
    Addressee () ;
    Addressee (char* str) ;
    Addressee (gString& s) ;
    Addressee (Addressee& copy) ;

    virtual ~Addressee () ;

    // Public data-members
    t_uint32 address;
    gString display;

    // Special methods
    virtual gString& Printable (int maxLen=60) ;

    // Operators
    Addressee& operator= (Addressee& copy) ;

protected:
    int thisCopyPrintable (gString& sIn, gString& sOut, int maxLen) ;
};

////////////////////////////////////////////////////////////

extern int safe_create (const char* strPath, int await) ;
extern int safe_append (const char* strPath, int await) ;

////////////////////////////////////////////////////////////
#endif //ILAMBDA_NETPAIR_X_H

