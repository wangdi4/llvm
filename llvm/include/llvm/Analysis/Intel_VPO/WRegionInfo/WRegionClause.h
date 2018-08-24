#if INTEL_COLLAB // -*- C++ -*-
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
/// \file
/// This file defines the classes that represent OpenMP clauses.
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VPO_WREGIONCLAUSE_H
#define LLVM_ANALYSIS_VPO_WREGIONCLAUSE_H

#include <vector>
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/Intel_VPO/Utils/VPOAnalysisUtils.h"
#include "llvm/IR/Metadata.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"

namespace llvm {

namespace vpo {

class WRegionNode;
class WRegionUtils;

extern void printFnPtr(Function *Fn, formatted_raw_ostream &OS,
                       bool PrintType=true);

// for readability; VAR and EXPR match OpenMP4.1 specs
typedef Value* VAR;
typedef Value* EXPR;
typedef Function* RDECL;

// Tables used for debug printing
extern std::unordered_map<int, StringRef> WRNDefaultName;
extern std::unordered_map<int, StringRef> WRNAtomicName;
extern std::unordered_map<int, StringRef> WRNCancelName;
extern std::unordered_map<int, StringRef> WRNProcBindName;

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
    bool  IsNonPod;  // true for a C++ NONPOD var
    bool  IsVla;     // true for variable-length arrays (C99)
    EXPR  VlaSize;   // size of vla array can be an int expression
    int   ThunkIdx;  // used for task/taskloop codegen
    MDNode *AliasScope; // alias info (loads)  to help registerize private vars
    MDNode *NoAlias;    // alias info (stores) to help registerize private vars

  public:
    Item(VAR Orig) :
      OrigItem(Orig), NewItem(nullptr), ParmItem(nullptr), IsNonPod(false),
      IsVla(false), VlaSize(nullptr), ThunkIdx(-1), AliasScope(nullptr),
      NoAlias(nullptr) {}
    virtual ~Item() = default;

    void setOrig(VAR V)          { OrigItem = V;    }
    void setNew(VAR V)           { NewItem = V;     }
    void setParm(VAR V)          { ParmItem = V;    }
    void setIsNonPod(bool Flag)  { IsNonPod = Flag; }
    void setIsVla(bool Flag)     { IsVla = Flag;    }
    void setVlaSize(EXPR Size)   { VlaSize = Size;  }
    void setThunkIdx(int I)      { ThunkIdx = I;    }
    void setAliasScope(MDNode *M){ AliasScope = M;  }
    void setNoAlias(MDNode *M)   { NoAlias = M;     }

    VAR  getOrig()     const { return OrigItem; }
    VAR  getNew()      const { return NewItem;  }
    VAR  getParm()     const { return ParmItem; }
    bool getIsNonPod() const { return IsNonPod; }
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
    PrivateItem(const Use *Args) :
      Item(nullptr), Constructor(nullptr), Destructor(nullptr) {
      // PRIVATE nonPOD Args are: var, ctor, dtor
      Value *V = cast<Value>(Args[0]);
      setOrig(V);
      V = cast<Value>(Args[1]);
      Constructor = cast<Function>(V);
      V = cast<Value>(Args[2]);
      Destructor = cast<Function>(V);
    }
    void setConstructor(RDECL Ctor) { Constructor = Ctor; }
    void setDestructor(RDECL Dtor)  { Destructor  = Dtor; }
    RDECL getConstructor() const { return Constructor; }
    RDECL getDestructor()  const { return Destructor;  }

    void print(formatted_raw_ostream &OS, bool PrintType=true) const {
      if (getIsNonPod()) {
        OS << "NONPOD(";
        getOrig()->printAsOperand(OS, PrintType);
        OS << ", CTOR: ";
        printFnPtr(getConstructor(), OS, PrintType);
        OS << ", DTOR: ";
        printFnPtr(getDestructor(), OS, PrintType);
        OS << ") ";
      } else  //invoke parent's print function for regular case
        Item::print(OS, PrintType);
    }
};

class LastprivateItem; // forward declaration
class MapItem;         // forward declaration

//
//   FirstprivateItem: OMP FIRSTPRIVATE clause item
//   (cf PAROPT_OMP_FIRSTPRIVATE_NODE)
//
class FirstprivateItem : public Item
{
  private:
    LastprivateItem *InLastprivate; // LastprivateItem with the same opnd
    MapItem *InMap;                 // MapItem with the same opnd
    RDECL CopyConstructor;
    RDECL Destructor;

