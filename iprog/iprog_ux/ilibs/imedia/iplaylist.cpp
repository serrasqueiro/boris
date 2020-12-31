// iplaylist.cpp

#include <string.h>

#include "iplaylist.h"

#include "lib_iobjs.h"
#include "lib_imedia.h"



const t_uint8 sM3uPlaylist::defaultM3U8header[ 4 ]={
    0xEF, 0xBB, 0xBF, 0x0    // [0xEF][0xBB][0xBF]#EXTM3U[0x0D]...
};

////////////////////////////////////////////////////////////
sVplPlaylist::sVplPlaylist ()
    : nEntries( 0 ),
      splitChr( 0x01 ),
      entries( nil )
{
 ReInit();
}


sVplPlaylist::~sVplPlaylist ()
{
 Release();
}


gList* sVplPlaylist::EntryPtr (int idx)
{
 gList* ptrList;

 if ( idx<1 || idx>nEntries ) return nil;
 ASSERTION(entries,"entries");
 ptrList = entries[ idx ];
 ASSERTION(ptrList,"ptrList");
 return ptrList;
}


char* sVplPlaylist::Entry (int idx)
{
 gList* ptrList;

 if ( idx<1 || idx>nEntries ) return nil;
 ptrList = entries[ idx ];
 ASSERTION(ptrList,"ptrList");
 return ptrList->Str( 1 );
}


void sVplPlaylist::ReInit ()
{
 memset(attrOrder, 0, sizeof( attrOrder ));
 strcpy(attrOrder, "%n.%a.%t.%s.%S.%T.%A.%f.%R.%C");
 memset(comment, 0, sizeof( comment ));
 DBGPRINT("DBG: attrOrder: %s\n",attrOrder);
}


void sVplPlaylist::Release ()
{
 DBGPRINT_MIN("DBG: sVplPlaylist::Release (old nEntries: %d)\n",nEntries);
 for (int iter=0; iter<=nEntries; iter++) {
     if ( entries && entries[ iter ] ) {
	 delete entries[ iter ];
     }
 }
 delete[] entries; entries = nil;
 nEntries = 0;
 comment[ 0 ] = 0;
 parts.Delete();
}


void sVplPlaylist::Show (bool doShowAll)
{
 char splitStr[ 2 ];

 splitStr[ 0 ] = splitChr;
 splitStr[ 1 ] = 0;
 DBGPRINT_MIN("DBG: Show(doShowAll=%c) %d (%p)\n",
	      ISyORn( doShowAll ),
	      nEntries,
	      entries);
 if ( doShowAll ) {
     printf("playlist #%d %s%s\n",
	    nEntries,
	    comment[ 0 ] ? " " : "",
	    comment);
 }
 for (int iter=1; entries && iter<=nEntries; iter++) {
     if ( entries[ iter ] ) {
	 if ( doShowAll ) {
	     entries[ iter ]->Show();
	 }
	 else {
	     gParam split( entries[ iter ]->Str( 1 ), splitStr );
	     printf("%s\n",split.Str( 1 ));
	 }
     }
 }
}

////////////////////////////////////////////////////////////
// Aux playlist functions
////////////////////////////////////////////////////////////
const sMdaEncodeCodes* ipl_attr_encoded_by_percent (iMdaFileName& fileName, const char* strPercent, int& index)
{
 int iter( 0 );
 const sMdaEncodeCodes* ptrEncodeCode( fileName.defaultEncodeCodes );

 for (index=-1; ; iter++) {
     if ( ptrEncodeCode[ iter ].usage==-1 )
	 break;
     if ( strPercent!=nil && strPercent[ 0 ]!=0 && strcmp( strPercent, ptrEncodeCode[ iter ].strPercent )==0 ) {
	 index = iter;
	 break;
     }
 }
 return &ptrEncodeCode[ iter ];
}


