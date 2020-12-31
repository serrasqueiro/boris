#ifndef LIB_CYSOFT_X_H
#define LIB_CYSOFT_X_H

#define LIB_CYSOFT_VERSION_MAJOR        0
#define LIB_CYSOFT_VERSION_MINOR        2


#ifdef CYS_DO_SHOW_MEM
#define CYS_SHOW_MEM(args...) ;
#else
#define CYS_SHOW_MEM(args...) fprintf(stderr,args)
#endif

#define CYS_NOT_SHOW(args...) ;


#include "imconv.h"

#include "phash.h"


#endif //LIB_CYSOFT_X_H

