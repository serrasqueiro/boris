// ibase.cpp

#include <string.h>

#include "ibase.h"
////////////////////////////////////////////////////////////
// Static members
t_uchar gStringBase::base64[ 128 ]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
t_int16 gStringBase::convFromBase64[ 256 ];
const t_int16 gStringBase::sizeOfFastIdx=256;  // 256 ASCIIs: for indexing

////////////////////////////////////////////////////////////
gStringBase::gStringBase ()
    : convertCode( 0 ),
      padChr( (t_uchar)'=' )
{
 thisInitBase64( sizeOfFastIdx );
}

gStringBase::gStringBase (char* aStr)
    : gString( aStr ),
      convertCode( 0 ),
      padChr( (t_uchar)'=' )
{
 thisInitBase64( sizeOfFastIdx );
}

gStringBase::~gStringBase ()
{
}

bool gStringBase::UseNow (eBase newBase)
{
 switch ( newBase ) {
 case e_Base64:
     BS_ASSERTION(sizeof(base64)>=64,"64");
     base64[ 63 ] = '/';
     break;
 case e_Base65:
     base64[ 63 ] = '%';
     break;
 default:
     return false;
 }
 return true;
}

int gStringBase::thisInitBase64 (t_int16 size16bit)
{
 t_int16 iter( 0 );
 t_uint16 value;

 BS_ASSERTION(size16bit>0,"size16bit>0");
 DBGPRINT_BAS("DBG_BAS: thisInitBase64(%u), re-hash? %c (%d)\n",
	      size16bit,
	      ISyORn( convFromBase64[ size16bit-1 ]!=-1 ),
	      convFromBase64[ size16bit ]);
 size16bit--;
 BS_ASSERTION(size16bit==255,"size16bit==255");
 if ( convFromBase64[ size16bit ]==-1 ) return 0;

 for ( ; iter<=size16bit; iter++) {
     convFromBase64[ iter ] = -1;
 }
 for (iter=0; (value = (t_int16)base64[ iter ])!=0; iter++) {
     convFromBase64[ value ] = iter;
 }
 DBGPRINT_BAS("DBG_BAS: thisInitBase64: slash=%d, perc.=%d base64='%s'\n",
	      convFromBase64[ '/' ],
	      convFromBase64[ '%' ],
	      base64);
 return 1;
}

////////////////////////////////////////////////////////////
gString64::gString64 ()
{
}

gString64::gString64 (char* aStr)
    : gStringBase( aStr )
{
}


gString64::gString64 (gString& s)
    : gStringBase( s.Str() )
{
}


gString64::gString64 (eBase base, char* aStr)
    : gStringBase( aStr ? aStr : (char*)"\0" )
{
 ASSERTION(base==e_Base64,"base==e_Base64");
}


gString64::~gString64 ()
{
}


bool gString64::UseNow (eBase newBase)
{
 if ( gStringBase::UseNow( newBase )==false ) return false;
 // Re-init fast index array:
 ASSERTION(sizeOfFastIdx>=256,"sizeOfFastIdx>=256");
 convFromBase64[ sizeOfFastIdx-1 ] = -2;  // Force re-hash
 thisInitBase64( sizeOfFastIdx );
 DBGPRINT_MIN("DBG: UseNow newBase=%d, last pin (%d): %d\n",
	      newBase,
	      sizeOfFastIdx,
	      convFromBase64[ sizeOfFastIdx-1 ]);
 return true;
}


char* gString64::Encode64 ()
{
 unsigned len( Length() );
 t_uchar* newStr( new t_uchar[ len*2+4 ] );  // Note: (len+1)*4/3 would be enough
 t_uchar* src( (t_uchar*)Str() );
 t_uchar* destStr( newStr );

 if ( newStr==nil ) {
     convertCode = -1;
     return nil;
 }

 convertCode = 0;  // Note encode cannot fail.

#ifdef DEBUG
 unsigned dbgUsedLen( 0 ), dbgMaxLen( len );
 memset( newStr, (int)'~', len*2+4 );
#else
 newStr[ 0 ] = 0;
#endif

 for ( ; len>=3; len-=3) {
     myBase64Encode_3to4( src, destStr );
     src += 3;
     destStr += 4;
     DBGPRINT("DBG: len: %u, dbgUsedLen: %u,%u,%u,%u (max=%u, theoricalMax=%u, initial len=%u)\n",
	      len,
	      ++dbgUsedLen,++dbgUsedLen,++dbgUsedLen,++dbgUsedLen,dbgMaxLen*2,(dbgMaxLen+1)*4/3,dbgMaxLen);
 }

 switch( len ) {
 case 2:
     myBase64Encode_2to4( src, destStr );
     break;
 case 1:
     myBase64Encode_1to4( src, destStr );
     break;
 case 0:
     destStr[ 0 ] = 0;
     break;
 default:
     ASSERTION_FALSE("uops");
 }

 destStr[ 4 ] = 0;  // ...or before xxx_Nto4 memset( destStr, 0x0, 5 ) => to force a null-terminated base64-string!

 sResult.Set( newStr );
 delete[] newStr;

 return sResult.Str();
}


