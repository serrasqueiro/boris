#ifndef SUMA_X_H
#define SUMA_X_H

#include "lib_iobjs.h"


////////////////////////////////////////////////////////////

extern int file_sumA (const char* strFile, int optUnused, t_uint16& checksum) ;

extern int calc_sumA (int fd, t_uchar* buf, unsigned bufSize, t_uint64 reqBytes, t_uint64& totalBytes, t_uint16& checksum) ;

////////////////////////////////////////////////////////////
#endif //SUMA_X_H

