// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
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

#ifndef __COMPILATION_UTILS_H__
#define __COMPILATION_UTILS_H__

#include "exceptions.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Module.h"

#include <PipeCommon.h>
#include <OCLAddressSpace.h>

#include <set>
#include <string>
#include <vector>
#include <map>

namespace llvm {
  class CallInst;
  class Function;
  class Module;
  class Value;
}
using namespace llvm;

namespace Intel { namespace OpenCL { namespace DeviceBackend {

  DEFINE_EXCEPTION(CompilerException)

  namespace OclVersion {

    static const unsigned CL_VER_1_0 = 100;
    static const unsigned CL_VER_1_1 = 110;
    static const unsigned CL_VER_1_2 = 120;
    static const unsigned CL_VER_2_0 = 200;
    // The default revision defined by OpenCL spec
    static const unsigned CL_VER_DEFAULT = CL_VER_1_2;

    unsigned CLStrToVal(const char* S);
    unsigned CLVersionToVal(uint64_t major, uint64_t minor);
  }

  // Document what source language this module was translated from.
  // Values defined by SPIR-V spec.
  namespace OclLanguage {
    static const unsigned OpenCL_CPP = 4;
  }

  namespace ChannelPipeMetadata {
    struct ChannelPipeMD {
      int PacketSize;
      int PacketAlign;
      int Depth;
      StringRef IO;
    };

    ChannelPipeMD getChannelPipeMetadata(
        GlobalVariable *Channel,
        int ChannelDepthEmulationMode = CHANNEL_DEPTH_MODE_STRICT);
  }

  struct PipeKind {
    enum ScopeKind {
      WORK_ITEM,
      WORK_GROUP,
      SUB_GROUP
    };

    /// Access direction: read or write. Note that this direction also applies
    /// to 'commit' and 'reserve' operations.
    enum AccessKind {
      READ,
      WRITE
    };

    /// Operation which is performed on a pipe object.
    enum OpKind {
      NONE,              ///< not a pipe built-in
      READWRITE,         ///< actual read or write
      READWRITE_RESERVE, ///< read or write with reserve_id
      RESERVE,           ///< reserve operation
      COMMIT             ///< commit operation
    };

    ScopeKind Scope;
    AccessKind Access;
    OpKind Op = OpKind::NONE;
    bool Blocking = false;
    bool IO = false;
    bool FPGA = false;
    std::string SimdSuffix = "";

    bool operator == (const PipeKind &LHS) const {
      return Scope      == LHS.Scope      &&
             Access     == LHS.Access     &&
             Op         == LHS.Op         &&
             Blocking   == LHS.Blocking   &&
             IO         == LHS.IO         &&
             SimdSuffix == LHS.SimdSuffix &&
             FPGA       == LHS.FPGA;
    }

    operator bool () const {
      return Op != OpKind::NONE;
    }
  };

  struct ChannelKind {
    enum AccessKind {
      NONE, // not a channel
      READ,
      WRITE
    };

    AccessKind Access;
    bool Blocking;

    bool operator == (const ChannelKind &LHS) const {
      return Access   == LHS.Access   &&
             Blocking == LHS.Blocking;
    }

    operator bool () const {
      return Access != AccessKind::NONE;
    }
  };

  /// @brief  CompilationUtils class used to provide helper utilies that are
  ///         used by several other classes.
  class CompilationUtils {

  public:
    /// We use a SetVector to ensure determinstic iterations
    typedef SetVector<Function*> FunctionSet;
    /// @brief Removes the from the given basic block the instruction pointed
    ///        by the given iterator
    /// @param pBB         A Basic block from which the instruction needs to be removed
    /// @param it          An iterator pointing to the instruction that needs to be removed
    /// @returns An iterator to the next instruction after the instruction that was removed
    static BasicBlock::iterator removeInstruction(BasicBlock* pBB, BasicBlock::iterator it);

