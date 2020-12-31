// langed.cpp

#include "lib_langed.h"

#include "iarg.h"
#include "isoconv.h"

#include "wordunicode.h"

////////////////////////////////////////////////////////////
// Globals
// ---------------------------------------------------------
sLanged mainLang;

////////////////////////////////////////////////////////////
const sLangParam* langed_langcode_params (int langCode, int mask, t_int32& ioCode)
{
 static short idx;
 static const sLangParam basicLangs[]={
     { 0, "en", "en-US", "english" },
     { -1, "\0", "\0", "\0" }
 };

 ioCode = 0;
 return &basicLangs[ idx ];
}

////////////////////////////////////////////////////////////
// sLanged struct/methods
////////////////////////////////////////////////////////////
int sLanged::Init ()
{
 t_int32 ioCode( 0 );
 const sLangParam* ptrParm( langed_langcode_params( currentLangCode, 0, ioCode ) );

 if ( ptrParm==nil ) return -1;
 sAbbrev.Set( (char*)ptrParm->abbrev );

 myUniData = ptrUniData;
 ASSERTION(myUniData,"myUniData");

 return 0;
}


void sLanged::Release ()
{
 delete cached;
 cached = nil;
}

////////////////////////////////////////////////////////////
int init_languages (sLanged& langed)
{
 int idx( 0 ), maxLen( 0 );
 t_uchar* strEq;

 langed.Init();
 ASSERTION(langed.myUniData,"init_languages()");
 ulang_prepare_custom_iso( 0, *(langed.myUniData) );

 // Check the maximum number of substitution strings:
 for (int len; idx<256; idx++) {
     strEq = langed.myUniData->hashUcs16Eq[ idx ];
     ASSERTION(strEq,"strEq");
     len = strlen( (char*)strEq );
     if ( len>maxLen ) maxLen = len;
 }

 langed.maxUcs16EqLen = maxLen;
 return 0;
}

////////////////////////////////////////////////////////////
// langed functions
////////////////////////////////////////////////////////////
int langed_init ()
{
 int error( init_languages( mainLang ) );
 DBGPRINT("lange_init() returns %d\n",error);
 return error;
}


int langed_finit ()
{
 gStorageControl::Self().DeletePool();
 return 0;
}


int langed_split_words (const t_uchar* buf, int mask, gList& listed)
{
 int iter( 0 ), wordLen( -1 );
 int code( -1 );
 t_uchar uChr, aChr, nextChr;
 gString entry;

 for ( ; (uChr = buf[ iter ])!=0; ) {
     aChr = uChr;
     nextChr = buf[ ++iter ];
     if ( (aChr>='a' && aChr<='z') ||
	  (aChr>='A' && aChr<='Z') ) {
	 code = 1;
     }
     else {
	 if ( aChr=='-' || aChr=='\'' || aChr==LANGED_ASCII8BIT_ACCUTE_ACCENT ) {
	     code =
		 (aChr>='a' && aChr<='z') ||
		 (aChr>='A' && aChr<='Z');
	 }
	 else {
	     code = 0;
	 }
     }
     if ( code==1 ) {
	 entry.Add( (char)aChr );
     }
     else {
	 wordLen = entry.Length();
	 if ( wordLen ) {
	     if ( uChr=='\n' ) entry.Add( '\n' );
	     listed.Add( entry );
	 }
	 entry.Delete();
     }
 }

 wordLen = entry.Length();
 if ( wordLen>0 ) listed.Add( entry );
 return (int)listed.N();
}


char* langed_charset (gString& charsetName, gString& charsetSuggested, t_unitable& charset)
{
 gParam aSplit( charsetName, "-" );
 int iter( 2 ), nrItems( (int)aSplit.Str( iter ) );

 charset = 0;

 charsetSuggested.Set( aSplit.Str( 1 ) );

 for ( ; iter<=nrItems; iter++) {
     gString sItem( aSplit.Str( iter ) );
     if ( sItem.IsEmpty() ) continue;
     if ( charsetSuggested.Length() ) {
	 if ( iter<nrItems )
	     charsetSuggested.Add( '-' );
     }
     charsetSuggested.AddString( sItem );
 }
 return charsetSuggested.Str();
}

////////////////////////////////////////////////////////////


