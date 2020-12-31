// ilist.cpp

#include <string.h>

#include "ilist.h"
#include "istring.h"
#include "ifile.h"

////////////////////////////////////////////////////////////
// Static members
// ---------------------------------------------------------
const char* gListGeneric::strNil="\0";
////////////////////////////////////////////////////////////
// gElem - Generic elements of lists
// ---------------------------------------------------------
gElem::gElem ()
    : gStorage( e_ResvdStore ),
      prev( nil ),
      next( nil ),
      me( nil )
{
}


gElem::gElem (gStorage* newObj)
    : gStorage( e_ResvdStore ),
      prev( nil ),
      next( nil ),
      me( newObj )
{
 ASSERTION(me!=nil,"me!=nil");
}


gElem::~gElem ()
{
 delete me;
 me = nil;
}


gStorage* gElem::NewObject ()
{
 ASSERTION_FALSE("gElem::NewObject: Not used");
 return nil;
}


t_uchar* gElem::ToString (const t_uchar* uBuf)
{
 ASSERTION(me!=nil && uBuf!=nil,"me!=nil");
 return me->ToString( uBuf );
}


char* gElem::Str (unsigned idx)
{
 ASSERTION(me!=nil,"me!=nil");
 return me->Str();
}


bool gElem::SaveGuts (FILE* f)
{
 ASSERTION(me!=nil,"gElem::SaveGuts");
 return me->SaveGuts( f );
}


bool gElem::RestoreGuts (FILE* f)
{
 ASSERTION(me!=nil,"gElem::SaveGuts");
 return me->SaveGuts( f );
}
////////////////////////////////////////////////////////////
// gListGeneric - Generic list handling
// ---------------------------------------------------------
gListGeneric::gListGeneric (eStorage aKind, const char* aStr)
    : gStorage( aKind, e_StgDefault ),
      size( 0 ),
      pStart( nil ),
      pEnd( nil ),
      pCurrent( nil ),
      sShown( aStr )
{
}


gListGeneric::gListGeneric (eStorage aKind, gString& s)
    : gStorage( aKind, e_StgDefault ),
      size( 0 ),
      pStart( nil ),
      pEnd( nil ),
      pCurrent( nil ),
      sShown( s )
{
}


gListGeneric::~gListGeneric ()
{
 thisDelete();
}


bool gListGeneric::IsValidIndex (unsigned idx)
{
 return thisIndex( idx );
}


char* gListGeneric::Str (unsigned idx)
{
 thisIndex( idx );
 if ( pCurrent==nil ) return (char*)strNil;
 return pCurrent->Str();
}


gStorage::eStorage gListGeneric::ElementsKind ()
{
 eStorage thisKind( e_NoStorage ), lastKind( e_NoStorage );
 unsigned idx( 1 ), nDiffs( 0 );

 if ( thisIndex( idx )==false ) return e_List;

 ASSERTION(size>0,"size>0");
 // Here, list is not empty:
 // it can be all-of-the-same-kind, or variously filled.
 for (pCurrent=pStart; pCurrent!=nil; pCurrent=pCurrent->next) {
     thisKind = pCurrent->me->Kind();
     if ( thisKind!=lastKind && lastKind!=e_NoStorage ) nDiffs++;
     lastKind = thisKind;
 }
 if ( nDiffs>0 ) return e_List;
 return thisKind;
}


gElem& gListGeneric::GetElement (unsigned idx)
{
 static gElem tempElem;
 if ( thisIndex( idx ) ) return *pCurrent;
 return tempElem;
}


gElem* gListGeneric::GetElementPtr (unsigned idx)
{
 if ( thisIndex( idx ) ) return pCurrent;
 return nil;
}


gStorage* gListGeneric::GetObjectPtr (unsigned idx)
{
 gElem* pElem = GetElementPtr( idx );
 ASSERTION(pElem!=nil,"GetObjectPtr(1)");
 ASSERTION(pElem->me!=nil,"GetObjectPtr(2)");
 return pElem->me;
}


gStorage* gListGeneric::GetFirstObjectPtr ()
{
 if ( pStart==nil ) return nil;
 return pStart->me;
}


