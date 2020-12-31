// filters.cpp


#include "filters.h"

////////////////////////////////////////////////////////////
gList* filter_from_string (gString& filter)
{
 gList* newObj( new gList );
 gList* inOrOut;
 gList* ptrOut;
 gElem* pItem;
 gString* myItem;
 t_int16 insOrOuts( 0 );
 const char* strSeparator( ";" );

 ASSERTION(newObj,"newObj");

 inOrOut = new gList;
 newObj->AppendObject( inOrOut );  // include
 inOrOut = new gList;
 ptrOut = inOrOut;
 newObj->AppendObject( inOrOut );  // exclude
 ASSERTION(newObj->N()==2,"2");

 if ( filter.Find( "include:" ) ) {
     insOrOuts |= 1;
 }
 if ( filter.Find( "exclude:" ) ) {
     insOrOuts |= 2;
 }

 if ( insOrOuts ) {
     gString sCopy;

     if ( insOrOuts & 2 ) {
	 inOrOut = ptrOut;
	 sCopy.Set( strstr( filter.Str( filter.Find( "exclude:" ) ), ":" )+1 );

	 gParam several( sCopy, strSeparator );

	 for (pItem=several.StartPtr(); pItem; pItem=pItem->next) {
	     myItem = (gString*)pItem->me;
	     if ( myItem->Find( "..." )+3-1==myItem->Length() ) {
		 myItem->Delete( myItem->Find( "..." ) );
		 myItem->Add( "*" );
		 inOrOut->Add( *myItem );
		 inOrOut->EndPtr()->iValue = myItem->Length()-1;
	     }
	     else {
		 if ( myItem->Find( "..." )==1 ) {
		     myItem->Delete( 1, 2 );
		     (*myItem)[ 1 ] = '*';
		     inOrOut->Add( *myItem );
		     inOrOut->EndPtr()->iValue = -1;
		     inOrOut->EndPtr()->me->iValue = myItem->Length()-1;
		 }
		 else {
		     inOrOut->Add( *myItem );
		 }
	     }
	 }
     }// outs
 }

 return newObj;
}


int is_filtered_string (gString& aStr, gList& insAndOuts)
{
 gList* outs;
 gElem* pTest;
 char* strExp;
 int whence;
 int rule( 0 );
 unsigned pos( 0 );
 bool tic( false );

 DBGPRINT("is_filtered_string('%s', insAndOuts#%u\n",
     aStr.Str(), insAndOuts.N());

 ASSERTION(insAndOuts.N()>=2,"insAndOuts?");
 outs = (gList*)insAndOuts.StartPtr()->next->me;

 for (pTest=outs->StartPtr(); pTest; pTest=pTest->next) {
     rule++;
     strExp = pTest->Str();
     whence = pTest->iValue;

     switch ( whence ) {
     case -1:
	 // something like		*pattern
	 pos = aStr.FindBack( strExp+1 );
	 if ( pos > 0 ) {
	     unsigned afterPos( pos + ((pTest->me->Length()) - 1) );
	     // pTest->me is the length excluding '*'
	     tic =  (afterPos==aStr.Length()+1);

	     DBGPRINT("is_filtered_string('%s', insAndOuts#%u), exclude={%s}, pos=%u,findPos=%u len=%u, tic=%d, rule=%d\n",
		      aStr.Str(), insAndOuts.N(),
		      strExp,
		      pos, afterPos, aStr.Length(),
		      tic,
		      rule);
	 }
	 break;

     case 0:
	 tic = aStr.Match( strExp );
	 break;

     default:
	 tic = strncmp( aStr.Str(), strExp, whence )==0;
	 break;
     }

     if ( tic ) return rule;  // a positive number of the rule that matched!
 }

 return 0;
}

////////////////////////////////////////////////////////////

