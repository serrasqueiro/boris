// handler.cpp

#include "handler.h"
#include "lib_ilog.h"

////////////////////////////////////////////////////////////
// Aux functions
////////////////////////////////////////////////////////////

int ilf_which_month (const char* strMonth, int three, gDateTime& aDate)
{
 if ( strMonth ) {
     for (int iter=1; iter<=12; iter++) {
	 if ( strncmp( aDate.monthNames[ iter ].abbrev, strMonth, 3 )==0 )
	     return iter;
     }
 }
 return 0;
}


int ilf_flush_log (int handleIn, int handleOut, int dateIdx, gDateTime& aDate, t_uint16 mask, char* lineBuf, int& idx)
{
 int error( 0 ), didWrite;
 int find, len( -1 );
 int year( -1 );
 int month( 0 );
 int day( -1 );
 char sep( '\0' );
 char* str;

 if ( (mask & 256)==0 ) {
     if  ( dateIdx > 0 ) {
	 lineBuf[ --dateIdx ] = 0;
	 str = lineBuf+dateIdx+1;
	 find = gio_findstr( str, "]" );
	 if ( find>=0 ) {
	     str[ find ] = 0;
	     len = strlen( str );
	     if ( len>11 ) {
		 sep = str[ 11 ];
		 if ( sep==':' || sep==' ' ) {
		     str[ 11 ] = ' ';

		     if ( (mask & 128)==0 ) {
			 char buf[ 64 ];
			 month = ilf_which_month( str+3, 3, aDate );
			 year = atoi( str+7 );
			 day = atoi( str );

			 if ( day>=1 && day<=31 && month>=1 && year>=1900 ) {
			     snprintf(buf, sizeof(buf), "%04u-%02u-%02u ", year % 10000, month, day);
			     memcpy( str, buf, strlen( buf ) );
			 }
		     }

		     write( handleOut, lineBuf, dateIdx );
		     write( handleOut, str, strlen( str ) );
		     str += find+1;
		     len = strlen( str );
		     error = write( handleOut, str, len )!=len;
		 }
	     }
	 }
	 else {
	     len = -1;
	 }
     }
 }

 if ( len==-1 ) {
     didWrite = write( handleOut, lineBuf, idx );
     error = didWrite!=idx;
 }
 lineBuf[ idx = 0 ] = 0;
 return error;
}

////////////////////////////////////////////////////////////
int ilf_dump_apache_logs (int handleIn, int handleOut, t_uint16 mask, gDateTime& aDate, gString& lastLine)
{
 char locBuf[ 4096 ];
 char lineBuf[ 4096*4 ];
 char chr, lastChr( ' ' );
 int bufSize( sizeof( locBuf )-4 ), maxLine( bufSize * 4 );
 int didRead( -1 );
 int iter( 0 );
 int idx( 0 ), dateIdx( 0 );
 int error( 0 );

 // Apache logs:
 //	LogFormat "%h %l %u %t \"%r\" %>s %b" common

 ASSERTION(handleIn!=-1,"handleIn!=-1");
 ASSERTION(handleOut!=-1,"handleOut!=-1");

 memset( locBuf, 0x0, sizeof( locBuf ) );
 memset( lineBuf, 0x0, sizeof( lineBuf ) );
 lastLine.Reset();

 do {
     didRead = read( handleIn, locBuf, bufSize );
     locBuf[ bufSize ] = locBuf[ bufSize+1 ] = 0;
     if ( didRead <=0 ) break;

     for (iter=0; iter<didRead; iter++) {
	 chr = locBuf[ iter ];
	 if ( chr=='\r' ) continue;

	 lineBuf[ idx++ ] = chr;
	 lineBuf[ idx ] = 0;
	 if ( idx > maxLine ) {
	     for ( ; read( handleIn, &chr, sizeof(char) )==1; ) {
		 if ( chr=='\n' ) break;
	     }
	     strcat( lineBuf, " [...]\n" );
	 }

	 if ( chr=='\n' ) {
	     error = ilf_flush_log( handleIn, handleOut, dateIdx, aDate, mask, lineBuf, idx );
	     if ( error ) break;
	     dateIdx = 0;
	 }
	 else {
	     if ( lastChr==' ' ) {
		 if ( chr=='[' ) dateIdx = idx;
	     }
	 }

	 lastChr = chr;
     }
 } while ( error==0 );

 lastLine.Set( lineBuf );

 DBGPRINT("DBG: didRead finally: %d, lastLine: {%s}\n",
	  didRead,
	  lastLine.Str());
 return error;
}

////////////////////////////////////////////////////////////