gStorage* gListGeneric::GetLastObjectPtr ()
{
 if ( pEnd==nil ) return nil;
 return pEnd->me;
}


int gListGeneric::GetFirstInt ()
{
 if ( size<1 ) return 0;
 return pStart->me->GetInt();
}


int gListGeneric::GetLastInt ()
{
 if ( size<1 ) return 0;
 return pEnd->me->GetInt();
}


unsigned gListGeneric::Delete (unsigned startPos, unsigned endPos)
{
 unsigned
     counter( 0 ),
     oldSize( N() ),
     pIter,
     p0( startPos==0 ? 1 : startPos ),
     p1( endPos==0 ? oldSize : (gMIN(endPos,oldSize)) );
 gElem* pTemp;
 gElem* pNext;

 if ( startPos==0 && endPos==0 ) {
     thisDelete();
     return oldSize;
 }

 if ( p0>p1 ) return 0;

 // Delete only some positions
 // Place current pointer on first element to delete.
 thisIndex( p0 );
 ASSERTION(p0>=1 && p1<=oldSize,"Delete(1)");

 for (pIter=p0, pTemp=GetElementPtr(p0); pIter<=p1; pIter++) {
     ASSERTION(size>0,"Delete(2)");
     size--;
     counter++;
     pCurrent = pTemp;
     ASSERTION(pCurrent!=nil,"pCurrent!=nil");
     pNext = pTemp->next;
     if ( pTemp->prev==nil ) {
	 // Start of list
	 pStart = pNext;
	 pTemp = pNext;
	 if ( pTemp!=nil ) pTemp->prev = nil;
     }
     else {
	 if ( pNext==nil ) {
	     // At end of list
	     pTemp = pTemp->prev;
	     pTemp->next = nil;
	     pEnd = pTemp;
	 }
	 else {
	     // At middle of list
	     // (pTemp->prev is not nil!)
	     pTemp = pTemp->prev;
	     pTemp->next = pNext;
	     pNext->prev = pTemp;
	     pTemp = pNext;  // Placed iterator in next position
	 }
     }
     ASSERTION(pCurrent!=nil,"pCurrent!=nil");
     delete pCurrent;
     if ( pTemp==nil ) {
	 pStart = pEnd = nil;
     }
     else {
	 if ( pTemp->next==nil ) pEnd = pTemp;
	 if ( pTemp->prev==nil ) pStart = pTemp;
     }
 }//end FOR
 pCurrent = nil;
 return counter;
}


gStorage* gListGeneric::NewObject ()
{
 ASSERTION_FALSE("gListGeneric::NewObject: Not used");
 return nil;
}


t_uchar* gListGeneric::ToString (const t_uchar* uBuf)
{
 // Output notation:
 //     Empty list               ===>    ()
 //     List of strings          ===>    (me and you)
 //     List of strings and nums ===>    (I am '31 years old)
 //     List of sentences        ===>    ("I use a suit." "You use a dress.")
 //     List with some resvd chrs.==>    (123-a.com sold '100 domains)
 ;
 return nil;
}


bool gListGeneric::SaveGuts (FILE* f)
{
 unsigned i, n=N();
 gString sBlank( ' ' );

 if ( CanSave( f )==false ) return false;
 for (i=1; i<=n; i++) {
     if ( i>1 ) sBlank.SaveGuts( f );
     GetObjectPtr( i )->SaveGuts( f );
 }
 return true;
}


bool gListGeneric::RestoreGuts (FILE* f)
{
 return CanRestore( f );
}


unsigned gListGeneric::thisDelete ()
{
 unsigned countCheck=0;
 gElem* pNext;

 for (pCurrent=pStart; pCurrent!=nil; )
 {
     pNext = pCurrent->next;
     delete pCurrent;
     pCurrent = pNext;
     countCheck++;
 }
 ASSERTION(countCheck==size,"countCheck==size");
 size = 0;
 pStart = pEnd = pCurrent = nil;
 return countCheck;
}


