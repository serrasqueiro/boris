// isubs.cpp

#include "lib_isubs.h"
#include "isubs.h"

////////////////////////////////////////////////////////////
// Globals
// ---------------------------------------------------------
// <>


////////////////////////////////////////////////////////////
SubEntry::SubEntry (int value)
{
}


SubEntry::SubEntry (gString& talk, int value)
{
}


SubEntry::~SubEntry ()
{
}


int SubEntry::ConvSubsFromTo (gString& line, char separator)
{
 static char dum[ 4 ];
 static char first, aChr;
 static char* dig;
 static char* subTimeStr;
 static bool dot;
 int idx( 0 ), hasDot( 0 );
 int ms, t[ 2 ];
 int timeError( 0 );

 t[ 0 ] = t[ 1 ] = 0;

 dum[ 0 ] = separator;
 gParam two( line, dum );

 if ( two.N()<2 )
     return 0;

 first = two.Str( 2 )[ 0 ];
 if ( two.N()>2 ) {
     if ( first<'0' || first>'9' ) two.Delete( 2, 2 );
 }

 for ( ; idx<2; idx++) {
     subTimeStr = dig = two.Str( idx+1 );
     if ( dig[ 0 ] ) {
	 // Remove string when first blank (or non valid) is found, e.g.
	 //		'00:23:01,240 --'
	 for (dot=0; (aChr = *dig)!=0; ) {
	     if ( aChr=='-' || aChr<=' ' ) {
		 *dig = 0;
	     }
	     else {
		 if ( (dot = (aChr=='.' || aChr==',')) || aChr==':' ) {
		     *dig = ' ';
		     if ( dot ) hasDot++;
		 }
		 dig++;
	     }
	 }
     }

     timeError += ToSubTime( subTimeStr, fromTo[ idx ] )!=0;
     t[ idx ] = fromTo[ idx ].Millis();
     DBGPRINT_MIN("[%02d:%02u:%02u,%03u] ",
		  fromTo[ idx ].hh,
		  fromTo[ idx ].mm,
		  fromTo[ idx ].ss,
		  fromTo[ idx ].mi);
 }
 ms = t[ 1 ] - t[ 0 ];
 DBGPRINT_MIN("{%s} - {%s}, dots: %d, ms=%d, timeError=%d\n",
	      two.Str(1),
	      two.Str(2),
	      hasDot,
	      ms,
	      timeError);
 if ( timeError ) return 0;  // 0 is inevitably wrong.
 return ms;
}


int SubEntry::ToSubTime (const char* blankSub, sSubTime& subTime)
{
 int error;
 int hour( 0 ), minu( 0 ), secs( 0 ), milli( 0 );
 error = sscanf( blankSub, "%d%d%d%d",
		 &hour,
		 &minu,
		 &secs,
		 &milli ) < 3;
 subTime.hh = hour;
 subTime.mm = minu;
 subTime.ss = secs;
 subTime.mi = milli;
 return error;
}


char* SubEntry::thisBuildLineTalk (gString& sLine)
{
 gElem* ptrTalk( StartPtr() );
 ASSERTION(ptrTalk->next && ptrTalk->next->next,"thisBuildLineTalk");

 ptrTalk = ptrTalk->next->next;

 for ( ; ptrTalk; ptrTalk=ptrTalk->next) {
     gString sThis( ptrTalk->Str() );
     sThis.Trim();
     if ( sLine.Length() ) {
	 if ( sThis.Length() ) {
	     sLine.Add( " " );
	 }
     }
     sLine.AddString( sThis );
 }
 return sLine.Str();
}

////////////////////////////////////////////////////////////
Subs::Subs (char* aStr)
{
}


Subs::~Subs ()
{
}


int Subs::AddFromLine (const t_uchar* uBuf)
{
 int error( 0 );
 int value( -1 );
 bool surelyInvalid( false );

 if ( uBuf ) {
     gString sLine( (char*)uBuf );
     gString sTrim( sLine );

     sTrim.Trim();

     if ( sTrim.IsEmpty() ) {
	 return AddFromLine( nil );
     }

     if ( entry.N()==0 ) {
	 entry.iValue = atoi( sTrim.Str() );
     }
     entry.Add( sLine );
 }
 else {
     if ( entry.N() ) {
	 // Add existing entry, if not empty
	 value = entry.iValue;
	 surelyInvalid = value<0;

	 if ( surelyInvalid==false ) {
	     surelyInvalid = entry.N()<3;
	     error = (int)surelyInvalid;

	     if ( surelyInvalid ) {
		 fprintf(stderr,"Uops, %u line(s), {%s}\n",
			 entry.N(),
			 entry.Str( 1 ));
	     }
	     else {
		 error = thisAddEntry( entry )!=0;
		 entry.iValue = 0;
		 entry.Delete();
	     }
	 }
	 else {
	     fprintf(stderr,"Uops, subs#%u %d error=%d {%s}\n",
		     N(),
		     value,
		     error,
		     uBuf);
	 }
     }
 }
 return error;
}


int Subs::thisAddEntry (SubEntry& one)
{
 SubEntry* newEntry;
 gElem* ptr( nil );
 gString* aStr;
 int ms;

 if ( one.N()==0 ) return 0;  // an empty entry is simply ignored.

 newEntry = new SubEntry( one.iValue );
 ASSERTION(newEntry,"newEntry");
#if 1
 newEntry->Add( one.Str( 1 ) );
 ptr = one.CurrentPtr();
 ASSERTION(ptr,"ptr");
 ptr = ptr->next;
 ASSERTION(ptr,"ptr");
 aStr = (gString*)ptr->me;
 // 'aStr' is e.g. 00:23:01,240 --> 00:23:03,356
 //	iValue will be the milliseconds the subtitle will be, in this case 2116ms,
 //	or 0 on error (no "-->" symbol), negative on other errors.
 ms = newEntry->ConvSubsFromTo( *aStr );
 newEntry->Add( *aStr );
 newEntry->EndPtr()->me->iValue = ms;

 for (ptr=ptr->next; ptr; ptr=ptr->next) {
     newEntry->Add( ptr->Str() );
 }
#else
 // Simpler: but does not check TimeString() !
 newEntry->CopyList( one );
#endif
 newEntry->iValue = one.iValue;
 DBGPRINT("DBG: thisAddEntry %d\n",newEntry->iValue);

 AppendObject( newEntry );
 return 0;
}

////////////////////////////////////////////////////////////
int isubs_dump_subs_report (FILE* fOut, FILE* fErr, Subs& subs, gList& problems)
{
 int error( 0 );
 int iter( 0 ), value( -1 );
 unsigned mismatches( 0 );
 gElem* ptrElem( subs.StartPtr() );
 gElem* ptrEntry;
 gString* pLine;

 if ( fOut ) {
     for ( ; ptrElem; ptrElem=ptrElem->next) {
	 gString* prob( nil );

	 iter++;
	 value = ptrElem->me->iValue;
	 if ( iter!=value ) {
	     prob = new gString( 256, '\0' );
	     if ( fErr ) {
		 fprintf(fErr,"%d!=%d ",
			 iter,
			 value);
	     }
	 }

	 for (ptrEntry=((SubEntry*)ptrElem->me)->StartPtr(); ptrEntry; ptrEntry=ptrEntry->next) {
	     pLine = (gString*)ptrEntry->me;
	     gString* toShow( pLine );

	     if ( prob ) {
		 if ( ptrEntry->next && ptrEntry->next->next ) {
		     toShow = (gString*)ptrEntry->next->next->me;
		 }
		 prob->iValue = iter;
		 snprintf(prob->Str(), 250, "got %d: %s",
			  value,
			  toShow->Str());
		 if ( toShow->Length() > 80 ) {
		     strcat(prob->Str(), "..." );
		 }
		 problems.AppendObject( prob );
		 prob = nil;  // gets deleted in the list afterwards!
	     }
	     fprintf(fOut,"%s\r\n",
		     pLine->Str());
	 }

	 fprintf(fOut,"\r\n");

	 if ( prob ) {
	     prob->Set( "Nothing on index" );
	     problems.AppendObject( prob );
	 }
     }

     mismatches = problems.N();
     error = mismatches>0;
 }

 return error;
}


int isubs_dump_subs (FILE* fOut, Subs& subs)
{
 gList problems;
 FILE* fErr( stderr );
 int index( 0 );
 int error( isubs_dump_subs_report( fOut, fErr, subs, problems ) );
 unsigned mismatches( problems.N() );

 if ( mismatches ) {
     if ( fErr ) {
	 fprintf(fErr,"Subtitles mismatched sequence(s): %d\n",mismatches);

	 for (gElem* ptrElem=problems.StartPtr(); ptrElem; ptrElem=ptrElem->next) {
	     index++;
	     fprintf(stderr, "Bogus #%d: %d {%s}\n",
		     index,
		     ptrElem->me->iValue,
		     ptrElem->me->Str());
	 }
     }
 }
 return error;
}


int isubs_file_get (FILE* fOut, const char* strFile, gList& flaws, Subs& subs)
{
 int error;
 int line( 0 );
 gFileText fIn( (char*)strFile );
 bool isOk( true );
 bool hasNewLine( false );
 t_uchar* uBuf;

 error = fIn.IsOpened()==false;
 if ( error ) return 2;

 for ( ; fIn.ReadLine( isOk, hasNewLine ); ) {
     uBuf = fIn.UBuffer();
     line++;
     if ( subs.AddFromLine( uBuf ) ) {
	 flaws.Add( line );
     }
 }

 if ( subs.AddFromLine( nil ) ) {
     flaws.Add( line );
 }

 error = isubs_dump_subs( fOut, subs );
 DBGPRINT("DBG: isubs_file_get returns %d, flaws: %u\n",
	  error,
	  flaws.N());
 return error;
}


int isubs_dump_file_report (FILE* fOut, FILE* fErr, const char* strFile, Subs& subs)
{
 int error;
 gList flaws;
 gElem* ptrElem;

 error =  isubs_file_get( fOut, strFile, flaws, subs );
 DBGPRINT("DBG: isubs_dump_file_report error: %d, flaws: %u\n", error, flaws.N());
 if ( flaws.N() ) {
     if ( fErr ) {
	 fprintf(fErr, "Flaws %sfound: %u\n",
		 error ? "and errors " : "",
		 flaws.N());
	 for (ptrElem=flaws.StartPtr(); ptrElem; ptrElem=ptrElem->next) {
	     fprintf(fErr, "Flaw at line: %d\n", ptrElem->me->iValue);
	 }
     }

     error |= 1;
 }
 return error;
}


int isubs_dump_file (FILE* fOut, const char* strFile, Subs& subs)
{
 int error( isubs_dump_file_report( fOut, stderr, strFile, subs ) );
 DBGPRINT("DBG: isubs_dump_file returns %d\n", error);
 return error;
}

////////////////////////////////////////////////////////////

