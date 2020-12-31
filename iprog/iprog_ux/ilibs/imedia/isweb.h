#ifndef iISWEB_X_H
#define iISWEB_X_H

#include "lib_imedia.h"


#define isw_print(level,args...) { \
		 if ( level>=0 ) { \
			if ( LogStream() ) fprintf(LogStream(),args); \
		 } \
		}

#if defined(DEBUG) || defined(DEBUG_MIN)
#define ISW_DEBUG(args...) printf(args)
#else
#define ISW_DEBUG(args...) ;
#endif

#ifdef ISW_DO_SHOW_MEM
#define ISW_SHOW_MEM(args...) fprintf(stderr,args)
#else
#define ISW_SHOW_MEM(args...) ;
#endif


#define ISW_SINGLE_QUOTE '\''

#define ISW_MASK_ERR_NOTE   -10

#define ISW_MASK_DONT_TOUCH  -9
#define ISW_MASK_QUOTED      -8
#define ISW_MASK_NO_STR_FIX  ISW_MASK_QUOTED

#define ISW_MASK_EMPTY       -2

////////////////////////////////////////////////////////////
struct sHtmlStatus {
    sHtmlStatus ()
	: quoteState( 0 ),
	  singleQuoted( 0 ),
	  doubleQuoted( 0 ),
	  keyState( 0 ),
	  metaEquiv( 0 ),
	  builtCharset( 0 ),
	  openPre( false ) {
    }

    int quoteState, singleQuoted, doubleQuoted;
    int keyState;
    int metaEquiv;
    int builtCharset;
    bool openPre;

    gList metaHttpEquiv;
    gList logIncompleteQuote;

    bool ValidCharset () {
	return builtCharset>0;
    }

    int CharsetCode () {
	return builtCharset;
    }

    bool CharsetISO8859_Western () {
	return builtCharset==885901;
    }

    bool IsCharsetUTF () {
	return builtCharset>=800 && builtCharset<=1600;
    }

    int CharsetFamily () {
	return builtCharset<=0 ? 0 : builtCharset/100;
    }

    // Set methods

    int AddHttpEquiv (gString* newStr, const char* strTag, int iLine, int mask=0) ;

    int SetAnyCharset (gString& sCharset, const char* strCharset, int isoCharset, int mask=0) ;
};

////////////////////////////////////////////////////////////
class gWebChassis : public gList {
public:
    gWebChassis () ;

    virtual ~gWebChassis () ;

    // Public data-members
    gList opts;

    bool trimLeftAlways;  // Default: true
    bool suppressScript;  // Default: false
    bool keywordBuild;    // Default: true

    gList built;  // filled when keywordBuild=true
    gString sLastAdded;

    // ...and states:
    sHtmlStatus htmlStatus;

    // Get methods
    virtual FILE* LogStream () {
	return logStream;
    }

    virtual int ScriptState () {
	return scriptState;
    }

    // Set methods
    virtual gElem* AddMe (gStorage* newObj, int iLine, int mask=0) ;

    // Useful methods

    int AddToList (gList& list, char* aStr, int iLine, int optValue=0) ;
    int AddToList (gList& list, gString& s, int iLine, int optValue=0) ;

    int AddToListRemnant (gList& list, gString& s, int iLine, int optValue=0) ;

    int AddHtmlString (gList& list, gString& s, int iLine, int mask=0) ;

    int AdjustScriptLines (gString& s, int posStart, int posEnd, int mask=0) ;

    virtual int Finit (gList& list) ;

    int KeydumpFlush (gList& items) ;

protected:
    int scriptState;
    FILE* logStream;
    gStorage* ptrLastStr;

    // Help methods
    int thisSpecialUpString (gString& s, int& quoteState) ;

    int thisAddHtml (gString* words, int iLine=-1) ;

private:
    // Operators,etc
    gWebChassis (gWebChassis& ) ;  //empty
    gWebChassis& operator= (gWebChassis& ) ;  //empty
};

////////////////////////////////////////////////////////////

extern int isw_simple_html_filter_list (gList& inHTML, gList& outHTML, gWebChassis& chassis) ;

extern int isw_simple_html_filter_file (const char* strIn, gList& listHTML, gWebChassis& chassis) ;

extern int isw_html_keydump (gList& listHTML, gWebChassis& chassis) ;

////////////////////////////////////////////////////////////
#endif //iISWEB_X_H

