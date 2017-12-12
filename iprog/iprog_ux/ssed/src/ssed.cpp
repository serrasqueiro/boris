// ssed
//
// A simple sed, with multiple commands.


#define thisProgramVersion "Version 1.3"
#define thisProgramCopyright "Prized Season & Sons"
#define thisProgramYear 2017
#define thisProgramCopy "This is free software (GPL)\n\
There is no warranty, not even for MERCHANTABILITY or\n\
FITNESS FOR A PARTICULAR PURPOSE."

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "lib_imedia.h"
#include "lib_ilog.h"

#include "sIO.h"
#include "log.h"


#ifdef iDOS_SPEC
#include <windows.h>
#endif //iDOS_SPEC

#include "sUrlNames.h"
#include "sNorm.h"
#include "sHistogram.h"
#include "sDates.h"
#include "sBatch.h"


#define UNDO_PREFIX "."
#define UNDO_SUFFIX ".tmp"
#define SSED_CONFIG_FILE "SSED_CONFIG"
#define SSED_CONFIG_NAME "ssed.config"


// Globals
gSLog lGlobLog;
IMEDIA_DECLARE;


#define clr_string(args...) clean_string( __LINE__, args )

////////////////////////////////////////////////////////////
// HTML utility macros
////////////////////////////////////////////////////////////

#ifdef OPTIMIZED_SPEED

#define h_out_chr(aChr) { if ( fOut!=stdout ) { if ( aChr ) { printf("%c", aChr); } } }

#define h_out_str(s) { if ( fOut!=stdout ) { printf("%s", s.Str()); } }

#else

#define h_out_chr(aChr) html_dump_char( fOut, htmlized, (aChr)=='\n', aChr )

#define h_out_str(s) html_dump_string( fOut, s )

#endif //OPTIMIZED_SPEED


// #define hprint(fOut, args...) fprintf(fOut, args)
#define hprint(fOut, args...) ;


void extra_unis (gUniCode& uni) ;

////////////////////////////////////////////////////////////
struct sOpts {
    sOpts ()
	: substituteSimilarSlashes( false ),
	  linesSubsts( 0 ),
	  countSubsts( 0 ),
	  debug( 0 ),
	  ptrElem( nil ) {
    }

    ~sOpts () {
    }

    bool substituteSimilarSlashes;

    gList expr1;
    gList expr2;

    int linesSubsts;
    int countSubsts;
    int debug;

    // Opt-ins
    gElem* ptrElem;

    // Methods
    void ResetCounters () {
	linesSubsts = countSubsts = 0;
    }

    int Subst (gString& sLine, gString& to) {
	int code( 0 );
	int pos;
	char chr;
	gElem* ptrPair( expr2.StartPtr() );
	gString sCopy;

	for (ptrElem=expr1.StartPtr(); ptrElem; ptrElem=ptrElem->next, ptrPair=ptrPair->next) {
	    int loopFree( 0 );
	    gString myExpr( ptrElem->Str() );
	    pos = ptrElem->me->iValue;
	    if ( pos>0 ) {
		// Todo, a fixed position
	    }
	    else {
		for (int idx=1, keepIdx=1, len=(int)sLine.Length(); idx<=len; idx++) {
		    pos = 0;
		    chr = sLine[ keepIdx = idx ];

		    if ( ++loopFree >= MAX_INT16_I ) break;

		    if ( chr==myExpr[ 1 ] ) {
			for (pos=2; pos<=(int)myExpr.Length(); pos++) {
			    idx++;
			    if ( sLine[ idx ]!=myExpr[ pos ] ) {
				pos = -1;
				break;
			    }
			}
			if ( pos<=-1 ) {
			    // We didn't find the pattern 'myExpr' in it
			    sCopy.Add( chr );
			    idx = keepIdx;
			}
			else {
			    gString substed( ptrPair->Str() );
			    sCopy.AddString( substed );
			    code++;
			}
		    }
		    else {
			sCopy.Add( chr );

			// Note: old code was not prone to replacing e.g. "Ab" by "AbC"
		    }

		    DBGPRINT_MIN("DBG: %d DOING, pos=%d: %c\t{%s}  {%s}\n",
				 loopFree,
				 pos, chr,
				 sCopy.Str(),
				 sLine.Str( idx ));
		}

		if ( loopFree >= MAX_INT16_I )
		    to = sLine;
		else
		    to = sCopy;
	    }
	}
	linesSubsts += (code>0);
	countSubsts += code;
	return code;
    }
};


struct sOptSed {
    sOptSed ()
	: isVerbose( false ),
	  isVeryVerbose( false ),
	  doAll( false ),
	  allowUTF8( true ),
	  sedLevel( 0 ),
	  zValue( -1 ) {
    }

    ~sOptSed () {
    }

    bool isVerbose, isVeryVerbose;
    bool doAll;
    const bool allowUTF8;  // not configurable with options!
    int sedLevel;
    int zValue;

    gString sConfig;
    gString sOutput;
    gString sLogFile;

    gString sTmpFile;
    gString sContent;  // file with content ('lines' command)

    gList myConfig;

    sHtmlized htmlized;

    gUniCode uni;

    void PrepareUni () {
	uni.Build();
	extra_unis( uni );
    }
};


// Globals
t_uint16 lineIndex=0;
t_uchar wholeLine[ 16 * 1024 ];
sHtmlized basicData;

////////////////////////////////////////////////////////////
int command_from_str (const char* cmdStr)
{
 int iter( 0 );
 const char* str;
 const sPair pairs[]={
     { 1, "test" },
     { 2, "to-base64" },
     { 3, "from-base64-txt" },
     { 4, "from-base64-bin" },
     { 5, "to-base65" },
     { 6, "from-base65-txt" },
     { 7, "from-base65-bin" },
     { 8, "-" },
     { 9, "from-utf8" },
     {10, "to-utf8-stdin	@	UNIMPLEMENTED" },
     {11, "url-name-builder"},
     {12, "auth-mime64" },
     {13, "norm"},
     {14, "undo" },
     {15, "line-histogram" },
     {16, "lines" },
     {17, "line-rand" },
     {18, "subst" },
     {19, "anchor" },
     {20, "datex" },
     {21, "dated" },
     {22, "timex" },
     {23, "from-base" },
     {24, "from-date" },
     {25, "tee" },
     {26, "strings" },
     {27, "string" },
     {28, "nice-urlx" },
     {29, "nice-xurl" },
     {30, "unescape" },
     {31, "cat" },
     {32, "batch" },
     {33, "from-any-utf8" },  // see '9'
     { -1, nil },
     { -1, nil }
 };

 if ( cmdStr==nil ) return -1;
 for ( ; (str = pairs[ iter ].str)!=nil; iter++) {
     if ( cmdStr[0]!=0 && strcmp( str, cmdStr )==0 ) return pairs[ iter ].value;
 }
 return -1;
}


int print_help (char* progStr)
{
 const char
     *msgHelp = "%s - %s\n\n\
Usage:\n\
        %s command [--help] [OPTION] [NAME ...]\n\
\n\
Commands are:\n\
   test              Test lib: dump iobjs and imedia major / minor version\n\
   auth-mime64       dump Base64 from user and pass provided in args\n\
   to-base64         convert strings in args to Base64\n\
   from-date         Show date in other format\n\
   from-base         Convert from base (e.g. 16 = hex)\n\
   from-base64-txt   convert Base64 in args to text\n\
   from-base64-bin   convert Base64 in args to binary\n\
   to-base65         convert strings in args to Base65\n\
   from-base65-txt   convert Base65 in args to text\n\
   from-base65-bin   convert Base65 in args to binary\n\
   from-utf8         convert from UTF-8 to ISO 8859-1\n\
   from-any-utf8     convert from UTF-8 to ISO 8859-1 best match\n\
   url-name-builder  Simple URL name builder\n\
   nice-urlx         Intelligent URLX builder\n\
   nice-xurl         Intelligent URLX retriever\n\
   unescape          Unescape '%%ab' (hex) sequences\n\
   norm CMD          Norm commands (e.g. du)\n\
   string            dump ISO-8859-1 Latin-1 strings\n\
   strings           like 'strings' command\n\
   subst             Substitute strings\n\
   lines             Display one line of a text-file each time you invoke\n\
   line-histogram    Text-lines histogram\n\
   line-rand         Show random lines from text\n\
   datex             Show text with normalized dates\n\
   dated             Similar to datex, better for multiple logs\n\
   timex             Show TOD and execution at given time\n\
   anchor            Show htmlized text\n\
   cat               Dump to stdout or file\n\
   tee               Dump to stdout (and file)\n\
\n\
Options are:\n\
   -h           This help (or --help / --version)\n\
   -v           Verbose (use twice, more verbose)\n\
   -c X         Use configuration file X (or --config)\n\
   -d N         Debug level N (0..9)\n\
   -l X         Use log file X\n\
   -o X         Use output file X (or --output)\n\
   -z X         Z value: e.g. number of chars to convert\n\
\n";

 printf(msgHelp,
	progStr,
	thisProgramVersion,
	progStr,
	progStr);
 return 0;
}


int print_help_command (char* cmdStr)
{
 int cmdNr;
 const char
     *msgHelpI[] = {
	 nil,
	 "%s %s [arg ...]\n\
\n\
Dump iobjs / imedia library major and minor versions.\n\
\n\
If any arg is given, it will show the string-hashes of each one.\n\
You can use -z M to specify the modulus of output.\n\
",
	 "%s : %s [arg ...]\n\
\n\
Convert arguments to Base64 string.\n\
\n\
Example:\n\
	ssed to-base64 '0123456789A?' => MDEyMzQ1Njc4OUE/\n\
but\n\
	ssed to-base65 '0123456789A?' => MDEyMzQ1Njc4OUE%\n\
(the last char in Base65 is '%').\n\
",	// 2
	 "%s %s [arg ...]\n\
\n\
Convert arguments from Base64 to text.\n\
If character is not readable, exits with error code 1.\n\
",	// 3
	 "%s %s [arg ...]\n\
\n\
Convert arguments from Base64 to binary.\n\
",	// 4
	 "%s : %s [arg ...]\n\
\n\
Convert arguments to Base65 string.\n\
\n\
This is similar to 'to-base64' command, except the last char of alphabet uses\n\
a percentage (%) instead of a slash (/).\n\
Similarly to Base64 (RFC 3548), for each 3 ASCII octets it will produce\n\
at least 4 (Base64) chars. For instance: 'Abcd' will produce 'QWJjZA=='.\n\
 \n\
Other example: 'WOE=' is 'Xá' (X and 'a' with accute accent.)\n\
",	// 5
	 "%s %s [arg ...]\n\
\n\
Convert arguments from Base65 to text.\n\
If character is not readable, exits with error code 1.\n\
",	// 6
	 "%s %s [arg ...]\n\
\n\
Convert arguments from Base65 to binary.\n\
\n\
Example:\n\
	ssed from-base65-bin WAE= -o /tmp/output.bin\n\
dumps two characters into file: X and NUL (ASCII 0).\n\
You can also limit the number of output characters, using -z.\n\
Similar to command from-base64-bin, but using a slightly different alphabet.\n\
",	// 7
	 nil,	// 8
	 "%s %s [OPTIONS] [ARGS ...]\n\
\n\
Convert from UTF-8 to ISO8859-1.\n\
\n\
Options are:\n\
	-z 1	Show invalid lines.\n\
",	// 9
	 "%s %s\n\
\n\
\t\n\
",	// 10
	 "%s %s [-z MASK] [arg ...]\n\
\n\
URL name builder\n\
\n\
The mask allows, optionally, to specify how many (back)slashes are allowed.\n\
\n\
Examples:\n\
	echo \"Wham! - I'm Your Man\" | ssed url-name-builder\n\
		Wham!+-+I%%27m+Your+Man\n\
\n\
	ssed url-name-builder 'abc\\band\\name'\n\
		abc\\band\\name\n\
\n\
	ssed url-name-builder -z 1 'abc\\band\\name'\n\
		band\\name\n\
",	// 11
	"%s %s USER PASS\n\
\n\
Displays the Base64 for '\\000'USER'\\000'PASS.\n\
Helpful when doing plain-text authentication in SMTP.\n\
",	// 12
	"%s %s CMD [OPTIONS] [arg ...]\n\
",	// 13
	nil, // 14
	"%s %s [file ...]\n\
\n\
Show histogram for file(s)\n\
",	// 15
	"%s %s first|next [file ...]\n\
\n\
Show each line, one by one, of file(s)\n\
", // 16
	"%s %s [file ...]\n\
\n\
Options:\n\
	-a	Show everything garbled (fetches first into memory)\n\
	-z N	Random factor per 1000 (default is 100)\n\
", // 17
	"%s %s [OPTIONS] expr1 expr2 [file ...]\n\
\n\
Substitute expr1 by expr2 on file(s)\n\
\n\
Options:\n\
	-a	Rewrite file(s) always\n\
\n\
Files specified will get filled with file contents\n\
(for those replacements were done).\n\
\n\
If UNDO env var exists, ssed places a copy of the original file at that path.\n\
\n\
Examples:\n\
	ssed pat1 toString2 file*\n\
		-> substitutes pat1 by toString2\n\
\n\
	ssed -c x.conf Abc/ Def/\n\
		-> substitutes either Abc\\ or Abc/ by Def\\ or Def/\n\
\n\
x.conf (configuration) contains a line:\n\
	substitute_similar_slashes=true\n\
\n\
A special case:\n\
	ssed subst . trim file...\n\
trims the text file(s), or stdin when no file is entered.\n\
",	// 18
	"%s %s [-z MASK] [file ...]\n\
\n\
Lists HTML from text file.\n\
\n\
Options:\n\
	-z 1		Dumps new-lines where needed.\n\
\n\
	-d N		stdout debug info when N>3.\n\
",	// 19
	"%s %s [-z MASK] [file ...]\n\
\n\
MASKs are:\n\
	0	Convert DD-MM-YYYY into YYYY-MM-DD\n\
	1	Convert Jan DD into YYYY-MM-DD\n\
	N>1900	Convert Jan DD into N-MM-DD (N for the year)\n\
\n\
Examples:\n\
	ssed datex my_file.txt\n\
\n\
To show a log in YYYY-MM-DD format (TOD):\n\
	ssed datex -z 1 /var/log/messages*\n\
",	// 20
	"%s %s [-z MASK] [file ...]\n\
\n\
Same as datex, except:\n\
- when providing similar arguments (files), ssed chooses the older names first.\n\
",	// 21
	"%s %s [OPTIONS] [HH:[MM:[SS]]]\n\
\n\
Show TOD (time of day) at given time.\n\
\n\
Options:\n\
	-z N	Seconds, shown precision (3 is the default; 6 shows microsecs)\n\
",  // 22 (timex)
	"%s %s BASE [number ...]\n\
\n\
Convert from base (usually 'hex' as base 16);\n\
",	// 23
	"%s %s [-z N] [date ...]\n\
\n\
Show normalized date.\n\
\n\
-z N stands for the following formats:\n\
	0	YYYY-MM-DD [HH:MM[:SS]]\n\
	1	YYYYMMDD [HH:MM[:SS]]\n\
	2	YYYY-MM-DD [HH:MM[:SS]] or MMM [D]D [remaining string]\n\
\n\
Each arg displays one date line, the complete date provided should use e.g.:\n\
	2012-12-29@23:57:58\n\
	or instead of '@' a blank or semi-colon (';') or '_' can be used.\n\
",  // 24
	"%s %s [OPTIONS] [DATE_FORMAT]\n\
\n\
tee, or -o FILE to dump to stdout and file.\n\
\n\
-a appends to output instead of overwritting.\n\
-z 1 uses text-mode (instead of default binary mode).\n\
\n\
Date formats are one of:\n\
	C - ctime,\n\
	TOD - YYYY-MM-DD,\n\
or even empty\n\
",	// 25
	"%s %s [FILE ...]\n\
\n\
Dump only string from input(s), similar to standard unix command 'strings'.\n\
",  // 26
	"%s %s [FILE ...]\n\
\n\
Dump only string from input(s), accepting special chars.\n\
",  // 27
	"%s %s [OPTIONS] [URL ...]\n\
\n\
Generates an URLX from stdin or aguments,\n\
even if arguments are full of garbage.\n\
\n\
Options:\n\
	-a	checks URL before displaying it\n\
\n\
\
",  // 28
	"%s %s [URLX]\n\
\n\
Displays URLs associated with URLX.\n\
\n\
\
",  // 29
	"%s %s [-z MASK] [ARGS ...]\n\
\n\
Unescapes arguments that are possibly escaped:\n\
	 %%ab	or	%%AB	mean ASCII 0xAB, i.e. 171d\n\
\n\
-z 1	escapes chars whenever necessary\n\
-z 2	escapes chars always\n\
-z 10	only unescapes valid ASCII 7bit chars\n\
",  // 30
	"%s %s [-z MASK] [FILE ...]\n\
\n\
Cat file(s).\n\
\n\
Options:\n\
-z 1:\n\
- when providing similar arguments (files), ssed chooses the older names first.\n\
",  // 31
	"\0",  // 32
	"%s %s [ARGS ...]\n\
\n\
Similar to from-utf8, except it accepts best match from UCS4 to UCS2 to fit\n\
ISO8859-1.\n\
",  // 33
	nil,
	nil,
	nil,
	nil,
	nil };

 cmdNr = command_from_str( cmdStr );
 if ( cmdNr<0 ) {
     printf("Invalid command: %s\n",cmdStr);
     return -1;
 }

 printf(msgHelpI[ cmdNr ],
	gFileControl::Self().sProgramName.Str(),
	cmdStr);
 return 0;
}

