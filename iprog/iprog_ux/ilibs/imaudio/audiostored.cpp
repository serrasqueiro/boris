// audiostored.cpp

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "audiostored.h"

#include "ioaux.h"  // from ilog

////////////////////////////////////////////////////////////
// Static members
// ---------------------------------------------------------
// <>
////////////////////////////////////////////////////////////
imTag::imTag (char* aStr)
    : gString( aStr ),
      recordingYear( -1 ),
      releaseYear( -1 )
{
 memset( trackId, 0x0, sizeof(trackId) );
}


imTag::~imTag ()
{
}


gString& imTag::TrackDesc ()
{
 thisOneOfDescription( trackId, 0, sData );
 return sData;
}


gString& imTag::DiscDesc ()
{
 thisOneOfDescription( trackId, 2, sData );
 return sData;
}


int imTag::thisOneOfDescription (t_int8* values, int whence, gString& sResult)
{
 char smallBuf[ 32 ];
 // Returns 1 on error!
 sData.SetEmpty();
 if ( values[ whence ]>0 && values[ whence+1 ]>0 ) {
     sprintf( smallBuf, "%02d/%02d", values[ whence ], values[ whence+1 ] );
     sData.Set( smallBuf );
     return values[ whence ]>values[ whence+1 ];
 }
 return 1;
}

////////////////////////////////////////////////////////////
imTags::imTags ()
    : tagKind( noTag ),
      nOfTags( 0 ),
      tagIds( nil )
{
}


imTags::~imTags ()
{
}


bool imTags::IsOk ()
{
 int iterTag( 1 );
 bool tagOk;

 if ( tagKind==noTag || tagKind==unknownFormat ) return true;
 ASSERTION(tagIds,"tagIds");

 for ( ; iterTag<=nOfTags; iterTag++) {
     tagOk = tagIds[ iterTag ].IsOk();
     if ( tagOk==false ) return false;
 }

 return true;
}


bool imTags::SetTag (eTagKind whichTag, char* strTag)
{
 tagKind = whichTag;

 switch ( whichTag ) {
 case noTag:
     delete[] tagIds;
     tagIds = nil;
     nOfTags = 0;
     break;

 case id3v1:
     ASSERTION(strTag==nil,"TODO otherwise");
     break;

 case id3v2:
     ASSERTION(strTag,"TODO otherwise");  // parse ID3v2 string
     // e.g. TAG: ID3[02][00][00][00][00][00]%TT2[00][00][10][00]Heart of Glass[00]TP1[00][00][09][00]Blondie[00][EOH] (47) Blondie - Heart of Glass.mp3

     DBGPRINT_MIN("DBG: TAG id3v2 {%s}\n",strTag);
     break;

 case apeTag:
     DBGPRINT_MIN("DBG: APETAG {%s}\n",strTag);
     // TODO!
     break;

 case unknownFormat:
 default:
     DBGPRINT_MIN("DBG: TAG_INVALID {%s}\n",strTag);
     tagKind = unknownFormat;
     break;
 }

 return true;
}

////////////////////////////////////////////////////////////
imAudioPiece::imAudioPiece ()
    : builtTag( nil ),
      nrNotices( 0 ),
      nrWarns( 0 ),
      nrErrors( 0 ),
      milliseconds( 0 ),
      avgBitrateKbps( 0 ),
      bitrateKind( bitrateNotKnown )
{
}


imAudioPiece::~imAudioPiece ()
{
}


bool imAudioPiece::IsOk ()
{
 return IsEmpty()==false;
}


imTag* imAudioPiece::PreferredTag ()
{
 return builtTag;
}


bool imAudioPiece::SetErrorLine (gString& sLine)
{
 unsigned pos;
 long ms( -1L );
 int bitrate( -1 );
 gString sFirst( sLine );
 char bufLine[ sLine.Length()+8 ];

 sErrorLine.Delete();
 bufLine[ 0 ] = 0;

 nrNotices = nrWarns = nrErrors = 0;
 milliseconds = 0;
 avgBitrateKbps = 0;
 bitrateKind = bitrateNotKnown;

 if ( sFirst.Find( "Error(s):" )==1 ) {
     sFirst.Delete( 1, 9 );
     sFirst.Trim();
     pos = sFirst.Find( "average var." );
     if ( pos ) {
	 sFirst.Delete( pos + 8, pos + 12 );
     }
     pos = sFirst.Find( ',' );
     nrErrors = (t_int16)atoi( sFirst.Str() );
     if ( pos ) {
	 sFirst.Delete( 1, pos );
	 sFirst.Trim();
     }
     sscanf( sFirst.Str(), "%ld ms; %s bitrate: %d",
	     &ms,
	     bufLine,
	     &bitrate );
     if ( ms>=0 && bitrate>=0 ) {
	 sErrorLine = sFirst;
	 milliseconds = (t_uint32)ms;
	 avgBitrateKbps = bitrate;
	 if ( strstr( bufLine, "aver" )==0 ) {
	     bitrateKind = bitrateVariable;
	 }
	 else {
	     bitrateKind = strstr( bufLine, "constant" )==nil ? bitrateInvalid : bitrateConstant;
	 }
     }
 }

 DBGPRINT("DBG: {%s}, %d ms, %d kbps (bitrate kind: %d)\n\n",
	  sErrorLine.Str(),
	  milliseconds,
	  avgBitrateKbps,
	  bitrateKind);
 return sErrorLine.Length();
}


int imAudioPiece::NormalizeErrors ()
{
 // Based on 'infos' strings, it normalizes into errors / warnings / notices:
 // see also:	media/audio/samples/report_errors.txt

 gElem* pElem( infos.StartPtr() );
 imAudioPiece* piece;
 char* strError;

 for (nrErrors=0; pElem; pElem=pElem->next) {
     piece = (imAudioPiece*)pElem->me;
     strError = piece->Str();
     DBGPRINT_MIN("DBG: NormalizeErrors (nrErrors=%d): {%s}\n\n",nrErrors,strError);
     if ( gio_findstr( strError, " last frame"  )>1 ||
	  gio_findstr( strError, " first frame" )>1 ||
	  gio_findstr( strError, " junk: possible " )>1 ||
	  gio_findstr( strError, " tag trailer " )>1 ) {
	 nrWarns++;
     }
     else {
	 if ( gio_findstr( strError, " bitrate switching " )>1 ||
	      gio_findstr( strError, " bit switching " )>1 ||
	      gio_findstr( strError, "mode switching " )>=0 ) {
	     nrNotices++;
	 }
	 else {
	     nrErrors++;
	 }
     }
 }
 DBGPRINT_MIN("DBG: NormalizeErrors (nrErrors=%d) FINAL\n\n",nrErrors);
 return nrErrors;
}

////////////////////////////////////////////////////////////

