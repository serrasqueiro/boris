// acdb.cpp


#include <stdlib.h>
#include <unistd.h>

#include "acdb.h"

////////////////////////////////////////////////////////////
const char* sCdpDisc::cdbKeys[]={
    nil,
    "DISCID=",  // 1
    "DLENGTH=",  // 2
    "DLOADED=",  // 3
    "DTITLE=",  // 4
    "DARTIST=",  // 5
    "DLABEL=",  // 6
    "DCOMP=",  // 7
    "DSET=",  // 8 (0 means no; 1 means yes; ignored
    "DSETNUMBER=",  // 9
    "DSETTOTAL=",  // 10
    "DYEAR=",  // 11
    "REGION=",  // 12
    "LANGUAGE=",  // 13
    "DGENRE1=",  // 14
    "DGENRE2=",  // 15
    "DNOTES=",  // 16
    "TRACKS=",  // 17
    nil,
    nil
    // Not used:
    //		DPLAYED=1...
    //		DCOVER=...
};


bool sCdpDisc::SetByRawString (gString& sRaw, int optPosEq)
{
 bool isOk( optPosEq>0 );
 gString sLeft;
 gString sValue( sRaw );
 int value;
 int whichKey;
 char* strLeft;

 if ( optPosEq>0 ) {
     sLeft.CopyFromTo( sRaw, 1, optPosEq );
     sValue.Delete( 1, optPosEq );
 }

 strLeft = sLeft.Str();
 value = atoi( sValue.Str() );

 whichKey = WhichIndexFromKey( sLeft );
 DBGPRINT("DBG: SetByRawString(%s,%d), key#%d\n",
	  strLeft,
	  optPosEq,
	  whichKey);

 switch ( whichKey ) {
 case 1:
     cddb1 = 0;
     sscanf( strLeft, "%x", &cddb1 );
     break;

 case 2:
     length = (t_int32)value;
     break;

 case 3:
     // Ignored
     break;

 case 4:
     dTitle = sValue;
     break;

 case 5:
     dArtist = sValue;
     break;

 case 6:
     dLabel = sValue;
     break;

 case 7:
     isCompilation = value!=0;
     break;

 case 8:
     // Ignored
     break;

 case 9:
     discSet = (t_int16)value;
     break;

 case 10:
     discSetNumber = (t_int16)value;
     break;

 case 11:
     year = value<0 ? 0 : (t_uint16)value;
     break;

 case 12:
     sRegion = sValue;
     break;

 case 13:
     sLanguage = sValue;
     break;

 case 14:
     genre1 = value;
     break;

 case 15:
     genre2 = value;
     break;

 case 17:  // TRACKS=1,2,...
     buf_to_list( sValue.UStr(), sValue.Length(), ',', tracksListed );
     //printf("DBG: listed TRACKS: "); tracksListed.Show();
     break;

 case 16:
     // No break here
 default:
     if ( rawNotes ) {
	 rawNotes->Add( sRaw );
     }
     break;
 }
 return isOk;
}

////////////////////////////////////////////////////////////
bool sCdpCdb::AddTitleFromRawName (gString& sRaw)
{
 gString sCopy;
 gString sValue;

 ASSERTION(sRaw.Length(),"sRaw");
 if ( EqLine( sRaw, sCopy, sValue ) ) {
     titles.Add( sValue );
     titles.EndPtr()->me->iValue = sCopy.iValue;
     DBGPRINT_MIN("DBG: AddTitleFromRawName(%s): {%s}, title#%d\n",
		  sRaw.Str(),
		  sValue.Str(),
		  sCopy.iValue);
 }
 return sValue.iValue>0;
}


bool sCdpCdb::EqLine (gString& sRaw, gString& sLeft, gString& sValue)
{
 bool isOk;
 int value( -1 );
 unsigned uPos( sRaw.Find( '=' ) );

 ASSERTION(sRaw.Length(),"sRaw");
 sValue.Delete();
 sLeft.CopyFromTo( sRaw, 1, uPos );
 if ( uPos ) {
     sValue.CopyFromTo( sRaw, uPos+1 );
 }
 isOk = findback_non_digit( sLeft.UStr(), value )>=0;
 sLeft.iValue = value;
 sValue.iValue = atoi( sValue.Str() );
 return isOk;
}

////////////////////////////////////////////////////////////
int buf_to_list (const t_uchar* uBuf, int bufSize, t_uchar newLine, gList& result)
{
 int countLines( 0 );
 int idx( 0 );
 int size( 0 );
 bool toIntToo( newLine==',' );
 t_uchar uChr;
 t_uchar* aStr;

 // newLine is typically '\n';
 // if it is ',' (comma), then additionally added strings will be attempted to be converted to int too.

 result.Delete();
 if ( uBuf==nil ) return -1;
 if ( bufSize<=0 ) return 0;

 aStr = (t_uchar*)calloc( bufSize+1, sizeof(t_uchar) );

 for ( ; idx<bufSize; idx++) {
     uChr = uBuf[ idx ];

     if ( uChr=='\r' ) continue;

     if ( uChr==newLine ) {
	 result.Add( aStr );
	 if ( toIntToo ) {
	     result.EndPtr()->me->iValue = atoi( (char*)aStr );
	 }
	 size = 0;
     }
     else {
	 aStr[ size++ ] = uChr;
     }
     aStr[ size ] = 0;
 }

 if ( size ) {
     result.Add( aStr );
     if ( toIntToo ) {
	 result.EndPtr()->me->iValue = atoi( (char*)aStr );
     }
 }
 free( aStr );
 countLines = (int)result.N();
 return countLines;
}


int findback_non_digit (const t_uchar* uBuf, int& value)
{
 int len, idx;
 t_uchar uChr;
 const char* aStr( (char*)uBuf );

 ASSERTION(uBuf,"uBuf");
 value = 0;

 for (len=strlen( aStr ), idx=len-1; idx>0; ) {
     uChr = uBuf[ idx-- ];
     if ( uChr=='=' ) break;
 }
 for ( ; idx>=0; idx--) {
     uChr = uBuf[ idx ];
     if ( uChr<'0' || uChr>'9' ) {
	 value = atoi( aStr+idx+1 );
	 return value;
     }
 }
 return -1;
}

////////////////////////////////////////////////////////////
int acdb_read (int fdIn, sCdpCdb& cdp)
{
 const int maxRead( 16 * 1024 );
 int error( fdIn==-1 );
 int didRead;
 t_uchar* uBuf;
 gList input;

 cdp.ReleaseAll();
 if ( error ) return -1;

 uBuf = (t_uchar*)calloc( maxRead+1, sizeof(t_uchar) );
 if ( uBuf==nil ) return -1;

 didRead = read( fdIn, uBuf, maxRead );
 if ( didRead<=0 ) {
     return (int)(cdp.anError = ece_INPUT_TOO_SHORT);
 }
 if ( didRead>=maxRead ) {
     return (int)(cdp.anError = ece_INPUT_TOO_BIG);
 }

#ifdef ACDB_STORE_ALL
 if ( cdp.rawCredits==nil ) cdp.rawCredits = new gList;
 if ( cdp.rawSegments ) cdp.rawSegments = new gList;
 if ( cdp.disc.rawNotes ) cdp.disc.rawNotes = new gList;
#endif

 buf_to_list( uBuf, didRead, '\n', input );
 return acdb_parse( input, cdp );
}


