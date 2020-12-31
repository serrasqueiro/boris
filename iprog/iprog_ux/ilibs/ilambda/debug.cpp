// debug.cpp, for libilambda

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib_iobjs.h"

#include "lib_ilambda.h"
#include "icrc32.h"


#ifdef ILAMBDA_UINT20
#else
typedef t_uint32 t_uint20;
#endif

#define YOU_NEED_20BITS		t_uint20


#define STR_SAMPLE_DBA_FIELDS	"{jday:uint32; minute-tracker:uint16} sent:uint64; recv:uint64; sent-tcp:uint64; recv-uint64:uint64"

////////////////////////////////////////////////////////////
int dbg_parse_json (const char* strFile)
{
 int code;
 iJSON* son( nil );
 gElem* ptrElem;

 FILE* fIn( fopen( strFile, "rt" ) );

 if ( fIn==nil ) return 2;

 son = ijson_get_from_handle( fileno( fIn ) );
 ASSERTION(son,"son");
 code = son->iValue;

 fclose( fIn );
 fprintf(stderr,"parse_json returned %d\n",code);

 ptrElem = son->hints.StartPtr();
 if ( ptrElem ) {
     for ( ; ptrElem; ptrElem=ptrElem->next) {
	 fprintf(stderr,"Hint %d\t:\t%s\n",
		 ptrElem->me->iValue,
		 ptrElem->Str());
     }
 }
 else {
     fprintf(stderr,"No errors/warns\n");
 }

 delete son;
 return code;
}


int dbg_calc_crc32 (const char* strFile)
{
 int error;
 FILE* fIn( fopen( strFile, "rb" ) );
 iCRC32* calc;

 if ( fIn==nil ) return 2;

 calc = crc32_from_handle( fileno( fIn ) );
 error = calc->lastOpError;
 fclose( fIn );

 printf("CRC32: %08X\n",calc->iValue);
 delete calc;
 return error;
}

////////////////////////////////////////////////////////////
t_uint20 dbg_test_string_hash (const char* strIn)
{
 gString sName( (char*)strIn );
 t_uint20 hashMillion;

 printf("dbg_test_string_hash: {%s}\n\
>>>>\n",strIn);

 hashMillion = (unsigned)sName.Hash() % PRIME_MILLION_NP0;

 printf("hashMillion: %06u (ok, original was %d)\n",
	hashMillion,
	sName.Hash());

 printf("<<<<\n\n");

 return hashMillion;
}

////////////////////////////////////////////////////////////
int dbg_test_hash ()
{
 const int result( 0 );

 printf("dbg_test_hash:\n\
>>>>\n");

 printf("EMPTY: no test!\n");

 printf("<<<<\n\n");
 return result;
}


int do_debug (int argc, char* argv[])
{
 int error( 0 );

 dbg_test_hash();

 dbg_test_string_hash( "Henrique" );

 error = dbg_test_string_hash( "." ) != 77496;
 ASSERTION(error==0,"Million-hash of a dot (.) is 77496");

 dbg_test_string_hash( argv[ 1 ] );

 if ( argv[ 1 ] ) {
     const char* secondArg( (argv[ 2 ] && strcmp(argv[ 2 ],".")==0) ? STR_SAMPLE_DBA_FIELDS : argv[ 2 ]);

     // Run json parse if first arg is somehow 'json':
     if ( strncmp( argv[ 1 ], "json", 3 )==0 ) {
	 error = dbg_parse_json( argv[ 2 ] );
	 printf("dbg_parse_json returned %d\n",error);
     }

     error = dbg_calc_crc32( argv[ 1 ] );
     printf("dbg_calc_32(%s) returned %d\n",argv[ 1 ],error);

     // About dba:
     // -> a really simple database

     DBA aDb;

     gList* fields( dba_fields_from_string( secondArg ) );
     int iter( 0 );

     for (gElem* p=fields->StartPtr(); p; p=p->next) {
	 iter++;
	 int brackets( p->iValue );
	 gList* pField( (gList*)p->me );

	 printf("(#%u)\t", pField->N());
	 printf("field %d\t%d  %s%s:%s%s",
		iter,
		p->iValue,
		brackets ? "{" : "(",
		pField->Str( 1 ), pField->Str( 2 ),
		brackets ? "}" : ")");
	 if ( pField->N()>2 ) {
	     int dot( pField->GetObjectPtr( 3 )->iValue );

	     printf(" [%s].%d",
		    pField->Str( 3 ),
		    dot);
	 }
	 printf("\n");
     }

     iter = 3;
     if ( argv[ 2 ] && (argv+iter)!=nil ) {
	 for ( ; iter<argc; iter++) {
	     aDb.AddConfigLine( argv[ iter ] );
	 }
     }

     // Dump 'aDb' configurations
     printf("\nconfigs:\n");

     gList* pConfig( aDb.Config( nil ) );
     int confIter( 0 );
     for (gElem* pIter=pConfig->StartPtr(); pIter; pIter=pIter->next) {
	 gList* pLine( (gList*)pIter->me );
	 confIter++;
	 printf("conf %d:\t", confIter); pLine->Show();
     }

     aDb.InitDB( argv[ 1 ] );

     delete fields;
 }
 else {
     // No arg

     const char* strFile( "debug.cpp" );
     t_uint16 checksum( 0x1234 );

     error = file_sumA( strFile, 0, checksum );

     if ( error ) {
	 perror( strFile );
	 fprintf(stderr, "Bogus, error code: %d;\n\
\n\
This is a testing program, debug.cpp is at iprog_ux/ilibs/ilambda/...\n", error);
     }
     else {
	 printf("A %05u\t%s\n", checksum, strFile);
     }
 }

 return error!=0;
}


int main (int argc, char* argv[])
{
 int error;

 gINIT;
 error = do_debug( argc, argv );
 DBGPRINT("DBG: Objs undeleted: %d\n",gStorageControl::Self().NumObjs());
 gEND;

 printf("Returning %d\n",error);
}
////////////////////////////////////////////////////////////

