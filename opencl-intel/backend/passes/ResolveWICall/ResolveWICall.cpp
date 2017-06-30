/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#define DEBUG_TYPE "resolve-wi-call"
#include "ResolveWICall.h"
#include "CompilationUtils.h"
#include "OCLAddressSpace.h"
#include "common_dev_limits.h"
#include "OCLPassSupport.h"
#include "CallbackDesc.h"

#include "llvm/IR/DataLayout.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include <algorithm>

using namespace Intel::OpenCL::DeviceBackend;

extern "C" {
  ModulePass* createResolveWICallPass() {
    return new intel::ResolveWICall();
  }
}

namespace intel {


  const unsigned int BYTE_SIZE = 8;
  char ResolveWICall::ID = 0;


  OCL_INITIALIZE_PASS(ResolveWICall, "resolve-wi-call",
    "Resolve OpenCL built-in calls to callbacks",
    false,
    false
    )

    static bool isEnqueueKernelFunctionType(unsigned FuncType){
      return FuncType == ICT_ENQUEUE_KERNEL_LOCALMEM ||
             FuncType == ICT_ENQUEUE_KERNEL_EVENTS_LOCALMEM;
    }
    static bool NeedsRuntimeHandleParam(unsigned FuncType) {
      return isEnqueueKernelFunctionType(FuncType) || FuncType == ICT_PRINTF;
    }

    bool ResolveWICall::runOnModule(Module &M) {
      m_pModule = &M;
      m_pLLVMContext = &M.getContext();
      m_IAA = &getAnalysis<ImplicitArgsAnalysis>();
      unsigned PointerSize = M.getDataLayout().getPointerSizeInBits(0);
      m_IAA->initDuringRun(PointerSize);
      m_sizeTTy = IntegerType::get(*m_pLLVMContext, PointerSize);

      m_bPrefetchDecl = false;
      m_pStructNDRangeType = NULL;

      // extended execution flags
      m_ExtExecDecls.clear();

      m_oclVersion = CompilationUtils::getCLVersionFromModuleOrDefault(M);
      m_nonUniformLocalSize = CompilationUtils::fetchCompilerOption(
        M, "-cl-uniform-work-group-size").empty() == false;

      // Run on all defined function in the module
      for ( Module::iterator fi = M.begin(), fe = M.end(); fi != fe; ++fi ) {
        Function *pFunc = dyn_cast<Function>(&*fi);
        if ( !pFunc || pFunc->isDeclaration () ) {
          // Function is not defined inside module
          continue;
        }

        if (pFunc->getName() == "__global_pipes_ctor")
          continue;

        clearPerFunctionCache();
        m_F = pFunc;
        runOnFunction(pFunc);
      }

      return true;
  }

