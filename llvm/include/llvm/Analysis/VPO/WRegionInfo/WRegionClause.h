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
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VPO_WREGIONCLAUSE_H
#define LLVM_ANALYSIS_VPO_WREGIONCLAUSE_H

#include <vector>
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/VPO/Utils/VPOUtils.h"

namespace llvm {

namespace vpo {

class WRegionUtils;

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
  friend class WRegionUtils;
  private:
    VAR   OrigItem;  // original var 
    VAR   NewItem;   // new version (eg private) of the var
    VAR   ParmItem;  // formal parm in outlined entry; usually holds &OrigItem
    bool  IsNonpod;  // true for a C++ NONPOD var
    bool  IsVla;     // true for variable-length arrays (C99)
    EXPR  VlaSize;   // size of vla array can be an int expression

  protected:
    Item(VAR Orig) :
      OrigItem(Orig), NewItem(nullptr), ParmItem(nullptr),
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

  public:
    SharedItem(VAR Orig) : Item(Orig), IsPassedDirectly(false) {} 
    void setIsPassedDirectly(bool Flag) { IsPassedDirectly = Flag; }
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

  public:
    PrivateItem(VAR Orig) :
      Item(Orig), Constructor(nullptr), Destructor(nullptr) {} 
    void setConstructor(RDECL Ctor) { Constructor = Ctor; }
    void setDestructor(RDECL Dtor)  { Destructor  = Dtor; }
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

  public:
    FirstprivateItem(VAR Orig) :
      Item(Orig), CopyConstructor(nullptr), Destructor(nullptr) {} 
    void setCopyConstructor(RDECL Cctor) { CopyConstructor = Cctor; }
    void setDestructor(RDECL Dtor)       { Destructor  = Dtor;      }
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

  public:
    LastprivateItem(VAR Orig) : 
      Item(Orig), Constructor(nullptr), Destructor(nullptr), Copy(nullptr) {} 
    void setConstructor(RDECL Ctor) { Constructor = Ctor; }
    void setDestructor(RDECL Dtor)  { Destructor  = Dtor; }
    void setCopy(RDECL Cpy)         { Copy = Cpy;         }
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
public:
  typedef enum WRNReductionKind {
    WRNReductionError = 0,
    WRNReductionSum,
    WRNReductionMult,
    WRNReductionSub,
    WRNReductionMax,
    WRNReductionMin,
    WRNReductionAnd,
    WRNReductionOr,
    WRNReductionBand,
    WRNReductionBor,
    WRNReductionIeor,
    WRNReductionBxor,
    WRNReductionEqv,
    WRNReductionNeqv,
    WRNReductionUdr   //user-defined reduction
  } WRNReductionKind;

  private:
    WRNReductionKind type; // reduction operation
    RDECL combiner;
    RDECL initializer;

  public:
    ReductionItem(VAR Orig, WRNReductionKind op=WRNReductionError): 
      Item(Orig), type(op), combiner(nullptr), initializer(nullptr) {}
    void setType(WRNReductionKind op) { type = op;          }
    void setCombiner(RDECL comb)      { combiner = comb;    }
    void setInitializer(RDECL init)   { initializer = init; }
    WRNReductionKind getType() const { return type;        }
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

  public:
    CopyinItem(VAR Orig) : Item(Orig), Copy(nullptr) {} 
    void setCopy(RDECL Cpy) { Copy = Cpy; }
    RDECL getCopy() const { return Copy; }
};


//
//   CopyprivateItem: OMP COPYPRIVATE clause item
//
class CopyprivateItem : public Item
{
  private:
    RDECL Copy;

  public:
    CopyprivateItem(VAR Orig) : Item(Orig), Copy(nullptr) {} 
    void setCopy(RDECL Cpy) { Copy = Cpy; }
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

  public:
    LinearItem(VAR Orig) : Item(Orig), Step(0) {} 
    void setStep(int S) { Step = S; }
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

  public:
    DependItem(VAR V=nullptr) : base(V), isOut(false), isArraySection(false),
      lowerBound(nullptr), length(nullptr), stride(nullptr) {}

    void setOrig(VAR V)         { base = V; }
    void setIsOut(bool Flag)    { isOut = Flag; }
    void setIsArrSec(bool Flag) { isArraySection = Flag; }
    void setLb(EXPR lb)         { lowerBound = lb;   }
    void setLength(EXPR len)    { length = len;  }
    void setStride(EXPR str)    { stride = str;  }

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

