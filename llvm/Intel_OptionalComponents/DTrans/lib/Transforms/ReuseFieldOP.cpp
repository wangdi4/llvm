//===--------------- ReuseFieldOP.cpp - DTransReuseFieldOPPass ------------===//
//
// Copyright (C) 2022-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the DTrans reuse field optimization pass.
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Transforms/ReuseFieldOP.h"

#include "Intel_DTrans/Analysis/DTrans.h"
#include "Intel_DTrans/Analysis/DTransAnnotator.h"
#include "Intel_DTrans/Analysis/DTransSafetyAnalyzer.h"
#include "Intel_DTrans/DTransCommon.h"
#include "Intel_DTrans/Transforms/DTransOptUtils.h"

#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/IR/Operator.h"
#include "llvm/InitializePasses.h"

#include <map>

using namespace llvm;
using namespace dtransOP;

#define DEBUG_TYPE "dtrans-reusefieldop"

namespace {
class ReuseFieldOPImpl {
public:
  typedef std::pair<Type *, size_t> StructFieldTy;
  typedef std::pair<Value *, StoreInst *> VITy;
  typedef SmallDenseMap<Value *, VITy> BP2VIMapTy;
  typedef SmallDenseMap<StructFieldTy, BP2VIMapTy> SF2BP2VIMapTy;
  typedef SmallDenseMap<
      Type *,
      SmallDenseMap<size_t, SmallDenseMap<size_t, SmallVector<StoreInst *, 2>>>>
      MissingFieldsListTy;
  typedef std::map<StructFieldTy, SmallVector<size_t>> StructField2FieldsMapTy;
  typedef SmallDenseMap<dtrans::StructInfo *, SmallSet<size_t, 4>>
      CandidateStructFieldsTy;

  ReuseFieldOPImpl(
      DTransSafetyInfo &DTInfo,
      std::function<const TargetLibraryInfo &(const Function &)> GetTLI)
      : DTInfo(&DTInfo), GetTLI(GetTLI) {}

  bool run(Module &M);

private:
  DTransSafetyInfo *const DTInfo;
  std::function<const TargetLibraryInfo &(const Function &)> GetTLI;

  bool doCollection(Module &M);
  bool doTransformation(Module &M);

  bool
  collectCandidateStructFields(StructField2FieldsMapTy &StructField2FieldsMap,
                               CandidateStructFieldsTy &CandidateStructFields);

  bool isValidPtrOfPtr(Module &M, Value *V, SmallVectorImpl<size_t> &CandFields,
                       Value *&CorePtr);
  bool isValidPtr(Module &M, Value *V, Value *CorePtr);

  // Returns false if \p I is not supported instruction.
  bool IsInstructionSupported(Instruction *I);

  // Classify each candidate store instruction by structure/field and the
  // base pointer of GEP.
  bool classifyCandStoreInst(SF2BP2VIMapTy &SF2BP2VIMap,
                             CandidateStructFieldsTy &CandidateStructFields,
                             BasicBlock *BB);

  void foldToSameValue(SF2BP2VIMapTy &SF2BP2VIMap);

  bool storesReachLegallyWithSameValue(SF2BP2VIMapTy &SF2BP2VIMap, Type *Struct,
                                       size_t Field0, size_t Field1,
                                       Value *BasePointer, VITy VI0, VITy VI1);

  void classifyFieldsByValue(SF2BP2VIMapTy &SF2BP2VIMap,
                             StructField2FieldsMapTy &StructField2FieldsMap,
                             MissingFieldsListTy &MissingFieldsList);

  void calcFieldsIntersect(Module &M,
                           StructField2FieldsMapTy &StructField2FieldsMap,
                           MissingFieldsListTy &MissingFieldsList);

  std::map<Type *, std::map<size_t, size_t>> StructReuseFieldMap;
  std::map<StructFieldTy, SmallVector<LoadInst *>> StructField2Loads;
};

} // end anonymous namespace

template <class T>
static SmallVector<T> disjoint(SmallVectorImpl<T> &SetA,
                               SmallVectorImpl<T> &SetB) {
  SmallVector<T> NewSet;

  uint64_t I0 = 0, E0 = SetA.size();
  uint64_t I1 = 0, E1 = SetB.size();

  while (I0 < E0 && I1 < E1) {
    if (SetA[I0] == SetB[I1]) {
      ++I0;
      ++I1;
    } else if (SetA[I0] > SetB[I1]) {
      NewSet.push_back(SetB[I1]);
      ++I1;
    } else {
      NewSet.push_back(SetA[I0]);
      ++I0;
    }
  }

  while (I0 < E0)
    NewSet.push_back(SetA[I0++]);

  while (I1 < E1)
    NewSet.push_back(SetB[I1++]);

  return NewSet;
}

template <class T>
static SmallVector<T> intersect(SmallVectorImpl<T> &SetA,
                                SmallVectorImpl<T> &SetB) {
  SmallVector<T> NewSet;

  uint64_t I0 = 0, E0 = SetA.size();
  uint64_t I1 = 0, E1 = SetB.size();

  while (I0 < E0 && I1 < E1) {
    if (SetA[I0] == SetB[I1]) {
      NewSet.push_back(SetA[I0]);
      ++I0;
      ++I1;
    } else if (SetA[I0] > SetB[I1]) {
      ++I1;
    } else {
      ++I0;
    }
  }

  return NewSet;
}

// Returns false if \p I is not supported instruction.
bool ReuseFieldOPImpl::IsInstructionSupported(Instruction *I) {
  // TODO: Add more unsupported instructions
  if (isa<InvokeInst>(I))
    return false;
  return true;
}

