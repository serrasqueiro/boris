#ifndef iLIB_DUB_X_H
#define iLIB_DUB_X_H

#define LIB_DUB_VERSION_MAJOR        1
#define LIB_DUB_VERSION_MINOR        0


#include "dub.h"
#include "filters.h"

////////////////////////////////////////////////////////////
struct sPairOpt {
    int value;
    const char* str;
};

////////////////////////////////////////////////////////////
struct sTripleOpt {
    int value;
    const char* fixStr;
    char* str;
};

////////////////////////////////////////////////////////////

#endif //iLIB_DUB_X_H

