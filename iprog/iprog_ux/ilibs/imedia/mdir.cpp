// smart.cpp


#include <errno.h>
#include <unistd.h>

#include "lib_iobjs.h"
#include "mdir.h"

////////////////////////////////////////////////////////////
Media::Media ()
{
}


Media::Media (const char* aStr)
    : iEntry( aStr )
{
}


Media::~Media ()
{
}


void Media::NormalizedName (const char* strName, gString& result)
{
 t_uchar chr;

 result.SetEmpty();

 if ( strName ) {
     result.Set( (char*)strName );
     result.Trim();

     for (int idx=result.Length(); idx>1; idx--) {
	 chr = result[ idx ];
	 if ( chr=='\\' || chr=='/' ) {
	     result.Delete( idx );
	 }
	 else {
	     break;
	 }
     }
     result.TrimRight();

     for (int idx=1; ; ) {
	 chr = result[ idx ];
	 if ( chr=='\\' || chr=='/' ) {
	     if ( chr==result[ idx+1 ] ) {
		 result.Delete( 1, 1 );
		 continue;
	     }
	 }
	 break;
     }
 }
}


Media* Media::DirList (const char* dName)
{
 Media* result;
 struct dirent* dp;
 DIR* apDir;
 bool any( false );
 bool hasParent( false );
 char* name( nil );
 iEntry* entry;

 lastOpError = 0;
 if ( dName==nil ) return nil;

 result = new Media;
 ASSERTION(result,"Mem");

 NormalizedName( dName, display );

 gDirStream dirStream( (char*)dName );
 apDir = dirStream.pdir;

 for ( ; apDir; any=true) {
     dp = readdir( apDir );
     if ( dp==NULL ) {
	 if ( any==false ) {
	     lastOpError = errno;
	 }
	 break;
     }

     name = dp->d_name;

     if ( name[ 0 ]==0 ) continue;
     if ( strcmp(name,".")==0 || (hasParent = (strcmp(name,"..")==0)) ) continue;

     entry = new iEntry( name );
     ASSERTION(entry,"entry");

     result->AppendObject( entry );
 }

 return result;
}

////////////////////////////////////////////////////////////
iEntry* dir_normalized (bool complain, gDir& aDir)
{
 int error( 0 );
 unsigned pos;
 gElem* ptrIter;
 gList* pFailed( &aDir.errUnstatL );
 iEntry* pFound( nil );

 if ( complain ) {
     pFound = new iEntry;
 }

 if ( pFailed->N() ) {
     for (ptrIter=pFailed->StartPtr(); ptrIter; ptrIter=ptrIter->next) {
	 gString sName( ptrIter->Str() );
	 for ( ; ; ) {
	     pos = sName.Find( "a`" );
	     if ( pos ) {
		 sName.Delete( pos+1, pos+1 );
		 sName[ pos ] = 'à';
		 continue;
	     }
	     break;
	 }

	 gFileStat aStat( sName );
	 error = aStat.HasStat();
	 if ( error==0 ) {
	     aDir.Add( sName );
	 }
	 if ( pFound ) {
	     gString msg( sName.Length()+64, '\0' );
	     sprintf(msg.Str(), "%s: %s",
		     error ? "Unread entry" : "Corrected entry",
		     sName.Str());
	     pFound->Add( msg );
	 }
     }
 }
 return pFound;
}

////////////////////////////////////////////////////////////

