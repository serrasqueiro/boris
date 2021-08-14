// iNtpStats.cpp

#define thisProgramVersion "Version 1.1"
#define thisProgramCopyright "Prized Season & Sons"
#define thisProgramYear 2014
#define thisProgramCopy "This is free software (GPL)\n\
There is no warranty, not even for MERCHANTABILITY or\n\
FITNESS FOR A PARTICULAR PURPOSE."


#include <stdio.h>
#include <errno.h>

#include "iCntpDates.h"

////////////////////////////////////////////////////////////
int print_help (const char* progStr)
{
 const char
     *msgHelp = "%s - %s\n\n\
Usage:\n\
        %s [loop] [OPTION]\n\
\n\
Options are:\n\
   -h           This help (or --help / --version)\n\
   -v           Verbose (use twice, more verbose)\n\
   -l X         Use log-file X (or --log-file)\n\
   -0           Resolution 0\n\
   -1           Resolution 1\n\
\n\
Shows ntp peerstats (usually at /var/log/ntp/) in a simpler way;\n\
if loop arg is provided, it shows loopstats instead.\n\
\n\
Default resolution shows offsets in seconds, e.g. 0.010514378,\n\
resolution 0 shows only seconds, resolution 1 shows up to 100ms.\n\
\n";

 printf(msgHelp,
	progStr,
	thisProgramVersion,
	progStr);
 return 0;
}


int print_version (char* progStr)
{
 const char
     *msgVersion = "%s - %s\n\
\n\
Written by Henrique Moreira.\n\
\n\
Build \
011\
\n\
\n\
Copyright (C) %u %s.\n\
%s\n";

 printf(msgVersion,
	progStr,
	thisProgramVersion,
	thisProgramYear,thisProgramCopyright,
	thisProgramCopy);
 return 0;
}

////////////////////////////////////////////////////////////
int clean_up_input (char* str)
{
 static char chr;
 int bogus( 0 );

 if ( str ) {
     for ( ; (chr = *str)!=0; str++) {
	 if ( chr=='\t' ) {
	     chr = ' ';
	 }
	 else {
	     if ( chr < ' ' || chr >= 127 ) {
		 if ( chr!='\r' && chr!='\n' ) bogus++;
		 (*str) = '\0';
	     }
	 }
     }
 }
 return bogus;
}


int parse_peerstats (FILE* fIn, int mask, FILE* fOut, FILE* fRepErr)
{
 char buf[ 512 ];
 char* line;
 int error( 0 );
 int nrLine( 0 );
 int which( mask );
 e_LocalhostParse localhostOpt( e_substitute_by_localhost );

 for ( ; fgets( buf, sizeof( buf )-1, fIn ); ) {
     nrLine++;
     clean_up_input( buf );

     if ( buf[ 0 ] < ' ' ) continue;
     if ( buf[ 0 ]=='#' ) {
	 fprintf(stdout, "%s\n", buf);
	 continue;
     }

     line = new_parsed_peerstats( buf, which, localhostOpt, error );
     if ( line ) {
         fprintf(fOut, "%s\n", line);
	 delete[] line;
     }
     else {
	 if ( error ) {
	     if ( fRepErr ) {
		 fprintf(fRepErr, "Uops line %d: %s\n", nrLine, buf);
	     }
	 }
     }
 }
 return 0;
}


int parse_loopstats (FILE* fIn, int mask, FILE* fOut, FILE* fRepErr)
{
 char buf[ 512 ];
 int error( 0 );
 int nrLine( 0 );
 char* line;
 e_LocalhostParse localhostOpt( e_substitute_by_localhost );

 for ( ; fgets( buf, sizeof( buf )-1, fIn ); ) {
     nrLine++;
     clean_up_input( buf );

     if ( buf[ 0 ] < ' ' ) continue;
     if ( buf[ 0 ]=='#' ) {
	 fprintf(stdout, "%s\n", buf);
	 continue;
     }

     line = new_parsed_loopstats( buf, 0, localhostOpt, error );
     if ( line ) {
         fprintf(fOut, "%s\n", line);
	 delete[] line;
     }
     else {
	 if ( error ) {
	     if ( fRepErr ) {
		 fprintf(fRepErr, "Uops line %d: %s\n", nrLine, buf);
	     }
	 }
     }
 }
 return 0;
}


int go (int argc, char** argv)
{
 int error;
 int thisError( -1 );
 FILE* fIn( stdin );
 FILE* fOut( stdout );
 FILE* fRepErr( stderr );
 gArg arg( argv );
 gString sLog;
 bool isVerbose( false );
 bool isVeryVerbose( false );
 bool resolution0( false );
 bool resolution1( false );

 arg.AddParams( 1, "-vv|--verbose -h|--help --version\
 -0|--resolution-0\
 -1|--resolution-1\
 -l:%s|--log-file:%s\
" );

 // Parse command line
 error = arg.FlushParams();
 arg.FindOptionOccurr( "verbose", isVerbose, isVeryVerbose );

 if ( arg.FindOption('h') || error!=0 ) {
     print_help( arg.Program() );
     return 0;
 }
 if ( arg.FindOption("version") ) {
     print_version( arg.Program() );
     return 0;
 }

 gString sFirst( arg.Str( 1 ) );

 resolution0 = arg.FindOption('0')>0;

 resolution1 = arg.FindOption('1')>0;

 if ( arg.FindOption("log-file",sLog) ) {
    fOut = fopen( sLog.Str(), "a+" );
    if ( fOut==nil ) {
       fprintf(stderr, "Unable to use log: %s: %s\n", strerror( errno ), sLog.Str());
       return 1;
    }
 }
 // end parse of args

 int mask( resolution0==true );

 if ( mask==0 ) {
    mask += ((resolution1==true) * 2);
 }

 if ( error ) {
    printf("DBG: error %d, mask: %d\n", error, mask);
    return 0;
 }

 if ( sFirst.Find( "loop", true )==1 ) {
    thisError = parse_loopstats( fIn, mask, fOut, fRepErr );
 }
 if ( thisError==-1 ) {
    DBGPRINT("peerstats, mask: %d\n", mask);
    thisError = parse_peerstats( fIn, mask, fOut, fRepErr );
 }

 if ( fOut && fOut!=stdout ) fclose( fOut );

 error = thisError;
 DBGPRINT("DBG: go returns %d\n", error);
 return error;
}

////////////////////////////////////////////////////////////
int main (int argc, char** argv)
{
 int error;

 gINIT;

 error = go( argc, argv );

 gEND;
 return error;
}
////////////////////////////////////////////////////////////

