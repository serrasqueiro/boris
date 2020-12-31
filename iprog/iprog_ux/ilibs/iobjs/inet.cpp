// inet.cpp

#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "inet.h"
#include "ifile.h"

////////////////////////////////////////////////////////////
// Static members
////////////////////////////////////////////////////////////
int gSocket::sockCount=0;

////////////////////////////////////////////////////////////
gIpAddr::gIpAddr (const char* ipStr)
    : b1( 0 ),
      b2( 0 ),
      b3( 0 ),
      b4( 0 )
{
 DBGPRINT_MIN("DBG: gIpAddr '%s'\n",ipStr);
 if ( ipStr!=nil ) SetAddrFromStr( ipStr );
}


gIpAddr::gIpAddr (t_uint8 c1, t_uint8 c2, t_uint8 c3, t_uint8 c4)
    : b1( c1 ),
      b2( c2 ),
      b3( c3 ),
      b4( c4 )
{
}


gIpAddr::~gIpAddr ()
{
}


bool gIpAddr::IsOk ()
{
 return (b1 | b2 | b3 | b4)!=0;
}


t_uchar* gIpAddr::String ()
{
 gString sTemp( 30, ' ' );
 addrStr = sTemp;
 sprintf( addrStr.Str(), "%u.%u.%u.%u", b1, b2, b3, b4 );
 return (t_uchar*)addrStr.Str();
}


t_uchar* gIpAddr::ToString (const t_uchar* uBuf)
{
 t_uchar* uStr( String() );
 if ( uBuf==nil ) return uStr;
 strcpy( (char*)uBuf, (char*)uStr );
 return (t_uchar*)uBuf;
}


void gIpAddr::Reset ()
{
 gControl::Reset();
 b1 = b2 = b3 = b4 = 0;
}


void gIpAddr::SetAddr (t_gIpAddr nboAddr)
{
 b4 = nboAddr & 0xFF;
 nboAddr >>= 8;
 b3 = nboAddr & 0xFF;
 nboAddr >>= 8;
 b2 = nboAddr & 0xFF;
 nboAddr >>= 8;
 b1 = nboAddr & 0xFF;
 nboAddr >>= 8;
 ASSERTION(nboAddr==0,"nboAddr>32bit?");
}


bool gIpAddr::SetAddrFromStr (const char* str)
{
 int a1=-1, a2=-1, a3=-1, a4=-1;
 ASSERTION(str!=nil,"str!=nil");
 Reset();
 lastOpError = sscanf(str,"%d.%d.%d.%d",&a1,&a2,&a3,&a4);
 lastOpError = -1*(lastOpError!=4);
 if ( lastOpError!=0 ) return false;
 lastOpError = a1<0 || a1>255;
 lastOpError += a2<0 || a2>255;
 lastOpError += a3<0 || a3>255;
 lastOpError += a4<0 || a4>255;
 lastOpError = -lastOpError;
 if ( lastOpError!=0 ) return false;
 b1 = (t_uint8)a1;
 b2 = (t_uint8)a2;
 b3 = (t_uint8)a3;
 b4 = (t_uint8)a4;
 return true;
}


t_gIpAddr gIpAddr::GetNetworkAddress ()
{
 unsigned long nBO = thisGetIP( b1, b2, b3, b4 );
 return (t_gIpAddr)nBO;
}


int gIpAddr::GetHostByName (const char* hostname)
{
 struct hostent* hp=nil;
 char** hAddr;

 ASSERTION(hostname!=nil,"hostname!=nil");
 Reset();
 hp = gethostbyname( hostname );
 lastOpError = hp==nil;
 if ( lastOpError!=0 ) {
     SetError( errno );
     return -1;
 }

 hAddr = &hp->h_addr;
 if ( hAddr ) {
     thisSetIPfromHostEnt( *hAddr, hp->h_length );
 }
 else {
     SetError( 1 );
     return 1;
 }
 return 0;
}


int gIpAddr::GetHostByAddr (gString& sRes)
{
 gList lRes;
 return thisGetHostByAddr( (char*)String(), sRes, lRes );
}


gIpAddr& gIpAddr::operator= (gIpAddr& copy)
{
 b1 = copy.b1;
 b2 = copy.b2;
 b3 = copy.b3;
 b4 = copy.b4;
 return *this;
}


