// dba.cpp
//
// a very simple Unix-style database


#include <string.h>
#include <errno.h>

#include "dba.h"

#include "lib_ilog.h"

#define CRC32_DEF_INIT	0xffffffff


////////////////////////////////////////////////////////////
void init_dbiHeader (dbiHeader& h, t_uint32 nrTables)
{
 memset( &h, 0x0, sizeof(dbiHeader) );
 strcpy( h.head, "dbi" );
 strcpy( h.versionMajor, "10" );
 h.versionMinor[ 0 ] = '1';
 h.versionMinor[ 1 ] = '2';
 h.versionEnhPkg[ 0 ] = h.versionEnhPkg[ 1 ] = ' ';
 strcpy( h.magicNumber, "12345687" );
#if 0
 h.nrTables = DB_NUM_4_BYTE( 0x12345678 );
#else
 h.nrTables = DB_NUM_4_BYTE( nrTables );
#endif
 memset( h.pad, ' ', sizeof(h.pad) );
 h.num01FE = DB_NUM_2_BYTE( 0x01FE );
}

////////////////////////////////////////////////////////////
DBA::DBA ()
    : uCRC( 0 ),
      dbiHandle( -1 )
{
 sShown.Set( "mydb" );
}


DBA::~DBA ()
{
 CloseDB();
}


int DBA::ValidError (int error, bool forceError)
{
 const int errMax( 127 );

 if ( error<=-1 ) {
     sErrorMsg.Set( "(unknown)" );
     return 1;
 }
 if ( error>=errMax ) {
     error = errMax;
 }
 if ( forceError ) {
     if ( error==0 ) {
	 error = 1;
	 sErrorMsg.iValue = error;
	 sErrorMsg.Set( "(error)" );
	 return error;
     }
 }

 sErrorMsg.iValue = error;
 sErrorMsg.Set( strerror( error ) );
 return error;
}


gList* DBA::Config (const char* strParam)
{
 gElem* pElem( configs.StartPtr() );
 gList* pList;
 gString* pString;

 if ( strParam ) {
     for ( ; pElem; pElem=pElem->next) {
	 pList = (gList*)pElem->me;
	 pString = (gString*)pList->StartPtr()->me;
	 if ( pString->Match( (char*)strParam ) ) {
	     return pList;
	 }
     }
 }
 else {
     return &configs;
 }
 return nil;
}


gFileStat* DBA::DoStat (const char* fileOrDir)
{
 if ( fileOrDir ) {
     aStat.Update( (char*)fileOrDir );
 }
 else {
     return nil;
 }
 return &aStat;
}


bool DBA::ValidateTableName (gString& s)
{
 gString result;
 return ValidateTableName( s, result );
}


bool DBA::ValidateTableName (gString& s, gString& result)
{
 int iter( 1 );

 result = s;
 result.UpString();

 for (char chr; (chr = result[ iter ])!=0; iter++) {
     if ( chr>='A' && chr<='Z' )
	 continue;
     if ( iter>1 ) {
	 if ( (chr>='0' && chr<='9') || (chr=='_' || chr=='-') ) {
	     continue;
	 }
     }
     result.Delete( iter );
     return false;
 }
 return result.Length()>0;
}


gList* DBA::AddConfigLine (const char* strLine)
{
 gParam aLine( (char*)strLine, "=", gParam::e_StopSplitOnFirst );
 return AddConfig( aLine.Str( 1 ), aLine.Str( 2 ) );
}


gList* DBA::AddConfig (const char* strLeft, const char* strRight)
{
 gList* ptrFind;
 gList* confItem( nil );
 gString sLeft( (char*)strLeft );
 gString sRight( strRight ? (char*)strRight : (char*)"" );

 sLeft.Trim();
 sRight.Trim();

 DBGPRINT("DBG: AddConfig(%s,%s)\n", strLeft, strRight);

 if ( sLeft.Length() ) {
     ptrFind = Config( sLeft.Str() );

     if ( ptrFind==nil ) {
	 confItem = new gList;
	 ASSERTION(confItem,"confItem");
	 confItem->Add( sLeft );
	 confItem->Add( sRight );
	 confItem->EndPtr()->me->iValue = atoi( sRight.Str() );
	 configs.AppendObject( confItem );
     }
     else {
	 *(gString*)(ptrFind->StartPtr()->next->me) = sRight;
	 confItem = ptrFind;
     }
 }
 return confItem;
}


