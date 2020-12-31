// probability.cpp

#include "probability.h"

////////////////////////////////////////////////////////////
// Aux functions
////////////////////////////////////////////////////////////
void im_init ()
{
}


int im_atoi (const char* aString)
{
 if ( aString ) {
     return atoi( aString );
 }
 return 0;
}


int im_atoi_default (const char* aString, int valueOnError)
{
 if ( aString ) {
     return atoi( aString );
 }
 return valueOnError;
}


int im_power (int x, int y)
{
 int num( 1 );
 if ( x ) {
     if ( y<=0 ) {
	 return 1;
     }
     for ( ; y>0; y--) {
	 num *= x;
     }
 }
 else {
     // if ( y==0 ) return -1 ==> should be an error, but we just return 0
     return 0;
 }
 return num;
}


double im_power_d (double x, double y)
{
 double num( 1 );
 double a( y );

 if ( (double)((int)y) != y ) {
     return -1;
 }
 if ( x ) {
     if ( y<0 ) {
	 a = -y;
     }
     for ( ; a>0; a--) {
	 num *= x;
     }
     if ( y<0 ) {
	 return 1.0 / num;
     }
 }
 else {
     // if ( y==0 ) return -1 ==> should be an error, but we just return 0
     return 0;
 }
 return num;
}


int im_factorial (int x)
{
 int count( 1 );
 double over( 1.0 );

 if ( x < 0 ) {
     return -1;
 }

 for ( ; x>1; x--) {
     count *= x;
     over *= (double)x;
     if ( (int)over != count ) {
	 return -1;
     }
 }
 return count;
}


double im_factorial_d (double d)
{
 double count( 1.0 );

 if ( d < 0.0 ) {
     return -1;
 }

 for ( ; d>1; d--) {
     count *= d;
     if ( count < 0 ) {
	 return -1;
     }
 }
 return count;
}


#ifndef PURE_MATH
int im_permutation (int n, int r)
{
 int num( 1 );

 // e.g. P(9,3) is 9 x 8 x 7

 if ( n < 0 || r < 0 ) {
     return -1;
 }
 if ( n < r ) {
     return -1;
 }
 for ( ; r>0; r--) {
     num *= n;
     n--;
 }
 return num;
}
#else
int im_permutation (int n, int r)
{
 int q;
 int num;
 if ( n < 0 || r < 0 ) {
     return -1;
 }
 q = im_factorial( n - r );
 if ( q==-1 ) {
     return -1;
 }
 ASSERTION(q!=0,"q!=0");
 num = im_factorial( n );
 if ( num < 1 ) {
     return -1;
 }
 return num / q;
}
#endif  //PURE_MATH


double im_permutation_d (double n, double r)
{
 double num( 1 );

 // see also im_permutation()

 if ( n < 0 || r < 0 ) {
     return -1;
 }
 if ( n < r ) {
     return -1;
 }
 for ( ; r>0; r--) {
     num *= n;
     n--;
 }
 return num;
}



int im_combination (int n, int r)
{
 double result;
 result = im_combination_d( (double)n, (double)r );
 if ( result >= (double)MAX_LONG_L ) {
     // Overflow
     return -1;
 }
 return (int)result;
}


#ifndef PURE_MATH
double im_combination_d (double n, double r)
{
 double iter;
 double d( n-r );
 double mirror( r );
 double num( 1 );

 // C(16,3) is
 //	16! / (3! * (16-3)!) = 16! / (3! * 13!)
 //
 // We can simplify this by the same result, which is:
 //	16 x 15 x 14 / 3!

 if ( n < 0 || r < 0 ) {
     return -1;
 }

 // Take advantage of symmetry in Pascale's Triangle
 //
 // HINT:	http://mathforum.org/dr.math/faq/faq.comb.perm.html

 if ( d < r ) {
     mirror = d;
     d = r;
 }

 for (iter=n; iter>d; iter--) {
     num *= iter;
     IMH_PRINT("C(%d,%d), d=%d, combination without repetition, numerator: %0.1f\n",
	       (int)n, (int)r,
	       (int)d,
	       num);
 }

 for ( ; mirror>1; mirror--) {
     num /= mirror;
 }

 return num;
}
#else
double im_combination_d (double n, double r)
{
 double u;
 double q1, q2;

 if ( n < 0 || r < 0 ) {
     return -1;
 }
 if ( n < r ) {
     // Invalid!
     return -1;
 }
 u = im_factorial_d( n );
 q1 = im_factorial_d( r );
 q2 = im_factorial_d( n - r );

 return u / (q1 * q2);
}
#endif //PURE_MATH

////////////////////////////////////////////////////////////