void gIpAddr::Show (bool doShowAll)
{
 iprint("%s%s",String(),doShowAll?"\n":"\0");
}


unsigned long gIpAddr::thisGetIP (t_uint8 a1, t_uint8 a2, t_uint8 a3, t_uint8 a4)
{
 unsigned long result;
 result = a1;
 result <<= 8;
 result += a2;
 result <<= 8;
 result += a3;
 result <<= 8;
 result += a4;
 return result;
}


unsigned long gIpAddr::thisGetHostByteOrder (t_gIpAddr netByteOrder)
{
 unsigned long result( ntohl( (unsigned long)netByteOrder ) );
 return result;
}


int gIpAddr::thisSetIPfromHostByteOrder (in_addr_t hostByteOrder)
{
 unsigned long nBO( htonl( hostByteOrder ) );
 // nBO is the network-byte-order
 SetAddr( (t_gIpAddr)nBO );
 return 0;
}


int gIpAddr::thisSetIPfromHostEnt (const char* hAddr, int h_length)
{
 struct sockaddr_in hostAddressData;

 ASSERTION(hAddr,"hAddr");
 memset( (char*)&hostAddressData, 0, sizeof(hostAddressData) );
 memcpy( &hostAddressData.sin_addr, hAddr, h_length );

 in_addr_t hostByteOrder( hostAddressData.sin_addr.s_addr );
 return thisSetIPfromHostByteOrder( hostByteOrder );
}


int gIpAddr::thisGetHostByAddr (const char* ipStr, gString& sRes, gList& lRes)
{
 struct hostent* hp( nil );
 char* inAddrTmp;
 char* str;
 char** pStr;
 struct in_addr hardInAddr;

 ASSERTION(ipStr!=nil,"ipStr!=nil");
 memset( &hardInAddr, 0x0, sizeof(hardInAddr) );
 inAddrTmp = (char*)&hardInAddr;

 hardInAddr.s_addr = inet_addr( ipStr );
 if ( hardInAddr.s_addr==INADDR_NONE ) {
     return SetError( 2 );
 }
 hp = gethostbyaddr( inAddrTmp, sizeof(hardInAddr), AF_INET );
 if ( hp==nil ) {
     return SetError( -1 );
 }
 sRes.Set( (char*)hp->h_name );

 // Aliases for host
 pStr = hp->h_aliases;
 while ( (str = *pStr)!=nil ) {
     pStr++;
     lRes.Add( str );
 }

 return 0;
}

////////////////////////////////////////////////////////////
gHostAddr::gHostAddr (const char* hostStr)
    : destHostStr( hostStr )
{
 if ( hostStr ) {
     GetHostByName( hostStr );
 }
}


gHostAddr::gHostAddr (t_uint8 c1, t_uint8 c2, t_uint8 c3, t_uint8 c4)
    : gIpAddr( c1, c2, c3, c4 )
{
}


gHostAddr::gHostAddr (gIpAddr& ipAddr)
    : gIpAddr( ipAddr.b1, ipAddr.b2, ipAddr.b3, ipAddr.b4 )
{
}


gHostAddr::~gHostAddr ()
{
}


bool gHostAddr::IsOk ()
{
 return gIpAddr::IsOk();
}


t_uchar* gHostAddr::String ()
{
 if ( gIpAddr::IsOk() ) return gIpAddr::String();
 return (t_uchar*)destHostStr.Str();
}


bool gHostAddr::SetHostName (const char* hostStr)
{
 destHostStr.SetEmpty();
 if ( hostStr==nil || hostStr[0]==0 ) return false;
 destHostStr.Set( hostStr );
 return true;
}


void gHostAddr::Show (bool doShowAll)
{
 if ( doShowAll==false ) {
     gIpAddr::Show( false );
 }
 iprint("%s:%s\n",destHostStr.Str(),String());
}

////////////////////////////////////////////////////////////
gSocket::gSocket (int socketHandle, eSocketKind socketKind)
    : gInt( socketHandle ),
      sockKind( socketKind ),
      hostConnected( nil )
{
 sockCount++;
}


gSocket::~gSocket ()
{
 delete hostConnected;
 hostConnected = nil;
 sockCount--;
}


bool gSocket::IsOk ()
{
 return IsOpened() && IsConnected();
}


bool gSocket::IsOpened ()
{
 return iValue!=-1;
}


int gSocket::Handle ()
{
 ASSERTION(IsOpened(),"IsOpened()");
 return iValue;
}


bool gSocket::IsConnected ()
{
 return hostConnected!=nil;
}


bool gSocket::Open (const t_uchar* uName)
{
 char sBuf[ 50 ];

 if ( uName==nil ) {
     // 'Invent' one string
     sprintf(sBuf,"%s#%d",sockKind==e_TCP?"TCP":"UDP",sockCount);
 }
 else {
     // The code below induces problems
     //strncpy( sBuf, (char*)uName, 49 );
     //uName[49] = 0;
     // (not used due to bug of glibc)
     ASSERTION(strlen((char*)uName)+1<50,"Node-name too long");
     strcpy( sBuf, (char*)uName );
 }

 iValue = socket( AF_INET, SOCK_STREAM, 0 );
 ioMask = iValue==-1;  // Out of memory!
 return iValue!=-1;
}


bool gSocket::Close ()
{
 if ( iValue==-1 ) return false;
 close( iValue );
 iValue = -1;
 return true;
}


bool gSocket::SetConnection (gIpAddr& ipAddr)
{
 // Set connection twice? Return false
 if ( IsOk() ) return false;
 hostConnected = new gHostAddr( ipAddr );
 ASSERTION(hostConnected!=nil,"hostConnected!=nil");
 return hostConnected->IsOk();
}


bool gSocket::SetConnection (gHostAddr& hostAddr)
{
 bool isOk;
 gIpAddr ipAddr( hostAddr.GetB1(), hostAddr.GetB2(), hostAddr.GetB3(), hostAddr.GetB4() );
 isOk = SetConnection( ipAddr );
 if ( isOk==false ) return false;
 return hostConnected->SetHostName( hostAddr.GetHostName() );
}

////////////////////////////////////////////////////////////
gNetConnect::gNetConnect ()
{
}


gNetConnect::~gNetConnect ()
{
}


bool gNetConnect::Read (t_uchar& c)
{
 static t_uchar uChr;
 ssize_t nOctetsRead;
 // In DOS, 'recv' is used often
#ifdef iDOS_SPEC
 static char cChr;
 nOctetsRead = recv( Handle(), &cChr, 1, 0 );
 uChr = (t_uchar)cChr;
#else
 nOctetsRead = read( Handle(), &uChr, 1 );
#endif //iDOS_SPEC (~)
 if ( nOctetsRead==1 ) {
     c = uChr;
     lastOpError = 0;
     return true;
 }
 SetError( errno );
 return false;
}


bool gNetConnect::Read (t_uchar* uBuf, unsigned nBytes)
{
 ssize_t nOctetsRead;
 lastOpError = 0;
 if ( nBytes==0 ) return true;
 // Always initialize memory
 memset( uBuf, 0x0, (size_t)nBytes );
 // Read from socket
#ifdef iDOS_SPEC
 nOctetsRead = recv( Handle(), (char*)uBuf, (size_t)nBytes, 0 );
#else
 nOctetsRead = read( Handle(), uBuf, (size_t)nBytes );
 DBGPRINT("DBG: read(%d)=%u\n",(unsigned)nBytes,(int)nOctetsRead);
#endif //iDOS_SPEC (~)
 if ( nOctetsRead==(ssize_t)nBytes ) return true;
 SetError( errno );
 return false;
}


bool gNetConnect::ReadLine (gString& s)
{
 static t_uchar uChr;

 s.SetEmpty();
 for ( ; Read( uChr )==true; ) {
     if ( uChr=='\r' ) continue;
     if ( uChr=='\n' ) return true;
     s.Add( uChr );
 }
 return false;
}


int gNetConnect::Write (const char* s)
{
 return Write( s, strlen(s) );
}


int gNetConnect::Write (gString& s)
{
 return Write( s.Str(), s.Length() );
}


int gNetConnect::Write (const char* s, unsigned nOctets)
{
 ssize_t nOctetsWritten, nOctetsToWrite = (ssize_t)nOctets;;
 ASSERTION(s!=nil,"s!=nil");
 if ( nOctets==0 ) return 0;
 // In DOS, 'send' is used often
#ifdef iDOS_SPEC
 nOctetsWritten = send( Handle(), s, nOctets, 0 );
#else
 nOctetsWritten = write( Handle(), s, nOctets );
#endif //iDOS_SPEC (~)
 if ( nOctetsWritten==nOctetsToWrite ) {
     return lastOpError = 0;
 }
 return SetError( errno );
}

////////////////////////////////////////////////////////////
gTcpConnect::gTcpConnect (const char* destHostname, t_gPort cPort, bool doConnect)
    : destAddr( destHostname ),
      destPort( cPort ),
      skt( -1, gSocket::e_TCP )
{
 SetError( destAddr.lastOpError );
 if ( lastOpError==0 && doConnect==true ) Connect();
}


gTcpConnect::gTcpConnect (t_uint8 c1, t_uint8 c2, t_uint8 c3, t_uint8 c4, t_gPort cPort, bool doConnect)
    : destAddr( c1, c2, c3, c4 ),
      destPort( cPort ),
      skt( -1, gSocket::e_TCP )
{
 lastOpError = destAddr.IsOk();
 if ( lastOpError==0 && doConnect==true ) Connect();
}


gTcpConnect::~gTcpConnect ()
{
 skt.Close();
}


bool gTcpConnect::IsOk ()
{
 return skt.IsOpened();
}


int gTcpConnect::Handle ()
{
 int fd( skt.Handle() );
 ASSERTION(IsOk(),"gTcpConnect::Handle (1)");
 ASSERTION(fd!=-1,"gTcpConnect::Handle (2)");
 return fd;
}


t_uchar* gTcpConnect::String ()
{
 return destAddr.String();
}


t_uchar* gTcpConnect::ToString (const t_uchar* uBuf)
{
 t_uchar* uStr( String() );
 if ( uBuf==nil ) return uStr;
 strcpy( (char*)uBuf, (char*)uStr );
 return (t_uchar*)uBuf;
}


bool gTcpConnect::Connect ()
{
 bool isOk( skt.IsOpened()==false );
 int error;
 if ( isOk==false ) {
     DBGPRINT("gTcpConnect::Connect {%s} already opened\n",String());
     SetError( -1 );
     return false;
 }
 isOk = skt.Open( String() );
 if ( isOk==false ) return false;
 error = thisConnect( skt );
 isOk = error==0;
 DBGPRINT("gTcpConnect::Connect {%s} Ok? %c, error=%d\n", String(), ISyORn( isOk ), error);
 return isOk;
}


bool gTcpConnect::Close ()
{
 return skt.Close();
}


void gTcpConnect::Show (bool doShowAll)
{
 iprint("IP:%s%s\n",
	String(),
	doShowAll ?
	(skt.IsOpened() ? "[opened]" : "[closed]")
	:
	"\0");
}


int gTcpConnect::thisConnect (gSocket& aSocket)
{
 int socketId( aSocket.Handle() );
 struct sockaddr_in toHostAddressData;
 int sizeofAddrData( sizeof(toHostAddressData) );
 int error( 0 );

 memset( (char*)&toHostAddressData, 0, sizeofAddrData );
 toHostAddressData.sin_family = AF_INET;
 toHostAddressData.sin_addr.s_addr = destAddr.GetHostAddress();
 toHostAddressData.sin_port = htons( destPort );

 lastOpError = connect( socketId, (struct sockaddr*)&toHostAddressData, sizeofAddrData )!=0;
 if ( lastOpError!=0 ) {
     error = errno;
     // In iDOS_SPEC, sometimes g++ does not set errno correctly
     if ( error==0 ) {
         error = 1;
     }
     SetError( error );
     aSocket.Close();
 }
 else {
     // Inform the socket to whom the connection was established
     aSocket.SetConnection( destAddr );
 }
 DBGPRINT_MIN("DBG: thisConnect error=%d, handle=%d\n",
	      error,
	      aSocket.iValue);
 return error;
}

////////////////////////////////////////////////////////////

