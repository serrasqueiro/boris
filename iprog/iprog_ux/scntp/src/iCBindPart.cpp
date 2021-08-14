// iCBindPart.cpp

#include <string.h>
#include <unistd.h>
#ifdef iDOS_SPEC
;
#else
#include <sys/socket.h>  //socket... (sys/types.h)
#endif //~iDOS_SPEC

#include "iCBindPart.h"

////////////////////////////////////////////////////////////
// Static members
// ---------------------------------------------------------
// <>
////////////////////////////////////////////////////////////
gXGenericNetServer::gXGenericNetServer (t_gPort aPort)
    : verboseLevel( 1 ),
      handleServe( -1 ),
      bindPort( aPort )
{
}

gXGenericNetServer::~gXGenericNetServer ()
{
}

int gXGenericNetServer::thisOpenSocketTcp (t_gPort aPort, int protocol, bool isVerbose)
{
 // Protocol is usually 0: pseudo 'ip' (refer to /etc/protocols)
 int fd = socket( AF_INET, SOCK_STREAM, protocol );
 if ( fd<0 ) {
     if ( isVerbose )
	 perror("server: can't open stream socket");
     return -1;
 }
 return fd;
}

int gXGenericNetServer::thisBind (t_gPort aPort, int backlog, bool isVerbose)
{
 unsigned short serverPort( aPort );
 struct sockaddr_in serverAddressData;
 char msgErr[128];

 sprintf( msgErr, "server: unable to bind on port %u", aPort );
 memset( (char*)&serverAddressData, 0, sizeof(serverAddressData) );
 serverAddressData.sin_family = AF_INET;
 serverAddressData.sin_addr.s_addr = htonl( INADDR_ANY );
 serverAddressData.sin_port = htons( serverPort );

#ifdef iDOS_SPEC
 ;
#else
 int serverSocketOptional = 1;
 setsockopt( handleServe, SOL_SOCKET, SO_REUSEADDR, (char*)&serverSocketOptional, sizeof(int) );
#endif //~iDOS_SPEC

 if ( bind( handleServe, (struct sockaddr*)&serverAddressData, sizeof(serverAddressData) ) < 0 ) {
     listMsgs.Add( msgErr );
     if ( verboseLevel>0 ) {
	 perror( msgErr );
     }
     return -1;  // unable to bind
 }
 return listen( handleServe, backlog );
}

////////////////////////////////////////////////////////////
gAltNetServer::gAltNetServer (t_gPort aPort, int backlog)
    : gXGenericNetServer( aPort ),
      backlogOne( backlog )
{
 handleServe = thisOpenSocketTcp( aPort, 0, true );
 thisInitClientData();
}

gAltNetServer::~gAltNetServer ()
{
 if ( handleServe>=0 ) {
     shutdown( handleServe, 2 ); //close( handleServe )
     DBGPRINT("DBG: closed handleServe: %d\n",handleServe);
 }
}

gNetPair& gAltNetServer::ClientIp ()
{
 in_addr_t hostByteOrder = clientAddressData.sin_addr.s_addr;
 unsigned long nBO = htonl( hostByteOrder );
 ipClient.SetAddr( nBO );
 return ipClient;
}

void gAltNetServer::Reset ()
{
 gXGenericNetServer::Reset();
 ipClient.Reset();
}

int gAltNetServer::Accept ()
{
#ifdef FREESCO
 int addrLen = (int)clientAddressDataLength;
#else
#ifdef iDOS_SPEC
 int addrLen = clientAddressDataLength;
#else
 socklen_t addrLen = clientAddressDataLength;
#endif //iDOS_SPEC
#endif //FREESCO
 // it returns the 'handleIn', i.e. the client socket-handle:
 int socketFd = accept( handleServe, (struct sockaddr*)&clientAddressData, &addrLen );
 in_addr_t clientHBO = clientAddressData.sin_addr.s_addr;
 ipClient.SetIpFromNetworkByteOrder( clientHBO );
 ipClient.SetHandled( socketFd );
 return socketFd;
}

int gAltNetServer::BindAll ()
{
 gList bindList;
 return BindByList( bindList );
}

int gAltNetServer::BindByList (gList& bindList)
{
 if ( handleServe<0 ) return -1;
 if ( thisBindByList( bindList, bindList.N()==false, bindPort, backlogOne )==0 ) return 0;
 shutdown( handleServe,2 ); //close( handleServe )
 handleServe = -1;
 return 1;
}

int gAltNetServer::thisInitClientData ()
{
 clientAddressDataLength = sizeof(clientAddressData);
 // Initializing this data is not strictly required...
 memset( &clientAddressData, 0x0, clientAddressDataLength );
 return clientAddressDataLength;
}

int gAltNetServer::thisBindByList (gList& bindList, bool toAll, t_gPort aPort, int backlog)
{
 unsigned short serverPort = aPort;
 struct sockaddr_in serverAddressData;
 char msgErr[128];

 // This method is very similar to 'thisBind', but accepts an individual IP to bind (when toAll is false)

 sprintf( msgErr, "server: unable to bind on port %u", aPort );
 memset( (char*)&serverAddressData, 0, sizeof(serverAddressData) );

 serverAddressData.sin_family = AF_INET;
 serverAddressData.sin_port = htons( serverPort );
 serverAddressData.sin_addr.s_addr = htonl( INADDR_ANY );
 DBGPRINT("DBG: thisBindByList toAll?%c n:%u (%s[...])\n",
	  ISyORn(toAll),
	  bindList.N(),
	  bindList.Str(1));

 if ( toAll==false ) {
     gIpAddr boundIP( bindList.Str(1) );
     //serverAddressData.sin_addr.s_addr = inet_addr( "192.168.0.1" );  //=boundIP.GetNetworkAddress();
     serverAddressData.sin_addr.s_addr = boundIP.GetHostAddress();
     DBGPRINT("SERVING: '%s' (0x%08X, hostaddr:0x%08X)\n",boundIP.String(),(unsigned)boundIP.GetNetworkAddress(),serverAddressData.sin_addr.s_addr);
     if ( boundIP.GetNetworkAddress()==0 ) return 1;
 }

#ifdef iDOS_SPEC
 ;
#else
 int serverSocketOptional = 1;
 setsockopt( handleServe, SOL_SOCKET, SO_REUSEADDR, (char*)&serverSocketOptional, sizeof(int) );
#endif //~iDOS_SPEC

 if ( bind( handleServe, (struct sockaddr*)&serverAddressData, sizeof(serverAddressData) ) < 0 ) {
     perror( msgErr );
     return -1;  // unable to bind
 }

 return listen( handleServe, backlog );
}
////////////////////////////////////////////////////////////

