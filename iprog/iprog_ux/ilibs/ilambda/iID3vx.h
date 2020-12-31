#ifndef IID3VX_X_H
#define IID3VX_X_H

#include "ilist.h"

////////////////////////////////////////////////////////////
struct sID3main {
    t_uchar tag[ 3 ];
    t_uint8 version[ 2 ];
    t_uint8 flags;
    t_uint8 size[ 4 ];
    t_uint8 frameId[ 4 ];
    t_uint8 frameSize[ 4 ];
    t_uint8 frameFlags[ 2 ];
};

////////////////////////////////////////////////////////////
class ID3VX : public gList {
public:
    ID3VX (char* strName=nil) ;

    virtual ~ID3VX () ;

    // Public data-members
    sID3main myTag;
    t_uint32 tSize;

    gList inBuffer; // buffer translated in char by char
    gList inLines;  // useful for a fancy output


    // Get methods

    int GetCode () {
	return code;
    }


    // Set methods

    virtual void Reset () ;

    virtual bool SetTag (sID3main& aTag) ;


    // Special methods

    virtual t_uint32 Value4Bytes (t_uint8 values[ 4 ]) {
	// ulong iSize = (ulong)frameSize[0] << 21 | (ulong)frameSize[1] << 14 | (ulong)frameSize[2] << 7 | (ulong)frameSize[3];
	return
	    (t_uint32)values[0] << 21 | (t_uint32)values[1] << 14 | (t_uint32)values[2] << 7 | (t_uint32)values[3];
    }

    virtual int NewRead (int handle, t_uint32 inSize) ;

private:
    int code;

    // Operators,empty
    ID3VX (ID3VX& copy) ;  //empty
    ID3VX& operator= (ID3VX& ) ;  //empty
};

////////////////////////////////////////////////////////////
#endif //IID3VX_X_H