  Function* ResolveWICall::runOnFunction(Function *pFunc) {
    // Getting the implicit arguments
    CompilationUtils::getImplicitArgs(pFunc, NULL, &m_pWorkInfo, &m_pWGId,
                                      &m_pBaseGlbId, &m_pSpecialBuf,
                                      &m_pRuntimeHandle);

    std::vector<Instruction*> toRemoveInstructions;
    std::vector<CallInst*> toHandleCalls;
    // Go through function instructions and search calls
    for ( inst_iterator ii = inst_begin(pFunc), ie = inst_end(pFunc); ii != ie; ++ii ) {

      CallInst *pCall = dyn_cast<CallInst>(&*ii);
      if ( !pCall ) {
        continue;
      }
      // Call instruction
      toHandleCalls.push_back(pCall);
    }
    SmallVector<Value*, 16> ExtExecArgs;
    for ( std::vector<CallInst*>::iterator ii = toHandleCalls.begin(),
      ie = toHandleCalls.end(); ii != ie; ++ii ) {
        ExtExecArgs.clear();

        CallInst *pCall = dyn_cast<CallInst>(*ii);
        assert(pCall && "Expected a CallInst");
        std::string calledFuncName = pCall->getCalledFunction()->getName().str();
        TInternalCallType calledFuncType = getCallFunctionType(calledFuncName);

        Value *pNewRes = NULL;
        switch ( calledFuncType ) {

        case ICT_GET_SPECIAL_BUFFER:
          pNewRes = m_pSpecialBuf;
          break;

        case ICT_GET_BASE_GLOBAL_ID:
        case ICT_GET_WORK_DIM:
        case ICT_GET_GLOBAL_SIZE:
        case ICT_GET_LOCAL_SIZE:
        case ICT_GET_ENQUEUED_LOCAL_SIZE:
        case ICT_GET_NUM_GROUPS:
        case ICT_GET_GROUP_ID:
        case ICT_GET_GLOBAL_OFFSET:
          // Recognize WI info functions
          pNewRes = updateGetFunction(pCall, calledFuncType);
          assert(pNewRes && "Expected updateGetFunction to succeed");
          break;
        case ICT_PRINTF:
          if (!m_ExtExecDecls.count(ICT_PRINTF))
            addExternFunctionDeclaration(
                calledFuncType, getOrCreatePrintfFuncType(), "opencl_printf");
          pNewRes = updatePrintf(pCall);
          assert(pNewRes && "Expected updatePrintf to succeed");
          break;
        case ICT_ENQUEUE_KERNEL_EVENTS_LOCALMEM:
        case ICT_ENQUEUE_KERNEL_LOCALMEM: {
          const VarArgsCallbackDesc *Desc =
              getVarArgsCallbackDesc(calledFuncType);
          if (!m_ExtExecDecls.count(calledFuncType)) {
            FunctionType *FT = 0;
            FT = getOrCreateEnqueueKernelFuncType(calledFuncType);
            addExternFunctionDeclaration(calledFuncType, FT,
                                         Desc->CallbackName);
          }
          // Copy all non-variadic call operands
          unsigned NonVariadicArgCount = Desc->NonVarArgCount;
          ExtExecArgs.append(pCall->op_begin(),
                             pCall->op_begin() + NonVariadicArgCount);
          addLocalMemArgs(ExtExecArgs, pCall, NonVariadicArgCount);
          // Add the RuntimeInterface arg
          ExtExecArgs.push_back(getOrCreateRuntimeInterface());
          // Add the Block2KernelMapper arg
          ExtExecArgs.push_back(getOrCreateBlock2KernelMapper());
          // Add the RuntimeHandle arg if needed
          ExtExecArgs.push_back(m_pRuntimeHandle);
          pNewRes =
              updateEnqueueKernelFunction(ExtExecArgs, Desc->CallbackName, pCall);
          assert(pNewRes && "ExtExecution. Expected non-NULL results");
        } break;
        case ICT_PREFETCH:
          addPrefetchDeclaration();
          // Substitute extern operand with function parameter
          updatePrefetch(pCall);
          // prefetch* function returns void, no need to replace its usages!
          break;
        default:
          continue;
        }

        if (pNewRes) {
          // Replace pCall usages with new calculation
          pCall->replaceAllUsesWith(pNewRes);
        }
        // Add pCall it to toRemoveInstruction container
        toRemoveInstructions.push_back(pCall);
    }

    //Remove all instructions in m_toRemoveInstructions
    for( std::vector<Instruction*>::iterator ii = toRemoveInstructions.begin(),
      ie = toRemoveInstructions.end(); ii != ie; ++ii ) {
        Instruction *pInst = dyn_cast<Instruction>(*ii);
        assert( pInst && "remove instruction container contains non instruction!" );
        pInst->eraseFromParent();
    }

    return pFunc;
  }

  Value* ResolveWICall::updateGetFunction(CallInst *pCall, TInternalCallType type) {
    assert(pCall && "Invalid CallInst");
    if (type == ICT_GET_WORK_DIM) {
      IRBuilder<> B(pCall);
      return m_IAA->GenerateGetFromWorkInfo(NDInfo::WORK_DIM, m_pWorkInfo, B);
    }
    BasicBlock *pBlock = pCall->getParent();
    Value *pResult = NULL;  // Object that holds the resolved value


    uint64_t overflowValue = 0;
    switch( type ) {
    case ICT_GET_BASE_GLOBAL_ID:
    case ICT_GET_GROUP_ID:
    case ICT_GET_GLOBAL_OFFSET:
      break;
    case ICT_GET_NUM_GROUPS:
    case ICT_GET_LOCAL_SIZE:
    case ICT_GET_ENQUEUED_LOCAL_SIZE:
    case ICT_GET_GLOBAL_SIZE:
      overflowValue = 1;
      break;
    default:
      assert( false && "Unhandled internal call type!" );
    }

    // check if the function argument is constant
    IntegerType *I32Ty = IntegerType::get(*m_pLLVMContext, 32);
    Constant *const_overflow = ConstantInt::get(pCall->getType(), overflowValue);
    ConstantInt *pVal = dyn_cast<ConstantInt>(pCall->getArgOperand(0));

    if ( NULL != pVal ) {
      // in case of constant argument we can check it "offline" if it's inbound
      unsigned int indexValue = (unsigned int)*pVal->getValue().getRawData();

      if ( indexValue >= MAX_WORK_DIM ) {
        // return overflow result (OCL SPEC requirement)
        return const_overflow;
      }
      return updateGetFunctionInBound(pCall, type, pCall);
    }

    // The indx isn't constant we should add inbound check "online"

    // Create three basic blocks to contain the dim check as follows
    // entry: (old basic block tail)
    //   %0 = icmp ult i32 %dimndx, MAX_WORK_DIM
    //   br i1 %0, label %get.wi.properties, label %split.continue
    //
    // get.wi.properties:  (new basic block in case of in bound)
    //   ... ; load the property
    //   br label %split.continue
    //
    // split.continue:  (the second half of the splitted basic block head)
    //   %4 = phi i32 [ %res, %get.wi.properties ], [ out-of-bound-value, %entry ]

    // first need to split the current basic block to two BB's and create new BB
    BasicBlock *getWIProperties = BasicBlock::Create(*m_pLLVMContext, "get.wi.properties", pBlock->getParent());
    BasicBlock *splitContinue = pBlock->splitBasicBlock(BasicBlock::iterator(pCall), "split.continue");

    // A.change the old basic block to the detailed entry
    // Entry:1. remove the unconditional jump instruction
    pBlock->getTerminator()->eraseFromParent();

    // Entry:2. add the entry tail code (as described up)
    ConstantInt *max_work_dim_i32 = ConstantInt::get(I32Ty, MAX_WORK_DIM);
    ICmpInst *checkIndex = new ICmpInst(ICmpInst::ICMP_ULT, pCall->getArgOperand(0), max_work_dim_i32, "check.index.inbound");
    pBlock->getInstList().push_back(checkIndex);
    BranchInst::Create(getWIProperties, splitContinue, checkIndex, pBlock);

    // B.Build the get.wi.properties block
    // Now retrieve address of the DIM count

    BranchInst::Create(splitContinue, getWIProperties);
    Instruction *pInsertBefore = getWIProperties->getTerminator();
    pResult = updateGetFunctionInBound(pCall, type, pInsertBefore);

    // C.Create Phi node at the first of the spiltted BB
    PHINode *pAttrResult = PHINode::Create(pCall->getType(), 2, "", splitContinue->getFirstNonPHI());
    pAttrResult->addIncoming(pResult, getWIProperties);
    pAttrResult->addIncoming(const_overflow, pBlock);

    return pAttrResult;
  }

  Value *ResolveWICall::updateGetFunctionInBound(CallInst *pCall, TInternalCallType type,
                                                 Instruction *pInsertBefore) {
    IRBuilder<> Builder(pInsertBefore);
    std::string Name;
    switch (type) {
    case ICT_GET_GLOBAL_OFFSET:
    case ICT_GET_GLOBAL_SIZE:
    case ICT_GET_NUM_GROUPS:
      return m_IAA->GenerateGetFromWorkInfo(InternalCall2NDInfo(type),
                                            m_pWorkInfo,
                                            pCall->getArgOperand(0), Builder);
    case ICT_GET_LOCAL_SIZE:
      return m_IAA->GenerateGetLocalSize(m_nonUniformLocalSize,
                                         m_pWorkInfo, m_pWGId, pCall->getArgOperand(0),
                                         Builder);
    case ICT_GET_ENQUEUED_LOCAL_SIZE:
      return m_IAA->GenerateGetEnqueuedLocalSize(m_pWorkInfo, pCall->getArgOperand(0),
                                                 Builder);
    case ICT_GET_BASE_GLOBAL_ID:
      return m_IAA->GenerateGetBaseGlobalID(m_pBaseGlbId,
                                            pCall->getArgOperand(0), Builder);
    case ICT_GET_GROUP_ID:
      return m_IAA->GenerateGetGroupID(m_pWGId, pCall->getArgOperand(0),
                                       Builder);
    default:
      break;
    }
    assert(false && "Unexpected ID function");
    return 0;
  }

