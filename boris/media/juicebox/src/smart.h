#ifndef SMART_X_H
#define SMART_X_H


#include "lib_imedia.h"

////////////////////////////////////////////////////////////

struct sNameHTML {
    t_uint16 usableChr;
    const char* str;
    const char* name;
};

////////////////////////////////////////////////////////////

extern Media* smart_rename (Media& nameBy, t_uint32 uMask) ;

extern bool NameUnquote (gString& aString) ;

extern int PhraseUnHTML (gString& words, gList* pIssues) ;

extern int NameFromRawWPL (gString& sLine) ;

extern int NameFromWPL (gString& sLine, t_uint16 mask, gString& sNew) ;

extern int NameFromWPL_tid (gString& sLine, t_uint16 mask, gString& sNew, gString& sTID, gString& sCID) ;

////////////////////////////////////////////////////////////
#endif //SMART_X_H

