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

namespace intel {

char OCLVPOCheckVF::ID = 0;
bool OCLVPOCheckVF::checkVFConstraints(Function *F) {
  auto KMD = KernelMetadataAPI(F);
  // Only allow specifying one of CL_CONFIG_CPU_VECTORIZER_MODE,
  // intel_vec_len_hint and intel_reqd_sub_group_size.
  bool multiConstraint =
      (KMD.VecLenHint.hasValue() && KMD.ReqdIntelSGSize.hasValue()) ||
      ((m_transposeSize != TRANSPOSE_SIZE_NOT_SET &&
        m_transposeSize != TRANSPOSE_SIZE_AUTO) &&
       KMD.hasVecLength());

  // No need to do the vectorization in the case of multi constraints.
  // We will throw compfail after optimizer.
  if (multiConstraint)
    return false;
  return true;
}

void OCLVPOCheckVF::applyVFConstraints(Function *F) {
  auto KMD = KernelMetadataAPI(F);
  auto KIMD = KernelInternalMetadataAPI(F);

  // In O0 case, InstCounter not run.
  if (!KIMD.OclRecommendedVectorLength.hasValue()) {
    m_kernelToVF[F] = 1;
    return;
  }

  // Allow CL_CONFIG_CPU_VECTORIZER_MODE, intel_vec_len_hint and
  // intel_reqd_sub_group_size overriding recommend vec length.
  if (KMD.hasVecLength() && KMD.getVecLength()) {
    // For intel_vec_len_hint, allow falling back to recommend VF.
    if (KMD.VecLenHint.hasValue()) {
      m_canFallBack = true;
    }
    m_kernelToVF[F] = KMD.getVecLength();
    return;
  }

  if (m_transposeSize != TRANSPOSE_SIZE_NOT_SET &&
      m_transposeSize != TRANSPOSE_SIZE_AUTO) {
    m_kernelToVF[F] = m_transposeSize;
    return;
  }
  m_kernelToVF[F] = KIMD.OclRecommendedVectorLength.get();
}

// In these cases, we can't vectorize the kernel and will set VF to 1.
bool OCLVPOCheckVF::hasUnsupportedPatterns(Function *F) {
  auto KMD = KernelMetadataAPI(F);

  // Check TID call with variable argument.
  // Check special calls in non-inlined functions.
  RuntimeServices *services =
      getAnalysis<BuiltinLibInfo>().getRuntimeServices();
  if (!CanVectorizeImpl::canVectorizeForVPO(*F, services)) {
    m_kernelToVF[F] = 1;
    m_canFallBack = false;
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
      m_kernelToVF[F] = 1;
      return true;
    }
  }
  return false;
}

static inline void
collectSubGroupIndirectUsers(Module *pModule,
                             std::set<Function *> &sgIndirectUsers) {
  std::set<Function *> sgFuncs;
  for (Function &Fn : *pModule) {
    if (Fn.isDeclaration() && Fn.getName().contains("sub_group"))
      sgFuncs.insert(&Fn);
  }
  std::set<Function *> sgDirectUsers;
  std::set<Function *> sgNewUsers;
  intel::LoopUtils::fillDirectUsers(&sgFuncs, &sgDirectUsers, &sgNewUsers);
  intel::LoopUtils::fillFuncUsersSet(sgDirectUsers, sgIndirectUsers);
}

bool OCLVPOCheckVF::checkSGSemantics(
    Function *F, const std::set<Function *> &sgIndirectUsers) {
  auto KIMD = KernelInternalMetadataAPI(F);
  bool isBroken = false;
  if (KIMD.KernelHasSubgroups.hasValue() && KIMD.KernelHasSubgroups.get()) {
    // TODO: Update this after subgroup emulation is done.
    if (m_kernelToVF[F] == 1) {
      if (m_canFallBack) {
        m_kernelToVF[F] = KIMD.OclRecommendedVectorLength.get();
        m_checkState[std::string(F->getName())].isVFFalledBack = true;
      } else {
        return false;
      }
    }
  }
  // Check whether there is subgroup call is in a subroutine.
  isBroken |= sgIndirectUsers.count(F);
  return !isBroken;
}

