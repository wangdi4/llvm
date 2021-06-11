//===-- DPCPPKernelCompilationUtils.h - Function declarations -*- C++ -----===//
//
// Copyright (C) 2020-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_COMPILATION_UTILS_H
#define LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_COMPILATION_UTILS_H

#include "KernelArgType.h"
#include "Utils/MetadataAPI.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"

namespace llvm {

namespace KernelAttribute {
// Attributes
extern const char *SyclKernel;
extern const char *NoBarrierPath;
extern const char *KernelWrapper;

extern const char *BarrierBufferSize;
extern const char *LocalBufferSize;
extern const char *PrivateMemorySize;

extern const char *ScalarKernel;
extern const char *VectorizedKernel;
extern const char *VectorizedMaskedKernel;
extern const char *VectorizedWidth;
extern const char *RecommendedVL;
extern const char *VectorVariants;

extern const char *BlockLiteralSize;

inline StringRef getAttributeAsString(const Function &F, StringRef Attr) {
  assert(F.hasFnAttribute(Attr) && "Function doesn't have this attribute!");
  return F.getFnAttribute(Attr).getValueAsString();
}

inline bool getAttributeAsBool(const Function &F, StringRef Attr) {
  assert(F.hasFnAttribute(Attr) && "Function doesn't have this attribute!");
  StringRef AttrStr = getAttributeAsString(F, Attr);
  assert((AttrStr == "true" || AttrStr == "false") &&
         "This attribute must be set as true or false!");
  return AttrStr == "true";
}
inline bool getAttributeAsBool(const Function &F, StringRef Attr,
                               bool Default) {
  return F.hasFnAttribute(Attr) ? getAttributeAsBool(F, Attr) : Default;
}

inline int getAttributeAsInt(const Function &F, StringRef Attr) {
  assert(F.hasFnAttribute(Attr) && "Function doesn't have this attribute!");
  int Res = 0;
  bool Status = to_integer(getAttributeAsString(F, Attr), Res);
  (void)Status;
  assert(Status && "This attribute must have a numeric value!");
  return Res;
}
inline int getAttributeAsInt(const Function &F, StringRef Attr, int Default) {
  return F.hasFnAttribute(Attr) ? getAttributeAsInt(F, Attr) : Default;
}
} // namespace KernelAttribute

namespace DPCPPKernelCompilationUtils {

enum AddressSpace {
  ADDRESS_SPACE_PRIVATE = 0,
  ADDRESS_SPACE_GLOBAL = 1,
  ADDRESS_SPACE_CONSTANT = 2,
  ADDRESS_SPACE_LOCAL = 3,
  ADDRESS_SPACE_LAST_STATIC = ADDRESS_SPACE_LOCAL,
  ADDRESS_SPACE_GENERIC = 4
};

enum BarrierType : char { BARRIER_NO_SCOPE, BARRIER_WITH_SCOPE };

namespace OclVersion {
enum {
  CL_VER_1_0 = 100,
  CL_VER_1_1 = 110,
  CL_VER_1_2 = 120,
  CL_VER_2_0 = 200,
  CL_VER_DEFAULT = CL_VER_1_2
};
} // namespace OclVersion

/// We use a SetVector to ensure determinstic iterations.
using FuncSet = SetVector<Function *>;

/// Return true if string is __enqueue_kernel_*.
bool isEnqueueKernel(StringRef S);
bool isEnqueueKernelLocalMem(StringRef S);
bool isEnqueueKernelEventsLocalMem(StringRef S);

/// generatedFromOCLCPP - check that IR was generated from OCL C++
/// from "!spirv.Source" named metadata
bool isGeneratedFromOCLCPP(const Module &M);

/// Return true if string is plain or mangled get_enqueued_local_size.
bool isGetEnqueuedLocalSize(StringRef S);

/// Return true if string is plain or mangled get_global_linear_id.
bool isGetGlobalLinearId(StringRef S);

/// Return true if string is plain or mangled get_local_linear_id.
bool isGetLocalLinearId(StringRef S);

/// Return true if string is plain or mangled get_global_size.
bool isGetGlobalSize(StringRef S);

/// Return true if string is plain or mangled get_group_id.
bool isGetGroupId(StringRef S);

/// Return true if string is plain or mangled get_local_size.
bool isGetLocalSize(StringRef S);

/// Return true if string is plain or mangled get_num_groups.
bool isGetNumGroups(StringRef S);

/// Return true if string is plain or mangled get_work_dim.
bool isGetWorkDim(StringRef S);

/// Return true if the function is global constructor or destructor (listed in
/// @llvm.global_ctors variable). NOTE: current implementation is *the only*
/// workaround for global ctor/dtor for pipes. See TODO inside the
/// implementation.
bool isGlobalCtorDtor(Function *F);

/// Return true if the function is global constructor or destructor or it's
/// from standard C++.
bool isGlobalCtorDtorOrCPPFunc(Function *F);

/// Return true if string is plain or mangled get_global_offset.
bool isGlobalOffset(StringRef S);

/// Return true if string is mangled prefetch.
bool isPrefetch(StringRef S);

/// Return "get_base_global_id."
StringRef nameGetBaseGID();

/// Return true if string is "get_special_buffer."
bool isGetSpecialBuffer(StringRef S);

/// Return true if string is printf.
bool isPrintf(StringRef S);

/// Returns the mangled name of the function get_global_id.
std::string mangledGetGID();

/// Return the mangled name of the function get_global_size.
std::string mangledGetGlobalSize();

/// Return the mangled name of the function get_global_offset.
std::string mangledGetGlobalOffset();

/// Return the mangled name of the function get_local_id.
std::string mangledGetLID();

/// Returns the mangled name of the function get_local_size.
std::string mangledGetLocalSize();

/// Returns the mangled name of the function barrier.
std::string mangledBarrier();

/// Returns the mangled name of the function work_group_barrier.
std::string mangledWGBarrier(BarrierType BT);

/// Collect all kernel functions.
inline auto getKernels(Module &M) {
  return DPCPPKernelMetadataAPI::KernelList(M);
}


/// Collect all kernel functions including vectorized and vectorized masked
/// kernel.
/// \param M the module to search kernel function inside.
/// \returns FuncSet containing all kernel functions.
FuncSet getAllKernels(Module &M);

/// Get function attribute which is a function.
Function *getFnAttributeFunction(Module &M, Function &F, StringRef AttrKind);

// TODO: Preserved for OCL backend, will be removed later.
/// Get function attribute which is an integer.
template <typename T>
void getFnAttributeInt(Function *F, StringRef AttrKind, T &Value) {
  if (!F->hasFnAttribute(AttrKind))
    return;
  bool Res = to_integer(F->getFnAttribute(AttrKind).getValueAsString(), Value);
  (void)Res;
  assert(Res && "Failed to get integer attribute");
}

/// Get a string from function attribute which is a list of string.
StringRef getFnAttributeStringInList(Function &F, StringRef AttrKind,
                                     unsigned Idx);

/// Add more arguments of a function.
Function *AddMoreArgsToFunc(Function *F, ArrayRef<Type *> NewTypes,
                            ArrayRef<const char *> NewNames,
                            ArrayRef<AttributeSet> NewAttrs, StringRef Prefix);

/// Replaces a CallInst with a new CallInst which has the same arguments as
/// orignal plus more args appeneded.
/// \param OldC the original CallInst to be replaced.
/// \param NewArgs New arguments to append to existing arguments.
/// \param NewF a suitable prototype of new Function to be called.
/// \returns the new CallInst.
CallInst *AddMoreArgsToCall(CallInst *OldC, ArrayRef<Value *> NewArgs,
                            Function *NewF);

/// Replaces an indirect CallInst with a new indirect CallInst which has the
/// same arguments as orignal plus more args appeneded.
/// \param OldC the original CallInst to be replaced.
/// \param NewArgs New arguments to append to existing.
/// arguments. \returns the new CallInst.
CallInst *addMoreArgsToIndirectCall(CallInst *OldC, ArrayRef<Value *> NewArgs);

/// Obtain CL version from "!opencl.ocl.version" named metadata.
unsigned fetchCLVersionFromMetadata(const Module &M);

void getAllSyncBuiltinsDecls(FuncSet &Set, Module *M);

/// Retrieves the pointer to the implicit arguments added to the given function
/// \param F The function for which implicit arguments need to be retrieved.
/// \param LocalMem The LocalMem argument, nullptr if this argument shouldn't be
/// retrieved.
/// \param WorkDim The WorkDim argument, nullptr if this argument shouldn't be
/// retrieved.
/// \param WGId The WGId argument, nullptr if this argument shouldn't be
/// retrieved.
/// \param BaseGlbId The pBaseGlbId argument, nullptr if this argument shouldn't
/// be retrieved.
/// \param SpecialBuf The SpecialBuf argument, nullptr if this argument
/// shouldn't be retrieved.
/// \param RunTimeHandle The RunTimeHandle argument.
void getImplicitArgs(Function *F, Value **LocalMem, Value **WorkDim,
                     Value **WGId, Value **BaseGlbId, Value **SpecialBuf,
                     Value **RunTimeHandle);

/// Retrieves requested TLS global variable in the given module
/// \param M The module for which global variable needs to be retrieved.
/// \param Idx The ImplicitArgsUtils::ImplicitArg index of the requested global.
GlobalVariable *getTLSGlobal(Module *M, unsigned Idx);

/// This function can only be used when ToBB is Entry block.
void moveAllocaToEntry(BasicBlock *FromBB, BasicBlock *EntryBB);

/// Fills a vector of KernelArgument with arguments representing F's SYCL/OpenCL
/// level arguments.
/// \param M The module.
/// \param F The kernel for which to create argument vector.
/// \param Arguments Output KernelArgument vector.
/// \param MemoryArguments Output memory argument indices.
void parseKernelArguments(Module *M, Function *F, bool UseTLSGlobals,
                          std::vector<KernelArgument> &Arguments,
                          std::vector<unsigned int> &MemoryArguments);

/// Return a type name without .N suffix (if any).
StringRef stripStructNameTrailingDigits(StringRef TyName);

/// Update references to old functions in metadata with new ones.
void updateFunctionMetadata(Module *M,
                            DenseMap<Function *, Function *> &FunctionMap);

/// Recursively update metadata nodes with new functions
void updateMetadataTreeWithNewFuncs(
    Module *M, DenseMap<Function *, Function *> &FunctionMap,
    MDNode *MDTreeNode, SmallSet<MDNode *, 8> &Visited);

} // namespace DPCPPKernelCompilationUtils
} // namespace llvm

#endif