  public:
    AlignedItem(VAR V=nullptr) : base(V), alignment(0) {}
    void setOrig(VAR V)      { base = V; }
    void setAlign(int align) { alignment = align; }
    VAR  getOrig()  const { return base; }
    int  getAlign() const { return alignment; }
};



//
// The list-type clauses are essentially vectors of the clause items above
//            
template <typename ClauseItem> class Clause
{
  friend class WRegionUtils;
  private:
    typedef typename std::vector<ClauseItem*>       ItemArray;
    typedef typename ItemArray::iterator            Iterator;
    typedef typename ItemArray::const_iterator      ConstIterator;

    ItemArray C;
    int ClauseID;

  protected:
    // Create a new item for VAR V and append it to the clause
     void add(VAR V) { ClauseItem *P = new ClauseItem(V); C.push_back(P); }

  public:
    int getClauseID()        const { return ClauseID;     }
    void setClauseID(int ID)       { ClauseID = ID;       }
    int size()               const { return C.size();     }
    int capacity()           const { return C.capacity(); }
    ClauseItem front()       const { return C.front();    }
    ClauseItem back()        const { return C.back();     }
    ConstIterator begin()    const { return C.begin();    }
    ConstIterator end()      const { return C.end();      }
    Iterator begin()               { return C.begin();    }
    Iterator end()                 { return C.end();      }

    typedef iterator_range<Iterator> ItemsRange;
    typedef iterator_range<ConstIterator> ConstItemsRange;

    ItemsRange  items() {
      return ItemsRange(begin(), end());
    }
    ConstItemsRange items() const {
      return ConstItemsRange(begin(), end());
    }

    void print(formatted_raw_ostream &OS) const {
      StringRef S = VPOUtils::getClauseName(getClauseID());
      OS << S << " clause, size=" << size() << ": " ;
      for (auto I=begin(); I != end(); ++I) {
        OS << "(" << *((*I)->getOrig()) << ") ";
      }
    }

    // search the clause for 
    Iterator findOrig(VAR V) { 
      for (auto I : items())
        if ((*I)->getOrig() == V) 
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

typedef enum WRNDefaultKind {
    WRNDefaultAbsent = 0,     // default clause not present
    WRNDefaultNone,           // default(none)
    WRNDefaultShared,         // default(shared)
    WRNDefaultPrivate,        // default(private) // Fortran only
    WRNDefaultFirstprivate    // default(firstprivate) //Fortran only
} WRNDefaultKind;

typedef enum WRNAtomicKind {
    WRNAtomicUpdate = 0,
    WRNAtomicRead,
    WRNAtomicWrite,
    WRNAtomicCapture
} WRNAtomicKind;

typedef enum WRNCancelKind {
    WRNCancelError = 0,
    WRNCancelParallel,
    WRNCancelLoop,
    WRNCancelSections,
    WRNCancelTaskgroup,
} WRNCancelKind;
//
// The values of the enums are used to invoke the RTL,
// so do not change them
//
typedef enum WRNProcBindKind {
    WRNProcBindAbsent = 0,   // proc_bind clause not present
    WRNProcBindTrue   = 1,   // what is this for?
    WRNProcBindMaster = 2,   // proc_bind(master)
    WRNProcBindClose  = 3,   // proc_bind(close)
    WRNProcBindSpread = 4    // proc_bind(srpead)
} WRNProcBindKind;


//
// The values of the enums are used to invoke the RTL,
// so do not change them
//
typedef enum WRNScheduleKind {
    WRNScheduleCrewloop                = 18,
    WRNScheduleStatic                  = 33,
    WRNScheduleStaticEven              = 34,
    WRNScheduleDynamic                 = 35,
    WRNScheduleGuided                  = 36,
    WRNScheduleRuntime                 = 37,
    WRNScheduleAuto                    = 38,
    WRNScheduleTrapezoidal             = 39,
    WRNScheduleStaticGreedy            = 40,
    WRNScheduleStaticBalanced          = 41,
    WRNScheduleGUIDEDIterative         = 42,
    WRNScheduleGUIDEDAnalytical        = 43,

    WRNScheduleOrderedStatic           = 65,
    WRNScheduleOrderedStaticEven       = 66,
    WRNScheduleOrderedDynamic          = 67,
    WRNScheduleOrderedGuided           = 68,
    WRNScheduleOrderedRuntime          = 69,
    WRNScheduleOrderedAuto             = 70,

    WRNScheduleOrderedTrapezoidal      = 71,
    WRNScheduleOrderedStaticGreedy     = 72,
    WRNScheduleOrderedStaticBalanced   = 73,
    WRNScheduleOrderedGuidedITerative  = 74,
    WRNScheduleOrderedGuidedAnalytical = 75,

    WRNScheduleDistributeStatic        = 91,
    WRNScheduleDistributeStaticEven    = 92
} WRNScheduleKind;


} // End namespace vpo

} // End namespace llvm

#endif