    /// @brief  Retrieves the pointer to the implicit arguments added to the given function
    /// @param  pFunc        The function for which implicit arguments need to be retrieved
    /// @param  ppLocalMem   The pLocalMem argument, NULL if this argument shouldn't be retrieved
    /// @param  ppWorkDim    The pWorkDim argument, NULL if this argument shouldn't be retrieved
    /// @param  ppWGId       The pWGId argument, NULL if this argument shouldn't be retrieved
    /// @param  ppBaseGlbId  The pBaseGlbId argument, NULL if this argument shouldn't be retrieved
    /// @param  ppSpecialBuf The SpecialBuf argument, NULL if this argument shouldn't be retrieved
    /// @param  ppCtx        The pCtx argument, NULL if this argument shouldn't be retrieved
    /// @param  ppExtExecCtx The ExtendedExecutionContext argument, NULL if this argument shouldn't be retrieved
    static void getImplicitArgs(Function *pFunc, Value **ppLocalMem,
                                Value **ppWorkDim, Value **ppWGId,
                                Value **ppBaseGlbId, Value **ppSpecialBuf,
                                Value **ppRunTimeHandle);

    /// @brief  Retrieves requested TLS global variable in the given module
    /// @param  pModule The module for which global variable needs to be
    /// retrieved
    /// @param  Idx     The ImplicitArgsUtils::ImplicitArg index of the
    /// requested global
    static GlobalVariable *getTLSGlobal(Module *pModule, unsigned Idx);

    /// @brief Moves alloca instructions from FromBB to ToBB
    static void moveAlloca(BasicBlock *FromBB, BasicBlock *ToBB);

    /// @brief collect built-ins declared in the module that require
    //         relaxation of noduplicate attribute to convergent.
    static void getAllSyncBuiltinsDclsForNoDuplicateRelax(
      FunctionSet &functionSet, Module *pModule);

    /// @brief collect built-ins declared in the module and force synchronization.
    //         I.e. implemented using barrier built-in.
    /// @param functionSet container to insert all synchronized built-ins into
    /// @param pModule the module to search synchronize built-ins declarations in
    static void getAllSyncBuiltinsDcls(FunctionSet &functionSet, Module *pModule);

    /// @brief collect all kernel functions
    /// @param functionSet container to insert all kernel function into
    /// @param pModule the module to search kernel function inside
    static void getAllKernels(FunctionSet &functionSet, Module *pModule);

    /// @brief collect all kernel wrapper functions
    /// @param functionSet container to insert all kernel wrapper function into
    /// @param pModule the module to search kernel wrapper function inside
    static void getAllKernelWrappers(FunctionSet &functionSet, Module *pModule);

    /// @brief  fills a vector of cl_kernel_argument with arguments representing pFunc's
    ///         OpenCL level arguments
    /// @param pModule    The module
    /// @param pFunc      The kernel for which to create argument vector
    /// @param arguments  OUT param, the cl_kernel_argument which represent pFunc's
    ///                   OpenCL level argument
    static void
    parseKernelArguments(Module *pModule, Function *pFunc, bool useTLSGlobals,
                         std::vector<cl_kernel_argument> & /* OUT */ arguments,
                         std::vector<unsigned int> & /* OUT */ memoryArguments);

    static Function *AddMoreArgsToFunc(Function *F, ArrayRef<Type *> NewTypes,
                                       ArrayRef<const char *> NewNames,
                                       ArrayRef<AttributeSet> NewAttrs,
                                       StringRef Prefix);
    // AddMoreArgsToCall - Replaces a CallInst with a new CallInst which has the
    // same arguments as orignal plus more args appeneded.
    // Returns the new CallInst
    // OldC - the original CallInst to be replaced
    // NewArgs - New arguments to append to existing arguments
    // NewF - a suitable prototype of new Function to be called
    static CallInst *AddMoreArgsToCall(CallInst *OldC,
                                       ArrayRef<Value *> NewArgs,
                                       Function *NewF);

