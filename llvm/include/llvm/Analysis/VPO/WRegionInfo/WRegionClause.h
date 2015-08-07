//===----------------- WRegionClause.h - Clauses ----------------*- C++ -*-===//
//
//   Copyright (C) 2015 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
//
//   This file defines the classes that represent OpenMP clauses.
//   //TODO: capitalize variable names
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VPO_WREGIONCLAUSE_H
#define LLVM_ANALYSIS_VPO_WREGIONCLAUSE_H

#include <vector>

namespace llvm {

namespace vpo {

// for readability; VAR and EXPR match OpenMP4.1 specs
typedef Value* VAR;
typedef Value* EXPR;
typedef Value* RDECL;

//
// Classes below represent list items used in many OMP clauses 
// The actual clause (a list of items) is then of type vector<item>
//            
//   Item: base class NOT intended to be instantiated as is.
//         Contains members common to most list items in OMP clauses
//            
//   SharedItem:       derived class for an item in the SHARED       clause
//   PrivateItem:      derived class for an item in the PRIVATE      clause
//   FirstprivateItem: derived class for an item in the FIRSTPRIVATE clause
//   LastprivateItem:  derived class for an item in the LASTPRIVATE  clause
//   ReductionItem:    derived class for an item in the REDUCTION    clause
//   CopyinItem:       derived class for an item in the COPYIN       clause
//   CopyprivateItem:  derived class for an item in the COPYPRIVATE  clause
//   LinearItem:       derived class for an item in the LINEAR       clause
//


//
//   Item: abstract base class NOT intended to be instantiated directly.
//         Contains members common to list items in OMP clauses
//
class Item 
{
  private:
    VAR   OrigItem;  // original var 
    VAR   NewItem;   // new version (eg private) of the var
    VAR   ParmItem;  // formal parm in outlined entry; usually holds &OrigItem
    bool  IsNonpod;  // true for a C++ NONPOD var
    bool  IsVla;     // true for variable-length arrays (C99)
    EXPR  VlaSize;   // size of vla array can be an int expression

  protected:
    Item(VAR Orig=nullptr, VAR Newv=nullptr, VAR Parm=nullptr) :
      OrigItem(Orig), NewItem(Newv), ParmItem(Parm),
      IsNonpod(false), IsVla(false), VlaSize(nullptr) {}
    void setOrig(VAR V)          { OrigItem = V;    }
    void setNew(VAR V)           { NewItem = V;     }
    void setParm(VAR V)          { ParmItem = V;    }
    void setIsNonpod(bool Flag)  { IsNonpod = Flag; }
    void setIsVla(bool Flag)     { IsVla = Flag;    }
    void setVlaSize(EXPR Size)   { VlaSize = Size;  }

  public:
    VAR  getOrig()     const { return OrigItem; }
    VAR  getNew()      const { return NewItem;  }
    VAR  getParm()     const { return ParmItem; }
    bool getIsNonpod() const { return IsNonpod; }
    bool getIsVla()    const { return IsVla;    }
    EXPR getVlaSize()  const { return VlaSize;  }
};

//
//   SharedItem: OMP SHARED clause item
//   (cf PAROPT_OMP_SHARED_NODE)
//
class SharedItem : public Item
{
  private:
    bool  IsPassedDirectly;

  protected:
    SharedItem(VAR Orig=nullptr, VAR Newv=nullptr, VAR Parm=nullptr) :
      Item(Orig, Newv, Parm), IsPassedDirectly(false) {} 
    void setIsPassedDirectly(bool Flag) { IsPassedDirectly = Flag; }

  public:
    bool getIsPassedDirectly() const { return IsPassedDirectly; }
};


//
//   PrivateItem: OMP PRIVATE clause item
//   (cf PAROPT_OMP_PRIVATE_NODE)
//
class PrivateItem : public Item 
{
  private:
    RDECL Constructor;
    RDECL Destructor;

  protected:
    PrivateItem(VAR Orig=nullptr, VAR Newv=nullptr, VAR Parm=nullptr) :
      Item(Orig, Newv, Parm), Constructor(nullptr), Destructor(nullptr) {} 
    void setConstructor(RDECL Ctor) { Constructor = Ctor; }
    void setDestructor(RDECL Dtor)  { Destructor  = Dtor; }

