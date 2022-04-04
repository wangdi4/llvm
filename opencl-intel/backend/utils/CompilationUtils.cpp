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

#include "CompilationUtils.h"
#include "ImplicitArgsUtils.h"
#include "NameMangleAPI.h"
#include "ParameterType.h"
#include "TypeAlignment.h"
#include "cl_sys_defines.h"
#include "cl_types.h"

#include "CL/cl.h"

#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/ExecutionEngine/Orc/ExecutionUtils.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"

using namespace llvm;
using namespace llvm::NameMangleAPI;
using namespace DPCPPKernelMetadataAPI;

namespace Intel { namespace OpenCL { namespace DeviceBackend {

  //TODO-MERGE: update value of CompilationUtils::NUM_IMPLICIT_ARGS wherever it is now
  const unsigned int CompilationUtils::LOCL_VALUE_ADDRESS_SPACE = 3;

  const std::string CompilationUtils::ATTR_KERNEL_CALL_ONCE = "kernel-call-once";
  const std::string CompilationUtils::ATTR_KERNEL_UNIFORM_CALL = "kernel-uniform-call";
  const std::string CompilationUtils::ATTR_KERNEL_CONVERGENT_CALL = "kernel-convergent-call";

  const std::string CompilationUtils::ATTR_VECTOR_VARIANT_FAILURE = "vector-variant-failure";
  const std::string CompilationUtils::ATTR_VALUE_FAILED_TO_VECTORIZE =
      "failed to match a vector variant for an indirect call";
  const std::string CompilationUtils::ATTR_VALUE_FAILED_TO_LOWER_INDIRECT_CALL =
      "failed to find a masked vector variant for an indirect call";

  const std::string CompilationUtils::ATTR_HAS_VPLAN_MASK = "has-vplan-mask";
  const std::string CompilationUtils::ATTR_HAS_SUBGROUPS = "has-sub-groups";

  const std::string CompilationUtils::ATTR_RECURSION_WITH_BARRIER =
      "barrier_with_recursion";

  const std::string CompilationUtils::NAME_GET_GID = "get_global_id";
  const std::string CompilationUtils::NAME_GET_BASE_GID = "get_base_global_id.";
  const std::string CompilationUtils::NAME_GET_LID = "get_local_id";
  const std::string CompilationUtils::NAME_GET_SPECIAL_BUFFER = "get_special_buffer.";

  const std::string CompilationUtils::NAME_GET_LINEAR_GID = "get_global_linear_id";
  const std::string CompilationUtils::NAME_GET_LINEAR_LID = "get_local_linear_id";
  const std::string CompilationUtils::NAME_GET_SUB_GROUP_ID = "get_sub_group_id";
  const std::string CompilationUtils::NAME_GET_SUB_GROUP_LOCAL_ID = "get_sub_group_local_id";

  const std::string CompilationUtils::NAME_GET_WORK_DIM = "get_work_dim";
  const std::string CompilationUtils::NAME_GET_GLOBAL_SIZE = "get_global_size";
  const std::string CompilationUtils::NAME_GET_LOCAL_SIZE = "get_local_size";
  const std::string CompilationUtils::NAME_GET_SUB_GROUP_SIZE = "get_sub_group_size";
  const std::string CompilationUtils::NAME_GET_MAX_SUB_GROUP_SIZE = "get_max_sub_group_size";
  const std::string CompilationUtils::NAME_GET_ENQUEUED_LOCAL_SIZE = "get_enqueued_local_size";
  const std::string CompilationUtils::NAME_GET_NUM_GROUPS = "get_num_groups";
  const std::string CompilationUtils::NAME_GET_NUM_SUB_GROUPS = "get_num_sub_groups";
  const std::string CompilationUtils::NAME_GET_ENQUEUED_NUM_SUB_GROUPS = "get_enqueued_num_sub_groups";
  const std::string CompilationUtils::NAME_GET_GROUP_ID = "get_group_id";
  const std::string CompilationUtils::NAME_GET_GLOBAL_OFFSET = "get_global_offset";
  const std::string CompilationUtils::NAME_PRINTF = "printf";

  const std::string CompilationUtils::NAME_ASYNC_WORK_GROUP_COPY = "async_work_group_copy";
  const std::string CompilationUtils::NAME_WAIT_GROUP_EVENTS = "wait_group_events";
  const std::string CompilationUtils::NAME_PREFETCH = "prefetch";
  const std::string CompilationUtils::NAME_ASYNC_WORK_GROUP_STRIDED_COPY = "async_work_group_strided_copy";

  const std::string CompilationUtils::NAME_WORK_GROUP_RESERVE_READ_PIPE = "__work_group_reserve_read_pipe";
  const std::string CompilationUtils::NAME_WORK_GROUP_COMMIT_READ_PIPE = "__work_group_commit_read_pipe";
  const std::string CompilationUtils::NAME_WORK_GROUP_RESERVE_WRITE_PIPE = "__work_group_reserve_write_pipe";
  const std::string CompilationUtils::NAME_WORK_GROUP_COMMIT_WRITE_PIPE = "__work_group_commit_write_pipe";

  const std::string CompilationUtils::NAME_SUB_GROUP_RESERVE_READ_PIPE = "__sub_group_reserve_read_pipe";
  const std::string CompilationUtils::NAME_SUB_GROUP_COMMIT_READ_PIPE = "__sub_group_commit_read_pipe";
  const std::string CompilationUtils::NAME_SUB_GROUP_RESERVE_WRITE_PIPE = "__sub_group_reserve_write_pipe";
  const std::string CompilationUtils::NAME_SUB_GROUP_COMMIT_WRITE_PIPE = "__sub_group_commit_write_pipe";

  // KMP acquire/release
  const std::string CompilationUtils::NAME_IB_KMP_ACQUIRE_LOCK = "__builtin_IB_kmp_acquire_lock";
  const std::string CompilationUtils::NAME_IB_KMP_RELEASE_LOCK = "__builtin_IB_kmp_release_lock";

  // atomic fence functions
  const std::string CompilationUtils::NAME_ATOMIC_WORK_ITEM_FENCE = "atomic_work_item_fence";

  // mem_fence functions
  const std::string CompilationUtils::NAME_MEM_FENCE = "mem_fence";
  const std::string CompilationUtils::NAME_READ_MEM_FENCE = "read_mem_fence";
  const std::string CompilationUtils::NAME_WRITE_MEM_FENCE = "write_mem_fence";

  // Extended execution var args OpenCL 2.x
  const std::string CompilationUtils::NAME_ENQUEUE_KERNEL = "enqueue_kernel";

  const std::string CompilationUtils::NAME_GET_KERNEL_SG_COUNT_FOR_NDRANGE = "get_kernel_sub_group_count_for_ndrange";
  const std::string CompilationUtils::NAME_GET_KERNEL_MAX_SG_SIZE_FOR_NDRANGE = "get_kernel_max_sub_group_size_for_ndrange";

  const std::string CompilationUtils::BARRIER_FUNC_NAME = "barrier";
  const std::string CompilationUtils::WG_BARRIER_FUNC_NAME = "work_group_barrier";
  const std::string CompilationUtils::SG_BARRIER_FUNC_NAME = "sub_group_barrier";

  //work-group functions
  const std::string CompilationUtils::NAME_WORK_GROUP_ALL = "work_group_all";
  const std::string CompilationUtils::NAME_WORK_GROUP_ANY = "work_group_any";
  const std::string CompilationUtils::NAME_WORK_GROUP_BROADCAST = "work_group_broadcast";
  const std::string CompilationUtils::NAME_WORK_GROUP_REDUCE_ADD = "work_group_reduce_add";
  const std::string CompilationUtils::NAME_WORK_GROUP_SCAN_EXCLUSIVE_ADD = "work_group_scan_exclusive_add";
  const std::string CompilationUtils::NAME_WORK_GROUP_SCAN_INCLUSIVE_ADD = "work_group_scan_inclusive_add";
  const std::string CompilationUtils::NAME_WORK_GROUP_REDUCE_MIN = "work_group_reduce_min";
  const std::string CompilationUtils::NAME_WORK_GROUP_SCAN_EXCLUSIVE_MIN = "work_group_scan_exclusive_min";
  const std::string CompilationUtils::NAME_WORK_GROUP_SCAN_INCLUSIVE_MIN = "work_group_scan_inclusive_min";
  const std::string CompilationUtils::NAME_WORK_GROUP_REDUCE_MAX = "work_group_reduce_max";
  const std::string CompilationUtils::NAME_WORK_GROUP_SCAN_EXCLUSIVE_MAX = "work_group_scan_exclusive_max";
  const std::string CompilationUtils::NAME_WORK_GROUP_SCAN_INCLUSIVE_MAX = "work_group_scan_inclusive_max";
  const std::string CompilationUtils::NAME_WORK_GROUP_REDUCE_MUL = "work_group_reduce_mul";
  const std::string CompilationUtils::NAME_WORK_GROUP_SCAN_EXCLUSIVE_MUL = "work_group_scan_exclusive_mul";
  const std::string CompilationUtils::NAME_WORK_GROUP_SCAN_INCLUSIVE_MUL = "work_group_scan_inclusive_mul";
  const std::string CompilationUtils::NAME_WORK_GROUP_REDUCE_BITWISE_AND = "work_group_reduce_bitwise_and";
  const std::string CompilationUtils::NAME_WORK_GROUP_REDUCE_BITWISE_OR = "work_group_reduce_bitwise_or";
  const std::string CompilationUtils::NAME_WORK_GROUP_REDUCE_BITWISE_XOR = "work_group_reduce_bitwise_xor";
  const std::string CompilationUtils::NAME_WORK_GROUP_REDUCE_LOGICAL_AND = "work_group_reduce_logical_and";
  const std::string CompilationUtils::NAME_WORK_GROUP_REDUCE_LOGICAL_OR = "work_group_reduce_logical_or";
  const std::string CompilationUtils::NAME_WORK_GROUP_REDUCE_LOGICAL_XOR = "work_group_reduce_logical_xor";
  const std::string CompilationUtils::NAME_FINALIZE_WG_FUNCTION_PREFIX = "__finalize_";