bool gListGeneric::thisIndex (unsigned& idx)
{
 bool isOk( idx>=1 && idx<=size );
 unsigned i;

 pCurrent = nil;

 if ( idx<1 ) idx = 1;
 isOk = idx<=size;
 if ( idx>size ) idx = size;
 if ( isOk==false ) return false;

 for (i=1, pCurrent=pStart; i<idx; i++) {
     pCurrent = pCurrent->next;
     ASSERTION(pCurrent!=nil,"pCurrent!=nil");
 }
 ASSERTION(pCurrent!=nil,"pCurrent!=nil");
 ASSERTION(pCurrent->me!=nil,"pCurrent->me!=nil");
 return true;
}


bool gListGeneric::thisAppend (gStorage* newObj)
{
 // Returns true iff new re-allocation performed.
 // Since no vector functions are present (yet),
 // returns always false.
 ;
 gElem* newElem;

 ASSERTION(newObj!=nil,"newObj!=nil");
 newElem = new gElem( newObj );
 ASSERTION(newElem!=nil,"newElem!=nil");

 if ( size==0 ) {
     ASSERTION(pStart==nil && pEnd==nil,"::thisAppend(1)");
     pStart = newElem;
 }
 else {
     newElem->prev = pEnd;
     pEnd->next = newElem;
 }

 pCurrent = pEnd = newElem;
 size++;

 return false;
}


int gListGeneric::operator[] (int idx)
{
 unsigned aIdx( (unsigned)idx );
 if ( idx<0 || thisIndex( aIdx )==false )
     return -1;
 return pCurrent->GetInt();
}

////////////////////////////////////////////////////////////
gList::gList (const char* aStr)
    : gListGeneric( e_List, aStr )
{
}


gList::gList (gString& s)
    : gListGeneric( e_List, s )
{
}


gList::~gList ()
{
}


int gList::GetInt (unsigned idx)
{
 int value( GetListInt( idx ) );
 return value;
}


int gList::GetListInt (unsigned idx, eStrict strictness)
{
 if ( thisIndex( idx )==false ) return 0;

 gStorage* myElem( pCurrent->me );
 eStorage stgElem( myElem->Kind() );

 if ( stgElem!=e_SInt ) {
     if ( stgElem==e_UInt ) {
	 if ( strictness!=relaxed )
	     return 0;
     }
     else {
	 return 0;
     }
 }
 return myElem->GetInt();
}


unsigned gList::GetListUInt (unsigned idx, eStrict strictness)
{
 if ( thisIndex( idx )==false ) return 0;

 gStorage* myElem( pCurrent->me );
 eStorage stgElem( myElem->Kind() );

 if ( stgElem!=e_UInt ) {
     if ( stgElem==e_SInt ) {
	 if ( strictness!=relaxed )
	     return 0;
	 return myElem->GetInt();
     }
     else {
	 return 0;
     }
 }
 return myElem->GetInt();
}


unsigned gList::Match (const char* s)
{
 unsigned i( 1 );
 char* thisStr;

 if ( s==nil ) return 0;

 for (pCurrent=pStart; pCurrent; pCurrent=pCurrent->next) {
     thisStr = pCurrent->me->Str();
     if ( thisStr!=nil && strcmp( thisStr, s )==0 )
	 return i;
     i++;
 }
 return 0;
}


unsigned gList::FindFirst (const char* s,
			   unsigned strPos,
			   eFindCriteria findCriteria)
{
 // Returns the index of first occurrence obeying 'findCriteria'
 // for the string 's' in list (on position strPos).
 gList posL;
 if ( s==nil || s[0]==0 ) return 0;
 return thisFind( s, strPos, findCriteria, true, posL );
}


unsigned gList::Find (const char* s,
		      unsigned strPos,
		      eFindCriteria findCriteria)
{
 gList posL;
 if ( s==nil || s[0]==0 ) return 0;
 return thisFind( s, strPos, findCriteria, false, posL );
}


unsigned gList::FindAny (const char* s,
			 unsigned strPos,
			 eFindCriteria findCriteria,
			 gList& posL)
{
 if ( s==nil || s[0]==0 ) return 0;
 return thisFind( s, strPos, findCriteria, false, posL );
}