  Value* ResolveWICall::updatePrintf(CallInst *pCall) {

    assert( m_pRuntimeHandle && "Context pointer m_pRuntimeHandle created as expected" );
    const DataLayout &DL = m_pModule->getDataLayout();

    // Find out the buffer size required to store all the arguments.
    // Note: CallInst->getNumOperands() returns the number of operands in
    // the instruction, including its destination as #0. Since this is
    // a printf call and we're interested in all the arguments after the
    // format string, we start with #2.
    //
    assert(pCall->getNumArgOperands() > 0 && "Expect printf to have a format string");
    unsigned total_arg_size = 0;
    for ( unsigned numarg = 1; numarg < pCall->getNumArgOperands(); ++numarg ) {
      Value *arg = pCall->getArgOperand(numarg);
      unsigned argsize = DL.getTypeAllocSize(arg->getType());
      total_arg_size += argsize;
    }

    // Types used in several places
    //
    IntegerType *int32_type = IntegerType::get(*m_pLLVMContext, 32);
    IntegerType *int8_type = IntegerType::get(*m_pLLVMContext, 8);

    // Create the alloca instruction for allocating the buffer on the stack.
    // Also, handle the special case where printf got no vararg arguments:
    // printf("hello");
    // Since we have to pass something into the 'args' argument of
    // opencl_printf, and 'alloca' with size 0 is undefined behavior, we
    // just allocate a dummy buffer of size 1. opencl_printf won't look at
    // it anyway.
    //
    ArrayType *buf_arr_type;
    if ( pCall->getNumArgOperands() == 1 ) {
      buf_arr_type = ArrayType::get(int8_type, 1);
    } else {
      buf_arr_type = ArrayType::get(int8_type, total_arg_size);
    }
    // TODO: add comment
    AllocaInst *buf_alloca_inst = new AllocaInst(buf_arr_type, "temp_arg_buf",
      &*pCall->getParent()->getParent()->getEntryBlock().begin());

    // Generate instructions to store the operands into the argument buffer
    //
    unsigned buf_pointer_offset = 0;
    for ( unsigned numarg = 1; numarg < pCall->getNumArgOperands(); ++numarg ) {
      std::vector<Value*> index_args;
      index_args.push_back(getConstZeroInt32Value());
      index_args.push_back(ConstantInt::get(int32_type, buf_pointer_offset));

      // getelementptr to compute the address into which this argument will
      // be placed
      //
      GetElementPtrInst *gep_instr = GetElementPtrInst::CreateInBounds(
        buf_alloca_inst, ArrayRef<Value*>(index_args), "", pCall);

      Value *arg = pCall->getArgOperand(numarg);
      Type *argtype = arg->getType();

      // bitcast from generic i8* address to a pointer to the argument's type
      //
      CastInst *cast_instr = CastInst::CreatePointerCast(gep_instr, PointerType::getUnqual(argtype), "", pCall);

      // store argument into address. Alignment forced to 1 to make vector
      // stores safe.
      //
      (void) new StoreInst(arg, cast_instr, false, 1, pCall);

      // This argument occupied some space in the buffer.
      // Advance the buffer pointer offset by its size to know where the next
      // argument should be placed.
      //
      unsigned argsize = DL.getTypeAllocSize(arg->getType());
      buf_pointer_offset += argsize;
    }

    // Create a pointer to the buffer, in order to pass it to the function
    //
    std::vector<Value*> index_args;
    index_args.push_back(getConstZeroInt32Value());
    index_args.push_back(getConstZeroInt32Value());

    GetElementPtrInst *ptr_to_buf = GetElementPtrInst::CreateInBounds(
      buf_alloca_inst, ArrayRef<Value*>(index_args), "", pCall);

    // Finally create the call to opencl_printf
    //
    Function *pFunc = m_pModule->getFunction("opencl_printf");
    assert(pFunc && "Expect builtin printf to be declared before use");

    SmallVector<Value*, 16> params;
    params.push_back(pCall->getArgOperand(0));
    params.push_back(ptr_to_buf);
    Value *RuntimeInterface = getOrCreateRuntimeInterface();
    params.push_back(RuntimeInterface);
    params.push_back(m_pRuntimeHandle);
    CallInst *res = CallInst::Create(pFunc, params, "translated_opencl_printf_call", pCall);
    res->setDebugLoc(pCall->getDebugLoc());
    return res;
  }

  void ResolveWICall::updatePrefetch(llvm::CallInst *pCall) {

    DataLayout const& DL = m_pModule->getDataLayout();

    unsigned int uiSizeT = DL.getPointerSizeInBits(0);

    // Create new call instruction with extended parameters
    SmallVector<Value*, 4> params;
    // push original parameters
    // Need bitcast to a general pointer
    CastInst *pBCPtr = CastInst::CreatePointerCast(pCall->getArgOperand(0),
      PointerType::get(IntegerType::get(*m_pLLVMContext, 8), 0), "", pCall);
    params.push_back(pBCPtr);
    // Put number of elements
    params.push_back(pCall->getArgOperand(1));
    // Distinguish element size
    PointerType *pPTy = dyn_cast<PointerType>(pCall->getArgOperand(0)->getType());
    assert(pPTy && "Must be a pointer");
    Type *pPT = pPTy->getElementType();

    assert(pPT->getPrimitiveSizeInBits() && "Not primitive type, not valid calculation");
    unsigned int uiSize = DL.getPrefTypeAlignment(pPT);

    params.push_back(ConstantInt::get(IntegerType::get(*m_pLLVMContext, uiSizeT), uiSize));
    Function *pPrefetch = m_pModule->getFunction("lprefetch");
    CallInst::Create(pPrefetch, ArrayRef<Value*>(params), "", pCall);
  }

  FunctionType* ResolveWICall::getOrCreatePrintfFuncType() {
    // The prototype of opencl_printf is:
    // int opencl_printf(__constant char *format, char *args, void *pCallback, void *pRuntimeHandle)
    //
    std::vector<Type*> params;
    // The 'format' string is in constant address space (address space 2)
    params.push_back(PointerType::get(IntegerType::get(*m_pLLVMContext, 8), 2));
    params.push_back(PointerType::get(IntegerType::get(*m_pLLVMContext, 8), 0));
    params.push_back(m_IAA->getWorkGroupInfoMemberType(NDInfo::RUNTIME_INTERFACE));
    params.push_back(m_pRuntimeHandle->getType());

    return FunctionType::get(Type::getInt32Ty(*m_pLLVMContext), params, false);
  }

  void ResolveWICall::addPrefetchDeclaration() {
    if ( m_bPrefetchDecl ) {
      // Prefetch declaration already added
      return;
    }

    unsigned int uiSizeT = m_pModule->getDataLayout().getPointerSizeInBits(0);

    std::vector<Type*> params;
    // Source Pointer
    params.push_back(PointerType::get(IntegerType::get(*m_pLLVMContext, 8), 0));
    // Number of elements
    params.push_back(IntegerType::get(*m_pLLVMContext, uiSizeT));
    // Element size
    params.push_back(IntegerType::get(*m_pLLVMContext, uiSizeT));
    FunctionType *pNewType = FunctionType::get(Type::getVoidTy(*m_pLLVMContext), params, false);
    Function::Create(pNewType, Function::ExternalLinkage, "lprefetch", m_pModule);

    m_bPrefetchDecl = true;
  }

  TInternalCallType ResolveWICall::getCallFunctionType(std::string calledFuncName) {

    if( calledFuncName == CompilationUtils::NAME_GET_BASE_GID ) {
      return ICT_GET_BASE_GLOBAL_ID;
    }
    if( calledFuncName == CompilationUtils::NAME_GET_SPECIAL_BUFFER ) {
      return ICT_GET_SPECIAL_BUFFER;
    }
    if( CompilationUtils::isGetWorkDim(calledFuncName) )
      return ICT_GET_WORK_DIM;
    if(CompilationUtils::isGetGlobalSize(calledFuncName))
      return ICT_GET_GLOBAL_SIZE;
    if(CompilationUtils::isGetNumGroups(calledFuncName))
      return ICT_GET_NUM_GROUPS;
    if(CompilationUtils::isGetGroupId(calledFuncName))
      return ICT_GET_GROUP_ID;
    if(CompilationUtils::isGlobalOffset(calledFuncName))
      return ICT_GET_GLOBAL_OFFSET;
    if(calledFuncName == CompilationUtils::NAME_PRINTF)
      return ICT_PRINTF;
    if(CompilationUtils::isPrefetch(calledFuncName))
      return ICT_PREFETCH;

    // OpenCL2.0 built-ins to resolve
    if (m_oclVersion == OclVersion::CL_VER_2_0) {
      if( CompilationUtils::isEnqueueKernelLocalMem(calledFuncName))
        return ICT_ENQUEUE_KERNEL_LOCALMEM;
      if( CompilationUtils::isEnqueueKernelEventsLocalMem(calledFuncName))
        return ICT_ENQUEUE_KERNEL_EVENTS_LOCALMEM;
      if(CompilationUtils::isGetLocalSize(calledFuncName))
        return ICT_GET_LOCAL_SIZE;
      if(CompilationUtils::isGetEnqueuedLocalSize(calledFuncName))
        return ICT_GET_ENQUEUED_LOCAL_SIZE;
    } else {
      // built-ins which behavior is different in OpenCL versions older than 2.0
      if(CompilationUtils::isGetLocalSize(calledFuncName))
        return ICT_GET_ENQUEUED_LOCAL_SIZE;
    }
    return ICT_NONE;
  }
  // address space generated by clang for queue_t, clk_event_t, ndrange_t types
  const int EXTEXEC_OPAQUE_TYPES_ADDRESS_SPACE = Utils::OCLAddressSpace::Global;
  Type * ResolveWICall::getQueueType() const {
    return PointerType::getInt8PtrTy(*m_pLLVMContext, EXTEXEC_OPAQUE_TYPES_ADDRESS_SPACE);
  }