  //sub-group functions
  const std::string CompilationUtils::NAME_SUB_GROUP_ALL = "sub_group_all";
  const std::string CompilationUtils::NAME_SUB_GROUP_ANY = "sub_group_any";
  const std::string CompilationUtils::NAME_SUB_GROUP_BROADCAST = "sub_group_broadcast";
  const std::string CompilationUtils::NAME_SUB_GROUP_REDUCE_ADD = "sub_group_reduce_add";
  const std::string CompilationUtils::NAME_SUB_GROUP_SCAN_EXCLUSIVE_ADD = "sub_group_scan_exclusive_add";
  const std::string CompilationUtils::NAME_SUB_GROUP_SCAN_INCLUSIVE_ADD = "sub_group_scan_inclusive_add";
  const std::string CompilationUtils::NAME_SUB_GROUP_REDUCE_MIN = "sub_group_reduce_min";
  const std::string CompilationUtils::NAME_SUB_GROUP_SCAN_EXCLUSIVE_MIN = "sub_group_scan_exclusive_min";
  const std::string CompilationUtils::NAME_SUB_GROUP_SCAN_INCLUSIVE_MIN = "sub_group_scan_inclusive_min";
  const std::string CompilationUtils::NAME_SUB_GROUP_REDUCE_MAX = "sub_group_reduce_max";
  const std::string CompilationUtils::NAME_SUB_GROUP_SCAN_EXCLUSIVE_MAX = "sub_group_scan_exclusive_max";
  const std::string CompilationUtils::NAME_SUB_GROUP_SCAN_INCLUSIVE_MAX = "sub_group_scan_inclusive_max";

  //Images
  const std::string CompilationUtils::OCL_IMG_PREFIX  = "opencl.image";
  const std::string CompilationUtils::IMG_2D        = OCL_IMG_PREFIX + "2d_t";
  const std::string CompilationUtils::IMG_2D_ARRAY  = OCL_IMG_PREFIX + "2d_array_t";
  const std::string CompilationUtils::IMG_3D        = OCL_IMG_PREFIX + "3d_t";

  const char* CompilationUtils::ImageTypeNames[] = {
      "opencl.image1d_ro_t",                  "opencl.image1d_array_ro_t",
      "opencl.image1d_wo_t",                  "opencl.image1d_array_wo_t",
      "opencl.image1d_rw_t",                  "opencl.image1d_array_rw_t",
      "opencl.image2d_ro_t",                  "opencl.image1d_buffer_ro_t",
      "opencl.image2d_wo_t",                  "opencl.image1d_buffer_wo_t",
      "opencl.image2d_rw_t",                  "opencl.image1d_buffer_rw_t",
      "opencl.image2d_array_ro_t",            "opencl.image2d_depth_ro_t",
      "opencl.image2d_array_wo_t",            "opencl.image2d_depth_wo_t",
      "opencl.image2d_array_rw_t",            "opencl.image2d_depth_rw_t",
      "opencl.image2d_array_depth_ro_t",      "opencl.image2d_msaa_ro_t",
      "opencl.image2d_array_depth_wo_t",      "opencl.image2d_msaa_wo_t",
      "opencl.image2d_array_depth_rw_t",      "opencl.image2d_msaa_rw_t",
      "opencl.image2d_array_msaa_ro_t",       "opencl.image2d_msaa_depth_ro_t",
      "opencl.image2d_array_msaa_wo_t",       "opencl.image2d_msaa_depth_wo_t",
      "opencl.image2d_array_msaa_rw_t",       "opencl.image2d_msaa_depth_rw_t",
      "opencl.image2d_array_msaa_depth_ro_t", "opencl.image3d_ro_t",
      "opencl.image2d_array_msaa_depth_wo_t", "opencl.image3d_wo_t",
      "opencl.image2d_array_msaa_depth_rw_t", "opencl.image3d_rw_t"};

  //Argument qualifiers
  const std::string CompilationUtils::WRITE_ONLY = "write_only";
  const std::string CompilationUtils::READ_ONLY  = "read_only";
  const std::string CompilationUtils::NONE       = "none";
  //Type qualifiers
  const std::string CompilationUtils::SAMPLER   = "sampler_t";

  unsigned OclVersion::CLStrToVal(const char* S) {
    // Constants for OpenCL spec revisions
    const unsigned VersionValues[] = {
      CL_VER_1_0, CL_VER_1_1, CL_VER_1_2, CL_VER_2_0, CL_VER_3_0
    };
    // The possible values that can be passed to be -cl-std compile option
    const StringRef VersionStrings[] = {
      "CL1.0", "CL1.1", "CL1.2", "CL2.0", "CL3.0"
    };

    auto B = std::begin(VersionStrings);
    auto E = std::end(VersionStrings);
    auto I = std::find(B, E, S);
    assert(I != E && "Bad Value for -cl-std option");

    return VersionValues[std::distance(B, I)];
  }

  unsigned OclVersion::CLVersionToVal(uint64_t major, uint64_t minor) {
    return major * 100 + minor * 10;
  }

  ChannelPipeMetadata::ChannelPipeMD
  ChannelPipeMetadata::getChannelPipeMetadata(GlobalVariable *Channel,
                                              int ChannelDepthEmulationMode) {
    auto GVMetadata = GlobalVariableMetadataAPI(Channel);

    assert(GVMetadata.PipePacketSize.hasValue() &&
           GVMetadata.PipePacketAlign.hasValue() &&
           "Channel metadata must contain packet_size and packet_align");

    ChannelPipeMD CMD;
    CMD.PacketSize = GVMetadata.PipePacketSize.get();
    CMD.PacketAlign = GVMetadata.PipePacketAlign.get();
    CMD.Depth =
        GVMetadata.PipeDepth.hasValue() ? GVMetadata.PipeDepth.get() : 0;
    CMD.IO = GVMetadata.PipeIO.hasValue() ? GVMetadata.PipeIO.get() : "";

    if (!GVMetadata.PipeDepth.hasValue() &&
        ChannelDepthEmulationMode == CHANNEL_DEPTH_MODE_DEFAULT) {
      GVMetadata.DepthIsIgnored.set(true);
    }

    return CMD;
  }

  BasicBlock::iterator CompilationUtils::removeInstruction(BasicBlock* pBB, BasicBlock::iterator it) {
    BasicBlock::InstListType::iterator prev;

    if ( pBB->begin() == it ) {
      prev = pBB->end();
    } else {
      prev = it;
      --prev;
    }

    Instruction* pInst = &*it;
    pInst->removeFromParent();
    pInst->dropAllReferences();

    if ( pBB->end() == prev ) {
      return pBB->begin();
    }

    return ++prev;
  }

  void CompilationUtils::getImplicitArgs(Function *pFunc, Value **ppLocalMem,
                                         Value **ppWorkDim, Value **ppWGId,
                                         Value **ppBaseGlbId,
                                         Value **ppSpecialBuf,
                                         Value **ppRunTimeHandle) {

      assert( pFunc && "Function cannot be null" );
      assert(pFunc->arg_size() >= ImplicitArgsUtils::NUM_IMPLICIT_ARGS &&
             "implicit args was not added!");

      // Iterating over explicit arguments
      Function::arg_iterator DestI = pFunc->arg_begin();

      // Go over the explicit arguments
      for (unsigned int i = 0;
           i < pFunc->arg_size() - ImplicitArgsUtils::NUM_IMPLICIT_ARGS; ++i) {
        ++DestI;
      }

      // Retrieve all the implicit arguments which are not NULL

      if ( nullptr != ppLocalMem ) {
          *ppLocalMem = &*DestI;
      }
      ++DestI;

      if ( nullptr != ppWorkDim ) {
          *ppWorkDim = &*DestI;
      }
      ++DestI;

      if ( nullptr != ppWGId ) {
          *ppWGId = &*DestI;
      }
      ++DestI;

      if ( nullptr != ppBaseGlbId ) {
          *ppBaseGlbId = &*DestI;
      }
      ++DestI;

      if ( nullptr != ppSpecialBuf ) {
          *ppSpecialBuf = &*DestI;
      }
      ++DestI;

      if ( nullptr != ppRunTimeHandle ) {
          *ppRunTimeHandle = &*DestI;
      }
      ++DestI;
      assert(DestI == pFunc->arg_end());

  }

  GlobalVariable *CompilationUtils::getTLSGlobal(Module *pModule,
                                                 unsigned Idx) {
    assert(pModule && "Module cannot be null");
    return pModule->getGlobalVariable(ImplicitArgsUtils::getArgName(Idx));
  }

  void CompilationUtils::moveAlloca(BasicBlock *FromBB, BasicBlock *ToBB) {
    auto InsertionPt = ToBB->getFirstInsertionPt();
    SmallVector<Instruction *, 4> ToMove;
    for (auto &I : *FromBB) {
      if (isa<AllocaInst>(&I))
        ToMove.push_back(&I);
    }
    for (auto *I : ToMove)
      I->moveBefore(*ToBB, InsertionPt);
  }

  void CompilationUtils::moveAllocaDbgDeclare(BasicBlock &FromBB,
                                              BasicBlock &ToBB) {
    auto InsertionPt = ToBB.getFirstInsertionPt();
    SmallVector<Instruction *, 4> ToMove;
    for (auto &I : FromBB) {
      if (isa<AllocaInst>(&I) || isa<DbgDeclareInst>(&I))
        ToMove.push_back(&I);
    }
    for (auto *I : ToMove)
      I->moveBefore(ToBB, InsertionPt);
  }

  void CompilationUtils::getAllSyncBuiltinsDclsForNoDuplicateRelax(
      FunctionSet &functionSet, Module *pModule) {
    getAllSyncBuiltinsDcls(functionSet, pModule);
    // add sub_group_barrier separately. It does not require
    // following Barrier compilation flow, but
    // requires noduplicate relaxation.
    for (auto &F : pModule->functions())
      if (F.isDeclaration()) {
      llvm::StringRef func_name = F.getName();
      if (func_name == CompilationUtils::mangledSGBarrier(CompilationUtils::BARRIER_NO_SCOPE) ||
          func_name == CompilationUtils::mangledSGBarrier(CompilationUtils::BARRIER_WITH_SCOPE) ||
          CompilationUtils::isKMPAcquireReleaseLock(std::string(func_name)))
        functionSet.insert(&F);
      }
  }

  void CompilationUtils::getAllSyncBuiltinsDclsForKernelUniformCallAttr(
    FunctionSet &functionSet, Module *pModule) {
    //Clear old collected data!
    functionSet.clear();

    for (auto &F : pModule->functions())
      if (F.isDeclaration()) {
        llvm::StringRef func_name = F.getName();
        if (
          func_name == CompilationUtils::mangledBarrier() ||
          func_name == CompilationUtils::mangledWGBarrier(CompilationUtils::BARRIER_NO_SCOPE) ||
          func_name == CompilationUtils::mangledWGBarrier(CompilationUtils::BARRIER_WITH_SCOPE) ||
          func_name == CompilationUtils::mangledSGBarrier(CompilationUtils::BARRIER_NO_SCOPE) ||
          func_name == CompilationUtils::mangledSGBarrier(CompilationUtils::BARRIER_WITH_SCOPE) ||
          CompilationUtils::isKMPAcquireReleaseLock(std::string(func_name)) ||
          CompilationUtils::isWorkGroupAsyncOrPipeBuiltin(std::string(func_name), pModule)) {
            functionSet.insert(&F);
        }
      }
  }

  void CompilationUtils::getAllSyncBuiltinsDcls(FunctionSet &functionSet,
                                                Module *pModule, bool IsWG) {
    //Clear old collected data!
    functionSet.clear();

    for ( Module::iterator fi = pModule->begin(), fe = pModule->end(); fi != fe; ++fi ) {
      if ( !fi->isDeclaration() ) continue;
      llvm::StringRef func_name = fi->getName();
      if (IsWG) {
        if (/* WG barrier built-ins */
            func_name == CompilationUtils::mangledBarrier() ||
            func_name == CompilationUtils::mangledWGBarrier(
                             CompilationUtils::BARRIER_NO_SCOPE) ||
            func_name == CompilationUtils::mangledWGBarrier(
                             CompilationUtils::BARRIER_WITH_SCOPE) ||
            /* work group built-ins */
            CompilationUtils::isWorkGroupBuiltin(std::string(func_name)) ||
            /* built-ins synced as if were called by a single work item */
            CompilationUtils::isWorkGroupAsyncOrPipeBuiltin(
                std::string(func_name), pModule)) {
          // Found synchronized built-in declared in the module add it to the
          // container set.
          functionSet.insert(&*fi);
        }
      } else {
        if (/* SG barrier built-ins */
            func_name == CompilationUtils::mangledSGBarrier(
                             CompilationUtils::BARRIER_NO_SCOPE) ||
            func_name == CompilationUtils::mangledSGBarrier(
                             CompilationUtils::BARRIER_WITH_SCOPE) ||
            CompilationUtils::isSubGroupBuiltin(std::string(func_name))) {
          // Found synchronized built-in declared in the module add it to the
          // container set.
          functionSet.insert(&*fi);
        }
      }
    }
  }

  void CompilationUtils::getAllKernels(FunctionSet &functionSet, Module *pModule) {
    // Clear old collected data!
    functionSet.clear();

    // List all kernels in module
    for (auto* pSclFunc : KernelList(pModule)) {
      functionSet.insert(pSclFunc);
      //Check if there is a vectorized version
      auto kimd = KernelInternalMetadataAPI(pSclFunc);
      // Need to check if Vectorized Kernel Value exists, it is not guaranteed that
      // Vectorized is running in all scenarios.
      if (kimd.VectorizedKernel.hasValue() && kimd.VectorizedKernel.get()) {
        functionSet.insert(kimd.VectorizedKernel.get());
      }
      if (kimd.VectorizedMaskedKernel.hasValue() && kimd.VectorizedMaskedKernel.get()) {
        functionSet.insert(kimd.VectorizedMaskedKernel.get());
      }
    }
  }

  std::vector<cl_kernel_argument_info>
  CompilationUtils::parseKernelArgumentInfos(Function *F) {
    assert(F && "Invalid function");

    std::vector<cl_kernel_argument_info> ArgInfos;

    MDNode *AddressQualifiers = F->getMetadata("kernel_arg_addr_space");
    MDNode *AccessQualifiers = F->getMetadata("kernel_arg_access_qual");
    MDNode *TypeNames = F->getMetadata("kernel_arg_type");
    MDNode *TypeQualifiers = F->getMetadata("kernel_arg_type_qual");
    MDNode *ArgNames = F->getMetadata("kernel_arg_name");
    MDNode *HostAccessible = F->getMetadata("kernel_arg_host_accessible");
    MDNode *LocalMemSize = F->getMetadata("local_mem_size");

    unsigned KernelArgCount = F->arg_size();
    for (unsigned int I = 0; I < KernelArgCount; ++I) {
      Argument *Arg = F->getArg(I);
      cl_kernel_argument_info ArgInfo;
      memset(&ArgInfo, 0, sizeof(ArgInfo));

      // Address qualifier
      unsigned AddrQ = 0;
      if (AddressQualifiers) {
        assert(AddressQualifiers->getNumOperands() == KernelArgCount &&
               "If kernel has 'kernel_arg_addr_space' metadata, its operand "
               "count must match with kernel arg count!");
        ConstantInt *AddressQualifier =
            mdconst::dyn_extract<ConstantInt>(AddressQualifiers->getOperand(I));
        assert(AddressQualifier &&
               "AddressQualifier is not a valid ConstantInt*");
        AddrQ = AddressQualifier->getZExtValue();
      } else {
        // kernel_arg_addr_space might not exist for a SYCL kernel.
        // Decode from the kernel argument itself.
        if (auto *PTy = dyn_cast<PointerType>(Arg->getType()))
          AddrQ = PTy->getAddressSpace();
      }
      switch (AddrQ) {
      case 0:
        ArgInfo.addressQualifier = CL_KERNEL_ARG_ADDRESS_PRIVATE;
        break;
      case 1:
        ArgInfo.addressQualifier = CL_KERNEL_ARG_ADDRESS_GLOBAL;
        break;
      case 2:
        ArgInfo.addressQualifier = CL_KERNEL_ARG_ADDRESS_CONSTANT;
        break;
      case 3:
        ArgInfo.addressQualifier = CL_KERNEL_ARG_ADDRESS_LOCAL;
        break;
      default:
        throw std::string("Invalid address qualifier: ") +
            std::to_string(AddrQ);
        break;
      }

      // Access qualifier
      // kernel_arg_access_qual might not exist for a SYCL kernel, leave it as
      // "none" by default.
      StringRef AccessQ = "none";
      if (AccessQualifiers) {
        assert(AccessQualifiers->getNumOperands() == KernelArgCount &&
               "If kernel has 'kernel_arg_access_qual' metadata, its operand "
               "count must match with kernel arg count!");
        AccessQ = cast<MDString>(AccessQualifiers->getOperand(I))->getString();
      }
      ArgInfo.accessQualifier =
          StringSwitch<cl_kernel_arg_access_qualifier>(AccessQ)
              .Case("read_only", CL_KERNEL_ARG_ACCESS_READ_ONLY)
              .Case("write_only", CL_KERNEL_ARG_ACCESS_WRITE_ONLY)
              .Case("read_write", CL_KERNEL_ARG_ACCESS_READ_WRITE)
              .Default(CL_KERNEL_ARG_ACCESS_NONE);

      // Type qualifier
      // kernel_arg_type_qual might not exist for a SYCL kernel, leave it as ""
      // by default.
      StringRef TypeQ = "";
      if (TypeQualifiers) {
        assert(TypeQualifiers->getNumOperands() == KernelArgCount &&
               "If kernel has 'kernel_arg_type_qual' metadata, its operand "
               "count must match with kernel arg count!");
        TypeQ = cast<MDString>(TypeQualifiers->getOperand(I))->getString();
      }
      ArgInfo.typeQualifier = 0;
      if (TypeQ.contains("const"))
        ArgInfo.typeQualifier |= CL_KERNEL_ARG_TYPE_CONST;
      if (TypeQ.contains("restrict"))
        ArgInfo.typeQualifier |= CL_KERNEL_ARG_TYPE_RESTRICT;
      if (TypeQ.contains("volatile"))
        ArgInfo.typeQualifier |= CL_KERNEL_ARG_TYPE_VOLATILE;
      if (TypeQ.contains("pipe"))
        ArgInfo.typeQualifier |= CL_KERNEL_ARG_TYPE_PIPE;

      // Type name
      std::string TypeName = "";
      if (TypeNames) {
        assert(TypeNames->getNumOperands() == KernelArgCount &&
               "If kernel has 'kernel_arg_type' metadata, its operand count "
               "must match with kernel arg count!");
        TypeName = cast<MDString>(TypeNames->getOperand(I))->getString().str();
      } else {
        // kernel_arg_type might not exist for a SYCL kernel.
        // Decode from the kernel argument itself.
        raw_string_ostream OS(TypeName);
        Arg->getType()->print(OS, /*IsForDebug*/ false, /*NoDetails*/ true);
        OS.flush();
      }
      ArgInfo.typeName = STRDUP(TypeName.c_str());

      if (ArgNames) {
        // Parameter name
        MDString *ArgName = cast<MDString>(ArgNames->getOperand(I));
        ArgInfo.name = STRDUP(ArgName->getString().str().c_str());
      }

      if (HostAccessible) {
        auto *HostAccessibleFlag =
            cast<ConstantAsMetadata>(HostAccessible->getOperand(I));

        ArgInfo.hostAccessible =
            HostAccessibleFlag &&
            cast<ConstantInt>(HostAccessibleFlag->getValue())->isOne();
      }

      if (LocalMemSize) {
        auto *LocalMemSizeFlag =
            cast<ConstantAsMetadata>(LocalMemSize->getOperand(I));

        ArgInfo.localMemSize =
            cast<ConstantInt>(LocalMemSizeFlag->getValue())->getZExtValue();
      }

      ArgInfos.push_back(ArgInfo);
    }
    return ArgInfos;
  }