////////////////////////////////////////////////////////////
int print_version (char* progStr)
{
 const char
     *msgVersion = "%s - %s\n\
\n\
Build \
062\
\n\
\n\
Written by Henrique Moreira.\n\
\n\
Copyright (C) %u %s.\n\
%s\n";

 printf(msgVersion,
	progStr,
	thisProgramVersion,
	thisProgramYear,thisProgramCopyright,
	thisProgramCopy);
 return 0;
}

////////////////////////////////////////////////////////////
// Aux functions
////////////////////////////////////////////////////////////
int start_log (FILE* fRepErr, gArg& arg, sOptSed& opt, int& debugLevel)
{
 gString logFile;

 ASSERTION(fRepErr,"fRepErr");

 if ( debugLevel ) {
     lGlobLog.dbgLevel = debugLevel;
 }
 else {
     lGlobLog.dbgLevel = debugLevel = LOG_WARN + (int)opt.isVerbose + (int)opt.isVeryVerbose * 2;
 }
 if ( arg.FindOption( 'l', logFile ) ) {
     if ( lGlobLog.SetName( logFile )==false ) {
	 fprintf(fRepErr, "Unable to use log-file: %s\n",logFile.Str());
	 return 1;
     }
     opt.sLogFile = logFile;
 }
 return 0;
}


const char* ssed_config_default ()
{
 static char ssedConfName[ 256 ];
 const char* strConfig( getenv( SSED_CONFIG_FILE ) );

 memset(ssedConfName, 0x0, sizeof(ssedConfName));

 if ( strConfig==nil ) {
     const char* strApp( getenv( "APPDATA" ) );
     if ( strApp ) {
	 snprintf(ssedConfName, sizeof(ssedConfName)-1, "%s" gSLASHSTR SSED_CONFIG_NAME, strApp);
	 gFileStat aStat( ssedConfName );
	 if ( aStat.HasStat() && aStat.IsDirectory()==false ) strConfig = ssedConfName;
     }
 }

 return strConfig;
}


void extra_unis (gUniCode& uni)
{
 const t_uchar valids[]="\t\n";
 t_uchar uChr;
 for (short idx=0; (uChr = valids[ idx ])!=0; idx++) {
     uni.hash256Basic[ gUniCode::e_Basic_Printable ][ uChr ] = (char)uChr;
 }
}


int config_str_boolean (gString& sValue, int& error)
{
 gString sCheck( sValue );
 int value;

 sCheck.Trim();
 sCheck.UpString();
 value = sCheck.Match( "TRUE" ) || sCheck.Match( "YES" );
 error = (value==1 || sCheck.Match( "FALSE" ) || sCheck.Match( "NO" ))==0;
 DBGPRINT("DBG: config line: %d, error=%d, '%s' => value=%d\n",sValue.iValue,error,sValue.Str(),value);
 return value;
}


const char* config_bool_str (int value)
{
 return value ? "true" : "false";
}


gString* find_config (gList& myConfig, const char* strEq, gString& rValue)
{
 gString* ptrStr( nil );
 int pos;

 rValue.Reset();
 if ( myConfig.FindFirst( (char*)strEq, 1, e_FindExactPosition ) ) {
     ptrStr = (gString*)myConfig.CurrentPtr()->me;
     pos = (int)ptrStr->Find( '=' );
     if ( pos>0 ) {
	 rValue.Set( ptrStr->Str( pos ) );
	 rValue.iValue = atoi( rValue.Str() );
     }
 }
 return ptrStr;
}


gString* find_config_statement (gList& myConfig, const char* str, int& error)
{
 gString* ptrStr( nil );
 gString sEq( (char*)str );
 sEq.Add( '=' );

 if ( myConfig.FindFirst( (char*)str, 1, e_FindExactPosition ) ) {
     ptrStr = (gString*)myConfig.CurrentPtr()->me;

     // This function is to find statements (without rValue),
     // so, if any STATEMENT= is found, is an error.
     error = myConfig.FindFirst( sEq.Str(), 1, e_FindExactPosition )>0;
 }
 return ptrStr;
}


int generic_read_config (gList& input, gList& conf)
{
 int line( 0 );
 unsigned uPos;
 gElem* ptrElem( input.StartPtr() );

 for ( ; ptrElem; ptrElem=ptrElem->next) {
     line++;
     gString sLine( ptrElem->Str() );
     sLine.Trim();
     if ( sLine[ 1 ]=='#' || sLine.IsEmpty() ) continue;

     // Trim blanks before '='
     uPos = sLine.Find( '=' );
     if ( uPos>1 ) {
	 gString sCopy;
	 gString sRight;
	 sCopy.CopyFromTo( sLine, 1, uPos-1 );
	 sCopy.Trim();
	 sRight.CopyFromTo( sLine, uPos+1 );
	 sRight.Trim();
	 sLine = sCopy;
	 sLine.Add( '=' );
	 sLine.AddString( sRight );
     }
     conf.Add( sLine );
     conf.EndPtr()->me->iValue = line;
 }
 return 0;
}


int listed_read (gList& input, gList& lines)
{
 int line( 0 );
 gElem* ptrElem( input.StartPtr() );

 for ( ; ptrElem; ptrElem=ptrElem->next) {
     line++;
     gString sLine( ptrElem->Str() );
     if ( line==1 ) {
	 if ( sLine[ 1 ]=='#' ) {
	     lines.Add( sLine );
	     continue;
	 }
     }
     lines.Add( sLine );
     if ( sLine.Length() ) {
	 lines.EndPtr()->me->iValue = line;
     }
 }
 return 0;
}


int ssed_read_config_file (const char* strConfigFile, FILE* fRepErr, gList& myConfig)
{
 int error( 0 );

 ASSERTION(fRepErr,"fRepErr");

 if ( strConfigFile==nil ) {
     strConfigFile = ssed_config_default();
 }
 if ( strConfigFile==nil || strConfigFile[ 0 ]==0 )
     return -1;

 gFileFetch config( (char*)strConfigFile );
 if ( config.IsOpened()==false ) {
     LOG_ERR("Unable to open: %s", strConfigFile);
     return 2;
 }

 error = generic_read_config( config.aL, myConfig );

 if ( error ) {
     myConfig.Delete();
     LOG_ERR("Error in config file: %s", strConfigFile);
 }
 return error;
}


int cat_file (FILE* fIn, FILE* fOut, int option, int& problem, int& warns)
{
 int lines( 1 );
 t_uchar uChr;
 const bool convertWeird( (option & 8)!=0 );
 const bool convertZeroToBlank( (option & 16)!=0 );

 ASSERTION(fIn,"fIn");
 warns = 0;
 problem = 0;  // no problem (at line "0")

 for ( ; fscanf(fIn, "%c", &uChr)==1; ) {
     if ( uChr==0 ) {
	 warns++;
	 if ( convertZeroToBlank ) {
	     uChr = ' ';
	     if ( problem==0 ) problem = lines;
	 }
     }
     if ( (uChr>13 && uChr<' ') || (uChr>=127 && uChr<0xA0) ) {
	 warns++;
	 if ( convertWeird ) {
	     uChr = '.';
	     if ( problem==0 ) problem = lines;
	 }
     }
     else {
	 lines += (uChr=='\n');
     }
     if ( fOut ) {
	 fprintf(fOut, "%c", uChr);
     }
 }
 return lines;
}


int show_from_date (FILE* fIn, FILE* fOut, int option, const char* aStr)
{
 int error( 0 );
 t_int32 iValue;
 bool isValue( gStorageControl::Self().ConvertToInt32( (char*)aStr, iValue )==0 );
 gDateTime* aDate( nil );
 gDateString::eDateFormat dateFormat( (gDateString::eDateFormat)(option > 3 ? 0 : option ) );

 ASSERTION(fIn,"fIn");
 ASSERTION(fIn,"fOut");

 if ( isValue ) {
     aDate = new gDateTime( iValue );
 }
 else {
     if ( aStr==nil || aStr[ 0 ]==0 ) {
	 aDate = new gDateTime;
     }
     else {
	 gString sDate, sTime;
	 short which( 0 );
	 const char* ptr( aStr );

	 for (char chr; (chr = *ptr)!=0; ptr++) {
	     if ( chr==':' ) {
		 which = 2;
	     }
	     if ( chr<=' ' || chr==';' || chr=='_' || chr=='@' ) {
		 which = 1;
	     }
	     else {
		 if ( which ) {
		     sTime.Add( chr );
		 }
		 else {
		     sDate.Add( chr );
		 }
	     }
	 }
	 if ( which >= 1 ) {
	     DBGPRINT("DBG: sDate {%s}, sTime {%s}\n", sDate.Str(), sTime.Str());
	     aDate = new gDateString( sDate, sTime, dateFormat );
	 }
	 else {
	     aDate = new gDateString( (char*)aStr, dateFormat );
	 }
	 error = aDate->IsOk()==false;
     }
 }

 fprintf(fOut, "%04u-%02u-%02u %02u:%02u:%02u\n",
	 aDate->year, aDate->month, aDate->day,
	 aDate->hour, aDate->minu, aDate->sec);

 delete aDate;
 return error;
}


int show_from_base (int base, const char* aStr, const char* strBase, FILE* fOut)
{
 const char* strNum( aStr );
 char thisChr, which;
 int error( 0 );
 int sum( 0 );
 long long maxValue( 0 );

 ASSERTION(fOut,"fOut");

 if ( base==16 ) {
     if ( strncmp( strNum, "0x", 2 )==0 ) {
	 strNum += 2;
     }
     else {
	 if ( strNum[ 0 ]=='x' ) strNum++;
     }
 }

 DBGPRINT("DBG: base=%d, strNum {%s}, strBase {%s}\n\n",
	  base,
	  strNum,
	  strBase);

 for ( ; (thisChr = *strNum)!=0; strNum++) {
     if ( thisChr>='a' && thisChr<='z' ) {
	 thisChr -= 32;
     }
     for (int find=0; (which = strBase[ find ])!=0; find++) {
	 if ( which==thisChr ) {
	     maxValue = sum;
	     sum *= base;
	     sum += find;
	     if ( sum < maxValue ) {
		 error = 4;  // overflow
	     }
	     break;
	 }
     }

     if ( which==0 ) {
	 error = 1;
	 break;
     }
 }
 if ( error > 1 ) {
     return error;
 }
 fprintf(fOut,"%d\n",sum);
 return error;
}


const char* url_builder_last_two (const char* aStr, int mask)
{
 int len( ssun_length( aStr ) );
 int iter( len-1 );
 int countSlash( 0 );

 if ( mask==-1 ) return aStr;
 if ( len<=0 ) return aStr;

 for ( ; iter>0; iter--) {
     if ( aStr[ iter ]=='/' || aStr[ iter ]=='\\' ) {
	 countSlash++;
	 if ( countSlash>mask ) return aStr+iter+1;
     }
 }
 return aStr;
}


