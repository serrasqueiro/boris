#ifndef I_AUDIOSTORED_X_H
#define I_AUDIOSTORED_X_H

#include "imaudio.h"


#define dump_im_prefix		"\t"
#define dump_imAudioPiece(p,buf,maxSize) \
		snprintf(buf,maxSize, \
			dump_im_prefix "OK: %c, notices: %d, warns: %d, errors: %d\n" \
			dump_im_prefix "bitrate: %s, %d Kbps\n" \
			dump_im_prefix "length: %u (ms); %u:%02u.%03u\n" \
			"%c\n" \
			"\n" \
			"\n" \
			, \
			ISyORn( p.IsOk() ), \
			p.nrNotices, p.nrWarns, p.nrErrors, \
			p.bitrateKind==imAudioPiece::bitrateConstant ? "constant" : (p.bitrateKind==imAudioPiece::bitrateVariable ? "variable" : "(unknown)"), \
			p.avgBitrateKbps, \
			p.milliseconds, \
			(p.milliseconds / 1000)/60, (p.milliseconds / 1000)%60, p.milliseconds%1000, \
			'\0' \
			);


typedef t_uint8 t_media_index;

////////////////////////////////////////////////////////////
class imTag : public gString {
public:
    imTag (char* aStr=nil) ;  // the title

    virtual ~imTag () ;

    // Public data-members
    gString sArtist;  // also known as Track Artist in ID3v2
    gString sAlbumArtist;

    gString sAlbum;

    gString sBandOrOrch;  // also known as Band/ Orchestra in ID3v2

    t_int16 recordingYear;  // not on ID3v1
    t_int16 releaseYear;  // just year, on ID3v1

    t_int8 trackId[ 4 ];  // track #0 of #1, disc #2 of #3

    // Get methods
    virtual gString& TrackDesc () ;

    virtual gString& DiscDesc () ;

protected:
    gString sData;

    int thisOneOfDescription (t_int8* values, int whence, gString& sResult) ;
};
////////////////////////////////////////////////////////////
class imTags : public gList {
public:
    imTags () ;

    virtual ~imTags () ;

    enum eTagKind {
	noTag,
	id3v1,
	id3v2,
	apeTag,
	unknownFormat = 4
    };

    // Get methods
    virtual bool IsOk () ;

    virtual int NTags () {
	return nOfTags;
    }

    // Set methods
    virtual bool SetTag (eTagKind whichTag, char* strTag) ;

protected:
    eTagKind tagKind;
    int nOfTags;
    imTag* tagIds;

private:
    // Operators,empty
    imTags (imTags& ) ; //empty
    imTags& operator= (imTags& ) ; //empty
};

////////////////////////////////////////////////////////////
class imAudioPiece : public gString {
public:
    // Public enums
    enum eBitrate {
	bitrateNotKnown,
	bitrateConstant,
	bitrateVariable,
	bitrateInvalid
    };

    // Constructors, etc
    imAudioPiece () ;

    virtual ~imAudioPiece () ;

    // Get methods
    virtual bool IsOk () ;

    virtual imTag* PreferredTag () ;

    // Set methods
    virtual bool SetErrorLine (gString& sLine) ;

    virtual int NormalizeErrors () ;

    // Public data-members
    imTags tags;
    imTag* builtTag;
    gList infos;
    gString sErrorLine;
    t_int16 nrNotices;
    t_int16 nrWarns;
    t_int16 nrErrors;

    t_uint32 milliseconds;
    t_int16 avgBitrateKbps;
    eBitrate bitrateKind;

private:
    // Operators,empty
    imAudioPiece (imAudioPiece& ) ; //empty
    imAudioPiece& operator= (imAudioPiece& ) ; //empty
};


////////////////////////////////////////////////////////////
// Audio-stored functions
////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
#endif //I_AUDIOSTORED_X_H