int gList::Tidy (int value, bool basedOnValue)
{
 gList trash;
 gElem* pNext;
 gElem* pLast( nil );
 gElem* pDesc( StartPtr() );
 unsigned deletedElems( 0 );

 if ( basedOnValue ) {
     for ( ; pDesc; pDesc=pNext) {
	 pNext = pDesc->next;
	 if ( pDesc->me->iValue==value ) {
	     // Delete this element!
	     deletedElems++;
	     trash.AppendObject( pDesc );
	     if ( pLast ) {
		 pLast->next = pNext;
	     }
	     else {
		 // The first element of the list
		 pStart = pNext;
	     }
	     if ( pNext ) {
		 pNext->prev = pLast;
	     }
	     else {
		 pEnd = pLast;
	     }
	 }
	 else {
	     pLast = pDesc;
	 }
     }
 }
 else {
     size = 0;
     for (gElem* ptr=pDesc; ptr; ptr=ptr->next) {
	 size++;
     }
 }

 ASSERTION(size >= deletedElems, "Tidy()");
 size -= deletedElems;
 pCurrent = pStart;  // Just any valid placeholder
 return (int)deletedElems;
}


unsigned gList::Add (int v)
{
 gInt* newObj;
 newObj = new gInt(v);
 thisAppend( newObj );
 return size;
}


unsigned gList::Add (unsigned v)
{
 gUInt* newObj;
 newObj = new gUInt(v);
 thisAppend( newObj );
 return size;
}


unsigned gList::Add (gString& copy)
{
 return Add( copy.Str() );
}


unsigned gList::Add (const char* s)
{
 return Add( (t_uchar*)s );
}


unsigned gList::Add (t_uchar* s)
{
 gString* newObj;
 newObj = new gString( s );
 thisAppend( newObj );
 return size;
}


gList& gList::AddFrom (gElem* pElem, bool doUpChar)
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
     AppendObject( newObj );
     thisElem = thisElem->next;
 }
 return *this;
}


gList& gList::CopyFrom (gElem* pElem, bool doUpChar)
{
 thisDelete();
 return AddFrom( pElem, doUpChar );
}


gList& gList::CopyList (gList& aL, bool doUpChar)
{
 thisDelete();
 return CopyFrom( aL.StartPtr() );
}


unsigned gList::DeleteString (gString& s)
{
 // Returns 0 if no string found, 1 otherwise
 // Delete the first string found.

 unsigned pos = Match( s.Str() );
 if ( pos==0 ) return 0;
 Delete( pos, pos );
 return 1;
}


unsigned gList::DeleteStrings (gString& s)
{
 // Returns number of elements deleted.
 // Delete all strings found

 unsigned count=0, pos;
 while ( (pos = Match( s.Str() ))>0 ) {
     Delete( pos, pos );
     count++;
 }
 return count;
}


int gList::InsertObject (gStorage* ptrNew)
{
 // Returns -1 if no object was requested to be inserted,
 // or on error;
 // 1 if list was empty, or 2 otherwise (if ptrNew was inserted on the list.)

 // This method inserts an object in the first position of the list.

 if ( ptrNew==nil ) return -1;

 if ( size==0 ) {
     AppendObject( ptrNew );
     pCurrent = EndPtr();
     return 1;
 }

 gElem* keepStart( pStart );
 gElem* newElem( new gElem( ptrNew ) );
 if ( newElem==nil ) return -1;

 pStart = newElem;
 newElem->next = keepStart;
 keepStart->prev = newElem;
 size++;

 pCurrent = newElem;
 return 2;
}


int gList::InsertHere (gStorage* ptrNew)
{
 gElem* ptrElem( StartPtr() );
 gElem* ptrFirst( ptrElem );
 gElem* newElem( nil );

 if ( ptrNew==nil ) return -1;

 if ( ptrElem==nil || pCurrent==nil ) {
     AppendObject( ptrNew );
     pCurrent = EndPtr();
     return 1;
 }

 ptrElem = pCurrent;

 if ( ptrElem==ptrFirst ) {
     InsertObject( ptrNew );
     return 1;
 }

 newElem = new gElem( ptrNew );
 if ( newElem==nil ) return -1;

 pCurrent = ptrElem->prev;
 ptrElem->prev = newElem;
 newElem->next = ptrElem;
 ptrElem = pCurrent;
 ASSERTION(ptrElem,"ptrElem");
 if ( ptrElem ) {
     ptrElem->next = newElem;
 }
 newElem->prev = ptrElem;

 pCurrent = newElem;
 size++;
 return 3;
}