bool ReuseFieldOPImpl::collectCandidateStructFields(
    StructField2FieldsMapTy &StructField2FieldsMap,
    CandidateStructFieldsTy &CandidateStructFields) {

  LLVM_DEBUG(dbgs() << "Reuse field: looking for "
                       "candidate structure field.\n");

  for (dtrans::TypeInfo *TI : DTInfo->type_info_entries()) {
    auto *StInfo = dyn_cast<dtrans::StructInfo>(TI);
    if (!StInfo)
      continue;

    // Don't try to delete fields from literal structures.
    if (cast<StructType>(StInfo->getLLVMType())->isLiteral())
      continue;

    LLVM_DEBUG(dbgs() << "LLVM Type: ";
               StInfo->getLLVMType()->print(dbgs(), true, true);
               dbgs() << "\n");

    if (DTInfo->testSafetyData(TI, dtrans::DT_ReuseField)) {
      LLVM_DEBUG({ dbgs() << "  Rejecting based on safety data.\n"; });
      continue;
    }

    if (DTInfo->getMaxTotalFrequency() != StInfo->getTotalFrequency()) {
      LLVM_DEBUG({ dbgs() << "  Rejecting based on heuristic.\n"; });
      continue;
    }

    SmallSet<size_t, 4> CandidateFieldsSet;
    SmallVector<size_t> CandidateFieldsList;
    size_t NumFields = StInfo->getNumFields();

    // The candidate field must be i64.
    for (size_t I = 0; I < NumFields; ++I) {
      dtrans::FieldInfo &FI = StInfo->getField(I);
      if (FI.getLLVMType()->isIntegerTy(64)) {
        CandidateFieldsSet.insert(I);
        CandidateFieldsList.push_back(I);
      }
    }

    if (CandidateFieldsList.size() >= 2) {
      // Assume each field always set the same value with other fields when
      // do initialize.
      for (auto Field : CandidateFieldsList)
        StructField2FieldsMap[std::make_pair(
            StInfo->getLLVMType(), (size_t)Field)] = CandidateFieldsList;

      CandidateStructFields[StInfo] = CandidateFieldsSet;
    }
  }

  if (CandidateStructFields.size() != 1) {
    LLVM_DEBUG(dbgs() << "  No candidates found.\n");
    return false;
  }

  LLVM_DEBUG({
    for (auto CandidateStructFieldsPair : CandidateStructFields) {
      auto StInfo = CandidateStructFieldsPair.first;
      dbgs() << "Candidate structure for reuse field: ";
      StInfo->getLLVMType()->print(dbgs(), true, true);
      auto FieldsSet = CandidateStructFieldsPair.second;
      size_t NumFields = StInfo->getNumFields();
      dbgs() << "\tFields: { ";
      for (size_t I = 0; I < NumFields; ++I)
        if (FieldsSet.count(I))
          dbgs() << I << " ";
      dbgs() << "}\n";
    }
  });

  return true;
}