int show_auth_mime64 (FILE* fOut,
		      gString& sUser,
		      gString& sPass)
{
 FILE* fShow( fOut ? fOut : stdout );

 gString* newStr( imb_auth_mime64( sUser.UStr(), sPass.UStr() ) );
 if ( newStr ) {
     printf("USER: %s, PASS: %s\n",
	    sUser.Str(),
	    sPass.Str());
     fprintf(fShow,"%s\n",newStr->Str());
 }
 else {
     fprintf(stderr,"Bogus!\n");
     return 1;
 }
 delete newStr;
 return 0;
}


int recognize_tag (gString& toFind, gList* ptrExtra)
{
 int iter( 1 );
 const char* str;
 const char* basicTags[]={
     nil,
     "XML",
     "HTML",
     "HEAD",
     "HEADER",
     "A",
     "P",
     "BR",
     "TITLE",
     "BODY",
     nil,
     nil
 };

 for ( ; (str = basicTags[ iter ])!=nil; iter++) {
     gString sCheck( "<" );
     sCheck.Add( (char*)str );
     sCheck.Add( ' ' );
     if ( toFind.Find( sCheck )==1 ) return iter;
     sCheck[ sCheck.Length() ] = '>';
     if ( toFind.Find( sCheck )==1 ) return iter;
 }
 if ( ptrExtra ) {
     iter = ptrExtra->Match( toFind.Str() );
     return 0-iter;
 }
 return 0;
}


int char_breaks_url (t_uchar uChr)
{
 int doesBreak( uChr<=' ' || uChr==',' || uChr==';' );
 return doesBreak;
}


int my_string_proto (gString& sProto)
{
 int hash( 0 );
 unsigned pos( 1 );
 t_uchar uChr;
 const unsigned maxProtoString( 6 );  // http has only 4, but we allow little bit more

#define PROTO_HASH_HTTP 551

 for ( ; (uChr = sProto[ pos ])!=0; pos++) {
     if ( (uChr>='a' && uChr<='z') ) {
	 uChr -= 32;
     }
     if ( uChr>='A' && uChr<='Z' ) {
	 hash <<= 3;
	 hash += (uChr - 'A');
     }
     else {
	 if ( uChr==':' ) {
	     DBGPRINT_MIN("DBG: proto {%s}, hash: %d, %d\n",
			  sProto.Str(),
			  hash,
			  hash & PRIME_MILLION_NP0);
	     return (hash & PRIME_MILLION_NP0);
	 }
     }
     if ( pos > maxProtoString ) return -1;
 }
 return -1;
}


t_uchar pretty_uri (gString& sURI, gString& pretty)
{
 int pos( 0 ), start( -1 );
 int countSlash( 0 );
 t_uint32 hex( 0 );
 t_uchar uChr;
 char uHex[ 6 ];
 t_uchar* uStr( sURI.UStr() );

 for ( ; (uChr = uStr[ pos ])!=0; ) {
     gString sToAdd;

     pos++;

     switch ( uChr ) {
     case '%':
	 uHex[ 0 ] = (char)uStr[ pos ];
	 uHex[ 1 ] = (char)uStr[ pos+1 ];
	 uHex[ 2 ] = 0;
	 if ( sURI.ConvertHexToUInt32( uHex, e_DigConvAny, hex )==0 ) {
	     uChr = (t_uchar)hex;
	     pos += 2;
	 }
	 break;

     case ':':
	 if ( start<=-1 ) {
	     countSlash = uStr[ pos ]=='/' && uStr[ pos+1 ]=='/';
	     countSlash *= 2;
	 }
	 continue;

     case '/':
	 countSlash--;
	 if ( countSlash==0 ) {
	     start = pos;
	     continue;
	 }
	 break;

     case '+':
     case '&':
	 sToAdd.Set( " " );
	 sToAdd.Add( uChr );
	 sToAdd.Set( " " );
	 break;

     case '_':
     case '=':
	 uChr = ' ';
	 break;

     default:
	 break;
     }

     if ( start > 0 ) {
	 if ( sToAdd.Length() ) {
	     pretty.AddString( sToAdd );
	 }
	 else {
	     pretty.Add( uChr );
	 }
     }
 }

 if ( start>0 ) {
     pos = pretty.Find( "." );
     if ( pretty.Find( "ww", true )==1 && (pos==3 || pos==4) ) {
	 pretty.Delete( 1, pos );
     }
     return start;
 }

 pretty = sURI;
 return 0;
}


int clean_string (int dbgLine, FILE* fOut, int newValue, gString& aString)
{
 if ( aString.Length() ) {
     LOG_DBG("line %d: newValue=%d, clean_string {%s}\n",
	     dbgLine,
	     newValue,
	     aString.Str());
 }

 if ( fOut ) {
     if ( my_string_proto( aString )==PROTO_HASH_HTTP ||
	  aString.Find( ':' )==5 ) {
	 gString pretty;

	 pretty_uri( aString, pretty );

	 // hprint(fOut,"<A HREF=\"%s\">%s</A>", aString.Str(), pretty.Str())

	 gString oBuf( aString.Length() + pretty.Length() + 32, '\0' );
	 sprintf(oBuf.Str(), "<A HREF=\"%s\">%s</A>", aString.Str(), pretty.Str());
	 h_out_str( oBuf );
	 hprint(fOut, "%s", oBuf.Str());
     }
     else {
	 h_out_str( aString );
	 hprint(fOut, "%s", aString.Str());
     }
 }

 if ( aString.Length() ) {
     LOG_DBG("line: %d, newValue=%d, clean_string {%s}\n",
	     dbgLine,
	     newValue,
	     aString.Str());
 }
 aString.Reset();
 aString.iValue = newValue;
 return 0;
}


int anchored_config (FILE* fRepErr, gList& myConfig, int zValue, bool isStdout, bool sourceIsHtml, sHtmlized& htmlized)
{
 int error( 0 );
 int line( 0 );
 int thisError;
 int pos;
 gList posL;
 gList errors;
 gElem* ptrLine( nil );
 gElem* ptrElem( myConfig.StartPtr() );
 gString* ptrMe;
 gList built;

 htmlized.Reset();
 htmlized.dumpNL = zValue>0;
 htmlized.dumpNL |= (2 * (zValue==0));

 // Pre-check
 for ( ; ptrElem; ptrElem=ptrElem->next) {
     ptrMe = (gString*)ptrElem->me;
     ptrLine = ptrMe->Find( '=' ) ? ptrElem : nil;
     if ( ptrLine ) {
	 built.Add( *ptrMe );
	 built.EndPtr()->me->iValue = ptrMe->iValue;
     }
 }

 // Config e.g. base href=http://www.google.com/
 pos = built.FindAny( "base href=", 1, e_FindExactPosition, posL );
 thisError = posL.N()>1;
 if ( thisError ) {
     error = 1;
     errors.Add( "'base href' can only be set once" );
     errors.EndPtr()->me->iValue = line = (built.CurrentPtr()->me->iValue);
 }
 else {
     if ( pos ) {
	 ptrLine = built.CurrentPtr();
	 htmlized.addBaseHREF.Add( ptrLine->me->Str( 10 ));
	 ptrLine->iValue = -1;
     }
 }

 // Config e.g. dump_type=no_script
 posL.Reset();
 pos = built.FindAny( "dump_type=", 1, e_FindExactPosition, posL );
 thisError = posL.N()>1;
 if ( thisError ) {
     error = 1;
     errors.Add( "dump_type can only be set once" );
     errors.EndPtr()->me->iValue = line = (built.CurrentPtr()->me->iValue);
 }
 else {
     if ( pos ) {
	 ptrLine = built.CurrentPtr();
	 pos = strncmp( ptrLine->me->Str( 10 ), "no_script", strlen( "no_script" ) )==0;
	 htmlized.dumpType = (sHtmlized::eDumpType)pos;
	 ptrLine->iValue = -1;
     }
 }

 posL.Reset();
 built.FindAny( "exclude=", 1, e_FindExactPosition, posL );
 if ( posL.N() ) {
     for (gElem* one=posL.StartPtr(); one; one=one->next) {
	 ptrLine = built.GetElementPtr( one->me->iValue );
	 gString* myStr( (gString*)ptrLine->me );
	 ptrLine->iValue = -1;
	 pos = myStr->Find( '=' );

	 gString sWhat( myStr->Str( pos ) );
	 sWhat.Trim();
	 htmlized.excluded.Add( sWhat );
     }

     // Optimize exclusions

     for (gElem* hint=htmlized.excluded.StartPtr(); hint; hint=hint->next) {
	 gString sNamed( hint->Str() );
	 gString sHint;
	 bool isTag( sNamed[ 1 ]=='<' && sNamed[ sNamed.Length() ]=='>' );
	 bool isStar( false );

	 if ( isTag ) {
	     sNamed.Delete( 1, 1 );
	     sNamed.Delete( sNamed.Length() );
	     sNamed.Trim();
	 }

	 isStar = sNamed[ sNamed.Length() ]=='*';
	 if ( isStar ) {
	     sNamed.Delete( sNamed.Length() );
	 }

	 if ( isTag ) {
	     sHint.Add( "<" );
	 }

	 int code( isTag ? (int)isStar : (isStar ? -1 : 1) );

	 sHint.AddString( sNamed );
	 htmlized.fastExcluded.Add( sHint );
	 htmlized.fastExcluded.EndPtr()->me->iValue = code;

	 sHint.UpString();
	 htmlized.fastExcluded.Add( sHint );
	 htmlized.fastExcluded.EndPtr()->me->iValue = code;
     }
 }

 // Dump unused config file lines:

 for (ptrElem=built.StartPtr(); ptrElem; ptrElem=ptrElem->next) {
     if ( ptrElem->iValue!=-1 ) {
	 if ( fRepErr ) {
	     LOG_ERR("Unused config (line %d): %s",
		     ptrElem->me->iValue,
		     find_value( ptrElem->me->iValue, built.StartPtr() )->Str());
	 }
     }
 }

 // Dump bogus lines:

 if ( fRepErr ) {
     for (ptrElem=errors.StartPtr(); ptrElem; ptrElem=ptrElem->next) {
	 line = ptrElem->me->iValue;
	 LOG_ERR("Config error at line %d%s%s",
		 line,
		 ptrElem->Str()[ 0 ] ? ": " : ".",
		 ptrElem->Str());
     }
 }

#if 0
 printf("exclude: "); htmlized.excluded.Show(); printf("\n");
 for (gElem* hints=htmlized.fastExcluded.StartPtr(); hints; hints=hints->next) {
     printf("fast exclusion [%d] %s\n",
	    hints->me->iValue,
	    hints->Str());
 }
#endif
 return error;
}


char* content_name (gString& sResult)
{
 char* strEnv( getenv( "SSED_LINES" ) );
 gString sTemp( 512, '\0' );

 if ( strEnv==nil || strEnv[ 0 ]==0 ) {
     gFileControl::Self().CtrlGetTempPath( sTemp.Str(), 512 );
     strEnv = sTemp.Str();
 }
 ASSERTION(strEnv,"strEnv");
 sResult.Set( strEnv );
 sResult.Add( gSLASHCHR );
 sResult.Add( "temp.lines" );
 return strEnv;
}


char* content_index (gString& sResult )
{
 char* strEnv( getenv( "SSED_LINE_INDEX" ) );
 gString sTemp( 512, '\0' );

 if ( strEnv==nil || strEnv[ 0 ]==0 ) {
     gFileControl::Self().CtrlGetTempPath( sTemp.Str(), 512 );
     strEnv = sTemp.Str();
 }
 ASSERTION(strEnv,"strEnv");
 sResult.Set( strEnv );
 sResult.Add( gSLASHCHR );
 sResult.Add( "temp.index" );
 return strEnv;
}


int check_text_line (const char* strLine, int didRead, int& len)
{
 // Return 1 if is a CR/NL file; 0 if all ok; -1 if it seems a binary file
 bool hasCR( false );
 int errors( 0 ), warns( 0 );
 char chr( '\0' );

 len = 0;
 if ( strLine==nil ) return 0;

 for (int iter=0; iter<didRead; iter++) {
     chr = strLine[ iter ];
     len++;
     if ( chr=='\n' ) {
	 break;
     }
     else {
	 if ( chr=='\r' ) {
	     hasCR = true;
	 }
	 else {
	     if ( chr!='\t' && chr<' ' ) {
		warns++;
		errors += (chr>=0 && chr<' ');
	     }
	 }
     }
 }
 DBGPRINT("DBG: len: %d, errors: %d, warns: %d\n", len, errors, warns);
 if ( errors * 10 + warns * 2 > len ) return -1;  // maybe a binary file?
 return (int)hasCR;
}

////////////////////////////////////////////////////////////
int do_test (FILE* fOut, const char* strOpt, sOptSed& opt)
{
 const char* strPre1( "iobjs" );
 const char* strPre2( "imedia" );
 const char* strConfig( ssed_config_default() );
 unsigned uVal;

 DBGPRINT("DBG: do_test(fOut?%c, %s, ...)\n",
	  ISyORn( fOut!=nil ),
	  strOpt);
 fOut = fOut ? fOut : stdout;

 if ( strOpt && strOpt[ 0 ] ) {
     gString sNew( (char*)strOpt );
     uVal = (unsigned)sNew.Hash();

     switch ( opt.zValue ) {
     case -1:
	 fprintf(fOut,"%d\n",sNew.Hash());
	 break;
     case 0:
	 fprintf(fOut,"%u\n",uVal);
	 break;
     default:
	 fprintf(fOut,"%d\n",uVal % (unsigned)opt.zValue);
	 break;
     }
     return 0;
 }
 fprintf(fOut,"%s:\t%d %d\n",
	 strPre1,
	 LIB_VERSION_ILIB_MAJOR,
	 LIB_VERSION_ILIB_MINOR);
 fprintf(fOut,"%s:\t%d %d\n",
	 strPre2,
	 LIB_IMEDIA_VERSION_MAJOR,
	 LIB_IMEDIA_VERSION_MINOR);

 if ( strConfig && strConfig[ 0 ] ) {
     fprintf(fOut,"config: %s\n",strConfig);
 }

 if ( opt.isVerbose ) {
     printf("ISO %s %s (%u)\n",
	    ptrUniData->Is8859() ? "8859" : "?",
	    ptrUniData->IsIsoLatin() ? "Latin" : "(other)",
	    ptrUniData->inUse->UniTable());
 }
 return 0;
}