    /// @brief Replaces an indirect CallInst with a new indirect CallInst which
    ///        has the same arguments as orignal plus more args appeneded.
    /// @param OldC the original CallInst to be replaced
    /// @NewArgs New arguments to append to existing arguments
    /// @returns the new CallInst
    static CallInst *AddMoreArgsToIndirectCall(CallInst *OldC,
                                              ArrayRef<Value *> NewArgs);
    static bool isGetWorkDim(const std::string&);
    static bool isGetGlobalId(const std::string&);
    static bool isGetGlobalSize(const std::string&);
    static bool isGetLocalId(const std::string&);
    static bool isGetLocalSize(const std::string&);
    static bool isGetSubGroupSize(const std::string&);
    static bool isGetMaxSubGroupSize(const std::string&);
    static bool isGetEnqueuedLocalSize(const std::string&);
    static bool isGetGlobalLinearId(const std::string&);
    static bool isGetLocalLinearId(const std::string&);
    static bool isGetSubGroupLocalId(const std::string&);
    static bool isGetSubGroupId(const std::string&);
    static bool isGetNumGroups(const std::string&);
    static bool isGetNumSubGroups(const std::string&);
    static bool isGetEnqueuedNumSubGroups(const std::string&);
    static bool isGetGroupId(const std::string&);
    static bool isGlobalOffset(const std::string&);
    static bool isAsyncWorkGroupCopy(const std::string&);
    static bool isAsyncWorkGroupStridedCopy(const std::string&);
    static bool isWorkGroupReserveReadPipe(const std::string&);
    static bool isWorkGroupCommitReadPipe(const std::string&);
    static bool isWorkGroupReserveWritePipe(const std::string&);
    static bool isWorkGroupCommitWritePipe(const std::string&);
    static bool isSubGroupReserveReadPipe(const std::string&);
    static bool isSubGroupCommitReadPipe(const std::string&);
    static bool isSubGroupReserveWritePipe(const std::string&);
    static bool isSubGroupCommitWritePipe(const std::string&);
    static bool isWaitGroupEvents(const std::string&);
    static bool isPrefetch(const std::string&);
    static bool isMemFence(const std::string&);
    static bool isReadMemFence(const std::string&);
    static bool isWriteMemFence(const std::string&);
    static bool isEnqueueKernel(const std::string& S);
    static bool isEnqueueKernelLocalMem(const std::string&);
    static bool isEnqueueKernelEventsLocalMem(const std::string&);
    static bool isWorkGroupAll(const std::string&);
    static bool isWorkGroupAny(const std::string&);
    static bool isSubGroupAll(const std::string&);
    static bool isSubGroupAny(const std::string&);
    static bool isSubGroupBarrier(const std::string&);
    static bool isWorkGroupBroadCast(const std::string&);
    static bool isSubGroupBroadCast(const std::string&);
    static bool isWorkGroupReduceAdd(const std::string&);
    static bool isWorkGroupScanExclusiveAdd(const std::string&);
    static bool isWorkGroupScanInclusiveAdd(const std::string&);
    static bool isWorkGroupReduceMin(const std::string&);
    static bool isWorkGroupScanExclusiveMin(const std::string&);
    static bool isWorkGroupScanInclusiveMin(const std::string&);
    static bool isWorkGroupReduceMax(const std::string&);
    static bool isWorkGroupScanExclusiveMax(const std::string&);
    static bool isWorkGroupScanInclusiveMax(const std::string&);
    static bool hasWorkGroupFinalizePrefix(const std::string&);
    static bool isSubGroupReduceAdd(const std::string&);
    static bool isSubGroupScanExclusiveAdd(const std::string&);
    static bool isSubGroupScanInclusiveAdd(const std::string&);
    static bool isSubGroupReduceMin(const std::string&);
    static bool isSubGroupScanExclusiveMin(const std::string&);
    static bool isSubGroupScanInclusiveMin(const std::string&);
    static bool isSubGroupReduceMax(const std::string&);
    static bool isSubGroupScanExclusiveMax(const std::string&);
    static bool isSubGroupScanInclusiveMax(const std::string&);
    static std::string appendWorkGroupFinalizePrefix(const std::string&);
    static std::string removeWorkGroupFinalizePrefix(const std::string&);

    static bool isWorkGroupBuiltin(const std::string&);
    static bool isWorkGroupAsyncOrPipeBuiltin(const std::string&, const Module*);
    static bool isWorkGroupScan(const std::string&);
    static bool isWorkGroupMin(const std::string&);
    static bool isWorkGroupMax(const std::string&);
    static bool isWorkGroupUniform(const std::string&);

    static bool isAtomicBuiltin(const std::string&);
    static bool isWorkItemPipeBuiltin(const std::string&);
    static bool isAtomicWorkItemFenceBuiltin(const std::string&);

    static bool isPipeBuiltin(const std::string&);
    static bool isReadPipeBuiltin(const std::string&);
    static bool isWritePipeBuiltin(const std::string&);
    static PipeKind getPipeKind(const std::string&);
    static std::string getPipeName(PipeKind);
    static ChannelKind getChannelKind(const std::string&);

