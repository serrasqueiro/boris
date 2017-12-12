#ifndef SSED_BATCH_X_H
#define SSED_BATCH_X_H

#include "lib_iobjs.h"


#define BTC_WARN_CHARS  "\x96ô"

////////////////////////////////////////////////////////////

extern gList* read_batch_input (int readHandle) ;

extern int btc_run_batch (FILE* fOut, gList& aBatch, gString& lastLine) ;

extern int btc_add_once (const char* strIn, gList& result) ;

////////////////////////////////////////////////////////////
#endif //SSED_BATCH_X_H

