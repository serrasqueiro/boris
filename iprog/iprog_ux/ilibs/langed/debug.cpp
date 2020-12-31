// debug.cpp, for liblanged

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib_iobjs.h"
#include "lib_imedia.h"
#include "lib_langed.h"

#include "wordunicode.h"


IMEDIA_DECLARE;

////////////////////////////////////////////////////////////
int usage_option (const char* strCommand)
{
 const char firstChr( strCommand ? strCommand[ 0 ] : '\0' );

 const char* helps[]={
     "debug LETTER\n\
\n\
a		Basic test\n\
\n\
b N		Get up to N words of each letter\n\
\n\
c N		Same as above, but output is buffered first\n\
\n\
d N		Display words, one by line.\n\
\n\
e 1 file [...]	Display named strings of file(s)\n\
\n\
f 0 file [...]	Display strings (including escaped chars) of file(s)\n\
",
     "a\n\
",
     "b N [file]\n\
\n\
E.g. N=10, get 10 words starting with 'a', 'b', ...etc.\n\
If N<0 there is no limit.\n\
\n\
Examples:\n\
	debug b 12 /usr/share/dict/words\n\
		(displays dictionary first 12 words of each letter)\n\
",
     "c xxx\n\
\n\
Similar to b, but uses 'InsertOrderedUnique'.\n\
\n\
",
     "d 0\n\
\n\
In the future other values than 0 can be used.\n\
\n\
",
     "e 1 file [file ...]\n\
\n\
Display named strings for file(s)\n\
",
     "f 0 file [file ...]\n\
\n\
Display named strings (with escaped chars marked with \\0xAB) for file(s)\n\
\n\
0xAB represents the ASCII hex.\n\
Here '\r' (CR, ASCII 13d, will be still seen.\n\
",
     nil,
     nil,
     nil
 };

 switch ( firstChr ) {
 case 'a':
 case 'b':
 case 'c':
 case 'd':
 case 'e':
     printf("debug %s\n",helps[ firstChr+1-'a' ]);
     break;

 default:
     printf("%s\n",helps[ 0 ]);
     break;
 }
 return 0;
}

////////////////////////////////////////////////////////////
int check_string (const char* str)
{
 printf("check_string(%s):\n",str);

 return 0;
}


int dbg_test (const char* str)
{
 int error( 0 );
 printf("%s%s%d.%d\n",
	str ? str : "\0",
	str ? "\t" : "\0",
	LIB_ILANGED_VERSION_MAJOR,
	LIB_ILANGED_VERSION_MINOR);
 return error!=0;
}


int dump_some_dictionary_words (FILE* fIn,
				FILE* fOut,
				int n,
				const char* strFile,
				gList* ptrListed)
{
 int error( 0 );
 char first;
 char buf[ 1024 ];
 const int bufSize( sizeof( buf ) );

 gInt counts[ 'z'+1 ];

 memset( buf, 0x0, bufSize );

 for ( ; fgets( buf, bufSize, fIn ); ) {
     first = buf[ 0 ];
     error = buf[ bufSize-1 ]!=0 || first==0;
     if ( error ) {
	 memset( buf, 0x0, bufSize );
     }
     else {
	 buf[ strlen( buf )-1 ] = 0;

	 if ( first>='A' && first<='Z' ) first += 32;
	 error = first<'a' || first>'z';

	 if ( error ) {
	     fprintf(stderr,"Invalid: %s\n",buf);
	 }
	 else {
	     counts[ first ].Incr();
	     if ( n<0 || counts[ first ].GetInt()<=n ) {
		 if ( fOut )
		     fprintf(fOut,"%s\n",buf);

		 if ( buf[ 0 ] ) {
		     if ( ptrListed ) {
			 gString* newStr( new gString( buf ) );
			 if ( ptrListed->InsertOrderedUnique( newStr )==-1 ) {
			     fprintf(stderr,"Not unique: %s\n",buf);
			     delete newStr;
			 }
		     }
		 }
	     }
	 }
     }
 }
 return error!=0;
}


int dbg_test_b (int n, const char* str)
{
 int error;
 FILE* fIn( stdin );
 FILE* fOut( stdout );
 const char* strFileIn( str );

 if ( strFileIn && strFileIn[ 0 ] ) {
     fIn = fopen( strFileIn, "rt" );
     if ( fIn==nil ) return 2;
 }

 error = dump_some_dictionary_words( fIn, fOut, n, str, nil );
 if ( fIn!=stdin ) fclose( fIn );

 return error;
}


int dbg_test_c (int n, const char* str)
{
 int error;
 FILE* fIn( stdin );
 const char* strFileIn( str );

 gList listed;

 if ( strFileIn && strFileIn[ 0 ] ) {
     fIn = fopen( strFileIn, "rt" );
     if ( fIn==nil ) return 2;
 }

 error = dump_some_dictionary_words( fIn, nil, n, str, &listed );
 if ( fIn!=stdin ) fclose( fIn );

 for (gElem* ptrElem=listed.StartPtr(); ptrElem; ptrElem=ptrElem->next) {
     printf("%s\n",ptrElem->Str());
 }

 return error;
}


