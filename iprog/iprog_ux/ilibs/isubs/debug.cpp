// debug.cpp, for libisubs

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib_isubs.h"


IMEDIA_DECLARE;

////////////////////////////////////////////////////////////
int usage_option (const char* strCommand)
{
 const char firstChr( strCommand ? strCommand[ 0 ] : '\0' );

 const char* helps[]={
     "debug LETTER\n\
\n\
a		Show lib versions\n\
b		Dump subs file\n\
c		Collect words from subs file\n\
d		Dump HTML keys\n\
e		Forward / back subtitles\n\
f		Forward / back subtitles from index N\n\
",
     "\
a ...\n\
",
     "\
b ...\n\
",
     "\
c ...\n\
",
     "\
d ...\n\
",
     "\
e ...\n\
",
     "\
f ...\n\
",
     nil,
     nil,
     nil
 };

 int iter( 0 );
 for ( ; helps[ iter ]; ) iter++; iter--;

 if ( firstChr<'a' || firstChr-'a'>=iter ) {
     printf("%s\n",helps[ 0 ]);
 }
 else {
     printf("debug %s\n",helps[ firstChr+1-'a' ]);
 }
 return 0;
}

////////////////////////////////////////////////////////////
int dump_html_keys (FILE* fOut, gList& pat, gList& lines)
{
 gElem* ptrElem( lines.StartPtr() );
 unsigned pos;
 gString sPattern( pat.Str( 1 ) );

 ASSERTION(fOut,"fOut");

 for ( ; ptrElem; ptrElem=ptrElem->next) {
     gString sLine( ptrElem->Str() );
     if ( sLine[ 1 ]=='<' ) {
	 pos = sLine.Find( sPattern );
	 if ( pos ) {
	     sLine.Delete( 1, pos-1 + sPattern.Length() );
	     pos = sLine.Find( '"' );
	     if ( pos ) {
		 sLine.Delete( pos );
	     }
	     fprintf(fOut,"%s\n",sLine.Str());
	 }
	 DBGPRINT_MIN("DBG: pos=%u {%s}\n",
		      pos,
		      sLine.Str());
     }
 }
 return 0;
}

////////////////////////////////////////////////////////////
int collect_line (gString& sLine, gList& words)
{
 static t_uint16 progress;
 static long repeats;
 static int idx;
 unsigned uLen( sLine.Length() );
 t_uchar lastChr( sLine[ uLen ] );

 if ( uLen==0 ) return -1;

 if ( uLen>1 ) {
     if ( lastChr=='.' ) {
	 for (idx=(int)uLen; idx>1 && (lastChr = sLine[ idx ])=='.'; idx--) {
	     sLine[ idx ] = 0;
	 }
     }
 }

 gString* pNew( new gString( sLine ) );
 ASSERTION(pNew,"pNew");

 progress++;
 if ( (progress % 1000)==0 ) {
     fprintf(stderr,"Progress: %d, repeated words: %ld\n",
	     progress,
	     repeats);
 }

 if ( words.InsertOrderedUnique( pNew )==-1 ) {
     repeats++;
     words.CurrentPtr()->me->iValue++;
     delete pNew;
 }
 else {
     words.CurrentPtr()->me->iValue = 1;
 }
 return 0;
}


int collect_para (int level, gString& sLine, int rangeMin, int rangeMax, int& quoted, gList& words)
{
 gString s;
 int idx( 1 ), maxIdx( -1 );
 unsigned pos( 0 );
 bool isEndKey( false );

 level++;
 ASSERTION(level<1000,"level<1000");

 if ( rangeMin>rangeMax ) return 0;

 s.CopyFromTo( sLine, rangeMin, rangeMax );

 DBGPRINT("Level: %d, Q? %c %d %d	[%s] {%s}\n",
	  level,
	  ISyORn( quoted!=0 ),
	  rangeMin, rangeMax,
	  s.Str(),
	  sLine.Str());

 for (maxIdx=strlen( s.Str() ); idx<=maxIdx; idx++) {
     if ( s[ idx ]=='<' ) {
	 gString sRem;

	 s[ idx ] = '@';
	 sRem.CopyFromTo( s, idx );
	 pos = sRem.Find( '>' );
	 isEndKey = sRem[ 2 ]=='/';

	 if ( pos ) {
	     gString sEnd;
	     sEnd.CopyFromTo( sRem, pos+1, sRem.Length() );

	     if ( idx>1 ) {
		 collect_para( level, s, 1, idx-1, quoted, words );
	     }

	     sRem.Str()[ pos ] = 0;
	     quoted = (isEndKey==false);
	     return collect_para( level, sEnd, 1, sEnd.Length(), quoted, words );
	 }
     }
 }

 if ( quoted ) {
     fprintf(stderr,"Suppressed: %s [from %d to %d]\n",
	     s.Str(),
	     rangeMin, rangeMax);
 }
 else {
     gList sentence;
     gElem* ptrIter;

     s.SplitAnyChar( "\t ,;", sentence );

     for (ptrIter=sentence.StartPtr(); ptrIter; ptrIter=ptrIter->next) {
	 collect_line( *((gString*)ptrIter->me), words );
     }
 }// end ELSE: not quoted

 return 0;
}

////////////////////////////////////////////////////////////

int dbg_test (const char* str)
{
 int error( 0 );

 printf("%s%slanged MAJOR/MINOR %d.%d,\n\
	isubs MAJOR/MINOR %d.%d\n\
\n\
",
	str ? str : "\0",
	str ? "\t" : "\0",
	LIB_ILANGED_VERSION_MAJOR, LIB_ILANGED_VERSION_MINOR,
	LIB_ISUBS_VERSION_MAJOR, LIB_ISUBS_VERSION_MINOR);
 return error!=0;
}


int incr_time_string (int secs, gString& s)
{
 static char miniBuf[ 64 ];
 int total( 0 );
 gParam colon( s, "," );
 if ( colon.N()!=2 ) return -1;

 gString sLeft( colon.Str( 1 ) );
 gParam triple( sLeft, ":" );
 if ( triple.N()>=3 ) {
    total = (int)(atoi( triple.Str( 1 ) )* 60*60 + atoi( triple.Str( 2 ) )* 60 + atoi( triple.Str( 3 ) ));
 }
 else {
    total = (int)(atoi( triple.Str( 1 ) )* 60 + atoi( triple.Str( 2 ) ));
 }
 total += secs;
 if ( total<0 ) return 1;
 s.Reset();

 if ( triple.N()>=3 ) {
    sprintf(miniBuf,"%02u",total / (60*60));
    s.Add( miniBuf );
    s.Add( ":" );
 }
 sprintf(miniBuf,"%02u",(total / 60) % 60);
 s.Add( miniBuf );
 s.Add( ":" );
 sprintf(miniBuf,"%02u",total % 60);
 s.Add( miniBuf );
 s.Add( "," );
 s.Add( colon.Str( 2 ) );
 return 0;
}


int subs_time_range_change_offset (int secs, gString& sTimeRange)
{
 gParam fromto( sTimeRange, ">" );
 if ( fromto.N()!=2 ) return 2;
 gString sFrom( fromto.Str( 1 ) );
 gString sTo( fromto.Str( 2 ) );

 for ( ; sFrom.Length() && sFrom[ sFrom.Length() ]=='-'; ) {
     sFrom.Delete( sFrom.Length() );
 }
 for ( ; sTo.Length() && sTo[ sTo.Length() ]=='-'; ) {
     sTo.Delete( sTo.Length() );
 }
 sFrom.Trim();
 sTo.Trim();
 if ( sFrom.Find( ":" )==0 ) return 1;
 if ( sTo.Find( ":" )==0 ) return 1;

 incr_time_string( secs, sFrom );
 incr_time_string( secs, sTo );

 sTimeRange = sFrom;
 sTimeRange.Add( " --> " );
 sTimeRange.AddString( sTo );
 return 0;
}


int subs_prepare (const char* strFilename, int offset, int fromIdx, long nrLines, gElem* ptrSrt, gList& out)
{
 long iLine( 0 );
 int error( 0 );
 int countErrors( 0 );
 int state( 1 );
 int expectedRef( 1 );  // srt start at 1!
 t_uint32 value;
 gString sTimeRange;
 gString* ptrLine;
 gList talk;

 if ( ptrSrt==nil || nrLines<2 ) {
    fprintf(stderr,"%s: File too short.\n",strFilename);
 }

 for ( ; ptrSrt; ptrSrt=ptrSrt->next) {
     iLine++;
     ptrLine = (gString*)ptrSrt->me;
     value = ptrLine->ConvertToUInt32();
     if ( state<=1 ) {
        ptrLine->TrimRight();
        if ( value==0 ) {
             fprintf(stderr,"%s:%ld: Bogus value: {%s}\n",
                     strFilename, iLine, ptrLine->Str());
        }
        if ( ptrLine->Length()==0 ) {
             state = 0;
             continue;
        }
        if ( expectedRef!=(int)value ) {
             fprintf(stderr,"%s:%ld: Value not in sequence: {%s}\n",
                     strFilename, iLine, ptrLine->Str());
        }
        expectedRef = (int)value;
        state = 2;
     }
     else {
        if ( state==2 ) {
             sTimeRange = *ptrLine;
             state = 3;
        }
        else {
             DBGPRINT("len=%d, {%s}\n",ptrLine->Length(),ptrLine->Str());
             if ( ptrLine->Length()==0 ) {
                // Flush existing stuff
                gList* item( new gList );
                gList* flow( new gList );

                ASSERTION(item,"item");
                ASSERTION(flow,"flow");
                DBGPRINT_MIN("subs_time_range_change_offset(expectedRef=%d >= fromIdx=%d) %c\n",
			     expectedRef,
			     offset,
			     ISyORn(expectedRef >= fromIdx));

                error = subs_time_range_change_offset( expectedRef >= fromIdx ? offset : 0, sTimeRange );
                if ( error ) {
                     fprintf(stderr,"%s: wrong time range {%s}\n",strFilename,sTimeRange.Str());
                     countErrors++;
                }

                item->iValue = expectedRef;
                item->Add( sTimeRange );
                item->Add( "." );
                flow->CopyList( talk );
                item->AppendObject( flow );

                state = 0;
                expectedRef++;
                talk.Reset();
                sTimeRange.Reset();
                out.AppendObject( item );
             }
             else {
                talk.Add( *ptrLine );
                state++;
             }
        }
     }
 }

 if ( state!=0 ) {
    countErrors++;
    fprintf(stderr,"%s: unfinished state (%d)\n",strFilename,state);
 }
 return countErrors;
}


int dbg_dump_srt (const char* strFile)
{
 FILE* fOut( stdout );
 Subs subs;
 int error( isubs_dump_file( fOut, strFile, subs ) );

 DBGPRINT("DBG: dbg_dump_srt returns %d\n",error);
 return error;
}


int dbg_rewrite_subs_srt (char chrBackOrFwd, int seconds, char* const files[])
{
 char* srt;
 int error( 0 );
 int secs( chrBackOrFwd=='b' ? -seconds : seconds );
 FILE* fOut( stdout );

 if ( chrBackOrFwd!='b' && chrBackOrFwd!='f' ) return -1;

 for (files++; files && (srt = *files)!=nil; files++) {
     gFileFetch* inSubs;
     gList out;

     inSubs = new gFileFetch( srt );
     ASSERTION(inSubs,"inSubs");
     if ( inSubs->IsOpened() ) {
        fprintf(stderr,"Processing (%c, secs=%d): %s, lines: %u\n",chrBackOrFwd,secs,srt,inSubs->aL.N());
        error = subs_prepare( srt, secs, 0, (long)inSubs->aL.N(), inSubs->aL.StartPtr(), out );
        delete inSubs;

        if ( error ) {
            fprintf(stderr,"%s: Cowardly refusing to rewrite subs.\n",srt);
        }
        else {
#if 0
            fOut = stdout;
#else
            fOut = fopen( srt, "wb" );
#endif
            if ( fOut==nil ) {
                fprintf(stderr,"%s: cannot rewrite.\n",srt);
                continue;
            }

            for (gElem* dump=out.StartPtr(); dump; dump=dump->next) {
                gList* ptrItem( (gList*)dump->me );
                gList* follow( (gList*)ptrItem->StartPtr()->next->next->me );
                fprintf(fOut,"%d\r\n%s\r\n",ptrItem->iValue,ptrItem->Str( 1 ));
                for (int sub=1; sub<=(int)follow->N(); sub++) {
                    fprintf(fOut,"%s\r\n",follow->Str( sub ));
                }
                fprintf(fOut,"\r\n");
            }

            if ( fOut!=stdout ) fclose( fOut );
        }// IF error
     }
 }
 return error;
}

