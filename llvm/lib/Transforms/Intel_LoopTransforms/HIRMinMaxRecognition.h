//===- HIRMinMaxRecognition.h - Declaration of HIRMinMaxRecognition Pass -===//
//
// Copyright (C) 2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===--------------------------------------------------------------------===//
//
// Note:
// This file contains HIRMinMaxRecognition pass's class declaration.
// This allows a complete HIRMinMaxRecognition class declaration available to the
// HIRLoopTransformUtils.cpp file,so that it can instantiate a complete
// HIRMinMaxRecognition object to use inside the independent HIR Loop Transform
// Utility.
//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_MINMAXRECOGNITIONIMPL_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_MINMAXRECOGNITIONIMPL_H

#include "llvm/Transforms/Intel_LoopTransforms/HIRTransformPass.h"

namespace llvm {
namespace loopopt {

struct DirectionVector;
class HIRFramework;

struct MinMaxCandidate {
  HLInst *Store = nullptr;
  HLInst *Ext = nullptr;
  HLIf *If = nullptr;
  HLInst *Trunc = nullptr;
  RegDDRef *MinMaxOp1 = nullptr;
  RegDDRef *MinMaxOp2 = nullptr;
  bool IsMin = false;
  bool IsSigned = false;
  bool IsFloat = false;

  MinMaxCandidate() {}

  MinMaxCandidate(RegDDRef *MMLHS, HLInst *St, HLInst *E, HLIf *I, HLInst *T,
                  RegDDRef *MMOp1, RegDDRef *MMOp2, bool IsMinFlag,
                  bool IsSignedFlag, bool IsFloatFlag)
      : Store(St), Ext(E), If(I), Trunc(T), MinMaxOp1(MMOp1), MinMaxOp2(MMOp2),
        IsMin(IsMinFlag), IsSigned(IsSignedFlag), IsFloat(IsFloatFlag) {}

  void dump() {
    formatted_raw_ostream OS(dbgs());
    OS << "\n MinMax candidate:\n";
    if (Ext) {
      OS << "Ext: ";
      Ext->dump();
    }

    Store->getLvalDDRef()->dump();
    OS << " = ";
    if (IsFloat) {
      OS << "f";
    } else {
      OS << (IsSigned ? "s" : "u");
    }
    OS << (IsMin ? "min (" : "max (");
    MinMaxOp1->dump();
    OS << ", ";
    MinMaxOp2->dump();
    OS << ")\n";

    if (Trunc) {
      OS << "Trunc: ";
      Trunc->dump();
    }
  }
};

class HIRMinMaxRecognition {
  HIRFramework &HIRF;

public:
  HIRMinMaxRecognition(HIRFramework &HIRF) : HIRF(HIRF) {}

  bool run();

  bool isMinOrMaxPattern(HLLoop *Loop, MinMaxCandidate &MMCand);
  void doMinMaxTransformation(HLLoop *Loop, MinMaxCandidate &MMCand);
};
} // namespace loopopt
} // namespace llvm

#endif
