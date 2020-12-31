#ifndef iSUBS_X_H
#define iSUBS_X_H


#include "lib_iobjs.h"

////////////////////////////////////////////////////////////
struct sSubTime {
    sSubTime ()
	: hh( 0 ),
	  mm( 0 ),
	  ss( 0 ),
	  mi( 0 ) {
    }

    ~sSubTime () {
    }

    t_int16 hh;
    t_uint8 mm;
    t_uint8 ss;
    t_uint16 mi;

    int Millis () {
	return
	    (hh * 3600 +
	     mm * 60 +
	     ss) * 1000 +
	    mi;
    }

    void Adjust (int millisecs) {
	int ms = Millis() + millisecs;
	int v = ms;
	if ( v < 0 ) v = -ms;
	mi = v % 1000;
	v /= 1000;
	ss = v % 60;
	v /= 60;
	mm = v % 60;
	v /= 60;
	hh = v;
    }
};

////////////////////////////////////////////////////////////
class SubEntry : public gList {
public:
    SubEntry (int value=0) ;

    SubEntry (gString& talk, int value=0) ;

    virtual ~SubEntry () ;

    // Public data-members
    sSubTime fromTo[ 2 ];
    gString sTalk;  // only used when the whole sentence is built for e.g. string-search!
    gString sUpTalk;  // uppercase string

    // Get methods
    virtual char* TimeString () {
	return Str( 2 );
    }

    virtual char* Talk (int index) {
	return Str( index+2 );
    }

    virtual int TalkLines () {
	return N() ? (int)N()-2 : -1;
    }

    // Special methods
    virtual int ConvSubsFromTo (gString& line, char separator='>') ;

    virtual int ToSubTime (const char* blankSub, sSubTime& subTime) ;

    virtual void SubUpString (gString& sUp) {
	sUp.UpString();  // not fully ISO-8859-1 Latin1, but ok...
    }

    // Set methods
    virtual bool AdjustSecs (int secs) {
	if ( secs >= (MAX_INT_VALUE / 1000) )
	    return false;
	return AdjustMillis( secs * 1000 );
    }

    virtual bool AdjustMillis (int millisecs) {
	fromTo[ 0 ].Adjust( millisecs );
	fromTo[ 1 ].Adjust( millisecs );
	return fromTo[ 0 ].Millis() <= fromTo[ 1 ].Millis();
    }

    virtual void CopySubTimeFromTo (sSubTime& from, sSubTime& to) {
	CopySubTime( 0, from );
	CopySubTime( 1, to );
    }

    virtual bool CopySubTime (int idx, sSubTime& fromOrTo) {
	t_uint8 which( idx!=0 );
	fromTo[ which ].hh = fromOrTo.hh;
	fromTo[ which ].mm = fromOrTo.mm;
	fromTo[ which ].ss = fromOrTo.ss;
	fromTo[ which ].mi = fromOrTo.mi;
	return idx>=0 && idx<2;
    }

    virtual char* BuildLineTalk (bool doUpTalk=true) {
	char* strResult;
	sTalk.SetEmpty();
	strResult = thisBuildLineTalk( sTalk );
	if ( doUpTalk ) {
	    sUpTalk = sTalk;
	    SubUpString( sUpTalk );
	}
	return strResult;
    }

protected:
    char* thisBuildLineTalk (gString& sLine) ;

private:
    // Operators,empty
    SubEntry (SubEntry& ) ; //empty
    SubEntry& operator= (SubEntry& ) ; //empty
};

////////////////////////////////////////////////////////////
class Subs : public gList {
public:
    Subs (char* aStr=nil) ;

    virtual ~Subs () ;

    // Set methods
    virtual int AddFromLine (const t_uchar* uBuf) ;

    virtual void BuildTalks (bool doUpTalk=true) {
	gElem* ptrElem( StartPtr() );
	for ( ; ptrElem; ptrElem=ptrElem->next) {
	    ((SubEntry*)ptrElem->me)->BuildLineTalk( doUpTalk );
	}
    }

protected:
    SubEntry entry;  // the last one

    int thisAddEntry (SubEntry& one) ;

private:
    // Operators,empty
    Subs (Subs& ) ; //empty
    Subs& operator= (Subs& ) ; //empty
};

////////////////////////////////////////////////////////////
// useful functions
////////////////////////////////////////////////////////////

extern int isubs_dump_subs (FILE* fOut, Subs& subs) ;

extern int isubs_dump_file (FILE* fOut, const char* strFile, Subs& subs) ;

////////////////////////////////////////////////////////////
#endif //iSUBS_X_H

