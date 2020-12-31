#ifndef IMAUDIO_X_H
#define IMAUDIO_X_H

#include <string.h>

#include "iarg.h"
#include "ilist.h"

#ifdef DEBUG_PIO
#define DBGPRINT_PIO(args...) printf(args)
#else
#define DBGPRINT_PIO(args...) ;
#endif

////////////////////////////////////////////////////////////
class imAudio : public gList {
public:
    imAudio () ;

    virtual ~imAudio () ;

private:
    // Operators,empty
    imAudio (imAudio& ) ; //empty
    imAudio& operator= (imAudio& ) ; //empty
};

////////////////////////////////////////////////////////////
#endif //IMAUDIO_X_H

