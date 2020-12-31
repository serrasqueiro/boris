#ifndef I_LISTED_X_H
#define I_LISTED_X_H

#include <string.h>

#include "lib_imaudio.h"


#define IMA_NO_SUPPORT 0
#define IMA_BASIC_SUPPORT 1
#define IMA_NATIVE_SUPPORT 2
#define IMA_FULL_SUPPORT 3

#define IMA_SCHEMA 129

////////////////////////////////////////////////////////////
enum eFileType {
	e_unknown,
	e_textFile,
	e_otherTextFile,
	a_audioRaw = 8,
	e_audioCompressed,
	e_audioLossless,  // ...but compressed
	e_videoMpg2,
	e_videoVOB,  // usually MPG2
	e_videoBUP,  // Video VTS (on normal DVD)
	e_videoIFO,  // Video IFO (on normal DVD)
	e_videoMpg4,
	e_videoWMV,
	e_videoOther,
	e_anyImage = 32,
	e_imageUncompressed,  // MIME: image/jpeg
	e_imageCompressedJPG,  // jpg, jpeg
	e_imageLossless,  // gif, png
	e_imageOther,  // TIF, pcx, others (or even Thumbs.db)
	e_md5sum = 64,  // text-file with MD5 hashes (md5sum)
	e_md5suma = 65,  // text-file containing a faster checksum ('suma')
	e_mp3check,  // .mp3check.txt is a text-file containing the resume of one or more directories with mp3
	e_anyPlaylist = 128,
	e_playList_m3u = 129,
	e_playList_m3u8 = 130,
	e_playList_vpl = 131,
	e_isoGeneral = 192,
	e_isoCD,
	e_isoDVD_1L,  // one Layer DVD
	e_isoDVD_2L,
	e_fileUnsupported = 16383  // (2^14-1)
};


struct sFileTypeSupportDef {
    eFileType fileType;
    const char* mimeType;
    const char* strExts;
};


struct sFileTypeSupport {
    sFileTypeSupport ()
	: idxDef( 0 ),
	  supported( 0 ),
	  strExts( nil ),  // never allocated
	  userMime( nil ),  // allocated when used
	  extensions( nil ) {
	delete[] userMime;
    }

    ~sFileTypeSupport () {
	delete extensions;
    }

    t_uint16 idxDef;
    t_uint8 supported;

    const char* strExts;
    char* userMime;
    gList* extensions;

    // Methods
    int AddExtensions (const char* strExts) ;
};

////////////////////////////////////////////////////////////
// Media 'guess' files structs
////////////////////////////////////////////////////////////
struct sId3v1Bin {
#pragma pack(1)
    t_uchar sTag[ 3 ];
    t_uchar title[ 30 ];
    t_uchar artist[ 30 ];
    t_uchar album[ 30 ];
    char chrYear[ 4 ];
    t_uchar comment[ 30 ];  // ID3v1.1 allows last two bytes (16bits) to represent the track number
    t_uint8 uGenre;
};


struct sId3v1Text {
    sId3v1Text ()
	: genre( -1 ) {
    }

    ~sId3v1Text () {
    }

    gString sTitle;
    gString sArtist;
    gString sYear;
    gString sComment;
    t_int16 genre;
};


struct sGuessAudio {
    sGuessAudio ()
	: size( -1 ),
	  optID3v1( nil ),
	  level( 0 ) {
    }

    ~sGuessAudio () {
	delete optID3v1;
    }

    gString sOriPathName;
    gString sOriBaseName;

    int size;

    sId3v1Text* optID3v1;

    int level;  // unused
};

////////////////////////////////////////////////////////////
// Media listing structs
////////////////////////////////////////////////////////////
struct sListingConf {
    sListingConf ()
	: lastError( 0 ),
	  lastStat( nil ),
	  which( 0 ),
	  debug( 0 ) {
	ToDefault();
	InitExtensions();
    }

    ~sListingConf () {
    }

    static const char* defaultPreferredLists;
    static const sFileTypeSupportDef defaultExtensionSupport[];

    sFileTypeSupport supportedExtensions[ 256 ];  // hard limit does not hurt too much

    gList rawConf;
    gList hashedExtensions;

    gString sPreferredDirLists;  // first '.m3u', then '.pls', etc.

    int lastError;
    gFileStat* lastStat;  // Never deleted
    unsigned which;

    int debug;

    // Methods
    void ToDefault () ;

    bool InitExtensions () ;

    bool HashConfs () ;

    sFileTypeSupport& SupportByExt (const char* strExt) {
	t_uint8 supported( 0 );
	return SupportByExt( strExt, supported );
    }

    sFileTypeSupport& SupportByExtAnyCase (const char* strExt) {
	t_uint8 supported( 0 );
	char chr;
	char dup[ 48 ];

	dup[ 0 ] = 0;
	if ( strExt ) {
	    strncpy( dup, strExt, 47 );
	    dup[ 47 ] = 0;
	    for (int iter=0; (chr = dup[ iter ])!=0; iter++) {
		if ( chr>='A' && chr<='Z' ) {
		    dup[ iter ] += 32;
		}
	    }
	}
	return SupportByExt( dup, supported );
    }

    sFileTypeSupport& SupportByExt (const char* strExt, t_uint8& supported) ;

    const char* MimeByIndex (unsigned which) {
	return supportedExtensions[ which % 256 ].userMime;
    }

    const sFileTypeSupportDef& FindByFileType (eFileType fileType) {
	eFileType aType;
	for (int idx=0; (aType = defaultExtensionSupport[ idx ].fileType)!=e_fileUnsupported; idx++) {
	    if ( aType==fileType )
		return defaultExtensionSupport[ idx ];
	}
	return defaultExtensionSupport[ 0 ];
    }
};

////////////////////////////////////////////////////////////
// listed media functions
////////////////////////////////////////////////////////////

extern char* ima_basename (const char* strName) ;

extern char* ima_dirname (const char* strName) ;

extern int ima_file_extension (const char* strName, int dotFiles, gString& sExt) ;

extern gList* ima_ordered_dir (gDir& aDir) ;

extern gList* ima_open_dir (const char* strPath, gFileStat* ptrStat, sListingConf& aConf) ;

extern gList* ima_basic_open (int fdIn, gFileStat* ptrStat, sListingConf& aConf) ;

////////////////////////////////////////////////////////////
#endif //I_LISTED_X_H

