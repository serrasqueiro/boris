#ifndef iCALENDAR_X_H
#define iCALENDAR_X_H

#include <time.h>  //gmtime/localtime, mktime, (struct tm) ...

#include "istring.h"

////////////////////////////////////////////////////////////
struct sPairExpl {
    char abbrev[ 4 ];
    const char* name;
};
////////////////////////////////////////////////////////////
class gDateTime : public gStorage {
public:
    enum eKindTime {
	e_Now,
	e_UTC
    };

    enum eTodFormat {
	e_YYYYMMDD_opt_HHMMSS,
	e_YYYYMMDD_HHMMSS,
	e_YYYYMMDD,
	e_MonthDay_opt_HHMMSS,
	e_MonthDay_HHMMSS,
	e_YYYYMMDD_opt_HHMM_wORwoSS,
	e_TOD_Invalid
    };

    gDateTime (eKindTime aKind=e_Now) ;

    gDateTime (t_stamp aStamp) ;

    gDateTime (int iYear, int iMonth, int iDay,
	       t_uint8 uHour=0, t_uint8 uMin=0, t_uint8 uSec=0) ;

    virtual ~gDateTime ();

    // Public data-members
    int lastOpError;
    t_int16 year;
    t_uint8 month;
    t_uint8 day;
    t_uint8 hour, minu, sec;
    t_uint8 wday;  //day of the week (0=sunday)
    t_uint16 yday; //day of the year
    t_int8 isdst;  //daylight saving time (-1 on error)

    static int minimumYear;
    static sPairExpl monthNames[ 13 ];
    static sPairExpl weekdayNames[ 7 ];
    static sPairExpl weekdayNames2Letter[ 7 ];

    // Get methods
    virtual bool IsOk () ;

    virtual char* WeekdayString () {
	return (char*)weekdayNames[ wday % 7 ].name;
    }

    virtual char* WeekdayAbbrev () {
	return weekdayNames[ wday % 7 ].abbrev;
    }

    // Set methods
    virtual void Reset () ;

    virtual int SetError (int opError) ;

    virtual bool FromTOD (char* strTOD, eTodFormat todFormat=e_YYYYMMDD_opt_HHMMSS) {
	unsigned pos( 0 );
	// Note!
	//	input string 'strTOD' is re-written
	//
	return FromStringTOD( strTOD, pos, todFormat );
    }

    virtual bool FromStringTOD (char* strTOD, unsigned& pos, eTodFormat todFormat=e_YYYYMMDD_opt_HHMMSS) ;

    // Specific methods
    virtual t_stamp GetTimeStamp () ;

    virtual bool SetTimeStamp (t_stamp aStamp) ;

    t_uint8 DaysOfMonth (int aYear, t_uint8 aMonth) ;

    // Save/etc methods
    virtual gStorage* NewObject () ;
    virtual t_uchar* ToString (const t_uchar* uBuf) ;
    virtual bool SaveGuts (FILE* f) ;
    virtual bool RestoreGuts (FILE* f) ;

protected:
    static const t_uint8 tblCalDurMonth[ 13 ];
    static const sPairExpl defaultMonths[ 13 ];
    static const sPairExpl weekdays3Letter[ 7 ];
    static const sPairExpl weekdays2Letter[ 7 ];

    // Protected methods
    int thisCalCheck () ;
    int thisDateTimeCheck (gDateTime& aDtTm, int minYear) ;
    int thisDateCheck (int aYear, t_uint8 aMonth, t_uint8 aDay, int minYear) ;
    int thisTimeCheck (t_uint8 aHour, t_uint8 aMin, t_uint8 aSec) ;

    int thisConvertNow (eKindTime aKind) ;
    int thisConvertFromStamp (t_stamp aStamp, bool isUTC=true) ;
    int thisConvertToStamp (gDateTime& aDtTm, t_stamp& aStamp) ;
    int thisConvertTo_libc_tm (gDateTime& aDtTm, struct tm* pTM) ;

    virtual int FromTOD (char* strTOD, int len, int optYear, unsigned& pos, eTodFormat todFormat, eTodFormat& resultFormat) ;

private:
    void thisCalInit () ;

    // Operators,empty
    gDateTime (gDateTime& ) ; //empty
    gDateTime& operator= (gDateTime& ) ; //empty
};
////////////////////////////////////////////////////////////
class gTimeStamp : public gUInt {
public:
    gTimeStamp (t_stamp aStamp=0) ;
    gTimeStamp (gDateTime& aDtTm) ;

    virtual ~gTimeStamp () ;

    // Public data-members
    short iValid;

    // Get methods
    virtual bool IsOk () ;
    // .

    // Set methods
    bool SetStamp (t_stamp aStamp) ;

    virtual unsigned SetUInt (unsigned v) ;

private:
    // Operators,empty
    gTimeStamp (gTimeStamp& ) ; //empty
    gTimeStamp& operator= (gTimeStamp& ) ; //empty
};
////////////////////////////////////////////////////////////
#endif //iCALENDAR_X_H

