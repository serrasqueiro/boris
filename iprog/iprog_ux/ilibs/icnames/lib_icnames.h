#ifndef LIB_ICNAMES_X_H
#define LIB_ICNAMES_X_H

#define LIB_ICNAMES_VERSION_MAJOR	1
#define LIB_ICNAMES_VERSION_MINOR	0

#ifndef iprint
#if defined(DEBUG) || defined(DEBUG_MIN)
#define iprint(args...) printf(args)
#else
#define iprint(args...) ;
#endif
#endif //iprint

#define ICN_TLD_HASH(v)		((unsigned)(v) % 0x10000)


#include "icnames.h"
#include "iordered.h"
#include "ntp_analyst.h"



#endif //LIB_ICNAMES_X_H

