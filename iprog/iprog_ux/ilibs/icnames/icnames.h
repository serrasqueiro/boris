#ifndef iCNAMES_X_H
#define iCNAMES_X_H


#include "ilist.h"

////////////////////////////////////////////////////////////
struct sIcnTlds {
    sIcnTlds ()
	: ptrCurrent( nil ),
	  ptrExtra( nil ),
	  hashedCc( nil ) {
	for (t_uint16 idx=0; idx<26; idx++) {
	    xtraTlds[ idx ] = nil;
	}
    }

    ~sIcnTlds () {
	Release();
    }

    char bufDns[ 512 ];

    gList orderedCcTlds;
    gList allTlds;

    gElem* ptrCurrent;
    gElem* ptrExtra;  // the first xtra domain, e.g. "com"

    gString* hashedCc;  // hashed country-codes (hashedCc[0] contains all letters for Country-codes starting with 'a'
    gString** xtraTlds[ 26 ];  // xtraTlds[ 'c'-'a'=2 ][ 1 ] is "com"

    // Methods
    bool Init () ;

    void Release () ;

    int HashedCountryCode (gString& ccTwoLetter, const char* strOptName) ;

    bool Optimize () {
	return OptimizeTlds();
    }

    bool OptimizeTlds () ;

    gString* DomainXtraFromStr (const char* str) {
	// Requires "Optimize(Tlds)" first
	gString* ptrMe;
	char chr;
	if ( str && (chr = str[ 0 ])>='a' && chr<='z' ) {
	    chr -= 'a';
	    for (int iter=1; (ptrMe = xtraTlds[ (int)chr ][ iter ])!=nil; iter++) {
		if ( ptrMe->Match( (char*)str ) ) return ptrMe;
	    }
	}
	return nil;
    }

    gString* DomainXtraFromString (gString& aStr, bool bestCase=false) {
	// Requires "Optimize(Tlds)" first
	if ( bestCase ) {
	    gString aDown( aStr );
	    aDown.DownString();
	    return DomainXtraFromStr( aDown.Str() );
	}
	return DomainXtraFromStr( aStr.Str() );
    }

    gString* ValidDomain (char chrA, char chrB) {
	if ( chrA>='a' && chrA<='z' ) {
	    if ( chrB>='a' && chrB<='z' ) {
		chrA -= 'a';
		if ( hashedCc[ (int)chrA ].Find( chrB ) )
		    return &hashedCc[ (int)chrA ];
	    }
	}
	return nil;
    }

    gString* AnyDomain (const char* str) {
	gString* ptrMe( nil );

	if ( str && str[ 0 ] ) {
	    ptrMe = DomainXtraFromStr( str );
	    if ( ptrMe ) return ptrMe;

	    // Not found: try country-code (only two letters)
	    if ( str[ 1 ] && str[ 2 ]==0 ) {
		ptrMe = ValidDomain( str[ 0 ], str[ 1 ] );
	    }
	}
	return ptrMe;
    }
};

////////////////////////////////////////////////////////////
// icn functions
////////////////////////////////////////////////////////////

extern sIcnTlds* myIcnTlds;


extern int icn_init () ;
extern void icn_finit () ;

extern char* icn_tld_find_first () ;  // Top-Level-Domain, first
extern char* icn_tld_find_next () ;

extern char* icn_cctld_find_first () ;  // Country-code, Top-Level-Domain, first
extern char* icn_cctld_find_next () ;

extern char* icn_dns_reversed (const char* strDNS, t_uint32 mask) ;

////////////////////////////////////////////////////////////
#endif //iCNAMES_X_H

