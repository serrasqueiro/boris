// iCntpBServer.cpp


#define thisProgramVersion "Version 2.1"
#define thisProgramCopyright "Prized Season & Sons"
#define thisProgramYear 2020
#define thisProgramCopy "This is free software (GPL)\n\
There is no warranty, not even for MERCHANTABILITY or\n\
FITNESS FOR A PARTICULAR PURPOSE."

#include <stdio.h>
#include <string.h>
#include <errno.h>

#ifdef iDOS_SPEC
#define STR_HELP_SET_UID ""
#else
#define STR_HELP_SET_UID "   -u X         Use user-id (or --uid)\n"
#endif //~iDOS_SPEC

#include "itime.h"
#include "imath.h"
#include "icalendar.h"

#include "iCntpConfig.h"
#include "iCProxy.h"
#include "iCntpResponse.h"
#include "iCntpHelper.h"

#include "iXmeasrv.h"
#include "rfc822date.h"

#include "lib_ilog.h"
#include "lib_icntp.h"


#define LISTEN_QUEUE_SIZE 1

#ifdef DEBUG
#define CNTP_WRCL_DEF_STALL 1
#else
#define CNTP_WRCL_DEF_STALL 5	// Default client stall is 5 seconds
#endif //~DEBUG...

////////////////////////////////////////////////////////////
// Globals
int codeMConnectionAlarmBye=1;
const char* strMConnectionAlarmBye="Bailing out.";
sXmeasureOpt xMeasures;


// Forward declarations...
int month_hashed (int hash) ;

int guess_date (gList& args, gDateTime& myTime, int& errorWhere) ;

////////////////////////////////////////////////////////////
int print_help (char* progStr)
{
 const char
     *msgHelp = "%s - %s\n\n\
Usage:\n\
        %s command [OPTION] [interface ...]\n\
\n\
Commands are:\n\
   date         A simple date and time shower\n\
   dates        Another (alternative) date and time shower\n\
   date-ux      Show basic friendly date (fixed format)\n\
                Similar to POSIX: date --iso-8601=seconds\n\
\n\
   vmsvc        VMware CNTP behaviour\n		\
%s\
   proxy        Transparent proxy\n\
   xmeasures    Experimental measurement server\n\
\n\
Options are:\n\
   -h           This help (or --help / --version)\n\
   -v           Verbose (use twice, more verbose)\n\
   -d N         Use debug level N (or --debug)\n\
   -k N         Keep showing date each N seconds\n\
   -l X         Use log-file X (or --log-file)\n\
   -m           Show miliseconds\n\
   -p X         Use port X (or --port), instead of default (%u)\n\
   -r X         Use remote-host (or --remote-host; format is host:port or host)\n\
   --pid-file X Use pid file X\n\
"
STR_HELP_SET_UID
"   --stats      Show statistics\n\
\n\
Interfaces are optional arguments,\n\
e.g.: 192.168.0.1 binds only to that interface\n\
%s";
#ifdef iDOS_SPEC
 const char* msgExtra = "\n\
Example:\n\
   scntp vmsvc\n\
";
#else
 const char* msgExtra = "\n\
Examples:\n\
   scntp vmsvc &\n\
or\n\
   scntp vmsvcd\n\
";
#endif
#ifdef iDOS_SPEC
 const char* msgDaemon = "\0";
#else
 const char* msgDaemon = "   vmsvcd       VMware CNTP behaviour (daemon)\n";
#endif //~iDOS_SPEC

 printf(msgHelp,
	progStr,
	thisProgramVersion,
	progStr,
	msgDaemon,
	DEF_CNTP_PORT,
	msgExtra);
 return 0;
}

////////////////////////////////////////////////////////////
int print_version (char* progStr)
{
 const char
     *msgVersion = "%s - %s\n\
Written by Henrique Moreira.\n\
\n\
Build \
017\
\n\
\n\
Copyright (C) %u %s.\n\
%s\n";

 printf(msgVersion,
	progStr,
	thisProgramVersion,
	thisProgramYear,thisProgramCopyright,
	thisProgramCopy);
 return 0;
}

////////////////////////////////////////////////////////////
gAltNetServer* set_server (t_gPort serverPort)
{
 gAltNetServer* netServer = new gAltNetServer( serverPort, LISTEN_QUEUE_SIZE );
 ASSERTION(netServer,"netServer");
 return netServer;
}

////////////////////////////////////////////////////////////
// glib required functions
////////////////////////////////////////////////////////////
int gglobal_EventTerminate (int v)
{
 DBGPRINT("DBG: gglobal_EventTerminate %d\n",v);

 switch ( v ) {
 case GX_SIGPIPE:
     return -1;  // Ignore...

 case GX_SIGSEGV:
     fprintf(stderr,"Uops...\n");
     return 127;

 case GX_SIGINT:
     MY_DEF_LOG( "Interrupted by user" );
     flush_log_a();
     return 0;  // quit

 case GX_SIGHUP:
     MY_LOG( LOG_NOTICE, "cntp-server HUP (signal %d)",v);
     flush_log_a();
     break;

 case GX_SIGTERM:
 default:
     my_bailout( v );
     return 0;
 }

 return -1;  // end of signal processing
}


void global_alarm_event (int v)
{
 DBGPRINT("ALARM: %s!\n",tod_date());
 if ( sGlobClient.IsEmpty()==false ) {
     MY_LOG( LOG_WARN, "Client taking too long: %s",sGlobClient.Str() );
 }
}


void global_alarm_event_none (int v)
{
 DBGPRINT("NO ALARM: %s!\n",tod_date());
}


void MySignalHandler (int signalId)
{
 int exitCode( gglobal_EventTerminate( signalId ) );

 if ( exitCode==0 ) {
     gEND;
     exit( 0 );
 }
}


////////////////////////////////////////////////////////////
// Work functions
////////////////////////////////////////////////////////////
int dump_date_unix (gList& args, int options)
{
 char timeStr[128];
 struct tm* tm;
 time_t uxTime;

 time(&uxTime);
 tm = localtime(&uxTime);
 if (options > 0) {
     strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S %z (%Z)", tm);
 }
 else {
     strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S %z", tm);
 }
 printf("%s\n", timeStr);
 return 0;
}


int dump_date (FILE* fRepErr, const char* cmdSubStr, gList& args, int options)
{
 int error( -1 );
 int iter( 1 ), nIter( (int)args.N() );
 t_uint32 uValue( 0 );
 gList values;
 gElem* pElem;
 char buf[ 256 ];

 const bool showStamp( options >= 3 );

 FILE* fOut( stdout );

 ASSERTION(cmdSubStr,"cmdSubStr");

 for (iter=1; iter<=nIter; iter++) {
     error = gStorageControl::Self().ConvertToUInt32( args.Str( iter ), uValue );
     if ( error ) break;
     values.Add( (int)uValue );
 }

 pElem = values.StartPtr();

 if ( (error==0 && uValue>=STAMP_1970_JAN02) || strcmp(cmdSubStr,"s")==0 ) {
     for (iter=1, nIter=(int)values.N(); iter<=nIter; iter++, pElem=pElem->next) {
	 t_stamp stamp( pElem->me->iValue );
	 gDateTime myTime( stamp );

	 if ( options ) {
	     dump_rfc822_date( fOut, myTime, -1, false, 0 );
	 }
	 else {
	     tod_date_cntpas( myTime, 0, buf, sizeof( buf )-1 );
	     fprintf(fOut, "%s", buf);
	 }
	 if ( showStamp ) {
	     fprintf(fOut, "\t%lu", (unsigned long)myTime.GetTimeStamp());
	 }
	 fprintf(fOut, "\n");
     }
 }
 else {
     // Try to guess date...

     gDateTime myTime;
     int errorWhere( 0 );

     myTime.day = 0;
     error = guess_date( args, myTime, errorWhere ) || myTime.day<=0;
     if ( error ) {
	 if ( fRepErr ) {
	     if ( errorWhere ) {
		 fprintf(fRepErr, "Unable to understand date: %s\n", args.Str( errorWhere ));
	     }
	     else {
		 if ( args.N() ) {
		     fprintf(fRepErr, "Unable to understand date.\n");
		 }
		 else {
		     fprintf(fRepErr, "No date entered.\n");
		 }
	     }
	 }
     }
     else {
	 if ( options ) {
	     dump_rfc822_date( fOut, myTime, -1, false, 0 );
	 }
	 else {
	     tod_date_cntpas( myTime, 0, buf, sizeof( buf )-1 );
	     fprintf(fOut, "%s", buf);
	 }
	 if ( showStamp ) {
	     fprintf(fOut, "\t%lu", (unsigned long)myTime.GetTimeStamp());
	 }
	 fprintf(fOut, "\n");
     }
 }

 return 0;
}


int do_prepare_xmeasures (gList& arg, sOptCntp& opt, sXmeasureOpt& xmeasureOpt)
{
 int error;
 gList confParams;
 gList* newParam;

 if ( opt.sRemoteHost.Length() ) {
     newParam = new gList( "allow_remote_point" );
     newParam->Add( opt.sRemoteHost );
     confParams.AppendObject( newParam );
 }

 error = ixm_configure_server( confParams, xmeasureOpt );
 return error;
}



int serve_xmeasure_request (gAltNetServer* netServer,
			    FILE* fRepErr,
			    sOptCntp& opt,
			    sXmeasureOpt& xmeasureOpt,
			    gCNetSource& clientSource)
{
 int code;  // <0: Denied client, otherwise: accepted
 int verboseLevel( opt_to_VerboseLevel( opt ) );
 gString sMsg;
 char* clientStr;
 char* todDateStr( nil );

 ASSERTION(netServer,"netServer");
 ASSERTION(clientSource.handleIn>=0,"clientSource.handleIn>=0");

 clientSource.BestGuessIdFromIP( netServer->ClientIp() );

 gString sSource( clientStr = clientSource.Str() );
 todDateStr = tod_date();

 code = ixm_check_client( clientSource, sMsg );

 if ( verboseLevel>=9 ) {
     if ( fRepErr )
	 fprintf(fRepErr,"%s %s client: %s (%s)\n",
		 todDateStr,
		 code<0 ? "Denied" : "Accepted",
		 clientStr,
		 sMsg.Str());
 }
 if ( code<0 ) return 2;

 code = ixm_handle_request( clientSource.handleIn, xmeasureOpt );

 return code;
}


int do_process_xmeasures (gList& arg, gAltNetServer* netServer, bool isDaemon, FILE* fRepErr, sOptCntp& opt, sXmeasureOpt& xmeasureOpt)
{
 static t_gTicElapsed totalCpuWasted;

 int thisError( 0 );
 int childPid( -1 );
 unsigned n( arg.N() );

 gTimerTic ticDummy;  // start count here

 char* str( arg.Str( 1 ) );
 gList listInterface;

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
     my_bye( fRepErr, 0, 0, arg );
     return 1;
 }

#ifdef iDOS_SPEC
 return -1;

#else
 if ( isDaemon ) {
     childPid = fork();

     if ( childPid<0 ) {
	 if ( fRepErr ) fprintf(fRepErr,"Uops, fork failed\n");
	 return -1;
     }

     if ( childPid>0 ) {
	 fclose( stdout );
	 gEND;
	 exit( 0 );  // Parent-dies normally
     }

     MY_DEF_LOG( "xmeasures starting (pid %u)",GX_GETPID() );
     my_start_daemon( GX_GETPID(), 0 );
 }

 for ( ; ; ) {
     gCNetSource clientSource;

     DBGPRINT("DBG: accepting connections (port:%u)\n",(unsigned)netServer->BindPort());
     clientSource.handleIn = netServer->Accept();
     if ( clientSource.handleIn<0 ) {
	 if ( opt.isVeryVerbose ) perror("Accept error");
	 clientSource.SetError( errno );
	 MY_LOG( LOG_WARN, "Unable to accept client (%s)", clientSource.GetErrorStr() );
	 SLEEP_SEC( 5 );
	 continue;
     }

     gTimerTic tic;  // start count here

     thisError = serve_xmeasure_request( netServer, fRepErr, opt, xmeasureOpt, clientSource );

     // Some stats
     opt.nAccepts++;

     if ( opt.fStat ) {
	 t_gTicElapsed cpuWasted( tic.CpuTics() );  // Mandatory to have tic updated.
	 t_uint32 elapsedMillisec( tic.GetMilisec() );
	 fprintf(opt.fStat,"%s\t%s\telapsed: %u, waiting %u (CPU: %0.0f) ms\n",
		 my_client_source( clientSource, 16, 0 ),
		 thisError ? "NotOk" : "OK",
		 elapsedMillisec,
		 ticDummy.GetMilisec(),
		 (float)cpuWasted);
	 totalCpuWasted += cpuWasted;
	 if ( opt.isVeryVerbose && fRepErr ) {
	     fprintf(fRepErr,"Total CPU used: %0.0f ms\n",(float)totalCpuWasted);
	 }
     }

     if ( thisError ) {
	 MY_LOG( LOG_WARN, "Responded to bogus client (%d): %s",
		 thisError,
		 clientSource.String() );
     }
     else {
	 MY_LOG( LOG_INFO, "Responded client: %s", clientSource.String() );

	 if ( opt.isVeryVerbose ) {
	     flush_log_a();
	 }
     }

     netServer->ClientIp().Reset();  // closes remote connection
 } //endless loop

