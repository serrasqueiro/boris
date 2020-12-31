// cue.cpp

#include <string.h>

#include "cue.h"

////////////////////////////////////////////////////////////
iCue::iCue (gElem* pElem)
{
 thisCopyFromElement( pElem );
}


iCue::iCue (const char* aStr)
{
}


iCue::~iCue ()
{
}


gList* iCue::NewLine (gString& sLine)
{
 gList* newObj( new gList );
 // #warning TODO
 // double-quotes parsing please!
 return newObj;
}


int iCue::thisParseCue (gList& lines)
{
 int line( 0 );
 gElem* pElem( lines.StartPtr() );

 //	REM GENRE "Classic Rock"
 //	REM DATE 1985
 //	REM DISCID hex-string
 //	REM COMMENT "ExactAudioCopy v0.95b4"
 //	PERFORMER "Sup"
 //	TITLE "Breakfast"
 //	FILE "Sup - Bound.flac" WAVE
 //	  TRACK 01 AUDIO
 //	    TITLE "Cannon"
 //	    PERFORMER "Sup"
 //	    INDEX 00 00:00:00
 //	    INDEX 01 00:00:33
 //	  TRACK 02 AUDIO
 //	    TITLE "Still in Love"
 //	    PERFORMER "Sup"
 //	    INDEX 01 07:39:00
 //	...

 for ( ; pElem; pElem=pElem->next) {
     line++;

     gList* pLine( NewLine( *(gString*)(pElem->me) ) );

     printf("%d\t{%s}\n", line, pLine->Str());
 }

 /*
 gString sDiscID;
 gString sPerformer;
 gString sTitle;
 gString sFile;
 */
 // #warning TODO
 return -1;
}


int iCue::thisAddIgnored (gString& sLine, int line)
{
 if ( sLine.Length() ) {
     ignored.Add( sLine );
     ignored.EndPtr()->iValue = line;
     return 1;
 }
 return 0;
}

////////////////////////////////////////////////////////////

