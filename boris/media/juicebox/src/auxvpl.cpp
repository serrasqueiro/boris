// auxvpl.cpp
//
//	Auxiliar vpl play list entry builder

#include "auxvpl.h"

////////////////////////////////////////////////////////////
iEntry* new_vpl_entry (gString& sLine, unsigned pos)
{
 bool isVpl( pos > 0 );
 iEntry* ptrVPL( nil );
 gString preExtInf;  // #EXTINF:  <-- intention is the caller adds this too
 gString sName, sArtist;
 gString* myStr( nil );
 int secs( 0 );

 if ( isVpl ) {
     gParam vplFields( sLine.Str( pos ), JCL_ASCII01S );

     ptrVPL = new iEntry;
     ASSERTION(ptrVPL,"ptrVPL");

     for (gElem* pFields=vplFields.StartPtr(); pFields; pFields=pFields->next) {
	 myStr = (gString*)pFields->me;

	 unsigned posEq( myStr->Find( "=" ) );
	 if ( posEq ) {
	     if ( myStr->Find( "TIME=" )==1 ) {
		 secs = atoi( myStr->Str( strlen("TIME=") ) );
		 ptrVPL->iValue = secs;
	     }
	     else {
		 if ( myStr->Find( "NAME=" )==1 ) {
		     sName.Set( myStr->Str( 5 ) );
		 }
		 else {
		     if ( myStr->Find( "ATST=" )==1 ) {
			 sArtist.Set( myStr->Str( 5 ) );
		     }
		 }
	     }
	 }// IF posEq...
     }//end FOR

     sName.Trim();
     sArtist.Trim();

     if ( sArtist.Length() && sArtist.Find( "unknown", true )==0 ) {
	 preExtInf.AddString( sArtist );
     }
     if ( sName.Length() && sName.Find( "track", true )==0 ) {
	 if ( preExtInf.Length() ) {
	     preExtInf.Add( " - " );
	 }
	 preExtInf.AddString( sName );
     }

     gString sExtInf( preExtInf.Length() + 64, '\0' );

     sprintf(sExtInf.Str(), "#EXTINF:%d,%s",
	     secs,
	     preExtInf.Str());
     ptrVPL->SetComment( sExtInf.Str() );
 }

 return ptrVPL;
}


iEntry* search_entry_display (iEntry& entries, iEntry& oneEntry)
{
 iEntry* thisEntry;
 char* strThere;
 char* strOne;

 // Compares both e.g.
 //
 //	#EXTINF:182,Artist - Title\nfile.mp3
 // and:
 //	#EXTINF:182,Artist - Title\nfile.mp3 (from one entry)
 // AND also checks whether filename (e.g. file.mp3) is common to both.
 //
 // Either check is considered a match.

 for (gElem* pElem=entries.StartPtr(); pElem; pElem=pElem->next) {
     thisEntry = (iEntry*)pElem->me;
     if ( thisEntry->display.Match( oneEntry.display ) ) {
	 return thisEntry;
     }

     strThere = thisEntry->Str();
     strOne = oneEntry.Str();

     if ( strcmp( strThere, strOne )==0 ) {
	 return thisEntry;
     }
 }
 return nil;
}

////////////////////////////////////////////////////////////

