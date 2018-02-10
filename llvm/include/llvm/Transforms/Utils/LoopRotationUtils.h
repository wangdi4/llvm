//===- LoopRotationUtils.h ----------------------------------------------*- C++
//-*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file exposes an interface to transform the loop into the do-while loop.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_UTILS_LOOPROTATIONUTIL_H
#define LLVM_TRANSFORMS_UTILS_LOOPROTATIONUTIL_H

namespace llvm {

class AssumptionCache;
class DominatorTree;
class Loop;
class LoopInfo;
class ScalarEvolution;
struct SimplifyQuery;
class TargetTransformInfo;

bool LoopRotation(Loop *L, unsigned MaxHeaderSize, LoopInfo *LI,
                  const TargetTransformInfo *TTI, AssumptionCache *AC,
                  DominatorTree *DT, ScalarEvolution *SE,
                  const SimplifyQuery &SQ);

} // namespace llvm

#endif
