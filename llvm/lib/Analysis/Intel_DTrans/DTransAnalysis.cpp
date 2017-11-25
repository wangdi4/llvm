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

#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Analysis/Intel_DTrans/DTrans.h"
#include "llvm/Analysis/Intel_DTrans/DTransAnalysis.h"
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

using namespace llvm;

#define DEBUG_TYPE "dtransanalysis"

static cl::opt<bool> DTransPrintAllocations("dtrans-print-allocations",
                                            cl::ReallyHidden);

static cl::opt<bool> DTransPrintAnalyzedTypes("dtrans-print-types",
                                              cl::ReallyHidden);

namespace {

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
      llvm::Type *ArgTy = Arg->getType();
      if (DTInfo.isTypeOfInterest(ArgTy)) {
        DEBUG(dbgs() << "Type " << *ArgTy
                     << " is used as a function argument.\n");
        // This may be safe, but we'll flag it for now until whole program
        // function modeling is complete.
        setBaseTypeInfoSafetyData(ArgTy, dtrans::UnhandledUse);
      }
    }
  }

  void visitBitCastInst(BitCastInst &I) {
    // Collect the types involed in this cast.
    llvm::Type *SrcTy = I.getSrcTy();
    llvm::Type *DestTy = I.getDestTy();

    // TODO: Maintain a map of local values that are cast to various types
    //       and defer setting safety info until we've seen how the value is
    //       used. Casts to and from i8* will frequently turn out to be
    //       safe.

    // If the destination type is of interest, make sure the source value
    // came from a safe place.
    if (DTInfo.isTypeOfInterest(DestTy))
      setBaseTypeInfoSafetyData(DestTy, dtrans::UnhandledUse);

    // If the source type is of interest, make sure the destination value
    // will be used in safe ways.
    if (DTInfo.isTypeOfInterest(SrcTy))
      setBaseTypeInfoSafetyData(SrcTy, dtrans::UnhandledUse);
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
    PointerType *LoadPtrTy = cast<PointerType>(Ptr->getType());
    llvm::Type *LoadedTy = LoadPtrTy->getElementType();

    if (!DTInfo.isTypeOfInterest(LoadedTy))
      return;

    // FIXME: If what's being loaded is a pointer value, we may be able to
    //        trace where the value came from and decide whether or not it
    //        is safe.

    // If we get here, the load is loading one or more elements of a type
    // we're interested in using the aggregate type. Mark that in our
    // safety info for the type.
    DEBUG(dbgs() << "DTRANS: Aggregate type is loaded from memory: "
                 << *LoadedTy << "\n");
    // TODO: Make a proper safety data flag for this.
    setBaseTypeInfoSafetyData(LoadedTy, dtrans::UnhandledUse);
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
    Value *Val = I.getValueOperand();
    llvm::Type *StoredTy = Val->getType();

    if (!DTInfo.isTypeOfInterest(StoredTy))
      return;

    // FIXME: If what's being stored is a pointer value, we may be able to
    //        trace where the value is used and decide whether or not it
    //        is safe.

    // If we get here, the store is writing one or more elements of a type
    // we're interested in using the aggregate type. Mark that in our safety
    // info for the type.
    DEBUG(dbgs() << "DTRANS: (unsafe) Aggregate type is stored to memory: "
                 << *StoredTy << "\n");
    // TODO: Make a proper safety data flag for this.
    setBaseTypeInfoSafetyData(StoredTy, dtrans::UnhandledUse);
  }

  void visitGetElementPtrInst(GetElementPtrInst &I) {
    DEBUG(dbgs() << "DTRANS: Analyzing GEP uses:\n   " << I << "\n");
    // TODO: Associate the parent type of the pointer so we can properly
    //       evaluate the uses.
    llvm::Type *GEPSrcTy = I.getSourceElementType();
    setBaseTypeInfoSafetyData(GEPSrcTy, dtrans::UnhandledUse);
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
    // Collect the types involed in this cast.
    llvm::Type *SrcTy = I.getSrcTy();
    llvm::Type *DestTy = I.getDestTy();

    // If the source type is of interest, make sure the destination value
    // will be used in safe ways. If this is anything other than a
    // pointer being cast to a pointer-sized int and stored in a location
    // that we know is a pointer-to-pointer to the type to which our source
    // pointer points, we'll conservatively treat it as an unsafe cast.
    //
    // The safe usage looks like this:
    //
    //   %ps.as.i = ptrtoint %struct.s* %ps to i64
    //   %pps.as.pi = bitcast %struct.s** %pps to i64*
    //   store i64 %ps.as.i, i64* %pps.as.pi
    //
    if (!DTInfo.isTypeOfInterest(SrcTy))
      return;
    if (!DestTy->isIntegerTy(DL.getPointerSizeInBits())) {
      DEBUG(dbgs() << "DTRANS: (unsafe?) Pointer to aggregate type is cast to "
                      "a non-pointer-sized integer:\n    "
                   << I << "\n");
      setBaseTypeInfoSafetyData(SrcTy, dtrans::BadCasting);
    }

    // If we get here, we need to track the result as a value of interest
    // and analyze its uses as if they were pointers to the source type.
    setBaseTypeInfoSafetyData(SrcTy, dtrans::UnhandledUse);
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
      recordUnimplementedSafetyInfo(I, Ty);
    }

    for (Value *Arg : I.operands()) {
      llvm::Type *ArgTy = Arg->getType();
      if (DTInfo.isTypeOfInterest(ArgTy)) {
        recordUnimplementedSafetyInfo(I, ArgTy);
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