// 1. Classify each candidate store instruction by structure-field pair and the
// base pointer of GEP.
// 2. Save each candidate load instruction.
bool ReuseFieldOPImpl::classifyCandStoreInst(
    SF2BP2VIMapTy &SF2BP2VIMap, CandidateStructFieldsTy &CandidateStructFields,
    BasicBlock *BB) {

  // Return true if the structure-field is a candidate pair.
  auto isCandidateStructField = [&](StructFieldTy StructField) {
    Type *Ty = StructField.first;
    size_t Field = StructField.second;

    if (!Ty || !Ty->isStructTy())
      return false;

    auto *StInfo = DTInfo->getStructInfo(cast<StructType>(Ty));
    auto StructIt = CandidateStructFields.find(StInfo);
    if (StructIt != CandidateStructFields.end())
      return CandidateStructFields[StInfo].count(Field) > 0;

    return false;
  };

  // Return true if the structure-field is a candidate pair.
  auto isCandidateDTransTypeField =
      [&](std::pair<DTransType *, size_t> StructField) {
        DTransType *Ty = StructField.first;
        size_t Field = StructField.second;

        return isCandidateStructField(std::make_pair(Ty->getLLVMType(), Field));
      };

  // If GEP is accessing struct field, it returns the field.
  // It doesn't handle if NumIndices > 2.
  auto getAccessStructField = [&](GEPOperator *GEP) {
    int32_t NumIndices = GEP->getNumIndices();
    if (NumIndices > 2)
      return std::make_pair((DTransType *)nullptr, (size_t)0);
    if (NumIndices == 1)
      return DTInfo->getByteFlattenedGEPElement(GEP);
    auto ElemTy = GEP->getSourceElementType();
    if (!isa<StructType>(ElemTy))
      return std::make_pair((DTransType *)nullptr, (size_t)0);
    size_t FieldIndex =
        (size_t)(cast<ConstantInt>(GEP->getOperand(2))->getLimitedValue());
    return std::make_pair(
        DTInfo->getStructInfo(cast<StructType>(ElemTy))->getDTransType(),
        FieldIndex);
  };

  // Make sure the load/store will not access the candidate structure field.
  auto AnalyzeMultiElemLdSt = [&](Instruction *I) {
    Value *PtrOp;
    if (auto *LI = dyn_cast<LoadInst>(I))
      PtrOp = LI->getPointerOperand();
    else if (auto *SI = dyn_cast<StoreInst>(I))
      PtrOp = SI->getPointerOperand();
    else
      llvm_unreachable("Unexpected Instruction for MultiElem ld/st");

    auto *Sel = dyn_cast<SelectInst>(PtrOp);
    if (!Sel)
      return false;

    Value *TV = Sel->getTrueValue();
    Value *FV = Sel->getFalseValue();
    GEPOperator *TGEP = dyn_cast<GEPOperator>(TV);
    GEPOperator *FGEP = dyn_cast<GEPOperator>(FV);
    if (!TGEP || !FGEP)
      return false;
    auto TElem = getAccessStructField(TGEP);
    if (!TElem.first)
      return false;
    auto FElem = getAccessStructField(FGEP);
    if (!FElem.first)
      return false;

    if (isCandidateDTransTypeField(TElem) || isCandidateDTransTypeField(FElem))
      return false;

    return true;
  };

  auto isValidPointerOperand = [&](Value *PointerOperand) {
    // Make sure all the load/store's pointer operand is GEP, just for
    // processing easier.
    auto GEP = dyn_cast<GEPOperator>(PointerOperand);
    if (!GEP) {
      LLVM_DEBUG(dbgs() << "Pointer operand is not a GEP.\n");
      return false;
    }

    // Only support 1/2 indices.
    int32_t NumIndices = GEP->getNumIndices();
    if (NumIndices != 1 && NumIndices != 2) {
      LLVM_DEBUG(dbgs() << "Don't support " << NumIndices << " indices.\n");
      return false;
    }

    // Zero as the first operand of GEP is common.
    ConstantInt *C = dyn_cast<ConstantInt>(GEP->idx_begin());
    if (!C || !C->isZero()) {
      LLVM_DEBUG(dbgs() << "The 0-th index must be 0.\n");
      return false;
    }

    return true;
  };

  // 1. Record all the stores which will access the candidate field by a
  // 2-level hashtable. The hashtable's key is struct-field pair and the base
  // pointer of store's GEP, the hashtable's value also contains the stored
  // value since sometime optimizers don't fold two same stored value to one. If
  // we make sure two store saved the same value, we can change table's value
  // directly and lookup the value quickly.
  // 2. Record all the candidate load instructions.
  for (auto II = BB->begin(), E = BB->end(); II != E; ++II) {
    Instruction &Inst = *II;
    if (!IsInstructionSupported(&Inst)) {
      LLVM_DEBUG(dbgs() << "Doesn't support instrution:" << Inst << "\n";);
      return false;
    }

    if (DTInfo->isMultiElemLoadStore(&Inst)) {
      if (!AnalyzeMultiElemLdSt(&Inst)) {
        LLVM_DEBUG(dbgs() << "Multi-element load/store instrution:" << Inst
                          << "\n";);
        return false;
      }
      continue;
    }

    StructFieldTy SF;
    Value *PointerOperand = nullptr;

    // TODO: Check load/store is simple?
    if (auto SI = dyn_cast<StoreInst>(&Inst)) {
      SF = DTInfo->getStoreElement(SI);
      PointerOperand = SI->getPointerOperand();
    } else if (auto LI = dyn_cast<LoadInst>(&Inst)) {
      SF = DTInfo->getLoadElement(LI);
      PointerOperand = LI->getPointerOperand();
    } else {
      continue;
    }

    if (isCandidateStructField(SF)) {
      LLVM_DEBUG(dbgs() << "New Inst:" << Inst << "\n\tStruct Type:"
                        << *SF.first << "\tField:" << SF.second << "\n");

      if (!isValidPointerOperand(PointerOperand))
        return false;

      if (auto SI = dyn_cast<StoreInst>(&Inst)) {
        auto BasePointer =
            cast<GEPOperator>(PointerOperand)->getPointerOperand();
        if (SF2BP2VIMap[SF].find(BasePointer) != SF2BP2VIMap[SF].end()) {
          LLVM_DEBUG(dbgs() << "Mult-store to the same field.\n");
          return false;
        }
        auto StValue = SI->getValueOperand();
        SF2BP2VIMap[SF][BasePointer] = std::make_pair(StValue, SI);
      } else {
        auto LI = cast<LoadInst>(&Inst);
        StructField2Loads[SF].push_back(LI);
      }
    }
  }

  return true;
}

// Try to match the following pattern, and assume SI1 and SI2 save the same
// value.
// %37 = getelementptr %struct.arc, ptr %25, i64 0, i32 1         <- LdGEP
// %38 = load i64, ptr %37, align 8                               <- Load1
// %39 = getelementptr %struct.arc, ptr %32, i64 0, i32 1         <- StGEP1
// store i64 %38, ptr %39, align 8                                <- SI1
// %40 = load i64, ptr %37, align 8                               <- Load2
// %41 = getelementptr %struct.arc, ptr %32, i64 0, i32 8         <- StGEP2
// store i64 %40, ptr %41, align 8                                <- SI2
//
// Since the pointer of %40 is equal to %38, and SafetyDataTest can guarantee
// there is no partial store between SI1 and Load2, so we can make sure %40 is
// equal to %38.
void ReuseFieldOPImpl::foldToSameValue(SF2BP2VIMapTy &SF2BP2VIMap) {
  for (auto SF2BP2VIPair : SF2BP2VIMap) {
    auto BP2VIMap = SF2BP2VIPair.getSecond();
    for (auto BP2VIPair : BP2VIMap) {
      Value *BasePointer = BP2VIPair.getFirst();
      Value *StValue = BP2VIPair.getSecond().first;
      StoreInst *StInst = BP2VIPair.getSecond().second;

      auto Load1 = dyn_cast<LoadInst>(StValue);
      if (!Load1 || Load1->getNumUses() != 1)
        continue;

      auto StGEP1 =
          dyn_cast<GetElementPtrInst>(Load1->getNextNonDebugInstruction());
      if (!StGEP1)
        continue;

      auto SI1 = dyn_cast<StoreInst>(StGEP1->getNextNonDebugInstruction());
      if (!SI1 || SI1->getPointerOperand() != StGEP1 || SI1 != StInst)
        continue;

      auto Load2 = dyn_cast<LoadInst>(SI1->getNextNonDebugInstruction());
      if (!Load2 || Load2->getPointerOperand() != Load1->getPointerOperand() ||
          Load2->getNumUses() != 1)
        continue;

      auto StGEP2 =
          dyn_cast<GetElementPtrInst>(Load2->getNextNonDebugInstruction());
      if (!StGEP2 || StGEP2->getPointerOperand() != StGEP1->getPointerOperand())
        continue;

      auto SI2 = dyn_cast<StoreInst>(StGEP2->getNextNonDebugInstruction());
      if (!SI2 || SI2->getPointerOperand() != StGEP2 ||
          SI2->getValueOperand() != Load2)
        continue;

      auto StElem2 = DTInfo->getStoreElement(SI2);
      if (SF2BP2VIPair.getFirst().first != StElem2.first)
        continue;

      auto VI2 = SF2BP2VIMap[StElem2][BasePointer];
      if (VI2.first != Load2 || VI2.second != SI2)
        continue;

      LLVM_DEBUG(dbgs() << "\nFold following stores with same value:\n";
                 dbgs() << "\t" << *SI1 << "\n\t" << *SI2;);

      SF2BP2VIMap[StElem2][BasePointer].first = Load1;
    }
  }
}

