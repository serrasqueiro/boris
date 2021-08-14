#ifndef G_CPROXY_X_H
#define G_CPROXY_X_H

#include "iCntpConfig.h"
#include "iCBindPart.h"

////////////////////////////////////////////////////////////

extern int cxpxy_process_proxy (gArg& arg,
				gAltNetServer* netServer,
				bool isDaemon,
				FILE* fOut,
				FILE* fRepErr,
				sOptCntp& opt) ;

////////////////////////////////////////////////////////////
#endif //G_CPROXY_X_H

