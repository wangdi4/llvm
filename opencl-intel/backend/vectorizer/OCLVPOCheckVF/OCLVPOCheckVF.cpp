// INTEL CONFIDENTIAL
//
// Copyright 2020 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
/// \file
///
/// This Pass check all vectorization factor related issues for OCL side
/// and set proper VF.
/// The flow is:
/// 1. Check whether there is more than one VF constraint. Check fails
///    if more than one of CL_CONFIG_CPU_VECTORIZER_MODE, intel_vec_len_hint
///    and intel_reqd_sub_group_size is defined.
/// 2. Set initial VF according to given constraints. Initial means this VF
///    may be fallbacked. Currently just let intel_vec_len_hint can be
///    fallbacked.
/// 3. Check unsupported patterns in the kernel.
///    currently including: variable TID access
///                         barrier or TID calls in non-inlined functions
///                         unsupported vec_type_hint
///    Some other checks may also need to be added the list.
///    If there patterns are found, the kernel can't be vectorized and VF
///    can't be fallbacked.
/// 4. Check SubGroup semantics.
///    Currently, subgroup need to run as a vectorized kernel, so if
///    kernel can't be vectoried or the subgroup call is in a non-inlined
///    function, we will get incorrect result, i.e subgroup semantics is broken.
/// 5. Check horizontal builtins.
///    If there is a sub_group/work_group call in the kernel, the limitation for
///    VF should be more strict than kernels without these calls. For example,
///    we can't set VF to 32 on AVX512, otherwise, we will get a vectorized
///    kernel with 32 scalar sub_group calls. If there isn't any this kind of
///    calls, we can relax sub_group size to gain better performance.
/// 6. Set final VF.

#include "OCLVPOCheckVF.h"

#include "CompilationUtils.h"
#include "LoopUtils.h"
#include "MetadataAPI.h"

#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"

using namespace Intel::MetadataAPI;

extern bool EnableSubGroupEmulation;

