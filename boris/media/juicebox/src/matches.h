#ifndef MATCHES_X_H
#define MATCHES_X_H


#include "lib_imaudio.h"
#include "lib_imedia.h"


////////////////////////////////////////////////////////////
struct sSortBy {
    sSortBy ()
	: byCode( 0 ),
	  mainAscend( true ),
	  limit( MAX_INT_VALUE ) {
    }

    int byCode;
    bool mainAscend;
    int limit;
    gList codes;

    bool NonBasicSort () {
	return byCode > 0;
    }

    int SetCode (int code, bool ascending=true) {
	mainAscend = ascending;
	byCode |= code;
	codes.Add( (char*)(ascending ? ">" : "<") );
	codes.EndPtr()->me->iValue = byCode;
	return byCode;
    }
};

////////////////////////////////////////////////////////////
struct sSimilarMedia {
    sSimilarMedia () ;
    ~sSimilarMedia () ;

    gList names;
    gList baseNames;
    gList* pList;

    gList stats[ 10 ];

    void AddNames (gList& copy) ;

    bool Analysis () ;

    void Show () ;
};

////////////////////////////////////////////////////////////
// Useful functions
////////////////////////////////////////////////////////////

extern gList* new_matched_file (gString& name, int mask) ;

extern gList* new_files_there (const char* strPath, gList& candidates, gString& usedExtension) ;

extern iEntry* new_files_at_dir (const char* strPath, iEntry& extensions) ;

extern gList* new_rename_rules (gList& input, int mask, gString& returnMsg) ;

extern int add_to_list (gElem* ptrIn, gList& out) ;

extern gList* copy_names (gList& copy) ;

extern gList* join_lists (gList& L1, gList& L2) ;

extern char* does_match_extension (gString& s, const char* strExt) ;

extern char* uri_to_ucs4 (gString& sURI, bool forceBackslash) ;

extern char* str_slash (const char* str, char& resultChr) ;

extern char* str_simpler_path (char* result) ;

extern gString* trim_filepath (const char* strInput) ;

extern int sub_str_compare (const char* str, const char* subStr) ;

extern char* slash_or_backslash (const char* str) ;

////////////////////////////////////////////////////////////

extern iString* new_stat_criteria (sSortBy& criteria, t_uint16 type, gString& sFilePath) ;

////////////////////////////////////////////////////////////
#endif //MATCHES_X_H

