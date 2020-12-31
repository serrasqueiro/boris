#ifndef iTIME_X_H
#define iTIME_X_H

#include "istorage.h"
////////////////////////////////////////////////////////////
struct sTimeSeconds {
    sTimeSeconds ()
	: sec( 0 ),
	  microSec( 0 ) {
    }
    t_uint32 sec, microSec;

    void ToDefault () {
	sec = microSec = 0;
    }
    double GetMiliSecs () {
	return (double)sec*1000.0 + (double)microSec/1000.0;
    }

    void Copy (sTimeSeconds& copy) {
	sec = copy.sec;
	microSec = copy.microSec;
    }

    // Operators,empty
    sTimeSeconds (sTimeSeconds& ) ; //empty
    sTimeSeconds& operator= (sTimeSeconds& ) ; //empty
};
////////////////////////////////////////////////////////////
class gTimer : public gUInt {
public:
    // Public enums
    enum eTimerPrec {
	e_PrecTic
    };

    gTimer (eTimerPrec aPrecision) ;
    virtual ~gTimer () ;

    // Public data-members
    double tic;

    // Get methods
    // .

    // Specific methods
    virtual double GetSecondsFromTics (t_gTicElapsed aElapsed) ;
    virtual double GetMilisecsFromTics (t_gTicElapsed aElapsed) {
	return GetSecondsFromTics( aElapsed )*1000.0;
    }

    // Set methods
    virtual void Start () ;

protected:
    t_gTicElapsed ticStart, ticLast;

private:
    // Operators,empty
    gTimer (gTimer& ) ; //empty
    gTimer& operator= (gTimer& ) ; //empty
};
////////////////////////////////////////////////////////////
class gTimerTic : public gTimer {
public:
    gTimerTic () ;
    virtual ~gTimerTic () ;

    // Get methods
    virtual t_gTicElapsed CpuTics () ;
    virtual double GetElapsedMilisec () ;
    virtual t_uint32 GetMilisec () ;
    // .

    // Set methods
    virtual void Reset () ;

    // Specific methods
    void GetTimeNow (sTimeSeconds& aTime) {
	thisGetTime( aTime );
    }
    double GetMilisecsDiff (sTimeSeconds& aTime1, sTimeSeconds& aTime0) ;

protected:
    t_gTicElapsed ticDiff;
    sTimeSeconds timeStart, timeEnd;

    int thisGetTime (sTimeSeconds& aTime) ;

private:
    // Operators,empty
    gTimerTic (gTimerTic& ) ; //empty
    gTimerTic& operator= (gTimerTic& ) ; //empty
};
////////////////////////////////////////////////////////////
#endif //iTIME_X_H

