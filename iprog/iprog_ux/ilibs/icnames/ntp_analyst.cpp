// ntp_analyst.cpp

#include "ntp_analyst.h"

//  Return Modified Julian Date given calendar year,
//  month (1-12), and day (1-31). See sci.astro FAQ.
//  - Valid for Gregorian dates from 17-Nov-1858.
//
long nys_DateToMjd (int y, int m, int day)
{
 if ( y>=1858 && m>=1 && m<=12 && day>=1 && day<=31 ) {
    return
        367 * y
        - 7 * (y + (m + 9) / 12) / 4
        - 3 * ((y + (m - 9) / 7) / 100 + 1) / 4
        + 275 * m / 9
        + day
        + 1721028
        - 2400000;
 }
 return -1;
}

//  Calculate number of seconds since 1-Jan-1900.
//  - Ignores UTC leap seconds!
//
t_uint64 nys_SecondsSince1900 (int y, int m, int day)
{
 long days = nys_DateToMjd(y, m, day) - nys_DateToMjd(1900, 1, 1);
 return (t_uint64)days * 86400;
}

