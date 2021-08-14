// iCProxy.cpp

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#ifdef iDOS_SPEC
//	typedef unsigned int __socklen_t;
//	typedef __socklen_t socklen_t;
#else
#include <sys/socket.h>  //socket... (sys/types.h)
// For: inet_ntoa:
#include <netinet/in.h>
#include <arpa/inet.h>
#endif //iDOS_SPEC

#include "iCProxy.h"
#include "iCIO.h"
#include "iCntpConfig.h"

////////////////////////////////////////////////////////////
int cxpxy_manage_connection (gAltNetServer* netServer,
			     gTcpConnect* connection,
			     FILE* fRepErr,
			     int serverSocketId,
			     int clientSocketId,
			     int proxySocketId,
			     int verboseLevel,
			     t_uint32& countBytesCli,
			     t_uint32& countBytesSrv)
{
 fd_set readfds;
 fd_set writefds;
 int countFailS( 0 ), countNoActivity( 0 );
 int selectResult;
 int readBytes, wroteBytes;
 static char ioBuf[ 4096 ];

 for ( ; countFailS<3; ) {
     FD_ZERO( &readfds );
     FD_ZERO( &writefds );
     FD_SET( clientSocketId, &readfds );
     FD_SET( proxySocketId, &readfds );
     FD_SET( clientSocketId, &writefds );
     FD_SET( proxySocketId, &writefds );

     ioBuf[0] = 0;

     selectResult = select( FD_SETSIZE, &readfds, &writefds, 0, 0 );
     if (selectResult < 0) {
	 countFailS++;
	 continue;
     }

     if ( FD_ISSET( clientSocketId, &readfds ) ) {
   	 DBGPRINT("DBG: reading from client, writing to proxy\n");
	 countFailS = -1;

	 readBytes = cxio_read( clientSocketId, ioBuf, sizeof( ioBuf ) );
	 if ( readBytes<=0 ) break;

	 countBytesCli += readBytes;
	 DBGPRINT("DBG: read %d byte(s) from client, total:%d, sending to proxy\n",
		  readBytes,
		  countBytesCli);
	 wroteBytes = cxio_write( proxySocketId, ioBuf, readBytes );
	 DBGPRINT("DBG: sent to proxy %d byte(s)\n",wroteBytes);

	 if ( wroteBytes != readBytes ) {
	     if ( fRepErr )
		 fprintf(fRepErr,"Short-write to proxy: %d/%d\n",
			 wroteBytes,
			 readBytes);
	     break;
	 }
     }//end (client>proxy)

     if ( FD_ISSET( proxySocketId, &readfds ) ) {
   	 DBGPRINT_MIN("DBG: reading from proxy, writing to client\n");
	 countFailS = -1;

	 readBytes = cxio_read( proxySocketId, ioBuf, sizeof( ioBuf ) );
	 if ( readBytes<=0 ) break;

	 countBytesSrv += readBytes;
	 DBGPRINT("DBG: read %d byte(s) from proxy, sending to client\n",readBytes);
	 wroteBytes = cxio_write( clientSocketId, ioBuf, readBytes );

	 if ( wroteBytes != readBytes ) {
	     if ( fRepErr )
		 fprintf(fRepErr,"Short-write from proxy: %d/%d\n",
			 wroteBytes,
			 readBytes);
	     break;
	 }
     }

     if ( countFailS<0 ) {
	 countFailS = countNoActivity = 0;
     }
     else {
	 countNoActivity++;
	 if ( countNoActivity>=200 ) {
	     DBGPRINT_MIN("DBG: Taking a nap [%d]\n",countNoActivity);
	     gFileControl::Self().MiliSecSleep( countNoActivity );
	     if ( countNoActivity>=1000 ) {
		 DBGPRINT_MIN("DBG: Taking a sleep\n");
		 SLEEP_SEC( 5 );
		 countNoActivity = 100;
	     }
	 }
     }
 }//end 'managed' FOR
 return 0;
}


