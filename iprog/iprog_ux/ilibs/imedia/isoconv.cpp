// isoconv.cpp

#include <stdlib.h>
#include <string.h>

#include "lib_imedia.h"
#include "isoconv.h"

////////////////////////////////////////////////////////////
// sIsoUni
// ---------------------------------------------------------
sIsoUni::sIsoUni (t_unitable aIsoTable)
    : isoTable( aIsoTable ),
      uniCode( nil ),
      inUse( nil ),
      buffers( nil ),
      hashUcs8Custom( customUcs8[ 0 ] )
{
 hashUcs8Custom[ 0 ] = -128;
 for (int idx=0; idx<256; idx++) {
     hashUcs16Eq[ idx ] = nil;
     hashUcs16Strs[ idx ] = nil;
 }
 strUcs16Buf[ 0 ] = strUcs16Buf[ 1 ] = strUcs16Buf[ 2 ] = strUcs16Buf[ 3 ] = 0;
}


bool sIsoUni::Release ()
{
 int idx( 0 );

 isoTable = 0;
 delete uniCode; uniCode = nil;
 inUse = nil;

 for ( ; idx<256; idx++) {
     if ( hashUcs16Eq[ idx ] ) {
	 free( hashUcs16Eq[ idx ] );
	 hashUcs16Eq[ idx ] = nil;
     }
     if ( hashUcs16Strs[ idx ] ) {
	 free( hashUcs16Strs[ idx ] );
	 hashUcs16Strs[ idx ] = nil;
     }
 }

 delete buffers; buffers = nil;

 return isoForCountry.N()==0;
}


int sIsoUni::RefactorUcs16Eq ()
{
 const char* strUCS;
 int len;

 DBGPRINT("DBG: sIsoUni::RefactorUcs16Eq(), idx0 addr: %p\n", hashUcs16Strs[ 0 ]);
 for (int idx=0; idx<256; idx++) {
     strUCS = (char*)hashUcs16Eq[ idx ];
     ASSERTION(strUCS,"strUCS");
     len = strlen( strUCS );
     if ( hashUcs16Strs[ idx ] ) {
	 free( hashUcs16Strs[ idx ] );
     }
     hashUcs16Strs[ idx ] = (t_uchar*)calloc( len+2, sizeof(t_uchar) );
     strcpy((char*)hashUcs16Strs[ idx ], strUCS);
 }
 return 0;
}

////////////////////////////////////////////////////////////
// Aux functions
////////////////////////////////////////////////////////////
int iso_release_data (sIsoUni& data)
{
 int error( data.Release()==false );
 return error;
}


int iso_allowed_adjust (gUniCode& uniCode,
			int optMask,
			const char* newAdjust)
{
 char* strPrintable( uniCode.hash256Basic[ gUniCode::e_Basic_Printable ] );

 ASSERTION(newAdjust==nil,"newAdjust");  // Not yet supported

 if ( optMask==0 ) {
     strPrintable[ IMB_ISO8859_1_CHR_ACUTE_ACCENT ] = '\'';
 }
 return 0;
}

////////////////////////////////////////////////////////////
int imb_iso_init_internal (const char* strPath, sIsoUni& data)
{
 int error;

 ASSERTION(strPath==nil,"imb_iso_init: strPath is ignored");
 ASSERTION(data.isoTable>0,"isoTable");
 delete data.uniCode;
 data.uniCode = new gUniCode;
 ASSERTION(data.uniCode,"data.uniCode");
 data.inUse = data.uniCode;

 error = data.inUse->Build();

 iso_allowed_adjust( *data.inUse, 0, nil );

 // Init hash data members

 // <START OF custom UCS8>

 memcpy( data.hashUcs8Printable, data.inUse->hash256Basic[ gUniCode::e_Basic_Printable ], sizeof(data.hashUcs8Printable) );
 memcpy( data.customUcs8[ 0 ], data.hashUcs8Printable, sizeof(data.hashUcs8Printable) );

 memset( data.customUcs8[ 1 ], -1, 256 );
 memset( data.customUcs8[ 2 ], -1, 256 );
 memset( data.customUcs8[ 3 ], -1, 256 );

 // The custom UCS8 allows better control;
 // for instance, ISO-8859-1 should not allow broken characters
 // like those from 128 to 0xA0 (160d); euro-sign (128d), for instance,
 // might be allowed in some places: but this customized plan forbids it.
 // by marking them as '0' in the customized hash,
 // applications can signal these characters in a better way.

 memset( data.hashUcs8Custom+128, 0, 160-128+1 );

 // <END OF custom UCS8>

 printu("iso_init %lu returns, error=%d\n",
	(unsigned long)data.isoTable,
	error);
 return error;
}


