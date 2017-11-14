//===---------------- DTransAnalysis.cpp - DTrans Analysis ----------------===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
// This file does DTrans analysis.
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_DTrans/DTransAnalysis.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Analysis/Intel_DTrans/DTrans.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Operator.h"
#include "llvm/Support/raw_ostream.h"

#include <algorithm>
#include <map>
#include <set>

using namespace llvm;

#define DEBUG_TYPE "dtransanalysis"

static cl::opt<bool> DTransPrintAllocations("dtrans-print-allocations",
                                            cl::ReallyHidden);

static cl::opt<bool> DTransPrintAnalyzedTypes("dtrans-print-types",
                                              cl::ReallyHidden);

namespace {

/// Information describing type alias information for temporary values used
/// within a function.
///
/// This class is used within the DTransAnalysis to track the types of data
/// that a value may point to, independent of the the type of the value.
/// For example, consider the following line of IR:
///
///   %t = bitcast %struct.S* %ps to i8*
///
/// The type of %t is i8*, but because the value was created using a bitcast
/// we know that it points to a memory block that is treated as %struct.S
/// elsewhere in the IR.
///
/// This class is also used to track information about whether or not a value
/// points to an element within an aggregate type.  If a pointer value is
/// obtained using a GetElementPtr instruction, that pointer is a pointer to an
/// element in the base object.  In addition, if the pointer is subsequently
/// cast to another type, the bitcast value is also tracked as a pointer to the
/// same element.  For instance,
///
///   %struct.S = type { i32, i32 }
///   ...
///   %pb = getelementptr %struct.S, %struct.S* %ps, i64 0, i32 1
///   %tb = bitcast i32* %pb to i8*
///
/// In this case, %pb and %tb are both tracked as pointers to the second
/// element of the %struct.S structure.
class LocalPointerInfo {
public:
  typedef std::set<std::pair<llvm::Type *, size_t>> ElementPointeeSet;
  typedef std::set<std::pair<llvm::Type *, size_t>> &ElementPointeeSetRef;
  typedef SmallPtrSet<llvm::Type *, 3> PointerTypeAliasSet;
  typedef SmallPtrSetImpl<llvm::Type *> &PointerTypeAliasSetRef;

  LocalPointerInfo()
      : HasBeenAnalyzed(false), AliasesToAggregatePointer(false) {}

  void setAnalyzed() { HasBeenAnalyzed = true; }
  bool getAnalyzed() { return HasBeenAnalyzed; }

  bool canAliasToAggregatePointer() {
    assert(HasBeenAnalyzed);
    return AliasesToAggregatePointer;
  }

  void addPointerTypeAlias(llvm::Type *T) {
    // This should only be called for integers that are returned by PtrToInt
    // instructions or actual pointers.
    assert(T->isPointerTy() || T->isIntegerTy());
    // Check to see if this is a pointer (at any level of indirection) to
    // an aggregate type. That will make it faster later to tell if a value
    // is interesting or not.
    Type *BaseTy = T;
    while (BaseTy->isPointerTy())
      BaseTy = BaseTy->getPointerElementType();
    if (BaseTy->isAggregateType())
      AliasesToAggregatePointer = true;
    // Save this alias.
    PointerTypeAliases.insert(T);
  }

  // If a pointer is pointing to an element of an aggregate, we want to track
  // that information.
  void addElementPointee(llvm::Type *Base, size_t ElemIdx) {
    ElementPointees.insert(std::make_pair(Base, ElemIdx));
  }

  bool canPointToType(llvm::Type *T) { return PointerTypeAliases.count(T) > 0; }
  bool pointsToSomeElement() { return ElementPointees.size() > 0; }

  PointerTypeAliasSetRef getPointerTypeAliasSet() { return PointerTypeAliases; }
  ElementPointeeSetRef getElementPointeeSet() { return ElementPointees; }

  void merge(LocalPointerInfo &Other) {
    // This routine is called during analysis, so don't change HasBeenAnalyzed.
    AliasesToAggregatePointer |= Other.AliasesToAggregatePointer;
    for (auto *Ty : Other.PointerTypeAliases)
      PointerTypeAliases.insert(Ty);
    for (auto &Pair : Other.ElementPointees)
      ElementPointees.insert(Pair);
  }

private:
  bool HasBeenAnalyzed;
  bool AliasesToAggregatePointer;
  PointerTypeAliasSet PointerTypeAliases;
  ElementPointeeSet ElementPointees;
};

class LocalPointerAnalyzer {
public:
  LocalPointerInfo &getLocalPointerInfo(Value *V) {
    // If we don't already have an entry for this pointer, do some analysis.
    if (!LocalMap.count(V))
      analyzeValue(V);
    // Now the information we want will be in the map.
    LocalPointerInfo &Info = LocalMap[V];
    assert(Info.getAnalyzed() && "Local pointer analysis failed.");
    return Info;
  }

private:
  // We cannot use DenseMap or ValueMap here because we are inserting values
  // during recursive calls to analyzeValue() and with a DenseMap or ValueMap
  // that would cause the LocalPointerInfo to be copied and our local
  // reference to it to be invalidated.
  std::map<Value *, LocalPointerInfo> LocalMap;
  SmallPtrSet<Value *, 8> InProgressValues;

  void analyzeValue(Value *V) {
    // If we're already working on this value (for instance, tracing the
    // incoming values of a PHI node), don't go any further.
    if (!InProgressValues.insert(V).second)
      return;

    // If we've already analyzed this value, there is no need to
    // repeat the work.
    LocalPointerInfo &Info = LocalMap[V];
    if (Info.getAnalyzed())
      return;

    // If this value is derived from another local value, follow the
    // collect info from the source operand.
    if (isDerivedValue(V))
      collectSourceOperandInfo(V, Info);

    // If this value has a pointer type, add the type of the value.
    // An example of a value that would be used here but not have a pointer
    // type is the result of a PtrToInt instruction.
    llvm::Type *VTy = V->getType();
    if (isa<PointerType>(VTy))
      Info.addPointerTypeAlias(VTy);

    // TODO: Add GEP check.

    // Mark the info as analyzed.
    Info.setAnalyzed();

    // Erase this value from the in-progress set.
    // The 'analyzed' flag will be sufficient to prevent future re-analysis.
    InProgressValues.erase(V);
  }

  void collectSourceOperandInfo(Value *V, LocalPointerInfo &Info) {
    // In each case, the call to analyzeValue performs a check which will
    // prevent infinite recursion if we track back to a block we've
    // already visited.
    if (auto *PN = dyn_cast<PHINode>(V)) {
      for (Value *InVal : PN->incoming_values()) {
        analyzeValue(InVal);
        Info.merge(LocalMap[InVal]);
      }
      return;
    }
    if (auto *Sel = dyn_cast<SelectInst>(V)) {
      Value *TV = Sel->getTrueValue();
      analyzeValue(TV);
      Info.merge(LocalMap[TV]);
      Value *FV = Sel->getFalseValue();
      analyzeValue(FV);
      Info.merge(LocalMap[FV]);
      return;
    }
    if (isa<CastInst>(V) || isa<BitCastOperator>(V) ||
        isa<PtrToIntOperator>(V)) {
      Value *SrcVal = cast<User>(V)->getOperand(0);
      analyzeValue(SrcVal);
      Info.merge(LocalMap[SrcVal]);
      return;
    }
    // The caller should have checked isDerivedValue() before calling this
    // function, and the above cases should cover all possible derived
    // values.
    llvm_unreachable("Unexpected class for derived value!");
  }

  bool isDerivedValue(Value *V) {
    // TODO: Consider whether it will be necessary to handle llvm::MemoryAccess.

    // These value types transform other values into a new temporary.
    // GetElementPtr isn't in this list because the pointer it returns is
    // referring to a different logical object (a field) than the input
    // value, even though it points to the same block of memory.
    // The GetElementPtr case will be handled elsewhere.
    if (isa<CastInst>(V) || isa<PHINode>(V) || isa<SelectInst>(V) ||
        isa<BitCastOperator>(V) || isa<PtrToIntOperator>(V))
      return true;

    // This assert is here to catch cases that I haven't thought about.
    assert(isa<GlobalVariable>(V) || isa<Argument>(V) || isa<AllocaInst>(V) ||
           isa<LoadInst>(V) || isa<CallInst>(V) || isa<GetElementPtrInst>(V) ||
           isa<Constant>(V) || isa<GEPOperator>(V));

    return false;
  }
};

class DTransInstVisitor : public InstVisitor<DTransInstVisitor> {
public:
  DTransInstVisitor(DTransAnalysisInfo &Info, const DataLayout &DL,
                    const TargetLibraryInfo &TLI)
      : DTInfo(Info), DL(DL), TLI(TLI) {}

  void visitCallInst(CallInst &CI) {
    Function *F = CI.getCalledFunction();
    dtrans::AllocKind Kind = dtrans::getAllocFnKind(F, TLI);
    if (Kind != dtrans::AK_NotAlloc) {
      analyzeAllocationCall(CI, Kind);
    } else {
      // If this is not an allocation call, but it returns an interesting
      // type value, record that.
      llvm::Type *RetTy = CI.getType();
      if (DTInfo.isTypeOfInterest(RetTy)) {
        if (F) {
          DEBUG(dbgs() << "Type " << *RetTy << " is returned by a call to "
                       << F->getName() << "\n");
          // TODO: Record this information?
        } else {
          DEBUG(dbgs() << "Type " << *RetTy
                       << " is returned by an indirect function call.\n");
          // TODO: Record this information?
        }
        // This is probably safe, but we'll flag it for now until we're sure.
        setBaseTypeInfoSafetyData(RetTy, dtrans::UnhandledUse);
      }
    }

    for (Value *Arg : CI.arg_operands()) {
      if (isValueOfInterest(Arg)) {
        DEBUG(dbgs() << "DTRANS: Unhandled use -- value passed as argument.\n"
                     << "  " << *Arg << "\n");
        // This may be safe, but we'll flag it for now until whole program
        // function modeling is complete.
        setValueTypeInfoSafetyData(Arg, dtrans::UnhandledUse);
      }
    }
  }

