// set.cpp

#include "set.h"

////////////////////////////////////////////////////////////
// Static members
// ---------------------------------------------------------
iSet::eSetOrder iSet::defaultOrder=e_ascending;

////////////////////////////////////////////////////////////
// Set classes
////////////////////////////////////////////////////////////
iSet::iSet ()
    : lastOpError( 0 ),
      setOrder( defaultOrder ),
      repeatedValues( false )
{
}


iSet::iSet (const char* strInit)
    : lastOpError( 0 ),
      setOrder( defaultOrder ),
      repeatedValues( false )
{
 thisAddFromStr( strInit );
}


iSet::iSet (iSet& copy)
    : lastOpError( 0 ),
      setOrder( defaultOrder ),
      repeatedValues( false )
{
 thisAddAt( copy.StartPtr() );
}


iSet::~iSet ()
{
}


gList& iSet::operator= (iSet& copy)
{
 return CopyFrom( copy.StartPtr(), false );
}


iSet& iSet::operator= (const char* strInit)
{
 Reset();
 thisAddFromStr( strInit );
 return *this;
}


unsigned iSet::Add (int v)
{
 bool valid;
 gString* newStr( thisNewStrFromNumber( v, "%d" ) );
 lastOpError = 0;
 valid = thisAddOne( *this, newStr, false );
 if ( valid==false ) {
     delete newStr;
 }
 return valid==true;  // returns 1 if value (as string) was added
}


unsigned iSet::Add (unsigned v)
{
 bool valid;
 gString* newStr( thisNewStrFromNumber( v, "%u" ) );
 lastOpError = 0;
 valid = thisAddOne( *this, newStr, false );
 if ( valid==false ) {
     delete newStr;
 }
 return valid==true;  // returns 1 if value (as string) was added
}


unsigned iSet::Add (gString& copy)
{
 lastOpError = 0;
 thisAddOne( *this, new gString( copy.Str() ), false );
 return lastOpError!=0;
}


unsigned iSet::Add (const char* s)
{
 lastOpError = 0;
 if ( s ) {
     thisAddOne( *this, new gString( s ), false );
 }
 return lastOpError==0;  // returns 1 if string was added
}


unsigned iSet::Add (t_uchar* s)
{
 return Add( (char*)s );
}


gList& iSet::CopyFrom (gElem* pFrom, bool doUpChar)
{
 thisDelete();
 lastOpError = 0;
 return AddFrom( pFrom, doUpChar );
}


gList& iSet::AddFrom (gElem* pElem, bool doUpChar)
{
 gStorage* aObj;
 gStorage* newObj;
 gElem* thisElem( pElem );

 for ( ; thisElem; ) {
     aObj = thisElem->me;
     ASSERTION(aObj!=nil,"aObj!=nil");
     newObj = aObj->NewObject();
     ASSERTION(newObj,"newObj");
     if ( doUpChar && newObj->IsString() ) {
	 ((gString*)newObj)->UpString();
     }
     if ( repeatedValues==false ) {
	 if ( Match( newObj->Str() ) ) {
	     DBGPRINT("DBG: skipped duplicate {%s}\n", newObj->Str());
	     delete newObj;
	     newObj = nil;
	     lastOpError++;
	 }
     }
     if ( newObj ) {
	 thisAddOne( *this, newObj, false );
     }
     thisElem = thisElem->next;
 }
 return *this;
}


bool iSet::thisAddOne (iSet& pList, gStorage* cell, bool ordered)
{
 bool valid( true );
 bool isNumeric;
 gString* aString;
 int value( -1 );

 ASSERTION(cell,"cell");
 aString = (gString*)cell;
 if ( pList.repeatedValues==false ) {
     // Check if any element exists
     if ( pList.Match( aString->Str() ) ) {
	 valid = false;
	 lastOpError++;
     }
 }
 if ( valid ) {
     isNumeric = gFileControl::Self().ConvertToInt32( aString->Str(), value )==0;
     pList.thisAppend( aString );
     if ( isNumeric==false ) {
	 value = I_NUM_INVALID;
     }
     pList.EndPtr()->me->iValue = value;
 }
 return valid;
}


int iSet::thisAddFromStr (const char* strInit)
{
 return
     thisAddFromStrAs( strInit, " ", false );
}

int iSet::thisAddFromStrAs (const char* strInit, const char* separator, bool ordered)
{
 gParam args( (char*)strInit, (char*)separator );
 gElem* p( args.StartPtr() );

 lastOpError = 0;

 for ( ; p; p=p->next) {
     gString* newCell( new gString( p->me->Str() ) );
     ASSERTION(newCell,"newCell");
     thisAddOne( *this, newCell, ordered );
 }
 return lastOpError!=0;
}


