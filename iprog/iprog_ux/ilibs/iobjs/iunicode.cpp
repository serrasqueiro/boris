// iunicode.cpp

#include <string.h>

#include "iunicode.h"
#include "iarg.h"
////////////////////////////////////////////////////////////
// Static members
// ---------------------------------------------------------
const t_unitable gUniCode::defaultUniTable=885901;

const char* gUniCode::defaultUniLettersISO8859_1="\
0041;LETTER A;Lu;0;L;;;;;N;;;;0061;\n\
0042;LETTER B;Lu;0;L;;;;;N;;;;0062;\n\
0043;LETTER C;Lu;0;L;;;;;N;;;;0063;\n\
0044;LETTER D;Lu;0;L;;;;;N;;;;0064;\n\
0045;LETTER E;Lu;0;L;;;;;N;;;;0065;\n\
0046;LETTER F;Lu;0;L;;;;;N;;;;0066;\n\
0047;LETTER G;Lu;0;L;;;;;N;;;;0067;\n\
0048;LETTER H;Lu;0;L;;;;;N;;;;0068;\n\
0049;LETTER I;Lu;0;L;;;;;N;;;;0069;\n\
004A;LETTER J;Lu;0;L;;;;;N;;;;006A;\n\
004B;LETTER K;Lu;0;L;;;;;N;;;;006B;\n\
004C;LETTER L;Lu;0;L;;;;;N;;;;006C;\n\
004D;LETTER M;Lu;0;L;;;;;N;;;;006D;\n\
004E;LETTER N;Lu;0;L;;;;;N;;;;006E;\n\
004F;LETTER O;Lu;0;L;;;;;N;;;;006F;\n\
0050;LETTER P;Lu;0;L;;;;;N;;;;0070;\n\
0051;LETTER Q;Lu;0;L;;;;;N;;;;0071;\n\
0052;LETTER R;Lu;0;L;;;;;N;;;;0072;\n\
0053;LETTER S;Lu;0;L;;;;;N;;;;0073;\n\
0054;LETTER T;Lu;0;L;;;;;N;;;;0074;\n\
0055;LETTER U;Lu;0;L;;;;;N;;;;0075;\n\
0056;LETTER V;Lu;0;L;;;;;N;;;;0076;\n\
0057;LETTER W;Lu;0;L;;;;;N;;;;0077;\n\
0058;LETTER X;Lu;0;L;;;;;N;;;;0078;\n\
0059;LETTER Y;Lu;0;L;;;;;N;;;;0079;\n\
005A;LETTER Z;Lu;0;L;;;;;N;;;;007A;\n\
0061;LETTER A;Ll;0;L;;;;;N;;;0041;;0041\n\
0062;LETTER B;Ll;0;L;;;;;N;;;0042;;0042\n\
0063;LETTER C;Ll;0;L;;;;;N;;;0043;;0043\n\
0064;LETTER D;Ll;0;L;;;;;N;;;0044;;0044\n\
0065;LETTER E;Ll;0;L;;;;;N;;;0045;;0045\n\
0066;LETTER F;Ll;0;L;;;;;N;;;0046;;0046\n\
0067;LETTER G;Ll;0;L;;;;;N;;;0047;;0047\n\
0068;LETTER H;Ll;0;L;;;;;N;;;0048;;0048\n\
0069;LETTER I;Ll;0;L;;;;;N;;;0049;;0049\n\
006A;LETTER J;Ll;0;L;;;;;N;;;004A;;004A\n\
006B;LETTER K;Ll;0;L;;;;;N;;;004B;;004B\n\
006C;LETTER L;Ll;0;L;;;;;N;;;004C;;004C\n\
006D;LETTER M;Ll;0;L;;;;;N;;;004D;;004D\n\
006E;LETTER N;Ll;0;L;;;;;N;;;004E;;004E\n\
006F;LETTER O;Ll;0;L;;;;;N;;;004F;;004F\n\
0070;LETTER P;Ll;0;L;;;;;N;;;0050;;0050\n\
0071;LETTER Q;Ll;0;L;;;;;N;;;0051;;0051\n\
0072;LETTER R;Ll;0;L;;;;;N;;;0052;;0052\n\
0073;LETTER S;Ll;0;L;;;;;N;;;0053;;0053\n\
0074;LETTER T;Ll;0;L;;;;;N;;;0054;;0054\n\
0075;LETTER U;Ll;0;L;;;;;N;;;0055;;0055\n\
0076;LETTER V;Ll;0;L;;;;;N;;;0056;;0056\n\
0077;LETTER W;Ll;0;L;;;;;N;;;0057;;0057\n\
0078;LETTER X;Ll;0;L;;;;;N;;;0058;;0058\n\
0079;LETTER Y;Ll;0;L;;;;;N;;;0059;;0059\n\
007A;LETTER Z;Ll;0;L;;;;;N;;;005A;;005A\n\
00AA;FEMININE ORDINAL INDICATOR;Ll;0;L;<super> 0061;;;;N;;;;;\n\
00B5;MICRO SIGN;Ll;0;L;<compat> 03BC;;;;N;;;039C;;039C\n\
00BA;MASCULINE ORDINAL INDICATOR;Ll;0;L;<super> 006F;;;;N;;;;;\n\
00C0;LETTER A WITH GRAVE;Lu;0;L;0041 0300;;;;N;LETTER A GRAVE;;;00E0;\n\
00C1;LETTER A WITH ACUTE;Lu;0;L;0041 0301;;;;N;LETTER A ACUTE;;;00E1;\n\
00C2;LETTER A WITH CIRCUMFLEX;Lu;0;L;0041 0302;;;;N;LETTER A CIRCUMFLEX;;;00E2;\n\
00C3;LETTER A WITH TILDE;Lu;0;L;0041 0303;;;;N;LETTER A TILDE;;;00E3;\n\
00C4;LETTER A WITH DIAERESIS;Lu;0;L;0041 0308;;;;N;LETTER A DIAERESIS;;;00E4;\n\
00C5;LETTER A WITH RING ABOVE;Lu;0;L;0041 030A;;;;N;LETTER A RING;;;00E5;\n\
00C6;LETTER AE;Lu;0;L;;;;;N;LETTER A E;ash *;;00E6;\n\
00C7;LETTER C WITH CEDILLA;Lu;0;L;0043 0327;;;;N;LETTER C CEDILLA;;;00E7;\n\
00C8;LETTER E WITH GRAVE;Lu;0;L;0045 0300;;;;N;LETTER E GRAVE;;;00E8;\n\
00C9;LETTER E WITH ACUTE;Lu;0;L;0045 0301;;;;N;LETTER E ACUTE;;;00E9;\n\
00CA;LETTER E WITH CIRCUMFLEX;Lu;0;L;0045 0302;;;;N;LETTER E CIRCUMFLEX;;;00EA;\n\
00CB;LETTER E WITH DIAERESIS;Lu;0;L;0045 0308;;;;N;LETTER E DIAERESIS;;;00EB;\n\
00CC;LETTER I WITH GRAVE;Lu;0;L;0049 0300;;;;N;LETTER I GRAVE;;;00EC;\n\
00CD;LETTER I WITH ACUTE;Lu;0;L;0049 0301;;;;N;LETTER I ACUTE;;;00ED;\n\
00CE;LETTER I WITH CIRCUMFLEX;Lu;0;L;0049 0302;;;;N;LETTER I CIRCUMFLEX;;;00EE;\n\
00CF;LETTER I WITH DIAERESIS;Lu;0;L;0049 0308;;;;N;LETTER I DIAERESIS;;;00EF;\n\
00D0;LETTER ETH;Lu;0;L;;;;;N;;Icelandic;;00F0;\n\
00D1;LETTER N WITH TILDE;Lu;0;L;004E 0303;;;;N;LETTER N TILDE;;;00F1;\n\
00D2;LETTER O WITH GRAVE;Lu;0;L;004F 0300;;;;N;LETTER O GRAVE;;;00F2;\n\
00D3;LETTER O WITH ACUTE;Lu;0;L;004F 0301;;;;N;LETTER O ACUTE;;;00F3;\n\
00D4;LETTER O WITH CIRCUMFLEX;Lu;0;L;004F 0302;;;;N;LETTER O CIRCUMFLEX;;;00F4;\n\
00D5;LETTER O WITH TILDE;Lu;0;L;004F 0303;;;;N;LETTER O TILDE;;;00F5;\n\
00D6;LETTER O WITH DIAERESIS;Lu;0;L;004F 0308;;;;N;LETTER O DIAERESIS;;;00F6;\n\
00D8;LETTER O WITH STROKE;Lu;0;L;;;;;N;LETTER O SLASH;;;00F8;\n\
00D9;LETTER U WITH GRAVE;Lu;0;L;0055 0300;;;;N;LETTER U GRAVE;;;00F9;\n\
00DA;LETTER U WITH ACUTE;Lu;0;L;0055 0301;;;;N;LETTER U ACUTE;;;00FA;\n\
00DB;LETTER U WITH CIRCUMFLEX;Lu;0;L;0055 0302;;;;N;LETTER U CIRCUMFLEX;;;00FB;\n\
00DC;LETTER U WITH DIAERESIS;Lu;0;L;0055 0308;;;;N;LETTER U DIAERESIS;;;00FC;\n\
00DD;LETTER Y WITH ACUTE;Lu;0;L;0059 0301;;;;N;LETTER Y ACUTE;;;00FD;\n\
00DE;LETTER THORN;Lu;0;L;;;;;N;;Icelandic;;00FE;\n\
00DF;LETTER SHARP S;Ll;0;L;;;;;N;;German;;;\n\
00E0;LETTER A WITH GRAVE;Ll;0;L;0061 0300;;;;N;LETTER A GRAVE;;00C0;;00C0\n\
00E1;LETTER A WITH ACUTE;Ll;0;L;0061 0301;;;;N;LETTER A ACUTE;;00C1;;00C1\n\
00E2;LETTER A WITH CIRCUMFLEX;Ll;0;L;0061 0302;;;;N;LETTER A CIRCUMFLEX;;00C2;;00C2\n\
00E3;LETTER A WITH TILDE;Ll;0;L;0061 0303;;;;N;LETTER A TILDE;;00C3;;00C3\n\
00E4;LETTER A WITH DIAERESIS;Ll;0;L;0061 0308;;;;N;LETTER A DIAERESIS;;00C4;;00C4\n\
00E5;LETTER A WITH RING ABOVE;Ll;0;L;0061 030A;;;;N;LETTER A RING;;00C5;;00C5\n\
00E6;LETTER AE;Ll;0;L;;;;;N;LETTER A E;ash *;00C6;;00C6\n\
00E7;LETTER C WITH CEDILLA;Ll;0;L;0063 0327;;;;N;LETTER C CEDILLA;;00C7;;00C7\n\
00E8;LETTER E WITH GRAVE;Ll;0;L;0065 0300;;;;N;LETTER E GRAVE;;00C8;;00C8\n\
00E9;LETTER E WITH ACUTE;Ll;0;L;0065 0301;;;;N;LETTER E ACUTE;;00C9;;00C9\n\
00EA;LETTER E WITH CIRCUMFLEX;Ll;0;L;0065 0302;;;;N;LETTER E CIRCUMFLEX;;00CA;;00CA\n\
00EB;LETTER E WITH DIAERESIS;Ll;0;L;0065 0308;;;;N;LETTER E DIAERESIS;;00CB;;00CB\n\
00EC;LETTER I WITH GRAVE;Ll;0;L;0069 0300;;;;N;LETTER I GRAVE;;00CC;;00CC\n\
00ED;LETTER I WITH ACUTE;Ll;0;L;0069 0301;;;;N;LETTER I ACUTE;;00CD;;00CD\n\
00EE;LETTER I WITH CIRCUMFLEX;Ll;0;L;0069 0302;;;;N;LETTER I CIRCUMFLEX;;00CE;;00CE\n\
00EF;LETTER I WITH DIAERESIS;Ll;0;L;0069 0308;;;;N;LETTER I DIAERESIS;;00CF;;00CF\n\
00F0;LETTER ETH;Ll;0;L;;;;;N;;Icelandic;00D0;;00D0\n\
00F1;LETTER N WITH TILDE;Ll;0;L;006E 0303;;;;N;LETTER N TILDE;;00D1;;00D1\n\
00F2;LETTER O WITH GRAVE;Ll;0;L;006F 0300;;;;N;LETTER O GRAVE;;00D2;;00D2\n\
00F3;LETTER O WITH ACUTE;Ll;0;L;006F 0301;;;;N;LETTER O ACUTE;;00D3;;00D3\n\
00F4;LETTER O WITH CIRCUMFLEX;Ll;0;L;006F 0302;;;;N;LETTER O CIRCUMFLEX;;00D4;;00D4\n\
00F5;LETTER O WITH TILDE;Ll;0;L;006F 0303;;;;N;LETTER O TILDE;;00D5;;00D5\n\
00F6;LETTER O WITH DIAERESIS;Ll;0;L;006F 0308;;;;N;LETTER O DIAERESIS;;00D6;;00D6\n\
00F8;LETTER O WITH STROKE;Ll;0;L;;;;;N;LETTER O SLASH;;00D8;;00D8\n\
00F9;LETTER U WITH GRAVE;Ll;0;L;0075 0300;;;;N;LETTER U GRAVE;;00D9;;00D9\n\
00FA;LETTER U WITH ACUTE;Ll;0;L;0075 0301;;;;N;LETTER U ACUTE;;00DA;;00DA\n\
00FB;LETTER U WITH CIRCUMFLEX;Ll;0;L;0075 0302;;;;N;LETTER U CIRCUMFLEX;;00DB;;00DB\n\
00FC;LETTER U WITH DIAERESIS;Ll;0;L;0075 0308;;;;N;LETTER U DIAERESIS;;00DC;;00DC\n\
00FD;LETTER Y WITH ACUTE;Ll;0;L;0079 0301;;;;N;LETTER Y ACUTE;;00DD;;00DD\n\
00FE;LETTER THORN;Ll;0;L;;;;;N;;Icelandic;00DE;;00DE\n\
00FF;LETTER Y WITH DIAERESIS;Ll;0;L;0079 0308;;;;N;LETTER Y DIAERESIS;;0178;;0178\n\
";
////////////////////////////////////////////////////////////
gUniCode::gUniCode (const char* strTableName, t_unitable iTable)
    : tableName( strTableName ),
      uniTable( iTable ? iTable : defaultUniTable ),
      defaultAllocationMask( 3 ),
      ptrUniElems( nil )
{
 memset( hash256Basic, -1, sizeof(hash256Basic) );
 memset( hash256User, 0x0, sizeof(hash256User) );

 if ( uniTable==defaultUniTable ) {
     thisListFromStr( (char*)defaultUniLettersISO8859_1, uniListed );
 }
}


