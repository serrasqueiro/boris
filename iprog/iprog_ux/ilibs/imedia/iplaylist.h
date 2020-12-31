#ifndef iPLAYLIST_X_H
#define iPLAYLIST_X_H

#include "ilist.h"

#include "inames.h"

////////////////////////////////////////////////////////////
struct sVplPlaylist {
    sVplPlaylist () ;

    ~sVplPlaylist () ;

    int nEntries;
    char splitChr;
    char attrOrder[ 64 ];
    char comment[ 40 ];  // "#VUPlayer playlist"
    gList** entries;   // raw-list (with 0x01)
    gList parts;      // each entry is a list

    gList* EntryPtr (int idx) ;

    char* Entry (int idx) ;

    void ReInit () ;

    void Release () ;

    void Show (bool doShowAll=true) ;
};

////////////////////////////////////////////////////////////
struct sM3uPlaylist {
    sM3uPlaylist ()
	: nEntries( 0 ) {
	memset( earlyHeader, 0x0, 4 );
    }

    ~sM3uPlaylist () {
    }

    static const t_uint8 defaultM3U8header[ 4 ];
    t_uint8 earlyHeader[ 4 ];

    int nEntries;
};

////////////////////////////////////////////////////////////

extern int ipl_vpl_from_file (const char* strFile, sVplPlaylist& playlist) ;

extern int ipl_vpl_from_raw (gList& textEntries, sVplPlaylist& playlist) ;

extern gElem* ipl_vpl_raw_to_parts (sVplPlaylist& playlist) ;


extern int ipl_find_any_slash (gString& s) ;

extern int ipl_find_any_slash_pos (gString& s, int& pos) ;

extern int ipl_add_strings_from (gElem* ptrFrom, gList& result) ;

////////////////////////////////////////////////////////////
#endif //iPLAYLIST_X_H