  unsigned CompilationUtils::fetchCLVersionFromMetadata(const Module &M) {
    /*
    Example of the metadata
    !opencl.ocl.version = !{!6}
    !6 = !{i32 2, i32 0}
    */

    // TODO Remove the block once OpenCL CPU BE compiler is able to handle
    // LLVM IR converted from SPIR-V correctly.
    if(DPCPPKernelCompilationUtils::isGeneratedFromOCLCPP(M))
       return OclVersion::CL_VER_2_0;

    auto oclVersion =
        ModuleMetadataAPI(const_cast<llvm::Module *>(&M)).OpenCLVersionList;

    if (oclVersion.hasValue())
      return OclVersion::CLVersionToVal(oclVersion.getItem(0),
                                        oclVersion.getItem(1));

    // Always return an OpenCL version to avoid any issues
    // in manually written LIT tests.
    return OclVersion::CL_VER_DEFAULT;
  }

  bool CompilationUtils::getDebugFlagFromMetadata(Module *M) {
    if (llvm::NamedMDNode *CompileOptsNamed =
            M->getNamedMetadata("opencl.compiler.options")) {

      llvm::MDTupleTypedArrayWrapper<llvm::MDString> CompileOpts(
          cast<llvm::MDTuple>(CompileOptsNamed->getOperand(0)));

      for (llvm::MDString *Opt : CompileOpts) {
        if (Opt->getString() == "-g") {
          return true;
        }
      }
    }

    return false;
  }

  bool CompilationUtils::getOptDisableFlagFromMetadata(Module *M) {
    if (llvm::NamedMDNode *CompileOptsNamed =
            M->getNamedMetadata("opencl.compiler.options")) {

      llvm::MDTupleTypedArrayWrapper<llvm::MDString> CompileOpts(
          cast<llvm::MDTuple>(CompileOptsNamed->getOperand(0)));

      for (llvm::MDString *Opt : CompileOpts) {
        if (Opt->getString() == "-cl-opt-disable") {
          return true;
        }
      }
    }

    return false;
  }

  bool CompilationUtils::hasFDivWithFastFlag(Module *M) {
    for (Function &F : *M)
      for (BasicBlock &B : F)
        for (Instruction &I : B)
          if (I.getOpcode() == Instruction::FDiv && I.isFast())
            return true;

    return false;
  }