#define DEBUG_TYPE "check-vf"
namespace intel {

char OCLVPOCheckVF::ID = 0;

bool OCLVPOCheckVF::checkVFConstraints(Function *F) {
  auto KMD = KernelMetadataAPI(F);
  // Only allow specifying one of CL_CONFIG_CPU_VECTORIZER_MODE,
  // intel_vec_len_hint and intel_reqd_sub_group_size.
  bool MultiConstraint =
      (KMD.VecLenHint.hasValue() && KMD.ReqdIntelSGSize.hasValue()) ||
      ((TransposeSize != TRANSPOSE_SIZE_NOT_SET &&
        TransposeSize != TRANSPOSE_SIZE_AUTO) &&
       KMD.hasVecLength());

  // No need to do the vectorization in the case of multi-constraints.
  // We will throw compfail after optimizer.
  if (MultiConstraint)
    return false;
  return true;
}

void OCLVPOCheckVF::applyVFConstraints(Function *F) {
  LLVM_DEBUG(dbgs() << "Checking VF Constraints:\n");
  auto KMD = KernelMetadataAPI(F);
  auto KIMD = KernelInternalMetadataAPI(F);

  // In O0 case, InstCounter not run.
  if (!KIMD.OclRecommendedVectorLength.hasValue()) {
    KernelToVF[F] = 1;
    LLVM_DEBUG(dbgs() << "Initial VF<O0 Mode>: " << KernelToVF[F] << "\n");
    return;
  }

  // Allow CL_CONFIG_CPU_VECTORIZER_MODE, intel_vec_len_hint and
  // intel_reqd_sub_group_size overriding recommend vec length.
  if (KMD.hasVecLength() && KMD.getVecLength()) {
    // For intel_vec_len_hint, allow falling back to recommend VF.
    if (KMD.VecLenHint.hasValue()) {
      CanFallBack = true;
    }
    KernelToVF[F] = KMD.getVecLength();
    LLVM_DEBUG(dbgs() << "Initial VF<From VecLength>: " << KernelToVF[F]
                      << "\n");
    return;
  }

  if (TransposeSize != TRANSPOSE_SIZE_NOT_SET &&
      TransposeSize != TRANSPOSE_SIZE_AUTO) {
    KernelToVF[F] = TransposeSize;
    LLVM_DEBUG(dbgs() << "Initial VF<From Env Var>: " << KernelToVF[F] << "\n");
    return;
  }

  KernelToVF[F] = KIMD.OclRecommendedVectorLength.get();
  LLVM_DEBUG(dbgs() << "Initial VF<From InstCounter>: " << KernelToVF[F]
                    << "\n");
}

// In these cases, we can't vectorize the kernel and will set VF to 1.
bool OCLVPOCheckVF::hasUnsupportedPatterns(Function *F) {
  LLVM_DEBUG(dbgs() << "Checking Unsupported Patterns:\n");
  auto KMD = KernelMetadataAPI(F);
  // Check TID call with variable argument.
  // Check special calls in non-inlined functions.
  RuntimeServices *Services =
      getAnalysis<BuiltinLibInfo>().getRuntimeServices();
  if (!CanVectorizeImpl::canVectorizeForVPO(*F, Services)) {
    KernelToVF[F] = 1;
    LLVM_DEBUG(dbgs() << "Can't be vectorized<variable TID call OR special "
                         "non-inlined function>\n");
    CanFallBack = false;
    return true;
  }

  // Check vec_type_hint.
  auto VecTypeHint = KMD.VecTypeHint;
  auto VecLenHint = KMD.VecLenHint;

  if (!VecLenHint.hasValue() && VecTypeHint.hasValue()) {
    Type *VTHTy = VecTypeHint.getType();
    if (!VTHTy->isFloatTy() && !VTHTy->isDoubleTy() && !VTHTy->isIntegerTy(8) &&
        !VTHTy->isIntegerTy(16) && !VTHTy->isIntegerTy(32) &&
        !VTHTy->isIntegerTy(64)) {
      KernelToVF[F] = 1;
      LLVM_DEBUG(dbgs() << "Can't be vectorized<VecTypeHint>\n");
      return true;
    }
  }
  return false;
}

static inline void
collectSubGroupIndirectUsers(Module *M, std::set<Function *> &SGIndirectUsers) {
  std::set<Function *> SGFuncs;
  for (Function &Fn : *M) {
    if (Fn.isDeclaration() && Fn.getName().contains("sub_group"))
      SGFuncs.insert(&Fn);
  }
  std::set<Function *> SGDirectUsers;
  std::set<Function *> SGNewUsers;
  intel::LoopUtils::fillDirectUsers(&SGFuncs, &SGDirectUsers, &SGNewUsers);
  intel::LoopUtils::fillFuncUsersSet(SGDirectUsers, SGIndirectUsers);
}

bool OCLVPOCheckVF::checkSGSemantics(
    Function *F, const std::set<Function *> &SGIndirectUsers) {
  LLVM_DEBUG(dbgs() << "Checking SubGroup Semantics:\n");
  auto KIMD = KernelInternalMetadataAPI(F);
  bool IsBroken = false;
  if (KIMD.KernelHasSubgroups.hasValue() && KIMD.KernelHasSubgroups.get()) {
    // No need to check whether VF can be falled back, checkHorizontalOps did
    // this.
    if (KernelToVF[F] == 1) {
      LLVM_DEBUG(dbgs() << "sub-group is broken<VF is 1> \n");
      return false;
    }
    // Check whether there is subgroup call is in a subroutine.
    IsBroken |= SGIndirectUsers.count(F);
    if (IsBroken) {
      LLVM_DEBUG(dbgs() << "sub-group is broken<In a subroutine> \n");
      return false;
    }
  }
  return true;
}

// TODO: update this function after larger VF work/subgroup builtins are
// implemented and/or subgroup emulation is done. Note subgroup calls and
// workgroup calls in subroutine have been checked before.
std::vector<std::pair<std::string, unsigned>>
OCLVPOCheckVF::checkHorizontalOps(Function *F) {
  LLVM_DEBUG(dbgs() << "Checking Horizontal Operations:\n");
  auto KIMD = KernelInternalMetadataAPI(F);
  unsigned &VF = KernelToVF[F];

  static std::set<unsigned> SupportedWorkGroupVFs = {1, 4, 8, 16};
  static std::set<unsigned> SupportedSubGroupVFs = {4, 8, 16, 32, 64};

  if (EnableSubGroupEmulation)
    SupportedSubGroupVFs.insert((unsigned)1);

  std::vector<std::pair<std::string, unsigned>> UnimplementBuiltins;

  for (Instruction &I : instructions(F)) {
    if (CallInst *CI = dyn_cast<CallInst>(&I)) {
      Function *CF = CI->getCalledFunction();
      if (!(CF && CF->isDeclaration()))
        continue;
      StringRef FnName = CF->getName();
      // Check subgroup calls.
      if (FnName.contains("sub_group") && !FnName.contains("barrier") &&
          SupportedSubGroupVFs.count(VF) == 0) {
        if (CanFallBack && KIMD.OclRecommendedVectorLength.hasValue()) {
          VF = KIMD.OclRecommendedVectorLength.get();
          CheckState[std::string(F->getName())].isVFFalledBack = true;
          LLVM_DEBUG(dbgs() << "VF fall back to " << VF
                            << " due to unsupported sub_group width");
        } else {
          UnimplementBuiltins.push_back({std::string(FnName), VF});
          LLVM_DEBUG(dbgs() << VF << "is unsupported for sub_group");
        }
      }
      // Check workgroup calls.
      if (FnName.contains("work_group") && !FnName.contains("barrier") &&
          SupportedWorkGroupVFs.count(VF) == 0) {
        if (CanFallBack && KIMD.OclRecommendedVectorLength.hasValue()) {
          VF = KIMD.OclRecommendedVectorLength.get();
          LLVM_DEBUG(dbgs() << "VF fall back to " << VF
                            << " due to unsupported work_group width");
          CheckState[std::string(F->getName())].isVFFalledBack = true;
        } else {
          UnimplementBuiltins.push_back({std::string(FnName), VF});
          LLVM_DEBUG(dbgs() << VF << "is unsupported for work_group");
        }
      }
    }
  }
  return UnimplementBuiltins;
}

bool OCLVPOCheckVF::runOnModule(Module &M) {

  bool CheckFail = false;
  CheckState.clear();
  KernelToVF.clear();
  KernelToSGEmuSize.clear();

  unsigned AutoEmuSize = getAutoEmuSize();

  auto Kernels = KernelList(M).getList();

  std::set<Function *> sgIndirectUsers;
  collectSubGroupIndirectUsers(&M, sgIndirectUsers);

  for (Function *kernel : Kernels) {
    auto KMD = KernelMetadataAPI(kernel);
    std::string kernelName = std::string(kernel->getName());
    CanFallBack = false;

    LLVM_DEBUG(dbgs() << "\nProcessing " << kernelName << "\n");

    if (!checkVFConstraints(kernel)) {
      CheckState[kernelName].isMultiConstraint = true;
      CheckFail = true;
    }

    applyVFConstraints(kernel);

    CheckState[kernelName].hasUnsupportedPatterns =
        hasUnsupportedPatterns(kernel);

    auto unimplementOps = checkHorizontalOps(kernel);
    if (!unimplementOps.empty()) {
      CheckState[kernelName].unimplementOps = unimplementOps;
      CheckFail = true;
    }

    if (EnableSubGroupEmulation) {
      KernelToSGEmuSize[kernel] = KernelToVF[kernel];
      LLVM_DEBUG(dbgs() << "Initial SGEmuSize: " << KernelToSGEmuSize[kernel]
                        << "\n");
    }

    if (!checkSGSemantics(kernel, sgIndirectUsers)) {
      CheckFail = true;
      if (!EnableSubGroupEmulation)
        CheckState[kernelName].isSubGroupBroken = true;
    } else {
      // Subgroup semantics is maintained, no need to do emulation.
      if (EnableSubGroupEmulation) {
        KernelToSGEmuSize[kernel] = 0;
        LLVM_DEBUG(dbgs() << "No need to do SG Emulation\n");
      }
    }

    if (KernelToSGEmuSize[kernel] == 1) {
      if (KMD.hasVecLength()) {
        KernelToSGEmuSize[kernel] = KMD.getVecLength();
        LLVM_DEBUG(dbgs() << "Get SG Emulation size from Metadata: "
                          << KernelToSGEmuSize[kernel] << "\n");
      } else {
        KernelToSGEmuSize[kernel] = AutoEmuSize;
        LLVM_DEBUG(dbgs() << "Get auto SG Emulation size: " << AutoEmuSize
                          << "\n");
      }
    }
  }

  // When check failed, no need to run vplan.
  if (CheckFail) {
    for (auto &Item : KernelToVF)
      Item.second = 1;
  }

  bool Changed = false;
  // Update VF to metadata.
  for (auto &Item : KernelToVF) {
    Function *Kernel = Item.first;
    unsigned VF = Item.second;
    // TODO report invalid VF as build fail and dump to build log.
    assert(VF && (VF & (VF - 1)) == 0 && "VF is not power of 2");
    auto KIMD = KernelInternalMetadataAPI(Kernel);
    if (KIMD.OclRecommendedVectorLength.hasValue() &&
        VF != (unsigned)KIMD.OclRecommendedVectorLength.get()) {
      KIMD.OclRecommendedVectorLength.set(VF);
      Changed = true;
    }
    LLVM_DEBUG(dbgs() << Kernel->getName() << " Final VF is: " << VF << "\n");
  }

  for (auto &Item : KernelToSGEmuSize) {
    Function *Kernel = Item.first;
    unsigned SGEmuSize = Item.second;
    if (SGEmuSize == 0)
      continue;
    auto KIMD = KernelInternalMetadataAPI(Kernel);
    assert(!KIMD.SubgroupEmuSize.hasValue() &&
           "Unexpected subgroup_emu_size metadata");
    KIMD.SubgroupEmuSize.set(SGEmuSize);
    // Go thru barrier passes when the kernel will be sg-emulated.
    KIMD.NoBarrierPath.set(false);
    LLVM_DEBUG(dbgs() << Kernel->getName()
                      << " Final SG Emulation size is: " << SGEmuSize << "\n");
    Changed = true;
  }

  return Changed;
}
} // namespace intel

extern "C" Pass *
createOCLVPOCheckVFPass(const intel::OptimizerConfig &Config,
                        Intel::OpenCL::DeviceBackend::TStringToVFState &State) {
  return new intel::OCLVPOCheckVF(Config, State);
}
