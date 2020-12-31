#ifndef iPOPAUTH_X_H
#define iPOPAUTH_X_H

#include <string.h>

#include "ibase.h"
#include "ilist.h"
#include "ifilestat.h"
#include "inet.h"

#ifdef DEBUG_PIO
#define DBGPRINT_PIO(args...) printf(args)
#else
#define DBGPRINT_PIO(args...) ;
#endif

typedef t_uint16 t_os_id;
////////////////////////////////////////////////////////////
class iPopUserPass : public gString {
public:
    // Public enums
    enum ePassError {
	e_PassNone,
	e_PassOk,
	e_PassUndecoded,
	e_PassIsInvalid
    };

    // Constructors, etc.
    iPopUserPass (const char* strUser, const char* strPass=nil) ;

    virtual ~iPopUserPass () ;

    gString pass;

    virtual bool IsOk () {
	return Length()>0 && pass.Length()>0;
    }

    // Set methods
    virtual void Reset () ;

    virtual bool SetUser (const char* aStr, bool doIgnoreNL=true, int minSize=1, int maxSize=80) {
	return thisSetStrLimited( aStr, minSize, maxSize, 6, *this )==0;
    }

    virtual bool SetPass (const char* aStr, bool doIgnoreNL=true, int minSize=0, int maxSize=80) {
	return thisSetStrLimited( aStr, minSize, maxSize, 5, pass )==0;
    }

    // Special methods
    virtual char* PassBase65 () ;

    virtual int FromBase65 (const char* aStr, gString& sResult) ;

protected:
    ePassError passStatus;

    int thisSetStrLimited (const char* aStr,
			   int minSize,
			   int maxSize,
			   t_uint16 mask,
			   gString& sResult) ;

private:
    gString encodes;

    // Operators,empty
    iPopUserPass (iPopUserPass& ) ; //empty
    iPopUserPass& operator= (iPopUserPass& ) ; //empty
};
////////////////////////////////////////////////////////////
class iCredential : public iPopUserPass {
public:
    enum eCredStatus {
	e_NotKnown,
	e_Invalid,
	e_Valid
    };

    iCredential (const char* strUser=nil, const char* strPass=nil) ;

    virtual ~iCredential () ;

    // Get methods
    virtual bool IsOk () {
	return credStatus==e_Valid;
    }

    virtual eCredStatus CredStatus () {
	return credStatus;
    }

    virtual char* FindKeyword (const char* strKeyc, gList& list) ;

    // Special methods
    virtual int CreateFile (const char* filename) ;

    virtual int FromFile (const char* strFile=nil) ;

    virtual int ToFile (const char* strFile, bool reuse=true) ;

protected:
    eCredStatus credStatus;

    int thisReadCredentialsFromFile (const char* strFile, gList& outList) ;

private:
    gList contents;

    // Empties
    iCredential (iCredential& ) ;  //empty
    iCredential& operator= (iCredential& ) ; //empty
};
////////////////////////////////////////////////////////////

extern int ipcl_strcmp (const char* str1, const char* str2) ;

extern int ipcl_strcmp_case (const char* str1, const char* str2) ;

extern int ipcl_strncmp (const char* str1, const char* str2, size_t nChars) ;

extern int ipcl_strncmp_case (const char* str1, const char* str2, size_t nChars) ;

extern int ipcl_str_to_int (const char* strValue, int minValue, int maxValue, int errorValue) ;

extern int ipcl_ip_from_hostaddr (const char* strIpOrHost, gIpAddr& ip) ;

extern int ipcl_host_from_ipaddr (gIpAddr& ip, gString& sHost) ;

////////////////////////////////////////////////////////////
#endif //iPOPAUTH_X_H