  bool CompilationUtils::generatedFromSPIRV(const Module &M) {
    /*
    Example of the metadata
    !spirv.Source = !{!0}
    */
    auto spirvSource =
        ModuleMetadataAPI(const_cast<llvm::Module *>(&M)).SPIRVSourceList;
    return spirvSource.hasValue();
  }

Function *CompilationUtils::AddMoreArgsToFunc(
    Function *F, ArrayRef<Type *> NewTypes, ArrayRef<const char *> NewNames,
    ArrayRef<AttributeSet> NewAttrs, StringRef Prefix) {
  assert(NewTypes.size() == NewNames.size());
  SmallVector<AttributeSet, 16> NAttrs;
  NAttrs.assign(NewAttrs.begin(), NewAttrs.end());
  NAttrs.append(NewTypes.size() - NewAttrs.size(), {});
  // Initialize with all original arguments in the function sugnature
  SmallVector<llvm::Type *, 16> Types;

  for (Function::const_arg_iterator I = F->arg_begin(), E = F->arg_end();
       I != E; ++I) {
    Types.push_back(I->getType());
  }
  Types.append(NewTypes.begin(), NewTypes.end());
  FunctionType *NewFTy = FunctionType::get(F->getReturnType(), Types, false);
  // Change original function name.
  std::string Name = F->getName().str();
  F->setName("__" + F->getName() + "_before." + Prefix);
  // Create a new function with explicit and implict arguments types
  Function *NewF = Function::Create(NewFTy, F->getLinkage(), Name, F->getParent());
  // Copy old function attributes (including attributes on original arguments) to new function.
  NewF->copyAttributesFrom(F);
  NewF->copyMetadata(F, 0);
  // Set original arguments' names
  Function::arg_iterator NewI = NewF->arg_begin();
  for (Function::const_arg_iterator I = F->arg_begin(), E = F->arg_end();
       I != E; ++I, ++NewI) {
    NewI->setName(I->getName());
  }
  // Set new arguments` names
  for (unsigned I = 0, E = NewNames.size(); I < E; ++I, ++NewI) {
    Argument *A = &*NewI;
    A->setName(NewNames[I]);
    for (auto Attr : NAttrs[I])
      A->addAttr(Attr);
  }
  // Since we have now created the new function, splice the body of the old
  // function right into the new function, leaving the old body of the function empty.
  NewF->getBasicBlockList().splice(NewF->begin(), F->getBasicBlockList());
  assert(F->isDeclaration() && "splice did not work, original function body is not empty!");

  // Set DISubprogram as an original function has. Do it before delete body
  // since DISubprogram will be deleted too
  NewF->setSubprogram(F->getSubprogram());

  // Delete original function body - this is needed to remove linkage (if exists)
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
  llvm::Module *M = F->getParent();
  assert(M && "Module is NULL");
  auto kernels = KernelList(M).getList();
  std::replace_if(std::begin(kernels), std::end(kernels),
          [F](llvm::Function *Func) { return F == Func; }, NewF);
  KernelList(M).set(kernels);

  return NewF;
}

CallInst *CompilationUtils::AddMoreArgsToCall(CallInst *OldC,
                                              ArrayRef<Value *> NewArgs,
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

CallInst *
CompilationUtils::AddMoreArgsToIndirectCall(CallInst *OldC,
                                            ArrayRef<Value *> NewArgs) {
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

template <reflection::TypePrimitiveEnum Ty>
static std::string optionalMangleWithParam(const char*const N){
  reflection::FunctionDescriptor FD;
  FD.Name = N;
  reflection::ParamType *pTy =
    new reflection::PrimitiveType(Ty);
  reflection::RefParamType UI(pTy);
  FD.Parameters.push_back(UI);
  return mangle(FD);
}

template <reflection::TypePrimitiveEnum Ty>
static std::string mangleWithParam(const char*const N, unsigned int numOfParams){
  reflection::FunctionDescriptor FD;
  FD.Name = N;
  for(unsigned int i=0; i<numOfParams ; ++i) {
    reflection::ParamType *pTy =
      new reflection::PrimitiveType(Ty);
    reflection::RefParamType UI(pTy);
    FD.Parameters.push_back(UI);
  }
  return mangle(FD);
}

static std::string mangleWithParam(const char*const N,
                                   ArrayRef<reflection::TypePrimitiveEnum> Types){
  reflection::FunctionDescriptor FD;
  FD.Name = N;
  for (const auto &Ty:Types) {
    reflection::ParamType *pTy =
      new reflection::PrimitiveType(Ty);
    reflection::RefParamType UI(pTy);
    FD.Parameters.push_back(UI);
  }
  return mangle(FD);
}

std::string CompilationUtils::mangledMemFence() {
  return optionalMangleWithParam<reflection::PRIMITIVE_UINT>(NAME_MEM_FENCE.c_str());
}

std::string CompilationUtils::mangledReadMemFence() {
  return optionalMangleWithParam<reflection::PRIMITIVE_UINT>(NAME_READ_MEM_FENCE.c_str());
}

std::string CompilationUtils::mangledWriteMemFence() {
  return optionalMangleWithParam<reflection::PRIMITIVE_UINT>(NAME_WRITE_MEM_FENCE.c_str());
}

std::string CompilationUtils::mangledAtomicWorkItemFence() {
  reflection::TypePrimitiveEnum Params[] = {
    reflection::PRIMITIVE_UINT,
    reflection::PRIMITIVE_MEMORY_ORDER,
    reflection::PRIMITIVE_MEMORY_SCOPE };

   return mangleWithParam(NAME_ATOMIC_WORK_ITEM_FENCE.c_str(), Params);
}

std::string CompilationUtils::mangledGetGID() {
  return optionalMangleWithParam<reflection::PRIMITIVE_UINT>(NAME_GET_GID.c_str());
}

std::string CompilationUtils::mangledGetGlobalSize() {
  return optionalMangleWithParam<reflection::PRIMITIVE_UINT>(NAME_GET_GLOBAL_SIZE.c_str());
}

std::string CompilationUtils::mangledGetGlobalOffset() {
  return optionalMangleWithParam<reflection::PRIMITIVE_UINT>(NAME_GET_GLOBAL_OFFSET.c_str());
}

std::string CompilationUtils::mangledGetLID() {
  return optionalMangleWithParam<reflection::PRIMITIVE_UINT>(NAME_GET_LID.c_str());
}

std::string CompilationUtils::mangledGetGroupID() {
  return optionalMangleWithParam<reflection::PRIMITIVE_UINT>(NAME_GET_GROUP_ID.c_str());
}

std::string CompilationUtils::mangledGetLocalSize() {
  return optionalMangleWithParam<reflection::PRIMITIVE_UINT>(NAME_GET_LOCAL_SIZE.c_str());
}

std::string CompilationUtils::mangledGetNumGroups() {
  return optionalMangleWithParam<reflection::PRIMITIVE_UINT>(NAME_GET_NUM_GROUPS.c_str());
}

std::string CompilationUtils::mangledGetEnqueuedLocalSize(){
  return mangleWithParam<reflection::PRIMITIVE_UINT>(NAME_GET_ENQUEUED_LOCAL_SIZE.c_str(), 1);
}

std::string CompilationUtils::mangledBarrier() {
  return optionalMangleWithParam<reflection::PRIMITIVE_UINT>(BARRIER_FUNC_NAME.c_str());
}

std::string CompilationUtils::mangledWGBarrier(BARRIER_TYPE wgBarrierType) {
  switch(wgBarrierType) {
  case BARRIER_NO_SCOPE:
    return mangleWithParam<reflection::PRIMITIVE_UINT>(
      WG_BARRIER_FUNC_NAME.c_str(), 1);
  case BARRIER_WITH_SCOPE: {
    reflection::TypePrimitiveEnum Params[] = {
      reflection::PRIMITIVE_UINT,
      reflection::PRIMITIVE_MEMORY_SCOPE };

    return mangleWithParam(WG_BARRIER_FUNC_NAME.c_str(), Params);
  }
  }

  llvm_unreachable("Unknown work_group_barrier version");
  return "";
}

std::string CompilationUtils::mangledSGBarrier() {
  return optionalMangleWithParam<reflection::PRIMITIVE_UINT>(
    SG_BARRIER_FUNC_NAME.c_str());
}

std::string CompilationUtils::mangledSGBarrier(BARRIER_TYPE sgBarrierType) {
  switch(sgBarrierType) {
  case BARRIER_NO_SCOPE:
    return mangleWithParam<reflection::PRIMITIVE_UINT>(
      SG_BARRIER_FUNC_NAME.c_str(), 1);
  case BARRIER_WITH_SCOPE : {
    reflection::TypePrimitiveEnum Params[] = {
      reflection::PRIMITIVE_UINT,
      reflection::PRIMITIVE_MEMORY_SCOPE };

    return mangleWithParam(SG_BARRIER_FUNC_NAME.c_str(), Params);
  }
  }

  llvm_unreachable("Unknown sub_group_barrier version");
  return "";
}
std::string CompilationUtils::mangledGetMaxSubGroupSize() {
  return optionalMangleWithParam<reflection::PRIMITIVE_VOID>(
    NAME_GET_MAX_SUB_GROUP_SIZE.c_str());
}

std::string CompilationUtils::mangledNumSubGroups() {
  return optionalMangleWithParam<reflection::PRIMITIVE_VOID>(
    NAME_GET_NUM_SUB_GROUPS.c_str());
}

std::string CompilationUtils::mangledGetSubGroupId() {
  return optionalMangleWithParam<reflection::PRIMITIVE_VOID>(
    NAME_GET_SUB_GROUP_ID.c_str());
}

std::string CompilationUtils::mangledEnqueuedNumSubGroups() {
  return optionalMangleWithParam<reflection::PRIMITIVE_VOID>(
    NAME_GET_ENQUEUED_NUM_SUB_GROUPS.c_str());
}

std::string CompilationUtils::mangledGetSubGroupSize() {
  return optionalMangleWithParam<reflection::PRIMITIVE_VOID>(
    NAME_GET_SUB_GROUP_SIZE.c_str());
}

std::string CompilationUtils::mangledGetSubGroupLocalId() {
  return optionalMangleWithParam<reflection::PRIMITIVE_VOID>(
    NAME_GET_SUB_GROUP_LOCAL_ID.c_str());
}

std::string CompilationUtils::mangledGetGlobalLinearId() {
  return optionalMangleWithParam<reflection::PRIMITIVE_VOID>(
      NAME_GET_LINEAR_GID.c_str());
}

std::string CompilationUtils::mangledGetLocalLinearId() {
  return optionalMangleWithParam<reflection::PRIMITIVE_VOID>(
      NAME_GET_LINEAR_LID.c_str());
}

static bool isOptionalMangleOf(const std::string &LHS, const std::string &RHS) {
  //LHS should be mangled
  const char*const LC = LHS.c_str();
  if (!isMangledName(LC))
    return false;
  return stripName(LC) == RHS;
}

static bool isMangleOf(const std::string& LHS, const std::string& RHS) {
  const char*const LC = LHS.c_str();
  if (!isMangledName(LC))
    return false;
  return stripName(LC) == RHS;
}

bool CompilationUtils::isGetWorkDim(const std::string& S){
  return isOptionalMangleOf(S, NAME_GET_WORK_DIM);
}

bool CompilationUtils::isGetGlobalId(const std::string& S){
  return isOptionalMangleOf(S, NAME_GET_GID);
}

bool CompilationUtils::isGetLocalId(const std::string& S){
  return isOptionalMangleOf(S, NAME_GET_LID);
}

bool CompilationUtils::isGetSubGroupId(const std::string& S){
  return isOptionalMangleOf(S, NAME_GET_SUB_GROUP_ID);
}

bool CompilationUtils::isGetGlobalLinearId(const std::string& S){
  return isOptionalMangleOf(S, NAME_GET_LINEAR_GID);
}

bool CompilationUtils::isGetLocalLinearId(const std::string& S){
  return isOptionalMangleOf(S, NAME_GET_LINEAR_LID);
}

bool CompilationUtils::isGetSubGroupLocalId(const std::string& S){
  return isOptionalMangleOf(S, NAME_GET_SUB_GROUP_LOCAL_ID);
}

bool CompilationUtils::isGetGlobalSize(const std::string& S){
  return isOptionalMangleOf(S, NAME_GET_GLOBAL_SIZE);
}

bool CompilationUtils::isGetLocalSize(const std::string& S){
  return isOptionalMangleOf(S, NAME_GET_LOCAL_SIZE);
}

bool CompilationUtils::isGetSubGroupSize(const std::string& S){
  return isOptionalMangleOf(S, NAME_GET_SUB_GROUP_SIZE);
}

bool CompilationUtils::isGetMaxSubGroupSize(const std::string& S){
  return isOptionalMangleOf(S, NAME_GET_MAX_SUB_GROUP_SIZE);
}

bool CompilationUtils::isGetEnqueuedLocalSize(const std::string& S){
  return isMangleOf(S, NAME_GET_ENQUEUED_LOCAL_SIZE);
}

bool CompilationUtils::isGetNumGroups(const std::string& S){
  return isOptionalMangleOf(S, NAME_GET_NUM_GROUPS);
}

bool CompilationUtils::isGetNumSubGroups(const std::string& S){
  return isOptionalMangleOf(S, NAME_GET_NUM_SUB_GROUPS);
}

bool CompilationUtils::isGetEnqueuedNumSubGroups(const std::string& S){
  return isOptionalMangleOf(S, NAME_GET_ENQUEUED_NUM_SUB_GROUPS);
}

bool CompilationUtils::isGetGroupId(const std::string& S){
  return isOptionalMangleOf(S, NAME_GET_GROUP_ID);
}

bool CompilationUtils::isGlobalOffset(const std::string& S){
  return isOptionalMangleOf(S, NAME_GET_GLOBAL_OFFSET);
}

bool CompilationUtils::isAsyncWorkGroupCopy(const std::string& S){
  return isMangleOf(S, NAME_ASYNC_WORK_GROUP_COPY);
}

bool CompilationUtils::isKMPAcquireReleaseLock(const std::string& S){
  return (S == NAME_IB_KMP_ACQUIRE_LOCK) || (S == NAME_IB_KMP_RELEASE_LOCK);
}

bool CompilationUtils::isAsyncWorkGroupStridedCopy(const std::string& S){
  return isMangleOf(S, NAME_ASYNC_WORK_GROUP_STRIDED_COPY);
}

bool CompilationUtils::isWorkGroupReserveReadPipe(const std::string &S) {
  return S == NAME_WORK_GROUP_RESERVE_READ_PIPE;
}

bool CompilationUtils::isWorkGroupCommitReadPipe(const std::string &S) {
  return S == NAME_WORK_GROUP_COMMIT_READ_PIPE;
}

bool CompilationUtils::isWorkGroupReserveWritePipe(const std::string &S) {
  return S == NAME_WORK_GROUP_RESERVE_WRITE_PIPE;
}

bool CompilationUtils::isWorkGroupCommitWritePipe(const std::string &S) {
  return S == NAME_WORK_GROUP_COMMIT_WRITE_PIPE;
}

bool CompilationUtils::isSubGroupReserveReadPipe(const std::string &S) {
  return S == NAME_SUB_GROUP_RESERVE_READ_PIPE;
}

bool CompilationUtils::isSubGroupCommitReadPipe(const std::string &S) {
  return S == NAME_SUB_GROUP_COMMIT_READ_PIPE;
}

bool CompilationUtils::isSubGroupReserveWritePipe(const std::string &S) {
  return S == NAME_SUB_GROUP_RESERVE_WRITE_PIPE;
}

bool CompilationUtils::isSubGroupCommitWritePipe(const std::string &S) {
  return S == NAME_SUB_GROUP_COMMIT_WRITE_PIPE;
}

bool CompilationUtils::isMemFence(const std::string& S){
  return isOptionalMangleOf(S, NAME_MEM_FENCE);
}

bool CompilationUtils::isReadMemFence(const std::string& S){
  return isOptionalMangleOf(S, NAME_READ_MEM_FENCE);
}

bool CompilationUtils::isWriteMemFence(const std::string& S){
  return isOptionalMangleOf(S, NAME_WRITE_MEM_FENCE);
}

bool CompilationUtils::isWaitGroupEvents(const std::string& S){
  return isOptionalMangleOf(S, NAME_WAIT_GROUP_EVENTS);
}

bool CompilationUtils::isPrefetch(const std::string& S){
  return isMangleOf(S, NAME_PREFETCH);
}

bool CompilationUtils::isEnqueueKernel(const std::string& S) {
  return S == "__enqueue_kernel_basic" ||
         S == "__enqueue_kernel_basic_events" ||
         S == "__enqueue_kernel_varargs" ||
         S == "__enqueue_kernel_events_varargs";
}

bool CompilationUtils::isEnqueueKernelLocalMem(const std::string& S){
  return S == "__enqueue_kernel_varargs";
}

bool CompilationUtils::isEnqueueKernelEventsLocalMem(const std::string& S){
  return S == "__enqueue_kernel_events_varargs";
}

bool CompilationUtils::isWorkGroupAll(const std::string& S) {
  return isMangleOf(S, NAME_WORK_GROUP_ALL);
}

bool CompilationUtils::isWorkGroupAny(const std::string& S) {
  return isMangleOf(S, NAME_WORK_GROUP_ANY);
}

bool CompilationUtils::isSubGroupAll(const std::string& S) {
   return isMangleOf(S, NAME_SUB_GROUP_ALL);
}

bool CompilationUtils::isSubGroupAny(const std::string& S) {
  return isMangleOf(S, NAME_SUB_GROUP_ANY);
}

bool CompilationUtils::isSubGroupBarrier(const std::string& S) {
  return isMangleOf(S, SG_BARRIER_FUNC_NAME);
}

bool CompilationUtils::isWorkGroupBroadCast(const std::string& S) {
  return isMangleOf(S, NAME_WORK_GROUP_BROADCAST);
}

bool CompilationUtils::isSubGroupBroadCast(const std::string& S) {
  return isMangleOf(S, NAME_SUB_GROUP_BROADCAST);
}

bool CompilationUtils::isWorkGroupReduceAdd(const std::string& S) {
  return isMangleOf(S, NAME_WORK_GROUP_REDUCE_ADD);
}

bool CompilationUtils::isWorkGroupScanExclusiveAdd(const std::string& S) {
  return isMangleOf(S, NAME_WORK_GROUP_SCAN_EXCLUSIVE_ADD);
}

bool CompilationUtils::isWorkGroupScanInclusiveAdd(const std::string& S) {
  return isMangleOf(S, NAME_WORK_GROUP_SCAN_INCLUSIVE_ADD);
}

bool CompilationUtils::isWorkGroupReduceMul(const std::string& S) {
  return isMangleOf(S, NAME_WORK_GROUP_REDUCE_MUL);
}

bool CompilationUtils::isWorkGroupScanExclusiveMul(const std::string& S) {
  return isMangleOf(S, NAME_WORK_GROUP_SCAN_EXCLUSIVE_MUL);
}

bool CompilationUtils::isWorkGroupScanInclusiveMul(const std::string& S) {
  return isMangleOf(S, NAME_WORK_GROUP_SCAN_INCLUSIVE_MUL);
}

bool CompilationUtils::isWorkGroupReduceMin(const std::string& S) {
  return isMangleOf(S, NAME_WORK_GROUP_REDUCE_MIN);
}

bool CompilationUtils::isWorkGroupScanExclusiveMin(const std::string& S) {
  return isMangleOf(S, NAME_WORK_GROUP_SCAN_EXCLUSIVE_MIN);
}

bool CompilationUtils::isWorkGroupScanInclusiveMin(const std::string& S) {
  return isMangleOf(S, NAME_WORK_GROUP_SCAN_INCLUSIVE_MIN);
}

bool CompilationUtils::isWorkGroupReduceMax(const std::string& S) {
  return isMangleOf(S, CompilationUtils::NAME_WORK_GROUP_REDUCE_MAX);
}

bool CompilationUtils::isWorkGroupScanExclusiveMax(const std::string& S) {
  return isMangleOf(S, NAME_WORK_GROUP_SCAN_EXCLUSIVE_MAX);
}

bool CompilationUtils::isWorkGroupScanInclusiveMax(const std::string& S) {
  return isMangleOf(S, NAME_WORK_GROUP_SCAN_INCLUSIVE_MAX);
}

bool CompilationUtils::isWorkGroupReduceBitwiseAnd(const std::string& S) {
  return isMangleOf(S, NAME_WORK_GROUP_REDUCE_BITWISE_AND);
}

bool CompilationUtils::isWorkGroupReduceBitwiseOr(const std::string& S) {
  return isMangleOf(S, NAME_WORK_GROUP_REDUCE_BITWISE_OR);
}

bool CompilationUtils::isWorkGroupReduceBitwiseXor(const std::string& S) {
  return isMangleOf(S, NAME_WORK_GROUP_REDUCE_BITWISE_XOR);
}

bool CompilationUtils::isWorkGroupReduceLogicalAnd(const std::string& S) {
  return isMangleOf(S, NAME_WORK_GROUP_REDUCE_LOGICAL_AND);
}

bool CompilationUtils::isWorkGroupReduceLogicalOr(const std::string& S) {
  return isMangleOf(S, NAME_WORK_GROUP_REDUCE_LOGICAL_OR);
}

bool CompilationUtils::isWorkGroupReduceLogicalXor(const std::string& S) {
  return isMangleOf(S, NAME_WORK_GROUP_REDUCE_LOGICAL_XOR);
}

bool CompilationUtils::isSubGroupReduceAdd(const std::string& S) {
  return isMangleOf(S, NAME_SUB_GROUP_REDUCE_ADD);
}

bool CompilationUtils::isSubGroupScanExclusiveAdd(const std::string& S) {
  return isMangleOf(S, NAME_SUB_GROUP_SCAN_EXCLUSIVE_ADD);
}

bool CompilationUtils::isSubGroupScanInclusiveAdd(const std::string& S) {
  return isMangleOf(S, NAME_SUB_GROUP_SCAN_INCLUSIVE_ADD);
}

bool CompilationUtils::isSubGroupReduceMin(const std::string& S) {
  return isMangleOf(S, NAME_SUB_GROUP_REDUCE_MIN);
}

bool CompilationUtils::isSubGroupScanExclusiveMin(const std::string& S) {
  return isMangleOf(S, NAME_SUB_GROUP_SCAN_EXCLUSIVE_MIN);
}

bool CompilationUtils::isSubGroupScanInclusiveMin(const std::string& S) {
  return isMangleOf(S, NAME_SUB_GROUP_SCAN_INCLUSIVE_MIN);
}

bool CompilationUtils::isSubGroupReduceMax(const std::string& S) {
  return isMangleOf(S, NAME_SUB_GROUP_REDUCE_MAX);
}

bool CompilationUtils::isSubGroupScanExclusiveMax(const std::string& S) {
  return isMangleOf(S, NAME_SUB_GROUP_SCAN_EXCLUSIVE_MAX);
}

bool CompilationUtils::isSubGroupScanInclusiveMax(const std::string& S) {
  return isMangleOf(S, NAME_SUB_GROUP_SCAN_INCLUSIVE_MAX);
}

bool CompilationUtils::hasWorkGroupFinalizePrefix(const std::string& S) {
  if (!isMangledName(S.c_str())) return false;
  std::string funcName = std::string(stripName(S.c_str()));
  return StringRef(funcName).startswith(NAME_FINALIZE_WG_FUNCTION_PREFIX);
}

std::string CompilationUtils::appendWorkGroupFinalizePrefix(const std::string& S) {
  assert(isMangledName(S.c_str()) && "expected mangled name of work group built-in");
  reflection::FunctionDescriptor fd = demangle(S.c_str());
  fd.Name = NAME_FINALIZE_WG_FUNCTION_PREFIX + fd.Name;
  std::string finalizeFuncName = mangle(fd);
  return finalizeFuncName;
}

std::string CompilationUtils::removeWorkGroupFinalizePrefix(const std::string& S) {
  assert(hasWorkGroupFinalizePrefix(S) && "expected finilize prefix");
  reflection::FunctionDescriptor fd = demangle(S.c_str());
  fd.Name = fd.Name.substr(NAME_FINALIZE_WG_FUNCTION_PREFIX.size());
  std::string funcName = mangle(fd);
  return funcName;
}

bool CompilationUtils::isWorkGroupBuiltin(const std::string& S) {
  return isWorkGroupUniform(S) ||
         isWorkGroupDivergent(S);
}

bool CompilationUtils::isWorkGroupAsyncOrPipeBuiltin(const std::string& S, const Module* pModule) {
  return CompilationUtils::isAsyncWorkGroupCopy(S) ||
         CompilationUtils::isAsyncWorkGroupStridedCopy(S) ||
         (OclVersion::CL_VER_2_0 <= fetchCLVersionFromMetadata(*pModule) &&
          (CompilationUtils::isWorkGroupReserveReadPipe(S) ||
           CompilationUtils::isWorkGroupCommitReadPipe(S) ||
           CompilationUtils::isWorkGroupReserveWritePipe(S) ||
           CompilationUtils::isWorkGroupCommitWritePipe(S)));
}

bool CompilationUtils::isWorkGroupScan(const std::string& S) {
  return isWorkGroupScanExclusiveAdd(S) ||
         isWorkGroupScanInclusiveAdd(S) ||
         isWorkGroupScanExclusiveMin(S) ||
         isWorkGroupScanInclusiveMin(S) ||
         isWorkGroupScanExclusiveMax(S) ||
         isWorkGroupScanInclusiveMax(S) ||
         isWorkGroupScanExclusiveMul(S) ||
         isWorkGroupScanInclusiveMul(S);
}

bool CompilationUtils::isWorkGroupMin(const std::string& S) {
  return isWorkGroupReduceMin(S)             ||
         isWorkGroupScanExclusiveMin(S) ||
         isWorkGroupScanInclusiveMin(S);
}

bool CompilationUtils::isWorkGroupMax(const std::string& S) {
  return isWorkGroupReduceMax(S)             ||
         isWorkGroupScanExclusiveMax(S) ||
         isWorkGroupScanInclusiveMax(S);
}

// TODO: add ballot function, refactor DPCPPKernelVecClone -
// opencl-vec-uniform-return.
bool CompilationUtils::isSubGroupUniform(const std::string &S) {
  return isGetSubGroupSize(S) || isGetSubGroupId(S) ||
         isGetMaxSubGroupSize(S) || isGetNumSubGroups(S) ||
         isGetEnqueuedNumSubGroups(S) || isSubGroupAll(S) || isSubGroupAny(S) ||
         isSubGroupBroadCast(S) || isSubGroupReduceAdd(S) ||
         isSubGroupReduceMin(S) || isSubGroupReduceMax(S);
}

bool CompilationUtils::isSubGroupDivergent(const std::string &S) {
  return isGetSubGroupLocalId(S) || isSubGroupScan(S);
}

bool CompilationUtils::isSubGroupScan(const std::string &S) {
  return isSubGroupScanExclusiveAdd(S) || isSubGroupScanInclusiveAdd(S) ||
         isSubGroupScanExclusiveMin(S) || isSubGroupScanInclusiveMin(S) ||
         isSubGroupScanExclusiveMax(S) || isSubGroupScanInclusiveMax(S);
}

bool CompilationUtils::isSubGroupBuiltin(const std::string &S) {
  return isSubGroupUniform(S) || isSubGroupDivergent(S);
}

bool CompilationUtils::isWGUniform(const std::string &S) {
  return isWorkGroupUniform(S) || isGetMaxSubGroupSize(S) ||
         isGetNumSubGroups(S) || isGetEnqueuedNumSubGroups(S);
}

bool CompilationUtils::isWGDivergent(const std::string &S) {
  return isWorkGroupDivergent(S) ||
         (isSubGroupBuiltin(S) && !isWGUniform(S));
}

bool CompilationUtils::isWorkGroupUniform(const std::string& S) {
  return isWorkGroupAll(S)       ||
         isWorkGroupAny(S)       ||
         isWorkGroupBroadCast(S) ||
         isWorkGroupReduceAdd(S) ||
         isWorkGroupReduceMin(S) ||
         isWorkGroupReduceMax(S) ||
         isWorkGroupReduceMul(S) ||
         isWorkGroupReduceBitwiseAnd(S) ||
         isWorkGroupReduceBitwiseOr(S) ||
         isWorkGroupReduceBitwiseXor(S) ||
         isWorkGroupReduceLogicalAnd(S) ||
         isWorkGroupReduceLogicalOr(S) ||
         isWorkGroupReduceLogicalXor(S);
}

bool CompilationUtils::isWorkGroupDivergent(const std::string &S) {
  return isWorkGroupScan(S);
}

bool CompilationUtils::isImagesUsed(const Module &M) {
  for (unsigned i = 0,
       e = sizeof(ImageTypeNames)/sizeof(ImageTypeNames[0]); i < e; ++i) {
    if (StructType::getTypeByName(M.getContext(), ImageTypeNames[i]))
      return true;
  }

  return false;
}

bool CompilationUtils::isAtomicBuiltin(const std::string& funcName){
  // S is atomic built-in name if
  // - it's mangled (only built-in function names are mangled)
  // - it starts with "atom" (only atomic built-ins has "atom" prefix)
  if (!isMangledName(funcName.c_str()))
    return false;
  return std::string(stripName(funcName.c_str())).compare(0, 4, "atom") == 0;
}

bool CompilationUtils::isWorkItemPipeBuiltin(const std::string& funcName){
  auto Kind = getPipeKind(funcName);
  return Kind && Kind.Scope == PipeKind::WORK_ITEM;
}

bool CompilationUtils::isAtomicWorkItemFenceBuiltin(const std::string& funcName){
  // S is atomic built-in name if
  // - it's mangled (only built-in function names are mangled)
  // - it's equal to "atomic_work_item_fence" string
  if (!isMangledName(funcName.c_str()))
    return false;
  return stripName(funcName.c_str()) == NAME_ATOMIC_WORK_ITEM_FENCE;

}

bool CompilationUtils::isReadPipeBuiltin(const std::string& funcName) {
  auto Kind = getPipeKind(funcName);
  return Kind &&
    Kind.Scope == PipeKind::WORK_ITEM &&
    Kind.Access == PipeKind::READ &&
    (Kind.Op == PipeKind::READWRITE ||
     Kind.Op == PipeKind::READWRITE_RESERVE);
}

bool CompilationUtils::isWritePipeBuiltin(const std::string& funcName) {
  auto Kind = getPipeKind(funcName);
  return Kind &&
    Kind.Scope == PipeKind::WORK_ITEM &&
    Kind.Access == PipeKind::WRITE &&
    (Kind.Op == PipeKind::READWRITE ||
     Kind.Op == PipeKind::READWRITE_RESERVE);
}

PipeKind CompilationUtils::getPipeKind(const std::string &Name) {
  PipeKind Kind;
  Kind.Op = PipeKind::NONE;

  llvm::StringRef N(Name);
  if (!N.consume_front("__")) {
    return Kind;
  }

  if (N.consume_front("sub_group_")) {
    Kind.Scope = PipeKind::SUB_GROUP;
  } else if (N.consume_front("work_group_")) {
    Kind.Scope = PipeKind::WORK_GROUP;
  } else {
     Kind.Scope = PipeKind::WORK_ITEM;
  }

  if (N.consume_front("commit_")) {
    Kind.Op = PipeKind::COMMIT;
  } else if (N.consume_front("reserve_")) {
    Kind.Op = PipeKind::RESERVE;
  }

  if (N.consume_front("read_")) {
    Kind.Access = PipeKind::READ;
  } else if (N.consume_front("write_")) {
    Kind.Access = PipeKind::WRITE;
  } else {
    Kind.Op = PipeKind::NONE;
    return Kind; // not a pipe built-in
  }

  if (!N.consume_front("pipe")) {
    Kind.Op = PipeKind::NONE;
    return Kind; // not a pipe built-in
  }

  if (Kind.Op == PipeKind::COMMIT || Kind.Op == PipeKind::RESERVE) {
    // rest for the modifiers only appliy to read/write built-ins
    return Kind;
  }

  if (N.consume_front("_2")) {
    Kind.Op = PipeKind::READWRITE;
  } else if (N.consume_front("_4")) {
    Kind.Op = PipeKind::READWRITE_RESERVE;
  }

  // FPGA extension.
  if (N.consume_front("_bl")) {
    Kind.Blocking = true;
  } else {
    Kind.Blocking = false;
  }
  if (N.consume_front("_io")) {
    Kind.IO = true;
  } else {
    Kind.IO = false;
  }

  if (N.consume_front("_fpga")) {
    Kind.FPGA = true;
  }

  if (N.consume_front("_") && N.startswith("v")) {
    Kind.SimdSuffix = std::string(N);
  }

  assert(Name == getPipeName(Kind) &&
         "getPipeKind() and getPipeName() are not aligned!");

  return Kind;
}

std::string CompilationUtils::getPipeName(PipeKind Kind) {
  assert(Kind.Op != PipeKind::NONE && "Invalid pipe kind.");

  std::string Name("__");

  switch (Kind.Scope) {
  case PipeKind::WORK_GROUP:
    Name += "work_group_"; break;
  case PipeKind::SUB_GROUP:
    Name += "sub_group_"; break;
  case PipeKind::WORK_ITEM:
    break;
  }

  switch (Kind.Op) {
  case PipeKind::COMMIT:
    Name += "commit_"; break;
  case PipeKind::RESERVE:
    Name += "reserve_"; break;
  default:
    break;
  }

  switch (Kind.Access) {
  case PipeKind::READ:
    Name += "read_"; break;
  case PipeKind::WRITE:
    Name += "write_"; break;
  }
  Name += "pipe";

  switch (Kind.Op) {
  case PipeKind::READWRITE:
    Name += "_2"; break;
  case PipeKind::READWRITE_RESERVE:
    Name += "_4"; break;
  default:
    return Name; // rest for the modifiers only appliy to read/write
                 // built-ins
  }

  if (Kind.Blocking) {
     Name += "_bl";
  }
  if (Kind.IO) {
     Name += "_io";
  }

  if (Kind.FPGA) {
    Name += "_fpga";
  }

  if (!Kind.SimdSuffix.empty()) {
    Name += "_";
    Name += Kind.SimdSuffix;
  }

  return Name;
}

bool CompilationUtils::isPipeBuiltin(const std::string &Name) {
  return getPipeKind(Name);
}

ChannelKind CompilationUtils::getChannelKind(const std::string &Name) {
  ChannelKind Kind;

  std::tie(Kind.Access, Kind.Blocking)
    = StringSwitch<std::pair<ChannelKind::AccessKind, bool>>(Name)
    .StartsWith("_Z18read_channel_intel",  {ChannelKind::READ, true})
    .StartsWith("_Z21read_channel_nb_intel",  {ChannelKind::READ, false})
    .StartsWith("_Z19write_channel_intel",  {ChannelKind::WRITE, true})
    .StartsWith("_Z22write_channel_nb_intel",  {ChannelKind::WRITE, false})
    .Default({ChannelKind::NONE, false});

  return Kind;
}

Function *CompilationUtils::importFunctionDecl(Module *Dst,
                                               const Function *Orig,
                                               bool DuplicateIfExists) {
  assert(Dst && "Invalid module");
  assert(Orig && "Invalid function");

  std::vector<StructType *> DstSTys = Dst->getIdentifiedStructTypes();
  FunctionType *OrigFnTy = Orig->getFunctionType();

  SmallVector<Type *, 8> NewArgTypes;
  bool changed = false;
  for (auto *ArgTy : Orig->getFunctionType()->params()) {
    NewArgTypes.push_back(ArgTy);

    auto *STy = getStructFromTypePtr(ArgTy);
    if (!STy)
      continue;

    for (auto *DstSTy : DstSTys) {
      if (isSameStructType(DstSTy, STy)) {
        NewArgTypes.back() =
          mutatePtrElementType(cast<PointerType>(ArgTy), DstSTy);
        changed = true;
        break;
      }
    }
  }

  FunctionType *NewFnType =
    (!changed) ? OrigFnTy
               : FunctionType::get(Orig->getReturnType(),
                                   NewArgTypes,
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

StringRef CompilationUtils::stripStructNameTrailingDigits(StringRef TyName) {
  size_t Dot = TyName.find_last_of('.');

  // remove a '.' followed by any number of digits
  if (TyName.npos != Dot) {
    if (TyName.npos == TyName.find_first_not_of("0123456789", Dot + 1)) {
      return TyName.substr(0, Dot);
    }
  }

  return TyName;
}

bool CompilationUtils::isSameStructPtrType(Type *Ty1, Type *Ty2) {
  auto *PtrSTy1 = dyn_cast<PointerType>(Ty1);
  auto *PtrSTy2 = dyn_cast<PointerType>(Ty2);
  if (!PtrSTy1 || !PtrSTy2) {
    return false;
  }

  auto *STy1 = dyn_cast<StructType>(PtrSTy1->getElementType());
  auto *STy2 = dyn_cast<StructType>(PtrSTy2->getElementType());

  if (!STy1 || !STy2) {
    return false;
  }

  return isSameStructType(STy1, STy2);
}

bool CompilationUtils::isSameStructType(StructType *STy1, StructType *STy2) {
  if (!STy1->hasName() || !STy2->hasName()) {
    return false;
  }

  return 0 == stripStructNameTrailingDigits(STy1->getName())
    .compare(stripStructNameTrailingDigits(STy2->getName()));
}

StructType* CompilationUtils::getStructFromTypePtr(Type *Ty) {
  auto *PtrTy = dyn_cast<PointerType>(Ty);
  if (!PtrTy)
    return nullptr;

  // Handle also pointer to pointer to ...
  while (auto *PtrTyNext = dyn_cast<PointerType>(PtrTy->getElementType()))
    PtrTy = PtrTyNext;

  return dyn_cast<StructType>(PtrTy->getElementType());
}

StructType *CompilationUtils::getStructByName(StringRef Name, const Module *M) {
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

PointerType * CompilationUtils::mutatePtrElementType(
    PointerType *SrcPTy, Type *DstTy) {
  // The function changes the base type of SrcPTy to DstTy
  // SrcPTy = %struct.__pipe_t addrspace(1)**
  // DstTy  = %struct.__pipe_t.1
  // =>
  // %struct.__pipe_t.1 addrspace(1)**

  assert(SrcPTy && DstTy && "Invalid types!");

  SmallVector<PointerType *, 2> Types { SrcPTy };
  while ((SrcPTy = dyn_cast<PointerType>(SrcPTy->getElementType())))
    Types.push_back(SrcPTy);

  for (auto It = Types.rbegin(), E = Types.rend(); It != E; ++It)
    DstTy = PointerType::get(DstTy, (*It)->getAddressSpace());

  return cast<PointerType>(DstTy);
}

size_t CompilationUtils::getArrayNumElements(const ArrayType *ArrTy) {
  size_t NumElements = 1;
  Type *Ty = cast<Type>(const_cast<ArrayType*>(ArrTy));
  while (auto *InnerArrayTy = dyn_cast<ArrayType>(Ty)) {
    NumElements *= InnerArrayTy->getNumElements();
    Ty = InnerArrayTy->getElementType();
  }

  return NumElements;
}

Type * CompilationUtils::getArrayElementType(const ArrayType *ArrTy) {
  Type *ElemTy = ArrTy->getElementType();
  while (auto *InnerArrayTy = dyn_cast<ArrayType>(ElemTy)) {
    ElemTy = InnerArrayTy->getElementType();
  }

  return ElemTy;
}

ArrayType * CompilationUtils::createMultiDimArray(
    Type *Ty, const ArrayRef<size_t> &Dimensions) {
  ArrayType *MDArrayTy = nullptr;
  for (int i = Dimensions.size() - 1; i >= 0; --i) {
    if (!MDArrayTy) {
      MDArrayTy = ArrayType::get(Ty, Dimensions[i]);
    } else {
      MDArrayTy = ArrayType::get(MDArrayTy, Dimensions[i]);
    }
  }

  return MDArrayTy;
}

void CompilationUtils::getArrayTypeDimensions(
    const ArrayType *ArrTy, SmallVectorImpl<size_t> &Dimensions) {
  const ArrayType *InnerArrTy = ArrTy;
  do {
    Dimensions.push_back(InnerArrTy->getNumElements());
  } while ((InnerArrTy = dyn_cast<ArrayType>(InnerArrTy->getElementType())));
}

bool CompilationUtils::isGlobalCtorDtor(Function *F) {
  // TODO: implement good solution based on value of @llvm.global_ctors variable
  return F->getName() == "__pipe_global_ctor" ||
         F->getName() == "__pipe_global_dtor";
}

bool CompilationUtils::isGlobalCtorDtorOrCPPFunc(Function *F) {
  assert(F && "Invalid input for global ctor / dtor / cpp func check");
  return isGlobalCtorDtor(F) || F->hasFnAttribute("not-ocl-dpcpp");
}

static void recordCtorDtors(
    llvm::iterator_range<llvm::orc::CtorDtorIterator> CtorDtors,
    std::vector<std::string> &CtorDtorNames) {
  if (CtorDtors.empty())
    return;

  std::map<unsigned, std::vector<const llvm::Function *>> CtorDtorsByPriority;
  for (auto CtorDtor : CtorDtors) {
    assert(CtorDtor.Func && CtorDtor.Func->hasName() &&
           "Ctor/Dtor must be a named function");
    if (CtorDtor.Data &&
        llvm::cast<llvm::GlobalValue>(CtorDtor.Data)->isDeclaration())
      continue;

    if (CtorDtor.Func->hasLocalLinkage()) {
      CtorDtor.Func->setLinkage(llvm::GlobalValue::ExternalLinkage);
      CtorDtor.Func->setVisibility(llvm::GlobalValue::HiddenVisibility);
    }

    CtorDtorsByPriority[CtorDtor.Priority].push_back(CtorDtor.Func);
  }

  for (auto &KV : CtorDtorsByPriority) {
    for (auto &Func : KV.second)
      CtorDtorNames.push_back(Func->getName().str());
  }
}

void CompilationUtils::recordGlobalCtorDtors(
    llvm::Module &M, std::vector<std::string> &CtorNames,
    std::vector<std::string> &DtorNames) {
  recordCtorDtors(llvm::orc::getConstructors(M), CtorNames);
  recordCtorDtors(llvm::orc::getDestructors(M), DtorNames);
}

bool CompilationUtils::isBlockInvocationKernel(Function *F) {
  // TODO: Is there a better way to detect block invoke kernel?
  if (F->getName().contains("_block_invoke_") &&
      F->getName().endswith("_kernel_separated_args"))
    return true;

  return false;
}

void CompilationUtils::updateMetadataTreeWithNewFuncs(
    Module *M, DenseMap<Function *, Function *> &FunctionMap,
    MDNode *MDTreeNode, std::set<MDNode *> &Visited) {
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

void CompilationUtils::updateFunctionMetadata(
    Module *M, DenseMap<Function *, Function *> &FunctionMap) {
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
      std::set<MDNode *> Visited;
      updateMetadataTreeWithNewFuncs(M, FunctionMap, MDNodeOp, Visited);
    }
  }
}

bool CompilationUtils::isImplicitGID(AllocaInst *AI) {
  StringRef Name = AI->getName();
  static const std::vector<StringRef> ImplicitGIDs = {
      "__ocl_dbg_gid0", "__ocl_dbg_gid1", "__ocl_dbg_gid2"};
  for (auto &GID : ImplicitGIDs) {
    if (Name.equals(GID))
      return true;
  }
  return false;
}

std::string CompilationUtils::AppendWithDimension(std::string S,
                                                  int Dimension) {
  if (Dimension >= 0)
    S += '0' + Dimension;
  else
    S += "var";
  return S;
}

std::string CompilationUtils::AppendWithDimension(std::string S,
                                                  const Value *Dimension) {
  int D = -1;
  if (const ConstantInt *C = dyn_cast<ConstantInt>(Dimension))
    D = C->getZExtValue();
  return AppendWithDimension(S, D);
}

 bool CompilationUtils::hasByvalByrefArgs(Function *F) {
   for (auto &Arg : F->args()) {
     if (Arg.hasByValAttr() || Arg.hasByRefAttr())
       return true;
   }
   return false;
 }

}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {
