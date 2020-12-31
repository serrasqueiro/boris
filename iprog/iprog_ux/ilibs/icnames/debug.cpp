// debug.cpp, for libicnames

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib_iobjs.h"
#include "lib_icnames.h"

////////////////////////////////////////////////////////////
int check_string (const char* str, sIcnTlds& tlds)
{
 gString* ptrMe;

 printf("check_string(%s):\n",str);

 ptrMe = tlds.DomainXtraFromStr( str );
 printf("Xtra	%s\n",
	ptrMe ? ptrMe->Str() : "?");

 if ( str && strlen( str )==2 ) {
     ptrMe = tlds.ValidDomain( str[ 0 ], str[ 1 ] );
     printf("Valid	%s\n",
	    ptrMe ? ptrMe->Str() : "?");
 }

 ptrMe = tlds.AnyDomain( str );
 printf("Any	%s\n",
	ptrMe ? ptrMe->Str() : "?");

 printf("@dns	%s\n",
	icn_dns_reversed( str, 1 ));

 return 0;
}


int dbg_test (const char* str)
{
 int error( 0 );
 int iter( 0 );
 int check, flow;
 bool shown( true );
 t_uchar uCharA( 'a' ), uCharB;
 const char* strTLD;
 gList hashVals;

 printf("ccTLDs:\n");
 strTLD = icn_cctld_find_first();
 for ( ; strTLD; iter++) {
     if ( iter>=10 && strcmp( strTLD, "v" )<0 ) {
	 // Suppress the tenth and TLDs up to "v"X
	 if ( shown ) {
	     printf("\t[...]\n");
	     shown = false;
	 }
     }
     else {
     printf("\t0x%04x\t%s\n",
	    ICN_TLD_HASH( myIcnTlds->ptrCurrent->me->iValue ),
	    strTLD);
     }
     strTLD = icn_tld_find_next();
 }
 printf(".\n\n");

 printf("TLDs:\n");
 strTLD = icn_tld_find_first();
 for (shown=true; strTLD; iter++) {
     hashVals.Add( (int)ICN_TLD_HASH( myIcnTlds->ptrCurrent->me->iValue ) );
     if ( strlen( strTLD )==2 && strcmp( strTLD, "az" )>0 && strcmp( strTLD, "v" )<0 ) {
	 if ( shown ) {
	     printf("\t[...]\n");
	     shown = false;
	 }
     }
     else {
	 printf("\t0x%04x\t%s\n",
		ICN_TLD_HASH( myIcnTlds->ptrCurrent->me->iValue ),
		strTLD);
     }
     strTLD = icn_tld_find_next();
 }
 printf(".\n\n");

 // CHECK repetitions!
 // (It may happen, depending on configured domain strings.)

 for (iter=1; iter<=(int)hashVals.N(); iter++) {
     check = hashVals.GetListInt( iter );

     // Check for repetitions:
     for (flow=1; flow<iter; flow++) {
	 if ( check==hashVals.GetListInt( flow ) ) {
	     error++;
	     fprintf(stderr,"Repeated hash value: 0x%04x\n",(unsigned)check);
	 }
     }
 }

 // Display country hashes

 for (check=-1; uCharA<='z'; uCharA++) {
     for (iter=1; (uCharB = myIcnTlds->hashedCc[ uCharA-'a' ][ iter ])!=0; iter++) {
	 check = (iter % 25)==0;
	 printf(" %c%c%s",
		uCharA, uCharB,
		check ? "\n" : "\0");
	 // iter%25 avoids too long lines
     }
     if ( check==0 ) printf("\n");
     check = -1;
 }

 return error!=0;
}


int do_debug (const char* str)
{
 int error( dbg_test( str ) );

 myIcnTlds->Optimize();

 check_string( str, *myIcnTlds );

 return error;
}


void test_ntp_analyst ()
{
 const int year=2017;
 const int month=2;
 const int day=25;
 printf("++++\ntest_ntp_analyst()\n");
 printf("nys_DateToMjd(1900, 1, 1)=%ld\n", nys_DateToMjd(1900, 1, 1));
 printf("nys_DateToMjd(%d, %d, %d)=%ld\n",
	year, month, day,
	nys_DateToMjd(year, month, day));

 t_uint64 sec( (t_uint64)NTPSEC_FROM_1970JAN_EPOCH );
 t_uint64 anySec;

 anySec = nys_SecondsSince1900(1900, 1, 1);
 printf("\t\tseconds since  1 Jan 1900: %llu\n", anySec);
 anySec = nys_SecondsSince1900(1970, 1, 1);
 printf("\t\tseconds since  1 Jan 1970: %llu\n", anySec);
 anySec = nys_SecondsSince1900(year, month, day);
 printf("\t\tseconds since %02d Feb %04d: %llu\n",
	day, year,
	anySec);
 printf("\
\t\t\t-> in hex:         0x%016llX\n\
\t\t\t-> lowest 32bits:  0x%08lX\n\
\t\t\t-> in dec. sign.:  %ld\n",
	anySec,
	(unsigned long)(anySec & 0xFFFFFFFF),
	(long)((unsigned long)(anySec & 0xFFFFFFFF)));
 printf("\n\
Seconds at 1970 in NTP timestamp: 2,208,988,800 i.e.\t%lu\n",
	NTPSEC_FROM_1970JAN_EPOCH);
 printf("\n");
 printf("check: %ld\n", (long)((t_uint64)NTPSEC_FROM_1970JAN_EPOCH - sec));
}


int main (int argc, char* argv[])
{
 int error = 0;

 gINIT;

#ifdef TEST_ONLY_ORDER
#warning TODO
#endif

 icn_init();

 error = do_debug( argv[ 1 ] );
 printf("do_debug returned %d\n",error);

 test_ntp_analyst();
 printf("++++\n");

 printf("DBG: Objs: %d\n",gStorageControl::Self().NumObjs());

 myIcnTlds->Release();
 delete myIcnTlds;

 gEND;
 printf("DBG: Objs undeleted: %d\n",gStorageControl::Self().NumObjs());

 return error;
}
////////////////////////////////////////////////////////////

