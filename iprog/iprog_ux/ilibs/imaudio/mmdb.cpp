// mmdb.cpp

#include "mmdb.h"

#include "ioaux.h"

////////////////////////////////////////////////////////////
// Aux functions
////////////////////////////////////////////////////////////
int valid_dbb_sanity (sMmDb& mmdb, t_uint32 sanityMask)
{
 bool isDirectory( false );
 int error( mmdb_entry_is_directory( mmdb.sBasePath, isDirectory ) );

 ASSERTION(mmdb.scheme!=0x0000,"mmdb.scheme");
 if ( error ) return error;
 error = isDirectory==false;
 return error;
}

////////////////////////////////////////////////////////////
// mmdb functions
////////////////////////////////////////////////////////////
int adbb_open (const char* strPath, sMmDb& mmdb)
{
 int error;
 char* strDup( strdup( strPath ) );

 // strdup because strPath can be exactly sBasePath already
 mmdb.sBasePath.Set( strDup );
 ASSERTION(strDup,"strDup");
 free( strDup );

 mmdb_entry_trim( mmdb.sBasePath, 5 );
 error = valid_dbb_sanity( mmdb, 0 );
 DBGPRINT("DBG: valid_dbb_sanity( {%s}, 0 )=%d\n",
	  mmdb.sBasePath.Str(),
	  error);
 mmdb.runCode = error==0;
 return error;
}


int adbb_close (sMmDb& mmdb)
{
 int error( mmdb.runCode<1 );
 mmdb.runCode = -1;
 return error;
}

////////////////////////////////////////////////////////////
int mmdb_entry_trim (gString& sName, int mask)
{
 bool isDirectory( false );
 bool isRootDir( false );

 // mask has 1: slash is added for directories
 // mask has 4: a dot is added

 DBGPRINT_MIN("DBG: mmdb_entry_trim (BEFORE1): {%s}\n",sName.Str());

 if ( sName[1]==gSLASHCHR ) {
     isRootDir = sName.FindExcept( gSLASHSTR )==0;
 }

 gio_dir_trim( sName );
 if ( mask & 4 ) {  // Allow '.' input, result is the same ('.')
     if ( sName.IsEmpty() ) {
	 if ( isRootDir ) {
	     sName.Add( gSLASHSTR );
	 }
	 else {
	     sName.Add( "." );
	 }
     }
 }
 if ( mask & 1 ) {
     mmdb_entry_is_directory( sName, isDirectory );
     if ( isDirectory ) sName.Add( gSLASHSTR );
 }
 DBGPRINT_MIN("DBG: mmdb_entry_trim (AFTER), IsDIR? %c (%d): {%s}\n",
	      ISyORn( isDirectory ), mask,
	      sName.Str());
 return isDirectory==true;
}


sFileStat* mmdb_entry_stat (gString& sName, int& error)
{
 static sFileStat aStat;
 gFileStat aFile( sName );

 aStat.Copy( aFile.status );
 error = aFile.lastOpError;
 DBGPRINT_MIN("DBG stat, error: %d (inode: %d): %s\n", error, aStat.inode, sName.Str());
 return &aStat;
}


int mmdb_entry_is_directory (gString& sName, bool& isDirectory)
{
 int error;
 sFileStat* ptrStat( mmdb_entry_stat( sName, error ) );
 isDirectory = ptrStat->IsDirectory();
 return error;
}


int mmdb_list_errors (int errorCode, gList& list, FILE* fReport, FILE* fLog)
{
 bool isLastElem;
 gElem* ptrElem( list.StartPtr() );
 const char* strSep( "\n" );

 ASSERTION(fLog==nil,"fLog not implemented");
 if ( fReport==nil ) return -1;

 if ( errorCode>0 ) {
     // If error is negative, no error-code is displayed
     fprintf(fReport,"Error-code: %d\n",errorCode);
 }
 else {
     if ( errorCode==-9 ) {
	 strSep = " ";
     }
     DBGPRINT_MIN("DBG: Error-code: %d (msgs# %u)\n",errorCode,list.N());
 }

 for ( ; ptrElem; ptrElem=ptrElem->next) {
     isLastElem = ptrElem->next==nil;

     if ( ptrElem->me->Kind()==gStorage::e_List ) {
	 mmdb_list_errors( -9, *((gList*)ptrElem->me), fReport, fLog );
     }
     else {
	 fprintf(fReport,"%s%s",
		 ptrElem->Str(),
		 isLastElem ? "\n" : strSep);
     }
 }
 return 0;
}

////////////////////////////////////////////////////////////

