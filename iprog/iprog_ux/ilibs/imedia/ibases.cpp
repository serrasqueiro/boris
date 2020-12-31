// ibases.cpp

#include <string.h>
#include <errno.h>

#include "ibases.h"
#include "istring.h"


#ifdef DEBUG_IB
#define ib_print(args...) printf(args)		// Note ASCII >126d may appear!
#else
#define ib_print(args...) ;
#endif


// 'Recode' macros:

#define MASK(Length) ((unsigned) ~(~0 << (Length)))


/* Read next data byte and check its value, discard an illegal sequence.
   This macro is meant to be used only within the `while' loop in
   `transform_utf8_ucs[24]'.  */
#define GET_DATA_BYTE \
  imb_next_byte( subtask );						\
  if (character == EOF)							\
    {									\
      RETURN_IF_NOGO( ereINVALID_INPUT, subtask );		\
      break;								\
    }									\
  else if ((MASK (2) << 6 & character) != 1 << 7)			\
    {									\
      DBGPRINT("DBG: utf8 error [A]: char %dd: %d != %d\n",character,(MASK (2) << 6 & character),1 << 7);	\
      RETURN_IF_NOGO( ereINVALID_INPUT, subtask );		\
      continue;								\
    }									\
  else


#define GET_DATA_BYTE_AT(Position) \
	GET_DATA_BYTE	value |= (MASK (6) & character) << Position

#define imb_next_byte(subtask) DBGPRINT_MIN("DBG: %s: line %d: iter=%d, width=%d (%dd)\n",__FILE__,__LINE__,iter,width,bufIn[iter]);	\
				character = (int)bufIn[ iter++ ];	\
				if ( iter > width ) break;

#define put_ucs4(value,subtask) \
	bufOut[ outSize++ ] = value; if ( outSize>maxBufOut ) { RETURN_IF_NOGO( ereSYSTEM_ERROR, subtask ); }


// Maintain maximum of ERROR and current error:

#define RETURN_IF_NOGO(error,subtask) \
	DBGPRINT("DBG: %s: utf8_to_ucs4, line %d (iter=%d): error %d\n",__FILE__,__LINE__,iter,error); \
	curError = error; \
	if ( error>returnError ) { returnError = error; break; }


////////////////////////////////////////////////////////////
int imb_absolute (int value)
{
 return value>=0 ? value : -value;
}


int imb_strlen (const t_uchar* strIn)
{
 if ( strIn ) return strlen( (char*)strIn );
 return 0;
}


t_uchar* imb_utf8_str_to_ISO8859 (const t_uchar* bufIn, int subOne)
{
 eRecodeError returnError( ereINTERNAL_ERROR );
 t_uchar* ptrStr( imb_utf8_str_to_ucs2( bufIn, returnError ) );
 if ( returnError>ereNO_ERROR ) {
     delete[] ptrStr;
     return nil;
 }
 return ptrStr;
}


t_uchar* imb_utf8_str_to_ucs2 (const t_uchar* bufIn, eRecodeError& returnError)
{
 const t_uint32 eightBitMask( 0xFF );
 sUtfBox box;
 t_uchar* result( imb_utf8_str_to_ucs2_limit( bufIn, eightBitMask, box ) );

 // If output results in UCS4 bigger than 255, it will return ereUNTRANSLATABLE;
 // if eightBitMask is 0, only 0x7F is not allowed.

 returnError = box.ucs2Error;
 return result;
}