int do_lines (FILE* fIn,
	      FILE* fOut,
	      const char* strOpt,
	      bool isFirst,
	      bool isNext,
	      int how,
	      sOptSed& opt)
{
 int error( 0 );
 int did( -1 );
 int handleIn( fileno( fIn ) );
 int tmpHandle( -1 );
 t_uchar uChr( '\0' );
 gString sLine;
 long lineNr( 0 ), offset( 0 );
 bool isEof( true );
 FILE* fTemp( nil );
 const char* strTempFile( opt.sTmpFile.Str() );

 DBGPRINT("DBG: fIn: %p%s, fOut: %p, {%s} how: %d, isFirst? %c\n",
	  fIn, fIn==stdin ? " (stdin)" : "",
	  fOut,
	  strOpt,
	  how,
	  ISyORn( isFirst ));
 ASSERTION(fOut,"fOut");

 if ( isFirst ) {
     if ( opt.sContent.Length() ) {
	 ASSERTION(fIn==stdin,"fIn==stdin");

	 fTemp = fopen( opt.sContent.Str(), "wt" );
	 if ( fTemp ) {
	     tmpHandle = fileno( fTemp );
	 }
	 if ( tmpHandle==-1 ) {
	     return 4;
	 }
	 for ( ; fscanf(fIn,"%c",&uChr)==1; ) {
	     did = write( tmpHandle, &uChr, 1 )==1;
	     if ( did!=1 ) {
		 fclose( fTemp );
		 remove( opt.sContent.Str() );
		 return 1;
	     }
	 }

	 // temporary file has the contents sent by stdin,
	 // now it's a matter of re-using it
	 fclose( fTemp );
     }
 }

 if ( opt.sContent.Length() ) {
     fIn = fopen( opt.sContent.Str(), "r" );
 }
 if ( fIn==nil ) return -1;
 handleIn = fileno( fIn );

 if ( isFirst ) {
     fTemp = fopen( strTempFile, "wb" );
     fclose( fTemp );
 }
 fTemp = fopen( strTempFile, "rb" );
 if ( fTemp==nil ) {
     // No index file, do not bail with error!
     errno = 0;
     return 1;
 }
 fscanf(fTemp, "%ld %ld", &offset, &lineNr);

 if ( isFirst==false ) {
     off_t posDone;

     ASSERTION(handleIn!=fileno(stdin),"lseek stdin!");
     if ( offset>=0 ) {
	 posDone = lseek( handleIn, offset, SEEK_SET );
	 error = (posDone==(off_t)-1);
	 DBGPRINT("DBG: lseek %d: offset=%ld {%s}\n",
		  (int)posDone,
		  offset,
		  opt.sContent.Str());
     }
 }

 if ( error==0 ) {
     for (uChr='\0'; io_readchr( handleIn, uChr )==1; ) {
	 offset++;
	 isEof = false;
	 fprintf(fOut,"%c",uChr);

	 if ( uChr=='\n' ) {
	     lineNr++;
	     break;
	 }
     }

     fclose( fTemp );
     fTemp = fopen( strTempFile, "wb" );
     if ( fTemp ) {
	 fprintf(fTemp,"%ld %ld\n",offset, lineNr);
     }
     DBGPRINT("DBG: offset: %ld line: %ld, errno=%d\n",
	      offset,
	      lineNr,
	      errno);

     if ( (how & 16)==0 ) {
	 // In this mode EOF is signaled by a dot ('.')
	 if ( isEof ) {
	     fprintf(fOut, ".\n");
	 }
     }

 }// IF no error

 if ( isEof ) {
     // end of file or any seek error, remove file with indexes:
     remove( strTempFile );
     error = errno;
 }

 if ( fIn!=stdin ) {
     fclose( fIn );
 }
 return error;
}


int do_to_base6x (FILE* fOut,
		  const char* aStr,
		  sOptSed& opt,
		  bool isBase65)
{
 const gString64::eBase base( isBase65 ? gString64::e_Base65 : gString64::e_Base64 );
 const unsigned srcLen( strlen( aStr ) );
 t_uchar* strIn( (t_uchar*)aStr );
 char* outString;
 t_uint32 outLen( 0 );
 gString64 my( (char*)strIn );

 DBGPRINT_MIN("DBG: my.iValue=%d\n",my.iValue);
 DBGPRINT_MIN("DBG: my={%s}, {%s}=[empty], convertCode=%d=[0]\n",
	      my.Str(),
	      my.LastString().Str(),
	      my.convertCode);
 ASSERTION(fOut,"fOut");

 my.UseNow( base );
 outString = my.EncodeBinTo64( srcLen, strIn, outLen );
 DBGPRINT("DBG: my.EncodeBinTo64(%d,{%s},%u)\n",
	  srcLen,
	  strIn,
	  outLen);

 fprintf(fOut,"%s\n",outString);

 // A simpler method: Encode64()
 DBGPRINT_MIN("DBG: Encode64(), OK? %c\n",
	      ISyORn( strcmp( my.Encode64(), outString )==0 ));
 return my.convertCode!=0;
}


int do_from_base6x (FILE* fOut,
		    const char* aStr,
		    int limitChars,  // -1: no limit
		    sOptSed& opt,
		    bool isBase65,
		    bool isBinAllowed)
{
 const gString64::eBase base( isBase65 ? gString64::e_Base65 : gString64::e_Base64 );
 const unsigned srcLen( strlen( aStr ) );
 const unsigned bufLen( srcLen );  // more than we need!
 t_uchar* outString;
 t_uint32 iter( 0 );
 t_uint32 outLen( 0 );
 gString64 my( (char*)aStr );
 t_uchar binBuffer[ bufLen+1 ];
 int error;
 int weirdIdx( 0 ), weirdChar( 0 ), weirdLast( 0 );

 ASSERTION(fOut,"fOut");
 memset( binBuffer, 0x0, bufLen );

 my.UseNow( base );
 outString = my.Decode64ToBin( bufLen, binBuffer, outLen );
 error = my.convertCode;

 DBGPRINT("DBG: Decode64ToBin() of {%s}: error=%d, outLen=%u\n",
	  aStr,
	  error,
	  outLen);
 ASSERTION(outString,"outString");

 if ( isBinAllowed ) {
     for ( ; iter<outLen; iter++) {
	 if ( limitChars>=0 && (int)iter>=limitChars )
	     break;
	 fprintf(fOut, "%c", outString[ iter ]);
     }
 }
 else {
     DBGPRINT_MIN("DBG: outLen=%u (strlen is %d): {%s}\n",
		  outLen,
		  strlen( (char*)outString ),
		  outString);

     for (t_uchar uChr='\0'; iter<outLen; ) {
	 weirdLast = (int)uChr;
	 uChr = outString[ iter++ ];
	 // Note: 0x00A9 is the Copyright sign

	 if ( uChr<' ' || (uChr>=127 && uChr<=0xA9) ) {
	     if ( weirdIdx==0 ) {
		 weirdIdx = (int)iter;
		 weirdChar = (int)uChr;
		 if ( weirdLast ) {
		     weirdIdx--;
		 }
	     }
	 }
     }

     if ( weirdChar ) {
	 if ( opt.allowUTF8 ) {
	     gString sCopy( outString );
	     sCopy.Delete( weirdIdx );
	     LOG_ME(LOG_NOTICE, "The following string has UTF8 chars (first: %03dd, at position %d) %s[...]\n",
		    weirdChar,
		    weirdIdx,
		    sCopy.Str());

	     fprintf(fOut,"%s\n",outString);
	 }
	 else {
	     error = 2;
	 }
     }
     else {
	 fprintf(fOut,"%s\n",outString);
     }
 }
 return error;
}


int do_convert_from_utf8 (FILE* fIn, FILE* fOut, FILE* fDetails, int zValue, sOptSed& opt)
{
 const bool showAnyway( opt.doAll );
 const t_uint32 maxMask( (zValue & 1) ? 0xFFFF : 0xFF );
 const bool showISO( opt.sedLevel > 6 );

 t_uchar* newStr;
 char buf[ 4096 ];
 t_uchar uChr;
 int countErrors( 0 );
 int len, line( 1 ), incrLine( 0 );
 eRecodeError returnError( ereNO_ERROR );
 sUtfBox box;

 FILE* fRepErr( fDetails );

 DBGPRINT("DBG: fIn %p (%d), fOut %p (%d), fDetails %p\n",
	  fIn, (fIn ? fileno(fIn) : -1),
	  fOut, (fOut ? fileno(fOut) : -1),
	  fDetails);
 DBGPRINT("DBG: sedLevel=%d, doAll? %c, isVerbose? %c, isVeryVerbose? %c\n",
	  opt.sedLevel,
	  ISyORn( opt.doAll ),
	  ISyORn( opt.isVerbose ), ISyORn( opt.isVeryVerbose ));
 ASSERTION(fIn,"fIn");
 ASSERTION(fOut,"fOut");

 LOG_ME((fOut==stdout && showISO) ? LOG_WARN : LOG_NONE, "Output may have warnings!");

 for ( ; fgets( buf, sizeof(buf)-1, fIn ); ) {
     len = strlen( buf );
     if ( len<=0 ) break;
     len--;
     if ( buf[ len ]=='\n' ) {
	 buf[ len-- ] = 0;
	 incrLine = 1;
     }
     else {
	 incrLine = 0;
     }

     newStr = imb_utf8_str_to_ucs2_limit( (t_uchar*)buf, maxMask, box );
     returnError = box.ucs2Error;
     if ( returnError ) {
	 countErrors++;
     }

     if ( newStr ) {
	 if ( showISO ) {
	     if ( returnError==ereUNTRANSLATABLE ) {
		 char same;
		 printf("Line %d, UniCode 0x%04X ", line, box.lastUCS2);
		 for (int idx=0; (uChr = newStr[ idx ])!=0; idx++) {
		     same = opt.uni.hash256Basic[ gUniCode::e_Basic_Printable ][ uChr ];
		     if ( same==-1 && uChr!=255 ) {
			 if ( opt.isVerbose ) {
			     printf("[0x%02X]", (unsigned)uChr);
			 }
			 else {
			     printf("?");
			 }
		     }
		     else {
			 printf("%c", uChr);
		     }
		 }
		 if ( uChr!='\n' ) {
		     printf("\n");
		 }
	     }
	 }

	 // Result sent to output file:
	 fprintf(fOut,"%s%s",
		 newStr,
		 incrLine ? "\n" : "");
	 delete[] newStr;
     }
     else {
	 LOG_ME(LOG_NOTICE, "Line %d: result string empty", line);
	 if ( showAnyway ) {
	     fprintf(fOut,"%s%s",
		     buf,
		     incrLine ? "\n" : "");
	 }
     }

     if ( showISO==false && returnError!=ereNO_ERROR ) {
	 if ( fRepErr ) {
	     char* strORI( buf );
	     for ( ; (*strORI)<' '; strORI++) {
		 if ( (*strORI)==0 ) {
		     strORI = buf;
		     break;
		 }
	     }
	     if ( opt.isVeryVerbose ) {
		 LOG_ME(returnError==ereUNTRANSLATABLE ? LOG_WARN : LOG_ERROR, "%s%d: line %d, 0x%04X\n\t%-.70s%s",
			returnError==ereUNTRANSLATABLE ? "Not translatable, " : "\0",
			returnError,
			line,
			box.lastUCS2,
			strORI,
			strlen( strORI )>=62 ? "[...]" : "\0");
	     }
	     else {
		 fprintf(fRepErr, "%s%d: line %d, 0x%04X\n",
			 returnError==ereUNTRANSLATABLE ? "Not translatable, " : "\0",
			 returnError,
			 line,
			 box.lastUCS2);
	     }
	 }
     }

     line += incrLine;
 }
 return countErrors!=0;
}


int do_convert_from_any_utf8 (FILE* fIn, FILE* fOut, FILE* fDetails, int zValue, sOptSed& opt)
{
 int result( do_convert_from_utf8( fIn, fOut, fDetails, zValue, opt ) );
 return result;
}


int do_url_builder (FILE* fIn, FILE* fOut, const char* aStr, int zValue, int cmdNr)
{
 char* newStr;
 char buf[ 4096 ];

 ASSERTION(fOut,"fOut");

 if ( aStr ) {
     aStr = url_builder_last_two( aStr, zValue );
     newStr = ssun_url_builder_name( aStr, zValue );
     fprintf(fOut,"%s\n",newStr);
     free( newStr );
 }
 else {
     if ( fIn==nil ) return 2;

     for ( ; fgets( buf, sizeof(buf)-1, fIn ); ) {
	 aStr = url_builder_last_two( buf, zValue );
	 newStr = ssun_url_builder_name( aStr, zValue );
	 fprintf(fOut,"%s",newStr);
	 free( newStr );
     }
 }

 return 0;
}


