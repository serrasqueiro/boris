#ifndef iXMEASRV_X_H
#define iXMEASRV_X_H

#include "inet.h"

#include "iCIp.h"


#define XMEASURES_PROTO_MAJOR	1
#define XMEASURES_PROTO_MINOR	0

#ifdef gDOS_SPEC
#define IXM_OS "Win32"
#else
 #ifdef linux
 #define IXM_OS "Linux"
 #else
 #define IXM_OS "Unix"
 #endif
#endif


////////////////////////////////////////////////////////////
struct sComOpt {
    sComOpt () {
    }

    ~sComOpt () {
    }

    gString sPidFile;
};

////////////////////////////////////////////////////////////
struct sXmeasureOpt {
    sXmeasureOpt ()
	: numericVersionMajor( XMEASURES_PROTO_MAJOR ),
	  numericVersionMinor( XMEASURES_PROTO_MINOR ),
	  patchLetter( 'a' ),
	  simpleAuth( true ) {
	char life[ 16 ];
	snprintf( life, 15, "%02u.%02u%c",
		  XMEASURES_PROTO_MAJOR,
		  XMEASURES_PROTO_MINOR,
		  patchLetter);
	sVersion.Set( life );
	serverVersion.Set( "xmeasures (" IXM_OS ") " );
	serverVersion.AddString( sVersion );
    }

    const t_uint8 numericVersionMajor;
    const t_uint8 numericVersionMinor;
    const t_uchar patchLetter;

    bool simpleAuth;
    gList acceptableLiasons;  // allowed remote points


    gString sVersion;
    gString serverVersion;

    sComOpt common;
};

////////////////////////////////////////////////////////////

extern sXmeasureOpt xMeasures;


extern int ixm_configure_server (gList& confParams, sXmeasureOpt& xOpt) ;

extern int ixm_check_client (gCNetSource& clientSource, gString& sMsg) ;

extern int ixm_handle_request (int handle, sXmeasureOpt& xOpt) ;

////////////////////////////////////////////////////////////

extern char* ixm_safe_strchr (const char* aStr, char chr) ;

extern int ixm_http_text_response (int handle,
				   int httpCode,
				   char* buf,
				   const int bufSize,
				   const char* strServerVersion,
				   const char* str1,
				   const char* str2,
				   int& didWrite) ;

////////////////////////////////////////////////////////////
#endif //iXMEASRV_X_H