  void visitBitCastInst(BitCastInst &I) {
    // Collect the types involed in this cast.
    llvm::Type *SrcTy = I.getSrcTy();
    llvm::Type *DestTy = I.getDestTy();
    llvm::Type *BytePtrTy = llvm::Type::getInt8PtrTy(I.getContext());
    llvm::Type *IntPtrPtrTy =
        llvm::Type::getIntNPtrTy(I.getContext(), DL.getPointerSizeInBits());

    auto isElementZeroAccess = [](llvm::Type *SrcTy, llvm::Type *DestTy) {
      if (!DestTy->isPointerTy() || !SrcTy->isPointerTy())
        return false;
      llvm::Type *SrcPointeeTy = SrcTy->getPointerElementType();
      llvm::Type *DestPointeeTy = DestTy->getPointerElementType();
      if (SrcPointeeTy->isStructTy())
        return DestPointeeTy == SrcPointeeTy->getStructElementType(0);
      if (SrcPointeeTy->isArrayTy())
        return DestPointeeTy == SrcPointeeTy->getArrayElementType();
      return false;
    };

    if (DTInfo.isTypeOfInterest(DestTy)) {
      if (SrcTy == BytePtrTy) {
        // If we are casting from a i8* to a type of interest, make sure
        // the source operand is known to alias DestTy and does not alias to
        // any other types.
        LocalPointerInfo &LPI = LPA.getLocalPointerInfo(I.getOperand(0));
        bool AliasesDestTy = false;
        for (auto *AliasTy : LPI.getPointerTypeAliasSet()) {
          if (AliasTy == BytePtrTy)
            continue;
          if (AliasTy == DestTy) {
            AliasesDestTy = true;
            continue;
          }
          // If we source pointer aliases to another type, the cast is unsafe.
          DEBUG(dbgs() << "dtrans: unsafe cast of aliased pointer:\n"
                       << "  " << I << "\n");
          setValueTypeInfoSafetyData(&I, dtrans::BadCasting);
          return;
        }
        if (!AliasesDestTy) {
          DEBUG(dbgs() << "dtrans: unsafe cast of i8* to unexpected type:\n"
                       << "  " << I << "\n");
          setValueTypeInfoSafetyData(&I, dtrans::BadCasting);
          return;
        }
        // TODO: Check for element pointer use.
      } else {
        // If DestTy points to a type that matches the type of SrcTy
        // element zero, this is an element access.
        if (isElementZeroAccess(SrcTy, DestTy)) {
          // TODO: If this is an element zero access, we need to track that.
          DEBUG(dbgs() << "dtrans: bitcast used to access element zero:\n"
                       << "  " << I << "\n");
          setValueTypeInfoSafetyData(&I, dtrans::UnhandledUse);
          return;
        } else {
          DEBUG(dbgs() << "dtrans: unsafe cast of pointer:\n"
                       << "  " << I << "\n");
          setValueTypeInfoSafetyData(&I, dtrans::BadCasting);
          return;
        }
      }
      // We don't need to check the source type any further.
      return;
    }

    // If the source value is of interest, make sure the destination value
    // will be used in safe ways.
    if (isValueOfInterest(I.getOperand(0))) {
      // Casting to i8* is safe (though the uses may still be unsafe).
      if (DestTy == BytePtrTy)
        return;

      // If the source is a pointer to a pointer, and the destination is
      // a pointer to a pointer-sized integer, the cast is safe (though the
      // uses may still be unsafe).
      if (SrcTy->isPointerTy() &&
          SrcTy->getPointerElementType()->isPointerTy() &&
          DestTy == IntPtrPtrTy)
        return;

      // If the source value is an i8*, we need to look for the type of
      // interest to which it is known to alias. (We know there is one
      // because of the isValueOfInterest check.)
      if (SrcTy == BytePtrTy) {
        LocalPointerInfo &LPI = LPA.getLocalPointerInfo(I.getOperand(0));
        for (auto *AliasTy : LPI.getPointerTypeAliasSet()) {
          // If this aliases to multiple types of interest those will have
          // already been marked with bad casting elsewhere, so we can just
          // take the first one and stop looking.
          if (DTInfo.isTypeOfInterest(AliasTy)) {
            SrcTy = AliasTy;
            break;
          }
        }
      }

      assert(DTInfo.isTypeOfInterest(SrcTy));

      // The only other legal cast is a cast to the type of the first
      // element in an aggregate (which is a way to access that element
      // without using a GEP instruction).
      if (isElementZeroAccess(SrcTy, DestTy)) {
        // TODO: If this is an element zero access, we need to track that.
        DEBUG(dbgs() << "dtrans: bitcast used to access element zero:\n"
                     << "  " << I << "\n");
        setBaseTypeInfoSafetyData(SrcTy, dtrans::UnhandledUse);
      } else {
        // TODO: If SrcTy is a struct and DestTy is smaller than the struct's
        //       element zero type, it's likely that element zero is actually
        //       a union. We might want to handle that as something other than
        //       a bad cast.
        DEBUG(dbgs() << "dtrans: unsafe cast of pointer:\n"
                     << "  " << I << "\n");
        setBaseTypeInfoSafetyData(SrcTy, dtrans::BadCasting);
      }
    }
  }

  void visitLoadInst(LoadInst &I) {
    // Load instructions that read individual fields from a struct or array
    // are handled by following the address obtained via GetElementPtr.
    // Loads of a pointer to a pointer (which is generally a field access)
    // are also safe.
    //
    // What we are looking for here are loads which read an aggregate type
    // from memory using a pointer to the aggregate.

    // The load instruction looks like this:
    //
    //   <val> = load <ty>, <ty*> <op>
    //
    // Where '<op>' is the Value representing the address from which to load
    // '<ty*>' is the type of the '<op>' value (always a pointer type) and
    // '<ty>' is the type of the value loaded (always the element type of the
    // operand pointer type.
    Value *Ptr = I.getPointerOperand();
    if (!isValueOfInterest(Ptr))
      return;

    // FIXME: If what's being loaded is a pointer value, we may be able to
    //        trace where the value came from and decide whether or not it
    //        is safe.

    // If we get here, the load is loading one or more elements of a type
    // we're interested in using the aggregate type. Mark that in our
    // safety info for the type.
    DEBUG(dbgs() << "DTRANS: Aggregate type is loaded from memory: " << I
                 << "\n");
    // TODO: Make a proper safety data flag for this.
    setValueTypeInfoSafetyData(Ptr, dtrans::UnhandledUse);
  }

