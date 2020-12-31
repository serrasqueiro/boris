// gmath.cpp

#include <math.h>

#include "imath.h"
#include "icalendar.h"
////////////////////////////////////////////////////////////
gIntDegree::gIntDegree (int d)
    : gInt( d ),
      rValue( 0 )
{
}

gIntDegree::~gIntDegree ()
{
}

int gIntDegree::NearestInt (float aValue)
{
 float absValue = aValue;
 if ( aValue<0 ) absValue = -aValue;
 int iVal = (int)absValue, oVal = iVal;
 oVal += (absValue - (float)iVal > 0.5);
 return aValue>=0 ? oVal : -oVal;
}

gIntDegree::gIntDegree (gIntDegree& copy)
{
 SetInt( copy.GetInt() );
}

gIntDegree& gIntDegree::operator= (gIntDegree& copy)
{
 SetInt( copy.GetInt() );
 return *this;
}
////////////////////////////////////////////////////////////
gDegreeCelsius::gDegreeCelsius (int d)
    : gIntDegree( d )
{
}

gDegreeCelsius::~gDegreeCelsius ()
{
}

int gDegreeCelsius::ConvertToFahrenheit (int d)
{
 // F to Celsius:
 //   (5/9)*(DegreeF-32)

 // Celsius to F
 //   (1.8*deg C)+32

 rValue = 1.8 * (float)d + 32.0;
 return NearestInt( rValue );
}
////////////////////////////////////////////////////////////
gDegreeFh::gDegreeFh (int d)
    : gIntDegree( d )
{
}

gDegreeFh::~gDegreeFh ()
{
}

int gDegreeFh::ConvertToCelsius (int d)
{
 // F to Celsius:
 //   (5/9)*(DegreeF-32)
 rValue = (float)(d-32) * 5.0 / 9.0;
 return NearestInt( rValue );
}
////////////////////////////////////////////////////////////
gRandom::gRandom (unsigned uVal)
    : gInt( 0 ),
      uRange( uVal )
{
 SetInt( (int)thisGetRandom( uRange ) );
}

gRandom::~gRandom ()
{
}

void gRandom::Reset ()
{
 SetInt( (int)thisGetRandom( uRange ) );
}

void gRandom::SetSeed (unsigned uVal)
{
 srand( uVal );
}

void gRandom::GarbleSeed (unsigned uVal)
{
 gDateTime aDtTm( gDateTime::e_Now );
 uVal += (unsigned)aDtTm.GetTimeStamp();
 SetSeed( uVal );
}

unsigned gRandom::thisGetRandom (unsigned uVal)
{
 unsigned uRand;
 if ( uVal==0 ) return 0;
 uRand = rand() % uVal;
 return uRand;
}
////////////////////////////////////////////////////////////

