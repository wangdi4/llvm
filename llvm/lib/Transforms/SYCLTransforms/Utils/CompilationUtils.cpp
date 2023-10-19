//===-- CompilationUtils.cpp - Compilation utilities -------------*- C++ *-===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Regex.h"
#include "llvm/Transforms/SYCLTransforms/Utils/ImplicitArgsUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/LoopUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/MetadataAPI.h"
#include "llvm/Transforms/SYCLTransforms/Utils/NameMangleAPI.h"
#include "llvm/Transforms/SYCLTransforms/Utils/ParameterType.h"
#include "llvm/Transforms/SYCLTransforms/Utils/TypeAlignment.h"
#include <regex>

using namespace llvm;
using namespace NameMangleAPI;

namespace llvm {

const StringRef UserVariantPrefix = "user.";

// Attributes
const StringRef KernelAttribute::CallOnce = "kernel-call-once";
const StringRef KernelAttribute::CallParamNum = "call-params-num";
const StringRef KernelAttribute::ConvergentCall = "kernel-convergent-call";
const StringRef KernelAttribute::HasSubGroups = "has-sub-groups";
const StringRef KernelAttribute::HasVPlanMask = "has-vplan-mask";
const StringRef KernelAttribute::OCLVecUniformReturn =
    "opencl-vec-uniform-return";
const StringRef KernelAttribute::UniformCall = "kernel-uniform-call";
const StringRef KernelAttribute::VectorVariantFailure = "vector-variant-failure";

namespace {
// Document what source language this module was translated from.
// Values defined by SPIR-V spec.
namespace OclLanguage {
static const unsigned OpenCL_CPP = 4;
}

const StringRef NAME_GET_GID = "get_global_id";
const StringRef NAME_GET_LID = "get_local_id";
const StringRef NAME_GET_LINEAR_GID = "get_global_linear_id";
const StringRef NAME_GET_LINEAR_LID = "get_local_linear_id";
const StringRef NAME_GET_SUB_GROUP_LOCAL_ID = "get_sub_group_local_id";
const StringRef NAME_GET_GLOBAL_SIZE = "get_global_size";
const StringRef NAME_GET_LOCAL_SIZE = "get_local_size";
const StringRef NAME_GET_SUB_GROUP_SIZE = "get_sub_group_size";
const StringRef NAME_GET_MAX_SUB_GROUP_SIZE = "get_max_sub_group_size";
const StringRef NAME_GET_GROUP_ID = "get_group_id";
const StringRef NAME_GET_SUB_GROUP_ID = "get_sub_group_id";
const StringRef NAME_GET_NUM_GROUPS = "get_num_groups";
const StringRef NAME_GET_NUM_SUB_GROUPS = "get_num_sub_groups";
const StringRef NAME_GET_ENQUEUED_NUM_SUB_GROUPS =
    "get_enqueued_num_sub_groups";
const StringRef NAME_GET_WORK_DIM = "get_work_dim";
const StringRef NAME_GET_GLOBAL_OFFSET = "get_global_offset";
const StringRef NAME_GET_ENQUEUED_LOCAL_SIZE = "get_enqueued_local_size";
const StringRef NAME_BARRIER = "barrier";
const StringRef NAME_WG_BARRIER = "work_group_barrier";
const StringRef NAME_SG_BARRIER = "sub_group_barrier";
const StringRef NAME_TLS_LOCAL_IDS = "__LocalIds";
const StringRef NAME_PREFETCH = "prefetch";
const StringRef NAME_WAIT_GROUP_EVENTS = "wait_group_events";
const StringRef SAMPLER = "sampler_t";

// atomic fence functions
const StringRef NAME_ATOMIC_WORK_ITEM_FENCE = "atomic_work_item_fence";

// work-group functions
const StringRef NAME_WORK_GROUP_ALL = "work_group_all";
const StringRef NAME_WORK_GROUP_ANY = "work_group_any";
const StringRef NAME_WORK_GROUP_BROADCAST = "work_group_broadcast";
const StringRef NAME_WORK_GROUP_IDENTITY = "work_group_identity";
const StringRef NAME_WORK_GROUP_REDUCE_ADD = "work_group_reduce_add";
const StringRef NAME_WORK_GROUP_SCAN_EXCLUSIVE_ADD =
    "work_group_scan_exclusive_add";
const StringRef NAME_WORK_GROUP_SCAN_INCLUSIVE_ADD =
    "work_group_scan_inclusive_add";
const StringRef NAME_WORK_GROUP_REDUCE_MUL = "work_group_reduce_mul";
const StringRef NAME_WORK_GROUP_SCAN_EXCLUSIVE_MUL =
    "work_group_scan_exclusive_mul";
const StringRef NAME_WORK_GROUP_SCAN_INCLUSIVE_MUL =
    "work_group_scan_inclusive_mul";
const StringRef NAME_WORK_GROUP_REDUCE_MIN = "work_group_reduce_min";
const StringRef NAME_WORK_GROUP_SCAN_EXCLUSIVE_MIN =
    "work_group_scan_exclusive_min";
const StringRef NAME_WORK_GROUP_SCAN_INCLUSIVE_MIN =
    "work_group_scan_inclusive_min";
const StringRef NAME_WORK_GROUP_REDUCE_MAX = "work_group_reduce_max";
const StringRef NAME_WORK_GROUP_SCAN_EXCLUSIVE_MAX =
    "work_group_scan_exclusive_max";
const StringRef NAME_WORK_GROUP_SCAN_INCLUSIVE_MAX =
    "work_group_scan_inclusive_max";
const StringRef NAME_WORK_GROUP_SCAN_EXCLUSIVE_BITWISE_AND =
    "work_group_scan_exclusive_bitwise_and";
const StringRef NAME_WORK_GROUP_SCAN_INCLUSIVE_BITWISE_AND =
    "work_group_scan_inclusive_bitwise_and";
const StringRef NAME_WORK_GROUP_SCAN_EXCLUSIVE_BITWISE_OR =
    "work_group_scan_exclusive_bitwise_or";
const StringRef NAME_WORK_GROUP_SCAN_INCLUSIVE_BITWISE_OR =
    "work_group_scan_inclusive_bitwise_or";
const StringRef NAME_WORK_GROUP_SCAN_EXCLUSIVE_BITWISE_XOR =
    "work_group_scan_exclusive_bitwise_xor";
const StringRef NAME_WORK_GROUP_SCAN_INCLUSIVE_BITWISE_XOR =
    "work_group_scan_inclusive_bitwise_xor";
const StringRef NAME_ASYNC_WORK_GROUP_COPY = "async_work_group_copy";
const StringRef NAME_ASYNC_WORK_GROUP_STRIDED_COPY =
    "async_work_group_strided_copy";
const StringRef NAME_WORK_GROUP_RESERVE_READ_PIPE =
    "__work_group_reserve_read_pipe";
const StringRef NAME_WORK_GROUP_COMMIT_READ_PIPE =
    "__work_group_commit_read_pipe";
const StringRef NAME_WORK_GROUP_RESERVE_WRITE_PIPE =
    "__work_group_reserve_write_pipe";
const StringRef NAME_WORK_GROUP_COMMIT_WRITE_PIPE =
    "__work_group_commit_write_pipe";
const StringRef NAME_WORK_GROUP_REDUCE_BITWISE_AND =
    "work_group_reduce_bitwise_and";
const StringRef NAME_WORK_GROUP_REDUCE_BITWISE_OR =
    "work_group_reduce_bitwise_or";
const StringRef NAME_WORK_GROUP_REDUCE_BITWISE_XOR =
    "work_group_reduce_bitwise_xor";
const StringRef NAME_WORK_GROUP_REDUCE_LOGICAL_AND =
    "work_group_reduce_logical_and";
const StringRef NAME_WORK_GROUP_REDUCE_LOGICAL_OR =
    "work_group_reduce_logical_or";
const StringRef NAME_WORK_GROUP_REDUCE_LOGICAL_XOR =
    "work_group_reduce_logical_xor";
const StringRef NAME_WORK_GROUP_JOINT_SORT_ASCEND =
    "__devicelib_default_work_group_joint_sort_ascending_";
const StringRef NAME_WORK_GROUP_JOINT_SORT_DESCEND =
    "__devicelib_default_work_group_joint_sort_descending_";
const StringRef NAME_WORK_GROUP_PRIVATE_CLOSE_SORT_ASCEND =
    "__devicelib_default_work_group_private_sort_close_ascending_";
const StringRef NAME_WORK_GROUP_PRIVATE_CLOSE_SORT_DESCEND =
    "__devicelib_default_work_group_private_sort_close_descending_";
const StringRef NAME_WORK_GROUP_PRIVATE_SPREAD_SORT_ASCEND =
    "__devicelib_default_work_group_private_sort_spread_ascending_";
const StringRef NAME_WORK_GROUP_PRIVATE_SPREAD_SORT_DESCEND =
    "__devicelib_default_work_group_private_sort_spread_descending_";
const StringRef NAME_WORK_GROUP_PRIVATE_SORT_COPY =
    "__devicelib_default_work_group_private_sort_copy";
const StringRef NAME_WORK_GROUP_PRIVATE_SPREAD_SORT_COPY =
    "__devicelib_default_work_group_private_spread_sort_copy";

const StringRef NAME_FINALIZE_WG_FUNCTION_PREFIX = "__finalize_";

// KMP acquire/release
const StringRef NAME_IB_KMP_ACQUIRE_LOCK = "__builtin_IB_kmp_acquire_lock";
const StringRef NAME_IB_KMP_RELEASE_LOCK = "__builtin_IB_kmp_release_lock";

// subgroup functions
const StringRef NAME_SUB_GROUP_ALL = "sub_group_all";
const StringRef NAME_SUB_GROUP_ANY = "sub_group_any";
const StringRef NAME_SUB_GROUP_BROADCAST = "sub_group_broadcast";
const StringRef NAME_SUB_GROUP_REDUCE_ADD = "sub_group_reduce_add";
const StringRef NAME_SUB_GROUP_REDUCE_MIN = "sub_group_reduce_min";
const StringRef NAME_SUB_GROUP_REDUCE_MAX = "sub_group_reduce_max";
const StringRef NAME_SUB_GROUP_SCAN_EXCLUSIVE_ADD =
    "sub_group_scan_exclusive_add";
const StringRef NAME_SUB_GROUP_SCAN_INCLUSIVE_ADD =
    "sub_group_scan_inclusive_add";
const StringRef NAME_SUB_GROUP_SCAN_EXCLUSIVE_MIN =
    "sub_group_scan_exclusive_min";
const StringRef NAME_SUB_GROUP_SCAN_INCLUSIVE_MIN =
    "sub_group_scan_inclusive_min";
const StringRef NAME_SUB_GROUP_SCAN_EXCLUSIVE_MAX =
    "sub_group_scan_exclusive_max";
const StringRef NAME_SUB_GROUP_SCAN_INCLUSIVE_MAX =
    "sub_group_scan_inclusive_max";
const StringRef NAME_SUB_GROUP_SHUFFLE = "sub_group_shuffle";
const StringRef NAME_SUB_GROUP_SHUFFLE_DOWN = "sub_group_shuffle_down";
const StringRef NAME_SUB_GROUP_SHUFFLE_UP = "sub_group_shuffle_up";
const StringRef NAME_SUB_GROUP_SHUFFLE_XOR = "sub_group_shuffle_xor";
const StringRef NAME_SUB_GROUP_BLOCK_READ = "sub_group_block_read";
const StringRef NAME_SUB_GROUP_BLOCK_WRITE = "sub_group_block_write";
const StringRef NAME_SUB_GROUP_SORT_DESCEND =
    "__devicelib_default_sub_group_private_sort_descending_";
const StringRef NAME_SUB_GROUP_SORT_ASCEND =
    "__devicelib_default_sub_group_private_sort_ascending_";

/// Not mangled names.
const StringRef NAME_GET_BASE_GID = "get_base_global_id.";
const StringRef NAME_GET_SPECIAL_BUFFER = "get_special_buffer.";
const StringRef NAME_PRINTF = "printf";
const StringRef NAME_PRINTF_OPENCL = "__opencl_printf";

#if INTEL_CUSTOMIZATION
/// Matrix slicing support.
const StringRef NAME_GET_SUB_GROUP_SLICE_LENGTH = "get_sub_group_slice_length.";
const StringRef NAME_GET_SUB_GROUP_ROWSLICE_ID = "get_sub_group_rowslice_id";
const StringRef NAME_SUB_GROUP_ROWSLICE_EXTRACTELEMENT =
    "sub_group_rowslice_extractelement";
const StringRef NAME_SUB_GROUP_ROWSLICE_INSERTELEMENT =
    "sub_group_rowslice_insertelement";
const StringRef NAME_SUB_GROUP_INSERT_ROWSLICE_TO_MATRIX =
    "sub_group_insert_rowslice_to_matrix";
#endif // INTEL_CUSTOMIZATION
} // namespace

static cl::opt<std::string> OptVectInfoFile("sycl-vect-info", cl::Hidden,
                                            cl::desc("Builtin VectInfo list"),
                                            cl::value_desc("filename"));

namespace CompilationUtils {

static unsigned CLVersionToVal(uint64_t Major, uint64_t Minor) {
  return Major * 100 + Minor * 10;
}

static bool isMangleOf(StringRef LHS, StringRef RHS) {
  if (!isMangledName(LHS))
    return false;
  return stripName(LHS) == RHS;
}

bool isEnqueueKernel(StringRef S) {
  return S == "__enqueue_kernel_basic" ||
         S == "__enqueue_kernel_basic_events" ||
         S == "__enqueue_kernel_varargs" ||
         S == "__enqueue_kernel_events_varargs";
}

bool isEnqueueKernelLocalMem(StringRef S) {
  return S == "__enqueue_kernel_varargs";
}

bool isEnqueueKernelEventsLocalMem(StringRef S) {
  return S == "__enqueue_kernel_events_varargs";
}

bool isGeneratedFromOCLCPP(const Module &M) {
  /*
  Example of the metadata
  !spirv.Source = !{!0}
  !0 = !{i32 4, i32 100000}
  */
  NamedMDNode *Node = M.getNamedMetadata("spirv.Source");
  if (Node && Node->getNumOperands()) {
    auto Op = Node->getOperand(0);
    if (Op->getNumOperands()) {
      uint64_t Val =
          mdconst::extract<ConstantInt>(Op->getOperand(0))->getZExtValue();
      return Val == OclLanguage::OpenCL_CPP;
    }
  }
  return false;
}

// TODO: use product solution for checking IR generated from OpenMP
// offloading, instead of the hack (checking the global variable name).
//
// In most cases, of course, there will be at least one kernel, thus, at least
// one OpenMP offload entries table entry, and, thus,
// __omp_offloading_entries_table_size definition will be generated. But it is
// possible that an OpenMP program just defines some non-kernel functions, and
// OpenMP offload entries table is empty. This may be used for library-like
// OpenMP offload code compilation. On the other hand, we do not support
// runtime linking of such libraries currently.
bool isGeneratedFromOMP(const Module &M) {
  // If IR is generated from OpenMP offloading code, it has spirv source
  // metadata (OpenCL CPP), and a global variable named as
  // __omp_offloading_entries_table.
  if (isGeneratedFromOCLCPP(M) &&
      M.getGlobalVariable("__omp_offloading_entries_table"))
    return true;
  return false;
}

// spirv.Generator metadata is generated by llvm-spirv translator.
bool generatedFromSPIRV(const Module &M) {
  return M.getNamedMetadata("spirv.Generator") != nullptr;
}

bool isImplicitGID(AllocaInst *AI) {
  StringRef Name = AI->getName();
  static const std::vector<StringRef> ImplicitGIDs = {
      "__ocl_dbg_gid0", "__ocl_dbg_gid1", "__ocl_dbg_gid2"};
  for (auto &GID : ImplicitGIDs) {
    if (Name.equals(GID))
      return true;
  }
  return false;
}

std::string AppendWithDimension(const Twine &S, int Dimension) {
  return Dimension >= 0 ? (S + Twine(Dimension)).str() : (S + "var").str();
}

std::string AppendWithDimension(const Twine &S, const Value *Dimension) {
  int D = -1;
  if (const ConstantInt *C = dyn_cast<ConstantInt>(Dimension))
    D = C->getZExtValue();
  return AppendWithDimension(S, D);
}

bool isGetEnqueuedLocalSize(StringRef S) {
  return isMangleOf(S, NAME_GET_ENQUEUED_LOCAL_SIZE);
}

bool isUserVariantOfGetEnqueuedLocalSize(StringRef S) {
  return S.consume_front(UserVariantPrefix) && isGetEnqueuedLocalSize(S);
}

bool isGetGlobalLinearId(StringRef S) {
  return isMangleOf(S, NAME_GET_LINEAR_GID);
}

bool isGetLocalLinearId(StringRef S) {
  return isMangleOf(S, NAME_GET_LINEAR_LID);
}

bool isGetGlobalSize(StringRef S) {
  return isMangleOf(S, NAME_GET_GLOBAL_SIZE);
}

bool isUserVariantOfGetGlobalSize(StringRef S) {
  return S.consume_front(UserVariantPrefix) && isGetGlobalSize(S);
}

bool isGetGroupId(StringRef S) {
  return isMangleOf(S, NAME_GET_GROUP_ID);
}

bool isGetLocalSize(StringRef S) {
  return isMangleOf(S, NAME_GET_LOCAL_SIZE);
}

bool isUserVariantOfGetLocalSize(StringRef S) {
  return S.consume_front(UserVariantPrefix) && isGetLocalSize(S);
}

bool isGetNumGroups(StringRef S) {
  return isMangleOf(S, NAME_GET_NUM_GROUPS);
}

bool isUserVariantOfGetNumGroups(StringRef S) {
  return S.consume_front(UserVariantPrefix) && isGetNumGroups(S);
}

bool isGetWorkDim(StringRef S) {
  return isMangleOf(S, NAME_GET_WORK_DIM);
}

bool isGetLocalId(StringRef S) { return isMangleOf(S, NAME_GET_LID); }

bool isGetGlobalId(StringRef S) { return isMangleOf(S, NAME_GET_GID); }

bool isAtomicBuiltin(StringRef S) {
  // S is atomic built-in name if
  // - it's mangled (only built-in function names are mangled)
  // - it starts with "atom" (only atomic built-ins has "atom" prefix)
  if (!isMangledName(S))
    return false;
  return stripName(S).startswith("atom");
}

bool isAtomicWorkItemFenceBuiltin(StringRef S) {
  // S is atomic built-in name if
  // - it's mangled (only built-in function names are mangled)
  // - it's equal to "atomic_work_item_fence" string
  if (!isMangledName(S))
    return false;
  return stripName(S) == NAME_ATOMIC_WORK_ITEM_FENCE;
}

bool isGlobalCtorDtor(Function *F) {
  // TODO: implement good solution based on value of @llvm.global_ctors variable
  return F->getName() == "__pipe_global_ctor" ||
         F->getName() == "__pipe_global_dtor";
}

bool isGlobalCtorDtorOrCPPFunc(Function *F) {
  assert(F && "Invalid input for global ctor / dtor / cpp func check");
  return isGlobalCtorDtor(F) || F->hasFnAttribute("not-ocl-sycl");
}

bool isGlobalOffset(StringRef S) {
  return isMangleOf(S, NAME_GET_GLOBAL_OFFSET);
}

StringRef nameGetBaseGID() { return NAME_GET_BASE_GID; }

StringRef nameSpecialBuffer() { return NAME_GET_SPECIAL_BUFFER; }

bool isPrefetch(StringRef S) { return isMangleOf(S, NAME_PREFETCH); }

bool isOpenCLPrintf(StringRef S) { return S == NAME_PRINTF_OPENCL; }

bool isPrintf(StringRef S) { return S == NAME_PRINTF; }

StringRef nameOpenCLPrintf() { return NAME_PRINTF_OPENCL; }

StringRef namePrintf() { return NAME_PRINTF; }

// Work-Group builtins
bool isWorkGroupAll(StringRef S) { return isMangleOf(S, NAME_WORK_GROUP_ALL); }

bool isWorkGroupAny(StringRef S) { return isMangleOf(S, NAME_WORK_GROUP_ANY); }

std::pair<bool, bool> isWorkGroupBroadCast(StringRef S) {
  bool IsMaskedBroadcast =
      isMangleOf(S, (Twine("__") + NAME_WORK_GROUP_BROADCAST).str());
  return {IsMaskedBroadcast || isMangleOf(S, NAME_WORK_GROUP_BROADCAST),
          IsMaskedBroadcast};
}

bool isWorkGroupIdentity(StringRef S) {
  return isMangleOf(S, NAME_WORK_GROUP_IDENTITY);
}

bool isWorkGroupReduceAdd(StringRef S) {
  return isMangleOf(S, NAME_WORK_GROUP_REDUCE_ADD);
}

bool isWorkGroupReduceMul(StringRef S) {
  return isMangleOf(S, NAME_WORK_GROUP_REDUCE_MUL);
}

bool isWorkGroupReduceMin(StringRef S) {
  return isMangleOf(S, NAME_WORK_GROUP_REDUCE_MIN);
}

bool isWorkGroupReduceMax(StringRef S) {
  return isMangleOf(S, NAME_WORK_GROUP_REDUCE_MAX);
}

bool isWorkGroupReduceBitwiseAnd(StringRef S){
  return isMangleOf(S, NAME_WORK_GROUP_REDUCE_BITWISE_AND);
}

bool isWorkGroupReduceBitwiseOr(StringRef S){
  return isMangleOf(S, NAME_WORK_GROUP_REDUCE_BITWISE_OR);
}

bool isWorkGroupReduceBitwiseXor(StringRef S){
  return isMangleOf(S, NAME_WORK_GROUP_REDUCE_BITWISE_XOR);
}

bool isWorkGroupReduceLogicalAnd(StringRef S){
  return isMangleOf(S, NAME_WORK_GROUP_REDUCE_LOGICAL_AND);
}

bool isWorkGroupReduceLogicalOr(StringRef S){
  return isMangleOf(S, NAME_WORK_GROUP_REDUCE_LOGICAL_OR);
}

bool isWorkGroupReduceLogicalXor(StringRef S){
  return isMangleOf(S, NAME_WORK_GROUP_REDUCE_LOGICAL_XOR);
}

bool isWorkGroupScanExclusiveAdd(StringRef S) {
  return isMangleOf(S, NAME_WORK_GROUP_SCAN_EXCLUSIVE_ADD);
}

bool isWorkGroupScanInclusiveAdd(StringRef S) {
  return isMangleOf(S, NAME_WORK_GROUP_SCAN_INCLUSIVE_ADD);
}

bool isWorkGroupScanExclusiveMul(StringRef S) {
  return isMangleOf(S, NAME_WORK_GROUP_SCAN_EXCLUSIVE_MUL);
}

bool isWorkGroupScanInclusiveMul(StringRef S) {
  return isMangleOf(S, NAME_WORK_GROUP_SCAN_INCLUSIVE_MUL);
}

bool isWorkGroupScanExclusiveMin(StringRef S) {
  return isMangleOf(S, NAME_WORK_GROUP_SCAN_EXCLUSIVE_MIN);
}

bool isWorkGroupScanInclusiveMin(StringRef S) {
  return isMangleOf(S, NAME_WORK_GROUP_SCAN_INCLUSIVE_MIN);
}

bool isWorkGroupScanExclusiveMax(StringRef S) {
  return isMangleOf(S, NAME_WORK_GROUP_SCAN_EXCLUSIVE_MAX);
}

bool isWorkGroupScanInclusiveMax(StringRef S) {
  return isMangleOf(S, NAME_WORK_GROUP_SCAN_INCLUSIVE_MAX);
}

bool isWorkGroupScanExclusiveBitwiseAnd(StringRef S) {
  return isMangleOf(S, NAME_WORK_GROUP_SCAN_EXCLUSIVE_BITWISE_AND);
}

bool isWorkGroupScanInclusiveBitwiseAnd(StringRef S) {
  return isMangleOf(S, NAME_WORK_GROUP_SCAN_INCLUSIVE_BITWISE_AND);
}

bool isWorkGroupScanExclusiveBitwiseOr(StringRef S) {
  return isMangleOf(S, NAME_WORK_GROUP_SCAN_EXCLUSIVE_BITWISE_OR);
}

bool isWorkGroupScanInclusiveBitwiseOr(StringRef S) {
  return isMangleOf(S, NAME_WORK_GROUP_SCAN_INCLUSIVE_BITWISE_OR);
}

bool isWorkGroupScanExclusiveBitwiseXor(StringRef S) {
  return isMangleOf(S, NAME_WORK_GROUP_SCAN_EXCLUSIVE_BITWISE_XOR);
}

bool isWorkGroupScanInclusiveBitwiseXor(StringRef S) {
  return isMangleOf(S, NAME_WORK_GROUP_SCAN_INCLUSIVE_BITWISE_XOR);
}

bool isWorkGroupReserveReadPipe(StringRef S) {
  return S == NAME_WORK_GROUP_RESERVE_READ_PIPE;
}

bool isWorkGroupCommitReadPipe(StringRef S) {
  return S == NAME_WORK_GROUP_COMMIT_READ_PIPE;
}

bool isWorkGroupReserveWritePipe(StringRef S) {
  return S == NAME_WORK_GROUP_RESERVE_WRITE_PIPE;
}

bool isWorkGroupCommitWritePipe(StringRef S) {
  return S == NAME_WORK_GROUP_COMMIT_WRITE_PIPE;
}

bool isAsyncWorkGroupCopy(StringRef S) {
  return isMangleOf(S, NAME_ASYNC_WORK_GROUP_COPY);
}

bool isAsyncWorkGroupStridedCopy(StringRef S) {
  return isMangleOf(S, NAME_ASYNC_WORK_GROUP_STRIDED_COPY);
}

bool isWorkGroupSortBuitinByRegex(StringRef S, StringRef FuncKeyWords) {
  reflection::FunctionDescriptor FD = demangle(S);
  std::string FName;
  FName = FD.isNull() ? S : FD.Name;
  std::string DataTypes = "i8|i16|i32|i64|u8|u16|u32|u64|f16|f32|f64";
  const std::string Pattern =
      ("^" + FuncKeyWords + "p[13](" + DataTypes + ")_u32_p[13]i8$").str();
  Regex PatternRegex(Pattern);
  return PatternRegex.match(FName);
}

bool isWorkGroupKeyValueSortBuitinByRegex(StringRef S, StringRef FuncKeyWords) {
  reflection::FunctionDescriptor FD = demangle(S);
  std::string FName;
  FName = FD.isNull() ? S : FD.Name;
  std::string DataTypes = "i8|i16|i32|i64|u8|u16|u32|u64|f16|f32|f64";
  const std::string KeyValuePattern =
      ("^" + FuncKeyWords + "p[13](" + DataTypes + ")_p[13](" + DataTypes +
       ")_u32_p[13]i8$")
          .str();
  Regex KeyValuePatternRegex(KeyValuePattern);
  return KeyValuePatternRegex.match(FName);
}

bool isWorkGroupJointSort(StringRef S) {
  return isWorkGroupSortBuitinByRegex(S, NAME_WORK_GROUP_JOINT_SORT_ASCEND) ||
         isWorkGroupSortBuitinByRegex(S, NAME_WORK_GROUP_JOINT_SORT_DESCEND);
}

bool isWorkGroupKeyValueJointSort(StringRef S) {
  return isWorkGroupKeyValueSortBuitinByRegex(
             S, NAME_WORK_GROUP_JOINT_SORT_ASCEND) ||
         isWorkGroupKeyValueSortBuitinByRegex(
             S, NAME_WORK_GROUP_JOINT_SORT_DESCEND);
}

bool isWorkGroupPrivateSpreadSort(StringRef S) {
  return isWorkGroupSortBuitinByRegex(
             S, NAME_WORK_GROUP_PRIVATE_SPREAD_SORT_ASCEND) ||
         isWorkGroupSortBuitinByRegex(
             S, NAME_WORK_GROUP_PRIVATE_SPREAD_SORT_DESCEND);
}

bool isWorkGroupPrivateCloseSort(StringRef S) {
  return isWorkGroupSortBuitinByRegex(
             S, NAME_WORK_GROUP_PRIVATE_CLOSE_SORT_ASCEND) ||
         isWorkGroupSortBuitinByRegex(
             S, NAME_WORK_GROUP_PRIVATE_CLOSE_SORT_DESCEND);
}

bool isWorkGroupKeyValuePrivateSpreadSort(StringRef S) {
  return isWorkGroupKeyValueSortBuitinByRegex(
             S, NAME_WORK_GROUP_PRIVATE_SPREAD_SORT_ASCEND) ||
         isWorkGroupKeyValueSortBuitinByRegex(
             S, NAME_WORK_GROUP_PRIVATE_SPREAD_SORT_DESCEND);
}

bool isWorkGroupKeyValuePrivateCloseSort(StringRef S) {
  return isWorkGroupKeyValueSortBuitinByRegex(
             S, NAME_WORK_GROUP_PRIVATE_CLOSE_SORT_ASCEND) ||
         isWorkGroupKeyValueSortBuitinByRegex(
             S, NAME_WORK_GROUP_PRIVATE_CLOSE_SORT_DESCEND);
}

bool isWorkGroupPrivateSort(StringRef S) {
  return isWorkGroupPrivateCloseSort(S) || isWorkGroupPrivateSpreadSort(S);
}

bool isWorkGroupKeyValuePrivateSort(StringRef S) {
  return isWorkGroupKeyValuePrivateSpreadSort(S) ||
         isWorkGroupKeyValuePrivateCloseSort(S);
}

bool isWorkGroupKeyOnlySort(StringRef S) {
  return isWorkGroupPrivateSort(S) || isWorkGroupJointSort(S);
}

bool isWorkGroupKeyValueSort(StringRef S) {
  return isWorkGroupKeyValuePrivateSort(S) || isWorkGroupKeyValueJointSort(S);
}

bool isWorkGroupSort(StringRef S) {
  return isWorkGroupKeyOnlySort(S) || isWorkGroupKeyValueSort(S);
}

static bool isKMPAcquireReleaseLock(StringRef S) {
  return (S == NAME_IB_KMP_ACQUIRE_LOCK) || (S == NAME_IB_KMP_RELEASE_LOCK);
}

PipeKind getPipeKind(StringRef Name) {
  PipeKind Kind;
  Kind.Op = PipeKind::OpKind::None;

  StringRef S(Name);
  if (!S.consume_front("__"))
    return Kind;

  if (S.consume_front("sub_group_"))
    Kind.Scope = PipeKind::ScopeKind::SubGroup;
  else if (S.consume_front("work_group_"))
    Kind.Scope = PipeKind::ScopeKind::WorkGroup;
  else
    Kind.Scope = PipeKind::ScopeKind::WorkItem;

  if (S.consume_front("commit_"))
    Kind.Op = PipeKind::OpKind::Commit;
  else if (S.consume_front("reserve_"))
    Kind.Op = PipeKind::OpKind::Reserve;

  if (S.consume_front("read_"))
    Kind.Access = PipeKind::AccessKind::Read;
  else if (S.consume_front("write_"))
    Kind.Access = PipeKind::AccessKind::Write;
  else {
    Kind.Op = PipeKind::OpKind::None;
    return Kind; // not a pipe built-in
  }

  if (!S.consume_front("pipe")) {
    Kind.Op = PipeKind::OpKind::None;
    return Kind; // not a pipe built-in
  }

  if (Kind.Op == PipeKind::OpKind::Commit ||
      Kind.Op == PipeKind::OpKind::Reserve) {
    // rest for the modifiers only appliy to read/write built-ins
    return Kind;
  }

  if (S.consume_front("_2"))
    Kind.Op = PipeKind::OpKind::ReadWrite;
  else if (S.consume_front("_4"))
    Kind.Op = PipeKind::OpKind::ReadWriteReserve;

  // FPGA extension.
  if (S.consume_front("_bl"))
    Kind.Blocking = true;
  else
    Kind.Blocking = false;

  if (S.consume_front("_io"))
    Kind.IO = true;
  else
    Kind.IO = false;

  if (S.consume_front("_fpga"))
    Kind.FPGA = true;

  if (S.consume_front("_") && S.startswith("v"))
    Kind.SimdSuffix = std::string(S);

  assert(Name == getPipeName(Kind) &&
         "getPipeKind() and getPipeName() are not aligned!");

  return Kind;
}

std::string getPipeName(PipeKind Kind) {
  assert(Kind.Op != PipeKind::OpKind::None && "Invalid pipe kind");

  std::string Name("__");

  switch (Kind.Scope) {
  case PipeKind::ScopeKind::WorkGroup:
    Name += "work_group_";
    break;
  case PipeKind::ScopeKind::SubGroup:
    Name += "sub_group_";
    break;
  case PipeKind::ScopeKind::WorkItem:
    break;
  }

  switch (Kind.Op) {
  case PipeKind::OpKind::Commit:
    Name += "commit_";
    break;
  case PipeKind::OpKind::Reserve:
    Name += "reserve_";
    break;
  default:
    break;
  }

  switch (Kind.Access) {
  case PipeKind::AccessKind::Read:
    Name += "read_";
    break;
  case PipeKind::AccessKind::Write:
    Name += "write_";
    break;
  }
  Name += "pipe";

  switch (Kind.Op) {
  case PipeKind::OpKind::ReadWrite:
    Name += "_2";
    break;
  case PipeKind::OpKind::ReadWriteReserve:
    Name += "_4";
    break;
  default:
    // Rest of the modifiers only apply to read/write built-ins.
    return Name;
  }

  if (Kind.Blocking)
    Name += "_bl";

  if (Kind.IO)
    Name += "_io";

  if (Kind.FPGA)
    Name += "_fpga";

  if (!Kind.SimdSuffix.empty()) {
    Name += "_";
    Name += Kind.SimdSuffix;
  }

  return Name;
}

bool isPipeBuiltin(StringRef Name) { return getPipeKind(Name); }

Type *getArrayElementType(const ArrayType *ArrTy) {
  Type *ElemTy = ArrTy->getElementType();
  while (auto *InnerArrayTy = dyn_cast<ArrayType>(ElemTy))
    ElemTy = InnerArrayTy->getElementType();

  return ElemTy;
}

void getArrayTypeDimensions(const ArrayType *ArrTy,
                            SmallVectorImpl<size_t> &Dimensions) {
  const ArrayType *InnerArrTy = ArrTy;
  do {
    Dimensions.push_back(InnerArrTy->getNumElements());
  } while ((InnerArrTy = dyn_cast<ArrayType>(InnerArrTy->getElementType())));
}

StructType *getStructByName(StringRef Name, const Module *M) {
  std::vector<StructType *> StructTys = M->getIdentifiedStructTypes();

  for (auto *STy : StructTys) {
    if (!STy->hasName())
      continue;

    if (stripStructNameTrailingDigits(STy->getName())
            .equals(stripStructNameTrailingDigits(Name)))
      return STy;
  }

  return nullptr;
}

bool isWorkItemPipeBuiltin(StringRef S) {
  auto Kind = getPipeKind(S);
  return Kind && Kind.Scope == PipeKind::ScopeKind::WorkItem;
}

bool isWaitGroupEvents(StringRef S) {
  return isMangleOf(S, NAME_WAIT_GROUP_EVENTS);
}

bool isWorkGroupAsyncOrPipeBuiltin(StringRef S, const Module &M) {
  return isAsyncWorkGroupCopy(S) || isAsyncWorkGroupStridedCopy(S) ||
         (hasOcl20Support(M) &&
          (isWorkGroupReserveReadPipe(S) || isWorkGroupCommitReadPipe(S) ||
           isWorkGroupReserveWritePipe(S) || isWorkGroupCommitWritePipe(S)));
}

bool isWorkGroupScan(StringRef S) {
  return isWorkGroupScanExclusiveAdd(S) || isWorkGroupScanInclusiveAdd(S) ||
         isWorkGroupScanExclusiveMin(S) || isWorkGroupScanInclusiveMin(S) ||
         isWorkGroupScanExclusiveMax(S) || isWorkGroupScanInclusiveMax(S) ||
         isWorkGroupScanExclusiveMul(S) || isWorkGroupScanInclusiveMul(S) ||
         isWorkGroupScanExclusiveBitwiseAnd(S) ||
         isWorkGroupScanInclusiveBitwiseAnd(S) ||
         isWorkGroupScanExclusiveBitwiseOr(S) ||
         isWorkGroupScanInclusiveBitwiseOr(S) ||
         isWorkGroupScanExclusiveBitwiseXor(S) ||
         isWorkGroupScanInclusiveBitwiseXor(S);
}

bool isWorkGroupBuiltinUniform(StringRef S) {
  return isWorkGroupAll(S) || isWorkGroupAny(S) ||
         isWorkGroupBroadCast(S).first || isWorkGroupReduceAdd(S) ||
         isWorkGroupReduceMin(S) || isWorkGroupReduceMax(S) ||
         isWorkGroupReduceMul(S) || isWorkGroupReduceBitwiseAnd(S) ||
         isWorkGroupReduceBitwiseOr(S) || isWorkGroupReduceBitwiseXor(S) ||
         isWorkGroupReduceLogicalAnd(S) || isWorkGroupReduceLogicalOr(S) ||
         isWorkGroupReduceLogicalXor(S);
}

bool isWorkGroupMin(StringRef S) {
  return isWorkGroupReduceMin(S) || isWorkGroupScanExclusiveMin(S) ||
         isWorkGroupScanInclusiveMin(S);
}

bool isWorkGroupMax(StringRef S) {
  return isWorkGroupReduceMax(S) || isWorkGroupScanExclusiveMax(S) ||
         isWorkGroupScanInclusiveMax(S);
}

bool isWorkGroupMul(StringRef S) {
  return isWorkGroupReduceMul(S) || isWorkGroupScanExclusiveMul(S) ||
         isWorkGroupScanInclusiveMul(S);
}

bool isWorkGroupBuiltinDivergent(StringRef S) {
  return isWorkGroupScan(S) || isWorkGroupSort(S);
}

bool isWorkGroupUniform(StringRef S) {
  return isWorkGroupBuiltinUniform(S) || isGetMaxSubGroupSize(S) ||
         isGetNumSubGroups(S) || isGetEnqueuedNumSubGroups(S) ||
         isWorkGroupIdentity(S);
}

bool isWorkGroupDivergent(StringRef S) {
  return isWorkGroupBuiltinDivergent(S) ||
         (isSubGroupBuiltin(S) && !isWorkGroupUniform(S));
}

bool hasWorkGroupFinalizePrefix(StringRef S) {
  if (!isMangledName(S))
    return false;
  return stripName(S).startswith(NAME_FINALIZE_WG_FUNCTION_PREFIX);
}

std::string appendWorkGroupFinalizePrefix(StringRef S) {
  assert(isMangledName(S) && "expected mangled name of work group built-in");
  reflection::FunctionDescriptor FD = demangle(S);
  FD.Name = NAME_FINALIZE_WG_FUNCTION_PREFIX.str() + FD.Name;
  std::string finalizeFuncName = mangle(FD);
  return finalizeFuncName;
}

std::string removeWorkGroupFinalizePrefix(StringRef S) {
  assert(hasWorkGroupFinalizePrefix(S) && "expected finalize prefix");
  reflection::FunctionDescriptor FD = demangle(S);
  FD.Name = FD.Name.substr(NAME_FINALIZE_WG_FUNCTION_PREFIX.size());
  std::string FuncName = mangle(FD);
  return FuncName;
}

std::string getWorkGroupIdentityFinalize(StringRef S) {
  assert(isMangledName(S) && "expected mangled name of work group built-in");
  reflection::FunctionDescriptor FD = demangle(S);
  // Only preserve the first parameter
  FD.Parameters.resize(1);
  FD.Name = (NAME_FINALIZE_WG_FUNCTION_PREFIX + NAME_WORK_GROUP_IDENTITY).str();
  std::string finalizeFuncName = mangle(FD);
  return finalizeFuncName;
}

bool isWorkGroupBuiltin(StringRef S) {
  return isWorkGroupBuiltinUniform(S) || isWorkGroupBuiltinDivergent(S);
}

bool isWorkGroupBarrier(StringRef S) {
  return S == mangledBarrier() || S == mangledWGBarrier(BarrierType::NoScope) ||
         S == mangledWGBarrier(BarrierType::WithScope);
}

/// Subgroup builtin functions

bool isGetSubGroupSize(StringRef S) {
  return isMangleOf(S, NAME_GET_SUB_GROUP_SIZE);
}

bool isGetMaxSubGroupSize(StringRef S) {
  return isMangleOf(S, NAME_GET_MAX_SUB_GROUP_SIZE);
}

bool isGetNumSubGroups(StringRef S) {
  return isMangleOf(S, NAME_GET_NUM_SUB_GROUPS);
}

bool isGetEnqueuedNumSubGroups(StringRef S) {
  return isMangleOf(S, NAME_GET_ENQUEUED_NUM_SUB_GROUPS);
}

bool isGetSubGroupId(StringRef S) {
  return isMangleOf(S, NAME_GET_SUB_GROUP_ID);
}

bool isGetSubGroupLocalId(StringRef S) {
  return isMangleOf(S, NAME_GET_SUB_GROUP_LOCAL_ID);
}

bool isSubGroupAll(StringRef S) { return isMangleOf(S, NAME_SUB_GROUP_ALL); }

bool isSubGroupAny(StringRef S) { return isMangleOf(S, NAME_SUB_GROUP_ANY); }

bool isSubGroupBroadCast(StringRef S) {
  return isMangleOf(S, NAME_SUB_GROUP_BROADCAST);
}

static bool isSubGroupBallot(StringRef S) {
  return StringSwitch<bool>(S)
      .Case("sub_group_ballot", true)
      .Case("sub_group_inverse_ballot", true)
      .Case("sub_group_ballot_bit_extract", true)
      .Case("sub_group_ballot_bit_count", true)
      .Case("sub_group_ballot_inclusive_scan", true)
      .Case("sub_group_ballot_exclusive_scan", true)
      .Case("sub_group_ballot_find_lsb", true)
      .Case("sub_group_ballot_find_msb", true)
      .Default(false);
}

bool isSubGroupReduceAdd(StringRef S) {
  return isMangleOf(S, NAME_SUB_GROUP_REDUCE_ADD);
}

bool isSubGroupScanExclusiveAdd(StringRef S) {
  return isMangleOf(S, NAME_SUB_GROUP_SCAN_EXCLUSIVE_ADD);
}

bool isSubGroupScanInclusiveAdd(StringRef S) {
  return isMangleOf(S, NAME_SUB_GROUP_SCAN_INCLUSIVE_ADD);
}

bool isSubGroupReduceMin(StringRef S) {
  return isMangleOf(S, NAME_SUB_GROUP_REDUCE_MIN);
}

bool isSubGroupScanExclusiveMin(StringRef S) {
  return isMangleOf(S, NAME_SUB_GROUP_SCAN_EXCLUSIVE_MIN);
}

bool isSubGroupScanInclusiveMin(StringRef S) {
  return isMangleOf(S, NAME_SUB_GROUP_SCAN_INCLUSIVE_MIN);
}

bool isSubGroupReduceMax(StringRef S) {
  return isMangleOf(S, NAME_SUB_GROUP_REDUCE_MAX);
}

bool isSubGroupScanExclusiveMax(StringRef S) {
  return isMangleOf(S, NAME_SUB_GROUP_SCAN_EXCLUSIVE_MAX);
}

bool isSubGroupScanInclusiveMax(StringRef S) {
  return isMangleOf(S, NAME_SUB_GROUP_SCAN_INCLUSIVE_MAX);
}

bool isSubGroupSort(StringRef S) {
  reflection::FunctionDescriptor SortFD = demangle(S);
  StringRef FuncName = SortFD.isNull() ? S : SortFD.Name;
  if (!FuncName.consume_front(NAME_SUB_GROUP_SORT_ASCEND) &&
      !FuncName.consume_front(NAME_SUB_GROUP_SORT_DESCEND))
    return false;

  SmallVector<std::string, 11> AvailableSuffix = {"i8",  "i16", "i32", "i64",
                                                  "u8",  "u16", "u32", "u64",
                                                  "f16", "f32", "f64"};
  if (find(AvailableSuffix.begin(), AvailableSuffix.begin(), FuncName))
    return true;
  return false;
}

bool isSubGroupScan(StringRef S) {
  return isSubGroupScanExclusiveAdd(S) || isSubGroupScanInclusiveAdd(S) ||
         isSubGroupScanExclusiveMin(S) || isSubGroupScanInclusiveMin(S) ||
         isSubGroupScanExclusiveMax(S) || isSubGroupScanInclusiveMax(S);
}

static bool isSubGroupShuffle(StringRef S) {
  if (!isMangledName(S))
    return false;
  StringRef Name = stripName(S);
  (void)Name.consume_front("intel_");
  return Name == NAME_SUB_GROUP_SHUFFLE ||
         Name == NAME_SUB_GROUP_SHUFFLE_DOWN ||
         Name == NAME_SUB_GROUP_SHUFFLE_UP ||
         Name == NAME_SUB_GROUP_SHUFFLE_XOR;
}

static bool isIntelSubGroupBlockReadWrite(StringRef S, StringRef ReadOrWrite) {
  if (!isMangledName(S))
    return false;
  StringRef Name = stripName(S);
  if (!Name.consume_front("intel_") || !Name.consume_front(ReadOrWrite))
    return false;
  // Check e.g. intel_sub_group_block_read
  if (Name.empty())
    return true;
  // Check e.g. intel_sub_group_block_read8
  unsigned Num;
  return !Name.consumeInteger(10, Num) && (Num == 2 || Num == 4 || Num == 8) &&
         Name.empty();
}

static bool isSubGroupBlockRead(StringRef S) {
  return isIntelSubGroupBlockReadWrite(S, NAME_SUB_GROUP_BLOCK_READ);
}

static bool isSubGroupBlockWrite(StringRef S) {
  return isIntelSubGroupBlockReadWrite(S, NAME_SUB_GROUP_BLOCK_WRITE);
}

bool isSubGroupNonUniformFlow(StringRef S) {
  if (!isMangledName(S))
    return false;
  S = stripName(S);
  if (isSubGroupBallot(S))
    return true;
  if (!S.consume_front("sub_group_non_uniform_"))
    return false;

  // From extension cl_khr_subgroup_non_uniform_arithmetic.
  if (S.consume_front("reduce_") || S.consume_front("scan_inclusive_") ||
      S.consume_front("scan_exclusive_"))
    return S.equals("add") || S.equals("min") || S.equals("max") ||
           S.equals("mul") || S.equals("and") || S.equals("or") ||
           S.equals("xor") || S.equals("logical_and") ||
           S.equals("logical_or") || S.equals("logical_xor");

  // From extension cl_khr_subgroup_non_uniform_vote and cl_khr_subgroup_ballot.
  return S.equals("all") || S.equals("any") || S.equals("all_equal") ||
         S.equals("broadcast");
}

// TODO: refactor OCLVecClone - opencl-vec-uniform-return.
bool isSubGroupUniformFlowUniformRet(StringRef S) {
  return isGetSubGroupSize(S) || isGetSubGroupId(S) ||
         isGetMaxSubGroupSize(S) || isGetNumSubGroups(S) ||
         isGetEnqueuedNumSubGroups(S) || isSubGroupAll(S) || isSubGroupAny(S) ||
         isSubGroupBroadCast(S) || isSubGroupReduceAdd(S) ||
         isSubGroupReduceMin(S) || isSubGroupReduceMax(S) || isSubGroupSort(S);
}

bool isSubGroupUniformFlowNonUniformRet(StringRef S) {
  return isGetSubGroupLocalId(S) || isSubGroupScan(S) || isSubGroupShuffle(S) ||
         isSubGroupBlockRead(S) || isSubGroupBlockWrite(S);
}

bool isSubGroupBuiltin(StringRef S) {
  return isSubGroupUniformFlowUniformRet(S) ||
         isSubGroupUniformFlowNonUniformRet(S) || isSubGroupNonUniformFlow(S);
}

bool isSubGroupBarrier(StringRef S) {
  return S == mangledSGBarrier(BarrierType::NoScope) ||
         S == mangledSGBarrier(BarrierType::WithScope);
}

#if INTEL_CUSTOMIZATION
bool isGetSubGroupSliceLength(StringRef S) {
  return S.equals(NAME_GET_SUB_GROUP_SLICE_LENGTH);
}

bool isSubGroupRowSliceExtractElement(StringRef S) {
  // Might be vectorized as '_ZGVbN16u_sub_group_rowslice_extractelement'
  return S.contains(NAME_SUB_GROUP_ROWSLICE_EXTRACTELEMENT);
}

bool isSubGroupRowSliceInsertElement(StringRef S) {
  // Might be vectorized as '_ZGVbN16uv_sub_group_rowslice_insertelement'
  return S.contains(NAME_SUB_GROUP_ROWSLICE_INSERTELEMENT);
}

bool isSubGroupInsertRowSliceToMatrix(StringRef S) {
  // Has type mangle suffix.
  return S.startswith(NAME_SUB_GROUP_INSERT_ROWSLICE_TO_MATRIX);
}
#endif // INTEL_CUSTOMIZATION

template <reflection::TypePrimitiveEnum... ParamTys>
static std::string optionalMangleWithParam(StringRef N) {
  reflection::FunctionDescriptor FD;
  FD.Name = N.str();
  for (auto PT : {ParamTys...})
    FD.Parameters.push_back(new reflection::PrimitiveType(PT));
  return mangle(FD);
}

template <reflection::TypePrimitiveEnum Ty>
static std::string mangleWithParam(StringRef N, unsigned NumOfParams) {
  reflection::FunctionDescriptor FD;
  FD.Name = N.str();
  for (unsigned I = 0; I < NumOfParams; ++I)
    FD.Parameters.push_back(new reflection::PrimitiveType(Ty));
  return mangle(FD);
}

static std::string
mangleWithParam(StringRef N, ArrayRef<reflection::TypePrimitiveEnum> Types) {
  reflection::FunctionDescriptor FD;
  FD.Name = N.str();
  for (const auto &Ty : Types) {
    reflection::ParamType *pTy = new reflection::PrimitiveType(Ty);
    reflection::RefParamType UI(pTy);
    FD.Parameters.push_back(UI);
  }
  return mangle(FD);
}

std::string mangledAtomicWorkItemFence() {
  reflection::TypePrimitiveEnum Params[] = {reflection::PRIMITIVE_UINT,
                                            reflection::PRIMITIVE_MEMORY_ORDER,
                                            reflection::PRIMITIVE_MEMORY_SCOPE};

  return mangleWithParam(NAME_ATOMIC_WORK_ITEM_FENCE, Params);
}

std::string getWorkGroupSortCopyName(StringRef SortFuncName, bool ToScratch) {
  reflection::FunctionDescriptor FD;
  bool IsSpread = isWorkGroupKeyValuePrivateSpreadSort(SortFuncName) ||
                  isWorkGroupPrivateSpreadSort(SortFuncName);
  FD.Name = (IsSpread && !ToScratch) ? NAME_WORK_GROUP_PRIVATE_SPREAD_SORT_COPY
                                     : NAME_WORK_GROUP_PRIVATE_SORT_COPY;

  reflection::FunctionDescriptor SortFD = demangle(SortFuncName);
  for (auto &TypeIter : SortFD.Parameters) {
    // Not add mask to copy builtin params, now assume sort is uniform
    // Only mask param in vector workgroup sort builtin is vector of uint
    if (TypeIter->getTypeId() == reflection::TYPE_ID_VECTOR) {
      reflection::VectorType *Vec =
          dyn_cast<reflection::VectorType>(TypeIter.get());
      if (Vec->getScalarType()->getTypeId() == reflection::TYPE_ID_PRIMITIVE) {
        reflection::PrimitiveType *PrimitiveParam =
            dyn_cast<reflection::PrimitiveType>(Vec->getScalarType().get());
        if (PrimitiveParam->getPrimitive() == reflection::PRIMITIVE_UINT)
          continue;
      }
    }
    // Other type can just be added to Parameters
    FD.Parameters.push_back(TypeIter);
  }
  // int type param, for local_id
  FD.Parameters.push_back(
      new reflection::PrimitiveType(reflection::PRIMITIVE_LONG));
  // int type param, for local_size
  FD.Parameters.push_back(
      new reflection::PrimitiveType(reflection::PRIMITIVE_LONG));
  // bool type param, for direction
  FD.Parameters.push_back(
      new reflection::PrimitiveType(reflection::PRIMITIVE_BOOL));
  return mangle(FD);
}

std::string mangledGetGID() {
  return optionalMangleWithParam<reflection::PRIMITIVE_UINT>(NAME_GET_GID);
}

std::string mangledGetGlobalSize() {
  return optionalMangleWithParam<reflection::PRIMITIVE_UINT>(
      NAME_GET_GLOBAL_SIZE);
}

std::string mangledGetGlobalOffset() {
  return optionalMangleWithParam<reflection::PRIMITIVE_UINT>(
      NAME_GET_GLOBAL_OFFSET);
}

std::string mangledGetLID() {
  return optionalMangleWithParam<reflection::PRIMITIVE_UINT>(NAME_GET_LID);
}

std::string mangledGetGroupID() {
  return optionalMangleWithParam<reflection::PRIMITIVE_UINT>(NAME_GET_GROUP_ID);
}

std::string mangledGetNumGroups() {
  return optionalMangleWithParam<reflection::PRIMITIVE_UINT>(
      NAME_GET_NUM_GROUPS);
}

std::string mangledGetLocalSize() {
  return optionalMangleWithParam<reflection::PRIMITIVE_UINT>(
      NAME_GET_LOCAL_SIZE);
}

std::string mangledGetEnqueuedLocalSize() {
  return mangleWithParam<reflection::PRIMITIVE_UINT>(
      NAME_GET_ENQUEUED_LOCAL_SIZE, 1);
}

std::string mangledBarrier() {
  return optionalMangleWithParam<reflection::PRIMITIVE_UINT>(NAME_BARRIER);
}

std::string mangledWGBarrier(BarrierType BT) {
  switch (BT) {
  case BarrierType::NoScope:
    return optionalMangleWithParam<reflection::PRIMITIVE_UINT>(NAME_WG_BARRIER);
  case BarrierType::WithScope:
    return optionalMangleWithParam<reflection::PRIMITIVE_UINT,
                                   reflection::PRIMITIVE_MEMORY_SCOPE>(
        NAME_WG_BARRIER);
  }

  llvm_unreachable("Unknown work_group_barrier version");
  return "";
}

std::string mangledSGBarrier(BarrierType BT) {
  switch (BT) {
  case BarrierType::NoScope:
    return optionalMangleWithParam<reflection::PRIMITIVE_UINT>(NAME_SG_BARRIER);
  case BarrierType::WithScope:
    return optionalMangleWithParam<reflection::PRIMITIVE_UINT,
                                   reflection::PRIMITIVE_MEMORY_SCOPE>(
        NAME_SG_BARRIER);
  }

  llvm_unreachable("Unknown sub_group_barrier version");
  return "";
}

std::string mangledGetMaxSubGroupSize() {
  return optionalMangleWithParam<reflection::PRIMITIVE_VOID>(
      NAME_GET_MAX_SUB_GROUP_SIZE);
}

std::string mangledNumSubGroups() {
  return optionalMangleWithParam<reflection::PRIMITIVE_VOID>(
      NAME_GET_NUM_SUB_GROUPS);
}

std::string mangledGetSubGroupId() {
  return optionalMangleWithParam<reflection::PRIMITIVE_VOID>(
      NAME_GET_SUB_GROUP_ID);
}

std::string mangledEnqueuedNumSubGroups() {
  return optionalMangleWithParam<reflection::PRIMITIVE_VOID>(
      NAME_GET_ENQUEUED_NUM_SUB_GROUPS);
}

std::string mangledGetSubGroupSize() {
  return optionalMangleWithParam<reflection::PRIMITIVE_VOID>(
      NAME_GET_SUB_GROUP_SIZE);
}

std::string mangledGetSubGroupLocalId() {
  return optionalMangleWithParam<reflection::PRIMITIVE_VOID>(
      NAME_GET_SUB_GROUP_LOCAL_ID);
}

std::string mangledGetGlobalLinearId() {
  return optionalMangleWithParam<reflection::PRIMITIVE_VOID>(
      NAME_GET_LINEAR_GID);
}

std::string mangledGetLocalLinearId() {
  return optionalMangleWithParam<reflection::PRIMITIVE_VOID>(
      NAME_GET_LINEAR_LID);
}

std::pair<bool, unsigned> isTIDGenerator(const CallInst *CI) {
  if (!CI || !CI->getCalledFunction())
    return {false, 0};

  StringRef FName = CI->getCalledFunction()->getName();
  if (!isGetGlobalId(FName) && !isGetLocalId(FName) &&
      !isGetSubGroupLocalId(FName))
    return {false, 0}; // not a get_***_id function.

  // Early exit for subgroup TIDs that do not take any operands.
  // Dummy Dim 0 as subgroup does not have a clear dimension.
  if (isGetSubGroupLocalId(FName))
    return {true, 0};

  // Go on checking the first argument for other TIDS.
  Value *Op = CI->getArgOperand(0);

  // Report the dimension of the request. It should be a constant since
  // ResolveVarTIDCall pass is already run.
  auto Dim =
      static_cast<unsigned>(cast<ConstantInt>(Op)->getValue().getZExtValue());

  // This is indeed a TID generator.
  return {true, Dim};
}

Function *importFunctionDecl(Module *Dst, const Function *Orig,
                             bool DuplicateIfExists) {
  assert(Orig && "Invalid types!");
  FunctionType *NewFnType = Orig->getFunctionType();

  if (!DuplicateIfExists)
    return cast<Function>(Dst->getOrInsertFunction(Orig->getName(), NewFnType,
                                                   Orig->getAttributes())
                              .getCallee());

  // Create a declaration of the function to import disrespecting the fact of
  // it's existence in the module.
  Function *NewF = Function::Create(NewFnType, GlobalVariable::ExternalLinkage,
                                    Orig->getName(), Dst);
  NewF->setAttributes(Orig->getAttributes());

  return NewF;
}

FuncSet getAllKernels(Module &M) {
  auto Kernels = getKernels(M);

  // List all kernels in module
  FuncSet VectorizedFSet;
  for (auto *F : Kernels) {
    // Need to check if Vectorized Kernel Value exists, it is not guaranteed
    // that Vectorized is running in all scenarios.
    SYCLKernelMetadataAPI::KernelInternalMetadataAPI KIMD(F);
    Function *VectorizedF = KIMD.VectorizedKernel.hasValue()
                                ? KIMD.VectorizedKernel.get()
                                : nullptr;
    if (VectorizedF)
      VectorizedFSet.insert(VectorizedF);
    Function *VectorizedMaskedF = KIMD.VectorizedMaskedKernel.hasValue()
                                      ? KIMD.VectorizedMaskedKernel.get()
                                      : nullptr;
    if (VectorizedMaskedF)
      VectorizedFSet.insert(VectorizedMaskedF);
  }

  FuncSet FSet(Kernels.begin(), Kernels.end());
  FSet.insert(VectorizedFSet.begin(), VectorizedFSet.end());
  return FSet;
}

Function *getFnAttributeFunction(Module &M, Function &F, StringRef AttrKind) {
  if (F.hasFnAttribute(AttrKind)) {
    return M.getFunction(F.getFnAttribute(AttrKind).getValueAsString());
  }
  return nullptr;
}

StringRef getFnAttributeStringInList(Function &F, StringRef AttrKind,
                                     unsigned Idx) {
  if (!F.hasFnAttribute(AttrKind))
    return "";
  StringRef Attr = F.getFnAttribute(AttrKind).getValueAsString();
  SmallVector<StringRef, 4> AttrVec;
  SplitString(Attr, AttrVec);
  assert(AttrVec.size() > Idx && "Index is out of range");
  return AttrVec[Idx];
}

void moveAlloca(BasicBlock *FromBB, BasicBlock *ToBB) {
  auto InsertionPt = ToBB->getFirstInsertionPt();
  SmallVector<Instruction *, 4> ToMove;
  for (auto &I : *FromBB) {
    if (isa<AllocaInst>(&I))
      ToMove.push_back(&I);
  }
  for (auto *I : ToMove)
    I->moveBefore(*ToBB, InsertionPt);
}

void collectDependentInstsToMove(Instruction *I, BasicBlock *ToBB,
                                 SmallVectorImpl<Instruction *> &ToMove) {
  if (I->getNumOperands()) {
    // Collect dependent instructions to move.
    Use *TheUse = &*I->op_begin();
    for (Use *U : make_range(po_begin(TheUse), po_end(TheUse))) {
      Value *V = U->get();
      if (auto *Inst = dyn_cast<Instruction>(V)) {
        assert(!isa<PHINode>(V) && "PHINode is not handled");
        if (Inst->getParent() != ToBB &&
            llvm::find(ToMove, Inst) == ToMove.end())
          ToMove.push_back(Inst);
      }
    }
  }
  ToMove.push_back(I);
}

void moveInstructionIf(BasicBlock *FromBB, BasicBlock *ToBB,
                       function_ref<bool(Instruction &)> Predicate) {
  SmallVector<Instruction *, 16> ToMove;
  for (auto &I : *FromBB)
    if (Predicate(I))
      collectDependentInstsToMove(&I, ToBB, ToMove);

  auto MovePos = ToBB->getFirstInsertionPt();
  for (auto *I : ToMove)
    I->moveBefore(*ToBB, MovePos);
}

InstVec getCallInstUsersOfFunc(const Module &M, StringRef FuncName) {
  InstVec CIs;
  if (Function *F = M.getFunction(FuncName)) {
    for (User *U : F->users())
      if (auto *CI = dyn_cast<CallInst>(U))
        CIs.push_back(CI);
  }
  return CIs;
}

FuncSet getAllSyncBuiltinsDecls(Module &M, bool IsWG) {
  FuncSet FSet;

  for (Function &F : M) {
    if (!F.isDeclaration())
      continue;
    StringRef FName = F.getName();
    if (IsWG) {
      if (isWorkGroupBarrier(FName) ||
          /* work group built-ins */
          isWorkGroupBuiltin(FName) ||
          /* built-ins synced as if were called by a single work item */
          isWorkGroupAsyncOrPipeBuiltin(FName, M) || isWorkGroupSort(FName))
        FSet.insert(&F);
    } else {
      if (isSubGroupBarrier(FName) || isSubGroupBuiltin(FName))
        FSet.insert(&F);
    }
  }
  return FSet;
}

FuncSet getAllSyncBuiltinsDeclsForNoDuplicateRelax(Module &M) {
  FuncSet FSet = getAllSyncBuiltinsDecls(M);

  // Add sub_group_barrier separately. It does not require following Barrier
  // compilation flow, but requires noduplicate relaxation.
  for (Function &F : M) {
    if (!F.isDeclaration())
      continue;
    llvm::StringRef FName = F.getName();
    if (isSubGroupBarrier(FName) || isKMPAcquireReleaseLock(FName))
      FSet.insert(&F);
  }

  return FSet;
}

FuncSet getAllSyncBuiltinsDeclsForKernelUniformCallAttr(Module &M) {
  FuncSet FSet;

  for (Function &F : M) {
    if (!F.isDeclaration())
      continue;
    llvm::StringRef FName = F.getName();
    if (isWorkGroupBarrier(FName) || isSubGroupBarrier(FName) ||
        isKMPAcquireReleaseLock(FName) ||
        isWorkGroupAsyncOrPipeBuiltin(FName, M)) {
      FSet.insert(&F);
    }
  }
  return FSet;
}

static std::string addSuffixInFunctionName(std::string FuncName,
                                           StringRef Suffix) {
  return (Twine("__") + FuncName + Twine("_before.") + Suffix).str();
}

static void replaceScalarKernelInVectorizerMetadata(Function *VFunc,
                                                    Function *NewF,
                                                    Function *OldF) {
  SYCLKernelMetadataAPI::KernelInternalMetadataAPI VKIMD(VFunc);
  if (!VKIMD.ScalarKernel.hasValue() || VKIMD.ScalarKernel.get() != OldF)
    return;
  VKIMD.ScalarKernel.set(NewF);
}

static void replaceScalarKernelInMetadata(Function *OldF, Function *NewF) {
  SYCLKernelMetadataAPI::KernelInternalMetadataAPI KIMD(NewF);
  if (KIMD.VectorizedKernel.hasValue()) {
    Function *VectorizedF = KIMD.VectorizedKernel.get();
    replaceScalarKernelInVectorizerMetadata(VectorizedF, NewF, OldF);
  }
  if (KIMD.VectorizedMaskedKernel.hasValue()) {
    Function *VectorizedMaskedF = KIMD.VectorizedMaskedKernel.get();
    replaceScalarKernelInVectorizerMetadata(VectorizedMaskedF, NewF, OldF);
  }
}

static void replaceVectorizedKernelInMetadata(Function *OldF, Function *NewF) {
  SYCLKernelMetadataAPI::KernelInternalMetadataAPI VKIMD(NewF);
  if (!VKIMD.ScalarKernel.hasValue()) {
    return;
  }
  Function *ScalarFunc = VKIMD.ScalarKernel.get();
  if (!ScalarFunc) {
    return;
  }
  SYCLKernelMetadataAPI::KernelInternalMetadataAPI KIMD(ScalarFunc);
  if (KIMD.VectorizedKernel.hasValue() && KIMD.VectorizedKernel.get() == OldF) {
    KIMD.VectorizedKernel.set(NewF);
  } else if (KIMD.VectorizedMaskedKernel.hasValue() &&
             KIMD.VectorizedMaskedKernel.get() == OldF) {
    KIMD.VectorizedMaskedKernel.set(NewF);
  }
}

Function *AddMoreArgsToFunc(Function *F, ArrayRef<Type *> NewTypes,
                            ArrayRef<const char *> NewArgumentNames,
                            ArrayRef<AttributeSet> NewAttrs, StringRef Suffix) {
  assert(NewTypes.size() == NewArgumentNames.size());
  // Initialize with all original arguments in the function signature.
  SmallVector<Type *, 16> Types;

  for (Function::const_arg_iterator I = F->arg_begin(), E = F->arg_end();
       I != E; ++I) {
    Types.push_back(I->getType());
  }
  Types.append(NewTypes.begin(), NewTypes.end());
  FunctionType *NewFTy = FunctionType::get(F->getReturnType(), Types, false);
  // Change original function name.
  std::string OrigFuncName = F->getName().str();
  F->setName(addSuffixInFunctionName(OrigFuncName, Suffix));

  // Create a new function with explicit and implict arguments types
  Function *NewF =
      Function::Create(NewFTy, F->getLinkage(), OrigFuncName, F->getParent());
  // Copy old function attributes (including attributes on original arguments)
  // to new function.
  NewF->copyAttributesFrom(F);
  NewF->copyMetadata(F, 0);
  NewF->setCallingConv(F->getCallingConv());
  NewF->setDSOLocal(F->isDSOLocal());

  // Set original arguments' names.
  Function::arg_iterator NewI = NewF->arg_begin();
  for (Function::const_arg_iterator I = F->arg_begin(), E = F->arg_end();
       I != E; ++I, ++NewI) {
    NewI->setName(I->getName());
  }
  // Set new arguments' names.
  for (unsigned I = 0, E = NewArgumentNames.size(); I < E; ++I, ++NewI) {
    Argument *A = &*NewI;
    A->setName(NewArgumentNames[I]);
    if (!NewAttrs.empty())
      for (const auto &Attr : NewAttrs[I])
        A->addAttr(Attr);
  }

  // Since we have now created the new function, splice the body of the old
  // function right into the new function, leaving the old body of the function
  // empty.
  NewF->splice(NewF->begin(), F);
  assert(F->isDeclaration() &&
         "splice does not work, original function body is not empty!");

  // Set DISubprogram as an original function has. Do it before delete body
  // since DISubprogram will be deleted too.
  NewF->setSubprogram(F->getSubprogram());

  // Add comdat to newF and drop from F.
  NewF->setComdat(F->getComdat());
  F->setComdat(nullptr);

  // Delete original function body - this is needed to remove linkage (if
  // exists).
  F->deleteBody();
  // Loop over the argument list, transferring uses of the old arguments over to
  // the new arguments.
  for (Function::arg_iterator I = F->arg_begin(), E = F->arg_end(),
                              NI = NewF->arg_begin();
       I != E; ++I, ++NI) {
    // Replace the users to the new version.
    I->replaceAllUsesWith(&*NI);
  }

  // Since the name of F function is added with suffix, we have to replace it
  // with original name of F function (now it's name of NewF function) with NewF
  // function name in the metadata for vectorized kernel, masked kernel and
  // scalar kernel
  replaceVectorizedKernelInMetadata(F, NewF);
  replaceScalarKernelInMetadata(F, NewF);

  return NewF;
}

CallInst *AddMoreArgsToCall(CallInst *OldC, ArrayRef<Value *> NewArgs,
                            Function *NewF) {
  assert(OldC && "CallInst is NULL");
  assert(NewF && "function is NULL");
  assert(OldC->arg_size() + NewArgs.size() == NewF->arg_size() &&
         "Function argument number mismatch");

  SmallVector<Value *, 16> Args;
  for (unsigned I = 0, E = OldC->arg_size(); I != E; ++I)
    Args.push_back(OldC->getArgOperand(I));
  Args.append(NewArgs.begin(), NewArgs.end());

  // Replace the original function with a call
  CallInst *NewC = CallInst::Create(NewF, Args, "", OldC);
  NewC->setCallingConv(OldC->getCallingConv());
  NewC->setAttributes(NewF->getAttributes());

  // Copy debug metadata to new function if available
  if (OldC->hasMetadata()) {
    NewC->setDebugLoc(OldC->getDebugLoc());
  }

  OldC->replaceAllUsesWith(NewC);
  // Need to erase from parent to make sure there are no uses for the called
  // function when we delete it
  OldC->eraseFromParent();
  return NewC;
}

CallInst *addMoreArgsToIndirectCall(CallInst *OldC, ArrayRef<Value *> NewArgs) {
  assert(OldC && "CallInst is NULL");
  assert(!OldC->getCalledFunction() && "Not an indirect call");

  SmallVector<Value *, 16> Args;
  // Copy existing arguments
  for (unsigned I = 0, E = OldC->arg_size(); I != E; ++I)
    Args.push_back(OldC->getArgOperand(I));
  // And append new arguments
  Args.append(NewArgs.begin(), NewArgs.end());

  auto *FPtrType = cast<PointerType>(OldC->getCalledOperand()->getType());
  auto *FType = OldC->getFunctionType();
  SmallVector<Type *, 16> ArgTys;
  for (const auto &V : Args)
    ArgTys.push_back(V->getType());

  auto *NewFType =
      FunctionType::get(FType->getReturnType(), ArgTys, /* vararg = */ false);
  auto *Cast = CastInst::CreatePointerCast(
      OldC->getCalledOperand(),
      PointerType::get(NewFType, FPtrType->getAddressSpace()), "", OldC);
  assert(Cast && "Failed to create CastInst");

  // Replace the original function with a call
  auto *NewC = CallInst::Create(NewFType, Cast, Args, "", OldC);
  assert(NewC && "Failed to create CallInst");
  NewC->setCallingConv(OldC->getCallingConv());

  // Copy debug metadata to new function if available
  if (OldC->hasMetadata())
    NewC->setDebugLoc(OldC->getDebugLoc());

  OldC->replaceAllUsesWith(NewC);
  // Erasing from parent is not really necessary, but let's cleanup a little bit
  // here
  OldC->eraseFromParent();
  return NewC;
}

unsigned fetchCLVersionFromMetadata(const Module &M) {
  /*
  Example of the metadata
  !opencl.ocl.version = !{!6}
  !6 = !{i32 2, i32 0}
  */

  NamedMDNode *Node = M.getNamedMetadata("opencl.ocl.version");
  if (Node && Node->getNumOperands()) {
    auto *Op = Node->getOperand(0);
    if (Op->getNumOperands() >= 2) {
      uint64_t Major =
          mdconst::extract<ConstantInt>(Op->getOperand(0))->getZExtValue();
      uint64_t Minor =
          mdconst::extract<ConstantInt>(Op->getOperand(1))->getZExtValue();
      return CLVersionToVal(Major, Minor);
    }
  }

  // Always return an OpenCL version to avoid any issues
  // in manually written LIT tests.
  return OclVersion::CL_VER_DEFAULT;
}

bool hasOcl20Support(const Module &M) {
  return isGeneratedFromOCLCPP(M) ||
         fetchCLVersionFromMetadata(M) >= OclVersion::CL_VER_2_0;
}

void getImplicitArgs(Function *pFunc, Value **LocalMem, Value **WorkDim,
                     Value **WGId, Value **BaseGlbId, Value **SpecialBuf,
                     Value **RunTimeHandle) {

  assert(pFunc && "Function cannot be null");
  assert(pFunc->arg_size() >= ImplicitArgsUtils::NUM_IMPLICIT_ARGS &&
         "implicit args was not added!");

  // Iterating over explicit arguments
  Function::arg_iterator DestI = pFunc->arg_begin();

  // Go over the explicit arguments
  for (unsigned int i = 0;
       i < pFunc->arg_size() - ImplicitArgsUtils::NUM_IMPLICIT_ARGS; ++i)
    ++DestI;

  // Retrieve all the implicit arguments which are not NULL

  if (nullptr != LocalMem)
    *LocalMem = &*DestI;
  ++DestI;

  if (nullptr != WorkDim)
    *WorkDim = &*DestI;
  ++DestI;

  if (nullptr != WGId)
    *WGId = &*DestI;
  ++DestI;

  if (nullptr != BaseGlbId)
    *BaseGlbId = &*DestI;
  ++DestI;

  if (nullptr != SpecialBuf)
    *SpecialBuf = &*DestI;
  ++DestI;

  if (nullptr != RunTimeHandle)
    *RunTimeHandle = &*DestI;
  ++DestI;
  assert(DestI == pFunc->arg_end());
}

Type *getSLMBufferElementType(LLVMContext &C) { return IntegerType::get(C, 8); }

Type *getWorkGroupInfoElementType(LLVMContext &C,
                                  ArrayRef<Type *> WGInfoMembersTypes) {
  return StructType::get(C, WGInfoMembersTypes, false);
}

Type *getWorkGroupIDElementType(Module *M) { return LoopUtils::getIndTy(M); }

bool hasTLSGlobals(Module &M) {
  for (int Idx = 0; Idx < ImplicitArgsUtils::IA_NUMBER; ++Idx)
    if (getTLSGlobal(&M, Idx))
      return true;
  return false;
}

GlobalVariable *getTLSGlobal(Module *M, unsigned Idx) {
  assert(M && "Module cannot be null");
  return M->getNamedGlobal(ImplicitArgsUtils::getArgNameWithPrefix(Idx));
}

StringRef getTLSLocalIdsName() { return NAME_TLS_LOCAL_IDS; }

Value *createGetPtrToLocalId(Value *LocalIdValues, Type *LIdsTy, Value *Dim,
                             IRBuilderBase &Builder) {
  SmallVector<Value *, 4> Indices;
  Indices.push_back(Builder.getInt64(0));
  Indices.push_back(Dim);
  return Builder.CreateInBoundsGEP(
      LIdsTy, LocalIdValues, Indices,
      CompilationUtils::AppendWithDimension("pLocalId_", Dim));
}

void parseKernelArguments(Module *M, Function *F, bool HasTLSGlobals,
                          std::vector<KernelArgument> &Arguments,
                          std::vector<unsigned int> &MemArguments) {
  size_t ArgsCount = F->arg_size();
  if (!HasTLSGlobals)
    ArgsCount -= ImplicitArgsUtils::NUM_IMPLICIT_ARGS;

  SYCLKernelMetadataAPI::KernelMetadataAPI KMD(F);
  SYCLKernelMetadataAPI::KernelInternalMetadataAPI KIMD(F);
  SmallVector<Type *, 16> ArgTypes;
  // TODO fix LIT which don't have ArgTypeNullValList yet.
  if (KIMD.ArgTypeNullValList.hasValue()) {
    assert((ArgsCount - KIMD.ArgTypeNullValList.size() <= 1) &&
           "arg count mismatch");
    llvm::transform(KIMD.ArgTypeNullValList, std::back_inserter(ArgTypes),
                    [](Constant *C) { return C->getType(); });
    // Masked kernel has an additional mask argument.
    if (ArgsCount > KIMD.ArgTypeNullValList.size())
      ArgTypes.push_back(F->getArg(ArgsCount - 1)->getType());
  } else {
    llvm::transform(F->args(), std::back_inserter(ArgTypes),
                    [](Argument &A) { return A.getType(); });
  }

  unsigned int CurrentOffset = 0;
  Function::arg_iterator arg_it = F->arg_begin();
  for (unsigned i = 0; i < ArgsCount; ++i) {
    KernelArgument CurArg;
    bool isMemoryObject = false;

    Argument *pArg = &*arg_it;
    // Set argument sizes and offsets
    switch (ArgTypes[i]->getTypeID()) {
    case Type::FloatTyID:
      CurArg.Ty = KRNL_ARG_FLOAT;
      CurArg.SizeInBytes = sizeof(float);
      break;

    case Type::StructTyID: {
      StructType *STy = cast<StructType>(arg_it->getType());
      CurArg.Ty = KRNL_ARG_COMPOSITE;
      DataLayout dataLayout(M);
      CurArg.SizeInBytes = dataLayout.getTypeAllocSize(STy);
      break;
    }
    case Type::PointerTyID: {
      // check kernel is block_invoke kernel
      // in that case 0 argument is block_literal pointer
      // update with special type
      // should be before handling ptrs by addr space
      PointerType *PTy = cast<PointerType>(arg_it->getType());
      if ((i == 0) && KIMD.BlockLiteralSize.hasValue()) {
        CurArg.Ty = KRNL_ARG_PTR_BLOCK_LITERAL;
        CurArg.SizeInBytes = KIMD.BlockLiteralSize.get();
        break;
      }

      if (pArg->hasByValAttr()) {
        // Check by pointer vector passing, used in long16 and double16
        if (auto *VTy = dyn_cast<FixedVectorType>(pArg->getParamByValType())) {
          unsigned int uiNumElem = (unsigned int)VTy->getNumElements();
          unsigned int uiElemSize =
              VTy->getContainedType(0)->getPrimitiveSizeInBits() / 8;
          CurArg.Ty = KRNL_ARG_VECTOR_BY_REF;
          CurArg.SizeInBytes = uiNumElem & 0xFFFF;
          CurArg.SizeInBytes |= (uiElemSize << 16);
          break;
        }
      }
      CurArg.SizeInBytes = M->getDataLayout().getPointerSize(0);
      // Detect pointer qualifier
      // Test for opaque types: images, queue_t, pipe_t
      std::string ArgTyStr;
      if (generatedFromSPIRV(*M)) {
        assert(KMD.ArgBaseTypeList.hasValue() &&
               "expect kernel_arg_base_type metadata");
        ArgTyStr = KMD.ArgBaseTypeList.getItem(i);
      }
      if (!ArgTyStr.empty()) {
        // TODO: Why default type is INTEGER????
        CurArg.Ty = KRNL_ARG_INT;
        StringRef ArgTyStrRef(ArgTyStr);
        std::ignore = ArgTyStrRef.consume_front("opencl.");
        // Get opencl opaque type.
        // It is safe to use startswith while there are no names which aren't
        // prefix of another name.
        if (ArgTyStrRef.startswith("image1d_array_"))
          CurArg.Ty = KRNL_ARG_PTR_IMG_1D_ARR;
        else if (ArgTyStrRef.startswith("image1d_buffer_"))
          CurArg.Ty = KRNL_ARG_PTR_IMG_1D_BUF;
        else if (ArgTyStrRef.startswith("image1d_"))
          CurArg.Ty = KRNL_ARG_PTR_IMG_1D;
        else if (ArgTyStrRef.startswith("image2d_depth_"))
          CurArg.Ty = KRNL_ARG_PTR_IMG_2D_DEPTH;
        else if (ArgTyStrRef.startswith("image2d_array_depth_"))
          CurArg.Ty = KRNL_ARG_PTR_IMG_2D_ARR_DEPTH;
        else if (ArgTyStrRef.startswith("image2d_array_"))
          CurArg.Ty = KRNL_ARG_PTR_IMG_2D_ARR;
        else if (ArgTyStrRef.startswith("image2d_"))
          CurArg.Ty = KRNL_ARG_PTR_IMG_2D;
        else if (ArgTyStrRef.startswith("image3d_"))
          CurArg.Ty = KRNL_ARG_PTR_IMG_3D;
        else if (ArgTyStrRef.startswith("pipe_ro_t"))
          CurArg.Ty = KRNL_ARG_PTR_PIPE_T;
        else if (ArgTyStrRef.startswith("pipe_wo_t"))
          CurArg.Ty = KRNL_ARG_PTR_PIPE_T;
        else if (ArgTyStrRef.startswith("queue_t"))
          CurArg.Ty = KRNL_ARG_PTR_QUEUE_T;
        else if (ArgTyStrRef.startswith("clk_event_t"))
          CurArg.Ty = KRNL_ARG_PTR_CLK_EVENT_T;
        else if (ArgTyStrRef.startswith("sampler_t"))
          CurArg.Ty = KRNL_ARG_PTR_SAMPLER_T;
        switch (CurArg.Ty) {
        case KRNL_ARG_PTR_IMG_1D:
        case KRNL_ARG_PTR_IMG_1D_ARR:
        case KRNL_ARG_PTR_IMG_1D_BUF:
        case KRNL_ARG_PTR_IMG_2D:
        case KRNL_ARG_PTR_IMG_2D_ARR:
        case KRNL_ARG_PTR_IMG_2D_DEPTH:
        case KRNL_ARG_PTR_IMG_2D_ARR_DEPTH:
        case KRNL_ARG_PTR_IMG_3D:
          // Setup image pointer
          isMemoryObject = true;
          break;
        case KRNL_ARG_PTR_PIPE_T:
          isMemoryObject = true;
          break;
        case KRNL_ARG_PTR_QUEUE_T:
        case KRNL_ARG_PTR_CLK_EVENT_T:
        case KRNL_ARG_PTR_SAMPLER_T:
          isMemoryObject = false;
          break;

        default:
          break;
        }
        // Check this is a special OpenCL C opaque type.
        if (KRNL_ARG_INT != CurArg.Ty)
          break;
      }

      // Calculate size for a struct pointer with byval attribute.
      if (pArg->hasByValAttr() && pArg->getParamByValType()->isStructTy()) {
        // struct or struct*
        // Deal with structs passed by value. These are user-defined structs and
        // ndrange_t.
        if (PTy->getAddressSpace() == 0) {
          StructType *STy = cast<StructType>(pArg->getParamByValType());
          assert(
              !STy->isOpaque() &&
              "cannot handle user-defined opaque types with an unknown size");
          DataLayout dataLayout(M);
          CurArg.SizeInBytes = dataLayout.getTypeAllocSize(STy);
          CurArg.Ty = KRNL_ARG_COMPOSITE;
          break;
        }
      }
      switch (PTy->getAddressSpace()) {
      case 0:
      case 1: // Global Address space
        CurArg.Ty = KRNL_ARG_PTR_GLOBAL;
        isMemoryObject = true;
        break;
      case 2:
        CurArg.Ty = KRNL_ARG_PTR_CONST;
        isMemoryObject = true;
        break;
      case 3: // Local Address space
        CurArg.Ty = KRNL_ARG_PTR_LOCAL;
        break;

      default:
        llvm_unreachable("unexpected address space for pointer type");
      }
    } break;

    case Type::TargetExtTyID: {
      auto *TETy = cast<TargetExtType>(ArgTypes[i]);
      CurArg.SizeInBytes =
          M->getDataLayout().getTypeAllocSize(TETy->getLayoutType());

      if (TETy->getName() == "spirv.DeviceEvent") {
        CurArg.Ty = KRNL_ARG_PTR_CLK_EVENT_T;
      } else if (TETy->getName() == "spirv.Image") {
        enum ImageIntParam : unsigned {
          ImageDim = 0,
          ImageDepth = 1,
          ImageArrayed = 2,
          ImageMultiSampled = 3,
          ImageSampled = 4,
          ImageFormat = 5,
          ImageAccess = 6
        };
        auto IntParams = TETy->int_params();
        if (IntParams[ImageArrayed]) {
          if (IntParams[ImageDepth] && IntParams[ImageDim] == 1)
            CurArg.Ty = KRNL_ARG_PTR_IMG_2D_ARR_DEPTH;
          else if (IntParams[ImageDim] == 1)
            CurArg.Ty = KRNL_ARG_PTR_IMG_2D_ARR;
          else if (IntParams[ImageDim] == 0)
            CurArg.Ty = KRNL_ARG_PTR_IMG_1D_ARR;
          else
            assert(false && "unhandled array image TargetExtType");
        } else if (IntParams[ImageDepth] && IntParams[ImageDim] == 2) {
          CurArg.Ty = KRNL_ARG_PTR_IMG_2D_DEPTH;
        } else if (IntParams[ImageDim] == 5) {
          CurArg.Ty = KRNL_ARG_PTR_IMG_1D_BUF;
        } else if (IntParams[ImageDim] == 2) {
          CurArg.Ty = KRNL_ARG_PTR_IMG_3D;
        } else if (IntParams[ImageDim] == 1) {
          CurArg.Ty = KRNL_ARG_PTR_IMG_2D;
        } else if (IntParams[ImageDim] == 0) {
          CurArg.Ty = KRNL_ARG_PTR_IMG_1D;
        } else {
          assert(false && "unhandled image TargetExtType");
        }
      } else if (TETy->getName() == "spirv.Sampler") {
        CurArg.Ty = KRNL_ARG_PTR_SAMPLER_T;
      } else if (TETy->getName() == "spirv.Pipe") {
        CurArg.Ty = KRNL_ARG_PTR_PIPE_T;
      } else if (TETy->getName() == "spirv.Queue") {
        CurArg.Ty = KRNL_ARG_PTR_QUEUE_T;
      } else {
        assert(false && "unhandled TargetExtType");
      }

      switch (CurArg.Ty) {
      case KRNL_ARG_PTR_IMG_1D:
      case KRNL_ARG_PTR_IMG_1D_ARR:
      case KRNL_ARG_PTR_IMG_1D_BUF:
      case KRNL_ARG_PTR_IMG_2D:
      case KRNL_ARG_PTR_IMG_2D_ARR:
      case KRNL_ARG_PTR_IMG_2D_DEPTH:
      case KRNL_ARG_PTR_IMG_2D_ARR_DEPTH:
      case KRNL_ARG_PTR_IMG_3D:
        // Setup image pointer
        isMemoryObject = true;
        break;
      case KRNL_ARG_PTR_PIPE_T:
        isMemoryObject = true;
        break;
      case KRNL_ARG_PTR_QUEUE_T:
      case KRNL_ARG_PTR_CLK_EVENT_T:
      case KRNL_ARG_PTR_SAMPLER_T:
        isMemoryObject = false;
        break;

      default:
        break;
      }
    } break;

    case Type::IntegerTyID: {
      if (KMD.ArgBaseTypeList.hasValue() &&
          KMD.ArgBaseTypeList.getItem(i) == SAMPLER) {
        CurArg.Ty = KRNL_ARG_SAMPLER;
        CurArg.SizeInBytes = sizeof(_sampler_t);
      } else {
        IntegerType *ITy = cast<IntegerType>(arg_it->getType());
        CurArg.Ty = KRNL_ARG_INT;
        CurArg.SizeInBytes = M->getDataLayout().getTypeAllocSize(ITy);
      }
    } break;

    case Type::DoubleTyID:
      CurArg.Ty = KRNL_ARG_DOUBLE;
      CurArg.SizeInBytes = sizeof(double);
      break;

    case Type::FixedVectorTyID: {
      FixedVectorType *Vector = cast<FixedVectorType>(arg_it->getType());
      CurArg.Ty = KRNL_ARG_VECTOR;
      CurArg.SizeInBytes = (unsigned int)(Vector->getNumElements() == 3
                                              ? 4
                                              : Vector->getNumElements());
      CurArg.SizeInBytes |=
          (Vector->getContainedType(0)->getPrimitiveSizeInBits() / 8) << 16;
    } break;

    default:
      llvm_unreachable("Unhandled parameter type");
    }

    // update offset
    assert(0 != CurArg.SizeInBytes && "argument size must be set");
    // Align current location to meet type's requirements
    CurrentOffset = TypeAlignment::align(TypeAlignment::getAlignment(CurArg),
                                         CurrentOffset);
    CurArg.OffsetInBytes = CurrentOffset;
    // Advance offset beyond this argument
    CurrentOffset += TypeAlignment::getSize(CurArg);

    if (isMemoryObject) {
      MemArguments.push_back(i);
    }
    Arguments.push_back(CurArg);
    ++arg_it;
  }
}

StringRef stripStructNameTrailingDigits(StringRef TyName) {
  // remove a '.' followed by any number of digits
  size_t Dot = TyName.find_last_of('.');
  if (TyName.npos != Dot &&
      TyName.npos == TyName.find_first_not_of("0123456789", Dot + 1))
    return TyName.substr(0, Dot);

  return TyName;
}

void updateFunctionMetadata(Module *M,
                            DenseMap<Function *, Function *> &FunctionMap) {
  // Update the references in Function metadata.
  // All the function metadata we are interested in is flat by design
  // (see Metadata API).

  // iterate over the functions we need update metadata for
  // (in other words, all the functions pass have created)
  for (const auto &FuncKV : FunctionMap) {
    auto F = FuncKV.second;
    SmallVector<std::pair<unsigned, MDNode *>, 8> MDs;
    F->getAllMetadata(MDs);

    for (const auto &MD : MDs) {
      auto MDNode = MD.second;
      if (MDNode->getNumOperands() > 0) {
        Metadata *MDOp = MDNode->getOperand(0);
        if (auto *FuncAsMD = dyn_cast_or_null<ConstantAsMetadata>(MDOp))
          if (auto *NodeFunc = mdconst::dyn_extract<Function>(FuncAsMD)) {
            if (FunctionMap.count(NodeFunc) > 0)
              MDNode->replaceOperandWith(
                  0, ConstantAsMetadata::get(FunctionMap[NodeFunc]));
          }
      }
    }
  }

  // Now respect the Module-level metadata.
  for (const auto &NamedMDNode : M->named_metadata()) {
    for (int ui = 0, ue = NamedMDNode.getNumOperands(); ui < ue; ui++) {
      // Replace metadata with metadata containing information about the wrapper
      MDNode *MDNodeOp = NamedMDNode.getOperand(ui);
      SmallSet<MDNode *, 8> Visited;
      updateMetadataTreeWithNewFuncs(M, FunctionMap, MDNodeOp, Visited);
    }
  }
}

void updateMetadataTreeWithNewFuncs(
    Module *M, DenseMap<Function *, Function *> &FunctionMap,
    MDNode *MDTreeNode, SmallSet<MDNode *, 8> &Visited) {
  // Avoid infinite loops due to possible cycles in metadata
  if (Visited.count(MDTreeNode))
    return;
  Visited.insert(MDTreeNode);

  for (int i = 0, e = MDTreeNode->getNumOperands(); i < e; ++i) {
    Metadata *MDOp = MDTreeNode->getOperand(i);
    if (!MDOp)
      continue;
    if (MDNode *MDOpNode = dyn_cast<MDNode>(MDOp)) {
      updateMetadataTreeWithNewFuncs(M, FunctionMap, MDOpNode, Visited);
    } else if (ConstantAsMetadata *FuncAsMD =
                   dyn_cast<ConstantAsMetadata>(MDOp)) {
      if (auto *MDNodeFunc = mdconst::dyn_extract<Function>(FuncAsMD)) {
        if (FunctionMap.count(MDNodeFunc) > 0)
          MDTreeNode->replaceOperandWith(
              i, ConstantAsMetadata::get(FunctionMap[MDNodeFunc]));
        // TODO: Check if the old metadata has to bee deleted manually to
        // avoid memory leaks.
      }
    }
  }
}

Instruction *createInstructionFromConstantWithReplacement(
    Constant *Original, Value *From, Value *To, Instruction *InsertPoint) {
  if (auto *CE = dyn_cast<ConstantExpr>(Original)) {
    auto *Inst = CE->getAsInstruction();
    Inst->insertBefore(InsertPoint);
    Inst->setDebugLoc(InsertPoint->getDebugLoc());
    Inst->replaceUsesOfWith(From, To);
    return Inst;
  }

  auto *CA = cast<ConstantAggregate>(Original);
  IRBuilder<> B(InsertPoint);
  Value *V = UndefValue::get(CA->getType());
  for (unsigned I = 0; I < CA->getNumOperands(); ++I) {
    Value *Op = CA->getOperand(I);
    if (Op == From)
      Op = To;
    if (isa<ConstantVector>(CA))
      V = B.CreateInsertElement(V, Op, I, "insert.vec.element." + Twine(I));
    else // insertvalue for Struct or Array
      V = B.CreateInsertValue(V, Op, I, "insert.agg.value." + Twine(I));
  }
  return cast<Instruction>(V);
}

bool hasFunctionCallInCGNodeIf(CallGraphNode *Node,
                               function_ref<bool(const Function *)> Condition) {
  // Always skips the root node by starting from ++df_begin().
  for (auto It = ++df_begin(Node); It != df_end(Node); ++It)
    if (Condition(It->getFunction()))
      return true;
  return false;
}

void mapFunctionCallInCGNodeIf(CallGraphNode *Node,
                               function_ref<bool(const Function *)> Condition,
                               function_ref<void(Function *)> MapFunc) {
  // Always skips the root node by starting from ++df_begin().
  for (auto It = ++df_begin(Node); It != df_end(Node); ++It) {
    Function *CalledFunc = It->getFunction();
    if (Condition(CalledFunc))
      MapFunc(CalledFunc);
  }
}

#define PRIM_TYPE(prim_type_enum)                                              \
  (new reflection::PrimitiveType(prim_type_enum))
#define PRIM_POINTER_TYPE(pointee_type, attrs)                                 \
  (new reflection::PointerType(                                                \
      PRIM_TYPE(pointee_type),                                                 \
      std::vector<reflection::TypeAttributeEnum> attrs))
#define CONST_GLOBAL_PTR(pointee_type)                                         \
  PRIM_POINTER_TYPE(pointee_type,                                              \
                    ({reflection::ATTR_GLOBAL, reflection::ATTR_CONST}))
#define GLOBAL_PTR(pointee_type)                                               \
  PRIM_POINTER_TYPE(pointee_type, ({reflection::ATTR_GLOBAL}))
#define VECTOR_TYPE(element_type, len)                                         \
  (new reflection::VectorType(element_type, len))
#define INT2_TYPE VECTOR_TYPE(PRIM_TYPE(reflection::PRIMITIVE_INT), 2)

static reflection::TypeVector
widenParameters(const reflection::TypeVector ScalarParams, unsigned int VF) {
  reflection::TypeVector VectorParams;
  for (auto &Param : ScalarParams) {
    if (auto *VecParam = dyn_cast<reflection::VectorType>(Param.get())) {
      int widen_len = VecParam->getLength() * VF;
      VectorParams.emplace_back(
          VECTOR_TYPE(VecParam->getScalarType(), widen_len));
    } else {
      VectorParams.emplace_back(VECTOR_TYPE(Param, VF));
    }
  }

  return VectorParams;
}

using VectInfoList =
    std::vector<std::tuple<std::string, std::string, std::string>>;

// Container storing all the vector info entries.
// Each entry would be a tuple of three strings:
// 1. scalar variant name
// 2. "kernel-call-once" | ""
// 3. mangled vector variant name
static VectInfoList ExtendedVectInfos;

VectInfoList &getExtendedVectInfos() { return ExtendedVectInfos; }

static void
pushSGBlockBuiltinDivergentVectInfo(const Twine &BaseName, unsigned int len,
                                    unsigned int VF,
                                    reflection::TypeVector ScalarParams) {
  // Get mangled name of scalar variant
  reflection::FunctionDescriptor ScalarFunc{
      (BaseName + (len == 1 ? "" : Twine(len))).str(), ScalarParams,
      reflection::width::SCALAR};
  std::string ScalarMangleName = NameMangleAPI::mangle(ScalarFunc);

  // Get mangled name of vector variant
  reflection::TypeVector VectorParams = widenParameters(ScalarParams, VF);
  size_t v_num = VectorParams.size();
  // Add mask param
  auto *Mask = VECTOR_TYPE(PRIM_TYPE(reflection::PRIMITIVE_UINT), VF);
  VectorParams.push_back(Mask);
  reflection::FunctionDescriptor VectorFunc{
      (BaseName + Twine(len) + "_" + Twine(VF)).str(), VectorParams,
      reflection::width::NONE};
  std::string VectorMangleName = NameMangleAPI::mangle(VectorFunc);

  std::vector<llvm::VFParamKind> ParamKinds(v_num, llvm::VFParamKind::Vector);

#if INTEL_CUSTOMIZATION
  auto Variant = VFInfo::get(llvm::VFISAKind::SSE, true, VF, ParamKinds,
                             ScalarMangleName, VectorMangleName);

  ExtendedVectInfos.push_back({ScalarMangleName,
                               std::string(KernelAttribute::CallOnce),
                               std::move(Variant.FullName)});
#endif // end INTEL_CUSTOMIZATION
}

static void pushSGBlockBuiltinDivergentVectInfo(
    StringRef TySuffix, reflection::TypePrimitiveEnum Ty,
    std::vector<unsigned int> Lens, std::vector<unsigned int> VFs) {
  const Twine SG_BLOCK_READ_PREFIX("intel_sub_group_block_read");
  const Twine SG_BLOCK_WRITE_PREFIX("intel_sub_group_block_write");

  for (unsigned int Len : Lens) {
    for (unsigned int VF : VFs) {
      // sub_group_block_read(const __global T*)
      pushSGBlockBuiltinDivergentVectInfo(SG_BLOCK_READ_PREFIX + TySuffix, Len,
                                          VF, {CONST_GLOBAL_PTR(Ty)});
      // sub_group_block_read(readonly image2d_t, int2)
      pushSGBlockBuiltinDivergentVectInfo(
          SG_BLOCK_READ_PREFIX + TySuffix, Len, VF,
          {PRIM_TYPE(reflection::PRIMITIVE_IMAGE_2D_RO_T), INT2_TYPE});
      // sub_group_block_read(readwrite image2d_t, int2)
      pushSGBlockBuiltinDivergentVectInfo(
          SG_BLOCK_READ_PREFIX + TySuffix, Len, VF,
          {PRIM_TYPE(reflection::PRIMITIVE_IMAGE_2D_RW_T), INT2_TYPE});

      if (Len == 1) {
        // sub_group_block_write(__global T*, T)
        pushSGBlockBuiltinDivergentVectInfo(SG_BLOCK_WRITE_PREFIX + TySuffix,
                                            Len, VF,
                                            {GLOBAL_PTR(Ty), PRIM_TYPE(Ty)});
        // sub_group_block_write(writeonly image2d_t, int2, T)
        pushSGBlockBuiltinDivergentVectInfo(
            SG_BLOCK_WRITE_PREFIX + TySuffix, Len, VF,
            {PRIM_TYPE(reflection::PRIMITIVE_IMAGE_2D_WO_T), INT2_TYPE,
             PRIM_TYPE(Ty)});
        // sub_group_block_write(readwrite image2d_t, int2, T)
        pushSGBlockBuiltinDivergentVectInfo(
            SG_BLOCK_WRITE_PREFIX + TySuffix, Len, VF,
            {PRIM_TYPE(reflection::PRIMITIVE_IMAGE_2D_RW_T), INT2_TYPE,
             PRIM_TYPE(Ty)});
      } else {
        // sub_group_block_write(__global T*, T<Len>)
        pushSGBlockBuiltinDivergentVectInfo(
            SG_BLOCK_WRITE_PREFIX + TySuffix, Len, VF,
            {GLOBAL_PTR(Ty), VECTOR_TYPE(PRIM_TYPE(Ty), Len)});
        // sub_group_block_write(writeonly image2d_t, int2, T<Len>)
        pushSGBlockBuiltinDivergentVectInfo(
            SG_BLOCK_WRITE_PREFIX + TySuffix, Len, VF,
            {PRIM_TYPE(reflection::PRIMITIVE_IMAGE_2D_WO_T), INT2_TYPE,
             VECTOR_TYPE(PRIM_TYPE(Ty), Len)});
        // sub_group_block_write(readwrite image2d_t, int2, T<Len>)
        pushSGBlockBuiltinDivergentVectInfo(
            SG_BLOCK_WRITE_PREFIX + TySuffix, Len, VF,
            {PRIM_TYPE(reflection::PRIMITIVE_IMAGE_2D_RW_T), INT2_TYPE,
             VECTOR_TYPE(PRIM_TYPE(Ty), Len)});
      }
    }
  }
}

#if INTEL_CUSTOMIZATION
static void pushSGRowSliceBuiltinVectInfo() {
  const static SmallVector<StringRef, 6> DataTypes = {"i8",   "i16", "i32",
                                                      "bf16", "f32", "f16"};
  const static SmallVector<unsigned, 5> VFs = {4, 8, 16, 32, 64};
  for (StringRef DataType : DataTypes) {
    for (unsigned VF : VFs) {
      // e.g. i32 @sub_group_rowslice_extractelement.i32(i64 %rowslice.id)
      std::string ExtractElementBaseName =
          (NAME_SUB_GROUP_ROWSLICE_EXTRACTELEMENT + "." + DataType).str();
      // e.g. void @sub_group_rowslice_insertelement.i32(i64 %rowslice.id, i32
      // %data)
      std::string InsertElementBaseName =
          (NAME_SUB_GROUP_ROWSLICE_INSERTELEMENT + "." + DataType).str();
      // The first argument %rowslice.id is always uniform.
      ExtendedVectInfos.push_back(
          {ExtractElementBaseName, std::string(KernelAttribute::CallOnce),
           ("_ZGVbN" + Twine(VF) + "u_" + ExtractElementBaseName).str()});
      ExtendedVectInfos.push_back(
          {ExtractElementBaseName, std::string(KernelAttribute::CallOnce),
           ("_ZGVbM" + Twine(VF) + "u_" + ExtractElementBaseName).str()});
      ExtendedVectInfos.push_back(
          {InsertElementBaseName, std::string(KernelAttribute::CallOnce),
           ("_ZGVbN" + Twine(VF) + "uv_" + InsertElementBaseName).str()});
      ExtendedVectInfos.push_back(
          {InsertElementBaseName, std::string(KernelAttribute::CallOnce),
           ("_ZGVbM" + Twine(VF) + "uv_" + InsertElementBaseName).str()});
    }
  }
}
#endif // INTEL_CUSTOMIZATION

reflection::TypePrimitiveEnum getPrimitiveTypeOfString(StringRef T) {
  return llvm::StringSwitch<reflection::TypePrimitiveEnum>(T)
      .Case("i8", reflection::PRIMITIVE_CHAR)
      .Case("u8", reflection::PRIMITIVE_UCHAR)
      .Case("i16", reflection::PRIMITIVE_SHORT)
      .Case("u16", reflection::PRIMITIVE_USHORT)
      .Case("i32", reflection::PRIMITIVE_INT)
      .Case("u32", reflection::PRIMITIVE_UINT)
      .Case("i64", reflection::PRIMITIVE_LONG)
      .Case("u64", reflection::PRIMITIVE_ULONG)
      .Case("f16", reflection::PRIMITIVE_HALF)
      .Case("f32", reflection::PRIMITIVE_FLOAT)
      .Case("f64", reflection::PRIMITIVE_DOUBLE)
      .Default(reflection::PRIMITIVE_NONE);
}

static void
pushWGSortBuiltinVectInfo(const reflection::TypeVector &KeyValueParams,
                          const reflection::TypeVector &OtherParams,
                          const SmallVector<llvm::VFParamKind, 4> &ParamKinds,
                          StringRef BuiltinName, StringRef BuiltinMangledName) {
  const static SmallVector<unsigned, 5> VFs = {4, 8, 16, 32, 64};
  for (unsigned VF : VFs) {
    // Get params type of vector builtin
    reflection::TypeVector VectorParams = widenParameters(KeyValueParams, VF);
    VectorParams.insert(VectorParams.end(), OtherParams.begin(),
                        OtherParams.end());
    // Add mask param type
    auto *Mask = VECTOR_TYPE(PRIM_TYPE(reflection::PRIMITIVE_UINT), VF);
    VectorParams.push_back(Mask);

    // Get mangled name of vector builtin
    reflection::FunctionDescriptor VectorFunc{BuiltinName.data(), VectorParams,
                                              reflection::width::NONE};
    std::string VectorName = NameMangleAPI::mangle(VectorFunc);
    auto Variant = VFInfo::get(llvm::VFISAKind::SSE, true, VF, ParamKinds,
                               BuiltinMangledName, VectorName);
    ExtendedVectInfos.push_back({BuiltinMangledName.data(),
                                 std::string(KernelAttribute::CallOnce),
                                 std::move(Variant.FullName)});
  }
}

static void pushWGSortBuiltinVectorInfo(const Module &M) {
  SmallVector<StringRef, 6> AllWorkGroupSortBuiltinBasicNames;
  for (auto &F : M) {
    if (!F.isDeclaration())
      continue;
    StringRef FnName = F.getName();
    if (!isWorkGroupSort(FnName))
      continue;

    reflection::FunctionDescriptor SortFD = demangle(FnName);
    reflection::TypeVector DataScalarParams, OtherScalarParams;
    SmallVector<llvm::VFParamKind, 4> ParamKinds;

    // e.g.
    // key-only sort : void
    // __devicelib_default_work_group_joint_sort_ascending_p1i32_u32_p3i8(int*
    // key, uint n, byte* scratch);
    // The first arg needs to be widen.
    // key-value sort : void
    // __devicelib_default_work_group_joint_sort_ascending_p3u32_p3u32_u32_p1i8(uint*
    // key, uint* value, uint n, byte* scratch);
    // The first and second args need to be widen.
    unsigned int DataArgNum = isWorkGroupKeyOnlySort(FnName) ? 1 : 2;
    assert(DataArgNum == SortFD.Parameters.size() - 2 &&
           "Unknown work group sort builtin");
    for (unsigned int Idx = 0; Idx < DataArgNum; ++Idx) {
      DataScalarParams.push_back(SortFD.Parameters[Idx]);
      ParamKinds.push_back(llvm::VFParamKind::Vector);
    }
    for (unsigned int Idx = DataArgNum; Idx < SortFD.Parameters.size(); ++Idx) {
      ParamKinds.push_back(llvm::VFParamKind::OMP_Uniform);
      OtherScalarParams.push_back(SortFD.Parameters[Idx]);
    }
    pushWGSortBuiltinVectInfo(DataScalarParams, OtherScalarParams, ParamKinds,
                              SortFD.Name, FnName);
  }
}

static void initializeVectInfoOnce(ArrayRef<VectItem> VectInfos) {
  // Load Table-Gen'erated VectInfo.gen
  if (!VectInfos.empty()) {
    ExtendedVectInfos.insert(ExtendedVectInfos.end(), std::begin(VectInfos),
                             std::end(VectInfos));
  } else if (OptVectInfoFile.getNumOccurrences()) {
    static ErrorOr<std::unique_ptr<MemoryBuffer>> BufOrErr =
        MemoryBuffer::getFile(OptVectInfoFile, /* IsText */ true);
    if (BufOrErr) {
      SmallVector<StringRef, 0> Items;
      Items.reserve(8192 * 3);
      SplitString(BufOrErr.get()->getBuffer(), Items, " \t\n\v\f\r,{}");
      assert(Items.size() % 3 == 0 &&
             "Invalid number of items in VectInfo.gen");
      for (size_t I = 0; I < Items.size(); I += 3)
        ExtendedVectInfos.emplace_back(Items[I].trim('\"'),
                                       Items[I + 1].trim('\"'),
                                       Items[I + 2].trim('\"'));
    }
  }

  // Add extra vector info for 'sub_group_ballot'
  ExtendedVectInfos.push_back(
      {"intel_sub_group_ballot", std::string(KernelAttribute::CallOnce),
       "_ZGVbM4v_intel_sub_group_balloti(intel_sub_group_ballot_vf4)"});
  ExtendedVectInfos.push_back(
      {"intel_sub_group_ballot", std::string(KernelAttribute::CallOnce),
       "_ZGVbM8v_intel_sub_group_balloti(intel_sub_group_ballot_vf8)"});
  ExtendedVectInfos.push_back(
      {"intel_sub_group_ballot", std::string(KernelAttribute::CallOnce),
       "_ZGVbM16v_intel_sub_group_balloti(intel_sub_group_ballot_vf16)"});

  // Add extra vector info for 'sub_group_block_read*', 'sub_group_block_write*'
  std::vector<std::tuple<std::string, reflection::TypePrimitiveEnum,
                         std::vector<unsigned int>, std::vector<unsigned int>>>
      Entries{
          {"", reflection::PRIMITIVE_UINT, {1, 2, 4, 8}, {4, 8, 16, 32, 64}},
          {"_uc",
           reflection::PRIMITIVE_UCHAR,
           {1, 2, 4, 8, 16},
           {4, 8, 16, 32, 64}},
          {"_us",
           reflection::PRIMITIVE_USHORT,
           {1, 2, 4, 8},
           {4, 8, 16, 32, 64}},
          {"_ui", reflection::PRIMITIVE_UINT, {1, 2, 4, 8}, {4, 8, 16, 32, 64}},
          {"_ul",
           reflection::PRIMITIVE_ULONG,
           {1, 2, 4, 8},
           {4, 8, 16, 32, 64}},
      };
  for (auto &Entry : Entries) {
    pushSGBlockBuiltinDivergentVectInfo(std::get<0>(Entry), std::get<1>(Entry),
                                        std::get<2>(Entry), std::get<3>(Entry));
  }
#if INTEL_CUSTOMIZATION
  // Add extra vector info for 'sub_group_rowslice_extractelement.*' and
  // 'sub_group_rowslice_insertelement.*'
  pushSGRowSliceBuiltinVectInfo();
#endif // INTEL_CUSTOMIZATION
}

static llvm::once_flag InitializeVectInfoFlag;

void initializeVectInfo(ArrayRef<VectItem> VectInfos, const Module &M) {
  // Load all vector info into ExtendedVectInfos, at most once.
  llvm::call_once(InitializeVectInfoFlag,
                  [&]() { initializeVectInfoOnce(VectInfos); });

  // Add extra vector info for work group sort builtin that is used in the
  // module. It can't be initialized just once, because there could be multiple
  // modules in the application.
  static SmallPtrSet<const Module *, 8> VisitedModules;
  if (VisitedModules.insert(&M).second)
    pushWGSortBuiltinVectorInfo(M);
}

static std::string getFormatStr(Value *V) {
  Type *T = V->getType();
  std::string Name = V->getName().str();

  if (T->isIntegerTy(32))
    return Name + ": %d ";
  if (T->is16bitFPTy())
    return Name + ": (half/bf16 hex) 0x%X ";
  if (T->isFloatTy())
    return Name + ": %f ";
  if (T->isDoubleTy())
    return Name + ": %lf ";
  if (T->isPointerTy())
    return Name + "%p";
  llvm_unreachable("Can't print this value");
}

void insertPrintf(const Twine &Prefix, IRBuilder<> &Builder,
                  ArrayRef<Value *> Inputs, ArrayRef<StringRef> InputPrefixes) {
  auto *BB = Builder.GetInsertBlock();
  auto &Context = BB->getContext();
  unsigned StrAddrSpace = 2;

  // Declare printf function
  auto *StrType = PointerType::get(Type::getInt8Ty(Context), StrAddrSpace);
  SmallVector<Type *, 1> ArgList{StrType};
  auto *FuncType = FunctionType::get(Type::getInt32Ty(Context), ArgList, true);
  FunctionCallee PrintFuncConst =
      BB->getModule()->getOrInsertFunction("printf", FuncType);
  auto *PrintFunc = cast<Function>(PrintFuncConst.getCallee());

  SmallVector<Value *, 16> TempInputs;
  SmallVector<std::string, 16> TempInputPrefixes;
  auto FetchInputPrefix = [&]() {
    return InputPrefixes.empty() ? ""
                                 : ("[" + InputPrefixes.front().str() + "]");
  };
  for (auto *I : Inputs) {
    if (auto *T = dyn_cast<FixedVectorType>(I->getType())) {
      unsigned Len = T->getNumElements();
      StringRef Name = I->getName();
      for (unsigned Idx = 0; Idx < Len; ++Idx) {
        auto *Ele =
            Builder.CreateExtractElement(I, Idx, Name + "." + Twine(Idx));
        TempInputs.push_back(Ele);
        // Add "[prefix][idx]" for each vector element
        TempInputPrefixes.push_back(FetchInputPrefix() + "[" +
                                    Twine(Idx).str() + "]");
      }
    } else {
      TempInputs.push_back(I);
      TempInputPrefixes.push_back(FetchInputPrefix());
    }
    // Drop used input prefix
    if (!InputPrefixes.empty())
      InputPrefixes = InputPrefixes.drop_front();
  }
  assert(TempInputs.size() == TempInputPrefixes.size() &&
         "Number of prefixes for input doesn't match!");

  std::string FormatStr = "PRINT " + Prefix.str() + " ";
  SmallVector<Value *, 16> TempInputsCast;
  for (const auto &[I, V] : enumerate(TempInputs)) {
    Type *T = V->getType();
    Value *NewV = V;
    if (T->isIntegerTy())
      if (!T->isIntegerTy(32))
        NewV = Builder.CreateIntCast(V, Builder.getInt32Ty(), false,
                                     V->getName() + "cast.");
    FormatStr += TempInputPrefixes[I];
    FormatStr += getFormatStr(NewV);
    if (T->is16bitFPTy())
      NewV = Builder.CreateBitCast(NewV, Builder.getInt16Ty(),
                                   NewV->getName() + "cast.");
    TempInputsCast.push_back(NewV);
  }
  FormatStr += "\n";

  Constant *StrVal =
      ConstantDataArray::getString(BB->getContext(), FormatStr, true);
  auto *ArrayType = StrVal->getType();
  auto *FormatStrGV = new GlobalVariable(
      *BB->getModule(), ArrayType, true, GlobalValue::InternalLinkage, StrVal,
      "format.str.", 0, GlobalVariable::NotThreadLocal, StrAddrSpace);

  auto *StrPtr = Builder.CreateConstGEP2_32(ArrayType, FormatStrGV, 0, 0);

  // Insert call.
  SmallVector<Value *, 4> Args{StrPtr};
  Args.append(TempInputsCast.begin(), TempInputsCast.end());
  Builder.CreateCall(PrintFunc, Args, "PRINT.");
}

void insertPrintf(const Twine &Prefix, Instruction *IP,
                  ArrayRef<Value *> Inputs, ArrayRef<StringRef> InputPrefixes) {
  IRBuilder<> Builder(IP);
  insertPrintf(Prefix, Builder, Inputs, InputPrefixes);
}

void insertPrintf(const Twine &Prefix, BasicBlock *BB, ArrayRef<Value *> Inputs,
                  ArrayRef<StringRef> InputPrefixes) {
  IRBuilder<> Builder(BB);
  insertPrintf(Prefix, Builder, Inputs, InputPrefixes);
}

bool isValidMatrixType(FixedVectorType *MatrixType) {
  Type *DataType = MatrixType->getElementType();
  switch (DataType->getTypeID()) {
  case Type::IntegerTyID:
    switch (DataType->getIntegerBitWidth()) {
    case 8:
    case 16: // bf16 is implemented using i16 in DPC++ header
    case 32:
      return true;
    default:
      return false;
    }
  case Type::FloatTyID:
  case Type::BFloatTyID:
  case Type::HalfTyID:
    return true;
  default:
    return false;
  }
}

/// Copied from llvm/lib/IR/Function.cpp:813
/// Returns a stable mangling for the type specified for use in the name
/// mangling scheme used by 'any' types in intrinsic signatures.  The mangling
/// of named types is simply their name.  Manglings for unnamed types consist
/// of a prefix ('p' for pointers, 'a' for arrays, 'f_' for functions)
/// combined with the mangling of their component types.  A vararg function
/// type will have a suffix of 'vararg'.  Since function types can contain
/// other function types, we close a function type mangling with suffix 'f'
/// which can't be confused with it's prefix.  This ensures we don't have
/// collisions between two unrelated function types. Otherwise, you might
/// parse ffXX as f(fXX) or f(fX)X.  (X is a placeholder for any other type.)
/// The HasUnnamedType boolean is set if an unnamed type was encountered,
/// indicating that extra care must be taken to ensure a unique name.
static std::string getMangledTypeStr(Type *Ty, bool &HasUnnamedType) {
  std::string Result;
  if (PointerType *PTyp = dyn_cast<PointerType>(Ty)) {
    Result += "p" + utostr(PTyp->getAddressSpace());
  } else if (ArrayType *ATyp = dyn_cast<ArrayType>(Ty)) {
    Result += "a" + utostr(ATyp->getNumElements()) +
              getMangledTypeStr(ATyp->getElementType(), HasUnnamedType);
  } else if (StructType *STyp = dyn_cast<StructType>(Ty)) {
    if (!STyp->isLiteral()) {
      Result += "s_";
      if (STyp->hasName())
        Result += STyp->getName();
      else
        HasUnnamedType = true;
    } else {
      Result += "sl_";
      for (auto *Elem : STyp->elements())
        Result += getMangledTypeStr(Elem, HasUnnamedType);
    }
    // Ensure nested structs are distinguishable.
    Result += "s";
  } else if (FunctionType *FT = dyn_cast<FunctionType>(Ty)) {
    Result += "f_" + getMangledTypeStr(FT->getReturnType(), HasUnnamedType);
    for (size_t i = 0; i < FT->getNumParams(); i++)
      Result += getMangledTypeStr(FT->getParamType(i), HasUnnamedType);
    if (FT->isVarArg())
      Result += "vararg";
    // Ensure nested function types are distinguishable.
    Result += "f";
  } else if (VectorType *VTy = dyn_cast<VectorType>(Ty)) {
    ElementCount EC = VTy->getElementCount();
    if (EC.isScalable())
      Result += "nx";
    Result += "v" + utostr(EC.getKnownMinValue()) +
              getMangledTypeStr(VTy->getElementType(), HasUnnamedType);
  } else if (Ty) {
    switch (Ty->getTypeID()) {
    default:
      llvm_unreachable("Unhandled type");
    case Type::VoidTyID:
      Result += "isVoid";
      break;
    case Type::MetadataTyID:
      Result += "Metadata";
      break;
    case Type::HalfTyID:
      Result += "f16";
      break;
    case Type::BFloatTyID:
      Result += "bf16";
      break;
    case Type::FloatTyID:
      Result += "f32";
      break;
    case Type::DoubleTyID:
      Result += "f64";
      break;
    case Type::X86_FP80TyID:
      Result += "f80";
      break;
    case Type::FP128TyID:
      Result += "f128";
      break;
    case Type::PPC_FP128TyID:
      Result += "ppcf128";
      break;
    case Type::X86_MMXTyID:
      Result += "x86mmx";
      break;
    case Type::X86_AMXTyID:
      Result += "x86amx";
      break;
    case Type::IntegerTyID:
      Result += "i" + utostr(cast<IntegerType>(Ty)->getBitWidth());
      break;
    }
  }
  return Result;
}

static CallInst *generateCall(Module *M, StringRef FnName, Type *ReturnType,
                              ArrayRef<Value *> Args, IRBuilder<> &Builder,
                              const Twine &Name = "",
                              AttributeList AttrList = AttributeList()) {
  SmallVector<Type *> ArgTypes;
  for (auto *Arg : Args)
    ArgTypes.push_back(Arg->getType());
  auto *FuncType = FunctionType::get(ReturnType, ArgTypes, false);
  auto Func = M->getOrInsertFunction(FnName, FuncType, AttrList);
  return Builder.CreateCall(Func, Args, Name);
}

CallInst *createGetMaxSubGroupSizeCall(Instruction *IP, const Twine &Name) {
  IRBuilder<> Builder(IP);
  auto &Ctx = IP->getContext();
  auto AL = AttributeList()
                .addFnAttribute(Ctx, Attribute::getWithMemoryEffects(
                                         Ctx, MemoryEffects::none()))
                .addFnAttribute(Ctx, Attribute::NoUnwind)
                .addFnAttribute(Ctx, Attribute::WillReturn);
  return generateCall(IP->getModule(), mangledGetMaxSubGroupSize(),
                      Builder.getInt32Ty(), {}, Builder, Name, AL);
}

CallInst *createGetSubGroupLocalIdCall(Instruction *IP, const Twine &Name) {
  IRBuilder<> Builder(IP);
  auto &Ctx = IP->getContext();
  auto AL = AttributeList()
                .addFnAttribute(Ctx, Attribute::getWithMemoryEffects(
                                         Ctx, MemoryEffects::none()))
                .addFnAttribute(Ctx, Attribute::NoUnwind)
                .addFnAttribute(Ctx, Attribute::WillReturn);
  return generateCall(IP->getModule(), mangledGetSubGroupLocalId(),
                      Builder.getInt32Ty(), {}, Builder, Name, AL);
}

#if INTEL_CUSTOMIZATION
CallInst *createGetSubGroupSliceLengthCall(unsigned TotalElementCount,
                                           Instruction *IP, const Twine &Name) {
  IRBuilder<> Builder(IP);
  auto *Arg = Builder.getInt32(TotalElementCount);
  auto AL =
      AttributeList()
          .addFnAttribute(IP->getContext(),
                          Attribute::getWithMemoryEffects(IP->getContext(),
                                                          MemoryEffects::none()))
          .addFnAttribute(IP->getContext(), Attribute::NoUnwind)
          .addFnAttribute(IP->getContext(), Attribute::WillReturn)
          .addFnAttribute(IP->getContext(), KernelAttribute::ConvergentCall);
  return generateCall(IP->getModule(), NAME_GET_SUB_GROUP_SLICE_LENGTH,
                      Builder.getInt64Ty(), {Arg}, Builder, Name, AL);
}

CallInst *createGetSubGroupRowSliceIdCall(Value *Matrix, unsigned R, unsigned C,
                                          Value *Index, Instruction *IP,
                                          const Twine &Name) {
  auto *MatrixType = cast<FixedVectorType>(Matrix->getType());
  assert(isValidMatrixType(MatrixType) && "Unsupported matrix type");
  assert(MatrixType->getNumElements() == (R * C) &&
         "Matrix size doesn't match");
  IRBuilder<> Builder(IP);
  auto *Rows = Builder.getInt32(R);
  auto *Cols = Builder.getInt32(C);
  SmallVector<Value *> Args = {Matrix, Rows, Cols, Index};
  bool HasUnnamedType = false;
  std::string FnName = NAME_GET_SUB_GROUP_ROWSLICE_ID.str() + "." +
                       getMangledTypeStr(MatrixType, HasUnnamedType) + "." +
                       getMangledTypeStr(Index->getType(), HasUnnamedType);
  auto AL = AttributeList()
                .addFnAttribute(IP->getContext(), KernelAttribute::UniformCall)
                .addFnAttribute(IP->getContext(),
                                KernelAttribute::OCLVecUniformReturn);
  return generateCall(IP->getModule(), FnName, Builder.getInt64Ty(), Args,
                      Builder, Name, AL);
}

CallInst *createSubGroupRowSliceExtractElementCall(Value *RowSliceId,
                                                   Type *ReturnType,
                                                   Instruction *IP,
                                                   const Twine &Name) {
  IRBuilder<> Builder(IP);
  bool HasUnnamedType = false;
  std::string FnName = NAME_SUB_GROUP_ROWSLICE_EXTRACTELEMENT.str() + "." +
                       getMangledTypeStr(ReturnType, HasUnnamedType);
  auto AL = AttributeList().addFnAttribute(IP->getContext(),
                                           KernelAttribute::CallOnce);
  return generateCall(IP->getModule(), FnName, ReturnType, {RowSliceId},
                      Builder, Name, AL);
}

CallInst *createSubGroupRowSliceInsertElementCall(Value *RowSliceId,
                                                  Value *Data,
                                                  Instruction *IP) {
  IRBuilder<> Builder(IP);
  bool HasUnnamedType = false;
  std::string FnName = NAME_SUB_GROUP_ROWSLICE_INSERTELEMENT.str() + "." +
                       getMangledTypeStr(Data->getType(), HasUnnamedType);
  auto AL = AttributeList().addFnAttribute(IP->getContext(),
                                           KernelAttribute::CallOnce);
  return generateCall(IP->getModule(), FnName, Builder.getVoidTy(),
                      {RowSliceId, Data}, Builder, "", AL);
}

CallInst *createSubGroupInsertRowSliceToMatrixCall(Value *RowSliceId,
                                                   Type *ReturnMatrixType,
                                                   Instruction *IP,
                                                   const Twine &Name) {
  IRBuilder<> Builder(IP);
  bool HasUnnamedType = false;
  std::string FnName = NAME_SUB_GROUP_INSERT_ROWSLICE_TO_MATRIX.str() + "." +
                       getMangledTypeStr(ReturnMatrixType, HasUnnamedType);
  auto AL = AttributeList()
                .addFnAttribute(IP->getContext(), KernelAttribute::UniformCall)
                .addFnAttribute(IP->getContext(),
                                KernelAttribute::OCLVecUniformReturn);
  return generateCall(IP->getModule(), FnName, ReturnMatrixType, {RowSliceId},
                      Builder, Name, AL);
}
#endif // INTEL_CUSTOMIZATION

// Utility function for calculating private/local memory size with post order
// traversal.
void calculateMemorySizeWithPostOrderTraversal(
    CallGraph &CG, DenseMap<Function *, size_t> &FnDirectSize,
    DenseMap<Function *, size_t> &FnSize) {
  // The recursive function or function that calls no other functions will be
  // visited firstly and its private/local memory size will be calculated. And
  // then private/local memory size for its callers will be calculated.
  //
  // The steps for calculating private/local memory size:
  //   1. Visit F function in post order traversal of call graph.
  //     1.1. Visit CalledFunc functions called by F (in call graph node of F)
  //          and get maximum memory size for the CalledFunc functions.
  //       1.1.1. Get the cached memory size for CalledFunc, if it's calculated.
  //              Otherwise:
  //         1.2.1. Calculate its memory size for CalledFunc.
  //         1.2.2. If CalledFunc is recursive function, multiply its memory
  //                size by (MAX_RECURSION_DEPTH - 1).
  //       1.1.2. Compare the memory size for called functions and save maximum
  //              memory size.
  //     1.2. Add the maximum memory size for called functions to memory size
  //          for F function.
  //
  // Post order traversal examples with recursion function
  //   Example 1: test -> foo -> foo
  //   Steps
  //     1. Visit foo function
  //       1.1. Visit called functions: foo
  //         1.1.1. Calculate memory size for foo function and multiply it by
  //                (MAX_RECURSION_DEPTH - 1).
  //       1.2. Add it to memory size for foo function
  //     2. Visit test function
  //       2.1. Visit called functions: foo
  //         2.1.1. Get cached memory size for foo function
  //       2.2. Add it to memory size for test function
  //   Example 2: test -> foo -> bar -> foo -> bar
  //   Steps
  //     1. Visit bar function
  //       1.1. Visit called functions: foo
  //         1.1.1. Calculate memory size for foo function and multiply
  //                it by (MAX_RECURSION_DEPTH - 1).
  //       1.2. Add memory size for foo function to memory size for bar function
  //     2. Visit foo function
  //       2.1. Visit called functions: bar
  //         2.1.1. Multiply direct size by (MAX_RECURSION_DEPTH - 1) and add it
  //                to memory size for bar function
  //         2.1.2. Get cached memory size for bar function
  //       2.2. Add memory size for bar function to memory size for foo function
  //     3. Visit test function
  //       3.1. Visit called functions: foo
  //         3.1.1. Get cached memory size for foo function
  //       3.2. Add memory size for foo function to memory size for test
  //            function
  DenseSet<Function *> VisitedSet;
  for (auto I = po_begin(&CG), E = po_end(&CG); I != E; I++) {
    Function *F = I->getFunction();
    if (!F || F->isDeclaration())
      continue;
    size_t MaxSize = 0;
    const CallGraphNode *CGNode = CG[F];
    for (auto &CI : *CGNode) {
      Function *CalledFunc = CI.second->getFunction();
      if (!CalledFunc || CalledFunc->isDeclaration())
        continue;

      auto CalledFMD = SYCLKernelMetadataAPI::FunctionMetadataAPI(CalledFunc);
      bool IsRecursive =
          CalledFMD.RecursiveCall.hasValue() && CalledFMD.RecursiveCall.get();
      if (!FnSize.count(CalledFunc)) {
        assert(FnDirectSize.count(CalledFunc) && "No direct size calculated!");
        FnSize[CalledFunc] = FnDirectSize[CalledFunc];
        if (IsRecursive)
          FnSize[CalledFunc] *= (MAX_RECURSION_DEPTH - 1);
        VisitedSet.insert(CalledFunc);
      } else {
        if (!VisitedSet.count(CalledFunc)) {
          // Multiply direct size by (MAX_RECURSION_DEPTH - 1) and add it to
          // memory size for indirect recursion (foo -> bar -> foo -> bar)
          if (IsRecursive)
            FnSize[CalledFunc] +=
                FnDirectSize[CalledFunc] * (MAX_RECURSION_DEPTH - 1);
          VisitedSet.insert(CalledFunc);
        }
      }
      MaxSize = std::max(MaxSize, FnSize[CalledFunc]);
    }

    FnSize[F] = MaxSize + FnDirectSize[F];
  }
}

uint64_t getNumElementsOfNestedArray(const ArrayType *ArrTy) {
  uint64_t NumElements = ArrTy->getNumElements();
  while (auto *InnerArrayTy = dyn_cast<ArrayType>(ArrTy->getElementType())) {
    NumElements *= InnerArrayTy->getNumElements();
    ArrTy = InnerArrayTy;
  }
  return NumElements;
}

void patchNotInlinedTIDUserFunc(
    Module &M, IRBuilderBase &Builder, const FuncSet &KernelAndSyncFuncs,
    const InstVec &TIDCallsToFix,
    DenseMap<Function *, Value *> &PatchedFToLocalIds,
    PointerType *LocalIdAllocTy,
    function_ref<Value *(CallInst *CI)> CreateLIDArg) {
  FuncSet FuncsToPatch;
  SetVector<CallInst *> CIsToPatch;
  DenseMap<ConstantExpr *, Function *> ConstBitcastsToPatch;
  FuncVec WorkList;

  // Initialize the set of functions that need patching by selecting the
  // functions which contain direct calls to get_*_id() and are w/o syncs.
  for (auto *I : TIDCallsToFix) {
    Function *Caller = cast<CallInst>(I)->getFunction();
    FuncsToPatch.insert(Caller);
    WorkList.push_back(Caller);
  }

  // Traverse back the call graph and find the set of all functions which need
  // to be patched. Also find the corresponding call instructions.
  // Functions which need to be patched are either:
  // 1. Functions w/o sync instructions which are direct callers of get_*_id()
  //    (handled in the loop above).
  // 2. Functions which are direct callers of functions described in 1 or
  //    (recursively) functions defined in this line which do not contain sync
  //    instructions.
  SmallPtrSet<Function *, 8> Visited;
  while (!WorkList.empty()) {
    Function *F = WorkList.pop_back_val();
    Visited.insert(F);
    for (User *U : F->users()) {
      // OCL2.0 : handle constant expression with bitcast of function pointer.
      if (auto *CE = dyn_cast<ConstantExpr>(U)) {
        if ((CE->getOpcode() == Instruction::BitCast ||
             CE->getOpcode() == Instruction::AddrSpaceCast) &&
            CE->getType()->isPointerTy()) {
          ConstBitcastsToPatch[CE] = F;
          continue;
        }
      }

      auto *CI = dyn_cast<CallInst>(U);
      if (!CI)
        continue;
      CIsToPatch.insert(CI);
      Function *Caller = CI->getFunction();
      assert(Caller && "invalid caller");
      if (KernelAndSyncFuncs.contains(Caller))
        continue;
      FuncsToPatch.insert(Caller);
      if (!Visited.contains(Caller))
        WorkList.push_back(Caller);
    }
  }

  DenseMap<Function *, Function *> OldF2PatchedF;

  // Setup stuff needed for adding another argument to patched functions.
  auto &Ctx = M.getContext();
  auto NewAttrSet =
      AttributeSet::get(Ctx, {Attribute::get(Ctx, Attribute::NoAlias)});

  // Patch the functions.
  for (Function *OldF : FuncsToPatch) {
    Function *PatchedF = AddMoreArgsToFunc(OldF, LocalIdAllocTy, {"local.ids"},
                                           {NewAttrSet}, "patched");
    OldF2PatchedF[OldF] = PatchedF;
    PatchedFToLocalIds[PatchedF] = &*(PatchedF->arg_end() - 1);
  }

  // Patch the calls.
  for (CallInst *CI : CIsToPatch) {
    Function *Caller = CI->getFunction();
    Function *Callee = CI->getCalledFunction();
    assert(OldF2PatchedF.count(Callee) && "callee not found in the map");
    Function *PatchedF = OldF2PatchedF[Callee];
    auto It = PatchedFToLocalIds.find(Caller);
    Value *NewArg;
    if (It != PatchedFToLocalIds.end())
      NewArg = It->second;
    else
      NewArg = CreateLIDArg(CI);
    SmallVector<Value *, 1> NewArgs(1, NewArg);
    (void)AddMoreArgsToCall(CI, NewArgs, PatchedF);
  }

  // Patch the constant function ptr addr bitcasts. Used in OpenCL 2.0 extended
  // execution.
  for (auto &Pair : ConstBitcastsToPatch) {
    ConstantExpr *CE = Pair.first;
    Function *F = Pair.second;
    assert(OldF2PatchedF.count(F) && "patched function not found in the map");
    Function *PatchedF = OldF2PatchedF[F];

    // This case happens when global block variable is used.
    auto *NewCE = ConstantExpr::getPointerCast(PatchedF, CE->getType());
    CE->replaceAllUsesWith(NewCE);
  }

  // Erase the functions since they're replaced with the ones patched.
  for (Function *OldF : FuncsToPatch)
    OldF->eraseFromParent();
}

bool hasFDivWithFastFlag(Module *M) {
  for (Function &F : *M)
    for (BasicBlock &B : F)
      for (Instruction &I : B)
        if (I.getOpcode() == Instruction::FDiv && I.isFast())
          return true;

  return false;
}

const char *ImageTypeNames[] = {"opencl.image1d_ro_t",
                                "opencl.image1d_array_ro_t",
                                "opencl.image1d_wo_t",
                                "opencl.image1d_array_wo_t",
                                "opencl.image1d_rw_t",
                                "opencl.image1d_array_rw_t",
                                "opencl.image2d_ro_t",
                                "opencl.image1d_buffer_ro_t",
                                "opencl.image2d_wo_t",
                                "opencl.image1d_buffer_wo_t",
                                "opencl.image2d_rw_t",
                                "opencl.image1d_buffer_rw_t",
                                "opencl.image2d_array_ro_t",
                                "opencl.image2d_depth_ro_t",
                                "opencl.image2d_array_wo_t",
                                "opencl.image2d_depth_wo_t",
                                "opencl.image2d_array_rw_t",
                                "opencl.image2d_depth_rw_t",
                                "opencl.image2d_array_depth_ro_t",
                                "opencl.image2d_msaa_ro_t",
                                "opencl.image2d_array_depth_wo_t",
                                "opencl.image2d_msaa_wo_t",
                                "opencl.image2d_array_depth_rw_t",
                                "opencl.image2d_msaa_rw_t",
                                "opencl.image2d_array_msaa_ro_t",
                                "opencl.image2d_msaa_depth_ro_t",
                                "opencl.image2d_array_msaa_wo_t",
                                "opencl.image2d_msaa_depth_wo_t",
                                "opencl.image2d_array_msaa_rw_t",
                                "opencl.image2d_msaa_depth_rw_t",
                                "opencl.image2d_array_msaa_depth_ro_t",
                                "opencl.image3d_ro_t",
                                "opencl.image2d_array_msaa_depth_wo_t",
                                "opencl.image3d_wo_t",
                                "opencl.image2d_array_msaa_depth_rw_t",
                                "opencl.image3d_rw_t"};

bool isImagesUsed(const Module &M) {
  for (unsigned i = 0, e = sizeof(ImageTypeNames) / sizeof(ImageTypeNames[0]);
       i < e; ++i) {
    if (StructType::getTypeByName(M.getContext(), ImageTypeNames[i]))
      return true;
  }

  return false;
}

bool isBlockInvocationKernel(Function *F) {
  // TODO: Is there a better way to detect block invoke kernel?
  // And can this be replaced with the same function in BlockUtils.cpp?
  if (F->getName().contains("_block_invoke_") &&
      F->getName().endswith("_kernel_separated_args"))
    return true;

  return false;
}

TinyPtrVector<DbgDeclareInst *> findDbgUses(Value *V) {
  TinyPtrVector<DbgDeclareInst *> DIs = FindDbgDeclareUses(V);
  if (!DIs.empty())
    return DIs;

  // Try debug info of addrspacecast user.
  for (auto *U : V->users()) {
    if (auto *ASC = dyn_cast<AddrSpaceCastInst>(U)) {
      DIs = FindDbgDeclareUses(ASC);
      if (!DIs.empty())
        break;
    }
  }
  return DIs;
}

Type *getLLVMTypeFromReflectionType(LLVMContext &C,
                                    const reflection::RefParamType &PT) {
  auto *ParamTy = PT.get();
  auto *VPT = dyn_cast<reflection::VectorType>(ParamTy);
  if (VPT)
    ParamTy = VPT->getScalarType().get();
  Type *Ty = StringSwitch<Type *>(ParamTy->toString())
                 .Case("bool", Type::getInt1Ty(C))
                 .EndsWith("char", Type::getInt8Ty(C))
                 .EndsWith("short", Type::getInt16Ty(C))
                 .EndsWith("int", Type::getInt32Ty(C))
                 .EndsWith("long", Type::getInt64Ty(C))
                 .Case("half", Type::getHalfTy(C))
                 .Case("float", Type::getFloatTy(C))
                 .Case("double", Type::getDoubleTy(C))
                 .Default(nullptr);
  assert(Ty && "Unhandled primitive type");
  if (VPT)
    Ty = FixedVectorType::get(Ty, VPT->getLength());
  return Ty;
}

DenseMap<Function *, InstVecVec>
getTIDCallsInFuncs(Module &M, StringRef TIDName, FuncSet &Funcs) {
  DenseMap<Function *, InstVecVec> FuncToGIDCalls;
  for (auto *F : Funcs)
    FuncToGIDCalls[F] = InstVecVec{MAX_WORK_DIM, InstVec{}};
  if (auto *F = M.getFunction(TIDName)) {
    for (User *U : F->users()) {
      auto *CI = cast<CallInst>(U);
      Function *UserF = CI->getFunction();
      if (Funcs.contains(UserF)) {
        auto Idx = cast<ConstantInt>(CI->getArgOperand(0))->getZExtValue();
        FuncToGIDCalls[UserF][Idx].push_back(CI);
      }
    }
  }
  return FuncToGIDCalls;
}

} // end namespace CompilationUtils
} // end namespace llvm
