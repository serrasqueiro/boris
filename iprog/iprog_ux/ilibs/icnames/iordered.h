#ifndef iORDERED_X_H
#define iORDERED_X_H


#include "ilist.h"

////////////////////////////////////////////////////////////
struct sIOrdered {
    sIOrdered ()
	: fd( -1 ),
	  usedBytes( 0 ),
	  sizeEstimation( 0 ),
	  sizeBytes( 0 ),
	  lines( 0 ),
	  repeated( 0 ),
	  ptrLast( nil ) {
    }

    ~sIOrdered () {
	ReleaseAll();
    }

    int fd;
    long usedBytes;
    long sizeEstimation;
    long sizeBytes;

    int lines;
    int repeated;

    gList lista;
    gString named;
    gString sFirst;
    gString sLast;

    gElem* ptrLast;

    // Methods

    int CopyRefs (sIOrdered& copy) {
	fd = copy.fd;
	named = copy.named;
	return fd;
    }

    void Release () {
	sFirst.Delete();
	sLast.Delete();
	lista.Delete(); ptrLast=nil;
	usedBytes = 0;
	repeated = 0;
    }

    bool ReleaseAll (bool closeHandle=true) {
	Release();
	if ( closeHandle )
	    return CloseHandle();
	fd = -1;
	sizeEstimation = sizeBytes = 0;
	return false;
    }

    bool CloseHandle () ;

    int AddUnique (const char* oBuf, int length, int maxLenCompare=-1) ;
};

////////////////////////////////////////////////////////////
// icn 'ordered' functions
////////////////////////////////////////////////////////////

extern int icn_iordered_read (int fdIn, sIOrdered& order) ;

////////////////////////////////////////////////////////////
#endif //iORDERED_X_H

