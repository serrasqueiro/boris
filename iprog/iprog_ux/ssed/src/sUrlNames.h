#ifndef SSED_URL_NAMES_X_H
#define SSED_URL_NAMES_X_H

#include "lib_iobjs.h"


////////////////////////////////////////////////////////////

enum eTextRefLine {
    e_TextNUL,
    e_TextNewLine = 1,
    e_HtmlNewLine,
    e_TextOther,
    e_TextEOF
};


struct sHtmlized {
    sHtmlized ()
	: lines( 0 ),
	  outLines( 0 ),
	  state( 0 ),
	  first( -1 ),
	  scripted( 0 ),
	  dumpType( e_Transparent ),
	  dumpNL( 0 ),
	  countExcluded( 0 ) {
    }

    ~sHtmlized () {
    }

    enum eDumpType {
	e_Transparent,
	e_NoScript
    };

    int lines;
    int outLines;
    int state;
    int first;
    int scripted;  // 0: not within <script> ... </script>

    eDumpType dumpType;

    int dumpNL;  // 1: new-line after tag <xyz>... ; 2: no <BR/> ever!

    gString iname;
    gList tags;

    gList excluded, fastExcluded;
    int countExcluded;

    gList recognizedTagsExtra;

    gList addBaseHREF;

    gString sHierarchyHexChars;
    gList hierarchyShown;

    void Reset () {
	lines = outLines = 0;
	state = 0;
	first = -1;
	iname.Reset();
	tags.Delete();
	addBaseHREF.iValue = 0;
	hierarchyShown.Delete();
    }
};

////////////////////////////////////////////////////////////

extern t_uint16 lineIndex;
extern t_uchar wholeLine[ 16 * 1024 ];
extern sHtmlized basicData;

////////////////////////////////////////////////////////////

extern int ssun_length (const char* aStr) ;

extern char* ssun_url_builder_name (const char* aStr, int mask) ;

extern char* ssun_url_builder_name_len (const char* aStr, int len, int mask) ;

extern int char_is_alpha (t_uchar uChr) ;

extern int char_is_alpha_eq (t_uchar uChr, int up, t_uchar& altChr) ;

////////////////////////////////////////////////////////////

extern gElem* find_value (int value, gElem* pElem) ;

extern gList* new_urlx_from_file (int inputHandle, bool checkURL) ;

extern gList* new_urlx_from_list (FILE* fOut, gString& sTempFile, gList& args, int acceptGarbage, bool checkURL) ;

extern int gen_unescape (gList& input, gList& output, int mask) ;

////////////////////////////////////////////////////////////

extern void html_dump_char (FILE* fOut, sHtmlized& htmlized, bool flushBuffer, t_uchar aChr) ;

extern void html_dump_string (FILE* fOut, gString& s) ;

////////////////////////////////////////////////////////////
#endif //SSED_URL_NAMES_X_H

