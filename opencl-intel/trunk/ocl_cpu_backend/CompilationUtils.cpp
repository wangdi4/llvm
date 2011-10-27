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
                                              const std::string& args,
                                              std::vector<cl_kernel_argument>& /* OUT */ arguments) {
  // Check maximum number of arguments to kernel
  unsigned int count = 0;

  if(!args.empty())
  {
    std::string::size_type pos = 0;

    pos = args.find(", ", 0);
    while(pos != std::string::npos)
    {
      count++;
      pos = args.find(", ", pos+1);
      }
    count ++;
    }

  if ( (CPU_KERNEL_MAX_ARG_COUNT + NUMBER_IMPLICIT_ARGS) < count )
  {
    throw Exceptions::CompilerException(std::string("Too many arguments in kernel<") + pFunc->getName().str() + ">" , CL_DEV_BUILD_ERROR);
  }

  size_t argsCount = pFunc->getArgumentList().size() - NUMBER_IMPLICIT_ARGS;
  unsigned int localMemCount = 0;
  std::string remArgString(args);
  std::string currArgString;

  llvm::Function::arg_iterator arg_it = pFunc->arg_begin();
  for (unsigned i=0; i<argsCount; ++i)
  {
    cl_kernel_argument curArg;

    std::string::size_type pos = remArgString.find(", ", 0);
    if(pos == std::string::npos)
    {
        currArgString = remArgString;
        remArgString.clear();
    }
    else
    {
        currArgString = remArgString.substr(0, pos);
        remArgString = remArgString.substr(pos + 2);
    }

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
            const llvm::StructType *STy = llvm::cast<llvm::StructType>(arg_it->getType());
            curArg.type = CL_KRNL_ARG_COMPOSITE;
            TargetData targetData(pModule);
            curArg.size_in_bytes = targetData.getTypeAllocSize(STy);
            break;
        }
    case llvm::Type::PointerTyID:
      {
        const llvm::PointerType *PTy = llvm::cast<llvm::PointerType>(arg_it->getType());
        if ( pArg->hasByValAttr() && PTy->getElementType()->getTypeID() == llvm::Type::VectorTyID )
        {
          // Check by pointer vector passing, used in long16 and double16
          const llvm::VectorType *pVector = llvm::dyn_cast<llvm::VectorType>(PTy->getElementType());
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
        const std::string &imgArg = pFunc->getParent()->getTypeName(PTy->getElementType());
        int inx = imgArg.find("struct._image");
        if ( -1 != inx )    // Image identifier was found
        {
          // Get dimension of the image strlen("struct._image")
          char dim = imgArg.at(13);
          // Setup image pointer
          curArg.type = ('2' == dim ? CL_KRNL_ARG_PTR_IMG_2D : CL_KRNL_ARG_PTR_IMG_3D);
          curArg.size_in_bytes = (currArgString.find("__rd") != std::string::npos) ? 0 : 1;    // Set RW/WR flag
          break;
        }

        //test for structs
        const llvm::Type *Ty = PTy->getContainedType(0);
        if ( true == Ty->isStructTy() ) // struct or struct*
        {
          int inx = currArgString.find("*");
          if( -1 == inx ) //We're dealing with real struct and not struct pointer
          {
            const llvm::StructType *STy = llvm::cast<llvm::StructType>(Ty);
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
          //assert( ('2' == strArgs[i]) || ('1' == strArgs[i]) );
          break;
        case 2:
          curArg.type = CL_KRNL_ARG_PTR_CONST;
          //assert('8' == strArgs[i]);
          break;
        case 3: // Local Address space
          curArg.type = CL_KRNL_ARG_PTR_LOCAL;
          ++localMemCount;
          //assert('9' == strArgs[i]);
          break;

        default:
          assert(0);
        }}
        break;

    case llvm::Type::IntegerTyID:
        {
          if (currArgString.find("sampler_t") != std::string::npos)
          {
            curArg.type = CL_KRNL_ARG_SAMPLER;
            curArg.size_in_bytes = 0;
          }
#if (defined(_M_X64) || defined(__LP64__))
      // In llvm 2.7 & 2.8 there is a workaroung passing short2/ushort2 as kernel parameters  (we pass them as i64)
      // This is the conversion from i64 kernel parameter createwd bny clang instead of short2/ushort2 
      // to the actual short2/ushort2 parameter
          else if (currArgString.find("short2") != std::string::npos) // also works for ushort2
          {
            curArg.type = CL_KRNL_ARG_VECTOR;
            curArg.size_in_bytes = 2; // num elements
            curArg.size_in_bytes |= sizeof(short) << 16; // size of single element
          }
      // In llvm 2.7 & 2.8 there is a workaroung passing char4/uchar4 as kernel parameters  (we pass them as i64)
      // This is the conversion from i64 kernel parameter createwd bny clang instead of char4/uchar4 
      // to the actual char4/uchar4 parameter
          else if (currArgString.find("char4") != std::string::npos) // also works for uchar4
          {
            curArg.type = CL_KRNL_ARG_VECTOR;
            curArg.size_in_bytes = 4; // num elements
            curArg.size_in_bytes |= sizeof(char) << 16; // size of single element
          }
#endif /*(defined(_M_X64) || defined(__LP64__) */
          else
          {
            const llvm::IntegerType *ITy = llvm::cast<llvm::IntegerType>(arg_it->getType());
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
        const llvm::VectorType *pVector = llvm::dyn_cast<llvm::VectorType>(arg_it->getType());
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

  int vecIndex = 0;
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