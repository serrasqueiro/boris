#ifndef iCONFIG_X_H
#define iCONFIG_X_H

#include "ilist.h"
#include "ifile.h"
#include "ifilestat.h"
#include "icalendar.h"

#define CONF_CHR_LINE_ERR 0x255
////////////////////////////////////////////////////////////
class gFileFetch : public gFileText {
public:
    gFileFetch (int maxLines=-1) ;
    gFileFetch (gString& sFName, int maxLines=-1) ;
    gFileFetch (const char* fName, int maxLines=-1, bool aShowProgress=false) ;
    gFileFetch (gString& sInput, bool aShowProgress) ;

    virtual ~gFileFetch () ;

    // Public data-members
    gList aL;
    bool doEndNewLine;

    // Get methods
    virtual bool IsBufferOk () {
	return isFetchBufferOk;
    }

    virtual char* Str (unsigned idx=0) {
	return aL.Str( idx );
    }

    virtual bool BufferAutoAdjust () {
	return doResize;
    }

    // Set methods
    bool Fetch (gString& sFName) ;

    void AdjustBuffer (bool doBufferResize) {
	doResize = doBufferResize;
    }

    virtual bool SetFileReport (FILE* fRep) ;
    virtual bool SetDeviceReport (eDeviceKind aDKind) ;

protected:
    int maxNLines;
    bool isFetchBufferOk;
    bool doResize;

    bool doShowProgress;
    FILE* fVRepErr;

    int thisReadFile (bool& isOk, gList& zL) ;
    int thisReadAll (bool& isOk, bool& isBufOk, gList& zL) ;
    int thisReadFileThrough (gList& zL, t_uint32& nBytes) ;
    int thisReadStringAsFile (gString& sInput, gList& zL) ;

private:
    // Operators,empty
    gFileFetch (gFileFetch& ) ; //empty
    gFileFetch& operator= (gFileFetch& ) ; //empty
};
////////////////////////////////////////////////////////////
#endif //iCONFIG_X_H