int imb_iso_release ()
{
 int result;
 ASSERTION(ptrUniData,"ptrUniData");
 result = iso_release_data( *ptrUniData );
 delete ptrUniData;
 ptrUniData = nil;
 return result;
}


int imb_iso_show (FILE* fOut, sIsoUni& data)
{
 const char* strTableName;
 const t_uchar* pangram;
 const char* strSamples[]={
     "Henrique: ça c´est Parfait '0123456789'!",
     "Voix ambiguë d'un cur qui, au zéphyr, préfère les jattes de kiwis.",
     "En god stil må først og fremst være klar. Den må være passende.",  // 100% Norwegian
     "50" IMB_ISO8859_1_STR_EURO_SIGN " to reach Hoffmanstraße.",
     nil,
     nil
 };

 gUniCode* ptrCode( data.inUse );
 t_uchar strBuf[ 300 ];
 t_uint16 isoVariant( 0 );
 int iter( 0 ), idx, outIdx;

 if ( fOut==nil ) return -1;
 ASSERTION(ptrCode,"uniData.inUse");

 strTableName = ptrCode->tableName.Str();
 data.IsIsoLatin( isoVariant );

 fprintf(fOut,"<UniCode>\n");
 fprintf(fOut,"\tIsoUni [isoTable: %lu] %sISO-8859-%02u%s\n",
	 (unsigned long)data.isoTable,
	 data.Is8859() ? "\0" : "Non-",
	 isoVariant,
	 data.IsIsoLatin() ? " (Latin1)" : "\0");

 if ( strTableName )
     fprintf(fOut,"\ttableName: {%s}\n",strTableName);
 else
     fprintf(fOut,"\ttableName: nil\n");

 fprintf(fOut,"\
	nUniEntries: %u\n\
",
	 ptrCode->uniListed.N());

 //ptrCode->uniListed.Show();

 fprintf(fOut,"</UniCode>\n");

 for ( ; (pangram = (t_uchar*)strSamples[ iter ])!=nil; iter++) {
     const t_uchar* strShown( imb_iso_str( pangram, 0, sizeof(strBuf), strBuf ) );
     fprintf(fOut,"\n%s\n%s\n",
	     pangram,
	     strShown);

     fprintf(fOut,"%s\n",
	     imb_iso_str_custom( pangram, 0, data.hashUcs8Custom, sizeof(strBuf), strBuf ));
     fprintf(fOut,"%s\n",
	     strcmp( (char*)strShown, (char*)pangram )==0 ? "==> OK" : "\0");
 }

 for (iter=0; iter<=(int)gUniCode::e_Basic_Hash; iter++) {
     gUCharBuffer ucBuf;
     const char* strHash;
     const bool isCustom( iter>=(int)gUniCode::e_Basic_Hash );

     if ( isCustom ) {
	 strHash = ptrUniData->hashUcs8Custom;
     }
     else {
	 strHash = ptrCode->hash256Basic[ iter ];
     }

     for (idx=0, outIdx=0; idx<256; idx++) {
	 char hashChr( strHash[ idx ] );
	 ucBuf.uBuf[ outIdx++ ] = (hashChr==-1) ? IMB_DUMP_NEUTRAL : (hashChr ? hashChr : '^');
	 switch ( idx ) {
	 case ' ':
	 case 64:
	 case 96:
	 case 'z':
	 case 128:
	 case 0xA0:
	 case 191:
	     ucBuf.uBuf[ outIdx++ ] = '\n';
	     break;
	 default:
	     break;
	 }
     }
     printf("hash256Basic[%d%s]:\n%s\n\n",
	    iter, isCustom ? " (hashUcs8Custom)" : "\0",
	    ucBuf.uBuf);
 }
 return 0;
}


int imb_iso_init (const char* strPath, sIsoUni* newUniData)
{
 imd_print("imb_iso_init(%s,data=sIsoUni)\n", strPath ? strPath : "nil");
 ASSERTION(newUniData,"newUniData");
 ptrUniData = newUniData;
 return
     imb_iso_init_internal( strPath, *ptrUniData );
}


t_uchar* imb_iso_str (const t_uchar* strIn, int mask, int maxBufSize, t_uchar* strBuf)
{
 const char* basicHash;
 gUniCode* usedCode( ptrUniData->inUse );

 IMB_SANE( "imb_iso_str" );
 basicHash = usedCode->hash256Basic[ gUniCode::e_Basic_Printable ];
 return imb_iso_str_custom( strIn, mask, basicHash, maxBufSize, strBuf );
}