    static const std::string NAME_GET_GID;
    static const std::string NAME_GET_LID;
    static const std::string NAME_GET_SUB_GROUP_ID;
    static const std::string NAME_GET_SUB_GROUP_LOCAL_ID;

    static const std::string NAME_GET_WORK_DIM;
    static const std::string NAME_GET_GLOBAL_SIZE;
    static const std::string NAME_GET_LOCAL_SIZE;
    static const std::string NAME_GET_SUB_GROUP_SIZE;
    static const std::string NAME_GET_MAX_SUB_GROUP_SIZE;
    static const std::string NAME_GET_ENQUEUED_LOCAL_SIZE;
    static const std::string NAME_GET_NUM_GROUPS;
    static const std::string NAME_GET_NUM_SUB_GROUPS;
    static const std::string NAME_GET_ENQUEUED_NUM_SUB_GROUPS;
    static const std::string NAME_GET_GROUP_ID;
    static const std::string NAME_GET_GLOBAL_OFFSET;
    static const std::string NAME_PRINTF;

    static const std::string NAME_ASYNC_WORK_GROUP_COPY;
    static const std::string NAME_WAIT_GROUP_EVENTS;
    static const std::string NAME_PREFETCH;
    static const std::string NAME_ASYNC_WORK_GROUP_STRIDED_COPY;

    static const std::string NAME_WORK_GROUP_RESERVE_READ_PIPE;
    static const std::string NAME_WORK_GROUP_COMMIT_READ_PIPE;
    static const std::string NAME_WORK_GROUP_RESERVE_WRITE_PIPE;
    static const std::string NAME_WORK_GROUP_COMMIT_WRITE_PIPE;

    static const std::string NAME_SUB_GROUP_RESERVE_READ_PIPE;
    static const std::string NAME_SUB_GROUP_COMMIT_READ_PIPE;
    static const std::string NAME_SUB_GROUP_RESERVE_WRITE_PIPE;
    static const std::string NAME_SUB_GROUP_COMMIT_WRITE_PIPE;

    static const std::string NAME_ATOMIC_WORK_ITEM_FENCE;

    static const std::string NAME_MEM_FENCE;
    static const std::string NAME_READ_MEM_FENCE;
    static const std::string NAME_WRITE_MEM_FENCE;

    static const std::string NAME_GET_LINEAR_GID;
    static const std::string NAME_GET_LINEAR_LID;

    static const std::string BARRIER_FUNC_NAME;
    static const std::string WG_BARRIER_FUNC_NAME;
    static const std::string SG_BARRIER_FUNC_NAME;

    //work-group functions
    static const std::string NAME_WORK_GROUP_ALL;
    static const std::string NAME_WORK_GROUP_ANY;
    static const std::string NAME_WORK_GROUP_BROADCAST;
    static const std::string NAME_WORK_GROUP_REDUCE_ADD;
    static const std::string NAME_WORK_GROUP_SCAN_EXCLUSIVE_ADD;
    static const std::string NAME_WORK_GROUP_SCAN_INCLUSIVE_ADD;
    static const std::string NAME_WORK_GROUP_REDUCE_MIN;
    static const std::string NAME_WORK_GROUP_SCAN_EXCLUSIVE_MIN;
    static const std::string NAME_WORK_GROUP_SCAN_INCLUSIVE_MIN;
    static const std::string NAME_WORK_GROUP_REDUCE_MAX;
    static const std::string NAME_WORK_GROUP_SCAN_EXCLUSIVE_MAX;
    static const std::string NAME_WORK_GROUP_SCAN_INCLUSIVE_MAX;
    static const std::string NAME_FINALIZE_WG_FUNCTION_PREFIX;

    //sub-group functions
    static const std::string NAME_SUB_GROUP_ALL;
    static const std::string NAME_SUB_GROUP_ANY;
    static const std::string NAME_SUB_GROUP_BROADCAST;
    static const std::string NAME_SUB_GROUP_REDUCE_ADD;
    static const std::string NAME_SUB_GROUP_SCAN_EXCLUSIVE_ADD;
    static const std::string NAME_SUB_GROUP_SCAN_INCLUSIVE_ADD;
    static const std::string NAME_SUB_GROUP_REDUCE_MIN;
    static const std::string NAME_SUB_GROUP_SCAN_EXCLUSIVE_MIN;
    static const std::string NAME_SUB_GROUP_SCAN_INCLUSIVE_MIN;
    static const std::string NAME_SUB_GROUP_REDUCE_MAX;
    static const std::string NAME_SUB_GROUP_SCAN_EXCLUSIVE_MAX;
    static const std::string NAME_SUB_GROUP_SCAN_INCLUSIVE_MAX;
    static const std::string NAME_FINALIZE_SG_FUNCTION_PREFIX;