  public:
    RDECL getConstructor() const { return Constructor; }
    RDECL getDestructor()  const { return Destructor;  }
};


//
//   FirstprivateItem: OMP FIRSTPRIVATE clause item
//   (cf PAROPT_OMP_FIRSTPRIVATE_NODE)
//
class FirstprivateItem : public Item 
{
  private:
    RDECL CopyConstructor;
    RDECL Destructor;

  protected:
    FirstprivateItem(VAR Orig=nullptr, VAR Newv=nullptr, VAR Parm=nullptr) :
      Item(Orig, Newv, Parm), CopyConstructor(nullptr), Destructor(nullptr) {} 
    void setCopyConstructor(RDECL Cctor) { CopyConstructor = Cctor; }
    void setDestructor(RDECL Dtor)       { Destructor  = Dtor;      }

  public:
    RDECL getCopyConstructor() const { return CopyConstructor; }
    RDECL getDestructor()      const { return Destructor;      }
};


//
//   LastprivateItem: OMP LASTPRIVATE clausclause item
//   (cf PAROPT_OMP_LASTPRIVATE_NODE)
//
class LastprivateItem : public Item 
{
  private:
    RDECL Constructor;
    RDECL Destructor;
    RDECL Copy;

  protected:
    LastprivateItem(VAR Orig=nullptr, VAR Newv=nullptr, VAR Parm=nullptr) :
      Item(Orig, Newv, Parm), Constructor(nullptr), Destructor(nullptr), 
      Copy(nullptr) {} 
    void setConstructor(RDECL Ctor) { Constructor = Ctor; }
    void setDestructor(RDECL Dtor)  { Destructor  = Dtor; }
    void setCopy(RDECL Cpy)         { Copy = Cpy;         }

  public:
    RDECL getConstructor() const { return Constructor; }
    RDECL getDestructor()  const { return Destructor; }
    RDECL getCopy()        const { return Copy; }
};


//
//   ReductionItem: OMP REDUCTION clause item
//   (cf PAROPT_OMP_REDUCTION_NODE)
//
class ReductionItem : public Item 
{
  typedef enum VPOReductionKind {
    VPOReductionError = 0,
    VPOReductionSum,
    VPOReductionMult,
    VPOReductionSub,
    VPOReductionMax,
    VPOReductionMin,
    VPOReductionAnd,
    VPOReductionOr,
    VPOReductionBand,
    VPOReductionBor,
    VPOReductionIeor,
    VPOReductionBxor,
    VPOReductionEqv,
    VPOReductionNeqv,
    VPOReductionUdr   //user-defined reduction
  } VPOReductionKind;

  private:
    VPOReductionKind type; // reduction operation
    RDECL combiner;
    RDECL initializer;

  protected:
    ReductionItem(VPOReductionKind op=VPOReductionError, VAR Orig=nullptr,
      VAR Newv=nullptr, VAR Parm=nullptr): Item(Orig, Newv, Parm),
                     type(op), combiner(nullptr), initializer(nullptr) {}

    void setType(VPOReductionKind op) { type = op;          }
    void setCombiner(RDECL comb)      { combiner = comb;    }
    void setInitializer(RDECL init)   { initializer = init; }

  public:
    VPOReductionKind getType() const { return type;        }
    RDECL getCombiner()        const { return combiner;    }
    RDECL getInitializer()     const { return initializer; }
};


//
//   CopyinItem: OMP COPYIN clausclause item
//   (cf PAROPT_OMP_COPYIN_NODE)
//
class CopyinItem : public Item
{
  private:
    RDECL Copy;

  protected:
    CopyinItem(VAR Orig=nullptr, VAR Newv=nullptr, VAR Parm=nullptr) :
      Item(Orig, Newv, Parm), Copy(nullptr) {} 
    void setCopy(RDECL Cpy) { Copy = Cpy; }

