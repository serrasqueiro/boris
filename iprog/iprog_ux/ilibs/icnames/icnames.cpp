// icnames.cpp

#include "lib_icnames.h"
#include "iarg.h"

#include "tld_listed.h"

////////////////////////////////////////////////////////////
// Globals
// ---------------------------------------------------------
sIcnTlds* myIcnTlds;

////////////////////////////////////////////////////////////
bool sIcnTlds::Init ()
{
 Release();
 hashedCc = new gString[ 26 ];
 ASSERTION(hashedCc,"hashedCc");
 memset( bufDns, 0x0, sizeof(bufDns) );
 return true;
}


void sIcnTlds::Release ()
{
 if ( hashedCc ) {
     delete[] hashedCc;
     hashedCc = nil;
 }

 for (t_uint16 idx=0; idx<26; idx++) {
     if ( xtraTlds[ idx ] ) {
	 delete[] xtraTlds[ idx ];
	 xtraTlds[ idx ] = nil;
     }
 }
 ptrCurrent = nil;
 ptrExtra = nil;
 orderedCcTlds.Delete();
 allTlds.Delete();
}


int sIcnTlds::HashedCountryCode (gString& ccTwoLetter, const char* strOptName)
{
 int code( -1 );
 t_uchar uCharA( ccTwoLetter[ 1 ] );
 t_uchar uCharB( ccTwoLetter[ 2 ] );

 if ( uCharA<'a' || uCharA>'z' ) return -1;
 if ( uCharB<'a' || uCharB>'z' ) return -1;
 if ( ccTwoLetter.Length()!=2 ) return -2;

 uCharA -= 'a';

 if ( hashedCc ) {
     code = 0;
     if ( hashedCc[ uCharA ].Find( uCharB ) )
	 code = 1;  // Repeated
     else
	 hashedCc[ uCharA ].Add( uCharB );
 }
 return code;
}


bool sIcnTlds::OptimizeTlds ()
{
 int count, iter;
 t_uchar uCharA( 'a' ), firstChr;
 t_uint16 idx( 0 );
 gElem* ptrElem;
 gString* ptrMe;

 if ( ptrExtra==nil ) return false;

 // Iterate each one of the letters to see how many there are:
 for ( ; idx<26; uCharA++, idx++) {
     count = 0;
     for (ptrElem=ptrExtra; ptrElem; ptrElem=ptrElem->next) {
	 firstChr = ptrElem->Str()[ 0 ];
	 count += firstChr==uCharA;
     }

     xtraTlds[ idx ] = new gString*[ count+2 ];
     ASSERTION(xtraTlds[ idx ],"xtraTlds");
     xtraTlds[ idx ][ 0 ] = nil;
     ptrElem = ptrExtra;

     for (iter=1; iter<=count; iter++) {
	 for ( ; ptrElem; ) {
	     ptrMe = (gString*)ptrElem->me;
	     firstChr = (*ptrMe)[ 1 ];
	     ptrElem = ptrElem->next;
	     if ( firstChr==uCharA ) {
		 xtraTlds[ idx ][ iter ] = ptrMe;
		 break;
	     }
	 }
     }
     xtraTlds[ idx ][ iter ] = nil;

#if 0
     printf("DBG: %c\t",uCharA);
     for (int dbgX=1; ; dbgX++) {
	 ptrMe = xtraTlds[ idx ][ dbgX ];
	 if ( ptrMe==nil ) break;
	 printf(" %s (hash is: 0x%04x)",
		ptrMe->Str(),
		ICN_TLD_HASH( ptrMe->iValue ));
     }
     printf("\n");
#endif
 }
 return true;
}

////////////////////////////////////////////////////////////

int tlds_add_to_list (gList& inList, bool doOrder, gList& outList)
{
 // outList is added with valid items found in 'inList';
 // an item is valid if has valid characters and is not empty

 int error( 0 ), code;
 int countEmpty( 0 );
 bool notInserted;
 gElem* ptrElem( inList.StartPtr() );

 for ( ; ptrElem; ptrElem=ptrElem->next) {
     gString sNew( ptrElem->Str() );
     sNew.ConvertAnyChrTo( "\t\n", ' ' );
     sNew.Trim();
     if ( sNew.Match( "__" ) ) continue;  // Ignore

     if ( sNew.Length() ) {
	 gString* ptrNew( new gString( sNew ) );
	 ASSERTION(ptrNew,"ptrNew");
	 ptrNew->Hash();

	 if ( doOrder ) {
	     code = outList.InsertOrderedUnique( ptrNew );
	     notInserted = code==-1;
	     if ( notInserted ) {
		 delete ptrNew;
		 error = -1;
	     }
	 }
	 else {
	     outList.AppendObject( ptrNew );
	 }
     }
     else {
	 countEmpty++;
     }
 }
 if ( error ) return error;  // -1 if there is any repetition
 return countEmpty;
}


int init_tlds (sIcnTlds& tlds)
{
 int error;
 int countErrors( 0 );
 gString ccTld( (char*)GS_ALLOWED_CCTLD );
 gString xtraTld( (char*)GS_ALLOWED_CCTLD_XTRA );
 gElem* ptrElem;

 // Initialize top level domains
 tlds.Init();

 if ( tlds.allTlds.N() || tlds.orderedCcTlds.N() )
     return -1;  // Initializing twice?

 ccTld.DownString();

 gParam myCountryCodes( ccTld, "\n" );
 error = tlds_add_to_list( myCountryCodes, true, tlds.orderedCcTlds );
 countErrors += (error!=0);
 iprint("tlds_add_to_list: empty=%d (size now is: %u)\n",
	error,
	tlds.orderedCcTlds.N());

 // Hash country-codes:
 for (ptrElem=tlds.orderedCcTlds.StartPtr(); ptrElem; ptrElem=ptrElem->next) {
     gString sCC( ptrElem->me->Str() );
     tlds.HashedCountryCode( sCC, nil );
 }

 gParam allTLD( xtraTld, "\n" );
 error = tlds_add_to_list( tlds.orderedCcTlds, false, tlds.allTlds );
 countErrors += (error!=0);
 iprint("tlds_add_to_list: empty=%d (size now is: %u)\n",
	error,
	tlds.allTlds.N());

 tlds.ptrExtra = tlds.allTlds.EndPtr();

 error = tlds_add_to_list( allTLD, false, tlds.allTlds );
 countErrors += (error!=0);

 tlds.ptrExtra = tlds.ptrExtra->next;

 return countErrors;
}

////////////////////////////////////////////////////////////
// icn functions
////////////////////////////////////////////////////////////
int icn_init ()
{
 int error;
 myIcnTlds = new sIcnTlds;
 ASSERTION(myIcnTlds,"myIcnTlds");
 error = init_tlds( *myIcnTlds );
 iprint("init_tlds() returns %d\n",error);
 return error;
}


void icn_finit ()
{
 if ( myIcnTlds ) {
     delete myIcnTlds;
     myIcnTlds = nil;
 }
}


char* icn_tld_find_first ()
{
 gElem* ptrElem( myIcnTlds->allTlds.StartPtr() );
 if ( ptrElem ) {
     myIcnTlds->ptrCurrent = ptrElem;
     return ptrElem->Str();
 }
 return nil;
}


char* icn_tld_find_next ()
{
 gElem* ptrElem( myIcnTlds->ptrCurrent );
 if ( ptrElem ) {
     ptrElem = myIcnTlds->ptrCurrent->next;
     myIcnTlds->ptrCurrent = ptrElem;
     if ( ptrElem ) {
	 return ptrElem->Str();
     }
 }
 return nil;

}


char* icn_cctld_find_first ()
{
 gElem* ptrElem( myIcnTlds->orderedCcTlds.StartPtr() );

 if ( ptrElem ) {
     myIcnTlds->ptrCurrent = ptrElem;
     return ptrElem->Str();
 }
 return nil;
}


char* icn_cctld_find_next ()
{
 // Either country-code TLD or TLD is to be used,
 // not one and then the other.
 // Simplification...

 return icn_tld_find_next();
}


char* icn_dns_reversed (const char* strDNS, t_uint32 mask)
{
 int len, iter;
 const int bufSize( sizeof( myIcnTlds->bufDns ) );
 char* buf( myIcnTlds->bufDns );
 char* strRight( nil );
 gString sTemp( (char*)strDNS );

 buf[ 0 ] = 0;
 if ( strDNS==nil || strDNS[ 0 ]==0 ) return buf;

 sTemp.Trim();
 len = (int)sTemp.Length();

 if ( len<=0 ) return buf;

 iter = (int)sTemp.FindBack( '@' );

 if ( iter>0 ) {
     strRight = sTemp.Str( iter );
     sTemp.Str()[ iter-1 ] = 0;
     snprintf( buf, bufSize, "%s@%s", strRight, sTemp.Str() );
 }
 return buf;
}

////////////////////////////////////////////////////////////