int DBA::RemoveConfig (const char* strLeft)
{
 int iter( 0 ), found( 0 );
 gList* ptrConf;
 gElem* pElem( configs.StartPtr() );

 for ( ; pElem; pElem=pElem->next) {
     iter++;
     ptrConf = (gList*)pElem->me;
     if ( ((gString*)ptrConf->StartPtr()->me)->Match( (char*)strLeft ) ) {
	 found = iter;
	 break;
     }
 }
 if ( found ) {
     configs.Delete( found, found );
 }
 return found;
}


int DBA::InitDB (const char* strDbFileName)
{
 gString aDbFileName( (char*)strDbFileName );
 return InitDB( aDbFileName );
}


int DBA::InitDB (gString& aDbFileName)
{
 int error( 0 );
 const char* strPath( dba_simpler_path( aDbFileName, 0 ) );

 CommonInit();
 printf("InitDB(%s): error=%d\n",
	strPath, error);

  return error;
}


int DBA::InitDB (gList& params)
{
 CommonInit();
 return 0;
}


void DBA::CommonInit ()
{
 const char* strLine;

 strLine = getenv( "DBA_DEFAULT_PATH" );
 if ( strLine==nil ) {
      strLine = getenv( "DB_PATH" );
 }

 AddConfig( "DB_PATH", strLine );
}


bool DBA::BuildName (gString& sPath, const char* strName, const char* strExt, gString& result)
{
 const char* strPath( dba_simpler_path( sPath, 0 ) );

 result.Set( (char*)strPath );
 if ( result[ result.Length() ]!=gSLASHCHR ) {
     result.Add( gSLASHCHR );
 }
 result.Add( (char*)strName );
 result.Add( (char*)strExt );
 return true;
}


int DBA::CreateDB (gString& sPathName, bool forceRewrite)
{
 gList params;
 return CreateDB( sPathName, params, forceRewrite );
}


int DBA::CreateDB (gString& sPathName, gList& params, bool forceRewrite)
{
 int error;
 int handle;
 dbiHeader h;
 const char* strPath( dba_simpler_path( sPathName, 0 ) );
 gFileStat* ptrStat;

 gio_mkdir( strPath, 0, 0 );
 ptrStat = DoStat( strPath );
 if ( ptrStat->IsDirectory()==false ) {
     return ValidError( errno );
 }

 BuildName( sPathName, sShown.Str(), ".dbi", sControlFile );
 DBGPRINT("DBG: dbi name: %s!\n", sControlFile.Str());

 ptrStat = DoStat( sControlFile.Str() );
 if ( ptrStat->HasStat() ) {
     DBGPRINT("DBG: dbi exists, forceRewrite: %c\n", ISyORn( forceRewrite ));
     if ( forceRewrite==false ) {
	 return ValidError( -1 );
     }
 }

 init_dbiHeader( h, 0 );

 handle = ilf_create( sControlFile.Str(), 0, error );
 DBGPRINT("DBG: ilf_create, handle=%d, error=%d\n", handle, error);
 if ( handle!=-1 ) {
     write( handle, &h, sizeof(h) );
     close( handle );

     error = OpenDB( sPathName );
 }
 else {
     return ValidError( errno );
 }
 return error;
}


int DBA::OpenDB (gString& sPathName)
{
 int error( 0 );
 int iter( 0 );
 dbiHeader h;

 BuildName( sPathName, sShown.Str(), ".dbi", sControlFile );
 DBGPRINT("DBG: dbi name: %s!\n", sControlFile.Str());

 dbiHandle = ilf_openrw( sControlFile.Str(), 0, error );
 if ( dbiHandle==-1 ) {
     return ValidError( errno );
 }

 error = read( dbiHandle, &h, sizeof(h) )!=sizeof(h);
 h.nrTables = DB_NUM_4_BYTE( h.nrTables );
 h.num01FE = DB_NUM_2_BYTE( h.num01FE );
 h.magicNumber[ 9 ] = 0;

 if ( error ) {
     return ValidError( -1 );  // premature EOF
 }
 error = strncmp( h.head, "dbi", 3 )!=0;

 for ( ; iter<(int)h.nrTables; ) {
     char chrTeste( 'A'+iter );

     if ( chrTeste>'Z' ) break;  // just debug...
     iter++;

     gString sNew;
     sNew.Add( chrTeste );

     tables.Add( sNew );
 }

 DBGPRINT("DBG: dbiHandle=%d, error=%d, %s, num01FE=0x%04X, nrTables=%u (0x%08X)\n",
	  dbiHandle,
	  error,
	  h.magicNumber,
	  h.num01FE,
	  h.nrTables, h.nrTables);
 return error;
}


