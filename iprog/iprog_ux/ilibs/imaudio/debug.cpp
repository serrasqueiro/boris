// debug.cpp, for libimaudio

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib_imaudio.h"

////////////////////////////////////////////////////////////
void usage ()
{
 printf("\
debug ALL\n\
	-> Shows all supported extensions\n\
\n\
OR\n\
\n\
debug FILE\n\
	-> Shows file attr\n\
		(being implemented)\n\
");
}

////////////////////////////////////////////////////////////
int dbg_open_dir (FILE* fError, const char* strDir, FILE* fOut)
{
 sListingConf aConf;
 gList* newDir( ima_open_dir( strDir, nil, aConf ) );
 gElem* ptrElem;

 ASSERTION(fOut,"fOut");

 if ( newDir==nil ) {
     fprintf(fError,"Uops, dir error: %d: %s\n",
	     aConf.lastError,
	     strDir);
     return 2;
 }

 for (ptrElem=((gList*)(newDir->GetObjectPtr( 2 )))->StartPtr(); ptrElem; ptrElem=ptrElem->next) {
     fprintf(fOut,"%s\n",ptrElem->Str());
 }
 return 0;
}


int dbg_open_file (FILE* fError, const char* strFile, int fdIn, gFileStat* ptrStat, FILE* fOut)
{
 int error( 0 );
 int idx( 1 );
 sListingConf aConf;
 gList* newGet( ima_basic_open( fdIn, ptrStat, aConf ) );
 gElem* ptrElem;

 ASSERTION(fOut,"fOut");
 if ( newGet==nil ) {
     fprintf(fError,"Uops, file error (size: %lu): %d: %s\n",
	     ptrStat ? (unsigned long)ptrStat->statusL.USize() : 0,
	     aConf.lastError,
	     strFile);
     return 2;
 }

 for (ptrElem=newGet->StartPtr(); ptrElem; ptrElem=ptrElem->next) {
     fprintf(fOut,"[%d] %d\t%s\n",
	     idx,
	     ptrElem->me->iValue,
	     ptrElem->Str());
     idx++;
 }

 // Deleting 'newGet' list
 delete newGet;

 return error;
}


int dbg_test (const char* strArg1)
{
 int error( -1 );
 const char* strFile( strArg1 );
 FILE* fIn( strFile ? fopen( strFile, "rt" ) : stdin );
 FILE* fError( stderr );
 FILE* fOut( stdout );
 bool isDir;

 gFileStat aStat( (char*)strFile );
 isDir = aStat.HasStat() && aStat.IsDirectory();

 if ( isDir ) {
     return dbg_open_dir( fError, strFile, fOut );
 }

 if ( fIn==nil ) {
     fprintf(fError,"Cannot open: %s\n",strFile);
     return 2;
 }

 error = dbg_open_file( fError, strFile, fileno( fIn ), &aStat, fOut );
 if ( fIn!=stdin ) fclose( fIn );
 return error;
}


int do_check_all ()
{
 int index;
 t_uint8 supported;

 gString sName;
 char* strExt;
 const char* strMimeType;
 sListingConf aConf;

 gElem* ptrElem( aConf.hashedExtensions.StartPtr() );
 sFileTypeSupport* ptrSup;

 for ( ; ptrElem; ptrElem=ptrElem->next) {
     strExt = ptrElem->Str();
     sName.Set( "name" );
     sName.Add( strExt );

     index = ptrElem->me->iValue;
     ptrSup = &aConf.SupportByExt( strExt, supported );

     ASSERTION(index>=0 && index<256,"check!");
     ASSERTION(supported==aConf.supportedExtensions[ index ].supported,"check-2!");

     // Find MIME-type
     strMimeType = aConf.FindByFileType( (eFileType)index ).mimeType;

     printf("%s\tindex: %d\tsupported=%u [%s] %s",
	    sName.Str(),
	    index,
	    supported,
	    strMimeType,
	    ptrSup->extensions ? ": " : "\n");
     if ( ptrSup->extensions ) {
	 ptrSup->extensions->Show();
     }
 }
 return 0;
}


void dbg_dump_id3v1_genres (FILE* fOut, int mask)
{
 v1_dump_genres( fOut, mask );
}

////////////////////////////////////////////////////////////
int do_debug (int argc, char* argv[])
{
 int value( atoi( argv[ 1 ] ) );
 bool isValue( value!=0 || strcmp( argv[ 1 ], "0" )==0 );
 int error( 0 );

 if ( isValue==false ) {
     error = dbg_test( argv[ 1 ] );
 }

 printf("\nID3v1 MP3 genres (abbrev.)\n");
 dbg_dump_id3v1_genres( stdout, 0 );
 printf("\nID3v1 MP3 genres:\n");
 dbg_dump_id3v1_genres( stdout, 4 );
 return error;
}


int main (int argc, char* argv[])
{
 int error;

 gINIT;

 if ( argv[ 1 ]==NULL ) {
     usage();
     gEND;
     return 0;
 }

 if ( strcmp( argv[ 1 ], "ALL" )==0 ) {
     error = do_check_all();
 }
 else {
     error = do_debug( argc, argv );
 }

 gEND;

 DBGPRINT_MIN("DBG: Objs undeleted: %d\n",gStorageControl::Self().NumObjs());
 return error;
}
////////////////////////////////////////////////////////////

