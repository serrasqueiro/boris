// iLambdaNetPair.cpp


#include <string.h>
#include <unistd.h>  // needed due to Win32 port: 'close'/'recv/send' functions
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#ifdef iDOS_SPEC
#include <winsock2.h>
#else
#include <sys/socket.h>
#endif //iDOS_SPEC


#include "iLambdaNetPair.h"

#include "lib_iobjs.h"


////////////////////////////////////////////////////////////
// Static members
// ---------------------------------------------------------
// <>
////////////////////////////////////////////////////////////
iNetPair::iNetPair ()
    : handled( -1 ),
      ioFlags( 0 ),
      lastIOvalue( 0 )
{
}

iNetPair::~iNetPair ()
{
 thisCloseHandled();
}

int iNetPair::SocketRead (void *buf, size_t count)
{
 ASSERTION(buf,"buf");
 return thisSocketRead( handled, buf, count, ioFlags );
}

int iNetPair::SocketWrite (void *buf, size_t count)
{
 ASSERTION(buf,"buf");
 return thisSocketWrite( handled, buf, count, ioFlags );
}

void iNetPair::Reset ()
{
 thisCloseHandled();
 gIpAddr::Reset();
}

int iNetPair::SetHandled (int aHandle)
{
 return handled = aHandle;
}

int iNetPair::SetFlags (int flags)
{
 ASSERTION(flags>=0,"flags>=0");
 return ioFlags = flags;
}

int iNetPair::thisCloseHandled ()
{
 DBGPRINT("DBG: iNetPair::thisCloseHandled [%s] handled:%d\n",Str(),handled);
 return thisShutDownSocket( handled );
}

int iNetPair::thisShutDownSocket (int& fd)
{
 if ( fd==-1 ) return -1;
#ifdef gNO_SHUTDOWN_SUPPORT
 close( fd );
#else
 shutdown( fd, 2 );  // Not just: close( handled )
#endif
 fd = -1;
 return 0;
}

int iNetPair::thisSocketRead (int fd, void *buf, size_t count, int flags)
{
 ASSERTION(fd!=-1,"fd!=-1");
 if ( count<=0 ) return 0;
#ifdef iDOS_SPEC
 lastIOvalue = recv( fd, (char*)buf, count, flags );
#else
 lastIOvalue = read( fd, buf, count );
#endif
 return lastIOvalue;
}

int iNetPair::thisSocketWrite (int fd, void *buf, size_t count, int flags)
{
 ASSERTION(fd!=-1,"fd!=-1");
 if ( count<=0 ) return 0;
#ifdef iDOS_SPEC
 lastIOvalue = send( fd, (char*)buf, count, flags );
#else
 lastIOvalue = write( fd, buf, count );
#endif
 return lastIOvalue;
}

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
 DBGPRINT("DBG: trying fd: %d, aPort: %d, errno: %d\n", fd, aPort, errno);
 if ( fd<0 ) {
     if ( errno>0 && isVerbose ) {
	 perror("server: can't open stream socket");
     }
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
 if ( handleServe!=-1 ) {
     shutdown( handleServe, 2 ); //close( handleServe )
     DBGPRINT("DBG: closed handleServe: %d\n",handleServe);
 }
}

iNetPair& gAltNetServer::ClientIp ()
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
 if ( handleServe==-1 ) return -1;
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
int create_or_append (FILE* fReport, const char* strPath, t_uint16 toAppend, int await)
{
 int status( 1 );
 int handle( -1 );
 int existsAlready( 0 );
 //bool forceRecreation( false );
 bool forcedRecreationError( false );
 mode_t mode( 00600 );
 t_uchar* uPath( (t_uchar*)strPath );

 // 'await' is the total time to await, in miliseconds, before retrying a rewrite

 // RETURNS:
 //	>=0 for a valid handle,
 //	-1 for an invalid handle,
 //	OR -2 for a re-creation fault!

 if ( strPath && uPath[ 0 ]>' ' ) {
     for (short retry=10; retry>0; retry--) {
	 gFileStat aStat( (char*)strPath );

	 if ( aStat.HasStat() && aStat.IsDirectory()==false ) {
	     existsAlready++;
	     if ( await > 0 ) {
		 gFileControl::Self().MiliSecSleep( await / 10 );
	     }
	     else {
		 break;
	     }
	 }
	 else {
	     break;
	 }
     }

     if ( existsAlready ) {
	 if ( (toAppend & 1)==0 ) {
	     remove( strPath );
	 }
     }
     if ( toAppend & 1 ) {
	 handle = open( strPath, UNIVERSAL_APPEND, mode );
     }
     else {
	 handle = creat( strPath, mode );
     }

     if ( handle==-1 && await>=0 ) {
	 if ( existsAlready ) {
	     status = remove( strPath )!=-1;
	     //forceRecreation = true;
	     forcedRecreationError = status==0;
	 }
	 handle = creat( strPath, mode );
     }

     if ( handle==-1 ) {
	 if ( forcedRecreationError ) {
	     if ( fReport ) {
		 fprintf(fReport, "Unable to re-create: %s\n", strPath);
	     }
	 }
	 return -1;
     }

     // Check if recently (re-)created file belongs to yourself
#ifdef iDOS_SPEC
     const uid_t fileUser( 0 );
#else
     struct stat newStat;

     memset( &newStat, 0x0, sizeof( newStat ) );
     status = fstat( handle, &newStat );
     const uid_t fileUser( newStat.st_uid );
     const uid_t myUser( getuid() );

     status = myUser==fileUser;
     DBGPRINT("DBG: creat handle: %d, SAME USER? %c, myUser=%d fileUser=%d\n",
	      handle,
	      ISyORn( status==1 ),
	      myUser, fileUser);
#endif

     if ( status!=1 ) {
	 close( handle );
	 status = remove( strPath )!=0;
	 if ( status ) {
	     if ( fReport ) {
		 if ( fileUser ) {
		     fprintf(fReport, "Unable to rebuild file: %s\n", strPath);
		 }
		 else {
		     fprintf(fReport, "Unable to rebuild file (uid: %d): %s\n", fileUser, strPath);
		 }
	     }
	     return -2;
	 }

	 handle = creat( strPath, mode );
     }
     return handle;
 }

 return -1;
}

////////////////////////////////////////////////////////////
Addressee::Addressee ()
    : address( 0 )
{
}


Addressee::Addressee (char* str)
    : address( 0 )
{
 sShown.Set( str );
}

Addressee::Addressee (gString& s)
    : address( 0 )
{
 sShown = s;
}


Addressee::Addressee (Addressee& copy)
    : address( copy.address )
{
 display = copy.display;
}


Addressee::~Addressee ()
{
}


gString& Addressee::Printable (int maxLen)
{
 thisCopyPrintable( sShown, display, maxLen );
 return display;
}


Addressee& Addressee::operator= (Addressee& copy)
{
 address = copy.address;
 display = copy.display;

 for (pCurrent=copy.StartPtr(); pCurrent; pCurrent=pCurrent->next) {
     Add( pCurrent->Str() );
 }
 return *this;
}


int Addressee::thisCopyPrintable (gString& sIn, gString& sOut, int maxLen)
{
 int invalid( -1 );
 int dot( 0 );
 t_uchar c;

 sOut.SetEmpty();

 for (int iter=1, len=sIn.Length(), k=0; iter<=len && (c = sIn[ iter ])!=0; iter++) {
     k++;
     if ( maxLen!=0 && k >= (maxLen * 2 / 3) ) {
	 if ( dot==0 ) {
	     sOut.Add( "[...]" );
	 }
	 dot++;
	 if ( k > maxLen - 6 ) {
	     k = MIN_INT16_I;
	 }
     }
     else {
	 if ( c < ' ' || c >= 127 ) {
	     if ( invalid==-1 ) {
		 invalid = (int)c;
	     }
	 }
	 else {
	     sOut.Add( c );
	 }
     }
 }

 if ( sOut.IsEmpty() ) {
     sOut.Set( "." );
     invalid = 0;
 }
 sOut.iValue = invalid;
 return dot;  // number of chars suppressed
}


////////////////////////////////////////////////////////////
int safe_create (const char* strPath, int await)
{
 const FILE* fReport( await==-1 ? nil : stderr );
 int result( create_or_append( (FILE*)fReport, strPath, 0, await ) );
 ASSERTION(result>=-2,"?");
 return result;
}


int safe_append (const char* strPath, int await)
{
 const FILE* fReport( await==-1 ? nil : stderr );
 int result( create_or_append( (FILE*)fReport, strPath, 1, await ) );
 ASSERTION(result>=-2,"?");
 return result;
}

////////////////////////////////////////////////////////////