int flush_anchor (sHtmlized& htmlized,
		  FILE* fOut,
		  FILE* fRepErr,
		  int lineNr,
		  eTextRefLine refLine,
		  gString& aString,
		  t_uchar* buf)
{
 // Both aString and buf changed, and index of buffer is returned.

 int iter( 0 );
 int length;
 t_int16 proto( -1 );
 t_int16 quote( 0 );  // bit 1 is quote, 2nd bit is double-quote (")
 t_int16 state( htmlized.state );  // opened: >=1, closed: 0
 t_uchar uChr( 0 ), lastChr( 0 );
 t_uchar* uStr;
 char* nextStr( nil );

 gString sAnchor;
 gString* ptrFirst( &htmlized.iname );
 gList* ptrTags( &htmlized.tags );

 aString.Add( buf );
 buf[ 0 ] = 0;
 length = (int)aString.Length();
 uStr = aString.UStr();

 htmlized.lines++;

 for ( ; iter<length; iter++) {
     lastChr = uChr;
     uChr = uStr[ iter ];

     if ( uChr=='\'' ) {
	 if ( quote & 1 ) {
	     quote = 0;
	 }
	 else {
	     if ( char_is_alpha( lastChr )<=0 ) {
		 quote |= 1;
	     }
	 }
     }
     else {
	 if ( uChr=='"' ) {
	     if ( quote & 2 ) {
		 quote = 0;
	     }
	     else {
		 quote |= 2;
	     }
	 }
     }

     switch ( uChr ) {
     case '<':
	 if ( quote==0 ) {
	     state++;

	     sAnchor.iValue = 0;
	     sAnchor.Add( uChr );

	     if ( htmlized.first==-1 && ptrTags->N()==0 && ptrFirst->IsEmpty()==true ) {
		 ptrFirst->Add( '<' );
		 htmlized.first = 1;
	     }
	     else {
		 if ( htmlized.dumpNL & 1 ) {
		     htmlized.outLines++;
		     h_out_chr( '\n' );
		     hprint(fOut, "\n");
		 }
	     }
	 }

	 if ( state ) {
	     nextStr = (char*)(uStr+iter);
	     if ( uStr[ iter+1 ]=='/' ) {
		 if ( htmlized.addBaseHREF.iValue==0 && htmlized.addBaseHREF.N() ) {
		     if ( strncmp( nextStr, "</head", 4 )==0 ||
			  strncmp( nextStr, "</HEAD", 4 )==0 ) {
			 fprintf(fOut,"<base href=\"%s\">\n",htmlized.addBaseHREF.Str( 1 ));
			 htmlized.addBaseHREF.iValue = lineNr;
		     }
		 }
		 if ( htmlized.scripted==1 ) {
		     if ( strncmp( nextStr, "</script", strlen( "<script" ) )==0 ||
			  strncmp( nextStr, "</SCRIPT", strlen( "<script" ) )==0 ) {
			 gString sNew( nextStr );
			 int posGreater( sNew.Find( '>' ) );
			 if ( posGreater ) {
			     iter += posGreater-1;
			     state = 0;
			 }
			 htmlized.scripted = 0;
			 clr_string( nil, -1, sAnchor );
			 continue;
		     }
		 }
	     }
	     else {
		 if ( htmlized.dumpType==sHtmlized::e_NoScript ) {
		     if ( strncmp( nextStr, "<script", strlen( "<script" ) )==0 ||
			  strncmp( nextStr, "<SCRIPT", strlen( "<script" ) )==0 ) {
			 htmlized.scripted = 1;
			 clr_string( nil, -1, sAnchor );
			 continue;
		     }
		 }
	     }
	 }// end IF state
	 // No break here
	 break;

     case '>':
	 if ( quote==0 ) {
	     state--;
	     if ( state<0 ) {
		 state = 0;
	     }

	     if ( state==0 ) {
		 if ( ptrFirst->Length() && ptrTags->N()==0 ) {
		     t_uchar second( (*ptrFirst)[ 2 ] );
		     ptrFirst->Add( '>' );
		     ptrTags->Add( *ptrFirst );
		     ptrFirst->UpString();
		     if ( second=='?' || second=='!' || recognize_tag( *ptrFirst, &htmlized.recognizedTagsExtra ) ) {
			 htmlized.first = 8;
		     }
		     else {
			 htmlized.first = 0;
		     }
		     DBGPRINT("DBG: ptrFirst {%s}, htmlized.first=%d\n",
			      ptrTags->Str( 1 ),
			      htmlized.first);
		 }
	     }
	 }
	 break;

     case ':':
	 if ( state ) {
	     clr_string( fOut, -1, sAnchor );
	 }
	 else {
	     proto = sAnchor.Match( "http", true );
	     if ( proto ) {
		 sAnchor.iValue = 0;
		 sAnchor.Add( uChr );
	     }
	     else {
		 clr_string( fOut, -1, sAnchor );
	     }
	 }
	 break;

     default:
	 if ( state ) {
	     clr_string( fOut, -1, sAnchor );

	     if ( htmlized.first==1 ) {
		 ptrFirst->Add( uChr );
	     }
	 }
	 else {
	     if ( char_breaks_url( uChr ) ) {
		 clr_string( fOut, 0, sAnchor );
	     }
	     else {
		 if ( sAnchor.iValue!=-1 ) {
		     sAnchor.Add( uChr );
		 }
	     }
	 }
	 break;
     }

     if ( sAnchor.iValue!=-1 && sAnchor.Length() ) {
     }
     else {
	 if ( htmlized.scripted!=1 ) {
	     h_out_chr( uChr );
	     hprint(fOut, "%c", uChr);
	 }
     }
 }// end FOR (aString...)

 clr_string( fOut, -1, sAnchor );

 if ( length ) {
     if ( state==0 && quote==0 ) {
	 if ( refLine==e_TextNewLine &&
	      htmlized.first!=8 &&
	      (htmlized.dumpNL & 2)==0 ) {
	     fprintf(fOut,"<BR/>");
	 }
     }
     if ( fRepErr ) {
	 if ( state ) {
	     LOG_ME(LOG_WARN, "Line %d, unclosed text (state: %d)",
		    lineNr,
		    state);
	 }
	 if ( quote ) {
	     LOG_ME(LOG_WARN, "Line %d, unclosed quotes (state: %d, quote: %d)",
		     lineNr,
		     state,
		     quote);
	 }
     }
 }

 if ( refLine!=e_TextEOF || (length>0 && (htmlized.dumpNL & 1)!=0) ) {
     htmlized.outLines++;
     h_out_chr( '\n' );
     hprint(fOut, "\n");
 }

 DBGPRINT_MIN("DBG: length=%d, lines: %d, refLine=%d\n",
	      length,
	      htmlized.outLines,
	      refLine);

 aString.Delete();
 return 0;
}


int do_anchor (const char* strIn,
	       FILE* fIn,
	       FILE* fOut,
	       FILE* fRepErr,
	       bool sourceIsHtml,
	       sOptSed& opt)
{
 static short once;
 static int idx, handle;
 static t_uchar buf[ 4096 ];
 static const int bufSize( sizeof( buf ) );

 int error( 0 );
 int lineNr( 0 );
 long inputLine( 0 );
 long chrs( 0 );

 bool bufferBogus( false );

 eTextRefLine refLine( e_TextNUL );
 t_uchar uChr( 0 );
 gString sKeep;

 const bool showBuffers( opt.sedLevel > 6 );

 FILE* fDebug( ((opt.sedLevel > 3) && (fOut!=stdout)) ? stdout : nil );

 ASSERTION(fOut,"fOut");
 ASSERTION(fIn,"fIn");

 handle = fileno( fIn );

 if ( once<=0 ) {
     error = anchored_config( fRepErr, opt.myConfig, opt.zValue, fOut==stdout, sourceIsHtml, opt.htmlized );
     if ( error ) {
	 LOG_ERR("Invalid config, error-code: %d", error);
	 return 4;
     }

     if ( fRepErr ) {
	 if ( opt.htmlized.addBaseHREF.N() ) {
	     LOG_ME(LOG_NOTICE, "Using base href=%s", opt.htmlized.addBaseHREF.Str());
	 }
     }
     once = 1;
 }

 for (idx=0; io_readchr( handle, uChr )==1; ) {
     buf[ idx ] = 0;
     chrs++;

     if ( idx>=bufSize-2 ) {
	 sKeep.Add( buf );
	 buf[ idx = 0 ] = 0;
     }
     if ( uChr=='\r' ) continue;

     if ( uChr=='\0' || uChr=='\n' ) {
	 refLine = (eTextRefLine)(uChr=='\n');
	 if ( refLine==e_TextNewLine ) {
	     inputLine++;
	     if ( sourceIsHtml ) {
		 refLine = e_HtmlNewLine;
	     }
	 }

	 if ( fDebug ) {
	     fprintf(fDebug, "Line %ld: len=%d (ASCII: %3dd)\n",
		     inputLine,
		     idx,
		     (int)uChr);

	     if ( showBuffers ) {
		 fprintf(fDebug, "\t");
		 dump_buffer( fDebug, 55, buf, idx );
		 fprintf(fDebug, "\n\n");
	     }
	 }

	 idx = flush_anchor( opt.htmlized, fOut, fRepErr, ++lineNr, refLine, sKeep, buf );

	 // Stdin handle is 0 (seek is not interesting
	 bufferBogus = handle!=0 && chrs!=(long)lseek( handle, 0L, SEEK_CUR);
	 if ( bufferBogus ) {
	     if ( fDebug ) {
		 fprintf(fDebug, "Only parsed (%d) %ld line(s)\n", handle, inputLine);
	     }
	     break;
	 }
     }
     else {
	 if ( uChr!=128 && uChr>=127 && uChr<0xA0 ) {
	     LOG_ME(LOG_WARN, "Invalid char: %ud", uChr);
	 }
	 else {
	     buf[ idx++ ] = uChr;
	     buf[ idx ] = 0;
	 }
     }
 }
 flush_anchor( opt.htmlized, fOut, fRepErr, lineNr, e_TextEOF, sKeep, buf );

 if ( opt.isVeryVerbose ) {
     const char* strTag( nil );

     if ( opt.htmlized.first & 8 ) {
	 strTag = opt.htmlized.tags.Str( 1 );
     }

     LOG_ME(LOG_INFO, "Dumped %d lines (%s%ld, in: %d, out: %d)%s%s%s",
	    lineNr,
	    bufferBogus ? "only " : "",
	    inputLine,
	    opt.htmlized.lines,
	    opt.htmlized.outLines,
	    strTag ? " HTML/XML: " : "",
	    strTag ? strTag : "",
	    opt.htmlized.state ? " - unclosed text" : "");

     if ( opt.htmlized.countExcluded ) {
	 LOG_ME(LOG_INFO, "Excluded line(s): %d", opt.htmlized.countExcluded);
     }
 }
 return 0;
}


int do_norm (gList& args,
	     int subCmdIdx,
	     gList& capaList,
	     FILE* fIn,
	     FILE* fOut,
	     FILE* fRepErr,
	     sOptSed& opt)
{
#ifdef HAS_NORM
 int nrArgs( args.N() );
 int minArgs( capaList.GetObjectPtr( 4 )->GetInt() );
 int verboseLevel( opt.isVerbose + opt.isVeryVerbose*2 );
 int option( (int)opt.doAll );
 const char* strCommandName( capaList[ 1 ] );
 const char* strDescription( capaList[ 2 ] );

 if ( opt.zValue>-1 ) {
    option |= opt.zValue;
 }

 DBGPRINT("do_norm subCmdIdx: %d\n",subCmdIdx);
 DBGPRINT("in-isStdin?%c, out-isStdout?%c, fRepErr?%c, option: %d\n",
	  ISyORn( fIn==stdin ),
	  ISyORn( fOut==stdout ),
	  ISyORn( fRepErr!=0 ),
	  option);

 if ( nrArgs<minArgs ) {
     fprintf(stderr,"Insufficient args (%d) to %s (%s)\n",
	     nrArgs,
	     strDescription,
	     strCommandName);
     return 0;
 }

 return
     norm_command( subCmdIdx, capaList, fIn, fOut, fRepErr, args, option, verboseLevel );

#else

 return 0;

#endif //~HAS_NORM
}


int do_from_base (int base,
		  const char* optStrBase,
		  gList& args,
		  FILE* fIn,
		  FILE* fOut,
		  int zValue)
{
 static char strBase[ 128 ];
 static const char alpha64[]=I_BASE64_ALPHABET;

 gString sBase( (char*)optStrBase );
 gElem* ptrElem( args.StartPtr() );
 const char* strNum;
 int error( 0 );
 int thisError;

 // Because we use strBase as static, and call this function only once, we do not initialize it

 sBase.DownString();

 if ( base<=0 ) {
     if ( sBase.Find( "hex" )==1 )
	 base = 16;
     else {
	 if ( sBase.Find( "oct" )==1 ) {
	     base = 8;
	 }
     }
 }
 if ( base<=0 ) return -1;

 if ( base>16 ) {
     if ( base>64 ) return -1;

     strncpy( strBase, alpha64, base );
 }
 else {
     int aBase( base > 10 ? 10 : base );
     strncpy( strBase, alpha64 + 26+26, aBase );
     strcat( strBase, alpha64 );
     strBase[ base ] = 0;
 }

 DBGPRINT("DBG: do_from_base base=%d, strBase {%s}\n",
	  base,
	  strBase);

 for ( ; (ptrElem=ptrElem->next)!=nil; ) {
     strNum = ptrElem->Str();
     if ( strNum[ 0 ] ) {
	 thisError = show_from_base( base, strNum, strBase, fOut );
	 if ( thisError ) {
	     error = thisError;
	 }
     }
 }
 return error;
}


int do_undo (const char* strUndo, gString& sUndoRef)
{
 int len;
 int pos;
 bool isOk( false );
 const char* strEnv( strUndo );
 const int error( 0 );

 sUndoRef.Set( (char*)strEnv );

 if ( sUndoRef.Length() ) {
      sUndoRef.Add( gSLASHCHR );
      sUndoRef.Add( ".last" );

      gFileFetch ref( sUndoRef );
      if ( ref.IsOpened() ) {
	  gString sBase( ref.aL.Str( 1 ) );
	  gString sName;

	  len = strlen( UNDO_SUFFIX );
	  if ( len > 0 && (pos = ((int)sBase.FindBack( UNDO_SUFFIX )))+len==(int)sBase.Length()+1 && pos > 0 ) {
	      sName.Set( sBase.BaseName( UNDO_SUFFIX ) );
	      isOk = sName.Find( UNDO_PREFIX )==1;
	      sName.Delete( 1, strlen( UNDO_PREFIX ) );
	  }

	  if ( isOk ) {
	      gFileStat aStat( sName.Str() );

	      isOk = aStat.HasStat() && aStat.IsDirectory()==false;

	      if ( isOk ) {
		  printf("Undo: copy {%s} to {%s}, ok? %c\n",
			 sBase.Str(),
			 sName.Str(),
			 ISyORn( isOk ));
	      }
	  }
	  return error;
      }
 }
 else {
     return -1;
 }
 return 2;
}


