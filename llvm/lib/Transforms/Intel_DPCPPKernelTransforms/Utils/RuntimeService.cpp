//===- RuntimeService.cpp - Runtime service ------------------------*- C++-===//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/RuntimeService.h"
#include "NameMangleAPI.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"

using namespace llvm;
using namespace DPCPPKernelCompilationUtils;
using namespace NameMangleAPI;
using namespace reflection;

Function *
RuntimeService::findFunctionInBuiltinModules(StringRef FuncName) const {
  for (Module *M : BuiltinModules) {
    Function *RetFunction = M->getFunction(FuncName);
    if (RetFunction)
      return RetFunction;
  }
  return nullptr;
}

bool RuntimeService::hasNoSideEffect(StringRef FuncName) const {
  // Work item builtins and llvm intrinsics are not in runtime module so check
  // them first.
  if (isWorkItemBuiltin(FuncName))
    return true;
  if (isSafeLLVMIntrinsic(FuncName))
    return true;

  // FIXME remove volcano specific code in the following block.
  {
    if (FuncName.contains("fake.extract.element") ||
        FuncName.contains("fake.insert.element"))
      return true;
    auto isRetByVectorBuiltin = [](StringRef Name) {
      reflection::FunctionDescriptor FD = demangle(Name);
      if (FD.isNull())
        return false;
      return FD.Name.find("__retbyvector_") != std::string::npos;
    };
    if (isRetByVectorBuiltin(FuncName))
      return true;
  }

  // If it is not a built-in, don't know if it has side effect.
  Function *FuncRT = findFunctionInBuiltinModules(FuncName);
  if (!FuncRT)
    return false;

  // Special case builtins that access memory but has no side effects.
  if (isSyncWithNoSideEffect(FuncName))
    return true;
  if (isImageDescBuiltin(FuncName))
    return true;

  // Respect horizontal builtin here, treat them as having a side effect.
  // So far these are only VPlan style masked functions.
  if (needsVPlanStyleMask(FuncName))
    return false;

  // All builtins that don't access memory and don't throw have no side effects.
  if (FuncRT->doesNotAccessMemory() && FuncRT->doesNotThrow())
    return true;

  // OpenCL 2.0 ndrange_1D/ndrange_2D/ndrange_3D builtins have a sret argument,
  // so doesNotAccessMemory() returns false.
  auto IsNdrangeNdBuiltin = [](StringRef S) {
    return S.startswith("_Z10ndrange_");
  };
  return IsNdrangeNdBuiltin(FuncName);
}

bool RuntimeService::isAtomicBuiltin(StringRef FuncName) const {
  if (!findFunctionInBuiltinModules(FuncName))
    return false;
  return DPCPPKernelCompilationUtils::isAtomicBuiltin(FuncName);
}

std::tuple<bool, bool, unsigned>
RuntimeService::isTIDGenerator(const CallInst *CI) const {
  if (!CI || !CI->getCalledFunction())
    return {false, false, 0};

  StringRef FName = CI->getCalledFunction()->getName();
  if (!isGetGlobalId(FName) && !isGetLocalId(FName) &&
      !isGetSubGroupLocalId(FName))
    return {false, false, 0}; // not a get_***_id function.

  // Early exit for subgroup TIDs that do not take any operands.
  // Dummy Dim 0 as subgroup does not have a clear dimension.
  if (isGetSubGroupLocalId(FName))
    return {true, false, 0};

  // Go on checking the first argument for other TIDS.
  Value *Op = CI->getArgOperand(0);

  // Check if the argument is constant - if not, we cannot determine if
  // the call will generate different IDs per different vectorization lanes.
  if (!isa<ConstantInt>(Op))
    return {false, true, 0};

  // Report the dimension of the request.
  auto Dim =
      static_cast<unsigned>(cast<ConstantInt>(Op)->getValue().getZExtValue());

  // This is indeed a TID generator.
  return {true, false, Dim};
}

bool RuntimeService::isImageDescBuiltin(StringRef FuncName) const {
  return StringSwitch<bool>(FuncName)
      .Case("_Z16get_image_height", true)
      .Case("_Z15get_image_width", true)
      .Case("_Z15get_image_depth", true)
      .Case("_Z27get_image_channel", true)
      .Case("_Z13get_image_dim_", true)
      .Default(false);
}

bool RuntimeService::isSafeLLVMIntrinsic(StringRef FuncName) const {
  return StringSwitch<bool>(FuncName)
      .Case("llvm.var.annotation", true)
      .Case("llvm.dbg.declare", true)
      .Case("llvm.dbg.value", true)
      .Case("llvm.dbg.label", true)
      .Case("llvm.dbg.address", true)
      .Case("llvm.assume", true)
      .Default(false);
}

