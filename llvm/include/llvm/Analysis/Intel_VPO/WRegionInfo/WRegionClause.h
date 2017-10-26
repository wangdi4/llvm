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
#include "llvm/Analysis/Intel_VPO/Utils/VPOAnalysisUtils.h"
#include "llvm/IR/Metadata.h"

namespace llvm {

namespace vpo {

class WRegionNode;
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
//   MapItem:          derived class for an item in the MAP          clause
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
    int   ThunkIdx;  // used for task/taskloop codegen
    MDNode *AliasScope; // alias info (loads)  to help registerize private vars
    MDNode *NoAlias;    // alias info (stores) to help registerize private vars

  public:
    Item(VAR Orig) :
      OrigItem(Orig), NewItem(nullptr), ParmItem(nullptr), IsNonpod(false),
      IsVla(false), VlaSize(nullptr), ThunkIdx(-1), AliasScope(nullptr),
      NoAlias(nullptr) {}

    void setOrig(VAR V)          { OrigItem = V;    }
    void setNew(VAR V)           { NewItem = V;     }
    void setParm(VAR V)          { ParmItem = V;    }
    void setIsNonpod(bool Flag)  { IsNonpod = Flag; }
    void setIsVla(bool Flag)     { IsVla = Flag;    }
    void setVlaSize(EXPR Size)   { VlaSize = Size;  }
    void setThunkIdx(int I)      { ThunkIdx = I;    }
    void setAliasScope(MDNode *M){ AliasScope = M;  }
    void setNoAlias(MDNode *M)   { NoAlias = M;     }

    VAR  getOrig()     const { return OrigItem; }
    VAR  getNew()      const { return NewItem;  }
    VAR  getParm()     const { return ParmItem; }
    bool getIsNonpod() const { return IsNonpod; }
    bool getIsVla()    const { return IsVla;    }
    EXPR getVlaSize()  const { return VlaSize;  }
    int getThunkIdx()  const { return ThunkIdx; }
    MDNode *getAliasScope() const { return AliasScope; }
    MDNode *getNoAlias()    const { return NoAlias; }

    virtual void print(formatted_raw_ostream &OS, bool PrintType=true) const {
      OS << "(" ; 
      getOrig()->printAsOperand(OS, PrintType); 
      OS << ") ";
    }

    // Conditional lastprivate:
    // Abort if these methods are invoked from anything but a LastprivateItem.
    virtual void setIsConditional(bool B){ 
     llvm_unreachable("Unexpected keyword: CONDITIONAL");
    }
    virtual bool getIsConditional() const {
     llvm_unreachable("Unexpected keyword: CONDITIONAL");
    }
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
    bool  IsConditional;    // conditional lastprivate
    RDECL Constructor;
    RDECL Destructor;
    RDECL Copy;

  public:
    LastprivateItem(VAR Orig) : Item(Orig), IsConditional(false),
      Constructor(nullptr), Destructor(nullptr), Copy(nullptr) {}
    void setIsConditional(bool B)   { IsConditional = B; }
    void setConstructor(RDECL Ctor) { Constructor = Ctor; }
    void setDestructor(RDECL Dtor)  { Destructor  = Dtor; }
    void setCopy(RDECL Cpy)         { Copy = Cpy;         }
    bool  getIsConditional() const { return IsConditional; }
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
    WRNReductionMax,
    WRNReductionMin,
    WRNReductionUdr   // user-defined reduction
  } WRNReductionKind;

  private:
    WRNReductionKind Ty; // reduction operation
    bool  IsUnsigned;    // for min/max reduction; default is signed min/max
    RDECL Combiner;
    RDECL Initializer;

