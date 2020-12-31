// inames.cpp

#include <string.h>

#include "inames.h"
#include "isoconv.h"

#include "lib_iobjs.h"

////////////////////////////////////////////////////////////
// Static members
// ---------------------------------------------------------
const sMdaEncodeCodes iMdaFileName::defaultEncodeCodes[]={
    	{ 3, "%A", "ALBM", "Album" },
	{ 2, "%a", "ATST", "Artist" },
	{ 0, "%C", "CHNL", "Channels" },  // Number of channels
	{ 0, "%f", "FREQ", "Frequency" },  // in Hz: 44100 Hz, or so
	{ 4, "%n", "NAME", "Name" },  // Track-name
	{ 0, "%R", "RATE", "Rate" },  // Rate or average rate, in Kbps
	{ 0, "%S", "SIZE", "Size" },  // Size, in bytes
	{ 0, "%s", "TIME", "Time" },  // Time, in seconds
	{ 1, "%t", "TRKN", "Track" },  // Track number: 1..n
	{ 0, "%T", "TYPE", "Type" },  // Type: 7=mp3; 9=ogg; 10=wma; 8=wav; 11=flac
	{ -1, NULL, NULL, NULL }
};

// Example of fields from VPL:
//
//	H:\some_path\a.mp3
//		this is the path.
//	NAME=Look
//		the title name.
//	ATST=Bruce
//		the artist name.
//	ALBM=Scenes
//		the album name.
//	GENR=Folk Rock
//		Genre id, or description.
//	YEAR=1988
//		year.
//	TYPE=7
//		type of file: 7 is mp3.
//	RATE=128
//		bitrate (actually average).
//	FREQ=44100
//		frequency.
//	CHNL=2
//		number of channels.
//	SIZE=5266411
//		file size, in bytes.
//	TIME=329
//		time, in seconds.



////////////////////////////////////////////////////////////
// iMdaFileName - Handling media file names
// ---------------------------------------------------------
iMdaFileName::iMdaFileName (char* aStr)
    : gString( aStr )
{

}


iMdaFileName::~iMdaFileName ()
{

}

////////////////////////////////////////////////////////////
// ptm functions
////////////////////////////////////////////////////////////
int ptm_join_words_str (gList& listIn, const char* strBlank, gString& sResult)
{
 int count( 0 );
 gElem* ptrElem( listIn.StartPtr() );
 gString* ptrStr;

 for ( ; ptrElem; ptrElem=ptrElem->next) {
     ptrStr = (gString*)ptrElem->me;
     if ( ptrStr->Length()==0 ) continue;
     if ( sResult.Length() ) {
	 sResult.Add( (char*)strBlank );
     }
     sResult.AddString( *ptrStr );
     count++;
 }
 return count;
}


int ptm_join_words (gList& listIn, gString& sResult)
{
 return ptm_join_words_str( listIn, " ", sResult );
}


int ptm_cutoff_openclose (const char* strCut, gString& s)
{
 int countCut( 0 );
 int error( 0 );
 char chrOpen( *strCut );
 char chrClose( 0 );
 unsigned pos, iter;
 unsigned uLen( s.Length() );
 gString name( s );

 if ( chrOpen==0 ) return -2;
 chrClose = *(++strCut);
 ASSERTION(chrClose,"chrClose");

 for ( ; chrOpen!=0; ) {
     pos = name.Find( chrOpen );
     if ( pos ) {
	 for (iter=pos+1; iter<=uLen; iter++) {
	     if ( name[ iter ]==chrClose )
		 break;
	 }
	 if ( iter<=uLen ) {
	     countCut++;
	     name.Delete( pos, iter );
	     uLen = name.Length();
	     continue;
	 }
	 error = -1;
	 break;
     }

     if ( *(++strCut)==0 ) break;
     chrOpen = *(++strCut);
     ASSERTION(chrOpen,"chrOpen");
     chrClose = *(++strCut);
     ASSERTION(chrClose,"chrClose");
 }
 if ( error<0 ) return error;
 s = name;
 return countCut;
}


int ptm_cutoff_brackets (gString& sName)
{
 const char* strCut( "<> {} [] ()" );
 int countCut( ptm_cutoff_openclose( strCut, sName ) );
 return countCut;
}


int ptm_name_string (gString& inName, int namesMask, const t_uchar** optUcsEqStrs, gString& outName)
{
 int error( 0 );
 gList* newList;

 outName.Delete();

 if ( inName.Length() ) {
     newList = ptm_new_name( inName, nil, namesMask, optUcsEqStrs );
     error = newList==nil;
     if ( error ) return 1;

     outName.Set( newList->StartPtr()->next->me->Str() );
     delete newList;
 }
 return error;
}


