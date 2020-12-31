// dub.cpp


#include <string.h>
#include <errno.h>

#include "dub.h"


const t_uint32 mDir::defaultSortMask=1;

////////////////////////////////////////////////////////////
mDir::mDir (char* dName, bool ordered)
    : gDirGeneric( e_StoreExtend ),
      sortMask( defaultSortMask ),
      apDir( nil ),
      names( nil ),
      timestamps( nil )

{
 if ( dName ) {
     GetDir( dName, ordered, nil, nil );
 }
}
    

mDir::mDir (gString dirName, bool ordered)
    : gDirGeneric( e_StoreExtend ),
      sortMask( defaultSortMask ),
      apDir( nil ),
      names( nil ),
      timestamps( nil )
{
 if ( dirName.Length() ) {
     GetDir( dirName.Str(), ordered, nil, nil );
 }
}


mDir::~mDir ()
{
 delete[] names;
 delete[] timestamps;
}


void mDir::AddDir (char* s)
{
 Add( s );
}


void mDir::AddFile (char* s)
{
 Add( s );
}


int mDir::GetDir (const char* strPath, bool ordered, const char* strGlobPositive, const char* strGlobNegative)
{
 struct dirent* dp( nil );
 int count( 0 );
 gString sPath;
 gFileStat aStat;
 char* s;

 thisCleanUp();

 if ( strPath ) {
     sDirName.SetDirName( (char*)strPath );  // Note: Set() wouldn't be enough

     if ( apDir ) {
	 closedir( apDir );
     }
     apDir = opendir( strPath );

     if ( apDir ) {
	 gList stampA;
	 gList stampM;
	 gList stampC;

	 for ( ; ; count++) {
	     dp = readdir( apDir );
	     if ( count==0 && dp==NULL ) {
		 lastOpError = errno;
	     }
	     if ( dp==NULL ) break;

	     s = (char*)dp->d_name;
	     gString sName( s );

	     if ( sName.Match( ".." ) ) continue;

	     sPath = sDirName;
	     sPath.AddString( sName );

	     aStat.Update( sPath.Str() );
	     if ( sName.Match( "." ) ) {
		 iValue = aStat.status.mTime;
		 continue;
	     }

	     if ( aStat.statusL.IsDirectory() ) {
		 sName.Add( gSLASHCHR );
		 AddDir( sName.Str() );
	     }
	     else {
		 AddFile( sName.Str() );
	     }

	     stampA.Add( (int)aStat.status.aTime );
	     stampM.Add( (int)aStat.status.mTime );
	     stampC.Add( (int)aStat.status.cTime );

	     EndPtr()->iValue = (int)aStat.status.USize();
	 }

	 closedir( apDir ); apDir = nil;

	 ASSERTION(count>=(int)N(),"count?");

	 // Update stamps
	 names = new gString[ N()+1 ];
	 timestamps = new sTriplet[ N()+1 ];
	 gElem* ptrA( stampA.StartPtr() );
	 gElem* ptrM( stampM.StartPtr() );
	 gElem* ptrC( stampC.StartPtr() );;

	 int iter( 0 );
	 for (gElem* pIter=StartPtr(); pIter; pIter=pIter->next) {
	     iter++;
	     /*
	       pIter->me->iValue = ptrM->me->iValue;
	       pIter->iValue = ptrC->me->iValue;
	       printf("%9d %9d  %s\n", pIter->me->iValue, pIter->iValue, pIter->Str());
	     */
	     timestamps[ iter ].stampA = ptrA->me->iValue;
	     timestamps[ iter ].stampM = ptrM->me->iValue;
	     timestamps[ iter ].stampC = ptrC->me->iValue;
	     ptrA = ptrA->next;
	     ptrM = ptrM->next;
	     ptrC = ptrC->next;
	 }
     }
     else {
	 lastOpError = errno;
     }
 }

 return lastOpError;
}


void mDir::thisSortFromTo (gList& from, t_uint32 mask, gList& to)
{
 unsigned iter( 0 ), numElements( 0 );
 gElem* ptrElem( from.StartPtr() );
 const char* strThis;
 char* strMx( nil );

 to.Delete();

 // Basic sorting!
 //
 //	mask
 //		1		sort by name, ascending
 //		2		sort by name, descending

 if ( names ) {
     if ( mask & 3 ) {
	 for ( ; ptrElem; ptrElem=ptrElem->next) {
	     names[ ++iter ].Set( ptrElem->Str() );
	 }
	 for (numElements=iter; to.N()<numElements; ) {
	     strMx = nil;
	     for (iter=1; iter<=numElements; iter++) {
		 strThis = names[ iter ].Str();
		 if ( strThis[ 0 ]==0 ) continue;
		 if ( strMx==nil ) {
		     strMx = (char*)strThis;
		 }
		 else {
		     if ( mask & 1 ) {
			 if ( strcmp( strThis, strMx )<0 ) {
			     strMx = (char*)strThis;
			 }
		     }
		     else {
			 if ( strcmp( strThis, strMx )>0 ) {
			     strMx = (char*)strThis;
			 }
		     }
		 }
	     }
	     to.Add( strMx );
	     DBGPRINT_MIN("thisSortFromTo %u\tadded: %s\n", to.N(), strMx);
	     strMx[ 0 ] = 0;
	 }
     }
     else {
	 to.CopyList( from );
     }
 }
}

////////////////////////////////////////////////////////////

