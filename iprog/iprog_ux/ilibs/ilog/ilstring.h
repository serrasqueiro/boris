#ifndef gILSTRING_X_H
#define gILSTRING_X_H

#include "istring.h"

////////////////////////////////////////////////////////////
// ils_... (string manip. functions)
////////////////////////////////////////////////////////////
extern int ils_string_sane (gString& s, int optMask, char substChr) ;

extern int ils_string_toint (const char* str, int optMask, int defaultValueOnError) ;

extern t_int64 ils_string_toint64 (const char* str, int optMask, t_int64 defaultValueOnError) ;

////////////////////////////////////////////////////////////
#endif //gILSTRING_X_H