// Check if the two store instructions are saved the same value, and they can be
// reached legally.
bool ReuseFieldOPImpl::storesReachLegallyWithSameValue(
    SF2BP2VIMapTy &SF2BP2VIMap, Type *Struct, size_t Field0, size_t Field1,
    Value *BasePointer, VITy VI0, VITy VI1) {

  auto ReachLegally = [&](StoreInst *From, StoreInst *To) {
    uint32_t Count = 0;
    const uint32_t Limit = 10;

    auto *II = From->getNextNonDebugInstruction();
    while (Count++ < Limit && II != nullptr) {
      if (II == To)
        return true;

      if (II->mayWriteToMemory()) {
        if (auto SI = dyn_cast<StoreInst>(II)) {
          // If there is other store who saves value to the field of 'From' or
          // 'To', we can't guarantee the saved value of 'From' is same with
          // 'To'.
          auto IIElem = DTInfo->getStoreElement(SI);
          auto FromElem = DTInfo->getStoreElement(From);
          auto ToElem = DTInfo->getStoreElement(To);

          assert(FromElem.first && ToElem.first &&
                 "FromElem and ToElem must be analyzable.");

          if (IIElem.first == FromElem.first &&
              IIElem.second == FromElem.second)
            return false;

          if (IIElem.first == ToElem.first && IIElem.second == ToElem.second)
            return false;
        } else
          return false;
      } else if (II->mayReadFromMemory()) {
        if (auto LI = dyn_cast<LoadInst>(II)) {
          // If there is other load who gets the value of 'To' before 'To',
          // we can't change the load's field to the field of 'From'.
          auto IIElem = DTInfo->getLoadElement(LI);
          auto ToElem = DTInfo->getStoreElement(To);

          assert(ToElem.first && "ToElem must be analyzable.");

          if (IIElem.first == ToElem.first && IIElem.second == ToElem.second)
            return false;
        } else
          return false;
      }

      II = II->getNextNonDebugInstruction();
    }

    return false;
  };

  Value *Value0 = VI0.first;
  StoreInst *SI0 = VI0.second;

  Value *Value1 = VI1.first;
  StoreInst *SI1 = VI1.second;

  LLVM_DEBUG(dbgs() << "Candidate 0:" << *SI0 << "\tField:" << Field0
                    << "\nCandidate 1:" << *SI1 << "\tField:" << Field1
                    << "\n";);

  if (Value1 != Value0) {
    LLVM_DEBUG(dbgs() << "Bail Out: Different values.\n");
    return false;
  }

  if (!(ReachLegally(SI0, SI1) || ReachLegally(SI1, SI0))) {
    LLVM_DEBUG(dbgs() << "Bail Out: SI0 can't reach to SI1 legally.\n");
    return false;
  }

  LLVM_DEBUG(dbgs() << "Is candidate!\n");
  return true;
}

