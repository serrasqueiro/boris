// debug.cpp, for libcysoft

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib_iobjs.h"

#include "lib_cysoft.h"
////////////////////////////////////////////////////////////
void dbg_test_bases (const char* strIn)
{
 const char* str( strcmp( strIn, "Abc" ) ? strIn : "123" );
 int value( atoi( str ) );
 int result;
 printf("dbg_test_bases: {%s}\n\
>>>>\n",str);

 gMBase52 nam;
 result = nam.FromInt( value );

 printf("nam.FromInt( %d ), result=%d, {%s}\n",
	value,
	result,
	nam.Str());
 printf("ShowAlpha: {%s}\n",
	nam.ShowAlpha());

 printf("<<<<\n\n");
}


int do_run_hashes (const char* strDictionary)
{
 char buf[ 1024 ];
 char* strAlpha;
 int len;
 int error( -1 );
 int hashed;
 bool isStdin( strDictionary==nil );
 gList ordered;
 gList* newObj;

 FILE* fIn( isStdin ? stdin : fopen( strDictionary, "rt" ) );
 FILE* fOut( stdout );

 if ( fIn==nil ) return 2;

 for ( ; fgets( buf, sizeof(buf), fIn ); ) {
     if ( buf[ 0 ]==0 ) break;
     if ( buf[ (len = strlen( buf ))-1 ]=='\n' ) {
	 buf[ --len ] = 0;
     }

     gString word( buf );
     gMBase52 alpha;

#ifdef DEBUG_WITH_SMALL_HASH
     hashed = (int)((unsigned)word.Hash() % 13);
#else
     hashed = word.Hash();  // This is the normal case: 32bit hash
#endif

     fprintf(fOut,"%d\t%s\n",
	     hashed,
	     buf);
     alpha.FromInt( hashed );
     strAlpha = alpha.ShowAlpha();

     newObj = new gList;
     ASSERTION(newObj,"newObj");
     newObj->Add( strAlpha );
     newObj->Add( buf );

     newObj->iValue = hashed;
     error = ordered.InsertOrderedUnique( newObj );

     CYS_NOT_SHOW("-> %d\t{%s}\n\n",error,newObj->Str());

     if ( error==-1 ) {
	 gElem* ptrCurrent( ordered.CurrentPtr() );
	 if ( strcmp( ptrCurrent->me->Str( 2 ), buf ) ) {
	     fprintf(stderr,"Two with same hash (%d): {%s}, {%s}\n",
		     newObj->iValue,
		     ptrCurrent->Str( 2 ),
		     buf);
	     ordered.InsertOrdered( newObj );
	 }
	 else {
	     delete newObj;
	 }
     }
 }

 if ( isStdin==false ) fclose( fIn );

 // Dump ordered list (no repetitions):
 gElem* ptrElem( ordered.StartPtr() );

 for ( ; ptrElem; ptrElem=ptrElem->next) {
     strAlpha = ptrElem->Str();

     printf("::: %s\t%d: %s\n",
	    strAlpha,
	    ptrElem->me->iValue,
	    ptrElem->me->Str( 2 ));
 }

 return 0;
}


void dbg_test_tod (const char* strIn)
{
 char* newStrDup( nil );
 char* str( strIn==NULL ? (newStrDup = strdup( "2010-12-30 13:47:59 End queue" )) : (char*)strIn );
 gDateTime see;
 unsigned pos( 0 );
 unsigned long aStamp( 0 );
 bool got;

 printf(">>>>\n");
 printf("TOD is {%s}\n\n",str);

 got = see.FromStringTOD( str, pos );
 aStamp = (unsigned long)see.GetTimeStamp();

 printf("{%s}: Ok? %c, date makes sense? %c, pos=%u\n",
	str,
	ISyORn( got ),
	ISyORn( aStamp!=0 ),
	pos);
 printf("date: {%s}, time: {%s},\nrest: {%s}\n\n",
	str,
	str+11,
	str+pos);
 printf("Stamp: %lu\n",aStamp);
 printf("%04u-%02u-%02u %02u:%02u:%02u\n",
	see.year, see.month, see.day,
	see.hour, see.minu, see.sec);
 printf("<<<<\n\n");

 // Free previously allocated memory
 if ( newStrDup ) {
     free( newStrDup );
 }
}

////////////////////////////////////////////////////////////
int dbg_test_hash ()
{
 const char strOne[ 2 ]="\x1";
 char buf[ 16 ];
 int iter( 0 ), idxArray( 0 );
 long hashValue;

 const long pyHashLetter[]={
     -468864544,  // "a"
     -340864157,  // "b"
     -212863774,  // "c"
     790306557,   // "henrique"
     -1
 };

 printf(">>>>\n");

 printf("0x01, 0x%x, hash:\t%ld\n",
	strOne[0],
	cys_string_hash( strOne ));
 printf("...\n");

 for ( ; iter<=(int)'c'; iter++) {
     buf[ 0 ] = iter;
     buf[ 1 ] = 0;
     hashValue = cys_str_hash( buf, 1 );

     printf("%dd\t%ld\t",
	    buf[ 0 ],
	    hashValue);
     if ( iter>='a' ) {
	 printf("\t%ld %s",
		pyHashLetter[ idxArray ],
		hashValue==pyHashLetter[ idxArray ] ? "OK" : "Fail");
	 idxArray++;
     }
     printf("\n");

     if ( iter==7 ) iter = (int)'a'-1;
 }

 strcpy( buf, "henrique" );
 hashValue = cys_str_hash( buf, strlen( buf ) );
 printf("{%s}\t%ld\t",
	buf,
	hashValue);
 printf("\t%ld\n",pyHashLetter[ idxArray ]);

 printf("<<<<\n\n");
 return 0;
}


