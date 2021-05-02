// iobjs.h -- gobj library
//	This library is not GPL!
//	See more details at the LICENSE file.

#ifndef iOBJS_X_H
#define iOBJS_X_H

#include <stdlib.h>

#define LIB_VERSION_ILIB_MAJOR	4
#define LIB_VERSION_ILIB_MINOR	3


// Changes on 4.2
//	- cPair is now 'constant pair', better than sPair, for args!
//
// Changes on 4.0:
//	- Wall and Werror, -Wdeprecated (char* as const char*)
//
// Changes on 3.1:
//	- gStorage has ioMask (int)
//
// Changes on 3.4:
//	- added ibase_Prime and _NextPrime


#ifndef false
#define false 0
#endif

#ifndef true
#define true  1
#endif

typedef unsigned char t_uchar;
typedef t_uchar t_uint8;
typedef char t_int8;
typedef unsigned short t_uint16;
typedef short t_int16;

typedef t_uchar t_desc_char;

typedef unsigned long long t_uint64;
typedef long long t_int64;

typedef t_uint16 t_gPort;

typedef t_uint64 t_inode;

#if defined(linux) || defined(iDOS_SPEC)
#define gUINT_IS_32bit
#define MAX_INT_VALUE 2147483647
#else
#error iobjs only supports >=32bit processors.
#endif //normal

#ifdef gUINT_IS_32bit
// 32bit word microprocessor specific
typedef unsigned t_uint32;
typedef int t_int32;
#define MAX_UINT_UL 4294967295UL
#else
#error No 16bit
#endif //iUINT_IS_32bit

typedef t_uint32 t_uint4B;
typedef t_uint16 t_uint2B;

typedef t_uint32 t_gTicElapsed;
typedef t_uint32 t_stamp;  // traditional Unix epoch-time (1970...)


// Detection of included net-services
#ifdef linux
#include <netinet/in.h>
#ifdef IN6_IS_ADDR_V4MAPPED
#define gX_KERN2_4
#else
#define gX_KERN2_0
#endif
#else
typedef t_int32 uid_t;
#endif //~linux


#ifdef SWAP_16bit16
// Not usual: ab|cd => cd|ab
#define gENDLITTLE4(x) ((x<<16) | (x>>16))
#else
// Usual on Intel based OSs: ab|cd => dc|ba
#define gENDLITTLE2(x) ((x<<8) | (x>>8))
#define gENDLITTLE4(x) ( (x<<24) | ((x&0xFF0000)>>8) | ((x&0xFF00)<<8) | ((x&0xFF000000)>>24) )
#endif //SWAP_16bit16

// OS-Specific (or Machine-Specific)
#define TO_UD2(x) ((t_uint16)gENDLITTLE2(x))
#define TO_UD4(x) ((t_uint32)gENDLITTLE4(x))
#define FROM_UD2(x) TO_UD2(x)   // (De-)Coding is symetric
#define FROM_UD4(x) TO_UD4(x)   // (De-)Coding is symetric

#define MAX_INT16_I 32767  // 2^15-1
#define MIN_INT16_I -32768
#define MAX_UINT16_U 65535U  // 2^16-1

#ifdef MIN_INT32
#define MIN_INT32BITS	MIN_INT32
#else
#define MIN_INT32BITS	(~0x7fffffff)  // -2147483648
#endif
#ifdef MAX_INT32
#define MAX_INT32BITS	MAX_INT32
#else
#define MAX_INT32BITS	2147483647
#endif

#define MAX_DLINT32 2147483647L
#define MIN_DLINT32 -2147483647L
#define MAX_DUINT32 2147483646UL

#define MAX_LONG_L 2147483648LL
#define MIN_LONG_L -2147483648LL
#define MAX_U4B_ULL 4294967295ULL

#define MAX_STAMP_UL MAX_DUINT32

#ifdef iDOS_SPEC
#define gEND_CLEANUP_WSA WSACleanup()
#else
#define gEND_CLEANUP_WSA //Nothing needed for non-Win32/DOS
#endif //~iDOS_SPEC

// File macros
#ifdef iDOS_SPEC
#ifndef __S_IFIFO
#define __S_IFIFO       0010000 // FIFO
#endif //__S_IFIFO
#endif //iDOS_SPEC
#define iFILE_S_IFIFO __S_IFIFO
// <end of File macros>

