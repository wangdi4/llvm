//===------- DTransImmutableAnalysis.cpp - DTrans Immutable Analysis ------===//
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This stores 'immutable' part of DTransAnalysis pass.
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Analysis/DTransImmutableAnalysis.h"
#include "Intel_DTrans/DTransCommon.h"

using namespace llvm;

AnalysisKey DTransImmutableAnalysis::Key;

char DTransImmutableAnalysisWrapper::ID = 0;
INITIALIZE_PASS(DTransImmutableAnalysisWrapper, "dtrans-immutable-analysis",
                "dtrans immutable analysis", false, true)

DTransImmutableAnalysisWrapper::DTransImmutableAnalysisWrapper()
    : ImmutablePass(ID) {
  initializeDTransImmutableAnalysisWrapperPass(
      *PassRegistry::getPassRegistry());
}

ImmutablePass *llvm::createDTransImmutableAnalysisWrapperPass() {
  return new DTransImmutableAnalysisWrapper();
}

DTransImmutableInfo DTransImmutableAnalysis::run(Module &M,
                                                 ModuleAnalysisManager &AM) {
  return DTransImmutableInfo();
}

void DTransImmutableInfo::addStructFieldInfo(
    StructType *StructTy, unsigned FieldNum,
    SmallPtrSetImpl<Constant *> &LikelyValues,
    SmallPtrSetImpl<Constant *> &LikelyIndirectArrayValues) {

  StructInfo *SInfo = nullptr;

  auto Iter = StructInfoMap.find(StructTy);

  if (Iter == StructInfoMap.end()) {
    SInfo = new StructInfo(StructTy);

    StructInfoMap.insert(std::make_pair(StructTy, SInfo));
  } else {
    SInfo = Iter->second;
  }

  // Overwrite previous entries. Multiple runs of dtrans analysis can 'refine'
  // this information.
  SInfo->Fields[FieldNum].LikelyValues.assign(LikelyValues.begin(),
                                              LikelyValues.end());

  SInfo->Fields[FieldNum].LikelyIndirectArrayValues.assign(
      LikelyIndirectArrayValues.begin(), LikelyIndirectArrayValues.end());
}

const SmallVectorImpl<llvm::Constant *> *
DTransImmutableInfo::getLikelyConstantValues(StructType *StructTy,
                                             unsigned FieldNum) {
  auto Iter = StructInfoMap.find(StructTy);

  if (Iter == StructInfoMap.end()) {
    return nullptr;
  }

  return &Iter->second->Fields[FieldNum].LikelyValues;
}

const SmallVectorImpl<llvm::Constant *> *
DTransImmutableInfo::getLikelyIndirectArrayConstantValues(StructType *StructTy,
                                                          unsigned FieldNum) {
  auto Iter = StructInfoMap.find(StructTy);

  if (Iter == StructInfoMap.end()) {
    return nullptr;
  }

  return &Iter->second->Fields[FieldNum].LikelyIndirectArrayValues;
}

void DTransImmutableInfo::print(raw_ostream &OS) const {

  for (auto &Info : StructInfoMap) {
    OS << "StructType: ";
    Info.first->print(OS);
    OS << "\n";

    unsigned FieldNum = 0;
    for (auto &FieldInfo : Info.second->Fields) {
      OS.indent(2);
      OS << "Field " << FieldNum << ":\n";

      OS.indent(4);
      OS << "Likely Values: ";

      for (auto &Val : FieldInfo.LikelyValues) {
        Val->printAsOperand(OS, false);
        OS << " ";
      }

      OS << "\n";

      OS.indent(4);
      OS << "Likely Indirect Array Values: ";

      for (auto &Val : FieldInfo.LikelyIndirectArrayValues) {
        Val->printAsOperand(OS, false);
        OS << " ";
      }

      OS << "\n";
      FieldNum++;
    }
  }
}
