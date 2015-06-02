//===-------- VPOUtils.h - Utility Functions Used for VPO -------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This header file defines the VPOUtils class and its member functions used
// for VPO passes.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORM_VPO_UTILS_VPOUTILS_H
#define LLVM_TRANSFORM_VPO_UTILS_VPOUTILS_H

namespace llvm {

class Function;
class LoopInfo;
class DominatorTree;
class StringRef;
class CallInst;

namespace vpo {

/// \brief This class contains a set of utility functions used by VPO passes.
class VPOUtils {

public:
    /// Constructor and destructor
    VPOUtils() {}
    ~VPOUtils() {}

    /// \brief This function restructures the CFG on demand, where each
    /// directive for Cilk, OpenMP, Offload, Vectorization is put into a
    /// standalone basic block. This is a pre-required process for WRegion
    /// construction for each function. 
    ///
    /// Note that, since WRegion construction requires
    /// DominatorTreeWrapperPass and LoopInfoWrapperPass to be executed prior
    /// to it, when calling CFGRestructuring, we need to update DominatorTree
    /// and LoopInfo whenever a basic block splitting happens.
    static void CFGRestructuring(Function &F, DominatorTree *DT = nullptr,
                     LoopInfo *LI = nullptr);

    /// \brief Return the string representation of the metadata argument used
    /// within a call to the llvm.intel.directive intrinsic.
    static StringRef getDirectiveMetadataString(CallInst *Call);
};

} // End vpo namespace

} // End llvm namespace
#endif