gUniCode::~gUniCode ()
{
 delete[] ptrUniElems;
}


void gUniCode::Reset ()
{
 gControl::Reset();
 uniTable = 0;
 uniListed.Delete();
 delete[] ptrUniElems; ptrUniElems = nil;
}


int gUniCode::Build ()
{
 t_unicode code( 0 ), codeMin( MAX_UINT16_U ), codeMax( 0 );
 t_unicode inverseCode;
 unsigned normalCaseLetter;
 unsigned iter( 1 ), nUniEntries( uniListed.N() );
 t_uchar uChr;
 char inverseCase;
 gList* newInfo( nil );

 delete[] ptrUniElems;
 ptrUniElems = new sUniElement[ nUniEntries+1 ];
 if ( ptrUniElems==nil ) return -1;

 for ( ; iter<=nUniEntries; iter++) {
     if ( thisUniElementFromStr( uniListed.Str( iter ), defaultAllocationMask, ptrUniElems[ iter ] )==0 ) {
	 code = ptrUniElems[ iter ].code;
	 if ( code < codeMin ) codeMin = code;
	 if ( code > codeMax ) codeMax = code;
     }
 }

 DBGPRINT("DBG: Creating hashes for UCS16 between 0x%04X and 0x%04X (code: %u)\n",
	  codeMin,
	  codeMax,
	  code);
 if ( code==0 ) return -1;
 codeMax++;

 // Create different basic hashes
 for (uChr='A'; uChr<='Z'; uChr++) {
     hash256Basic[ e_Basic_Alpha26 ][ uChr ] = (char)(uChr+32);
     hash256Basic[ e_Basic_Alpha26 ][ uChr+32 ] = (char)uChr;
     hash256Basic[ e_Basic_Alpha ][ uChr+32 ] = 'l';  // Lower case letter
     hash256Basic[ e_Basic_DigiAlpha ][ uChr+32 ] = (char)uChr;
 }

 for (uChr='0'; uChr<='9'; uChr++) {
     hash256Basic[ e_Basic_DigiAlpha ][ uChr ] = uChr;
 }

 for (iter=1; iter<=nUniEntries; iter++) {
     sUniElement* ptrChr( &ptrUniElems[ iter ] );
     code = ptrChr->code;
     if ( code>=256 ) continue;

     if ( ptrChr->twoLetter[ 0 ]=='L' ) {
	 inverseCase = 'a';

	 if ( ptrChr->strCompat ) {
	     normalCaseLetter = 0;
	     sscanf( ptrChr->strCompat, "%X", &normalCaseLetter );
	     if ( normalCaseLetter>='A' && normalCaseLetter<256 ) hash256User[ e_Basic_Alpha26 ][ code ] = (char)normalCaseLetter;
	 }

	 if ( (code>='A' && code<='Z') ||
	      (ptrChr->strCompat && ptrChr->strCompat[ 0 ]=='0') ) {
	     // Has also a valid compatibility
	     hash256Basic[ e_Basic_Alpha ][ code ] = ptrChr->twoLetter[ 1 ];

	     if ( ptrChr->upperEq )
		 inverseCode = ptrChr->upperEq;
	     else
		 inverseCode = ptrChr->lowerEq;
	     if ( inverseCode>='0' && inverseCode<256 ) {
		 inverseCase = (char)inverseCode;
	     }
	     else {
		 inverseCase = '!';
	     }
	     hash256Basic[ e_Basic_DigiAlpha ][ code ] = inverseCase;
	 }
	 else {
	     DBGPRINT("DBG: basic hash, skipping weird letter: UCS8 %03ud %c, strCompat=%s\n",
		      code,
		      (code<' ') ? '?' : ((code>=127 && code<=0xA0) ? '!' : code),
		      ptrChr->strCompat);
	     newInfo = new gList;
	     ASSERTION(newInfo,"newInfo");
	     newInfo->Add( (int)code );
	     newInfo->Add( ptrChr->twoLetter );
	     newInfo->Add( ptrChr->strCompat );
	     hash256BasicSkipInfo[ e_Basic_Alpha ].AppendObject( newInfo );
	 }
     }
 }

 memcpy( hash256Basic[ e_Basic_Printable ],
	 hash256Basic[ e_Basic_DigiAlpha ],
	 256 );

 hash256Basic[ (int)e_Basic_Printable ][ (int)' ' ] = ' ';

 for (uChr=33; uChr<127; uChr++) {
     if ( hash256Basic[ e_Basic_Printable ][ uChr ]==-1 ) {
	 hash256Basic[ e_Basic_Printable ][ uChr ] =
	     (uChr=='_' ? ' ' : uChr);
     }
 }

#if 0
 for (iter=1; iter<=nUniEntries; iter++) {
     code = ptrUniElems[ iter ].code;
     if ( code>=256 ) continue;
     printf("UCS8 %c %03ud \
type=%c (=0) \
twoLetter=%c%c \
strCompat={%s} \
strNormal={%s} \
upperEq %c \
lowerEq %c \
upperAlt %c \
hash:%d\
\n\n",
	    code, iter,
	    ptrUniElems[ iter ].type,
	    ptrUniElems[ iter ].twoLetter[ 0 ],
	    ptrUniElems[ iter ].twoLetter[ 1 ],
	    ptrUniElems[ iter ].strCompat,
	    ptrUniElems[ iter ].strNormal,
	    ptrUniElems[ iter ].upperEq,
	    ptrUniElems[ iter ].lowerEq,
	    ptrUniElems[ iter ].upperAlt,
	    hash256Basic[ e_Basic_Printable ][ code ]);
 }
#endif  // debug only, note non standard 7bit ASCII will be shown!

#if 0
 for (iter=(unsigned)' '; iter<256; iter++) {
     if ( iter>=127 && iter<0xC0 ) continue;  // actually only chrs 127d to 0xA0 are unreadable, except 128d (Euro sign)
     printf("UCS8 %ud\t0x%02X %c %c\n",
	    iter, iter,
	    iter,
	    hash256Basic[ e_Basic_Printable ][ iter ]);
 }
#endif  // debug only, note non standard 7bit ASCII will be shown!

 return 0;
}


