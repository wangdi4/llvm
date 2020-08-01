#if INTEL_COLLAB // -*- C++ -*-
//===----------------- WRegionClause.h - Clauses ----------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// This file defines the classes that represent OpenMP clauses.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_VPO_WREGIONCLAUSE_H
#define LLVM_ANALYSIS_VPO_WREGIONCLAUSE_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/VPO/Utils/VPOAnalysisUtils.h"
#include "llvm/IR/Metadata.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/CommandLine.h" // INTEL
#include <tuple>
#include <vector>
#if INTEL_CUSTOMIZATION
#include "llvm/Analysis/Intel_LoopAnalysis/IR/RegDDRef.h"
#include <type_traits>

using HVAR = llvm::loopopt::RegDDRef *;
using HEXPR = llvm::loopopt::RegDDRef *;

#endif // INTEL_CUSTOMIZATION

namespace llvm {

namespace vpo {

extern cl::opt<bool> UseDevicePtrIsDefaultByRef;
class WRegionNode;
class WRegionUtils;

extern void printFnPtr(Function *Fn, formatted_raw_ostream &OS,
                       bool PrintType=true);

// for readability; VAR and EXPR match OpenMP4.1 specs
typedef Value* VAR;
typedef Value* EXPR;
typedef Function* RDECL;
#if INTEL_CUSTOMIZATION
enum IRKind { LLVMIR, HIR };
template <IRKind IR>
    using VarType = typename std::conditional<IR == HIR, HVAR, VAR>::type;
template <IRKind IR>
    using ExprType = typename std::conditional<IR == HIR, HEXPR, EXPR>::type;
#endif // INTEL_CUSTOMIZATION

// Tables used for debug printing
extern DenseMap<int, StringRef> WRNDefaultName;
extern DenseMap<int, StringRef> WRNDefaultmapBehaviorName;
extern DenseMap<int, StringRef> WRNDefaultmapCategoryName;
extern DenseMap<int, StringRef> WRNAtomicName;
extern DenseMap<int, StringRef> WRNCancelName;
extern DenseMap<int, StringRef> WRNProcBindName;
extern DenseMap<int, StringRef> WRNLoopBindName;
extern DenseMap<int, StringRef> WRNLoopOrderName;

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

  public:
    /// Enum of various derived Item kinds for LLVM's RTTI (isa, cast etc.)
    enum ItemKind {
      IK_Shared,
      IK_Private,
      IK_Firstprivate,
      IK_Lastprivate,
      IK_Reduction,
      IK_Copyin,
      IK_Copyprivate,
      IK_Linear,
      IK_Uniform,
      IK_Map,
      IK_IsDevicePtr,
      IK_UseDevicePtr
    };

  private :
    VAR   OrigItem;  // original var
    Type *OrigItemElemType; // original var type/ element type if PointerType
#if INTEL_CUSTOMIZATION
    HVAR HOrigItem;       // original var for HIR
    bool IsF90DopeVector; // true for a F90 dope vector
    EXPR F90DVNumElements; // number of elements in the F90 DV
    bool IsWILocal = false;
#endif // INTEL_CUSTOMIZATION
    VAR   NewItem;   // new version (eg private) of the var. For tasks, it's
                     // the offset into the thunk for the new var
    VAR   OrigGEP;   // TASK only: offset in thunk for the addr of orig var
    bool  IsByRef;   // true for a by-reference var
    bool  IsNonPod;  // true for a C++ NONPOD var
    bool  IsVla;     // true for variable-length arrays (C99)
    bool IsPointerToPointer; // true if var is a pointer to pointer (e.g. i32**)
    EXPR ThunkBufferSize; // Tasks: size in bytes of the space needed for the
                          // item, in the buffer at the end of task thunk (e.g.
                          // C99 VLAs)
    EXPR NewThunkBufferSize; // Tasks: ThunkBufferSize readable inside the
                             // outlined function
    int PrivateThunkIdx;     // Tasks: index for the var in task's private thunk
    int SharedThunkIdx;      // Tasks: index for the var in task's shared thunk
    EXPR ThunkBufferOffset;  // Tasks: offset to the buffer for the item in the
                             // task's thunk
    MDNode *AliasScope; // alias info (loads)  to help registerize private vars
    MDNode *NoAlias;    // alias info (stores) to help registerize private vars
    const ItemKind Kind; // Item kind for LLVM's RTTI
    // Utility function.
    // TODO: OPAQUEPOINTER: VTy Will be passed as a Item String by Front End
    // after Opaque Pointers are introduced.
    void setValueType(VAR &V, Type *&VTy) {
      if (V)
        VTy = isa<PointerType>(V->getType())
                  ? V->getType()->getPointerElementType()
                  : V->getType();
      else
        VTy = nullptr;
      return;
    }

  public:
    Item(VAR Orig, ItemKind K)
#if INTEL_CUSTOMIZATION
        : OrigItem(Orig), HOrigItem(nullptr), IsF90DopeVector(false),
          F90DVNumElements(nullptr), NewItem(nullptr), OrigGEP(nullptr),
#else
        : OrigItem(Orig), NewItem(nullptr), OrigGEP(nullptr),
#endif // INTEL_CUSTOMIZATION
          IsByRef(false), IsNonPod(false), IsVla(false),
          // TODO: Initialize IsPointerToPointer to false when clang changes
          // to emit PTR_TO_PTR are in.
          IsPointerToPointer(UseDevicePtrIsDefaultByRef),
          ThunkBufferSize(nullptr), NewThunkBufferSize(nullptr),
          PrivateThunkIdx(-1), SharedThunkIdx(-1), ThunkBufferOffset(nullptr),
          AliasScope(nullptr), NoAlias(nullptr), Kind(K) {
      setValueType(OrigItem, OrigItemElemType);
    }
    virtual ~Item() = default;