  void visitStoreInst(StoreInst &I) {
    // Store instructions that write individual fields of a struct or array
    // are handled by following the address obtained via GetElementPtr.
    // Stores of a pointer to a pointer (which is generally a field write)
    // are also safe.
    //
    // What we are looking for here are stores which write an aggregate type
    // to memory using a pointer to the aggregate.

    // The store instruction looks like this:
    //
    //   store <ty> <val>, <ty*> <ptr>
    //
    // Where '<val>' is the Value being stored, '<ty>' is the type of that
    // value, '<ptr> is the Value representing the address at which the store
    // occurs, and '<ty*>' is the type of the '<ptr>' value (always a pointer
    // type with '<ty>' as its element type).
    if (!isValueOfInterest(I.getValueOperand()) &&
        !isValueOfInterest(I.getPointerOperand()))
      return;

    // FIXME: If what's being stored is a pointer value, we may be able to
    //        trace where the value is used and decide whether or not it
    //        is safe.

    DEBUG(dbgs() << "DTRANS: Store instruction handling is unimplemented: " << I
                 << "\n");
    // TODO: Make a proper safety data flag for this.
    setValueTypeInfoSafetyData(I.getValueOperand(), dtrans::UnhandledUse);
    setValueTypeInfoSafetyData(I.getPointerOperand(), dtrans::UnhandledUse);
  }

  void visitGetElementPtrInst(GetElementPtrInst &I) {
    DEBUG(dbgs() << "DTRANS: Analyzing GEP uses:\n   " << I << "\n");
    // TODO: Associate the parent type of the pointer so we can properly
    //       evaluate the uses.
    Value *Src = I.getPointerOperand();
    if (!isValueOfInterest(Src))
      return;
    setValueTypeInfoSafetyData(Src, dtrans::UnhandledUse);
  }

  void visitPHINode(PHINode &I) {
    // PHI Nodes in LLVM just merge other values, nothing can be accessed
    // or dereferenced through a PHI node, so they are always safe. We
    // may need to follow uses through a PHI for other instruction types
    // but that happens elsewhere.
  }

  void visitSelectInst(SelectInst &I) {
    // Select instruction in LLVM just select among other values, nothing can
    // be accessed or dereferenced through a Select instruction, so they are
    // always safe. We may need to follow uses through a Select for other
    // instruction types but that happens elsewhere.
  }

  void visitPtrToIntInst(PtrToIntInst &I) {
    // If the source value is of interest, check to see if it is being cast
    // as a pointer-sized integer. If this is anything other than a
    // pointer being cast to a pointer-sized integer, it is a bad cast.
    // Otherwise, we will analyze this instruction when it is used.
    //
    // The safe usage looks like this:
    //
    //   %ps.as.i = ptrtoint %struct.s* %ps to i64
    //   %pps.as.pi = bitcast %struct.s** %pps to i64*
    //   store i64 %ps.as.i, i64* %pps.as.pi
    //
    // We will check this more closely when we visit the store instruction.
    Value *Src = I.getPointerOperand();
    if (!isValueOfInterest(Src))
      return;
    if (!I.getDestTy()->isIntegerTy(DL.getPointerSizeInBits())) {
      DEBUG(dbgs() << "DTRANS: (unsafe?) Pointer to aggregate type is cast to "
                      "a non-pointer-sized integer:\n    "
                   << I << "\n");
      setValueTypeInfoSafetyData(Src, dtrans::BadCasting);
    }

    // The isValueOfInterest() routine analyzes all PtrToInt result values
    // when they are used. Nothing more is needed here.
  }

  void visitReturnInst(ReturnInst &I) {
    // Return instructions are always safe. When a field within an aggregate
    // type is being returned, the field is accessed through a GEP and a load
    // with the loaded value being passed to the return instruction. We'll
    // look for that case where the GEP uses are being processed.

    // Here we're just interested in noting if a type of interest is returned.
    llvm::Type *RetTy = I.getType();
    if (!DTInfo.isTypeOfInterest(RetTy))
      return;

    DEBUG(dbgs() << "DTRANS: An aggregate type is returned by function "
                 << I.getParent()->getParent()->getName());
    // TODO: Record this?
    // This is probably safe, but we'll flag it for now until we're sure.
    setBaseTypeInfoSafetyData(RetTy, dtrans::UnhandledUse);
  }

  void visitICmpInst(ICmpInst &I) {
    // Compare instructions are always safe. When a compare is referencing
    // a field within an aggregate type, it does so through a GEP and a load
    // with the loaded value being passed to the compare. Therefore, we
    // do not need to track field information here either.

    // It is not possible to compare aggregate types directly.
    assert(!I.getOperand(0)->getType()->isAggregateType() &&
           !I.getOperand(1)->getType()->isAggregateType() &&
           "Unexpected compare of aggregate types.");
  }

  void visitAllocaInst(AllocaInst &I) {
    llvm::Type *Ty = I.getAllocatedType();
    if (!DTInfo.isTypeOfInterest(Ty))
      return;

    DEBUG(dbgs() << "DTRANS: (unsafe) Type used by a stack variable: " << *Ty
                 << "\n    " << I << "\n");
    // TODO: Set specific safety info.
    setBaseTypeInfoSafetyData(Ty, dtrans::UnhandledUse);
  }

  // All instructions not handled by other visit functions.
  void visitInstruction(Instruction &I) {
    // Any instruction that we haven't yet modeled should be conservatively
    // treated as though it is doing something unsafe if it either returns
    // a value with a type of interest or takes a value as an operand that
    // is of a type of interest.
    llvm::Type *Ty = I.getType();
    if (DTInfo.isTypeOfInterest(Ty)) {
      DEBUG(dbgs() << "DTRANS: (unsafe) Type used by an unmodeled "
                      "instruction:\n"
                   << "    " << I << "\n");
      setBaseTypeInfoSafetyData(Ty, dtrans::UnhandledUse);
    }

    for (Value *Arg : I.operands()) {
      if (isValueOfInterest(Arg)) {
        setValueTypeInfoSafetyData(Arg, dtrans::UnhandledUse);
      }
    }
  }