int DBA::CreateTable (const char* strTableName)
{
 gString sTableName( (char*)strTableName );
 return CreateTable( sTableName );
}


int DBA::CreateTable (gString& sTableName)
{
 gString sName;

 if ( ValidateTableName( sTableName, sName )==false ) {
     DBGPRINT("DBG: sTableName(%s) invalid: %s\n", sTableName.Str(), sName.Str());
     sErrorMsg.Set( "invalid name" );
     return 2;
 }

 if ( tables.Match( sName.Str() ) ) {
     sErrorMsg = sName;
     return 8;  // duplicate table?
 }
 tables.Add( sName );

 return thisUpdateDBI();
}


bool DBA::CloseDB ()
{
 if ( dbiHandle!=-1 ) {
     close( dbiHandle );
     return true;
 }
 return false;
}


int DBA::thisUpdateDBI ()
{
 int error;
 dbiHeader h;

 if ( dbiHandle==-1 ) {
     sErrorMsg.Set( "db not opened" );
     return 2;
 }

 // update dbi
 gio_file_seek( dbiHandle, 0, SEEK_SET );

 init_dbiHeader( h, tables.N() );
 error = write( dbiHandle, &h, sizeof(h) )!=sizeof(h);

 return error;
}

////////////////////////////////////////////////////////////
// Aux functions
////////////////////////////////////////////////////////////
int best_tail_num (gString& s, int base)
{
 int val( 0 );

 for (int idx=(int)s.Length(), m=1, v; idx>1; idx--) {
     if ( m > 100000000 ) {
	 return -1;
     }
     v = (int)s[ idx ] - '0';
     if ( v>=0 && v<=9 ) {
	 val += (v * m);
     }
     else {
	 break;
     }
     m *= base;
 }
 return val;
}


int best_nibble_match (int bits)
{
 int one( (bits % 4)!=0 );
 return bits / 4 + one;
}

////////////////////////////////////////////////////////////
char* dba_simpler_path (gString& aStr, int mask)
{
 static char aPath[ 1024 ];
 int iter( 0 );
 char slash( '\0' );
 char chr;
 char* str( aStr.Str() );

 memset(aPath, 0x0, 1024);
 if ( aStr.Length() >= sizeof(aPath)-4 ) return aPath;

 for ( ; (chr = *str)!=0; str++) {
     char toAdd( '\0' );

     if ( chr=='\\' || chr=='/' ) {
	 if ( (mask & 1)==0 ) {
	     chr = gSLASHCHR;
	     slash = chr;
	 }
	 else {
	     slash = '/';
	 }
     }
     else {
	 toAdd = chr;
     }

     if ( toAdd ) {
	 if ( slash ) {
	     aPath[ iter++ ] = slash;
	 }
	 aPath[ iter++ ] = toAdd;
	 slash = '\0';
     }
 }
 if ( slash ) {
     aPath[ iter++ ] = slash;
 }
 aPath[ iter ] = 0;  // un-needed, but safer
 return aPath;
}


