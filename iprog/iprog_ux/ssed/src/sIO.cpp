// sIO.cpp

#include <unistd.h>

#include "sIO.h"

////////////////////////////////////////////////////////////
// IO functions
////////////////////////////////////////////////////////////
int io_readchr (int handle, t_uchar& uChr)
{
 int result;

 ASSERTION(handle!=-1,"handle");
 // printf("PRE-READ (pos: %ld)\n", (long)lseek( handle, 0L, SEEK_CUR))
 result = read( handle, &uChr, 1 );
 // You could add here a timer - to handle read timeouts
 return result;
}


int io_read (int handle, void* ptrBuf, size_t bytes)
{
 ssize_t readBytes( 0 );

 ASSERTION(handle!=-1,"handle");
 if ( ptrBuf ) {
     readBytes = read( handle, ptrBuf, bytes );
 }
 return readBytes==(ssize_t)bytes;
}

////////////////////////////////////////////////////////////

