// sBatch.cpp

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "sBatch.h"

////////////////////////////////////////////////////////////
gList* btc_listed_args (gString& sLine)
{
 t_uchar chr;
 short quote( 0 );
 gList* result( new gList );
 gString* thisStr;
 gString sWord;

 ASSERTION(result,"result");

 for (int idx=1; (chr = sLine[ idx ])!=0; idx++) {
     if ( quote==0 && chr==' ' ) {
	 if ( sWord.Length() ) {
	     result->Add( sWord );
	     sWord.SetEmpty();
	     continue;
	 }
     }
     else {
	 if ( chr=='"' ) {
	     quote = quote==0;
	 }
     }
     sWord.Add( chr );
 }
 if ( sWord.Length() ) {
     result->Add( sWord );
 }

 // Remove double-quotes where applicable
 for (gElem* pElem=result->StartPtr(); pElem; pElem=pElem->next) {
     thisStr = (gString*)pElem->me;
     unsigned len( thisStr->Length() );
     if ( len >= 2 ) {
	 if ( (*thisStr)[ 1 ]=='"' && (*thisStr)[ len ]=='"' ) {
	     thisStr->Delete( len );
	     thisStr->Delete( 1, 1 );
	 }
     }
 }
 return result;
}

////////////////////////////////////////////////////////////
gList* read_batch_input (int readHandle)
{
 gList* input;
 gList* node;
 gString* myStr;
 t_uchar chr;
 gString line;

 if ( readHandle==-1 ) return nil;
 input = new gList;
 ASSERTION(input,"input");

 for ( ; read(readHandle, &chr, 1)==1; ) {
     if ( chr<' ' ) {
	 if ( chr=='\t' ) {
	     chr = '\t';
	 }
	 else {
	     if ( chr=='\n' ) {
		 node = new gList;
		 ASSERTION(node,"node (1)");
		 node->Add( line );
		 node->AppendObject( new gList );
		 input->AppendObject( node );
		 line.SetEmpty();
		 continue;
	     }
	 }
     }
     if ( chr>=' ' ) {
	 line.Add( (char)chr );
     }
 }
 if ( line.Length() ) {
     node = new gList;
     ASSERTION(node,"node (2)");
     node->Add( line );
     node->AppendObject( new gList );
     input->AppendObject( node );
 }

 // Preprocess
 for (gElem* pElem=input->StartPtr(); pElem; pElem=pElem->next) {
     int preCode( -1 );
     myStr = (gString*)(((gList*)pElem->me)->StartPtr()->me);
     chr = (*myStr)[ 1 ];
     if ( chr>' ' ) {
	 if ( chr=='#' || myStr->Match( "rem" ) || myStr->Find( "rem " )==1 ) {
	     // Comment
	 }
	 else {
	     gList* params = btc_listed_args( *myStr );
	     if ( params ) {
		 gList* listedArg( (gList*)(((gList*)(pElem->me))->StartPtr()->next->me) );
		 preCode = (int)params->N();
		 listedArg->CopyList( *params );

		 //printf("listedArg: "); listedArg->Show(); printf("!\n");
	     }
	 }
     }
     pElem->iValue = preCode;
 }
 return input;
}


int btc_run_batch (FILE* fOut, gList& aBatch, gString& lastLine)
{
 const int maxBuf( 4096 );
 gString* myStr;
 gString sWarnChrs;
 gList* ptrList;
 gElem* pElem( aBatch.StartPtr() );
 FILE* aPipe;
 unsigned len;
 char outBuf[ maxBuf ];

 lastLine.SetEmpty();

 for ( ; pElem; pElem=pElem->next) {
     aPipe = nil;
     if ( pElem->iValue > 0 ) {
	 ptrList = (gList*)pElem->me;
	 myStr = (gString*)(ptrList->StartPtr()->me);
	 DBGPRINT_MIN("DBG: EXECUTE: %s\n", myStr->Str());

	 myStr->Trim();
	 len = myStr->Length();

#ifdef iDOS_SPEC
	 if ( (*myStr)[ 1 ]==':' ) {
	     len = 0;
	 }
#endif
	 if ( len ) {
	     aPipe = popen( myStr->Str(), "r" );
	     if ( fOut ) {
		 t_uchar chr;
		 gString sBad( BTC_WARN_CHARS );
		 fprintf(fOut, "%s\n", myStr->Str());
		 for (int iter=1; (chr = (*myStr)[ iter ])!=0; iter++) {
		     if ( sBad.Find( chr ) ) {
			 fprintf(fOut, "#  invalid char 0x%02X, at position: %d\n", (unsigned)chr, iter);
			 sWarnChrs.Set( "Invalid chrs: " );
			 gString sCopy( *myStr );
			 sCopy.Delete( iter+1 );
			 sWarnChrs.AddString( sCopy );
			 sWarnChrs.Add( "[...]" );
			 break;
		     }
		}
	     }
	 }
     }
     if ( aPipe ) {
	 for (memset(outBuf,0x0,sizeof(outBuf)); fgets( outBuf, sizeof( outBuf ), aPipe ); ) {
	     fprintf(stderr,"%s", outBuf);
	 }
	 if ( outBuf[ 0 ]>=' ' ) {
	     lastLine.Set( outBuf );
	 }
	 fclose( aPipe );
     }
 }
 if ( sWarnChrs.Length() ) {
     lastLine = sWarnChrs;
 }
 return 0;
}


int btc_add_once (const char* strIn, gList& result)
{
 if ( result.Match( strIn ) ) return -1;
 result.Add( strIn );
 return 0;
}

////////////////////////////////////////////////////////////