int do_subst_file (FILE* fRepErr, bool doStdin, sOpts& opts, const char* strFile, gList& outLines)
{
 int anySubst( 0 );
 int substCode( -1 );
 gString aFile( (char*)strFile );
 gFileFetch inFile( (char*)strFile );
 gElem* ptrElem( inFile.aL.StartPtr() );
 const char* strEnv( getenv( "UNDO" ) );

 gString sUndoBase( (char*)strEnv );
 gString sUndoFile;
 gString sUndoRef;  // a file with just the reference of the last file used
 bool allowUndo( false );
 FILE* fUndo( nil );

 DBGPRINT("do_subst_file doStdin? %c, lines: %u, OK? %c {%s}\n",
	  ISyORn( doStdin ),
	  inFile.aL.N(),
	  ISyORn( inFile.IsOpened() ),
	  strFile);

 if ( inFile.IsOpened()==false )
     return 2;

 if ( strFile ) {
     if ( sUndoBase.Length() ) {
	 if ( sUndoBase.Find( gSLASHCHR ) ) {
	     sUndoFile = sUndoBase;
	     sUndoFile.Add( gSLASHCHR );
	     sUndoRef = sUndoFile;
	     sUndoFile.Add( UNDO_PREFIX );
	     sUndoFile.Add( aFile.BaseName() );
	     sUndoFile.Add( UNDO_SUFFIX );
	     sUndoRef.Add( ".last" );

	     DBGPRINT("DBG: skip undo? %d || %d\n", sUndoFile.Match( sUndoRef ), sUndoFile.Match( (char*)strFile ));
	     if ( sUndoFile.Match( sUndoRef ) || sUndoFile.Match( (char*)strFile ) ) {
	     }
	     else {
		 allowUndo = true;
		 fUndo = fopen( sUndoFile.Str(), "wb" );
	     }

	     if ( fRepErr ) {
		 DBGPRINT("DBG: Using UNDO? %d: %s\n", fUndo!=nil, sUndoFile.Str());
		 if ( fUndo ) {
		     LOG_ME(LOG_NOTICE, "Using UNDO at: %s", sUndoBase.Str());
		 }
		 else {
		     if ( allowUndo ) {
			 LOG_ERR("Unable to use UNDO: %s", sUndoFile.Str());
		     }
		     else {
			 LOG_ERR("UNDO clashes with original file, denied: %s", sUndoBase.Str());
		     }
		 }
	     }
	 }
	 else {
	     fprintf(stderr, "UNDO path is not absolute: %s\n", sUndoBase.Str());
         }
     }
     else {
	 // No undo used by user
     }
 }

 for ( ; ptrElem; ptrElem=ptrElem->next) {
     gString sLine( ptrElem->Str() );
     gString to;

     if ( fUndo ) {
	 fprintf(fUndo, "%s\n", sLine.Str());
     }

     substCode = opts.Subst( sLine, to );
     if ( substCode ) {
	 if ( fRepErr ) {
	     if ( anySubst==0 ) {
		 if ( strFile ) {
		     fprintf(fRepErr, "%s:", strFile);
		 }
	     }
	     if ( doStdin==false ) {
		 fprintf(fRepErr, "\n\
%s\n\
%s\n\
",
			 sLine.Str(),
			 to.Str());
	     }
	 }
	 anySubst = 1;
	 outLines.Add( to );
     }
     else {
	 outLines.Add( sLine );
     }
 }

 if ( fUndo ) {
     fclose( fUndo );
     if ( sUndoRef.Length() ) {
	 gFileFetch ref( sUndoRef );
	 if ( ref.IsOpened() ) {
	     gString sLast( ref.Str( 1 ) );
	     if ( sLast.Length() ) {
		 if ( sLast.Find( sUndoBase )!=1 ) {
		     fprintf(stderr, "Ignoring: %s\n", sUndoRef.Str());
		 }
		 else {
		     // Remove last undo-file
		     DBGPRINT("DBG: remove old file referenced by %s: %s\n", sUndoRef.Str(), sLast.Str());
		     remove( sLast.Str() );
		 }
	     }
	 }
     } // end IF any sUndoRef file

     if ( sUndoRef.Length() && sUndoFile.Length() ) {
	 fUndo = fopen( sUndoRef.Str(), "wb" );
	 DBGPRINT("DBG: reference file: {%s}, p=0x%p\n", sUndoRef.Str(), fUndo);
	 if ( fUndo ) {
	     fprintf(fUndo, "%s\n", sUndoFile.Str());
	     fclose( fUndo );
	 }
     }
 }

 return anySubst;
}


int do_trim_file (FILE* fRepErr, bool doStdin, sOpts& opts, const char* strFile, bool& doDOS, gList& outLines)
{
 int anySubst( 0 );
 int len( -1 ), diffLen;
 int check;
 int didRead;
 FILE* fIn( doStdin ? stdin : nil );
 gFileFetch inFile( (char*)strFile );
 gElem* ptrElem( inFile.aL.StartPtr() );
 char firstLine[ 4096 ];

 if ( inFile.IsOpened()==false )
     return 2;

 if ( fIn==nil ) {
     fIn = fopen( strFile, "rb" );
 }
 if ( fIn==nil ) return 2;

 firstLine[ 0 ] = 0;
 didRead = io_read( fileno( fIn ), firstLine, sizeof(firstLine)-1 );
 fclose( fIn );
 check = check_text_line( firstLine, didRead, len );

 if ( didRead<0 ) return 0;
 if ( check<=-1 ) {
     LOG_ME(LOG_INFO, "Skipping binary: %s", strFile);
     return 0;
 }

 if ( doDOS==false ) {
     doDOS = check==1;
 }

 LOG_ME(LOG_NOTICE, "Checking: %s%s",
	strFile,
	doDOS ? " (DOS text-file)" : "");

 for ( ; ptrElem; ptrElem=ptrElem->next) {
     gString* pLine( (gString*)ptrElem->me );
     len = pLine->Length();
     pLine->TrimRight();
     diffLen = len - (int)pLine->Length();
     if ( diffLen ) {
	  anySubst = 1;
	  opts.linesSubsts++;
	  opts.countSubsts += diffLen;
     }
     outLines.Add( *pLine );
 }
 return anySubst;
}


int do_subst_files (FILE* dump,
		    FILE* fRepErr,
		    gString& sExpr1,
		    gString& sExpr2,
		    int verboseLevel,
		    bool writeAlways,
		    gList& args,
		    gList& myConfig)
{
 bool doStdin( args.N()==0 );
 bool doDOS( false );
 int error( 0 );
 int writeError( 0 );
 int thisError( -1 );
 int iter( 1 ), maxIter( args.N() + (int)doStdin );
 int pos, value( -1 );
 gString rValue;
 gString* ptrStr;
 const char* strFile( nil );
 const char* strDosModeCR( "\0" );
 sOpts opts;

 // Parse config

 //	substitute_similar_slashes=true
 ptrStr = find_config( myConfig, "substitute_similar_slashes=", rValue );
 if ( ptrStr ) {
     value = config_str_boolean( rValue, thisError );
     error += thisError;
     opts.substituteSimilarSlashes = value==1;
 }

 if ( find_config_statement( myConfig, "dos_mode", pos ) ) {
     doDOS = true;
     if ( pos ) error = 1;  // 'dos_mode' is a statement
 }

 if ( error ) {
     fprintf(stderr,"At least one config error.\n");
     return -1;
 }

 // fill-in 'opts'
 opts.ResetCounters();
 opts.expr1.Add( sExpr1 );
 opts.expr2.Add( sExpr2 );

 if ( opts.substituteSimilarSlashes ) {
     if ( sExpr1.Find( '/' ) ) {
	 sExpr1.ConvertChrTo( '/', '\\' );
	 sExpr2.ConvertChrTo( '/', '\\' );
	 opts.expr1.Add( sExpr1 );
	 opts.expr2.Add( sExpr2 );
     }
 }

 if ( verboseLevel>=9 ) {
     fprintf(stderr,"Similar slashes: %s\
%c",
	     config_bool_str( (int)opts.substituteSimilarSlashes ),
	     '\n');
 }

 // Iterate through the files in arguments

 for ( ; iter<=maxIter; iter++) {
     gList outLines;
     int substCode( -1 );

     strFile = doStdin ? nil : args.Str( iter );
     if ( sExpr1.Match( "." )==1 ) {
	 if ( sExpr2.Match( "trim" ) ) {
	     substCode = do_trim_file( fRepErr, doStdin, opts, strFile, doDOS, outLines );

	     DBGPRINT("DBG: do_trim_file (%s) doStdin? %c, returned %d, lines: %u\n",
		      strFile,
		      ISyORn( doStdin ),
		      substCode,
		      outLines.N());
	 }
     }
     if ( substCode==-1 ) {
	 substCode = do_subst_file( fRepErr, doStdin, opts, strFile, outLines );
     }
     if ( doDOS ) {
	 strDosModeCR = "\r";
     }

     thisError = substCode>=2;
     if ( thisError ) {
	 error = thisError;
	 LOG_ERR("Cannot open: %s", strFile);
     }
     else {
	 FILE* fOut( dump );
	 gElem* ptrElem( outLines.StartPtr() );
	 bool dumpEverything( dump && dump!=stdout );

	 ASSERTION(fOut,"fOut");

	 if ( substCode==1 || doStdin==true || writeAlways ) {
	     // Overwrite without using a temporary first:
	     if ( doStdin==false ) {
		 fOut = fopen( strFile, "wb" );
	     }
	     if ( fOut ) {
		 for ( ; ptrElem; ptrElem=ptrElem->next) {
		     fprintf(fOut,"%s%s\n",ptrElem->Str(),strDosModeCR);
		     if ( dumpEverything ) {
			 fprintf(dump, "%s%s\n",ptrElem->Str(),strDosModeCR);
		     }
		 }
		 if ( fOut!=stdout ) {
		     fclose( fOut );
		     fOut = nil;
		 }
	     }
	     else {
		 writeError++;
		 LOG_ERR("Cannot replace file: %s", strFile);
	     }
	 }

	 if ( substCode==0 ) {
	     if ( strFile && strFile[ 0 ] ) {
		 LOG_ME(LOG_WARN, "%s: nothing subst'ed%s",
			strFile,
			writeAlways ? " (but rewritten)" : "");
	     }
	     else {
		 LOG_ME(LOG_NOTICE, "Nothing subst'ed");
	     }
	 }
	 else {
	     if ( strFile ) {
		 LOG_ME(LOG_NOTICE, "%s: replaced file; %d substs, %d line(s)",
			strFile,
			opts.countSubsts,
			opts.linesSubsts);
	     }
	     else {
		 LOG_ME(LOG_NOTICE, "%d substs, %d line(s)",
			opts.countSubsts,
			opts.linesSubsts);
	     }
	 }
     }
 }//end FOR

 if ( writeError ) {
     error = 13;
     LOG_ME(LOG_WARN, "Cannot replace at least one file, returning error-code %d", error);
 }
 return error;
}


bool apply_line_rand (FILE* fOut,
		      int randomFactor,
		      int whence,
		      bool doAll,
		      gList& ioKeep,
		      short& i,
		      t_uchar* uBuf)
{
 gRandom r( whence );
 int value( r.GetInt()+1 );
 bool doIt( whence > 0 && value <= randomFactor );

 if ( doIt ) {
     if ( doAll ) {
	 ioKeep.Add( uBuf );
     }
     else {
	 fprintf(fOut, "%s\n", uBuf);
     }
 }

 uBuf[ i = 0 ] = 0;
 return doIt;
}


int line_rand (FILE* fIn, FILE* fOut, int verboseLevel, bool doAll, int zValue)
{
 int error( 0 );
 int randomFactor( zValue>=0 ? zValue : 100 );
 int whence( 1000 );
 short i( 0 );
 char chr( '\0' );
 t_uchar uBuf[ 4096 ];
 gRandom r;
 gList ioKeep;

 ASSERTION(fIn,"fIn");
 ASSERTION(fOut,"fOut");
 memset(uBuf, 0x0, sizeof(uBuf));

 r.GarbleSeed( IX_GETPID() );
 if ( verboseLevel ) {
     fprintf(stderr, "random %d for %d\n",
	     randomFactor,
	     whence);
 }

 for ( ; fscanf(fIn,"%c",&chr)==1; ) {
     if ( chr && chr!='\r' ) {
	 if ( chr=='\n' ) {
	     apply_line_rand( fOut, randomFactor, whence, doAll, ioKeep, i, uBuf );
	 }
	 else {
	     uBuf[ i++ ] = chr;
	 }
     }
     uBuf[ i ] = 0;
 }

 if ( i ) {
     apply_line_rand( fOut, randomFactor, whence, doAll, ioKeep, i, uBuf );
 }

 if ( doAll ) {
     // Show all lines kept
     for (int pos, elems=(int)ioKeep.N(); elems>0; elems--) {
	 gRandom r( elems );
	 pos = r.GetInt()+1;
	 fprintf(fOut, "%s\n", ioKeep.Str( pos ));
	 ioKeep.Delete( pos, pos );
     }
 }
 return error;
}