  Type * ResolveWICall::getClkEventType() const {
    return PointerType::getInt8PtrTy(*m_pLLVMContext, EXTEXEC_OPAQUE_TYPES_ADDRESS_SPACE);
  }

  Type * ResolveWICall::getKernelEnqueueFlagsType() const {
    return IntegerType::get(*m_pLLVMContext, 32);
  }

  Type * ResolveWICall::getNDRangeType() const {
    return PointerType::getInt8PtrTy(*m_pLLVMContext, EXTEXEC_OPAQUE_TYPES_ADDRESS_SPACE);
  }

  Type * ResolveWICall::getBlockLocalMemType() const {
    // void (^block)(local void *, ?), - OpenCL
    // void (i8 addrspace(3)*, ...)*  - LLVM representation
    return PointerType::get(FunctionType::get(Type::getVoidTy(*m_pLLVMContext),
      PointerType::get(Type::getInt8Ty(*m_pLLVMContext), Utils::OCLAddressSpace::Local),
      true), 0);
  }

  Type * ResolveWICall::getEnqueueKernelRetType() const {
    return IntegerType::get(*m_pLLVMContext, ENQUEUE_KERNEL_RETURN_BITS);
  }

  ConstantInt * ResolveWICall::getConstZeroInt32Value() const {
    return ConstantInt::get(Type::getInt32Ty(*m_pLLVMContext), 0);
  }

  Type* ResolveWICall::getLocalMemBufType() const {
    return IntegerType::get(*m_pLLVMContext, 32);
  }

  void ResolveWICall::addLocalMemArgs(SmallVectorImpl<Value *> &args,
                                      CallInst *pCall,
                                      const unsigned LocalMemArgsOffs) {

      assert(pCall->getNumArgOperands() > LocalMemArgsOffs && "Expect enqueue_kernel to have local mem size arguments");
      // get number of local buffers
      const unsigned numLocalBuffers = pCall->getNumArgOperands() - LocalMemArgsOffs;

      // array with local mem sizes
      ArrayType *localbuf_arr_type = ArrayType::get(getLocalMemBufType(), numLocalBuffers);
      // issue alloca for array
      AllocaInst *localbuf_alloca_inst = new AllocaInst(localbuf_arr_type, "localmem_arg_buf",
        &*pCall->getParent()->getParent()->getEntryBlock().begin());
      // helper int type
      Type* int32_type = IntegerType::get(*m_pLLVMContext, 32);
      // parse argument with local buf sizes
      // fill in array with local buf sizes
      for(unsigned numarg=LocalMemArgsOffs, cnt=0;
        numarg < pCall->getNumArgOperands();
        ++numarg, ++cnt) {
          llvm::SmallVector<Value*, 2> index_args;
          index_args.push_back(getConstZeroInt32Value());
          assert(getLocalMemBufType() == int32_type && "Localmem size type assumed to be int");
          index_args.push_back(ConstantInt::get(getLocalMemBufType(), cnt));
          // issue GEP
          GetElementPtrInst *gep_instr = GetElementPtrInst::CreateInBounds(
            localbuf_alloca_inst, ArrayRef<Value*>(index_args), "", pCall);

          // code below handles incorrect types of local buffer size argument in enqueue_kernel()
          // TODO: revisit and remove code when following issue is fixed in clang
          // CSSD100018602 elior [OpenCL 2.0] Clang should perform type check of variadic argument of enqueue_kernel built-in
          //
          Value *bufSize = pCall->getArgOperand(numarg);
          Type *bufSizeTy = bufSize->getType();
          assert(bufSizeTy->isIntegerTy() && "enqueue_kernel():: local buffer size argument expected to be uint type");
          // replace noninteger with zero
          if(!bufSizeTy->isIntegerTy()) {
            errs() << "WARNING: " <<
              "enqueue_kernel():: local buffer size argument expected to be uint type" <<
              "setting argument " << bufSize << " to zero\n";
            bufSize = ConstantInt::get(Type::getInt32Ty(m_pModule->getContext()), APInt(32, 0));
          }

          // If bufsize scalar size is not 32 then Zext or Trunc to get to 32
          if (bufSizeTy->getScalarSizeInBits() < getLocalMemBufType()->getScalarSizeInBits()) {
            bufSize = new ZExtInst(pCall->getArgOperand(numarg), getLocalMemBufType(), "", pCall);
          }
          else if (bufSizeTy->getScalarSizeInBits() > getLocalMemBufType()->getScalarSizeInBits()) {
            bufSize = new TruncInst(pCall->getArgOperand(numarg), getLocalMemBufType(), "", pCall);
          }
          /////////////////////////////////////////////////////////////////////

          // issue store of current local buf size
          (void) new StoreInst(bufSize, gep_instr, false, 1, pCall);
      }

      // get pointer to first element in array
      llvm::SmallVector<Value*, 2> index_args;
      index_args.push_back(getConstZeroInt32Value());
      index_args.push_back(getConstZeroInt32Value());
      GetElementPtrInst *ptr_to_localmem_buf =
          GetElementPtrInst::CreateInBounds(localbuf_alloca_inst, index_args,
                                            "", pCall);

      // add argument uint *localbuf_size
      args.push_back(ptr_to_localmem_buf);
      // add argument uint localbuf_size_len
      args.push_back(ConstantInt::get(int32_type, numLocalBuffers));
  }