// Walk through each element of the 2-level hashtable, if there is another
// struct-field and base pointer save the same value with it, just record
// this and update the StructField2FieldsMap table which records every
// struct-field pairs have the same value with key by intersecting.
void ReuseFieldOPImpl::classifyFieldsByValue(
    SF2BP2VIMapTy &SF2BP2VIMap, StructField2FieldsMapTy &StructField2FieldsMap,
    MissingFieldsListTy &MissingFieldsList) {

  // Walk though each struct-field pair.
  for (auto SF2BP2VIIt = SF2BP2VIMap.begin(), SF2BP2VIE = SF2BP2VIMap.end();
       SF2BP2VIIt != SF2BP2VIE; ++SF2BP2VIIt) {

    auto &SF2BP2VI0 = *SF2BP2VIIt;
    StructFieldTy SF0 = SF2BP2VI0.getFirst();
    auto &BP2VI0 = SF2BP2VI0.getSecond();

    Type *Struct0 = SF0.first;
    size_t Field0 = SF0.second;

    SmallVector<size_t> FullCandFields;
    for (size_t Field = 0, E = Struct0->getStructNumElements(); Field < E;
         ++Field)
      FullCandFields.push_back(Field);

    // Walk though each base pointer.
    for (auto BP2VIIt0 = BP2VI0.begin(), BP2VIE0 = BP2VI0.end();
         BP2VIIt0 != BP2VIE0; ++BP2VIIt0) {
      auto &BP2VI0 = *BP2VIIt0;
      Value *BP0 = BP2VI0.getFirst();
      VITy &VI0 = BP2VI0.getSecond();

      SmallVector<size_t> SameValueFields;
      SameValueFields.push_back(Field0);

      for (auto SF2BP2VIIt1 = SF2BP2VIMap.begin(); SF2BP2VIIt1 != SF2BP2VIE;
           ++SF2BP2VIIt1) {

        auto &SF2BP2VI1 = *SF2BP2VIIt1;
        StructFieldTy SF1 = SF2BP2VI1.getFirst();
        auto &BP2VI1 = SF2BP2VI1.getSecond();

        Type *Struct1 = SF1.first;
        size_t Field1 = SF1.second;

        // Don't handle different structures.
        if (Struct1 != Struct0)
          continue;

        // Make sure they aren't the same field.
        if (Field0 == Field1)
          continue;

        for (auto BP2VIIt1 = BP2VI1.begin(), BP2VIE1 = BP2VI1.end();
             BP2VIIt1 != BP2VIE1; ++BP2VIIt1) {

          auto &BP2VI1 = *BP2VIIt1;
          Value *BP1 = BP2VI1.getFirst();
          VITy &VI1 = BP2VI1.getSecond();

          // Make sure they use the same base pointer.
          if (BP0 != BP1)
            continue;

          LLVM_DEBUG(dbgs() << "\nStructure:" << *Struct0;
                     dbgs() << " Base pointer:" << *BP0 << "\n";);

          if (!storesReachLegallyWithSameValue(SF2BP2VIMap, Struct0, Field0,
                                               Field1, BP0, VI0, VI1))
            continue;

          // Means for this base pointer, field0 saved the same value with
          // field1 in this basic block.
          SameValueFields.push_back(Field1);
          break;
        }
      }

      // Disjoint/intersect only support sorted date.
      std::sort(SameValueFields.begin(), SameValueFields.end());

      {
        // Record limited field0's store instruction, when field0 doesn't have
        // the same value with the missing field.
        auto MissingFields = disjoint(FullCandFields, SameValueFields);
        for (size_t MissingField : MissingFields) {
          auto &StoreList = MissingFieldsList[Struct0][Field0][MissingField];
          if (StoreList.size() < 2)
            StoreList.push_back(VI0.second);
        }
      }

      // Update the same value fields for this struct-field pair.
      StructField2FieldsMap[SF0] =
          intersect(SameValueFields, StructField2FieldsMap[SF0]);
    }
  }
}

