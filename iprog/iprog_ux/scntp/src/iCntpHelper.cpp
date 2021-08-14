// iCntpHelper.cpp

#include <stdio.h>
#include <string.h>
#include <time.h>  // ctime, etc
#include <sys/types.h>

#include "iCntpHelper.h"

#include "iXmeasrv.h"

////////////////////////////////////////////////////////////

extern void MySignalHandler (int signalId) ;

////////////////////////////////////////////////////////////
int my_bye (FILE* fRepErr, int signalId, int mask, gList& optActions)
{
 unsigned iter( 1 ), n( optActions.N() );
 gElem* ptrElem( optActions.StartPtr() );

 // mask is currently unused

 if ( fRepErr ) {
     fprintf(fRepErr,"Unable to bind server");
     for ( ; ptrElem; iter++, ptrElem=ptrElem->next) {
	 fprintf(fRepErr,"%s%s%s%s",
		 iter==1 ? " (" : "\0",
		 ptrElem->Str(),
		 iter<n ? " " : "\0",
		 iter>=n ? ")" : "\0");
     }
     fprintf(fRepErr,": bailing out.\n");
 }
 return 0;
}


int my_bailout (int signalId)
{
 gString sPidFile( xMeasures.common.sPidFile );

 flush_log_a();

 my_helper_signal_on( GX_SIGALRM, false );
 my_helper_signal_on( GX_SIGTERM, false );
 MY_DEF_LOG( "cntp-server (signal %d)%s",
	     signalId,
	     signalId==GX_SIGTERM ? " - killed" : "\0");

 if ( sPidFile.Length() ) {
     remove( sPidFile.Str() );
     printf("Removed pid-file: %s\n",sPidFile.Str());
 }
 return 0;
}


int my_start_daemon (int aPid, int mask)
{
 int error( 0 );
 gString sPidFile( xMeasures.common.sPidFile );

 if ( sPidFile.Length() ) {
     FILE* fNew( fopen( sPidFile.Str(), "wt" ) );
     if ( fNew ) {
	 fprintf(fNew,"%d",aPid);
	 fclose( fNew );
     }
     else {
	 error = 4;
     }
 }

 flush_log_a();
 return error;
}


int my_helper_signal_on (int signalId, bool doSignal)
{
 if ( signalId<0 ) return -1;
#ifdef iDOS_SPEC
 ;
#else
 if ( doSignal )
     signal( signalId, MySignalHandler );
 else
     signal( signalId, SIG_IGN );
#endif
 return 0;
}

int my_helper_signal_hup (bool doSignal)
{
 return my_helper_signal_on( GX_SIGHUP, doSignal );
}

int my_helper_signal_ignore (int signalId)
{
#ifdef iDOS_SPEC
 ;
#else
 DBGPRINT("DBG: my_helper_sig_ignore (%d)\n",signalId);
 signal( signalId, SIG_IGN );
#endif
 return 0;
}

unsigned my_helper_alarm (unsigned seconds)
{
#ifdef iDOS_SPEC
 return 0;
#else
 DBGPRINT("DBG: my_alarm(%u)\n",seconds);
 return alarm( seconds );
#endif
}

int my_helper_setuid (unsigned myUserId)
{
 int result( 0 );

#ifdef iDOS_SPEC
 return result;
#else
 uid_t uid( (uid_t)myUserId );

 result = seteuid( uid );
 DBGPRINT("DBG: seteuid(%u) returned %d\n",myUserId,result);
 result += setuid( uid );
 DBGPRINT("DBG: setuid(%u) code %d\n",myUserId,result);
 return result;
#endif //~iDOS_SPEC
}

char* my_helper_ctime_current ()
{
 time_t currentEpoch;
 int len;
 char c;
 char* str;
 currentEpoch = time( NULL );
 str = ctime( &currentEpoch );
 if ( str==NULL || str[0]==0 ) return str;
 for (len = strlen(str); len>1 && (c = str[ --len ])<=' '; ) {
     str[ len ] = 0;
 }
 return str;
}

////////////////////////////////////////////////////////////
// Aux. functions
////////////////////////////////////////////////////////////
int limits (int num, int aMin, int aMax)
{
 if ( num < aMin ) return aMin;
 if ( num > aMax ) return aMax;
 return num;
}


int set_time_hhmmss (const char* strTime, gDateTime& myTime)
{
 gParam timed( (char*)strTime, ":" );

 myTime.hour = limits( atoi( timed.Str( 1 ) ), 0, 23 );
 myTime.minu = limits( atoi( timed.Str( 2 ) ), 0, 59 );
 myTime.sec = limits( atoi( timed.Str( 3 ) ), 0, 60 );
 if ( myTime.sec >= 60 && myTime.hour != 23 ) {
     myTime.sec = 59;
 }
 return timed.N()<2;
}


int month_hashed (int hash)
{
 static const t_int16 hashedMonths[ 14 ]={
	0,
	2590,	// 1
	1618,	// 2
	3362,	// 3
	530,	// 4
	3369,	// 5
	2910,	// 6
	2908,	// 7
	599,	// 8
	4960,	// 9
	3908,	// 10
	3846,	// 11
	1107,	// 12
	-1
 };
 t_int16 tic( (t_int16)hash );

 // Algorithm to obtain this hash is to multiply 'uppercase letter - 65 + 1' by 16.

 if ( hash < 0 || hash > 19683 ) return -1;  // 19683 is (26+1)^3

 for (int iter=1; iter<=12; iter++) {
     if ( hashedMonths[ iter ]==tic ) return iter;
 }
 return 0;
}


