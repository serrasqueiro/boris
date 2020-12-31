// ilstring.cpp

#include <string.h>

#include "ilstring.h"

////////////////////////////////////////////////////////////
// ils_... (string manip. functions)
////////////////////////////////////////////////////////////
int ils_string_sane (gString& s, int optMask, char substChr)
{
 int countSubst( 0 );
 char* str( s.Str() );
 char chr( 0 );

 ASSERTION(optMask==0 || optMask==1,"ils_string_sane");
 // optMask 1 can be used for Unix filenames

 switch ( optMask ) {
 case 0:
     for (t_uchar uChr; (uChr = (t_uchar)*str)!=0; str++) {
	 if ( uChr<' ' || (uChr>=127 && uChr<0xA0) || uChr==255 ) {
	     *str = substChr;
	     countSubst++;
	 }
     }
     break;

 case 1:
 default:
     for ( ; (chr = *str)!=0; str++) {
	 if ( chr<' ' || chr>=127 ) {
	     *str = substChr;
	     countSubst++;
	 }
     }
     break;
 }

 return countSubst;
}


int ils_string_toint (const char* str, int optMask, int defaultValueOnError)
{
 int value;

 // optMask:
 //	0: normal atoi
 //	1: seek first digit (best-effort)

 if ( str==nil ) return defaultValueOnError;
 value = atoi( str );

 if ( defaultValueOnError==0 )
     return value;

 if ( value==0 ) {
     if ( str[0]=='0' ) return 0;
     if ( optMask & 1 ) {
	 for (char chr; (chr = *str)!=0; str++) {
	     if ( (chr=='-') || (chr>='0' && chr<='9') )
		 return ils_string_toint( str, 0, defaultValueOnError );
	 }
     }
     return defaultValueOnError;
 }

 return value;
}


t_int64 ils_string_toint64 (const char* str, int optMask, t_int64 defaultValueOnError)
{
 if ( str==nil ) return defaultValueOnError;
 ASSERTION(optMask==0,"ils_string_toint64");

 long aLong( atol( str ) );
 t_int64 value( (t_int64)aLong );

 if ( defaultValueOnError==0 )
     return value;

 if ( aLong==0 ) {
     if ( str[0]=='0' ) return 0;
     return defaultValueOnError;
 }

 return value;
}

////////////////////////////////////////////////////////////

