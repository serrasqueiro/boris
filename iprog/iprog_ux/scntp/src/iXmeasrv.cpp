// iXmeasrv.cpp

#include <stdio.h>
#include <string.h>
#include <time.h>  // ctime, etc

#include "iXmeasrv.h"

#include "iCIO.h"

////////////////////////////////////////////////////////////
int ixm_configure_server (gList& confParams, sXmeasureOpt& xOpt)
{
 ;
#if 1
 printf("confParams: "); confParams.Show(); printf("-->\n");
 for (gElem* ptrDbg( confParams.StartPtr() ); ptrDbg; ptrDbg=ptrDbg->next) {
     printf("\t{%s\t%s}\n",
	    ((gList*)ptrDbg->me)->Shown().Str(),
	    ptrDbg->Str());
 }
#endif

 return 0;
}


char* ixm_http_date ()
{
 time_t aStamp( time( NULL ) );
 return ctime( &aStamp );
}


int ixm_check_client (gCNetSource& clientSource, gString& sMsg)
{
 printf("ixm_check_client: {%s}, handleIn: %d\n",
	clientSource.Str(),
	clientSource.handleIn);
 return 0;
}


int ixm_handle_request (int handle, sXmeasureOpt& xOpt)
{
 static const int maxIter( 15 );
 static const int lineBufSize( 2047 );
 static t_uchar uLines[ maxIter+1 ][ lineBufSize+1 ];
 static char parsedBuf[ 3 ][ lineBufSize+1 ];

 int code( -1 ), iter( 1 );
 int okResult( -1 ), returnCode( 1 );
 int idx( 0 ), lastPos( -1 );
 int protoViol( -1 );
 int writtenBytes( 0 );
 ssize_t didRead( -1 ), didWrite( -1 );
 char* buf;
 gList inlines;
 gList pairs[ 2 ];

 uLines[ 0 ][ 0 ] = 0;

 for ( ; iter<maxIter; iter++) {
     buf = (char*)uLines[ iter ];
     buf[ 0 ] = 0;
     didRead = cxio_read( handle, buf, lineBufSize );
     buf[ lineBufSize ] = 0;
     if ( didRead<=0 ) break;

     if ( buf[ didRead-1 ]!='\n' ) return 1;

     parsedBuf[ 1 ][ 0 ] = parsedBuf[ 2 ][ 0 ] = 0;
     code = sscanf( buf, "%s%s%s",
		    parsedBuf[ 0 ],
		    parsedBuf[ 1 ],
		    parsedBuf[ 2 ] );

     if ( iter==1 ) {
	 if ( code!=3 ) continue;

	 protoViol = strncmp( parsedBuf[ 2 ], "HTTP/", 5 )!=0;

	 if ( protoViol ) {
	     code = strncmp( parsedBuf[ 2 ], "XMSR/", 5 )!=0;
	     protoViol = code;
	 }

	 if ( protoViol==0 ) {
	     // Split several lines
	     for (idx=0; idx<didRead; idx++) {
		 if ( buf[ idx ]=='\r' ) {
		     buf[ idx ] = 0;
		 }
		 else {
		     if ( buf[ idx ]=='\n' ) {
			 char* strAdd( buf+lastPos+1 );
			 char* strBlank;
			 buf[ idx ] = 0;
			 inlines.Add( strAdd );
			 lastPos = idx;

			 strBlank = ixm_safe_strchr( strAdd, ' ' );
			 if ( strBlank && strBlank[ 1 ] ) {
			     strBlank[ 0 ] = 0;
			     pairs[ 0 ].Add( strAdd );
			     pairs[ 1 ].Add( strBlank+1 );
			 }
		     }
		 }
	     }

	     if ( inlines.N() && inlines.Str( inlines.N() )[ 0 ]==0 ) {
		 break;
	     }
	 }// end IF
     }
     else {
	 if ( buf[ 0 ]==0 ) {
	     protoViol = 8;
	     break;
	 }

	 if ( buf[ didRead-1 ]=='\n' ) {
	     buf[ --didRead ] = 0;
	     if ( didRead>0 ) {
		 if ( buf[ didRead-1 ]=='\r' )
		     buf[ --didRead ] = 0;
	     }

	     if ( didRead<=0 ) break;

	     pairs[ 0 ].Add( parsedBuf[ 0 ] );
	     pairs[ 1 ].Add( parsedBuf[ 1 ] );
	 }
	 else {
	     protoViol = 8|4;  // 12d indicates client closed too early
	     break;
	 }
     }
 }// end (input read)

 for (short dbgIter=1; dbgIter<=(int)inlines.N(); dbgIter++) { printf(" #%d {%s}\n\n",dbgIter,inlines.Str(dbgIter)); }

 for (short dbgIter=0; dbgIter<=iter; dbgIter++) { printf(" +%d {%s}\n\n",dbgIter,uLines[dbgIter]); }

 for (short dbgIter=1; dbgIter<=(int)pairs[0].N(); dbgIter++) { printf(" p[%d] {%s} {%s}\n\n",dbgIter,pairs[0].Str(dbgIter),pairs[1].Str(dbgIter)); }

 DBGPRINT("DBG: handle: %d, protoViol: %d, iter=%d, inlines.N()=%u\n",
	  handle,
	  protoViol,
	  iter,
	  inlines.N());

 if ( iter>=maxIter || protoViol ) {
     //		HTTP/1.1 400 Bad Request
     //		Date: Thu, 23 Dec 2010 02:06:27 GMT
     //		Server: Apache/1.3.33 (Unix) PHP/4.3.10
     //		Connection: close
     //		Content-Type: text/html; charset=iso-8859-1

     buf = (char*)uLines[ 0 ];
     code = ixm_http_text_response( handle,
				    400,  // Bad request
				    buf,
				    lineBufSize,
				    xOpt.serverVersion.Str(),
				    nil,
				    nil,
				    writtenBytes );
	 didWrite = (ssize_t)writtenBytes;

     if ( code==0 ) {
	 snprintf( buf, lineBufSize, "this server could not understand your request (%d)\n",
		   protoViol==-1 ? 400 : protoViol );
	 didRead = strlen( buf );
	 didWrite = cxio_write( handle, buf, didRead );
     }
 }
 else {
     buf = (char*)uLines[ 0 ];
     snprintf( buf, lineBufSize, "{%s} [%s]\n",
	       parsedBuf[ 2 ],
	       uLines[ 1 ] );
     didRead = strlen( buf );
     didWrite = cxio_write( handle, buf, didRead );
     okResult = didRead!=didWrite;
 }

 code = didRead!=didWrite;
 returnCode = code!=0 || okResult!=0;

 DBGPRINT_MIN("DBG: returnCode: %d (%d), okResult: %d\n\n{%s}\n{%s}\n{%s}\n\n",
	      returnCode, code,
	      okResult,
	      parsedBuf[ 0 ],
	      parsedBuf[ 1 ],
	      parsedBuf[ 2 ]);
 return returnCode;
}

////////////////////////////////////////////////////////////
char* ixm_safe_strchr (const char* aStr, char chr)
{
 ASSERTION(aStr,"strchr");
 if ( chr=='\t' ) {
     chr = ' ';
 }
 return strchr( (char*)aStr, chr );
}


int ixm_http_text_response (int handle,
			    int httpCode,
			    char* buf,
			    const int bufSize,
			    const char* strServerVersion,
			    const char* str1,
			    const char* str2,
			    int& didWrite)
{
 static int bufLen;
 const char* strHttpDate( ixm_http_date() );

 switch ( httpCode ) {
 case 400:
     snprintf( buf, bufSize, "HTTP/1.1 400 Bad Request\n\
Date: %sServer: %s\n\
Connection: close\n\
Content-Type: text/plain\
\n\n",
	       strHttpDate,
	       strServerVersion );
     break;

 default:
     snprintf( buf, bufSize, "HTTP/1.1 %d ?\n\
Date: %sServer: %s\n\
Connection: close\n\
Content-Type: text/plain\
\n\n",
	       httpCode,
	       strHttpDate,
	       strServerVersion );
     break;
 }

 bufLen = strlen( buf );
 didWrite = cxio_write( handle, buf, bufLen );
 return bufLen!=didWrite;
}

////////////////////////////////////////////////////////////

