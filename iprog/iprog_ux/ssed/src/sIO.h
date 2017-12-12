#ifndef SSED_IO_X_H
#define SSED_IO_X_H

#include "lib_iobjs.h"

////////////////////////////////////////////////////////////

int io_readchr (int handle, t_uchar& uChr) ;

int io_read (int handle, void* ptrBuf, size_t bytes) ;

////////////////////////////////////////////////////////////
#endif //SSED_IO_X_H

