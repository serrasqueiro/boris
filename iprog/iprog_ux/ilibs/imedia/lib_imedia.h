#ifndef LIB_IMEDIA_X_H
#define LIB_IMEDIA_X_H

#define LIB_IMEDIA_VERSION_MAJOR	2
#define LIB_IMEDIA_VERSION_MINOR	1

#include "inames.h"
#include "ibases.h"
#include "isweb.h"
#include "isoconv.h"

#include "iplaylist.h"
#include "acdb.h"
#include "entry.h"
#include "mdir.h"
#include "cue.h"


#if defined(DEBUG) || defined(DEBUG_MIN)
#define imd_print(args...) printf(args)
#define imd_print_error(args...) fprintf(stderr,args)
#else
#define imd_print(args...) ;
#define imd_print_error(args...) ;
#endif

#if defined(DEBUG)
#define printd(args...) printf(args)
#else
#define printd(args...) ;
#endif


#define IMEDIA_DECLARE	sIsoUni* ptrUniData=nil;  // place at top of cpp at main
#define IMEDIA_INIT	imb_iso_init( nil, new sIsoUni )  // place at main()


#endif //LIB_IMEDIA_X_H

