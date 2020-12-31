#ifndef iPOPCLIENT_X_H
#define iPOPCLIENT_X_H

#include <string.h>

#include "istring.h"
#include "ipopauth.h"

#define POP_CMD_OUT_MAXSIZE	128  // must be <sizeof(lineBuffer)
////////////////////////////////////////////////////////////
typedef t_int32 t_msg_id;

struct sBoxStorage {
    sBoxStorage ()
	: nrMsgs( 0 ),
	  msgOld( 0 ),
	  msgNew( 0 ),
	  msgIdMin( -1 ),
	  msgIdMax( -1 ),
	  tagArray( nil ) {
    }

    ~sBoxStorage () {
	delete[] tagArray;
    }

    gString sPath;
    int nrMsgs;  // total number of messages
    t_stamp msgOld;  // oldest message timestamp
    t_stamp msgNew;  // newest message timestamp
    t_msg_id msgIdMin;
    t_msg_id msgIdMax;
    gList tags;
    gList ids;

    sBoxStorage* tagArray;

    void ResetMe () {
	nrMsgs = 0;
	tags.Delete();
	ids.Delete();
	delete[] tagArray; tagArray = nil;
    }

    int UpdateMe (int msgId,
		  const char* strName,
		  t_stamp stamp,
		  int mask) ;
};
////////////////////////////////////////////////////////////
class iPopClient : public gString {
public:
    enum ePopPhase {
	e_NotConnected,
	e_PopUserPass,
	e_PopSession,
	e_PopPreQuit,  // after RSET
	e_PopDidQuit,
	e_Disconnected  // upon a IO-error (functionally the same as e_NotConnected)
    };

    // Public data-members
    static const t_gPort defaultPopPort;
    static const int lineBufferSz;
    char lineBuffer[ 512 ];
    gIpAddr destAddr;
    gList capa;

    iPopClient (char* aHostStr=nil) ;

    virtual ~iPopClient () ;

    // Get methods
    virtual ePopPhase PopPhase () {
	return phase;
    }

    virtual bool HasUIDL () {
	return hasUIDL;
    }

    // Set methods
    virtual int Connect (t_gPort aPort=defaultPopPort, char* aHostStr=nil) {
	return thisConnectToPopServer( aPort, aHostStr );
    }

    virtual bool CloseSocket (bool updatePhase=true) ;

    bool Write () {
	if ( lineBuffer[0]==0 ) return false;
	return Write( lineBuffer );
    }

    bool Write (const char* aStr) ;

    bool Write (char* aStr) ;

    bool Read () {
	memset( lineBuffer, 0, lineBufferSz+1 );
	return thisRead( popHandle, lineBuffer, lineBufferSz, ioBytes, false )==0;
    }

    bool ReadLine () {
	memset( lineBuffer, 0, lineBufferSz+1 );
	return thisRead( popHandle, lineBuffer, lineBufferSz, ioBytes, true )==0;
    }

    // Special methods
    virtual int PopReset () ;

    virtual int UserAuthenticate (iCredential& userAuth) ;

    virtual int Capa () {
	if ( phase!=e_PopSession ) return 400;  // Bad request!
	capa.Delete();
	return thisGetCapabilities( capa );
    }

    virtual void CrToBlank (gString& s) {
	unsigned iter( 1 ), sized( s.Length() );
	for ( ; iter<=sized; iter++) {
	    if ( s[ iter ]=='\r' ) s[ iter ] = ' ';
	}
    }

protected:
    int popHandle;
    int ioBytes;
    ePopPhase phase;

    int thisConnectToPopServer (t_gPort aPort, char* aHostStr) ;

    int thisDisconnectPop () ;

    int thisWrite (int handle, char* aStr, int bytesToWrite, int& bytesWritten) ;

    int thisRead (int handle,
		  char* outStr,
		  int maxBytesToRead,
		  int& bytesRead,
		  bool stripNlCr=true) ;

    int thisGetCapabilities (gList& capaList) ;

private:
    bool hasUIDL;

    // Empties
    iPopClient (iPopClient& ) ;  //empty
    iPopClient& operator= (iPopClient& ) ; //empty
};
////////////////////////////////////////////////////////////
struct sGenEntryConf {
    const t_int16 idx;
    const char* strKey;  // The keyword
    const char* strObs;  // Short observation/comment
    const char* strDefault;
    int mask;
    int store;
};
////////////////////////////////////////////////////////////
#endif //iPOPCLIENT_X_H

