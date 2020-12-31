// debug.cpp, for libimedia

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib_imedia.h"


#ifndef DEBUG_OUT_PATH
#define DEBUG_OUT_PATH "/mnt/tmp"
#endif

#define DBG_PATH_SLASH DEBUG_OUT_PATH "/"

#define TMP_OUT_1 DBG_PATH_SLASH "isw.debug.txt"
#define TMP_OUT_2 DBG_PATH_SLASH "isw.debug.2.txt"
#define TMP_OUT_3 DBG_PATH_SLASH "isw.debug.3.txt"


#define STR_LATIN1_a_tilde "�"

#define SAMPLE_STR_FLUTUO       "Susana Félix - Flutuo"
#define CONVERT_UTF_STR_SAMPLE1 "Zé Maluco_" "\xE2\x80\x93" "_Anyone!"  // Not UCS2, 0x2013 is m-dash


// Globals
IMEDIA_DECLARE;		// sIsoUni* ptrUniData=nil;



struct ChrStat {
    t_unicode code;
    int occurred;
};

////////////////////////////////////////////////////////////
int ptm_prepare_custom_iso (int optTable, sIsoUni& data)
{
 int idx( 0 );
 char* myUcs8( data.customUcs8[ 3 ] );
 bool isInitialized( myUcs8[ 0 ]==0 );
 char hashed;
 t_uchar uChr;
 gUniCode* inUse( data.inUse );

 // ----> This function was copied almost 100% from boradb/src/processtext.cpp

 if ( isInitialized ) return -1;  // Nothing to do again

 ASSERTION(inUse,"inUse");

 for ( ; idx<256; idx++) {
     if ( idx<' ' ) {
	 data.hashUcs16Eq[ idx ] = (t_uchar*)calloc( 2, sizeof(t_uchar));
     }
     else {
	 data.hashUcs16Eq[ idx ] = (t_uchar*)calloc( 4, sizeof(t_uchar));
     }
 }

 for (idx=0; idx<' '; idx++) {
     myUcs8[ idx ] = 0;
 }

 for ( ; idx<127; idx++) {
     hashed = data.hashUcs8Custom[ idx ];
     myUcs8[ idx ] = hashed;
 }

 for ( ; idx<256; idx++) {
     hashed = data.hashUcs8Custom[ idx ];
     if ( hashed==-1 ) {
	 hashed = '~';
     }
     else {
	 uChr = (t_uchar)hashed;
	 hashed = inUse->hash256User[ gUniCode::e_Basic_Alpha26 ][ idx ];

	 if ( hashed==0 || hashed==-1 ) {
		 hashed = (char)idx;
	 }
	 else {
	     data.hashUcs16Eq[ idx ][ 0 ] = hashed;
	     data.hashUcs16Eq[ idx ][ 1 ] = 0;
	 }
     }
     myUcs8[ idx ] = hashed;
 }

 for (uChr='A'; uChr<='Z'; uChr++) {
     data.hashUcs16Eq[ uChr ][ 0 ] = uChr;
     ASSERTION(data.hashUcs16Eq[ uChr ][ 1 ]==0,"!iso (1)");
 }
 for (uChr='a'; uChr<='z'; uChr++) {
     data.hashUcs16Eq[ uChr ][ 0 ] = uChr;
     ASSERTION(data.hashUcs16Eq[ uChr ][ 1 ]==0,"!iso (2)");
 }
 for (uChr=47; uChr<='9'; uChr++) {
     idx = uChr==47 ? ' ' : (int)uChr;
     data.hashUcs16Eq[ idx ][ 0 ] = idx<'0' ? ' ' : idx;
     ASSERTION(data.hashUcs16Eq[ idx ][ 1 ]==0,"!iso (3)");
 }

 // German special chars, accepted:
 myUcs8[ 0xDF ] = 0xDF;		// 'LETTER SHARP S
 myUcs8[ 0xFF ] = 0;

 // also to the custom UCS8:
 data.hashUcs8Custom[ 0xDF ] = 0xDF;
 strcpy( (char*)data.hashUcs16Eq[ 0xDF ], "ss" );

 data.RefactorUcs16Eq();

 // Adjust blank and '_' into '.'
 strcpy( (char*)data.hashUcs16Strs[ ' ' ], "." );
 strcpy( (char*)data.hashUcs16Strs[ '_' ], "." );

 return 0;
}


int dbg_dump_cdb (FILE* fOut, int fdIn, int optUnused)
{
 sCdpCdb cdp;
 int error( acdb_read( fdIn, cdp ) );
 return error;
}