bool ReuseFieldOPImpl::isValidPtrOfPtr(Module &M, Value *V,
                                       SmallVectorImpl<size_t> &CandFields,
                                       Value *&CorePtr) {
  auto StructField = dyn_cast<GetElementPtrInst>(V);
  if (!StructField)
    return false;

  auto StructIndex =
      dyn_cast<GetElementPtrInst>(StructField->getPointerOperand());
  if (!StructIndex || StructIndex->getNumIndices() != 1)
    return false;

  StoreInst *StoreStructIndex = nullptr;
  for (auto GEPStructIndexUser : StructIndex->users()) {
    if (auto ElemGEP = dyn_cast<GetElementPtrInst>(GEPStructIndexUser)) {
      if (ElemGEP->getNumIndices() != 2)
        return false;
      if (StructIndex != ElemGEP->getPointerOperand())
        return false;
    } else if (auto SI = dyn_cast<StoreInst>(GEPStructIndexUser)) {
      if (StoreStructIndex)
        return false;
      if (StructIndex != SI->getValueOperand())
        return false;
      StoreStructIndex = SI;
    } else
      return false;
  }

  if (!StoreStructIndex)
    return false;

  auto StoreAddress =
      dyn_cast<GetElementPtrInst>(StoreStructIndex->getPointerOperand());
  if (!StoreAddress || StoreAddress->getNumIndices() != 1)
    return false;

  CorePtr = StoreAddress;
  auto LoadAddress = dyn_cast<LoadInst>(StoreAddress->getPointerOperand());
  if (!LoadAddress)
    return false;

  auto LdElem = DTInfo->getLoadElement(LoadAddress);
  Type *GVTy = LdElem.first;
  size_t GVCandidateField = LdElem.second;

  if (!GVTy || !GVTy->isStructTy())
    return false;

  if (DTInfo->testSafetyData(DTInfo->getStructInfo(cast<StructType>(GVTy)),
                             dtrans::DT_ReuseFieldPtrOfPtr))
    return false;

  auto GVField = dyn_cast<GetElementPtrInst>(LoadAddress->getPointerOperand());
  if (!GVField || GVField->getNumIndices() != 2)
    return false;

  auto GV = dyn_cast<GlobalVariable>(GVField->getPointerOperand());
  if (!GV)
    return false;

  for (auto GVUser : GV->users()) {
    if (auto SI = dyn_cast<StoreInst>(GVUser)) {
      if (SI->getPointerOperand() != GV)
        return false;
      continue;
    }

    auto GVField = dyn_cast<GEPOperator>(GVUser);
    if (!GVField || !GVField->hasOneUse())
      return false;

    auto NumIndices = GVField->getNumIndices();
    if (NumIndices <= 1)
      return false;

    auto Idx = dyn_cast<ConstantInt>(GVField->getOperand(2));
    if (!Idx)
      return false;

    size_t FieldIndex = (size_t)(Idx->getLimitedValue());

    // Skip if it's not accessing the candidate field.
    if (FieldIndex != GVCandidateField)
      continue;

    assert(NumIndices == 2);

    // Store value to the candidate field of the global variable is OK, we
    // want to check if:
    // 1. There is any read of candidate field.
    // 2. Propagate the address to other place.
    if (auto SI = dyn_cast<StoreInst>(*GVField->user_begin())) {
      if (SI->getPointerOperand() != GVField)
        return false;
      continue;
    }

    auto LoadAddress = dyn_cast<LoadInst>(*GVField->user_begin());
    if (!LoadAddress || !LoadAddress->hasOneUse())
      return false;

    auto IndexOfPtrOfPtr =
        dyn_cast<GetElementPtrInst>(*LoadAddress->user_begin());
    if (!IndexOfPtrOfPtr || IndexOfPtrOfPtr->getNumIndices() != 1)
      return false;

    for (auto IndexOfPtrOfPtrUser : IndexOfPtrOfPtr->users()) {
      auto StructPtr = dyn_cast<LoadInst>(IndexOfPtrOfPtrUser);
      if (!StructPtr) {
        // Store is OK, if it is the pointer operand.
        if (auto SI = dyn_cast<StoreInst>(IndexOfPtrOfPtrUser))
          if (SI->getPointerOperand() == IndexOfPtrOfPtr)
            continue;
        return false;
      }

      for (auto StructPtrUser : StructPtr->users()) {
        if (auto StructField = dyn_cast<GEPOperator>(StructPtrUser)) {
          // Limitation can be relaxed in the future.
          if (StructField->getNumIndices() != 2)
            return false;

          if (StructField->getPointerOperand() != StructPtr)
            return false;

          auto Idx = dyn_cast<ConstantInt>(StructField->getOperand(2));
          if (!Idx)
            return false;

          auto AccessField = (size_t)(Idx->getLimitedValue());

          // If we don't even access the candidate field, we will not do any
          // read of the candidate field.
          if (std::find(CandFields.begin(), CandFields.end(), AccessField) !=
              CandFields.end())
            return false;

        } else if (auto PHI = dyn_cast<PHINode>(StructPtrUser)) {
          if (PHI->getNumUses() != 1)
            return false;

          // Program propagates the address to other place, need to check in
          // further.
          auto StoreStruct = dyn_cast<StoreInst>(*PHI->user_begin());
          if (!StoreStruct)
            return false;

          // Program just propagates the address to its own.
          if (StoreStruct->getPointerOperand() != IndexOfPtrOfPtr)
            return false;

        } else if (auto P2I = dyn_cast<PtrToIntInst>(StructPtrUser)) {
          if (!P2I->hasOneUse())
            return false;

          auto Sub = dyn_cast<BinaryOperator>(*P2I->user_begin());
          if (!Sub || !Sub->hasOneUse())
            return false;

          auto Div = dyn_cast<BinaryOperator>(*Sub->user_begin());
          if (!Div || !Div->hasOneUse())
            return false;

          auto ArcI = dyn_cast<GetElementPtrInst>(*Div->user_begin());
          if (!ArcI || !ArcI->hasOneUse() || ArcI->getNumIndices() != 1)
            return false;

          auto SI = dyn_cast<StoreInst>(*ArcI->user_begin());
          if (!SI)
            return false;

          if (SI->getPointerOperand() != IndexOfPtrOfPtr)
            return false;
        } else {
          if (!isa<CmpInst>(StructPtrUser))
            return false;
        }
      }
    }
  }

  return true;
}