#endif //~iDOS_SPEC
}


int serve_BasicVNTP_request (gAltNetServer* netServer,
			     FILE* fRepErr,
			     sOptCntp& opt,
			     gCNetSource& clientSource)
{
 int code;
 gString sMsg;
 int verboseLevel( opt_to_VerboseLevel( opt ) );
 char* clientStr;
 char* todDateStr( nil );

 if ( netServer==nil ) return -9;

 DBGPRINT("DBG: Accept hook: '%s'\n",netServer->ClientIp().String());
 ASSERTION(clientSource.handleIn>=0,"clientSource.handleIn>=0");

 clientSource.BestGuessIdFromIP( netServer->ClientIp() );
 code = my_client_check( clientSource, fRepErr, sMsg );
 gString sSource( clientStr = (char*)clientSource.String() );
 todDateStr = tod_date();
 DBGPRINT_MIN("DBG: my_client_check(%s) returned %d [%s]\n",
	      clientStr,
	      code,
	      sMsg.Str());

  if ( verboseLevel>=9 ) {
     if ( fRepErr )
	 fprintf(fRepErr,"%s %s client: %s (%s)\n",
		 todDateStr,
		 code<0 ? "Denied" : "Accepted",
		 clientStr,
		 sMsg.Str());
 }
 if ( code<0 ) return 2;

 sGlobClient = clientStr;
#ifndef iDOS_SPEC
 signal( GX_SIGALRM, global_alarm_event );
#endif
 my_helper_alarm( 10 );

 code = cntp_BasicVNTP_HandleClient( clientSource.handleIn );
 DBGPRINT("DBG: cntp...HandleClient returned %d\n",code);

 my_helper_signal_ignore( GX_SIGALRM );

 if ( code ) {
     if ( fRepErr )
	 fprintf(fRepErr,"%s Stalling client %u secs: %s\n",
		 tod_date(),
		 CNTP_WRCL_DEF_STALL,
		 clientStr);
     MY_LOG( LOG_INFO, "Stalling client %u secs: %s",
	     CNTP_WRCL_DEF_STALL,
	     sSource.Str() );
     // No flush!	->	flush_log_a()

     // Stall the client, since there was an invalid request
     SLEEP_SEC( CNTP_WRCL_DEF_STALL );
 }

 return code;
}

