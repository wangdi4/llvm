//===-- DPCPPKernelCompilationUtils.h - Function declarations -*- C++ -----===//
//
// Copyright (C) 2020-2022 Intel Corporation. All rights reserved.
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
#include "llvm/Analysis/CallGraph.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"

namespace llvm {

// A tuple of three strings:
// 1. scalar variant name
// 2. "kernel-call-once" | ""
// 3. mangled vector variant name
using VectItem = std::tuple<const char *, const char *, const char *>;

enum class SyncType { None, Barrier, DummyBarrier };

namespace KernelAttribute {
// Attributes
extern const StringRef CallOnce;
extern const StringRef CallParamNum;
extern const StringRef ConvergentCall;
extern const StringRef HasSubGroups;
extern const StringRef HasVPlanMask;
extern const StringRef OCLVecUniformReturn;
extern const StringRef RecursionWithBarrier;
extern const StringRef UniformCall;
extern const StringRef VectorVariants;
extern const StringRef VectorVariantFailure;

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

enum class BarrierType { NoScope, WithScope };

struct PipeKind {
  enum class ScopeKind { WorkItem, WorkGroup, SubGroup };

  /// Access direction: read or write. Note that this direction also applies
  /// to 'commit' and 'reserve' operations.
  enum class AccessKind { Read, Write };

  /// Operation which is performed on a pipe object.
  enum class OpKind {
    None,             ///< not a pipe built-in
    ReadWrite,        ///< actual read or write
    ReadWriteReserve, ///< read or write with reserve_id
    Reserve,          ///< reserve operation
    Commit            ///< commit operation
  };

  ScopeKind Scope;
  AccessKind Access;
  OpKind Op = OpKind::None;
  bool Blocking = false;
  bool IO = false;
  bool FPGA = false;
  std::string SimdSuffix = "";

  bool operator==(const PipeKind &LHS) const {
    return Scope == LHS.Scope && Access == LHS.Access && Op == LHS.Op &&
           Blocking == LHS.Blocking && IO == LHS.IO &&
           SimdSuffix == LHS.SimdSuffix && FPGA == LHS.FPGA;
  }

