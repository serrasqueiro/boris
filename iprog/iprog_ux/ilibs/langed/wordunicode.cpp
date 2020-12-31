// wordunicode.cpp

#include "lib_langed.h"

#include "wordunicode.h"


// Forward declarations
t_uchar* wuc_new_7bit_string (const t_uchar* strIn, int length, t_uint32 uMask, const t_uchar** optUcsEqStrs, sLanged& langed) ;

t_uchar* wuc_new_8bit_string (const t_uchar* strIn, int length, t_uint32 uMask, const t_uchar** optUcsEqStrs, sLanged& langed) ;


////////////////////////////////////////////////////////////
// Aux functions for unicode
////////////////////////////////////////////////////////////
int ulang_prepare_custom_iso (int optTable, sIsoUni& data)
{
 int idx( 0 );
 char* myUcs8( data.customUcs8[ 2 ] );
 bool isInitialized( myUcs8[ 0 ]==0 );
 char hashed;
 t_uchar uChr;
 gUniCode* inUse( data.inUse );

 // ----> see also similar function at boradb/src/processtext.cpp;
 //		unlike boradb, it uses customUcs8[ 2 ] !

 if ( isInitialized ) {
     imd_print_error("ulang_prepare_custom_iso() twice?\n");
     return -1;  // Nothing to do again
 }

 ASSERTION(inUse,"inUse");

 for ( ; idx<256; idx++) {
     if ( idx<' ' ) {
	 data.hashUcs16Eq[ idx ] = new t_uchar[ 2 ];
	 data.hashUcs16Eq[ idx ][ 0 ] = data.hashUcs16Eq[ idx ][ 0 ] = 0;
     }
     else {
	 data.hashUcs16Eq[ idx ] = new t_uchar[ 6 ];
	 memset( data.hashUcs16Eq[ idx ], 0x0, 6 );
     }
 }

 for (idx=0; idx<' '; idx++) {
     myUcs8[ idx ] = 0;
 }
 myUcs8[ '\t' ] = ' ';
 myUcs8[ '\n' ] = '\n';

 for ( ; idx<127; idx++) {
     hashed = data.hashUcs8Custom[ idx ];
     myUcs8[ idx ] = hashed;
 }

 for ( ; idx<256; idx++) {
     hashed = inUse->hash256User[ gUniCode::e_Basic_Alpha26 ][ idx ];
     if ( (hashed>='a' && hashed<='z') ||
	  (hashed>='A' && hashed<='Z') ) {
	 data.hashUcs16Eq[ idx ][ 0 ] = hashed;
	 data.hashUcs16Eq[ idx ][ 1 ] = 0;
     }
     else {
	 hashed = 0;
     }
     myUcs8[ idx ] = hashed;
 }

 for (uChr='A'; uChr<='Z'; uChr++) {
     data.hashUcs16Eq[ uChr ][ 0 ] = uChr;
     ASSERTION(data.hashUcs16Eq[ uChr ][ 1 ]==0,"!iso (1)");
     myUcs8[ uChr ] = uChr;
 }
 for (uChr='a'; uChr<='z'; uChr++) {
     data.hashUcs16Eq[ uChr ][ 0 ] = uChr;
     ASSERTION(data.hashUcs16Eq[ uChr ][ 1 ]==0,"!iso (2)");
     myUcs8[ uChr ] = uChr;
 }
 for (uChr='0'; uChr<='9'; uChr++) {
     idx = (int)uChr;
     data.hashUcs16Eq[ idx ][ 0 ] = idx<'0' ? ' ' : idx;
     ASSERTION(data.hashUcs16Eq[ idx ][ 1 ]==0,"!iso (3)");
 }

 // German special chars, accepted:
 myUcs8[ 0xDF ] = 0xDF;		// 'LETTER SHARP S
 myUcs8[ 0x9C ] = 0x9C;		// Windows codepage extension for 'oe' liason (french)
 myUcs8[ 0x80 ] = 0x80;		// 'EURO SIGN

 // also to the custom UCS8:
 data.hashUcs8Custom[ 0xDF ] = 0xDF;
 strcpy( (char*)data.hashUcs16Eq[ 0xDF ], "ss" );
 strcpy( (char*)data.hashUcs16Eq[ 0x80 ], "EUR." );  // euro sign

 data.RefactorUcs16Eq();

 // Adjust '_' into blank:
 strcpy( (char*)data.hashUcs16Strs[ '_' ], " " );

 return 0;
}


t_uchar* ulang_new_7bit_string (const t_uchar* strIn, const t_uchar** optUcsEqStrs)
{
 const t_uint32 uMask( LANGED_ASCII8BIT_MIDDLE_DOT );

 if ( strIn==nil ) return nil;
 return wuc_new_7bit_string( strIn, strlen( (char*)strIn ), uMask, optUcsEqStrs, mainLang );
}