    void setOrig(VAR V) {
      OrigItem = V;
      setValueType(OrigItem, OrigItemElemType);
    }
    void setOrigElemType(Type *VTy) { OrigItemElemType = VTy; }
    void setNew(VAR V) { NewItem = V; }
    void setOrigGEP(VAR V)        { OrigGEP = V;        }
    void setIsByRef(bool Flag)    { IsByRef = Flag;     }
    void setIsNonPod(bool Flag)   { IsNonPod = Flag;    }
    void setIsVla(bool Flag)      { IsVla = Flag;       }
    void setIsPointerToPointer(bool Flag) { IsPointerToPointer = Flag; }
    void setThunkBufferSize(EXPR Size) { ThunkBufferSize = Size; }
    void setNewThunkBufferSize(EXPR Size) { NewThunkBufferSize = Size; }
    void setPrivateThunkIdx(int I) { PrivateThunkIdx = I; }
    void setSharedThunkIdx(int I) { SharedThunkIdx = I; }
    void setThunkBufferOffset(EXPR O) { ThunkBufferOffset = O; }
    void setAliasScope(MDNode *M) { AliasScope = M;     }
    void setNoAlias(MDNode *M)    { NoAlias = M;        }

    VAR getOrig()           const { return OrigItem;       }
    Type *getOrigElemType() const { return OrigItemElemType; }
    VAR getNew()            const { return NewItem;        }
    VAR getOrigGEP()        const { return OrigGEP;        }
    bool getIsByRef()       const { return IsByRef;        }
    bool getIsNonPod()      const { return IsNonPod;       }
    bool getIsVla()         const { return IsVla;          }
    bool getIsPointerToPointer() const { return IsPointerToPointer; }
    EXPR getThunkBufferSize() const { return ThunkBufferSize; }
    EXPR getNewThunkBufferSize() const { return NewThunkBufferSize; }
    int getPrivateThunkIdx() const { return PrivateThunkIdx; }
    int getSharedThunkIdx() const { return SharedThunkIdx; }
    EXPR getThunkBufferOffset() const { return ThunkBufferOffset; }
    MDNode *getAliasScope() const { return AliasScope;     }
    MDNode *getNoAlias()    const { return NoAlias;        }
    ItemKind getKind()      const { return Kind;           }
#if INTEL_CUSTOMIZATION
    void setHOrig(HVAR V)         { HOrigItem = V;         }
    template <IRKind IR = LLVMIR> VarType<IR> getOrig() const;
    void setIsF90DopeVector(bool Flag) { IsF90DopeVector = Flag;  }
    bool getIsF90DopeVector()  const   { return IsF90DopeVector;  }
    void setF90DVNumElements(EXPR Size){ F90DVNumElements = Size; }
    EXPR getF90DVNumElements() const   { return F90DVNumElements; }
    void setIsWILocal(bool Flag)       { IsWILocal = Flag; }
    bool getIsWILocal()          const { return IsWILocal; }
#endif // INTEL_CUSTOMIZATION

    void printOrig(formatted_raw_ostream &OS, bool PrintType=true) const {

#if INTEL_CUSTOMIZATION
      if (getIsF90DopeVector()) {
        if (getIsByRef())
          OS << "F90_DV,";
        else
          OS << "F90_DV(";
      }
#endif // INTEL_CUSTOMIZATION
      if (getIsByRef())
        OS << "BYREF(";
      if (getIsPointerToPointer())
        OS << "PTR_TO_PTR(";
#if INTEL_CUSTOMIZATION
      if (HOrigItem)
        HOrigItem->print(OS, PrintType);
      else
#endif // INTEL_CUSTOMIZATION
      getOrig()->printAsOperand(OS, PrintType);
      if (getIsPointerToPointer())
        OS << ")";
      if (getIsByRef())
        OS << ")";
#if INTEL_CUSTOMIZATION
      else if (getIsF90DopeVector())
        OS << ")";
#endif // INTEL_CUSTOMIZATION
    }

    virtual void print(formatted_raw_ostream &OS, bool PrintType=true) const {
#if INTEL_CUSTOMIZATION
      if (getIsF90DopeVector()) {
        OS << "F90_DV";
        if (getIsByRef() || getIsPointerToPointer())
          OS << ",";
      }
#endif // INTEL_CUSTOMIZATION
      if (getIsByRef()) {
        OS << "BYREF";
        if (getIsPointerToPointer())
          OS << ",";
      }
      if (getIsPointerToPointer())
        OS << "PTR_TO_PTR";
      OS << "(" ;
#if INTEL_CUSTOMIZATION
      if (HOrigItem)
        HOrigItem->print(OS, PrintType);
      else
#endif // INTEL_CUSTOMIZATION
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
    SharedItem(VAR Orig) : Item(Orig, IK_Shared), IsPassedDirectly(false) {}
    void setIsPassedDirectly(bool Flag) { IsPassedDirectly = Flag; }
    bool getIsPassedDirectly() const { return IsPassedDirectly; }
    static bool classof(const Item *I) { return I->getKind() == IK_Shared; }
};

class AllocateItem; // forward declaration

//
//   PrivateItem: OMP PRIVATE clause item
//   (cf PAROPT_OMP_PRIVATE_NODE)
//
class PrivateItem : public Item
{
  private:
    AllocateItem *InAllocate; // AllocateItem with the same opnd
    RDECL Constructor;
    RDECL Destructor;

  public:
    PrivateItem(VAR Orig)
        : Item(Orig, IK_Private), InAllocate(nullptr), Constructor(nullptr),
          Destructor(nullptr) {}
    PrivateItem(const Use *Args)
        : Item(nullptr, IK_Private), InAllocate(nullptr), Constructor(nullptr),
          Destructor(nullptr) {
      // PRIVATE nonPOD Args are: var, ctor, dtor
      Value *V = cast<Value>(Args[0]);
      setOrig(V);
      V = cast<Value>(Args[1]);
      Constructor = cast<Function>(V);
      V = cast<Value>(Args[2]);
      Destructor = cast<Function>(V);
    }
    void setInAllocate(AllocateItem *AI) { InAllocate = AI; }
    void setConstructor(RDECL Ctor) { Constructor = Ctor; }
    void setDestructor(RDECL Dtor)  { Destructor  = Dtor; }
    AllocateItem *getInAllocate() const { return InAllocate; }
    RDECL getConstructor() const { return Constructor; }
    RDECL getDestructor()  const { return Destructor;  }