  operator bool() const { return Op != OpKind::None; }
};

namespace OclVersion {
enum {
  CL_VER_1_0 = 100,
  CL_VER_1_1 = 110,
  CL_VER_1_2 = 120,
  CL_VER_2_0 = 200,
  CL_VER_DEFAULT = CL_VER_1_2
};
} // namespace OclVersion

/// Helpful shortcuts for structures.
/// We use a SetVector to ensure determinstic iterations.
using BBSet = SetVector<BasicBlock *>;
using FuncSet = SetVector<Function *>;
using InstSet = SetVector<Instruction *>;
using BBVec = SmallVector<BasicBlock *, 16>;
using FuncVec = SmallVector<Function *, 16>;
using InstVec = SmallVector<Instruction *, 4>;
using InstVecVec = SmallVector<InstVec, 4>;
using ValueVec = SmallVector<Value *, 4>;

/// Return true if this alloca instruction is created by ImplicitGID Pass.
bool isImplicitGID(AllocaInst *AI);

/// Append the dimension string to S.
std::string AppendWithDimension(StringRef S, int Dimension);
std::string AppendWithDimension(StringRef S, const Value *Dimension);

/// Return true if string is __enqueue_kernel_*.
bool isEnqueueKernel(StringRef S);
bool isEnqueueKernelLocalMem(StringRef S);
bool isEnqueueKernelEventsLocalMem(StringRef S);

/// generatedFromOCLCPP - check that IR was generated from OCL C++
/// from "!spirv.Source" named metadata
bool isGeneratedFromOCLCPP(const Module &M);

/// generatedFromOMP - check that IR was generated from OpenMP.
bool isGeneratedFromOMP(const Module &M);

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

/// Return true if string is plain or mangled get_local_id.
bool isGetLocalId(StringRef S);

/// Return true if string is plain or mangled get_global_id.
bool isGetGlobalId(StringRef S);

///  Return true if string is name of atomic builtin.
bool isAtomicBuiltin(StringRef S);

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

/// Return true if string is "__opencl_printf".
bool isOpenCLPrintf(StringRef S);

/// Return true if string is printf.
bool isPrintf(StringRef S);

/// Return "__opencl_printf.".
StringRef nameOpenCLPrintf();

/// Return "printf".
StringRef namePrintf();

/// Return pipe kind from builtin name.
PipeKind getPipeKind(StringRef Name);

/// Returns pipe builtin name of a kind.
std::string getPipeName(PipeKind);

/// Return true if string is name of work-item pipe builtin.
bool isWorkItemPipeBuiltin(StringRef S);

/// Return true if \p S is wait_group_events builtin.
bool isWaitGroupEvents(StringRef S);

/// \name WorkGroup Builtin
/// \param S function name
/// @{
bool isWorkGroupReserveReadPipe(StringRef S);
bool isWorkGroupCommitReadPipe(StringRef S);
bool isWorkGroupReserveWritePipe(StringRef S);
bool isWorkGroupCommitWritePipe(StringRef S);
bool isWorkGroupAll(StringRef S);
bool isWorkGroupAny(StringRef S);
bool isWorkGroupBroadCast(StringRef S);
bool isWorkGroupReduceAdd(StringRef S);
bool isWorkGroupScanExclusiveAdd(StringRef S);
bool isWorkGroupScanInclusiveAdd(StringRef S);
bool isWorkGroupReduceMul(StringRef S);
bool isWorkGroupScanExclusiveMul(StringRef S);
bool isWorkGroupScanInclusiveMul(StringRef S);
bool isWorkGroupReduceMin(StringRef S);
bool isWorkGroupScanExclusiveMin(StringRef S);
bool isWorkGroupScanInclusiveMin(StringRef S);
bool isWorkGroupReduceMax(StringRef S);
bool isWorkGroupScanExclusiveMax(StringRef S);
bool isWorkGroupScanInclusiveMax(StringRef S);
bool isWorkGroupReduceBitwiseAnd(StringRef S);
bool isWorkGroupReduceBitwiseOr(StringRef S);
bool isWorkGroupReduceBitwiseXor(StringRef S);
bool isWorkGroupReduceLogicalAnd(StringRef S);
bool isWorkGroupReduceLogicalOr(StringRef S);
bool isWorkGroupReduceLogicalXor(StringRef S);
bool isWorkGroupBuiltin(StringRef S);
bool isWorkGroupAsyncOrPipeBuiltin(StringRef S, const Module &M);
bool isWorkGroupScan(StringRef S);
bool isWorkGroupMin(StringRef S);
bool isWorkGroupMax(StringRef S);
bool isWorkGroupMul(StringRef S);
bool isAsyncWorkGroupCopy(StringRef S);
bool isAsyncWorkGroupStridedCopy(StringRef S);
bool isWorkGroupBarrier(StringRef S);
/// }@

/// Returns true if \p S is a name of workgroup builtin, and it's uniform inside
/// a workgroup.
bool isWorkGroupBuiltinUniform(StringRef S);

/// Returns true if \p S is a name of workgroup builtin, and it's divergent
/// inside a workgroup.
bool isWorkGroupBuiltinDivergent(StringRef S);

/// Returns true if \p S is a workgroup or subgroup builtin, and it is uniform
/// inside a workgroup.
bool isWorkGroupUniform(StringRef S);

/// Returns true if \p S is a workgroup or subgroup builtin, and it is divergent
/// inside a workgroup.
bool isWorkGroupDivergent(StringRef S);

/// Returns true if \p S has "__finalize_" prefix.
bool hasWorkGroupFinalizePrefix(StringRef S);

/// Append "__finalize_" to \p S.
std::string appendWorkGroupFinalizePrefix(StringRef S);

/// Remove "__finalize_" prefix for \p S.
std::string removeWorkGroupFinalizePrefix(StringRef S);

/// Returns struct type with corresponding name if such exists
/// The main difference from Module::getTypeByName is that this function
/// doesn't account '.N' suffixes while comparing type names.
/// For example, if module contains only '__pipe_t.2' type:
///   * Module::getTypeByName('__pipe_t') will return nullptr
///   * getStructByName('__pipe_t', M) will return '__pipe_t_.2' type
StructType *getStructFromTypePtr(Type *Ty);

/// Check if two types actually have the same opaque ptr type.
/// Such types appear when 2 or more types were created with the same name.
/// These types differ only in .N name suffix, e.g.: %opencl.image2d_ro_t and
/// %opencl.image2d_ro_t.0
bool isSameStructType(StructType *STy1, StructType *STy2);

/// Replaces innermost element type from a pointer to a given struct type.
PointerType *mutatePtrElementType(PointerType *SrcPTy, Type *DstTy);

/// Import a declaration of \p Orig into module \p Dst.
Function *importFunctionDecl(Module *Dst, const Function *Orig,
                             bool DuplicateIfExists = false);

/// Returns the mangled name of the function atomic_work_item_fence.
std::string mangledAtomicWorkItemFence();

/// Returns the mangled name of the function get_global_id.
std::string mangledGetGID();

/// Return the mangled name of the function get_global_size.
std::string mangledGetGlobalSize();

/// Return the mangled name of the function get_global_offset.
std::string mangledGetGlobalOffset();

/// Return the mangled name of the function get_local_id.
std::string mangledGetLID();

/// Returns the mangled name of the function get_group_id.
std::string mangledGetGroupID();

/// Returns the mangled name of the function get_num_groups.
std::string mangledGetNumGroups();

/// Returns the mangled name of the function get_local_size.
std::string mangledGetLocalSize();

/// Returns the mangled name of the function get_enqueued_local_size.
std::string mangledGetEnqueuedLocalSize();

/// Returns the mangled name of the function barrier.
std::string mangledBarrier();

/// Returns the mangled name of the function work_group_barrier.
std::string mangledWGBarrier(BarrierType BT);

/// Returns the mangled name of the function sub_group_barrier.
/// \param BT
///   BarrierType::NoScope
///     void sub_group_barrier (cl_mem_fence_flags flags)
///   BarrierType::WithScope
///     void sub_group_barrier (cl_mem_fence_flags flags, memory_scope scope)
std::string mangledSGBarrier(BarrierType BT);

/// Returns the mangled name of the function get_num_sub_groups.
std::string mangledNumSubGroups();

/// Returns the mangled name of the function get_sub_group_id.
std::string mangledGetSubGroupId();

/// Returns the mangled name of the function get_enqueued_num_sub_groups.
std::string mangledEnqueuedNumSubGroups();

/// Returns the mangled name of the function get_sub_group_size.
std::string mangledGetSubGroupSize();

/// Returns the mangled name of the function get_max_sub_group_size.
std::string mangledGetMaxSubGroupSize();

/// Returns the mangled name of the function get_sub_group_local_id.
std::string mangledGetSubGroupLocalId();

/// Returns the mangled name of the function get_global_linear_id.
std::string mangledGetGlobalLinearId();

/// Returns the mangled name of the function get_local_linear_id.
std::string mangledGetLocalLinearId();

/// \name subgroup builtins.
/// \param S function name.
/// @{
bool isGetSubGroupSize(StringRef S);
bool isGetMaxSubGroupSize(StringRef S);
bool isGetNumSubGroups(StringRef S);
bool isGetEnqueuedNumSubGroups(StringRef S);
bool isGetSubGroupId(StringRef S);
bool isGetSubGroupLocalId(StringRef S);
bool isSubGroupAll(StringRef S);
bool isSubGroupAny(StringRef S);
bool isSubGroupBroadCast(StringRef S);
bool isSubGroupReduceAdd(StringRef S);
bool isSubGroupScanExclusiveAdd(StringRef S);
bool isSubGroupScanInclusiveAdd(StringRef S);
bool isSubGroupReduceMin(StringRef S);
bool isSubGroupScanExclusiveMin(StringRef S);
bool isSubGroupScanInclusiveMin(StringRef S);
bool isSubGroupReduceMax(StringRef S);
bool isSubGroupScanExclusiveMax(StringRef S);
bool isSubGroupScanInclusiveMax(StringRef S);
bool isSubGroupScan(StringRef S);
/// }@

/// Returns true if \p S is a name of subgroup builtin, and it's uniform inside
/// a subgroup.
bool isSubGroupUniform(StringRef S);

/// Returns true if \p S is a name of subgroup builtin, and it's non-uniform
/// (divergent) inside a subgroup.
bool isSubGroupDivergent(StringRef S);

/// Returns true if \p S is a name of subgroup builtin.
bool isSubGroupBuiltin(StringRef S);

/// Returns true if \p S is the name of subgroup barrier.
bool isSubGroupBarrier(StringRef S);

/// Returns true if \p S is the name of get_sub_group_slice_length.
bool isGetSubGroupSliceLength(StringRef S);

/// Returns true if \p S is the name of sub_group_rowslice_extractelement.
bool isSubGroupRowSliceExtractElement(StringRef S);

/// Returns true if \p S is the name of sub_group_rowslice_insertelement.
bool isSubGroupRowSliceInsertElement(StringRef S);

/// Returns true if \p S is the name of sub_group_insert_rowslice_to_matrix.
bool isSubGroupInsertRowSliceToMatrix(StringRef S);

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

/// Collect built-ins declared in the module and force synchronization, i.e.
/// implemented using barrier built-in.
/// \param M the module to search synchronize built-ins declarations in.
/// \param IsWG true for workgroup, false for subgroup.
/// \returns container to insert all synchronized built-ins into.
FuncSet getAllSyncBuiltinsDecls(Module &M, bool IsWG = true);

/// Collect built-ins declared in the module that require relaxation of
/// noduplicate attribute to convergent. Additionally assigns
/// "kernel-convergent-call" and "kernel-call-once" attributes (see LangRef).
/// \param M the module to search built-ins declarations in.
/// \returns container of collected built-ins.
FuncSet getAllSyncBuiltinsDeclsForNoDuplicateRelax(Module &M);

/// Collect built-ins declared in the module that require assigning of
/// "kernel-uniform-call" attribute (see LangRef for details).
FuncSet getAllSyncBuiltinsDeclsForKernelUniformCallAttr(Module &M);

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

/// Move instructions, which meets the requirements of Predicate, from FromBB to
/// ToBB.
void moveInstructionIf(BasicBlock *FromBB, BasicBlock *ToBB,
                       function_ref<bool(Instruction &)> Predicate);

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

inline bool hasByvalByrefArgs(const Function *F) {
  if (!F)
    return false;
  return llvm::any_of(F->args(), [](auto &Arg) {
    return Arg.hasByValAttr() || Arg.hasByRefAttr();
  });
}

/// Create instruction(s) which is semantically equivalent to the original
/// constant \a Original, with operand \a From replaced with \a To.
/// Only expect \a ConstantExpr and \a ConstantAggregate for \a Original.
Instruction *createInstructionFromConstantWithReplacement(
    Constant *Original, Value *From, Value *To, Instruction *InsertPoint);

/// Whether the CallGraphNode `Node` contains a call to the function that
/// satisfies the given `Condition`. This will perform a DFS on the CallGraph.
/// Returns true if `Node->getFunction()` calls target function
/// directly/indirectly.
bool hasFunctionCallInCGNodeIf(CallGraphNode *Node,
                               function_ref<bool(const Function *)> Condition);

/// Apply `MapFunc` to all functions satisfied with the given `Condition` in the
/// CallGraph `Node`.
void mapFunctionCallInCGNodeIf(CallGraphNode *Node,
                               function_ref<bool(const Function *)> Condition,
                               function_ref<void(Function *)> MapFunc);

void initializeVectInfoOnce(
    ArrayRef<VectItem> VectInfos,
    std::vector<std::tuple<std::string, std::string, std::string>>
        &ExtendedVectInfos);

/// Insert printf in the kernel for debug purpose.
void insertPrintf(const Twine &Prefix, Instruction *IP,
                  ArrayRef<Value *> Inputs = None);

/// Check whether the given FixedVectorType represents a valid SYCL matrix.
bool isValidMatrixType(FixedVectorType *MatrixType);

/// Create a get_sub_group_slice_length.() call.
/// SIGNATURE:
///   i64 get_sub_group_slice_length.(i32 immarg %total.element.count)
/// This internal builtin will be resolved by ResolveSubGroupWICall pass
/// as: ceil(%total.element.count / VF)
CallInst *createGetSubGroupSliceLengthCall(unsigned TotalElementCount,
                                           Instruction *IP,
                                           const Twine &Name = "");

/// Create a get_sub_group_rowslice_id() call.
/// SIGNATURE:
///   i64 get_sub_group_rowslice_id.<MatrixTypeMangle>.<IndexTypeMangle>(
///   <MatrixType> %matrix, i32 immarg R, i32 immarg C, <IndexType> %index)
/// where <MatrixType> must be a FixedVectorType, whose number of elements
/// equals to R times C;
/// and <IndexType> is an arbitrary integer type.
/// This internal builtin will be resolved by ResolveSubGroupWICall pass.
CallInst *createGetSubGroupRowSliceIdCall(Value *Matrix, unsigned R, unsigned C,
                                          Value *Index, Instruction *IP,
                                          const Twine &Name = "");

/// Create a sub_group_rowslice_extractelement() call.
/// SIGNATURE:
///   <ElementType> sub_group_rowslice_extractelement.<ElementTypeMangle>(i64
///   %rowslice.id)
/// where %rowslice.id must be a call of get_sub_group_rowslice_id;
/// and <ElementType> must match with the element type of the matrix argument
/// associated with %rowslice.id.
/// This internal builtin will be widen by vectorizer and resolved by
/// ResolveSubGroupWICall pass.
CallInst *createSubGroupRowSliceExtractElementCall(Value *RowSliceId,
                                                   Type *ReturnType,
                                                   Instruction *IP,
                                                   const Twine &Name = "");

/// Create a sub_group_rowslice_insertelement() call.
/// SIGNATURE:
///   void sub_group_rowslice_insertelement.<ElementTypeMangle>(
///   i64 %rowslice.id, <ElementType> %element)
/// where %rowslice.id must be a call of get_sub_group_rowslice_id;
/// and <ElementType> must match with the element type of the matrix argument
/// associated with %rowslice.id.
/// This internal builtin will be widen by vectorizer and resolved by
/// ResolveSubGroupWICall pass.
CallInst *createSubGroupRowSliceInsertElementCall(Value *RowSliceId,
                                                  Value *Data, Instruction *IP);

/// Create a sub_group_insert_rowslice_to_matrix() call.
/// SIGNATURE:
///   <MatrixType> sub_group_insert_rowslice_to_matrix.<MatrixTypeMangle>(i64
///   %rowslice.id)
/// where %rowslice.id must be a call of get_sub_group_rowslice_id;
/// and <MatrixType> must be the same as the typo of matrix argument associated
/// with %rowslice.id.
/// This internal builtin will be resolved by ResolveSubGroupWICall pass.
CallInst *createSubGroupInsertRowSliceToMatrixCall(Value *RowSliceId,
                                                   Type *ReturnMatrixType,
                                                   Instruction *IP,
                                                   const Twine &Name = "");

void calculateMemorySizeWithPostOrderTraversal(
    CallGraph &CG, DenseMap<Function *, size_t> &FnDirectSize,
    DenseMap<Function *, size_t> &FnSize);

} // namespace DPCPPKernelCompilationUtils
} // namespace llvm

#endif