int guess_date (gList& args, gDateTime& myTime, int& errorWhere)
{
 int error( -1 );
 int iter( 1 ), nIter( (int)args.N() );
 t_uint32 uValue( 0 );
 gElem* pElem( nil );

 gList split;
 gList vals;

 errorWhere = 0;

 for (iter=1; iter<=nIter; iter++) {
     gParam tup( args.Str( iter ), "-" );
     for (pElem=tup.StartPtr(); pElem; pElem=pElem->next) {
	 split.Add( pElem->Str() );
     }
 }

 for (iter=1, nIter=(int)split.N(); iter<=nIter; iter++) {
     gString s( split.Str( iter ) );
     int pseudo( 0 );
     int val;

     error = gStorageControl::Self().ConvertToUInt32( s.Str(), uValue );
     if ( error ) {
	 s.UpString();
	 error = -1;

	 for (int idx=1; idx<=(int)s.Length(); idx++) {
	     if ( s[ idx ]<'A' || s[ idx ]>'Z' ) {
		 switch ( s[ idx ] ) {
		 case ',':
		     s[ idx ] = ' ';
		     break;
		 default:
		     error = -1;
		     break;
		 }
		 break;
	     }
	 }

	 if ( error ) {
	     val = (int)(s[ 1 ] - 'A') + 1;
	     if ( val > 0 ) {
		 pseudo += val;
		 pseudo *= 16;
		 val = (int)(s[ 2 ] - 'A') + 1;
		 if ( val > 0 ) {
		     pseudo += val;
		     pseudo *= 16;
		     val = (int)(s[ 3 ] - 'A') + 1;
		     if ( val > 0 ) {
			 pseudo += val;
			 pseudo *= -1;
			 error = 0;
		     }
		 }
	     }
	 }//end only alpha
     }//end IF (not a number)
     else {
	 pseudo = (int)uValue;
     }

     if ( error ) {
	 if ( errorWhere==0 ) {
	     errorWhere = iter;
	 }
     }
     else {
	 DBGPRINT("DBG: iter: %d, pseudo: %d\n", iter, pseudo);
	 vals.Add( pseudo );
     }
 }

 int month( -1 );
 int year( -1 );
 gList usedPoint;

 for (pElem=vals.StartPtr(); pElem; pElem=pElem->next) {
     int xpt( pElem->me->iValue );
     int idx( month_hashed( -xpt ) );
     int used( 1 );
     if ( idx >= 1 ) {
	 if ( month < 0 ) {
	     month = idx;
	 }
     }
     else {
	 if ( xpt >= 1970 ) {
	     year = xpt;
	 }
	 else {
	     used = 0;
	 }
     }
     usedPoint.Add( "." );
     if ( used ) {
     }
     else {
	 usedPoint.EndPtr()->me->iValue = xpt;
     }
 }

 DBGPRINT("DBG: month: %d, year: %d, vals (#%u): %s\n",
	  month, year,
	  vals.N(), vals.Str( 1 ));

 // Oieee, month found?
 if ( month >= 1 ) {
     myTime.month = month;
 }
 if ( year > 0 ) {
     myTime.year = year;
 }

 for (iter=1, nIter=(int)vals.N(); iter<=nIter; iter++) {
     int value( usedPoint.GetObjectPtr( iter )->iValue );
     if ( value > 0 ) {
	 // Kind'a sloppy method here...
	 myTime.day = value;
     }
 }

#if 0
 usedPoint.Show();
 vals.Show();
 split.Show();
#endif

 int leftStrings( (int)args.N() - usedPoint.N() );
 // if leftStrings is one, probably we are still missing the HH:MM or HH:MM:SS
 DBGPRINT("DBG: leftStrings=%d, args.N()=%u\n", leftStrings, args.N());

 if ( leftStrings==-1 || leftStrings==1 ) {
     set_time_hhmmss( args.Str( (int)args.N() ), myTime );
 }

 DBGPRINT("DBG: guess_date errorWhere=%d, year=%d, current: " FMT_DTTM "\n\n",
	  errorWhere,
	  year,
	  myTime.year, myTime.month, myTime.day,
	  myTime.hour, myTime.minu, myTime.sec);
 return year < 1970;  // 1 means an error
}



int my_client_check (gCNetSource& clientSource,
		     FILE* fRepErr,
		     gString& sMsg)
{
 DBGPRINT_MIN("DBG: Client connected (%d): %s [id?%d:%s]\n",
	      clientSource.handleIn,
	      clientSource.String(),
	      clientSource.sourceGuessId,
	      clientSource.sourceGuessName.Str());
 // Currently no IP check is performed
 if ( iGlobDenyAll ) return -1;
 return 0;
}


char* my_client_source (gCNetSource& aClient, int codex, int mask)
{
 static char bufName[ 256 ];
 char* strSource( (char*)aClient.String() );

 // TODO: some minimal cache here

 bufName[ 0 ] = 0;
 if ( strSource==nil ) return bufName;

 snprintf( bufName, 128, "%s", strSource );
 return bufName;
}

////////////////////////////////////////////////////////////