  public:
    RDECL getCopy() const { return Copy; }
};


//
//   CopyprivateItem: OMP COPYPRIVATE clause item
//
class CopyprivateItem : public Item
{
  private:
    RDECL Copy;

  protected:
    CopyprivateItem(VAR Orig=nullptr, VAR Newv=nullptr, VAR Parm=nullptr) :
      Item(Orig, Newv, Parm), Copy(nullptr) {} 
    void setCopy(RDECL Cpy) { Copy = Cpy; }

  public:
    RDECL getCopy() const { return Copy; }
};


//
//   LinearItem: OMP LINEAR clause item
//
class LinearItem : public Item 
{
  private:
    int Step;   // 0 if unspecified

    // No need for ctor/dtor because OrigItem is either pointer or array base

  protected:
    LinearItem(VAR Orig=nullptr, VAR Newv=nullptr, VAR Parm=nullptr) :
      Item(Orig, Newv, Parm), Step(0) {} 
    void setStep(int S) { Step = S; }

  public:
    int getStep() const { return Step; }
};


//
// These two item classes for list-type clauses are not derived from the 
// base "Item" class above.
//            
//   DependItem    (for the depend  clause in task and target constructs)
//   AlignedItem   (for the aligned clause in simd constructs)
//
class DependItem 
{
  private:
    VAR   base;           // scalar item or base of array section
    bool  isOut;          // depend type: false for IN; true for OUT/INOUT
    bool  isArraySection; // if true, then lb, length, stride below are used
    EXPR  lowerBound;     // null if unspecified
    EXPR  length;         // null if unspecified
    EXPR  stride;         // null if unspecified

  protected:
    DependItem(VAR V=nullptr) : base(V), isOut(false), isArraySection(false),
      lowerBound(nullptr), length(nullptr), stride(nullptr) {}

    void setOrig(VAR V)         { base = V; }
    void setIsOut(bool Flag)    { isOut = Flag; }
    void setIsArrSec(bool Flag) { isArraySection = Flag; }
    void setLb(EXPR lb)         { lowerBound = lb;   }
    void setLength(EXPR len)    { length = len;  }
    void setStride(EXPR str)    { stride = str;  }

  public:
    VAR  getOrig()      const   { return base; }
    bool getIsOut()     const   { return isOut; }
    bool getIsArrSec()  const   { return isArraySection; }
    EXPR getLb()        const   { return lowerBound; }
    EXPR getLength()    const   { return length; }
    EXPR getStride()    const   { return stride; }
};


class AlignedItem 
{
  private:
    VAR   base;           // pointer or base of array
    int   alignment;      // 0 if unspecified

  protected:
    AlignedItem(VAR V=nullptr) : base(V), alignment(0) {}
    void setOrig(VAR V)      { base = V; }
    void setAlign(int align) { alignment = align; }