    //images
    static const std::string OCL_IMG_PREFIX;
    static const std::string IMG_2D;
    static const std::string IMG_2D_ARRAY;
    static const std::string IMG_3D;
    static const char* ImageTypeNames[];

    //kernel arg qualifiers
    static const std::string WRITE_ONLY;
    static const std::string READ_ONLY;
    static const std::string NONE;
    //kernel type qualifiers
    static const std::string SAMPLER;

    /// '3' is a magic number for global variables
    /// that were in origin kernel local variable!
    static const unsigned int LOCL_VALUE_ADDRESS_SPACE;

    static const std::string NAME_GET_BASE_GID;
    static const std::string NAME_GET_SPECIAL_BUFFER;

    static const std::string WG_BOUND_PREFIX;

    //////////////////////////////////////////////////////////////////
    // @brief returns the mangled name of the function mem_fence
    //////////////////////////////////////////////////////////////////
    static std::string mangledMemFence();
    //////////////////////////////////////////////////////////////////
    // @brief returns the mangled name of the function atomic_work_item_fence
    //////////////////////////////////////////////////////////////////
    static std::string mangledAtomicWorkItemFence();
    //////////////////////////////////////////////////////////////////
    // @brief returns the mangled name of the function get_global_id
    //////////////////////////////////////////////////////////////////
    static std::string mangledGetGID();
    //////////////////////////////////////////////////////////////////
    // @brief returns the mangled name of the function get_global_size
    //////////////////////////////////////////////////////////////////
    static std::string mangledGetGlobalSize();
    //////////////////////////////////////////////////////////////////
    // @brief returns the mangled name of the function get_global_offset
    //////////////////////////////////////////////////////////////////
    static std::string mangledGetGlobalOffset();
    //////////////////////////////////////////////////////////////////
    // @brief: returns the mangled name of the function get_local_id
    //////////////////////////////////////////////////////////////////
    static std::string mangledGetLID();
    //////////////////////////////////////////////////////////////////
    // @brief: returns the mangled name of the function get_group_id
    //////////////////////////////////////////////////////////////////
    static std::string mangledGetGroupID();
    //////////////////////////////////////////////////////////////////
    // @brief: returns the mangled name of the function get_local_size
    //////////////////////////////////////////////////////////////////
    static std::string mangledGetLocalSize();
    //////////////////////////////////////////////////////////////////
    // @brief: returns the mangled name of the function get_num_groups
    //////////////////////////////////////////////////////////////////
    static std::string mangledGetNumGroups();
    //////////////////////////////////////////////////////////////////
    // @brief: returns the mangled name of the function get_enqueued_local_size
    //////////////////////////////////////////////////////////////////
    static std::string mangledGetEnqueuedLocalSize();
    //////////////////////////////////////////////////////////////////
    // @brief: returns the mangled name of the barrier function
    //////////////////////////////////////////////////////////////////
    static std::string mangledBarrier();
    //////////////////////////////////////////////////////////////////
    // @brief: returns the mangled name of the sub_group_barrier
    //////////////////////////////////////////////////////////////////
    static std::string mangledSGBarrier();
    //////////////////////////////////////////////////////////////////
    // @brief: returns the mangled name of the work_group_barrier function
    // @param wgBarrierType
    //                      BARRIER_NO_SCOPE - for
    // void work_group_barrier (cl_mem_fence_flags flags)
    //                      BARRIER_WITH_SCOPE - for
    // void work_group_barrier (cl_mem_fence_flags flags, memory_scope scope)
    //////////////////////////////////////////////////////////////////
    typedef enum {
      BARRIER_NO_SCOPE,
      BARRIER_WITH_SCOPE
    } BARRIER_TYPE;
    static std::string mangledWGBarrier(BARRIER_TYPE wgBarrierType);
    //////////////////////////////////////////////////////////////////
    // @brief: returns the mangled name of the sub_group_barrier function
    // @param sgBarrierType
    //                      BARRIER_NO_SCOPE - for
    // void sub_group_barrier (cl_mem_fence_flags flags)
    //                      BARRIER_WITH_SCOPE - for
    // void sub_group_barrier (cl_mem_fence_flags flags, memory_scope scope)
    //////////////////////////////////////////////////////////////////
    static std::string mangledSGBarrier(BARRIER_TYPE sgBarrierType);

