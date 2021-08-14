#ifndef G_CNTP_RESPONSE_X_H
#define G_CNTP_RESPONSE_X_H

////////////////////////////////////////////////////////////

#ifdef DEBUG_RESP
#define DEBUG_WAIT_C(msg) { \
		char ch; \
		if ( msg ) printf("%s",msg); \
		do { fscanf(stdin,"%c",&ch); } while ( ch!='c' ); \
		}
#else
#define DEBUG_WAIT_C(msg) ;
#endif //~DEBUG_...
////////////////////////////////////////////////////////////

extern int cntp_BasicVNTP_HandleClient (int handle) ;

////////////////////////////////////////////////////////////
#endif //G_CNTP_RESPONSE_X_H