gList* ipl_new_part (sVplPlaylist& playlist, gList& entry)
{
 int error;
 int iValue;
 int attrType;
 gElem* ptrElem;
 gList* newPart;
 char* strValue( nil );
 char* strRaw( entry.Str( 1 ) );
 const char* strAttr;
 char strSplit[ 8 ];

 strSplit[ 0 ] = playlist.splitChr;
 strSplit[ 1 ] = 0;
 gParam attrs( strRaw, strSplit );

 error = attrs.N()<1;

 if ( error ) return nil;

 newPart = new gList;
 ASSERTION(newPart,"newPart");

 iMdaFileName fileName( attrs.Str( 1 ) );
 const sMdaEncodeCodes* corresponding;

 newPart->Add( fileName );

 for (int orderIdx=0, orderMax=strlen( playlist.attrOrder ); orderIdx<orderMax; orderIdx+=3) {
     strncpy( strSplit, playlist.attrOrder+orderIdx, 2 );
     strSplit[ 2 ] = 0;
     corresponding = ipl_attr_encoded_by_percent( fileName, strSplit, attrType );
     strAttr = corresponding->abbrev;
     strValue = nil;

     if ( strAttr ) {
	 for (ptrElem=attrs.StartPtr(); ptrElem; ptrElem=ptrElem->next) {
	     gParam thisAttr( ptrElem->Str(), "=", gParam::e_StopSplitOnFirst );

	     if ( thisAttr.N()<2 ) continue;

	     if ( strcmp( strAttr, thisAttr.Str( 1 ) )==0 ) {
		 strValue = thisAttr.Str( 2 );

		 switch ( strSplit[ 1 ] ) {
		 case 'C':  // %C	number of channels
		 case 'f':  // %f	frequency
		 case 'R':  // %R	rate
		 case 'S':  // %S	size
		 case 's':  // %s	time
		 case 't':  // %t	track number
		 case 'T':  // %T	7=mp3; 9=ogg; 8=wav; 11=flac
		     iValue = atoi( strValue );
		     newPart->Add( iValue );
		     break;

		 default:
		     newPart->Add( strValue );
		     // String fields have a hint: index 1..n of the defaultEncodeCodes goes into iValue:
		     newPart->EndPtr()->me->iValue = attrType + 1;
		     break;
		 }

		 break;
	     }
	 }

	 if ( strValue==nil ) {
	     newPart->Add( (char*)strAttr );
	     newPart->EndPtr()->me->iValue = -attrType-1;
	 }

#if 0
	 if ( strAttr ) {
	     printf("NEW %s: %s,%d\n",
		    strAttr,
		    newPart->EndPtr()->Str(),
		    newPart->EndPtr()->me->iValue);
	 }
#endif
     }
     else {
	 fprintf(stderr,"Invalid attrOrder {%s}: %s\n",
		 strSplit,
		 playlist.attrOrder);
     }
 }// end FOR

 return newPart;
}

////////////////////////////////////////////////////////////
int ipl_vpl_from_file (const char* strFile, sVplPlaylist& playlist)
{
 int error;
 gFileFetch* fetch;

 ASSERTION(playlist.entries==nil,"twice?");

 fetch = new gFileFetch( (char*)strFile );
 ASSERTION(fetch,"fetch");
 DBGPRINT("DBG: ipl_vpl_from_file %s: %d\n",strFile,fetch->lastOpError);

 if ( fetch->IsOpened()==false ) {
     return fetch->lastOpError;
 }

 error = ipl_vpl_from_raw( fetch->aL, playlist );
 delete fetch;

 DBGPRINT("DBG: ipl_vpl_from_file %s, returned %d\n",strFile,error);
 return error;
}


