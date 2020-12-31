#ifndef iARG_X_H
#define iARG_X_H

#include "ilist.h"
#include "icontrol.h"
#include "ifile.h"
#include "istring.h"

#define MAX_REAL_LVAL 2147483648.0
// MAX_LONG_L is 2147483648LL

////////////////////////////////////////////////////////////
// Global enums
// ---------------------------------------------------------
enum eParamConfig {
    e_ParamNotUsed,
    e_ParamUsedSingle,  // e.g.: -z (or --zero)
    e_ParamUsedRepeat,  // e.g.: -v -vv (or -v -v)
    e_ParamDashSimple,  // e.g.: -zero
    e_ParamNonPosix,    // e.g.: z (or zero)
};

////////////////////////////////////////////////////////////
struct cPair {
    int value;
    const char* str;
};

struct sPair {
    int value;
    char* str;
};


struct sFixPair {
    int value;
    const char* str;
};

////////////////////////////////////////////////////////////
class gParam : public gList {
public:
    // Public enums
    enum eParamCriteria {
	e_Normal,
	e_StopSplitOnFirst = 1,
	e_NormalQuoted
    };

    // Constructor,etc
    gParam () ;
    gParam (gString& copy, const char* sParamSplit=nil, eParamCriteria aParamCriteria=e_Normal) ;
    gParam (const char* s, const char* sParamSplit=nil, eParamCriteria aParamCriteria=e_Normal) ;
    virtual ~gParam () ;

    // Public data-members
    gString strParamSplit;
    static bool doAutoTrim;

    // Get methods
    virtual gStorage* GetNewObjectFromFormat (const char* s) ;
    virtual unsigned Find (gString& s, gString& sSubStr, int iCriteria) ;

protected:
    eParamCriteria paramCriteria;

    // Protected methods
    int thisSplit (const char* s, gString& strSplit, gList& resL) ;

private:
    // Operators,empty
    gParam (gParam& ) ; //empty
    gParam& operator= (gParam& ) ; //empty
};
////////////////////////////////////////////////////////////
struct sParamRaw {
    // Struct enums
    enum eParamFollow {
	e_ParamNoVal,
	e_ParamThisArg,
	e_ParamNextArg
    };

    sParamRaw (eParamFollow aParamFollow=e_ParamNoVal) ;

    ~sParamRaw () ;

    // Data-members
    eParamFollow paramFollow;
    gString sufStr;
    gString paramSep; // usually empty or "="
    gList members;
    unsigned posControl;
    gStorage* myParamVal;

    gStorage* GetParamVal () ;
    bool BuildParamVal (const char* s) ;
    bool AddMember (const char* s) ;
};

////////////////////////////////////////////////////////////
class gParamVal : public gControl {
public:
    gParamVal ()
	: gControl( e_StoreExtend ),
	  cVal( 0 ),
	  lVal( MAX_LONG_L ),
	  realVal( MAX_REAL_LVAL ),
	  repeats( -1 ),
	  errorCode( 0 ) {
    }

    virtual ~gParamVal () {
    }

    // Public data-members
    t_uchar cVal;    // e.g. -z
    gString allStr;  // e.g. --zero, if exists
    gString sufStr;  // suffix, e.g. zero
    gString sVal;    // e.g. --zero=__STR__
    long long lVal;  // e.g. --zero=__-123__
    float realVal;
    short repeats;   // e.g. -v -v turns repeats=2
    int errorCode;   // 0:ok; non-0:invalid

    // Get methods
    virtual char* Str (unsigned idx=0) ;

    bool GetChar (t_uchar& chr) {
	if ( cVal==0 ) return false;
	chr = cVal;
	return true;
    }

    long GetLongValue () {
	long val;
	if ( GetLong(val)==false ) return 0;
	return (int)val;
    }

    bool GetLong (long& val) {
	if ( lVal==MAX_LONG_L ) return false;
	val = (long)lVal;
	return true;
    }

    bool GetFloat (float& val) {
	if ( realVal>=MAX_REAL_LVAL ) return false;
	val = realVal;
	return true;
    }

    // Set methods
    bool SetLong (long val) {
	lVal = val;
	realVal = (long)val;
	return true;
    }

    bool SetString (const char* s) {
	lVal = MAX_LONG_L;
	realVal = MAX_REAL_LVAL;
	sVal.Set( s );
	return s!=nil;
    }

    void CopyParam (gParamVal& copy) ;
    virtual bool AddToList (gList& resL) ;
    virtual bool FillParam (gString& newSVal, gStorage* aObj) ;