t_uchar* gString64::Decode64 ()
{
 t_uint32 srcLen( (t_uint32)Length() );
 t_uint32 dstLen( 0 );
 t_uint32 outLen( 0 );
 t_uchar* dest( new t_uchar[ srcLen+8 ] );  // More than necessary (only around 3/4ths are required)

 if ( dest==nil ) {
     convertCode = -1;
     return nil;
 }

 dest[ 0 ] = 0;
 convertCode = (t_int16)myBase64Decode( (t_uchar*)Str(), srcLen, dest, dstLen, outLen );
 DBGPRINT("DBG: myBase64Decode returned %d (dstLen=%u, outLen=%u)\n",
	  convertCode,
	  dstLen,
	  outLen);

 if ( convertCode )
     sResult.SetEmpty();
 else
     sResult.Set( dest );
 delete[] dest;
 dest = (t_uchar*)sResult.Str();

 DBGPRINT("DBG: decoded '%s' to %u chars (remainer: %u)\n",
	  Str(),
	  (unsigned)outLen,
	  (unsigned)dstLen);
 return dest;
}


char* gString64::EncodeBinTo64 (unsigned srcLen, t_uchar* binBuffer, t_uint32& outLen)
{
 sResult.SetEmpty();
 if ( srcLen<1 )
     return sResult.Str();

 unsigned len( srcLen );
 t_uchar* newStr( new t_uchar[ len*2+4 ] );  // Note: (len+1)*4/3 would be enough
 t_uchar* src( binBuffer );
 t_uchar* destStr( newStr );

 outLen = 0;

 if ( newStr==nil ) {
     convertCode = -1;
     return nil;
 }

 convertCode = 0;  // Note encode cannot fail.

#ifdef DEBUG
 memset( destStr, (int)'~', len*2+4 );
 destStr[ len*2 + 4 - 1 ] = 0;
#endif
 destStr[ 0 ] = 0;

 for ( ; len>=3; len-=3) {
     myBase64Encode_3to4( src, destStr );
     src += 3;
     destStr += 4;
     outLen += 4;
 }

 memset( destStr, 0x0, 5 );
#ifdef DEBUG
 memset( destStr, (int)'^', 4 );
#endif

 outLen += (t_uint32)len;

 switch( len ) {
 case 2:
     myBase64Encode_2to4( src, destStr );
     break;
 case 1:
     myBase64Encode_1to4( src, destStr );
     break;
 case 0:
     break;
 default:
     ASSERTION_FALSE("uops");
 }

 DBGPRINT_MIN("DBG: Encode64(): outLen=%u, {%s}\n",
	      outLen,
	      newStr);

 sResult.Set( newStr );
 delete[] newStr;

 return sResult.Str();
}


