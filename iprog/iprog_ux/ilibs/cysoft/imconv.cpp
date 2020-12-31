// imconv -- Version 0.1
//
// Inspired on (c) 2009  gmconv.cpp, by H. Moreira

#include <stdio.h>
#include <string.h>

#include "imconv.h"
////////////////////////////////////////////////////////////
// Static members
// ---------------------------------------------------------
const char gMBase52::digibet[]="0123456789!%-@ABCDEFGH?JKLMN?PQRST?VWXYZabcde?ghi?k?m??pq?st????y?";
t_int8 gMBase52::hashibet[ 256 ]={ -2 };
t_int8 gMBase52::digi2alphabet[ 256 ];
t_int8 gMBase52::alpha2digibet[ 256 ];
const char gMBase52::symbols[ 54 ]="0123456789!%-@ABCDEFGHJKLMNPQRSTVWXYZabcdeghikmpqsty";
const char gMBase52::alphaSymbols[ 54 ]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
const int gMBase52::trigitMin=52*52;
////////////////////////////////////////////////////////////
// gMString - Base52 string manipulation
// ---------------------------------------------------------
gMBase52::gMBase52 (const char* aStr)
    : gString( aStr )
{
 thisRehashibet();
}


gMBase52::gMBase52 (gMBase52& copy)
    : gString( copy )
{
 thisRehashibet();
}


gMBase52::~gMBase52 ()
{
}


bool gMBase52::IsOk ()
{
 int value( -1 );
 return thisConvertBase52ToInt( str, Length(), false, value )==0;
}


bool gMBase52::ValidateSymbols ()
{
 // Run through each one of the digibet entries,
 // except '?', and check they match 'symbols' at right position.

 int iter( 0 ), symIter( 0 );
 char chr;
 for ( ; (chr = digibet[ iter ])!=0; iter++) {
     if ( chr=='?' ) continue;
     if ( chr!=symbols[ symIter ] ) return false;
     symIter++;
 }
 return true;
}


int gMBase52::ToInt ()
{
 int value( -1 );
 thisConvertBase52ToInt( str, Length(), false, value );
 return value;
}


int gMBase52::FromInt (int value)
{
 return thisConvertIntToBase52( iValue = value, *this );
}


char* gMBase52::FromTextStr (const char* aStr)
{
 // In this implementation each 2 bytes will be transformed into 3 bytes
 // (leading to nearly a 50% efficiency in transmission).
 // An even number of bytes in the original string:
 // the string is considered having an extra '\0'.
 // a zero length will be transformed into a single letter z.
 // Less common, but accepted, a NULL string is represented by letter n.

 int strIter( 0 );
 t_uchar* uStr( (t_uchar*)aStr );
 t_uint16 firstChr;
 t_uint16 secondChr;
 gString sTrigit;

 sUni.SetEmpty();

 if ( aStr==nil ) {
     sUni.Add( 'n' );
 }
 else {
     if ( aStr[ 0 ]==0 ) {
	 sUni.Add( 'z' );
     }
     else {
	 for ( ; (firstChr = (t_uint16)uStr[ strIter ])!=0; strIter++) {
	     secondChr = (t_uint16)uStr[ ++strIter ];
	     thisConvertIntToBase52( (int)(trigitMin + firstChr * 256 + secondChr), sTrigit );
	     sUni.AddString( sTrigit );
	     if ( secondChr==0 ) break;
	 }
     }
 }

 return sUni.Str();
}


char* gMBase52::ToTextStr (const char* aStr)
{
 // This method is idem-potent to 'FromTextStr'
 int error( 2 );
 int iter( 0 ), value( -1 );
 t_uchar* uStr( (t_uchar*)aStr );
 t_uchar trigits[ 4 ];
 t_uchar firstChr;
 t_uchar secondChr;

 sUni.SetEmpty();
 memset( trigits, 0x0, sizeof(trigits) );

 if ( aStr==nil || strcmp( aStr, "n" )==0 ) return nil;
 if ( strcmp( aStr, "z" ) ) {
     // Not an empty encoded string. Proceed.
     // We catch trigits, i.e. three base-52 chars

     for ( ; (trigits[ 0 ] = uStr[ iter ])!=0; ) {
	 error = (trigits[ 1 ] = uStr[ ++iter ])==0;
	 error += (trigits[ 2 ] = uStr[ ++iter ])==0;
	 iter++;
	 if ( error ) break;
	 error = thisConvertBase52ToInt( trigits, 3, false, value );
	 value -= trigitMin;
	 if ( error || value<0 ) break;
	 secondChr = value & 0xFF;
	 value >>= 8;
	 firstChr = value & 0xFF;  // Note: &0xFF not necessary

	 sUni.Add( firstChr );
	 sUni.Add( secondChr );
     }
     if ( error ) return nil;
 }

 return sUni.Str();
}


