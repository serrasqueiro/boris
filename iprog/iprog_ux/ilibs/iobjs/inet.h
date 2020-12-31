#ifndef iNET_X_H
#define iNET_X_H

#include "istring.h"
#include "ilist.h"
#include "icontrol.h"

#ifdef linux
#include <netinet/in.h>  //htonl...etc
#else
typedef t_uint32 in_addr_t;
#endif //linux

#define STG_SVC_IDX_SOCK 1
#define STG_SVC_KND_SOCK 128
////////////////////////////////////////////////////////////
// Generic definitions
////////////////////////////////////////////////////////////
typedef t_uint64 t_gIpAddr;  // Least significant nibble is b4

////////////////////////////////////////////////////////////
// inet classes
////////////////////////////////////////////////////////////
class gIpAddr : public gControl {
public:
    gIpAddr (const char* ipStr=nil) ;
    gIpAddr (t_uint8 c1, t_uint8 c2, t_uint8 c3, t_uint8 c4) ;

    virtual ~gIpAddr () ;

    // Public data-members (due to performance)
    t_uint8 b1, b2, b3, b4;

    // Get methods
    virtual bool IsOk () ;

    virtual t_uchar* String () ;

    virtual t_uchar* ToString (const t_uchar* uBuf) ;

    t_uint8 GetB1 () {
	return b1;
    }
    t_uint8 GetB2 () {
	return b2;
    }
    t_uint8 GetB3 () {
	return b3;
    }
    t_uint8 GetB4 () {
	return b4;
    }

    // Set methods
    virtual void Reset () ;

    virtual void SetAddr (t_gIpAddr nboAddr) ;

    virtual int SetIpFromNetworkByteOrder (in_addr_t nboAddr) {
	SetAddr( (t_gIpAddr)nboAddr );
	return nboAddr!=0;
    }

    virtual bool SetAddrFromStr (const char* str) ;

    // Specific methods
    virtual t_gIpAddr GetNetworkAddress () ;

    virtual in_addr_t GetHostAddress () {
	return thisGetHostByteOrder( GetNetworkAddress() );
    }

    virtual int GetHostByName (const char* hostname) ;

    virtual int GetHostByAddr (gString& sRes) ;

    // Operators,valid
    gIpAddr& operator= (gIpAddr& copy) ;

    // Show methods
    virtual void Show (bool doShowAll=true) ;

protected:
    unsigned long thisGetIP (t_uint8 a1, t_uint8 a2, t_uint8 a3, t_uint8 a4) ;

    unsigned long thisGetHostByteOrder (t_gIpAddr netByteOrder) ;

    int thisSetIPfromHostByteOrder (in_addr_t hostByteOrder) ;

    int thisSetIPfromHostEnt (const char* hAddr, int h_length) ;

    int thisGetHostByAddr (const char* ipStr, gString& sRes, gList& lRes) ;

private:
    gString addrStr;

    // Operators,empty
    gIpAddr (gIpAddr& ) ; //empty
};

////////////////////////////////////////////////////////////
class gHostAddr : public gIpAddr {
public:
    gHostAddr (const char* hostStr) ;
    gHostAddr (t_uint8 c1, t_uint8 c2, t_uint8 c3, t_uint8 c4) ;
    gHostAddr (gIpAddr& ipAddr) ;

    virtual ~gHostAddr () ;

    // Get methods
    virtual bool IsOk () ;
    virtual t_uchar* String () ;
    virtual char* GetHostName () {
	return destHostStr.Str();
    }

    // Set methods
    virtual bool SetHostName (const char* hostStr) ;

    // Show methods
    virtual void Show (bool doShowAll=true) ;

protected:
    gString destHostStr;

private:
    // Operators,empty
    gHostAddr (gHostAddr& ) ; //empty
    gHostAddr& operator= (gHostAddr& ) ;
};

////////////////////////////////////////////////////////////
class gSocket : public gInt {
public:
    // Public enums
    enum eSocketKind {
	e_None,
	e_TCP = 1,
	e_UDP = 2,
	e_other
    };

    gSocket (int socketHandle, eSocketKind socketKind) ;

    virtual ~gSocket () ;

    // Get methods
    virtual bool IsOk () ;
    virtual bool IsOpened () ;
    virtual bool IsConnected () ;
    virtual int Handle () ;

    // Set methods
    virtual bool Open (const t_uchar* uName) ;
    virtual bool Close () ;
    bool SetConnection (gIpAddr& ipAddr) ;
    bool SetConnection (gHostAddr& hostAddr) ;

protected:
    eSocketKind sockKind;
    static int sockCount;

private:
    gHostAddr* hostConnected;

    // Operators,empty
    gSocket (gSocket& ) ; //empty
    gSocket& operator= (gSocket& ) ; //empty
};

////////////////////////////////////////////////////////////
class gNetConnect : public gControl {
public:
    virtual ~gNetConnect () ;

    // Get methods
    virtual int Handle () = 0; //PURE

    virtual gSocket& Socket () = 0;  //PURE

    // I/O methods
    bool Read (t_uchar& c) ;
    bool Read (t_uchar* uBuf, unsigned nBytes) ;

    virtual bool ReadLine (gString& s) ;

    int Write (const char* s) ;
    int Write (gString& s) ;
    int Write (const char* s, unsigned nOctets) ;

protected:
    gNetConnect () ;

private:
    // Operators,etc
    gNetConnect (gNetConnect& ) ; //empty
    gNetConnect& operator= (gNetConnect& ) ; //empty
};

////////////////////////////////////////////////////////////
class gTcpConnect : public gNetConnect {
public:
    gTcpConnect (const char* destHostname, t_gPort cPort, bool doConnect=true) ;
    gTcpConnect (t_uint8 c1, t_uint8 c2, t_uint8 c3, t_uint8 c4, t_gPort cPort, bool doConnect=true) ;

    virtual ~gTcpConnect () ;

    virtual gSocket& Socket () {
	return skt;
    }

    // Public data-members
    gHostAddr destAddr;
    t_gPort destPort;
    gSocket skt;

    // Get methods
    virtual bool IsOk () ;
    virtual int Handle () ;
    virtual t_uchar* String () ;
    virtual t_uchar* ToString (const t_uchar* uBuf) ;

    // Set methods
    bool Connect () ;
    virtual bool Close () ;

    // Show methods
    virtual void Show (bool doShowAll=true) ;

protected:
    int thisConnect (gSocket& aSocket) ;

private:
    // Operators,empty
    gTcpConnect (gTcpConnect& ) ; //empty
    gTcpConnect& operator= (gTcpConnect& ) ; //empty
};
////////////////////////////////////////////////////////////
#endif //iNET_X_H

