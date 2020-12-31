#ifndef ID3V1GENRES_X_H
#define ID3V1GENRES_X_H


#include "lib_iobjs.h"

////////////////////////////////////////////////////////////
enum eID3v1WhichGenre {
    e_M_number,
    e_M_GenreName,
    e_M_GenreAbbrev
};

////////////////////////////////////////////////////////////

extern const char* v1_genre_name (eID3v1WhichGenre which, t_uint8 uGenre) ;

extern int v1_dump_genres (FILE* fOut, int mask) ;


////////////////////////////////////////////////////////////
#endif //ID3V1GENRES_X_H

