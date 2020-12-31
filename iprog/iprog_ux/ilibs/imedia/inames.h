#ifndef iMEDIA_X_H
#define iMEDIA_X_H

#include "istring.h"


// Note: in several applications prime-million is also known as: MM_NAMED_PRIME
#define ptm_nprime(v) (((unsigned)v)%PRIME_MILLION_NP0)

////////////////////////////////////////////////////////////
struct sMdaEncodeCodes {
    t_int16 usage;  // -1: the last, empty/null element; 0: unusual; 1: usual, ...etc.
    const char* strPercent;
    const char* abbrev;
    const char* strName;
};

////////////////////////////////////////////////////////////
class iMdaFileName : public gString {
public:
    // Public data-members
    static const sMdaEncodeCodes defaultEncodeCodes[];

    iMdaFileName (char* aStr) ;

    virtual ~iMdaFileName () ;

private:
    // Empties
    iMdaFileName (iMdaFileName& ) ;  //empty
    iMdaFileName& operator= (iMdaFileName& ) ; //empty
};

////////////////////////////////////////////////////////////
// ptm functions
////////////////////////////////////////////////////////////

extern int ptm_join_words (gList& listIn, gString& sResult) ;

extern int ptm_cutoff_openclose (const char* strCut, gString& s) ;

extern int ptm_cutoff_brackets (gString& sName) ;

extern int ptm_name_string (gString& inName, int namesMask, const t_uchar** optUcsEqStrs, gString& outName) ;

extern char* ptm_name_str (const char* strInName, int namesMask, const t_uchar** optUcsEqStrs) ;

extern gList* ptm_new_name (gString& inName, gList* newNames, int namesMask, const t_uchar** optUcsEqStrs) ;

extern gList* ptm_sort_names (gList& listed, int mask) ;

extern gList* ptm_invert_list (gList& listed, int mask) ;

////////////////////////////////////////////////////////////
#endif //iMEDIA_X_H

