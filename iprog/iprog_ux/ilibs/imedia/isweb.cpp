// isweb.cpp

// Simplified, and highly optimized HTML filter

#include <string.h>

#include "isweb.h"
////////////////////////////////////////////////////////////
// Static members
// ---------------------------------------------------------
// <>
////////////////////////////////////////////////////////////

int sHtmlStatus::AddHttpEquiv (gString* newStr, const char* strTag, int iLine, int mask)
{
 unsigned pos;
 gElem* ptrLast;
 gElem* ptrISO( nil );
 gString sNew( (char*)strTag );
 int isoCharset( 0 );

 sNew.TrimRight();
 if ( sNew[ sNew.Length() ]=='>' ) {
     sNew.Delete( sNew.Length() );
     sNew.TrimRight();
 }

 pos = sNew.Find( "<META ", true );
 if ( pos ) {
     sNew.Delete( pos+1, pos+5 );
 }

 if ( iLine==0 ) {
     iLine = -99;
     sNew.UpString();
 }
 sNew.ConvertChrTo( '=', ' ' );
 sNew.Trim();
 if ( sNew.IsEmpty() ) return -2;

 gString sUp( sNew );
 sUp.UpString();

 if ( sUp[1]=='<' ) {
     sUp.Delete( 1, 1 );
     sUp.TrimLeft();
 }

 if ( metaHttpEquiv.FindFirst( sUp.Str(), 1, e_FindFromPosition ) ) {
     return -1;
 }

 gParam charsetKeys( sUp, "CHARSET" );
 gParam mainKeys( sUp, " " );

 metaHttpEquiv.Add( sNew );
 ptrLast = metaHttpEquiv.EndPtr();
 ptrLast->iValue = iLine;
 ptrLast->me->iValue = mask;

 if ( charsetKeys.N() ) {
     gString sCharset( charsetKeys.Str(2) );
     gList keyCopy;

     keyCopy.CopyList( mainKeys, true );
     pos = keyCopy.Match( "CONTENT" );

     metaHttpEquiv.Add( charsetKeys.Str(1) );
     metaHttpEquiv.EndPtr()->iValue = 101;
     metaHttpEquiv.Add( sCharset );
     ptrISO = metaHttpEquiv.EndPtr();
     ptrISO->iValue = 102;

     if ( pos ) {
	 sNew.Set( keyCopy.Str( pos+1 ) );

	 if ( sNew.Find( '/' ) || sNew.Find( ';' ) ) {
	     gParam contentTypes( sNew, "/" );
	     for ( ; contentTypes.N(); ) {
		 gString sContentSlash( contentTypes.Str(1) );
		 sContentSlash.ConvertChrTo( ';', ' ' );
		 sContentSlash.Trim();
		 metaHttpEquiv.Add( sContentSlash );
		 metaHttpEquiv.EndPtr()->iValue = 103;
		 contentTypes.Delete( 1, 1 );
	     }
	 }
     }

     gParam stripIsoCode( sCharset, "-" );
     if ( stripIsoCode.N() ) {
	 int isoTableCode( atoi( stripIsoCode.Str(2) ) );
	 int isoSubCode( atoi( stripIsoCode.Str(3) ) );
	 isoCharset = isoTableCode * 100 + isoSubCode;

	 SetAnyCharset( sCharset, stripIsoCode.Str(1), isoCharset );

	 if ( ptrISO ) {
	     ptrISO->iValue = isoCharset;
	 }
     }
 }
 return mask;
}


int sHtmlStatus::SetAnyCharset (gString& sCharset, const char* strCharset, int isoCharset, int mask)
{
 if ( isoCharset<0 ) return -1;

 DBGPRINT_MEA("DBG: builtCharset[%s,%s], mask=%d, isoCharset=%d, current ValidCharset? %c, new is valid? %c\n",
	      sCharset.Str(),
	      strCharset,
	      mask,
	      isoCharset,
	      ISyORn( ValidCharset() ),
	      ISyORn( isoCharset>0 ));

 if ( isoCharset ) {
     switch ( isoCharset ) {
     case 800:	// UTF-8
     case 1600:	// UTF-16
	 if ( strcmp( strCharset, "UTF" ) )
	     return -2;
	 break;

     default:
	 break;
     }

     if ( mask==0 ) {
	 if ( ValidCharset()==false ) {
	     builtCharset = isoCharset;
	 }
     }
 }
 return builtCharset;
}