    void print(formatted_raw_ostream &OS, bool PrintType=true) const {
      if (getIsNonPod()) {
        OS << "NONPOD(";
        printOrig(OS, PrintType);
        OS << ", CTOR: ";
        printFnPtr(getConstructor(), OS, PrintType);
        OS << ", DTOR: ";
        printFnPtr(getDestructor(), OS, PrintType);
        OS << ") ";
      } else  //invoke parent's print function for regular case
        Item::print(OS, PrintType);
    }
    static bool classof(const Item *I) { return I->getKind() == IK_Private; }
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
    AllocateItem *InAllocate;       // AllocateItem with the same opnd
    RDECL CopyConstructor;
    RDECL Destructor;

    // True for firstprivate() pointer items, such as:
    //   int *p
    //   #pragma omp ... firstprivate(p)
    //
    // It is set to true, only iff transformation pass got rid of
    // the double pointer reference in the clause. For "omp target"
    // regions this attribute is used to guarantee mapping firstprivate()
    // pointers as zero-sized pointers (note that this is different from
    // mapping a pointer as literal, because host to device
    // pointer translation is still applied for zero-sized pointers).
    bool IsPointer = false;

  public:
    FirstprivateItem(VAR Orig)
        : Item(Orig, IK_Firstprivate), InLastprivate(nullptr), InMap(nullptr),
          InAllocate(nullptr), CopyConstructor(nullptr), Destructor(nullptr) {}
    FirstprivateItem(const Use *Args)
        : Item(nullptr, IK_Firstprivate), InLastprivate(nullptr),
          InMap(nullptr), InAllocate(nullptr), CopyConstructor(nullptr),
          Destructor(nullptr) {
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
    void setInAllocate(AllocateItem *AI) { InAllocate = AI; }
    void setCopyConstructor(RDECL Cctor) { CopyConstructor = Cctor; }
    void setDestructor(RDECL Dtor)       { Destructor  = Dtor;      }
    LastprivateItem *getInLastprivate() const { return InLastprivate; }
    MapItem *getInMap() const { return InMap; }
    AllocateItem *getInAllocate() const { return InAllocate; }
    RDECL getCopyConstructor() const { return CopyConstructor; }
    RDECL getDestructor()      const { return Destructor;      }
    void setIsPointer(bool Val) { IsPointer = Val; }
    bool getIsPointer() const { return IsPointer; }

    void print(formatted_raw_ostream &OS, bool PrintType=true) const {
      if (getIsNonPod()) {
        OS << "NONPOD(";
        printOrig(OS, PrintType);
        OS << ", CCTOR: ";
        printFnPtr(getCopyConstructor(), OS, PrintType);
        OS << ", DTOR: ";
        printFnPtr(getDestructor(), OS, PrintType);
        OS << ") ";
      } else  //invoke parent's print function for regular case
        Item::print(OS, PrintType);
    }
    static bool classof(const Item *I) {
      return I->getKind() == IK_Firstprivate;
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
    AllocateItem *InAllocate;         // AllocateItem with the same opnd
    RDECL Constructor;
    RDECL CopyAssign;
    RDECL Destructor;

  public:
    LastprivateItem(VAR Orig)
        : Item(Orig, IK_Lastprivate), IsConditional(false),
          InFirstprivate(nullptr), InAllocate(nullptr), Constructor(nullptr),
          CopyAssign(nullptr), Destructor(nullptr) {}
    LastprivateItem(const Use *Args)
        : Item(nullptr, IK_Lastprivate), IsConditional(false),
          InFirstprivate(nullptr), InAllocate(nullptr), Constructor(nullptr),
          CopyAssign(nullptr), Destructor(nullptr) {
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
    void setInAllocate(AllocateItem *AI) { InAllocate = AI; }
    void setConstructor(RDECL Ctor) { Constructor = Ctor; }
    void setCopyAssign(RDECL Cpy) { CopyAssign = Cpy;         }
    void setDestructor(RDECL Dtor) { Destructor  = Dtor; }
    bool getIsConditional() const { return IsConditional; }
    FirstprivateItem *getInFirstprivate() const { return InFirstprivate; }
    AllocateItem *getInAllocate() const { return InAllocate; }
    RDECL getConstructor() const { return Constructor; }
    RDECL getCopyAssign() const { return CopyAssign; }
    RDECL getDestructor() const { return Destructor; }

    void print(formatted_raw_ostream &OS, bool PrintType=true) const {
      if (getIsNonPod()) {
        OS << "NONPOD(";
        printOrig(OS, PrintType);
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
    static bool classof(const Item *I) {
      return I->getKind() == IK_Lastprivate;
    }
};

/// Contains information about an array section operand. Example:
///
/// \code
///   int x[10], (*yarrptr)[20];
///   #pragma omp for reduction (+: x[1:5], yarrptr[1][2:7])
/// \endcode
///
/// The incoming IR will look like:
///
/// \code
///   call llvm.region.entry() [ ... "QUAL.OMP.REDUCTION.ADD:ARRSECT"([10 x
///   i32]* x, 1, 1, 5, 1), "QUAL.OMP.REDUCTION.ADD:ARRSECT"([20 x i32]**
///   yarrptr, 2, 1, 1, 1, 2, 7, 1) ...]
/// \endcode
///
class ArraySectionInfo {
private:
  // This Vector is populated during WRegionCollection analysis.
  SmallVector<std::tuple<Value *, Value *, Value *>, 2>
      ArraySectionDims; ///< Vector of tuples of {LowerBound, Size, Stride}
                        ///< from higher to lower dimensions.
                        ///< (`x: {{1,5,1}}, yarrptr: {{1,1,1}, {2,7,1}}`)

  // The following fields are populated during the actual transformation, and
  // not WRegionCollection, because they may need Insertion of Instructions.
  Value *Size;        ///< Size (`x: 5, yarrptr: 7`).
  GlobalVariable *GVSize; ///< Global variable for variable sized array
                          ///< [section] size used by fast reduction.
  Value *Offset;      ///< Starting offset (`x: 1, yarrptr: 22`).
  Type *ElementType;  ///< Type of one element (`x: i32, yarrptr: i32`).
  bool BaseIsPointer; ///< Is base a pointer (`x: false, yarrptr: true`).

public:
  ArraySectionInfo()
      : Size(nullptr), GVSize(nullptr), Offset(nullptr), ElementType(nullptr),
        BaseIsPointer(false) {}

  void addDimension(const std::tuple<Value *, Value *, Value *> &Dim) {
    ArraySectionDims.push_back(Dim);
  }

  const SmallVectorImpl<std::tuple<Value *, Value *, Value *>> &
  getArraySectionDims() const {
    return ArraySectionDims;
  }

  Value *getSize() const { return Size; }
  void setSize(Value *Sz) { Size = Sz; }
  Value *getOffset() const { return Offset; }
  void setOffset(Value *Off) { Offset = Off; }
  Type *getElementType() const { return ElementType; }
  void setElementType(Type *Ty) { ElementType = Ty; }
  bool getBaseIsPointer() const { return BaseIsPointer; }
  void setBaseIsPointer(bool IsPtr) { BaseIsPointer = IsPtr; }
  GlobalVariable *getGVSize() const { return GVSize; }
  void setGVSize(GlobalVariable *Sz) { GVSize = Sz; }

  void print(formatted_raw_ostream &OS, bool PrintType = true) const;
  void print(raw_ostream &OS, bool PrintType = true) const;

  // Get tuples from Args and populate the ArraySectionDims
  void populateArraySectionDims(const Use *Args, unsigned NumArgs);
};

//
//   ReductionItem: OMP REDUCTION clause item
//   (cf PAROPT_OMP_REDUCTION_NODE)
//
class ReductionItem : public Item {
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
#if INTEL_CUSTOMIZATION
    WRNReductionEqv,  // Fortran
    WRNReductionNeqv, // Fortran
#endif // INTEL_CUSTOMIZATION
    WRNReductionMax,
    WRNReductionMin,
    WRNReductionUdr   // user-defined reduction
  } WRNReductionKind;

  private:
    WRNReductionKind Ty; // reduction operation
    bool  IsUnsigned;    // for min/max reduction; default is signed min/max
    bool  IsComplex;     // complex type
    bool  IsInReduction; // is from an IN_REDUCTION clause (task/taskloop)

    // TODO: Combiner and Initializer are Function*'s from UDR.
    //       We should change Value* to Function* below.
    RDECL Combiner;
    RDECL Initializer;
    RDECL Constructor;
    RDECL Destructor;
    ArraySectionInfo ArrSecInfo;
    AllocateItem *InAllocate; // AllocateItem with the same opnd

  public:
    ReductionItem(VAR Orig, WRNReductionKind Op = WRNReductionError)
        : Item(Orig, IK_Reduction), Ty(Op), IsUnsigned(false), IsComplex(false),
          IsInReduction(false), Combiner(nullptr), Initializer(nullptr),
          Constructor(nullptr), Destructor(nullptr), InAllocate(nullptr) {}
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
#if INTEL_CUSTOMIZATION
        case QUAL_OMP_REDUCTION_EQV:
        case QUAL_OMP_INREDUCTION_EQV:
          return WRNReductionEqv;
        case QUAL_OMP_REDUCTION_NEQV:
        case QUAL_OMP_INREDUCTION_NEQV:
          return WRNReductionNeqv;
#endif // INTEL_CUSTOMIZATION
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
#if INTEL_CUSTOMIZATION
        case WRNReductionEqv:
          return QUAL_OMP_REDUCTION_EQV;
        case WRNReductionNeqv:
          return QUAL_OMP_REDUCTION_NEQV;
#endif // INTEL_CUSTOMIZATION
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
    void setIsComplex(bool B)         { IsComplex = B;       }
    void setIsInReduction(bool B)     { IsInReduction = B;   }
    void setCombiner(RDECL Comb)      { Combiner = Comb;     }
    void setInitializer(RDECL Init)   { Initializer = Init;  }
    void setConstructor(RDECL Ctor)   { Constructor = Ctor;  }
    void setDestructor(RDECL Dtor)    { Destructor = Dtor;   }
    void setInAllocate(AllocateItem *AI) { InAllocate = AI;  }

    WRNReductionKind getType() const { return Ty;            }
    bool getIsUnsigned()       const { return IsUnsigned;    }
    bool getIsComplex()        const { return IsComplex;     }
    bool getIsInReduction()    const { return IsInReduction; }
    RDECL getCombiner()        const { return Combiner;      }
    RDECL getInitializer()     const { return Initializer;   }
    RDECL getConstructor()     const { return Constructor;   }
    RDECL getDestructor()      const { return Destructor;    }
    AllocateItem *getInAllocate() const { return InAllocate; }

    ArraySectionInfo &getArraySectionInfo() { return ArrSecInfo; }
    const ArraySectionInfo &getArraySectionInfo() const { return ArrSecInfo; }
    bool getIsArraySection() const {
      return !ArrSecInfo.getArraySectionDims().empty();
    };

    // Return a string for the reduction operation, such as "ADD" and "MUL"
    StringRef getOpName() const {
      int ClauseId = getClauseIdFromKind(Ty);
      return VPOAnalysisUtils::getReductionOpName(ClauseId);
    };

    // Don't use the default print() from the base class "Item", because
    // we need to print the Reduction operation too.
    void print(formatted_raw_ostream &OS, bool PrintType = true) const {
      OS << "(" << getOpName() << ": ";
      printOrig(OS, PrintType);
      if (getIsArraySection()) {
        OS << " ";
        ArrSecInfo.print(OS, PrintType);
      }
      OS << ") ";
    }
    static bool classof(const Item *I) { return I->getKind() == IK_Reduction; }
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
    CopyinItem(VAR Orig) : Item(Orig, IK_Copyin), Copy(nullptr) {}
    void setCopy(RDECL Cpy) { Copy = Cpy; }
    RDECL getCopy() const { return Copy; }
    static bool classof(const Item *I) { return I->getKind() == IK_Copyin; }
};


//
//   CopyprivateItem: OMP COPYPRIVATE clause item
//
class CopyprivateItem : public Item
{
  private:
    RDECL Copy;

  public:
    CopyprivateItem(VAR Orig) : Item(Orig, IK_Copyprivate), Copy(nullptr) {}
    void setCopy(RDECL Cpy) { Copy = Cpy; }
    RDECL getCopy() const { return Copy; }
    static bool classof(const Item *I) { return I->getKind() == IK_Copyprivate; }
};


//
//   LinearItem: OMP LINEAR clause item
//
class LinearItem : public Item
{
  private:
    EXPR Step;
#if INTEL_CUSTOMIZATION
    HEXPR HStep; // Item's Step for HIR
#endif // INTEL_CUSTOMIZATION
    bool IsIV; // Linear clause is for a loop IV, whose value is being
               // computed in each iteration using close-form computation on
               // the normalized loop IV.

    // No need for ctor/dtor because OrigItem is either pointer or array base

  public:
#if INTEL_CUSTOMIZATION
    LinearItem(VAR Orig)
        : Item(Orig, IK_Linear), Step(nullptr), HStep(nullptr), IsIV(false) {}
#else
    LinearItem(VAR Orig) : Item(Orig, IK_Linear), Step(nullptr), IsIV(false) {}
#endif // INTEL_CUSTOMIZATION
    void setStep(EXPR S) { Step = S; }
    EXPR getStep() const { return Step; }
#if INTEL_CUSTOMIZATION
    void setHStep(HEXPR S) { HStep = S; }
    template <IRKind IR = LLVMIR> ExprType<IR> getStep() const;
#endif // INTEL_CUSTOMIZATION
    void setIsIV(bool Flag) { IsIV = Flag; }
    bool getIsIV() const { return IsIV; }

    // Specialized print() to output the stride as well
    void print(formatted_raw_ostream &OS, bool PrintType=true) const {
      if (getIsIV())
        OS << "IV";
      OS << "(";
      printOrig(OS, PrintType);
      OS << ", ";
      auto *Step = getStep();
      assert(Step && "Null 'Step' for LINEAR clause.");
#if INTEL_CUSTOMIZATION
      if (HStep)
        HStep->print(OS, PrintType);
      else
#endif // INTEL_CUSTOMIZATION
      Step->printAsOperand(OS, PrintType);
      OS << ") ";
    }
    static bool classof(const Item *I) { return I->getKind() == IK_Linear; }
};

//
//   UniformItem: OMP UNIFORM clause item
//
class UniformItem : public Item
{
  public:
    UniformItem(VAR Orig) : Item(Orig, IK_Uniform) {}
    static bool classof(const Item *I) { return I->getKind() == IK_Uniform; }
};

// MAP CHAINS
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
  uint64_t MapType;
public:
  MapAggrTy(Value *BP, Value *SP, Value *Sz)
      : BasePtr(BP), SectionPtr(SP), Size(Sz), MapType(0) {}
  MapAggrTy(Value *BP, Value *SP, Value *Sz, uint64_t MT)
      : BasePtr(BP), SectionPtr(SP), Size(Sz), MapType(MT) {}
  void setBasePtr(Value *BP) { BasePtr = BP; }
  void setSectionPtr(Value *SP) { SectionPtr = SP; }
  void setSize(Value *Sz) { Size = Sz; }
  void setMapType(uint64_t MT) { MapType = MT;}
  Value *getBasePtr() const { return BasePtr; }
  Value *getSectionPtr() const { return SectionPtr; }
  Value *getSize() const { return Size; }
  uint64_t getMapType() const { return MapType; }
};

typedef SmallVector<MapAggrTy*, 2> MapChainTy;

class UseDevicePtrItem;
//
//   MapItem: OMP MAP clause item
//
class MapItem : public Item
{
private:
  unsigned MapKind;                 // bit vector for map kind and modifiers
  FirstprivateItem *InFirstprivate; // FirstprivateItem with the same opnd
  UseDevicePtrItem *InUseDevicePtr; // The map is for a use-device-ptr clause
  MapChainTy MapChain;
  ArraySectionInfo ArrSecInfo;    // For TARGET UPDATE TO/FROM clauses
  Instruction *BasePtrGEPForOrig; // GEP for Orig in the  baseptrs struct sent
                                  // to tgt runtime calls.

public:
  enum WRNMapKind {
    WRNMapNone       = 0x0000,
    // map types:
    WRNMapTo         = 0x0001,
    WRNMapFrom       = 0x0002,
    WRNMapAlloc      = 0x0004,
    WRNMapRelease    = 0x0008,
    WRNMapDelete     = 0x0010,
    // update to/from clauses:
    WRNMapUpdateTo   = 0x0020,
    WRNMapUpdateFrom = 0x0040,
    // map-type modifiers:
    WRNMapAlways     = 0x0080,
    WRNMapClose      = 0x0100,
    WRNMapPresent    = 0x0200,
  } WRNMapKind;

  MapItem(VAR Orig)
      : Item(Orig, IK_Map), MapKind(0), InFirstprivate(nullptr),
        InUseDevicePtr(nullptr), BasePtrGEPForOrig(nullptr) {}
  MapItem(MapAggrTy *Aggr)
      : Item(nullptr, IK_Map), MapKind(0), InFirstprivate(nullptr),
        InUseDevicePtr(nullptr), BasePtrGEPForOrig(nullptr) {
    MapChain.push_back(Aggr);
  }
  ~MapItem() {
    if (MapChain.empty())
      return;

    llvm::for_each(MapChain, [](MapAggrTy *V) { delete V; });
  }

  const MapChainTy &getMapChain() const { return MapChain; }
        MapChainTy &getMapChain()       { return MapChain; }

  // For map clauses that specify array section we allow
  // adding a single map chain element that represents
  // this array section.
  void setMapChainForArraySection(MapAggrTy *Aggr) {
    assert(getIsArraySection() &&
           "Setting map chaing for a map clause, "
           "which is not an array section.");
    assert(MapChain.size() == 0 &&
           "Setting map chain for a map clause with an array section twice.");
    MapChain.push_back(Aggr);
  }

  bool getIsMapChain() const { return !MapChain.empty(); }

  static unsigned getMapKindFromClauseId(int Id) {
    switch(Id) {
      case QUAL_OMP_TO:
        return WRNMapUpdateTo;
      case QUAL_OMP_MAP_TO:
        return WRNMapTo;
      case QUAL_OMP_FROM:
        return WRNMapUpdateFrom;
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
        return WRNMapNone;
    }
  };

  void setMapKind(unsigned MK) { MapKind = MK; }
  void setIsMapTo()      { MapKind |= WRNMapTo; }
  void setIsMapFrom()    { MapKind |= WRNMapFrom; }
  void setIsMapTofrom()  { MapKind |= WRNMapFrom | WRNMapTo; }
  void setIsMapAlloc()   { MapKind |= WRNMapAlloc; }
  void setIsMapRelease() { MapKind |= WRNMapRelease; }
  void setIsMapDelete()  { MapKind |= WRNMapDelete; }
  void setIsMapAlways()  { MapKind |= WRNMapAlways; }
  void setIsMapClose()   { MapKind |= WRNMapClose; }
  void setIsMapPresent() { MapKind |= WRNMapPresent; }
  void setInFirstprivate(FirstprivateItem *FI) { InFirstprivate = FI; }
  void setInUseDevicePtr(UseDevicePtrItem *UDPI) { InUseDevicePtr = UDPI; }
  void setBasePtrGEPForOrig(Instruction *GEP) { BasePtrGEPForOrig = GEP; }

  unsigned getMapKind()     const { return MapKind; }
  bool getIsMapNone()       const { return MapKind == WRNMapNone; }
  bool getIsMapTo()         const { return MapKind & WRNMapTo; }
  bool getIsMapFrom()       const { return MapKind & WRNMapFrom; }
  bool getIsMapTofrom()     const { return (MapKind & WRNMapFrom) &&
                                           (MapKind & WRNMapTo); }
  bool getIsMapAlloc()      const { return MapKind & WRNMapAlloc; }
  bool getIsMapRelease()    const { return MapKind & WRNMapRelease; }
  bool getIsMapDelete()     const { return MapKind & WRNMapDelete; }
  bool getIsMapAlways()     const { return MapKind & WRNMapAlways; }
  bool getIsMapClose()      const { return MapKind & WRNMapClose; }
  bool getIsMapPresent()    const { return MapKind & WRNMapPresent; }
  bool getIsMapUpdateTo()   const { return MapKind & WRNMapUpdateTo; }
  bool getIsMapUpdateFrom() const { return MapKind & WRNMapUpdateFrom; }
  FirstprivateItem *getInFirstprivate() const { return InFirstprivate; }
  UseDevicePtrItem *getInUseDevicePtr() const { return InUseDevicePtr; }
  Instruction *getBasePtrGEPForOrig() const { return BasePtrGEPForOrig; }

  ArraySectionInfo &getArraySectionInfo() { return ArrSecInfo; }
  const ArraySectionInfo &getArraySectionInfo() const { return ArrSecInfo; }
  bool getIsArraySection() const {
    return !ArrSecInfo.getArraySectionDims().empty();
  };

  void print(formatted_raw_ostream &OS, bool PrintType=true) const {
    if (getIsMapChain()) {
      OS << "CHAIN(" ;
      for (unsigned I=0; I < MapChain.size(); ++I) {
        MapAggrTy *Aggr = MapChain[I];
        Value *BasePtr = Aggr->getBasePtr();
        Value *SectionPtr = Aggr->getSectionPtr();
        Value *Size = Aggr->getSize();
        uint64_t MapType = Aggr->getMapType();
        OS << "<" ;
        BasePtr->printAsOperand(OS, PrintType);
        OS << ", ";
        SectionPtr->printAsOperand(OS, PrintType);
        OS << ", ";
        Size->printAsOperand(OS, PrintType);
        OS << ", ";
        OS << MapType;
        OS <<  "> ";
      }
      OS << ") ";
    } else if (getIsArraySection()) {
      ArrSecInfo.print(OS, PrintType);
      OS << " ";
    } else {
      OS << "(" ;
      getOrig()->printAsOperand(OS, PrintType);
      OS << ") ";
    }
  }

  static bool classof(const Item *I) { return I->getKind() == IK_Map; }
};


//
//   IsDevicePtrItem: OMP IS_DEVICE_PTR clause item
//
class IsDevicePtrItem : public Item
{
  public:
    IsDevicePtrItem(VAR Orig) : Item(Orig, IK_IsDevicePtr) {}
    static bool classof(const Item *I) {
      return I->getKind() == IK_IsDevicePtr;
    }
};


//
//   UseDevicePtrItem: OMP USE_DEVICE_PTR clause item
//
class UseDevicePtrItem : public Item
{
  MapItem *InMap;
  bool IsUseDeviceAddr; // true only if parsing a use_device_addr clause
public:
  UseDevicePtrItem(VAR Orig)
      : Item(Orig, IK_UseDevicePtr), InMap(nullptr), IsUseDeviceAddr(false) {}
  void setInMap(MapItem *MI) { InMap = MI; }
  void setIsUseDeviceAddr(bool Flag) { IsUseDeviceAddr = Flag; }
  MapItem *getInMap() const { return InMap; }
  bool getIsUseDeviceAddr() const { return IsUseDeviceAddr; }
  static bool classof(const Item *I) { return I->getKind() == IK_UseDevicePtr; }
};

//
// These item classes for list-type clauses are not derived from the
// base "Item" class above.
//
//   SubdeviceItem (for the Subdevice clause)
//   DependItem    (for the depend  clause in task and target constructs)
//   DepSinkItem   (for the depend(sink:<vec>) clause in ordered constructs)
//   DepSourceItem (for the depend(source) clause in ordered constructs)
//   AlignedItem   (for the aligned clause in simd constructs)
//   FlushItem     (for the flush clause)
//   AllocateItem  (for the allocate clause)
//   NontemporalItem (for the nontemporal clause in simd constructs)
//
// Clang collapses the 'n' loops for 'ordered(n)'. So VPO always
// receives a single EXPR for depend(sink:sink_expr), which is already in
// the form ' IV +/- offset'.
//
class SubdeviceItem
{
  private:
    EXPR  Level;          // null if unspecified
    EXPR  Start;          // null if unspecified
    EXPR  Length;         // null if unspecified
    EXPR  Stride;         // null if unspecified

  public:
    SubdeviceItem(const Use *Args) {
      Level = cast<Value>(Args[0]);
      Start = cast<Value>(Args[1]);
      Length = cast<Value>(Args[2]);
      Stride = cast<Value>(Args[3]);
    }

    void setLevel(EXPR Lev)     { Level = Lev;   }
    void setStart(EXPR Sta)     { Start = Sta;   }
    void setLength(EXPR Len)    { Length = Len;  }
    void setStride(EXPR Str)    { Stride = Str;  }

    EXPR setLevel()     const   { return Level;  }
    EXPR getStart()     const   { return Start;  }
    EXPR getLevel()     const   { return Length; }
    EXPR getStride()    const   { return Stride; }
    void print(formatted_raw_ostream &OS, bool PrintType=true) const {
        OS << "SUBDEVICE(" << Level << ", " << Start << ", " << Length << ", "
           << Stride << ")";
    }
};

class DependItem
{
  private:
    VAR   Base;           // scalar item or base of array section
    bool  IsByRef;        // true if Base is by-reference
    bool  IsIn;           // depend type: true for IN; false for OUT/INOUT
    EXPR  LowerBound;     // null if unspecified
    EXPR  Length;         // null if unspecified
    EXPR  Stride;         // null if unspecified
    ArraySectionInfo ArrSecInfo;

  public:
    DependItem(VAR V = nullptr)
        : Base(V), IsByRef(false), IsIn(true), LowerBound(nullptr),
          Length(nullptr), Stride(nullptr) {}

    void setOrig(VAR V)         { Base = V; }
    void setIsByRef(bool Flag)  { IsByRef = Flag; }
    void setIsIn(bool Flag)     { IsIn = Flag; }
    void setLb(EXPR Lb)         { LowerBound = Lb;   }
    void setLength(EXPR Len)    { Length = Len;  }
    void setStride(EXPR Str)    { Stride = Str;  }

    VAR  getOrig()      const   { return Base; }
    bool getIsByRef()   const   { return IsByRef; }
    bool getIsIn()      const   { return IsIn; }
    EXPR getLb()        const   { return LowerBound; }
    EXPR getLength()    const   { return Length; }
    EXPR getStride()    const   { return Stride; }
    ArraySectionInfo &getArraySectionInfo() { return ArrSecInfo; }
    const ArraySectionInfo &getArraySectionInfo() const { return ArrSecInfo; }
    bool getIsArraySection() const {
      return !ArrSecInfo.getArraySectionDims().empty();
    };

    void print(formatted_raw_ostream &OS, bool PrintType=true) const {
      if (getIsByRef())
        OS << "BYREF";
      if (getIsArraySection()) {
        OS << " ";
        ArrSecInfo.print(OS, PrintType);
      }
      OS << "(" ;
      getOrig()->printAsOperand(OS, PrintType);
      OS << ") ";
    }
};

class DepSrcSinkItem {
private:
  SmallVector<Value *, 3> DepExprs;

public:
  DepSrcSinkItem(SmallVectorImpl<Value *> &&DEs) : DepExprs(std::move(DEs)) {}
  const SmallVectorImpl<Value *> &getDepExprs() const { return DepExprs; }

  void print(formatted_raw_ostream &OS, bool PrintType = true) const {
    OS << "(";
    for (const auto *E : DepExprs) {
      E->printAsOperand(OS, PrintType);
      OS << " ";
    }
    OS << ") ";
  }
};

class DepSourceItem: public DepSrcSinkItem {
public:
  DepSourceItem(SmallVectorImpl<Value *> &&DEs)
      : DepSrcSinkItem(std::move(DEs)) {}
};

class DepSinkItem: public DepSrcSinkItem {
public:
  DepSinkItem(SmallVectorImpl<Value *> &&DEs)
      : DepSrcSinkItem(std::move(DEs)) {}
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

class NontemporalItem
{
  private:
    VAR Base; // pointer or base of array

  public:
    NontemporalItem(VAR V=nullptr) : Base(V) {}
    VAR getOrig() const { return Base; }

    void print(formatted_raw_ostream &OS, bool PrintType=true) const {
      OS << "(";
      getOrig()->printAsOperand(OS, PrintType);
      OS << ") ";
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

class AllocateItem
{
  private:
    VAR   Var;
    Value *Allocator; // intptr value for the Allocator handle (enum or pointer)

  public:
    AllocateItem(VAR V = nullptr) : Var(V), Allocator(nullptr) {}
    void setOrig(VAR V) { Var = V; }
    void setAllocator(Value *A) { Allocator = A; }
    VAR getOrig() const { return Var; }
    Value *getAllocator() const { return Allocator; }

    void print(formatted_raw_ostream &OS, bool PrintType = true) const {
      OS << "(";
      getOrig()->printAsOperand(OS, PrintType);
      OS << ", ";
      if (getAllocator())
        getAllocator()->printAsOperand(OS, PrintType);
      else
        OS << "NULL";
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
  friend class VPOParoptTransform;
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

  StringRef Name = VPOAnalysisUtils::getOmpClauseName(getClauseID());
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
typedef Clause<SharedItem>        SharedClause;
typedef Clause<PrivateItem>       PrivateClause;
typedef Clause<FirstprivateItem>  FirstprivateClause;
typedef Clause<LastprivateItem>   LastprivateClause;
typedef Clause<ReductionItem>     ReductionClause;
typedef Clause<CopyinItem>        CopyinClause;
typedef Clause<CopyprivateItem>   CopyprivateClause;
typedef Clause<LinearItem>        LinearClause;
typedef Clause<UniformItem>       UniformClause;
typedef Clause<MapItem>           MapClause;
typedef Clause<IsDevicePtrItem>   IsDevicePtrClause;
typedef Clause<UseDevicePtrItem>  UseDevicePtrClause;
typedef Clause<SubdeviceItem>     SubdeviceClause;
typedef Clause<DependItem>        DependClause;
typedef Clause<DepSinkItem>       DepSinkClause;
typedef Clause<DepSourceItem>     DepSourceClause;
typedef Clause<AlignedItem>       AlignedClause;
typedef Clause<NontemporalItem>   NontemporalClause;
typedef Clause<FlushItem>         FlushSet;
typedef Clause<AllocateItem>      AllocateClause;

typedef std::vector<SharedItem>::iterator        SharedIter;
typedef std::vector<PrivateItem>::iterator       PrivateIter;
typedef std::vector<FirstprivateItem>::iterator  FirstprivateIter;
typedef std::vector<LastprivateItem>::iterator   LastprivateIter;
typedef std::vector<ReductionItem>::iterator     ReductionIter;
typedef std::vector<CopyinItem>::iterator        CopyinIter;
typedef std::vector<CopyprivateItem>::iterator   CopyprivateIter;
typedef std::vector<LinearItem>::iterator        LinearIter;
typedef std::vector<UniformItem>::iterator       UniformIter;
typedef std::vector<MapItem>::iterator           MapIter;
typedef std::vector<IsDevicePtrItem>::iterator   IsDevicePtrIter;
typedef std::vector<UseDevicePtrItem>::iterator  UseDevicePtrIter;
typedef std::vector<SubdeviceItem>::iterator     SubdeviceIter;
typedef std::vector<DependItem>::iterator        DependIter;
typedef std::vector<DepSinkItem>::iterator       DepSinkIter;
typedef std::vector<DepSourceItem>::iterator     DepSourceIter;
typedef std::vector<AlignedItem>::iterator       AlignedIter;
typedef std::vector<NontemporalItem>::iterator   NontemporalIter;
typedef std::vector<FlushItem>::iterator         FlushIter;
typedef std::vector<AllocateItem>::iterator      AllocateIter;


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

typedef enum WRNDefaultmapBehavior {
    WRNDefaultmapAbsent = 0,      // defaultmap clause not present
    WRNDefaultmapAlloc,           // defaultmap(alloc        [:<category>] )
    WRNDefaultmapTo,              // defaultmap(to           [:<category>] )
    WRNDefaultmapFrom,            // defaultmap(from         [:<category>] )
    WRNDefaultmapToFrom,          // defaultmap(tofrom       [:<category>] )
    WRNDefaultmapFirstprivate,    // defaultmap(firstprivate [:<category>] )
    WRNDefaultmapNone,            // defaultmap(none         [:<category>] )
    WRNDefaultmapDefault,         // defaultmap(default      [:<category>] )
    WRNDefaultmapPresent          // defaultmap(present      [:<category>] )
} WRNDefaultmapBehavior;

// Table to get defaultmap behavior from ClauseID QUAL_OMP_DEFAULTMAP_<behavior>
extern DenseMap<int, WRNDefaultmapBehavior> WRNDefaultmapBehaviorFromClauseID;

typedef enum WRNDefaultmapCategory {
    WRNDefaultmapAllVars = 0,     // defaultmap(<behavior>) with no category
                                  // specified; <behavior> applies to all vars
    WRNDefaultmapAggregate,       // defaultmap(<behavior>:aggregate)
#if INTEL_CUSTOMIZATION
    // Fortran only
    WRNDefaultmapAllocatable,     // defaultmap(<behavior>:allocatable)
#endif // INTEL_CUSTOMIZATION
    WRNDefaultmapPointer,         // defaultmap(<behavior>:pointer)
    WRNDefaultmapScalar,          // defaultmap(<behavior>:scalar)
    WRNDefaultmapCategorySize     // last entry in enum
} WRNDefaultmapCategory;

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
// Support for binding rules in loop construct
//
typedef enum WRNLoopBindKind {
  WRNLoopBindAbsent = 0,
  WRNLoopBindTeams = 1,
  WRNLoopBindParallel = 2,
  WRNLoopBindThread = 3
} WRNLoopBindKind;

//
// Support for order clauses in loop construct
//
typedef enum WRNLoopOrderKind {
  WRNLoopOrderAbsent = 0,
  WRNLoopOrderConcurrent = 1
} WRNLoopOrderKind;

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
