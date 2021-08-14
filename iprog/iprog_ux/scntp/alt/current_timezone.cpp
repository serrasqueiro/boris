/* current_timezone.c
 */

#include <ctime>
#include <iomanip>
#include <sstream>
#include <iostream> // show strings (C++)

#ifdef DEBUG
#include <stdio.h>  // show strings
#include "debug.h"
#else
#define dprint(args...) ;
#endif


static constexpr time_t const NULL_TIME = -1;
static char time_str[128];


// returns a friendly time string in global 'time_str'
const char* friendly_time(const struct tm* tm)
{
    // user-friendly date
    if (!tm)
	return NULL;
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S %z (%Z)", tm);
    return time_str;
}

// returns difference in seconds from UTC at given time
// or at current time if not specified
long tz_offset(time_t when = NULL_TIME)
{
    // See	https://linux.die.net/man/3/localtime
    std::ostringstream os;
    if (when == NULL_TIME)
        when = std::time(nullptr);
    auto const tm = *std::localtime(&when);

    //	% date
    //		Sun Oct 18 14:33:34 BST 2020
    //
    // {tm_sec = 34, tm_min = 33, tm_hour = 14, tm_mday = 18, tm_mon = 9,
    //	tm_year = 120, tm_wday = 0, tm_yday = 291, tm_isdst = 1, tm_gmtoff = 3600,
    //	tm_zone = 0x418180 "BST"}
    // glibc has also: tm_zone is char*; int tm.tm_gmtoff for the offset

    os << std::put_time(&tm, "%z");
    friendly_time(&tm);
    std::string s = os.str();

    // s is in ISO 8601 format: "±HHMM"
    int h = std::stoi(s.substr(0,3), nullptr, 10);
    int m = std::stoi(s[0]+s.substr(3), nullptr, 10);
    dprint("Debug: output string '%s'; asctime()='%s'\nctime()='%s'\nstrftime()='%s'\n",
	   s.c_str(),
	   asctime(&tm),
	   ctime(&when),
	   time_str);
    return h * 3600 + m * 60;
}

int main()
{
    std::cout << tz_offset() << "\n" << time_str << "\n" << std::endl;
}