char* gMBase52::ShowAlpha ()
{
 int iter( 0 );
 char aChr;
 char* aStrUni;
 sUni.Set( str );

 for (aStrUni=sUni.Str(); (aChr = aStrUni[ iter ])!=0; iter++) {
     if ( digi2alphabet[ (int)((t_uchar)aChr) ]<0 )
         aChr = '?';
     else
         aChr = digi2alphabet[ (int)((t_uchar)aChr) ];
     aStrUni[ iter ] = aChr;
 }
 return aStrUni;
}


char* gMBase52::ShowDigibet ()
{
 int iter( 0 );
 char aChr;
 char* aStrUni;
 sUni.Set( str );

 for (aStrUni=sUni.Str(); (aChr = aStrUni[ iter ])!=0; iter++) {
     if ( alpha2digibet[ (int)((t_uchar)aChr) ]<0 )
         aChr = '?';
     else
         aChr = alpha2digibet[ (int)((t_uchar)aChr) ];
     aStrUni[ iter ] = aChr;
 }
 return aStrUni;
}


gMBase52& gMBase52::operator= (gMBase52& copy)
{
 Set( copy.Str() );
 iValue = copy.iValue;
 return *this;
}


int gMBase52::operator!= (gMBase52& copy)
{
 return Match( copy )==false;
}


int gMBase52::thisConvertBase52ToInt (const t_uchar* uStr, int length, bool ignoreErrors, int& value)
{
 // Returns 0 on success
 int iter( 0 ), thisDigit;

 for (value=0; iter<length; iter++) {
     value *= 52;
     thisDigit = (int)hashibet[ uStr[ iter ] ];
     if ( thisDigit<0 ) {
         if ( ignoreErrors ) continue;
         return -1;
     }
     value += thisDigit;
     DBGPRINT_MIN("DBG: ToInt('%s',...), value is now: %d (thisDigit=%d)\n",
                  uStr,
                  value,
                  thisDigit);
 }
 return 0;
}


int gMBase52::thisConvertIntToBase52 (int value, gString& sResult)
{
 t_uint8 mod;
 bool isNegative( value<0 );
 int iter( 0 ), iterBack, chrPos( 0 );
 char symBuf[ 1024 ];

 sResult.SetEmpty();
 if ( isNegative ) value = -value;

 do {
     mod = value % 52;
     symBuf[ chrPos++ ] = symbols[ mod ];
     //printf("mod:\t%d,\tsymbols[x]: %c\n", mod, symbols[ mod ]);
     symBuf[ chrPos ] = 0;
     value /= 52;
     if ( chrPos>=1023 ) return -1;  // Buffer overflow is academic!
 } while ( value>0 );

 for (iter=0, iterBack=chrPos; iter<chrPos; iter++) {
     sResult.Add( symBuf[ --iterBack ] );
 }
 return 0;
}


int gMBase52::thisRehashibet ()
{
 if ( hashibet[ 0 ]>=-1 ) return 0;  // Already hashed

 gString sDigits( (char*)symbols );
 gString sAlpha( (char*)alphaSymbols );
 t_int8 chrValue;

 for (int iter=0; iter<256; iter++) {
     hashibet[ iter ] = -1;
     digi2alphabet[ iter ] = -1;
     alpha2digibet[ iter ] = -1;
     if ( iter>' ' ) {
         chrValue = (t_int8)(sDigits.Find( (char)iter ))-1;
         hashibet[ iter ] = chrValue;
         if ( chrValue>=0 ) {
             // digi2alphabet['0']=-1, but digi2alphabet['A']=0, 'B'=1, etc.
             digi2alphabet[ iter ] = (t_int8)alphaSymbols[ (int)((t_uint8)chrValue) ];
         }

         chrValue = (t_int8)(sAlpha.Find( (char)iter ))-1;
         if ( chrValue>=0 ) {
             alpha2digibet[ iter ] = (t_int8)symbols[ (int)((t_uint8)chrValue) ];
         }
     }
 }
 return 1;
}
////////////////////////////////////////////////////////////