int iSet::thisAddAt (gElem* ptr, bool ordered)
{
 if ( ptr==nil ) return 0;  // nothing to add

 for ( ; ptr; ptr=ptr->next) {
     thisAddOne( *this, new gString( ptr->me->Str() ), ordered );
 }
 return 0;
}


gString* iSet::thisNewStrFromNumber (int v, const char* strFormat)
{
 gString* newStr( new gString( 24, '\0' ) );
 ASSERTION(newStr,"newStr");
 ASSERTION(strFormat,"strFormat");
 snprintf(newStr->Str(), 24, strFormat, v);
 //DBGPRINT("DBG: newStr is: %s, strlen: %d, Length(): %d\n",
 //	  newStr->Str(),
 //	  strlen( newStr->Str() ),
 //	  newStr->Length());
 return newStr;
}


iSet* iSet::thisNewOrderedSet (iSet& copy, gElem* pStart, eSetOrder byOrder)
{
 iSet* newSet;
 gElem* p( pStart );

 if ( pStart ) {
     newSet = new iSet;
     ASSERTION(newSet,"newSet");

     if ( byOrder==e_invalid ) {
	 return newSet;
     }

     DBGPRINT("DBG: thisNewOrderedSet(list.N()=%u, %p), own N()=%u, byOrder: %d\n",
	      copy.N(),
	      pStart,
	      N(),
	      byOrder);
     if ( byOrder==e_none ) {
	 for ( ; p; p=p->next) {
	     newSet->Add( p->me->iValue );
	 }
     }
     else {
	 t_uint32 iter( 1 ), uSize( copy.N() );
	 int* values( new int[ uSize+1 ] );
	 int worseValue( byOrder!=e_ascending ? MIN_INT32BITS : MAX_INT32BITS );
	 gStorage** elems( new gStorage*[ uSize+1 ] );

	 ASSERTION(values,"values");
	 ASSERTION(elems,"elems");

	 values[ 0 ] = 0;
	 elems[ 0 ] = nil;

	 for ( ; iter<=uSize; iter++, p=p->next) {
	     ASSERTION(p,"p(1)");
	     values[ iter ] = p->me->iValue;
	     elems[ iter ] = p->me;
	     DBGPRINT_MIN("iter=%d/%d: %d, %s\n", iter, uSize, p->me->iValue, p->me->Str());
	 }

	 int tries( (int)uSize );
	 int firstInvalid( 0 );

	 for (int myValue; newSet->N() < (unsigned)uSize; ) {
	     int currentBest( worseValue );
	     int keepIdx( -1 );

	     if ( tries-- < 0 ) {
		 lastOpError = -1;
		 break;
	     }

	     switch ( byOrder ) {
	     case e_none:
		 ASSERTION_FALSE("byOrder(e_none) -- UNSUPPORTED");
		 break;

	     case e_ascending:
		 for (iter=1; iter<=uSize; iter++) {
		     if ( elems[ iter ] ) {
			 myValue = values[ iter ];
			 if ( myValue < currentBest ) {
			     keepIdx = iter;
			     currentBest = myValue;
			 }
		     }
		 }
		 ASSERTION(keepIdx!=-1,"keepIdx");
		 myValue = values[ keepIdx ];
		 if ( myValue==I_NUM_INVALID ) {
		     newSet->Add( elems[ keepIdx ]->Str() );
		 }
		 else {
		     newSet->Add( myValue );
		 }
		 elems[ keepIdx ] = nil;
		 break;

	     case e_descending:
		 for (iter=1, firstInvalid=-1, keepIdx=-1; iter<=uSize; iter++) {
		     if ( elems[ iter ] ) {
			 myValue = values[ iter ];
			 if ( myValue > currentBest ) {
			     keepIdx = iter;
			     currentBest = myValue;
			 }
			 else {
			     if ( firstInvalid==-1 && myValue==I_NUM_INVALID ) {
				 firstInvalid = iter;
			     }
			 }
		     }
		 }
		 if ( keepIdx<1 && firstInvalid>=1 ) {
		     keepIdx = firstInvalid;
		 }
		 ASSERTION(keepIdx!=-1,"keepIdx");
		 myValue = values[ keepIdx ];
		 if ( myValue==I_NUM_INVALID ) {
		     newSet->Add( elems[ keepIdx ]->Str() );
		 }
		 else {
		     newSet->Add( myValue );
		 }
		 elems[ keepIdx ] = nil;
		 break;

	     default:
		 ASSERTION_FALSE("byOrder(?)");
		 break;
	     }
	 }

	 delete[] elems;
	 delete[] values;
     }

     printf("newSet, lastOpError=%d: ", lastOpError); newSet->Show();
     return newSet;
 }
 return nil;
}

////////////////////////////////////////////////////////////