t_uchar* imb_iso_str_custom (const t_uchar* strIn, int mask, const char* strHash, int maxBufSize, t_uchar* strBuf)
{
 return imb_iso_bin_custom( strIn, -1, mask|1, strHash, maxBufSize, strBuf );
}


t_uchar* imb_iso_bin (const t_uchar* strIn, int size, int mask, int maxBufSize, t_uchar* strBuf)
{
 const char* basicHash;
 gUniCode* usedCode( ptrUniData->inUse );

 IMB_SANE( "imb_iso_bin" );
 basicHash = usedCode->hash256Basic[ gUniCode::e_Basic_Printable ];
 return imb_iso_bin_custom( strIn, size, mask, basicHash, maxBufSize, strBuf );
}


t_uchar* imb_iso_bin_custom (const t_uchar* strIn, int size, int mask, const char* strHash, int maxBufSize, t_uchar* strBuf)
{
 const t_uchar substChr( '?' );
 const bool breakOnNul( mask & 1 );
 int inIndex( 0 );
 int outIndex( 0 );
 t_uchar uChr;
 const char* basicHash( strHash );

 ASSERTION(strBuf,"strBuf");

 if ( maxBufSize<=0 ) {
     strBuf[ 0 ] = 0;
 }
 else {
     memset( strBuf, 0x0, maxBufSize );
 }

 if ( breakOnNul ) {
     size = MAX_INT_VALUE;
 }

 if ( strIn ) {
     for ( ; inIndex<size; ) {
	 uChr = strIn[ inIndex++ ];

	 if ( maxBufSize>0 ) {
	     if ( outIndex+1>=maxBufSize ) return strBuf;
	     if ( breakOnNul==true && uChr==0 ) break;
	 }

	 if ( basicHash[ uChr ]==-1 ) uChr = substChr;

	 strBuf[ outIndex++ ] = uChr;
	 strBuf[ outIndex ] = 0;
     }
 }
 return strBuf;
}

////////////////////////////////////////////////////////////
void imb_init_words (t_uint16 which)
{
 char code;
 char newCode;
 char lowOrUp;
 const char* strFixed( "\t\n _" );  // these are mapped equally

 which %= (t_uint16)gUniCode::e_Basic_Hash;  // avoid vector overun

 for (t_uint16 uIdx=0; uIdx<256; uIdx++) {
     code = (char)uIdx;
     lowOrUp = ptrUniData->inUse->hash256Basic[ gUniCode::e_Basic_Alpha ][ uIdx ];
     if ( (code>='a' && code<='z') || (code>='A' && code<='Z') ) {
	 newCode = code;
     }
     else {
	 newCode = ptrUniData->hashUcs8Custom[ uIdx ];
	 if ( newCode==-1 ) {
	     newCode = '?';
	 }
	 else {
	     switch ( lowOrUp ) {
	     case 'u':  // upper-case
	     case 'l':  // lower-case
		 newCode = code;  // the index of the hash is, itself, the special accented letter ASCII code!
		 break;
	     default:
		 break;
	     }
	 }
     }
     ptrUniData->inUse->hash256User[ which ][ uIdx ] = newCode;
 }

 for (int idx=0; (code = strFixed[ idx ])!=0; idx++) {
     ASSERTION(code>=0,"code>=0");
     ptrUniData->inUse->hash256User[ which ][ (int)(t_uchar)code ] = code;
 }

 // Some specialities from Windows-1252 ASCII
 ptrUniData->inUse->hash256User[ which ][ IMB_ISO8859_1_CLOSE_QUOTE ] = (t_uint8)'\'';

#if 0
 gList upList, loList;
 gElem* ptr;
 for (ptr=upList.StartPtr(); ptr; ptr=ptr->next) {
     int decimal( (int)(t_uchar)(ptr->me->iValue) );
     printf("UP {%c}\t%dd\t0x%04X\n", (char)ptr->me->iValue, decimal, decimal);
 }
 for (ptr=loList.StartPtr(); ptr; ptr=ptr->next) {
     int decimal( (int)(t_uchar)(ptr->me->iValue) );
     printf("LO {%c}\t%dd\t0x%04X\n", (char)ptr->me->iValue, decimal, decimal);
 }
#endif
}


char* imb_nice_words (t_uint16 which, char* strResult)
{
 t_uint16 uIdx;
 char code;

 which %= (t_uint16)gUniCode::e_Basic_Hash;  // avoid vector overun

 for (char chr; (chr = *strResult)!=0; strResult++) {
     uIdx = (t_uchar)chr;
     code = ptrUniData->inUse->hash256User[ which ][ uIdx ];
     *strResult = code;
  }
  return strResult;
}

////////////////////////////////////////////////////////////

