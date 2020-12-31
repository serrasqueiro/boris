// iID3vx.cpp

#include <unistd.h>

#include "iID3vx.h"

////////////////////////////////////////////////////////////
// Static members
// ---------------------------------------------------------
// <>
////////////////////////////////////////////////////////////
ID3VX::ID3VX (char* strName)
    : gList( strName ),
      tSize( 0 ),
      code( 0 )
{
 //  http://stackoverflow.com/questions/16399604/how-to-read-id3v2-tag
}


ID3VX::~ID3VX ()
{
}


void ID3VX::Reset ()
{
 Delete();
 code = 0;
 tSize = 0;
 memset( &myTag, 0, sizeof(myTag) );
}


bool ID3VX::SetTag (sID3main& aTag)
{
 bool validTag( strncmp( (char*)aTag.tag, "ID3", 3 )==0 );
 memcpy( &myTag, &aTag, sizeof(sID3main) );
 tSize = Value4Bytes( myTag.frameSize );
 return validTag;
}


int ID3VX::NewRead (int handle, t_uint32 inSize)
{
 t_uint8 uChr;
 t_uint8* newBuf( nil );
 const int maxBuf( 1024 * 1024 );
 int bufSize( inSize > (t_uint32)maxBuf ? maxBuf : (int)inSize );
 int io;
 int value( -1 ), last( 0 );

 inBuffer.Delete();
 inLines.Delete();

 if ( handle!=-1 ) {
     newBuf = new t_uint8[ bufSize+1 ];
     if ( newBuf==nil ) return -1;  // overflow
     io = read( handle, newBuf, bufSize );

     for (int idx=0; idx<io; idx++, last=value) {
	 uChr = newBuf[ idx ];
	 value = (int)uChr;
	 if ( uChr>=' ' && uChr<127 ) {
	 }
	 else {
	     if ( uChr ) {
		 uChr = '.';
	     }
	     else {
		 uChr = ' ';
	     }
	 }
	 if ( last==0xFF && value==0xFE ) {
	     inBuffer.Add( "~~" );
	 }
	 else {
	     inBuffer.Add( uChr );
	 }
	 inBuffer.EndPtr()->me->iValue = value;
     }
 }

 delete[] newBuf;

 return 0;
}

////////////////////////////////////////////////////////////

