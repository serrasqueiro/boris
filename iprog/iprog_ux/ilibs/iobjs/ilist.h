#ifndef iLIST_X_H
#define iLIST_X_H

#include "istorage.h"
#include "istring.h"

////////////////////////////////////////////////////////////
// Common enums
// ---------------------------------------------------------
enum eFindCriteria {
    e_FindExactPosition,
    e_FindFromPosition,
    e_FindBeforePosition
};

////////////////////////////////////////////////////////////
// Arguments and their corresponding support classes
// ---------------------------------------------------------
////////////////////////////////////////////////////////////
class gElem : public gStorage {
public:
    gElem () ;
    gElem (gStorage* newObj) ;
    virtual ~gElem () ;

    // Public data-members
    gElem* prev;
    gElem* next;
    gStorage* me;

    // Get methods
    virtual int MaximumStorageSize () {
	return 1;
    }

    virtual char* Str (unsigned idx=0) ;

    virtual t_uchar* ToString (const t_uchar* uBuf) ;

    // Save/etc methods
    virtual gStorage* NewObject () ;
    virtual bool SaveGuts (FILE* f) ;
    virtual bool RestoreGuts (FILE* f) ;

private:
    //Operators,empty
    gElem (gElem& ) ; //empty
    gElem& operator= (gElem& ) ; //empty
};

////////////////////////////////////////////////////////////
class gListGeneric : public gStorage {
public:
    virtual ~gListGeneric () ;

    // Get methods
    virtual bool IsEmpty () {
	return size==0;
    }

    virtual unsigned N () {
	return size;
    }

    int GetInt () {
	return (int)size;
    }

    virtual bool IsValidIndex (unsigned idx) ;

    virtual char* Str (unsigned idx=0) ;

    virtual t_uchar* UStr (unsigned idx) {
	return (t_uchar*)Str( idx );
    }

    virtual gElem& GetElement (unsigned idx) ;
    virtual gElem* GetElementPtr (unsigned idx) ;

    virtual gStorage* GetObjectPtr (unsigned idx) ;
    virtual gStorage* GetFirstObjectPtr () ;
    virtual gStorage* GetLastObjectPtr () ;

    virtual int GetFirstInt () ;
    virtual int GetLastInt () ;

    virtual eStorage ElementsKind () ;

    virtual int MinimumStorageSize () {
	return -1;
    }
    virtual int MaximumStorageSize () {
	return -1;
    }

    // Set methods
    virtual void Reset () {
	Delete();
    }

    virtual unsigned Delete (unsigned startPos=0, unsigned endPos=0) ;

    virtual bool AppendObject (gStorage* newObj) {
	if ( newObj==nil ) return false;
	thisAppend( newObj );
	return true;
    }

    // Specific methods
    virtual gElem* StartPtr () {
	return pStart;
    }

    virtual gElem* EndPtr () {
	return pEnd;
    }

    virtual gElem* CurrentPtr () {
	return pCurrent;
    }

    virtual gString& Shown () {
	return sShown;
    }

    // Save/etc methods

    virtual gStorage* NewObject () ;

    virtual t_uchar* ToString (const t_uchar* uBuf) ;

    virtual bool SaveGuts (FILE* f) ;
    virtual bool RestoreGuts (FILE* f) ;

    // Operators,etc
    int operator[] (int idx) ;

protected:
    gListGeneric (eStorage aKind, const char* aStr) ;

    gListGeneric (eStorage aKind, gString& s) ;

    unsigned size;
    gElem* pStart;
    gElem* pEnd;
    gElem* pCurrent;
    gString sShown;
    static const char* strNil;

    void thisPreAllocate (unsigned toSize) ;
    unsigned thisDelete () ;

    bool thisIndex (unsigned& idx) ;
    bool thisAppend (gStorage* newObj) ;

private:
    // Operators,empty
    gListGeneric (gListGeneric& ) ;
    gListGeneric& operator= (gListGeneric& ) ;
};

////////////////////////////////////////////////////////////
class gList : public gListGeneric {
public:
    // Public data-members
    enum eStrict {
	relaxed,
	strict
    };

    gList (const char* aStr=nil) ;

    gList (gString& s) ;

    virtual ~gList () ;

    // Get methods

    int GetInt (unsigned idx) ;

    virtual int GetListInt (unsigned idx, eStrict strictness=relaxed) ;

    virtual unsigned GetListUInt (unsigned idx, eStrict strictness=relaxed) ;

    virtual int CompareStr (const char* aStr) {
	return CompareStrs( Str(), aStr );
    }

    // Find methods
    virtual unsigned Match (const char* s) ;

    virtual unsigned FindFirst (const char* s,
				unsigned strPos,
				eFindCriteria findCriteria) ;

    virtual unsigned Find (const char* s,
			   unsigned strPos,
			   eFindCriteria findCriteria) ;

    virtual unsigned FindAny (const char* s,
			      unsigned strPos,
			      eFindCriteria findCriteria,
			      gList& posL) ;

    // Set methods

    virtual int Tidy (int value=0, bool basedOnValue=true) ;

    unsigned Add (int v) ;
    unsigned Add (unsigned v) ;
    unsigned Add (gString& copy) ;
    unsigned Add (const char* s) ;
    unsigned Add (t_uchar* s) ;

    virtual gList& AddFrom (gElem* pFrom, bool doUpChar=false) ;

    virtual gList& CopyFrom (gElem* pFrom, bool doUpChar=false) ;

    virtual gList& CopyList (gList& aL, bool doUpChar=false) ;

    virtual unsigned DeleteString (gString& s) ;

    virtual unsigned DeleteStrings (gString& s) ;

    virtual int InsertObject (gStorage* ptrNew) ;

    virtual int InsertHere (gStorage* ptrNew) ;

    virtual int InsertBefore (gStorage* ptrNew) ;

    virtual int InsertOrdered (gStorage* ptrNew) ;

    virtual int InsertOrderedUnique (gStorage* ptrNew) ;

    // Save/etc methods
    virtual t_uchar* ToString (const t_uchar* uBuf) ;

    virtual bool SaveGuts (FILE* f) ;
    virtual bool RestoreGuts (FILE* f) ;

    // Special methods
    char* operator[] (int idx) ;

    // Show methods
    virtual void Show (bool doShowAll=true) ;

protected:
    unsigned thisFind (const char* s,
		       unsigned strPos,
		       eFindCriteria findCriteria,
		       bool doStopOnFirst,
		       gList& posL) ;

    int thisInsertOrdered (gStorage* ptrNew, int isUnique) ;

private:
    // Operators,empty
    gList (gList& ) ;
    gList& operator= (gList& ) ;
};
////////////////////////////////////////////////////////////
#endif //iLIST_X_H

