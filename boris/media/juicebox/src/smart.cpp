// smart.cpp


#include <errno.h>
#include <unistd.h>
#include <string.h>

#include "lib_iobjs.h"
#include "smart.h"

////////////////////////////////////////////////////////////
Media* smart_rename (Media& nameBy, t_uint32 uMask)
{
 Media* result;

 result = new Media;
 return result;
}


bool NameUnquote (gString& aString)
{
 if ( aString[ 1 ]=='"' && aString[ aString.Length() ]=='"' ) {
     aString.Delete( 1, 1 );
     aString.Delete( aString.Length() );
     aString.Trim();
     return 2;  // deleted at least two chars
 }
 return 0;
}


int PhraseUnHTML (gString& words, gList* pIssues)
{
 int count( 0 );
 t_uchar uChr;
 gList symbols;

 static const sNameHTML htmlNames[]={
     { 38, "&", "amp" },	// &amp;
     { 39, "'", "apos" },	// &apos;	Apostrophe
     { 126, "~", "tilde" },	// &tilde;	(ASCII 126d)
     { 0, "", nil },
     { 0, "", nil }
 };

 for (int iter=1; iter<=(int)words.Length(); iter++) {
     uChr = words[ iter ];
     if ( uChr=='&' ) {
	 if ( strchr( words.Str( iter ), ';' ) ) {
	     gString htmlName;
	     const char* name;
	     const char* newString( nil );

	     count++;
	     for (iter++; (uChr = words[ iter ])!=0; iter++) {
		 if ( uChr==';' ) break;
		 htmlName.Add( uChr );
	     }

	     for (int search=0; (name = htmlNames[ search ].name)!=nil; search++) {
		 if ( strcmp( name, htmlName.Str())==0 ) {
		     newString = htmlNames[ search ].str;
		     break;
		 }
	     }

	     if ( newString ) {
		 symbols.Add( newString );
	     }
	     else {
		 if ( pIssues ) {
		     pIssues->Add( htmlName );
		 }
	     }
	 }
     }
     else {
	 gString sChar( uChr );
	 symbols.Add( sChar );
     }
 }

 if ( count ) {
     words.SetEmpty();
     for (gElem* pElem=symbols.StartPtr(); pElem; pElem=pElem->next) {
	 words.Add( pElem->Str() );
     }
 }
 return count;
}


int NameFromRawWPL (gString& sLine)
{
 int result( 0 );
 int pos( sLine.Find("<media src=", true) );  // xspf playlists (or WPL)
 if ( pos>0 ) {
     sLine.Delete( 1, pos + strlen( "<media src=" ) - 1 );
     pos = sLine.Find( "/>" );
     if ( pos ) {
	 sLine.Delete( pos );
     }
     result = pos;
 }
 if ( sLine[ 1 ]=='"' && sLine[ sLine.Length() ]=='"' ) {
     sLine.Delete( 1, 1 );
     sLine.Delete( sLine.Length() );
     sLine.Trim();
 }
 return result;
}


int NameFromWPL (gString& sLine, t_uint16 mask, gString& sNew)
{
 gString sTID;
 gString sCID;
 int result( NameFromWPL_tid( sLine, mask, sNew, sTID, sCID ) );

 return result;
}


int NameFromWPL_tid (gString& sLine, t_uint16 mask, gString& sNew, gString& sTID, gString& sCID)
{
 int pos( sLine.Find("<media src=", true) );  // xspf playlists (or WPL)
 int posEnd;
 int safeID( 100 );

 if ( pos>0 ) {
     sNew = sLine;
     sNew.Delete( 1, pos + strlen( "<media src=" ) - 1 );
     posEnd = sNew.Find( "/>" );
     if ( posEnd ) {
	 sNew.Delete( posEnd );
     }
     else {
	 return -1;  // unfinished media-src tag
     }

     for ( ; safeID>0; safeID--) {
         int anyID( 0 );
	 int tidPos( sNew.Find( " tid=", true ) );
	 if ( tidPos ) {
	     sTID = sNew.Str( tidPos + strlen( "tid=" ) );
	     sNew.Delete( tidPos );
	     sNew.TrimRight();
	     anyID = 1;
	 }
	 else {
	     int cidPos( sNew.Find( " cid=", true ) );
	     if ( cidPos ) {
                 sCID = sNew.Str( cidPos + strlen( "cid=" ) );
		 sNew.Delete( cidPos );
		 sNew.TrimRight();
		 anyID = 1;
	     }
	 }
	 if ( anyID==0 ) break;
     }

     sNew.Trim();
     NameUnquote( sNew );
     NameUnquote( sTID );
     //printf("[%s], TID=%s\n\n", sNew.Str(), sTID.Str());

     // Clean-up special HTML
     PhraseUnHTML( sNew, nil );
 }

 return pos==0;  // returns 1 if no media-src tag was found
}

////////////////////////////////////////////////////////////