  void visitModule(Module &M) {
    // Call the base InstVisitor routine to visit each function.
    InstVisitor<DTransInstVisitor>::visitModule(M);

    // Now follow the uses of global variables.
    for (auto &GV : M.globals()) {
      // Get the type of this variable.
      llvm::Type *GVTy = GV.getType();

      // If this is an interesting type, analyze its uses.
      if (DTInfo.isTypeOfInterest(GVTy)) {
        // TODO: Look for an initializer list and set specific info.
        setBaseTypeInfoSafetyData(GVTy, dtrans::UnhandledUse);
        analyzeGlobalVariableUses(&GV);
      }
    }
  }

  void visitFunction(Function &F) {
    // Look at the function arguments to see if any of them are of interest.
    for (auto &Arg : F.args()) {
      // Get the type of this variable.
      llvm::Type *ArgTy = Arg.getType();

      // If this is an interesting type, analyze its uses.
      if (DTInfo.isTypeOfInterest(ArgTy)) {
        analyzeFnArgumentUses(F, &Arg);
      }
    }

    // Call the base class to visit the instructions in the function.
    InstVisitor<DTransInstVisitor>::visitFunction(F);
  }

private:
  DTransAnalysisInfo &DTInfo;
  const DataLayout &DL;
  const TargetLibraryInfo &TLI;

  // This helper class is used to track the types and aggregate elements to
  // which a local pointer value may refer. This information is created and
  // updated as needed.
  LocalPointerAnalyzer LPA;

  // There are frequent cases where a pointer to an aggregate type is
  // cast to either i8*, i64* or i64 and we need to look at the uses of
  // the cast value to determine whether or not it is being used in a way
  // that presents safety issues for the original type. This call allows
  // us to identify those cast values.
  //
  // This test current only supports immediate uses of cast values and
  // immediate uses of PHI or select users of those values. We may eventually
  // want to consider making a preliminary pass over entire functions to
  // identify all values that are indirectly dependent on a type of interest.
  bool isValueOfInterest(Value *V) {
    // If the type of the value is directly interesting, the answer is easy.
    if (DTInfo.isTypeOfInterest(V->getType()))
      return true;
    // Pointers to interesting types may also have been cast as i8*, i64,
    // or even (in the cast of pointers to poitners) i64*. Allocation calls
    // start as i8* and must be analyzed to determine if they are of interest.
    // If this value call instruction or a cast look to see if it is one of
    // these types and if it is, have our local pointer analyzer check it out.
    if ((isa<CallInst>(V) && isInt8Ptr(V)) ||
        ((isa<BitCastInst>(V) || isa<BitCastOperator>(V)) &&
         (isInt8Ptr(V) || isPtrSizeIntPtr(V))) ||
        ((isa<PtrToIntInst>(V) || isa<PtrToIntOperator>(V)) &&
         isPtrSizeInt(V))) {
      LocalPointerInfo &LPI = LPA.getLocalPointerInfo(V);
      return LPI.canAliasToAggregatePointer();
    }
    if (isa<PHINode>(V) || isa<SelectInst>(V)) {
      SmallPtrSet<Value *, 8> InProgressSet;
      return isPotentiallyRecursiveNodeOfInterest(V, InProgressSet);
    }
    return false;
  }

  bool isPotentiallyRecursiveNodeOfInterest(
      Value *V, SmallPtrSetImpl<Value *> &InProgressSet) {
    // Don't recurse into values we're already looking at.
    if (!InProgressSet.insert(V).second)
      return false;
    // If this value isn't potentially recursive, call the simple handler.
    if (!isa<PHINode>(V) && !isa<SelectInst>(V)) {
      return isValueOfInterest(V);
    }
    // Otherwise, we've got multiple incoming values to consider.
    if (auto *PN = dyn_cast<PHINode>(V)) {
      for (Value *IV : PN->incoming_values()) {
        if (isPotentiallyRecursiveNodeOfInterest(IV, InProgressSet)) {
          return true;
        }
        // else continue
      }
      // None of the incoming values were interesting, so neither is the PHI.
      return false;
    }
    // If the potentially recursive value isn't a PHI Node, it must be
    // a select instruction. The cast here will assert if it isn't.
    auto *Sel = cast<SelectInst>(V);
    if (isPotentiallyRecursiveNodeOfInterest(Sel->getTrueValue(),
                                             InProgressSet)) {
      return true;
    }
    if (isPotentiallyRecursiveNodeOfInterest(Sel->getFalseValue(),
                                             InProgressSet)) {
      return true;
    }
    // Nothing was interesting.
    return false;
  }

  inline bool isInt8Ptr(Value *V) {
    return (V->getType() == llvm::Type::getInt8PtrTy(V->getContext()));
  }

  inline bool isPtrSizeIntPtr(Value *V) {
    return (V->getType() == llvm::Type::getIntNPtrTy(
                                V->getContext(), DL.getPointerSizeInBits()));
  }

  inline bool isPtrSizeInt(Value *V) {
    return (V->getType() ==
            llvm::Type::getIntNTy(V->getContext(), DL.getPointerSizeInBits()));
  }