int gList::InsertBefore (gStorage* ptrNew)
{
 gElem* ptrElem( StartPtr() );

 if ( ptrNew==nil ) return -1;

 if ( ptrElem==nil || pCurrent==nil ) {
     AppendObject( ptrNew );
     pCurrent = EndPtr();
     return 1;
 }
 if ( ptrElem==pCurrent ) {
     InsertObject( ptrNew );
     return 1;
 }
 return InsertHere( ptrNew );
}


int gList::InsertOrdered (gStorage* ptrNew)
{
 return thisInsertOrdered( ptrNew, 0 );
}


int gList::InsertOrderedUnique (gStorage* ptrNew)
{
 return thisInsertOrdered( ptrNew, 1 );
}


t_uchar* gList::ToString (const t_uchar* uBuf)
{
 return gListGeneric::ToString( uBuf );
}


bool gList::SaveGuts (FILE* f)
{
 return gListGeneric::SaveGuts( f );
}


bool gList::RestoreGuts (FILE* f)
{
 return gListGeneric::RestoreGuts( f );
}


char* gList::operator[] (int idx)
{
 if ( idx<0 ) return nil;
 return Str( (unsigned)idx );
}


void gList::Show (bool doShowAll)
{
 unsigned i, n( N() );

 if ( doShowAll ) iprint("(");

 for (i=1; i<=n; i++) {
     gStorage* aObj;
     aObj = GetElementPtr(i)->me;
     ASSERTION(aObj!=nil,"aObj!=nil");
     if ( doShowAll ) {
	 aObj->Show( true );
	 iprint("%s",i==n?"\0":" ");
     }
     else {
	 iprint("%s%s",Str(i),i==n?"\0":" ");
     }
 }
 if ( doShowAll ) iprint(")\n");
}


unsigned gList::thisFind (const char* s,
			  unsigned strPos,
			  eFindCriteria findCriteria,
			  bool doStopOnFirst,
			  gList& posL)
{
 bool isOk;
 unsigned i( 1 ), pos, posFirst( 0 );
 gElem* ptrElem( pStart );
 gString* ptrStr;

 pCurrent = nil;

 for ( ; ptrElem; i++, ptrElem = ptrElem->next) {
     ptrStr = (gString*)ptrElem->me;
     pos = ptrStr->Find( s );
     if ( pos==0 ) continue;

     switch ( findCriteria ) {
     case e_FindExactPosition:
	 isOk = pos==strPos;
	 break;
     case e_FindFromPosition:
	 isOk = pos>=strPos;
	 break;
     case e_FindBeforePosition:
	 isOk = pos<strPos;
	 break;
     default:
	 return 0;
     }

     if ( isOk ) {
	 pCurrent = ptrElem;
	 if ( posFirst==0 ) posFirst = i;
	 posL.Add( i );
	 if ( doStopOnFirst ) return posFirst;
     }
 }
 return posFirst;
}


int gList::thisInsertOrdered (gStorage* ptrNew, int isUnique)
{
 int result( -1 );
 gElem* ptrElem( StartPtr() );
 gElem* ptrNext;
 gStorage* ptrStg;

 pCurrent = ptrElem;
 if ( ptrNew==nil ) return -1;

 for ( ; ptrElem; ) {
     ptrStg = ptrElem->me;
     ptrNext = ptrElem->next;

     if ( ptrStg ) {
	 pCurrent = ptrElem;
	 result = ptrStg->Compare( *ptrNew );
	 DBGPRINT_MIN("DBG: compare(%s,%s) = %d\n",ptrStg->Str(),ptrNew->Str(),result);

	 if ( result==0 ) {
	     if ( isUnique!=0 ) return -1;  // Not inserted
	 }
	 else {
	     if ( result>0 ) {
		 result = InsertBefore( ptrNew );
		 return result;
	     }
	 }
     }
     ptrElem = ptrNext;
 }

 AppendObject( ptrNew );
 pCurrent = EndPtr();
 return 0;
}
////////////////////////////////////////////////////////////