gList* dba_fields_from_string (const char* strFields)
{
 gList* raw( split_chars( strFields, "{}" ) );
 gList* result( new gList );
 gElem* pElem;
 gString* pStr;
 int brackets;

 // "{jday:uint32; minute-tracker:uint16} sent:uint64; recv:uint64; sent-tcp:uint64; recv-uint64:uint64"

 ASSERTION(raw,"raw");
 ASSERTION(result,"result");

 for (int check=1; check>=0; check--) {
     for (pElem=raw->StartPtr(); pElem; pElem=pElem->next) {
	 brackets = pElem->iValue;

	 if ( brackets==check ) {
	     pStr = (gString*)pElem->me;
	     pStr->Trim();
	     if ( pStr->Length() ) {
		 int err( 0 );
		 int type( brackets );
		 gList* semi;
		 semi = split_chars( pStr->Str(), ";" );  // separate firstly by semicolons

		 for (gElem* ptr=semi->StartPtr(); ptr; ptr=ptr->next) {
		     gParam colonPair( ptr->Str(), ":", gParam::e_StopSplitOnFirst );
		     if ( colonPair.N()==0 ) {
			 continue;
		     }
		     if ( colonPair.N()==1 ) {
			 colonPair.Add( "string" );
		     }

		     gString sFieldName( colonPair.Str( 1 ) );
		     gString sType( colonPair.Str( 2 ) );

		     sFieldName.Trim();
		     sType.Trim();

		     gParam spaced( sType.Str(), " ", gParam::e_StopSplitOnFirst );
		     gString sFieldType( spaced.Str( 1 ) );
		     gString sEtc( spaced.Str( 2 ) );

		     sFieldType.Trim();
		     sEtc.Trim();

		     err += (sFieldName.ConvertAnyChrTo( ":;, +/|\\?!@#$%='\"~^", '.' ) > 0);
		     err += (sFieldType.ConvertChrTo( ':', '.' ) > 0);
		     if ( err ) {
			 DBGPRINT("DBG: error (%d) in either sField Name|Type {%s} or {%s}\n",
				  err,
				  sFieldName.Str(),
				  sFieldType.Str());
			 type = -1;
		     }

		     gList* pair( new gList );
		     pair->Add( sFieldName );
		     pair->Add( sFieldType );

		     gString sNum( 64, '\0' );
		     int val( best_tail_num( sFieldType, 10 ) );

		     DBGPRINT("DBG: name: %s, field type '%s', sEtc (%s)\n",
			      sFieldName.Str(),
			      sFieldType.Str(),
			      sEtc.Str());
		     if ( sEtc.IsEmpty() ) {
			 if ( sFieldType.Find( "string" ) > 0 ) {
			     sEtc.Add( DBA_FIELD_STR_SURFACE );
			 }
			 else {
			     if ( sFieldType.Find( "int" ) > 0 ) {
				 sprintf(sNum.Str(), "h/%d", best_nibble_match( val ));
				 sEtc.AddString( sNum );
			     }
			 }
		     }
		     pair->Add( sEtc );
		     pair->EndPtr()->me->iValue = val;

		     result->AppendObject( pair );
		     result->EndPtr()->iValue = type;
		 }
		 delete semi;
	     }
	 }
     }
 }

 delete raw;
 return result;
}


gList* split_chars (const char* str, const char* split)
{
 gList* result( new gList );
 char chr;
 char s1, s2( '\0' );
 int loc( 0 );
 int iter( 0 );

 ASSERTION(result,"mem");
 if ( split==nil || split[ 0 ]==0 ) {
     if ( str ) {
	 result->Add( (char*)str );
     }
     return result;
 }

 s1 = split[ loc++ ];
 s2 = split[ loc++ ];
 ASSERTION(s1,"s1");

 gString sCopy( str ? (char*)str : (char*)"\0" );
 char* input( sCopy.Str() );
 int level( 0 );

 if ( s2==0 ) {
     for ( ; (chr = input[ iter ])!=0; ) {
	 if ( chr==s1 ) {
	     input[ iter ] = 0;
	     result->Add( input );
	     input += (iter+1);
	     iter = 0;
	 }
	 else {
	     iter++;
	 }
     }
 }
 else {
     for ( ; (chr = input[ iter ])!=0; ) {
	 if ( chr==s1 ) {
	     level++;
	     input[ iter ] = 0;
	     result->Add( input );
	     input += (iter+1);
	     iter = 0;
	     for ( ; (chr = input[ iter ])!=0; ) {
		 if ( chr==s1 ) {
		     level++;
		 }
		 if ( chr==s2 ) {
		     level--;
		     if ( level==0 ) {
			 input[ iter ] = 0;
			 result->Add( input );
			 result->EndPtr()->iValue = 1;
			 input += (iter+1);
			 iter = 0;
			 break;
		     }
		 }
		 iter++;
	     }
	 }
	 iter++;
     }// outer FOR
 }

 if ( iter ) {
     result->Add( input );
 }

 return result;
}

////////////////////////////////////////////////////////////