// TODO: update this function after larger VF work/subgroup builtins are
// implemented and/or subgroup emulation is done. Note subgroup calls and
// workgroup calls in subroutine have been checked before.
std::vector<std::pair<std::string, unsigned>>
OCLVPOCheckVF::checkHorizontalOps(Function *F) {
  auto KIMD = KernelInternalMetadataAPI(F);
  unsigned &VF = m_kernelToVF[F];

  std::vector<std::pair<std::string, unsigned>> unimplementBuiltins;

  for (Instruction &inst : instructions(F)) {
    if (CallInst *pCall = dyn_cast<CallInst>(&inst)) {
      Function *pCallee = pCall->getCalledFunction();
      if (!(pCallee && pCallee->isDeclaration()))
        continue;
      StringRef calleeName = pCallee->getName();
      // Check subgroup calls.
      if (calleeName.contains("sub_group") && !calleeName.contains("barrier")) {
        // 1. VF must be supported for the current arch.
        // 2. AVX has only x4 subgroup builtins.
        if (m_cpuId.isTransposeSizeSupported((ETransposeSize)VF) !=
                Intel::SUPPORTED ||
            (!m_cpuId.HasAVX2() && VF == TRANSPOSE_SIZE_8)) {
          if (m_canFallBack) {
            VF = KIMD.OclRecommendedVectorLength.get();
            m_checkState[std::string(F->getName())].isVFFalledBack = true;
          } else {
            unimplementBuiltins.push_back({std::string(calleeName), VF});
          }
        }
      }
      // Check workgroup calls.
      if (calleeName.contains("work_group") &&
          !calleeName.contains("barrier") &&
          (VF != TRANSPOSE_SIZE_1 && VF != TRANSPOSE_SIZE_4 &&
           VF != TRANSPOSE_SIZE_8 && VF != TRANSPOSE_SIZE_16)) {
        if (m_canFallBack) {
          VF = KIMD.OclRecommendedVectorLength.get();
          m_checkState[std::string(F->getName())].isVFFalledBack = true;
        } else {
          unimplementBuiltins.push_back({std::string(calleeName), VF});
        }
      }
    }
  }
  return unimplementBuiltins;
}

bool OCLVPOCheckVF::runOnModule(Module &M) {

  bool checkFailed = false;
  m_checkState.clear();
  m_kernelToVF.clear();
  auto Kernels = KernelList(M).getList();

  std::set<Function *> sgIndirectUsers;
  collectSubGroupIndirectUsers(&M, sgIndirectUsers);

  for (Function *kernel : Kernels) {
    std::string kernelName = std::string(kernel->getName());
    m_canFallBack = false;

    if (!checkVFConstraints(kernel)) {
      m_checkState[kernelName].isMultiConstraint = true;
      checkFailed = true;
    }

    applyVFConstraints(kernel);

    m_checkState[kernelName].hasUnsupportedPatterns =
        hasUnsupportedPatterns(kernel);

    if (!checkSGSemantics(kernel, sgIndirectUsers)) {
      m_checkState[kernelName].isSubGroupBroken = true;
      checkFailed = true;
    }

    auto unimplementOps = checkHorizontalOps(kernel);
    if (!unimplementOps.empty()) {
      m_checkState[kernelName].unimplementOps = unimplementOps;
      checkFailed = true;
    }
  }

  // When check failed, no need to run vplan.
  if (checkFailed) {
    for (auto &item : m_kernelToVF)
      item.second = 1;
  }

  bool changed = false;
  // Update VF to metadata.
  for (auto &item : m_kernelToVF) {
    Function *kernel = item.first;
    auto KIMD = KernelInternalMetadataAPI(kernel);
    if (KIMD.OclRecommendedVectorLength.hasValue() &&
        item.second != (unsigned)KIMD.OclRecommendedVectorLength.get()) {
      KIMD.OclRecommendedVectorLength.set(item.second);
      changed = true;
    }
  }

  return changed;
}
} // namespace intel

extern "C" Pass *
createOCLVPOCheckVFPass(const intel::OptimizerConfig &config,
                        Intel::OpenCL::DeviceBackend::TStringToVFState &state) {
  return new intel::OCLVPOCheckVF(config, state);
}
