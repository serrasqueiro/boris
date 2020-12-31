// iarg.cpp

#include <stdlib.h>
#include <string.h>

#include "iarg.h"
#include "istring.h"
////////////////////////////////////////////////////////////
// Static members
// ---------------------------------------------------------
bool gParam::doAutoTrim=true;

////////////////////////////////////////////////////////////
gParam::gParam ()
    : paramCriteria( gParam::e_Normal )
{
}


gParam::gParam (gString& copy, const char* sParamSplit, eParamCriteria aParamCriteria)
    : paramCriteria( aParamCriteria )
{
 if ( sParamSplit!=nil && sParamSplit[ 0 ]!=0 ) {
     strParamSplit.Set( sParamSplit );
     thisSplit( copy.Str(), strParamSplit, *this );
 }
 else {
     Add( copy );
 }
}


gParam::gParam (const char* s, const char* sParamSplit, eParamCriteria aParamCriteria)
    : paramCriteria( aParamCriteria )
{
 const char* aStr( s ? s : "\0" );
 if ( sParamSplit!=nil && sParamSplit[ 0 ]!=0 ) {
     strParamSplit.Set( sParamSplit );
     thisSplit( aStr, strParamSplit, *this );
 }
 else {
     if ( aStr[ 0 ] ) {
	 Add( (t_uchar*)aStr );
     }
 }
}


gParam::~gParam ()
{
}


gStorage* gParam::GetNewObjectFromFormat (const char* s)
{
 unsigned i, n, pos, nFmts( 0 );
 gList tempL;
 gList fmtL;
 gStorage* res( nil );

 if ( s==nil || s[0]==0 ) return nil;
 strParamSplit.Set( "%" );
 if ( thisSplit( s, strParamSplit, tempL )<1 ) return nil;

 n = tempL.N();
 ASSERTION(n>=2,"n>=2");  //Probably 'AddParams' used with wrong strings
 pos = tempL.Match( "" );
 for (i=pos+1; i<=n; i++) {
     nFmts++;
     fmtL.Add( tempL.Str(i) );
 }
 if ( nFmts==0 ) return nil;

 gString sFmt( fmtL.Str(1) );
 ASSERTION(sFmt.IsEmpty()==false,"sFmt.IsEmpty()==false");
 ASSERTION(nFmts==1,"nFmts==1");
 t_uchar aChr( sFmt[ 1 ] );

 switch ( aChr ) {
 case 'u':
     res = new gUInt;
     break;
 case 'd':
     res = new gInt;
     break;
 case 's':
     res = new gString;
     break;
 case 'f':
     res = new gReal;
     break;
 default:
     break;
 }
 return res;
}


unsigned gParam::Find (gString& s, gString& sSubStr, int iCriteria)
{
 if ( (eParamCriteria)iCriteria!=e_NormalQuoted ) return s.Find( sSubStr );
 gString sTemp( s );
 short quoteCount=0;
 short singlCount=0; //single-quote
 unsigned idx, len=sTemp.Length();
 for (idx=1; idx<=len; idx++) {
     if ( sTemp[idx]=='"' && singlCount==0 ) {
	 quoteCount = quoteCount==0;
	 continue;
     }
     else {
	 if ( sTemp[idx]=='\'' && quoteCount==0 ) {
	     singlCount = singlCount==0;
	     continue;
	 }
     }
     if ( quoteCount || singlCount ) sTemp[idx] = (t_uchar)127;
 }
 // A string like "abc' def" will not count the single-quote char!
 return sTemp.Find( sSubStr );
}


int gParam::thisSplit (const char* s, gString& strSplit, gList& resL)
{
 // E.g.: split ("abc|def x|yz"," "), or ("@abc@def","@")
 //     results in  ==>  abc|def, x|yz
 // Returns number of splits

 // Note:
 //   paramCriteria=e_StopSplitOnFirst means only the first split is done

 int result( 0 );
 unsigned countSplits( 0 );
 unsigned pos, splitLen( strSplit.Length() );
 gString aS( (char*)s );

 ASSERTION(splitLen>0,"splitLen>0");

 while ( (pos = Find(aS,strSplit,(int)paramCriteria))>0 ) {
     gString bS( aS );
     countSplits++;
     if ( pos==1 ) {
	 if ( paramCriteria!=e_NormalQuoted )
	     resL.Add( (t_uchar*)"\0" );
     }
     else {
	 aS[pos] = 0;
	 if ( doAutoTrim ) aS.Trim();
	 resL.Add( aS );
	 result++;
     }
     bS.Delete(1,pos+splitLen-1);
     aS.Copy( bS );
     //printf("DBG: SPLIT:[%s]\n",aS.Str());
     if ( paramCriteria==gParam::e_StopSplitOnFirst && countSplits>=(unsigned)paramCriteria ) break;
 }
 if ( aS.Length()==0 ) return result;
 result++;
 if ( doAutoTrim ) aS.Trim();
 resL.Add( aS );
 return result;
}
////////////////////////////////////////////////////////////
sParamRaw::sParamRaw (eParamFollow aParamFollow)
    : paramFollow( aParamFollow ),
      posControl( 0 ),
      myParamVal( nil )
{
}