int acdb_parse (gList& input, sCdpCdb& cdp)
{
 bool did;
 bool disc( false );
 int iter( 0 );
 int posEq( 0 );
 gElem* ptrStart( input.StartPtr() );
 gElem* ptrElem( ptrStart );
 char* strLine;

 cdp.anError = ece_NO_ERROR;

 // Check syntax and number
 for ( ; ptrElem; ptrElem=ptrElem->next) {
     did = false;
     strLine = ptrElem->Str();
     if ( strLine[ 0 ]=='#' ) continue;

     gString s( strLine );
     s.TrimRight();

     posEq = s.Find( '=' );
     if ( posEq==0 ) {
	 cdp.anError = ece_MISSING_EQ;
	 continue;
     }

     if ( strncmp( strLine, "CRD", 3 )==0 ) {
	 did = true;
	 // 'CRDNAME:0' or other 'CRD...':
	 if ( cdp.rawCredits ) {
	     cdp.rawCredits->Add( s );
	 }
     }
     else {
	 if ( s[ 1 ]=='S' ) {
	     did = true;
	     if ( cdp.rawSegments ) {
		 cdp.rawSegments->Add( s );
	     }
	 }
	 else {
	     disc = s[ 1 ]=='D';
	     did = disc || cdp.disc.WhichIndexFromLine( s )>0;
	     if ( did ) {
		 cdp.disc.SetByRawString( s, posEq );
	     }
	     else {
		 did = s[ 1 ]=='T';
		 if ( did ) {
		     if ( s.Find( "TTITLE" )==1 ) {
			 if ( cdp.AddTitleFromRawName( s ) ) {
			 }
		     }
		 }// is 'T...=...'
	     }
	 }
     }

     if ( did ) continue;

     fprintf(stderr,"Ignored: %s\n",strLine);
 }

 cdp.nTitles = (int)cdp.titles.N();

 delete[] cdp.ptrTitles;
 cdp.ptrTitles = new sCdpTitle[ cdp.nTitles+1 ];
 ASSERTION(cdp.ptrTitles,"cdp.ptrTitles");

 // Add titles
 for (iter=1, ptrElem=cdp.titles.StartPtr(); ptrElem; ptrElem=ptrElem->next) {
     cdp.ptrTitles[ iter ].nrTitle = iter;
     cdp.ptrTitles[ iter ].sName.Set( ptrElem->Str() );
     iter++;
 }

#if 0
 gElem* ptrDbg;
 printf("\nDBG: Credits:::\n");
 for (ptrDbg=(cdp.rawCredits ? cdp.rawCredits->StartPtr() : nil); ptrDbg; ptrDbg=ptrDbg->next) {
     printf("%s\n",ptrDbg->Str());
 }
 printf("\nDBG: Segments:::\n");
 for (ptrDbg=(cdp.rawSegments ? cdp.rawSegments->StartPtr() : nil); ptrDbg; ptrDbg=ptrDbg->next) {
     printf("%s\n",ptrDbg->Str());
 }
 if ( cdp.rawExtra ) printf("\nDBG: Extra!\n");
 for (ptrDbg=(cdp.rawExtra ? cdp.rawExtra->StartPtr() : nil); ptrDbg; ptrDbg=ptrDbg->next) {
     printf("%s\n",ptrDbg->Str());
 }

 printf("\nDISC rawNotes:::\n");
 for (ptrDbg=(cdp.disc.rawNotes ? cdp.disc.rawNotes->StartPtr() : nil); ptrDbg; ptrDbg=ptrDbg->next) {
     gString sDbg( ptrDbg->Str() );
     if ( sDbg.Length()>80 ) {
	 sDbg.InsertStr( "[...]", 80 );
	 sDbg.Delete( 80+5, sDbg.Length()-(10*(sDbg.Length()>80+10)) );
     }
     printf("%s\n",sDbg.Str());
 }

 printf("\nDISC tracksListed:::\n");
 ptrDbg = cdp.disc.tracksListed.StartPtr();
 if ( ptrDbg ) {
     gElem* dbgStart( ptrDbg );
     printf("Listed: ");
     for ( ; ptrDbg; ptrDbg=ptrDbg->next) {
	 printf("%s%d%s",
		dbgStart==ptrDbg ? "" : ",",
		ptrDbg->me->iValue,
		ptrDbg->next ? "" : "\n");
     }
 }

 // Show disc
 printf("dTitle: %s\n",cdp.disc.dTitle.Str());
 printf("dArtist: %s\n",cdp.disc.dArtist.Str());
 printf("dLabel: %s\n",cdp.disc.dLabel.Str());
 printf("Language: %s\n",cdp.disc.sLanguage.Str());
 printf("Region: %s\n",cdp.disc.sRegion.Str());

 for (iter=1; iter<=cdp.nTitles; iter++) {
     printf("Title-%02d: %s%s\n",
	    iter,
	    iter==cdp.ptrTitles[ iter ].nrTitle ? "  " : "!!",
	    cdp.ptrTitles[ iter ].sName.Str());
 }
#endif // debug only

 return 0;
}

////////////////////////////////////////////////////////////