    //////////////////////////////////////////////////////////////////
    // @brief: returns the mangled name of the function get_sub_group_local_id
    //////////////////////////////////////////////////////////////////
    static std::string mangledGetSubGroupLID();
    //////////////////////////////////////////////////////////////////
    // @brief: returns the mangled name of the function get_num_sub_groups
    //////////////////////////////////////////////////////////////////
    static std::string mangledNumSubGroups();
    //////////////////////////////////////////////////////////////////
    // @brief: returns the mangled name of the function get_sub_group_id
    //////////////////////////////////////////////////////////////////
    static std::string mangledGetSubGroupId();
    //////////////////////////////////////////////////////////////////
    // @brief: returns the mangled name of the function get_enqueued_num_sub_groups
    //////////////////////////////////////////////////////////////////
    static std::string mangledEnqueuedNumSubGroups();
    //////////////////////////////////////////////////////////////////
    // @brief: returns the mangled name of the function get_sub_group_size
    //////////////////////////////////////////////////////////////////
    static std::string mangledGetSubGroupSize();
    //////////////////////////////////////////////////////////////////
    // @brief: returns the mangled name of the function get_max_sub_group_size
    static std::string mangledGetMaxSubGroupSize();
    //////////////////////////////////////////////////////////////////
    // @brief: returns the mangled name of the function get_sub_group_local_id
    //////////////////////////////////////////////////////////////////
    static std::string mangledGetSubGroupLocalId();
    //////////////////////////////////////////////////////////////////
    // @brief: returns the mangled name of the function get_global_linear_id
    //////////////////////////////////////////////////////////////////
    static std::string mangledGetGlobalLinearId();
    //////////////////////////////////////////////////////////////////
    // @brief: returns the mangled name of the function get_local_linear_id
    //////////////////////////////////////////////////////////////////
    static std::string mangledGetLocalLinearId();

    static const std::string NAME_GET_DEFAULT_QUEUE;

    /// ndrange_t ndrange_1D (). matches function with 1, 2, 3 arguments
    static const std::string NAME_NDRANGE_1D;
    /// ndrange_t ndrange_2D (). matches function with 1, 2, 3 arguments
    static const std::string NAME_NDRANGE_2D;
    /// ndrange_t ndrange_3D (). matches function with 1, 2, 3 arguments
    static const std::string NAME_NDRANGE_3D;
    /// Enqueue kernel built-ins
    static const std::string NAME_ENQUEUE_KERNEL;

    /// get maximum work-group size that can be used
    /// to execute a block on a specific device
    /// uint get_kernel_work_group_size
    static const std::string NAME_GET_KERNEL_WG_SIZE;
    static const std::string NAME_GET_KERNEL_WG_SIZE_LOCAL;

    /// Returns the preferred multiple of work-group
    /// size for launch
    /// uint get_kernel_preferred_work_group_size_multiple
    static const std::string NAME_GET_KERNEL_PREFERRED_WG_SIZE_MULTIPLE;
    static const std::string NAME_GET_KERNEL_PREFERRED_WG_SIZE_MULTIPLE_LOCAL;

    static const std::string NAME_GET_KERNEL_SG_COUNT_FOR_NDRANGE;
    static const std::string NAME_GET_KERNEL_MAX_SG_SIZE_FOR_NDRANGE;

    /// int enqueue_marker (
    ///     queue_t queue,
    ///     uint num_events_in_wait_list,
    ///     const clk_event_t *event_wait_list,
    ///     clk_event_t *event_ret)
    static const std::string NAME_ENQUEUE_MARKER;

    /// void retain_event (clk_event_t event)
    static const std::string NAME_RETAIN_EVENT;

    /// void release_event (clk_event_t event)
    static const std::string NAME_RELEASE_EVENT;

