//===-- DPCPPKernelCompilationUtils.cpp - Function definitions -*- C++ ----===//
//
// Copyright (C) 2020-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"

#include "ImplicitArgsUtils.h"
#include "NameMangleAPI.h"
#include "ParameterType.h"
#include "TypeAlignment.h"

using namespace llvm::NameMangleAPI;

namespace llvm {

const char *KernelAttribute::SyclKernel = "sycl-kernel";
const char *KernelAttribute::NoBarrierPath = "no-barrier-path";
// TODO: We need to replace "kernel_wrapper" with "kernel-wrapper", but
// RT retrieves kernel wrapper with "kernel_wrapper" attribute.
const char *KernelAttribute::KernelWrapper = "kernel_wrapper";

const char *KernelAttribute::BarrierBufferSize = "barrier-buffer-size";
const char *KernelAttribute::LocalBufferSize = "local-buffer-size";
const char *KernelAttribute::PrivateMemorySize = "private-memory-size";

const char *KernelAttribute::ScalarKernel = "scalar-kernel";
const char *KernelAttribute::VectorizedKernel = "vectorized-kernel";
const char *KernelAttribute::VectorizedMaskedKernel =
    "vectorized-masked-kernel";
const char *KernelAttribute::VectorizedWidth = "vectorized-width";
const char *KernelAttribute::RecommendedVL = "recommended-vector-length";
const char *KernelAttribute::VectorVariants = "vector-variants";

const char *KernelAttribute::BlockLiteralSize = "block-literal-size";

const char *DPCPPKernelCompilationUtils::ATTR_RECURSION_WITH_BARRIER =
    "barrier_with_recursion";

namespace DPCPPKernelCompilationUtils {

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
const StringRef NAME_GET_GLOBAL_SIZE = "get_global_size";
const StringRef NAME_GET_LOCAL_SIZE = "get_local_size";
const StringRef NAME_GET_GROUP_ID = "get_group_id";
const StringRef NAME_GET_NUM_GROUPS = "get_num_groups";
const StringRef NAME_GET_WORK_DIM = "get_work_dim";
const StringRef NAME_GET_GLOBAL_OFFSET = "get_global_offset";
const StringRef NAME_GET_ENQUEUED_LOCAL_SIZE = "get_enqueued_local_size";
const StringRef NAME_BARRIER = "barrier";
const StringRef NAME_WG_BARRIER = "work_group_barrier";
const StringRef NAME_PREFETCH = "prefetch";
const StringRef SAMPLER = "sampler_t";

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

/// Not mangled names.
const StringRef NAME_GET_BASE_GID = "get_base_global_id.";
const StringRef NAME_GET_SPECIAL_BUFFER = "get_special_buffer.";
const StringRef NAME_PRINTF = "printf";
} // namespace

static unsigned CLVersionToVal(uint64_t Major, uint64_t Minor) {
  return Major * 100 + Minor * 10;
}

static bool isMangleOf(StringRef LHS, StringRef RHS) {
  if (!isMangledName(LHS))
    return false;
  return stripName(LHS) == RHS;
}

static bool isOptionalMangleOf(StringRef LHS, StringRef RHS) {
  if (LHS == RHS)
    return true;
  // LHS should be mangled
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
  return isOptionalMangleOf(S, NAME_GET_LINEAR_GID);
}

bool isGetLocalLinearId(StringRef S) {
  return isOptionalMangleOf(S, NAME_GET_LINEAR_LID);
}

bool isGetGlobalSize(StringRef S) {
  return isOptionalMangleOf(S, NAME_GET_GLOBAL_SIZE);
}

bool isGetGroupId(StringRef S) {
  return isOptionalMangleOf(S, NAME_GET_GROUP_ID);
}

bool isGetLocalSize(StringRef S) {
  return isOptionalMangleOf(S, NAME_GET_LOCAL_SIZE);
}

bool isGetNumGroups(StringRef S) {
  return isOptionalMangleOf(S, NAME_GET_NUM_GROUPS);
}

bool isGetWorkDim(StringRef S) {
  return isOptionalMangleOf(S, NAME_GET_WORK_DIM);
}

bool isGetLocalId(StringRef S) { return isOptionalMangleOf(S, NAME_GET_LID); }

bool isGetGlobalId(StringRef S) { return isOptionalMangleOf(S, NAME_GET_GID); }

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
  return isOptionalMangleOf(S, NAME_GET_GLOBAL_OFFSET);
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

PipeKind getPipeKind(StringRef S) {
  PipeKind Kind;
  Kind.Op = PipeKind::OK_None;

  if (!S.consume_front("__"))
    return Kind;

  if (S.consume_front("sub_group_"))
    Kind.Scope = PipeKind::SK_SubGroup;
  else if (S.consume_front("work_group_"))
    Kind.Scope = PipeKind::SK_WorkGroup;
  else
    Kind.Scope = PipeKind::SK_WorkItem;

  if (S.consume_front("commit_"))
    Kind.Op = PipeKind::OK_Commit;
  else if (S.consume_front("reserve_"))
    Kind.Op = PipeKind::OK_Reserve;

  if (S.consume_front("read_"))
    Kind.Access = PipeKind::AK_Read;
  else if (S.consume_front("write_"))
    Kind.Access = PipeKind::AK_Write;
  else {
    Kind.Op = PipeKind::OK_None;
    return Kind; // not a pipe built-in
  }

  if (!S.consume_front("pipe")) {
    Kind.Op = PipeKind::OK_None;
    return Kind; // not a pipe built-in
  }

  if (Kind.Op == PipeKind::OK_Commit || Kind.Op == PipeKind::OK_Reserve) {
    // rest for the modifiers only appliy to read/write built-ins
    return Kind;
  }

  if (S.consume_front("_2"))
    Kind.Op = PipeKind::OK_ReadWrite;
  else if (S.consume_front("_4"))
    Kind.Op = PipeKind::OK_ReadWriteReserve;

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

  return Kind;
}

bool isWorkItemPipeBuiltin(StringRef S) {
  auto Kind = getPipeKind(S);
  return Kind && Kind.Scope == PipeKind::SK_WorkItem;
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

bool isWorkGroupAsyncOrPipeBuiltin(StringRef S, const Module *pModule) {
  return isAsyncWorkGroupCopy(S) || isAsyncWorkGroupStridedCopy(S) ||
         (OclVersion::CL_VER_2_0 <= fetchCLVersionFromMetadata(*pModule) &&
          (isWorkGroupReserveReadPipe(S) || isWorkGroupCommitReadPipe(S) ||
           isWorkGroupReserveWritePipe(S) || isWorkGroupCommitWritePipe(S)));
}

template <reflection::TypePrimitiveEnum... ParamTys>
static std::string optionalMangleWithParam(StringRef N) {
  reflection::FunctionDescriptor FD;
  FD.Name = N.str();
  for (auto PT : {ParamTys...}) {
    reflection::RefParamType UI(new reflection::PrimitiveType(PT));
    FD.Parameters.push_back(UI);
  }
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

std::string mangledGetLocalSize() {
  return optionalMangleWithParam<reflection::PRIMITIVE_UINT>(
      NAME_GET_LOCAL_SIZE);
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

void moveAllocaToEntry(BasicBlock *FromBB, BasicBlock *EntryBB) {
  // This implementation is only correct when ToBB is an entry block.
  SmallVector<AllocaInst *, 8> Allocas;
  for (auto &I : *FromBB)
    if (auto *AI = dyn_cast<AllocaInst>(&I))
      Allocas.push_back(AI);

  if (EntryBB->empty()) {
    IRBuilder<> Builder(EntryBB);
    for (auto *AI : Allocas) {
      AI->removeFromParent();
      Builder.Insert(AI);
    }
    return;
  }

  Instruction *InsPt = EntryBB->getFirstNonPHI();
  assert(InsPt && "At least one non-PHI insruction is expected in ToBB");
  for (auto *AI : Allocas) {
    AI->moveBefore(InsPt);
  }
}

void getAllSyncBuiltinsDecls(FuncSet &FuncSet, Module *M) {
  // Clear old collected data!
  FuncSet.clear();

  // TODO: port handling of WG collectives here as well
  std::string BarrierNames[] = {mangledBarrier(),
                                mangledWGBarrier(BarrierType::NoScope),
                                mangledWGBarrier(BarrierType::WithScope)};
  for (const auto &BarrierName : BarrierNames) {
    auto *F = M->getFunction(BarrierName);

    if (F && F->isDeclaration())
      FuncSet.insert(F);
  }
}

Function *AddMoreArgsToFunc(Function *F, ArrayRef<Type *> NewTypes,
                            ArrayRef<const char *> NewNames,
                            ArrayRef<AttributeSet> NewAttrs, StringRef Prefix) {
  assert(NewTypes.size() == NewNames.size());
  assert(NewTypes.size() == NewAttrs.size());
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
  assert(OldC->getNumArgOperands() + NewArgs.size() == NewF->arg_size() &&
         "Function argument number mismatch");

  SmallVector<Value *, 16> Args;
  for (unsigned I = 0, E = OldC->getNumArgOperands(); I != E; ++I)
    Args.push_back(OldC->getArgOperand(I));
  Args.append(NewArgs.begin(), NewArgs.end());

  // Replace the original function with a call
  CallInst *NewC = CallInst::Create(NewF, Args, "", OldC);
  NewC->setCallingConv(OldC->getCallingConv());

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
  for (unsigned I = 0, E = OldC->getNumArgOperands(); I != E; ++I)
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
      if (ST) {
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

} // end namespace DPCPPKernelCompilationUtils
} // end namespace llvm
