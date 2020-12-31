#ifndef AUXSFV_X_H
#define AUXSFV_X_H

#include "lib_imedia.h"

////////////////////////////////////////////////////////////
// SFV help functions
////////////////////////////////////////////////////////////

extern iEntry* sfv_entries_from_file (const char* strFile) ;

extern int sfv_check (iEntry& entries, const char* strPath, FILE* fOut, FILE* fReport, int& missing) ;

extern iEntry* new_sfv_entry (gString& sLine) ;

extern int sfv_dump (iEntry& entries, const char* strPath, bool newLineCR, FILE* fOut, FILE* fReport) ;

////////////////////////////////////////////////////////////
#endif //AUXSFV_X_H

