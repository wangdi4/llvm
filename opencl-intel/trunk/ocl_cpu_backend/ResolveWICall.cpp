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

File Name:  ResolveWICall.cpp

\*****************************************************************************/

#include "ResolveWICall.h"
#include "CompilationUtils.h"

#include "cl_device_api.h"

#include "llvm/Support/InstIterator.h"
#include "llvm/Target/TargetData.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

  char ResolveWICall::ID = 0;

  ModulePass* createResolveWICallPass() {
    return new ResolveWICall();
  }

  bool ResolveWICall::runOnModule(Module &M) {
    m_pModule = &M;
    m_pLLVMContext = &M.getContext();

    m_bAsyncCopyDecl = false;
    m_bPrefetchDecl = false;
    m_bPrintfDecl = false;

    // Run on all defined function in the module
    for ( Module::iterator fi = M.begin(), fe = M.end(); fi != fe; ++fi ) {
      Function *pFunc = dyn_cast<Function>(&*fi);
      if ( !pFunc || pFunc->isDeclaration () ) {
        // Function is not defined inside module
        continue;
      }
      runOnFunction(pFunc);
    }

    return true;
  }

  Function* ResolveWICall::runOnFunction(Function *pFunc) {

    // Getting the implicit arguments
    CompilationUtils::getImplicitArgs(pFunc, NULL, &m_pWorkInfo, &m_pWGId,
        &m_pBaseGlbId, &m_pLocalId, &m_pIterCount, &m_pSpecialBuf, &m_pCurrWI, &m_pCtx);

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
    for ( std::vector<CallInst*>::iterator ii = toHandleCalls.begin(),
      ie = toHandleCalls.end(); ii != ie; ++ii ) {
    
      CallInst *pCall = dyn_cast<CallInst>(*ii);
      std::string calledFuncName = pCall->getCalledFunction()->getNameStr();
      TInternalCallType calledFuncType = getCallFunctionType(calledFuncName);

      Value *pNewRes = NULL;
      switch ( calledFuncType ) {

      case ICT_GET_ITER_COUNT:
        pNewRes = m_pIterCount;
        break;

      case ICT_GET_SPECIAL_BUFFER:
        pNewRes = m_pSpecialBuf;
        break;

      case ICT_GET_CURR_WI:
        pNewRes = m_pCurrWI;
        break;

      case ICT_GET_LOCAL_ID:
      case ICT_GET_GLOBAL_ID:
      case ICT_GET_WORK_DIM:
      case ICT_GET_GLOBAL_SIZE:
      case ICT_GET_LOCAL_SIZE:
      case ICT_GET_NUM_GROUPS:
      case ICT_GET_GROUP_ID:
      case ICT_GET_GLOBAL_OFFSET:
        // Recognize WI info functions
        pNewRes = updateGetFunction(pCall, calledFuncType);
        assert(pNewRes && "Expected updateGetFunction to succeed");
        break;

      case ICT_PRINTF:
        addPrintfDeclaration();
        pNewRes = updatePrintf(pCall);
        assert(pNewRes && "Expected updatePrintf to succeed");
        break;

      case ICT_ASYNC_WORK_GROUP_COPY:
        addAsyncCopyDeclaration();
        // Substitute extern operand with function parameter
        pNewRes = updateAsyncCopy(pCall, false);
        assert(pNewRes && "Expected updateAsyncCopy to succeed");
        break;

      case ICT_ASYNC_WORK_GROUP_STRIDED:
        addAsyncCopyDeclaration();
        // Substitute extern operand with function parameter
        pNewRes = updateAsyncCopy(pCall, true);
        assert(pNewRes && "Expected updateAsyncCopy to succeed");
        break;

      case ICT_WAIT_GROUP_EVENTS:
        addAsyncCopyDeclaration();
        // Substitute extern operand with function parameter
        updateWaitGroup(pCall);
        // Wait* function returns void, no need to replace its usages!
        break;

      case ICT_PREFETCH:
        addPrefetchDeclaration();
        // Substitute extern operand with function parameter
        updatePrefetch(pCall);
        // prefetch* function returns void, no need to replace its usages!
        break;

      default:
        continue;
      }

      if ( pNewRes ) {
        // Replace pCall usages with new calculation
        pCall->uncheckedReplaceAllUsesWith(pNewRes);
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
    BasicBlock *pBlock = pCall->getParent();
    Value *pResult = NULL;  // Object that holds the resolved value

    if ( type == ICT_GET_WORK_DIM ) {
      // Now retrieve address of the DIM count
      SmallVector<Value*, 4> params;
      params.push_back(ConstantInt::get(IntegerType::get(*m_pLLVMContext, 32), 0));
      params.push_back(ConstantInt::get(IntegerType::get(*m_pLLVMContext, 32), 0));
      GetElementPtrInst *pDimCntAddr =
        GetElementPtrInst::Create(m_pWorkInfo, params.begin(), params.end(), "", pCall);
      // Load the Value
      pResult = new LoadInst(pDimCntAddr, "", pCall);
      return pResult;
    }

    std::string overflowValueString = "0";
    switch( type ) {
    case ICT_GET_LOCAL_ID:
    case ICT_GET_GLOBAL_ID:
    case ICT_GET_GROUP_ID:
    case ICT_GET_GLOBAL_OFFSET:
      break;
    case ICT_GET_NUM_GROUPS:
    case ICT_GET_LOCAL_SIZE:
    case ICT_GET_GLOBAL_SIZE:
      overflowValueString = "1";
      break;
    default:
      assert( false && "Unhandled internal call type!" );
    }

    // check if the function argument is constant
    ConstantInt *pVal = dyn_cast<ConstantInt>(pCall->getArgOperand(0));

    if ( NULL != pVal ) {
      // in case of constant argument we can check it "offline" if it's inbound
      unsigned int indexValue = (unsigned int)*pVal->getValue().getRawData();

      if ( indexValue >= MAX_WORK_DIM ) {
        // return overflow result (OCL SPEC requirement)
        return ConstantInt::get(*m_pLLVMContext,
          APInt(sizeof(size_t) * BYTE_SIZE, StringRef(overflowValueString), 10));
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
    ConstantInt *max_work_dim_i32 = ConstantInt::get(*m_pLLVMContext, APInt(32, MAX_WORK_DIM, false));
    ICmpInst *checkIndex = new ICmpInst(ICmpInst::ICMP_ULT, pCall->getArgOperand(0), max_work_dim_i32, "check.index.inbound");
    pBlock->getInstList().push_back(checkIndex);
    BranchInst::Create(getWIProperties, splitContinue, checkIndex, pBlock);

    // B.Build the get.wi.properties block
    // Now retrieve address of the DIM count

    BranchInst::Create(splitContinue, getWIProperties);
    Instruction *pInsertBefore = getWIProperties->getTerminator();
    pResult = updateGetFunctionInBound(pCall, type, pInsertBefore);

    // C.Create Phi node at the first of the spiltted BB
    ConstantInt *const_overflow = ConstantInt::get(*m_pLLVMContext, APInt(sizeof(size_t) * BYTE_SIZE, StringRef(overflowValueString), 10));
    PHINode *pAttrResult = PHINode::Create(IntegerType::get(*m_pLLVMContext, sizeof(size_t) * BYTE_SIZE), "", splitContinue->getFirstNonPHI());
    pAttrResult->reserveOperandSpace(2);
    pAttrResult->addIncoming(pResult, getWIProperties);
    pAttrResult->addIncoming(const_overflow, pBlock);

    return pAttrResult;
  }

  Value* ResolveWICall::updateGetFunctionInBound(
    CallInst *pCall, TInternalCallType type, Instruction *pInsertBefore) {

    int iTableInx = 0;
    switch ( type ) {
    case ICT_GET_GLOBAL_OFFSET:
      iTableInx = 1; break;
    case ICT_GET_GLOBAL_SIZE:
      iTableInx = 2; break;
    case ICT_GET_LOCAL_SIZE:
      iTableInx = 3; break;
    case ICT_GET_NUM_GROUPS:
      iTableInx = 4; break;
    default:
      // This to solve compilation warning!
      iTableInx = 0;
    }

    if ( iTableInx > 0 || type == ICT_GET_LOCAL_ID ) {
      Argument *structure = (iTableInx > 0) ? m_pWorkInfo : m_pLocalId;
      SmallVector<Value*, 4> params;
      params.push_back( (iTableInx > 0) ?
        ConstantInt::get(IntegerType::get(*m_pLLVMContext, 32), 0) :
        pCall->getArgOperand(1) );
      params.push_back(ConstantInt::get(IntegerType::get(*m_pLLVMContext, 32), iTableInx));
      params.push_back(pCall->getArgOperand(0));

      GetElementPtrInst *pAddr =
        GetElementPtrInst::Create(structure, params.begin(), params.end(), "", pInsertBefore);

      // Load the Value
      return new LoadInst(pAddr, "", pInsertBefore);
    }
    if ( type == ICT_GET_GROUP_ID ) {
      GetElementPtrInst *pIdAddr = 
        GetElementPtrInst::Create(m_pWGId, pCall->getArgOperand(0), "", pInsertBefore);
      // Load the Value
      return new LoadInst(pIdAddr, "", pInsertBefore);
    }
    if ( type == ICT_GET_GLOBAL_ID ) {
      return calcGlobalId(pCall, pInsertBefore);
    }
    assert( false && "Unhandled internal call type!" );
    return NULL; // This to avoid compilation warning
  }

  Value* ResolveWICall::calcGlobalId(CallInst *pCall, Instruction *pInsertBefore) {

    SmallVector<Value*, 4> params;
    // Load local id values
    params.push_back(pCall->getArgOperand(1));
    params.push_back(ConstantInt::get(IntegerType::get(*m_pLLVMContext, 32), 0));
    params.push_back(pCall->getArgOperand(0));

    // Calculate the address of local id value
    GetElementPtrInst *pLclIdAddr =
      GetElementPtrInst::Create(m_pLocalId, params.begin(), params.end(), "", pInsertBefore);
    // Load the value of local id
    Value *pLocalIdVal = new LoadInst(pLclIdAddr, "", pInsertBefore);

    params.clear();
    // base global values
    params.push_back(ConstantInt::get(IntegerType::get(*m_pLLVMContext, 32), 0));
    params.push_back(ConstantInt::get(IntegerType::get(*m_pLLVMContext, 32), 0));
    params.push_back(pCall->getArgOperand(0));

    // Calculate the address of base global value
    GetElementPtrInst *pGlbBaseAddr =
      GetElementPtrInst::Create(m_pBaseGlbId, params.begin(), params.end(), "", pInsertBefore);
    // Load the value of base global
    Value *pBaseGlbIdVal = new LoadInst(pGlbBaseAddr, "", pInsertBefore);

    // Now add these two values
    Value *pGlbId = BinaryOperator::CreateAdd( pLocalIdVal, pBaseGlbIdVal, "", pInsertBefore);

    return pGlbId;
  }

  Value* ResolveWICall::updatePrintf(CallInst *pCall) {

    assert( m_pCtx && "Context pointer m_pCtx created as expected" );

    TargetData TD(m_pModule);
    
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
      unsigned argsize = TD.getTypeSizeInBits(arg->getType()) / 8;
      total_arg_size += argsize;
    }

    // Types used in several places
    //
    const IntegerType *int32_type = IntegerType::get(*m_pLLVMContext, 32);
    const IntegerType *int8_type = IntegerType::get(*m_pLLVMContext, 8);

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
      pCall->getParent()->getParent()->getEntryBlock().begin());

    // Generate instructions to store the operands into the argument buffer
    //
    unsigned buf_pointer_offset = 0;
    for ( unsigned numarg = 1; numarg < pCall->getNumArgOperands(); ++numarg ) {
      std::vector<Value*> index_args;
      index_args.push_back(ConstantInt::get(int32_type, 0));
      index_args.push_back(ConstantInt::get(int32_type, buf_pointer_offset));

      // getelementptr to compute the address into which this argument will 
      // be placed
      //
      GetElementPtrInst *gep_instr = GetElementPtrInst::CreateInBounds(
        buf_alloca_inst, index_args.begin(), index_args.end(), "", pCall);

      Value *arg = pCall->getArgOperand(numarg);
      const Type *argtype = arg->getType();

      // bitcast from generic i8* address to a pointer to the argument's type
      //
      BitCastInst *cast_instr = new BitCastInst(gep_instr, PointerType::getUnqual(argtype), "", pCall);

      // store argument into address. Alignment forced to 1 to make vector
      // stores safe.
      //
      (void) new StoreInst(arg, cast_instr, false, 1, pCall);

      // This argument occupied some space in the buffer. 
      // Advance the buffer pointer offset by its size to know where the next
      // argument should be placed.
      // 
      unsigned argsize = TD.getTypeSizeInBits(arg->getType()) / 8;
      buf_pointer_offset += argsize;        
    }

    // Create a pointer to the buffer, in order to pass it to the function
    //
    std::vector<Value*> index_args;
    index_args.push_back(ConstantInt::get(int32_type, 0));
    index_args.push_back(ConstantInt::get(int32_type, 0));

    GetElementPtrInst *ptr_to_buf = GetElementPtrInst::CreateInBounds(
      buf_alloca_inst, index_args.begin(), index_args.end(), "", pCall);

    // Finally create the call to opencl_printf
    //
    Function *pFunc = m_pModule->getFunction("opencl_printf");
    assert(pFunc && "Expect builtin printf to be declared before use");

    std::vector<Value*> params;
    params.push_back(pCall->getArgOperand(0));
    params.push_back(ptr_to_buf);
    params.push_back(m_pCtx);
    Value *res = CallInst::Create(pFunc, params.begin(), params.end(), "translated_opencl_printf_call", pCall);
    return res;
  }

  Value* ResolveWICall::updateAsyncCopy(llvm::CallInst *pCall, bool strided) {
    TargetData TD(m_pModule);

    assert( m_pCtx && "Context pointer m_pCtx created as expected" );

    // Create new call instruction with extended parameters
    SmallVector<Value*, 4> params;
    // push original parameters
    // Need bitcast to a general pointer
    CastInst *pBCDst = CastInst::Create(Instruction::BitCast, pCall->getArgOperand(0),
      PointerType::get(IntegerType::get(*m_pLLVMContext, 8), 0), "", pCall);
    params.push_back(pBCDst);
    CastInst *pBCSrc = CastInst::Create(Instruction::BitCast, pCall->getArgOperand(1),
      PointerType::get(IntegerType::get(*m_pLLVMContext, 8), 0), "", pCall);
    params.push_back(pBCSrc);
    params.push_back(pCall->getArgOperand(2));
    params.push_back(pCall->getArgOperand(3));
    if ( strided ) {
      params.push_back(pCall->getArgOperand(4));
    }
    // Distinguish operator size
    const PointerType *pPTy = dyn_cast<PointerType>(pCall->getArgOperand(0)->getType());
    assert(pPTy && "Must be a pointer");
    const Type *pPT = pPTy->getElementType();


	assert(pPT->getPrimitiveSizeInBits() && "Not primitive type, not valid calculation");
    unsigned int uiSize = TD.getPrefTypeAlignment(pPT);

	params.push_back(ConstantInt::get(IntegerType::get(*m_pLLVMContext,  sizeof(size_t) * BYTE_SIZE), uiSize));
    params.push_back(m_pCtx);
    assert(pPTy && "Must by a pointer type");

    Function *pNewAsyncCopy = NULL;
    if ( strided ) {
      pNewAsyncCopy = m_pModule->getFunction(pPTy->getAddressSpace() == 3 ? "lasync_wg_copy_strided_g2l" : "lasync_wg_copy_strided_l2g");
    } else {
      pNewAsyncCopy = m_pModule->getFunction(pPTy->getAddressSpace() == 3 ? "lasync_wg_copy_g2l" : "lasync_wg_copy_l2g");
    }

    Value *res = CallInst::Create(pNewAsyncCopy, params.begin(), params.end(), "", pCall);
    return res;
  }

  void ResolveWICall::updateWaitGroup(llvm::CallInst *pCall) {

    assert( m_pCtx && "Context pointer m_pCtx created as expected" );

    // Create new call instruction with extended parameters
    SmallVector<Value*, 4> params;
    params.push_back(pCall->getArgOperand(0));
    params.push_back(pCall->getArgOperand(1));
    params.push_back(m_pCtx);
    Function *pNewWait = m_pModule->getFunction("lwait_group_events");
    CallInst::Create(pNewWait, params.begin(), params.end(), "", pCall);
  }

  void ResolveWICall::updatePrefetch(llvm::CallInst *pCall) {

    TargetData TD(m_pModule);

    unsigned int uiSizeT = m_pModule->getPointerSize()*32;

    // Create new call instruction with extended parameters
    SmallVector<Value*, 4> params;
    // push original parameters
    // Need bitcast to a general pointer
    CastInst *pBCPtr = CastInst::Create(Instruction::BitCast, pCall->getArgOperand(0),
      PointerType::get(IntegerType::get(*m_pLLVMContext, 8), 0), "", pCall);
    params.push_back(pBCPtr);
    // Put number of elements
    params.push_back(pCall->getArgOperand(1));
    // Distinguish element size
    const PointerType *pPTy = dyn_cast<PointerType>(pCall->getArgOperand(0)->getType());
    assert(pPTy && "Must be a pointer");
    const Type *pPT = pPTy->getElementType();
	
	assert(pPT->getPrimitiveSizeInBits() && "Not primitive type, not valid calculation");
    unsigned int uiSize = TD.getPrefTypeAlignment(pPT);

    params.push_back(ConstantInt::get(IntegerType::get(*m_pLLVMContext, uiSizeT), uiSize));
    Function *pPrefetch = m_pModule->getFunction("lprefetch");
    CallInst::Create(pPrefetch, params.begin(), params.end(), "", pCall);
  }

  void ResolveWICall::addAsyncCopyDeclaration() {
    if ( m_bAsyncCopyDecl ) {
      // Async copy declarations already added
      return;
    }

    unsigned int uiSizeT = m_pModule->getPointerSize()*32;

    //event_t async_work_group_copy(void *pDst, void *pSrc, size_t numElem, event_t event,
    //                 size_t elemSize, LLVMExecMultipleWIWithBarrier **ppExec);
    std::vector<const Type*> params;
    params.push_back(PointerType::get(IntegerType::get(*m_pLLVMContext, 8), 0));
    params.push_back(PointerType::get(IntegerType::get(*m_pLLVMContext, 8), 0));
    params.push_back(IntegerType::get(*m_pLLVMContext, uiSizeT));
    params.push_back(IntegerType::get(*m_pLLVMContext, uiSizeT));
    params.push_back(IntegerType::get(*m_pLLVMContext, uiSizeT));
    params.push_back(PointerType::get(IntegerType::get(*m_pLLVMContext, uiSizeT), 0));
    FunctionType *pNewType = FunctionType::get(IntegerType::get(*m_pLLVMContext, uiSizeT), params, false);
    Function::Create(pNewType, (GlobalValue::LinkageTypes)0, "lasync_wg_copy_l2g", m_pModule);
    Function::Create(pNewType, (GlobalValue::LinkageTypes)0, "lasync_wg_copy_g2l", m_pModule);

    //event_t async_work_group_strided_copy(void *pDst, void *pSrc, size_t numElem, size_t stride, event_t event,
    //                 size_t elemSize, LLVMExecMultipleWIWithBarrier **ppExec);
    params.clear();
    params.push_back(PointerType::get(IntegerType::get(*m_pLLVMContext, 8), 0));
    params.push_back(PointerType::get(IntegerType::get(*m_pLLVMContext, 8), 0));
    params.push_back(IntegerType::get(*m_pLLVMContext, uiSizeT));
    params.push_back(IntegerType::get(*m_pLLVMContext, uiSizeT));
    params.push_back(IntegerType::get(*m_pLLVMContext, uiSizeT));
    params.push_back(IntegerType::get(*m_pLLVMContext, uiSizeT));
    params.push_back(PointerType::get(IntegerType::get(*m_pLLVMContext, uiSizeT), 0));
    pNewType = FunctionType::get(IntegerType::get(*m_pLLVMContext, uiSizeT), params, false);
    Function::Create(pNewType, (GlobalValue::LinkageTypes)0, "lasync_wg_copy_strided_l2g", m_pModule);
    Function::Create(pNewType, (GlobalValue::LinkageTypes)0, "lasync_wg_copy_strided_g2l", m_pModule);


    // void wait_group_events(int num_events, event_t event_list)
    params.clear();
    params.push_back(IntegerType::get(*m_pLLVMContext, 32));
    params.push_back(PointerType::get(IntegerType::get(*m_pLLVMContext, uiSizeT), 0));
    params.push_back(PointerType::get(IntegerType::get(*m_pLLVMContext, uiSizeT), 0));
    FunctionType *pWaitType = FunctionType::get(Type::getVoidTy(*m_pLLVMContext), params, false);
    Function::Create(pWaitType, (GlobalValue::LinkageTypes)0, "lwait_group_events", m_pModule);

    m_bAsyncCopyDecl = true;
  }

  void ResolveWICall::addPrintfDeclaration() {
    if (m_bPrintfDecl) {
      // Print declaration already added
      return;
    }

    // The prototype of opencl_printf is:
    // int opencl_printf(char *format, char *args, LLVMExecutable **ppExec)
    //
    std::vector<const Type*> params;
    // The 'format' string is in constant address space (address space 2)
    params.push_back(PointerType::get(IntegerType::get(*m_pLLVMContext, 8), 2));
    params.push_back(PointerType::get(IntegerType::get(*m_pLLVMContext, 8), 0));
    params.push_back(PointerType::get(IntegerType::get(*m_pLLVMContext, sizeof(size_t) * BYTE_SIZE), 0));
   
    FunctionType *pNewType = FunctionType::get(Type::getInt32Ty(*m_pLLVMContext), params, false);
    Function::Create(pNewType, Function::ExternalLinkage, "opencl_printf", m_pModule);

    m_bPrintfDecl = true;
  }

  void ResolveWICall::addPrefetchDeclaration() {
    if ( m_bPrefetchDecl ) {
      // Prefetch declaration already added
      return;
    }

    unsigned int uiSizeT = m_pModule->getPointerSize()*32;

    std::vector<const Type*> params;
    // Source Pointer
    params.push_back(PointerType::get(IntegerType::get(*m_pLLVMContext, 8), 0));
    // Number of elements
    params.push_back(IntegerType::get(*m_pLLVMContext, uiSizeT));
    // Element size
    params.push_back(IntegerType::get(*m_pLLVMContext, uiSizeT));
    FunctionType *pNewType = FunctionType::get(Type::getVoidTy(*m_pLLVMContext), params, false);
    Function::Create(pNewType, (GlobalValue::LinkageTypes)0, "lprefetch", m_pModule);

    m_bPrefetchDecl = true;
  }

  TInternalCallType ResolveWICall::getCallFunctionType(std::string calledFuncName) {

    if( calledFuncName == CompilationUtils::NAME_GET_LID ) {
      return ICT_GET_LOCAL_ID;
    }
    if( calledFuncName == CompilationUtils::NAME_GET_GID ) {
      return ICT_GET_GLOBAL_ID;
    }
    if( calledFuncName == CompilationUtils::NAME_GET_ITERATION_COUNT ) {
      return ICT_GET_ITER_COUNT;
    }
    if( calledFuncName == CompilationUtils::NAME_GET_SPECIAL_BUFFER ) {
      return ICT_GET_SPECIAL_BUFFER;
    }
    if( calledFuncName == CompilationUtils::NAME_GET_CURR_WI ) {
      return ICT_GET_CURR_WI;
    }
    if( calledFuncName == CompilationUtils::NAME_GET_WORK_DIM ) {
      return ICT_GET_WORK_DIM;
    }
    if( calledFuncName == CompilationUtils::NAME_GET_GLOBAL_SIZE ) {
      return ICT_GET_GLOBAL_SIZE;
    }
    if( calledFuncName == CompilationUtils::NAME_GET_LOCAL_SIZE ) {
      return ICT_GET_LOCAL_SIZE;
    }
    if( calledFuncName == CompilationUtils::NAME_GET_NUM_GROUPS ) {
      return ICT_GET_NUM_GROUPS;
    }
    if( calledFuncName == CompilationUtils::NAME_GET_GROUP_ID ) {
      return ICT_GET_GROUP_ID;
    }
    if( calledFuncName == CompilationUtils::NAME_GET_GLOBAL_OFFSET ) {
      return ICT_GET_GLOBAL_OFFSET;
    }
    if( calledFuncName == CompilationUtils::NAME_PRINTF ) {
      return ICT_PRINTF;
    }
    if( 0 == calledFuncName.find(CompilationUtils::NAME_ASYNC_WORK_GROUP_COPY) ) {
      return ICT_ASYNC_WORK_GROUP_COPY;
    }
    if( 0 == calledFuncName.find(CompilationUtils::NAME_WAIT_GROUP_EVENTS) ) {
      return ICT_WAIT_GROUP_EVENTS;
    }
    if( 0 == calledFuncName.find(CompilationUtils::NAME_PREFETCH) ) {
      return ICT_PREFETCH;
    }
    if( 0 == calledFuncName.find(CompilationUtils::NAME_ASYNC_WORK_GROUP_STRIDED_COPY) ) {
      return ICT_ASYNC_WORK_GROUP_STRIDED;
    }
    return ICT_NONE;
  }

}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {