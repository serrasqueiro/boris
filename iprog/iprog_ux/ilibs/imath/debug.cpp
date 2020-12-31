// debug.cpp, for libimath

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib_imath.h"


// Forward declaration

int dbg_test_all () ;

////////////////////////////////////////////////////////////
void usage ()
{
 printf("debug TEST_TYPE [value ...]\n\
\n\
TEST_TYPEs are:\n\
	a	ALL tests\n\
	!	factorial of value ...\n\
	P	permutation\n\
	C	combination (i.e. combinations without repetition)\n\
\n");
}

////////////////////////////////////////////////////////////
int dbg_test_factorial (const char* strNum)
{
 int value( im_atoi( strNum ) );
 int z( -1 );
 int result( 0 );
 double real;

 printf("strNum: '%s', value considered: %d\n", strNum, value);
 printf("%d! = %d\n", value, z = im_factorial( value ));
 result = z==-1;
 printf("%s\n", result ? "ERROR" : "OK");

 real = im_factorial_d( (double)value );
 printf("Now the double value: %0.1f%s\n\n",
	real,
	real>IMATH_LONG_TOO_LONG_D ? " (BIG)" : "");

 return result!=0;
}


int dbg_test_permutation (const char* strNum1, const char* strNum2)
{
 int n( im_atoi( strNum1 ) );
 int r( im_atoi( strNum2 ) );
 int z( -1 );
 int result( 0 );
 double real;

 z = im_permutation( n, r );
 printf("%d P %d = %d! / (%d - %d)! = %d\n",
	n, r,
	n, n, r,
	z);
 result = z==-1;
 printf("%s\n", result ? "ERROR" : "OK");

 real = im_permutation_d( (double)n, (double)r );
 printf("Now the double value: %0.1f%s\n\n",
	real,
	real>IMATH_LONG_TOO_LONG_D ? " (BIG)" : "");

 return result!=0;
}


int dbg_test_combination (const char* strNum1, const char* strNum2)
{
 int n( im_atoi( strNum1 ) );
 int r( im_atoi( strNum2 ) );
 int z( -1 );
 int result( 0 );
 double real;

 z = im_combination( n, r );
 printf("%d C %d = %d! / (%d - %d)! = %d\n",
	n, r,
	n, n, r,
	z);
 result = z==-1;
 printf("%s\n", result ? "ERROR" : "OK");

 real = im_combination_d( (double)n, (double)r );
 printf("Now the double value: %0.1f%s\n\n",
	real,
	real>IMATH_LONG_TOO_LONG_D ? " (BIG)" : "");

 return result!=0;
}


int do_debug (int nArgs, char* args[])
{
 int result( 0 );
 char* strArg;

 if ( nArgs==0 ) {
     usage();
     return 0;
 }

 switch ( args[ 1 ][ 0 ] ) {
 case 'a':
 case 'A':
     result = dbg_test_all();
     break;

 case '!':
     strArg = args[ 2 ];
     if ( strArg ) {
	 result = dbg_test_factorial( strArg );
	 strArg = args[ 3 ];
	 if ( strArg ) {
	     fprintf(stderr, "Ignored arg: %s!\n", strArg);
	 }
     }
     break;

 case 'p':
 case 'P':
     strArg = args[ 2 ];
     result = dbg_test_permutation( strArg, strArg ? args[ 3 ] : nil );
     break;

 case 'c':
 case 'C':
     strArg = args[ 2 ];
     result = dbg_test_combination( strArg, strArg ? args[ 3 ] : nil );
     break;

 default:
     usage();
     return 0;
 }
 return result;
}

