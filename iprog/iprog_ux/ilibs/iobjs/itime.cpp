// itime.cpp

#include <time.h>
#include <sys/time.h>  // gettimeofday,...
#include <math.h>  //round,ceil,...

#include "itime.h"
////////////////////////////////////////////////////////////
// gTimer - Generic timer classes
// ---------------------------------------------------------
gTimer::gTimer (eTimerPrec aPrecision)
    : tic( 0.0 ),
      ticStart( 0 ),
      ticLast( 0 )
{
 Start();
}


gTimer::~gTimer ()
{
}


double gTimer::GetSecondsFromTics (t_gTicElapsed aElapsed)
{
 double q, divider;
 // ANSI C: POSIX requires that CLOCKS_PER_SEC equals 1000000
 divider = (double)CLOCKS_PER_SEC;
 ASSERTION(divider>0,"divider>0");
 q = (double)aElapsed;
 return q / divider;
}


void gTimer::Start ()
{
 ; // clock_t type from clock...
 ticStart = (t_gTicElapsed)clock();
 tic = (double)ticStart;
}
////////////////////////////////////////////////////////////
gTimerTic::gTimerTic ()
    : gTimer( gTimer::e_PrecTic ),
      ticDiff( 0 )
{
 thisGetTime( timeStart );
}


gTimerTic::~gTimerTic ()
{
}


t_gTicElapsed gTimerTic::CpuTics ()
{
 clock_t uClock = clock();
 ASSERTION(uClock!=(clock_t)-1,"uClock!=-1");
 ticLast = (t_gTicElapsed)uClock;
 if ( ticLast < ticStart ) {
     ticLast += CLOCKS_PER_SEC;
 }
 ASSERTION(ticLast>=ticStart,"ticLast>=ticStart");
 ticDiff = ticLast - ticStart;
 tic = (double)ticDiff;
 ticStart = ticLast;
 GetTimeNow( timeEnd );
 return ticDiff;
}


double gTimerTic::GetElapsedMilisec ()
{
 return GetMilisecsDiff( timeEnd, timeStart );
}


t_uint32 gTimerTic::GetMilisec ()
{
 // Same as GetElapsedMilisec, but here an integer...
 return (t_uint32)( ceil( GetElapsedMilisec() ) );
}


void gTimerTic::Reset ()
{
 ticDiff = ticLast = 0;
 ticStart = (t_gTicElapsed)clock();
 timeStart.Copy( timeEnd );
}


double gTimerTic::GetMilisecsDiff (sTimeSeconds& aTime1, sTimeSeconds& aTime0)
{
 double total1 = aTime1.GetMiliSecs();
 double total0 = aTime0.GetMiliSecs();
 //ASSERTION(total1>=total0,"total1>=total0") ==>  // CpuTics not called
 if ( total1>=total0 ) return total1-total0;
 return 0.0;
}


int gTimerTic::thisGetTime (sTimeSeconds& aTime)
{
 static struct timeval aTimeVal;
 int result;
 aTime.ToDefault();
#ifdef linux
 result = gettimeofday( &aTimeVal, NULL );
#else
 result = 0;
#endif //linux
 if ( result!=0 ) return result;
 aTime.sec = aTimeVal.tv_sec;
 aTime.microSec = aTimeVal.tv_usec;
 DBGPRINT_MIN("DBG: sec.microSec: %lu.%06lu (result=%d)\n",
	      (unsigned long)aTime.sec, (unsigned long)aTime.microSec,
	      result);
 return 0;
}
////////////////////////////////////////////////////////////