int ipl_vpl_from_raw (gList& textEntries, sVplPlaylist& playlist)
{
 int iter( 0 ), nEntries( 0 );
 int secs( -1 );
 gElem* ptrElem( textEntries.StartPtr() );
 char* str;

 DBGPRINT("DBG: ipl_vpl_from_raw (entries: %u, playlist=%d=0)\n",
	  textEntries.N(),
	  playlist.nEntries);
 playlist.Release();

 for ( ; ptrElem; ptrElem=ptrElem->next) {
     str = ptrElem->Str();

     if ( str[ 0 ]==0 ) continue;

     if ( str[ 0 ]=='#' ) {
	 if ( playlist.comment[ 0 ]==0 ) {
	     strncpy(playlist.comment, str, sizeof(playlist.comment)-1);
	 }
	 continue;
     }

     nEntries++;
 }

 playlist.entries = new gList*[ nEntries+1 ];
 ASSERTION(playlist.entries,"playlist.entries");

 for (ptrElem=textEntries.StartPtr(); ptrElem; ptrElem=ptrElem->next) {
     str = ptrElem->Str();
     if ( strncmp( str, "#EXTINF:", 8 )==0 ) {
	 secs = atoi( str+8 );
     }
     if ( str[ 0 ]==0 || str[ 0 ]=='#' ) continue;

     iter++;
     playlist.entries[ iter ] = new gList;
     ASSERTION(playlist.entries[ iter ],"Mem!");
     playlist.entries[ iter ]->iValue = secs;
     playlist.entries[ iter ]->Add( ptrElem->Str() );
     secs = -1;
 }

 playlist.nEntries = nEntries;
 DBGPRINT("DBG: ipl_vpl_from_raw with nEntries=%d\n",nEntries);
 return 0;
}


int ipl_vpl_raw_into_parts (gList& entry, sVplPlaylist& playlist)
{
 // Builds up parts from raw list ('entries'):
 gList* newPart( ipl_new_part( playlist, entry ) );

 ASSERTION(newPart,"newPart");
 playlist.parts.AppendObject( newPart );

#if 0
     int dbgIndex( 0 );
     for (gElem* dbgElem=newPart->StartPtr(); dbgElem; dbgElem=dbgElem->next) {
	 const char* strQuote;

	 printf("%d/%d\t",
		++dbgIndex,
		(strlen( playlist.attrOrder )+1) / 3);
	 switch ( dbgElem->me->Kind() ) {
	 case gStorage::e_SInt:
	     printf("[%d] %d\n",
		    dbgElem->me->iValue,
		    ((gInt*)dbgElem->me)->GetInt());
	     break;

	 default:
	     strQuote = dbgElem->me->iValue<0 ? "\0" : "'",
	     printf("[%d] %s%s%s%s\n",
		    dbgElem->me->iValue,
		    strQuote,
		    dbgElem->me->Str(),
		    strQuote,
		    strQuote[ 0 ] ? "\0" : "??");
	     break;
	 }
     }
     printf("\n\n");
     newPart->Show(); printf("!\n\n");
#endif //unused

 return 1;
}


gElem* ipl_vpl_raw_to_parts (sVplPlaylist& playlist)
{
 gElem* ptrStart;
 const int nEntries( playlist.nEntries );

 for (int iter=1; iter<=nEntries; iter++) {
     if ( playlist.entries[ iter ] ) {
	 ipl_vpl_raw_into_parts( *playlist.entries[ iter ], playlist );
     }
 }
 ptrStart = playlist.parts.StartPtr();

#if 0
 for (gElem* ptrElem=ptrStart; ptrElem; ptrElem=ptrElem->next) {
     ASSERTION(ptrElem->me->Kind()==gStorage::e_List,"e_List");
 }
#endif
 return ptrStart;
}


int ipl_find_any_slash (gString& s)
{
 int pos( 0 );
 return ipl_find_any_slash_pos( s, pos );
}

int ipl_find_any_slash_pos (gString& s, int& pos)
{
 pos = (int)s.FindBack( '/' );
 if ( pos ) return pos;
 pos = (int)s.FindBack( '\\' );
 return pos;
}


int ipl_add_strings_from (gElem* ptrFrom, gList& result)
{
 int count( 0 );
 gStorage* ptrMe;

 for ( ; ptrFrom; ptrFrom=ptrFrom->next) {
    result.Add( ptrFrom->Str() );
    ptrMe = result.EndPtr()->me;
    if ( ptrMe ) {
	ptrMe->iValue = ptrFrom->me->iValue;
    }
    count++;
 }
 return count;
}

////////////////////////////////////////////////////////////

