// debug.cpp, for libiobjs

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib_iobjs.h"


// COMPILE
//	make debug
//
// USAGE:
//	debug [-|hash] [ARGS]
//
// Examples:
//	debug aString
//		-> dumps hash of aString
//	debug - Abc
//		-> dumps the unsigned hash of Abc
//
//	debug hash file1 file2
//		-> dumps line hashes of file1 and file2
//

////////////////////////////////////////////////////////////
int file_show_hash (FILE* fIn, FILE* fOut)
{
 gString s;
 char chr;

 ASSERTION(fIn,"fIn");
 ASSERTION(fOut,"fOut");

 for ( ; fscanf(fIn,"%c",&chr)==1; ) {
     if ( chr=='\n' ) {
	 fprintf(fOut,"%u\t%s\n",
		 s.Hash(),
		 s.Str());
	 s.Reset();
     }
     else {
	 if ( chr!='\r' ) {
	     s.Add( chr );
	 }
     }
 }

 if ( s.Length() ) {
     fprintf(fOut,"%u\t%s\n",
	     s.Hash(),
	     s.Str());
 }
 return 0;
}


int dbg_test (const char* strIn)
{
 int error;
 gString sNew( (char*)strIn );
 bool isOk( sNew.IsOk() );

 error = isOk==false;

 printf("sNew.Str()='%s' (strIn %s), iValue: %d\n",
	sNew.Str(),
	strIn ? (isOk ? "OK" : "uops!") : "nil",
	sNew.Hash());

 return error;
}


int dbg_test_unsigned (const char* strIn, unsigned modulus)
{
 gString sNew( (char*)strIn );

 if ( modulus ) {
     printf("Hash value (modulus %u%s, next prime: %u): %u\n",
	    modulus,
	    ibase_Prime( modulus ) ? " - PRIME NUMBER" : "",
	    ibase_NextPrime( modulus ),
	    (unsigned)sNew.Rehash() % modulus);
 }
 else {
     printf("Hash value: %u\n",(unsigned)sNew.Rehash());
 }
 return 0;
}


int dbg_hash (char* args[])
{
 int error( 0 );
 const char* str;
 FILE* fIn( stdin );
 FILE* fOut( stdout );

 if ( args==nil || args[ 0 ]==nil || args[ 1 ]==nil ) {
     file_show_hash( fIn, fOut );
 }
 else {
     for (args++; (str = *args)!=nil; args++) {
	 fIn = fopen( str, "rt" );
	 if ( fIn ) {
	     file_show_hash( fIn, fOut );
	     fclose( fIn );
	 }
	 else {
	     fprintf(stderr,"Uops: %s\n",str);
	 }
     }
 }
 return error;
}


int dbg_tidy (int nAllArgs, char* args[])
{
 int idx( 1 ), nArgs( nAllArgs-1 );
 int val;
 char* strArg;
 gList num;
 gElem* pElem;

 for ( ; idx<=nArgs; idx++) {
     strArg = args[ idx ];
     val = atoi( strArg );
     num.Add( strArg );
     num.EndPtr()->me->iValue = val;
 }

 num.Tidy( 0 );

 for (pElem=num.StartPtr(); pElem; pElem=pElem->next) {
     printf("  %s", pElem->Str());
 }
 printf("\n");
 printf("\nBackwards:%s\n", num.N() ? "" : " {{LIST IS EMPTY!!!}}");
 for (pElem=num.EndPtr(); pElem; pElem=pElem->prev) {
     printf("  %s", pElem->Str());
 }
 printf("\n");
 return 0;
}