int gUniCode::BuildFromList (gList& uniTableStrings, t_unitable iTable)
{
 uniListed.CopyList( uniTableStrings );
 return Build();
}


int gUniCode::thisListFromStr (const char* strLines, gList& uniList)
{
 int error;
 int result( 0 );
 gString s;
 char chr;
 sUniElement uniElem;

 for ( ; (chr = *strLines)!=0; strLines++) {
     if ( chr=='\r' ) continue;
     if ( chr=='\n' ) {
	 s.Trim();
	 if ( s.IsEmpty()==false && s[1]!='#' ) {
	     uniList.Add( s );
	     error = thisUniElementFromStr( s.Str(), 0, uniElem );
	     DBGPRINT_MIN("DBG: thisUniElementFromStr(%s,uniElem): 0x%04X returned %d\n",
			  s.Str(),
			  uniElem.code,
			  error);
	     DBGPRINT_MIN("DBG: UCS16=0x%04X %c upperEq:%02X, lowerEq:%02X, upperAlt=%02X\n",
			  uniElem.code,
			  (uniElem.code > ' ' && uniElem.code < 127) || uniElem.code > 0xA0 ? uniElem.code : '.',
			  uniElem.upperEq,
			  uniElem.lowerEq,
			  uniElem.upperAlt);
	     if ( error ) {
		 result++;
	     }
	 }
	 s.SetEmpty();
     }
     else {
	 s.Add( chr );
     }
 }
 return result;
}