////////////////////////////////////////////////////////////
gWebChassis::gWebChassis ()
    : trimLeftAlways( true ),
      suppressScript( false ),
      keywordBuild( true ),
      scriptState( 0 ),
      logStream( stderr ),
      ptrLastStr( nil )
{
}


gWebChassis::~gWebChassis ()
{
}


gElem* gWebChassis::AddMe (gStorage* newObj, int iLine, int mask)
{
 gElem* ptrLast;
 gString* newStr;

 ASSERTION(newObj,"AddMe (1)");

 if ( newObj->IsString() ) {
     newStr = (gString*)newObj;
     if ( mask<=ISW_MASK_NO_STR_FIX || htmlStatus.quoteState ) {
     }
     else {
	 newStr->Trim();

	 if ( newStr->Match( ">" ) ) {
	     htmlStatus.metaEquiv = 0;
	 }
	 else {
	     if ( newStr->Find( "<META HTTP-EQUIV", true ) ) {
		 htmlStatus.AddHttpEquiv( newStr, newStr->Str()+6, 0, mask );
		 htmlStatus.metaEquiv++;
	     }
	 }
     }

     if ( htmlStatus.metaEquiv ) {
	 if ( newStr->Find( "charset=" ) ) {
	     htmlStatus.AddHttpEquiv( newStr, newStr->Str(), iLine, mask );
	 }
     }
 }

 AppendObject( newObj );
 ptrLast = EndPtr();
 ASSERTION(ptrLast,"AddMe (2)");
 ptrLast->iValue = iLine;
 ISW_DEBUG("AddMe( {%d:%s}, %d )\n",
	   newObj->iValue,
	   newObj->Str(),
	   mask);
 ptrLastStr = newObj;

 if ( mask ) {
     ptrLastStr->iValue = mask;
 }
 return ptrLast;
}


int gWebChassis::AddToList (gList& list, char* aStr, int iLine, int optValue)
{
 gElem* ptrLast;
 gString* ptrStr;

 list.Add( aStr );
 ptrLast = list.EndPtr();
 ptrStr = (gString*)ptrLast->me;

 ptrLast->iValue = iLine;
 ptrStr->iValue = optValue;
 return 0;
}


int gWebChassis::AddToList (gList& list, gString& s, int iLine, int optValue)
{
 char* aStr( s.Str() );
 return AddToList( list, aStr, iLine, optValue );
}


int gWebChassis::AddToListRemnant (gList& list, gString& s, int iLine, int optValue)
{
 if ( s.IsEmpty() ) return -1;
 return AddToList( list, s, iLine, optValue );
}


int gWebChassis::AddHtmlString (gList& list, gString& s, int iLine, int mask)
{
 int countChanges( 0 );
 unsigned len( s.Length() );
 t_uchar uChr;
 const t_uchar middleDot( 0xB7 );

 ASSERTION(mask==0,"mask non 0");

 sLastAdded = s;

 if ( suppressScript ) {
     if ( scriptState ) {
	 return -2;
     }
 }

 if ( htmlStatus.IsCharsetUTF() ) {
 }
 else {
     for (unsigned idx=1; idx<=len; idx++) {
	 uChr = s[ idx ];
	 switch ( uChr ) {
	 case middleDot:
	     s[ idx ] = '.';
	     countChanges++;
	     break;

	 default:
	     if ( uChr!=128 && uChr>=127 && uChr<160 ) {
		 countChanges++;
		 s[ idx ] = '?';
		 break;
	     }
	 }
     }
 }

 list.Add( s );
 list.EndPtr()->me->iValue = mask;

 if ( keywordBuild ) {
     thisAddHtml( new gString( s ), iLine );
 }
 return countChanges;
}


int gWebChassis::AdjustScriptLines (gString& s, int posStart, int posEnd, int mask)
{
 s.Trim();
 thisSpecialUpString( s, mask );

 if ( posStart ) {
     scriptState = 1;
 }
 else {
     if ( posEnd ) {
	 scriptState = 0;
     }
 }

 return 0;
}


int gWebChassis::Finit (gList& list)
{
 int error;

 ISW_DEBUG("Finit( #%u )\n",list.N());
 ISW_DEBUG("htmlStatus: quoteState=%d, single=%d, double=%d\n",
	   htmlStatus.quoteState,
	   htmlStatus.singleQuoted,
	   htmlStatus.doubleQuoted);

#ifdef DEBUG
 for (unsigned dbgIdx=1; dbgIdx<=htmlStatus.metaHttpEquiv.N(); dbgIdx++) {
     printf("HTTP-EQUIV:%d: %d, %d {%s}\n",
	    dbgIdx,
	    htmlStatus.metaHttpEquiv.GetElement( dbgIdx ).iValue,
	    htmlStatus.metaHttpEquiv.GetObjectPtr( dbgIdx )->iValue,
	    htmlStatus.metaHttpEquiv.Str( dbgIdx ));
 }
 printf("Charset code %d\n%s\n",
	htmlStatus.builtCharset,
	htmlStatus.metaHttpEquiv.N() ? "" : " HTTP-EQUIV empty\n");
#endif

 error = htmlStatus.quoteState!=0;

 if ( error ) {
     AddMe( new gString( htmlStatus.singleQuoted ? ISW_SINGLE_QUOTE : '"' ),
	    -1,
	    ISW_MASK_ERR_NOTE );
 }

 return error;
}


int gWebChassis::KeydumpFlush (gList& items)
{
 gList* newList( new gList );

 ASSERTION(newList,"newList");
 newList->CopyList( items );
 built.AppendObject( newList );
 return 0;
}


int gWebChassis::thisSpecialUpString (gString& s, int& quoteState)
{
 char* aStr( s.Str() );
 char chr;

 for ( ; (chr = *aStr)!=0; aStr++) {
     if ( chr=='"' ) {
	 quoteState = quoteState==0;
     }
     else {
	 if ( chr=='\'' ) {
	     quoteState = quoteState==0;
	 }
	 else {
	     if ( quoteState ) {
	     }
	     else {
		 if ( chr>='a' && chr<='z' ) {
		     *aStr = chr-32;
		 }
	     }
	 }
     }
 }
 return 0;
}


int gWebChassis::thisAddHtml (gString* words, int iLine)
{
 int idx( 1 ), len;
 int oldQuoteState( 0 ), newQuoteState( 0 );
 int safePos( -1 );
 unsigned pos1;
 unsigned pos2;
 t_uchar uChr;
 gList raw;
 gString sQuote, sLine;
 gString* ptrStr;
 gElem* ptrElem;

 if ( words==nil ) {
     isw_print( 0, "Out of memory!\n" );
 }

 pos1 = words->Find( "<PRE>", true );
 pos2 = words->Find( "</PRE>", true );
 len = (int)words->Length();

 ISW_DEBUG("thisAddHtml: %d: %s\n\n",
	   iLine,
	   words->IsEmpty() ? "{EMPTY}" : words->Str());

 if ( pos1 ) {
     htmlStatus.openPre = true;
 }
 if ( pos2>pos1 ) {
     htmlStatus.openPre = false;
 }

 if ( htmlStatus.openPre ) {
     // Simplified, this ain't complete!
     words->iValue = ISW_MASK_DONT_TOUCH;
     AddMe( words, iLine );
     return ISW_MASK_DONT_TOUCH;
 }

 for ( ; idx<=len; idx++) {
     uChr = (*words)[ idx ];

     switch ( uChr ) {
     case '<':
	 if ( htmlStatus.quoteState==0 ) {
	     htmlStatus.keyState++;
	 }
	 break;

     case '>':
	 if ( htmlStatus.quoteState==0 ) {
	     htmlStatus.keyState = 0;
	 }
	 break;

     case '"':
     case ISW_SINGLE_QUOTE:
	 if ( htmlStatus.keyState<=0 ) break;

	 oldQuoteState = htmlStatus.quoteState;
	 if ( uChr=='"' ) {
	     if ( htmlStatus.quoteState ) {
		 if ( htmlStatus.doubleQuoted ) {
		     htmlStatus.doubleQuoted = 0;
		     htmlStatus.quoteState = 0;
		 }
	     }
	     else {
		 safePos = idx;
		 htmlStatus.quoteState = iLine;
		 htmlStatus.doubleQuoted = 1;
	     }
	 }
	 else {
	     if ( uChr==ISW_SINGLE_QUOTE ) {
		 if ( htmlStatus.quoteState ) {
		     if ( htmlStatus.singleQuoted ) {
			 htmlStatus.singleQuoted = 0;
			 htmlStatus.quoteState = 0;
		     }
		 }
		 else {
		     safePos = idx;
		     htmlStatus.quoteState = iLine;
		     htmlStatus.singleQuoted = 1;
		 }
	     }
	     // was a single quote...
	 }
	 newQuoteState = htmlStatus.quoteState;
	 break;

     default:
	 break;
     }

     if ( htmlStatus.quoteState ) {
	 sQuote.Add( uChr );
     }
     else {
	 if ( oldQuoteState==newQuoteState ) {
	     raw.AppendObject( new gString( (char)uChr ) );
	 }
	 else {
	     sQuote.Add( uChr );
	     raw.AppendObject( new gString( sQuote ) );
	     raw.EndPtr()->me->iValue = ISW_MASK_QUOTED;
	     sQuote.Delete();
	 }
     }

     oldQuoteState = newQuoteState;
 }// end FOR (idx)

 if ( safePos>=1 && htmlStatus.quoteState ) {
     htmlStatus.logIncompleteQuote.Add( words->Str() + safePos - 1 );
     htmlStatus.logIncompleteQuote.EndPtr()->iValue = iLine;
     htmlStatus.logIncompleteQuote.EndPtr()->me->iValue = safePos;
 }

 for (ptrElem=raw.StartPtr();
      ptrElem;
      ptrElem=ptrElem->next) {
     ptrStr = (gString*)ptrElem->me;

     switch ( ptrStr->Length() ) {
     case 0:
	 continue;

     case 1:
	 sLine.Add( ptrStr->Str() );
	 break;

     default:
	 if ( sLine.IsEmpty()==false ) {
	     AddMe( new gString( sLine ), iLine );
	 }
	 sLine.Delete();

	 AddMe( new gString( ptrStr->Str() ), iLine, ISW_MASK_QUOTED );
	 break;
     }
 }// end FOR (raw)

 if ( sLine.IsEmpty() ) {
     if ( len==0 ) {
	 AddMe( new gString, iLine, ISW_MASK_EMPTY );
     }
 }
 else {
     ptrStr = new gString( sLine );
     AddMe( ptrStr, iLine );
 }

 delete words;
 return 0;
}

////////////////////////////////////////////////////////////
// isw_simple_html_filter (simplified html nested filter)
// ---------------------------------------------------------
int isw_simple_html_filter_list (gList& inHTML, gList& outHTML, gWebChassis& chassis)
{
#ifdef DEBUG
 gElem* ptrDbg( inHTML.StartPtr() );

 for ( ; ptrDbg; ptrDbg=ptrDbg->next) {
     if ( ptrDbg->me->iValue ) {
     printf("COM line %d, %d: %s\n",
	    ptrDbg->iValue,
	    ptrDbg->me->iValue,
	    ptrDbg->Str());
     }
 }
 for (ptrDbg=inHTML.StartPtr(); ptrDbg; ptrDbg=ptrDbg->next) {
     if ( ptrDbg->me->iValue==0 ) {
     printf("HTM line %d, %d: %s\n",
	    ptrDbg->iValue,
	    ptrDbg->me->iValue,
	    ptrDbg->Str());
     }
 }
#endif //DEBUG

 gElem* ptrElem( inHTML.StartPtr() );
 gString* thereLine;
 int posStart( -1 ), posEnd( -1 );

 for ( ; ptrElem; ptrElem=ptrElem->next) {
     thereLine = (gString*)ptrElem->me;
     if ( thereLine->iValue==0 ) {
	 if ( chassis.trimLeftAlways ) {
	     thereLine->TrimLeft();
	     posStart = (int)thereLine->Find( "<SCRIPT", true );
	     posEnd = (int)thereLine->Find( "</SCRIPT", true );
	     if ( posStart==1 ||
		  posEnd==1 ) {
		 chassis.AdjustScriptLines( *thereLine, posStart, posEnd );
	     }
	 }

	 DBGPRINT_MIN("AddStr %d {%s}\n",ptrElem->iValue,thereLine->Str());

	 chassis.AddHtmlString( outHTML, *thereLine, ptrElem->iValue );
	 //// line above is similar to:  outHTML.Add( *thereLine )
	 outHTML.EndPtr()->iValue = ptrElem->iValue;

	 DBGPRINT_MIN("DBG: line %d, errorCode: %d, charset: %d\n",
		      ptrElem->iValue,
		      errorCode,
		      chassis.htmlStatus.CharsetCode());
     }
 }

 chassis.Finit( outHTML );
 return 0;
}


int isw_simple_html_filter_file (const char* strIn, gList& listHTML, gWebChassis& chassis)
{
 int error( 0 );
 int iLine( 0 );
 int state( 0 );
 gFileFetch inF( (char*)strIn );
 gElem* pElem( inF.aL.StartPtr() );
 gElem* ptrLast;
 gString* aLine;
 char* strLine;
 unsigned posOpenedComment( 0 );
 unsigned posClosedComment( 0 );
 gString sLeft, sRight;
 gList preList;

 for ( ; pElem; pElem=pElem->next) {
     chassis.AddToList( preList, strLine = pElem->Str(), ++iLine );
     ptrLast = preList.EndPtr();
     aLine = (gString*)ptrLast->me;

     posClosedComment = aLine->Find( "-->" );
     posOpenedComment = aLine->Find( "<!--" );

     if ( posOpenedComment ) {
	 aLine->iValue = state;

	 state = posClosedComment>0 ? -1 : 1;
	 sRight.CopyFromTo( *aLine, posOpenedComment );
	 aLine->Delete( posOpenedComment );
	 aLine->TrimRight();

	 chassis.AddToListRemnant( preList, sRight, iLine, state );

	 state = state==1;

	 if ( posClosedComment ) {
	     // Check if there is anything remaining, for example:
	     //		<!-- SpyLOG --></td>
	     //	...we want to preserve	</td>

	     posClosedComment = sRight.Find( "-->" );
	     if ( posClosedComment ) {
		 gString* thereLine( (gString*)preList.EndPtr()->me );
		 sRight.CopyFromTo( *thereLine, posClosedComment+3 );
		 thereLine->Delete( posClosedComment+3 );

		 sRight.TrimLeft();
		 chassis.AddToListRemnant( preList, sRight, iLine );
	     }
	 }
     }
     else {
	 if ( posClosedComment ) {
	     sLeft.CopyFromTo( *aLine, posClosedComment );
	     aLine->Delete( posClosedComment );
	     aLine->TrimRight();
	     aLine->iValue = -2;

	     sRight.CopyFromTo( sLeft, 4 );
	     sRight.TrimLeft();
	     sLeft.Delete( 4 );

	     chassis.AddToListRemnant( preList, sLeft, iLine, -3 );
	     chassis.AddToListRemnant( preList, sRight, iLine, 0 );

	     state = 0;
	 }
	 else {
	     aLine->iValue = state;
	 }
     }
 }

 error = isw_simple_html_filter_list( preList, listHTML, chassis );
 DBGPRINT("DBG: isw_simple_html_filter_list returned %d (#%u, then: %u)\n",
	  error,
	  preList.N(), listHTML.N());
 return error;
}


int isw_html_keydump (gList& listHTML, gWebChassis& chassis)
{
 gElem* ptrElem( listHTML.StartPtr() );
 gString* aStr;
 int iLine( -1 ), prevLine( -1 );
 int thisMask, lastMask( -1 );
 gList items;

 const char* strEnd( "\n" );
 const char* strFinit( strEnd );

 for ( ; ptrElem; ptrElem=ptrElem->next) {
     const char* strBefore( "\0" );
     aStr = (gString*)ptrElem->me;
     ASSERTION(aStr,"listHTML?!");

     iLine = ptrElem->iValue;
     thisMask = aStr->iValue;

     if ( iLine==prevLine ) {
	 strFinit = "";
	 if ( lastMask==ISW_MASK_QUOTED ) {
	     if ( aStr->Match( "/>" ) || (*aStr)[ 1 ]=='>' ) {
	     }
	     else {
		 strBefore = " ";
	     }
	 }
	 lastMask = aStr->iValue;
     }
     else {
	 strFinit = strEnd;
	 lastMask = -1;
     }

     if ( thisMask!=-2 ) {
	 // Not an empty line

	 gString text( aStr->Length()+20, '\0' );
	 sprintf( text.Str(), "%s%s%s",
		  strBefore,
		  strFinit,
		  aStr->Str() );
	 items.Add( text );
     }
     else {
	 if ( items.N() ) {
	     chassis.KeydumpFlush( items );
	     items.Delete();
	 }
	 chassis.built.Add( "" );
     }

     prevLine = iLine;
 }

 chassis.KeydumpFlush( items );

 return 0;
}
////////////////////////////////////////////////////////////