char* ptm_name_str (const char* strInName, int namesMask, const t_uchar** optUcsEqStrs)
{
 gList* listed( ptrUniData->buffers );
 gString inName( (char*)strInName );

 if ( listed ) delete listed;

 listed = ptm_new_name( inName, nil, namesMask, optUcsEqStrs );
 ptrUniData->buffers = listed;

 if ( listed ) {
     return listed->StartPtr()->next->me->Str();
 }

 listed = new gList;
 ptrUniData->buffers = listed;
 ASSERTION(listed,"listed");

 listed->Add( "" );
 return listed->Str( 1 );  // Empty string (but not nil)
}


gList* ptm_new_name (gString& inName, gList* newNames, int namesMask, const t_uchar** optUcsEqStrs)
{
 gList* newList( nil );
 t_uchar** hashStrings( (t_uchar**)optUcsEqStrs );
 gString sNew;
 gString sCut( inName );
 gString sBasic;
 unsigned pos;
 bool hasThe( false );
 bool suppressArtistWords( (namesMask & 1)==0 );
 t_uchar aChr;

 ASSERTION(newNames==nil,"newNames");  // unimplemented (for future merge...)

 if ( hashStrings==nil ) {
     hashStrings = (t_uchar**)&ptrUniData->hashUcs16Strs;
 }
 ASSERTION(hashStrings,"hashStrings");

 ptm_cutoff_brackets( sCut );
 gParam words( sCut, " " );

 pos = words.Match( "The" );
 if ( suppressArtistWords ) {
     if ( words.N()>1 && (pos==1 || pos==words.N()) ) {
	 hasThe = true;
	 words.Delete( pos, pos );
     }
     pos = words.Match( "THE" );
     if ( words.N()>1 && (pos==1 || pos==words.N()) ) {
	 hasThe = true;
	 words.Delete( pos, pos );
     }
 }

 sCut.Delete();
 ptm_join_words( words, sCut );

 if ( sCut.Match( "Unknown", true ) ) return nil;
 pos = inName.Find( "Data" );
 if ( pos && pos+4>=inName.Length() ) return nil;

 for (pos=1; pos<=sCut.Length(); pos++) {
     aChr = sCut[ pos ];
     printf("sCut={%s} pos=%d\t%s\n", sCut.Str(), pos, hashStrings[ aChr ]);
     sNew.Add( hashStrings[ aChr ] );
 }
 sNew.UpString();  // Now it gets upper-case

 // Wait!, before adding, check sanity of english words
 // --->>>

 if ( sNew.Find( "VARIOUS" )==1 || sNew.Find( "VARIOS" )==1 )
     return nil;

 // <<<---

 if ( hasThe ) {
     if ( sNew.Length() )
	 sNew.Add( "," );
     sNew.Add( "THE" );
 }

 gParam refit( sNew, "." );
 ptm_join_words_str( refit, ".", sBasic );

 newList = new gList;
 ASSERTION(newList,"newList");

 newList->Add( inName );
 newList->Add( sBasic );
 newList->GetLastObjectPtr()->Rehash();

 DBGPRINT("DBG: newList {%s; %s}\n",
	  inName.Str(),
	  sBasic.Str());
 return newList;
}


gList* ptm_sort_names (gList& listed, int mask)
{
 gList* newList( new gList );
 gElem* ptrElem( listed.StartPtr() );

 ASSERTION(newList,"newList");

 for (int code=0; ptrElem; ptrElem=ptrElem->next) {
     gString* newObj( new gString( ptrElem->me->Str() ) );
     ASSERTION(newObj,"newObj");

     code = newList->InsertOrderedUnique( newObj );
     if ( code==-1 ) {
	 delete newObj;
     }
     newObj->iValue = ptrElem->me->iValue;
 }
 return newList;
}


gList* ptm_invert_list (gList& listed, int mask)
{
 gList* newList( new gList );
 gElem* ptrElem( listed.EndPtr() );

 ASSERTION(newList,"newList");

 for ( ; ptrElem; ptrElem=ptrElem->prev) {
     gString* newObj( new gString( ptrElem->me->Str() ) );
     ASSERTION(newObj,"newObj");

     newList->AppendObject( newObj );
     newObj->iValue = ptrElem->me->iValue;
 }

 return newList;
}

////////////////////////////////////////////////////////////