void dbg_register_test (bool cleanDescriptions)
{
 int index( 0 );
 gString sName( "AnyName" );
 gString sAnonymous( "AnyAnonymous" );
 gString sNada( "nada" );
 gString* four( new gString( "four" ) );
 gString* eight( new gString( "eight" ) );

 gStorageControl::Self().RegisterDescriptionStr( sName, "Henry VIII - Your Magesty" );
 gStorageControl::Self().RegisterDescriptionStr( sAnonymous, "John Doe" );
 gStorageControl::Self().RegisterDescriptionStr( *four, "The fourth king" );
 printf("sName	%s, {%s}\n", sName.Str(), sName.DescriptionStr());
 printf("sAnon.	%s, {%s}\n", sAnonymous.Str(), sAnonymous.DescriptionStr());
 printf("sNada	%s, {%s}\n", sNada.Str(), sNada.DescriptionStr());
 printf("eight	%s, {%s}\n", eight->Str(), eight->DescriptionStr());

 delete four;
 delete eight;
 printf("# decriptions #\n"); gStorageControl::Self().DescriptionList().Show();

 printf("### Verbose view ###\n");
 for (gElem* pDesc=gStorageControl::Self().DescriptionList().StartPtr(); pDesc; pDesc=pDesc->next) {
     gList* pList( (gList*)pDesc->me );
     index++;
     printf("%4d: [description list i=%d] %14p ",
	    index,
	    pList->iValue,
	    pList->ValidLRef());
     for (gElem* pBlock=pList->StartPtr(); pBlock; pBlock=pBlock->next) {
	 printf("  %d@[%d.%s]\n",
		pBlock->iValue,
		pBlock->me->iValue, pBlock->me->Str());
     }
 }
 printf("###\n\n");

 gStorageControl::Self().TidyDescriptions();

 printf("### Verbose INVERSE view ###\n");
 for (gElem* pDesc=gStorageControl::Self().DescriptionList().EndPtr(); pDesc; pDesc=pDesc->prev) {
     gList* pList( (gList*)pDesc->me );
     index++;
     printf("%4d: [i=%d] %14p ",
	    index,
	    pList->iValue,
	    pList->ValidLRef());
     for (gElem* pBlock=pList->StartPtr(); pBlock; pBlock=pBlock->next) {
	 printf("  %d@[%d.%s]\n",
		pBlock->iValue,
		pBlock->me->iValue, pBlock->me->Str());
     }
 }
 printf("###\n\n");

 // User does not have to clean-up descriptions,
 // but we do it here to avoid displaying a wrong number of 'Objs undeleted'
 if ( cleanDescriptions ) {
     gStorageControl::Self().DescriptionList().Delete();
 }
}


int do_debug (int argc, char* argv[])
{
 int error;
 int line( 0 );
 const char* strArg1( argv[ 1 ] );

 if ( strArg1 ) {
     if ( strcmp( strArg1, "hash" )==0 ) {
	 return dbg_hash( argv+1 );
     }

     if ( strcmp( strArg1, "tidy" )==0 ) {
	 return dbg_tidy( argc-1, argv+1 );
     }

     if ( strcmp( strArg1, "read" )==0 ) {
	 gFileFetch input( argv[ 2 ] );
	 error = input.lastOpError;

	 fprintf(stderr, "%s: lastOpError=%d\n",
		 argv[ 2 ],
		 error);

	 for (gElem* ptr=input.aL.StartPtr(); ptr; ptr=ptr->next) {
	     line++;
	     printf("%d: %d:%d %s\n",
		    line,
		    ptr->iValue, ptr->me->iValue,
		    ptr->Str());
	 }
	 return error;
     }
 }

 if ( strArg1 && strArg1[ 0 ]=='-' ) {
     error = dbg_test_unsigned( argv[ 2 ], -atoi( strArg1 ) );
 }
 else {
     error = dbg_test( strArg1 );

     printf("gFileControl::Self().userId: %d, tmpPath: {%s}\n",
	    gFileControl::Self().userId,
	    gFileControl::Self().tmpPath);

     printf("---\n");
     dbg_register_test( false );

     printf("---\n");
     fprintf(stderr,"Error: %d\n",error);
 }

 return error!=0;
}


int main (int argc, char* argv[])
{
 int error;
 unsigned nDesc;

 gINIT;

 DBGPRINT("DBG: Objs: %d\n",gStorageControl::Self().NumObjs());
 error = do_debug( argc, argv );

 nDesc = gStorageControl::Self().DescriptionList().N();
 printf("Objs undeleted: %d; Pool: #%u; descriptions: #%u (objs: %u)\n",
	gStorageControl::Self().NumObjs(),
	gStorageControl::Self().Pool().N(),
	nDesc, nDesc * 5);

 gEND;

 return error;
}
////////////////////////////////////////////////////////////