  void analyzeAllocationCall(CallInst &CI, dtrans::AllocKind Kind) {
    DEBUG(dbgs() << "DTRANS: found allocation call.\n  " << CI << "\n");

    uint64_t AllocSize = 0;
    uint64_t AllocCount = 0;
    if (!dtrans::determineAllocSize(Kind, &CI, AllocSize, AllocCount)) {
      DEBUG(dbgs() << "  Unable to determine size of allocation.\n");
      // If the allocated pointer is cast to an aggregate type, treat that
      // type as unsafe.
      for (auto *U : CI.users()) {
        auto *BI = dyn_cast<BitCastInst>(U);
        if (!BI)
          continue;
        auto *CastTy = BI->getType();
        if (DTInfo.isTypeOfInterest(CastTy)) {
          // FIXME: Set this to something more specific.
          setBaseTypeInfoSafetyData(CastTy, dtrans::UnhandledUse);
        }
      }
      return;
    }

    // Identify the type of data being allocated.
    // We're looking for a pattern like this:
    //
    //   %t1 = call i8* @malloc(i64 %size)
    //   %t2 = bitcast i8* to %some_type*
    //
    // From this we'll compare the size allocated to the size of the type
    // to determine whether an array of objects was allocated.
    SmallPtrSet<dtrans::TypeInfo *, 4> CastTypeInfos;
    bool WasCastToNonPointer = false;
    bool HasNonCastUses = false;
    for (auto *U : CI.users()) {
      if (auto *BI = dyn_cast<BitCastInst>(U)) {
        // This is probably a cast to another pointer type
        auto PtrTy = dyn_cast<PointerType>(BI->getType());
        if (!PtrTy) {
          DEBUG(dbgs() << "    Mem is cast to non-pointer:\n      " << *BI
                       << "\n");
          WasCastToNonPointer = true;
          continue;
          // FIXME: This should probably do something with the case where the
          //        allocation is cast to an i64 and stored in a pointer
          //        field in a structure. We'll set generic unimplemented
          //        safety info below.
        }

        // Save this bitcast as a known alias of the allocated pointer.
        LocalPointerInfo &LPI = LPA.getLocalPointerInfo(&CI);
        LPI.addPointerTypeAlias(PtrTy);

        // Find out what it points to.
        llvm::Type *PointeeTy = PtrTy->getElementType();

        uint64_t ElemSize = DL.getTypeAllocSize(PointeeTy);
        uint64_t NumElements = (AllocSize * AllocCount) / ElemSize;

        dtrans::TypeInfo *UserTypeInfo;
        // If this is an allocation of a single scalar value, we don't need
        // to track it.
        if (NumElements == 1 && !PointeeTy->isAggregateType())
          continue;
        if (NumElements > 1 || !PointeeTy->isAggregateType())
          UserTypeInfo =
              DTInfo.getOrCreateTypeInfoForArray(PointeeTy, NumElements);
        else
          UserTypeInfo = DTInfo.getOrCreateTypeInfo(PointeeTy);

        if (((AllocSize * AllocCount) % ElemSize) != 0) {
          DEBUG(dbgs() << "    Mem cast to uneven number of  elements:\n"
                       << "      " << *BI << "\n"
                       << "      Size = " << ElemSize << "\n");
          // FIXME: This should be a more specific type of safety info.
          setBaseTypeInfoSafetyData(PointeeTy, dtrans::BadCasting);
        }

        assert(UserTypeInfo &&
               "Unable to create dtrans type for allocated type");
        CastTypeInfos.insert(UserTypeInfo);
        DEBUG(dbgs() << "    Mem is cast to pointer to type:\n      " << *BI
                     << "\n");

        // We will be interested in what happens after the bitcast, but we'll
        // handle that when we visit the bitcast instructions directly.
      } else {
        HasNonCastUses = true;
      }
    }

    // If the malloc wasn't cast to an aggregate type, we're finished.
    if (CastTypeInfos.empty()) {
      DEBUG(dbgs() << "  Allocation found without cast to aggregate type.\n"
                   << "  " << CI << "\n");
      return;
    }
    // If the value is cast to multiple types, mark them all as bad casting.
    bool WasCastToMultipleTypes = (CastTypeInfos.size() > 1);

    if (DTransPrintAllocations && WasCastToMultipleTypes)
      outs() << "dtrans: Detected allocation cast to multiple types.\n";

    if (DTransPrintAllocations && HasNonCastUses)
      outs() << "dtrans: Detected non-cast uses of allocated type.\n";

    // We expect to only see one type, but we loop to keep the code general.
    for (auto *TI : CastTypeInfos) {
      // FIXME: It's possible the types are compatible if there is a cast
      //        to the first element of a structure or, in the case of
      //        nested structures, some field in a nested structure that is
      //        in the same memory location as the parent structure.
      //        We should add support for that here.
      if (WasCastToNonPointer || WasCastToMultipleTypes)
        TI->setSafetyData(dtrans::BadCasting);

      if (HasNonCastUses)
        TI->setSafetyData(dtrans::UnhandledUse);

      if (DTransPrintAllocations) {
        outs() << "dtrans: Detected allocation cast to pointer type\n";
        outs() << "  " << CI << "\n";
        outs() << "    Detected type: " << *(TI->getLLVMType()) << "\n";
      }
    }
  }