t_uchar* gString64::Decode64ToBin (unsigned bufLen, t_uchar* binBuffer, t_uint32& outLen)
{
 t_uint32 srcLen( (t_uint32)Length() );
 t_uint32 dstLen( 0 );
 t_uchar* dest( binBuffer );
 t_uchar* uSrc( (t_uchar*)Str() );

 convertCode = 0;
 outLen = 0;
 sResult.SetEmpty();
 if ( bufLen<1 ) {
     return (t_uchar*)sResult.Str();
 }

 if ( dest==nil ) {
     convertCode = -1;  // Something to convert, but no buffer given!
     return nil;
 }

#ifdef DEBUG_BS
 memset( dest, (int)'~', bufLen ); dest[ bufLen-1 ] = 0;  // memset to zero is not necessary
#endif

 dest[ 0 ] = 0;
 DBGPRINT_MIN("DBG: calling myBase64Decode('%s',%d,dest,&%d,&%d)\n",
	      uSrc,
	      (int)srcLen,
	      (int)dstLen,
	      (int)outLen);
 convertCode = (t_int16)myBase64Decode( uSrc, srcLen, dest, dstLen, outLen );
 DBGPRINT("DBG: called myBase64Decode('%s',%d,dest,&%d,&%d): convertCode=%d\n",
	  uSrc,
	  (int)srcLen,
	  (int)dstLen,
	  (int)outLen,
	  (int)convertCode);
 DBGPRINT("DBG: called myBase64Decode('%s',%d,dest,&%d,&%d): convertCode=%d\n",
	  uSrc,
	  (int)srcLen,
	  (int)dstLen,
	  (int)outLen,
	  (int)convertCode);
 if ( dstLen>=bufLen || outLen>=bufLen ) {
     convertCode = -2;  // Buffer overun probably ocurred
     BS_ASSERTION_FALSE("overun");
     return nil;
 }

 dest[ dstLen ] = 0;

 DBGPRINT("DBG: bufLen=%u, len=%d, exp_len=%d, dest {%s}\n",
	  bufLen,
	  strlen( (char*)dest ),
	  bufLen * 3 / 4,
	  dest);

 return dest;
}

////////////////////////////////////////////////////////////
void gString64::myBase64Encode_3to4 (const t_uchar* src, t_uchar* dest)
{
 BS_ASSERTION(src && dest,"src && dest");
 ibase_Base64Encode_3to4( src, dest );
}


void gString64::myBase64Encode_2to4 (const t_uchar* src, t_uchar* dest)
{
 BS_ASSERTION(src && dest,"src && dest");
 ibase_Base64Encode_2to4( src, dest );
}


void gString64::myBase64Encode_1to4 (const t_uchar* src, t_uchar* dest)
{
 BS_ASSERTION(src && dest,"src && dest");
 ibase_Base64Encode_1to4( src, dest );
}


gStringBase::eB64status gString64::myBase64Decode (t_uchar* src,
						   t_uint32 srcLen,
						   t_uchar* dest,
						   t_uint32& dstLen,
						   t_uint32& outLen)
{
 eB64status rv( decode64_internal_error );

 BS_ASSERTION(src,"src");
 // Cut trailing '='
 if ( srcLen>0 && (srcLen & 3)==0 ) {
     DBGPRINT_MIN("DBG: myBase64Decode original srcLen=%u src[LAST]: %c\n",
		  srcLen,
		  src[ srcLen-1 ]);
     if ( src[ srcLen-1 ]==padChr ) {
	 if ( src[ srcLen-2 ]==padChr ) {
	     srcLen -= 2;
	 }
	 else {
	     srcLen -= 1;
	 }
     }
 }
 outLen = srcLen * 3 / 4;
 DBGPRINT_MIN("DBG: myBase64Decode, outLen=%u srcLen=%u\n",outLen,srcLen);
 dest[ outLen ] = 0;

 for (dstLen=0; srcLen>=4; srcLen-=4) {
     DBGPRINT_MIN("DBG: myBase64Decode(...'%s',...), srcLen=%u\n",
		  src,
		  srcLen);
     rv = myBase64Decode_4to3( src, dest );
     if ( rv!=decode64_success ) return decode64_failure;
     src += 4;
     dest += 3;
     dstLen += 3;
 }

 dstLen += srcLen;

 switch ( srcLen ) {
 case 3:
     rv = myBase64Decode_3to2( src, dest );
     break;
 case 2:
     DBGPRINT_BAS("BS_DBG: convFromBase64[%u]=%d, convFromBase64[%u]=%d\n",
		  src[0], convFromBase64[ src[0] ],
		  src[1], convFromBase64[ src[1] ]);
     rv = myBase64Decode_2to1( src, dest );
     break;
 case 1:
     rv = decode64_failure;
     break;
 case 0:
     rv = decode64_success;
     break;
 default:
     return decode64_internal_error;
 }
 DBGPRINT_BAS("BS_DBG: rv=%d, srcLen=%u\t%s\n",
	      rv,
	      (unsigned)srcLen,
	      src);
 return rv;
}


