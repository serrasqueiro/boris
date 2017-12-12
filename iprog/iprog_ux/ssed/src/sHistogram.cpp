// sHistogram.cpp


#include "sHistogram.h"

////////////////////////////////////////////////////////////
// Basic histogram functions
////////////////////////////////////////////////////////////
int calculate_histog_column (FILE* fIn, int charType, int column, gList& hist)
{
 bool didIncr( false );
 int len;
 int code;
 int iter, col;
 long iLine( 0 );
 char buf[ 4096 ];
 char* aStr;
 char* strLast;
 gString* newStr;

 ASSERTION(fIn,"fIn");

 for ( ; fgets( buf, sizeof(buf)-1, fIn ); ) {
     len = strlen( buf );
     if ( len<=0 ) break;

     if ( buf[ len-1 ]=='\n' ) {
	 iLine++;

	 buf[ --len ] = 0;
	 if ( len>0 && buf[ len-1 ]=='\r' ) {
	     buf[ --len ] = 0;
	 }
     }
     aStr = strLast = buf;

     for (iter=0, col=1, didIncr=true; iter<len; iter++) {
	 DBGPRINT_MIN("col=%d, iter=%d, incr? %c,\t%c\n",col,iter,ISyORn( didIncr ),buf[iter]);
	 if ( buf[ iter ]<=' ' ) {
	     if ( didIncr==false ) {
		 col++;
		 didIncr = true;
		 if ( col>=column ) {
		     aStr = strLast;
		     DBGPRINT_MIN("line: %ld, COL=%d: {%s}\n",
				  iLine,
				  col,
				  aStr);
		     break;
		 }
	     }
	 }
	 else {
	     didIncr = false;
	     strLast = buf + iter + 1;
	 }
     }

     if ( col<column ) continue;

     newStr = new gString( aStr );
     if ( newStr==nil ) {
	 fprintf(stderr,"Out of memory, line: %ld\n",iLine);
	 return -1;  // Out of mem.
     }

     code = hist.InsertOrderedUnique( newStr );

     if ( code==-1 ) {
	 hist.CurrentPtr()->me->iValue++;
	 delete newStr;
     }
     else {
	 newStr->iValue = 1;
     }
 }

 return 0;
}


int calculate_histogram (FILE* fIn, int charType, gList& hist)
{
 int len;
 int code;
 bool hasShown( false );
 long iLine( 0 );
 char buf[ 4096 ];

 gString* newStr;

 ASSERTION(fIn,"fIn");

 for ( ; fgets( buf, sizeof(buf)-1, fIn ); ) {
     len = strlen( buf );
     if ( len<=0 ) break;

     if ( buf[ len-1 ]=='\n' ) {
	 buf[ --len ] = 0;
	 if ( len>0 && buf[ len-1 ]=='\r' ) {
	     buf[ --len ] = 0;
	 }

	 iLine++;
	 if ( (iLine%100)==0 ) {
	     fprintf(stderr,"Line: %ld\r",iLine);
	     hasShown = true;
	 }
     }

     newStr = new gString( buf );
     if ( newStr==nil ) {
	 fprintf(stderr,"Out of memory, line: %ld\n",iLine);
	 return -1;  // Out of mem.
     }

     code = hist.InsertOrderedUnique( newStr );

     DBGPRINT_MIN("%ld\thist.N()=%u, code=%d, {%s}\n",
		  iLine,
		  hist.N(),
		  code,
		  newStr->Str());

     if ( code==-1 ) {
	 hist.CurrentPtr()->me->iValue++;
	 delete newStr;
     }
     else {
	 newStr->iValue = 1;
     }
 }

 if ( hasShown )
     fprintf(stderr,"Line: %ld\n",iLine);
 return 0;
}

////////////////////////////////////////////////////////////
// Exported functions
////////////////////////////////////////////////////////////
int show_histogram (FILE* fIn,
		    FILE* fOut,
		    const char* strSep,
		    int zValue,
		    int charType)
{
 int error( 0 );
 FILE* inFile( fIn ? fIn : stdin );
 FILE* outFile( fOut ? fOut : stdout );
 gList hist;
 gElem* ptrElem;

 if ( zValue>0 ) {
     error = calculate_histog_column( inFile, charType, zValue, hist );
 }
 else {
     error = calculate_histogram( inFile, charType, hist );
 }

 for (ptrElem=hist.StartPtr(); ptrElem; ptrElem=ptrElem->next) {
     fprintf(outFile,"%d\t%s\n",
	     ptrElem->me->iValue,
	     ptrElem->Str());
 }

 if ( outFile!=stdout && outFile!=stderr ) fclose( outFile );

 return error;
}


int dump_buffer (FILE* fOut, int sigh, const t_uchar* buf, int size)
{
 const int maxInt( MAX_INT16_I );
 int result( 0 );
 int nextSigh( size - ((sigh+1) / 2) );

 if ( fOut ) {
     for (int iter=0; iter<size; iter++) {
	 t_uchar chr( buf[ iter ] );
	 sigh--;
	 if ( iter >= nextSigh ) {
	     sigh = maxInt;
	 }
	 if ( sigh==0 ) {
	     fprintf(fOut, "[...]");
	 }
	 if ( sigh > 0 ) {
	     if ( chr >= 192 ) {
		 chr = '.';
	     }
	     else {
		 if ( chr < ' ' ) {
		     chr = ' ';
		 }
	     }
	     fprintf(fOut, "%c", chr);
	 }
	 else {
	     result++;  // keeps counting chars not printed
	 }
     }
 }
 return result;
}

////////////////////////////////////////////////////////////

