#if INTEL_FEATURE_SW_ADVANCED
//===------------------ Intel_LangRules.cpp -------------------------------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// Predicates based on language rules.
//
//===----------------------------------------------------------------------===//
//
// An option that indicates that a pointer to a struct could access
// somewhere beyond the boundaries of that struct:
//
// For example:
//
// %struct.A = type { i32, i32 }
// %struct.B = type { i16, i16, i16, i16 }
// %struct.C = type { %struct.A, %struct.B }
//
// define void @foo(%struct.A* nocapture) local_unnamed_addr #0 {
//   %2 = getelementptr inbounds %struct.A, %struct.A* %0, i64 1, i32 1
//   store i32 -1, i32* %2, align 4, !tbaa !2
//   ret void
// }
//
// define void @bar(%struct.C* nocapture) local_unnamed_addr #0 {
//   %2 = getelementptr inbounds %struct.C, %struct.C* %0, i64 0, i32 0
//   tail call void @foo(%struct.A* %2)
//   ret void
// }
//
// Here the getelementptr in @foo is accessing beyond the end of the inner
// %struct.A within %struct.C.
//
// This functionality has been moved here so it can be used outside the
// DTrans directories. (We will change the name of the -mllvm option when
// the appropriate change is made in the driver.)
//
// With respect to dtransanalysis, having -dtrans-outofboundsok=true will
// cause safety checks to be propagated from outer structs to inner structs.
// So, in the above example, if -dtrans-outofboundsok=false, 'Field address
// taken' will be true only for %structC. But if -dtrans-outofboundsok=true,
// it will also be true for %struct.A and %struct.B.
//

#include "llvm/Support/CommandLine.h"

using namespace llvm;

cl::opt<bool> LangRuleOutOfBoundsOK("dtrans-outofboundsok",
                                    cl::init(true), cl::ReallyHidden);

namespace llvm {

bool getLangRuleOutOfBoundsOK(void) {
  return LangRuleOutOfBoundsOK;
}
} // end namespace llvm
#endif // INTEL_FEATURE_SW_ADVANCED
