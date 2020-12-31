// iCIp.cpp

#ifdef iDOS_SPEC
#include <io.h>
#endif //iDOS_SPEC

#include <unistd.h>

#include "iCIp.h"

////////////////////////////////////////////////////////////
// Static members
// ---------------------------------------------------------
// <>
////////////////////////////////////////////////////////////
gCNetSource::gCNetSource (bool hasSearch)
    : handleIn( -1 ),
      sourceGuessId( -1 )
{
}

gCNetSource::~gCNetSource ()
{
 Close();
}

t_uchar* gCNetSource::String ()
{
 if ( sBestString.IsEmpty() ) {
     sBestString = sourceGuessName;
     if ( sBestString.IsEmpty()==false )
	 sBestString.Add( ':' );
     sBestString.Add( gIpAddr::String() );
 }
 return (t_uchar*)BestString();
}

bool gCNetSource::Close ()
{
 if ( handleIn<0 ) return false;
 close( handleIn );
 DBGPRINT_MIN("DBG: Closed handle: %d [%u.%u.%u.%u=%s]\n",handleIn,b1,b2,b3,b4,String());
 handleIn = -1;
 return true;
}

int gCNetSource::BestGuessIdFromIP (gIpAddr& ip)
{
 sourceGuessName.Delete();
 SetAddr( ip.GetNetworkAddress() );
 if ( b1==127 && b2==0 && b3==0 ) {
     sourceGuessId = 1;
     return 1;  // Local loop (e_Lo1) is treated as id 1
 }
 return 0;
}
////////////////////////////////////////////////////////////