int gUniCode::thisUniElementFromStr (const char* aStr, int allocateMask, sUniElement& uniElem)
{
 if ( aStr==nil ) return -1;

 gParam params( aStr, ";" );
 unsigned n( params.N() );

 // Minimum: 14 fields; usually 15 fields.
 if ( n<14 ) return -1;

 uniElem.code = ConvertHexToUInt32( params.Str( 1 ) );

 if ( allocateMask ) {
     uniElem.strDescription = DupChars( params.Str( 2 ) );
     uniElem.strCompat = DupChars( params.Str( 6 ) );
     uniElem.a1 = DupChars( params.Str( 7 ) );
     uniElem.a2 = DupChars( params.Str( 8 ) );
     uniElem.a3 = DupChars( params.Str( 9 ) );
     if ( allocateMask & 2 )
	 uniElem.strNormal = DupChars( params.Str( 10 ) );  // For letters, etc., 'N'
     if ( allocateMask & 4 )
	 uniElem.strAltDescription = DupChars( params.Str( 11 ) );
     uniElem.strNote = DupChars( params.Str( 12 ) );
 }

 strncpy( uniElem.twoLetter, params.Str( 3 ), 2 );
 uniElem.type = params.Str( 4 )[ 0 ];
 uniElem.upperEq = ConvertHexToUInt32( params.Str( 13 ) );
 uniElem.lowerEq = ConvertHexToUInt32( params.Str( 14 ) );
 uniElem.upperAlt = ConvertHexToUInt32( params.Str( 15 ) );

 return 0;
}
////////////////////////////////////////////////////////////