int dbg_test_one_base (const char* aStr)
{
 gMBase52 nam;
 const char* outStr( nam.FromTextStr( (char*)aStr ) );

 printf("%s\n", outStr);
 return 0;
}


int do_debug (int argc, char* argv[])
{
 int error( 0 );
 int thisError;
 int iter( 1 ), nMax( argc-1 );
 int value;
 char* str;
 char* longStr( nil );
 char* reversedStr( nil );
 char** ptrNewArgv( nil );
 bool isOk;
 long hashValue;

 if ( nMax>=1 ) {
     if ( strcmp( argv[ 1 ], "HASH" )==0 ) {
	 return do_run_hashes( argv[ 2 ] );
     }
 }

 if ( nMax<=0 ) {
     nMax = 4;
     ptrNewArgv = argv = (char**)malloc( sizeof(char*) * (nMax+1) );
     argv[ 1 ] = strdup( "Abc" );
     argv[ 2 ] = strdup( "EMPTY" );
     argv[ 3 ] = strdup( "NULL" );
     argv[ 4 ] = strdup( "Done" );
 }

 for ( ; iter<=nMax; iter++) {
     gMBase52 a;

     str = argv[ iter ];

     PyStringObject py( str );
     gString sTestHash( str );
     sTestHash.iValue = -1;  // Force hash

     hashValue = py.Hash();
     printf("%ld\tHash('%s')%s\n",
	    hashValue,
	    str,
	    cys_do_hash( sTestHash )==hashValue ? "\0" : "\tUops!!!");
     ASSERTION(hashValue==sTestHash.iValue,"hashValue");
#ifdef DEBUG_OCTAL
     if ( atol( str ) ) {
	 hashValue = atol( str );
     }
     printf("DBG: %ld (%lu)\t0x %lx\n",
	    hashValue,
	    (unsigned long)hashValue,
	    hashValue);
#endif

     if ( strcmp( str, "EMPTY" )==0 ) {
	 str[ 0 ] = 0;
     }
     else {
	 if ( strcmp( str, "NULL" )==0 ) {
	     str = nil;
	 }
     }
     printf("\
+++++++++++++++++\n%s\n\
+++++++++++++++++\n",
	    str);
     printf("{%s} {%s}\n",
	    str,
	    longStr = a.FromTextStr( str ));

     gMBase52 b;
     gString inverse( reversedStr = b.ToTextStr( longStr ) );
     if ( str ) {
	 isOk = inverse.Match( str );
     }
     else {
	 isOk = reversedStr==nil;
     }

     a.Set( str );
     value = a.ToInt();
     b.FromInt( value );
     printf("{%s} - valid base 52? %c\n",
	    str,
	    ISyORn( a.IsOk()) );
     printf("Base52 to Int is:\t%d, from: {%s|%s} %d|%d\n",
	    value,
	    a.Str(),
	    b.Str(),
	    a.iValue,
	    b.iValue);

     // ToTextStr...
     printf("%s\n",isOk ? "\0" : "ERROR!\n");
     error += (isOk==false);
 }
 if ( error ) {
     fprintf(stderr,"\nUops: has %d error(s)\n",error);
 }

 const char *strTest( "a" );
 const long pythonHashA( -468864544L );
 PyStringObject py( (char*)strTest );
 gString someWord( (char*)strTest );

 hashValue = py.ForceHash();
 thisError = hashValue!=pythonHashA;

 printf("Hash for 'a': %ld (expected: %ld) %s\n",
	hashValue,
	pythonHashA,
	thisError ? "ERROR: different from Python hash(\"a\")\n" : "OK!");

 hashValue = (int)someWord.Hash();
 printf("Another way to calc Hash of '%s': %ld = %d = %d\n",
	strTest,
	hashValue,
	someWord.StringHash( strTest ),
	someWord.StringHash( strTest, strlen( strTest  )));

 dbg_test_hash();

 dbg_test_bases( argv[1] );

 printf("+++++++++++++++++\n");
 dbg_test_one_base( "Henrique" );
 printf("+++++++++++++++++ ___\n");

 dbg_test_tod( strcmp( argv[1], "LOG" )==0 ? argv[2] : nil );

 // Free previously allocated memory

 if ( ptrNewArgv ) {
     for (iter=1; iter<=nMax; iter++) {
	 free( argv[ iter ] );
     }
     free( ptrNewArgv );
 }

 return error!=0;
}


int main (int argc, char* argv[])
{
 int error;

 gINIT;
 error = do_debug( argc, argv );
 CYS_SHOW_MEM("DBG: Objs undeleted: %d\n",gStorageControl::Self().NumObjs());
 gEND;

 printf("Returning %d\n",error);
 return error;
}
////////////////////////////////////////////////////////////

