#ifndef iWORDUNICODE_X_H
#define iWORDUNICODE_X_H


#include "lib_imedia.h"

////////////////////////////////////////////////////////////

extern int ulang_prepare_custom_iso (int optTable, sIsoUni& data) ;

extern t_uchar* ulang_new_7bit_string (const t_uchar* strIn, const t_uchar** optUcsEqStrs) ;

extern t_uchar* ulang_new_8bit_string (const t_uchar* strIn, const t_uchar** optUcsEqStrs) ;

extern t_uchar* ulang_new_8bit_eqstr (const t_uchar* strIn, t_uint32 uMask, const t_uchar** optUcsEqStrs) ;

////////////////////////////////////////////////////////////
#endif //iWORDUNICODE_X_H

