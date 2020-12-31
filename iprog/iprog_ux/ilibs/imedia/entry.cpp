// entry.cpp


#include "entry.h"

////////////////////////////////////////////////////////////
iString::iString (char* aStr)
    : gString( aStr ),
      equalFactor( 1 )
{
}


iString::iString (iString& copy)
    : gString( copy ),
      equalFactor( 1 )
{
}


iString::iString (unsigned nBytes, char c)
    : gString( nBytes, c ),
      equalFactor( 1 )
{
}


iString::~iString ()
{
}

////////////////////////////////////////////////////////////
iEntry::iEntry (gElem* pElem)
    : lastOpError( 0 )
{
 thisCopyFromElement( pElem );
}


iEntry::iEntry (const char* str)
    : lastOpError( 0 )
{
 thisAdd( (char*)str );
}


iEntry::~iEntry ()
{
}


int iEntry::SetComment (const char* strComment)
{
 char chr;
 int value( 0 );

 sComment.Set( (char*)strComment );

 if ( strComment ) {
     for (int iter=0; (chr = strComment[ iter ])>=' '; iter++) {
	 if ( chr>='0' && chr<='9' ) {
	     value = sComment.iValue = atoi( strComment + iter );
	     break;
	 }
     }
 }
 else {
     sComment.Reset();
 }
 return value;
}


int iEntry::thisAdd (char* aStr, int value)
{
 gString* myStr( new gString( aStr ) );
 ASSERTION(myStr,"myStr");
 thisAppend( myStr );
 return (myStr->iValue = value);
}


int iEntry::thisCopyFromElement (gElem* pElem)
{
 int count( 0 );
 for ( ; pElem; pElem=pElem->next) {
     thisAdd( pElem->Str(), pElem->me->iValue );
     count++;
 }
 return count;
}

////////////////////////////////////////////////////////////

