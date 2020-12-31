#ifndef IICRC32_X_H
#define IICRC32_X_H

#include "istring.h"


#define CRC_BUFFER_SIZE  8192

////////////////////////////////////////////////////////////
class iCRC32 : public gString {
public:
    iCRC32 () ;

    iCRC32 (char* strShown) ;

    virtual ~iCRC32 () ;

    // Public data-members
    int lastOpError;

    // Get methods
    virtual t_uint32 CRC32 () {
	return (t_uint32)iValue;
    }

    virtual t_uchar* ToStr () {
	Set( "01234567" );
	sprintf( (char*)str, "%08X", CRC32() );
	return str;
    }

    // Set methods
    virtual int SetCRC32 (t_uint32 uCRC) {
	return iValue = (int)uCRC;
    }

protected:
    int thisFromHex (const char* aStr, unsigned long& myValue) {
	myValue = 0;
	if ( aStr ) {
	    unsigned val;
	    if ( sscanf( aStr, "%X", &val )>=1 ) {
		myValue = val;
		return 0;
	    }
	}
	return -1;
    }
};

////////////////////////////////////////////////////////////

extern iCRC32* crc32_from_handle (int fd) ;

////////////////////////////////////////////////////////////
#endif //IICRC32_X_H

