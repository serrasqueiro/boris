// auxsfv.cpp
//
//	Auxiliar SFV functions (SFV - Simple File Verification)
//
//	CRC32 at icrc32.cpp


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "auxsfv.h"

#include "lib_ilambda.h"
#include "lib_imaudio.h"

////////////////////////////////////////////////////////////
iEntry* sfv_entries_from_file (const char* strFile)
{
 gFileFetch input( (char*)strFile );
 iEntry* result( new iEntry );
 gElem* pElem;
 iEntry* newObj;

 DBGPRINT("DBG: sfv_entries_from_file(%s): %d\n", strFile, input.lastOpError);
 ASSERTION(result,"result");

 result->SetComment( ima_dirname( strFile ) );
 if ( result->sComment.IsEmpty() ) {
     result->sComment.Set( "." );
 }

 for (pElem=input.aL.StartPtr(); pElem; pElem=pElem->next) {
     newObj = new_sfv_entry( *(gString*)(pElem->me) );

     if ( newObj ) {
	 DBGPRINT("DBG: 0x%08X %s {%s}\n",
		  newObj->iValue,
		  newObj->Str(),
		  newObj->sComment.Str());
	 result->AppendObject( newObj );
     }
 }

 return result;
}


int sfv_check (iEntry& entries, const char* strPath, FILE* fOut, FILE* fReport, int& missing)
{
 int error( 0 );
 int countErrors( 0 );
 gElem* pElem;
 iEntry* pLine;
 char* strName;
 t_uint32 there, here;

 missing = 0;

 for (pElem=entries.StartPtr(); pElem; pElem=pElem->next) {
     bool matched( false );

     pLine = (iEntry*)pElem->me;
     strName = pLine->Str();
     there = pLine->iValue;

     if ( strName[ 0 ] ) {
	 gString sFile( (char*)strPath );

	 if ( sFile.Length() ) {
	     if ( sFile[ sFile.Length() ]!=gSLASHCHR && sFile[ sFile.Length() ]!=altSLASHCHR ) {
		 sFile.Add( gSLASHCHR );
	     }
	 }
	 sFile.Add( strName );

	 fCRC32 calc;
	 error = calc.ComputeCRC32( sFile.Str(), here )!=0;

	 // Check whether calc CRC32 matches reported SFV CRC32
	 DBGPRINT("0x%08X vs 0x%08X %s\n",
		  there,
		  here,
		  sFile.Str());
	 matched = here==there;

	 if ( error ) {
	     missing++;
	     if ( fReport ) {
		 fprintf(fReport, "Cannot open: %s\n", sFile.Str());
	     }
	     if ( fOut ) {
		 fprintf(fOut, "0x%08X ?\n", there);
	     }
	 }
	 else {
	     if ( fOut ) {
		 if ( matched ) {
		     fprintf(fOut, "OK: 0x%08X %s\n", here, sFile.Str());
		 }
		 else {
		     fprintf(fOut, "ERROR: 0x%08X vs 0x%08X %s\n", here, there, sFile.Str());
		 }
	     }
	 }

	 countErrors += (matched==false);
     }//end if it's not just a comment, contains a file name string
 }

 return countErrors;
}


iEntry* new_sfv_entry (gString& sLine)
{
 int code( 2 );
 iEntry* ptrSFV( nil );
 gString sCopy( sLine );
 gString sHex;
 t_uint32 vRes( 0 );

 sCopy.Trim();

 if ( sCopy.Length() ) {
     ptrSFV = new iEntry;
     ASSERTION(ptrSFV,"ptrSFV");

     if ( sCopy[ 1 ]==';' ) {
	 sCopy.Delete( 1, 1 );
	 sCopy.Trim();
	 ptrSFV->SetComment( sCopy.Str() );
     }
     else {
	 for (int iter=(int)sCopy.Length(); iter>1; iter--) {
	     if ( sCopy[ iter ]<=' ' ) {
		 sHex.CopyFromTo( sCopy, iter+1 );
		 code = gStorageControl::Self().ConvertHexToUInt32( sHex.Str(), e_DigConvAny, vRes )!=0;
		 if ( code==0 ) {
		     sCopy.Delete( iter );
		     sCopy.TrimRight();
		 }
		 break;
	     }
	 }
	 if ( code==0 ) {
	     ptrSFV->iValue = (int)vRes;
	 }
	 ptrSFV->Add( sCopy );
     }
 }

 return ptrSFV;
}


int sfv_dump (iEntry& entries, const char* strPath, bool newLineCR, FILE* fOut, FILE* fReport)
{
 int error( 0 );
 t_uint32 there;
 char* strName;
 gElem* pElem;
 const char* carriageReturn( newLineCR ? "\r" : "" );

 for (pElem=entries.StartPtr(); pElem; pElem=pElem->next) {
     strName = pElem->Str();

     fCRC32 calc;
     error = calc.ComputeCRC32( strName, there )!=0;
     if ( error ) {
	 if ( fReport ) {
	     fprintf(fReport, "Could not open: %s\n", strName);
	 }
     }
     else {
	 if ( fOut ) {
	     fprintf(fOut, "%s %08X%s\n", strName, there, carriageReturn);
	 }
     }
 }

 return error;
}

////////////////////////////////////////////////////////////