  public:
    FirstprivateItem(VAR Orig)
        : Item(Orig), InLastprivate(nullptr), InMap(nullptr),
          CopyConstructor(nullptr), Destructor(nullptr) {}
    FirstprivateItem(const Use *Args)
        : Item(nullptr), InLastprivate(nullptr), InMap(nullptr),
          CopyConstructor(nullptr), Destructor(nullptr) {
      // FIRSTPRIVATE nonPOD Args are: var, cctor, dtor
      Value *V = cast<Value>(Args[0]);
      setOrig(V);
      V = cast<Value>(Args[1]);
      CopyConstructor = cast<Function>(V);
      V = cast<Value>(Args[2]);
      Destructor = cast<Function>(V);
    }
    void setInLastprivate(LastprivateItem *LI) { InLastprivate = LI; }
    void setInMap(MapItem *MI) { InMap = MI; }
    void setCopyConstructor(RDECL Cctor) { CopyConstructor = Cctor; }
    void setDestructor(RDECL Dtor)       { Destructor  = Dtor;      }
    LastprivateItem *getInLastprivate() const { return InLastprivate; }
    MapItem *getInMap() const { return InMap; }
    RDECL getCopyConstructor() const { return CopyConstructor; }
    RDECL getDestructor()      const { return Destructor;      }

    void print(formatted_raw_ostream &OS, bool PrintType=true) const {
      if (getIsNonPod()) {
        OS << "NONPOD(";
        getOrig()->printAsOperand(OS, PrintType);
        OS << ", CCTOR: ";
        printFnPtr(getCopyConstructor(), OS, PrintType);
        OS << ", DTOR: ";
        printFnPtr(getDestructor(), OS, PrintType);
        OS << ") ";
      } else  //invoke parent's print function for regular case
        Item::print(OS, PrintType);
    }
};


//
//   LastprivateItem: OMP LASTPRIVATE clausclause item
//   (cf PAROPT_OMP_LASTPRIVATE_NODE)
//
class LastprivateItem : public Item
{
  private:
    bool IsConditional;               // conditional lastprivate
    FirstprivateItem *InFirstprivate; // FirstprivateItem with the same opnd
    RDECL Constructor;
    RDECL CopyAssign;
    RDECL Destructor;

  public:
    LastprivateItem(VAR Orig)
        : Item(Orig), IsConditional(false), InFirstprivate(nullptr),
          Constructor(nullptr), CopyAssign(nullptr), Destructor(nullptr) {}
    LastprivateItem(const Use *Args)
        : Item(nullptr), IsConditional(false), InFirstprivate(nullptr),
          Constructor(nullptr), CopyAssign(nullptr), Destructor(nullptr) {
      // LASTPRIVATE nonPOD Args are: var, ctor, copy-assign, dtor
      Value *V = cast<Value>(Args[0]);
      setOrig(V);
      if (Constant *C = dyn_cast<Constant>(Args[1])) {
        // If a nonpod var is both lastprivate and firstprivate, the ctor in
        // the lastprivate OperanBundle is "i8* null" from clang. This is
        // because the var will not be initialized with a default constructor,
        // but rather with the copy-constructor from its firstprivate clause.
        // In this case, we stay with Constructor=nullptr.
        if (!(C->isNullValue())) {
          Constructor = cast<Function>(C);
        }
      }
      V = cast<Value>(Args[2]);
      CopyAssign = cast<Function>(V);
      V = cast<Value>(Args[3]);
      Destructor = cast<Function>(V);
    }
    void setIsConditional(bool B) { IsConditional = B; }
    void setInFirstprivate(FirstprivateItem *FI) { InFirstprivate = FI; }
    void setConstructor(RDECL Ctor) { Constructor = Ctor; }
    void setCopyAssign(RDECL Cpy) { CopyAssign = Cpy;         }
    void setDestructor(RDECL Dtor) { Destructor  = Dtor; }
    bool getIsConditional() const { return IsConditional; }
    FirstprivateItem *getInFirstprivate() const { return InFirstprivate; }
    RDECL getConstructor() const { return Constructor; }
    RDECL getCopyAssign() const { return CopyAssign; }
    RDECL getDestructor() const { return Destructor; }