////////////////////////////////////////////////////////////
int dbg_test (const char* strIn)
{
 int error;
 int iLine( -1 ), prevLine( -1 );
 int thisMask, lastMask( -1 );
 gElem* ptrElem;
 gWebChassis chassis;
 gList listHTML, outHTML;
 FILE* fOut( stdout );
 FILE* fDbg( nil );
 FILE* fDbg3( nil );

 const char* strEnd( "\n" );
 const char* strFinit( strEnd );

 error = isw_simple_html_filter_file( strIn, outHTML, chassis );

 gString sFirstLine( outHTML.Str( 1 ) );
 if ( sFirstLine[ 1 ]=='#' && sFirstLine.Find( " Player " )>0 ) {
     fDbg = fopen( strIn, "r" );
     if ( fDbg ) {
	 error = dbg_dump_cdb( fOut, fileno( fDbg ), 0 );
	 fclose( fDbg );
	 fprintf(stderr,"cdb read: %d\n",error);
	 return -1;
     }
     else {
	 fprintf(stderr,"Uops: %s\n",strIn);
     }
 }

 printf("dbg_test: {%s}\n\
>>>>\n",strIn);

 for (ptrElem=outHTML.StartPtr();
      ptrElem;
      ptrElem=ptrElem->next) {

     #ifdef DEBUG_MIN
     imd_print_error("%d: ",ptrElem->iValue);
     #endif
     fprintf(fOut,"%d:%s\n",ptrElem->iValue,ptrElem->Str());
 }

 imd_print_error("\n\n");
 printf("<<<<\n\n");

 fOut = fopen( TMP_OUT_1, "wt" );
 if ( fOut==nil ) return 13;

#ifdef ISW_DEBUG_RAW
 fDbg = fopen( TMP_OUT_2, "wt" );
 fDbg3= fopen( TMP_OUT_3, "wt" );
#endif

 for (ptrElem=chassis.StartPtr();
      ptrElem;
      ptrElem=ptrElem->next) {

     const char* strBefore( "\0" );
     gString* aStr( (gString*)ptrElem->me );

     ASSERTION(aStr,"chassis?!");

     iLine = ptrElem->iValue;
     thisMask = aStr->iValue;

     if ( fDbg ) {
	 fprintf(fDbg,"%d, %d:\t%s\n",
		 iLine,
		 thisMask,
		 ptrElem->Str());
     }

     if ( iLine==prevLine ) {
	 strFinit = "";
	 if ( lastMask==ISW_MASK_QUOTED ) {
	     if ( aStr->Match( "/>" ) || (*aStr)[ 1 ]=='>' ) {
		 // do not add a blank after quote,
		 // e.g. '<a href="quote"' then '>' is added without blank,
		 // like this:  <a href="quote">
		 // instead of: <a href="quote" >
	     }
	     else {
		 strBefore = " ";
	     }
	 }

	 lastMask = aStr->iValue;
     }
     else {
	 strFinit = strEnd;
	 lastMask = -1;
     }
     fprintf(fOut,"%s%s%s",
	     strBefore,
	     strFinit,
	     ptrElem->Str());

     prevLine = iLine;
 }

 fprintf(fOut,strEnd);

 fclose( fOut );
 if ( fDbg ) fclose( fDbg );

 // Now the chassis built up!
 if ( fDbg3 ) {
     isw_html_keydump( chassis, chassis );

     for (ptrElem=chassis.built.StartPtr();
	  ptrElem;
	  ptrElem=ptrElem->next) {

	 if ( ptrElem->me->IsString() ) {
	     fprintf(fDbg3,"%s\n",ptrElem->Str());
	 }
	 else {
	     gElem* ptrFollow( ((gList*)ptrElem->me)->StartPtr() );
	     for ( ; ptrFollow; ptrFollow=ptrFollow->next) {
		 fprintf(fDbg3,"%s",ptrFollow->Str());
	     }
	 }
     }
 }

 FILE* fErLog( stderr );
 gString* errStr;

 for (ptrElem=chassis.htmlStatus.logIncompleteQuote.StartPtr();
      ptrElem;
      ptrElem=ptrElem->next) {
     errStr = (gString*)ptrElem->me;
     fprintf(fErLog, "Line: %d, mask: %d:\n\t%s\n\n\n",
	     ptrElem->iValue,
	     errStr->iValue,
	     errStr->Str());
 }

 return error;
}