    // Special methods
    gParamVal* FindObj (gList& copyL, t_uchar chr, unsigned& idx) ;
    unsigned FindMainChar (gList& copyL, t_uchar chr) ;

    // Save/etc methods
    virtual gStorage* NewObject () ;
    virtual t_uchar* ToString (const t_uchar* uBuf) ;

    // Show methods
    virtual void Show (bool doShowAll=true) ;

private:
    // Operators,empty
    gParamVal (gParamVal& ) ; //empty
    gParamVal& operator= (gParamVal& ) ; //empty
};

////////////////////////////////////////////////////////////
class gParamElem : public gList {
public:
    gParamElem (eParamConfig aParamConfig=e_ParamNotUsed) ;

    virtual ~gParamElem () ;

    // Public data-members
    eParamConfig paramConfig;
    t_uchar mainChr;
    sParamRaw sRaw;

    // Get methods
    t_uchar MainChar () {
	return mainChr==0 ? ' ' : mainChr;
    }

    // Set methods
    unsigned Add (gString& copy) ;
    unsigned Add (const char* s) ;

    virtual gStorage* NewObject () ;

private:
    // Operators,empty
    gParamElem (gParamElem& ) ; //empty
    gParamElem& operator= (gParamElem& ) ; //empty
};

////////////////////////////////////////////////////////////
class gArg : public gList {
public:
    gArg (char** argv, char** envp=nil) ;
    virtual ~gArg () ;

    // Public data-members
    gString prog;
    gList env;
    gList param;
    gList opt;    // Flushed options
    gList errors; // Flush errors
    short nParamDashWordSimple;

    // Get methods
    unsigned NumberParams () {
	return param.N();
    }

    char* Program () {
	return programName.Str();
    }

    gParamVal* GetOptionPtr (unsigned idx) ;

    // Set methods
    int AddParams (t_uint16 paramStart, const char* s) ;

    int FlushParams () ;

    bool NoArg () {
	return N()==0;
    }

    bool FindOption (char c) ;
    bool FindOption (const char* s) ;
    bool FindOption (const char* s, int& val) ;
    bool FindOption (const char* s, long& val) ;
    bool FindOption (char c, gString& sRes) ;
    bool FindOption (const char* s, gString& sRes) ;
    bool FindOptionOccurr (const char* s, short& nRepeats) ;
    bool FindOptionOccurr (const char* s, bool& b1) ;
    bool FindOptionOccurr (const char* s, bool& b1, bool& b2) ;

    // Save/etc methods
    virtual t_uchar* ToString (const t_uchar* uBuf) ;
    virtual bool SaveGuts (FILE* f) { return true; }
    virtual bool RestoreGuts (FILE* f) { return true; }

protected:
    gParamElem* internParams;
    unsigned keepPos;

    // Protected methods
    bool thisProgramName (gString& copyName) ;

    bool thisAddOneParam (t_uint16 paramStart, const char* sParam, gList& resL) ;

    bool thisBuildInternParam (gList& paramIn, const char* strSepSplit) ;

    bool thisProcessParamElem (gParam& aParam, gParamElem& paramElem) ;

    bool thisProcessParamElem (gParam& aParam,
			       gParamElem& paramElem,
			       bool& isDashWordSimple) ;

    unsigned thisFindParamFromChr (t_uchar inChr,
				   unsigned nParams,
				   gParamElem* intParams) ;

    unsigned thisFindParamFromStr (const char* s,
				   unsigned nParams,
				   gParamElem* intParams,
				   bool& doMatch,
				   unsigned& possibleParamIdx) ;

    bool thisFillParamFromChr (t_uchar inChr,
			       unsigned nParams,
			       gParamElem* intParams,
			       gParamVal& paramVal) ;

    bool thisFillParamFromStr (const char* s,
			       unsigned paramRef,
			       unsigned nParams,
			       gParamElem* intParams,
			       gParamVal& paramVal) ;

    int thisFlushAll (gList& inputL,
		      gList& paramIn,
		      unsigned nParams,
		      gParamElem* intParams,
		      gList& resArgL,
		      gList& resOptL,
		      gList& resErrL) ;

private:
    eParamConfig paramLetter[256];
    gString programName;

    // Operators,empty
    gArg (gArg& ) ;
    gArg& operator= (gArg& ) ;
};
////////////////////////////////////////////////////////////
#endif //iARG_X_H

