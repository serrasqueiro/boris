// iCIO.cpp

#include <unistd.h>

#ifdef iDOS_SPEC
#include <winsock.h>
#endif //iDOS_SPEC
////////////////////////////////////////////////////////////
ssize_t cxio_read (int fd, void* buf, size_t count)
{
#ifdef iDOS_SPEC
 int readBytes = recv( fd, (char*)buf, count, 0 );
#else
 int readBytes = read( fd, buf, count );
#endif //~iDOS_SPEC
 return readBytes;
}


ssize_t cxio_write (int fd, void *buf, size_t count)
{
#ifdef iDOS_SPEC
 int code = send( fd, (char*)buf, count, 0 );
#else
 int code = write( fd, buf, count );
#endif //~iDOS_SPEC
 return code;
}
////////////////////////////////////////////////////////////