  static bool isPointerToStructType(Type *Ty) {
      // check it is pointer
      if(!Ty->isPointerTy())
          return false;

      Type * PtrTy = cast<PointerType>(Ty)->getElementType();
      // pointer type is struct
      if (!PtrTy->isStructTy())
          return false;
      return true;
  }

  Value *
  ResolveWICall::updateEnqueueKernelFunction(SmallVectorImpl<Value *> &NewParams,
                                       const StringRef FunctionName,
                                       CallInst *pCall) {
  // bitcast types from built-in argument type to type of callback function's
  // argument
  // handles scenario when types like ndrange_t.3, clk_event.2, queue_t.5, etc
  // are generated by ParseBitcodeFile
  // Appending .#number to type name happens when the LLVM context is reused by
  // BE for loading bytecode
  // we handle here ONLY pointer to struct and double pointer to struct
  Function *cbkF = m_pModule->getFunction(FunctionName);
  Function::arg_iterator AI = cbkF->arg_begin();
  for (SmallVectorImpl<Value *>::iterator it = NewParams.begin(), E = NewParams.end();
       it != E; ++it, ++AI) {
    Value *&NewParam = *it;
    Type *NewParamTy = NewParam->getType();
    Type* ExpectedArgTy = AI->getType();
    // check types are equal - no bitcast needed
    if (NewParamTy == AI->getType())
      continue;
    // check it is pointer
    if (!NewParamTy->isPointerTy()) {
      assert(0 && "Should not get here. Unsupported type of argument");
      continue;
    }
    Type *PtrTy = cast<PointerType>(NewParamTy)->getElementType();
    // pointer type is struct
    if (PtrTy->isStructTy()) {
      *it = CastInst::CreatePointerCast(NewParam, ExpectedArgTy, "",
                             pCall);
      continue;
    }
    // check pointer is to pointer
    if (!PtrTy->isPointerTy()) {
      assert(0 && "Should not get here. Unsupported type of argument");
      continue;
    }
    // double pointer points to structure
    Type *PPtrTy = cast<PointerType>(PtrTy)->getElementType();
    if (PPtrTy->isStructTy()) {
      NewParam = CastInst::CreatePointerCast(NewParam, ExpectedArgTy,
                                  "", pCall);
      continue;
    }
    assert(0 && "Should not get here. Unsupported type of argument");
  }
  CallInst *CI =
      CallInst::Create(m_pModule->getFunction(FunctionName), NewParams, "", pCall);

  // if return value type does not match return type
  // bitcast to return type
  Value *ret = CI;
  if (pCall->getType() != CI->getType()) {
    if (isPointerToStructType(pCall->getType()))
      ret = CastInst::CreatePointerCast(CI, pCall->getType(), "",
                             pCall);
    else
      assert(0 && "Should not get here. Unsupported type of return value");
  }
  return ret;
}

void ResolveWICall::clearPerFunctionCache() {
  m_F = 0;
  m_RuntimeInterface = 0;
  m_Block2KernelMapper = 0;
}

Value *ResolveWICall::getOrCreateBlock2KernelMapper() {
  IRBuilder<> Builder(&*m_F->getEntryBlock().begin());
  if (!m_Block2KernelMapper)
    m_Block2KernelMapper = m_IAA->GenerateGetFromWorkInfo(
        NDInfo::BLOCK2KERNEL_MAPPER, m_pWorkInfo, Builder);
  return m_Block2KernelMapper;
}

Value *ResolveWICall::getOrCreateRuntimeInterface() {
  IRBuilder<> Builder(&*m_F->getEntryBlock().begin());
  if (!m_RuntimeInterface)
    m_RuntimeInterface = m_IAA->GenerateGetFromWorkInfo(
        NDInfo::RUNTIME_INTERFACE, m_pWorkInfo, Builder);
  return m_RuntimeInterface;
}