  void analyzeGlobalVariableUses(GlobalVariable *GV) {
    DEBUG(dbgs() << "DTRANS: Analyzing global variable:\n    " << *GV << "\n");
    // We're already setting the unimplemented safety data when we set
    // this value in the caller, but that case might get implemented before
    // we've handled whatever needs to be done here, so we set it again here.
    //
    // Basically, all of these uses are going to be analyzed as we visit the
    // instructions, but if there is anything special we need to look for
    // because the value is a global, that probably needs to be done here.
    llvm::Type *GVTy = GV->getType();
    if (DTInfo.isTypeOfInterest(GVTy))
      setBaseTypeInfoSafetyData(GVTy, dtrans::UnhandledUse);
    // TODO: Do something with this.
    DEBUG({
      for (auto *U : GV->users())
        dbgs() << "      " << *U << "\n";
    }); // DEBUG
  }

  void analyzeFnArgumentUses(Function &F, Argument *Arg) {
    DEBUG(dbgs() << "DTRANS: Analyzing argument in " << F.getName() << ": "
                 << *Arg << "\n");
    // FIXME: There's probably nothing that needs to be done here, but we'll
    //        set the unimplemented safety data flag until that is confirmed.
    llvm::Type *ArgTy = Arg->getType();
    if (DTInfo.isTypeOfInterest(ArgTy))
      setBaseTypeInfoSafetyData(ArgTy, dtrans::UnhandledUse);
    DEBUG({
      for (auto *U : Arg->users())
        dbgs() << "      " << *U << "\n";
    }); // DEBUG
  }

  void analyzeByteFlattenedGEP(GetElementPtrInst *GEP, dtrans::TypeInfo *DTTI) {
    // Some early optimization (InstCombine?) will sometimes convert field
    // access to GEP using i8* and the field offset.
    DEBUG(dbgs() << "    Mem is passed directly to GEP.\n      " << *GEP
                 << "\n");
    // FIXME: Analyze this.
    DTTI->setSafetyData(dtrans::UnhandledUse);
  }

  // In many cases we need to set safety data based on a value that
  // was derived from a pointer to a type of interest, via a bitcast
  // or a ptrtoint cast. In those cases, this function is called to
  // propogate safety data to the interesting type.
  void setValueTypeInfoSafetyData(Value *V, dtrans::SafetyData Data) {
    // In some cases this function might have been called for multiple
    // operands, not all of which we are actually tracking.
    if (!isValueOfInterest(V))
      return;

    LocalPointerInfo &LPI = LPA.getLocalPointerInfo(V);
    for (auto *Ty : LPI.getPointerTypeAliasSet()) {
      if (DTInfo.isTypeOfInterest(Ty)) {
        setBaseTypeInfoSafetyData(Ty, Data);
      }
    }
  }

  // This is a helper function that retrieves the aggregate type through
  // zero or more layers of indirection and sets the specified safety data
  // for that type.
  void setBaseTypeInfoSafetyData(llvm::Type *Ty, dtrans::SafetyData Data) {
    llvm::Type *BaseTy = Ty;
    while (BaseTy->isPointerTy())
      BaseTy = cast<PointerType>(BaseTy)->getElementType();
    dtrans::TypeInfo *TI = DTInfo.getOrCreateTypeInfo(BaseTy);
    TI->setSafetyData(Data);
  }

  void recordUnimplementedSafetyInfo(Instruction &I, llvm::Type *Ty) {
    // An instruction for which analysis has not yet been implemented was
    // seen defining or using a type of interest. Mark that type with
    // 'unimplemented' safety data.
    DEBUG(dbgs() << "DTRANS: (unsafe) Type used by an unmodeled "
                    "instruction:\n"
                 << "    " << I << "\n");
    setBaseTypeInfoSafetyData(Ty, dtrans::UnhandledUse);
  }
};

} // end anonymous namespace

// Return true if we are interested in tracking values of the specified type.
//
// For now, let's limit this to aggregates and various levels of indirection
// to aggregates. At some point we may also be interested in pointers to
// scalars.
bool DTransAnalysisInfo::isTypeOfInterest(llvm::Type *Ty) {
  llvm::Type *BaseTy = Ty;

  // For pointers, see what they point to.
  while (BaseTy->isPointerTy())
    BaseTy = cast<PointerType>(BaseTy)->getElementType();

  return BaseTy->isAggregateType();
}

dtrans::TypeInfo *DTransAnalysisInfo::getTypeInfo(llvm::Type *Ty) const {
  // If we have this type in our map, return it.
  auto IT = TypeInfoMap.find(Ty);
  if (IT != TypeInfoMap.end())
    return IT->second;
  // If not, return nullptr.
  return nullptr;
}

dtrans::TypeInfo *
DTransAnalysisInfo::getOrCreateTypeInfoForArray(llvm::Type *Ty,
                                                uint64_t NumElements) {
  // We saw an allocation that was cast to a pointer, but we want to treat it
  // as an array. To do that, we'll create an LLVM array type to describe it.
  llvm::ArrayType *ArrTy = llvm::ArrayType::get(Ty, NumElements);
  return getOrCreateTypeInfo(ArrTy);
}