int dbg_rewrite_subs_at_srt (char chrBackOrFwd, int seconds, int fromIdx, char* const files[])
{
 char* srt;
 int error( 0 );
 int secs( chrBackOrFwd=='b' ? -seconds : seconds );
 FILE* fOut( stdout );

 if ( chrBackOrFwd!='b' && chrBackOrFwd!='f' ) return -1;

 for (files++; files && (srt = *files)!=nil; files++) {
     gFileFetch* inSubs;
     gList out;

     inSubs = new gFileFetch( srt );
     ASSERTION(inSubs,"inSubs");
     if ( inSubs->IsOpened() ) {
        fprintf(stderr,"Processing (%c, secs=%d): %s, lines: %u\n",chrBackOrFwd,secs,srt,inSubs->aL.N());
        error = subs_prepare( srt, secs, fromIdx, (long)inSubs->aL.N(), inSubs->aL.StartPtr(), out );
        delete inSubs;

        if ( error ) {
            fprintf(stderr,"%s: Cowardly refusing to rewrite subs.\n",srt);
        }
        else {
#if 0
            fOut = stdout;
#else
            fOut = fopen( srt, "wb" );
#endif
            if ( fOut==nil ) {
                fprintf(stderr,"%s: cannot rewrite.\n",srt);
                continue;
            }

            for (gElem* dump=out.StartPtr(); dump; dump=dump->next) {
                gList* ptrItem( (gList*)dump->me );
                gList* follow( (gList*)ptrItem->StartPtr()->next->next->me );
                fprintf(fOut,"%d\r\n%s\r\n",ptrItem->iValue,ptrItem->Str( 1 ));
                for (int sub=1; sub<=(int)follow->N(); sub++) {
                    fprintf(fOut,"%s\r\n",follow->Str( sub ));
                }
                fprintf(fOut,"\r\n");
            }

            if ( fOut!=stdout ) fclose( fOut );
        }// IF error
     }
 }
 return error;
}


int dbg_collect_words (const char* strSubsFile, FILE* fOut)
{
 Subs subs;
 int error;
 gElem* ptrOne( nil );

 isubs_dump_file( nil, strSubsFile, subs );
 error = subs.N()==0;

 if ( error ) {
     fprintf(stderr,"Error: %s\n",strSubsFile);
     return error;
 }

 ASSERTION(fOut,"fOut");

 gElem* ptrElem( subs.StartPtr() );
 gElem* ptrEntry;
 gString* pLine;
 gList words;
 int quoted( 0 );

 for ( ; ptrElem; ptrElem=ptrElem->next) {
     ptrEntry = ((SubEntry*)ptrElem->me)->StartPtr();
     ASSERTION(ptrEntry,"ptrEntry");

     ptrEntry = ptrEntry->next;

     if ( ptrEntry ) {
	 pLine = (gString*)ptrEntry->me;
	 if ( pLine->Find( "-->" )==0 ) {
	     fprintf(stderr,"Uops at iter %d {%s}\n",
		     ptrElem->me->iValue,
		     pLine->Str());
	 }
	 else {
	     ptrEntry = ptrEntry->next;
	 }
	 for ( ; ptrEntry; ptrEntry=ptrEntry->next) {
	     pLine = (gString*)ptrEntry->me;
	     collect_para( 0, *pLine, 1, pLine->Length(), quoted, words );
	 }
     }
 }

 for (ptrOne=words.StartPtr(); ptrOne; ptrOne=ptrOne->next) {
     fprintf(fOut,"%d\t%s\n",
	     ptrOne->me->iValue,
	     ptrOne->Str());
 }

 return 0;
}


int dbg_dump_html_keys (FILE* fOut, const char* strPattern, char* const strFiles[])
{
 const char* strFile;
 gList pat;

 ASSERTION(strFiles,"strFiles");
 strFiles++;

 pat.Add( (char*)strPattern );

 for ( ; (strFile = *strFiles)!=nil; strFiles++) {
     DBGPRINT("Parsing: %s\n",strFile);

     gFileFetch input( (char*)strFile );
     if ( input.IsOpened()==false ) {
	 fprintf(stderr,"Uops: %s\n",strFile);
	 return 2;
     }

     fprintf(stderr,"Parsing html /OR/ text input: %s\n",strFile);
     dump_html_keys( fOut, pat, input.aL );
 }
 return 0;
}


int do_debug (const char* strCommand, char* const args[])
{
 int error( 0 );
 const char* strFile;
 char* strOpt;

 if ( strcmp( strCommand, "lib" )==0 ) {
     printf("LIB_ILANGED_VERSION_MAJOR.MINOR %d.%d\n",
	    LIB_ILANGED_VERSION_MAJOR,
	    LIB_ILANGED_VERSION_MINOR);
     return 0;
 }

 strOpt = (char*)args[ 1 ];
 strFile = strOpt;

 switch ( strCommand[ 0 ] ) {
 case 'a':
     error = dbg_test( strFile );
     break;

 case 'b':
     error = dbg_dump_srt( strFile );
     break;

 case 'c':
     if ( strFile ) {
	 if ( args[ 2 ] ) {
	     fprintf(stderr,"Ignored params: %s...\n",args[ 2 ]);
	 }
     }
     error = dbg_collect_words( strFile, stdout );
     break;

 case 'd':
     if ( strFile && strFile[ 0 ] ) {
	 error = dbg_dump_html_keys( stdout, args[ 1 ], args+1 );
     }
     break;

 case 'e':
     if ( strOpt && strOpt[ 0 ] ) {
	 error = dbg_rewrite_subs_srt( strOpt[ 0 ], atoi( strOpt+1 ), args+1 );
     }
     else {
         fprintf(stderr,"Invalid bNNN fNNN, for back or forward NNN seconds...\n");
     }
     break;

 case 'f':
     if ( strOpt && strOpt[ 0 ] ) {
	 if ( args[ 1 ] && args[ 2 ] && args[ 2 ][ 0 ] ) {
	     int fromIdx( atoi( args[ 2 ] ) );
	     error = ( fromIdx<=0 && strcmp( args[ 2 ], "0" )!=0 );
	     if ( error==0 ) {
		 error = dbg_rewrite_subs_at_srt( strOpt[ 0 ], atoi( strOpt+1 ), fromIdx, args+1 );
	     }
	 }
	 else {
	     fprintf(stderr,"No from-index, or No file?!\n");
	 }
     }
     else {
         fprintf(stderr,"Invalid bNNN fNNN, for back or forward NNN seconds...\n");
     }
     break;

 default:
     return usage_option( nil );
 }
 return error;
}


int main (int argc, char* argv[])
{
 int error;

 gINIT;

 if ( argc<2 || (argv[ 2 ] && strcmp( argv[ 2 ], "-h" )==0) ) {
     gEND;
     return usage_option( argv[ 1 ] );
 }

 // Unicode, make hashing
 IMEDIA_INIT;

 langed_init();

 error = do_debug( argv[ 1 ], argv+1 );
 DBGPRINT("do_debug (%s) returned %d\n",
	  argv[ 1 ],
	  error);

 langed_finit();

 // Unicode, data release
 imb_iso_release();

 DBGPRINT("DBG: Objs: %d\n",gStorageControl::Self().NumObjs());
 gEND;

 return error;
}
////////////////////////////////////////////////////////////