////////////////////////////////////////////////////////////
int dbg_test_all ()
{
 int result;
 int value;
 int value2;
 int possibleCases;
 unsigned call;

 dbg_test_factorial( "3" );
 result = dbg_test_factorial( "4" );

 dbg_test_permutation( "11", "2" );
 printf("11! is: %d\n\n", im_factorial( 11 ));

 printf("NOW COMBINATIONS (without repetition):\n\n");
 dbg_test_combination( "16", "3" );
 dbg_test_combination( "16", "13" );

 value = im_combination( 16, 3 );
 result = value==-1;  // this is kept as is, simple error result on this test!
 if ( value==im_combination( 16, 13 ) ) {
     printf("C(16,3) = C(16,13) = %d%s\n", value, value==-1 ? " (UOPS)" : "");
 }

 printf("+++\n");
 value = im_combination( 49, 6 );
 printf("TOTOLOTO:\n\tCasos possiveis [possible outcomes]:\n\
		C(49,6) = %d\t%s\n",
	value,
	(long)value==13983816L ? "OK" : "Bogus!");

 printf("+++\n");
 value = im_power( 3, 13 );
 printf("TOTOBOLA, 1X2, 3 hipoteses vezes 13 jogos:\n\tCasos possiveis [possible outcomes]:\n\
		3^13 = %d\t%s\n",
	value,
	(long)value==1594323L ? "OK" : "Bogus!");

 printf("+++\n");
 value = im_combination( 50, 5 );  // 5 numeros
 value2 = im_combination( 11, 2 );  // 2 estrelas
 possibleCases = value * value2;

 printf("EUROMILHOES: 5 numeros (de 1 a 50) e 2 estrelas (de 1 a 11)\n\
		numeros apenas:     %d,\n\
		estrelas apenas:    %d,\n\
		casos possiveis:    %d\t%s\n",
	value,
	value2,
	possibleCases, 116531800==possibleCases ? "OK" : "Bogus!");

 printf("+++\n");
 printf("Powers:\n\
		2^3      = %0.3f\n\
		(-2)^3   = %0.3f\n\
		2^-3     = %0.3f\n\
		2^0.5    = %0.3f (UNSUPPORTED)\n\
",
	im_power_d( 2, 3 ),
	im_power_d( -2, 3 ),
	im_power_d( 2, -3 ),
	im_power_d( 2, 1.0/2.0 ));

 // Now testing iSet class
 printf("+++\n");
 const char* strInit( "14 37 30 13 23 + 8 2" );
 iSet newSet( strInit );
 gElem* p;
 gElem* next;

 printf("IsValidIndex( 0 ): %d, and 1? %d\n", newSet.IsValidIndex( 0 ), newSet.IsValidIndex( 1 ));
 printf("MinimumStorageSize(): %d\n", newSet.MinimumStorageSize());
 printf("MaximumStorageSize(): %d\n", newSet.MaximumStorageSize());
 printf("strInit: %s\n", strInit);

 for (p=newSet.StartPtr(); p; p=next) {
     next = p->next;
     // NOTE:
     //		-> in this implementation the values are stored at iValue
     //		   and not as an gInt storage object,
     //		   so you will never see "NUM_" there.
     printf("{%s%s%s}%s",
	    p->me->Kind()==gStorage::e_String ? "" : "NUM_",
	    p->me->iValue==I_NUM_INVALID ? "'" : "",
	    p->me->Str(),
	    next ? " " : "\n");
 }
 printf("iSet newSet.lastOpError=%d\n\n", newSet.lastOpError);

 printf("+++\n");
 iSet b( newSet );
 b.Show();
 printf("b.Add( 'nineth' );\n");
 b.Add( "nineth" );
 gString sTenth( "tenth" ); b.Add( sTenth );
 b.Show();

 printf("b.Delete( 9, 9 );\n");
 b.Delete( 9, 9 );
 b.Show();

 value = b.lastOpError;

 call = b.Add( 14 );
 printf("b.Add( 14 ) -- %s,\n	AllowedRepeats()? %c\n\n",
	call ? "ADDED; Bogus!" : "Not added, it exists already!, OK",
	ISyORn( b.AllowedRepeats() ));
 b.Show();

 printf("b.Add( 99 )\n");
 b.Add( 99 );
 b.Show();

 printf("b.lastOpError was: %d, now is: %d\n",
	value,
	b.lastOpError);

 printf("+++\n");
 iSet c;
 c.AllowRepeated( true );
 c = b;
 c.Show();

#if 1
 for (p=c.StartPtr(); p; p=p->next) {
     printf("TYPE:%d%s elem.iValue=%d, %13d '%s'\n",
	    p->me->Kind(), p->me->Kind()==gStorage::e_String ? "(e_String)" : "",
	    p->iValue,
	    p->me->iValue,
	    p->Str());
 }
 int dummy( 0 );
 for (p=c.StartPtr(); p; p=p->next) {
     dummy++;
     printf("\t%13d '%s' length(): %d\n",
	    p->me->iValue,
	    p->Str(),
	    p->me->Length());
 }
#endif

 call = c.Add( 14 );
 printf("'c' is a LIST, repetitions allowed, adding value 14 -- %s\n",
	call ? "ADDED" : "NOT added; Bogus!");
 c.Show();

 printf("+++\n");
 iSet d;
 d = c;
 DBGPRINT("DBG: Number of duplicates: d.lastOpError=%d\n", d.lastOpError);
 printf("'d' is a SET, no repetitions allowed; copying from 'c'\n");
 d.Show();

 gElem* keepLast( nil );
 const char* str1;
 const char* str2;
 int pos( 0 );

 p=d.StartPtr();
 for ( ; p; p=next) {
     int compared;
     pos++;
     next = p->next;
     str1 = p->Str();
     if ( next ) {
	 str2 = next->Str();
	 compared = d.CompareStrs( (char*)str1, (char*)str2 );
	 printf("\t#%-3d vs #%-3d ivalue:%4d  %s\t%s\t%d, %d\n",
		pos, pos+1,
		p->me->iValue==I_NUM_INVALID ? -999 : p->me->iValue,
		str1,
		str2,
		compared,
		p->me->CompareStr( (char*)str2 ));
     }

     keepLast = p;
 }

 ASSERTION(keepLast,"keepLast");

 printf("+++\n");
 for (p=keepLast; p; p=p->prev) {
     next = p->next;
     str1 = p->Str();
     printf("\t#%-3d  ivalue:%4d  %s\n",
	    pos,
	    p->me->iValue==I_NUM_INVALID ? -999 : p->me->iValue,
	    str1);
     pos--;
 }

 printf("+++\n");
 iSet ordered( d );
#if TEST_DESCENDING_SORT
 ordered.SetOrder( iSet::e_descending );
#endif
 ordered.Show();

 iSet* ptrOrdered;
 ptrOrdered = ordered.NewOrderedSet();
 printf("UN_ORDERED: "); ordered.Show();
 ptrOrdered->Show();
 printf("ORDERED___: "); ptrOrdered->Show();
 printf("eSetOrder (int) is: %d, lastOpError=%d\n", (int)ptrOrdered->Order(), ptrOrdered->lastOpError);

 printf("\nThe same list, in detail:\n");
 for (p=ptrOrdered->StartPtr(); p; p=p->next) {
     printf("\t%13d '%s' length(): %d\n",
	    p->me->iValue,
	    p->Str(),
	    p->me->Length());
 }

 delete ptrOrdered;

 printf("+++\n");

 return result;
}

////////////////////////////////////////////////////////////
int main (int argc, char* argv[])
{
 int error;

 gINIT;
 im_init();

 error = do_debug( argc-1, argv );
 DBGPRINT("DBG: Objs undeleted: %d\n",gStorageControl::Self().NumObjs());
 gEND;

 printf("Returning %d\n",error);
}

////////////////////////////////////////////////////////////

