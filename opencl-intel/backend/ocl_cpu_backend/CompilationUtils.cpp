/*****************************************************************************\

Copyright (c) Intel Corporation (2010-2011).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  CompilationUtils.cpp

\*****************************************************************************/

#include "CompilationUtils.h"

#include "llvm/Metadata.h"
#include "llvm/Instructions.h"
#include <llvm/Target/TargetData.h>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

  const unsigned int CompilationUtils::NUMBER_IMPLICIT_ARGS = 9;
  const unsigned int CompilationUtils::LOCL_VALUE_ADDRESS_SPACE = 3;

  const std::string CompilationUtils::NAME_GET_BASE_GID = "get_base_global_id.";
  const std::string CompilationUtils::NAME_GET_GID = "get_new_global_id.";
  const std::string CompilationUtils::NAME_GET_LID = "get_new_local_id.";
  const std::string CompilationUtils::NAME_GET_ITERATION_COUNT = "get_iter_count.";
  const std::string CompilationUtils::NAME_GET_SPECIAL_BUFFER = "get_special_buffer.";
  const std::string CompilationUtils::NAME_GET_CURR_WI = "get_curr_wi.";

  const std::string CompilationUtils::NAME_GET_WORK_DIM = "get_work_dim";
  const std::string CompilationUtils::NAME_GET_GLOBAL_SIZE = "get_global_size";
  const std::string CompilationUtils::NAME_GET_LOCAL_SIZE = "get_local_size";
  const std::string CompilationUtils::NAME_GET_NUM_GROUPS = "get_num_groups";
  const std::string CompilationUtils::NAME_GET_GROUP_ID = "get_group_id";
  const std::string CompilationUtils::NAME_GET_GLOBAL_OFFSET = "get_global_offset";
  const std::string CompilationUtils::NAME_PRINTF = "printf";

  const std::string CompilationUtils::NAME_ASYNC_WORK_GROUP_COPY = "_Z21async_work_group_copy";
  const std::string CompilationUtils::NAME_WAIT_GROUP_EVENTS = "_Z17wait_group_events";
  const std::string CompilationUtils::NAME_PREFETCH = "_Z8prefetch";
  const std::string CompilationUtils::NAME_ASYNC_WORK_GROUP_STRIDED_COPY = "_Z29async_work_group_strided_copy";

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

  void CompilationUtils::getImplicitArgs(Function *pFunc,
    Argument **ppLocalMem, Argument **ppWorkDim, Argument **ppWGId,
    Argument **ppBaseGlbId, Argument **ppLocalId, Argument **ppIterCount,
    Argument **ppSpecialBuf, Argument **ppCurrWI, Argument **ppCtx) {

      assert( pFunc && "Function cannot be null" );
      assert( pFunc->getArgumentList().size() >= NUMBER_IMPLICIT_ARGS && "implicit args was not added!" );
        
      // Iterating over explicit arguments
      Function::arg_iterator DestI = pFunc->arg_begin();
      
      // Go over the explicit arguments
      for ( unsigned int  i = 0;
        i < pFunc->getArgumentList().size() - NUMBER_IMPLICIT_ARGS; ++i ) {
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
      
      if ( NULL != ppLocalId ) {
          *ppLocalId = DestI;
      }
      ++DestI;
      
      if ( NULL != ppCtx ) {
          *ppCtx = DestI;
      }
      ++DestI;

      if ( NULL != ppIterCount ) {
          *ppIterCount = DestI;
      }
      ++DestI;

      if ( NULL != ppSpecialBuf ) {
          *ppSpecialBuf = DestI;
      }
      ++DestI;

      if ( NULL != ppCurrWI ) {
          *ppCurrWI = DestI;
      }
  }

  void CompilationUtils::getAllScalarKernels(std::set<Function*> &functionSet, Module *pModule) {
    //Clear old collected data!
    functionSet.clear();
    //Check for some common module errors, before actually diving in
    NamedMDNode *pOpenCLMetadata = pModule->getNamedMetadata("opencl.kernels");
    if ( !pOpenCLMetadata ) {
      //Module contains no MetaData, thus it contains no kernels
      return;
    }

    unsigned int numOfKernels = pOpenCLMetadata->getNumOperands();
    if ( numOfKernels == 0 ) {
      //Module contains no kernels
      return;
    }

    //List all kernels in module
    for ( unsigned int i = 0, e = numOfKernels; i != e; ++i ) {
      MDNode *elt = pOpenCLMetadata->getOperand(i);
      Value *field0 = elt->getOperand(0)->stripPointerCasts();
      if ( Function *pKernelFunc = dyn_cast<Function>(field0) )
      {
        //Add kernel to the list
        //Currently no check if kernel already added to the list!
        functionSet.insert(pKernelFunc);
      }
    }
  }

void CompilationUtils::parseKernelArguments(  Module* pModule, 
                                              Function* pFunc, 
                                              std::vector<cl_kernel_argument>& /* OUT */ arguments) {
  // Check maximum number of arguments to kernel
  NamedMDNode *MDArgInfo = pModule->getNamedMetadata("opencl.kernels");

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
 
  assert(FuncInfo);

  MDNode *MDImgAccess = NULL;
  //look for image access metadata
  for (int i = 1, e = FuncInfo->getNumOperands(); i < e; i++) {
    MDNode *tmpMD = dyn_cast<MDNode>(FuncInfo->getOperand(i));
    MDString *tag = dyn_cast<MDString>(tmpMD->getOperand(0));
    
    if (tag->getString() == "image_access_qualifier") {
      MDImgAccess = tmpMD;
      break;
    }
  }

  size_t argsCount = pFunc->getArgumentList().size() - NUMBER_IMPLICIT_ARGS;

  // This check is wrong - CPU_KERNEL_MAX_ARG_COUNT is meaningless.
  //if (CPU_KERNEL_MAX_ARG_COUNT < argsCount)
  //  throw Exceptions::CompilerException(std::string("Too many arguments in kernel<") + pFunc->getName().str() + ">" , CL_DEV_BUILD_ERROR);

  unsigned int localMemCount = 0;

  llvm::Function::arg_iterator arg_it = pFunc->arg_begin();
  for (unsigned i=0; i<argsCount; ++i)
  {
    cl_kernel_argument curArg;

    llvm::Argument* pArg = arg_it;
    // Set argument sizes
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
            TargetData targetData(pModule);
            curArg.size_in_bytes = targetData.getTypeAllocSize(STy);
            break;
        }
    case llvm::Type::PointerTyID:
      {
        llvm::PointerType *PTy = llvm::cast<llvm::PointerType>(arg_it->getType());
        if ( pArg->hasByValAttr() && PTy->getElementType()->getTypeID() == llvm::Type::VectorTyID )
        {
          // Check by pointer vector passing, used in long16 and double16
          llvm::VectorType *pVector = llvm::dyn_cast<llvm::VectorType>(PTy->getElementType());
          unsigned int uiNumElem = (unsigned int)pVector->getNumElements();;
          unsigned int uiElemSize = pVector->getContainedType(0)->getPrimitiveSizeInBits()/8;
          if ( (uiElemSize*uiNumElem) > 4*16 )
          {
            curArg.type = CL_KRNL_ARG_VECTOR;
            curArg.size_in_bytes = uiNumElem & 0xFFFF;
            curArg.size_in_bytes |= (uiElemSize << 16);
            break;
          }
        }
        curArg.size_in_bytes = 0;
        // Detect pointer qualifier
        // Test for image
        //const std::string &imgArg = pFunc->getParent()->getTypeName(PTy->getElementType());
        StructType *ST = dyn_cast<StructType>(PTy->getElementType());
        if(ST) {
          const std::string &imgArg = ST->getName();
          if ( std::string::npos != imgArg.find("struct._image"))    // Image identifier was found
          {
            curArg.type = CL_KRNL_ARG_INT;

            // Get dimension image type
            if(imgArg.find("struct._image1d_t") != std::string::npos)
                curArg.type = CL_KRNL_ARG_PTR_IMG_1D;
            else if(imgArg.find("struct._image1d_array_t") != std::string::npos)
                curArg.type = CL_KRNL_ARG_PTR_IMG_1D_ARR;
            else if(imgArg.find("struct._image1d_buffer_t") != std::string::npos)
                curArg.type = CL_KRNL_ARG_PTR_IMG_1D_BUF;
            else if (imgArg.find("struct._image2d_t") != std::string::npos)
                curArg.type = CL_KRNL_ARG_PTR_IMG_2D;
            else if (imgArg.find("struct._image2d_array_t") != std::string::npos)
                curArg.type = CL_KRNL_ARG_PTR_IMG_2D;
            else if(imgArg.find("struct._image3d_t") != std::string::npos)
                curArg.type = CL_KRNL_ARG_PTR_IMG_3D;

            // Setup image pointer
            if(curArg.type != CL_KRNL_ARG_INT) {
              ConstantInt *access = dyn_cast<ConstantInt>(MDImgAccess->getOperand(i+1));
              
              curArg.size_in_bytes = (access->getValue().getZExtValue() == 0) ? 0 : 1;    // Set RW/WR flag
              break;
            }
          }
        }

        //test for structs
        llvm::Type *Ty = PTy->getContainedType(0);
        if ( true == Ty->isStructTy() ) // struct or struct*
        {
          if(PTy->getAddressSpace() == 0) //We're dealing with real struct and not struct pointer
          {
            llvm::StructType *STy = llvm::cast<llvm::StructType>(Ty);
            TargetData targetData(pModule);
            curArg.size_in_bytes = targetData.getTypeAllocSize(STy);
            curArg.type = CL_KRNL_ARG_COMPOSITE;
            break;
          }
        }

        switch (PTy->getAddressSpace())
        {
        case 0: case 1: // Global Address space
          curArg.type = CL_KRNL_ARG_PTR_GLOBAL;
          break;
        case 2:
          curArg.type = CL_KRNL_ARG_PTR_CONST;
          break;
        case 3: // Local Address space
          curArg.type = CL_KRNL_ARG_PTR_LOCAL;
          ++localMemCount;
          break;

        default:
          assert(0);
        }}
        break;

    case llvm::Type::IntegerTyID:
        {
          ConstantInt *access = dyn_cast<ConstantInt>(MDImgAccess->getOperand(i+1));
          if (access->getValue().getSExtValue() == -1) //sampler_t
          {
            curArg.type = CL_KRNL_ARG_SAMPLER;
            curArg.size_in_bytes = 0;
          }
          else
          {
            llvm::IntegerType *ITy = llvm::cast<llvm::IntegerType>(arg_it->getType());
            curArg.type = CL_KRNL_ARG_INT;
            curArg.size_in_bytes = ITy->getBitWidth()/8;
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
        curArg.size_in_bytes = (unsigned int)pVector->getNumElements();
        curArg.size_in_bytes |= (pVector->getContainedType(0)->getPrimitiveSizeInBits()/8)<<16;
      }
      break;

    default:
      assert(0 && "Unhelded parameter type");
    }
    arguments.push_back(curArg);
    ++arg_it;
  }

  if ( localMemCount > CPU_MAX_LOCAL_ARGS )
  {
      throw Exceptions::CompilerException("Too much local arguments count", CL_DEV_BUILD_ERROR);
  }
}

void CompilationUtils::getKernelsMetadata( Module* pModule, 
                                      const SmallVectorImpl<Function*>& pVectFunctions, 
                                      std::map<Function*, MDNode*>& /* OUT */ kernelMetadata) {

  NamedMDNode *pModuleMetadata = pModule->getNamedMetadata("opencl.kernels");

  unsigned int vecIndex = 0;
  for (unsigned i = 0, e = pModuleMetadata->getNumOperands(); i != e; ++i)
  {
    // Obtain kernel function from annotation
    MDNode* metadata = pModuleMetadata->getOperand(i);
    Function *pFunc = dyn_cast<Function>(metadata->getOperand(0)->stripPointerCasts());
    if ( NULL == pFunc )
    {
      continue;   // Not a function pointer
    }

    // Get appropriate vector function
    Function *vecFunc = ( (size_t)vecIndex < pVectFunctions.size() ) ? 
        pVectFunctions[vecIndex] : NULL;

    // Map scalar function
    kernelMetadata[pFunc] = metadata;
    // Map vetor function if exists
    if(vecFunc != NULL)
    {
      kernelMetadata[vecFunc] = metadata;
    }

    vecIndex++;
  }
}

}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {