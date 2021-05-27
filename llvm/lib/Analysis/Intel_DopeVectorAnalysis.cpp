//===------- Intel_DopeVectorAnalysis.cpp ----------------------- -*------===//
//
// Copyright (C) 2019-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_DopeVectorAnalysis.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

#define DEBUG_TYPE "dopevector-analysis"

namespace llvm {

namespace dvanalysis {

extern bool isDopeVectorType(const Type *Ty, const DataLayout &DL,
                             uint32_t *ArrayRank,
                             Type **ElementType) {
  const unsigned int DVFieldCount = 7;
  const unsigned int PerDimensionCount = 3;
  Type* ElemType = nullptr;
  uint32_t ArRank = 0;

  // Helper to check that all types contained in the structure in the range
  // of (Begin, End) are of type \p TargType
  auto ContainedTypesMatch = [](const StructType *StTy,
                                const llvm::Type *TargType,
                                unsigned int Begin, unsigned int End) {
    for (unsigned Idx = Begin; Idx < End; ++Idx)
      if (StTy->getContainedType(Idx) != TargType)
        return false;

    return true;
  };

  auto *StTy = dyn_cast<StructType>(Ty);
  if (!StTy)
    return false;

  unsigned ContainedCount = StTy->getNumContainedTypes();
  if (ContainedCount != DVFieldCount)
    return false;

  llvm::Type *FirstType = StTy->getContainedType(0);
  if (!FirstType->isPointerTy())
    return false;
  ElemType = FirstType->getPointerElementType();

  // All fields are "long" type?
  llvm::Type *LongType =
      Type::getIntNTy(Ty->getContext(), DL.getPointerSizeInBits());
  if (!ContainedTypesMatch(StTy, LongType, 1U, ContainedCount - 1))
    return false;

  // Array of structures for each rank?
  llvm::Type *LastType = StTy->getContainedType(ContainedCount - 1);
  auto *ArType = dyn_cast<ArrayType>(LastType);
  if (!ArType)
    return false;
  ArRank = ArType->getArrayNumElements();

  // Structure for extent, stride, and lower bound?
  llvm::Type *ElemTy = ArType->getArrayElementType();
  auto *StElemTy = dyn_cast<StructType>(ElemTy);
  if (!StElemTy)
    return false;
  if (StElemTy->getNumContainedTypes() != PerDimensionCount)
    return false;
  if (!ContainedTypesMatch(StElemTy, LongType, 0U, PerDimensionCount))
    return false;

  *ArrayRank = ArRank;
  *ElementType = ElemType;
  return true;
}

extern bool isDopeVectorType(const Type *Ty, const DataLayout &DL) {
  uint32_t ArrayRank = 0;
  Type *ElementType = nullptr;
  return isDopeVectorType(Ty, DL, &ArrayRank, &ElementType);
}

extern bool isUplevelVarType(Type *Ty) {
  // For now, just check the type of the variable as being named
  // "%uplevel_type[.#]" In the future, the front-end should provide some
  // metadata indicator that a variable is an uplevel.
  auto *StTy = dyn_cast<StructType>(Ty);
  if (!StTy || !StTy->hasName())
    return false;

  StringRef TypeName = StTy->getName();
  // Strip a '.' and any characters that follow it from the name.
  TypeName = TypeName.take_until([](char C) { return C == '.'; });
  if (TypeName != "uplevel_type")
    return false;

  return true;
}

extern bool isValidUseOfSubscriptCall(const SubscriptInst &Subs,
                                      const Value &Base,
                                      uint32_t ArrayRank, uint32_t Rank,
                                      bool CheckForTranspose,
                                      Optional<uint64_t> LowerBound,
                                      Optional<uint64_t> Stride) {
  LLVM_DEBUG({
    dbgs().indent((ArrayRank - Rank) * 2 + 4);
    dbgs() << "Checking call: " << Subs << "\n";
  });

  if (Subs.getArgOperand(PtrOpNum) != &Base)
    return false;

  if (CheckForTranspose) {
    unsigned RankParam = Subs.getRank();
    if (RankParam != Rank)
      return false;
  }

  if (LowerBound) {
    auto LBVal = dyn_cast<ConstantInt>(Subs.getLowerBound());
    if (!LBVal || LBVal->getLimitedValue() != *LowerBound)
      return false;
  }

  if (Stride) {
    auto StrideVal = dyn_cast<ConstantInt>(Subs.getStride());
    if (!StrideVal || StrideVal->getLimitedValue() != *Stride)
      return false;
  }

  return true;
}

// Helper function to check whether \p V is a GEP that corresponds to a field
// within an uplevel type.
static bool isFieldInUplevelTypeVar(Value *V) {
  auto *GEP = dyn_cast<GetElementPtrInst>(V);
  if (!GEP)
    return false;
  return isUplevelVarType(
      GEP->getPointerOperand()->getType()->getPointerElementType());
}

Optional<uint64_t> getConstGEPIndex(const GEPOperator &GEP,
                                    unsigned int OpNum) {
  auto FieldIndex = dyn_cast<ConstantInt>(GEP.getOperand(OpNum));
  if (FieldIndex)
    return Optional<uint64_t>(FieldIndex->getLimitedValue());
  return None;
}

Optional<unsigned int> getArgumentPosition(const CallBase &CI,
                                           const Value *Val) {
  Optional<unsigned int> Pos;
  unsigned int ArgCount = CI.getNumArgOperands();
  for (unsigned int ArgNum = 0; ArgNum < ArgCount; ++ArgNum)
    if (CI.getArgOperand(ArgNum) == Val) {
      if (Pos)
        return None;

      Pos = ArgNum;
    }

  return Pos;
}

// Return true if the input CallBase is a call to a function that allocates
// memory (e.g. for_alloc_allocate_handle)
static bool isCallToAllocFunction(CallBase *Call,
    std::function<const TargetLibraryInfo &(Function &F)> &GetTLI) {

  if (!Call || !Call->getCalledFunction())
    return false;

  Function *F = Call->getCalledFunction();

  LibFunc TheLibFunc;
  const TargetLibraryInfo &TLI = GetTLI(*const_cast<Function *>(F));
  if (TLI.getLibFunc(F->getName(), TheLibFunc) && TLI.has(TheLibFunc)) {
    // TODO: We may want to expand this in the future for other forms of
    // alloc
    switch (TheLibFunc) {
      case LibFunc_for_allocate_handle:
      case LibFunc_for_alloc_allocatable_handle:
        return true;
      default:
        return false;
    }
  }

  return false;
}

// Given a Value and the TargetLibraryInfo, check if it is a BitCast
// and is only used for data allocation. Return the call to the data
// alloc function.
static CallBase *bitCastUsedForAllocation(Value *Val,
    std::function<const TargetLibraryInfo &(Function &F)> &GetTLI) {

  if (!Val)
    return nullptr;

  auto *BC = dyn_cast<BitCastOperator>(Val);
  if (!BC || !BC->hasOneUser())
    return nullptr;

  CallBase *Call = dyn_cast<CallBase>(BC->user_back());
  if (!Call || !isCallToAllocFunction(Call, GetTLI))
    return nullptr;

  return Call;
}

// Return true if the input value is:
//   * A store instruction that writes into the input Pointer
//   * A load instruction
// Else return false. This function also updates if the dope vector field has
// been read or written.
bool DopeVectorFieldUse::analyzeLoadOrStoreInstruction(Value *V,
                                                       Value *Pointer) {
  if (!V)
    return false;

  if (auto *SI = dyn_cast<StoreInst>(V)) {
    // Make sure the store is to the field address, and that it's not the
    // field address being stored somewhere.
    if (SI->getValueOperand() != Pointer) {
      Stores.insert(SI);
      IsWritten = true;
    } else {
      return false;
    }
  } else if (auto *LI = dyn_cast<LoadInst>(V)) {
    Loads.insert(LI);
    IsRead = true;
  } else {
    return false;
  }

  return true;
}

// Traverse through the users of a field address and check where they are used
// for loading or storing data.
void DopeVectorFieldUse::analyzeUses() {
  if (IsBottom)
    return;

  if (FieldAddr.empty())
    return;

  for (auto *FAddr : FieldAddr) {
    for (auto *U : FAddr->users()) {
      if (!analyzeLoadOrStoreInstruction(U, FAddr)) {
          IsBottom = true;
          break;
      }
    }
  }
}

// Store the input subscript instruction if:
//  * The lower bound, stride and index are constants
//  * The index is in the limit of the dope vector rank (DVRank)
//  * The current dope vector field is the extent, stride or lowerbound
//
// This function basically collects the subscript instructions that are
// used to access the extent, stride and lower bound fields of a dope
// vector. All the subscripts will be analyzed in
// DopeVectorFieldUse::analyzeSubscriptUses().
void DopeVectorFieldUse::collectSubscriptInformation(SubscriptInst *SI,
    DopeVectorFieldType FieldType, unsigned long DVRank) {

  if (!SI) {
    IsBottom = true;
    return;
  }

  // This function is only for extent, stride and lowerbound
  assert(FieldType >= DopeVectorFieldType::DV_ExtentBase &&
         "Trying to add subscript information for a dope vector "
         "field that is not an extent, stride or lower bound");

  // All entries in the subscript should be constant
  if (!isa<ConstantInt>(SI->getLowerBound()) ||
      !isa<ConstantInt>(SI->getStride()) ||
      !isa<ConstantInt>(SI->getIndex())) {
    IsBottom = true;
    return;
  }

  // Collect the array entry and make sure it is within the bounds of DVRank
  ConstantInt *Index = cast<ConstantInt>(SI->getIndex());
  uint64_t ArrayEntry = Index->getZExtValue();
  if (ArrayEntry >= DVRank) {
    IsBottom = true;
    return;
  }

  Subscripts.insert(SI);
}

// Traverse through the users of a subscript instruction and check if they are
// load, store or PHINode instructions. The goal of this function is to find
// where the data for the extent, stride or lower bound is loaded and stored.
// Any unsupported use will set the dope vector field as bottom.
void DopeVectorFieldUse::analyzeSubscriptsUses() {

  // Return true if all the incoming values in the PHINode are subscript
  // instructions, they have the same rank, stide, lower bound, the pointer
  // operand as SI, and they are in the Subscripts set.
  auto AnalyzePHINode = [&, this](PHINode *PHI, SubscriptInst *SI) -> bool {
    for (unsigned int I = 0, E = PHI->getNumIncomingValues(); I < E; I++) {
      SubscriptInst *PHISI = dyn_cast<SubscriptInst>(PHI->getIncomingValue(I));
      if (!PHISI)
        return false;

      if (PHISI == SI)
        continue;

      if (PHISI->getRank() != SI->getRank() ||
          PHISI->getStride() != SI->getStride() ||
          PHISI->getLowerBound() != SI->getLowerBound() ||
          PHISI->getIndex() != SI->getIndex())
        return false;

      if (Subscripts.count(PHISI) == 0)
        return false;
    }

    // PHINodes should be used for loading data
    for (auto *U : PHI->users()) {
      if (auto *LI = dyn_cast<LoadInst>(U)) {
        Loads.insert(LI);
        IsRead = true;
      } else {
        return false;
      }
    }

    return true;
  };

  if (IsBottom)
    return;

  if (FieldAddr.empty() || Subscripts.empty())
    return;

  for (auto *SI : Subscripts) {
    // The operand of the subscript instruction should be the same as the
    // field address.
    if (FieldAddr.count(SI->getPointerOperand()) == 0) {
      IsBottom = true;
      return;
    }

    // Traverse through the users of the subscript instruction and check for
    // Load, Store and PHINodes.
    for (auto *U : SI->users()) {
      if (isa<StoreInst>(U) || isa<LoadInst>(U)) {
        // The subscript should be used for load or store
        if(!analyzeLoadOrStoreInstruction(U, SI)) {
          IsBottom = true;
          break;
        }
      } else if (auto *PHI = dyn_cast<PHINode>(U)) {
        // There is a chance that a subscript can be used in a PHINode. This
        // happens when the input code checks if the data is allocated takes
        // one path, else takes another path. If this is the case, then all
        // the incoming values of the PHINode should be subscript instructions
        // with the same rank, lower bound, pointer operand and index.
        if (!AnalyzePHINode(PHI, SI)) {
          IsBottom = true;
          break;
        }
      } else {
        IsBottom = true;
        break;
      }
    }
  }
}

// Traverse through the collected store instructions and identify the possible
// constant value for the current dope vector field.
void DopeVectorFieldUse::identifyConstantValue() {
  if (!getIsSingleValue())
    return;

  StoreInst *SI = *Stores.begin();
  ConstantInt *Const = dyn_cast<ConstantInt>(SI->getValueOperand());
  if (!Const)
    return;

  ConstantValue = Const;
}

// Turn on the property of storing multiple pointer accesses
void DopeVectorFieldUse::setAllowMultipleFieldAddresses() {

  // First check that no data was collected
  if (IsBottom || IsRead || IsWritten)
    return;

  if (!Loads.empty() || !Subscripts.empty() ||
      !Stores.empty() || !FieldAddr.empty() || ConstantValue)
    return;

  AllowMultipleFieldAddresses = true;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void DopeVectorFieldUse::dump() const { print(dbgs()); }
void DopeVectorFieldUse::print(raw_ostream &OS, const Twine &Header) const {
  OS << Header;
  print(OS);
}

void DopeVectorFieldUse::print(raw_ostream &OS) const {
  if (FieldAddr.empty()) {
    OS << "  Not set\n";
    return;
  }

  for (auto *FAddr : FieldAddr)
    OS << *FAddr << "\n";

  OS << "  Analysis :";
  OS << (IsBottom ? " BOTTOM" : "");
  OS << (IsRead ? " READ" : "");
  OS << (IsWritten ? " WRITTEN" : "");
  OS << "\n";

  OS << "  Stores   : " << Stores.size() << "\n";
  for (auto *V : Stores)
    OS << "    " << *V << "\n";

  OS << "  Loads    : " << Loads.size() << "\n";
  for (auto *V : Loads)
    OS << "    " << *V << "\n";
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

void DopeVectorAnalyzer::setInvalid() {
  LLVM_DEBUG(dbgs() << "  DV-Invalid: " << *DVObject << "\n");
  IsValid = false;
}

bool DopeVectorAnalyzer::checkMayBeModified() const {
  if (!IsValid)
    return true;

  if (PtrAddr.getIsBottom() || ElementSizeAddr.getIsBottom() ||
      CodimAddr.getIsBottom() || FlagsAddr.getIsBottom() ||
      DimensionsAddr.getIsBottom())
    return true;

  if (PtrAddr.getIsWritten() || ElementSizeAddr.getIsWritten() ||
      CodimAddr.getIsWritten() || FlagsAddr.getIsWritten() ||
      DimensionsAddr.getIsWritten())
    return true;

  for (const auto &Field : LowerBoundAddr)
    if (Field.getIsBottom() || Field.getIsWritten())
      return true;

  for (const auto &Field : StrideAddr)
    if (Field.getIsBottom() || Field.getIsWritten())
      return true;

  for (const auto &Field : ExtentAddr)
    if (Field.getIsBottom() || Field.getIsWritten())
      return true;

  return false;
}

bool
DopeVectorAnalyzer::checkArrayPointerUses(SubscriptInstSet *SubscriptCalls) {
  // Get a set of Value objects that hold the address of the array pointer.
  SmallPtrSet<Value *, 8> ArrayPtrValues;
  const DopeVectorFieldUse &PtrAddr = getPtrAddrField();
  if (!getAllValuesHoldingFieldValue(PtrAddr, ArrayPtrValues)) {
    LLVM_DEBUG(dbgs() << "Unsupported use of array pointer address:\n");
    return false;
  }

  // Now check all uses of the address to be sure they are only used to move
  // the address to another var (Select or PhiNode), or are used in a
  // subscript intrinsic call.
  for (auto *ArrPtr : ArrayPtrValues) {
    LLVM_DEBUG(dbgs() << "  Uses: " << *ArrPtr << "\n");
    for (auto *PtrUser : ArrPtr->users()) {
      if (isa<SelectInst>(PtrUser) || isa<PHINode>(PtrUser)) {
        continue;
      } else if (auto *Subs = dyn_cast<SubscriptInst>(PtrUser)) {
        uint32_t ArrayRank = getRank();
        if (!isValidUseOfSubscriptCall(*Subs, *ArrPtr, ArrayRank,
                                       ArrayRank - 1, SubscriptCalls)) {
          LLVM_DEBUG(dbgs() << "Array address: " << *ArrPtr
                            << " not in subscript call: " << *Subs << "\n");
          return false;
        }
        if (SubscriptCalls)
          SubscriptCalls->insert(Subs);
      } else {
        LLVM_DEBUG(dbgs() << "Unsupported use of array pointer address:\n"
                          << *PtrUser << "\n");
        return false;
      }
    }
  }

  return true;
}

// Forward declaration
static bool analyzeUplevelCallArg(uint32_t ArrayRank,
                                  SubscriptInstSet *SubscriptCalls, Function &F,
                                  uint64_t ArgPos, uint64_t FieldNum,
                                  SmallPtrSetImpl<const Function *> &Visited);

// This checks the uses of an uplevel variable for safety. Safe uses are:
// - If \p DVObject is non-null, we are analyzing the function that
//   initialized the uplevel var. In this case the dope vector member of the
//   uplevel can be written. Otherwise, writes are not allowed.
// - If the dope vector object is loaded from the uplevel variable, the uses
//   of the dope vector are checked to ensure the dope vector fields are not
//   modified.
// - If the uplevel variable is passed in a function call, a recursive call
//   will be made to this routine to check the usage of the uplevel in the
//   called function.
// Here 'ArrayRank' is the number of dimensions of the array represented
// by the dope vector, and 'SubscriptCalls', if not nullptr, is the set of
// subscript calls that reference the dope vector.
// 'Visited' is to avoid recursive re-entry into this routine when checking
// function calls made by 'F' that take the 'Uplevel' as a parameter.
static bool analyzeUplevelVar(uint32_t ArrayRank,
                              SubscriptInstSet *SubscriptCalls,
                              const Function &F, UplevelDVField &Uplevel,
                              Value *DVObject,
                              SmallPtrSetImpl<const Function *> &Visited) {
  if (!Visited.insert(&F).second)
    return true;

  Value *Var = Uplevel.first;
  uint64_t FieldNum = Uplevel.second;

  LLVM_DEBUG(dbgs() << "\nChecking use of uplevel variable in function: "
                    << F.getName() << " Field: " << FieldNum << "\n");

  // If the function makes use of the uplevel, then we expect there should be
  // an Instruction that is a GEP which gets the address of the DV field from
  // the uplevel variable. Collect all these GEPs into this vector for
  // analysis.
  SmallVector<GetElementPtrInst *, 4> DVFieldAddresses;

  // The uplevel variable may be passed to another function, collect the set
  // of {Function*, argument pos} pairs for functions that take this uplevel
  // as a parameter.
  FuncArgPosPairSet FuncsWithUplevelParams;

  for (auto *U : Var->users()) {
    auto *I = dyn_cast<Instruction>(U);
    assert(I && "Expected instruction\n");

    LLVM_DEBUG(dbgs() << "Uplevel var use: " << *I << "\n");

    if (auto *GEP = dyn_cast<GetElementPtrInst>(I)) {
      if (GEP->getNumIndices() == 2) {
        auto Idx0 = getConstGEPIndex(*(cast<GEPOperator>(GEP)), 1);
        auto Idx1 = getConstGEPIndex(*(cast<GEPOperator>(GEP)), 2);
        if (Idx0 && Idx1 && *Idx0 == 0) {
          // Ignore uses of other uplevel fields.
          if (*Idx1 != FieldNum)
            continue;

          DVFieldAddresses.push_back(GEP);
          continue;
        }
      }
      LLVM_DEBUG(dbgs() << "Unsupported usage of uplevel var:\n"
                        << *I << "\n");
      return false;
    } else if (auto *CI = dyn_cast<CallInst>(I)) {
      Function *F = CI->getCalledFunction();
      if (!F) {
        LLVM_DEBUG(dbgs() << "Uplevel var passed in indirect function call:\n"
                          << *CI << "\n");
        return false;
      }
      Optional<unsigned int> ArgPos = getArgumentPosition(*CI, Var);
      if (!ArgPos) {
        LLVM_DEBUG(dbgs() << "Uplevel var argument not unique in call:\n"
                          << *CI << "\n");
        return false;
      }
      FuncsWithUplevelParams.insert(FuncArgPosPair(F, *ArgPos));
    } else {
      LLVM_DEBUG(dbgs() << "Unsupported usage of uplevel var:\n"
                        << *I << "\n");

      return false;
    }
  }

  // Check the usage for all the GEPs that get the address of the dope vector
  // variable.
  // If the dope vector pointer field is loaded, check that all uses of the
  // dope vector are safe. If the dope vector pointer field is stored, check
  // that it is the write we expected that is initializing the uplevel.
  for (auto *DVFieldAddr : DVFieldAddresses)
    for (auto *U : DVFieldAddr->users()) {
      auto *I = dyn_cast<Instruction>(U);
      assert(I && "Expected instruction\n");

      if (auto *LI = dyn_cast<LoadInst>(I)) {
        DopeVectorAnalyzer DVA(LI);
        DVA.analyze(false);
        if (!DVA.analyzeDopeVectorUseInFunction(F, SubscriptCalls))
          return false;
      } else if (auto *SI = dyn_cast<StoreInst>(I)) {
        // The only store we expect to the DV field is the dope vector object
        // currently being analyzed.
        if (!DVObject || SI->getValueOperand() != DVObject) {
           LLVM_DEBUG(dbgs()
                   << "Store into uplevel var dope vector field no allowed\n");
           return false;
        }
      } else {
        return false;
      }
    }

  // Check all the functions that take the uplevel variable.
  for (auto &FuncArg : FuncsWithUplevelParams)
    if (!analyzeUplevelCallArg(ArrayRank, SubscriptCalls, *FuncArg.first,
                               FuncArg.second, FieldNum, Visited))
      return false;

  return true;
}

// Check a called function for usage of the uplevel variable for safety.
// Here 'ArrayRank' is the number of dimensions of the array represented
// by the dope vector, and 'SubscriptCalls', if not nullptr, is the set of
// subscript calls that reference the dope vector.
static bool analyzeUplevelCallArg(uint32_t ArrayRank,
                                  SubscriptInstSet *SubscriptCalls, Function &F,
                                  uint64_t ArgPos, uint64_t FieldNum,
                                  SmallPtrSetImpl<const Function *> &Visited) {
  if (F.isDeclaration())
    return false;

  assert(ArgPos < F.arg_size() && "Invalid argument position");
  auto Args = F.arg_begin();
  std::advance(Args, ArgPos);
  Argument *FormalArg = &(*Args);

  // Check the called function for its use of the uplevel passed in. We do
  // not allow the called function to store a new dope vector into the field,
  // so pass 'nullptr' for the DVObject.
  UplevelDVField LocalUplevel(FormalArg, FieldNum);
  if (!analyzeUplevelVar(ArrayRank, SubscriptCalls, F, LocalUplevel, nullptr,
                         Visited))
    return false;

  return true;
}

bool DopeVectorAnalyzer::getAllValuesHoldingFieldValue(
                                   const DopeVectorFieldUse &Field,
                                   SmallPtrSetImpl<Value *> &ValueSet) const {
  // Prime a worklist with all the direct loads of the field.
  SmallVector<Value *, 16> Worklist;
  llvm::copy(Field.loads(), std::back_inserter(Worklist));

  // Populate the set of objects containing the value loaded.
  while (!Worklist.empty()) {
    Value *V = Worklist.back();
    Worklist.pop_back();
    if (!ValueSet.insert(V).second)
      continue;

    for (auto *U : V->users())
      if ((isa<SelectInst>(U) || isa<PHINode>(U)) && !ValueSet.count(U))
        Worklist.push_back(U);
  }

  // Verify all the source nodes for PHI nodes and select instructions
  // originate from the field load (or another PHI/select).
  SmallVector<Value *, 4> IncomingVals;
  for (auto *V : ValueSet) {
    IncomingVals.clear();
    if (auto *Sel = dyn_cast<SelectInst>(V)) {
      IncomingVals.push_back(Sel->getTrueValue());
      IncomingVals.push_back(Sel->getFalseValue());
    } else if (auto *PHI = dyn_cast<PHINode>(V)) {
      for (Value *Val : PHI->incoming_values())
        IncomingVals.push_back(Val);
    }

    for (auto *ValIn : IncomingVals)
      if (!ValueSet.count(ValIn)) {
        LLVM_DEBUG(dbgs() << "Failed during check of:\n"
                          << *V
                          << "\nExpected PHI/select source to also be "
                             "in field value set: "
                          << *ValIn << "\n");
        return false;
      }
  }

  return true;
}

void DopeVectorAnalyzer::analyze(bool ForCreation) {
  LLVM_DEBUG(dbgs() << "\nChecking "
                    << (ForCreation ? "construction" : "use")
                    << " of dope vector: " << *DVObject << "\n");

  // Assume valid, until proven otherwise.
  IsValid = true;

  GetElementPtrInst *PerDimensionBase = nullptr;
  GetElementPtrInst *ExtentBase = nullptr;
  GetElementPtrInst *StrideBase = nullptr;
  GetElementPtrInst *LowerBoundBase = nullptr;

  for (auto *DVUser : DVObject->users()) {
    LLVM_DEBUG(dbgs() << "  DV user: " << *DVUser << "\n");
    if (auto *GEP = dyn_cast<GetElementPtrInst>(DVUser)) {
      // Find which of the fields this GEP is the address of.
      // Note: We expect the field addresses to only be seen at most one
      // time for each field, otherwise we do not support it.
      DopeVectorFieldType DVFieldType =
          identifyDopeVectorField(*(cast<GEPOperator>(GEP)));
      switch (DVFieldType) {
      default:
        setInvalid();
        return;

      case DopeVectorFieldType::DV_ArrayPtr:
        PtrAddr.addFieldAddr(GEP);
        break;
      case DopeVectorFieldType::DV_ElementSize:
        ElementSizeAddr.addFieldAddr(GEP);
        break;
      case DopeVectorFieldType::DV_Codim:
        CodimAddr.addFieldAddr(GEP);
        break;
      case DopeVectorFieldType::DV_Flags:
        FlagsAddr.addFieldAddr(GEP);
        break;
      case DopeVectorFieldType::DV_Dimensions:
        DimensionsAddr.addFieldAddr(GEP);
        break;
      case DV_Reserved:
        // Ignore uses of reserved
        break;

        // The following fields require additional forward looking analysis to
        // get to the actual address-of objects.
      case DopeVectorFieldType::DV_PerDimensionArray:
        if (PerDimensionBase) {
          setInvalid();
          return;
        }
        PerDimensionBase = GEP;
        break;
      case DopeVectorFieldType::DV_LowerBoundBase:
        if (LowerBoundBase) {
          setInvalid();
          return;
        }
        LowerBoundBase = GEP;
        break;
      case DopeVectorFieldType::DV_ExtentBase:
        if (ExtentBase) {
          setInvalid();
          return;
        }
        ExtentBase = GEP;
        break;

      case DopeVectorFieldType::DV_StrideBase:
        if (StrideBase) {
          setInvalid();
          return;
        }
        StrideBase = GEP;
        break;
      }
    } else if (const auto *CI = dyn_cast<CallInst>(DVUser)) {
      Function *F = CI->getCalledFunction();
      if (!F) {
        LLVM_DEBUG(dbgs() << "Dope vector passed in indirect function call:\n"
                          << *CI << "\n");
        setInvalid();
        return;
      }

      Optional<unsigned int> ArgPos = getArgumentPosition(*CI, DVObject);
      if (!ArgPos) {
        LLVM_DEBUG(dbgs() << "Dope vector argument not unique in call:\n"
                          << *CI << "\n");
        setInvalid();
        return;
      }

      // Save the function for later analysis.
      FuncsWithDVParam.insert({F, *ArgPos});
    } else if (auto *SI = dyn_cast<StoreInst>(DVUser)) {
      // Check if the store is saving the dope vector object into an uplevel
      // var. Save the variable and field number for later analysis. (The
      // dope vector should only ever need to be stored to a single uplevel,
      // but make sure we didn't see one yet.)
      if (SI->getValueOperand() == DVObject) {
        Value *PtrOp = SI->getPointerOperand();
        if (isFieldInUplevelTypeVar(PtrOp) && Uplevel.first == nullptr) {
          LLVM_DEBUG(dbgs() << "Dope vector needs uplevel analysis: "
                            << *SI << "\n");
          auto PtrGEP = cast<GEPOperator>(PtrOp);
          auto Idx0 = getConstGEPIndex(*PtrGEP, 1);
          auto Idx1 = getConstGEPIndex(*PtrGEP, 2);
          if (Idx0 && Idx1 && *Idx0 == 0) {
            Uplevel = UplevelDVField(PtrGEP->getPointerOperand(), *Idx1);
            continue;
          }
        }
      }

      LLVM_DEBUG(dbgs() << "Unsupported StoreInst using dope vector object\n"
                        << *DVUser << "\n");
      setInvalid();
      return;
    } else {
      LLVM_DEBUG(dbgs() << "Unsupported use of dope vector object\n"
                        << *DVUser << "\n");
      setInvalid();
      return;
    }
  }

  // We expect either the per-dimension base or base addresses of the
  // individual components, not both.
  if (PerDimensionBase) {
    if (ExtentBase || StrideBase || LowerBoundBase) {
      setInvalid();
      return;
    }

    std::pair<GetElementPtrInst *, FindResult> Result;
    Result = findPerDimensionArrayFieldGEP(*PerDimensionBase, DVR_Extent);
    if (Result.second == FR_Valid)
      ExtentBase = Result.first;
    Result = findPerDimensionArrayFieldGEP(*PerDimensionBase, DVR_Stride);
    if (Result.second == FR_Valid)
      StrideBase = Result.first;
    Result = findPerDimensionArrayFieldGEP(*PerDimensionBase, DVR_LowerBound);
    if (Result.second == FR_Valid)
      LowerBoundBase = Result.first;
  }

  // Check the uses of the fields to make sure there are no unsupported uses,
  // and collect the loads and stores. For the PtrAddr field, we will need to
  // later analyze all the reads that get the address of the array to ensure
  // the address does not escape the module. For the dope vector strides, we
  // will need to analyze all the writes to the field to be sure the expected
  // value is being stored. For other fields, we may not need to collect all
  // the loads and stores, but for now, collect them all.
  PtrAddr.analyzeUses();
  ElementSizeAddr.analyzeUses();
  CodimAddr.analyzeUses();
  FlagsAddr.analyzeUses();
  DimensionsAddr.analyzeUses();

  // During dope vector creation, we expect to be see all the fields being set
  // up.
  if (ForCreation) {
    if (!PtrAddr.hasFieldAddr() || !ElementSizeAddr.hasFieldAddr() ||
        !CodimAddr.hasFieldAddr() || !FlagsAddr.hasFieldAddr() ||
        !DimensionsAddr.hasFieldAddr()) {
      LLVM_DEBUG(dbgs() << "Unsupported use of dope vector object: "
                           "Could not find addresses for all fields.\n");
      setInvalid();
      return;
    }
  }
  // Verify all the uses of the fields present were successfully analyzed.
  if (PtrAddr.getIsBottom() || ElementSizeAddr.getIsBottom() ||
      CodimAddr.getIsBottom() || FlagsAddr.getIsBottom() ||
      DimensionsAddr.getIsBottom()) {
    LLVM_DEBUG(dbgs() << "Unsupported use of dope vector object: Could "
                         "not analyze all fields.\n");

    setInvalid();
    return;
  }

  // If a field was found that corresponds to the Extent, Stride or
  // LowerBounds fields, reserve space for all of them, then collect all the
  // loads/stores that use those fields.
  if (ExtentBase || StrideBase || LowerBoundBase) {
    ExtentAddr.resize(Rank);
    StrideAddr.resize(Rank);
    LowerBoundAddr.resize(Rank);
    for (unsigned Dim = 0; Dim < Rank; ++Dim) {
      if (ExtentBase) {
        Value *Ptr = findPerDimensionArrayFieldPtr(*ExtentBase, Dim);
        if (Ptr) {
          DopeVectorFieldUse &ExtentField = ExtentAddr[Dim];
          ExtentField.addFieldAddr(Ptr);
          ExtentField.analyzeUses();
          if (ExtentField.getIsBottom()) {
            setInvalid();
            return;
          }
        }
      }
      if (StrideBase) {
        Value *Ptr = findPerDimensionArrayFieldPtr(*StrideBase, Dim);
        if (Ptr) {
          DopeVectorFieldUse &StrideField = StrideAddr[Dim];
          StrideField.addFieldAddr(Ptr);
          StrideField.analyzeUses();
          if (StrideField.getIsBottom()) {
            setInvalid();
            return;
          }
        }
      }
      if (LowerBoundBase) {
        Value *Ptr = findPerDimensionArrayFieldPtr(*LowerBoundBase, Dim);
        if (Ptr) {
          DopeVectorFieldUse &LBField = LowerBoundAddr[Dim];
          LBField.addFieldAddr(Ptr);
          LBField.analyzeUses();
          if (LBField.getIsBottom()) {
            setInvalid();
            return;
          }
        }
      }

      // For dope vector creation, we expect to find writes for all the fields
      if (ForCreation) {
        if (!ExtentAddr[Dim].hasFieldAddr() ||
            !StrideAddr[Dim].hasFieldAddr() ||
            !LowerBoundAddr[Dim].hasFieldAddr()) {
          LLVM_DEBUG(dbgs() << "Unsupported use of dope vector object: Could "
                               "not find addresses for all ranks.\n");
          setInvalid();
          return;
        }

        if (!ExtentAddr[Dim].getIsWritten() ||
            !StrideAddr[Dim].getIsWritten() ||
            !LowerBoundAddr[Dim].getIsWritten()) {
          LLVM_DEBUG(dbgs() << "Unsupported use of dope vector object: "
                               "Could not find writes for all ranks.\n");
          setInvalid();
          return;
        }
      }
    }
  }
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void DopeVectorAnalyzer::dump() const { print(dbgs()); }

void DopeVectorAnalyzer::print(raw_ostream &OS) const {
  OS << "DopeVectorAnalyzer: " << *DVObject << "\n";
  OS << "IsValid: " << (IsValid ? "true" : "false") << "\n";

  PtrAddr.print(OS, "PtrAddr:");
  ElementSizeAddr.print(OS, "ElementSize:");
  CodimAddr.print(OS, "Codim:");
  FlagsAddr.print(OS, "Flags:");
  DimensionsAddr.print(OS, "Dimensions:");
  for (unsigned Dim = 0; Dim < LowerBoundAddr.size(); ++Dim) {
    std::string DimStr = std::to_string(Dim);
    LowerBoundAddr[Dim].print(OS, "LowerBound" + DimStr);
  }
  for (unsigned Dim = 0; Dim < StrideAddr.size(); ++Dim) {
    std::string DimStr = std::to_string(Dim);
    StrideAddr[Dim].print(OS, "Stride" + DimStr);
  }

  for (unsigned Dim = 0; Dim < ExtentAddr.size(); ++Dim) {
    std::string DimStr = std::to_string(Dim);
    ExtentAddr[Dim].print(OS, "Extent" + DimStr);
  }
  OS << "\n";
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

DopeVectorFieldType
DopeVectorAnalyzer::identifyDopeVectorField(const GEPOperator &GEP,
                                            uint64_t DopeVectorIndex) {
  assert(GEP.getSourceElementType()->isStructTy() && "Expected struct type");

  // Array index should always be zero.
  auto ArrayIdx = getConstGEPIndex(GEP, 1);
  if (!ArrayIdx || *ArrayIdx != 0)
    return DopeVectorFieldType::DV_Invalid;

  // If the dope vector index specified by the user is 1 then return
  // DopeVectorFieldType::DV_Invalid. Index 0 is reserved for the array
  // index and it's value should be 0.
  if (DopeVectorIndex == 1)
    return DopeVectorFieldType::DV_Invalid;

  // If DopeVectorIndex is greater than 0, then we need to compute the offset
  // of the rest of indices that access the fields of the dope vector.
  uint64_t DopeVectorOffSet = 0;
  if (DopeVectorIndex > 0)
    DopeVectorOffSet = DopeVectorIndex - 1;

  unsigned NumIndices = GEP.getNumIndices();
  if (NumIndices < 2 + DopeVectorOffSet || NumIndices > 4 + DopeVectorOffSet)
    return DopeVectorFieldType::DV_Invalid;

  // The address for the first 6 fields of the dope vector are accessed
  // directly with a GEP of the form:
  //     %field4 = getelementptr
  //               { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] },
  //               { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }*
  //               %"var$08", i64 0, i32 4
  if (NumIndices == 2 + DopeVectorOffSet) {
    auto FieldIdx = getConstGEPIndex(GEP, 2 + DopeVectorOffSet);
    assert(FieldIdx &&
           "Field index should always be constant for struct type");
    assert(FieldIdx
        <= static_cast<uint64_t>(DopeVectorFieldType::DV_PerDimensionArray) &&
        "expected dope vector to have a maximum of 7 fields");
    return static_cast<DopeVectorFieldType>(*FieldIdx);
  }

  // The per-dimension array elements may be accessed using either of the
  // following forms:
  //   %16 = getelementptr
  //         { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] },
  //         { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %2,
  //         i64 0, i32 6, i64 0
  //
  // or:
  //
  //   %14 = getelementptr { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64,
  //   i64 }] },
  //         { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* %3,
  //         i64 0, i32 6, i64 0, i32 1
  //
  // For the first form, another GEP will follow to get the index from the
  // per-array dimension. For the second form, the field may be passed
  // directly to a subscript intrinsic.
  if (NumIndices == 3 + DopeVectorOffSet) {
    auto FieldIdx = getConstGEPIndex(GEP, 2 + DopeVectorOffSet);
    if (FieldIdx != static_cast<uint64_t>(DV_PerDimensionArray))
      return DopeVectorFieldType::DV_Invalid;

    // We only expect the GEP to use 0 for last index which corresponds to the
    // per-dimension array base, and then be followed by another GEP to get
    // the specific structure element.
    auto SubIdx = getConstGEPIndex(GEP, 3 + DopeVectorOffSet);
    assert(SubIdx && "Field index should always be constant for struct type");
    if (*SubIdx != 0)
      return DopeVectorFieldType::DV_Invalid;
    return DopeVectorFieldType::DV_PerDimensionArray;
  }

  assert(NumIndices == 4 + DopeVectorOffSet && "Only expected case 4 to be left");

  // The second form of access directly gets the address of the Lower Bound,
  // Stride or Extent field of the first array element.
  auto SubIdx = getConstGEPIndex(GEP, 4 + DopeVectorOffSet);
  assert(SubIdx && "Field index should always be constant for struct type");
  switch (*SubIdx) {
  default:
    return DopeVectorFieldType::DV_Invalid;
  case 0:
    return DopeVectorFieldType::DV_ExtentBase;
  case 1:
    return DopeVectorFieldType::DV_StrideBase;
  case 2:
    return DopeVectorFieldType::DV_LowerBoundBase;
  }
  return DopeVectorFieldType::DV_Invalid;
}

std::pair<GetElementPtrInst *, DopeVectorAnalyzer::FindResult>
DopeVectorAnalyzer::findPerDimensionArrayFieldGEP(GetElementPtrInst &GEP,
                              DopeVectorRankFields RankFieldType) {
  std::pair<GetElementPtrInst *, FindResult> InvalidResult = {nullptr,
                                                              FR_Invalid};
  unsigned int FieldNum;
  switch (RankFieldType) {
  case DVR_Extent:
    FieldNum = 0;
    break;
  case DVR_Stride:
    FieldNum = 1;
    break;
  case DVR_LowerBound:
    FieldNum = 2;
    break;
  }

  // Find the GEP that corresponds to the per-dimension element wanted. There
  // should only be one, if there are more, we do not support it.
  GetElementPtrInst *FieldGEP = nullptr;
  for (auto *U : GEP.users()) {
    if (auto *GEPU = dyn_cast<GetElementPtrInst>(U)) {
      if (GEPU->getNumIndices() != 2)
        return InvalidResult;

      auto ArIdx = getConstGEPIndex(*(cast<GEPOperator>(GEPU)), 1);
      if (!ArIdx || *ArIdx != 0)
        return InvalidResult;

      // Check that there is only one instance of field being searched for.
      auto FieldIdx = getConstGEPIndex(*(cast<GEPOperator>(GEPU)), 2);
      assert(FieldIdx && "Field index of struct must be constant");
      if (*FieldIdx == FieldNum) {
        if (FieldGEP)
          return InvalidResult;
        FieldGEP = GEPU;
      }
    } else {
      return InvalidResult;
    }
  }

  // No instances using field. Return a constructed value that holds a
  // nullptr, as a valid analysis result.
  if (!FieldGEP)
    return {nullptr, FR_Valid};

  return {FieldGEP, FR_Valid};
}

Value *
DopeVectorAnalyzer::findPerDimensionArrayFieldPtr(GetElementPtrInst &FieldGEP,
                                                  unsigned Dimension) {
  // Find the address element
  Instruction *Addr = nullptr;
  for (auto *U : FieldGEP.users()) {
    if (auto *Subs = dyn_cast<SubscriptInst>(U)) {
      auto *IdxVal = dyn_cast<ConstantInt>(Subs->getIndex());
      if (!IdxVal)
        return nullptr;
      if (IdxVal->getLimitedValue() == Dimension) {
        if (Addr)
          return nullptr;
        Addr = Subs;
      }
    } else {
      return nullptr;
    }
  }

  return Addr;
}

bool
DopeVectorAnalyzer::analyzeDopeVectorUseInFunction(const Function &F,
                                                   SubscriptInstSet
                                                      *SubscriptCalls) {
  LLVM_DEBUG({
    dbgs() << "\nDope vector collection for function: " << F.getName() << "\n"
           << *getDVObject() << "\n";
    dump();
  });

  // Verify that the dope vector fields are not written.
  if (checkMayBeModified()) {
    LLVM_DEBUG(dbgs() << "Dope vector fields modified in function: "
                      << F.getName() << "\n");
    return false;
  }

  // Check that the DV object was not forwarded to another function call. We
  // could allow this by analyzing all the uses within that function,
  // but we currently do not.
  if (getNumberCalledFunctions()) {
    LLVM_DEBUG(dbgs() << "Dope vector passed to another function within: "
                      << F.getName() << "\n");
    return false;
  }

  // Check that the array pointer does not escape to another memory location.
  SubscriptInstSet LocalSubscriptCalls;
  SubscriptInstSet *LocalSubscriptCallsCheck = SubscriptCalls ?
      &LocalSubscriptCalls : nullptr;
  if (!checkArrayPointerUses(LocalSubscriptCallsCheck)) {
    LLVM_DEBUG(dbgs() << "Array pointer address may escape from: "
                      << F.getName() << "\n");
    return false;
  }

  if (SubscriptCalls && !LocalSubscriptCalls.empty()) {
    // Check the stride value used in the subscript calls.
    if (!checkSubscriptStrideValues(LocalSubscriptCalls)) {
      LLVM_DEBUG(dbgs() << "Subscript call with unsupported stride in: "
                        << F.getName() << "\n");
      return false;
    }

    // Save the set of subscript calls that use the dope vector for
    // profitability analysis.
    SubscriptCalls->insert(LocalSubscriptCalls.begin(),
      LocalSubscriptCalls.end());
  }

  // If there was a store of the dope vector into an uplevel variable, check
  // the uses of the uplevel variable.
  UplevelDVField Uplevel = getUplevelVar();
  SmallPtrSet<const Function *, 16> Visited;
  if (Uplevel.first && !analyzeUplevelVar(getRank(), SubscriptCalls, F, Uplevel,
                                          getDVObject(), Visited))
    return false;
  return true;
}

bool
DopeVectorAnalyzer::checkSubscriptStrideValues(const SubscriptInstSet
                                                    &SubscriptCalls) {
  SmallVector<SmallPtrSet<Value *, 4>, FortranMaxRank> StrideLoads;

  // Function to check the stride value used in an instruction that is a
  // subscript call or holds the result of a subscript call. This is to
  // verify that the value used for the stride was identified as one of the
  // expected values when information was collected for the dope vector.
  //
  // If the instruction is a subscript call, then verify that the value used for
  // the stride operand is a member of the \p StrideLoads set. If the subscript
  // call is not the final Dimension, then recurse to check the stride value
  // for the next level subscript call.
  // If the instruction is a PHINode or Select instruction that holds the result
  // of a subscript call, then check all the users to find where the subscript
  // result is used for next dimension.
  std::function<bool(const SmallVectorImpl<SmallPtrSet<Value *, 4>> &,
                     const Instruction &, uint32_t,
                     SmallPtrSetImpl<const Instruction *> &)>
      CheckCall =
          [&CheckCall](
              const SmallVectorImpl<SmallPtrSet<Value *, 4>> &StrideLoads,
              const Instruction &I, uint32_t Dimension,
              SmallPtrSetImpl<const Instruction *> &Visited) -> bool {
    if (!Visited.insert(&I).second)
      return true;

    if (auto *Subs = dyn_cast<SubscriptInst>(&I)) {
      Value *StrideOp = Subs->getStride();
      if (!StrideLoads[Dimension].count(StrideOp))
        return false;

      if (Dimension == 0)
        return true;
    }

    for (auto *UU : I.users())
      if (auto *Subs2 = dyn_cast<SubscriptInst>(UU)) {
        if (!CheckCall(StrideLoads, *Subs2, Dimension - 1, Visited))
          return false;
      } else if (isa<SelectInst>(UU) || isa<PHINode>(UU)) {
        const Instruction *I2 = dyn_cast<Instruction>(UU);
        if (!CheckCall(StrideLoads, *I2, Dimension, Visited))
          return false;
      } else {
        return false;
      }

    return true;
  };

  // For each dimension of the variable, get the set of objects that hold the
  // value for the stride loaded from the dope vector object.
  for (unsigned Dim = 0; Dim < getRank(); ++Dim) {
    if (!hasStrideField(Dim))
      return false;

    const DopeVectorFieldUse &StrideField = getStrideField(Dim);
    StrideLoads.push_back(SmallPtrSet<Value *, 4>());
    auto &LoadSet = StrideLoads.back();
    bool Valid = getAllValuesHoldingFieldValue(StrideField, LoadSet);
    if (!Valid)
      return false;
  }

  // Check all the subscript calls to ensure the stride value comes from the
  // dope vector.
  SmallPtrSet<const Instruction*, 16> Visited;
  for (auto *Subs : SubscriptCalls)
    if (!CheckCall(StrideLoads, *Subs, getRank() - 1, Visited))
      return false;

  return true;
}

// Given an Value and LLVM Type that was already identified as dope vector then
// construct a DopeVectorInfo.
DopeVectorInfo::DopeVectorInfo(Value *DVObject, Type *DVType,
                               bool AllowMultipleFieldAddresses) :
    DVObject(DVObject), AllocSite(nullptr), AllocSiteSet(false) {

  assert(DVType->isStructTy() && DVType->getStructNumElements() == 7 &&
         DVType->getContainedType(DopeVectorFieldType::DV_PerDimensionArray)
               ->isArrayTy() && "Invalid type for dope vector object");
  // The rank of the dope vector can be determined using the array length of
  // array that is the last field of the dope vector.
  Rank = DVType->getContainedType(DopeVectorFieldType::DV_PerDimensionArray)
               ->getArrayNumElements();

  AnalysisRes = DopeVectorInfo::AnalysisResult::AR_Top;
  LLVMDVType = cast<StructType>(DVType);
  ConstantsPropagated = false;

  ExtentAddr.resize(Rank);
  StrideAddr.resize(Rank);
  LowerBoundAddr.resize(Rank);

  if (AllowMultipleFieldAddresses) {
    PtrAddr.setAllowMultipleFieldAddresses();
    ElementSizeAddr.setAllowMultipleFieldAddresses();
    CodimAddr.setAllowMultipleFieldAddresses();
    FlagsAddr.setAllowMultipleFieldAddresses();
    DimensionsAddr.setAllowMultipleFieldAddresses();

    for (unsigned long I = 0; I < Rank; I++) {
      ExtentAddr[I].setAllowMultipleFieldAddresses();
      StrideAddr[I].setAllowMultipleFieldAddresses();
      LowerBoundAddr[I].setAllowMultipleFieldAddresses();
    }
  }
}

// Given a dope vector field type, return the DopeVectorFieldUse corresponding
// to that field. If the field is DV_ExtentBase, DV_StrideBase or
// DV_LowerBoundBase then the ArrayEntry is needed. If the field is
// DV_Reserved, then return nullptr.
DopeVectorFieldUse* DopeVectorInfo::getDopeVectorField(
    DopeVectorFieldType DVFieldType, uint64_t ArrayEntry) {

  // Return the DopeVectorField use corresponding to the array entry
  auto GetArrayEntry = [&, ArrayEntry](SmallVectorImpl<DopeVectorFieldUse>
                                       &DVFieldArray) -> DopeVectorFieldUse* {

    assert(ArrayEntry < DVFieldArray.size() && "Trying to access an out of "
        "bounds entry in the per dimension array");

    return &(DVFieldArray[ArrayEntry]);
  };

  switch(DVFieldType) {
    case DopeVectorFieldType::DV_ArrayPtr:
      return &PtrAddr;
    case DopeVectorFieldType::DV_ElementSize:
      return &ElementSizeAddr;
    case DopeVectorFieldType::DV_Codim:
      return &CodimAddr;
    case DopeVectorFieldType::DV_Flags:
      return &FlagsAddr;
    case DopeVectorFieldType::DV_Dimensions:
      return &DimensionsAddr;
    case DopeVectorFieldType::DV_Reserved:
      return nullptr;
    case DopeVectorFieldType::DV_PerDimensionArray:
      return &DimensionsAddr;
    case DopeVectorFieldType::DV_ExtentBase:
      return GetArrayEntry(ExtentAddr);
    case DopeVectorFieldType::DV_StrideBase:
      return GetArrayEntry(StrideAddr);
    case DopeVectorFieldType::DV_LowerBoundBase:
      return GetArrayEntry(LowerBoundAddr);
    default:
      llvm_unreachable("Trying to collect field from the dope "
                       "vector that is out of bounds");
  }

  return nullptr;
}

// This function checks if the information for all the fields in the dope
// vector was collected correctly.
void DopeVectorInfo::validateDopeVector() {

  // We are going to follow the same principle as global constant propagation.
  // If there is only one store instruction and the global variable is loaded
  // anywhere, then the global variable should be initialized first (store
  // must happen before the load). Else, if the store instruction wasn't
  // executed it means that the user code didn't properly initialize the
  // global variable. In this case the load instruction is loading garbage,
  // which is an undefined behavior.
  //
  // In this case, the user is accessing an array, therefore it must be
  // allocated before accessing any information, if not this is undefined
  // behavior. The dope vector is used to access information in the array
  // therefore the information for the dope vector must be initialized
  // first and the array it must be allocated. If there is one store
  // instruction for a dope vector field and that store instruction will
  // always execute if the call to allocate the array executes, then it means
  // that the dope vector field will be initialized when the array is
  // allocated. Any load to the dope vector field will use initialized data
  // because the array was allocated (store must happen before the load).
  // If there is an use of the dope vector field without allocating the array
  // first then there is an access to an unallocated array, which is
  // undefined behavior.
  auto StoreHappensWithAllocation =
      [this](DopeVectorFieldUse &Field) -> bool {
    if (!Field.getIsSingleValue())
      return false;

    StoreInst *SI = *Field.stores().begin();

    // If the store instruction is in the same basic block as the alloc-site,
    // then it means that the store will execute since the allocation will
    // execute.
    //
    // NOTE: This is conservative. We may need to extend this in the future
    // to address the following cases:
    //
    //   1) Function "foo" can store to information to the dope vector fields,
    //      then it calls "bar" which allocates the array. If there is no
    //      branching between the store instructions and the call to "bar",
    //      nor between the entry block in "bar" and the call to the alloc
    //      function, then the store instruction and the alloc site will
    //      always execute (foo -> bar -> alloc).
    //
    //   2) If there are unconditional branches between the store
    //      instructions and the call to allocate function.
    //
    //   3) If there are conditional branches but we can prove that
    //      it won't matter which path is taken, the store and the call
    //      to alloc will execute.
    //
    //   4) If there are conditional branches but we can prove at compile
    //      time which path will always be taken.
    if (SI->getParent() != AllocSite->getParent())
      return false;

    return true;
  };

  // Invalidate the data collected if at least one field is Bottom,
  // if writing or reading into the field is not allowed, or if
  // it didn't pass the store-alloc test.
  auto ValidateDopeVectorField =
      [&StoreHappensWithAllocation, this](DopeVectorFieldUse &Field,
                                          bool ComputeConstant,
                                          bool WriteAllowed,
                                          bool ReadAllowed) -> bool {
    if (Field.getIsBottom())
      return false;

    if (!WriteAllowed && Field.getIsWritten()) {
      AnalysisRes = DopeVectorInfo::AnalysisResult::AR_WriteIllegality;
      return false;
    }

    if (!ReadAllowed && Field.getIsRead()) {
      AnalysisRes = DopeVectorInfo::AnalysisResult::AR_ReadIllegality;
      return false;
    }

    if (ComputeConstant)
      Field.identifyConstantValue();

    // If we found the constant, then we need to make sure that the store
    // executes every time the memory is allocated.
    if (Field.getConstantValue() &&
        !StoreHappensWithAllocation(Field)) {
      AnalysisRes = DopeVectorInfo::AnalysisResult::AR_AllocStoreIllegality;
      return false;
    }

    return true;
  };

  if (AnalysisRes == DopeVectorInfo::AnalysisResult::AR_Invalid)
    return;

  if (!AllocSite) {
    AnalysisRes = DopeVectorInfo::AnalysisResult::AR_NoAllocSite;
    return;
  }

  bool PassValidation = true;

  // Pointer address should not be written, only allocated and read
  PassValidation &= ValidateDopeVectorField(PtrAddr,
                                            false /* ComputeConstant */,
                                            false /* WriteAllowed */,
                                            true /* ReadAllowed */);

  // The element size, co-dimension, flags and dimensions should be
  // written but not read. This is for storing information that the
  // compiler can use for analysis.
  PassValidation &= ValidateDopeVectorField(ElementSizeAddr,
                                            true /* ComputeConstant */,
                                            true /* WriteAllowed */,
                                            false /* ReadAllowed */);
  PassValidation &= ValidateDopeVectorField(CodimAddr,
                                            true /* ComputeConstant */,
                                            true /* WriteAllowed */,
                                            false /* ReadAllowed */);

  // NOTE: The FE can generate a load to the flags field, then do some
  // operations and followed by a store to the same field. In summary,
  // there will be a read before a write for this field. We are not
  // going to compute any constant information related to the flags field
  // until this issue is resolved. On the other hand, the field should be
  // only used by load and/or store instructions. Any other use should
  // invalidate the information for the current dope vector.
  PassValidation &= ValidateDopeVectorField(FlagsAddr,
                                            false /* ComputeConstant */,
                                            true /* WriteAllowed */,
                                            true /* ReadAllowed */);

  PassValidation &= ValidateDopeVectorField(DimensionsAddr,
                                            true /* ComputeConstant */,
                                            true /* WriteAllowed */,
                                            false /* ReadAllowed */);

  // This is the actual information of the dope vector. The extent,
  // stride and lower bounds fields can be read and written. If the
  // array pointer is not read, then none of these fields should be
  // read.
  // NOTE: There is a chance to extend this analysis for a possible
  // dead global dope vector elimination (CMPLRLLVM-28117).
  bool ReadAllowed = PtrAddr.getIsRead();
  for (uint64_t I = 0; I < Rank; I++) {
    PassValidation &= ValidateDopeVectorField(ExtentAddr[I],
                                              true /* ComputeConstant */,
                                              true /* WriteAllowed */,
                                              ReadAllowed);
    PassValidation &= ValidateDopeVectorField(StrideAddr[I],
                                              true /* ComputeConstant */,
                                              true /* WriteAllowed */,
                                              ReadAllowed);
    PassValidation &= ValidateDopeVectorField(LowerBoundAddr[I],
                                              true /* ComputeConstant */,
                                              true /* WriteAllowed */,
                                              ReadAllowed);
  }

  if (PassValidation)
    AnalysisRes = DopeVectorInfo::AnalysisResult::AR_Pass;
  else if (AnalysisRes == DopeVectorInfo::AnalysisResult::AR_Top)
    AnalysisRes = DopeVectorInfo::AnalysisResult::AR_Invalid;
}

// Return true if all pointers that access the dope vector fields were
// collected correctly by tracing the users of V, else return false. V
// represents a pointer to a nested dope vector (could come from a BitCast,
// GEP or an Argument). If AllowCheckForAllocSite is true then allow to
// trace the BitCast instructions as allocation sites, else any BitCast
// found is treated as an ilegal access and the function will return false.
static bool collectNestedDopeVectorFieldAddress(NestedDopeVectorInfo *NestedDV,
    Value *V, std::function<const TargetLibraryInfo &(Function &F)> &GetTLI,
    SetVector<Value *> &ValueChecked, bool AllowCheckForAllocSite) {

  // Return true if the users of the input GEP are subscript instructions.
  // The subscript instructions represents the access to the per dimension
  // array entry. This function is only used to collect data for the
  // extent, stride and lower bound.
  auto CollectAccessForExtentStrideAndLB = [NestedDV](GEPOperator *GEP,
      DopeVectorFieldType DVFieldType) -> bool {

    assert(GEP && "Trying to collect data for extent, stride or lower "
                 "bound without providing the access");
    assert(DVFieldType > DopeVectorFieldType::DV_PerDimensionArray &&
           DVFieldType < DopeVectorFieldType::DV_Invalid &&
           "Trying to collect data from extent, stride or lower bound "
           "without providing the proper field type");

    // All users should be array subscripts
    for (auto *ArrayAccess : GEP->users()) {
      auto *SI = dyn_cast<SubscriptInst>(ArrayAccess);
      if (!SI)
        return false;

      if (!isa<ConstantInt>(SI->getStride()) ||
          !isa<ConstantInt>(SI->getLowerBound()) ||
          !isa<ConstantInt>(SI->getIndex()))
        return false;

      // The index should be less than the rank
      ConstantInt *SIIndex = cast<ConstantInt>(SI->getIndex());
      unsigned long SubScriptIndex = SIIndex->getZExtValue();
      if (SubScriptIndex >= NestedDV->getRank())
        return false;

      auto *DVField = NestedDV->getDopeVectorField(DVFieldType,
                                                   SubScriptIndex);
      if (DVField->getIsBottom())
        return false;

      DVField->addFieldAddr(SI);
    }
    return true;
  };

  // If the input GEP is accessing the per dimension array field (field 6)
  // then we need to go through the users and collect the extent, stride
  // and lower boound accesses.
  auto CollectAccessForPerDimensionArray =
      [&CollectAccessForExtentStrideAndLB](GEPOperator *GEP) -> bool {

    for (auto *DimUser : GEP->users()) {

      // The users of the GEP will be another GEP accessing the
      // extent, stride or lower bound
      auto GEPUser = dyn_cast<GEPOperator>(DimUser);
      if (!GEPUser)
        return false;

      // This case handles when the GEP has 2 indices, more than 2
      // should be handled to a direct access of the extent, stride
      // or lower bound
      if (!GEPUser->hasAllConstantIndices() ||
        GEPUser->getNumIndices() != 2)
        return false;

      // Array index should be 0
      auto ArrayIdx = getConstGEPIndex(*GEPUser, 1);
      if (!ArrayIdx || *ArrayIdx != 0)
        return false;

      // Identify if GEPUser is accessing the extent, stride or
      // lower bound
      auto Idx = getConstGEPIndex(*GEPUser, 2);
      if (!Idx)
        return false;

      // We could add this as part of the switch-and-case below, but clang
      // doesn't like adding the default case when all cases in the enum
      // are covered.
      if (*Idx >
          (uint64_t)DopeVectorAnalyzer::DopeVectorRankFields::DVR_LowerBound)
        return false;

      DopeVectorFieldType DVFieldType;
      switch((DopeVectorAnalyzer::DopeVectorRankFields)*Idx) {
        case DopeVectorAnalyzer::DopeVectorRankFields::DVR_Extent:
          DVFieldType = DopeVectorFieldType::DV_ExtentBase;
          break;
        case DopeVectorAnalyzer::DopeVectorRankFields::DVR_Stride:
          DVFieldType = DopeVectorFieldType::DV_StrideBase;
          break;
        case DopeVectorAnalyzer::DopeVectorRankFields::DVR_LowerBound:
          DVFieldType = DopeVectorFieldType::DV_LowerBoundBase;
          break;
      }

      // Collect the uses of the extent, stride or lower bound
      if (!CollectAccessForExtentStrideAndLB(GEPUser, DVFieldType))
        return false;
    }
    return true;
  };

  // Return true if we can collect the dope vector field accesses
  // in the input call. This function will find which argument is
  // Val in the input Call, check that the argument attributes are
  // set correctly and recurse collectNestedDopeVectorFieldAddress
  // where the argument now represents a pointer to the nested dope
  // vector.
  //
  // NOTE: AllowCheckForAllocSite will be false in this case since
  // a function should not allocate a dope vector that is passed as
  // pointer. This conservative, we may want to relax this in the future.
  auto CollectAccessFromCall = [NestedDV](CallBase *Call, Value *Val,
      std::function<const TargetLibraryInfo &(Function &F)> &GetTLI,
      SetVector<Value *> &ValueChecked) -> bool {

    // Indirect calls or declarations aren't allowed
    // NOTE: In case of declarations, we may be able to mark the
    // libfuncs with some attributes that won't stop the data
    // collection.
    if (Call->isIndirectCall())
      return false;

    Function *F = Call->getCalledFunction();
    if (!F || F->isDeclaration())
      return false;

    // Find the actual argument
    uint64_t NumArgs = Call->getNumArgOperands();
    uint64_t ArgNo = 0;
    bool ArgFound = false;
    for (uint64_t I = 0; I < NumArgs; I++) {
      if (Call->getArgOperand(I) == Val) {

        if (ArgFound)
          return false;

        ArgNo = I;
        ArgFound = true;
      }
    }

    if (!ArgFound)
      return false;

    // Collect the formal argument
    auto *Arg = F->getArg(ArgNo);

    // Should be marked as "ptrnoalias", "assume_shape", "readonly" and
    // "noalias"
    if (!Arg->hasAttribute("ptrnoalias") ||
        !Arg->hasAttribute("assumed_shape") ||
        !Arg->onlyReadsMemory() ||
        !Arg->hasNoAliasAttr())
      return false;

    // Recurse, arg represents now the pointer to the nested dope vector
    if (!collectNestedDopeVectorFieldAddress(NestedDV, Arg, GetTLI,
        ValueChecked, false))
      return false;

    return true;
  };

  // Return true if the input GEP is used only by a BitCast for
  // data allocation
  auto GetCallForAllocation = [](GEPOperator *GEP,
      std::function<const TargetLibraryInfo &(Function &F)> &GetTLI)
      -> CallBase* {

    if (!GEP->hasOneUser())
      return nullptr;

    auto *BC = dyn_cast<BitCastInst>(GEP->user_back());
    if (!BC)
      return nullptr;

    // NOTE: Most likely we will need to adjust this BitCast when opaque
    // pointers are available.
    return bitCastUsedForAllocation(BC, GetTLI);
  };

  // Return true if the input GEP is accessing a dope vector field and store
  // it. Special cases:
  //
  //   * DV_ArrayPtr: GEP could be used for allocate the array
  //
  //   * DV_PerDimensionArray: We need to trace if the GEP is used to access
  //         the extent, stride or lower bound
  //
  //   * DV_ExtentBase, DV_StrideBase or DV_LowerBoundBase: The GEP directly
  //         accesses the extent, stride or lower bound, collect the accesses
  auto CollectAccessFromGEP = [NestedDV, AllowCheckForAllocSite,
      &CollectAccessForPerDimensionArray, &CollectAccessForExtentStrideAndLB,
      &GetCallForAllocation] (GEPOperator *GEP, uint64_t DopeVectorIndex,
      std::function<const TargetLibraryInfo &(Function &F)> &GetTLI) -> bool {

    DopeVectorFieldType DVFieldType =
        DopeVectorAnalyzer::identifyDopeVectorField(*GEP, DopeVectorIndex);

    if (DVFieldType >= DopeVectorFieldType::DV_Invalid)
      return false;

    if (DVFieldType == DopeVectorFieldType::DV_Reserved)
      return true;

    // There is a chance that the field address 0 is used for allocating
    // the array
    if (DVFieldType == DopeVectorFieldType::DV_ArrayPtr) {
      if (auto *Call = GetCallForAllocation(GEP, GetTLI)) {
        if (!AllowCheckForAllocSite) {
          NestedDV->setAllocSite(nullptr);
          return false;
        }

        if (NestedDV->isAllocSiteSet()) {
          NestedDV->setAllocSite(nullptr);
          return false;
        }

        NestedDV->setAllocSite(Call);
        return true;
      }
    }

    // The GEP is used to access the array pointer, element size,
    // codimension, flags
    if (DVFieldType < DopeVectorFieldType::DV_PerDimensionArray) {
      auto *DVField = NestedDV->getDopeVectorField(DVFieldType);
      if (DVField->getIsBottom())
        return false;
     DVField->addFieldAddr(cast<Value>(GEP));
    } else if (DVFieldType == DopeVectorFieldType::DV_PerDimensionArray) {
      // Check the accesses through the per dimension array
      if(!CollectAccessForPerDimensionArray(GEP))
        return false;
    } else {
      // If we reach this point then it means that it is a direct access to the
      // extent, stride and lower bound.
      if (!CollectAccessForExtentStrideAndLB(GEP, DVFieldType))
        return false;
    }
    return true;
  };

  // Return true if the input GEP is used for a accessing the dope vector
  // fields through a structure that contains a dope vector as field. For
  // example:
  //
  //    %92 = getelementptr inbounds %"ARR_MOD$.btT_TESTTYPE",
  //          %"ARR_MOD$.btT_TESTTYPE"* %78, i64 0, i32 1, i32 6, i64 0, i32 1
  //
  // In the example above, assume that the type %"ARR_MOD$.btT_TESTTYPE" is a
  // structure where field 1 is a dope vector. The GEP %92 basically is
  // accessing the stride in the dope vector that is field 1 of
  // %"ARR_MOD$.btT_TESTTYPE".
  auto IsGEPUsedForAccessingField = [NestedDV](GEPOperator *GEP) {
    if (GEP->getNumIndices() < 3)
      return false;

    if (!GEP->hasAllConstantIndices())
      return false;

    auto ArrayIdx = getConstGEPIndex(*GEP, 1);
    if (!ArrayIdx || *ArrayIdx != 0)
      return false;

    // NOTE: This check is conservative, for now it accepts one level of nested
    // dope vector. Therefore index 2 in the GEP represents the access to the
    // dope vector.
    auto NestedDVIndx = getConstGEPIndex(*GEP, 2);
    if (!NestedDVIndx || *NestedDVIndx != NestedDV->getFieldNum())
      return false;

    // Find if the rest of the indices are accessing a dope vector field
    DopeVectorFieldType DVFieldType =
        DopeVectorAnalyzer::identifyDopeVectorField(*GEP, 2);

    return DVFieldType < DopeVectorFieldType::DV_Invalid;
  };

  if (!V)
    return false;

  if (!ValueChecked.insert(V))
    return true;

  // If the input Value is a GEP with more than 2 indices then there is a
  // chance that the GEP represents an access to a structure with
  // a field that is a dope vector (nested dope vector) and a field in
  // the dope vector.
  auto *GEPOp = dyn_cast<GEPOperator>(V);
  if (GEPOp && IsGEPUsedForAccessingField(GEPOp) &&
      CollectAccessFromGEP(GEPOp, 2, GetTLI))
    return true;

  // If we reach this point then it means that V represents a pointer to
  // a dope vector. Go through the users of V to identify how the fields of
  // the dope vector are used.
  for (auto *U : V->users()) {
    // GEP should be accessing dope vector fields
    if(auto *GEP = dyn_cast<GEPOperator>(U)) {
      if (!CollectAccessFromGEP(GEP, 0, GetTLI))
        return false;
    } else if(auto *BC = dyn_cast<BitCastOperator>(U)) {
      // BitCast should be only used for allocating data
      if (NestedDV->isAllocSiteSet() || !AllowCheckForAllocSite) {
        NestedDV->setAllocSite(nullptr);
        return false;
      }

      CallBase *Call = bitCastUsedForAllocation(BC, GetTLI);
      if (!Call) {
        NestedDV->setAllocSite(nullptr);
        return false;
      }

      NestedDV->setAllocSite(Call);
    } else if (CallBase *Call = dyn_cast<CallBase>(U)) {
      // Calls should only load data
      if (!CollectAccessFromCall(Call, V, GetTLI, ValueChecked))
        return false;
    } else {
      return false;
    }
  }

  return true;
}

// Analyze that all the dope vector fields are used to load and store data
void NestedDopeVectorInfo::analyzeNestedDopeVector() {

  PtrAddr.analyzeUses();
  ElementSizeAddr.analyzeUses();
  CodimAddr.analyzeUses();
  FlagsAddr.analyzeUses();
  DimensionsAddr.analyzeUses();

  for (unsigned long I = 0; I < Rank; I++) {
    ExtentAddr[I].analyzeUses();
    StrideAddr[I].analyzeUses();
    LowerBoundAddr[I].analyzeUses();
  }

  // Load and stores collected, now validate the dope vector
  validateDopeVector();
}

// Return true if the input BitCast operator is used for allocation
bool GlobalDopeVector::collectAndAnalyzeAllocSite(BitCastOperator *BC) {
  if (!BC || !GlobalDVInfo || GlobalDVInfo->isAllocSiteSet())
    return false;

  CallBase *Call = bitCastUsedForAllocation(BC, GetTLI);
  if (!Call)
    return false;

  GlobalDVInfo->setAllocSite(Call);
  return true;
}

// Return true if the input GEPOperator is accessing a dope vector field,
// collect the uses and analyze that there is no ilegal use for it. Else
// return false.
bool
GlobalDopeVector::collectAndAnalyzeGlobalDopeVectorField(GEPOperator *GEP) {

  if (!GEP || !GlobalDVInfo)
    return false;

  // Check which dope vector field is being accessed and
  // collect the information related to the load and stores
  DopeVectorFieldType DVFieldType =
      DopeVectorAnalyzer::identifyDopeVectorField(*GEP);

  if (DVFieldType >= DopeVectorFieldType::DV_Invalid)
    return false;

  if (DVFieldType == DopeVectorFieldType::DV_Reserved)
    return true;

  // If the field type is DV_PerDimensionArray or lower then check
  // the load and stores (analyzeUses).
  if (DVFieldType < DopeVectorFieldType::DV_PerDimensionArray) {
    DopeVectorFieldUse *DVField =
        GlobalDVInfo->getDopeVectorField(DVFieldType);
    if (!DVField || DVField->getIsBottom())
      return false;

    DVField->addFieldAddr(GEP);

    DVField->analyzeUses();
    if (DVField->getIsBottom())
      return false;
  } else if (DVFieldType > DopeVectorFieldType::DV_PerDimensionArray) {
    // If the GEPOperator directly accesses the extent, stride or lower bound
    // fields, then it means that the array entry should be accessed by a
    // subscript. We need to collect the subscript and check where the
    // data is used for load and store. Each subscript accesses a different
    // entry in the per dimension-array. To simplify the analysis, we are
    // going to collect and organize each subscript in the corresponding
    // array entry (collectSubscriptInformation) and then we are going to
    // analyze them (analyzeSubscriptUses).
    for (auto *U : GEP->users()) {
      // All entries in the subscript should be constant
      if (auto *SI = dyn_cast<SubscriptInst>(U)) {
        // Collect the array entry
        ConstantInt *Index = dyn_cast<ConstantInt>(SI->getIndex());
        if (!Index)
          return false;
        uint64_t ArrayEntry = Index->getZExtValue();
        if (ArrayEntry >= GlobalDVInfo->getRank())
          return false;

        // Find the DopeVectorField use corresponding to the array entry
        DopeVectorFieldUse *DVField =
            GlobalDVInfo->getDopeVectorField(DVFieldType, ArrayEntry);
        if (!DVField || DVField->getIsBottom())
          return false;

        // Store the current subscirpt
        DVField->addFieldAddr(GEP);
        DVField->collectSubscriptInformation(SI, DVFieldType,
                                             GlobalDVInfo->getRank());
        if (DVField->getIsBottom())
          return false;
      } else {
        return false;
      }
    }

    // Check that all subscripts used for collecting the extent, stride and
    // lower bound are used for loading and storing data.
    for (uint64_t I = 0, E = GlobalDVInfo->getRank(); I < E; I++) {
      auto *ExtentField =
        GlobalDVInfo->getDopeVectorField(DopeVectorFieldType::DV_ExtentBase,
                                         I);
      ExtentField->analyzeSubscriptsUses();
      if (ExtentField->getIsBottom())
        return false;

      auto *StrideField =
        GlobalDVInfo->getDopeVectorField(DopeVectorFieldType::DV_StrideBase,
                                         I);
      StrideField->analyzeSubscriptsUses();
      if (StrideField->getIsBottom())
        return false;

      auto *LowerBoundField = GlobalDVInfo->getDopeVectorField(
          DopeVectorFieldType::DV_LowerBoundBase, I);
      LowerBoundField->analyzeSubscriptsUses();
      if (LowerBoundField->getIsBottom())
        return false;
    }
  } else {
    // NOTE: If the DVFieldType is DV_PerDimensionArray it means that the
    // extent, stride and lower bound are being accessed. We may need
    // to extend the data colection to support this operation.
    return false;
  }

  return true;
}

// Collect the nested dope vectors and store the access to them. Return false
// if there is any illegal access, else return true.
bool GlobalDopeVector::collectNestedDopeVectorFromSubscript(
    SubscriptInst *SI, const DataLayout &DL) {

  // Given a Value that is a GetElementPointerInst or a BitCastInst,
  // then find if it is accessing a structure, if so collect the field
  // number and type.
  //
  // This may need some extra work for with opaque pointers. A dope
  // vector with an opaque pointer might look as follows:
  //
  //   %opaque_dv = { p0*, i64, i64, i64, i64, i64, 1 x [i64, i64, i64] }
  //
  // This means that we don't know the pointer type for field 0.
  auto GetNestedDVTypeFromValue = [](Value *Val) ->
      std::pair<Type *, uint64_t> {

    std::pair<Type *, uint64_t> NullPair = std::make_pair(nullptr, 0);

    uint64_t RetIdx = 0;
    Type *RetType = nullptr;

    // If it is a BitCast, the destination is a pointer to a structure.
    // This is accessing field 0.
    if (auto BC = dyn_cast<BitCastInst>(Val)) {
      Type *DestTy = BC->getDestTy();
      Type *SrcTy = BC->getSrcTy();
      if (!SrcTy->isPointerTy() || !DestTy->isPointerTy())
        return NullPair;

      // NOTE: This may need to be updated for opaque pointers.
      SrcTy = SrcTy->getPointerElementType();
      if (!SrcTy->isStructTy() || SrcTy->getStructNumElements() == 0)
        return NullPair;

      Type *ZeroType = SrcTy->getStructElementType(0);
      DestTy = DestTy->getPointerElementType();
      if (ZeroType == DestTy)
        RetType = DestTy;
    } else if (auto GEP = dyn_cast<GetElementPtrInst>(Val)) {
      // If it is a GEP, then all indices are constants
      if (!GEP->hasAllConstantIndices())
        return NullPair;

      Type *SrcType = GEP->getSourceElementType();
      auto ArrayIdx = getConstGEPIndex(*(cast<GEPOperator>(GEP)), 1);
      if (!ArrayIdx || *ArrayIdx != 0)
        return NullPair;

      auto Idx = getConstGEPIndex(*(cast<GEPOperator>(GEP)), 2);
      if (!Idx)
        return NullPair;

      uint64_t FieldIdx = *Idx;

      if (FieldIdx >= SrcType->getStructNumElements())
        return NullPair;

      RetType = SrcType->getStructElementType(FieldIdx);
      RetIdx = FieldIdx;
    } else {
      llvm_unreachable("Trying to collect nested dope vector from an "
                       "invalid instruction");
    }

    return std::make_pair(RetType, RetIdx);
  };

  // Find the nested dope vector info, if not found then
  // create a new entry.
  auto FindOrMakeNestedDopeVector = [&, this](Type *DVType,
                                              uint64_t FieldNum) ->
                                              NestedDopeVectorInfo* {

    auto *NestedDV = getNestedDopeVector(FieldNum);
    if (!NestedDV) {
      NestedDV = new NestedDopeVectorInfo(Glob, DVType, FieldNum,
          true /* AllowMultipleFieldAddresses*/);
      NestedDopeVectors.insert(NestedDV);
    } else {
      assert(NestedDV->getLLVMStructType() == DVType &&
             "Nested dope vector type mismatch with the type found by the "
             "subscript analysis");
    }

    return NestedDV;
  };

  // Return a ConstantInt value for the stride of 'SI', if one can be
  // easily found.
  //
  auto GetConstantStride = [this](SubscriptInst *SI) -> ConstantInt * {
    Value *VS = SI->getStride();
    // If the stride is a literal constant, return it.
    if (auto CI0 = dyn_cast<ConstantInt>(VS))
      return CI0;
    // Otherwise, look for a LoadInst, fed by a SubscriptInst, fed by a
    // GEPOperator.
    auto LI = dyn_cast<LoadInst>(VS);
    if (!LI)
      return nullptr;
    auto SIS = dyn_cast<SubscriptInst>(LI->getPointerOperand());
    if (!SIS)
      return nullptr;
    // Is this a GEPOPerator indexing the stride of a global dope vector?
    auto GEPO = dyn_cast<GEPOperator>(SIS->getPointerOperand());
    if (GEPO->getPointerOperand() != Glob)
      return nullptr;
    if (DopeVectorAnalyzer::identifyDopeVectorField(*GEPO) != DV_StrideBase)
      return nullptr;
    // Get the dimension for the stride in the dope vector.
    auto CI = dyn_cast<ConstantInt>(SIS->getIndex());
    if (!CI)
      return nullptr;
    // Look up the stride value in the DopeVectorInfo.
    uint64_t Dim = CI->getZExtValue();
    DopeVectorInfo *DVI = getGlobalDopeVectorInfo();
    assert(DVI && "Expecting dope vector info");
    DopeVectorFieldUse *SF = DVI->getDopeVectorField(
        DopeVectorFieldType::DV_StrideBase, Dim);
    return SF->getConstantValue();
  };

  // Return 'true' if 'U' is the user of a SubscriptInst that can be
  // properly collected. In that case, update 'AllocSiteFound'.
  //
  auto CanCollectNDVSubscriptUser = [&, this](User *U,
                                              const DataLayout &DL,
                                              bool &AllocSiteFound) -> bool {
    if (auto *Call = bitCastUsedForAllocation(U, GetTLI)) {
      // A BitCast can used for allocating the array for the
      // nested dope vector at field 0. It represents a field 0 access.
      if (AllocSiteFound)
        return false;

      // NOTE: This may not work with opaque pointers. We may want to revise
      // it in the future.
      BitCastInst *BC = cast<BitCastInst>(U);
      Type *SrcTy = BC->getSrcTy();
      if (!SrcTy->isPointerTy())
        return false;

      StructType *StrSource =
          dyn_cast<StructType>(SrcTy->getPointerElementType());

      if (!StrSource || StrSource->getNumElements() == 0)
        return false;

      Type *FieldZeroType = StrSource->getElementType(0);
      if (!isDopeVectorType(FieldZeroType, DL))
        return false;

      auto *NestedDVInfo = FindOrMakeNestedDopeVector(FieldZeroType, 0);
      assert(NestedDVInfo && "Nested dope vector 0 couldn't be found\n");

      NestedDVInfo->setAllocSite(Call);
      AllocSiteFound = true;
    } else if (isa<GetElementPtrInst>(U) || isa<BitCastInst>(U)) {
      auto NestedDVTypePair = GetNestedDVTypeFromValue(U);
      if (!NestedDVTypePair.first)
        return false;

      if (isDopeVectorType(NestedDVTypePair.first, DL)) {
        auto *NestedDVInfo =
            FindOrMakeNestedDopeVector(NestedDVTypePair.first,
                                       NestedDVTypePair.second);
        assert(NestedDVInfo && "Nested dope vector couldn't be found\n");
        SetVector<Value *> ValueChecked;

        // Nested dope vector found, now collect the fields access
        if (!collectNestedDopeVectorFieldAddress(NestedDVInfo, U, GetTLI,
                                                 ValueChecked, true))
          return false;
      } else {
        // For now, if this is not a nested dope vector, give up if we
        // see anything other than a user which is a LoadInst or StoreInst
        // referencing U as a pointer operand. We can extend this analysis
        // if it proves to be useful to do that.
        for (User *V : U->users()) {
          if (auto SI = dyn_cast<StoreInst>(V)) {
            if (SI->getPointerOperand() != U)
              return false;
          } else if (!isa<LoadInst>(V)) {
            return false;
          }
        }
      }
    } else {
      // Subscript used for something else
      return false;
    }
    return true;
  };

  // If 'U' is a user of 'V' and is passed as an actual argument of a
  // CallBase, which calls a Function 'F' that is not address-taken and has
  // IR, return the formal argument of 'F' corresponding to that actual
  // argument. Otherwise, return 'nullptr'.
  //
  auto IsIPOPropagatable = [&](const Value *V, const User *U) -> Argument * {
    if (isa<IntrinsicInst>(U))
      return nullptr;
    if (auto CB = dyn_cast<CallBase>(U)) {
      if (CB->isIndirectCall())
        return nullptr;
      Function *F = CB->getCalledFunction();
      if (!F || F->hasAddressTaken() || F->isDeclaration())
        return nullptr;
      if (auto ArgPos = getArgumentPosition(*CB, V))
        return F->getArg(*ArgPos);
    }
    return nullptr;
  };

  // Return 'true' if each use of 'A' can be propagated to a CallBase that
  // calls a Function with IR that is not address taken, or can be collected
  // inside its Function as usual. Update 'AllocSiteFound' if the alloc site
  // for the dope vector is found.
  //
  std::function<bool(Argument *, const DataLayout &, bool &)>
      PropagateArgument = [&](Argument *A, const DataLayout &DL,
                              bool &AllocSiteFound) -> bool {
    for (User *U : A->users()) {
      if (Argument *NewA = IsIPOPropagatable(A, U)) {
        if (!PropagateArgument(NewA, DL, AllocSiteFound))
          return false;
      } else if (!CanCollectNDVSubscriptUser(U, DL, AllocSiteFound)) {
        return false;
      }
    }
    return true;
  };

  if (!SI)
    return false;

  ConstantInt *GlobalElementSize = GlobalDVInfo->getDopeVectorField(
      DopeVectorFieldType::DV_ElementSize)->getConstantValue();

  bool AllocSiteFound = false;

  // Unknown element size for the global dope vector, we can't collect the
  // information for nested dope vectors. The element size of the global
  // dope vector is needed to make sure that the stride in the subscript
  // instruction is collecting the right number of bits when accessing
  // each entry in the global array.
  if (!GlobalElementSize || GetConstantStride(SI) != GlobalElementSize)
    return false;

  // Traverse through the users of the array entry and find the information
  // related to the nested dope vectors
  for (User *U : SI->users()) {
    if (Argument *A = IsIPOPropagatable(SI, U)) {
      if (!PropagateArgument(A, DL, AllocSiteFound))
        return false;
    } else if (!CanCollectNDVSubscriptUser(U, DL, AllocSiteFound)) {
      return false;
    }
  }
  return true;
}

// This function will check if there are nested dope vectors for the global
// dope vector, collects the information and analyze if there is any ilegal
// access that could invalidate the data.
void
GlobalDopeVector::collectAndAnalyzeNestedDopeVectors(const DataLayout &DL) {

  // NOTE: This needs to be updated for opaque pointers
  auto GetResultTypeFromSubscript = [](SubscriptInst *SI) {
    Type *ResType = SI->getType();
    if (ResType->isPointerTy())
      ResType = ResType->getPointerElementType();

    return ResType;
  };

  // First check if there is any nested dope vector. At this point we know that
  // the global is a dope vector, but double check.
  assert(Glob == GlobalDVInfo->getDVObject() &&
         "Dope vector object mismatch with global");

  // If the global dope vector info is not valid then there is no point for
  // collecting the nested dope vectors
  if (GlobalDVInfo->getAnalysisResult() ==
      DopeVectorInfo::AnalysisResult::AR_Invalid)
    return;

  auto *PtrAddressField = GlobalDVInfo->getDopeVectorField(
      DopeVectorFieldType::DV_ArrayPtr);

  // If the pointer address is Bottom or is written then we bail out. There is
  // an use of the pointer address that invalidates the nested dope vectors.
  if (PtrAddressField->getIsBottom() ||
      PtrAddressField->getIsWritten())
    return;

  Type *SubscriptResultType = nullptr;
  bool NestedDVDataValid = true;

  // Go through all the places where the global array is loaded
  for (auto *LI : PtrAddressField->loads()) {
    for (auto *U : LI->users()) {

      // The user of the global array will be subscript
      if (auto *SI = dyn_cast<SubscriptInst>(U)) {

        // Collect the type that the subscript is loading.
        // The type should be the same accross all subscripts.
        Type *CurrSIType = GetResultTypeFromSubscript(SI);
        if (CurrSIType->isPointerTy()) {
          NestedDVDataValid = false;
          break;
        } else if (!SubscriptResultType) {
          SubscriptResultType = CurrSIType;
        } else if (SubscriptResultType != CurrSIType) {
          NestedDVDataValid = false;
          break;
        }

        // If the subscript is not accessing a structure then there
        // won't be nested dope vectors, still we need to check that
        // all the subscripts are accessing the same type
        if (!SubscriptResultType->isStructTy())
          continue;

        // Collect the possible nested dope vectors that the subscript
        // could be accessing
        if (!collectNestedDopeVectorFromSubscript(SI, DL)) {
          NestedDVDataValid = false;
          break;
        }
      } else {
        NestedDVDataValid = false;
        break;
      }
    }

    if (!NestedDVDataValid)
      break;
  }

  // Analyze the nested dope vectors if they were collected correctly
  NestedDVDataCollected = NestedDVDataValid;
  if (!NestedDVDataValid || NestedDopeVectors.empty())
    return;

  for (auto *NestedDV : NestedDopeVectors)
    NestedDV->analyzeNestedDopeVector();
}

// Validate that the data was collected correctly for the global dope vector
void GlobalDopeVector::validateGlobalDopeVector() {

  // If the global dope vector info is not valid then the analysis for
  // the fields of the global dope vector failed
  if (GlobalDVInfo->getAnalysisResult() !=
      DopeVectorInfo::AnalysisResult::AR_Pass) {
    AnalysisRes = GlobalDopeVector::AnalysisResult::AR_GlobalDVAnalysisFailed;
    return;
  }

  // If NestedDVDataCollected is false then it means that something happened
  // while collecting the nested dope vectors
  if (!NestedDVDataCollected) {
    AnalysisRes = GlobalDopeVector::AnalysisResult::AR_IncompleteNestedDVData;
    return;
  }

  // Check that all nested dope vectors were validated correctly
  for (auto *NestedDV : NestedDopeVectors) {
    if (NestedDV->getAnalysisResult() !=
        DopeVectorInfo::AnalysisResult::AR_Pass) {
      AnalysisRes = GlobalDopeVector::AnalysisResult::AR_BadNestedDV;
      return;
    }
  }

  // Analysis pass
  AnalysisRes = GlobalDopeVector::AnalysisResult::AR_Pass;
}

void GlobalDopeVector::collectAndValidate(const DataLayout &DL) {
  for (auto *U : Glob->users()) {
    if (auto *BC = dyn_cast<BitCastOperator>(U)) {
      // The BitCast should only be used for data allocation and
      // should happen only once
      if (!collectAndAnalyzeAllocSite(BC)) {
        getGlobalDopeVectorInfo()->invalidateDopeVectorInfo();
        break;
      }
    } else if (auto *GEP = dyn_cast<GEPOperator>(U)) {

      // The fields of the global dope vector are accessed through
      // a GEPOperator
      if (!collectAndAnalyzeGlobalDopeVectorField(GEP)) {
        getGlobalDopeVectorInfo()->invalidateDopeVectorInfo();
        break;
      }
    } else {
      // Any other use is invalid
      getGlobalDopeVectorInfo()->invalidateDopeVectorInfo();
      break;
    }
  }

  // Make sure that none of the fields the in the dope vector are
  // set to bottom
  getGlobalDopeVectorInfo()->validateDopeVector();

  // Collect any information related to the nested dope vectors
  collectAndAnalyzeNestedDopeVectors(DL);

  // Validate that the data was collected correctly
  validateGlobalDopeVector();
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void DopeVectorInfo::print(uint64_t Indent) {

  auto SimpleFieldUsePrint = [Indent](DopeVectorFieldUse &Field,
                                      uint64_t ArrayEntry,
                                      DopeVectorFieldType FieldType) -> void {

    std::string IndentString(Indent, ' ');
    StringRef IndentRef(IndentString);
    switch(FieldType) {
      case DopeVectorFieldType::DV_ArrayPtr:
        dbgs() << IndentString << "  [0] Array Pointer: ";
        break;
      case DopeVectorFieldType::DV_ElementSize:
        dbgs() << IndentString << "  [1] Element size: ";
        break;
      case DopeVectorFieldType::DV_Codim:
        dbgs() << IndentString << "  [2] Co-Dimension: ";
        break;
      case DopeVectorFieldType::DV_Flags:
        dbgs() << IndentString << "  [3] Flags: ";
        break;
      case DopeVectorFieldType::DV_Dimensions:
        dbgs() << IndentString << "  [4] Dimensions: ";
        break;
      case DopeVectorFieldType::DV_PerDimensionArray:
        dbgs() << IndentString << "  [6] Per-dimension array: ";
        break;
      case DopeVectorFieldType::DV_ExtentBase:
        dbgs() << IndentString << "  [6][" << ArrayEntry <<"] Extent: ";
        break;
      case DopeVectorFieldType::DV_StrideBase:
        dbgs() << IndentString << "  [6][" << ArrayEntry <<"] Stride: ";
        break;
      case DopeVectorFieldType::DV_LowerBoundBase:
        dbgs() << IndentString << "  [6][" << ArrayEntry <<"] Lower Bound: ";
        break;
      default:
        llvm_unreachable("Trying to access a field in the dope vector "
                         "that should not be accessed");
    }

    std::string Result = "";

    if (Field.getIsRead())
      Result = "Read";

    if (Field.getIsWritten()) {
      if (!Result.empty())
        Result += " | ";
      Result += "Written";
    }

    if (Field.getIsBottom()) {
      if (!Result.empty())
        Result += " | ";
      Result += "Bottom";
    }

    dbgs() << Result;

    if (Field.getConstantValue()) {
      if (!Result.empty())
        dbgs() << " | ";
      dbgs() << "Constant = " << *(Field.getConstantValue());
    }

    dbgs() << "\n";

  };

  std::string IndentString(Indent, ' ');
  StringRef IndentRef(IndentString);
  dbgs() << IndentRef << "Dope vector analysis result: ";

  // The following switch statement doesn't include a default case because
  // clang throws an error if all the enum cases are covered explicitly and
  // a default case is added.
  assert(AnalysisRes <= DopeVectorInfo::AnalysisResult::AR_Pass &&
         "Invalid dope vector analysis result");

  switch(AnalysisRes) {
    case DopeVectorInfo::AnalysisResult::AR_Top:
      dbgs() << "Incomplete data collection";
      break;
    case DopeVectorInfo::AnalysisResult::AR_Invalid:
      dbgs() << "Invalid data collection";
      break;
    case DopeVectorInfo::AnalysisResult::AR_ReadIllegality:
      dbgs() << "Invalid dope vector field read";
      break;
    case DopeVectorInfo::AnalysisResult::AR_WriteIllegality:
      dbgs() << "Invalid dope vector field write";
      break;
    case DopeVectorInfo::AnalysisResult::AR_NoAllocSite:
      dbgs() << "Alloc site wasn't found";
      break;
    case DopeVectorInfo::AnalysisResult::AR_AllocStoreIllegality:
      dbgs() << "Store won't execute with alloc site";
      break;
    case DopeVectorInfo::AnalysisResult::AR_Pass:
      dbgs() << "Pass";
      break;
  }
  dbgs() << "\n";

  dbgs() << IndentRef << "Constant propagation status:"
         << (ConstantsPropagated ? " " : " NOT ") << "performed\n";

  SimpleFieldUsePrint(PtrAddr, 0, DopeVectorFieldType::DV_ArrayPtr);
  SimpleFieldUsePrint(ElementSizeAddr, 0, DopeVectorFieldType::DV_ElementSize);
  SimpleFieldUsePrint(CodimAddr, 0, DopeVectorFieldType::DV_Codim);
  SimpleFieldUsePrint(FlagsAddr, 0, DopeVectorFieldType::DV_Flags);
  SimpleFieldUsePrint(DimensionsAddr, 0, DopeVectorFieldType::DV_Dimensions);
  for (uint64_t I = 0; I < Rank; I++) {
    SimpleFieldUsePrint(ExtentAddr[I], I, DopeVectorFieldType::DV_ExtentBase);
    SimpleFieldUsePrint(StrideAddr[I], I, DopeVectorFieldType::DV_StrideBase);
    SimpleFieldUsePrint(LowerBoundAddr[I], I,
        DopeVectorFieldType::DV_LowerBoundBase);
  }
}

void GlobalDopeVector::print() {
  dbgs() << "Global variable: " << Glob->getName() << "\n";
  dbgs() << "  LLVM Type: "
         << GlobalDVInfo->getLLVMStructType()->getName() << "\n";
  dbgs() << "  Global dope vector result: ";

  // The following switch statement doesn't include a default case because
  // clang throws an error if all the enum cases are covered explicitly and
  // a default case is added.
  assert(AnalysisRes <= GlobalDopeVector::AnalysisResult::AR_Pass &&
         "Invalid global dope vector analysis result");

  switch(AnalysisRes) {
    case GlobalDopeVector::AnalysisResult::AR_Top:
      dbgs() << "Analysis not performed";
      break;
    case GlobalDopeVector::AnalysisResult::AR_GlobalDVAnalysisFailed:
      dbgs() << "Failed to collect global dope vector info";
      break;
    case GlobalDopeVector::AnalysisResult::AR_IncompleteNestedDVData:
      dbgs() << "Failed to collect nested dope vectors' data";
      break;
    case GlobalDopeVector::AnalysisResult::AR_BadNestedDV:
      dbgs() << "At least one nested dope vector didn't pass analysis";
      break;
    case GlobalDopeVector::AnalysisResult::AR_Pass:
      dbgs() << "Pass";
      break;
  }

  dbgs() << "\n";
  GlobalDVInfo->print(2);
  if (NestedDopeVectors.size() > 0) {
    dbgs() << "  Nested dope vectors: " << NestedDopeVectors.size() << "\n";
    for (auto NestedDV : NestedDopeVectors) {
      dbgs() << "    Field[" << NestedDV->getFieldNum() << "]: "
             << NestedDV->getLLVMStructType()->getName() << "\n";
      NestedDV->print(6);
    }
  }
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

} // end namespace dvanalysis

} // end namespace llvm