bool ReuseFieldOPImpl::isValidPtr(Module &M, Value *V, Value *CorePtr) {

  auto StructField = dyn_cast<GetElementPtrInst>(V);
  if (!StructField)
    return false;

  auto StructIndex =
      dyn_cast<GetElementPtrInst>(StructField->getPointerOperand());
  if (!StructIndex || StructIndex->getNumIndices() != 1)
    return false;

  auto LoadPtr = dyn_cast<LoadInst>(StructIndex->getPointerOperand());
  if (!LoadPtr)
    return false;

  auto LdElem = DTInfo->getLoadElement(LoadPtr);
  Type *GVTy = LdElem.first;
  size_t GVCandidateField = LdElem.second;

  if (!GVTy || !GVTy->isStructTy())
    return false;

  if (DTInfo->testSafetyData(DTInfo->getStructInfo(cast<StructType>(GVTy)),
                             dtrans::DT_ReuseFieldPtr))
    return false;

  auto GVField = dyn_cast<GEPOperator>(LoadPtr->getPointerOperand());
  if (!GVField || GVField->getNumIndices() != 2)
    return false;

  auto GV = dyn_cast<GlobalVariable>(GVField->getPointerOperand());
  if (!GV)
    return false;

  auto isValid2IndicesGEP = [&](GetElementPtrInst *GEP) {
    if (GEP->getNumIndices() != 2)
      return false;

    for (auto User : GEP->users()) {
      auto II = dyn_cast<Instruction>(User);
      if (!II)
        return false;

      if (auto SI = dyn_cast<StoreInst>(II)) {
        if (SI->getPointerOperand() != GEP)
          return false;
      } else {
        if (!dtrans::DTransAnnotator::isDTransPtrAnnotation(*II))
          return false;
      }
    }
    return true;
  };

  for (auto GVUseIt = GV->use_begin(), GVUseE = GV->use_end();
       GVUseIt != GVUseE; ++GVUseIt) {
    auto GVUser = GVUseIt->getUser();
    auto GVPos = GVUseIt->getOperandNo();
    if (auto GVField = dyn_cast<GEPOperator>(GVUser)) {
      unsigned NumIndices = GVField->getNumIndices();
      if (NumIndices <= 1)
        return false;

      auto Idx = dyn_cast<ConstantInt>(GVField->getOperand(2));
      if (!Idx)
        return false;

      size_t FieldIndex = (size_t)(Idx->getLimitedValue());

      // Skip if it's not accessing candidate structure field.
      if (FieldIndex != GVCandidateField)
        continue;

      assert(NumIndices == 2);

      for (auto GVFieldUser : GVField->users()) {
        if (auto LI = dyn_cast<LoadInst>(GVFieldUser)) {
          // Get the start address of the candidate structure array now.
          // It's still safe if it doesn't access candidate fields.
          for (auto LoadUser : LI->users()) {
            if (auto GEP = dyn_cast<GetElementPtrInst>(LoadUser)) {
              if (GEP->getNumIndices() == 1) {
                for (auto GEPUser : GEP->users()) {
                  if (auto NextGEP = dyn_cast<GetElementPtrInst>(GEPUser)) {
                    if (!isValid2IndicesGEP(NextGEP))
                      return false;
                  } else if (auto SI = dyn_cast<StoreInst>(GEPUser)) {
                    if (SI->getPointerOperand() != CorePtr)
                      return false;
                  } else
                    return false;
                }
              } else if (!isValid2IndicesGEP(GEP))
                return false;

            } else if (auto CI = dyn_cast<CallInst>(LoadUser)) {
              Function *F = LI->getFunction();

              // It's safe if it is just call free.
              LibFunc Func;
              if (!GetTLI(*F).getLibFunc(*CI, Func) || Func != LibFunc_free)
                return false;
            } else if (!isa<CmpInst>(LoadUser))
              return false;
          }
        } else {
          if (auto SI = dyn_cast<StoreInst>(GVFieldUser))
            if (SI->getPointerOperand() == GVField)
              continue;
          return false;
        }
      }
    } else if (auto Caller = dyn_cast<CallInst>(GVUser)) {
      auto Callee = Caller->getCalledFunction();

      if (Callee->isDeclaration())
        continue;
      // FieldAddressTakenCall is active, but the argument
      // doesn't have users.
      if (Callee->getArg(GVPos)->hasNUsesOrMore(1))
        return false;
    } else {
      return false;
    }
  }

  return true;
}

// We know when the key field of StructField2FieldsMap is set, which fields will
// be set together, but we don't know when the mapped fields are set, if the key
// field is also set. In this function, we calculate the set whose elements are
// always set together each other.
void ReuseFieldOPImpl::calcFieldsIntersect(
    Module &M, StructField2FieldsMapTy &StructField2FieldsMap,
    MissingFieldsListTy &MissingFieldsList) {
  SmallSet<Type *, 8> VisitedStruct;

  LLVM_DEBUG(dbgs() << "======= StructField2FieldsMap =======\n";
             for (auto Field2FeildsA
                  : StructField2FieldsMap) {
               StructFieldTy SF0 = Field2FeildsA.first;
               SmallVector<size_t> MappingFields0 = Field2FeildsA.second;

               Type *StructTy = SF0.first;
               size_t Field0 = SF0.second;

               dbgs() << "LLVM Type: ";
               StructTy->print(dbgs(), true);
               dbgs() << " Field: " << Field0 << " Same value fields: { ";
               for (auto Field : MappingFields0)
                 dbgs() << Field << " ";
               dbgs() << "}\n";
             } dbgs()
             << "======= calcFieldsIntersect =======\n";);

  for (auto Field2FeildsA : StructField2FieldsMap) {
    StructFieldTy SF0 = Field2FeildsA.first;
    SmallVector<size_t> MappingFields0 = Field2FeildsA.second;

    Type *StructTy = SF0.first;
    size_t Field0 = SF0.second;

    if (VisitedStruct.count(StructTy))
      continue;

    LLVM_DEBUG(dbgs() << "LLVM Type: "; StructTy->print(dbgs(), true, true);
               dbgs() << " Field: " << Field0 << "\n";);

    // If there is one basic block doesn't set 2 fields together with the same
    // value, function 'isValidPtr' and 'isValidPtrOfPtr' will try to prove
    // program never read the value of these 2 fields from the issue structure
    // variable.
    if (MappingFields0.size() == 2) {
      size_t Field1 = MappingFields0[0];
      if (Field0 == Field1)
        Field1 = MappingFields0[1];

      auto SF1 = std::make_pair(StructTy, Field1);
      SmallVector<size_t> MappingFields1 = StructField2FieldsMap[SF1];

      auto ExistStores = MissingFieldsList[StructTy][Field1][Field0];
      if (MappingFields1.size() == 1 && ExistStores.size() == 1) {
        StoreInst *SI = ExistStores[0];
        Value *BasePointer = SI->getPointerOperand();

        Value *CorePtr = nullptr;
        if (isValidPtrOfPtr(M, BasePointer, MappingFields0, CorePtr) &&
            isValidPtr(M, BasePointer, CorePtr)) {
          LLVM_DEBUG(dbgs() << "Success!\n";);
          auto &ReuseFieldMap = StructReuseFieldMap[StructTy];
          ReuseFieldMap[Field0] = Field1;
          ReuseFieldMap[Field1] = Field1;
          VisitedStruct.insert(StructTy);
          continue;
        }
      }
    }

    SmallVector<size_t> MinFields = MappingFields0;
    for (auto Field : MappingFields0) {
      auto SF1 = std::make_pair(StructTy, (size_t)Field);
      MinFields = intersect(MinFields, StructField2FieldsMap[SF1]);
    }

    if (MinFields.size() > 1) {
      LLVM_DEBUG(dbgs() << "Success!\n";);
      auto &ReuseFieldMap = StructReuseFieldMap[StructTy];
      auto ToField = MinFields[0];
      for (auto Field : MinFields)
        ReuseFieldMap[Field] = ToField;
      VisitedStruct.insert(StructTy);
    }
  }
}

