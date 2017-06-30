/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#ifndef __COMPILATION_UTILS_H__
#define __COMPILATION_UTILS_H__

#include "cl_kernel_arg_type.h"
#include "exceptions.h"

#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/ADT/SetVector.h"
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
    /// Convenience constants for OpenCL spec revisions
    static const unsigned CL_VER_VALS[] = {CL_VER_1_0, CL_VER_1_1, CL_VER_1_2, CL_VER_2_0};
    // The possible values that can be passed to be -cl-std compile option
    static StringRef CL_VER_STRINGS[] = { "CL1.0", "CL1.1", "CL1.2", "CL2.0" };
    // The default revision defined by OpenCL spec
    static const unsigned CL_VER_DEFAULT = CL_VER_1_2;

    static unsigned CLStrToVal(const char* S) {
      const StringRef* B = OclVersion::CL_VER_STRINGS;
      const StringRef* E = B + sizeof(CL_VER_VALS)/sizeof(CL_VER_VALS[0]);
      const StringRef* I = std::find(B, E, S);
      if (I == E)
        assert(false && "Bad Value for -cl-std option");
      return CL_VER_VALS[I-B];
    }

    static unsigned CLVersionToVal(uint64_t major, uint64_t minor) {
      return major * 100 + minor * 10;
    }
  }

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
    static void getImplicitArgs(Function *pFunc, Argument **ppLocalMem,
                                Argument **ppWorkDim, Argument **ppWGId,
                                Argument **ppBaseGlbId, Argument **ppSpecialBuf,
                                Argument **ppRunTimeHandle);

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
    static void parseKernelArguments(  Module* pModule,
                                              Function* pFunc,
                                              std::vector<cl_kernel_argument>& /* OUT */ arguments,
                                              std::vector<unsigned int>&       /* OUT */ memoryArguments);

    /// @brief  maps between kernels (both scalar and vectorized) and their metdata
    /// @param pModule          The module
    /// @param pVectFunctions   The vectorized kernels, these kernel should be mapped
    ///                         to their scalar version metadata
    /// @param pVectFunctions   OUT param, maps between kernels (both scalar and
    ///                         vectorized) and their metdata
    static void getKernelsMetadata( Module* pModule,
                                    const SmallVectorImpl<Function*>& pVectFunctions,
                                    std::map<Function*, MDNode*>& /* OUT */ kernelMetadata);

    static Function *AddMoreArgsToFunc(Function *F, ArrayRef<Type *> NewTypes,
                                       ArrayRef<const char *> NewNames,
                                       ArrayRef<AttributeSet> NewAttrs,
                                       StringRef Prefix, bool IsKernel = false);
    // AddMoreArgsToCall - Replaces a CallInst with a new CallInst which has the
    // same arguments as orignal plus more args appeneded.
    // Returns the new CallInst
    // OldC - the original CallInst to be replaced
    // NewArgs - New arguments to append to existing arguments
    // NewF - a suitable prototype of new Function to be called
    static CallInst *AddMoreArgsToCall(CallInst *OldC,
                                       ArrayRef<Value *> NewArgs,
                                       Function *NewF);
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
    static bool isGetSubGroupLocalID(const std::string&);
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
    static bool isNDRange_1D(const std::string&);
    static bool isNDRange_2D(const std::string&);
    static bool isNDRange_3D(const std::string&);
    static bool isEnqueueKernelBasic(const std::string&);
    static bool isEnqueueKernelLocalMem(const std::string&);
    static bool isEnqueueKernelEvents(const std::string&);
    static bool isEnqueueKernelEventsLocalMem(const std::string&);
    static bool isGetKernelWorkGroupSize(const std::string&);
    static bool isGetKernelWorkGroupSizeLocal(const std::string&);
    static bool isGetKernelPreferredWorkGroupSizeMultiple(const std::string&);
    static bool isGetKernelPreferredWorkGroupSizeMultipleLocal(const std::string&);
    static bool isEnqueueMarker(const std::string&);
    static bool isGetDefaultQueue(const std::string&);
    static bool isRetainEvent(const std::string&);
    static bool isReleaseEvent(const std::string&);
    static bool isCreateUserEvent(const std::string&);
    static bool isSetUserEventStatus(const std::string&);
    static bool isCaptureEventProfilingInfo(const std::string&);
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
    //kernel arg qualifiers
    static const std::string WRITE_ONLY;
    static const std::string READ_ONLY;
    static const std::string NONE;
    //kernel type qualifiers
    static const std::string SAMPLER;
  public:
    /// '3' is a magic number for global variables
    /// that were in origin kernel local variable!
    static const unsigned int LOCL_VALUE_ADDRESS_SPACE;

    static const std::string NAME_GET_BASE_GID;
    static const std::string NAME_GET_SPECIAL_BUFFER;

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
    // @brief: returns the mangled name of the work_group_barrier function
    // @param wgBarrierType
    //                      WG_BARRIER_NO_SCOPE - for
    // void work_group_barrier (cl_mem_fence_flags flags)
    //                      WG_BARRIER_WITH_SCOPE - for
    // void work_group_barrier (cl_mem_fence_flags flags, memory_scope scope)
    //////////////////////////////////////////////////////////////////
    typedef enum {
      WG_BARRIER_NO_SCOPE,
      WG_BARRIER_WITH_SCOPE
    } WG_BARRIER_TYPE;
    static std::string mangledWGBarrier(WG_BARRIER_TYPE wgBarrierType);
    //////////////////////////////////////////////////////////////////
    // @brief: returns the name of the argument metadata node for the
    //given module
    //////////////////////////////////////////////////////////////////
    static std::string argumentAttribute(const llvm::Module&);

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

    /// getCLVersionFromModule - if the version exists in the module's metadata,
    /// stores the OpenCL version to 'Result' and returns true.
    /// Otherwise returns false
    static bool getCLVersionFromModule(const Module &M, unsigned &Result);

    /// fetchCompilerOption - if an option with specified prefix exists in
    /// the module's metadata returns an entire option string (the fist was met),
    /// Otherwise returns an empty string.
    static StringRef fetchCompilerOption(const Module &M, char const* prefix);

    /// fetchCLVersionFromMetadata - obtain CL version from "!opencl.ocl.version"
    /// named metadata
    static bool fetchCLVersionFromMetadata(const Module &M, unsigned &Result);

    /// getCLVersionFromModuleOrDefault - Return the version in the module's metadata,
    /// if it exists, otherwise the default version
    static unsigned getCLVersionFromModuleOrDefault(const Module &M) {
      unsigned version;
      return CompilationUtils::getCLVersionFromModule(M, version) ?
             version : OclVersion::CL_VER_DEFAULT;
    }

    /// type for extended execution context
    static Type * getExtendedExecContextType(LLVMContext &C);
    static void CloneDebugInfo(Function *SrcF, Function *DstF);

    /// Import a declaration of \p Orig into module \p Dst
    ///
    /// Parameter types will be translated to match the corresponding
    /// types in the \p Dst.
    ///
    /// @return function declaration Function* (if import succeed) or a
    /// bitcast if a function with the same name, but different type, is
    /// already exist in the \p Dst.
    static Constant *importFunctionDecl(Module *Dst, const Function *Orig);



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

    /// Return a pointer to struct from a pointer to type
    static StructType* getStructFromTypePtr(Type *T);

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
        Type *Ty, const SmallVectorImpl<size_t> &Dimensions);

    /// @brief Returns vector of numbers of elements in each dimension of the
    ///        ArrayType
    static void getArrayTypeDimensions(const ArrayType *ArrTy,
                                       SmallVectorImpl<size_t> &Dimensions);
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