    /// clk_event_t create_user_event ()
    static const std::string NAME_CREATE_USER_EVENT;

    /// void set_user_event_status (
    ///     clk_event_t event,
    ///     int status)
    static const std::string NAME_SET_USER_EVENT_STATUS;

    /// void capture_event_profiling_info (
    ///     clk_event_t event,
    ///     clk_profiling_info name,
    ///     global ulong *value)
    static const std::string NAME_CAPTURE_EVENT_PROFILING_INFO;

    /// fetchCLVersionFromMetadata - obtain CL version
    /// from "!opencl.ocl.version" named metadata
    static unsigned fetchCLVersionFromMetadata(const Module &M);

    /// getDebugFlagFromMetadata - check opencl.compiler.options
    /// for -g flag
    static bool getDebugFlagFromMetadata(Module *M);

    /// generatedFromOCLCPP - check that IR was generated from OCL C++
    /// from "!spirv.Source" named metadata
    static bool generatedFromOCLCPP(const Module &M);

    /// generatedFromSPIRV - check that IR was generated from SPIRV
    static bool generatedFromSPIRV(const Module &M);

    /// Import a declaration of \p Orig into module \p Dst
    ///
    /// Parameter types will be translated to match the corresponding
    /// types in the \p Dst.
    ///
    /// @return function declaration Function* (if import succeed) or a
    /// bitcast if a function with the same name, but different type, is
    /// already exist in the \p Dst.
    static Function *importFunctionDecl(Module *Dst, const Function *Orig);

    /// Check if at least one of the image types is defined in the module
    static bool isImagesUsed(const Module &M);

    /// Check if two types are pointers to the same opaque type
    /// @see isSameStructType
    static bool isSameStructPtrType(Type *Ty1, Type *Ty2);

    /// Check if two types actually have the same opaque ptr type.
    ///
    /// Such types appear when 2 or more types were created with the same name.
    /// These types differ only in .N name suffix, e.g.:
    /// %opencl.image2d_ro_t and %opencl.image2d_ro_t.0
    static bool isSameStructType(StructType *STy1, StructType *Ty2);

    /// Return a type name without .N suffix (if any)
    static StringRef stripStructNameTrailingDigits(StringRef TyName);

    /// Returns struct type with corresponding name if such exists
    /// The main difference from Module::getTypeByName is that this function
    /// doesn't account '.N' suffixes while comparing type names.
    ///
    /// For example, if module contains only '__pipe_t.2' type:
    ///   * Module::getTypeByName('__pipe_t') will return nullptr
    ///   * getStructByName('__pipe_t', M) will return '__pipe_t_.2' type
    static StructType *getStructByName(StringRef Name, const Module *M);

    /// Return a pointer to struct from a pointer to type
    static StructType* getStructFromTypePtr(Type *T);

    /// Replaces innermost element type from a pointer to a given struct type
    static PointerType *mutatePtrElementType(PointerType *SrcPTy, Type *DstTy);

    /// @brief Calculates the total number of elements contained by ArrayType
    ///
    /// Examples:
    ///   arr[5] -> 5
    ///   arr[5][4] -> 20
    ///   arr[5][4][3] -> 60
    ///
    /// @return Total number of elements
    static size_t getArrayNumElements(const ArrayType *ArrTy);

    /// @brief Returns the undelying type of the ArrayType
    ///
    /// Functionality is similar to clang::ASTContext::getBaseElementType
    static Type* getArrayElementType(const ArrayType *ArrTy);

    static ArrayType * createMultiDimArray(
        Type *Ty, const ArrayRef<size_t> &Dimensions);

    /// @brief Returns vector of numbers of elements in each dimension of the
    ///        ArrayType
    static void getArrayTypeDimensions(const ArrayType *ArrTy,
                                       SmallVectorImpl<size_t> &Dimensions);

    /// @brief Returns true if the function is global constructor or destructor
    //         (listed in @llvm.global_ctors variable)
    //
    //         NOTE: current implementation is *the only* workaround for global
    //         ctor/dtor for pipes. See TODO inside the implementation
    static bool isGlobalCtorDtor(Function *F);

    /// @brief Returns true if the function is a block invoke kernel
    static bool isBlockInvocationKernel(Function *F);

    /// @brief Returns true if a simple ocl loop is detected
    static bool hasLoopIdiom(const Module &M);