  public:
    ReductionItem(VAR Orig, WRNReductionKind Op=WRNReductionError): Item(Orig),
      Ty(Op), IsUnsigned(false), Combiner(nullptr), Initializer(nullptr) {}

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
        case QUAL_OMP_REDUCTION_MAX:
          return WRNReductionMax;
        case QUAL_OMP_REDUCTION_MIN:
          return WRNReductionMin;
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
        case WRNReductionMax:
          return QUAL_OMP_REDUCTION_MAX;
        case WRNReductionMin:
          return QUAL_OMP_REDUCTION_MIN;
        case WRNReductionUdr:
          return QUAL_OMP_REDUCTION_UDR;
        default: 
          llvm_unreachable("Unsupported Reduction Kind ");
      }
    };

    void setType(WRNReductionKind Op) { Ty = Op;          }
    void setIsUnsigned(bool B)        { IsUnsigned = B;   }
    void setCombiner(RDECL Comb)      { Combiner = Comb;    }
    void setInitializer(RDECL Init)   { Initializer = Init; }
    WRNReductionKind getType() const { return Ty;        }
    bool getIsUnsigned()       const { return IsUnsigned;  }
    RDECL getCombiner()        const { return Combiner;    }
    RDECL getInitializer()     const { return Initializer; }

    // Return a string for the reduction operation, such as "ADD" and "MUL"
    StringRef getOpName() const {
      int ClauseId = getClauseIdFromKind(Ty);
      return VPOAnalysisUtils::getReductionOpName(ClauseId);
    };

    // Don't use the default print() from the base class "Item", because
    // we need to print the Reduction operation too.
    void print(formatted_raw_ostream &OS, bool PrintType=true) const {
      OS << "(" << getOpName() << ": "; 
      getOrig()->printAsOperand(OS, PrintType); 
      OS << ") ";
    }
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
    int Step;   // default is 1

    // No need for ctor/dtor because OrigItem is either pointer or array base

  public:
    LinearItem(VAR Orig) : Item(Orig), Step(1) {} 
    void setStep(int S) { Step = S; }
    int getStep() const { return Step; }

    // Specialized print() to output the stride as well
    void print(formatted_raw_ostream &OS, bool PrintType=true) const {
      OS << "("; 
      getOrig()->printAsOperand(OS, PrintType); 
      OS << ", " << getStep() << ") ";
    }
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
//   MapItem: OMP MAP clause item
//
class MapItem : public Item 
{
private:
  unsigned MapKind;  // bit vector for map kind and modifiers

public:
  enum WRNMapKind {
    WRNMapNone      = 0x0000,
    WRNMapTo        = 0x0001,
    WRNMapFrom      = 0x0002,
    WRNMapTofrom    = 0x0004,
    WRNMapAlloc     = 0x0008,
    WRNMapRelease   = 0x0010,
    WRNMapDelete    = 0x0020,
    WRNMapAlways    = 0x0100  
  } WRNMapKind;

  MapItem(VAR Orig) : Item(Orig), MapKind(0) {} 

  static unsigned getMapKindFromClauseId(int Id) {
    switch(Id) {
      case QUAL_OMP_TO:
      case QUAL_OMP_MAP_TO:
        return WRNMapTo;
      case QUAL_OMP_FROM:
      case QUAL_OMP_MAP_FROM:
        return WRNMapFrom;
      case QUAL_OMP_MAP_TOFROM:
        return WRNMapTofrom;
      case QUAL_OMP_MAP_ALLOC:
        return WRNMapAlloc;
      case QUAL_OMP_MAP_RELEASE:
        return WRNMapRelease;
      case QUAL_OMP_MAP_DELETE:
        return WRNMapDelete;
      case QUAL_OMP_MAP_ALWAYS_TO:
        return WRNMapTo | WRNMapAlways;
      case QUAL_OMP_MAP_ALWAYS_FROM:
        return WRNMapFrom | WRNMapAlways;
      case QUAL_OMP_MAP_ALWAYS_TOFROM:
        return WRNMapTofrom | WRNMapAlways;
      case QUAL_OMP_MAP_ALWAYS_ALLOC:
        return WRNMapAlloc | WRNMapAlways;
      case QUAL_OMP_MAP_ALWAYS_RELEASE:
        return WRNMapRelease | WRNMapAlways;
      case QUAL_OMP_MAP_ALWAYS_DELETE:
        return WRNMapDelete | WRNMapAlways;
      default: 
        llvm_unreachable("Unsupported MAP Clause ID");
    }
  };

  void setMapKind(unsigned MK) { MapKind = MK; }
  void setIsMapTo()      { MapKind |= WRNMapTo; }
  void setIsMapFrom()    { MapKind |= WRNMapFrom; }
  void setIsMapTofrom()  { MapKind |= WRNMapTofrom; }
  void setIsMapAlloc()   { MapKind |= WRNMapAlloc; }
  void setIsMapRelease() { MapKind |= WRNMapRelease; }
  void setIsMapDelete()  { MapKind |= WRNMapDelete; }
  void setIsMapAlways()  { MapKind |= WRNMapAlways; }

  unsigned getMapKind()    const { return MapKind; }
  bool getIsMapTo()        const { return MapKind & WRNMapTo; }
  bool getIsMapFrom()      const { return MapKind & WRNMapFrom; }
  bool getIsMapTofrom()    const { return MapKind & WRNMapTofrom; }
  bool getIsMapAlloc()     const { return MapKind & WRNMapAlloc; }
  bool getIsMapRelease()   const { return MapKind & WRNMapRelease; }
  bool getIsMapDelete()    const { return MapKind & WRNMapDelete; }
  bool getIsMapAlways()    const { return MapKind & WRNMapAlways; }
};


//
//   IsDevicePtrItem: OMP IS_DEVICE_PTR clause item
//
class IsDevicePtrItem : public Item
{
  public:
    IsDevicePtrItem(VAR Orig) : Item(Orig) {}
};


//
//   UseDevicePtrItem: OMP USE_DEVICE_PTR clause item
//
class UseDevicePtrItem : public Item
{
  public:
    UseDevicePtrItem(VAR Orig) : Item(Orig) {}
};


//
// These item classes for list-type clauses are not derived from the 
// base "Item" class above.
//            
//   DependItem    (for the depend  clause in task and target constructs)
//   DepSinkItem   (for the depend(sink:<vec>) clause in ordered constructs)
//   AlignedItem   (for the aligned clause in simd constructs)
//
// TODO: we need a better array section representation;
//       the one hard-coded in DependItem only handles 1-dim.
//
class DependItem 
{
  private:
    VAR   Base;           // scalar item or base of array section
    bool  IsIn;           // depend type: true for IN; false for OUT/INOUT
    bool  IsArraySection; // if true, then lb, length, stride below are used
    EXPR  LowerBound;     // null if unspecified
    EXPR  Length;         // null if unspecified
    EXPR  Stride;         // null if unspecified

  public:
    DependItem(VAR V=nullptr) : Base(V), IsIn(true), IsArraySection(false),
      LowerBound(nullptr), Length(nullptr), Stride(nullptr) {}

    void setOrig(VAR V)         { Base = V; }
    void setIsIn(bool Flag)     { IsIn = Flag; }
    void setIsArrSec(bool Flag) { IsArraySection = Flag; }
    void setLb(EXPR Lb)         { LowerBound = Lb;   }
    void setLength(EXPR Len)    { Length = Len;  }
    void setStride(EXPR Str)    { Stride = Str;  }

    VAR  getOrig()      const   { return Base; }
    bool getIsIn()      const   { return IsIn; }
    bool getIsArrSec()  const   { return IsArraySection; }
    EXPR getLb()        const   { return LowerBound; }
    EXPR getLength()    const   { return Length; }
    EXPR getStride()    const   { return Stride; }

