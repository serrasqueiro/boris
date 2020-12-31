// iordered.cpp

#include <unistd.h>

#include "iordered.h"

////////////////////////////////////////////////////////////
bool sIOrdered::CloseHandle ()
{
 if ( fd==-1 ) return false;
 if ( fd==fileno( stdin ) || fd==fileno( stdout ) || fd==fileno( stderr ) ) {
 }
 else {
     close( fd );
     DBGPRINT_MIN("DBG: CLOSED fd=%d: {%s}\n",
		  fd,
		  named.Str());
 }
 fd = -1;
 return true;
}


int sIOrdered::AddUnique (const char* oBuf, int length, int maxLenCompare)
{
 static int iterOrder;
 int code( 1 );  // 1 means added
 int compared( -2 );
 gString* newStr( nil );

 ASSERTION(oBuf,"oBuf");
 if ( length<0 ) return 0;

 // You should remove iterOrder: it is only for debug:
 iterOrder++;

 if ( lista.N()==0 ) {
     lista.Add( (char*)oBuf );
     ptrLast = lista.EndPtr();
     sFirst.Set( (char*)oBuf );
     sLast = sFirst;
 }
 else {
     if ( maxLenCompare<0 ) {
	 compared = strcmp( oBuf, sLast.Str() )>0;
     }
     else {
	 compared = strncmp( oBuf, sLast.Str(), maxLenCompare )>=0;

	 DBGPRINT_MIN("%s%s::: #%d: compared=%d\n\n",
		      oBuf,
		      sLast.Str(),
		      maxLenCompare,
		      compared);
     }

     if ( compared>=1 ) {
	 lista.Add( (char*)oBuf );
	 ptrLast = lista.EndPtr();
	 ptrLast->me->iValue = -iterOrder;
	 sLast.Set( (char*)oBuf );
     }
     else {
	 // Should be ...e.g.
	 // for ( ; ptrLast; ptrLast=ptrLast->prev) { ... }

	 // but i am too lazy! Algorithm not as fast as it could.

	 newStr = new gString( (char*)oBuf );
	 ASSERTION(newStr,"newStr");
	 newStr->iValue = iterOrder;
	 code = lista.InsertOrderedUnique( newStr );
	 ptrLast = lista.CurrentPtr();

	 if ( ptrLast==lista.EndPtr() ) {
	     sLast.Set( (char*)oBuf );
	 }
	 if ( ptrLast==lista.StartPtr() ) {
	     sFirst.Set( (char*)oBuf );
	 }
     }
 }

 if ( code==-1 ) {
     // Repeated
     repeated++;
     delete newStr;
     ptrLast = lista.EndPtr();
 }
 else {
     lines++;
 }

 return code;
}

////////////////////////////////////////////////////////////
int icn_iordered_read (int fdIn, sIOrdered& order)
{
 // #warning TODO
 return 0;
}

////////////////////////////////////////////////////////////