t_uchar* imb_utf8_str_to_ucs2_limit (const t_uchar* bufIn, t_uint32 maxMask, sUtfBox& box)
{
 const t_uint32 masked( ~maxMask );
 const t_uint32 allow32Mask( ~0xFF );
 int len( imb_strlen( bufIn ) );
 int iter( 0 ), outSize( 0 );
 t_unicode* bufOut( new t_unicode[ len*2+1 ] );
 t_uint32 iterChr;
 t_uchar uChr;
 t_uchar* ptrStr( nil );
 eRecodeError returnError( ereINTERNAL_ERROR );

 ib_print("len=%d, maxMask=%u - masked: 0x%02x, {%s}\n",
	  len,
	  maxMask, (unsigned)masked,
	  bufIn);

 box.ucs2Error = returnError;  // clean, no error
 box.lastUCS2 = 0;
 box.strLast = bufIn;
 if ( bufOut==nil ) return nil;  // No memory!

 returnError = imb_transform_utf8_to_ucs4( len, bufIn, len*2, bufOut, outSize );
 if ( returnError<=ereNO_ERROR ) {
     ptrStr = new t_uchar[ outSize+4 ];
     if ( ptrStr ) {
	 for (ptrStr[0]=0; iter<outSize; iter++) {
	     iterChr = (t_uint32)bufOut[ iter ];
	     uChr = ptrStr[ iter ] = (t_uchar)iterChr;
	     ib_print("iter=%d, %c (0x%02X=%03dd), &=%d\n", iter, uChr, bufOut[ iter ], (unsigned)bufOut[ iter ], bufOut[ iter ] & masked);
	     if ( maxMask ) {
		 if ( iterChr & masked ) {
		     returnError = ereUNTRANSLATABLE;
		     box.lastUCS2 = (t_unicode)iterChr;
		     box.strLast = bufIn + iter;
		     break;
		 }
		 else {
		     if ( iterChr & allow32Mask ) {
			 returnError = ereUNTRANSLATABLE;
			 box.lastUCS2 = (t_unicode)iterChr;
			 //printf("%s! 0x%4X (%ud)\n{%s}\n\n", bufIn, iterChr, iterChr, bufIn+iter);
			 box.strLast = bufIn + iter;
			 iter = outSize+2;
			 strncpy((char*)ptrStr, (char*)bufIn, iter);  // fallback!
			 break;
		     }
		 }
	     }
	     else {
		 if ( uChr==0x7F ) {
		     returnError = ereAMBIGUOUS_OUTPUT;
		     break;
		 }
	     }
	 }
	 ptrStr[ iter ] = 0;
     }
     else {
	 returnError = ereINTERNAL_ERROR;
     }
 }
 ib_print("imb_utf8_str_to_ucs2(...,%d): %s outSize=%d, {%s}\n",
	  returnError,
	  returnError==ereUNTRANSLATABLE ? "(NOT UCS2, try UCS4)" : "",
	  outSize,
	  ptrStr);
 delete[] bufOut;
 box.ucs2Error = returnError;
 return ptrStr;
}


t_ucs4* imb_utf8_str_to_ucs4 (const t_uchar* bufIn, eRecodeError& returnError)
{
 const t_uint32 sixteenBitMask( 0xFFFF );
 sUtfBox box;
 t_ucs4* result( imb_utf8_str_to_ucs4_limit( bufIn, sixteenBitMask, box ) );

 returnError = box.ucs2Error;
 return result;
}


t_ucs4* imb_utf8_str_to_ucs4_limit (const t_uchar* bufIn, t_uint32 maxMask, sUtfBox& box)
{
 const t_uint32 masked( ~maxMask );
 int len( imb_strlen( bufIn ) );
 int iter( 0 ), outSize( 0 );
 t_unicode* bufOut( new t_unicode[ len*2+1 ] );
 t_ucs4 uChr;
 t_ucs4* ptrStr( nil );
 eRecodeError returnError( ereINTERNAL_ERROR );

 box.ucs2Error = returnError;
 if ( bufOut==nil ) return nil;  // No memory!

 returnError = imb_transform_utf8_to_ucs4( len, bufIn, len*2, bufOut, outSize );
 if ( returnError<=ereNO_ERROR ) {
     ptrStr = new t_ucs4[ outSize+2 ];
     if ( ptrStr ) {
	 for (iter=0; iter<outSize+2; iter++) {
	     ptrStr[ 0 ] = 0;
	 }
	 for (iter=0; iter<outSize; iter++) {
	     uChr = ptrStr[ iter ] = (t_ucs4)bufOut[ iter ];
	     ib_print("iter=%d, %c (0x%02X=%03dd), &=%d\n", iter, uChr, bufOut[ iter ], (unsigned)bufOut[ iter ], bufOut[ iter ] & masked);
	     if ( maxMask ) {
		 if ( (t_uint32)bufOut[ iter ] & masked ) {
		     ptrStr[ iter ] = 0;
		     returnError = ereUNTRANSLATABLE;
		     break;
		 }
	     }
	     else {
		 if ( uChr==0x7F ) {
		     ptrStr[ iter ] = 0;
		     returnError = ereAMBIGUOUS_OUTPUT;
		     break;
		 }
	     }
	 }
     }
     else {
	 returnError = ereINTERNAL_ERROR;
     }
 }
 delete[] bufOut;
 box.ucs2Error = returnError;
 return ptrStr;
}