gStringBase::eB64status gString64::myBase64Decode_4to3 (const t_uchar* src, t_uchar* dest)
{
 t_uint32 b32 = 0;
 t_int32 bits;
 int i;

 for (i=0; i<4; i++) {
   bits = convFromBase64[ src[i] ];
   DBGPRINT_MIN("DBG: 4to3: i=%d, bits=%d, src[i]='%c' (0x%02X)\n",
		i,
		(int)bits,
		src[ i ],
		(unsigned)src[ i ]);
   if ( bits < 0 ) return decode64_failure;
   b32 <<= 6;
   b32 |= bits;
 }

 dest[0] = (t_uchar)((b32 >> 16) & 0xFF);
 dest[1] = (t_uchar)((b32 >>  8) & 0xFF);
 dest[2] = (t_uchar)((b32      ) & 0xFF);
 return decode64_success;
}


gStringBase::eB64status gString64::myBase64Decode_3to2 (const t_uchar* src, t_uchar* dest)
{
 t_uint32 b32 = (t_uint32)0;
 t_int32 bits;
 t_uint32 ubits;

 bits = convFromBase64[ src[0] ];
 if ( bits < 0 ) return decode64_failure;

 b32 = (t_uint32)bits;
 b32 <<= 6;

 bits = convFromBase64[ src[1] ];
 if ( bits < 0 ) return decode64_failure;
 b32 |= (t_uint32)bits;
 b32 <<= 4;

 bits = convFromBase64[ src[2] ];
 if ( bits < 0 ) return decode64_failure;

 ubits = (t_uint32)bits;
 b32 |= (ubits >> 2);

 dest[0] = (t_uchar)((b32 >> 8) & 0xFF);
 dest[1] = (t_uchar)((b32     ) & 0xFF);

 return decode64_success;
}


gStringBase::eB64status gString64::myBase64Decode_2to1 (const t_uchar* src, t_uchar* dest)
{
 t_uint32 b32;
 t_uint32 ubits;
 t_int32 bits;

 bits = convFromBase64[ src[0] ];
 if ( bits < 0 ) return decode64_failure;

 ubits = (t_uint32)bits;
 b32 = (ubits << 2);

 bits = convFromBase64[ src[1] ];
 if ( bits < 0 ) {
   return decode64_failure;
 }

 ubits = (t_uint32)bits;
 b32 |= (ubits >> 4);

 dest[0] = (t_uchar)b32;

 return decode64_success;
}

////////////////////////////////////////////////////////////
const t_uchar* ibase_base64 ()
{
 static const t_uchar base64[ 128 ]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
 return base64;
}


const t_uchar ibase_base64_item (t_uint8 item)
{
#ifdef IBASE_OPTIMIZE_SPEED
 static const t_uchar base64[ 128 ]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
 return base64[ item ];
#else
 return ibase_base64()[ item % 64 ];
#endif
}


void ibase_Base64Encode_3to4 (const t_uchar* src, t_uchar* dest)
{
 t_uint32 b32 = 0;
 int i, j = 18;

 for (i=0; i<3; i++) {
     b32 <<= 8;
     b32 |= (t_uint32)src[i];
 }
 for (i=0; i<4; i++) {
     dest[i] = ibase_base64_item(  (t_uint32)((b32>>j) & 0x3F) );
     j -= 6;
 }
}


void ibase_Base64Encode_2to4 (const t_uchar* src, t_uchar* dest)
{
 dest[0] = ibase_base64_item( (t_uint32)((src[0]>>2) & 0x3F) );
 dest[1] = ibase_base64_item( (t_uint32)(((src[0] & 0x03) << 4) | ((src[1] >> 4) & 0x0F)) );
 dest[2] = ibase_base64_item( (t_uint32)((src[1] & 0x0F) << 2) );
 dest[3] = '=';
}


void ibase_Base64Encode_1to4 (const t_uchar* src, t_uchar* dest)
{
 dest[0] = ibase_base64_item( (t_uint32)((src[0]>>2) & 0x3F) );
 dest[1] = ibase_base64_item( (t_uint32)((src[0] & 0x03) << 4) );
 dest[2] = dest[3] = '=';
}


bool ibase_Prime (unsigned value)
{
 unsigned toDiv( 2 ), half( value/2 );

 if ( value==0 ) return false;
 if ( value<=7 ) return true;

 for ( ; toDiv<=half; toDiv++) {
     if ( (value % toDiv)==0 ) return false;
 }
 return true;
}


unsigned ibase_NextPrime (unsigned value)
{
 for (value++; ibase_Prime( value )==false; ) {
     value++;
 }
 return value;
}

////////////////////////////////////////////////////////////