void dump_date (FILE* fOut, const char* strFormat, int type, time_t stamp)
{
 static char aDate[ 256 ];
 static struct tm* now;
 char* strDate;

 if ( type<=0 ) return;
 if ( type==1 ) {
     strDate = ctime( &stamp );
     strcpy( aDate, strDate );
     for (int len=strlen( aDate ); len>0; ) {
	 len--;
	 if ( aDate[ len ]=='\n' ) {
	     aDate[ len ] = 0;
	     break;
	 }
     }
     fprintf(fOut, "%s ", aDate);
 }
 else {
     now = localtime( &stamp );
     fprintf(fOut, strFormat, now->tm_year+1900, now->tm_mon+1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
 }
}


int do_tee (FILE* fOut, gString& sDateFormat, sOptSed& opt)
{
 FILE* fIn( stdin );
 FILE* fTee( fOut==stdout ? nil : stdout );
 char chr;
 const char* asFormat( "%04u-%02u-%02u %02u:%02u:%02u " );
 int type( 0 );
 gString aLine;

 ASSERTION(fOut,"fOut");
 DBGPRINT("sDateFormat: %s\n", sDateFormat.Str());

 if ( sDateFormat.Length() ) {
     type = 1;
     if ( sDateFormat.Match( "TOD", true ) ) {
	 type = 2;
     }
 }

 for ( ; fscanf(fIn, "%c", &chr)==1; ) {
     aLine.Add( chr );
     if ( chr=='\n' ) {
	 if ( aLine.Length() > 1 ) {
	     dump_date( fOut, asFormat, type, time( NULL ) );
	 }
	 fprintf(fOut, "%s", aLine.Str());
	 if ( fTee ) {
	     fprintf(fTee, "%s", aLine.Str());
	 }
	 aLine.Delete();
     }
 }

 return 0;
}


int do_strings (int handle, FILE* fOut, int variant, bool isVerbose, gUniCode& uni)
{
 long lineNr( 1 );
 int weirdChrs( 0 );
 t_uchar uChr, toShow( '\0' );
 char same;

 const bool reqString( variant!=0 );  // command 27 ('string'), not 'strings'

 ASSERTION(handle!=-1,"handle");
 ASSERTION(fOut,"fOut");

 if ( reqString ) {
     for ( ; io_readchr( handle, uChr )==1; ) {
	 same = uni.hash256Basic[ gUniCode::e_Basic_Printable ][ uChr ];

	 if ( same==-1 && uChr!=255 ) {
	     weirdChrs++;

	     if ( isVerbose ) {
		 printf("[0x%02X]", (unsigned)uChr);
	     }
	 }
	 else {
	     fprintf(fOut, "%c", uChr);
	 }

	 if ( uChr=='\n' && weirdChrs ) {
	     LOG_ME(LOG_NOTICE, "line %ld has invalid char(s): %d",
		    lineNr,
		    weirdChrs);
	     weirdChrs = 0;
	     lineNr++;
	 }
     }
 }
 else {
     for ( ; io_readchr( handle, uChr )==1; ) {
	 if ( uChr=='\n' ) {
	     if ( weirdChrs ) {
		 LOG_ME(LOG_NOTICE, "line %ld has non-Latin1 char(s): %d",
			lineNr,
			weirdChrs);
	     }
	     weirdChrs = 0;
	     lineNr++;
	 }
	 same = uni.hash256Basic[ gUniCode::e_Basic_Printable ][ uChr ];
	 if ( same<=0 ) {
	     weirdChrs++;
	     if ( toShow=='\n' ) continue;
	     toShow = '\n';
	 }
	 else {
	     toShow = uChr;
	 }
	 fprintf(fOut, "%c", toShow);
     }

     if ( toShow!='\n' ) {
	 fprintf(fOut, "\n");
     }
 }

 return 0;
}


int do_urlx (gList& args, FILE* fIn, FILE* fOut, sOptSed& opt)
{
 const bool checkURL( opt.doAll );
 gString sTOD( "0000-00-00 12:00:00 Z" );
 gList* newURLX;
 gElem* pElem;

 DBGPRINT("args # %u  fIn=0x%p%s, fOut=0x%p%s\n",
	  args.N(),
	  fIn, fIn==stdin ? " (stdin)" : "",
	  fOut, fOut==stdout ? " (stdout)" : "");

 ASSERTION(fIn, "fIn");
 ASSERTION(fOut, "fOut");

 if ( args.N()==0 ) {
     newURLX = new_urlx_from_file( fileno( fIn ), checkURL );
 }
 else {
     newURLX = new_urlx_from_list( fOut, opt.sOutput, args, 1, checkURL );
 }
 if ( newURLX==nil ) {
     if ( fOut!=stdout ) {
	 perror( "Uops" );
     }
     return 1;
 }

 for (pElem=newURLX->StartPtr(); pElem; pElem=pElem->next) {
     printf("\t%d.%d\t{%s}\n",
	    pElem->iValue,
	    pElem->me->iValue,
	    pElem->Str());
 }

 delete newURLX;

 return 0;
}


int do_xurl (gList& args, FILE* fIn, FILE* fOut, sOptSed& opt)
{
 DBGPRINT("args # %u  fIn=0x%p%s, fOut=0x%p%s\n",
	  args.N(),
	  fIn, fIn==stdin ? " (stdin)" : "",
	  fOut, fOut==stdout ? " (stdout)" : "");

 fprintf(stderr, "do_xurl() - unimplemented\n");
 /// #warning TODO
 return 0;
}


int show_unescape (gList& args, FILE* fIn, FILE* fOut, sOptSed& opt, int mask)
{
 int error;
 gElem* pElem;
 gElem* pNext;
 gList output;

 ASSERTION(fOut,"fOut");
 ASSERTION(fIn,"fIn");

 error = gen_unescape( args, output, mask );

 for (pElem=output.StartPtr(); pElem; pElem=pNext) {
     pNext = pElem->next;
     fprintf(fOut, "%s%s",
	     pElem->Str(),
	     pNext ? " " : "\n");
 }
 return error;
}


int do_timex (gList& args, FILE* fIn, FILE* fOut, FILE* fRepErr, sOptSed& opt)
{
 int error( 0 );
 int wrongConsistency( 0 );
 t_int16 ms3or6( (int)opt.zValue );
 gString* pTime( nil );
 gList* timed( nil );
 gList* dateList( nil );
 gString* pFirst( nil );
 gElem* timy;

 time_t now( time( NULL ) );
 const char* strDate( nil );
 const bool showMillisEtc( ms3or6==3 || ms3or6==6 );

 FILE* fComplain( opt.isVerbose ? fRepErr : nil );

 ASSERTION(fOut,"fOut");
 ASSERTION(fIn,"fIn");
 ASSERTION(fRepErr,"fRepErr");

 if ( args.N() ) {
     pTime = (gString*)args.StartPtr()->me;
 }

 if ( pTime ) {
     timed = time_list( fComplain, *pTime, -1, ms3or6 );
 }

 //printf("timed: "); timed->Show(); printf("!\n");

 if ( timed ) {
     error = timed->iValue;
     DBGPRINT("DBG: error-code %d, N()=%u\n", timed->iValue, timed->N());

     wrongConsistency = time_list_fix( fComplain, *timed );
     if ( wrongConsistency ) {
	 LOG_ME(LOG_WARN, "Wrong consistency%s, code %d%s",
		(wrongConsistency & 64) ? "" : " (fixed)",
		wrongConsistency,
		(wrongConsistency & 32) ? " (minutes)" : "");
     }

#ifdef DEBUG
     for (gElem* p=timed->StartPtr(); p; p=p->next) { printf("\t(%p,%d) %d\t%s\n", p, p->iValue, p->me->iValue, p->Str()); }
#endif
     ASSERTION(timed->N() >= 3,"<3?");

     timy = timed->StartPtr();
     pFirst = (gString*)timy->me;

     if ( opt.isVerbose ) {
	 fprintf(fOut, "%s%s:%02d:%02d%s\n",
		 (*pFirst)[ 1 ]=='+' ? "in " : "", pFirst->Str(),
		 timy->next->me->iValue,
		 timy->next->next->me->iValue,
		 (timed->N() >= 4) ? (timy->next->next->next->Str()) : "");
     }
     else {
	 fprintf(fOut, "%02d:%02d:%02d\n",
		 pFirst->iValue,
		 timy->next->me->iValue,
		 timy->next->next->me->iValue);
     }
 }
 else {
     strDate = ctime( &now );  // POSIX date

     if ( showMillisEtc ) {
	 struct timeval tv;
	 struct timezone tz;
	 memset(&tz, 0, sizeof(tz));
	 gettimeofday(&tv, &tz);
	 dateList = ctime_date_micro( tv, ms3or6 );
     }
     else {
	 dateList = ctime_date( now );
     }
 }

 if ( strDate ) {
     gString* newStr( join_strings( *dateList, " " ) );

     if ( opt.isVerbose ) {
	 printf("%s\n", newStr->Str());
     }
     else {
	 printf("%s", strDate);
     }

     delete newStr;
 }

 delete dateList;
 delete timed;

 DBGPRINT("DBG: do_timex returns %d\n", error);
 return error;
}


int do_batch (const char* strInput, FILE* fIn, FILE* fOut, FILE* fRepErr, sOptSed& opt, int zValue)
{
 int error( 0 );
 int nCommands( 0 );
 gList* myBatch;
 gList commandsOnce;
 gString lastLine;

 DBGPRINT("strInput={%s}, fIn=%p, fOut=%p%s, fRepErr=%p%s, zValue=%d, verbose? %c,%c\n",
	  strInput,
	  fIn,
	  fOut, (fOut==stdout ? " (stdout)" : ""),
	  fRepErr, (fRepErr==stderr ? " (stderr)" : ""),
	  zValue,
	  ISyORn( opt.isVerbose ), ISyORn( opt.isVeryVerbose ));
 if ( fIn ) {
     myBatch = read_batch_input( fileno( fIn ) );
     DBGPRINT("DBG: read_batch_input() returned #%u, error-code: %d\n",
	      myBatch->N(), myBatch->iValue);

     for (gElem* dPtr=myBatch->StartPtr(); dPtr; dPtr=dPtr->next) {
	 if ( dPtr->iValue > 0 ) {

	     gList* dList( (gList*)dPtr->me );
	     gList* listedArg( (gList*)dList->StartPtr()->next->me );

	     DBGPRINT("%d\t%s. [#%u]\n",
		      dPtr->iValue,
		      dList->Str(),
		      listedArg->N());
#if 0
	     for (gElem* w=listedArg->StartPtr(); w; w=w->next) {
		 printf("\t{%s}\n%s", w->Str(), w->next ? "" : "\n");
	     }
#endif
	     nCommands++;
	     btc_add_once( listedArg->Str(), commandsOnce );
	     DBGPRINT("COMMAND# %d (different: #%u): %s\n",
		      nCommands,
		      commandsOnce.N(),
		      listedArg->Str());
	 }
     }

     if ( nCommands ) {
	 if ( nCommands!=(int)commandsOnce.N() ) {
	     LOG_ME(LOG_INFO, "%s: %d command(s), %u different, first: %s", strInput, nCommands, commandsOnce.N(), commandsOnce.Str( 1 ) );
	 }
	 else {
	     LOG_ME(LOG_INFO, "%s: %d command(s), first: %s", strInput, nCommands, commandsOnce.Str( 1 ) );
	 }
     }

     btc_run_batch( fOut, *myBatch, lastLine );
     if ( lastLine.Length() ) {
	 LOG_ME(LOG_WARN, "Last output: %s\n", lastLine.Str());
     }

     // Delete created list
     delete myBatch;
 }
 return error;
}

////////////////////////////////////////////////////////////
int do_run (int cmdNr,
	    gList& args,
	    FILE* fOut,
	    FILE* fRepErr,
	    sOptSed& opt)
{
 int subCmdIdx( -1 );
 int iter( 1 ), nArgs( args.N() ), nIters( nArgs ? nArgs : 1 );
 int how( 0 );
 int thisError( -1 ), error( 0 );
 int zValue( opt.zValue );
 int verboseLevel( (int)opt.isVerbose + (int)opt.isVeryVerbose*6 + (zValue>0)*2 );
 bool sourceIsHtml( false );

 gString sNamed;
 gList capaList;
 char* aStr( nil );
 const char* refStr( nil );

 FILE* fIn( stdin );
 FILE* fDetails( opt.isVerbose ? stderr : nil );

 ASSERTION(fOut,"fOut");

 gString sArgOne( args.Str( 1 ) );
 gString sArgTwo( args.Str( 2 ) );

 opt.PrepareUni();

 switch ( cmdNr ) {
 case 12:
     error = nArgs!=2;
     if ( error ) {
	 fprintf(stderr,"Assuming empty user and/or pass pairs.\n");
     }
     thisError = show_auth_mime64( fOut, sArgOne, sArgTwo );
     nIters = -1;
     break;

 default:
     break;
 }

 for (iter=1; iter>=1 && iter<=nIters; iter++) {
     aStr = args[ iter ];
     refStr = nArgs ? aStr : nil;

     switch ( cmdNr ) {
     case 1:	// test
	 thisError = do_test( fOut, aStr, opt );
	 break;

     case 2:	// to-base64
     case 5:    // to-base65
	 if ( zValue >= 0 ) {
	     if ( zValue < (int)strlen( aStr ) )
		 aStr[ zValue ] = 0;
	 }
	 thisError = do_to_base6x( fOut, aStr, opt, cmdNr==5 );
	 break;

     case 3:
     case 4:
     case 6:
     case 7:
	 thisError = do_from_base6x( fOut, aStr, zValue, opt, cmdNr>=5, cmdNr==4 || cmdNr==7 );
	 if ( thisError ) {
	     if ( fRepErr ) {
		 if ( thisError==2 ) {
		     LOG_ERR("Unable to convert to text: %s", aStr);
		 }
		 else {
		     LOG_ERR("Unable to convert from: %s", aStr);
		 }
	     }
	 }
	 break;

     case 9:
	 if ( nArgs ) {
	     fIn = fopen( aStr, "r" );
	     if ( fIn==nil ) {
		 LOG_ERR("Cannot open: %s", aStr);
		 thisError = 2;
	     }
	     else {
		 LOG_ME(LOG_INFO, "Opened: %s", aStr);
	     }
	 }
	 if ( fIn ) {
	     thisError = do_convert_from_utf8( fIn, fOut, fDetails, zValue<0 ? 0 : zValue, opt );
	 }
	 break;
     case 33:
	 if ( nArgs ) {
	     fIn = fopen( aStr, "r" );
	     if ( fIn==nil ) {
		 LOG_ERR("Cannot open: %s", aStr);
		 thisError = 2;
	     }
	     else {
		 LOG_ME(LOG_INFO, "Opened: %s", aStr);
	     }
	 }
	 if ( fIn ) {
	     thisError = do_convert_from_any_utf8( fIn, fOut, fDetails, 1, opt );
	 }
	 break;

     case 11:
	 if ( nArgs ) {
	     thisError = do_url_builder( nil, fOut, aStr, zValue, 11 );
	 }
	 else {
	     thisError = do_url_builder( fIn, fOut, nil, zValue, 11 );
	 }
	 break;

     case 13:
	 if ( nArgs==0 ) {
	     thisError = 1;
	     fprintf(stderr,"No 'norm' command given.\n");

	     norm_capa( nil, capaList );
	     printf( "\nAvailable commands:\n%s\n",
		     x_newstr_from_list( capaList, "\t%s\n", 0 ) );
	 }
	 else {
	     iter = -9;
	 }
	 break;

     case 14:  // undo
	 iter = -9;
	 break;

     case 15:  // line-histogram
	 if ( nArgs ) {
	     fIn = fopen( aStr, "r" );
	 }
	 DBGPRINT_MIN("DBG: show_histogram file? %c, zValue=%d\n",
		      ISyORn( fIn ),
		      zValue);
	 if ( fIn ) {
	     thisError = show_histogram( fIn, fOut, nil, zValue, 0 );
	 }
	 else {
	     thisError = 2;
	 }
	 DBGPRINT_MIN("DBG: Objs AFT: %d, error=%d\n",gStorageControl::Self().NumObjs(),error);
	 break;

     case 16:  // lines
	 ASSERTION(fIn,"fIn");
	 how = (strcmp( aStr, "first" )==0);
	 how |= (strcmp( aStr, "next" )==0)*2;
	 iter = -9;
	 break;

     case 17:  // line-rand
	 if ( nArgs ) {
	     fIn = fopen( aStr, "r" );
	 }
	 if ( fIn ) {
	     thisError = line_rand( fIn, fOut, verboseLevel, opt.doAll, zValue );
	 }
	 else {
	     thisError = 2;
	     if ( verboseLevel ) {
		 fprintf(stderr, "Cannot open file: %s, %s\n",
			 aStr,
			 gFileControl::Self().ErrorStr( errno ));
	     }
	 }
	 break;

     case 18:  // subst
	 iter = -9;
	 break;

     case 19:
	 if ( nArgs ) {
	     gString sName( aStr );
	     sourceIsHtml = sName.Find( ".htm", true )+5>=sName.Length();
	     DBGPRINT("DBG: Open %s, HTML? %c\n",
		      aStr,
		      ISyORn( sourceIsHtml ));

	     fIn = fopen( aStr, "r" );
	 }

	 if ( fIn==nil ) {
	     thisError = errno;
	     fprintf(stderr,"Uops (%s): %s\n",
		     gFileControl::Self().ErrorStr( thisError ),
		     aStr);
	 }
	 else {
	     thisError = do_anchor( refStr, fIn, fOut, fRepErr, sourceIsHtml, opt );
	 }
	 break;

     case 20:  // datex
	 if ( nArgs ) {
	     LOG_ME(LOG_INFO, "dump: %s (%d of %u)", aStr, iter, nArgs);
	     fIn = fopen( aStr, "r" );
	 }
	 if ( fIn ) {
	     thisError = show_datex( fIn, fOut, zValue );
	 }
	 else {
	     thisError = 2;
	 }
	 break;

     case 21:  // dated
	 // Same as above, but delayed because of order used
	 iter = -9;
	 break;

     case 30:  // unescape
	 iter = -1;

     case 31:  // cat
	 iter = -9;
	 break;

     case 22:	// timex
	 iter = -9;
	 break;

     case 23:
	 iter = -9;
	 break;

     case 24:	// from-date
	 thisError = show_from_date( fIn, fOut, zValue, aStr );
	 break;

     case 25:  // tee
	 iter = -1;
	 error = nArgs>1;
	 if ( error ) {
	     fprintf(stderr, "Ignored params: %d\n", nArgs-1);
	 }
	 do_tee( fOut, sArgOne, opt );
	 break;

     case 26:  // strings
     case 27:  // string
	 if ( nArgs ) {
	     fIn = fopen( aStr, "r" );
	 }

	 if ( fIn==nil ) {
	     thisError = errno;
	     fprintf(stderr,"Uops (%s): %s\n",
		     gFileControl::Self().ErrorStr( thisError ),
		     aStr);
	 }
	 else {
	     thisError = do_strings( fileno( fIn ), fOut, cmdNr==27, opt.isVerbose, opt.uni );
	 }
	 break;

     case 28:  // nice-urlx
     case 29:  // nice-xurl
	 iter = -9;
	 break;

     case 32:  // batch
	 if ( nArgs ) {
	     fIn = fopen( aStr, "r" );
	     if ( fIn ) {
		 if ( nArgs>1 ) {
		     LOG_ME(LOG_INFO, "batch: %s (%d of %u)", aStr, iter, nArgs);
		 }
		 else {
		     LOG_ME(LOG_INFO, "batch: %s", aStr);
		 }
	     }
	 }
	 if ( fIn ) {
	     thisError = do_batch( aStr, fIn, fOut, fRepErr, opt, zValue );
	 }
	 else {
	     thisError = 2;
	 }
	 break;

     case 12:
     case 10:
     case 8:
     default:
	 IM_ASSERTION_FALSE("uops\n");
     }

     if ( thisError ) {
	 error = thisError;
     }

     if ( fIn && fIn!=stdin ) {
	 fclose( fIn );
	 fIn = stdin;
     }
 }// end FOR (args)

 if ( iter<=-1 ) {
     switch ( cmdNr ) {
     case 9:
	 break;

     case 13:
	 aStr = args[ 1 ];
	 subCmdIdx = norm_capa( aStr, capaList );
	 thisError = subCmdIdx<=0;

	 if ( thisError ) {
	     if ( subCmdIdx ) {
		 fprintf(stderr,"Unsupported norm.\n");
	     }
	     else {
		 fprintf(stderr,"Invalid command: '%s'\n",aStr);
	     }
	 }
	 else {
	     args.Delete( 1, 1 );
	     error = do_norm( args, subCmdIdx, capaList, fIn, fOut, fRepErr, opt );
	     DBGPRINT("do_norm returned %d\n",error);
	 }
	 break;

     case 14:  // undo
	 if ( nArgs > 0 ) {
	     fprintf(stderr, "No arg required for 'undo'\n");
	 }
	 else {
	     error = do_undo( getenv( "UNDO" ), sNamed );
	     if ( error ) {
		 fprintf(stderr,"Unable to undo%s%s\n", sNamed.Length() ? ": " : "", sNamed.Str());
	     }
	 }
	 break;

     case 16:
	 aStr = args.Str( 2 );
	 nArgs--;
	 if ( nArgs>0 && aStr[ 0 ] ) {
	     fIn = fopen( aStr, "rt" );
	     thisError = fIn==nil;
	     if ( thisError ) {
		 fprintf(stderr,"Cannot open: %s\n",aStr);
	     }
	 }
	 else {
	     // std input
	     thisError = content_name( opt.sContent )[ 0 ]==0;
	     if ( thisError ) {
		 fprintf(stderr,"Unable to use env var: SSED_LINES\n");
		 return 1;
	     }
	 }
	 if ( thisError==0 ) {
	     error = -1;
	     content_index( opt.sTmpFile );
	     if ( opt.sTmpFile.Length() ) {
		 error = do_lines( fIn, fOut, aStr, how==1, how==2, how, opt );
	     }
	     DBGPRINT("DBG: do_lines error=%d {%s}\n",
		      error,
		      aStr);
	 }

	 if ( error ) {
	     if ( error<0 ) {
		 fprintf(stderr, "Internal error: %d\n", -error);
	     }
	     else {
		 if ( errno ) {
		     fprintf(stderr, "Error (%d): %s\n",
			     errno,
			     errno < 0 ? "??" : gFileControl::Self().ErrorStr( errno ));
		 }
		 else {
		     if ( opt.isVerbose ) {
			 fprintf(stderr, "<EOF>\n");
		     }
		 }
	     }
	 }
	 break;

     case 18:  // subst
	 if ( nArgs<2 ) {
	     fprintf(stderr,"Missing expr1 / expr2.\n");
	     return 1;
	 }
	 else {
	     bool writeAlways( opt.doAll );
	     gString sExpr1( args.Str( 1 ) );
	     gString sExpr2( args.Str( 2 ) );

	     args.Delete( 1, 2 );
	     if ( sExpr1.IsEmpty() ) {
		 return 2;
	     }

	     error = do_subst_files( fOut, fRepErr, sExpr1, sExpr2, verboseLevel, writeAlways, args, opt.myConfig );
	 }
	 break;

     case 21:  // dated
     case 31:  // cat
	 if ( nArgs ) {
	     bool tryOlderNamesFirst( (zValue & 1)!=0 || cmdNr!=31 );
	     int pos( -1 );
	     gList* order( ptm_sort_names( args, 0 ) );
	     gList* inv( ptm_invert_list( *order, 0 ) );
	     gList shown;
	     char* strFirst( order->Str( 1 ) );
	     gElem* ptrElem( inv->StartPtr() );

	     for ( ; ptrElem; ptrElem=ptrElem->next) {
		 gString* myStr( (gString*)ptrElem->me );
		 pos = myStr->Find( strFirst )==1;
		 if ( pos==0 ) break;
	     }

	     if ( pos>=1 && tryOlderNamesFirst ) {
		 shown.CopyList( *inv );
	     }
	     else {
		 shown.CopyList( args );
	     }

	     for (iter=1, nIters=(int)shown.N(); iter>=1 && iter<=nIters; iter++) {
		 aStr = shown.Str( iter );
		 fIn = fopen( aStr, "r" );
		 if ( fIn ) {
		     if ( cmdNr==31 ) {
			 int warns;
			 int problem;

			 cat_file( fIn, fOut, zValue, problem, warns );
			 if ( warns ) {
			     if ( opt.isVeryVerbose ) {
				 LOG_ME(LOG_WARN, "cat: %s, warn(s): %d, first seen at line: %d", aStr, warns, problem);
			     }
			     else {
				 LOG_ME(LOG_WARN, "cat: %s, warn(s): %d", aStr, warns);
			     }
			 }
			 else {
			     LOG_ME(LOG_INFO, "cat: %s", aStr);
			 }
		     }
		     else {
			 if ( nIters > 1 ) {
			     LOG_ME(LOG_INFO, "dump: %s (%d of %u)", aStr, iter, nArgs);
			 }
			 else {
			     LOG_ME(LOG_INFO, "dump: %s", aStr);
			 }
			 show_datex( fIn, fOut, zValue );
		     }
		     fclose( fIn );
		 }
		 else {
		     LOG_ERR("no dump: %s (%d of %u)", aStr, iter, nArgs);
		 }
	     }

	     delete order;
	     delete inv;
	 }
	 else {
	     thisError = show_datex( stdin, fOut, zValue );
	 }
	 break;

     case 22:	// timex
	 iter = -9;
	 error = do_timex( args, fIn, fOut, fRepErr, opt );
	 if ( error ) {
	     fprintf(stderr, "Invalid request (%d)\n", error);
	 }
	 break;

     case 23:	// from-base
	 if ( nArgs<1 ) {
	     fprintf(stderr,"Missing base: 16, 8, 10, etc.\n");
	 }
	 else {
	     error = do_from_base( atoi( aStr ), aStr, args, fIn, fOut, zValue );
	     if ( error<0 ) {
		 fprintf(stderr,"Invalid base: %s\n",aStr);
	     }
	 }
	 break;

     case 24:	// from-date
	 break;

     case 28:  // nice-urlx
	 error = do_urlx( args, fIn, fOut, opt );
	 break;

     case 29:  // nice-xurl
	 error = do_xurl( args, fIn, fOut, opt );
	 break;

     case 30:  // unescape
	 error = show_unescape( args, fIn, fOut, opt, opt.zValue );
	 break;

     default:
	 ASSERTION_FALSE("am?");
     }
 }
 DBGPRINT("DBG: do_run returns %d\n",error);
 return error;
}

////////////////////////////////////////////////////////////
int go (char** argv, char** envp, int& result)
{
 int error;
 gArg arg( argv, envp );
 sOptSed opt;
 char* str;
 int cmdNr;
 unsigned n( 0 );
 FILE* fOut( stdout );
 FILE* fRepErr( stderr );

 arg.AddParams( 1, "-vv|--verbose -h|--help --version\
 -a|--all\
 -c:%s|--config:%s\
 -d:%d|--debug:%d\
 -l:%s|--log-file:%s\
 -o:%s|--output:%s\
 -z:%d|--value:%d\
" );

 // Parse command line
 error = arg.FlushParams();
 n = arg.N();
 arg.FindOptionOccurr( "verbose", opt.isVerbose, opt.isVeryVerbose );

 gString firstCmd( arg.Str(1) );
 char* cmdStr( firstCmd.Str() );
 arg.Delete( 1, 1 );

 cmdNr = command_from_str( cmdStr );
 const bool isTee( cmdNr==25 );

 if ( arg.FindOption("version") ) {
     print_version( arg.Program() );
     return 0;
 }
 if ( (arg.FindOption('h') || error!=0 || n==0) && firstCmd.Match( "norm" )==false ) {
     if ( n>0 && cmdStr[0]!='-' ) {
	 print_help_command( cmdStr );
     }
     else {
	 print_help( str = arg.Program() );
	 printf("For command help, use: %s command --help\n",str);
     }
     return 0;
 }

 opt.doAll = arg.FindOption( 'a' );

 if ( arg.FindOption( "debug", opt.sedLevel ) ) {
     if ( opt.sedLevel > 9 ) {
	 LOG_ME(LOG_WARN, "Maximum debug level is 9. Continuing anyway.");
     }
 }

 if ( start_log( fRepErr, arg, opt, opt.sedLevel ) ) {
     return 1;
 }
 arg.FindOption( "value", opt.zValue );

 if ( arg.FindOption( 'c', opt.sConfig ) ) {
     error = ssed_read_config_file( opt.sConfig.Str(), stderr, opt.myConfig );
     if ( error ) return 1;
 }
 else {
     error = ssed_read_config_file( nil, stderr, opt.myConfig );
     if ( error>0 ) return 1;
 }

 if ( arg.FindOption( 'o', opt.sOutput ) ) {
     const bool useTextMode( opt.zValue==1 );
     if ( isTee && opt.doAll ) {
	 fOut = fopen( opt.sOutput.Str(),  useTextMode ? "a" : "ab" );
     }
     else {
	 fOut = fopen( opt.sOutput.Str(), useTextMode ? "w" : "wb" );
     }
     if ( fOut==nil ) {
	 LOG_ERR("Cannot use output: %s", opt.sOutput.Str());
	 return 1;
     }
 }

 if ( cmdNr>0 ) {
     error = do_run( cmdNr, arg, fOut, fRepErr, opt );
 }
 else {
     error = 1;
     LOG_ERR("Invalid command: %s", cmdStr);
 }

 return error;
}
////////////////////////////////////////////////////////////
int main (int argc, char** argv, char** envp)
{
 int error, result( 0 );

 gINIT;
 IMEDIA_INIT;

 error = go( argv, envp, result );

 imb_iso_release();

 LOG_FLUSH();
 DBGPRINT("DBG: Objs undeleted: %d\n",gStorageControl::Self().NumObjs());
 gEND;

 DBGPRINT_MIN("go: %d, result=%d\n",error,result);
 return error;
}