    void print(formatted_raw_ostream &OS, bool PrintType=true) const {
      OS << "(" ; 
      getOrig()->printAsOperand(OS, PrintType); 
      OS << ") ";
    }
};

class DepSinkItem 
{
  private:
    EXPR  SinkExpr;       // LoopVar +/- Offset (eg: i-1)
    VAR   LoopVar;        // LoopVar extracted from the SinkExpr
    EXPR  Offset;         // Offset extracted from the SinkExpr

  public:
    DepSinkItem(EXPR E) : SinkExpr(E), LoopVar(nullptr), Offset(nullptr) {}

    void setSinkExpr(EXPR S)    { SinkExpr = S;  }
    void setLoopVar(EXPR LV)    { LoopVar = LV;  }
    void setOffset(EXPR O)      { Offset = O;  }
    EXPR getSinkExpr()  const   { return SinkExpr; }
    EXPR getLoopVar()   const   { return LoopVar; }
    EXPR getOffset()    const   { return Offset; }

    void print(formatted_raw_ostream &OS, bool PrintType=true) const {
      OS << "(" ; 
      getSinkExpr()->printAsOperand(OS, PrintType); 
      OS << ") ";
    }
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

    void print(formatted_raw_ostream &OS, bool PrintType=true) const {
      OS << "("; 
      getOrig()->printAsOperand(OS, PrintType); 
      OS << ", " << getAlign() << ") ";
    }
};

class FlushItem
{
  private:
    VAR  Var;  // global, static, volatile values 

  public:
    FlushItem(VAR V=nullptr) : Var(V) {}
    void setOrig(VAR V)      { Var = V; }
    VAR  getOrig()  const { return Var; }

    void print(formatted_raw_ostream &OS, bool PrintType=true) const {
      OS << "(" ; 
      getOrig()->printAsOperand(OS, PrintType); 
      OS << ") ";
    }
};


//
// The list-type clauses are essentially vectors of the clause items above
//            
template <typename ClauseItem> class Clause
{
  friend class WRegionNode;
  friend class WRegionUtils;
  private:
    typedef typename std::vector<ClauseItem*>       ItemArray;
    typedef typename ItemArray::iterator            Iterator;
    typedef typename ItemArray::const_iterator      ConstIterator;

    ItemArray C;
    int ClauseID;

  public:
    // Constructor
    Clause();

  protected:
    // Create a new item for VAR V and append it to the clause
    void add(VAR V) { ClauseItem *P = new ClauseItem(V); C.push_back(P); }

  public:
    int getClauseID()               const { return ClauseID;     }
    void setClauseID(int ID)              { ClauseID = ID;       }
    bool empty()                    const { return C.empty();    }
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

    void print(formatted_raw_ostream &OS, unsigned Depth=0, 
                                          bool Verbose=false) const;
    // search the clause for 
    ClauseItem *findOrig(const VAR V) { 
      for (auto I : items())
        if (I->getOrig() == V) 
          return I;
      return nullptr;
    }
};

// print routine for template Clause classes
template <typename ClauseItem> void Clause<ClauseItem>::
print(formatted_raw_ostream &OS, unsigned Depth, bool Verbose) const {

  if (!Verbose && !size()) 
    return;  // Don't print absent clause message if !Verbose

  StringRef Name = VPOAnalysisUtils::getClauseName(getClauseID());
  OS.indent(2*Depth) << Name << " clause ";
  if (!size()) {  // this clause was not used in the directive
    OS << "is ABSENT\n";
    return;
  }
  OS << "(size=" << size() << "): " ;

  for (auto I: items())
    I->print(OS);

  OS << "\n";
}

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
typedef Clause<MapItem>          MapClause;
typedef Clause<IsDevicePtrItem>  IsDevicePtrClause;
typedef Clause<UseDevicePtrItem> UseDevicePtrClause;
typedef Clause<DependItem>       DependClause;
typedef Clause<DepSinkItem>      DepSinkClause;
typedef Clause<AlignedItem>      AlignedClause;
typedef Clause<FlushItem>        FlushSet;

