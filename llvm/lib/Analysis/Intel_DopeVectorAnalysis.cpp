//===------- Intel_DopeVectorAnalysis.cpp ----------------------- -*------===//
//
// Copyright (C) 2019-2019 Intel Corporation. All rights reserved.
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

// Helper routine to check whether a variable type is a type for an
// uplevel variable.
static bool isUplevelVarType(Type *Ty) {
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

// Helper function to check whether \p V is a GEP that corresponds to a field
// within an uplevel type.
static bool isFieldInUplevelTypeVar(Value *V) {
  auto *GEP = dyn_cast<GetElementPtrInst>(V);
  if (!GEP)
    return false;
  return isUplevelVarType(
      GEP->getPointerOperand()->getType()->getPointerElementType());
}

Optional<uint64_t> getConstGEPIndex(const GetElementPtrInst &GEP,
                                    unsigned int OpNum) {
  auto FieldIndex = dyn_cast<ConstantInt>(GEP.getOperand(OpNum));
  if (FieldIndex)
    return Optional<uint64_t>(FieldIndex->getLimitedValue());
  return None;
}

Optional<unsigned int> getArgumentPosition(const CallInst &CI,
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

void DopeVectorFieldUse::analyzeUses() {
  if (IsBottom)
    return;

  if (!FieldAddr)
    return;

  for (auto *U : FieldAddr->users()) {
    if (auto *SI = dyn_cast<StoreInst>(U)) {
      // Make sure the store is to the field address, and that it's not the
      // field address being stored somewhere.
      if (SI->getValueOperand() != FieldAddr) {
        Stores.insert(SI);
        IsWritten = true;
      } else {
        IsBottom = true;
        break;
      }
    } else if (auto *LI = dyn_cast<LoadInst>(U)) {
      Loads.insert(LI);
      IsRead = true;
    } else {
      IsBottom = true;
      break;
    }
  }
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void DopeVectorFieldUse::dump() const { print(dbgs()); }
void DopeVectorFieldUse::print(raw_ostream &OS, const Twine &Header) const {
  OS << Header;
  print(OS);
}

void DopeVectorFieldUse::print(raw_ostream &OS) const {
  if (!FieldAddr) {
    OS << "  Not set\n";
    return;
  }
  OS << *FieldAddr << "\n";
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
      DopeVectorAnalyzer::DopeVectorFieldType DVFieldType =
          identifyDopeVectorField(*GEP);
      switch (DVFieldType) {
      default:
        setInvalid();
        return;

      case DV_ArrayPtr:
        PtrAddr.setFieldAddr(GEP);
        break;
      case DV_ElementSize:
        ElementSizeAddr.setFieldAddr(GEP);
        break;
      case DV_Codim:
        CodimAddr.setFieldAddr(GEP);
        break;
      case DV_Flags:
        FlagsAddr.setFieldAddr(GEP);
        break;
      case DV_Dimensions:
        DimensionsAddr.setFieldAddr(GEP);
        break;
      case DV_Reserved:
        // Ignore uses of reserved
        break;

        // The following fields require additional forward looking analysis to
        // get to the actual address-of objects.
      case DV_PerDimensionArray:
        if (PerDimensionBase) {
          setInvalid();
          return;
        }
        PerDimensionBase = GEP;
        break;
      case DV_LowerBoundBase:
        if (LowerBoundBase) {
          setInvalid();
          return;
        }
        LowerBoundBase = GEP;
        break;
      case DV_ExtentBase:
        if (ExtentBase) {
          setInvalid();
          return;
        }
        ExtentBase = GEP;
        break;

      case DV_StrideBase:
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
          auto PtrGEP = cast<GetElementPtrInst>(PtrOp);
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
          ExtentField.setFieldAddr(Ptr);
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
          StrideField.setFieldAddr(Ptr);
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
          LBField.setFieldAddr(Ptr);
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

DopeVectorAnalyzer::DopeVectorFieldType
DopeVectorAnalyzer::identifyDopeVectorField(const GetElementPtrInst &GEP) {
  assert(GEP.getSourceElementType()->isStructTy() && "Expected struct type");

  // Array index should always be zero.
  auto ArrayIdx = getConstGEPIndex(GEP, 1);
  if (!ArrayIdx || *ArrayIdx != 0)
    return DV_Invalid;

  unsigned NumIndices = GEP.getNumIndices();
  if (NumIndices < 2 || NumIndices > 4)
    return DV_Invalid;

  // The address for the first 6 fields of the dope vector are accessed
  // directly with a GEP of the form:
  //     %field4 = getelementptr
  //               { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] },
  //               { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }*
  //               %"var$08", i64 0, i32 4
  if (NumIndices == 2) {
    auto FieldIdx = getConstGEPIndex(GEP, 2);
    assert(FieldIdx &&
           "Field index should always be constant for struct type");
    assert(FieldIdx <= static_cast<uint64_t>(DV_PerDimensionArray) &&
           "expected dope vector to have a maximum of 7 fields");
    return static_cast<DopeVectorAnalyzer::DopeVectorFieldType>(*FieldIdx);
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
  if (NumIndices == 3) {
    auto FieldIdx = getConstGEPIndex(GEP, 2);
    if (FieldIdx != static_cast<uint64_t>(DV_PerDimensionArray))
      return DV_Invalid;

    // We only expect the GEP to use 0 for last index which corresponds to the
    // per-dimension array base, and then be followed by another GEP to get
    // the specific structure element.
    auto SubIdx = getConstGEPIndex(GEP, 3);
    assert(SubIdx && "Field index should always be constant for struct type");
    if (*SubIdx != 0)
      return DV_Invalid;
    return DV_PerDimensionArray;
  }

  assert(NumIndices == 4 && "Only expected case 4 to be left");

  // The second form of access directly gets the address of the Lower Bound,
  // Stride or Extent field of the first array element.
  auto SubIdx = getConstGEPIndex(GEP, 4);
  assert(SubIdx && "Field index should always be constant for struct type");
  switch (*SubIdx) {
  default:
    return DV_Invalid;
  case 0:
    return DV_ExtentBase;
  case 1:
    return DV_StrideBase;
  case 2:
    return DV_LowerBoundBase;
  }
  return DV_Invalid;
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

      auto ArIdx = getConstGEPIndex(*GEPU, 1);
      if (!ArIdx || *ArIdx != 0)
        return InvalidResult;

      // Check that there is only one instance of field being searched for.
      auto FieldIdx = getConstGEPIndex(*GEPU, 2);
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

} // end namespace dvanalysis

} // end namespace llvm

