//===----------------- WRegionClause.h - Clauses ----------------*- C++ -*-===//
//
//   Copyright (C) 2016 Intel Corporation. All rights reserved.
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
#include "llvm/Transforms/Intel_VPO/Utils/VPOUtils.h"

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
//   UniformItem:      derived class for an item in the UNIFORM      clause
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
    WRNReductionSub,
    WRNReductionMult,
    WRNReductionAnd,
    WRNReductionOr,
    WRNReductionBxor,
    WRNReductionBand,
    WRNReductionBor,
    WRNReductionEqv,  // Fortran; currently unsupported 
    WRNReductionNeqv, // Fortran; currently unsupported 
    WRNReductionMax,  // Fortran; currently unsupported 
    WRNReductionMin,  // Fortran; currently unsupported 
    WRNReductionUdr   // user-defined reduction
  } WRNReductionKind;

  private:
    WRNReductionKind Ty; // reduction operation
    RDECL Combiner;
    RDECL Initializer;

  public:
    ReductionItem(VAR Orig, WRNReductionKind Op=WRNReductionError): 
      Item(Orig), Ty(Op), Combiner(nullptr), Initializer(nullptr) {}

    static WRNReductionKind getKindFromClauseId(int Id) {
      switch(Id) {
        case QUAL_OMP_REDUCTION_ADD:
          return WRNReductionSum;
        case QUAL_OMP_REDUCTION_SUB:
          return WRNReductionSub;
        case QUAL_OMP_REDUCTION_MUL:
          return WRNReductionMult;
        case QUAL_OMP_REDUCTION_AND:
          return WRNReductionAnd;
        case QUAL_OMP_REDUCTION_OR:
          return WRNReductionOr;
        case QUAL_OMP_REDUCTION_XOR:
          return WRNReductionBxor;
        case QUAL_OMP_REDUCTION_BAND:
          return WRNReductionBand;
        case QUAL_OMP_REDUCTION_BOR:
          return WRNReductionBor;
        case QUAL_OMP_REDUCTION_UDR:
          return WRNReductionUdr;
        default: 
          llvm_unreachable("Unsupported Reduction Clause ID");
      }
    };

    static int getClauseIdFromKind(WRNReductionKind Kind) {
      switch(Kind) {
        case WRNReductionSum:
          return QUAL_OMP_REDUCTION_ADD;
        case WRNReductionSub:
          return QUAL_OMP_REDUCTION_SUB;
        case WRNReductionMult:
          return QUAL_OMP_REDUCTION_MUL;
        case WRNReductionAnd:
          return QUAL_OMP_REDUCTION_AND;
        case WRNReductionOr:
          return QUAL_OMP_REDUCTION_OR;
        case WRNReductionBxor:
          return QUAL_OMP_REDUCTION_XOR;
        case WRNReductionBand:
          return QUAL_OMP_REDUCTION_BAND;
        case WRNReductionBor:
          return QUAL_OMP_REDUCTION_BOR;
        case WRNReductionUdr:
          return QUAL_OMP_REDUCTION_UDR;
        default: 
          llvm_unreachable("Unsupported Reduction Kind ");
      }
    };

    void setType(WRNReductionKind Op) { Ty = Op;          }
    void setCombiner(RDECL Comb)      { Combiner = Comb;    }
    void setInitializer(RDECL Init)   { Initializer = Init; }
    WRNReductionKind getType() const { return Ty;        }
    RDECL getCombiner()        const { return Combiner;    }
    RDECL getInitializer()     const { return Initializer; }
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
//   UniformItem: OMP UNIFORM clause item
//
class UniformItem : public Item
{
  public:
    UniformItem(VAR Orig) : Item(Orig) {}
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
    VAR   Base;           // scalar item or base of array section
    bool  IsOut;          // depend type: false for IN; true for OUT/INOUT
    bool  IsArraySection; // if true, then lb, length, stride below are used
    EXPR  LowerBound;     // null if unspecified
    EXPR  Length;         // null if unspecified
    EXPR  Stride;         // null if unspecified

  public:
    DependItem(VAR V=nullptr) : Base(V), IsOut(false), IsArraySection(false),
      LowerBound(nullptr), Length(nullptr), Stride(nullptr) {}

    void setOrig(VAR V)         { Base = V; }
    void setIsOut(bool Flag)    { IsOut = Flag; }
    void setIsArrSec(bool Flag) { IsArraySection = Flag; }
    void setLb(EXPR Lb)         { LowerBound = Lb;   }
    void setLength(EXPR Len)    { Length = Len;  }
    void setStride(EXPR Str)    { Stride = Str;  }

    VAR  getOrig()      const   { return Base; }
    bool getIsOut()     const   { return IsOut; }
    bool getIsArrSec()  const   { return IsArraySection; }
    EXPR getLb()        const   { return LowerBound; }
    EXPR getLength()    const   { return Length; }
    EXPR getStride()    const   { return Stride; }
};


class AlignedItem 
{
  private:
    VAR   Base;           // pointer or base of array
    int   Alignment;      // 0 if unspecified

  public:
    AlignedItem(VAR V=nullptr) : Base(V), Alignment(0) {}
    void setOrig(VAR V)      { Base = V; }
    void setAlign(int Align) { Alignment = Align; }
    VAR  getOrig()  const { return Base; }
    int  getAlign() const { return Alignment; }
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
    int getClauseID()               const { return ClauseID;     }
    void setClauseID(int ID)              { ClauseID = ID;       }
    int size()                      const { return C.size();     }
    int capacity()                  const { return C.capacity(); }
    const ClauseItem *front()       const { return C.front();    }
    const ClauseItem *back()        const { return C.back();     }
    ConstIterator begin()           const { return C.begin();    }
    ConstIterator end()             const { return C.end();      }
    Iterator begin()                      { return C.begin();    }
    Iterator end()                        { return C.end();      }
    ClauseItem *front()                   { return C.front();    }
    ClauseItem *back()                    { return C.back();     }

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
        if (getClauseID() == QUAL_OMP_LINEAR) {
          LinearItem *LinItem = (LinearItem*)(*I);
          OS << ", stride = " << LinItem->getStep();
        }
      }
      OS << "\n";
    }

    // search the clause for 
    ClauseItem *findOrig(const VAR V) { 
      for (auto I : items())
        if (I->getOrig() == V) 
          return I;
      return nullptr;
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
typedef Clause<UniformItem>      UniformClause;
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
typedef std::vector<UniformItem>::iterator      UniformIter;
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
