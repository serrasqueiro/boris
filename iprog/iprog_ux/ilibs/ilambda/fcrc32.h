#ifndef FCRC32_X_H
#define FCRC32_X_H

#include "istring.h"


#define CRC_BUFFER_SIZE  8192

////////////////////////////////////////////////////////////
class fCRC32 : public gString {
public:
    fCRC32 () ;

    fCRC32 (char* strShown) ;

    virtual ~fCRC32 () ;

    // Public data-members
    int lastOpError;

    // Get methods
    virtual t_uint32 CRC32 () {
	return (t_uint32)iValue;
    }

    virtual int ComputeCRC32 (const char* strName, t_uint32& outCrc32) ;

    unsigned long UpdateCRC (unsigned long CRC, const char *buffer, long count) ;

    virtual t_uchar* ToStr () {
	Set( "01234567" );
	sprintf( (char*)str, "%08X", CRC32() );
	return str;
    }

    // Set methods
    virtual int SetCRC32 (t_uint32 uCRC) {
	return iValue = (int)uCRC;
    }

    virtual int FromBuffer (t_uchar* uStr, unsigned uSize) {
	return 0;  // TODO
    }

    virtual int FromFile (const char* strFile) {
	return 0;  // TODO
    }

    virtual int ToSFV (gList& listedFiles, int trimPath, gList& outSFV) {
	return 0;  // TODO
    }

    virtual int FromSFV (gList& inSFV, const char* strPath, gList& checkedFiles) {
	return 0;  // TODO
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

    unsigned long bsdFileCRC (const char* filename) ;
};
////////////////////////////////////////////////////////////

extern int hex_char (char chr) ;

////////////////////////////////////////////////////////////
#endif //FCRC32_X_H

