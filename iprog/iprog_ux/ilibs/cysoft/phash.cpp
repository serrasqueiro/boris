// PyString hash
// (reused from Python 2.4.4)


#include <string.h>

#include "phash.h"


static long pystr_string_hash (PyStringObject *a) ;


#if defined(DEBUG_MIN)
#define DBGPRINT_HSH(args...) printf("%s:%d: ",__FILE__,__LINE__); printf(args);
#else
#define DBGPRINT_HSH(args...) ;
#endif

////////////////////////////////////////////////////////////
// Main classes
////////////////////////////////////////////////////////////
PyStringObject::PyStringObject (char* aStr)
    : gString( aStr ),
      hashValue( -1 )
{
}


PyStringObject::PyStringObject (PyStringObject& copy)
    : gString( copy.str ),
      hashValue( -1 )
{
}

PyStringObject::~PyStringObject ()
{
}

// Get methods
bool PyStringObject::IsOk ()
{
 return iValue==0;
}


long PyStringObject::ForceHash ()
{
 return pystr_string_hash( this );
}


bool PyStringObject::ValidateSymbols ()
{
 return IsOk();
}


int PyStringObject::ToInt ()
{
 return (int)ForceHash();
}

////////////////////////////////////////////////////////////
static long
pystr_string_hash (PyStringObject *a)
{
 register unsigned char *p;
 register long len, x;
 static long keepLength;

 if (a->hashValue != -1)
     return a->hashValue;

 keepLength = (long)a->Length();
 len = keepLength;
 p = (unsigned char *) a->Str();

 for (x=(*p << 7); --len>=0; p++) {
     x = (1000003*x) ^ (*p);
     DBGPRINT_HSH("DBG:\t%ld (len=%ld), {%s}\n",x,len,p);
 }
 x ^= keepLength;
 DBGPRINT_HSH("DBG:\t%ld\n",x);

 if (x == -1)
     x = -2;
 a->hashValue = x;
 return x;
}


long cys_str_hash (const char* str, int iLength)
{
 register t_uchar* p;
 register long len, x;

 len = (long)iLength;
 p = (t_uchar*)str;

 x=(*p << 7);
 while ( --len>=0 ) {
     x = (1000003*x) ^ *p++;
 }
 x ^= (long)iLength;
 if ( x==-1 ) return -2;
 return x;
}


long cys_string_hash (const char* str)
{
 // The compact-memory function would be just:
 //	PyStringObject words( (char*)str );
 //	return words.ForceHash();

 // ...but we want it faster!

 if ( str==nil )
     return -1;

 return cys_str_hash( str, strlen( str ) );
}


int cys_hash_reset (gString& s)
{
 return s.iValue = -1;
}


long cys_do_hash (gString& s)
{
 int iLength( (int)s.Length() );
 long x( (long)s.iValue );

 if ( x!=-1 ) return x;

 x = cys_str_hash( s.Str(), iLength );
 s.iValue = (int)x;
 return x;
}
////////////////////////////////////////////////////////////

