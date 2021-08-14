#ifndef G_CBINDPART_X_H
#define G_CBINDPART_X_H

#include "inet.h"

#include "iNetPair.h"

////////////////////////////////////////////////////////////
class gXGenericNetServer : public gInt {
public:
    // Public data-members
    int verboseLevel;

    virtual ~gXGenericNetServer () ;

    // Get methods
    virtual int Handle () {
	return handleServe;
    }

    virtual t_gPort BindPort () {
	return bindPort;
    }

    virtual bool IsOk () {
	return handleServe>=0 && bindPort>0;
    }

    virtual gList& Messages () {
	return listMsgs;
    }

protected:
    gXGenericNetServer (t_gPort aPort) ;
    int handleServe;
    t_gPort bindPort;
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
    virtual gNetPair& ClientIp () ;

    // Set methods
    virtual void Reset () ;

    int Accept () ;

    virtual int BindAll () ;
    virtual int BindByList (gList& bindList) ;

protected:
    struct sockaddr_in clientAddressData;
    int clientAddressDataLength;
    int backlogOne;
    gNetPair ipClient;

    int thisBindByList (gList& bindList, bool toAll, t_gPort aPort, int backlog) ;

private:
    int thisInitClientData () ;

    // Operators,empty
    gAltNetServer (gAltNetServer& ) ; //empty
    gAltNetServer& operator= (gAltNetServer& ) ; //empty
};

////////////////////////////////////////////////////////////
#endif //G_CBINDPART_X_H

