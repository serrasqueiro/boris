#ifndef PROBABILITY_X_H
#define PROBABILITY_X_H

#include "lib_iobjs.h"

////////////////////////////////////////////////////////////
#ifndef IMH_PRINT
#define IMH_PRINT(args...) DBGPRINT(args)
#endif

////////////////////////////////////////////////////////////

extern void im_init () ;

extern int im_atoi (const char* aString) ;
extern int im_atoi_default (const char* aString, int valueOnError) ;

extern int im_power (int x, int y) ;
extern double im_power_d (double x, double y) ;

extern int im_factorial (int x) ;
extern double im_factorial_d (double d) ;

extern int im_permutation (int n, int r) ;
extern double im_permutation_d (double n, double r) ;

extern int im_combination (int n, int r) ;
extern double im_combination_d (double n, double r) ;


////////////////////////////////////////////////////////////

#endif //PROBABILITY_X_H

