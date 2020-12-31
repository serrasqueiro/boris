// icontrol.cpp

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>  // va_start...

#include "ifile.h"
#include "icontrol.h"

////////////////////////////////////////////////////////////
// Static members
// ---------------------------------------------------------
int gControl::nErrors[ LOG_LOGMAX ];
gList gControl::lLog[ LOG_LOGMAX ];
int gControl::dbgLevelDefault=LOG_INFO;
char gControl::logBuf[4096];
int gStorageControl::rStaticStgObjs=0;
int gStorageControl::rStaticInitNrObjs=0;
gStorageControl::eDescriptionStoreType gStorageControl::descStoreType=e_ASCII_and_HTML;
gStorageControl gStorageControl::myself;

////////////////////////////////////////////////////////////
gControl::gControl (eStorage aKind)
    : gStorage( aKind, e_StgNoStore ),
      lastOpError( 0 ),
      dbgLevel( dbgLevelDefault )
{
 SetError( 0 );
}


t_uint16 gControl::GetRandom (t_uint16 maxRange)
{
 return (t_uint16)(rand())%maxRange;
}


void gControl::Reset ()
{
 lastOpError = 0;
}


void gControl::ResetLog ()
{
 for (int errKind=LOG_ERROR; errKind<LOG_LOGMAX; errKind++) {
     nErrors[ errKind ] = 0;
     lLog[ errKind ].Delete();
 }
}


int gControl::SetError (int opError)
{
 memset( sStrError, 0x0, sizeof(sStrError) );
 lastOpError = opError;
 if ( opError==0 ) return 0;
 if ( opError==-1 ) {
     strcpy( sStrError, "Internal Error" );
     return -1;
 }
 const char* str( strerror( opError ) );
 // E.g. "Connection refused" (111 = ECONNREFUSED), or "No such file or directory" (2 = ENOENT)
 if ( str ) {
     strncpy( sStrError, str, sizeof(sStrError)-1 );
 }
 else {
     sprintf( sStrError, "%d", opError );
 }
 return opError;
}


int gControl::Log (FILE* dbgFile, int level, const char* formatStr, ...)
{
 // Return 0 if no log applied, or -1 if dbgFile stream is nil
 static char kindErr[24], kindStr[128];

 if ( level<=LOG_NONE ) return 0;
 if ( level>=dbgLevel ) return 1;
 ASSERTION(formatStr!=nil,"formatStr!=nil");
 memset( logBuf, 0x0, sizeof(logBuf) );

 // Or: va_start( ap, formatStr ); vfprintf( dbgFile, formatStr, ap ); va_end( ap );
 va_list ap;
 va_start( ap, formatStr );
 vsnprintf( logBuf, sizeof(logBuf)-1/*force null ended string*/, formatStr, ap );
 va_end( ap );

 switch ( level ) {
 case LOG_ERROR:
     strcpy( kindErr, "Error" );
     break;
 case LOG_WARN:
 case LOG_NOTICE:
     strcpy( kindErr, "Warning" );
     break;
 default:
     level = LOG_LOGMAX-1;
     kindErr[0] = 0;
     break;
 }
 memset(kindStr, 0x0, sizeof(kindStr));
 if ( kindErr[0]!=0 ) {
     snprintf( kindStr, sizeof(kindStr)-1, "%s: ", kindErr );
 }
 nErrors[ level ]++;
 if ( dbgFile==nil ) {
     lLog[ level ].Add( logBuf );
     DBGPRINT_LOG("DBG: LOG(%d) %s%s\n",level,kindStr,logBuf);
     return 1;
 }
 fprintf( dbgFile, "%s%s", kindStr, logBuf );
 return 1;
}


int gControl::ClearLogMem (int level)
{
 // Return 0 if o.k.
 if ( level <= LOG_NONE ) return -1;
 if ( level >= LOG_LOGMAX ) return 1;
 lLog[ level ].Delete();
 return 0;
}


int gControl::ClearLogMemAll ()
{
 int level;
 for (level=LOG_NONE+1; ; level++)
     if ( ClearLogMem( level )!=0 ) return 0;
 return -1;
}


int gControl::ClearAllLogs ()
{
 ClearLogMemAll();
 return 0;
}


gStorage* gControl::NewObject ()
{
 ASSERTION_FALSE("gControl::NewObject");
 return nil;
}


void gControl::Show (bool doShowAll)
{
 int errKind( LOG_ERROR );  // 0

 if ( doShowAll ) {
     gStorage::Show( true );
 }
 iprint("lastOpError: %d\ndbgLevel: %d\nsName: %s\n",
	lastOpError,
	dbgLevel,
	sName.Str());

 if ( doShowAll ) {
     for ( ; errKind<LOG_LOGMAX; errKind++) {
	 iprint("nErrors LOG_%d: %d, %u\n",
		errKind,
		nErrors[ errKind ],
		lLog[ errKind ].N());
     }
 }
}

////////////////////////////////////////////////////////////
gStorageControl::gStorageControl ()
{
}


gStorageControl::~gStorageControl ()
{
 ASSERTION(rStaticInitNrObjs==-1,"gEND not called");
}


int gStorageControl::NumObjs ()
{
 return GetNumObjects()-rStaticStgObjs;
}


bool gStorageControl::Init ()
{
 if ( rStaticInitNrObjs )
     return false;

 // Just call singletons:
 gFileControl::Self().CtrlGetPid();

 // Currently allocated i-objects:
 rStaticInitNrObjs = GetNumObjects();
 StaticAlloc( "iobjs", rStaticInitNrObjs );

 DBGPRINT("DBG: rStaticInitNrObjs=%d\n",rStaticInitNrObjs);
 return true;
}


bool gStorageControl::StartAll ()
{
#ifdef iDOS_SPEC
 WSADATA wsaData;
 int error = WSAStartup (MAKEWORD(2,2), &wsaData)!=NO_ERROR;
 if ( error!=0 ) return false;
#endif //iDOS_SPEC
 return Init();
}


bool gStorageControl::End ()
{
 int numObjs;

 gFileControl::Self().ReleaseAll();
 Reset();

 numObjs = NumObjs();
 DBGPRINT("DBG: NumObjs()=%d (%d, total: %d)\n",NumObjs(),rStaticStgObjs,GetNumObjects());
 rStaticInitNrObjs = -1;
 lastOpError = numObjs;
 return numObjs<=0;
}


void gStorageControl::Reset ()
{
 gControl::Reset();
 ResetLog();
 DeletePool();
 DeleteDescriptions();
}


bool gStorageControl::DeletePool ()
{
 bool hasContents( pool.N()>0 );
 DBGPRINT("DBG: Deleting the gStorageControl pool: %u\n",pool.N());
 pool.Delete();
 return hasContents;
}


void gStorageControl::StaticAlloc (const char* msgStr, int incrNrStaticStgObjs)
{
 ASSERTION(msgStr!=nil,"msgStr!=nil");
 DBGPRINT_MIN("DBG:StaticAlloc:%s:(cur=%d)+%d\n",msgStr,rStaticStgObjs,incrNrStaticStgObjs);
 rStaticStgObjs += incrNrStaticStgObjs;
}


bool gStorageControl::RegisterDescription (gStorage& object, const t_desc_char* description)
{
 gList* newDesc;
 t_int32 index;

 if ( descStoreType==e_none ) return false;
 newDesc = new gList;
 ASSERTION(newDesc, "newDesc");

 newDesc->Add( (char*)description );
 newDesc->SetLRef( &object );
 descriptions.AppendObject( newDesc );
 index = (t_int32)descriptions.N();
 newDesc->iValue = index;
 object.SetDescription( index, description );
 return true;
}


bool gStorageControl::RegisterDescriptionStr (gStorage& object, const char* description)
{
 bool isOk;
 isOk = RegisterDescription( object, (t_desc_char*)description );
 return isOk;
}


void gStorageControl::TidyDescriptions ()
{
 // Removes unused elements
 descriptions.Tidy( -1 );
}


void gStorageControl::Show (bool doShowAll)
{
 if ( doShowAll ) {
     gControl::Show( true );
 }
 iprint("pool: "); pool.Show( true );
 iprint("NumObjs(): %d\n",NumObjs());
}
////////////////////////////////////////////////////////////

