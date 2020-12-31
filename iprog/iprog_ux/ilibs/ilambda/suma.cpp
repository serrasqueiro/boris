// suma.cpp
//
// sumA simple checksum


#include "suma.h"

#include "ioaux.h"

////////////////////////////////////////////////////////////
short sumA_limit_chosen (t_uint64 reqBytes, t_uint64 bSize)
{
 short result( 1 );
 ASSERTION(bSize,"bSize");
 for ( ; reqBytes>4; reqBytes/=4) result++;
 return result;
}


int intern_calc_sumA_stats (int fd,
			    t_uchar* buf,
			    unsigned bufSize,
			    short limitChoose,
			    t_uint64 reqBytes,
			    t_uint64& totalBytes,
			    t_uint16& checksum,
			    unsigned& statBytesRead)
{
 static off_t seekCur;
 ssize_t i, bytes_read( 0 );
 unsigned r, s( 0 ), sPart( 0 ), vPart;
 FILE* fSumProgress( nil );

 bool hasReqBytes( reqBytes>0 && fSumProgress!=nil );

 //int doIoProbe( (globSumA_reqBehMask & SUMA_BEH_MASK_IoProbe)!=0 );
 short fastChoose( limitChoose );

 off_t seekFwd( 0 );
 off_t seekAft( 0 );

 statBytesRead = 0;

 if ( fd<0 ) return -1;

 DBGPRINT("DBG: calc_sumA: reqBytes=%llu vs %llu, limitChoose: %d\n",
	  reqBytes,
	  totalBytes,
	  (int)limitChoose);

 do {
     fastChoose++;
     totalBytes += (t_uint64)bytes_read;
     DBGPRINT("DBG: did read: %d [%d/%d]%s\n",
	      (int)bytes_read,
	      fastChoose,
	      limitChoose,
	      fastChoose<=limitChoose ? "\0" : " (calc)");

     seekCur = gns_lseek( fd, 0, SEEK_CUR );
     if ( fastChoose<=limitChoose && seekCur+(off_t)bufSize<(off_t)reqBytes ) {
	 seekFwd = gns_lseek( fd, bufSize, SEEK_CUR );
	 seekAft = gns_lseek( fd, 0, SEEK_CUR );
	 DBGPRINT("DBG: seekCur=%ld,seekFwd=%ld,seekAft=%ld-%u (diff=%ld)\n",
		  seekCur,
		  seekFwd,
		  seekAft,
		  bufSize,
		  seekAft-bufSize-seekCur);
	 ASSERTION(seekAft>=seekFwd,"seekAft>=seekFwd");
	 continue;
     }

     bytes_read = read( fd, buf, bufSize );
     if ( bytes_read<=0 ) break;

     statBytesRead += (unsigned)bytes_read;  // Just stats

     if ( hasReqBytes ) {
#ifdef gDOS_SPEC
	 // DevC++ (at least 4.9.9.2!) has a bug with several %llu (long long):
	 fprintf(fSumProgress,"\r%u, ",statBytesRead);
	 fprintf(fSumProgress,"%llu/",totalBytes);
	 fprintf(fSumProgress,"%llu ",reqBytes);
	 fprintf(fSumProgress,"%llu%%",(t_uint64)totalBytes*100ULL / reqBytes);
#else
	 fprintf(fSumProgress,"\r%u, %llu/%llu %llu%%",
		 statBytesRead,
		 totalBytes,
		 reqBytes,
		 (t_uint64)totalBytes*100ULL / reqBytes);
#endif
     }

     fastChoose = 0;
     for (i=0, vPart=1; i<bytes_read; i++) {
	 sPart = buf[ i ];
	 s += (sPart ^ vPart);
	 vPart <<= 1;
	 if ( vPart>256 ) vPart = 1;
     }
 } while ( totalBytes < reqBytes );

 r = (s & 0xffff) + ((s & 0xffffffff) >> 16);
 checksum = (t_uint16)((r & 0xffff) + (r >> 16));

 // Question: should we avoid checksum being zero?
 //	=> if ( checksum==0 ) checksum = 0xffff	<= not used
 // No, and not in the future.

 int bogusCode( bytes_read<0 && totalBytes<reqBytes );

 if ( hasReqBytes ) {
     fprintf(fSumProgress,"%s\n",bogusCode ? " <- Error" : ".");
 }

 DBGPRINT("DBG: calc_sumA: %d reqBytes=%llu totalBytes effectively read=%llu\n",
	  bogusCode,
	  reqBytes,
	  totalBytes);

 // If there was an error, 1 is returned!
 return bogusCode;
}


int calc_sumA_stats (int fd, t_uchar* buf, unsigned bufSize, t_uint64 reqBytes, t_uint64& totalBytes, t_uint16& checksum,
		     unsigned& statBytesRead)
{
 short limitChoose( sumA_limit_chosen( reqBytes, (t_uint64)bufSize ) );
 int result( intern_calc_sumA_stats( fd,
				     buf,
				     bufSize,
				     limitChoose,
				     reqBytes,
				     totalBytes,
				     checksum,
				     statBytesRead ) );
 return result;
}

////////////////////////////////////////////////////////////
int file_sumA (const char* strFile, int optUnused, t_uint16& checksum)
{
 static t_uchar buf[ 8192 ];
 t_uint64 reqBytes( 0 );
 t_uint64 totalBytes( 0 );
 off_t seeked;
 int result( -1 );
 int fd( gns_open_readonly( (char*)strFile ) );

 if ( fd<0 ) return -1;

 seeked = gio_file_seek( fd, 0, SEEK_END );
 if ( seeked>=0 ) {
     gio_file_seek( fd, 0, SEEK_SET );
     reqBytes = (t_uint64)seeked;
     result = calc_sumA( fd, buf, sizeof(buf), reqBytes, totalBytes, checksum );
 }

 close( fd );
 return result;
}


int calc_sumA (int fd, t_uchar* buf, unsigned bufSize, t_uint64 reqBytes, t_uint64& totalBytes, t_uint16& checksum)
{
 unsigned statBytesRead( 0 );
 int error(
     calc_sumA_stats( fd,
		      buf,
		      bufSize,
		      reqBytes,
		      totalBytes,
		      checksum,
		      statBytesRead ) );
 return error;
}

////////////////////////////////////////////////////////////