    void print(formatted_raw_ostream &OS, bool PrintType=true) const {
      if (getIsNonPod()) {
        OS << "NONPOD(";
        getOrig()->printAsOperand(OS, PrintType);
        OS << ", CTOR: ";
        printFnPtr(getConstructor(), OS, PrintType);
        OS << ", COPYASSIGN: ";
        printFnPtr(getCopyAssign(), OS, PrintType);
        OS << ", DTOR: ";
        printFnPtr(getDestructor(), OS, PrintType);
        OS << ") ";
      } else  //invoke parent's print function for regular case
        Item::print(OS, PrintType);
    }
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
    WRNReductionAdd,
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
    bool  IsInReduction; // is from an IN_REDUCTION clause (task/taskloop)

    // NOTE: Combiner and Initializer are Function*'s from UDR. However,
    // currently lib/Transforms/Intel_VPO/Vecopt/VPOAvrLLVMCodeGen.cpp has code
    // that uses these fields to store instructions that initialize/combine
    // the reduction variable in non-UDR cases. After that code is cleaned up,
    // we can change Value* to Function* below.
    Value *Combiner;
    Value *Initializer;

  public:
    ReductionItem(VAR Orig, WRNReductionKind Op=WRNReductionError): Item(Orig),
      Ty(Op), IsUnsigned(false), IsInReduction(false), Combiner(nullptr),
      Initializer(nullptr) {}

    static WRNReductionKind getKindFromClauseId(int Id) {
      switch(Id) {
        case QUAL_OMP_REDUCTION_ADD:
        case QUAL_OMP_INREDUCTION_ADD:
          return WRNReductionAdd;
        case QUAL_OMP_REDUCTION_SUB:
        case QUAL_OMP_INREDUCTION_SUB:
          return WRNReductionSub;
        case QUAL_OMP_REDUCTION_MUL:
        case QUAL_OMP_INREDUCTION_MUL:
          return WRNReductionMult;
        case QUAL_OMP_REDUCTION_AND:
        case QUAL_OMP_INREDUCTION_AND:
          return WRNReductionAnd;
        case QUAL_OMP_REDUCTION_OR:
        case QUAL_OMP_INREDUCTION_OR:
          return WRNReductionOr;
        case QUAL_OMP_REDUCTION_BXOR:
        case QUAL_OMP_INREDUCTION_BXOR:
          return WRNReductionBxor;
        case QUAL_OMP_REDUCTION_BAND:
        case QUAL_OMP_INREDUCTION_BAND:
          return WRNReductionBand;
        case QUAL_OMP_REDUCTION_BOR:
        case QUAL_OMP_INREDUCTION_BOR:
          return WRNReductionBor;
        case QUAL_OMP_REDUCTION_MAX:
        case QUAL_OMP_INREDUCTION_MAX:
          return WRNReductionMax;
        case QUAL_OMP_REDUCTION_MIN:
        case QUAL_OMP_INREDUCTION_MIN:
          return WRNReductionMin;
        case QUAL_OMP_REDUCTION_UDR:
        case QUAL_OMP_INREDUCTION_UDR:
          return WRNReductionUdr;
        default:
          llvm_unreachable("Unsupported Reduction Clause ID");
      }
    };