  public:
    VAR  getOrig()  const { return base; }
    int  getAlign() const { return alignment; }
};



//
// The list-type clauses are essentially vectors of the clause items above
//            
template <typename ClauseItem> class Clause
{
  private:
    typedef typename std::vector<ClauseItem> ItemArray;
    typedef typename ItemArray::iterator     Iterator;

    ItemArray C;

  protected:
    // Create a new item for VAR V and append it to the clause
    void add(VAR V) { ClauseItem item(V); C.push_back(item); }

  public:
    int size()         const { return C.size();     }
    int capacity()     const { return C.capacity(); }
    Iterator begin()   const { return C.begin();    }
    Iterator end()     const { return C.end();      }
    ClauseItem front() const { return C.front();    }
    ClauseItem back()  const { return C.back();     }

    // search the clause for 
    Iterator findOrig(VAR V) { 
      for (auto I=begin(); I != end(); ++I)
        if (I->getOrig() == V) 
          return I;
      return end();
    }
};

/*
template <typename ClauseItem>
typename std::vector<ClauseItem>::iterator Clause<ClauseItem>::findOrig(VAR V)
{
  for (auto I=begin(), E=end(); I != E; ++I)
    if (I->getOrig() == V) return I;
  return E;
}
*/


//
// typedef for list-type clause classes and associated iterator types
//
typedef Clause<SharedItem>       SharedClause;
typedef Clause<PrivateItem>      PrivateClause;
typedef Clause<FirstprivateItem> FirstprivateClause;
typedef Clause<LastprivateItem>  LastprivateClause;
typedef Clause<ReductionItem>    ReductionClause;
typedef Clause<CopyinItem>       CopyinClause;
typedef Clause<CopyprivateItem>  CopyprivateClause;
typedef Clause<LinearItem>       LinearClause;
typedef Clause<DependItem>       DependClause;
typedef Clause<AlignedItem>      AlignedClause;

typedef std::vector<SharedItem>::iterator       SharedIter;
typedef std::vector<PrivateItem>::iterator      PrivateIter;
typedef std::vector<FirstprivateItem>::iterator FirstprivateIter;
typedef std::vector<LastprivateItem>::iterator  LastprivateIter;
typedef std::vector<ReductionItem>::iterator    ReductionIter;
typedef std::vector<CopyinItem>::iterator       CopyinIter;
typedef std::vector<CopyprivateItem>::iterator  CopyprivateIter;
typedef std::vector<LinearItem>::iterator       LinearIter;
typedef std::vector<DependItem>::iterator       DependIter;
typedef std::vector<AlignedItem>::iterator      AlignedIter;



//
// Support for other OMP clauses (not list-type)
//            

typedef enum VPODefaultKind {
    VPODefaultFalse = 0,      // default clause not present
    VPODefaultNone,           // default(none)
    VPODefaultShared,         // default(shared)
    VPODefaultPrivate,        // default(private) // Fortran only
    VPODefaultFirstprivate    // default(firstprivate) //Fortran only
} VPODefaultKind;

typedef enum VPOAtomicKind {
    VPOAtomicUpdate = 0,
    VPOAtomicRead,
    VPOAtomicWrite,
    VPOAtomicCapture
} VPOAtomicKind;

typedef enum VPOCancelKind {
    VPOCancelError = 0,
    VPOCancelParallel,
    VPOCancelLoop,
    VPOCancelSections,
    VPOCancelTaskgroup,
} VPOCancelKind;
//
// The values of the enums are used to invoke the RTL,
// so do not change them
//
typedef enum VPOProcBindKind {
    VPOProcBindFalse  = 0,   // proc_bind clause not present
    VPOProcBindTrue   = 1,   // what is this for?
    VPOProcBindMaster = 2,   // proc_bind(master)
    VPOProcBindClose  = 3,   // proc_bind(close)
    VPOProcBindSpread = 4    // proc_bind(srpead)
} VPOProcBindKind;


//
// The values of the enums are used to invoke the RTL,
// so do not change them
//
typedef enum VPOScheduleKind {
    VPOScheduleCrewloop                = 18,
    VPOScheduleStatic                  = 33,
    VPOScheduleStaticEven              = 34,
    VPOScheduleDynamic                 = 35,
    VPOScheduleGuided                  = 36,
    VPOScheduleRuntime                 = 37,
    VPOScheduleAuto                    = 38,
    VPOScheduleTrapezoidal             = 39,
    VPOScheduleStaticGreedy            = 40,
    VPOScheduleStaticBalanced          = 41,
    VPOScheduleGUIDEDIterative         = 42,
    VPOScheduleGUIDEDAnalytical        = 43,

    VPOScheduleOrderedStatic           = 65,
    VPOScheduleOrderedStaticEven       = 66,
    VPOScheduleOrderedDynamic          = 67,
    VPOScheduleOrderedGuided           = 68,
    VPOScheduleOrderedRuntime          = 69,
    VPOScheduleOrderedAuto             = 70,

    VPOScheduleOrderedTrapezoidal      = 71,
    VPOScheduleOrderedStaticGreedy     = 72,
    VPOScheduleOrderedStaticBalanced   = 73,
    VPOScheduleOrderedGuidedITerative  = 74,
    VPOScheduleOrderedGuidedAnalytical = 75,

    VPOScheduleDistributeStatic        = 91,
    VPOScheduleDistributeStaticEven    = 92
} VPOScheduleKind;


} // End namespace vpo

} // End namespace llvm

#endif