typedef std::vector<SharedItem>::iterator       SharedIter;
typedef std::vector<PrivateItem>::iterator      PrivateIter;
typedef std::vector<FirstprivateItem>::iterator FirstprivateIter;
typedef std::vector<LastprivateItem>::iterator  LastprivateIter;
typedef std::vector<ReductionItem>::iterator    ReductionIter;
typedef std::vector<CopyinItem>::iterator       CopyinIter;
typedef std::vector<CopyprivateItem>::iterator  CopyprivateIter;
typedef std::vector<LinearItem>::iterator       LinearIter;
typedef std::vector<UniformItem>::iterator      UniformIter;
typedef std::vector<MapItem>::iterator          MapIter;
typedef std::vector<IsDevicePtrItem>::iterator  IsDevicePtrter;
typedef std::vector<UseDevicePtrItem>::iterator UseDevicePtrter;
typedef std::vector<DependItem>::iterator       DependIter;
typedef std::vector<DepSinkItem>::iterator      DepSinkIter;
typedef std::vector<AlignedItem>::iterator      AlignedIter;
typedef std::vector<FlushItem>::iterator        FlushIter;


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
    WRNScheduleGuidedIterative         = 42,
    WRNScheduleGuidedAnalytical        = 43,

    WRNScheduleOrderedStatic           = 65,
    WRNScheduleOrderedStaticEven       = 66,
    WRNScheduleOrderedDynamic          = 67,
    WRNScheduleOrderedGuided           = 68,
    WRNScheduleOrderedRuntime          = 69,
    WRNScheduleOrderedAuto             = 70,

    WRNScheduleOrderedTrapezoidal      = 71,
    WRNScheduleOrderedStaticGreedy     = 72,
    WRNScheduleOrderedStaticBalanced   = 73,
    WRNScheduleOrderedGuidedIterative  = 74,
    WRNScheduleOrderedGuidedAnalytical = 75,

    WRNScheduleDistributeStatic        = 91,
    WRNScheduleDistributeStaticEven    = 92
} WRNScheduleKind;


class ScheduleClause 
{
  private:
    WRNScheduleKind Kind;
    EXPR            ChunkExpr;
    int             Chunk;
    bool            IsSchedMonotonic:1;
    bool            IsSchedNonmonotonic:1;
    bool            IsSchedSimd:1;

  public:
    void setKind(WRNScheduleKind K)        { Kind = K; }
    void setChunkExpr(EXPR E)              { ChunkExpr = E; }
    void setChunk(int C)                   { Chunk= C; }
    void setIsSchedMonotonic(bool Flag)    { IsSchedMonotonic = Flag; }
    void setIsSchedNonmonotonic(bool Flag) { IsSchedNonmonotonic = Flag; }
    void setIsSchedSimd(bool Flag)         { IsSchedSimd = Flag; }

    // constructor: default schedule when clause is not specified is
    // STATIC with unspecified chunksize or modifiers
    ScheduleClause(): Kind(WRNScheduleStaticEven), ChunkExpr(nullptr), 
                      Chunk(0), IsSchedMonotonic(false), 
                      IsSchedNonmonotonic(false), IsSchedSimd(false) {}
    WRNScheduleKind getKind()      const   { return Kind; }
    EXPR getChunkExpr()            const   { return ChunkExpr; }
    int  getChunk()                const   { return Chunk; }
    bool getIsSchedMonotonic()     const   { return IsSchedMonotonic; }
    bool getIsSchedNonmonotonic()  const   { return IsSchedNonmonotonic; }
    bool getIsSchedSimd()          const   { return IsSchedSimd; }

    void print(formatted_raw_ostream &OS, unsigned Depth=0, 
                                          bool Verbose=false) const;
};

} // End namespace vpo

} // End namespace llvm

#endif