    // There is no need to deal with the INREDUCTION variants, as the main
    // purpose of this routine is to support the getOpName() method to recover
    // the name of the reduction operation, which is the same for both
    // REDUCTION and INREDUCTION.
    static int getClauseIdFromKind(WRNReductionKind Kind) {
      switch(Kind) {
        case WRNReductionAdd:
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
          return QUAL_OMP_REDUCTION_BXOR;
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

    void setType(WRNReductionKind Op) { Ty = Op;             }
    void setIsUnsigned(bool B)        { IsUnsigned = B;      }
    void setIsInReduction(bool B)     { IsInReduction = B;   }
    void setCombiner(Value *Comb)     { Combiner = Comb;     }
    void setInitializer(Value *Init)  { Initializer = Init;  }
    WRNReductionKind getType() const { return Ty;            }
    bool getIsUnsigned()       const { return IsUnsigned;    }
    bool getIsInReduction()    const { return IsInReduction; }
    Value *getCombiner()       const { return Combiner;      }
    Value *getInitializer()    const { return Initializer;   }

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
    EXPR Step;

    // No need for ctor/dtor because OrigItem is either pointer or array base

  public:
    LinearItem(VAR Orig) : Item(Orig), Step(nullptr) {}
    void setStep(EXPR S) { Step = S; }
    EXPR getStep() const { return Step; }

    // Specialized print() to output the stride as well
    void print(formatted_raw_ostream &OS, bool PrintType=true) const {
      OS << "(";
      getOrig()->printAsOperand(OS, PrintType);
      OS << ", ";
      auto *Step = getStep();
      assert(Step && "Null 'Step' for LINEAR clause.");
      Step->printAsOperand(OS, PrintType);
      OS << ") ";
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

//  To support an aggregate object in a MAP clause, a chain of triples is used.
//  Each triple has a Base pointer, a Section pointer, and Size. Two classes
//  are defined:
//     'MapAggrTy'  represents a triple (BasePtr, SectionPtr, Size)
//     'MapChainTy' represents a chain of triples (ie, a vector of MapAggrTy*)
//
//  For example, given the struct S1 and the pointer ps below:
//
//  struct S1 {
//    int y;
//    double d[50];
//    struct S1 *next;
//  };
//  S1 *ps;
//
//  To carry out the semantics of MAP(ps->y), the libomptarget runtime needs a
//  triple that holds these three pieces of information:
//    * Base Pointer:           ps
//    * Section Pointer:        &(ps->y)
//    * Size of y:              4
//
//  For a longer pointer chain such MAP(ps->next->next->y), the runtime needs a
//  triple for each component pointer dereference, as shown below:
//
//              Base Pointer      Section Pointer         Size in bytes
//              ============      ===============         =============
//   Triple#1:   ps                &(ps->next)             sizeof(S1*) = 8
//   Triple#2:   &(ps->next)       &(ps->next->next)       sizeof(S1*) = 8
//   Triple#3:   &(ps->next->next) &(ps->next->next->y)    sizeof(int) = 4
//
//  Here's an example when an array is involved. For MAP(ps->next->d) we need:
//   Triple#1:   ps                &(ps->next)             sizeof(S1*) = 8
//   Triple#2:   &(ps->next)       &(ps->next->d[0])       50*sizeof(double)
//
//  It's similar for an array section. For MAP(ps->next->d[17:25]) we need:
//   Triple#1:   ps                &(ps->next)             sizeof(S1*) = 8
//   Triple#2:   &(ps->next)       &(ps->next->d[17])      25*sizeof(double)
//
class MapAggrTy
{
private:
  Value *BasePtr;
  Value *SectionPtr;
  Value *Size;
public:
  MapAggrTy(Value *BP, Value *SP, Value *Sz) : BasePtr(BP), SectionPtr(SP),
                                               Size(Sz) {}
  void setBasePtr(Value *BP) { BasePtr = BP; }
  void setSectionPtr(Value *SP) { SectionPtr = SP; }
  void setSize(Value *Sz) { Size = Sz; }
  Value *getBasePtr() const { return BasePtr; }
  Value *getSectionPtr() const { return SectionPtr; }
  Value *getSize() const { return Size; }
};

typedef SmallVector<MapAggrTy*, 2> MapChainTy;

//
//   MapItem: OMP MAP clause item
//
class MapItem : public Item
{
private:
  unsigned MapKind;                 // bit vector for map kind and modifiers
  FirstprivateItem *InFirstprivate; // FirstprivateItem with the same opnd
  MapChainTy MapChain;

public:
  enum WRNMapKind {
    WRNMapNone    = 0x0000,
    WRNMapTo      = 0x0001,
    WRNMapFrom    = 0x0002,
    WRNMapAlways  = 0x0004,
    WRNMapDelete  = 0x0008,
    WRNMapAlloc   = 0x0010,
    WRNMapRelease = 0x0020,
  } WRNMapKind;

  MapItem(VAR Orig) : Item(Orig), MapKind(0), InFirstprivate(nullptr) {}
  MapItem(MapAggrTy* Aggr): Item(nullptr), MapKind(0), InFirstprivate(nullptr){
    MapChain.push_back(Aggr);
  }

  const MapChainTy &getMapChain() const { return MapChain; }
        MapChainTy &getMapChain()       { return MapChain; }
  bool getIsMapChain() const { return MapChain.size() > 0; }

  static unsigned getMapKindFromClauseId(int Id) {
    switch(Id) {
      case QUAL_OMP_TO:
      case QUAL_OMP_MAP_TO:
        return WRNMapTo;
      case QUAL_OMP_FROM:
      case QUAL_OMP_MAP_FROM:
        return WRNMapFrom;
      case QUAL_OMP_MAP_TOFROM:
        return WRNMapFrom | WRNMapTo;
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
        return WRNMapTo | WRNMapFrom | WRNMapAlways;
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
  void setIsMapTofrom() { MapKind |= WRNMapFrom | WRNMapTo; }
  void setIsMapAlloc()   { MapKind |= WRNMapAlloc; }
  void setIsMapRelease() { MapKind |= WRNMapRelease; }
  void setIsMapDelete()  { MapKind |= WRNMapDelete; }
  void setIsMapAlways()  { MapKind |= WRNMapAlways; }
  void setInFirstprivate(FirstprivateItem *FI) { InFirstprivate = FI; }

  unsigned getMapKind()    const { return MapKind; }
  bool getIsMapTo()        const { return MapKind & WRNMapTo; }
  bool getIsMapFrom()      const { return MapKind & WRNMapFrom; }
  bool getIsMapTofrom() const { return MapKind & (WRNMapFrom | WRNMapTo); }
  bool getIsMapAlloc()     const { return MapKind & WRNMapAlloc; }
  bool getIsMapRelease()   const { return MapKind & WRNMapRelease; }
  bool getIsMapDelete()    const { return MapKind & WRNMapDelete; }
  bool getIsMapAlways()    const { return MapKind & WRNMapAlways; }
  FirstprivateItem *getInFirstprivate() const { return InFirstprivate; }

  void print(formatted_raw_ostream &OS, bool PrintType=true) const {
    if (getIsMapChain()) {
      OS << "CHAIN(" ;
      for (unsigned I=0; I < MapChain.size(); ++I) {
        MapAggrTy *Aggr = MapChain[I];
        Value *BasePtr = Aggr->getBasePtr();
        Value *SectionPtr = Aggr->getSectionPtr();
        Value *Size = Aggr->getSize();
        OS << "<" ;
        BasePtr->printAsOperand(OS, PrintType);
        OS << ", ";
        SectionPtr->printAsOperand(OS, PrintType);
        OS << ", ";
        Size->printAsOperand(OS, PrintType);
        OS << "> ";
      }
      OS << ") ";
    } else {
      OS << "(" ;
      getOrig()->printAsOperand(OS, PrintType);
      OS << ") ";
    }
  }
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
// Clang collapses the 'n' loops for 'ordered(n)'. So VPO always
// receives a single EXPR for depend(sink:sink_expr), which is already in
// the form ' IV +/- offset'.
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

// TODO: Delete extra fields. Only SinkExpr is used at the moment.
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
    /// Delete all clause items for the Clause.
    ~Clause() {
      for (auto *CI : C)
        delete CI;
      C.clear();
    }

  protected:
    /// Create a new item for VAR V and append it to the clause
    void add(VAR V) { ClauseItem *P = new ClauseItem(V); C.push_back(P); }
    void add(ClauseItem *P) { C.push_back(P); }

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

    bool print(formatted_raw_ostream &OS, unsigned Depth=0,
                                          unsigned Verbosity=1) const;
    // search the clause for
    ClauseItem *findOrig(const VAR V) {
      for (auto I : items())
        if (I->getOrig() == V)
          return I;
      return nullptr;
    }
};

/// \brief Print routine for template list-type Clause classes.
/// Returns true iff something is printed
template <typename ClauseItem> bool Clause<ClauseItem>::
print(formatted_raw_ostream &OS, unsigned Depth, unsigned Verbosity) const {

  if (Verbosity==0 && !size())
    return false;  // Don't print absent clause message if Verbosity==0

  StringRef Name = VPOAnalysisUtils::getClauseName(getClauseID());
  OS.indent(2*Depth) << Name << " clause";
  if (!size()) {  // this clause was not used in the directive
    OS << ": UNSPECIFIED\n";
    return true;
  }
  OS << " (size=" << size() << "): " ;

  for (auto I: items())
    I->print(OS);

  OS << "\n";
  return true;
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

    bool isDistSchedule() const {
      return Kind == WRNScheduleDistributeStatic ||
             Kind == WRNScheduleDistributeStaticEven;
    }
    bool print(formatted_raw_ostream &OS, unsigned Depth=0,
                                          unsigned Verbosity=1) const;
};

} // End namespace vpo

} // End namespace llvm

#endif // LLVM_ANALYSIS_VPO_WREGIONCLAUSE_H
#endif // INTEL_COLLAB
