/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "CompilationUtils.h"
#include "ImplicitArgsUtils.h"
#include "NameMangleAPI.h"
#include "MetaDataApi.h"
#include "ParameterType.h"
#include "TypeAlignment.h"
#include "cl_types.h"

#include "CL/cl.h"

#include "llvm/IR/Metadata.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/DebugInfo.h"

#include "llvm/IR/DataLayout.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/StringSwitch.h"
#include "BlockUtils.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

  //TODO-MERGE: update value of CompilationUtils::NUMBER_IMPLICIT_ARGS wherever it is now
  const unsigned int CompilationUtils::LOCL_VALUE_ADDRESS_SPACE = 3;

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
  //Argument qualifiers
  const std::string CompilationUtils::WRITE_ONLY = "write_only";
  const std::string CompilationUtils::READ_ONLY  = "read_only";
  const std::string CompilationUtils::NONE       = "none";
  //Type qualifiers
  const std::string CompilationUtils::SAMPLER   = "sampler_t";

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
    delete pInst;

    if ( pBB->end() == prev ) {
      return pBB->begin();
    }

    return ++prev;
  }

  void CompilationUtils::getImplicitArgs(
      Function *pFunc, Argument **ppLocalMem, Argument **ppWorkDim,
      Argument **ppWGId, Argument **ppBaseGlbId, Argument **ppSpecialBuf,
      Argument **ppRunTimeHandle) {

      assert( pFunc && "Function cannot be null" );
      assert( pFunc->getArgumentList().size() >= ImplicitArgsUtils::NUMBER_IMPLICIT_ARGS && "implicit args was not added!" );

      // Iterating over explicit arguments
      Function::arg_iterator DestI = pFunc->arg_begin();

      // Go over the explicit arguments
      for ( unsigned int  i = 0;
        i < pFunc->getArgumentList().size() - ImplicitArgsUtils::NUMBER_IMPLICIT_ARGS; ++i ) {
          ++DestI;
      }

      // Retrieve all the implicit arguments which are not NULL

      if ( NULL != ppLocalMem ) {
          *ppLocalMem = &*DestI;
      }
      ++DestI;

      if ( NULL != ppWorkDim ) {
          *ppWorkDim = &*DestI;
      }
      ++DestI;

      if ( NULL != ppWGId ) {
          *ppWGId = &*DestI;
      }
      ++DestI;

      if ( NULL != ppBaseGlbId ) {
          *ppBaseGlbId = &*DestI;
      }
      ++DestI;

      if ( NULL != ppSpecialBuf ) {
          *ppSpecialBuf = &*DestI;
      }
      ++DestI;

      if ( NULL != ppRunTimeHandle ) {
          *ppRunTimeHandle = &*DestI;
      }
      ++DestI;
      assert(DestI == pFunc->arg_end());

  }

  void CompilationUtils::getAllSyncBuiltinsDcls(FunctionSet &functionSet, Module *pModule) {
    //Clear old collected data!
    functionSet.clear();

    for ( Module::iterator fi = pModule->begin(), fe = pModule->end(); fi != fe; ++fi ) {
      if ( !fi->isDeclaration() ) continue;
      llvm::StringRef func_name = fi->getName();
      if ( /* barrier built-ins */
          func_name == CompilationUtils::mangledBarrier() ||
          func_name == CompilationUtils::mangledWGBarrier(CompilationUtils::WG_BARRIER_NO_SCOPE) ||
          func_name == CompilationUtils::mangledWGBarrier(CompilationUtils::WG_BARRIER_WITH_SCOPE) ||
          /* work group built-ins */
          CompilationUtils::isWorkGroupBuiltin(func_name)  ||
          /* built-ins synced as if were called by a single work item */
          CompilationUtils::isWorkGroupAsyncOrPipeBuiltin(func_name, pModule) ) {
            // Found synchronized built-in declared in the module add it to the container set.
            functionSet.insert(&*fi);
      }
    }
  }

  void CompilationUtils::getAllKernels(FunctionSet &functionSet, Module *pModule) {
    //Clear old collected data!
    functionSet.clear();

    //List all kernels in module
    MetaDataUtils mdUtils(pModule);
    MetaDataUtils::KernelsList::const_iterator itr = mdUtils.begin_Kernels();
    MetaDataUtils::KernelsList::const_iterator end = mdUtils.end_Kernels();
    for (; itr != end; ++itr) {
      Function *pSclFunc = (*itr)->getFunction();
      functionSet.insert(pSclFunc);
      if(mdUtils.findKernelsInfoItem(pSclFunc) == mdUtils.end_KernelsInfo()) {
        //No kernel info for this scalar kernel, in this case there is no vector
        //version for this kernel, just skip to next kernel.
        continue;
      }
      //Check if there is a vectorized version
      KernelInfoMetaDataHandle kimd = mdUtils.getKernelsInfoItem(pSclFunc);
      //Need to check if Vectorized Kernel Value exists, it is not guaranteed that
      //Vectorized is running in all scenarios.
      if (kimd->isVectorizedKernelHasValue() && kimd->getVectorizedKernel() != NULL) {
        functionSet.insert(kimd->getVectorizedKernel());
      }
    }
  }

  void CompilationUtils::getAllKernelWrappers(FunctionSet &functionSet, Module *pModule) {
    //Clear old collected data!
    functionSet.clear();

    //List all kernels in module
    Intel::MetaDataUtils mdUtils(pModule);
    MetaDataUtils::KernelsInfoMap::const_iterator itr = mdUtils.begin_KernelsInfo();
    MetaDataUtils::KernelsInfoMap::const_iterator end = mdUtils.end_KernelsInfo();
    for (; itr != end; ++itr) {
      KernelInfoMetaDataHandle kimd = itr->second;
      if(kimd->isKernelWrapperHasValue()) {
        functionSet.insert(kimd->getKernelWrapper());
      }
    }
  }

  void CompilationUtils::parseKernelArguments(  Module* pModule,
                                                Function* pFunc,
                                                std::vector<cl_kernel_argument>& /* OUT */ arguments,
                                                std::vector<unsigned int>&       /* OUT */ memoryArguments) {
    // Check maximum number of arguments to kernel
    MetaDataUtils mdUtils(pModule);
    if (!mdUtils.isKernelsHasValue()) {
      assert(false && "Internal Error: kernels metadata is missing");
      // workaround to overcome klockwork issue
      return;
    }
    KernelInfoMetaDataHandle kimd = mdUtils.getKernelsInfoItem(pFunc);
    Function *pOriginalFunc = pFunc;
    //Check if this is a vectorized version of the kernel
    if (kimd->isScalarizedKernelHasValue() && kimd->getScalarizedKernel()) {
      //Get the scalarized version of the vectorized kernel
      pOriginalFunc = kimd->getScalarizedKernel();
    }
    // Check is this is a block kernel (i.e. a kernel that is invoked from a
    // NDRange from an other kernel), and if so what is the size of the block
    // literal. This information exists only in metadata of the scalar version.
    unsigned BlockLiteralSize = 0;
    KernelInfoMetaDataHandle skimd = mdUtils.getKernelsInfoItem(pOriginalFunc);
    if (skimd->isBlockLiteralSizeHasValue()) {
      BlockLiteralSize = skimd->getBlockLiteralSize();
    }

    KernelMetaDataHandle kmd;
    MetaDataUtils::KernelsList::const_iterator itr = mdUtils.begin_Kernels();
    MetaDataUtils::KernelsList::const_iterator end = mdUtils.end_Kernels();
    for (; itr != end; ++itr) {
      if (pOriginalFunc == (*itr)->getFunction()) {
        kmd = *itr;
        break;
      }
    }

    if( NULL == kmd.get() ) {
      assert(false && "Intenal error: can't find the function info for the scalarized function");
      // workaround to overcome klockwork issue
      return;
    }

    size_t argsCount = pFunc->getArgumentList().size() - ImplicitArgsUtils::NUMBER_IMPLICIT_ARGS;

    unsigned int localMemCount = 0;
    unsigned int current_offset = 0;
    llvm::Function::arg_iterator arg_it = pFunc->arg_begin();
    for (unsigned i=0; i<argsCount; ++i)
    {
      cl_kernel_argument curArg;
      bool               isMemoryObject = false;
      curArg.access = CL_KERNEL_ARG_ACCESS_NONE;

      llvm::Argument* pArg = &*arg_it;
      // Set argument sizes and offsets
      switch (arg_it->getType()->getTypeID())
      {
      case llvm::Type::FloatTyID:
          curArg.type = CL_KRNL_ARG_FLOAT;
          curArg.size_in_bytes = sizeof(float);
          break;

      case llvm::Type::StructTyID:
          {
              llvm::StructType *STy = llvm::cast<llvm::StructType>(arg_it->getType());
              curArg.type = CL_KRNL_ARG_COMPOSITE;
              DataLayout dataLayout(pModule);
              curArg.size_in_bytes = dataLayout.getTypeAllocSize(STy);
              break;
          }
      case llvm::Type::PointerTyID:
        {
          // check kernel is block_invoke kernel
          // in that case 0 argument is block_literal pointer
          // update with special type
          // should be before handling ptrs by addr space
          if((i == 0) && BlockLiteralSize){
            curArg.type = CL_KRNL_ARG_PTR_BLOCK_LITERAL;
            curArg.size_in_bytes = BlockLiteralSize;
            break;
          }

          llvm::PointerType *PTy = llvm::cast<llvm::PointerType>(arg_it->getType());
          if ( pArg->hasByValAttr() && PTy->getElementType()->getTypeID() == llvm::Type::VectorTyID )
          {
            // Check by pointer vector passing, used in long16 and double16
            llvm::VectorType *pVector = llvm::dyn_cast<llvm::VectorType>(PTy->getElementType());
            unsigned int uiNumElem = (unsigned int)pVector->getNumElements();;
            unsigned int uiElemSize = pVector->getContainedType(0)->getPrimitiveSizeInBits()/8;
            //assert( ((uiElemSize*uiNumElem) < 8 || (uiElemSize*uiNumElem) > 4*16) &&
            //  "We have byval pointer for legal vector type larger than 64bit");
            curArg.type = CL_KRNL_ARG_VECTOR_BY_REF;
            curArg.size_in_bytes = uiNumElem & 0xFFFF;
            curArg.size_in_bytes |= (uiElemSize << 16);
            break;
          }
          curArg.size_in_bytes = pModule->getDataLayout().getPointerSize(0);
          // Detect pointer qualifier
          // Test for opaque types: images, queue_t, pipe_t
          StructType *ST = dyn_cast<StructType>(PTy->getElementType());
          if(ST) {
            char const oclOpaquePref[] = "opencl.";
            const size_t oclOpaquePrefLen = sizeof(oclOpaquePref) - 1; // sizeof also counts the terminating 0

            if (ST->getName().startswith(oclOpaquePref))
            {
              const StringRef structName = ST->getName().substr(oclOpaquePrefLen);
              // Get opencl opaque type.
              // It is safe to use startswith while there are no names which aren't prefix of another name.
              if (structName.startswith("image1d_array_"))
                  curArg.type = CL_KRNL_ARG_PTR_IMG_1D_ARR;
              else if (structName.startswith("image1d_buffer_"))
                  curArg.type = CL_KRNL_ARG_PTR_IMG_1D_BUF;
              else if(structName.startswith("image1d_"))
                  curArg.type = CL_KRNL_ARG_PTR_IMG_1D;
              else if (structName.startswith("image2d_depth_"))
                  curArg.type = CL_KRNL_ARG_PTR_IMG_2D_DEPTH;
              else if (structName.startswith("image2d_array_depth_"))
                  curArg.type = CL_KRNL_ARG_PTR_IMG_2D_ARR_DEPTH;
              else if (structName.startswith("image2d_array_"))
                  curArg.type = CL_KRNL_ARG_PTR_IMG_2D_ARR;
              else if (structName.startswith("image2d_"))
                  curArg.type = CL_KRNL_ARG_PTR_IMG_2D;
              else if (structName.startswith("image3d_"))
                  curArg.type = CL_KRNL_ARG_PTR_IMG_3D;
              else if (structName.startswith("pipe_t"))
                  curArg.type = CL_KRNL_ARG_PTR_PIPE_T;
              else if (structName.startswith("queue_t"))
                  curArg.type = CL_KRNL_ARG_PTR_QUEUE_T;
              else if (structName.startswith("clk_event_t"))
                  curArg.type = CL_KRNL_ARG_PTR_CLK_EVENT_T;
              else if (structName.startswith("sampler_t"))
                  curArg.type = CL_KRNL_ARG_PTR_SAMPLER_T;
              else {
                  assert(false && "did you forget to handle a new special OpenCL C opaque type?");
                  // TODO: Why default type is INTEGER????
                  curArg.type = CL_KRNL_ARG_INT;
              }

              switch(curArg.type) {
                case CL_KRNL_ARG_PTR_IMG_1D:
                case CL_KRNL_ARG_PTR_IMG_1D_ARR:
                case CL_KRNL_ARG_PTR_IMG_1D_BUF:
                case CL_KRNL_ARG_PTR_IMG_2D:
                case CL_KRNL_ARG_PTR_IMG_2D_ARR:
                case CL_KRNL_ARG_PTR_IMG_2D_DEPTH:
                case CL_KRNL_ARG_PTR_IMG_2D_ARR_DEPTH:
                case CL_KRNL_ARG_PTR_IMG_3D:
                // Setup image pointer
                  isMemoryObject = true;
                  curArg.access = (kmd->getArgAccessQualifierItem(i) == READ_ONLY) ?
                                  CL_KERNEL_ARG_ACCESS_READ_ONLY : CL_KERNEL_ARG_ACCESS_READ_WRITE;    // Set RW/WR flag
                  break;
                // FIXME: what about Apple?
                case CL_KRNL_ARG_PTR_PIPE_T:
                  // The default access qualifier for pipes is read_only.
                  curArg.access = (kmd->getArgAccessQualifierItem(i) == WRITE_ONLY) ?
                                  CL_KERNEL_ARG_ACCESS_WRITE_ONLY : CL_KERNEL_ARG_ACCESS_READ_ONLY;
                  isMemoryObject = true;
                  break;
                case CL_KRNL_ARG_PTR_QUEUE_T:
                case CL_KRNL_ARG_PTR_CLK_EVENT_T:
                case CL_KRNL_ARG_PTR_SAMPLER_T:
                  isMemoryObject = false;
                  break;

                default:
                  break;
              }
              // Check this is a special OpenCL C opaque type.
              if(CL_KRNL_ARG_INT != curArg.type)
                break;
            }
            else if(dyn_cast<PointerType>(PTy->getElementType()))
            {
              // Pointer to pointer case.
              assert(false && "pointer to pointer is not allowed in kernel arguments");
            }
          }

          llvm::Type *Ty = PTy->getContainedType(0);
          if ( Ty->isStructTy() ) // struct or struct*
          {
            // Deal with structs passed by value. These are user-defined structs and ndrange_t.
            if(PTy->getAddressSpace() == 0)
            {
              llvm::StructType *STy = llvm::cast<llvm::StructType>(Ty);
              assert(!STy->isOpaque() &&
                     "cannot handle user-defined opaque types with an unknown size");
              DataLayout dataLayout(pModule);
              curArg.size_in_bytes = dataLayout.getTypeAllocSize(STy);
              curArg.type = CL_KRNL_ARG_COMPOSITE;
              break;
            }
          }

          switch (PTy->getAddressSpace())
          {
          case 0: case 1: // Global Address space
            curArg.type = CL_KRNL_ARG_PTR_GLOBAL;
            isMemoryObject = true;
            break;
          case 2:
            curArg.type = CL_KRNL_ARG_PTR_CONST;
            isMemoryObject = true;
            break;
          case 3: // Local Address space
            curArg.type = CL_KRNL_ARG_PTR_LOCAL;
            ++localMemCount;
            break;

          default:
            assert(0);
          }
        }
        break;

      case llvm::Type::IntegerTyID:
          {
            if (kmd->getArgTypesItem(i) == SAMPLER)
            {
              curArg.type = CL_KRNL_ARG_SAMPLER;
              curArg.size_in_bytes = sizeof(_sampler_t);
            }
            else
            {
              llvm::IntegerType *ITy = llvm::cast<llvm::IntegerType>(arg_it->getType());
              curArg.type = CL_KRNL_ARG_INT;
              curArg.size_in_bytes = pModule->getDataLayout().getTypeAllocSize(ITy);
            }
          }
          break;

      case llvm::Type::DoubleTyID:
        curArg.type = CL_KRNL_ARG_DOUBLE;
        curArg.size_in_bytes = sizeof(double);
        break;

      case llvm::Type::VectorTyID:
        {
          llvm::VectorType *pVector = llvm::dyn_cast<llvm::VectorType>(arg_it->getType());
          curArg.type = CL_KRNL_ARG_VECTOR;
          curArg.size_in_bytes = (unsigned int)(pVector->getNumElements() == 3 ? 4 : pVector->getNumElements());
          curArg.size_in_bytes |= (pVector->getContainedType(0)->getPrimitiveSizeInBits()/8)<<16;
        }
        break;

      default:
        assert(0 && "Unhandled parameter type");
      }

      // update offset
      assert( 0 != curArg.size_in_bytes && "argument size must be set");
      // Align current location to meet type's requirements
      current_offset = TypeAlignment::align(TypeAlignment::getAlignment(curArg), current_offset);
      curArg.offset_in_bytes = current_offset;
      // Advance offset beyond this argument
      current_offset += TypeAlignment::getSize(curArg);

      if ( isMemoryObject ) {
        memoryArguments.push_back(i);
      }
      arguments.push_back(curArg);
      ++arg_it;
    }
  }

  bool CompilationUtils::getCLVersionFromModule(const Module &M, unsigned &Result) {
    StringRef opt = fetchCompilerOption(M, "-cl-std=");
    if(!opt.empty()) {
      opt = opt.drop_front(strlen("-cl-std="));
      Result = OclVersion::CLStrToVal(opt.data());
      return true;
    }
    return fetchCLVersionFromMetadata(M, Result);
  }

  bool CompilationUtils::fetchCLVersionFromMetadata(const Module &M, unsigned &Result) {
    /*
    Example of the metadata
    !opencl.ocl.version = !{!6}
    !6 = !{i32 2, i32 0}
    */
    NamedMDNode* namedMD = M.getNamedMetadata("opencl.ocl.version");
    // Metadata API uitls creates whole set of empty named metadata even if they are initially
    // absent in a module. That is why the 'if' statement below checks if MDNode has operands.
    if(!namedMD || namedMD->getNumOperands() == 0) return false;

    MDNode * versionMD = cast<MDNode>(namedMD->getOperand(0));
    assert(versionMD && versionMD->getNumOperands() == 2 && "this MDNode must have 2 operands");

    Metadata * majorMD = versionMD->getOperand(0);
    Metadata * minorMD = versionMD->getOperand(1);
    assert(majorMD && minorMD && "expected non-null metadata values");

    uint64_t major = mdconst::extract<ConstantInt>(majorMD)->getZExtValue();;
    uint64_t minor = mdconst::extract<ConstantInt>(minorMD)->getZExtValue();
    Result = OclVersion::CLVersionToVal(major, minor);
    return true;
  }

  StringRef CompilationUtils::fetchCompilerOption(const Module &M, char const* prefix) {
    /*
    Example of the metadata:
    !opencl.compiler.options = !{!0}
    !0 = metadata !{metadata !"-cl-std=CL2.0"}

    !opencl.compiler.options = !{!9}
    !9 = metadata !{metadata !"-cl-fast-relaxed-math", metadata !"-cl-std=CL2.0"}
    */
    NamedMDNode* namedMetadata = M.getNamedMetadata("opencl.compiler.options");

    if(!namedMetadata || namedMetadata->getNumOperands() < 1)
      return StringRef();
    MDNode* metadata = namedMetadata->getOperand(0);
    if(!metadata)
      return StringRef();

    for (uint32_t k = 0, e = metadata->getNumOperands(); k != e; ++k) {
      Metadata * pSubNode = metadata->getOperand(k);
      if (!isa<MDString>(pSubNode))
        continue;
      StringRef value = cast<MDString>(pSubNode)->getString();
      if(value.startswith(prefix)) return value;
    }
    return StringRef();
  }

Function *CompilationUtils::AddMoreArgsToFunc(
    Function *F, ArrayRef<Type *> NewTypes, ArrayRef<const char *> NewNames,
    ArrayRef<AttributeSet> NewAttrs, StringRef Prefix, bool IsKernel) {
  assert(NewTypes.size() == NewNames.size());
  assert(NewTypes.size() == NewAttrs.size());
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
  if (IsKernel) {
    // Only those who are kernel functions need to get this calling convention
    NewF->setCallingConv(CallingConv::C);
  }
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
    if (!NewAttrs.empty() && !NewAttrs[I].isEmpty())
      A->addAttr(NewAttrs[I]);
  }
  // Since we have now created the new function, splice the body of the old
  // function right into the new function, leaving the old body of the function empty.
  NewF->getBasicBlockList().splice(NewF->begin(), F->getBasicBlockList());
  assert(F->isDeclaration() && "splice did not work, original function body is not empty!");

  // Set DISubprogram as an original function has. Do it before delete body
  // since DISubprogram will be deleted too
  NewF->setSubprogram(F->getSubprogram());
  F->setSubprogram(nullptr);

  // Delete original function body - this is needed to remove linkage (if exists)
  F->deleteBody();
  // Loop over the argument list, transferring uses of the old arguments over to
  // the new arguments.
  for (Function::arg_iterator I = F->arg_begin(), E = F->arg_end(),
                              NI = NewF->arg_begin(), NE = NewF->arg_end();
       I != E; ++I, ++NI) {
    // Replace the users to the new version.
    I->replaceAllUsesWith(&*NI);
  }
  return NewF;
}

CallInst *CompilationUtils::AddMoreArgsToCall(CallInst *OldC,
                                              ArrayRef<Value *> NewArgs,
                                              Function *NewF) {
  assert(OldC && "CallInst is NULL");
  assert(OldC->getNumArgOperands() + NewArgs.size() == NewF->arg_size());
  assert(NewF && "function is NULL");

  SmallVector<Value *, 16> Args;
  for (unsigned I = 0, E = OldC->getNumArgOperands(); I != E; ++I)
    Args.push_back(OldC->getArgOperand(I));
  Args.append(NewArgs.begin(), NewArgs.end());

  // Replace the original function with a call
  CallInst *NewC = CallInst::Create(NewF, Args, "", OldC);

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

template <reflection::TypePrimitiveEnum Ty>
static std::string optionalMangleWithParam(const char*const N){
  reflection::FunctionDescriptor FD;
  FD.name = N;
  reflection::ParamType *pTy =
    new reflection::PrimitiveType(Ty);
  reflection::RefParamType UI(pTy);
  FD.parameters.push_back(UI);
  return mangle(FD);
}

template <reflection::TypePrimitiveEnum Ty>
static std::string mangleWithParam(const char*const N, unsigned int numOfParams){
  reflection::FunctionDescriptor FD;
  FD.name = N;
  for(unsigned int i=0; i<numOfParams ; ++i) {
    reflection::ParamType *pTy =
      new reflection::PrimitiveType(Ty);
    reflection::RefParamType UI(pTy);
    FD.parameters.push_back(UI);
  }
  return mangle(FD);
}

static std::string mangleWithParam(const char*const N,
                                   ArrayRef<reflection::TypePrimitiveEnum> Types){
  reflection::FunctionDescriptor FD;
  FD.name = N;
  for (const auto &Ty:Types) {
    reflection::ParamType *pTy =
      new reflection::PrimitiveType(Ty);
    reflection::RefParamType UI(pTy);
    FD.parameters.push_back(UI);
  }
  return mangle(FD);
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

std::string CompilationUtils::mangledWGBarrier(WG_BARRIER_TYPE wgBarrierType) {
  switch(wgBarrierType) {
  case WG_BARRIER_NO_SCOPE:
    return mangleWithParam<reflection::PRIMITIVE_UINT>(WG_BARRIER_FUNC_NAME.c_str(), 1);
  case WG_BARRIER_WITH_SCOPE: {
    reflection::TypePrimitiveEnum Params[] = {
      reflection::PRIMITIVE_UINT,
      reflection::PRIMITIVE_INT };

    return mangleWithParam(WG_BARRIER_FUNC_NAME.c_str(), Params);
  }
  default:
    assert(false && "Unknown work_group_barrier version");
    return "";
  }
}

static bool isOptionalMangleOf(const std::string& LHS, const std::string& RHS) {
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

bool CompilationUtils::isGetSubGroupLocalID(const std::string& S){
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

static bool isBlockWithArgs(const std::string& S, unsigned idx) {
  reflection::FunctionDescriptor fd = demangle(S.c_str());
  if(fd.parameters.size() <= idx) return false;
  reflection::ParamType * paramTy = fd.parameters[idx];

  if(paramTy->getTypeId() != reflection::TYPE_ID_BLOCK)
    return false;

  reflection::BlockType * blockTy = static_cast<reflection::BlockType*>(paramTy);
  if(blockTy->getNumOfParams() == 0) return false;
  // Blocks have 'void' argument only when they don't have local memory arguments
  const reflection::ParamType *tmp = blockTy->getParam(0);
  const reflection::PrimitiveType * blockArgTy =
    static_cast<const reflection::PrimitiveType*>(tmp);
  return blockArgTy->getPrimitive() != reflection::PRIMITIVE_VOID;
}

bool CompilationUtils::isEnqueueKernelLocalMem(const std::string& S){
  if(!isMangleOf(S, NAME_ENQUEUE_KERNEL)) return false;
  return isBlockWithArgs(S, 3);
}

bool CompilationUtils::isEnqueueKernelEventsLocalMem(const std::string& S){
  if(!isMangleOf(S, NAME_ENQUEUE_KERNEL)) return false;
  return isBlockWithArgs(S, 6);
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
  std::string funcName = stripName(S.c_str());
  return StringRef(funcName).startswith(NAME_FINALIZE_WG_FUNCTION_PREFIX);
}

std::string CompilationUtils::appendWorkGroupFinalizePrefix(const std::string& S) {
  assert(isMangledName(S.c_str()) && "expected mangled name of work group built-in");
  reflection::FunctionDescriptor fd = demangle(S.c_str());
  fd.name = NAME_FINALIZE_WG_FUNCTION_PREFIX + fd.name;
  std::string finalizeFuncName = mangle(fd);
  return finalizeFuncName;
}

std::string CompilationUtils::removeWorkGroupFinalizePrefix(const std::string& S) {
  assert(hasWorkGroupFinalizePrefix(S) && "expected finilize prefix");
  reflection::FunctionDescriptor fd = demangle(S.c_str());
  fd.name = fd.name.substr(NAME_FINALIZE_WG_FUNCTION_PREFIX.size());
  std::string funcName = mangle(fd);
  return funcName;
}

bool CompilationUtils::isWorkGroupBuiltin(const std::string& S) {
  return isWorkGroupUniform(S) ||
         isWorkGroupScan(S);
}

bool CompilationUtils::isWorkGroupAsyncOrPipeBuiltin(const std::string& S, const Module* pModule) {
  return CompilationUtils::isAsyncWorkGroupCopy(S) ||
         CompilationUtils::isAsyncWorkGroupStridedCopy(S) ||
         (OclVersion::CL_VER_2_0 <= getCLVersionFromModuleOrDefault(*pModule) && (
            CompilationUtils::isWorkGroupReserveReadPipe(S) ||
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
         isWorkGroupScanInclusiveMax(S);
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

bool CompilationUtils::isWorkGroupUniform(const std::string& S) {
  return isWorkGroupAll(S)       ||
         isWorkGroupAny(S)       ||
         isWorkGroupBroadCast(S) ||
         isWorkGroupReduceAdd(S) ||
         isWorkGroupReduceMin(S) ||
         isWorkGroupReduceMax(S);
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
  // S is work item pipe built-in name if
  // - it doesn't start w\ "work_group" and ends with "_pipe"
  StringRef name(funcName);
  return !name.startswith("__work_group") &&
         (name.endswith("_pipe") || name.endswith("_pipe_2") ||
          name.endswith("_pipe_4"));
}

bool CompilationUtils::isAtomicWorkItemFenceBuiltin(const std::string& funcName){
  // S is atomic built-in name if
  // - it's mangled (only built-in function names are mangled)
  // - it's equal to "atomic_work_item_fence" string
  if (!isMangledName(funcName.c_str()))
    return false;
  return stripName(funcName.c_str()) == "atomic_work_item_fence";

}

bool CompilationUtils::isPipeBuiltin(const std::string &Name) {
  return llvm::StringSwitch<bool>(Name)
#ifdef BUILD_FPGA_EMULATOR
    .Case("__read_pipe_2_intel", true)
    .Case("__read_pipe_4_intel", true)
    .Case("__read_pipe_2_bl_intel", true)
    .Case("__write_pipe_2_intel", true)
    .Case("__write_pipe_4_intel", true)
    .Case("__write_pipe_2_bl_intel", true)
#else
    .Case("__read_pipe_2", true)
    .Case("__read_pipe_4", true)
    .Case("__write_pipe_2", true)
    .Case("__write_pipe_4", true)
#endif
    .Case("__sub_group_commit_read_pipe", true)
    .Case("__sub_group_reserve_read_pipe", true)
    .Case("__work_group_commit_read_pipe", true)
    .Case("__work_group_reserve_read_pipe", true)
    .Case("__commit_write_pipe", true)
    .Case("__reserve_write_pipe", true)
    .Case("__sub_group_commit_write_pipe", true)
    .Case("__sub_group_reserve_write_pipe", true)
    .Case("__work_group_commit_write_pipe", true)
    .Case("__work_group_reserve_write_pipe", true)
    .Default(false);
}

bool CompilationUtils::isReadPipeBuiltin(const std::string &Name) {
  return llvm::StringSwitch<bool>(Name)
#ifdef BUILD_FPGA_EMULATOR
    .Case("__read_pipe_2_intel", true)
    .Case("__read_pipe_4_intel", true)
    .Case("__read_pipe_2_bl_intel", true)
#else
    .Case("__read_pipe_2", true)
    .Case("__read_pipe_4", true)
#endif
    .Default(false);
}

bool CompilationUtils::isWritePipeBuiltin(const std::string &Name) {
  return llvm::StringSwitch<bool>(Name)
#ifdef BUILD_FPGA_EMULATOR
    .Case("__write_pipe_2_intel", true)
    .Case("__write_pipe_4_intel", true)
    .Case("__write_pipe_2_bl_intel", true)
#else
    .Case("__write_pipe_2", true)
    .Case("__write_pipe_4", true)
#endif
    .Default(false);
}

Constant *CompilationUtils::importFunctionDecl(Module *Dst,
                                               const Function *Orig) {
  assert(Dst && "Invalid module");
  assert(Orig && "Invalid function");

  std::vector<StructType *> DstSTys = Dst->getIdentifiedStructTypes();
  FunctionType *OrigFnTy = Orig->getFunctionType();

  SmallVector<Type *, 8> NewArgTypes;
  bool changed = false;
  for (auto *ArgTy : Orig->getFunctionType()->params()) {
    NewArgTypes.push_back(ArgTy);

    auto *PtrSTy = dyn_cast<PointerType>(ArgTy);
    if (!PtrSTy)
      continue;

    auto *STy = dyn_cast<StructType>(PtrSTy->getElementType());
    if (!STy)
      continue;

    for (auto *DstSTy : DstSTys) {
      if (isSameStructType(DstSTy, STy)) {
        NewArgTypes.back() = PointerType::get(DstSTy,
                                              PtrSTy->getAddressSpace());
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

  return Dst->getOrInsertFunction(
    Orig->getName(), NewFnType, Orig->getAttributes());
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
    Type *Ty, const SmallVectorImpl<size_t> &Dimensions) {
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

}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {
