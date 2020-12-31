// ipopclient.cpp

#include "inet.h"
#include "iarg.h"

#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "ipopclient.h"

#include "ibase.h"
////////////////////////////////////////////////////////////
// Static members
// ---------------------------------------------------------
const t_gPort iPopClient::defaultPopPort=110;
const int iPopClient::lineBufferSz=511;
////////////////////////////////////////////////////////////
int sBoxStorage::UpdateMe (int msgId,
			   const char* strName,
			   t_stamp stamp,
			   int mask)
{
 nrMsgs++;
 if ( msgIdMin==-1 || msgId<msgIdMin )
     msgIdMin = msgId;
 if ( msgId>msgIdMax )
     msgIdMax = msgId;
 if ( stamp ) {
     if ( msgOld==0 || stamp<msgOld )
	 msgOld = stamp;
     if ( stamp>msgNew )
	 msgNew = stamp;
 }
 return stamp==0;  // Returns 1 if stamp is invalid
}
////////////////////////////////////////////////////////////
// iPopClient - Handles POP3 client requests
// ---------------------------------------------------------
iPopClient::iPopClient (char* aHostStr)
    : gString( aHostStr ),
      popHandle( -1 ),
      ioBytes( 0 ),
      phase( e_NotConnected ),
      hasUIDL( false )  // we do not know at the beginning: assuming no UIDL is supported
{
 lineBuffer[ 0 ] = 0;
 if ( aHostStr ) {
     thisConnectToPopServer( defaultPopPort, aHostStr );
 }
}


iPopClient::~iPopClient ()
{
 thisDisconnectPop();
}


bool iPopClient::CloseSocket (bool updatePhase)
{
 if ( updatePhase ) {
     phase = e_NotConnected;
 }
 DBGPRINT_MIN("DBG: iPopClient::CloseSocket phase=%d, popHandle=%d\n",
	      phase,
	      popHandle);
 if ( popHandle!=-1 ) {
     close( popHandle );
     popHandle = -1;
     return true;
 }
 return false;
}


bool iPopClient::Write (const char* aStr)
{
 int len;
 if ( aStr==nil ) return false;
 len = strlen( aStr );
 return thisWrite( popHandle, (char*)aStr, len, len )==0;
}


bool iPopClient::Write (char* aStr)
{
 int len;
 if ( aStr==nil ) return false;
 len = strlen( aStr );
 return thisWrite( popHandle, aStr, len, len )==0;
}


int iPopClient::PopReset ()
{
 int error( phase==e_NotConnected || phase==e_PopDidQuit || phase==e_Disconnected );
 if ( error ) return -1;
 error = Write( "RSET\r\n" );
 if ( error ) return 1;
 // Server should respond: "+OK [...]"
 ReadLine();
 phase = e_PopPreQuit;
 return lineBuffer[ 0 ]!='+';
}


int iPopClient::UserAuthenticate (iCredential& userAuth)
{
 int error;

 DBGPRINT("DBG: UserAuthenticate {%s}: phase=%d\n",userAuth.Str(),phase);

 // POP3 does not allow to authenticate twice:
 if ( phase!=e_PopUserPass ) return -1;

 snprintf( lineBuffer, 128, "USER %s\r\n", userAuth.Str() );
 error = Write()==false;
 if ( error ) return 1;
 error = ReadLine()==false;
 if ( error || lineBuffer[ 0 ]!='+' ) return 1;
 snprintf( lineBuffer, 128, "PASS %s\r\n", userAuth.pass.Str() );
 error = Write()==false;
 if ( error ) return 2;
 error = ReadLine()==false;
 DBGPRINT_MIN("DBG: Authenticated? %c {%s,%s}: {%s}\n",
	      ISyORn( (error || lineBuffer[ 0 ]!='+')==0 ),
	      userAuth.Str(),
	      userAuth.pass.Str(),
	      lineBuffer);
 if ( error || lineBuffer[ 0 ]!='+' ) {
     gString sLine( lineBuffer+1 );
     DBGPRINT("DBG: user&pass not successful, locked mailbox? %c, {%s}\n",
	      ISyORn( sLine.Find( "LOCK", true ) ),
	      lineBuffer);
     if ( sLine.Find( "LOCK", true ) ) {
	 CloseSocket( false );
	 phase = e_Disconnected;
	 return 8;
     }
     return 4;
 }

 phase = e_PopSession;
 return 0;
}


int iPopClient::thisConnectToPopServer (t_gPort aPort, char* aHostStr)
{
 struct sockaddr_in toHostAddressData;
 int sizeofAddrData( sizeof(toHostAddressData) );
 int ioOp;

 // Checking POP3 capabilities:
 //	+OK Teapop [0.3.7] - Teaspoon stirs around again <1255788643.BEED42@Llywellyn>
 //	capa
 //	+OK These are my limits, Sir
 //	TOP
 //	USER
 //	LOGIN-DELAY 900
 //	UIDL
 //	EXPIRE NEVER
 //	IMPLEMENTATION Teapop-0.3.7
 //	.
 //	quit
 // OR:
 //	+OK <Welcome.to.PTMail.POP3@pop4>
 //	capa
 //	-ERR authorization first

 // So: we are our client only requests CAPA after signing in:

 if ( aHostStr==nil ) return 0;
 CloseSocket();

 ioOp = ipcl_ip_from_hostaddr( aHostStr, destAddr );
 DBGPRINT_PIO("DBG: destAddr={%s}, %u.%u.%u.%u, IsOk? %c (ioOp=%d)\n",
	      aHostStr,
	      destAddr.b1, destAddr.b2, destAddr.b3, destAddr.b4,
	      ISyORn( destAddr.IsOk() ),
	      ioOp);
 if ( ioOp ) return -2;

 memset( (char*)&toHostAddressData, 0, sizeofAddrData );
 toHostAddressData.sin_family = AF_INET;
 toHostAddressData.sin_addr.s_addr = destAddr.GetHostAddress();
 toHostAddressData.sin_port = htons( aPort );

 popHandle = socket( AF_INET, SOCK_STREAM, 0 );
 if ( popHandle==-1 ) return -1;

 ioOp = connect( popHandle, (struct sockaddr*)&toHostAddressData, sizeofAddrData );
 if ( ioOp ) {
     // Cannot connect to server
     CloseSocket();
     return 13;
 }

 if ( ReadLine() && lineBuffer[0]=='+' ) {
     phase = e_PopUserPass;
     return 0;
 }
 return 500;  // Inspired on HTTP server error, http://en.wikipedia.org/wiki/List_of_HTTP_status_codes#5xx_Server_Error
}


int iPopClient::thisDisconnectPop ()
{
 bool ioOk;

 if ( popHandle==-1 ) return 0;

 // If connection is up, close it smoothly
 ioOk = Write( "quit\r\n" );
 if ( ioOk ) {
     if ( ReadLine() ) Set( lineBuffer );
     phase = e_PopDidQuit;
 }
 else {
     phase = e_Disconnected;
 }
 CloseSocket( false );
 return ioOk==false;
}


int iPopClient::thisWrite (int handle, char* aStr, int bytesToWrite, int& bytesWritten)
{
 bytesWritten = -2;
 if ( handle!=-1 ) {
     bytesWritten = write( handle, aStr, bytesToWrite );
     DBGPRINT_PIO("DBG: POP write (bytes=%d, written=%d): %s\n",
		  bytesToWrite, bytesWritten,
		  aStr);
 }
 return bytesWritten!=bytesToWrite;
}


int iPopClient::thisRead (int handle,
			  char* outStr,
			  int maxBytesToRead,
			  int& bytesRead,
			  bool stripNlCr)
{
 bytesRead = -2;
 if ( handle==-1 ) return -1;
 ASSERTION(outStr,"outStr");
 ASSERTION(maxBytesToRead>=0,"maxBytesToRead");
 bytesRead = read( handle, outStr, maxBytesToRead );
 if ( bytesRead<=0 ) return 1;
 if ( bytesRead>maxBytesToRead ) return -1;  // Academic

#if 0
 for (short ix=0; ix<bytesRead; ix++) {
     char dbgChr( outStr[ ix ] );
     printf("%02d\t%d\t%c\n",ix,dbgChr,dbgChr>=' ' && dbgChr<127 ? dbgChr : '.');
 }
#endif
 outStr[ bytesRead ] = 0;

 if ( stripNlCr ) {
     for (int iter=bytesRead-1; iter>=0; ) {
	 if ( outStr[ iter ]>'\r' ) break;
	 outStr[ iter-- ] = 0;
     }
 }
 DBGPRINT_PIO("DBG: POP read (bytesRead=%d): {%s}\n",
	      bytesRead,
	      outStr);
 return 0;
}


int iPopClient::thisGetCapabilities (gList& capaList)
{
 bool didWrite( Write( "CAPA\r\n" ) );
 bool didRead( didWrite && ReadLine() );
 bool hadDot( false );
 int error( didRead==false );

 if ( error || lineBuffer[ 0 ]!='+' ) return 1;

 for ( ; hadDot==false && (didRead = ReadLine())==true; ) {
     if ( strcmp( lineBuffer, "." )==0 ) break;
     gString sUp( lineBuffer );
     sUp.UpString();
     gParam listed( sUp, "\n" );

     for (unsigned iter=1; iter<=listed.N(); iter++) {
	 gString sLine( listed.Str( iter ) );
	 CrToBlank( sLine );
	 sLine.Trim();
	 hadDot = sLine.Match( "." );
	 if ( hadDot ) break;
	 capaList.Add( sLine );
     }
 }

 hasUIDL = capaList.Match( "UIDL" );
 DBGPRINT("DBG: CAPA Ok? %c, N=%u, hasUIDL? %c\n",
	  ISyORn( didRead ),
	  capaList.N(),
	  ISyORn( hasUIDL) );
 // A trailing dot, always, otherwise something got broken within POP3!
 return didRead==false;
}
////////////////////////////////////////////////////////////