sParamRaw::~sParamRaw ()
{
 delete myParamVal;
}


gStorage* sParamRaw::GetParamVal ()
{
 ASSERTION(myParamVal!=nil,"myParamVal!=nil");
 return myParamVal;
}


bool sParamRaw::BuildParamVal (const char* s)
{
 bool isOk;
 gParam param;

 delete myParamVal;
 myParamVal = param.GetNewObjectFromFormat( s );
 isOk = myParamVal!=nil;
#ifdef gDEBUG_PARAM
 if ( isOk ) {
     gUCharBuffer ucBuf;
     myParamVal->ToString( ucBuf.uBuf );
     printf("DBG: PARAM_VAL: [%s]=[",ucBuf.uBuf);
     myParamVal->Show(); printf("]\n");
 }
 else {
     printf("DBG: PARAM_VAL invalid: %s\n",s);
 }
#endif //iDEBUG_PARAM
 ASSERTION(isOk,"Invalid parameter format");
 return isOk;
}


bool sParamRaw::AddMember (const char* s)
{
 gString sMember( s );
 unsigned pos = sMember.FindExcept( "-" );
 members.Add( s );
 if ( pos>posControl ) {
     sufStr = (char*)(s+pos-1);
     posControl = pos;
 }
 return pos>0;
}
////////////////////////////////////////////////////////////
void gParamVal::CopyParam (gParamVal& copy)
{
 cVal = copy.cVal;
 allStr = copy.allStr;
 sufStr = copy.sufStr;
 sVal = copy.sVal;
 lVal = copy.lVal;
 realVal = copy.realVal;
 repeats = copy.repeats;
 errorCode = copy.errorCode;
}


bool gParamVal::FillParam (gString& newSVal, gStorage* aObj)
{
 // Given an object (aObj, even not filled), try to convert
 // string 'newSVal' into respective field.

 char* s;
 char* sDup;
 char** endptr;
 char c;
 long val;

 if ( aObj==nil ) return false;

 sVal = newSVal;
 s = sVal.Str();
 c = s[0];
 errorCode = 0;
 eStorage thisKind = aObj->Kind();
 switch ( thisKind ) {
 case e_UInt:
 case e_SInt:
     val = atol(s);
     if ( val==0 && c!='0' ) errorCode = 2;
     SetLong( val );
     DBGPRINT("DBG: Value(%s)=%ld, errorCode=%d (thisKind=%d)\n",s,val,errorCode,thisKind);
     break;
 case e_ULongInt:
 case e_SLongInt:
 case e_Real:
     sDup = strdup( s );
     endptr = &sDup;
     realVal = (float)strtod( s, endptr );
     lVal = (long long)realVal;
     c = sDup[0];
     errorCode = c==0 ? 0 : 3;
     break;
 default:
     errorCode = 1;
     break;
 }

 return errorCode==0;
}


char* gParamVal::Str (unsigned idx)
{
 ASSERTION(idx==0,"idx==0");
 return sufStr.Str();
}


bool gParamVal::AddToList (gList& resL)
{
 gParamVal* newObj;
 newObj = new gParamVal;
 ASSERTION(newObj!=nil,"newObj!=nil");
 newObj->CopyParam( *this );
 return resL.AppendObject( newObj );
}


gParamVal* gParamVal::FindObj (gList& copyL, t_uchar chr, unsigned& idx)
{
 // Returns the pointer of the list with 'cVal' equal to chr.
 // 'idx' gets the index in the list (or 0 if not found).
 unsigned i, n( copyL.N() );
 gStorage* res;

 idx = 0;
 for (i=1; i<=n; i++) {
     res = copyL.GetElementPtr( i )->me;
     ASSERTION(res!=nil,"res!=nil");
     gParamVal* aParamPtr = (gParamVal*)res;
     if ( chr==aParamPtr->cVal ) {
	 idx = i;
	 return aParamPtr;
     }
 }
 return nil;
}


unsigned gParamVal::FindMainChar (gList& copyL, t_uchar chr)
{
 // Finds chr as the main 'cVal' within list 'copyL'.
 // Returns 0 if no chr was found.

 unsigned idx;
 FindObj( copyL, chr, idx );
 return idx;
}


gStorage* gParamVal::NewObject ()
{
 gParamVal* a;
 a = new gParamVal;
 a->CopyParam( *this );
 return a;
}


t_uchar* gParamVal::ToString (const t_uchar* uBuf)
{
 return sufStr.UStr();
}


void gParamVal::Show (bool doShowAll)
{
 if ( realVal>=MAX_REAL_LVAL ) {
     iprint("cVal=%c, allStr=%s, sufStr=%s, sVal=%s, lVal=%ld, repeats=%d, err=%d%s",
	    cVal,
	    allStr.Str(),
	    sufStr.Str(),
	    sVal.Str(),
	    GetLongValue(),
	    repeats,
	    errorCode,
	    doShowAll?"@":"\0");
 }
 else {
     iprint("cVal=%c, allStr=%s, sufStr=%s, sVal=%s, rVal=%0.1f, repeats=%d, err=%d%s",
	    cVal,
	    allStr.Str(),
	    sufStr.Str(),
	    sVal.Str(),
	    realVal,
	    repeats,
	    errorCode,
	    doShowAll?"@":"\0");
 }
}
////////////////////////////////////////////////////////////
gParamElem::gParamElem (eParamConfig aParamConfig)
    : paramConfig(aParamConfig),
      mainChr(0)
{
}


gParamElem::~gParamElem ()
{
}


unsigned gParamElem::Add (gString& copy)
{
 return Add( copy.Str() );
}


unsigned gParamElem::Add (const char* s)
{
 ASSERTION(s!=nil,"s!=nil");
 gList::Add( s );

 gString sPreStr( s[0]!='-' ? (char*)"-" : (char*)"\0" );
 sPreStr.Add( s );

 gParam paramTry1( sPreStr, ":" );
 unsigned n1( paramTry1.N() );

 gParam paramTry2( sPreStr, "=" );
 unsigned n2( paramTry2.N() );

 if ( n1>=2 ) {
     sRaw.paramFollow = sParamRaw::e_ParamNextArg;
     sRaw.AddMember( paramTry1.Str(1) );
     sRaw.BuildParamVal( paramTry1.Str(2) );
 }
 else {
     if ( n2>=2 ) {
	 sRaw.paramFollow = sParamRaw::e_ParamThisArg;
	 sRaw.paramSep = (char*)"=";
	 sRaw.AddMember( paramTry2.Str(1) );
	 sRaw.BuildParamVal( paramTry2.Str(2) );
     }
     else {
	 sRaw.AddMember( sPreStr.Str() );
     }
 }
 return 0;
}


gStorage* gParamElem::NewObject ()
{
 ASSERTION_FALSE("Not needed");
 return nil;
}

////////////////////////////////////////////////////////////
gArg::gArg (char** argv, char** envp)
    : nParamDashWordSimple( 0 ),
      internParams( nil ),
      keepPos( 0 )
{
 char* strArg;

 if ( argv ) {
     prog.Set( *argv );
     for (argv++; (strArg = *argv)!=nil; argv++) {
	 Add( (t_uchar*)strArg );
     }
     if ( envp!=nil ) {
	 for ( ; *envp; envp++) {
	     env.Add( (t_uchar*)*envp );
	 }
     }
     thisProgramName( prog );
 }
 for (short i=0; i<256; i++) {
     paramLetter[i] = e_ParamNotUsed;
 }
}


gArg::~gArg ()
{
 delete[] internParams;
}


gParamVal* gArg::GetOptionPtr (unsigned idx)
{
 unsigned n=opt.N();
 ASSERTION(idx>=1 && idx<=n,"GetOptionPtr");
 return (gParamVal*)opt.GetObjectPtr( idx );
}


int gArg::AddParams (t_uint16 paramStart, const char* s)
{
 bool isOk;
 int result=0;
 unsigned i, n;

 ASSERTION(s!=nil,"s!=nil");

 gParam params(s," ");
 for (i=1, n=params.N(); i<=n; i++) {
     isOk = thisAddOneParam( paramStart++, params.Str(i), param );
     result += isOk==true;
 }
 return result;
}


int gArg::FlushParams ()
{
 bool isOk;
 int result;

 // Builds the internParams array
 isOk = thisBuildInternParam( param, "|" );
 if ( isOk==false ) return -1;

 gList resArgL;
 gList resOptL;
 gList resErrL;

 result = thisFlushAll( *this,
			param,
			NumberParams(),
			internParams,
			resArgL,
			resOptL,
			resErrL );

 DBGPRINT("DBG: FlushParams(result=%d)\n",result);

 CopyList( resArgL );
 opt.CopyList( resOptL );
 errors.CopyList( resErrL );

 return result;
}


bool gArg::FindOption (char c)
{
 gParamVal* aParamPtr;
 unsigned i, n( opt.N() );
 t_uchar chr=(t_uchar)c;

 keepPos = 0;
 if ( paramLetter[chr]==e_ParamNotUsed ) return false;
 // Find 'chr' in 'opt'
 for (i=1; i<=n; i++) {
     aParamPtr = GetOptionPtr( i );
     if ( chr==aParamPtr->cVal ) {
	 keepPos = i;
	 return true;
     }
 }
 return false;
}


bool gArg::FindOption (const char* s)
{
 long val;
 return FindOption( s, val );
}


bool gArg::FindOption (const char* s, int& val)
{
 long lVal;
 if ( FindOption( s, lVal )==false ) return false;
 if ( sizeof(int)!=sizeof(long) ) {
     if ( lVal>65535L ) return false;
 }
 val = (int)lVal;
 return true;
}


bool gArg::FindOption (const char* s, long& val)
{
 gParamVal* aParamPtr;

 keepPos = opt.Match( s );

 ////opt.Show(); printf("< opts.\n\n");
 ////DBGPRINT("DBG: keepPos of (%s): %u\n",s,keepPos);

 if ( keepPos==0 ) return false;
 aParamPtr = GetOptionPtr( keepPos );
 val = (long)(aParamPtr->lVal);
 return true;
}


bool gArg::FindOption (char c, gString& sRes)
{
 gParamVal* aParamPtr;
 if ( FindOption( c )==false ) return false;
 ASSERTION(keepPos>0,"keepPos>0");
 aParamPtr = GetOptionPtr( keepPos );
 sRes = aParamPtr->sVal;
 return true;
}


bool gArg::FindOption (const char* s, gString& sRes)
{
 gParamVal* aParamPtr;
 if ( FindOption( s )==false ) return false;
 aParamPtr = GetOptionPtr( keepPos );
 sRes = aParamPtr->sVal;
 return true;
}


bool gArg::FindOptionOccurr (const char* s, short& nRepeats)
{
 gParamVal* aParamPtr;
 if ( FindOption( s )==false ) return false;
 aParamPtr = (gParamVal*)opt.GetObjectPtr( keepPos );
 nRepeats = aParamPtr->repeats;
 return true;
}


bool gArg::FindOptionOccurr (const char* s, bool& b1)
{
 bool b2;
 return FindOptionOccurr( s, b1, b2 );
}


bool gArg::FindOptionOccurr (const char* s, bool& b1, bool& b2)
{
 short nRepeats;
 b1 = b2 = false;
 if ( FindOptionOccurr( s, nRepeats )==false ) return false;
 if ( nRepeats<=0 ) return false;
 b1 = nRepeats>=1;
 b2 = nRepeats>=2;
 return true;
}


t_uchar* gArg::ToString (const t_uchar* uBuf)
{
 return gList::ToString( uBuf );
}


bool gArg::thisProgramName (gString& copyName)
{
 unsigned pos( copyName.FindBack( gSLASHCHR ) );
 programName.CopyFromTo( prog, pos==0 ? 0 : pos+1 );
 gFileControl::Self().sProgramName = programName;
 return programName.IsEmpty()==false;
}


bool gArg::thisAddOneParam (t_uint16 paramStart, const char* sParam, gList& resL)
{
 ASSERTION(paramStart>0,"paramStart>0");
 ASSERTION(sParam!=nil,"sParam!=nil");
 if ( sParam[0]==0 ) return false;

 resL.Add( sParam );
 return true;
}


bool gArg::thisBuildInternParam (gList& paramIn, const char* strSepSplit)
{
 // Builds the internParams array separated from, e.g. pipe (|)
 bool isOk;
 bool isDashWordSimple;
 unsigned i, n=paramIn.N();

 ASSERTION(strSepSplit!=nil && strSepSplit[0]!=0,"strSepSplit!=nil");

 delete[] internParams;
 internParams = new gParamElem[n+1];

 for (i=1; i<=n; i++) {
     gParam aParam( paramIn.Str(i), strSepSplit );
     isOk = thisProcessParamElem( aParam, internParams[i], isDashWordSimple );
     ASSERTION(isOk,"gArg::thisBuildInternParam(1)");
     nParamDashWordSimple += (short)isDashWordSimple;
 }
 return true;
}


bool gArg::thisProcessParamElem (gParam& aParam,
				 gParamElem& paramElem,
				 bool& isDashWordSimple)
{
 bool isSingleDash, isRepeatedChr;
 unsigned i, paramN, pos, len;
 t_uchar chr, chr2=0, chrMain=0;
 eParamConfig paramConfig=e_ParamNotUsed;
 gList posNonDash;

 isDashWordSimple = false;

 paramN = aParam.N();
 if ( paramN==0 ) return false;

 //printf("DBG: param(%u): ",paramN); aParam.Show();

 // Check which style has been used. Examples:
 //      ==> -z|--zero   Single
 //      ==> -v|-vv      Repeat
 // in parallel, "-z|--zero" ...

 gString sFirst( aParam.Str(1) );
 chr = sFirst[1];
 len = sFirst.Length();
 if ( chr=='-' ) {
     isSingleDash = sFirst.Match("-");
     ASSERTION(sFirst.Match("--")==false,"sFirst.Match('--')==false");
     // Check all params have dash (-)
     for (i=1; i<=paramN; i++) {
	 gString sTemp( aParam.Str(i) );
	 posNonDash.Add( sTemp.FindExcept( "-" ) );
	 if ( sTemp[1]!='-' ) return false;
     }
     chr2 = sFirst[2];
     chrMain = chr2=='-' ? 0 : chr2;
     isRepeatedChr = chr2==sFirst[3];
     isDashWordSimple = isSingleDash==false && isRepeatedChr==false && chr2!='-' && len>2 && paramN==1;
     if ( isDashWordSimple ) {
	 paramConfig = e_ParamDashSimple;
	 chrMain = '~';
     }
     else {
	 // If more than one occurrence for character, return&fail
	 if ( chrMain!=0 && paramLetter[chrMain]!=e_ParamNotUsed ) return false;
	 paramConfig = e_ParamUsedSingle;
	 if ( isSingleDash ) {
	     chrMain = '-';
	 }
	 else {
	     if ( isRepeatedChr ) paramConfig = e_ParamUsedRepeat;
	 }
	 if ( chrMain!=0 ) paramLetter[chrMain] = paramConfig;
     }
 }
 else {
     if ( chr<=' ' ) return false;
     // Check no params have dash
     for (i=2; i<=paramN; i++) {
	 if ( aParam.Str(i)[0]=='-' ) return false;
     }
     paramLetter[0] = paramConfig = e_ParamNonPosix;
 }

 paramElem.paramConfig = paramConfig;
 paramElem.mainChr = chrMain;
 for (i=1; i<=paramN; i++) {
     gString thisParamStr( aParam.Str(i) );
     pos = posNonDash.GetListUInt( i );
     switch ( paramConfig ) {
     case e_ParamNonPosix:
	 paramElem.Add( thisParamStr );
	 break;
     case e_ParamUsedSingle:
     case e_ParamUsedRepeat:
	 if ( pos==2 ) pos=1; else pos=0;
	 paramElem.Add( thisParamStr.Str()+pos );
	 break;
     case e_ParamDashSimple:
	 ASSERTION(paramN==1,"paramN==1");
	 ASSERTION(pos=1,"pos==1");
	 paramElem.Add( thisParamStr );
	 break;
     case e_ParamNotUsed:
     default:
	 return false;
     }
 }
 return true;
}


unsigned gArg::thisFindParamFromChr (t_uchar inChr,
				     unsigned nParams,
				     gParamElem* intParams)
{
 unsigned i;
 t_uchar chr;

 if ( nParams==0 ) return false;
 ASSERTION(inChr!=0,"inChr!=0");
 for (i=1; i<=nParams; i++) {
     chr = intParams[i].MainChar();
     if ( chr==inChr ) return i;
 }
 return 0;
}


unsigned gArg::thisFindParamFromStr (const char* s,
				     unsigned nParams,
				     gParamElem* intParams,
				     bool& doMatch,
				     unsigned& possibleParamIdx)
{
 bool isThisParam;
 unsigned i, k, n, pos, paramRef, paramIdx;
 gList posL, posK;
 gString sArg( s );

 doMatch = false;
 possibleParamIdx = 0;

 if ( nParams==0 ) return false;
 ASSERTION(s!=nil,"s!=nil");
 for (i=1; i<=nParams; i++) {
     pos = intParams[i].sRaw.members.Match( s );
     if ( pos>0 ) {
	 doMatch = true;
	 return i;
     }
     for (k=1; k<=intParams[i].sRaw.members.N(); k++) {
	 gString paramStr( intParams[i].sRaw.members.Str(k) );
	 pos = sArg.Find( paramStr );
	 if ( pos==1 && paramStr.Length()>2 ) {
	     posL.Add( i );
	     posK.Add( k );
	 }
     }
 }

 // Iterate through posL
 n = posL.N();
 for (i=1; i<=n; i++) {
     paramRef = posL.GetListUInt( i );
     isThisParam = intParams[paramRef].sRaw.paramFollow==sParamRaw::e_ParamThisArg;
     if ( isThisParam==false ) continue;
     gString aParamSep( intParams[paramRef].sRaw.paramSep );
     // Add separator to option (e.g. "--zero=")
     // and check "--zero=" is first position within sArg
     paramIdx = posK.GetListUInt( i );
     gString sTemp( intParams[paramRef].sRaw.members.Str( paramIdx ) );
     sTemp.Add( aParamSep.Str() );
     pos = sArg.Find( sTemp );
     if ( pos>0 ) {
	 possibleParamIdx = paramIdx;
	 return paramRef;
     }
 }
 return 0;
}


bool gArg::thisFillParamFromChr (t_uchar inChr,
				 unsigned nParams,
				 gParamElem* intParams,
				 gParamVal& paramVal)
{
 unsigned paramRef, pos;

 paramRef = thisFindParamFromChr( inChr, nParams, intParams );
 if ( paramRef==0 ) return false;

 paramVal.cVal = inChr;
 pos = intParams[paramRef].sRaw.members.Find( "--", 1, e_FindExactPosition );
 if ( pos>0 ) paramVal.allStr = intParams[paramRef].sRaw.members.Str( pos );
 paramVal.sufStr = intParams[paramRef].sRaw.sufStr;
 paramVal.repeats = intParams[paramRef].paramConfig==e_ParamUsedRepeat ? 1 : -1;
 return true;
}


bool gArg::thisFillParamFromStr (const char* s,
				 unsigned paramRef,
				 unsigned nParams,
				 gParamElem* intParams,
				 gParamVal& paramVal)
{
 unsigned pos;

 if ( paramRef==0 ) return false;

 pos = intParams[paramRef].sRaw.members.Find( "--", 1, e_FindExactPosition );
 if ( pos>0 ) paramVal.allStr = intParams[paramRef].sRaw.members.Str( pos );
 paramVal.cVal = intParams[paramRef].MainChar();
 paramVal.sufStr = intParams[paramRef].sRaw.sufStr;
 paramVal.repeats = intParams[paramRef].paramConfig==e_ParamUsedRepeat ? 1 : -1;
 return true;
}