int dbg_test_d (int n, const char* str)
{
 int error( 0 );
 int line( 0 );
 FILE* fIn( stdin );
 FILE* fOut( stdout );
 t_uchar buf[ 1024 ];
 const int bufSize( sizeof( buf ) );
 gElem* ptrElem;

 memset( buf, 0x0, bufSize );

 for ( ; fgets( (char*)buf, bufSize, fIn ); ) {
     gList words;

     line++;
     langed_split_words( buf, 0, words );

     for (ptrElem=words.StartPtr(); ptrElem; ptrElem=ptrElem->next) {
	 if ( n ) {
	     fprintf(fOut,"%d\t",line);
	 }
	 fprintf(fOut,"%s\n",ptrElem->Str());
     }
 }

 return error;
}


int dbg_test_e (int maskOne, char* const args[])
{
 const char* strFile;
 t_uchar* strLine;
 char* aStr;
 int iter( 1 );
 FILE* fIn;
 gString* newStr;
 t_uchar buf[ 1024 ];

 for ( ; (strFile = args[ iter ])!=nil; iter++) {
     fIn = fopen( strFile, "rt" );
     if ( fIn ) {
	 for ( ; fgets( (char*)buf, sizeof(buf)-1, fIn ); ) {
	     if ( maskOne )
		 strLine = ulang_new_7bit_string( buf, nil );
	     else
		 strLine = ulang_new_8bit_eqstr( buf, 3, nil );
	     //strLine = ulang_new_8bit_string( buf, nil );

	     aStr = (char*)strLine;
	     newStr = (gString*)gStorageControl::Self().Pool().EndPtr()->me;
	     DBGPRINT("\nLength: %d = %u:\n%s%s",
		      strlen( aStr ),
		      newStr->Length(),
		      buf,
		      (buf[ 0 ] && buf[ strlen( aStr )-1 ]=='\n') ? "\0" : "\n");
	     printf("%s",strLine);
	 }
	 fclose( fIn );
     }
     else {
	 fprintf(stderr,"Uops: %s\n",strFile);
     }
 }
 return 0;
}


int dbg_test_f (char* const args[])
{
 return dbg_test_e( 0, args );
}


int do_debug (const char* strCommand, char* const args[])
{
 int error( 0 );

 if ( strcmp( strCommand, "lib" )==0 ) {
     printf("LIB_ILANGED_VERSION_MAJOR.MINOR %d.%d\n",
	    LIB_ILANGED_VERSION_MAJOR,
	    LIB_ILANGED_VERSION_MINOR);
     return 0;
 }

 switch ( strCommand[ 0 ] ) {
 case 'a':
     error = dbg_test( args[ 1 ] );
     break;

 case 'b':
     if ( args[ 1 ]==nil )
	 return usage_option( nil );
     error = dbg_test_b( atoi( args[ 1 ] ), args[ 2 ] );
     break;

 case 'c':
     if ( args[ 1 ]==nil )
	 return usage_option( nil );
     error = dbg_test_c( atoi( args[ 1 ] ), args[ 2 ] );
     break;

 case 'd':
     if ( args[ 1 ]==nil )
	 return usage_option( nil );
     error = dbg_test_d( atoi( args[ 1 ] ), args[ 2 ] );
     break;

 case 'e':
     if ( args[ 1 ]==nil )
	 return usage_option( nil );
     error = dbg_test_e( atoi( args[ 1 ] ), args+1 );
     break;

 case 'f':
     if ( args[ 1 ]==nil )
	 return usage_option( nil );
     error = dbg_test_f( args+1 );
     break;

 default:
     return usage_option( nil );
 }
 return error;
}


int main (int argc, char* argv[])
{
 int error;

 gINIT;

 if ( argc<2 || (argv[ 2 ] && strcmp( argv[ 2 ], "-h" )==0) ) {
     gEND;
     return usage_option( argv[ 1 ] );
 }

 // Unicode, make hashing
 IMEDIA_INIT;

 langed_init();

 error = do_debug( argv[ 1 ], argv+1 );
 printf("do_debug (%s) returned %d\n",
	argv[ 1 ],
	error);

 langed_finit();

 printf("DBG: Objs: %d (STILL)\n",gStorageControl::Self().NumObjs());

 // Unicode, data release
 imb_iso_release();

 printf("DBG: Objs: %d\n",gStorageControl::Self().NumObjs());
 gEND;
 printf("DBG: Objs undeleted: %d\n",gStorageControl::Self().NumObjs());

 return error;
}
////////////////////////////////////////////////////////////