gTcpConnect* cxpxy_serve_request (gAltNetServer* netServer,
				  sOptCntp& opt,
				  FILE* fRepErr,
				  int& code,
				  gString& sClientAddr,
				  int& clientSocketId,
				  sOptPxyStat& pxyStat)
{
 int serverSocketId( netServer->Handle() );
 int proxySocketId( -1 );
 bool isOk( false );
 bool doFlushAcceptErrors( opt.isVerbose );
 gTcpConnect* connection( nil );

 fd_set readfds;
 fd_set writefds;
 int selectResult;
 int countFailsR( 0 );

 const int defaultSleepOnSelectError( 2 );

 struct sockaddr_in clientAddressData;
 int clientAddressDataLength( sizeof( clientAddressData ) );

 code = -9;
 if ( netServer==nil ) return nil;

 for ( ; countFailsR<3; ) {
     FD_ZERO( &readfds );
     FD_ZERO( &writefds );
     FD_SET( serverSocketId, &readfds );
     FD_SET( serverSocketId, &writefds );

     if ( FD_ISSET( serverSocketId, &readfds ) ) {
	 DBGPRINT("DBG: waiting for TRIVIAL select\n");
	 selectResult = select( FD_SETSIZE, &readfds, &writefds, 0, 0 );
	 if ( selectResult<0 ) {
	     MY_LOG( LOG_WARN, "Select failed (sleeping %d)", defaultSleepOnSelectError );
	     if ( doFlushAcceptErrors )
		 flush_log_a();
	     SLEEP_SEC( defaultSleepOnSelectError );
	     continue;
	 }

	 gDateString stampNow;
	 pxyStat.stamp0 = stampNow.GetTimeStamp();
	 pxyStat.countCli++;

	 memset( &clientAddressData, 0x0, clientAddressDataLength );
	 DBGPRINT("DBG: waiting for client accept\n");
#ifdef iDOS_SPEC
	 clientSocketId = accept( serverSocketId,
				  (struct sockaddr*)&clientAddressData,
				  &clientAddressDataLength );
#else
	 clientSocketId = accept( serverSocketId,
				  (struct sockaddr*)&clientAddressData,
				  (socklen_t*)&clientAddressDataLength );
#endif
	 sClientAddr.Set( inet_ntoa( clientAddressData.sin_addr ) );
	 if ( clientSocketId<0 ) {
	     countFailsR++;
	     continue;
	 }

	 code = -1;
	 if ( connection ) {
	     isOk = connection->Handle()!=-1;
	     DBGPRINT("DBG: Is connected to remote-host: %s %c\n",
		      opt.sRemoteHost.Str(),
		      ISyORn( isOk ));
	 }
	 else {
	     connection = new gTcpConnect( opt.sRemoteHostOnly.Str(), opt.remotePort, false );
	     if ( connection==nil ) return nil;  // Uops, memory fault
	     isOk = connection->Connect();
	     proxySocketId = connection->Handle();
	     DBGPRINT("DBG: Connected remote-host: %s, %s:%u, OK? %c, lastOpError=%d {%s}\n",
		      opt.sRemoteHost.Str(),
		      opt.sRemoteHostOnly.Str(), opt.remotePort,
		      ISyORn( isOk ),
		      connection->lastOpError,
		      connection->GetErrorStr());
	 }
	 if ( isOk==false ) return connection;

	 pxyStat.countSrv++;
	 code = cxpxy_manage_connection( netServer,
					 connection,
					 fRepErr,
					 serverSocketId,
					 clientSocketId,
					 proxySocketId,
					 0,
					 pxyStat.bytesCli,
					 pxyStat.bytesSrv );
	 DBGPRINT("DBG: cxpxy_manage_connection returned %d\n",code);
	 return connection;
     }// end IF: trivial select!
 }
 return connection;
}
////////////////////////////////////////////////////////////
int cxpxy_cmd_serve_requests (gArg& arg,
			      gAltNetServer* netServer,
			      FILE* fRepErr,
			      sOptCntp& opt,
			      int aCmd)
{
 int code( 0 );
 int clientSocketId( -1 );
 int dummyCount( 0 );
 char* strClient;
 gTcpConnect* connection;
 gDateString stampInit;

 for ( ; code!=-9; ) {
     gString sClientAddr;
     sOptPxyStat pxyStat;

     DBGPRINT("DBG: accepting connections (port:%u)\n",(unsigned)netServer->BindPort());
     connection = cxpxy_serve_request( netServer, opt, fRepErr, code, sClientAddr, clientSocketId, pxyStat );
     strClient = sClientAddr.IsEmpty() ? (char*)"?" : sClientAddr.Str();
     DBGPRINT("DBG: cxpxy_serve_request returned %d (%s)\n",
	      code,
	      strClient);

     // Some stats
     opt.nAccepts++;
     opt.nStalls += code!=0;

     if ( clientSocketId!=-1 ) {
	 shutdown( clientSocketId, 2 );  // Note this socket was created directly: it must be closed, or some protocols like telnet, on the client side, will hang.
	 close( clientSocketId );
	 DBGPRINT("DBG: closed client [clientSocketId=%d]\n",clientSocketId);
	 clientSocketId = -1;
	 if ( opt.doShowStats ) {
	     gDateString stamp1;
	     dummyCount++;
	     pxyStat.secs = (long)stamp1.GetTimeStamp() - (long)pxyStat.stamp0;
	     MY_LOG( LOG_INFO, "Stats: %s; accepts: %u, errors: %u, secs: %ld, i/o: %u/%u, %u/%u",
			     strClient,
			     opt.nAccepts,
			     opt.nStalls,
			     pxyStat.secs,
			     pxyStat.countCli,
			     pxyStat.countSrv,
			     pxyStat.bytesCli,
			     pxyStat.bytesSrv );
	 }
	 MY_LOG( LOG_NOTICE, "Bye (%s): %s",
			 connection->GetErrorStr(),
			 strClient );
	 flush_log_a();
     }

     delete connection;

     if ( dummyCount>100 ) {
	 dummyCount = 0;
	 gDateString stampNow;
	 MY_LOG( LOG_NOTICE, "Stats: objs: %d, secs: %ld, started %s",
			 gStorageControl::Self().NumObjs(),
			 (long)stampNow.GetTimeStamp() - (long)stampInit.GetTimeStamp(),
			 stampInit.Str() );
     }
 }
 return 0;
}
////////////////////////////////////////////////////////////
int cxpxy_process_proxy (gArg& arg,
			 gAltNetServer* netServer,
			 bool isDaemon,
			 FILE* fOut,
			 FILE* fRepErr,
			 sOptCntp& opt)
{
 int code;
 unsigned n( arg.N() );
 char* str( arg.Str( 1 ) );
 gList listInterface;

 // The first part of this function is 'inspired' on do_process_vmsvc:
 str = arg.Str( 1 );
 if ( n>0 ) {
     n -= str[0]==0;
     if ( n>0 ) listInterface.Add( str );
     if ( n>1 ) {
	 if ( fRepErr )
	     fprintf(fRepErr,"Warning: currently only support all interfaces or one;\n\
entered %u interfaces, only using one: %s\n",
		     n,
		     str);
     }
 }

 if ( netServer->IsOk()==false || netServer->BindByList( listInterface )!=0 ) {
     if ( fRepErr ) {
	 fprintf(fRepErr,"Unable to bind server");
	 for (unsigned iter=1; iter<=n; iter++) {
	     fprintf(fRepErr,"%s%s%s%s",
		     iter==1 ? " (" : "\0",
		     arg.Str( iter ),
		     iter<n ? " " : "\0",
		     iter>=n ? ")" : "\0");
	 }
	 fprintf(fRepErr,": bailing out.\n");
     }
     return 1;
 }

 MY_DEF_LOG( "cntp-proxy starting (pid %u): %s",
	       GX_GETPID(),
	       opt.sRemoteHost.Str() );
 flush_log_a();
 code = cxpxy_cmd_serve_requests( arg, netServer, fRepErr, opt, 2 );
 DBGPRINT("DBG: cxpxy_process_proxy(%s) returned %d\n",sGlobClient.Str(),code);
 return code;
}
////////////////////////////////////////////////////////////