    /// @brief Recursively update metadata nodes with new functions
    static void updateMetadataTreeWithNewFuncs(
        Module *M, DenseMap<Function *, Function *> &FunctionMap,
        MDNode *MDTreeNode, std::set<MDNode *> &Visited);

    /// @brief Update references to old functions in metadata with new ones
    static void
    updateFunctionMetadata(Module *M,
                           DenseMap<Function *, Function *> &FunctionMap);
  };

  class OCLBuiltins {
  private:
    Module &TargetModule;
    SmallVector<Module *, 2> RTLs;

  public:
    OCLBuiltins(Module &TargetModule, const SmallVectorImpl<Module *> &RTLs)
        : TargetModule(TargetModule), RTLs(RTLs.begin(), RTLs.end()) {}

    Function *get(StringRef Name) {
      if (auto F = TargetModule.getFunction(Name))
        return F;

      for (auto *BIModule : RTLs) {
        if (auto *F = BIModule->getFunction(Name)) {
          return CompilationUtils::importFunctionDecl(&TargetModule, F);
        }
      }

      llvm_unreachable("Built-in not found.");
    }

    Module &getTargetModule() { return TargetModule; }
  };

  class PipeTypesHelper {
  private:
    Type *PipeRWTy = nullptr;
    Type *PipeROTy = nullptr;
    Type *PipeWOTy = nullptr;

  public:
    PipeTypesHelper(Type *PipeRWTy, Type *PipeROTy, Type *PipeWOTy)
        : PipeRWTy(PipeRWTy ? PointerType::get(PipeRWTy,
                                               Utils::OCLAddressSpace::Global)
                            : nullptr),
          PipeROTy(PipeROTy ? PointerType::get(PipeROTy,
                                               Utils::OCLAddressSpace::Global)
                            : nullptr),
          PipeWOTy(PipeWOTy ? PointerType::get(PipeWOTy,
                                               Utils::OCLAddressSpace::Global)
                            : nullptr) {}

    PipeTypesHelper(const Module &M)
        : PipeTypesHelper(M.getTypeByName("opencl.pipe_rw_t"),
                          M.getTypeByName("opencl.pipe_ro_t"),
                          M.getTypeByName("opencl.pipe_wo_t")) {}

    bool hasPipeTypes() const {
      return PipeRWTy || PipeROTy || PipeWOTy;
    }

    bool isLocalPipeType(Type *Ty) const {
      return (PipeROTy &&
              CompilationUtils::isSameStructPtrType(Ty, PipeROTy)) ||
             (PipeWOTy && CompilationUtils::isSameStructPtrType(Ty, PipeWOTy));
    }

    bool isGlobalPipeType(Type *Ty) const {
      return PipeRWTy && CompilationUtils::isSameStructPtrType(Ty, PipeRWTy);
    }

    bool isPipeType(Type *Ty) const {
      return isLocalPipeType(Ty) || isGlobalPipeType(Ty);
    }

    bool isPipeArrayType(Type *Ty) const {
      return isa<ArrayType>(Ty) &&
             isPipeType(
                 CompilationUtils::getArrayElementType(cast<ArrayType>(Ty)));
    }

    bool isPipe(const Value *V) const {
      Type *Ty = V->getType();
      if (isa<GlobalVariable>(V))
        Ty = cast<PointerType>(Ty)->getElementType();
      return isPipeType(Ty);
    }

    bool isPipeArray(const Value *V) const {
      Type *Ty = V->getType();
      if (isa<GlobalVariable>(V))
        Ty = cast<PointerType>(Ty)->getElementType();
      return isPipeArrayType(Ty);
    }
  };

  //
  // Base class for all functors, which supports immutability query.
  //

  class AbstractFunctor{
  protected:
    bool m_isChanged;
  public:
    AbstractFunctor(): m_isChanged(false){}

    virtual ~AbstractFunctor() {}

    bool isChanged()const{
      return m_isChanged;
    }
  };

  class FunctionFunctor: public AbstractFunctor {
  public:
    virtual void operator ()(llvm::Function&) = 0;
  };

  class BlockFunctor: public AbstractFunctor {
  public:
    virtual void operator ()(llvm::BasicBlock&) = 0;
  };

}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {

#endif // __COMPILATION_UTILS_H__