////////////////////////////////////////////////////////////
int serve_requests (gAltNetServer* netServer,
		    FILE* fRepErr,
		    sOptCntp& opt)
{
 int code;
 gTimerTic ticDummy;  // start count here
 gDateTime uTime;
 bool doFlushAcceptErrors( opt.isVerbose );
 static t_gTicElapsed totalCpuWasted;

 MY_LOG( LOG_NOTICE, "Started to serve requests (port %u): %s",opt.myPort,my_helper_ctime_current() );
 flush_log_a();

 for ( ; ; ) {
     gCNetSource clientSource;

     ticDummy.CpuTics(); ticDummy.Reset();
     DBGPRINT("DBG: accepting connections (port:%u)\n",(unsigned)netServer->BindPort());
     clientSource.handleIn = netServer->Accept();
     if ( clientSource.handleIn<0 ) {
	 if ( opt.isVeryVerbose ) perror("Accept error");
	 clientSource.SetError( errno );
	 MY_LOG( LOG_WARN, "Unable to accept client (%s)%s",
			 clientSource.GetErrorStr(),
			 doFlushAcceptErrors ? " (flushing)" : "\0" );
	 if ( doFlushAcceptErrors ) {
	     flush_log_a();
	 }
	 SLEEP_SEC( 5 );
	 continue;  // Proceed server, though there was an outstanding error
     }
     gTimerTic tic;  // start count here
     code = serve_BasicVNTP_request( netServer, fRepErr, opt, clientSource );
     opt.nStalls += code!=0;
     DBGPRINT("DBG: serve_BasicVNTP_request returned %d\n",code);
     if ( code<-1 ) {
	 return 4;  // Bogus return
     }

     // Some stats
     opt.nAccepts++;

     DBGPRINT("DBG: serve_request(%s) returned %d\n",
	      clientSource.String(),
	      code);
     DEBUG_WAIT_C("confirm close of connection (c)\t");

     if ( opt.fStat ) {
	 t_gTicElapsed cpuWasted( tic.CpuTics() );  // Mandatory to have tic updated.
	 t_uint32 elapsedMillisec( tic.GetMilisec() );
	 fprintf(opt.fStat,"%s\t%s\telapsed: %u, waiting %u (CPU: %0.0f) ms\n",
		 clientSource.String(),
		 code ? "NotOk" : "OK",
		 elapsedMillisec,
		 ticDummy.GetMilisec(),
		 (float)cpuWasted);
	 totalCpuWasted += cpuWasted;
	 if ( opt.isVeryVerbose && fRepErr ) {
	     fprintf(fRepErr,"Total CPU used: %0.0f ms\n",(float)totalCpuWasted);
	 }
     }

     if ( code==0 ) {
#ifdef DEBUG
	 static int dbgDaySim, dbgModus=10;
	 dbgDaySim++;
	 if ( opt.isVeryVerbose ) dbgModus = 3;
	 if ( (dbgDaySim%dbgModus)==0 ) uTime.day--;
	 DBGPRINT("DBG: simulating day before?%c (dbgModus=%d)\n",ISyORn((dbgDaySim%dbgModus)==0),dbgModus);
#endif //DEBUG...

	 MY_LOG( LOG_INFO, "Responded client: %s", my_client_source( clientSource, 1, 0 ) );

	 if ( uTime.day==iGlobDay ) {
	     if ( opt.isVeryVerbose ) flush_log_a();
	 }
	 else {
	     MY_LOG( LOG_INFO, "nAccepts: %u, nStalls: %u",
			     opt.nAccepts,
			     opt.nStalls );
	     flush_log_a();
	 }
	 uTime.day = iGlobDay;
     }
     else {
	 MY_LOG( LOG_WARN, "Responded to bogus client: %s", my_client_source( clientSource, -1, 0 ) );
     }

     netServer->ClientIp().Reset();  // closes remote connection
 }
 // Academic return: never reaches here
 return -1;
}

////////////////////////////////////////////////////////////
// Specific functions
////////////////////////////////////////////////////////////
int go_cmd_action (gArg& arg,
		   gAltNetServer* netServer,
		   FILE* fRepErr,
		   sOptCntp& opt,
		   int aCmd)
{
 int error = 0, thisError = 0;

 switch ( aCmd ) {
 case 1:
     thisError = serve_requests( netServer, fRepErr, opt );
     error = thisError!=0;
     break;
 default:
     ASSERTION_FALSE("Internal-error");
 };

 DBGPRINT("DBG: error=%d, thisError=%d\n",error,thisError);

 return error;
}


int do_process_vmsvc (gArg& arg, gAltNetServer* netServer, bool isDaemon, FILE* fOut, FILE* fRepErr, sOptCntp& opt)
{
 int thisError( 0 );
 unsigned n( arg.N() );
 char* str( arg.Str( 1 ) );
 gList listInterface;

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

#ifdef iDOS_SPEC
 MY_DEF_LOG( "cntp-server starting (pid %u)",GX_GETPID() );
 flush_log_a();
 thisError = go_cmd_action( arg, netServer, fRepErr, opt, 1 );
#else
 if ( isDaemon ) {
     int childPid( fork() );

     if ( childPid<0 ) {
	 if ( fRepErr ) fprintf(fRepErr,"Uops, fork failed\n");
	 return -1;
     }

     if ( childPid>0 ) {
	 fclose( stdout );
	 fclose( stderr );
	 gEND;
	 exit( 0 );  // Parent-dies normally
     }

     MY_DEF_LOG( "cntp-server daemon starting (pid %u)",GX_GETPID() );
     my_start_daemon( GX_GETPID(), 0 );
 }
 thisError = go_cmd_action( arg, netServer, fRepErr, opt, 1 );
#endif //~iDOS_SPEC
 DBGPRINT("DBG: do_process_vmsvc returning%s(?!) %d\n",isDaemon?" [DAEMON] ":"? uops",thisError);

 return thisError;
}


