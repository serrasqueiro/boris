#ifndef IODECONV_X_H
#define IODECONV_X_H

#include <string.h>

#include "icalendar.h"

#define TM_YEAR_BASE 1900

////////////////////////////////////////////////////////////
class gIOLarge : public gStorage {
public:
    virtual ~gIOLarge () ;

    // Public members
    t_uint64 llVal;

    // Get methods
    virtual bool IsOk () ;

    virtual char* Str () ;

    virtual gStorage* NewObject () ;

    // Set methods
    virtual void Reset () ;

    // Save/etc methods
    virtual t_uchar* ToString (t_uchar* uBuf) ;

    // Show methods
    virtual void Show (bool doShowAll=true) ;

protected:
    gIOLarge (eStorage aKind) ;
    gIOLarge (int refKind, t_uint64 val) ;

    gString sLarge;

private:
    // Operators,empty
    gIOLarge (gIOLarge& ) ; //empty
    gIOLarge& operator= (gIOLarge& ) ; //empty
};

////////////////////////////////////////////////////////////
class gIOUDecimal : public gIOLarge {
public:
    gIOUDecimal (t_uint64 val=0) ;
    virtual ~gIOUDecimal () ;

    // Get methods
    virtual char* ToHexStr (bool showUpper=true, bool showPrefix=true) ;

protected:
    int thisConvertToHexStr (t_uint64 aLlVal, bool showUpper, bool showPrefix, gString& sResult) ;
    int thisConvertUInt32ToHex (t_uint32 aVal, bool showUpper, bool showPrefix, gString& sResult) ;

private:
    // Operators,empty
    gIOUDecimal (gIOUDecimal& ) ; //empty
    gIOUDecimal& operator= (gIOUDecimal& ) ; //empty
};

////////////////////////////////////////////////////////////
class gIO2Powers : public gInt {
public:
    gIO2Powers (int val=0) ;
    virtual ~gIO2Powers () ;

    // Get methods
    virtual int Mods () {
	return sumMods;
    }

    virtual int Log2Power (int val) {
	// Return log2(val):
	return thisCalcLog2Power( val, sumMods );
    }

    virtual int Power2 (int expnt) {
	// Return 2^expnt
	return thisCalcPower2( 2, expnt );
    }

protected:
    int sumMods;

    int thisCalcLog2Power (int val, int& mods) ;

    int thisCalcPower2 (int base, int expnt) ;

private:
    // Operators,empty
    gIO2Powers (gIO2Powers& ) ; //empty
    gIO2Powers& operator= (gIO2Powers& ) ; //empty
};

////////////////////////////////////////////////////////////
class gEIntScale : public gInt {
public:
    gEIntScale (int value=0, int aScale=1) ;
    gEIntScale (gEIntScale& value) ;
    virtual ~gEIntScale () ;

    // Set methods
    virtual bool ConvertFromStr (char* str) ;

    // Operators,etc
    gEIntScale& operator= (gEIntScale& value) ;

protected:
    int scale;

    int thisConvertFromStr (char* str, int& result, int aScale) ;
};

////////////////////////////////////////////////////////////
class gDateString : public gDateTime {
public:
    // Public enums
    enum eLocalOrUTC {
	e_ReferenceUTC,
	e_ReferenceLocal,
	e_ReferenceAuto,
	e_NoReference,
	e_InvalidReference = 255
    };

    enum eDateFormat {
	e_LogLinear,	// YYYY-MM-DD [HH:MM[:SS]]
	e_LinearYYMMDD,	// YYYYMMDD [HH:MM[:SS]]
	e_LogGuess,	// YYYY-MM-DD [HH:MM[:SS]] or MMM [D]D [remaining string]
	e_LogMaster
    };

    gDateString (eDateFormat dateFormat=e_LogLinear) ;  // Now

    gDateString (char* str, eDateFormat dateFormat) ;

    gDateString (gString& sDtTm, eDateFormat dateFormat) ;

    gDateString (gString& sDate) ;

    gDateString (gString& sDate, gString& sTime, eDateFormat dateFormat=e_LogLinear) ;

    gDateString (t_stamp aStamp) ;

    virtual ~gDateString () ;

    // Get methods
    virtual char* DefaultDatePrintFormat () {
	return strDatePrintFormat;
    }

    virtual t_uchar* ToString (t_uchar* uBuf) ;

    virtual eDateFormat KindFormat () {
	return kindFormat;
    }

    virtual gString& RemainingString () {
	return sRemaining;
    }

    virtual t_int8 MonthAbbrevByStr (char* monthAbbrev) {
	thisHashMonthNamesEn();
	return thisMonthAbbrevByStr( monthAbbrev );
    }

    // Set methods
    virtual bool SetDefaultDatePrintFormat (char* s) ;

    virtual bool SetDatePrintFormat (char* s) ;

    virtual bool SetLocalTime (t_stamp aStamp=0) ;

    // Specific methods
    virtual t_stamp GetTimeStamp () ;

    // Operators,etc
    gDateString (gDateString& copy) ;
    gDateString& operator= (gDateString& copy) ;

protected:
    eDateFormat kindFormat;
    t_uint16 usedYear;
    gString sRemaining;
    gString sDatePrintFormat;
    static char* strDatePrintFormat;

    int thisConvertToDateTime (gString& sDate, gString& sTime, eDateFormat dateFormat, gDateString& my) ;
    int thisConvertDtTmToDateTime (gString& sDate, gString& sTime, eDateFormat dateFormat, gDateString& my) ;
    int thisConvertDtTmStrToDateTime (char* str, eDateFormat dateFormat) ;

    int thisConvertToStamp (gDateTime& aDtTm, t_stamp& aStamp) ;

    int thisConvertFromStampUTCorLocal (t_stamp aStamp, eLocalOrUTC localOrUTC) ;

    t_uint16 thisMonthHash (const char* monthAbbrev) ;

    t_int8 thisMonthAbbrevByStr (const char* monthAbbrev) {
	t_int8 iMonth( monthHashed[ thisMonthHash( monthAbbrev ) ] );
	if ( iMonth>=1 && strcmp( defaultMonths[ (unsigned)iMonth % 13 ].abbrev, monthAbbrev )==0 )
	    return iMonth;
	return -1;
    }

private:
    static const short monthHashSize;
    static t_int8 monthHashed[ 256 ];  // see 'enAbbrev' from calendarMonthNames

    int thisHashMonthNamesEn () ;
};

////////////////////////////////////////////////////////////

extern gList* ils_inversed_log_names_from_list (gList& in) ;

extern gList* ils_inversed_log_names_from_str (const char* str, int mask) ;

extern int tp_time_diff (const struct tm *a, const struct tm *b) ;

extern int tp_to_calendar_date (const struct tm *pTM, gDateTime &toTime) ;

extern char* pt_diff_to_rfc822_plus_minus_mm (int diffSecs) ;

////////////////////////////////////////////////////////////
#endif //IODECONV_X_H