eRecodeError imb_transform_utf8_to_ucs4 (int width,
					 const t_uchar* bufIn,
					 int maxBufOut,
					 t_unicode* bufOut,
					 int& outSize)
{
 int iter( 0 );
 int character;
 unsigned value( 0 );
 eRecodeError returnError( ereNO_ERROR );
 eRecodeError curError( ereNO_ERROR );

 // Inspired on 'recode' (3.6) <pinard@iro.umontreal.ca>

 ASSERTION(curError==ereNO_ERROR,"!");  // avoid warning
 outSize = 0;
 if ( maxBufOut<=0 ) return ereNO_ERROR;
 if ( bufIn==nil || bufOut==nil ) return ereINTERNAL_ERROR;

 for (character=(int)bufIn[ iter++ ]; iter<=width; ) {
     if ((character & MASK (4) << 4) == MASK (4) << 4)
	 if ((character & MASK (6) << 2) == MASK (6) << 2)
	     if ((character & MASK (7) << 1) == MASK (7) << 1) {
		 /* 7 bytes - more than 31 bits (that is, exactly 32 :-).  */
#if HANDLE_32_BITS
		 value = 0;
		 GET_DATA_BYTE_AT (30);
		 GET_DATA_BYTE_AT (24);
		 GET_DATA_BYTE_AT (18);
		 GET_DATA_BYTE_AT (12);
		 GET_DATA_BYTE_AT (6);
		 GET_DATA_BYTE_AT (0);
		 put_ucs4( value, subtask );
		 imb_next_byte( subtask );
#else
		 // Usually no 32-bits:
		 RETURN_IF_NOGO( ereINVALID_INPUT, subtask );
		 imb_next_byte( subtask );
#endif //~HANDLE_32_BITS
	     }
	     else {
		 /* 6 bytes - more than 26 bits, but not more than 31.  */
		 value = (MASK (1) & character) << 30;
		 GET_DATA_BYTE_AT (24);
		 GET_DATA_BYTE_AT (18);
		 GET_DATA_BYTE_AT (12);
		 GET_DATA_BYTE_AT (6);
		 GET_DATA_BYTE_AT (0);
		 put_ucs4( value, subtask );
		 imb_next_byte( subtask );
	     }
	 else if ((character & MASK (5) << 3) == MASK (5) << 3) {
	     /* 5 bytes - more than 21 bits, but not more than 26.  */
	     value = (MASK (2) & character) << 24;
	     GET_DATA_BYTE_AT (18);
	     GET_DATA_BYTE_AT (12);
	     GET_DATA_BYTE_AT (6);
	     GET_DATA_BYTE_AT (0);
	     put_ucs4( value, subtask );
	     imb_next_byte( subtask );
	 }
	 else {
	     /* 4 bytes - more than 16 bits, but not more than 21.  */
	     value = (MASK (3) & character) << 18;
	     GET_DATA_BYTE_AT (12);
	     GET_DATA_BYTE_AT (6);
	     GET_DATA_BYTE_AT (0);
	     put_ucs4( value, subtask );
	     imb_next_byte( subtask );
	 }
     else if ((character & MASK (2) << 6) == MASK (2) << 6)
	 if ((character & MASK (3) << 5) == MASK (3) << 5) {
	     /* 3 bytes - more than 11 bits, but not more than 16.  */
	     value = (MASK (4) & character) << 12;
	     GET_DATA_BYTE_AT (6);
	     GET_DATA_BYTE_AT (0);
	     put_ucs4( value, subtask );
	     imb_next_byte( subtask );
	 }
	 else {
	     /* 2 bytes - more than 7 bits, but not more than 11.  */
	     value = (MASK (5) & character) << 6;
	     GET_DATA_BYTE_AT (0);
	     put_ucs4( value, subtask );
	     imb_next_byte( subtask );
	 }
     else if ((character & 1 << 7) == 1 << 7) {
	 /* Valid only as a continuation byte.  */
	 RETURN_IF_NOGO( ereINVALID_INPUT, subtask );
	 imb_next_byte( subtask );
     }
     else {
	 /* 1 byte - not more than 7 bits (that is, ASCII).  */
	put_ucs4( MASK (8) & character, subtask );
	imb_next_byte( subtask );
     }

     DBGPRINT_MIN("DBG: utf8, loop: iter=%d, width=%d\n",iter,width);
 }

 DBGPRINT_MIN("DBG: Note: width=%d, value=%d, character=%dd\n",
	      width,
	      value,
	      character);
 DBGPRINT("DBG: returnError=%d, curError=%d, iter=%d, outSize=%d, maxBufOut=%d\n",
	  returnError, curError,
	  iter,
	  outSize,
	  maxBufOut);
 return returnError;
}

