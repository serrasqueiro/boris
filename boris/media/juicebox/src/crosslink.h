#ifndef CROSSLINK_X_H
#define CROSSLINK_X_H


#include "lib_imaudio.h"
#include "lib_imedia.h"  // ISO-8859-1, etc


#define JCL_maxNumberFilesPerHFS 65500

#define JCL_DEBUG_LEVEL (mylog.dbgLevel)


#if 0
#define J_DUMP(args...) { \
		printf("-> %s:%d: ",__FILE__,__LINE__); \
		printf(args); \
		printf("\n"); \
		}
#else
#define J_DUMP(args...) ;
#endif

#define oprint(args...) { if ( mylog.dbgLevel>6 ) printf(args); }

////////////////////////////////////////////////////////////
// Crosslink structs
////////////////////////////////////////////////////////////
enum ePlaylistType {
    e_pl_Unknown,
    e_pl_M3U,
    e_pl_VPL,
    e_pl_PLS,
    e_pl_raw
};


struct sPairedList {
    gString name;
    gList a;
    gList b;
};


struct sSubstedPaths {
    sSubstedPaths ()
	: freerun( 0 ) {
    }

    ~sSubstedPaths () {
    }

    int freerun;

    gList sPrefixSubsts;	// Triplets of { path, alias, relative },
				// e.g. { \images\music\ /net/mach/huge/ ../music }

    gList nonArtistDirs;	// e.g. { "mp3 snip" }

    void Release () {
	sPrefixSubsts.Delete();
	nonArtistDirs.Delete();
    }

    int AddNonArtistDir (const char* strPath) {
	return SplitSemicolonFromString( strPath, nonArtistDirs );
    }

    int NonArtistDirsFromString (const char* semicolonSeparatedStr) {
	gString aString( (char*)semicolonSeparatedStr );
	aString.SplitAnyChar( ';', nonArtistDirs );
	return (int)nonArtistDirs.N();
    }

    int FindNonArtistDir (const char* strFullPath, int& length) ;

    int SplitSemicolonFromString (const char* str, gList& listed) {
	return thisSplitSemicolonFromString( str, 0, listed );
    }

    int thisSplitSemicolonFromString (const char* str, int mask, gList& listed) ;
};


struct sCrossLinkConf {
    sCrossLinkConf (const char* strOutDir=nil)
	: sBaseDir( (char*)strOutDir ),
	  showProgress( false ),
	  complainFileCheck( true ),
	  dumpCommandsToErrorFile( true ),
	  appendedOutput( false ),
	  byOccurrence( true ),
	  freeCounter( 0 ),
	  maxCounter( 0 ),
	  volumeType( e_CD ),
	  listing( e_Numbered ),
	  usedNameFirst( e_TitleFirst ),
	  useBlanksMask( 1 ),
	  totalSeconds( 0 ),
	  blockSize( 10*1024 ),
	  maxNumberFiles( JCL_maxNumberFilesPerHFS ) {
	strcpy(digitsFormat, "%03d");
	InitOS();
    }

    ~sCrossLinkConf () {
    }

    enum eVolumeType {
	e_Invalid,
	e_CD,
	e_DVD_4dot7,
	e_DVD_dlayer
    };

    enum eOScommand {
	e_OS_Cat,
	e_OS_Copy,
	e_OS_ln,
	e_OS_unused
    };

    enum eListing {
	e_Numbered,	// No dir, but numbers
	e_NoNumber,	// No dir, no numbers
	e_unused_listing
    };

    enum eUsedTitle {
	e_UnusedAlways = 0,
	e_FileFirst,
	e_TitleFirst = 3  // Try title names, first, when possible...
    };

    gString sBaseDir;

    bool showProgress;
    bool complainFileCheck;
    bool dumpCommandsToErrorFile;
    bool appendedOutput;
    bool byOccurrence;

    int freeCounter;
    int maxCounter;
    eVolumeType volumeType;
    eListing listing;
    eUsedTitle usedNameFirst;
    int useBlanksMask;  // 0: '_' instead of blanks; otherwise: blanks instead of ' '

    int totalSeconds;
    t_uint32 blockSize;  // in octets (multiple of 1024)
    char digitsFormat[ 12 ];
    int maxNumberFiles;

    gString sCommandToError;

    sSubstedPaths substed;

    gString sOScommands[ e_OS_unused ];

    int InitOS () ;

    int SetVolumeType (eVolumeType aType) ;

    int ApproxMinutes (int secsToIgnore=0) {
	return (totalSeconds / 60) + ((totalSeconds % 60) > secsToIgnore);
    }
};


struct sPlayCheck {
    sPlayCheck ()
	: atNames( nil ) {
    }

    ~sPlayCheck () {
	delete atNames;
    }

    gList* atNames;

    // For checking...
    gString lastInputName;
    gString lastBasename;
    gString proposedName;

    void Release () {
	delete atNames; atNames = nil;
    }

    int CheckItem (int numbersGenCode, gList* ptrItem, gString& sName) ;
};


struct sInPlaylist {
    sInPlaylist ()
	: type( e_pl_Unknown ) {
	checks.atNames = new gList;
    }

    ~sInPlaylist () {
    }

    ePlaylistType type;
    gString playlistFile;
    gList items;

    sPlayCheck checks;
};


////////////////////////////////////////////////////////////

extern int jcl_absolute_path (const char* str) ;
extern int jcl_wipe_tail_cr_nl (char* strResult) ;

extern char* jcl_get_extension (gString& sIn, int mask, gString& sExt) ;

extern char* jcl_extension (gString& sIn, bool downCase) ;

extern ePlaylistType jcl_which_type (gString& sFile, gString& sExt) ;

extern int jcl_tracknr_indication (const char* str, int& position) ;

extern int jcl_numbered_entries (const char* strPath, int mask, int& error) ;

extern int jcl_config (const char* strConfFile, sCrossLinkConf& linkConf) ;

extern int jcl_add_pname (gString& myStr, gList& result) ;
extern int jcl_add_once (const char* aStr, gList& result) ;
extern int jcl_add_once_mask (const char* aStr, int value, int mask, gList& result) ;

extern int jcl_trim_line (gString& sLine, int mask) ;

extern int jcl_read_playlist (sCrossLinkConf& linkConf, gString& sFile, sInPlaylist& plays) ;

extern int jcl_anyread_playlist (sCrossLinkConf& linkConf, gString& sFile, sInPlaylist& plays) ;
extern int jcl_read_rawlist (sCrossLinkConf& linkConf, gList& lines, sInPlaylist& plays) ;

extern int jcl_build_crosslink (sCrossLinkConf& linkConf, sInPlaylist& plays) ;

extern int jcl_build_copy (sCrossLinkConf& linkConf, sInPlaylist& plays) ;

extern int jcl_dump (sCrossLinkConf& linkConf, sInPlaylist& plays, gList& outlist) ;

extern int jcl_generated (sCrossLinkConf& linkConf, sInPlaylist& plays, int mask, gList& outlist) ;


extern gString* jcl_add_ordered (const char* strName, int value, gList& listed) ;

extern int jcl_add_sort_paired (const char* strFile, bool doSort, int value, sPairedList& paired) ;

extern int jcl_add_paired (const char* strFile, int value, sPairedList& paired) ;

////////////////////////////////////////////////////////////

extern int file_status_code (gString& aFilePath, const bool isDir, t_uint32& uxPerm) ;

////////////////////////////////////////////////////////////
#endif //CROSSLINK_X_H