int dbg_samples (const char* strOneMore)
{
 const char* strSamples[]={
	"The Verve",
	"Smiths, The",
	"Pink,_Floyd",
	"Rodrigo Le" STR_LATIN1_a_tilde "o",
	nil,
	nil,
	nil
 };

 const char* str;
 int idx( 0 );

 for ( ; (str = strSamples[ idx ])!=nil; idx++) ;
 if ( strOneMore ) {
     strSamples[ idx ] = strOneMore;
 }

 printf("LEGEND:\n\
	+r	Original string\n\
	+h	ptm_nprime(); comma separated upstring\n\
	+x	same as +r\n\
	+y	dotted with original upstring\n\
\n");

 for (idx=0; (str = strSamples[ idx ])!=nil; idx++) {
     gString sName( (char*)str );
#ifndef VALGRIND_SIMPLE
     const char* newStr( nil );
     gList* newList;
     gStorage* me;
     newList = ptm_new_name( sName, nil, 0, nil );
     if ( newList ) {
	 me = newList->StartPtr()->next->me;
	 newStr = me->Str();
	 printf("\
+r		  %s\n\
+h	%08u  %s\n",
		str,
		ptm_nprime( me->iValue ),
		newStr);
	 delete newList;
     }
     else {
	 printf("\
+r			%s\n\
+h	%08u  <NADA>\n",
		str,
		-1);
     }

     printf("\
+x		  %s\n\
+y		  %s\n\n",
	    str,
	    ptm_name_str( str, 1, nil ));
#endif //~VALGRIND_SIMPLE
 }
 return 0;
}


int dbg_utf8 (const char* strUniFile)
{
 const char* str( nil );
 const char* strSTDIN( nil );
 const ChrStat itStats[]={
	{ 0x0153, 1 },
	{ 0x042F, 1 },
	{ 0x2013, 39 },
	{ 0x2014, 2 },
	{ 0x2019, 2 },
	{ 0x201C, 1 },
	{ 0x201D, 2 },
	{ 0x20AC, 1 },
	{ 0x2605, 2 },
	{ 0x3000, 1 },
	{ 0x30B9, 1 },
	{ 0x33CA, 1 },
	{ 0, -1 }
 };
 int idx;
 gFileFetch* fUNI( nil );
 t_unicode code;
 char strCode[ 16 ];
 gList* ptrUniDesc( nil );

 if ( strUniFile ) {
     fUNI = new gFileFetch( strUniFile );
     ptrUniDesc = &fUNI->aL;
 }

 printf("\n+++\n");

 for (idx=0; ; idx++) {
     int occurred( itStats[ idx ].occurred );
     if ( occurred<=-1 ) break;
     code = itStats[ idx ].code;
     snprintf(strCode, 16-1, "%04X;", code);

     unsigned pos( 0 );
     if ( ptrUniDesc ) {
	 pos = ptrUniDesc->FindFirst( strCode, 1, e_FindExactPosition );
     }
     if ( pos ) {
	 str = ptrUniDesc->CurrentPtr()->me->Str();
	 str += 4;
     }
     else {
	 str = "(no UNI text arg.)";
     }
     printf("ChrStat 0x%04X%5d %s\n",
	    code,
	    occurred,
	    str);
 }
 delete fUNI;
 printf("+++\n\n");

 gFileFetch input( strSTDIN );
 unsigned line( 0 );
 unsigned nPlainText126dLines( 0 );
 unsigned maxLine126d( 0 ), maxLine126dChrs( 0 );
 gElem* pStart( input.aL.StartPtr() );
 gElem* pIter( pStart );
 gString* pStr;

 for ( ; pIter; pIter=pIter->next) {
     line++;
     pStr = (gString*)pIter->me;
     unsigned greater126d( 0 );
     for (idx=1; idx<=(int)(pStr->Length()); idx++) {
	 if ( (*pStr)[ idx ]>126 ) {
	     greater126d++;
	 }
     }
     nPlainText126dLines += (greater126d==0);
     if ( greater126d > maxLine126dChrs ) {
	 maxLine126dChrs = greater126d;
	 maxLine126d = line;
     }
 }

 printf("Statistics:\n");
 printf("\
line %u, N# lines: %u\n\
nPlainText126dLines: %u\n\
Non-full text lines: %u\n\
maxLine126d: %u, #chrs there >126d: %u\n\
",
	line, input.aL.N(),
	nPlainText126dLines,
	line - nPlainText126dLines,
	maxLine126d, maxLine126dChrs);
 printf("+++\n\n");
 return 0;
}


int dbg_cue (const char* strFile)
{
 int error;
 gFileFetch cueInput( (char*)strFile );
 iCue cue;
 FILE* fOut( stdout );

 error = cue.Parse( cueInput.aL );

 fprintf(fOut, "DiscID: {%s}\nPerformer: {%s}\nTitle: {%s}\nFile: {%s}\nComment: {%s}",
	 cue.sDiscID.Str(),
	 cue.sPerformer.Str(),
	 cue.sTitle.Str(),
	 cue.sFile.Str(),
	 cue.sComment.Str());

 return error;
}