bool ReuseFieldOPImpl::doCollection(Module &M) {

  // It records the fields which always store the same value with key of the
  // map.
  StructField2FieldsMapTy StructField2FieldsMap;
  MissingFieldsListTy MissingFieldsList;
  CandidateStructFieldsTy CandidateStructFields;

  if (!collectCandidateStructFields(StructField2FieldsMap,
                                    CandidateStructFields))
    return false;

  for (Function &F : M) {
    if (F.isDeclaration() || F.isIntrinsic())
      continue;

    LLVM_DEBUG(dbgs() << "  Pruning in routine: " << F.getName() << "\n");

    for (auto BI = F.begin(), BE = F.end(); BI != BE; ++BI) {
      SF2BP2VIMapTy SF2BP2VIMap;
      BasicBlock *BB = &*BI;

      if (!classifyCandStoreInst(SF2BP2VIMap, CandidateStructFields, BB))
        return false;

      foldToSameValue(SF2BP2VIMap);

      classifyFieldsByValue(SF2BP2VIMap, StructField2FieldsMap,
                            MissingFieldsList);
    }
  }

  calcFieldsIntersect(M, StructField2FieldsMap, MissingFieldsList);

  if (StructReuseFieldMap.size() == 0) {
    LLVM_DEBUG({ dbgs() << "No candidate.\n"; });
    return false;
  }

  LLVM_DEBUG({
    for (auto StructFieldsPair : StructReuseFieldMap) {
      auto StTy = StructFieldsPair.first;
      dbgs() << "Reused structure: ";
      StTy->print(dbgs(), true, true);
      auto FieldsMap = StructFieldsPair.second;
      dbgs() << "\n    Field mapping are (From:To): { ";
      for (auto FieldMap : FieldsMap)
        dbgs() << FieldMap.first << ":" << FieldMap.second << " ";
      dbgs() << "}\n";
    }
  });

  return true;
}

// Walk through each load instruction, and modify its GEP's original field to
// mapped field.
bool ReuseFieldOPImpl::doTransformation(Module &M) {

  bool Changed = false;
  auto DL = M.getDataLayout();

  for (auto StructField2LoadsPair : StructField2Loads) {
    auto SF = StructField2LoadsPair.first;
    auto &Loads = StructField2LoadsPair.second;

    auto StructTy = cast<StructType>(SF.first);
    auto FromField = SF.second;

    auto ReuseFieldMap = StructReuseFieldMap[StructTy];
    auto ToFieldIt = ReuseFieldMap.find(FromField);

    if (ToFieldIt != ReuseFieldMap.end() &&
        ToFieldIt->first != ToFieldIt->second) {

      bool IsPacked = StructTy->isPacked();
      for (auto LI : Loads) {
        auto GEP = cast<GEPOperator>(LI->getPointerOperand());

        LLVM_DEBUG(dbgs() << "GEP (before): \n"; GEP->dump(););

        // Fix the field with the new cloned GEP.
        auto NewGEP = GetElementPtrInst::Create(
            StructTy, GEP->getPointerOperand(),
            {Constant::getNullValue(Type::getInt64Ty(M.getContext())),
             ConstantInt::get(Type::getInt32Ty(M.getContext()),
                              ToFieldIt->second)});
        NewGEP->insertBefore(LI);
        LI->replaceUsesOfWith(GEP, NewGEP);
        auto *AffectedGEP = cast<GEPOperator>(NewGEP);
        // Fix the alignment of load.
        dtrans::resetLoadStoreAlignment(AffectedGEP, DL, IsPacked);
        LLVM_DEBUG(dbgs() << "New GEP (after): \n"; NewGEP->dump();
                   dbgs() << "Load: \n"; LI->dump(););

        Changed = true;
      }
    }
  }

  return Changed;
}

bool ReuseFieldOPImpl::run(Module &M) {
  if (!doCollection(M))
    return false;

  return doTransformation(M);
}

bool ReuseFieldOPPass::runImpl(
    Module &M, DTransSafetyInfo &DTInfo,
    std::function<const TargetLibraryInfo &(const Function &)> GetTLI,
    WholeProgramInfo &WPInfo) {

  auto TTIAVX2 = TargetTransformInfo::AdvancedOptLevel::AO_TargetHasIntelAVX2;
  if (!WPInfo.isWholeProgramSafe() || !WPInfo.isAdvancedOptEnabled(TTIAVX2))
    return false;

  if (!DTInfo.useDTransSafetyAnalysis()) {
    LLVM_DEBUG(dbgs() << "  DTransSafetyAnalyzer results not available\n");
    return false;
  }

  ReuseFieldOPImpl Transformer(DTInfo, GetTLI);
  return Transformer.run(M);
}

PreservedAnalyses ReuseFieldOPPass::run(Module &M, ModuleAnalysisManager &AM) {
  auto &DTransInfo = AM.getResult<DTransSafetyAnalyzer>(M);
  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto GetTLI = [&FAM](const Function &F) -> TargetLibraryInfo & {
    return FAM.getResult<TargetLibraryAnalysis>(*(const_cast<Function *>(&F)));
  };
  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);

  if (!runImpl(M, DTransInfo, GetTLI, WPInfo))
    return PreservedAnalyses::all();

  // TODO: Mark the actual preserved analyses.
  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  return PA;
}