t_uchar* ulang_new_8bit_string (const t_uchar* strIn, const t_uchar** optUcsEqStrs)
{
 if ( strIn==nil ) return nil;
 return wuc_new_8bit_string( strIn, strlen( (char*)strIn ), 0, optUcsEqStrs, mainLang );
}


t_uchar* ulang_new_8bit_eqstr (const t_uchar* strIn, t_uint32 uMask, const t_uchar** optUcsEqStrs)
{
 // Same as function above, but it may use a different mask:
 // - mask & 1:
 //	allows showing the 8bit character, if valid;
 // - mask & 2:
 //	shows the expanded string

 if ( strIn==nil ) return nil;
 return wuc_new_8bit_string( strIn, strlen( (char*)strIn ), uMask, optUcsEqStrs, mainLang );
}


t_uchar* wuc_new_7bit_string (const t_uchar* strIn, int length, t_uint32 uMask, const t_uchar** optUcsEqStrs, sLanged& langed)
{
 int pos( 0 ), idx( 0 );
 t_uchar aChr;
 char aUcs8;
 char invalidUcs8( (char)uMask );
 gString* pNewString;
 t_uchar* strOut;
 char* strBuf;
 char* myUcs( langed.myUniData->customUcs8[ 2 ] );

 ASSERTION(strIn,"strIn");

 if ( length<0 ) return nil;

 strBuf = (char*)calloc( length+1, sizeof(char) );

 for ( ; pos<length; pos++) {
     aChr = strIn[ pos ];
     aUcs8 = myUcs[ aChr ];
     if ( aUcs8==0 ) {
	 switch ( aChr ) {
	 case '\r':
	     continue;
	 case 0x01:  // ASCII 1d is treated in a special way, kept as is
	     break;
	 default:
	     aUcs8 = (char)(invalidUcs8 & 0xFF);
	     break;
	 }
     }
     strBuf[ idx++ ] = (t_uchar)aUcs8;
 }
 strBuf[ idx ] = 0;

 pNewString = new gString( strBuf );
 ASSERTION(pNewString,"new_7bit_string");
 free( strBuf );

 strOut = pNewString->UStr();
 gStorageControl::Self().Pool().AppendObject( pNewString );
 
 return strOut;
}


t_uchar* wuc_new_8bit_string (const t_uchar* strIn, int length, t_uint32 uMask, const t_uchar** optUcsEqStrs, sLanged& langed)
{
 int pos( 0 ), idx( 0 );
 t_uchar aChr;
 char aUcs8;
 gString* pNewString;
 t_uchar* strOut;
 char* strBuf;
 char* myUcs( langed.myUniData->customUcs8[ 2 ] );

 const int maxEqLen( langed.maxUcs16EqLen );
 const int nrEscChrs( 5 );  // e.g. \0xAB (5 chrs.)
 const int nrMaxExpansionChrs( maxEqLen > nrEscChrs ? maxEqLen : nrEscChrs );

 const bool showOriginal8BitChar( (uMask & 1)!=0 );
 const bool showExpandedEqStr( (uMask & 2)!=0 );

 char strEscaped[ nrEscChrs+8 ];

 ASSERTION(strIn,"strIn");

 DBGPRINT("DBG: uMask=%u, maxEqLen=%d, nrEscChrs=%d, nrMaxExpansionChrs=%d\n",
	  uMask,
	  maxEqLen,
	  nrEscChrs,
	  nrMaxExpansionChrs);

 t_uchar** hashUcs16Strings( (t_uchar**)optUcsEqStrs );
 if ( hashUcs16Strings==nil ) {
     hashUcs16Strings = langed.myUniData->hashUcs16Eq;
 }
 ASSERTION(hashUcs16Strings,"hashUcs16Strings");

 if ( length<0 ) return nil;

 strBuf = (char*)calloc( length * nrMaxExpansionChrs + length + 4, sizeof(char) );

 for ( ; pos<length; pos++) {
     aChr = strIn[ pos ];
     aUcs8 = myUcs[ aChr ];

     if ( aChr=='\r' ) aUcs8 = '\r';

     if ( aUcs8==0 ) {
	 if ( showExpandedEqStr ) {
	     strcpy( strEscaped, (char*)hashUcs16Strings[ aChr ] );
	     if ( strEscaped[ 0 ]==0 )
		 strcpy( strEscaped, "??" );
	 }
	 else {
	     sprintf( strEscaped, "\\0x%02X", aChr );
	 }
	 strcat( strBuf, strEscaped );
	 idx += strlen( strEscaped );
     }
     else {
	 strBuf[ idx ] = showOriginal8BitChar ? aChr : (t_uchar)aUcs8;
	 idx++;
     }
 }
 strBuf[ idx ] = 0;

 pNewString = new gString( strBuf );
 ASSERTION(pNewString,"new_8bit_string");
 free( strBuf );

 pNewString->iValue = length;  // The original string length is stored in the string iValue (if needed)
 strOut = pNewString->UStr();
 gStorageControl::Self().Pool().AppendObject( pNewString );
 
 return strOut;
}

////////////////////////////////////////////////////////////

