#ifndef LIB_ILOG_X_H
#define LIB_ILOG_X_H

#define LIB_ILOG_VERSION_MAJOR        1
#define LIB_ILOG_VERSION_MINOR        2


#ifdef ILG_DO_SHOW_MEM
#define ILG_SHOW_MEM(args...) ;
#else
#define ILG_SHOW_MEM(args...) fprintf(stderr,args)
#endif

#define ILG_NOT_SHOW(args...) ;


#include "ilog.h"
#include "ilstring.h"
#include "ioaux.h"
#include "handler.h"



#endif //LIB_ILOG_X_H