// ---------------------------------------------------
// Singleton´s control
// ---------------------------------------------------
#define gINIT gStorageControl::Self().Init()
#define gINIT_SOCK gStorageControl::Self().StartAll();

#define gEND gStorageControl::Self().End(); gEND_CLEANUP_WSA

// ---------------------------------------------------
// Utilities
// ---------------------------------------------------
#define gMAX(a,b) ((a>b)?a:b)
#define gMIN(a,b) ((a<b)?a:b)

#define gRANGEX(v,a,b,x) ((v<a)?x:(v>b?x:v))
#define gRANGE0(v,a,b) gRANGEX(v,a,b,0)
// e.g.: gRANGE0(v,a,b) ((v<a)?0:(v>b?0:v))

#define ISyORn(x) (x ? 'Y' : 'N')

#ifdef iDOS_SPEC
#define gSLASHSTR "\\"
#define gSLASHCHR '\\'
#define altSLASHSTR "/"
#define altSLASHCHR '\\'
#define gDOS_LIB_CRT_DLL // e.g. getpid on libcrtdll.a
#undef gDOS_LIB_XIO // e.g. mkstemp on libiberty.a
#else
#define gSLASHSTR "/"
#define gSLASHCHR '/'
#define altSLASHSTR "\\"
#define altSLASHCHR '/'
#ifdef linux
#define gGX_SIG_SUPPORT
#define gGX_USR_SUPPORT
	#ifdef FREESCO
		// ZipSlack does not have this no unistd.h
		typedef unsigned int __socklen_t;
		typedef __socklen_t socklen_t;
	#endif
#endif //linux (specific self-defines)
#endif //iDOS_SPEC


#ifdef gX_KERN2_0
// If not regularly use: netinet/in.h:typedef uint32_t in_addr_t;
typedef t_uint32 in_addr_t;
#endif //iX_KERN2_0

#ifdef gX_KERN2_4
;
#endif //iX_KERN2_4


// ---------------------------------------------------
// Debugging
// ---------------------------------------------------

#ifdef DEBUG
#define DBGPRINT(args...) printf(args)
#else
#define DBGPRINT(args...) ;
#endif //DEBUG

#ifdef DEBUG_MIN
#define DBGPRINT_MIN(args...) printf(args)
#else
#define DBGPRINT_MIN(args...) ;
#endif //DEBUG

#ifdef DEBUG_MEA  //Memory all...highly verbose
#define DBGPRINT_MEA(args...) printf(args)
#else
#define DBGPRINT_MEA(args...) ;
#endif //DEBUG

#ifdef DEBUG_LOG
#define DBGPRINT_LOG(args...) printf(args)
#else
#define DBGPRINT_LOG(args...) ;
#endif //DEBUG_LOG

// ---------------------------------------------------
// Assertions
// ---------------------------------------------------

#define ASSERTION_FALSE(y) \
            { \
               /* gStorageControl::Self().assertedNr = -1*/; \
               fprintf(stderr,"ASSERTION[v%d.%d]:%s:%u:%s\n", \
			LIB_VERSION_ILIB_MAJOR, LIB_VERSION_ILIB_MINOR, \
			__FILE__,__LINE__,y); \
               exit(1); \
            }
#define ASSERTION(x,y) \
            { \
            if ( (x)==0 ) { \
               /* gStorageControl::Self().assertedNr = -2*/; \
               fprintf(stderr,"ASSERTION[v%d.%d]:%s:%u:%s\n", \
			LIB_VERSION_ILIB_MAJOR, LIB_VERSION_ILIB_MINOR, \
			__FILE__,__LINE__,y); \
               exit(1); \
            } \
            }

#define ASSERTION_USE(x,y) ASSERTION(x,y)
// ---------------------------------------------------

#ifndef iprint
#define iprint(args...) fprintf(gFileControl::Self().OutputStream( gFileControl::e_SafeStream ),args)
#endif

// bprint is usually for low-level or OS/machine specific debugging:
#ifndef bprint
#define bprint(args...) ;
#endif

// ---------------------------------------------------
#endif //iOBJS_X_H