int do_debug (int argc, char* argv[])
{
 int error;
 DBGPRINT("DBG: do_debug('%s' ...)\n", argv[ 1 ]);

 if ( str_compare( argv[ 1 ], "cue" )==0 ) {
     error = dbg_cue( argv[ 2 ] );
     return error;
 }
 else {
     if ( str_compare( argv[ 1 ], "utf8" )==0 ) {
	 error = dbg_utf8( argv[ 2 ] );
	 fprintf(stderr,"UTF8, error: %d\n",error);
	 return error;
     }
     else {
	 error = dbg_test( argv[ 1 ] );
     }
 }

 if ( error<0 ) return 0;
 fprintf(stderr,"Error: %d\n",error);

 error = ptm_prepare_custom_iso( 0, *ptrUniData );
 printf("ptm_prepare_custom_iso returned: %d\n",error);

 dbg_samples( argv[ 1 ] );

 gString* ptrStr( imb_auth_mime64( (t_uchar*)"henry", (t_uchar*)"pass" ) );

 if ( ptrStr ) {
     printf("Base64 for henry,pass: %s\n",ptrStr->Str());
     delete ptrStr;
 }

#ifndef NO_TEST_UTF8
 eRecodeError returnError( ereNO_ERROR );
 t_uchar* newStr( imb_utf8_str_to_ucs2( (t_uchar*)SAMPLE_STR_FLUTUO, returnError ) );
 sUtfBox box;

 printf("\n+++\nISO8859-1 got returnError=%d,\n\
	{%s}\n\n",
	returnError,
	newStr);
 delete[] newStr;

 DBGPRINT("Sample (has ASCII >126d chars: {" CONVERT_UTF_STR_SAMPLE1 "}\n");
 returnError = ereNO_ERROR;
 newStr = imb_utf8_str_to_ucs2_limit( (t_uchar*)CONVERT_UTF_STR_SAMPLE1, 0xFFFF, box );
 returnError = box.ucs2Error;

 printf("ISO8859-1 got returnError=%d,\n", returnError);
 printf("\tUniCode character UCS2 that gave error: 0x%04X%s\n",
	box.lastUCS2,
	box.lastUCS2==0x2013 ? " (Ok, m-dash)" : "?ERROR?");
 printf("\t");
 for (int strIdx=0; ; strIdx++) {
     t_uchar uChr( newStr[ strIdx ] );
     if ( uChr==0 ) {
	 printf("\n...\n");
	 break;
     }
     if ( uChr<' ' || uChr>126 ) {
	 printf("[hex:%02X]",uChr);
     }
     else {
	 printf("%c",uChr);
     }
 }
 delete[] newStr;

 returnError = ereNO_ERROR;
 newStr = imb_utf8_str_to_ucs2_limit( (t_uchar*)CONVERT_UTF_STR_SAMPLE1, 0xFF, box );
 returnError = box.ucs2Error;
 printf("ISO8859-1 got returnError=%d,\n\t%s\n", returnError, newStr);
 printf("\t--> %s\n", strcmp((char*)newStr, "Z" "\xE9" " Maluco_")==0 ? "OK" : "NOT_OK!");
 delete[] newStr;
#endif //~NO_TEST_UTF8

 printf("+++\n\n");
 return error!=0;
}


#if 0
int testa ()
{
 const char* strFilename( nil );
 gFileFetch input( strFilename );
 fprintf(stderr, "Lines: %u\n", input.aL.N());
 input.aL.Show();
 return 0;
}
#endif


int main (int argc, char* argv[])
{
 int error;

 gINIT;

 IMEDIA_INIT;  // imb_iso_init( nil, new sIsoUni )

 error = do_debug( argc, argv );

 FILE* fOutput( gFileControl::Self().OutputStream() );
 FILE* fReport( gFileControl::Self().ReportStream() );
 gStorageControl::Self().Show();
 printf("fOutput: %s\n",
	fOutput ? (fOutput==stdout ? "stdout" : "<other>") : "nil");
 printf("fReport: %s\n",
	fReport ? (fReport==stderr ? "stderr" : (fReport==stdout ? "stdout" : "<other>")) : "nil");

 imb_iso_release();

 ISW_SHOW_MEM("DBG: Objs undeleted: %d\n",gStorageControl::Self().NumObjs());
 gEND;

 return error;
}
////////////////////////////////////////////////////////////