int simple_show (FILE* fOut, bool doListen, t_gPort servingPort, int eachSecond, sOptCntp& opt)
{
 const int debug( lGlobLog.dbgLevel );
 const char tickControlChar( ' ' );
 const t_int32 wDeltaInit( -30 );  // -30 milliseconds is a good reduction to 1000 ms of Sleep, due to processing of own functions...
 const bool showTickMark( eachSecond>=1 && debug>=3 );
 const bool showTzName( debug>=6 );
 const char* currentTimeZone( NULL );

 FILE* fDump( fOut ? fOut : stdout );
 bool showMillis( opt.doShowMillis );
 t_int32 wMillisAverage( 0 ), wDeltaHalf( wDeltaInit );

 static t_int16 progress;
 static char tickmarks[ 12 ] = "\\|/-~!x";
 static const t_uint16 nrTickmarks( 4 );

 //static const t_uint16 deviationTick( 4 );  // tilde symbol (~)
 //static const t_uint16 noTimeTick( 5 );  // exclamation mark symbol
 //
 //netServer = set_server( opt.myPort )  <--- TODO!

 DBGPRINT("DBG: doListen? %c, anyPort? %c, myPort: %d, servingPort: %d\n",
	  ISyORn( doListen ),
	  ISyORn( opt.anyPort ),
	  opt.myPort,
	  servingPort);

 for ( ; ; progress++) {
     // Display the dates in different flavors
     int timeDiff( 0 );
     time_t now( 0 );
     t_uint16 milisecs( 0 );
     t_int16 second( -1 );
     t_uint32 waitMillis( eachSecond * 1000 + wDeltaHalf );

     static sRawDtTm current;
     static char outBuf[ 80 ];
     static char strEndLine[ 24 ];

     currentTimeZone = (tzname[ 0 ]) ? (tzname[ 0 ]) : "unknown";

     cntp_GetTimeOfDay( current );
     now = current.secs;
     milisecs = current.milisecs;

     if ( opt.isVerbose || showMillis ) {
	 struct tm* pTM( localtime( &now ) );
	 struct tm* pGM( nil );
	 struct tm keepLocal;
	 char* strTodDate( showMillis ? tod_date_miliseconds( milisecs ) : tod_date() );

	 memcpy(&keepLocal, pTM, sizeof(keepLocal));
	 pGM = gmtime( &now );
	 timeDiff = tp_time_diff( &keepLocal, pGM );

	 if ( opt.isVeryVerbose ) {
	     second = pGM->tm_sec;
	     if ( showTzName ) {
		 sprintf(outBuf, "[%s]\t%s\tLocal: %s",
			 currentTimeZone,
			 strTodDate,
			 keepLocal.tm_isdst ? "(daylight)" : "(std)");
	     }
	     else {
		 sprintf(outBuf, "%s\tLocal: %s",
			 strTodDate,
			 keepLocal.tm_isdst ? "(daylight)" : "(std)");
	     }
	 }
	 else {
	     gDateTime myTime;
	     DBGPRINT_MIN("timeDiff: %d, hour GMT: %02d, hour local: %02d, is: %02d\n\n",timeDiff,pGM->tm_hour,pTM->tm_hour,keepLocal.tm_hour);
	     tp_to_calendar_date( &keepLocal, myTime );

	     if ( showMillis ) {
		 strcpy(outBuf, rfc822_date_string( myTime, (int)milisecs, true, timeDiff ));

		 if ( milisecs < 400 ) {
		     waitMillis += (500+wDeltaHalf - milisecs);
		     if ( wDeltaHalf < 0 ) {
			 wDeltaHalf++;
		     }
		 }
		 else {
		     if ( milisecs > 1000-300 ) {
			 waitMillis -= 100;
			 wDeltaHalf = wDeltaInit;
		     }
		     // ...otherwise,
		     // between 400 and 700 ms: no change
		 }
	     }
	     else {
		 strcpy(outBuf, rfc822_date_string( myTime, -1, true, timeDiff ));
	     }

	     second = myTime.sec;
	 }
     }
     else {
	 ctime_trim_sec_string( now, sizeof(outBuf)-1, outBuf, second );
     }

     if ( showTickMark ) {
	 // TICK MARKS (clock-second-progress)
	 sprintf(strEndLine, "  %c  %c  %c     \r",
		 (second % 2 ? ((second > 30 ? ':' : '.')) : ' '),
		 opt.isVerbose ? tickmarks[ progress % nrTickmarks ] : ' ',
		 tickControlChar);
     }
     else {
	 strcpy( strEndLine, "\n" );
     }

     fprintf(fDump, "%s%s", outBuf, strEndLine);
     fflush( fDump );

     if ( eachSecond==0 ) break;

     if ( wMillisAverage ) {
	 wMillisAverage = (wMillisAverage + waitMillis) / 2;
     }
     else {
	 wMillisAverage = waitMillis;
     }

     // Go asleep briefly...
     gFileControl::Self().MiliSecSleep( waitMillis );
 }

 return 0;
}

////////////////////////////////////////////////////////////
int go (char** argv, char** envp, int& result)
{
 int error, thisError = -1;
 gArg arg( argv, envp );
 sOptCntp opt;
 int iVal;
 gString s;
 bool isVmSvc;
 bool isProxy;
 bool isDaemon( false );

 unsigned n;
 FILE* fOut( nil );
 FILE* fRepErr;

 gAltNetServer* netServer( nil );

 result = 0;

 arg.AddParams( 1, "-vv|--verbose -h|--help --version\
 -d:%d|--debug:%d\
 -k:%u|--keep:%u\
 -l:%s|--log-file:%s\
 -p:%u|--port:%u\
 -r:%s|--remote-host:%s\
 -m|--show-miliseconds\
 -u:%u|--uid:%u\
 --pid-file:%s\
 --stats\
 --deny-all\
" );
 // Parse command line
 error = arg.FlushParams();
 n = arg.N();
 arg.FindOptionOccurr( "verbose", opt.isVerbose, opt.isVeryVerbose );
 fRepErr = opt.isVerbose ? stderr : NULL;

 if ( arg.FindOption('h') || error!=0 ) {
     print_help( arg.Program() );
     return 0;
 }
 if ( arg.FindOption("version") ) {
     print_version( arg.Program() );
     return 0;
 }

 if ( arg.FindOption("debug",iVal) ) {
     if ( iVal<0 ) iVal = 0;
     if ( iVal>9 ) {
	 fprintf(stderr,"Required debug level: %d, using maximum: 9\n",iVal);
	 iVal = 9;
     }
     lGlobLog.dbgLevel = iVal;
 }
 else {
     lGlobLog.dbgLevel = opt.isVerbose ? (LOG_NOTICE+(opt.isVeryVerbose==true)) : LOG_WARN;
 }

 if ( arg.FindOption("uid",iVal) ) {
     if ( iVal<0 || iVal>MAX_INT16_I ) {
	 fprintf(stderr,"Invalid uid: %d\n",iVal);
	 return 1;
     }
     if ( my_helper_setuid( (unsigned)iVal ) ) {
	 perror( "Cannot setuid" );
	 return 1;
     }
 }

 if ( arg.FindOption( "pid-file", xMeasures.common.sPidFile ) ) {
     if ( xMeasures.common.sPidFile[ 1 ]!=gSLASHCHR ) {
	 fprintf(stderr,"Wrong pid-file: start with " gSLASHSTR "\n");
	 return 1;
     }
 }

 opt.doShowMillis = arg.FindOption( 'm' );

 int eachSecond( 0 );
 if ( arg.FindOption( "keep", eachSecond ) ) {
     if ( eachSecond<1 ) {
	 eachSecond = 1;
     }
 }

 if ( (opt.doShowStats = arg.FindOption("stats"))==true ) {
     opt.fStat = stdout;
 }

 if ( arg.FindOption("deny-all") ) {
     iGlobDenyAll = 1;
 }

 if ( arg.FindOption("log-file",opt.sLogFile) ) {
     if ( lGlobLog.SetName( opt.sLogFile )==false ) {
	 fprintf(stderr,"Cannot use file: %s\n",opt.sLogFile.Str());
	 return 1;
     }
     fGlobOut = lGlobLog.Stream();
 }

 if ( (opt.anyPort = arg.FindOption("port",iVal))==true ) {
     opt.myPort = (t_gPort)iVal;
 }

 if ( arg.FindOption('r',opt.sRemoteHost) ) {
     if ( opt.BuildRemoteHost() ) {
	 fprintf(stderr,"Invalid remote-host specified; you should use HOST, or HOST:PORT\n");
	 return 1;
     }
 }

 if ( n==0 ) {
     thisError = simple_show( fOut, opt.anyPort && opt.myPort>1000, opt.remotePort, eachSecond, opt );
 }
 else {
     if ( opt.myPort<=0 ) {
	 fprintf(stderr,"Invalid listening port specified.\n");
	 return 0;
     }
 }

 // parsing nearly ends...
 gString sCmd( arg.Str( 1 ) );
 char* cmdStr( sCmd.Str() );
 arg.Delete( 1, 1 );

 if ( strncmp(cmdStr, "date", 4)==0 ) {
     if ( strcmp(cmdStr, "date-ux")==0 ) {
	 return dump_date_unix(arg, opt.isVerbose +  opt.isVeryVerbose*3);
     }
     return dump_date( fRepErr, cmdStr+4, arg, opt.isVerbose +  opt.isVeryVerbose*3 );
 }

 isVmSvc = strncmp(cmdStr,"vmsvc",5)==0;
 if ( isVmSvc ) {
     isDaemon = strcmp(cmdStr,"vmsvcd")==0;
 }
 isProxy = strcmp(cmdStr,"proxy")==0;

#ifdef iDOS_SPEC
 if ( strcmp(cmdStr,"xmeasures")==0 ) {
     fprintf(stderr,"DOS/Win32 does not support xmeasures!\n");
     return 1;
 }

 if ( isDaemon ) {
     fprintf(stderr,"Assuming vmsvc (instead of %s): daemon mode is not supported in Win32 executables.\n",cmdStr);
 }
 isDaemon = false;
#endif //iDOS_SPEC

 if ( strcmp(cmdStr,"xmeasures")==0 ) {
     thisError = do_prepare_xmeasures( arg, opt, xMeasures );

     netServer = set_server( opt.myPort );
     thisError = do_process_xmeasures( arg, netServer, isDaemon, fRepErr, opt, xMeasures );
 }

 if ( isVmSvc ) {
     if ( opt.sRemoteHost.IsEmpty()==false ) {
	 fprintf(stderr,"Ignoring remote-host!\n");
     }
     netServer = set_server( opt.myPort );
     thisError = do_process_vmsvc( arg, netServer, isDaemon, fOut, fRepErr, opt );
 }
 else {
     if ( isProxy ) {
	 if ( opt.sRemoteHost.IsEmpty() ) {
	     fprintf(stderr,"Remote-host not specified!\n");
	 }
	 else {
	     netServer = set_server( opt.myPort );
	     thisError = cxpxy_process_proxy( arg, netServer, isDaemon, fOut, fRepErr, opt );
	 }
     }
 }

 delete netServer;

 // fOut / fGlobOut are closed by ~gCGenLog

 if ( thisError==-1 ) {
     fprintf(stderr,"Invalid command: %s\n",cmdStr);
     return 1;
 }

 return thisError;
}

////////////////////////////////////////////////////////////
int main (int argc, char** argv, char** envp)
{
 int result( 0 );

 gINIT_SOCK;

 my_helper_signal_on( GX_SIGPIPE, false );  // Ignore SIGPIPE
 my_helper_signal_on( GX_SIGINT, true );
 my_helper_signal_on( GX_SIGTERM, true );

 go( argv, envp, result );

 gEND;

 DBGPRINT("DBG: pid %u bailing out\n",(unsigned)gFileControl::Self().CtrlGetPid());
 DBGPRINT_MIN("go: %d, result=%d\n",error,result);

 return result;
}

