// iNetPair.cpp


#include <unistd.h>  // needed due to Win32 port: 'close'/'recv/send' functions

#ifdef iDOS_SPEC
#include <sys/types.h>
#include <winsock2.h>
#else
#include <sys/socket.h>
#endif //iDOS_SPEC


#include "iNetPair.h"

////////////////////////////////////////////////////////////
// Static members
// ---------------------------------------------------------
// <>
////////////////////////////////////////////////////////////
gNetPair::gNetPair ()
    : handled( -1 ),
      ioFlags( 0 ),
      lastIOvalue( 0 )
{
}

gNetPair::~gNetPair ()
{
 thisCloseHandled();
}

int gNetPair::SocketRead (void *buf, size_t count)
{
 ASSERTION(buf,"buf");
 return thisSocketRead( handled, buf, count, ioFlags );
}

int gNetPair::SocketWrite (void *buf, size_t count)
{
 ASSERTION(buf,"buf");
 return thisSocketWrite( handled, buf, count, ioFlags );
}

void gNetPair::Reset ()
{
 thisCloseHandled();
 gIpAddr::Reset();
}

int gNetPair::SetHandled (int aHandle)
{
 ASSERTION(handled<=-1,"handled<=-1");
 return handled = aHandle;
}

int gNetPair::SetFlags (int flags)
{
 ASSERTION(flags>=0,"flags>=0");
 return ioFlags = flags;
}

int gNetPair::thisCloseHandled ()
{
 DBGPRINT("DBG: gNetPair::thisCloseHandled [%s] handled:%d\n",Str(),handled);
 return thisShutDownSocket( handled );
}

int gNetPair::thisShutDownSocket (int& fd)
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

int gNetPair::thisSocketRead (int fd, void *buf, size_t count, int flags)
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

int gNetPair::thisSocketWrite (int fd, void *buf, size_t count, int flags)
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

