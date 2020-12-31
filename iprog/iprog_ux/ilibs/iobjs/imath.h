#ifndef iMATH_X_H
#define iMATH_X_H

#include "istorage.h"

#define PRIME_MILLION_NP1	1000003
#define PRIME_MILLION_NP0	 999983

////////////////////////////////////////////////////////////
// CLASS gIntDegree - abstract
////////////////////////////////////////////////////////////
class gIntDegree : public gInt {
public:
    virtual ~gIntDegree () ;

    // Public members
    float rValue;

    // Get methods
    int NearestInt (float aValue) ;

    // Set methods
    virtual int Convert () = 0;  //PURE

    // Operators,etc
    gIntDegree (gIntDegree& copy) ;
    gIntDegree& operator= (gIntDegree& copy) ;

protected:
    gIntDegree (int d) ;
};
////////////////////////////////////////////////////////////
// CLASS gDegreeCelsius, etc
////////////////////////////////////////////////////////////
class gDegreeCelsius : public gIntDegree {
public:
    gDegreeCelsius (int d=0) ;
    virtual ~gDegreeCelsius () ;

    virtual int Convert () {
	return ConvertToFahrenheit( GetInt() );
    }

    virtual int ConvertToFahrenheit (int d) ;

private:
    // Operators,empty
    gDegreeCelsius (gDegreeCelsius& ) ; //empty
    gDegreeCelsius& operator= (gDegreeCelsius& ) ; //empty
};

class gDegreeFh : public gIntDegree {
public:
    gDegreeFh (int d=0) ;
    virtual ~gDegreeFh () ;

    virtual int Convert () {
	return ConvertToCelsius( GetInt() );
    }

    virtual int ConvertToCelsius (int d) ;

private:
    // Operators,empty
    gDegreeFh (gDegreeFh& ) ; //empty
    gDegreeFh& operator= (gDegreeFh& ) ; //empty
};
////////////////////////////////////////////////////////////
class gRandom : public gInt {
public:
    gRandom (unsigned uVal=0) ;
    virtual ~gRandom () ;

    virtual void Reset () ;

    virtual void SetSeed (unsigned uVal) ;
    virtual void GarbleSeed (unsigned uVal=0) ;

protected:
    unsigned uRange;

    unsigned thisGetRandom (unsigned uVal) ;

private:
    // Operators,empty
    gRandom (gRandom& ) ; //empty
    gRandom& operator= (gRandom& ) ; //empty
};
////////////////////////////////////////////////////////////
#endif //iMATH_X_H

