//===- FeatureDesc.h - Features descriptions --------------------*- C++ -*-===//
//
// Copyright (C) 2023 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/FileSystem.h"
#include "llvm/Support/JSON.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Process.h"

#include <cstddef>
#include <sstream>
#include <thread>
#include <vector>

#ifndef __LLVM_TRANSFORMS_INSTRUMENTATION_INTEL_MLPGO_FEATURE_DESC_H__
#define __LLVM_TRANSFORMS_INSTRUMENTATION_INTEL_MLPGO_FEATURE_DESC_H__

namespace llvm {
namespace mlpgo {

/* source features */
struct BrSrcBBFeturesT {
#define BR_SRC_BB_FEATURE(Type, Name) Type Name;
#include "llvm/Transforms/Instrumentation/Intel_MLPGO/FeatureDescAll.def"
#undef BR_SRC_BB_FEATURE
};

/* successor features */
struct BrSuccFeaturesT {
#define BR_SUCC_BB_FEATURE(Type, Name) Type Name;
#include "llvm/Transforms/Instrumentation/Intel_MLPGO/FeatureDescAll.def"
#undef BR_SUCC_BB_FEATURE
};

class MLBrFeatureVec {
  size_t NumOfSucc = 0;
  // A storage for src BB features and all successors
  std::vector<unsigned char> FeaturesBlob;

public:
  MLBrFeatureVec(size_t NumOfSuccessors)
      : NumOfSucc(NumOfSuccessors),
        FeaturesBlob(
            NumOfSucc * sizeof(BrSuccFeaturesT) + sizeof(BrSrcBBFeturesT), 0) {}

  BrSrcBBFeturesT &getSrcBBFeatures() {
    return *reinterpret_cast<BrSrcBBFeturesT *>(FeaturesBlob.data());
  };

  const BrSrcBBFeturesT &getSrcBBFeatures() const {
    return *reinterpret_cast<const BrSrcBBFeturesT *>(FeaturesBlob.data());
  };

  BrSuccFeaturesT &getSuccFeatures(size_t Idx) {
    assert(Idx < NumOfSucc);
    BrSuccFeaturesT *FirstSucc = reinterpret_cast<BrSuccFeaturesT *>(
        FeaturesBlob.data() + sizeof(BrSrcBBFeturesT));
    return FirstSucc[Idx];
  }

  const BrSuccFeaturesT &getSuccFeatures(size_t Idx) const {
    assert(Idx < NumOfSucc);
    const BrSuccFeaturesT *FirstSucc =
        reinterpret_cast<const BrSuccFeaturesT *>(FeaturesBlob.data() +
                                                  sizeof(BrSrcBBFeturesT));
    return FirstSucc[Idx];
  }

  std::string getSrcAsString() const {
    std::stringstream FeaturesStr;

    const BrSrcBBFeturesT &Src = getSrcBBFeatures();

#define BR_SRC_BB_FEATURE(Type, Name) FeaturesStr << Src.Name << "|";
#include "llvm/Transforms/Instrumentation/Intel_MLPGO/FeatureDesc.def"
#undef BR_SRC_BB_FEATURE

    return FeaturesStr.str();
  }

  std::string getSuccAsString(size_t Idx) const {
    assert(Idx < NumOfSucc);
    std::stringstream FeaturesStr;
    const BrSuccFeaturesT &Succ = getSuccFeatures(Idx);

#define BR_SUCC_BB_FEATURE(Type, Name) FeaturesStr << Succ.Name << "|";
#include "llvm/Transforms/Instrumentation/Intel_MLPGO/FeatureDesc.def"
#undef BR_SUCC_BB_FEATURE

    FeaturesStr << Succ.SuccessorLLVMHeuristicProb << "|";
    FeaturesStr << Succ.SuccessorPGOProb << "|";
    FeaturesStr << (unsigned)((getSrcBBFeatures().srcBBCount >> 32) &
                              0xFFFFFFFF)
                << "|";
    FeaturesStr << (unsigned)((getSrcBBFeatures().srcBBCount) & 0xFFFFFFFF)
                << "|";
    // Print dummy 0 for compatiblity
    FeaturesStr << 0;

    return FeaturesStr.str();
  }

  void dump(raw_fd_ostream &OS) const {
    for (size_t SuccIdx = 0; SuccIdx < NumOfSucc; ++SuccIdx) {
      OS << getSrcAsString();
      OS << getSuccAsString(SuccIdx) << "\n";
    }
  }

  void dumpJSON(json::OStream &JOS) const {

    static unsigned long BRIterator = 0;
    static std::thread::id tid = std::this_thread::get_id();

    std::stringstream FeatureName;
    FeatureName << "BR_" << tid << "_" << BRIterator;
    ++BRIterator;

    JOS.attributeBegin(FeatureName.str());
    JOS.objectBegin();
    {
      {
        JOS.attributeBegin("SrcBBFeatures");
        JOS.objectBegin();

        // We can dump everything in JSON, parser will later take only necessary
        // features
#define BR_SRC_BB_FEATURE(Type, Name)                                          \
  JOS.attribute(#Name, getSrcBBFeatures().Name);
#include "llvm/Transforms/Instrumentation/Intel_MLPGO/FeatureDescAll.def"
#undef BR_SRC_BB_FEATURE

        JOS.objectEnd();
        JOS.attributeEnd();
      }

      for (size_t SuccIdx = 0; SuccIdx < getNumOfSucc(); ++SuccIdx) {

        std::stringstream SuccName;
        SuccName << "SuccBBFeatures_" << SuccIdx;

        JOS.attributeBegin(SuccName.str());
        JOS.objectBegin();
#define BR_SUCC_BB_FEATURE(Type, Name)                                         \
  JOS.attribute(#Name, getSuccFeatures(SuccIdx).Name);
#include "llvm/Transforms/Instrumentation/Intel_MLPGO/FeatureDescAll.def"
#undef BR_SUCC_BB_FEATURE

        JOS.objectEnd();
        JOS.attributeEnd();
      }
    }
    JOS.objectEnd();
    JOS.attributeEnd();
  }

  size_t getNumOfSucc() const { return NumOfSucc; }

  unsigned char *getRawFeatureVec() { return FeaturesBlob.data(); }
};

} // namespace mlpgo
} // namespace llvm

#endif //__LLVM_TRANSFORMS_INSTRUMENTATION_INTEL_MLPGO_FEATURE_DESC_H__
