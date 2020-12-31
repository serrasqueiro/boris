// id3v1genres.cpp


// Based on former:
//	gTagMp3Genres.cpp (formerly gdcTagMp3Genres.cpp of libgdcdb)


#include "id3v1genres.h"

////////////////////////////////////////////////////////////
struct sGenreID3v1 {
    t_int16 id;
    const char* name;
    const char* abbrev;
};

////////////////////////////////////////////////////////////
const char* v1_genre_id (eID3v1WhichGenre which, t_uint8 uGenre, int& id)
{
 static const sGenreID3v1 tagMp3Genres[ 256 ]={
	{1,	"Blues",	nil},
	{2,	"Classic Rock",	"Classic"},
	{3,	"Country",	nil},
	{4,	"Dance",	nil},
	{5,	"Disco",	nil},
	{6,	"Funk",	nil},
	{7,	"Grunge",	nil},
	{8,	"Hip-Hop",	nil},
	{9,	"Jazz",	nil},
	{10,	"Metal",	nil},
	{11,	"New Age",	nil},
	{12,	"Oldies",	nil},
	{13,	"Other",	nil},
	{14,	"Pop",	nil},
	{15,	"R&B",	nil},
	{16,	"Rap",	nil},
	{17,	"Reggae",	nil},
	{18,	"Rock",	nil},
	{19,	"Techno",	nil},
	{20,	"Industrial",	nil},
	{21,	"Alternative",	"Alt."},
	{22,	"Ska",	nil},
	{23,	"Death Metal",	nil},
	{24,	"Pranks",	nil},
	{25,	"Soundtrack",	nil},
	{26,	"Euro-Techno",	nil},
	{27,	"Ambient",	nil},
	{28,	"Trip-Hop",	nil},
	{29,	"Vocal",	nil},
	{30,	"Jazz+Funk",	nil},
	{31,	"Fusion",	nil},
	{32,	"Trance",	nil},
	{33,	"Classical",	nil},
	{34,	"Instrumental",	"Instrum."},
	{35,	"Acid",	nil},
	{36,	"House",	nil},
	{37,	"Game",	nil},
	{38,	"Sound Clip",	"Clip"},
	{39,	"Gospel",	nil},
	{40,	"Noise",	nil},
	{41,	"Alternative Rock",	"Alt. Rock"},
	{42,	"Bass",	nil},
	{43,	"Soul",	nil},
	{44,	"Punk",	nil},
	{45,	"Space",	nil},
	{46,	"Meditative",	nil},
	{47,	"Instrumental Pop",	"Instrum."},
	{48,	"Instrumental Rock",	"Instrum."},
	{49,	"Ethnic",	nil},
	{50,	"Gothic",	nil},
	{51,	"Darkwave",	nil},
	{52,	"Techno-Industrial",	"Techno-Indust."},
	{53,	"Electronic",	nil},
	{54,	"Pop-Folk",	nil},
	{55,	"Eurodance",	nil},
	{56,	"Dream",	nil},
	{57,	"Southern Rock",	"Southern"},
	{58,	"Comedy",	nil},
	{59,	"Cult",	nil},
	{60,	"Gangsta",	nil},
	{61,	"Top 40",	"Top"},
	{62,	"Christian Rap",	"Rap"},
	{63,	"Pop/Funk",	nil},
	{64,	"Jungle",	nil},
	{65,	"Native American",	"Native"},
	{66,	"Cabaret",	nil},
	{67,	"New Wave",	nil},
	{68,	"Psychedelic",	nil},
	{69,	"Rave",	nil},
	{70,	"Showtunes",	nil},
	{71,	"Trailer",	nil},
	{72,	"Lo-Fi",	nil},
	{73,	"Tribal",	nil},
	{74,	"Acid Punk",	"Acid"},
	{75,	"Acid Jazz",	"Acid"},
	{76,	"Polka",	nil},
	{77,	"Retro",	nil},
	{78,	"Musical",	nil},
	{79,	"Rock & Roll",	"Rock"},
	{80,	"Hard Rock",	nil},
	{81,	"Folk",	nil},
	{82,	"Folk/Rock",	nil},
	{83,	"National Folk",	"Folk"},
	{84,	"Swing",	nil},
	{85,	"Fast Fusion",	"Fusion"},
	{86,	"Bebop",	nil},
	{87,	"Latin",	nil},
	{88,	"Revival",	nil},
	{89,	"Celtic",	nil},
	{90,	"Bluegrass",	nil},
	{91,	"Avantgarde",	nil},
	{92,	"Gothic Rock",	"Gothic"},
	{93,	"Progressive Rock",	nil},
	{94,	"Psychadelic",	nil},
	{95,	"Symphonic Rock",	"Symphonic"},
	{96,	"Slow Rock",	nil},
	{97,	"Big Band",	nil},
	{98,	"Chorus",	nil},
	{99,	"Easy Listening",	"Easy list."},
	{100,	"Acoustic",	nil},
	{101,	"Humour",	nil},
	{102,	"Speech",	nil},
	{103,	"Chanson",	nil},
	{104,	"Opera",	nil},
	{105,	"Chamber Music",	"Chamber"},
	{106,	"Sonata",	nil},
	{107,	"Symphony",	nil},
	{108,	"Booty Bass",	"Booty"},
	{109,	"Primus",	nil},
	{110,	"Porn Groove",	"Porn"},
	{111,	"Satire",	nil},
	{112,	"Slow Jam",	"Slow"},
	{113,	"Club",	nil},
	{114,	"Tango",	nil},
	{115,	"Samba",	nil},
	{116,	"Folklore",	nil},
	{117,	"Ballad",	nil},
	{118,	"Power Ballad",	"Ballads"},
	{119,	"Rhythmic Soul",	"Rhythmic"},
	{120,	"Freestyle",	nil},
	{121,	"Duet",	nil},
	{122,	"Punk Rock",	"Punk"},
	{123,	"Drum Solo",	"Drum"},
	{124,	"A Cappella",	nil},
	{125,	"Euro-House",	nil},
	{126,	"Dance Hall",	"Dance"},
	{127,	"Goa",	nil},
	{128,	"Drum & Bass",	nil},
	{129,	"Club-House",	nil},
	{130,	"Hardcore",	nil},
	{131,	"Terror",	nil},
	{132,	"Indie",	nil},
	{133,	"BritPop",	nil},
	{134,	"Negerpunk",	nil},
	{135,	"Polsk Punk",	"Polsk"},
	{136,	"Beat",	nil},
	{137,	"Christian Gangsta Rap",	"Christian"},
	{138,	"Heavy Metal",	"Metal"},
	{139,	"Black Metal",	"Metal"},
	{140,	"Crossover",	nil},
	{141,	"Contemporary Christian",	"Christian"},
	{142,	"Christian Rock",	"Rock"},
	{143,	"Merengue",	nil},
	{144,	"Salsa",	nil},
	{145,	"Thrash Metal",	"Thrash"},
	{146,	"Anime",	nil},
	{147,	"JPop",	nil},
	{148,	"Synthpop",	nil},
	{149,	nil,	nil},
	{150,	nil,	nil},
	{151,	nil,	nil},
	{152,	nil,	nil},
	{153,	nil,	nil},
	{154,	nil,	nil},
	{155,	nil,	nil},
	{156,	nil,	nil},
	{157,	nil,	nil},
	{158,	nil,	nil},
	{159,	nil,	nil},
	{160,	nil,	nil},
	{161,	nil,	nil},
	{162,	nil,	nil},
	{163,	nil,	nil},
	{164,	nil,	nil},
	{165,	nil,	nil},
	{166,	nil,	nil},
	{167,	nil,	nil},
	{168,	nil,	nil},
	{169,	nil,	nil},
	{170,	nil,	nil},
	{171,	nil,	nil},
	{172,	nil,	nil},
	{173,	nil,	nil},
	{174,	nil,	nil},
	{175,	"Pop",	nil},
	{176,	nil,	nil},
	{177,	nil,	nil},
	{178,	nil,	nil},
	{179,	nil,	nil},
	{180,	nil,	nil},
	{181,	nil,	nil},
	{182,	nil,	nil},
	{183,	nil,	nil},
	{184,	nil,	nil},
	{185,	nil,	nil},
	{186,	nil,	nil},
	{187,	nil,	nil},
	{188,	nil,	nil},
	{189,	nil,	nil},
	{190,	nil,	nil},
	{191,	nil,	nil},
	{192,	nil,	nil},
	{193,	nil,	nil},
	{194,	nil,	nil},
	{195,	nil,	nil},
	{196,	nil,	nil},
	{197,	nil,	nil},
	{198,	nil,	nil},
	{199,	nil,	nil},
	{200,	nil,	nil},
	{201,	"New Wave",	nil},
	{202,	nil,	nil},
	{203,	nil,	nil},
	{204,	nil,	nil},
	{205,	nil,	nil},
	{206,	nil,	nil},
	{207,	nil,	nil},
	{208,	nil,	nil},
	{209,	nil,	nil},
	{210,	nil,	nil},
	{211,	nil,	nil},
	{212,	nil,	nil},
	{213,	nil,	nil},
	{214,	nil,	nil},
	{215,	nil,	nil},
	{216,	nil,	nil},
	{217,	nil,	nil},
	{218,	nil,	nil},
	{219,	nil,	nil},
	{220,	nil,	nil},
	{221,	nil,	nil},
	{222,	nil,	nil},
	{223,	nil,	nil},
	{224,	nil,	nil},
	{225,	nil,	nil},
	{226,	"Brasilian",	nil},
	{227,	nil,	nil},
	{228,	nil,	nil},
	{229,	nil,	nil},
	{230,	nil,	nil},
	{231,	nil,	nil},
	{232,	nil,	nil},
	{233,	nil,	nil},
	{234,	nil,	nil},
	{235,	nil,	nil},
	{236,	nil,	nil},
	{237,	nil,	nil},
	{238,	"Pop",	nil},
	{0,	nil,	nil},
	{240,	nil,	nil},
	{241,	nil,	nil},
	{242,	nil,	nil},
	{243,	nil,	nil},
	{244,	nil,	nil},
	{245,	nil,	nil},
	{246,	nil,	nil},
	{247,	nil,	nil},
	{248,	nil,	nil},
	{249,	nil,	nil},
	{250,	nil,	nil},
	{251,	nil,	nil},
	{252,	nil,	nil},
	{253,	nil,	nil},
	{254,	nil,	nil},
	{255,	nil,	nil},
	{0,	"Teen Pop",	"Teen Pop"}
 };

 id = (int)tagMp3Genres[ uGenre ].id;

 switch ( which ) {
 case e_M_GenreName:
     return tagMp3Genres[ uGenre ].name;

 case e_M_GenreAbbrev:
     if ( tagMp3Genres[ uGenre ].abbrev==nil ) {
	 return tagMp3Genres[ uGenre ].name;
     }
     return tagMp3Genres[ uGenre ].abbrev;

 case e_M_number:
     return nil;

 default:
     ASSERTION_FALSE("Genre?");
 }

 return nil;
}


const char* v1_genre_name (eID3v1WhichGenre which, t_uint8 uGenre)
{
 int id;
 return v1_genre_id( which, uGenre, id );
}

////////////////////////////////////////////////////////////
int v1_dump_genres (FILE* fOut, int mask)
{
 int iter( 0 ), id;
 const char* name;
 const char* abbrev;

 if ( fOut==nil ) return -1;

 for (iter=0; iter<256; iter++) {
     v1_genre_id( e_M_number, iter, id );
     name = v1_genre_name( e_M_GenreName, iter );
     abbrev = v1_genre_name( e_M_GenreAbbrev, iter );
     if ( name==nil ) continue;
     if ( mask ) {
	 if ( mask & 4 ) {
	     fprintf(fOut,"%d\t%s\t%s\n",
		     iter,
		     name,
		     abbrev);
	 }
	 else {
	     fprintf(fOut,"%d\t%s\n",
		     iter,
		     name);
	 }
     }
     else {
	 fprintf(fOut,"%d\t%s\n",
		 iter,
		 abbrev);
     }
 }
 return 0;
}

////////////////////////////////////////////////////////////

