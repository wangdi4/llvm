//===-- DPCPPKernelCompilationUtils.cpp - Function definitions -*- C++ ----===//
//
// Copyright (C) 2020-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/Intel_VectorVariant.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"

#include "ImplicitArgsUtils.h"
#include "NameMangleAPI.h"
#include "ParameterType.h"
#include "TypeAlignment.h"

using namespace llvm::NameMangleAPI;

namespace llvm {

// Attributes
const StringRef KernelAttribute::CallOnce = "kernel-call-once";
const StringRef KernelAttribute::CallParamNum = "call-params-num";
const StringRef KernelAttribute::ConvergentCall = "kernel-convergent-call";
const StringRef KernelAttribute::HasSubGroups = "has-sub-groups";
const StringRef KernelAttribute::HasVPlanMask = "has-vplan-mask";
const StringRef KernelAttribute::OCLVecUniformReturn =
    "opencl-vec-uniform-return";
const StringRef KernelAttribute::RecursionWithBarrier =
    "barrier_with_recursion";
const StringRef KernelAttribute::UniformCall = "kernel-uniform-call";
const StringRef KernelAttribute::VectorVariants = "vector-variants";
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
const StringRef NAME_PREFETCH = "prefetch";
const StringRef NAME_WAIT_GROUP_EVENTS = "wait_group_events";
const StringRef SAMPLER = "sampler_t";

// atomic fence functions
const StringRef NAME_ATOMIC_WORK_ITEM_FENCE = "atomic_work_item_fence";

// work-group functions
const StringRef NAME_WORK_GROUP_ALL = "work_group_all";
const StringRef NAME_WORK_GROUP_ANY = "work_group_any";
const StringRef NAME_WORK_GROUP_BROADCAST = "work_group_broadcast";
const StringRef NAME_WORK_GROUP_REDUCE_ADD = "work_group_reduce_add";
const StringRef NAME_WORK_GROUP_SCAN_EXCLUSIVE_ADD =
    "work_group_scan_exclusive_add";
const StringRef NAME_WORK_GROUP_SCAN_INCLUSIVE_ADD =
    "work_group_scan_inclusive_add";
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
const StringRef NAME_FINALIZE_WG_FUNCTION_PREFIX = "__finalize_";

// KMP acquire/release
const StringRef NAME_IB_KMP_ACQUIRE_LOCK = "__builtin_IB_kmp_acquire_lock";
const StringRef NAME_IB_KMP_RELEASE_LOCK = "__builtin_IB_kmp_release_lock";

// subgroup functions
const StringRef NAME_SUB_GROUP_ALL = "sub_group_all";
const StringRef NAME_SUB_GROUP_ANY = "sub_group_any";
const StringRef NAME_SUB_GROUP_BROADCAST = "sub_group_broadcast";
const StringRef NAME_SUB_GROUP_REDUCE_ADD = "sub_group_reduce_add";
const StringRef NAME_SUB_GROUP_SCAN_EXCLUSIVE_ADD =
    "sub_group_scan_exclusive_add";
const StringRef NAME_SUB_GROUP_SCAN_INCLUSIVE_ADD =
    "sub_group_scan_inclusive_add";
const StringRef NAME_SUB_GROUP_REDUCE_MIN = "sub_group_reduce_min";
const StringRef NAME_SUB_GROUP_SCAN_EXCLUSIVE_MIN =
    "sub_group_scan_exclusive_min";
const StringRef NAME_SUB_GROUP_SCAN_INCLUSIVE_MIN =
    "sub_group_scan_inclusive_min";
const StringRef NAME_SUB_GROUP_REDUCE_MAX = "sub_group_reduce_max";
const StringRef NAME_SUB_GROUP_SCAN_EXCLUSIVE_MAX =
    "sub_group_scan_exclusive_max";
const StringRef NAME_SUB_GROUP_SCAN_INCLUSIVE_MAX =
    "sub_group_scan_inclusive_max";

/// Not mangled names.
const StringRef NAME_GET_BASE_GID = "get_base_global_id.";
const StringRef NAME_GET_SPECIAL_BUFFER = "get_special_buffer.";
const StringRef NAME_PRINTF = "printf";

/// Matrix slicing support.
const StringRef NAME_GET_SUB_GROUP_SLICE_LENGTH = "get_sub_group_slice_length.";
const StringRef NAME_GET_SUB_GROUP_ROWSLICE_ID = "get_sub_group_rowslice_id";
const StringRef NAME_SUB_GROUP_ROWSLICE_EXTRACTELEMENT =
    "sub_group_rowslice_extractelement";
const StringRef NAME_SUB_GROUP_ROWSLICE_INSERTELEMENT =
    "sub_group_rowslice_insertelement";
const StringRef NAME_SUB_GROUP_INSERT_ROWSLICE_TO_MATRIX =
    "sub_group_insert_rowslice_to_matrix";
} // namespace

static cl::opt<std::string> OptVectInfoFile("dpcpp-vect-info",
                                            cl::desc("Builtin VectInfo list"),
                                            cl::value_desc("filename"));

namespace DPCPPKernelCompilationUtils {

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

std::string AppendWithDimension(StringRef S, int Dimension) {
  return Dimension >= 0 ? (S + Twine(Dimension)).str() : (S + "var").str();
}

std::string AppendWithDimension(StringRef S, const Value *Dimension) {
  int D = -1;
  if (const ConstantInt *C = dyn_cast<ConstantInt>(Dimension))
    D = C->getZExtValue();
  return AppendWithDimension(S, D);
}

bool isGetEnqueuedLocalSize(StringRef S) {
  return isMangleOf(S, NAME_GET_ENQUEUED_LOCAL_SIZE);
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

bool isGetGroupId(StringRef S) {
  return isMangleOf(S, NAME_GET_GROUP_ID);
}

bool isGetLocalSize(StringRef S) {
  return isMangleOf(S, NAME_GET_LOCAL_SIZE);
}

bool isGetNumGroups(StringRef S) {
  return isMangleOf(S, NAME_GET_NUM_GROUPS);
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

bool isGlobalCtorDtor(Function *F) {
  // TODO: implement good solution based on value of @llvm.global_ctors variable
  return F->getName() == "__pipe_global_ctor" ||
         F->getName() == "__pipe_global_dtor";
}

bool isGlobalCtorDtorOrCPPFunc(Function *F) {
  assert(F && "Invalid input for global ctor / dtor / cpp func check");
  return isGlobalCtorDtor(F) || F->hasFnAttribute("not-ocl-dpcpp");
}

bool isGlobalOffset(StringRef S) {
  return isMangleOf(S, NAME_GET_GLOBAL_OFFSET);
}

StringRef nameGetBaseGID() { return NAME_GET_BASE_GID; }

bool isGetSpecialBuffer(StringRef S) { return S == NAME_GET_SPECIAL_BUFFER; }

bool isPrefetch(StringRef S) { return isMangleOf(S, NAME_PREFETCH); }

bool isPrintf(StringRef S) { return S == NAME_PRINTF; }

// Work-Group builtins
bool isWorkGroupAll(StringRef S) { return isMangleOf(S, NAME_WORK_GROUP_ALL); }

bool isWorkGroupAny(StringRef S) { return isMangleOf(S, NAME_WORK_GROUP_ANY); }

bool isWorkGroupBroadCast(StringRef S) {
  return isMangleOf(S, NAME_WORK_GROUP_BROADCAST);
}

bool isWorkGroupReduceAdd(StringRef S) {
  return isMangleOf(S, NAME_WORK_GROUP_REDUCE_ADD);
}

bool isWorkGroupReduceMin(StringRef S) {
  return isMangleOf(S, NAME_WORK_GROUP_REDUCE_MIN);
}

bool isWorkGroupReduceMax(StringRef S) {
  return isMangleOf(S, NAME_WORK_GROUP_REDUCE_MAX);
}

bool isWorkGroupScanExclusiveAdd(StringRef S) {
  return isMangleOf(S, NAME_WORK_GROUP_SCAN_EXCLUSIVE_ADD);
}

bool isWorkGroupScanInclusiveAdd(StringRef S) {
  return isMangleOf(S, NAME_WORK_GROUP_SCAN_INCLUSIVE_ADD);
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

bool isWorkItemPipeBuiltin(StringRef S) {
  auto Kind = getPipeKind(S);
  return Kind && Kind.Scope == PipeKind::ScopeKind::WorkItem;
}

bool isWaitGroupEvents(StringRef S) {
  return isMangleOf(S, NAME_WAIT_GROUP_EVENTS);
}

bool isWorkGroupAsyncOrPipeBuiltin(StringRef S, const Module &M) {
  return isAsyncWorkGroupCopy(S) || isAsyncWorkGroupStridedCopy(S) ||
         (OclVersion::CL_VER_2_0 <= fetchCLVersionFromMetadata(M) &&
          (isWorkGroupReserveReadPipe(S) || isWorkGroupCommitReadPipe(S) ||
           isWorkGroupReserveWritePipe(S) || isWorkGroupCommitWritePipe(S)));
}

bool isWorkGroupScan(StringRef S) {
  return isWorkGroupScanExclusiveAdd(S) || isWorkGroupScanInclusiveAdd(S) ||
         isWorkGroupScanExclusiveMin(S) || isWorkGroupScanInclusiveMin(S) ||
         isWorkGroupScanExclusiveMax(S) || isWorkGroupScanInclusiveMax(S);
}

bool isWorkGroupUniform(StringRef S) {
  return isWorkGroupAll(S) || isWorkGroupAny(S) || isWorkGroupBroadCast(S) ||
         isWorkGroupReduceAdd(S) || isWorkGroupReduceMin(S) ||
         isWorkGroupReduceMax(S);
}

bool isWorkGroupMin(StringRef S) {
  return isWorkGroupReduceMin(S) || isWorkGroupScanExclusiveMin(S) ||
         isWorkGroupScanInclusiveMin(S);
}

bool isWorkGroupMax(StringRef S) {
  return isWorkGroupReduceMax(S) || isWorkGroupScanExclusiveMax(S) ||
         isWorkGroupScanInclusiveMax(S);
}

bool isWorkGroupDivergent(StringRef S) { return isWorkGroupScan(S); }

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
  assert(hasWorkGroupFinalizePrefix(S) && "expected finilize prefix");
  reflection::FunctionDescriptor FD = demangle(S);
  FD.Name = FD.Name.substr(NAME_FINALIZE_WG_FUNCTION_PREFIX.size());
  std::string FuncName = mangle(FD);
  return FuncName;
}

bool isWorkGroupBuiltin(StringRef S) {
  return isWorkGroupUniform(S) || isWorkGroupDivergent(S);
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

bool isSubGroupScan(StringRef S) {
  return isSubGroupScanExclusiveAdd(S) || isSubGroupScanInclusiveAdd(S) ||
         isSubGroupScanExclusiveMin(S) || isSubGroupScanInclusiveMin(S) ||
         isSubGroupScanExclusiveMax(S) || isSubGroupScanInclusiveMax(S);
}

// TODO: add ballot function, refactor OCLVecClone - opencl-vec-uniform-return.
bool isSubGroupUniform(StringRef S) {
  return isGetSubGroupSize(S) || isGetSubGroupId(S) ||
         isGetMaxSubGroupSize(S) || isGetNumSubGroups(S) ||
         isGetEnqueuedNumSubGroups(S) || isSubGroupAll(S) || isSubGroupAny(S) ||
         isSubGroupBroadCast(S) || isSubGroupReduceAdd(S) ||
         isSubGroupReduceMin(S) || isSubGroupReduceMax(S);
}

bool isSubGroupDivergent(StringRef S) {
  return isGetSubGroupLocalId(S) || isSubGroupScan(S);
}

bool isSubGroupBuiltin(StringRef S) {
  return isSubGroupUniform(S) || isSubGroupDivergent(S);
}

bool isSubGroupBarrier(StringRef S) {
  return S == mangledSGBarrier(BarrierType::NoScope) ||
         S == mangledSGBarrier(BarrierType::WithScope);
}

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

StructType *getStructFromTypePtr(Type *Ty) {
  auto *PtrTy = dyn_cast<PointerType>(Ty);
  if (!PtrTy)
    return nullptr;
  // Handle also pointer to pointer to ...
  while (auto *PtrTyNext = dyn_cast<PointerType>(PtrTy->getElementType()))
    PtrTy = PtrTyNext;
  return dyn_cast<StructType>(PtrTy->getElementType());
}

bool isSameStructType(StructType *STy1, StructType *STy2) {
  if (!STy1->hasName() || !STy2->hasName())
    return false;
  return 0 == stripStructNameTrailingDigits(STy1->getName())
                  .compare(stripStructNameTrailingDigits(STy2->getName()));
}

PointerType *mutatePtrElementType(PointerType *SrcPTy, Type *DstTy) {
  // The function changes the base type of SrcPTy to DstTy
  // SrcPTy = %struct.__pipe_t addrspace(1)**
  // DstTy  = %struct.__pipe_t.1
  // =>
  // %struct.__pipe_t.1 addrspace(1)**

  assert(SrcPTy && DstTy && "Invalid types!");

  SmallVector<PointerType *, 2> Types{SrcPTy};
  while ((SrcPTy = dyn_cast<PointerType>(SrcPTy->getElementType())))
    Types.push_back(SrcPTy);

  for (auto It = Types.rbegin(), E = Types.rend(); It != E; ++It)
    DstTy = PointerType::get(DstTy, (*It)->getAddressSpace());

  return cast<PointerType>(DstTy);
}

Function *importFunctionDecl(Module *Dst, const Function *Orig,
                             bool DuplicateIfExists) {
  assert(Dst && "Invalid module");
  assert(Orig && "Invalid function");

  std::vector<StructType *> DstSTys = Dst->getIdentifiedStructTypes();
  FunctionType *OrigFnTy = Orig->getFunctionType();

  SmallVector<Type *, 8> NewArgTypes;
  bool Changed = false;
  for (auto *ArgTy : Orig->getFunctionType()->params()) {
    NewArgTypes.push_back(ArgTy);

    auto *STy = getStructFromTypePtr(ArgTy);
    if (!STy)
      continue;

    for (auto *DstSTy : DstSTys) {
      if (isSameStructType(DstSTy, STy)) {
        NewArgTypes.back() =
            mutatePtrElementType(cast<PointerType>(ArgTy), DstSTy);
        Changed = true;
        break;
      }
    }
  }

  FunctionType *NewFnType =
      (!Changed) ? OrigFnTy
                 : FunctionType::get(Orig->getReturnType(), NewArgTypes,
                                     Orig->isVarArg());
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
    DPCPPKernelMetadataAPI::KernelInternalMetadataAPI KIMD(F);
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

void moveInstructionIf(BasicBlock *FromBB, BasicBlock *ToBB,
                       function_ref<bool(Instruction &)> Predicate) {
  SmallVector<Instruction *, 8> ToMove;
  for (auto &I : *FromBB)
    if (Predicate(I))
      ToMove.push_back(&I);

  auto MovePos = ToBB->getFirstInsertionPt();
  for (auto *I : ToMove)
    I->moveBefore(*ToBB, MovePos);
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
          isWorkGroupAsyncOrPipeBuiltin(FName, M))
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

Function *AddMoreArgsToFunc(Function *F, ArrayRef<Type *> NewTypes,
                            ArrayRef<const char *> NewNames,
                            ArrayRef<AttributeSet> NewAttrs, StringRef Prefix) {
  assert(NewTypes.size() == NewNames.size());
  // Initialize with all original arguments in the function sugnature.
  SmallVector<Type *, 16> Types;

  for (Function::const_arg_iterator I = F->arg_begin(), E = F->arg_end();
       I != E; ++I) {
    Types.push_back(I->getType());
  }
  Types.append(NewTypes.begin(), NewTypes.end());
  FunctionType *NewFTy = FunctionType::get(F->getReturnType(), Types, false);
  // Change original function name.
  std::string Name = F->getName().str();
  F->setName((Twine("__") + F->getName() + Twine("_before.") + Prefix).str());
  // Create a new function with explicit and implict arguments types
  Function *NewF =
      Function::Create(NewFTy, F->getLinkage(), Name, F->getParent());
  // Copy old function attributes (including attributes on original arguments)
  // to new function.
  NewF->copyAttributesFrom(F);
  NewF->copyMetadata(F, 0);
  // Set original arguments' names.
  Function::arg_iterator NewI = NewF->arg_begin();
  for (Function::const_arg_iterator I = F->arg_begin(), E = F->arg_end();
       I != E; ++I, ++NewI) {
    NewI->setName(I->getName());
  }
  // Set new arguments' names.
  for (unsigned I = 0, E = NewNames.size(); I < E; ++I, ++NewI) {
    Argument *A = &*NewI;
    A->setName(NewNames[I]);
    if (!NewAttrs.empty())
      for (auto Attr : NewAttrs[I])
        A->addAttr(Attr);
  }
  // Since we have now created the new function, splice the body of the old
  // function right into the new function, leaving the old body of the function
  // empty.
  NewF->getBasicBlockList().splice(NewF->begin(), F->getBasicBlockList());
  assert(F->isDeclaration() &&
         "splice did not work, original function body is not empty!");

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

  // Replace F by NewF in KernelList module Metadata (if any)
  using namespace DPCPPKernelMetadataAPI;
  llvm::Module *M = F->getParent();
  assert(M && "Module is NULL");
  auto Kernels = KernelList(M).getList();
  std::replace_if(
      std::begin(Kernels), std::end(Kernels),
      [F](llvm::Function *Func) { return F == Func; }, NewF);
  KernelList(M).set(Kernels);

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
  auto *FType = cast<FunctionType>(FPtrType->getElementType());
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

  // TODO Remove the block once OpenCL CPU BE compiler is able to handle
  // LLVM IR converted from SPIR-V correctly.
  if (isGeneratedFromOCLCPP(M))
    return OclVersion::CL_VER_2_0;

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

GlobalVariable *getTLSGlobal(Module *M, unsigned Idx) {
  assert(M && "Module cannot be null");
  return M->getGlobalVariable(ImplicitArgsUtils::getArgName(Idx));
}

void parseKernelArguments(Module *M, Function *F, bool UseTLSGlobals,
                          std::vector<KernelArgument> &Arguments,
                          std::vector<unsigned int> &MemArguments) {
  size_t ArgsCount = F->arg_size();
  if (!UseTLSGlobals)
    ArgsCount -= ImplicitArgsUtils::NUM_IMPLICIT_ARGS;
  unsigned int LocalMemCount = 0;
  unsigned int CurrentOffset = 0;
  Function::arg_iterator arg_it = F->arg_begin();
  for (unsigned i = 0; i < ArgsCount; ++i) {
    KernelArgument CurArg;
    bool isMemoryObject = false;

    Argument *pArg = &*arg_it;
    // Set argument sizes and offsets
    switch (arg_it->getType()->getTypeID()) {
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
      DPCPPKernelMetadataAPI::KernelInternalMetadataAPI KIMD(F);
      if ((i == 0) && KIMD.BlockLiteralSize.hasValue()) {
        auto *PTy = dyn_cast<PointerType>(pArg->getType());
        if (!PTy || !PTy->getElementType()->isIntegerTy(8))
          continue;

        CurArg.Ty = KRNL_ARG_PTR_BLOCK_LITERAL;
        CurArg.SizeInBytes = KIMD.BlockLiteralSize.get();
        break;
      }

      PointerType *PTy = cast<PointerType>(arg_it->getType());
      if (pArg->hasByValAttr() && isa<VectorType>(PTy->getElementType())) {
        // Check by pointer vector passing, used in long16 and double16
        FixedVectorType *Vector = cast<FixedVectorType>(PTy->getElementType());
        unsigned int uiNumElem = (unsigned int)Vector->getNumElements();
        ;
        unsigned int uiElemSize =
            Vector->getContainedType(0)->getPrimitiveSizeInBits() / 8;
        // assert( ((uiElemSize*uiNumElem) < 8 || (uiElemSize*uiNumElem) > 4*16)
        // &&
        //  "We have byval pointer for legal vector type larger than 64bit");
        CurArg.Ty = KRNL_ARG_VECTOR_BY_REF;
        CurArg.SizeInBytes = uiNumElem & 0xFFFF;
        CurArg.SizeInBytes |= (uiElemSize << 16);
        break;
      }
      CurArg.SizeInBytes = M->getDataLayout().getPointerSize(0);
      // Detect pointer qualifier
      // Test for opaque types: images, queue_t, pipe_t
      StructType *ST = dyn_cast<StructType>(PTy->getElementType());
      if (ST && ST->hasName()) {
        char const oclOpaquePref[] = "opencl.";
        const size_t oclOpaquePrefLen =
            sizeof(oclOpaquePref) - 1; // sizeof also counts the terminating 0

        if (ST->getName().startswith(oclOpaquePref)) {
          const StringRef structName = ST->getName().substr(oclOpaquePrefLen);
          // Get opencl opaque type.
          // It is safe to use startswith while there are no names which aren't
          // prefix of another name.
          if (structName.startswith("image1d_array_"))
            CurArg.Ty = KRNL_ARG_PTR_IMG_1D_ARR;
          else if (structName.startswith("image1d_buffer_"))
            CurArg.Ty = KRNL_ARG_PTR_IMG_1D_BUF;
          else if (structName.startswith("image1d_"))
            CurArg.Ty = KRNL_ARG_PTR_IMG_1D;
          else if (structName.startswith("image2d_depth_"))
            CurArg.Ty = KRNL_ARG_PTR_IMG_2D_DEPTH;
          else if (structName.startswith("image2d_array_depth_"))
            CurArg.Ty = KRNL_ARG_PTR_IMG_2D_ARR_DEPTH;
          else if (structName.startswith("image2d_array_"))
            CurArg.Ty = KRNL_ARG_PTR_IMG_2D_ARR;
          else if (structName.startswith("image2d_"))
            CurArg.Ty = KRNL_ARG_PTR_IMG_2D;
          else if (structName.startswith("image3d_"))
            CurArg.Ty = KRNL_ARG_PTR_IMG_3D;
          else if (structName.startswith("pipe_ro_t")) {
            CurArg.Ty = KRNL_ARG_PTR_PIPE_T;
          } else if (structName.startswith("pipe_wo_t")) {
            CurArg.Ty = KRNL_ARG_PTR_PIPE_T;
          } else if (structName.startswith("queue_t"))
            CurArg.Ty = KRNL_ARG_PTR_QUEUE_T;
          else if (structName.startswith("clk_event_t"))
            CurArg.Ty = KRNL_ARG_PTR_CLK_EVENT_T;
          else if (structName.startswith("sampler_t"))
            CurArg.Ty = KRNL_ARG_PTR_SAMPLER_T;
          else {
            assert(
                false &&
                "did you forget to handle a new special OpenCL C opaque type?");
            // TODO: Why default type is INTEGER????
            CurArg.Ty = KRNL_ARG_INT;
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
          // Check this is a special OpenCL C opaque type.
          if (KRNL_ARG_INT != CurArg.Ty)
            break;
        } else if (dyn_cast<PointerType>(PTy->getElementType())) {
          // Pointer to pointer case.
          assert(false &&
                 "pointer to pointer is not allowed in kernel arguments");
        }
      }

      Type *Ty = PTy->getContainedType(0);
      if (Ty->isStructTy()) // struct or struct*
      {
        // Deal with structs passed by value. These are user-defined structs and
        // ndrange_t.
        if (PTy->getAddressSpace() == 0) {
          StructType *STy = cast<StructType>(Ty);
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
        ++LocalMemCount;
        break;

      default:
        assert(0);
      }
    } break;

    case Type::IntegerTyID: {
      DPCPPKernelMetadataAPI::KernelMetadataAPI KMD(F);
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
      assert(0 && "Unhandled parameter type");
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
widenParameters(reflection::TypeVector ScalarParams, unsigned int VF) {
  reflection::TypeVector VectorParams;
  for (auto Param : ScalarParams) {
    if (auto *VecParam =
            reflection::dyn_cast<reflection::VectorType>(Param.get())) {
      int widen_len = VecParam->getLength() * VF;
      VectorParams.emplace_back(
          VECTOR_TYPE(VecParam->getScalarType(), widen_len));
    } else {
      VectorParams.emplace_back(VECTOR_TYPE(Param, VF));
    }
  }

  return VectorParams;
}

static void pushSGBlockBuiltinDivergentVectInfo(
    const Twine &BaseName, unsigned int len, unsigned int VF,
    reflection::TypeVector ScalarParams,
    std::vector<std::tuple<std::string, std::string, std::string>>
        &ExtendedVectInfos) {
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

  // Get vector variant string repr
  llvm::VectorVariant Variant{
      llvm::VectorVariant::ISAClass::XMM,
      true,
      VF,
      std::vector<VectorKind>(v_num, VectorKind::vector()),
      ScalarMangleName,
      VectorMangleName};

  ExtendedVectInfos.push_back({ScalarMangleName,
                               std::string(KernelAttribute::CallOnce),
                               Variant.toString()});
}

static void pushSGBlockBuiltinDivergentVectInfo(
    StringRef TySuffix, reflection::TypePrimitiveEnum Ty,
    std::vector<unsigned int> Lens, std::vector<unsigned int> VFs,
    std::vector<std::tuple<std::string, std::string, std::string>>
        &ExtendedVectInfos) {
  const Twine SG_BLOCK_READ_PREFIX("intel_sub_group_block_read");
  const Twine SG_BLOCK_WRITE_PREFIX("intel_sub_group_block_write");

  for (unsigned int Len : Lens) {
    for (unsigned int VF : VFs) {
      // sub_group_block_read(const __global T*)
      pushSGBlockBuiltinDivergentVectInfo(SG_BLOCK_READ_PREFIX + TySuffix, Len,
                                          VF, {CONST_GLOBAL_PTR(Ty)},
                                          ExtendedVectInfos);
      // sub_group_block_read(readonly image2d_t, int2)
      pushSGBlockBuiltinDivergentVectInfo(
          SG_BLOCK_READ_PREFIX + TySuffix, Len, VF,
          {PRIM_TYPE(reflection::PRIMITIVE_IMAGE_2D_RO_T), INT2_TYPE},
          ExtendedVectInfos);
      // sub_group_block_read(readwrite image2d_t, int2)
      pushSGBlockBuiltinDivergentVectInfo(
          SG_BLOCK_READ_PREFIX + TySuffix, Len, VF,
          {PRIM_TYPE(reflection::PRIMITIVE_IMAGE_2D_RW_T), INT2_TYPE},
          ExtendedVectInfos);

      if (Len == 1) {
        // sub_group_block_write(__global T*, T)
        pushSGBlockBuiltinDivergentVectInfo(
            SG_BLOCK_WRITE_PREFIX + TySuffix, Len, VF,
            {GLOBAL_PTR(Ty), PRIM_TYPE(Ty)}, ExtendedVectInfos);
        // sub_group_block_write(writeonly image2d_t, int2, T)
        pushSGBlockBuiltinDivergentVectInfo(
            SG_BLOCK_WRITE_PREFIX + TySuffix, Len, VF,
            {PRIM_TYPE(reflection::PRIMITIVE_IMAGE_2D_WO_T), INT2_TYPE,
             PRIM_TYPE(Ty)},
            ExtendedVectInfos);
        // sub_group_block_write(readwrite image2d_t, int2, T)
        pushSGBlockBuiltinDivergentVectInfo(
            SG_BLOCK_WRITE_PREFIX + TySuffix, Len, VF,
            {PRIM_TYPE(reflection::PRIMITIVE_IMAGE_2D_RW_T), INT2_TYPE,
             PRIM_TYPE(Ty)},
            ExtendedVectInfos);
      } else {
        // sub_group_block_write(__global T*, T<Len>)
        pushSGBlockBuiltinDivergentVectInfo(
            SG_BLOCK_WRITE_PREFIX + TySuffix, Len, VF,
            {GLOBAL_PTR(Ty), VECTOR_TYPE(PRIM_TYPE(Ty), Len)},
            ExtendedVectInfos);
        // sub_group_block_write(writeonly image2d_t, int2, T<Len>)
        pushSGBlockBuiltinDivergentVectInfo(
            SG_BLOCK_WRITE_PREFIX + TySuffix, Len, VF,
            {PRIM_TYPE(reflection::PRIMITIVE_IMAGE_2D_WO_T), INT2_TYPE,
             VECTOR_TYPE(PRIM_TYPE(Ty), Len)},
            ExtendedVectInfos);
        // sub_group_block_write(readwrite image2d_t, int2, T<Len>)
        pushSGBlockBuiltinDivergentVectInfo(
            SG_BLOCK_WRITE_PREFIX + TySuffix, Len, VF,
            {PRIM_TYPE(reflection::PRIMITIVE_IMAGE_2D_RW_T), INT2_TYPE,
             VECTOR_TYPE(PRIM_TYPE(Ty), Len)},
            ExtendedVectInfos);
      }
    }
  }
}

static void pushSGRowSliceBuiltinVectInfo(
    std::vector<std::tuple<std::string, std::string, std::string>>
        &ExtendedVectInfos) {
  const static SmallVector<StringRef, 4> DataTypes = {"i8", "i16", "i32",
                                                      "bf16", "f32"};
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

void initializeVectInfoOnce(
    ArrayRef<VectItem> VectInfos,
    std::vector<std::tuple<std::string, std::string, std::string>>
        &ExtendedVectInfos) {
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
                                        std::get<2>(Entry), std::get<3>(Entry),
                                        ExtendedVectInfos);
  }

  // Add extra vector info for 'sub_group_rowslice_extractelement.*' and
  // 'sub_group_rowslice_insertelement.*'
  pushSGRowSliceBuiltinVectInfo(ExtendedVectInfos);
}

static std::string getFormatStr(Value *V) {
  Type *T = V->getType();
  std::string Name = V->getName().str();

  if (T->isIntegerTy(32))
    return Name + ": %d ";
  if (T->isFloatTy())
    return Name + ": %f ";
  if (T->isDoubleTy())
    return Name + ": %lf ";
  if (T->isPointerTy())
    return Name + "%p";
  llvm_unreachable("Can't print this value");
}

void insertPrintf(const Twine &Prefix, Instruction *IP,
                  ArrayRef<Value *> Inputs) {
  auto &Context = IP->getContext();
  unsigned StrAddrSpace = 2;

  // Declare printf function
  auto *StrType = PointerType::get(Type::getInt8Ty(Context), StrAddrSpace);
  SmallVector<Type *, 1> ArgList{StrType};
  auto *FuncType = FunctionType::get(Type::getInt32Ty(Context), ArgList, true);
  FunctionCallee PrintFuncConst =
      IP->getModule()->getOrInsertFunction("printf", FuncType);
  auto *PrintFunc = cast<Function>(PrintFuncConst.getCallee());

  SmallVector<Value *, 16> TempInputs;
  IRBuilder<> Builder(IP);
  for (auto *I : Inputs) {
    if (auto *T = dyn_cast<FixedVectorType>(I->getType())) {
      unsigned Len = T->getNumElements();
      StringRef Name = I->getName();
      for (unsigned Idx = 0; Idx < Len; ++Idx) {
        auto *Ele =
            Builder.CreateExtractElement(I, Idx, Name + "." + Twine(Idx));
        TempInputs.push_back(Ele);
      }
    } else {
      TempInputs.push_back(I);
    }
  }

  std::string FormatStr = "PRINT " + Prefix.str() + " ";
  SmallVector<Value *, 16> TempInputsCast;
  for (auto *V : TempInputs) {
    Type *T = V->getType();
    if (T->isIntegerTy())
      if (!T->isIntegerTy(32))
        V = CastInst::CreateIntegerCast(V, Type::getInt32Ty(V->getContext()),
                                        false, V->getName() + "cast.", IP);
    FormatStr += getFormatStr(V);
    TempInputsCast.push_back(V);
  }
  FormatStr += "\n";

  Constant *StrVal =
      ConstantDataArray::getString(IP->getContext(), FormatStr, true);
  auto *ArrayType = StrVal->getType();
  auto *FormatStrGV = new GlobalVariable(
      *IP->getModule(), ArrayType, true, GlobalValue::InternalLinkage, StrVal,
      "format.str.", 0, GlobalVariable::NotThreadLocal, StrAddrSpace);

  SmallVector<Value *, 2> Idx(2, Builder.getInt32(0));
  auto *StrPtr = GetElementPtrInst::Create(ArrayType, FormatStrGV, Idx, "", IP);

  // Insert call.
  SmallVector<Value *, 4> Args{StrPtr};
  Args.append(TempInputsCast.begin(), TempInputsCast.end());
  Builder.CreateCall(PrintFunc, Args, "PRINT.");
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
    // Opaque pointer doesn't have pointee type information, so we just mangle
    // address space for opaque pointer.
    if (!PTyp->isOpaque())
      Result += getMangledTypeStr(PTyp->getElementType(), HasUnnamedType);
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
      for (auto Elem : STyp->elements())
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

CallInst *createGetSubGroupSliceLengthCall(unsigned TotalElementCount,
                                           Instruction *IP, const Twine &Name) {
  IRBuilder<> Builder(IP);
  auto *Arg = Builder.getInt32(TotalElementCount);
  return generateCall(IP->getModule(), NAME_GET_SUB_GROUP_SLICE_LENGTH,
                      Builder.getInt64Ty(), {Arg}, Builder);
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

} // end namespace DPCPPKernelCompilationUtils
} // end namespace llvm