int gArg::thisFlushAll (gList& inputL,
			gList& paramIn,
			unsigned nParams,
			gParamElem* intParams,
			gList& resArgL,
			gList& resOptL,
			gList& resErrL)
{
 // Parses arguments found in 'inputL' and computes 'resArgL','resOptL'
 // Expands POSIX options found in 'inputL' (e.g. -vh into -v -h)
 // into resOptL.
 // Remaining relevant arguments are stored at resArgL.
 //
 // Returns the internal code of error (0 on no error)

 bool isOk, isSingleDash, hasSingleDash;
 bool doExpandPOSIX = nParamDashWordSimple<=0;
 unsigned i, iterChar, n, pos, posExc, len, paramRef;
 unsigned inputSize = inputL.N();
 t_uchar chr;
 eParamConfig paramConfig;
 short* paramUsage;
 int countValErrors=0;
 int result=0;

 if ( nParams==0 ) return 0;

 paramUsage = new short[inputSize+1];

 for (i=1; i<=inputSize; i++) paramUsage[i] = 0;

 // First step is to find POSIX expansions
 gList dashPos;
 pos = inputL.FindAny( "-", 1, e_FindExactPosition, dashPos );
 n = dashPos.N();

 for (i=1; i<=n; i++) {
     pos = dashPos.GetListUInt(i);
     paramUsage[pos] = 1;
     gString sTemp( inputL.Str(pos) );
     gString sArg( sTemp );

     DBGPRINT_MIN("DBG: sArg[%u/%u], pos=%u: <%s>\n",i,n,pos,sArg.Str());
     posExc = sTemp.FindExcept( "-" );
     isSingleDash = sTemp.Match( "-" );
     if ( isSingleDash ) {
	 if ( paramLetter[(int)'-']==e_ParamNotUsed ) continue;
	 resOptL.Add( sTemp );
	 continue;
     }
     ASSERTION(posExc>0,"posExc>0");
     hasSingleDash = posExc==2;
     sTemp.Delete( 1, posExc-1 );
     len = sTemp.Length();

     // If has a single dash (e.g. -z or -zero) => hasSingleDash=true
     // we have to look for the word, or any of the expanded POSIX options.
     // Exception is doExpandPOSIX==false, where no expansion will be made.
     if ( hasSingleDash==true && doExpandPOSIX==true ) {
	 bool areAllChrsAllowed=true;
	 bool isRelaxedThrough=false;

	 // Validate all combinations, e.g. -vh,
	 // both -v and -h must be parameters allowed.
	 for (iterChar=1; iterChar<=len; iterChar++) {
	     chr = sTemp[iterChar];
	     paramConfig = paramLetter[chr];
	     isOk = paramConfig!=e_ParamNotUsed;
	     // Ok, unless at least one of the params has values: [(X Y)] or [(X=Y)]
	     paramRef = thisFindParamFromChr( chr, nParams, intParams );
	     if ( paramRef>0 ) {
		 ASSERTION(isOk,"isOk");
		 isRelaxedThrough = iterChar==1 && intParams[paramRef].sRaw.paramFollow==sParamRaw::e_ParamThisArg;
		 // Cannot have any follow parameters if more than one
		 // parameter is in the -string.
		 // Example: "-v -s:%u" and we have -vs
		 isOk =
		     intParams[paramRef].sRaw.paramFollow==sParamRaw::e_ParamNoVal ||
		     len==1 ||
		     isRelaxedThrough;
		 DBGPRINT("DBG: stringlen=%u, thisFindParamFromChr('%c',...):%u:isOk?%c\n",len,chr,paramRef,ISyORn(isOk));
	     }
	     if ( isOk==false && isRelaxedThrough==false ) areAllChrsAllowed = false;
	 }
	 if ( areAllChrsAllowed==false ) {
	     resArgL.Add( sArg );
	     resErrL.Add( sArg );
	     result++;
	     continue;
	 }

	 // We know all of them are to be expanded
	 for (iterChar=1; iterChar<=len; iterChar++) {
	     bool isRepeatable;
	     chr = sTemp[iterChar];

	     gParamVal paramVal;
	     paramRef = thisFindParamFromChr( chr, nParams, intParams );
	     isOk = thisFillParamFromChr( chr, nParams, intParams, paramVal );
	     isRepeatable = paramVal.repeats!=-1;
	     // DBGPRINT("Expanding arg%u (%u/%u), char%u: ",pos,i,n,iterChar); paramVal.Show(false); DBGPRINT("\n");
	     ASSERTION(isOk,"isOk");
	     // Argument found is to be expanded:
	     // the only exception is if one arg 'repeats'
	     // where this field will be incremented.
	     unsigned posChr=0;
	     if ( isRepeatable ) {
		 gParamVal* aParamPtr = paramVal.FindObj( resOptL, chr, posChr );
		 if ( posChr>0 ) {
		     aParamPtr->repeats++;
		 }
	     }

	     sParamRaw::eParamFollow aParamFollow = intParams[paramRef].sRaw.paramFollow;
	     bool hasFurtherVal = aParamFollow!=sParamRaw::e_ParamNoVal;
	     bool hasValWithin = aParamFollow==sParamRaw::e_ParamThisArg;

	     if ( posChr==0 ) {
		 paramVal.FindObj( resOptL, chr, posChr );
		 if ( posChr==0 ) {
		     if ( hasFurtherVal ) {
			 gString sTempVal( sTemp );

			 // If hasFurtherVal, either 'hasValWithin' or not.
			 // E.g. "-s=10" => hasValWithin=true!
			 // E.g. "-s 10" => hasValWithin=false!
			 // In the second case an additional arg must exist
			 if ( hasValWithin ) {
			     gString aParamSep( intParams[paramRef].sRaw.paramSep );
			     iterChar = len+1; //Exit expansion
			     sTempVal.Delete( 1, 1 );
			     chr = sTempVal[1];
			     unsigned posParamSep;
			     posParamSep = sTempVal.Find( aParamSep );
			     if ( posParamSep!=1 ) {
				 resArgL.Add( sArg );
				 resErrL.Add( sArg );
				 result++;
				 continue;
			     }
			     if ( aParamSep.Length()>0 )
				 sTempVal.Delete( 1, aParamSep.Length() );
			 }
			 else {
			     if ( pos>=inputSize ) {
				 resArgL.Add( sArg );
				 resErrL.Add( sArg );
				 result++;
				 continue;
			     }
			     // Mark argument as used by value ('2')
			     paramUsage[pos+1] = 2;
			     sTempVal = inputL.Str(pos+1);
			 }
			 // Fill contents of the parameter
			 // ==> paramVal.sVal = sTempVal
			 paramVal.FillParam( sTempVal, intParams[paramRef].sRaw.GetParamVal() );
			 countValErrors += paramVal.errorCode>=2;
		     }// end IF hasFurtherVal (single chr option)

		     // Finally add the parameter filled to the list of options
		     paramVal.AddToList( resOptL );
		 }
		 else {
		     // It is not a valid parameter if already parsed...
		     // which is the case here.
		     resArgL.Add( sArg );
		     resErrL.Add( sArg );
		     result++;
		     continue;
		 }
	     }
	 }
     }// end IF single-dash=T & doExpandPOSIX=T
     if ( hasSingleDash==true && doExpandPOSIX==false ) {
	 ASSERTION_FALSE("TODO");
     }
     if ( hasSingleDash==false ) {
	 // It is a double-dash option

	 bool doMatch=false;
	 unsigned possibleParamIdx = 0;
	 gParamVal paramVal;

	 paramRef = thisFindParamFromStr( sArg.Str(), nParams, intParams, doMatch, possibleParamIdx );
	 if ( paramRef==0 ) {
	     resArgL.Add( sArg );
	     resErrL.Add( sArg );
	     result++;
	     continue;
	 }

	 isOk = thisFillParamFromStr( sArg.Str(), paramRef, nParams, intParams, paramVal );
	 ASSERTION(isOk,"isOk");

	 sParamRaw::eParamFollow aParamFollow = intParams[paramRef].sRaw.paramFollow;
	 bool hasFurtherVal = aParamFollow!=sParamRaw::e_ParamNoVal;
	 bool hasValWithin = aParamFollow==sParamRaw::e_ParamThisArg;

	 if ( hasFurtherVal ) {
	     gString sTempVal( sArg );

	     DBGPRINT("DBG: sTempVal[%u/%u]: '%s', hasFurtherVal?%c, hasValWithin?%c, possibleParamIdx=%u\n",
		      i, n,
		      sTempVal.Str(),
		      ISyORn(hasFurtherVal),
		      ISyORn(hasValWithin),
		      possibleParamIdx);

	     // If hasFurtherVal, either 'hasValWithin' or not.
	     // E.g. "-s=10" or "--size=10" => hasValWithin=true!
	     // E.g. "-s 10" or "--size 10" => hasValWithin=false!
	     // In the second case an additional arg must exist
	     if ( hasValWithin ) {
		 ASSERTION(possibleParamIdx>0,"possibleParamIdx>0");
		 gString sParamString( intParams[paramRef].sRaw.members.Str(possibleParamIdx) );
		 gString aParamSep( intParams[paramRef].sRaw.paramSep );
		 sTempVal.Delete( 1, sParamString.Length() + aParamSep.Length() );
	     }
	     else {
		 if ( pos>=inputSize ) {
		     resArgL.Add( sArg );
		     resErrL.Add( sArg );
		     result++;
		     continue;
		 }
		 // Mark argument as used by value ('2')
		 paramUsage[pos+1] = 2;
		 sTempVal = inputL.Str(pos+1);
	     }
	     // Fill contents of the parameter
	     // ==> paramVal.sVal = sTempVal
	     paramVal.FillParam( sTempVal, intParams[paramRef].sRaw.GetParamVal() );
	     countValErrors += paramVal.errorCode>=2;
	 }// end IF hasFurtherVal (multi chr option)

	 // Finally add the parameter filled to the list of options
	 paramVal.AddToList( resOptL );
     }
 }

 for (i=1; i<=inputSize; i++) {
     if ( paramUsage[i]<=0 ) {
	 resArgL.Add( inputL.Str(i) );
     }
 }

 delete[] paramUsage;

 if ( countValErrors!=0 ) return -1;

 return result;
}
////////////////////////////////////////////////////////////