  // The prototype of ocl20_enqueue_kernel_events is:
  // int ocl20_enqueue_kernel_events_localmem(queue_t*, int /*kernel_enqueue_flags_t*/,
  //            ndrange_t,
  //            uint num_events_in_wait_list, clk_event_t *in_wait_list,
  //            clk_event_t *event_ret,
  //            void * (local void*) /*block_literal ptr*/,
  //            uint *localbuf_size, uint localbuf_size_len,
  //            ExtendedExecutionContext * pEEC)
  //
  // This function creates LLVM types for ALL 4 above enqueue_kernel callbacks
  FunctionType *ResolveWICall::getOrCreateEnqueueKernelFuncType(unsigned type) {
    assert(type == ICT_ENQUEUE_KERNEL_LOCALMEM ||
           type == ICT_ENQUEUE_KERNEL_EVENTS_LOCALMEM);
    SmallVector<Type *, 16> params;
    // queue_t
    params.push_back(getQueueType());
    // int /*kernel_enqueue_flags_t*/
    params.push_back(getKernelEnqueueFlagsType());
    // ndrange_t
    params.push_back(getNDRangeType());
    // events
    if (type == ICT_ENQUEUE_KERNEL_EVENTS_LOCALMEM) {
      // uint num_events_in_wait_list
      params.push_back(IntegerType::get(*m_pLLVMContext, 32));
      // clk_event_t *in_wait_list
      params.push_back(PointerType::get(getClkEventType(), 0));
      // clk_event_t *event_ret
      params.push_back(PointerType::get(getClkEventType(), 0));
    }
    // void * /*block_literal ptr*/
    params.push_back(getBlockLocalMemType());

    // local memory
    // uint * localbuf_size
    params.push_back(PointerType::get(getLocalMemBufType(), 0));
    // uint localbuf_size_len
    params.push_back(IntegerType::get(*m_pLLVMContext, 32));
    params.push_back(
        m_IAA->getWorkGroupInfoMemberType(NDInfo::RUNTIME_INTERFACE));
    params.push_back(
        m_IAA->getWorkGroupInfoMemberType(NDInfo::BLOCK2KERNEL_MAPPER));
    params.push_back(m_pRuntimeHandle->getType());
    // create function type
    return FunctionType::get(getEnqueueKernelRetType(), params, false);
  }

  void ResolveWICall::addExternFunctionDeclaration(unsigned type,
                                                   FunctionType *FT,
                                                   StringRef Name) {
    // check declaration exists
    if(m_ExtExecDecls.find(type) != m_ExtExecDecls.end())
      return;
    // create declaration
    Function::Create(
      FT, // function type
      Function::ExternalLinkage,
      Name, // function name
      m_pModule);
    // mark declaration is done
    m_ExtExecDecls.insert(type);
  }
  unsigned ResolveWICall::getPointerSize() const {
    unsigned pointerSizeInBits = m_pModule->getDataLayout().getPointerSizeInBits(0);
    assert((32 == pointerSizeInBits  || 64 == pointerSizeInBits) &&
           "Unsopported pointer size");
    return pointerSizeInBits;
  }
} // namespace intel
