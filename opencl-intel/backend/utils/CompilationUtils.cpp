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

#if defined(__APPLE__)
  #include "OpenCL/cl.h"
#else
  #include "CL/cl.h"
#endif

#include "llvm/IR/Metadata.h"
#include "llvm/IR/Instructions.h"
#include "llvm/DebugInfo.h"

#include "llvm/IR/DataLayout.h"
#include "llvm/ADT/SetVector.h"
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

  const std::string CompilationUtils::NAME_GET_WORK_DIM = "get_work_dim";
  const std::string CompilationUtils::NAME_GET_GLOBAL_SIZE = "get_global_size";
  const std::string CompilationUtils::NAME_GET_LOCAL_SIZE = "get_local_size";
  const std::string CompilationUtils::NAME_GET_ENQUEUED_LOCAL_SIZE = "get_enqueued_local_size";
  const std::string CompilationUtils::NAME_GET_NUM_GROUPS = "get_num_groups";
  const std::string CompilationUtils::NAME_GET_GROUP_ID = "get_group_id";
  const std::string CompilationUtils::NAME_GET_GLOBAL_OFFSET = "get_global_offset";
  const std::string CompilationUtils::NAME_PRINTF = "printf";

  const std::string CompilationUtils::NAME_ASYNC_WORK_GROUP_COPY = "async_work_group_copy";
  const std::string CompilationUtils::NAME_WAIT_GROUP_EVENTS = "wait_group_events";
  const std::string CompilationUtils::NAME_PREFETCH = "prefetch";
  const std::string CompilationUtils::NAME_ASYNC_WORK_GROUP_STRIDED_COPY = "async_work_group_strided_copy";

  const std::string CompilationUtils::NAME_WORK_GROUP_RESERVE_READ_PIPE = "work_group_reserve_read_pipe";
  const std::string CompilationUtils::NAME_WORK_GROUP_COMMIT_READ_PIPE = "work_group_commit_read_pipe";
  const std::string CompilationUtils::NAME_WORK_GROUP_RESERVE_WRITE_PIPE = "work_group_reserve_write_pipe";
  const std::string CompilationUtils::NAME_WORK_GROUP_COMMIT_WRITE_PIPE = "work_group_commit_write_pipe";

  const std::string CompilationUtils::NAME_MEM_FENCE = "mem_fence";
  const std::string CompilationUtils::NAME_READ_MEM_FENCE = "read_mem_fence";
  const std::string CompilationUtils::NAME_WRITE_MEM_FENCE = "write_mem_fence";
  // Extended execution var args OpenCL 2.x
  const std::string CompilationUtils::NAME_ENQUEUE_KERNEL_LOCALMEM = "_Z14enqueue_kernel9ocl_queuei9ndrange_tU13block_pointerFvPU3AS3vzEjz";
  const std::string CompilationUtils::NAME_ENQUEUE_KERNEL_EVENTS_LOCALMEM = "_Z14enqueue_kernel9ocl_queuei9ndrange_tjPKU3AS113ocl_clk_eventPU3AS113ocl_clk_eventU13block_pointerFvPU3AS3vzEjz";

  const std::string CompilationUtils::BARRIER_FUNC_NAME = "barrier";
  const std::string CompilationUtils::WG_BARRIER_FUNC_NAME = "work_group_barrier";

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

    Instruction* pInst = it;
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
          *ppLocalMem = DestI;
      }
      ++DestI;

      if ( NULL != ppWorkDim ) {
          *ppWorkDim = DestI;
      }
      ++DestI;

      if ( NULL != ppWGId ) {
          *ppWGId = DestI;
      }
      ++DestI;

      if ( NULL != ppBaseGlbId ) {
          *ppBaseGlbId = DestI;
      }
      ++DestI;

      if ( NULL != ppSpecialBuf ) {
          *ppSpecialBuf = DestI;
      }
      ++DestI;

      if ( NULL != ppRunTimeHandle ) {
          *ppRunTimeHandle = DestI;
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

#ifdef __APPLE__
      NamedMDNode *MDArgInfo = pModule->getNamedMetadata("opencl.kernels");
  if( NULL == MDArgInfo )
  {
      assert(false && "Internal Error: opencl.kernels metadata is missing");
      // workaround to overcome klockwork issue
      return;
  }

  // TODO: this hack is ugly, need to find the right way to get arg info
  // for the vectorized functions (Guy)
  if (pFunc->getName().startswith("____Vectorized_.")) {
    std::string scalarFuncName = pFunc->getName().slice(16,llvm::StringRef::npos).str();
    pFunc=pFunc->getParent()->getFunction("__" + scalarFuncName);
  }

  MDNode *FuncInfo = NULL;
  for (int i = 0, e = MDArgInfo->getNumOperands(); i < e; i++) {
    FuncInfo = MDArgInfo->getOperand(i);
    Value *field0 = FuncInfo->getOperand(0)->stripPointerCasts();

    if(pFunc == dyn_cast<Function>(field0))
      break;
  }

  if( NULL == FuncInfo )
  {
      assert(false && "Intenal error: can't find the function info for the scalarized function");
      // workaround to overcome klockwork issue
      return;
  }

  assert(FuncInfo->getNumOperands() > 1 && "Invalid number of kernel properties."
     " Are you running a workload recorded using old meta data format?");

    MDNode *MDImgAccess = NULL;
    //look for image access metadata
    for (int i = 1, e = FuncInfo->getNumOperands(); i < e; i++) {
      MDNode *tmpMD = dyn_cast<MDNode>(FuncInfo->getOperand(i));
      MDString *tag = dyn_cast<MDString>(tmpMD->getOperand(0));
      if (tag->getString() == "apple.cl.arg_metadata") {
        MDImgAccess = tmpMD;
        break;
      }
    }
#endif
    size_t argsCount = pFunc->getArgumentList().size() - ImplicitArgsUtils::NUMBER_IMPLICIT_ARGS;

    unsigned int localMemCount = 0;
#ifndef __APPLE__
    unsigned int current_offset = 0;
#endif
    llvm::Function::arg_iterator arg_it = pFunc->arg_begin();
    for (unsigned i=0; i<argsCount; ++i)
    {
      cl_kernel_argument curArg;
#ifndef __APPLE__
      bool               isMemoryObject = false;
#endif
      curArg.access = CL_KERNEL_ARG_ACCESS_NONE;

      llvm::Argument* pArg = arg_it;
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
          curArg.size_in_bytes = pModule->getDataLayout()->getPointerSize(0);
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
              if(structName.startswith("image1d_t"))
                  curArg.type = CL_KRNL_ARG_PTR_IMG_1D;
              else if (structName.startswith("image1d_array_t"))
                  curArg.type = CL_KRNL_ARG_PTR_IMG_1D_ARR;
              else if (structName.startswith("image1d_buffer_t"))
                  curArg.type = CL_KRNL_ARG_PTR_IMG_1D_BUF;
              else if (structName.startswith("image2d_t"))
                  curArg.type = CL_KRNL_ARG_PTR_IMG_2D;
              else if (structName.startswith("image2d_array_t"))
                  curArg.type = CL_KRNL_ARG_PTR_IMG_2D_ARR;
              else if (structName.startswith("image2d_depth_t"))
                  curArg.type = CL_KRNL_ARG_PTR_IMG_2D_DEPTH;
              else if (structName.startswith("image2d_array_depth_t"))
                  curArg.type = CL_KRNL_ARG_PTR_IMG_2D_ARR_DEPTH;
              else if (structName.startswith("image3d_t"))
                  curArg.type = CL_KRNL_ARG_PTR_IMG_3D;
              else if (structName.startswith("pipe_t"))
                  curArg.type = CL_KRNL_ARG_PTR_PIPE_T;
              else if (structName.startswith("queue_t"))
                  curArg.type = CL_KRNL_ARG_PTR_QUEUE_T;
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
#ifdef __APPLE__
                  MDNode *tmpMD = dyn_cast<MDNode>(MDImgAccess->getOperand(i+1));
                  assert((tmpMD->getNumOperands() > 0) && "image MD arg type is empty");
                  MDString *tag = dyn_cast<MDString>(tmpMD->getOperand(0));
                  assert(tag->getString() == "image" && "image MD arg type is not 'image'");
                  tag = dyn_cast<MDString>(tmpMD->getOperand(1));
                  curArg.access = (tag->getString() == "read") ? CL_KERNEL_ARG_ACCESS_READ_ONLY :
                                  CL_KERNEL_ARG_ACCESS_READ_WRITE;    // Set RW/WR flag
#else
                  isMemoryObject = true;
                  curArg.access = (kmd->getArgAccessQualifierItem(i) == READ_ONLY) ?
                                  CL_KERNEL_ARG_ACCESS_READ_ONLY : CL_KERNEL_ARG_ACCESS_READ_WRITE;    // Set RW/WR flag
#endif
                  break;
                // FIXME: what about Apple?
                case CL_KRNL_ARG_PTR_PIPE_T:
                  // The default access qualifier for pipes is read_only.
                  curArg.access = (kmd->getArgAccessQualifierItem(i) == WRITE_ONLY) ?
                                  CL_KERNEL_ARG_ACCESS_WRITE_ONLY : CL_KERNEL_ARG_ACCESS_READ_ONLY;
                  isMemoryObject = true;
                  break;
                case CL_KRNL_ARG_PTR_QUEUE_T:
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
#ifndef __APPLE__
            isMemoryObject = true;
#endif
            break;
          case 2:
            curArg.type = CL_KRNL_ARG_PTR_CONST;
#ifndef __APPLE__
            isMemoryObject = true;
#endif
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
  #ifdef __APPLE__
            MDNode *tmpMD = dyn_cast<MDNode>(MDImgAccess->getOperand(i+1));
            bool isSampler = false;
            if(tmpMD->getNumOperands() > 0) {
              MDString *tag = dyn_cast<MDString>(tmpMD->getOperand(0));
              if(tag->getString() == "sampler") //sampler_t
                  isSampler = true;
            }
            if(isSampler)
  #else
            if (kmd->getArgTypesItem(i) == SAMPLER)
  #endif
            {
              curArg.type = CL_KRNL_ARG_SAMPLER;
              curArg.size_in_bytes = sizeof(_sampler_t);
            }
            else
            {
              llvm::IntegerType *ITy = llvm::cast<llvm::IntegerType>(arg_it->getType());
              curArg.type = CL_KRNL_ARG_INT;
              curArg.size_in_bytes =  DataLayout(pModule->getDataLayout()).getTypeAllocSize(ITy);
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
        assert(0 && "Unhelded parameter type");
      }

      // update offset
#ifndef __APPLE__
      assert( 0 != curArg.size_in_bytes && "argument size must be set");
      // Align current location to meet type's requirements
      current_offset = TypeAlignment::align(TypeAlignment::getAlignment(curArg), current_offset);
      curArg.offset_in_bytes = current_offset;
      // Advance offset beyond this argument
      current_offset += TypeAlignment::getSize(curArg);

      if ( isMemoryObject ) {
        memoryArguments.push_back(i);
      }
#endif
      arguments.push_back(curArg);
      ++arg_it;
    }
  }

  bool CompilationUtils::getCLVersionFromModule(const Module &M, unsigned &Result) {
    StringRef opt = fetchCompilerOption(M, "-cl-std=");
    if(opt.empty()) return false;

    opt = opt.drop_front(strlen("-cl-std="));
    Result = OclVersion::CLStrToVal(opt.data());
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
      Value * pSubNode = metadata->getOperand(k);
      if (!isa<MDString>(pSubNode))
        continue;
      StringRef value = cast<MDString>(pSubNode)->getString();
      if(value.startswith(prefix)) return value;
    }
    return StringRef();
  }

  static const MDNode *findSubprogram(const DebugInfoFinder &finder,
                                      const Function *pFunc) {
    for (DebugInfoFinder::iterator iter = finder.subprogram_begin(),
                                   end = finder.subprogram_end();
         iter != end; iter++) {
      const MDNode *node = *iter;
      if (DISubprogram(node).describes(pFunc))
        return node;
    }
    return NULL;
  }
  static void SpliceDebugInfo(Function *SrcF, Function *DstF) {
    Module *M = SrcF->getParent();
    assert(M == DstF->getParent());
    DebugInfoFinder Finder;
    Finder.processModule(*M);
    if (const MDNode *SubProgramNode = findSubprogram(Finder, SrcF)) {
      DISubprogram(SubProgramNode).replaceFunction(DstF);
    }
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
    Argument *A = NewI;
    A->setName(NewNames[I]);
    if (!NewAttrs.empty() && !NewAttrs[I].isEmpty())
      A->addAttr(NewAttrs[I]);
  }
  // Since we have now created the new function, splice the body of the old
  // function right into the new function, leaving the old body of the function empty.
  NewF->getBasicBlockList().splice(NewF->begin(), F->getBasicBlockList());
  assert(F->isDeclaration() && "splice did not work, original function body is not empty!");
  // Delete original function body - this is needed to remove linkage (if exists)
  F->deleteBody();
  // Loop over the argument list, transferring uses of the old arguments over to
  // the new arguments.
  for (Function::arg_iterator I = F->arg_begin(), E = F->arg_end(),
                              NI = NewF->arg_begin(), NE = NewF->arg_end();
       I != E; ++I, ++NI) {
    // Replace the users to the new version.
    I->replaceAllUsesWith(NI);
  }
  SpliceDebugInfo(F, NewF);
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
#ifdef __APPLE__
  //Do not mangle
  return std::string(N);
#else
  reflection::FunctionDescriptor FD;
  FD.name = N;
  reflection::ParamType *pTy =
    new reflection::PrimitiveType(Ty);
  reflection::RefParamType UI(pTy);
  FD.parameters.push_back(UI);
  return mangle(FD);
#endif
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
  case WG_BARRIER_WITH_SCOPE:
    return mangleWithParam<reflection::PRIMITIVE_UINT>(WG_BARRIER_FUNC_NAME.c_str(), 2);
  default:
    assert(false && "Unknown work_group_barrier version");
    return "";
  }
}

static bool isOptionalMangleOf(const std::string& LHS, const std::string& RHS) {
#ifdef __APPLE__
  //LHS should not be mangled
  return LHS == RHS;
#else
  //LHS should be mangled
  const char*const LC = LHS.c_str();
  if (!isMangledName(LC))
    return false;
  return stripName(LC) == RHS;
#endif
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

bool CompilationUtils::isGetGlobalLinearId(const std::string& S){
  return isOptionalMangleOf(S, NAME_GET_LINEAR_GID);
}

bool CompilationUtils::isGetLocalLinearId(const std::string& S){
  return isOptionalMangleOf(S, NAME_GET_LINEAR_LID);
}

bool CompilationUtils::isGetGlobalSize(const std::string& S){
  return isOptionalMangleOf(S, NAME_GET_GLOBAL_SIZE);
}

bool CompilationUtils::isGetLocalSize(const std::string& S){
  return isOptionalMangleOf(S, NAME_GET_LOCAL_SIZE);
}

bool CompilationUtils::isGetEnqueuedLocalSize(const std::string& S){
  return isMangleOf(S, NAME_GET_ENQUEUED_LOCAL_SIZE);
}

bool CompilationUtils::isGetNumGroups(const std::string& S){
  return isOptionalMangleOf(S, NAME_GET_NUM_GROUPS);
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

bool CompilationUtils::isWorkGroupReserveReadPipe(const std::string& S){
  return isMangleOf(S, NAME_WORK_GROUP_RESERVE_READ_PIPE);
}

bool CompilationUtils::isWorkGroupCommitReadPipe(const std::string& S){
  return isMangleOf(S, NAME_WORK_GROUP_COMMIT_READ_PIPE);
}

bool CompilationUtils::isWorkGroupReserveWritePipe(const std::string& S){
  return isMangleOf(S, NAME_WORK_GROUP_RESERVE_WRITE_PIPE);
}

bool CompilationUtils::isWorkGroupCommitWritePipe(const std::string& S){
  return isMangleOf(S, NAME_WORK_GROUP_COMMIT_WRITE_PIPE);
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

bool CompilationUtils::isEnqueueKernelLocalMem(const std::string& S){
  // TODO: fix CSSD100018608 [OpenCL2.0]ExtExecution. Switch method of detetction of enqueue_kernel BI with variadic arguments to use mangler
  return (S == CompilationUtils::NAME_ENQUEUE_KERNEL_LOCALMEM);
}

bool CompilationUtils::isEnqueueKernelEventsLocalMem(const std::string& S){
  // TODO: fix CSSD100018608 [OpenCL2.0]ExtExecution. Switch method of detetction of enqueue_kernel BI with variadic arguments to use mangler
  return (S == CompilationUtils::NAME_ENQUEUE_KERNEL_EVENTS_LOCALMEM);
}

bool CompilationUtils::isWorkGroupAll(const std::string& S) {
  return isMangleOf(S, NAME_WORK_GROUP_ALL);
}

bool CompilationUtils::isWorkGroupAny(const std::string& S) {
  return isMangleOf(S, NAME_WORK_GROUP_ANY);
}

bool CompilationUtils::isWorkGroupBroadCast(const std::string& S) {
  return isMangleOf(S, NAME_WORK_GROUP_BROADCAST);
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
  // - it's mangled (only built-in function names are mangled)
  // - it doesn't start w\ "work_group" and ends with "_pipe"
  if (!isMangledName(funcName.c_str()))
    return false;
  StringRef name = stripName(funcName.c_str());
  return !name.startswith("work_group") && name.endswith("_pipe");
}

bool CompilationUtils::isAtomicWorkItemFenceBuiltin(const std::string& funcName){
  // S is atomic built-in name if
  // - it's mangled (only built-in function names are mangled)
  // - it's equal to "atomic_work_item_fence" string
  if (!isMangledName(funcName.c_str()))
    return false;
  return stripName(funcName.c_str()) == "atomic_work_item_fence";

}

}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {
