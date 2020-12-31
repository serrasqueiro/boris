// iJson.cpp


#include <unistd.h>

#include "iJson.h"

////////////////////////////////////////////////////////////
// Static members
// ---------------------------------------------------------
// <>
////////////////////////////////////////////////////////////
iJSON::iJSON (char* strName)
    : gList( strName ),
      state(  0 ),
      finito( e_Idle ),
      y( 0 ),
      x( 0 ),
      lastChr( 0 )
{
}


iJSON::~iJSON ()
{
}


void iJSON::Reset ()
{
 Delete();
 Start();
}


void iJSON::Start ()
{
 state = 0;
 finito = e_Idle;
 x = 0;
 y = 1;
 lastChr = 0;
 sKey.Reset();
 sValue.Reset();
 hints.Reset();
}


int iJSON::AddChar (t_uchar aChr)
{
 int quote( state % 100 );

 switch ( aChr ) {
 case '\r':
     return -2;

 case '\n':
     y++;
     x = 0;
     if ( quote ) {
	 AddWarn( 501, "New line during quote" );
     }
     break;

 default:
     x++;

     switch ( aChr ) {
     case '"':
	 if ( quote ) {
	     state -= quote;
	 }
	 else {
	     state++;
	 }
	 break;

     case ':':
	 if ( quote==0 ) {
	     if ( lastChr=='"' ) {
		 printf("state: %d\t[%s]\n",
			state,
			sKey.Str());
		 sKey.Delete();
		 finito = e_Finit;
	     }
	     else {
		 AddWarn( 502, "Invalid ':'" );
	     }
	 }
	 break;

     default:
	 if ( quote ) {
	     switch ( finito ) {
	     case e_Start:
		 sKey.Add( aChr );
		 break;

	     case e_Finit:
		 sValue.Add( aChr );
		 break;

	     case e_Idle:
	     default:
		 AddError( 104, "Bogus" );
		 break;
	     }
	 }
	 else {
	     if ( aChr=='{' ) {
		 finito = e_Start;
		 sValue.Reset();
		 state += 100;
		 DBGPRINT_MIN("Line: %d.%d: state\t%d >>>\n",
			      y, x,
			      state);
	     }
	     else {
		 if ( aChr=='}' ) {
		     printf("state: %d\t[%s]\n",
			state,
			sValue.Str());

		     if ( state>=100 ) {
			 state -= 100;
			 DBGPRINT_MIN("Line: %d.%d: state\t%d <<<\n",
				      y, x,
				      state);
		     }
		     else {
			 AddError( 102, "Bad closing bracket '}'" );
		     }
		 }
	     }
	 }
	 break;
     }// end CASE aChr...

     lastChr = aChr;
     break;
 }
 return 0;
}


int iJSON::AddLine (const char* aStr)
{
 t_uchar aChr;

 if ( aStr ) {
     for ( ; (aChr = (t_uchar)*aStr)!=0; aStr++) {
	 AddChar( aChr );
     }
 }
 x = 0;
 return 0;
}


int iJSON::AddError (int aErrNo, const char* strMsg)
{
 gString* newStr;
 int len;

 if ( aErrNo==0 ) return 0;

 len = strMsg ? strlen( strMsg ) : 0;
 newStr = new gString( len + 64, '\0' );
 sprintf( newStr->Str(), "line:%d.%d: %s", y, x, strMsg );
 hints.AppendObject( newStr );
 hints.EndPtr()->me->iValue = aErrNo;
 return aErrNo;
}


int iJSON::AddWarn (int aErrNo, const char* strMsg)
{
 ASSERTION(aErrNo>=0,"aErrNo");
 return AddError( -aErrNo, strMsg );
}


int iJSON::Flush ()
{
 if ( state!=0 ) {
     DBGPRINT("DBG: json Flush, state: %d\n",state);
     AddError( 105, "Incomplete" );
     return 1;
 }
 return 0;
}

////////////////////////////////////////////////////////////
iJSON* ijson_get_from_handle (int fd)
{
 iJSON* son( nil );
 t_uchar aChr( 0 );

 ASSERTION(fd!=-1,"fd!=-1");
 son = new iJSON;

 if ( son ) {
     son->Start();
     for ( ; read( fd, &aChr, 1 )==1; ) {
	 son->AddChar( aChr );
     }
     son->Flush();
 }
 else {
     ASSERTION_FALSE("son");
 }
 return son;
}

////////////////////////////////////////////////////////////