////////////////////////////////////////////////////////////
gString* imb_bin_to64 (int length,
		       t_uchar* binBuffer,
		       t_uint32& outLen)
{
 unsigned len( (unsigned)length );
 gString* ptrStr;

 outLen = 0;
 if ( length<0 ) return nil;

 if ( len<1 ) {
     ptrStr = new gString;
     return ptrStr;
 }

 // See also:
 //	char* gString64::EncodeBinTo64 (unsigned srcLen, t_uchar* binBuffer, t_uint32& outLen)

 t_uchar* newStr( new t_uchar[ len*2+4 ] );  // Note: (len+1)*4/3 would be enough
 t_uchar* src( binBuffer );
 t_uchar* destStr( newStr );

 if ( newStr==nil ) return nil;  // Memory: failed to allocate

 destStr[ 0 ] = 0;

 for ( ; len>=3; len-=3) {
     ibase_Base64Encode_3to4( src, destStr );
     src += 3;
     destStr += 4;
     outLen += 4;
 }

 memset( destStr, 0x0, 5 );
 outLen += (t_uint32)len;

 switch( len ) {
 case 2:
     ibase_Base64Encode_2to4( src, destStr );
     break;
 case 1:
     ibase_Base64Encode_1to4( src, destStr );
     break;
 case 0:
     break;
 default:
     ASSERTION_FALSE("uops");
 }

 ptrStr = new gString( newStr );
 ptrStr->iValue = outLen;
 delete[] newStr;

 return ptrStr;
}


gString* imb_auth_mime64 (const t_uchar* strUser,
			  const t_uchar* strPass)
{
 gString* ptrStr;
 int length1, length2, size;
 t_uint32 outLen;

 if ( strUser==nil || strPass==nil )
     return nil;

 length1 = strlen( (char*)strUser );
 length2 = strlen( (char*)strPass );

 size = length1+1 + length2+1;

 t_uchar* binBuffer( (t_uchar*)calloc( size, 1 ) );

 strcpy( (char*)(binBuffer+1), (char*)strUser );
 memcpy( binBuffer+1+length1+1, (char*)strPass, length2 );

 ptrStr = imb_bin_to64( size, binBuffer, outLen );
 free( binBuffer );

 return ptrStr;
}


int imb_dir_error (const char* strDir)
{
 // Returns 0 if directory exists, or otherwise the error-code returned by OS
 int error( -1 );
 if ( strDir==nil || strDir[ 0 ]==0 ) return -1;
 gFileStat aStat( (char*)strDir );
 if ( aStat.HasStat() ) {
    if ( aStat.IsDirectory() ) return 0;
 }
 error = errno;
 if ( error==0 ) return -1;
 return error;
}


gList* imb_check_any_of (const char* strDir, gList& anyOf)
{
 // Returns as soon as one of files in anyOf is found
 gList* result( nil );
 for (gElem* pElem=anyOf.StartPtr(); pElem; pElem=pElem->next) {
    result = imb_check_file( strDir, pElem->Str() );
    if ( result ) return result;
 }
 return nil;
}


gList* imb_check_file (const char* strDir, const char* aFilename)
{
 gList* result;
 gString sName( (char*)strDir );

 if ( aFilename==nil || aFilename[ 0 ]==0 ) return nil;
 if ( sName.Length() ) {
    if ( sName[ sName.Length() ]!='\\' && sName[ sName.Length() ]!='/' ) {
	sName.Add( gSLASHCHR );
    }
 }
 sName.Add( (char*)aFilename );
 gFileText check( sName.Str() );
 if ( check.IsOpened()==false ) return nil;
 result = new gList;
 ASSERTION(result,"result");
 result->Add( sName );
 result->Add( (char*)aFilename );
 return result;
}

////////////////////////////////////////////////////////////