bool RuntimeService::isScalarMinMaxBuiltin(StringRef FuncName, bool &IsMin,
                                           bool &IsSigned) const {
  // FuncName need to be mangled min or max.
  if (!isMangledName(FuncName))
    return false;
  StringRef StrippedName = stripName(FuncName);
  IsMin = StrippedName.equals("min");
  if (!IsMin && !StrippedName.equals("max"))
    return false;

  // Now that we know that this is min or max, demangle the builtin.
  FunctionDescriptor Desc = demangle(FuncName);
  assert(Desc.Parameters.size() == 2 && "min/max should have two parameters");
  // The argument type should be (u)int/(u)long
  RefParamType ArgTy = Desc.Parameters[0];
  const auto *PTy = reflection::dyn_cast<PrimitiveType>(ArgTy.get());
  if (!PTy)
    return false;
  TypePrimitiveEnum BasicTy = PTy->getPrimitive();
  IsSigned = (BasicTy == PRIMITIVE_INT || BasicTy == PRIMITIVE_LONG);
  if (!IsSigned && BasicTy != PRIMITIVE_UINT && BasicTy != PRIMITIVE_ULONG)
    return false;

  return true;
}

bool RuntimeService::isSyncWithNoSideEffect(StringRef FuncName) const {
  if (isWorkGroupBarrier(FuncName) || isSubGroupBarrier(FuncName))
    return true;

  if (isWaitGroupEvents(FuncName))
    return true;

  // builtin functions are always mangled.
  if (!isMangledName(FuncName))
    return false;

  StringRef Stripped = stripName(FuncName);
  return StringSwitch<bool>(Stripped)
      .Case("mem_fence", true)
      .Case("read_mem_fence", true)
      .Case("write_mem_fence", true)
      .Default(false);
}

bool RuntimeService::isWorkItemBuiltin(StringRef FuncName) const {
  return isGetGlobalId(FuncName) || isGetLocalId(FuncName) ||
         isGetLocalSize(FuncName) || isGetGlobalSize(FuncName) ||
         isGetGroupId(FuncName) || isGetWorkDim(FuncName) ||
         isGlobalOffset(FuncName) || isGetNumGroups(FuncName) ||
         FuncName == nameGetBaseGID() || isGetSubGroupId(FuncName) ||
         isGetSubGroupLocalId(FuncName) || isGetSubGroupSize(FuncName) ||
         isGetMaxSubGroupSize(FuncName) || isGetNumSubGroups(FuncName) ||
         // The following is applicable for OpenCL 2.0 or more recent versions.
         isGetEnqueuedLocalSize(FuncName) ||
         isGetEnqueuedNumSubGroups(FuncName);
}

bool RuntimeService::needsVPlanStyleMask(StringRef FuncName) const {
  return FuncName.contains("intel_sub_group_ballot") ||
         FuncName.contains("sub_group_all") ||
         FuncName.contains("sub_group_any") ||
         FuncName.contains("sub_group_broadcast") ||
         FuncName.contains("sub_group_reduce_add") ||
         FuncName.contains("sub_group_reduce_min") ||
         FuncName.contains("sub_group_reduce_max") ||
         FuncName.contains("sub_group_scan_exclusive_add") ||
         FuncName.contains("sub_group_scan_exclusive_min") ||
         FuncName.contains("sub_group_scan_exclusive_max") ||
         FuncName.contains("sub_group_scan_inclusive_add") ||
         FuncName.contains("sub_group_scan_inclusive_min") ||
         FuncName.contains("sub_group_scan_inclusive_max") ||
         FuncName.contains("intel_sub_group_shuffle_up") ||
         FuncName.contains("intel_sub_group_shuffle_down") ||
         FuncName.contains("intel_sub_group_shuffle_xor") ||
         FuncName.contains("intel_sub_group_shuffle_xor") ||
         FuncName.contains("intel_sub_group_shuffle") ||
         FuncName.contains("intel_sub_group_block_read") ||
         FuncName.contains("intel_sub_group_block_write");
}

bool RuntimeService::isSafeToSpeculativeExecute(StringRef FuncName) {
  // Work item builtins are not in runtime module so check them first.
  if (isWorkItemBuiltin(FuncName))
    return true;

  // Can not say anything on non-builtin function.
  Function *F = findFunctionInBuiltinModules(FuncName);
  if (!F)
    return false;

  // Special case built-ins that access memory but can be speculatively
  // executed.
  if (isImageDescBuiltin(FuncName))
    return true;

  // All built-ins that does not access memory and does not throw
  // can be speculatively executed.
  return F->doesNotAccessMemory() && F->doesNotThrow();
}