dtrans::TypeInfo *DTransAnalysisInfo::getOrCreateTypeInfo(llvm::Type *Ty) {
  // If we already have this type in our map, just return it.
  auto TI = getTypeInfo(Ty);
  if (TI)
    return TI;

  // Create the dtrans type info for this type and any sub-types.
  dtrans::TypeInfo *DTransTy;
  if (Ty->isPointerTy()) {
    // For pointer types, we want to record the pointer type info
    // and then record what it points to. We must add the pointer to the
    // map early in this case to avoid infinite recursion.
    DTransTy = new dtrans::PointerInfo(Ty);
    TypeInfoMap[Ty] = DTransTy;
    (void)getOrCreateTypeInfo(cast<PointerType>(Ty)->getElementType());
    return DTransTy;
  } else if (Ty->isArrayTy()) {
    dtrans::TypeInfo *ElementInfo =
        getOrCreateTypeInfo(Ty->getArrayElementType());
    DTransTy =
        new dtrans::ArrayInfo(Ty, ElementInfo, Ty->getArrayNumElements());
  } else if (Ty->isStructTy()) {
    SmallVector<llvm::Type *, 16> FieldTypes;
    for (llvm::Type *FieldTy : cast<StructType>(Ty)->elements()) {
      FieldTypes.push_back(FieldTy);
      // Create a DTrans type for the field, in case it is an aggregate.
      (void)getOrCreateTypeInfo(FieldTy);
    }
    DTransTy = new dtrans::StructInfo(Ty, FieldTypes);
  } else {
    assert(!Ty->isAggregateType() &&
           "DTransAnalysisInfo::getOrCreateTypeInfo unexpected aggregate type");
    DTransTy = new dtrans::NonAggregateTypeInfo(Ty);
  }

  TypeInfoMap[Ty] = DTransTy;
  return DTransTy;
}

INITIALIZE_PASS_BEGIN(DTransAnalysisWrapper, "dtransanalysis",
                      "Data transformation analysis", false, true)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_END(DTransAnalysisWrapper, "dtransanalysis",
                    "Data transformation analysis", false, true)

char DTransAnalysisWrapper::ID = 0;

ModulePass *llvm::createDTransAnalysisWrapperPass() {
  return new DTransAnalysisWrapper();
}

DTransAnalysisWrapper::DTransAnalysisWrapper() : ModulePass(ID) {
  initializeDTransAnalysisWrapperPass(*PassRegistry::getPassRegistry());
}

bool DTransAnalysisWrapper::doFinalization(Module &M) {
  Result.reset();
  return false;
}

bool DTransAnalysisWrapper::runOnModule(Module &M) {
  return Result.analyzeModule(
      M, getAnalysis<TargetLibraryInfoWrapperPass>().getTLI());
}

DTransAnalysisInfo::DTransAnalysisInfo() {}

DTransAnalysisInfo::~DTransAnalysisInfo() {
  // DTransAnalysisInfo owns the TypeInfo pointers in the TypeInfoMap.
  for (auto Entry : TypeInfoMap)
    delete Entry.second;
}

bool DTransAnalysisInfo::analyzeModule(Module &M, TargetLibraryInfo &TLI) {
  DTransInstVisitor Visitor(*this, M.getDataLayout(), TLI);
  Visitor.visit(M);

  if (DTransPrintAnalyzedTypes) {
    // This is really ugly, but it is only used during testing.
    // The type infos are stored in a map with pointer keys, and so the
    // order is non-deterministic. This copies them into a vector and sorts
    // them so that the order in which they are printed is deterministic.
    std::vector<dtrans::TypeInfo *> TypeInfoEntries;
    for (auto Entry : TypeInfoMap) {
      // At this point I don't think it's useful to print scalar types or
      // pointer types.
      if (isa<dtrans::ArrayInfo>(Entry.second) ||
          isa<dtrans::StructInfo>(Entry.second)) {
        TypeInfoEntries.push_back(Entry.second);
      }
    }

    std::sort(TypeInfoEntries.begin(), TypeInfoEntries.end(),
              [](dtrans::TypeInfo *A, dtrans::TypeInfo *B) {
                std::string TypeStrA;
                llvm::raw_string_ostream RSO_A(TypeStrA);
                A->getLLVMType()->print(RSO_A);
                std::string TypeStrB;
                llvm::raw_string_ostream RSO_B(TypeStrB);
                B->getLLVMType()->print(RSO_B);
                return RSO_A.str().compare(RSO_B.str()) < 0;
              });

    outs() << "================================\n";
    outs() << " DTRANS Analysis Types Created\n";
    outs() << "================================\n\n";
    for (auto TI : TypeInfoEntries) {
      if (auto *AI = dyn_cast<dtrans::ArrayInfo>(TI)) {
        printArrayInfo(AI);
      } else if (auto *SI = dyn_cast<dtrans::StructInfo>(TI)) {
        printStructInfo(SI);
      }
    }
  }

  return false;
}

void DTransAnalysisInfo::printStructInfo(dtrans::StructInfo *SI) {
  outs() << "DTRANS_StructInfo:\n";
  outs() << "  LLVMType: " << *(SI->getLLVMType()) << "\n";
  outs() << "  Number of fields: " << SI->getNumFields() << "\n";
  for (auto &Field : SI->getFields()) {
    outs() << "  Field LLVM Type: " << *(Field.getLLVMType()) << "\n";
  }
  SI->printSafetyData();
  outs() << "\n";
}

void DTransAnalysisInfo::printArrayInfo(dtrans::ArrayInfo *AI) {
  outs() << "DTRANS_ArrayInfo:\n";
  outs() << "  LLVMType: " << *(AI->getLLVMType()) << "\n";
  outs() << "  Number of elements: " << AI->getNumElements() << "\n";
  outs() << "  Element LLVM Type: " << *(AI->getElementLLVMType()) << "\n";
  AI->printSafetyData();
  outs() << "\n";
}

void DTransAnalysisInfo::reset() {
  // TODO: Release resources.
}

void DTransAnalysisWrapper::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<TargetLibraryInfoWrapperPass>();
}

char DTransAnalysis::PassID;

// Provide a definition for the static class member used to identify passes.
AnalysisKey DTransAnalysis::Key;

DTransAnalysisInfo DTransAnalysis::run(Module &M, AnalysisManager<Module> &AM) {
  DTransAnalysisInfo DTResult;
  DTResult.analyzeModule(M, AM.getResult<TargetLibraryAnalysis>(M));
  return DTResult;
}
