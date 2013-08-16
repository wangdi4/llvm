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

#include "llvm/Support/InstIterator.h"
#include "llvm/Version.h"
#if LLVM_VERSION == 3200
#include "llvm/DataLayout.h"
#else
#include "llvm/Target/TargetData.h"
#endif
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IRBuilder.h"

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

    bool ResolveWICall::runOnModule(Module &M) {
      m_pModule = &M;
      m_pLLVMContext = &M.getContext();

      m_bAsyncCopyDecl = false;
      m_bPrefetchDecl = false;
      m_bPrintfDecl = false;
      m_pStructNDRangeType = NULL;

      // extended execution flags
      m_ExtExecDecls.clear();
      
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
      &m_pBaseGlbId, &m_pLocalId, &m_pIterCount, &m_pSpecialBuf, &m_pCurrWI, 
      &m_pCtx, &m_pExtendedExecutionCtx);

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
        std::string calledFuncName = pCall->getCalledFunction()->getName().str();
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
        case ICT_GET_BASE_GLOBAL_ID:
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
#ifndef __APPLE__
        case ICT_PRINTF:
          addPrintfDeclaration();
          pNewRes = updatePrintf(pCall);
          assert(pNewRes && "Expected updatePrintf to succeed");
          break;

        case ICT_GET_DEFAULT_QUEUE:
          addExtExecFunctionDeclaration(calledFuncType);
          pNewRes = updateExtExecFunction(getExtExecFunctionParams(pCall),
            getExtExecCallbackName(calledFuncType),
            pCall);
          assert(pNewRes && "Expected updateGetDefaultQueue to succeed");
          break;

        case ICT_ENQUEUE_KERNEL_BASIC:
          addExtExecFunctionDeclaration(calledFuncType);
          pNewRes = updateExtExecFunction(getExtExecFunctionParams(pCall),
            getExtExecCallbackName(calledFuncType),
            pCall);
          assert(pNewRes && "Expected updateGetEnqueueKernelBasic to succeed");
          break;

        case ICT_ENQUEUE_KERNEL_LOCALMEM: {
          const uint32_t ICT_ENQUEUE_KERNEL_LOCALMEM_ARG_POS = 4;
          addExtExecFunctionDeclaration(calledFuncType);
          pNewRes = updateExtExecFunction(getEnqueueKernelLocalMemFunctionParams(pCall, ICT_ENQUEUE_KERNEL_LOCALMEM_ARG_POS),
            getExtExecCallbackName(calledFuncType),
            pCall);
          assert(pNewRes && "Expected updateGetEnqueueKernelLocalMem to succeed");
          break;
                                          }
        case ICT_ENQUEUE_KERNEL_EVENTS:
          addExtExecFunctionDeclaration(calledFuncType);
          pNewRes = updateExtExecFunction(getExtExecFunctionParams(pCall),
            getExtExecCallbackName(calledFuncType),
            pCall);
          assert(pNewRes && "Expected updateGetEnqueueKernelEvents to succeed");
          break;

        case ICT_ENQUEUE_KERNEL_EVENTS_LOCALMEM: {
          const uint32_t ICT_ENQUEUE_KERNEL_EVENTS_LOCALMEM_ARG_POS = 7;
          addExtExecFunctionDeclaration(calledFuncType);
          pNewRes = updateExtExecFunction(getEnqueueKernelLocalMemFunctionParams(pCall, ICT_ENQUEUE_KERNEL_EVENTS_LOCALMEM_ARG_POS),
            getExtExecCallbackName(calledFuncType),
            pCall);
          assert(pNewRes && "Expected updateGetEnqueueKernelEvents to succeed");
          break;
                                                 }
        case ICT_ENQUEUE_MARKER:
          addExtExecFunctionDeclaration(calledFuncType);
          pNewRes = updateExtExecFunction(getExtExecFunctionParams(pCall),
            getExtExecCallbackName(calledFuncType),
            pCall);
          assert(pNewRes && "Expected updateEnqueueMarker to succeed");
          break;

        case ICT_NDRANGE_1D:
          pNewRes = updateNDRangeND(pCall, 1);
          assert(pNewRes && "Expected updateNDRange1D to succeed");
          break;

        case ICT_NDRANGE_2D:
          pNewRes = updateNDRangeND(pCall, 2);
          assert(pNewRes && "Expected updateNDRange2D to succeed");
          break;

        case ICT_NDRANGE_3D:
          pNewRes = updateNDRangeND(pCall, 3);
          assert(pNewRes && "Expected updateNDRange3D to succeed");
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
#endif // __APPLE__
        default:
          continue;
        }

        if ( pNewRes ) {
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
    BasicBlock *pBlock = pCall->getParent();
    Value *pResult = NULL;  // Object that holds the resolved value

    if ( type == ICT_GET_WORK_DIM ) {
      // Now retrieve address of the DIM count
      SmallVector<Value*, 4> params;
      params.push_back(getConstZeroInt32Value());
      params.push_back(getConstZeroInt32Value());
      GetElementPtrInst *pDimCntAddr =
        GetElementPtrInst::Create(m_pWorkInfo, ArrayRef<Value*>(params), "", pCall);
      // Load the Value
      pResult = new LoadInst(pDimCntAddr, "", pCall);
      return pResult;
    }

    std::string overflowValueString = "0";
    switch( type ) {
    case ICT_GET_LOCAL_ID:
    case ICT_GET_GLOBAL_ID:
    case ICT_GET_BASE_GLOBAL_ID:
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
    PHINode *pAttrResult = PHINode::Create(IntegerType::get(*m_pLLVMContext, sizeof(size_t) * BYTE_SIZE), 2, "", splitContinue->getFirstNonPHI());
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
          getConstZeroInt32Value() :
        pCall->getArgOperand(1) );
        params.push_back(ConstantInt::get(IntegerType::get(*m_pLLVMContext, 32), iTableInx));
        params.push_back(pCall->getArgOperand(0));

        GetElementPtrInst *pAddr =
          GetElementPtrInst::Create(structure, ArrayRef<Value*>(params), "", pInsertBefore);

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
      if ( type == ICT_GET_BASE_GLOBAL_ID ) {
        return calcBaseGlobalId(pCall, pInsertBefore);
      }
      assert( false && "Unhandled internal call type!" );
      return NULL; // This to avoid compilation warning
  }

  Value* ResolveWICall::calcBaseGlobalId(CallInst *pCall, Instruction *pInsertBefore) {
    SmallVector<Value*, 4> params;
    // base global values
    params.push_back(getConstZeroInt32Value());
    params.push_back(getConstZeroInt32Value());
    params.push_back(pCall->getArgOperand(0));

    // Calculate the address of base global value
    GetElementPtrInst *pGlbBaseAddr =
      GetElementPtrInst::Create(m_pBaseGlbId, ArrayRef<Value*>(params), "", pInsertBefore);
    // Load the value of base global
    Value *pBaseGlbIdVal = new LoadInst(pGlbBaseAddr, "", pInsertBefore);
    return pBaseGlbIdVal;
  }

  Value* ResolveWICall::calcGlobalId(CallInst *pCall, Instruction *pInsertBefore) {

    SmallVector<Value*, 4> params;
    // Load local id values
    params.push_back(pCall->getArgOperand(1));
    params.push_back(getConstZeroInt32Value());
    params.push_back(pCall->getArgOperand(0));

    // Calculate the address of local id value
    GetElementPtrInst *pLclIdAddr =
      GetElementPtrInst::Create(m_pLocalId, ArrayRef<Value*>(params), "", pInsertBefore);
    // Load the value of local id
    Value *pLocalIdVal = new LoadInst(pLclIdAddr, "", pInsertBefore);

    params.clear();
    // base global values
    params.push_back(getConstZeroInt32Value());
    params.push_back(getConstZeroInt32Value());
    params.push_back(pCall->getArgOperand(0));

    // Calculate the address of base global value
    GetElementPtrInst *pGlbBaseAddr =
      GetElementPtrInst::Create(m_pBaseGlbId, ArrayRef<Value*>(params), "", pInsertBefore);
    // Load the value of base global
    Value *pBaseGlbIdVal = new LoadInst(pGlbBaseAddr, "", pInsertBefore);

    // Now add these two values
    Value *pGlbId = BinaryOperator::CreateAdd( pLocalIdVal, pBaseGlbIdVal, "", pInsertBefore);

    return pGlbId;
  }

  Value* ResolveWICall::updatePrintf(CallInst *pCall) {

    assert( m_pCtx && "Context pointer m_pCtx created as expected" );
#if LLVM_VERSION == 3200
    DataLayout DL(m_pModule);
#else
    TargetData DL(m_pModule);
#endif

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
      unsigned argsize = DL.getTypeSizeInBits(arg->getType()) / 8;
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
      pCall->getParent()->getParent()->getEntryBlock().begin());

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
      BitCastInst *cast_instr = new BitCastInst(gep_instr, PointerType::getUnqual(argtype), "", pCall);

      // store argument into address. Alignment forced to 1 to make vector
      // stores safe.
      //
      (void) new StoreInst(arg, cast_instr, false, 1, pCall);

      // This argument occupied some space in the buffer. 
      // Advance the buffer pointer offset by its size to know where the next
      // argument should be placed.
      // 
      unsigned argsize = DL.getTypeSizeInBits(arg->getType()) / 8;
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

    std::vector<Value*> params;
    params.push_back(pCall->getArgOperand(0));
    params.push_back(ptr_to_buf);
    params.push_back(m_pCtx);
    CallInst *res = CallInst::Create(pFunc, ArrayRef<Value*>(params), "translated_opencl_printf_call", pCall);
    res->setDebugLoc(pCall->getDebugLoc());
    return res;
  }

  Value* ResolveWICall::updateAsyncCopy(llvm::CallInst *pCall, bool strided) {
#if LLVM_VERSION == 3200
    DataLayout DL(m_pModule);
#else
    TargetData DL(m_pModule);
#endif

    assert( m_pCtx && "Context pointer m_pCtx created as expected" );

    // Create new call instruction with extended parameters
    SmallVector<Value*, 8> params;
    // push original parameters
    // Need bitcast to a general pointer
    CastInst *pBCDst = CastInst::Create(Instruction::BitCast, pCall->getArgOperand(0),
      PointerType::get(IntegerType::get(*m_pLLVMContext, 8), 0), "", pCall);
    params.push_back(pBCDst);
    CastInst *pBCSrc = CastInst::Create(Instruction::BitCast, pCall->getArgOperand(1),
      PointerType::get(IntegerType::get(*m_pLLVMContext, 8), 0), "", pCall);
    params.push_back(pBCSrc);
    params.push_back(pCall->getArgOperand(2));
    unsigned int eventIndex = 3;
    if ( strided ) {
      params.push_back(pCall->getArgOperand(3));
      eventIndex++;
    }
    //The reason for this change is because implementation of Event_t type in CLANG has changed. It is not void* anymore, 
    //and hence the bitcast was required in order to comply with function signature.
    CastInst *Event = CastInst::Create(Instruction::PtrToInt, pCall->getArgOperand(eventIndex),
      IntegerType::get(*m_pLLVMContext,  sizeof(size_t) * BYTE_SIZE),"", pCall);
    params.push_back(Event);

    // Distinguish operator size
    PointerType *pPTy = dyn_cast<PointerType>(pCall->getArgOperand(0)->getType());
    assert(pPTy && "Must be a pointer");
    Type *pPT = pPTy->getElementType();


    assert(pPT->getPrimitiveSizeInBits() && "Not primitive type, not valid calculation");
    unsigned int uiSize = DL.getPrefTypeAlignment(pPT);

    params.push_back(ConstantInt::get(IntegerType::get(*m_pLLVMContext,  sizeof(size_t) * BYTE_SIZE), uiSize));
    params.push_back(m_pCtx);
    assert(pPTy && "Must by a pointer type");

    Function *pNewAsyncCopy = NULL;
    if ( strided ) {
      pNewAsyncCopy = m_pModule->getFunction(pPTy->getAddressSpace() == 3 ? "lasync_wg_copy_strided_g2l" : "lasync_wg_copy_strided_l2g");
    } else {
      pNewAsyncCopy = m_pModule->getFunction(pPTy->getAddressSpace() == 3 ? "lasync_wg_copy_g2l" : "lasync_wg_copy_l2g");
    }
    Value *res = CallInst::Create(pNewAsyncCopy, ArrayRef<Value*>(params), "", pCall);
    CastInst *resCasted = CastInst::Create(Instruction::IntToPtr, res, pCall->getType(),"", pCall);
    return resCasted;
  }

  void ResolveWICall::updateWaitGroup(llvm::CallInst *pCall) {

    assert( m_pCtx && "Context pointer m_pCtx created as expected" );

    // Create new call instruction with extended parameters
    SmallVector<Value*, 4> params;
    params.push_back(pCall->getArgOperand(0));
    CastInst *pEvent = CastInst::Create(Instruction::BitCast, pCall->getArgOperand(1),
      PointerType::get(IntegerType::get(*m_pLLVMContext,  sizeof(size_t) * BYTE_SIZE), 0), "", pCall);
    params.push_back(pEvent);
    params.push_back(m_pCtx);
    Function *pNewWait = m_pModule->getFunction("lwait_group_events");
    CallInst::Create(pNewWait, ArrayRef<Value*>(params), "", pCall);
  }

  void ResolveWICall::updatePrefetch(llvm::CallInst *pCall) {

#if LLVM_VERSION == 3200
    DataLayout DL(m_pModule);
#else
    TargetData DL(m_pModule);
#endif

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
    PointerType *pPTy = dyn_cast<PointerType>(pCall->getArgOperand(0)->getType());
    assert(pPTy && "Must be a pointer");
    Type *pPT = pPTy->getElementType();

    assert(pPT->getPrimitiveSizeInBits() && "Not primitive type, not valid calculation");
    unsigned int uiSize = DL.getPrefTypeAlignment(pPT);

    params.push_back(ConstantInt::get(IntegerType::get(*m_pLLVMContext, uiSizeT), uiSize));
    Function *pPrefetch = m_pModule->getFunction("lprefetch");
    CallInst::Create(pPrefetch, ArrayRef<Value*>(params), "", pCall);
  }

  void ResolveWICall::addAsyncCopyDeclaration() {
    if ( m_bAsyncCopyDecl ) {
      // Async copy declarations already added
      return;
    }

    unsigned int uiSizeT = m_pModule->getPointerSize()*32;

    //event_t async_work_group_copy(void *pDst, void *pSrc, size_t numElem, event_t event,
    //                 size_t elemSize, LLVMExecMultipleWIWithBarrier **ppExec);
    std::vector<Type*> params;
    params.push_back(PointerType::get(IntegerType::get(*m_pLLVMContext, 8), 0));
    params.push_back(PointerType::get(IntegerType::get(*m_pLLVMContext, 8), 0));
    params.push_back(IntegerType::get(*m_pLLVMContext, uiSizeT));
    params.push_back(IntegerType::get(*m_pLLVMContext, uiSizeT));
    params.push_back(IntegerType::get(*m_pLLVMContext, uiSizeT));
    params.push_back(PointerType::get(IntegerType::get(*m_pLLVMContext, uiSizeT), 0));
    FunctionType *pNewType = FunctionType::get(IntegerType::get(*m_pLLVMContext, uiSizeT), params, false);
    Function::Create(pNewType, Function::ExternalLinkage, "lasync_wg_copy_l2g", m_pModule);
    Function::Create(pNewType, Function::ExternalLinkage, "lasync_wg_copy_g2l", m_pModule);

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
    Function::Create(pNewType, Function::ExternalLinkage, "lasync_wg_copy_strided_l2g", m_pModule);
    Function::Create(pNewType, Function::ExternalLinkage, "lasync_wg_copy_strided_g2l", m_pModule);


    // void wait_group_events(int num_events, event_t event_list)
    params.clear();
    params.push_back(IntegerType::get(*m_pLLVMContext, 32));
    params.push_back(PointerType::get(IntegerType::get(*m_pLLVMContext, uiSizeT), 0));
    params.push_back(PointerType::get(IntegerType::get(*m_pLLVMContext, uiSizeT), 0));
    FunctionType *pWaitType = FunctionType::get(Type::getVoidTy(*m_pLLVMContext), params, false);
    Function::Create(pWaitType, Function::ExternalLinkage, "lwait_group_events", m_pModule);

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
    std::vector<Type*> params;
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

    if(calledFuncName == CompilationUtils::NAME_GET_LID) {
      return ICT_GET_LOCAL_ID;
    }
    if( calledFuncName == CompilationUtils::NAME_GET_GID ) {
      return ICT_GET_GLOBAL_ID;
    }
    if( calledFuncName == CompilationUtils::NAME_GET_BASE_GID ) {
      return ICT_GET_BASE_GLOBAL_ID;
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
    if( CompilationUtils::isGetWorkDim(calledFuncName) )
      return ICT_GET_WORK_DIM;
    if(CompilationUtils::isGetGlobalSize(calledFuncName))
      return ICT_GET_GLOBAL_SIZE;
    if(CompilationUtils::isGetLocalSize(calledFuncName))
      return ICT_GET_LOCAL_SIZE;
    if(CompilationUtils::isGetNumGroups(calledFuncName))
      return ICT_GET_NUM_GROUPS;
    if(CompilationUtils::isGetGroupId(calledFuncName))
      return ICT_GET_GROUP_ID;
    if(CompilationUtils::isGlobalOffset(calledFuncName))
      return ICT_GET_GLOBAL_OFFSET;
    if(calledFuncName == CompilationUtils::NAME_PRINTF)
      return ICT_PRINTF;
    if(CompilationUtils::isAsyncWorkGroupCopy(calledFuncName))
      return ICT_ASYNC_WORK_GROUP_COPY;
    if(CompilationUtils::isWaitGroupEvents(calledFuncName))
      return ICT_WAIT_GROUP_EVENTS;
    if(CompilationUtils::isPrefetch(calledFuncName))
      return ICT_PREFETCH;
    if(CompilationUtils::isAsyncWorkGroupStridedCopy(calledFuncName))
      return ICT_ASYNC_WORK_GROUP_STRIDED;

    // OpenCL2.0 extended execution built-ins
    if(CompilationUtils::getCLVersionFromModule(*m_pModule) >= CompilationUtils::CL_VER_2_0){
      if( CompilationUtils::isGetDefaultQueue(calledFuncName))
        return ICT_GET_DEFAULT_QUEUE;
      if( CompilationUtils::isNDRange_1D(calledFuncName))
        return ICT_NDRANGE_1D;
      if( CompilationUtils::isNDRange_2D(calledFuncName))
        return ICT_NDRANGE_2D;
      if( CompilationUtils::isNDRange_3D(calledFuncName))
        return ICT_NDRANGE_3D;
      if( CompilationUtils::isEnqueueKernelBasic(calledFuncName))
        return ICT_ENQUEUE_KERNEL_BASIC;
      if( CompilationUtils::isEnqueueKernelLocalMem(calledFuncName))
        return ICT_ENQUEUE_KERNEL_LOCALMEM;
      if( CompilationUtils::isEnqueueKernelEvents(calledFuncName))
        return ICT_ENQUEUE_KERNEL_EVENTS;
      if( CompilationUtils::isEnqueueKernelEventsLocalMem(calledFuncName))
        return ICT_ENQUEUE_KERNEL_EVENTS_LOCALMEM;
      if( CompilationUtils::isEnqueueMarker(calledFuncName))
        return ICT_ENQUEUE_MARKER;
    }
    return ICT_NONE;
  }

  Type * ResolveWICall::getQueueType() const {
    return PointerType::get(m_pModule->getTypeByName("opencl.queue_t"), 0);
  }

  Type * ResolveWICall::getClkEventType() const {
    return PointerType::get(m_pModule->getTypeByName("opencl.clk_event_t"), 0);
  }

  Type * ResolveWICall::getKernelEnqueueFlagsType() const {
    return IntegerType::get(*m_pLLVMContext, 32);
  }

  Type * ResolveWICall::getNDRangeType() const {
    return PointerType::get(m_pModule->getTypeByName("opencl.ndrange_t"), 0);
  }

  Type * ResolveWICall::getBlockNoArgumentsType() const {
    return PointerType::get(FunctionType::get(Type::getVoidTy(*m_pLLVMContext), 
      false), 0);
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

  Type * ResolveWICall::getExtendedExecContextType() const {
    return PointerType::get(m_pModule->getTypeByName("struct.ExtendedExecutionContext"), 0);
  }

  Type * ResolveWICall::getSizeTType() const {
    return IntegerType::get(*m_pLLVMContext, m_pModule->getPointerSize()*32);
  }

  ConstantInt * ResolveWICall::getConstZeroInt32Value() const {
    return ConstantInt::get(Type::getInt32Ty(*m_pLLVMContext), 0);
  }

  Type* ResolveWICall::getLocalMemBufType() const {
    return IntegerType::get(*m_pLLVMContext, 32);
  }

  Type * ResolveWICall::getOrAddStructNDRangeType() {
    if(m_pStructNDRangeType)
      return m_pStructNDRangeType;
    // type -  size_t XXX[3]
    Type * arr3 = ArrayType::get(getSizeTType(), MAX_WORK_DIM);

    /*typedef struct _cl_work_description_type
    {
    unsigned int workDimension;
    size_t globalWorkOffset[MAX_WORK_DIM];
    size_t globalWorkSize[MAX_WORK_DIM];
    size_t localWorkSize[MAX_WORK_DIM];
    unsigned int minWorkGroupNum;
    } cl_work_description_type;
    typedef cl_work_description_type struct.__ndrange_t*/

    SmallVector<Type*, 5> elems;
    elems.push_back(IntegerType::get(*m_pLLVMContext, 32));
    elems.push_back(arr3);
    elems.push_back(arr3);
    elems.push_back(arr3);
    elems.push_back(IntegerType::get(*m_pLLVMContext, 32));
    m_pStructNDRangeType = StructType::create(*m_pLLVMContext, elems, "struct.__ndrange_t");
    return m_pStructNDRangeType;
  }

  /// @brief Store Value to 'unsigned int workDimension' in ndrange_t struct
  StoreInst* ResolveWICall::StoreWorkDim(Value* Ptr, uint64_t V, LLVMContext* pContext, Instruction* InsertBefore)
  {
    Value *GEPWorkDimParams[2];
    GEPWorkDimParams[0] = getConstZeroInt32Value();
    GEPWorkDimParams[1] = getConstZeroInt32Value();

    Value* GEPWorkDim = GetElementPtrInst::CreateInBounds(
      Ptr, ArrayRef<Value*>(GEPWorkDimParams), "GEPWorkDim", InsertBefore);

    Value *valueWorkDim = ConstantInt::get(IntegerType::get(*pContext, 32), V);
    // store workDim
    return new StoreInst(valueWorkDim, GEPWorkDim, InsertBefore);
  }

  /// @brief store value to one of Arrays in ndrange_t struct
  StoreInst* ResolveWICall::StoreNDRangeArrayElement(Value* Ptr, Value* V, const uint64_t ArrayPosition,
    const uint64_t ElementIndex, const Twine &Name, LLVMContext* pContext, Instruction* InsertBefore)
  {
    assert( (ArrayPosition > 0) && (ArrayPosition < 4) && "ArrayIndex is out of boundaries");
    assert( (ElementIndex < 3) && "ElementIndex is out of boundaries");
    Value *GEPParams[3];
    GEPParams[0] = getConstZeroInt32Value();
    GEPParams[1] = ConstantInt::get(IntegerType::get(*pContext, 32), ArrayPosition);
    GEPParams[2] = ConstantInt::get(IntegerType::get(*pContext, 32), ElementIndex);
    Value* GEPNDRangeArrElem =  GetElementPtrInst::CreateInBounds(Ptr, ArrayRef<Value*>(GEPParams), Name, InsertBefore);
    DEBUG(dbgs() << "store struct.__ndrange_t ArrayIndex = "<<ArrayPosition<<
      " ElementIndex = "<<ElementIndex<<" with Name "<<Name<<"\n");
    return new StoreInst(V, GEPNDRangeArrElem, InsertBefore);
  }

  /// map given argument index to index of Array within ndrange_t struct
  uint32_t ResolveWICall::MapIndexToIndexOfArray(const uint32_t Index, const uint32_t argsNum)
  {
    // ndrange_t ndrange_1D/2D/3D (size_t global_work_size)
    // ndrange_t ndrange_1D/2D/3D (size_t global_work_size, size_t local_work_size)
    // ndrange_t ndrange_1D/2D/3D (size_t global_work_offset, size_t global_work_size, size_t local_work_size)
    return (argsNum == 3)?(Index + 1) : ( Index + 2);
  }

  Value* ResolveWICall::updateNDRangeND(CallInst *pCall, const uint32_t WorkDim) {
    assert(m_pLLVMContext  && "m_pLLVMContext is NULL");
    assert( ((WorkDim > 0) && (WorkDim <= MAX_WORK_DIM)) && "Incorrect number of dimentions in ndrange_ND call, where N > 1");
    uint32_t argsNum = pCall->getNumArgOperands();
    const unsigned int MAX_ARGS = 3;
    assert( (argsNum > 0) && (argsNum <= MAX_ARGS) && "Incorrect number of input arguments");

    // allocate and initialize ndrage_t
    // %ndrange_t.1 = alloca %struct.__ndrange_t
    AllocaInst *allocStruct  = new AllocaInst(getOrAddStructNDRangeType(),"s.__ndrange_t", pCall);

    // store to unsigned workDimension
    StoreWorkDim(allocStruct, WorkDim, m_pLLVMContext, pCall);

    Value* args[MAX_ARGS];

    // number of work dims says about length of the input Arrays
    // process vector of element with same array indexes
    for(uint32_t i = 0; i < WorkDim; ++i)
    {
      // proccess each input argument
      // argument's indexes will be mapped to Array index within ndrange_t struct
      for(uint32_t j = 0; j < argsNum; ++j)
      {
        args[j] = pCall->getOperand(j);
        // extract operand
        Value* argValue = args[j];
        // check if operand is pointer
        // if so, then we are in 2D or in 3D ndrange BIs.
        // dereference pointer and get value
        if(args[j]->getType()->getTypeID() == Type::PointerTyID)
        {
          Value *Index = ConstantInt::get(IntegerType::get(*m_pLLVMContext, 32), i);
          Value* pArgValue = GetElementPtrInst::CreateInBounds(args[j], ArrayRef<Value*>(Index), "", pCall);
          argValue = new LoadInst(pArgValue, "", pCall);
        }
        // store extracted value(from orerand or from input array) to ndrange_t struct
        StoreNDRangeArrayElement(allocStruct, argValue, MapIndexToIndexOfArray(j, argsNum), i, "", m_pLLVMContext, pCall);
      }
    }

    // bitcast to opencl.ndrange_t
    CastInst *bitcastV =  CastInst::Create(Instruction::BitCast, allocStruct, 
      getNDRangeType(), "", pCall);

    return bitcastV;
  }


  void ResolveWICall::addLocalMemArgs(std::vector<Value*>& args, CallInst *pCall,
    const unsigned LocalMemArgsOffs){

      assert(pCall->getNumArgOperands() > LocalMemArgsOffs && "Expect enqueue_kernel to have local mem size arguments");
      // get number of local buffers
      const unsigned numLocalBuffers = pCall->getNumArgOperands() - LocalMemArgsOffs;

      // array with local mem sizes
      ArrayType *localbuf_arr_type = ArrayType::get(getLocalMemBufType(), numLocalBuffers);
      // issue alloca for array
      AllocaInst *localbuf_alloca_inst = new AllocaInst(localbuf_arr_type, "localmem_arg_buf",
        pCall->getParent()->getParent()->getEntryBlock().begin());
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
          // issue store of current local buf size
          (void) new StoreInst(pCall->getArgOperand(numarg), gep_instr, false, 1, pCall);
      }

      // get pointer to first element in array
      llvm::SmallVector<Value*, 2> index_args;
      index_args.push_back(getConstZeroInt32Value());
      index_args.push_back(getConstZeroInt32Value());
      GetElementPtrInst *ptr_to_localmem_buf = GetElementPtrInst::CreateInBounds(
        localbuf_alloca_inst, ArrayRef<Value*>(index_args), "", pCall);

      // add argument uint *localbuf_size
      args.push_back(ptr_to_localmem_buf);
      // add argument uint localbuf_size_len
      args.push_back(ConstantInt::get(int32_type, numLocalBuffers));
  }

  Value* ResolveWICall::updateExtExecFunction(std::vector<Value*> Params, const StringRef FunctionName, CallInst *InsertBefore)
  {
    // implicitly add Extended Execution Context
    Params.push_back(m_pExtendedExecutionCtx);

    return CallInst::Create(
      m_pModule->getFunction(FunctionName),
      ArrayRef<Value*>(Params),
      "", 
      InsertBefore);
  }

  std::vector<Value*> ResolveWICall::getExtExecFunctionParams(CallInst *pCall)
  {
    return std::vector<Value*>(pCall->op_begin(), pCall->op_begin() + pCall->getNumArgOperands() );
  }

  std::vector<Value*> ResolveWICall::getEnqueueKernelLocalMemFunctionParams(CallInst *pCall, const uint32_t FixedArgs){
    assert( m_pExtendedExecutionCtx && 
      "Extended Execution Context pointer m_pExtendedExecutionCtx not created as expected" );

    // copy arguments from initial call except size0, size1, ...
    std::vector<Value*> params(pCall->op_begin(), pCall->op_begin() + FixedArgs);

    // add arguments with local mem
    addLocalMemArgs(params, pCall, FixedArgs);

    return params;
  }

  FunctionType*  ResolveWICall::getDefaultQueueFunctionType(){
    return FunctionType::get(
      getQueueType(), // return type 
      getExtendedExecContextType(), // arg Extended Execution
      false);
  }

  // The prototype of ocl20_enqueue_kernel_basic is:
  // int ocl20_enqueue_kernel_basic(queue_t, int /*kernel_enqueue_flags_t*/, 
  //            ndrange_t, void * /*block_literal ptr*/,  ExtendedExecutionContext * pEEC)
  //
  // The prototype of ocl20_enqueue_kernel_events is:
  // int ocl20_enqueue_kernel_events(queue_t, int /*kernel_enqueue_flags_t*/, 
  //            ndrange_t, 
  //            uint num_events_in_wait_list, clk_event_t *in_wait_list,
  //            clk_event_t *event_ret,
  //            void * /*block_literal ptr*/, 
  //            ExtendedExecutionContext * pEEC)
  //
  // The prototype of ocl20_enqueue_kernel_basic is:
  // int ocl20_enqueue_kernel_localmem(queue_t, int /*kernel_enqueue_flags_t*/, 
  //            ndrange_t, void * (local void*) /*block_literal ptr*/, 
  //            uint *localbuf_size, uint localbuf_size_len,
  //            ExtendedExecutionContext * pEEC)
  //
  // The prototype of ocl20_enqueue_kernel_events is:
  // int ocl20_enqueue_kernel_events_localmem(queue_t, int /*kernel_enqueue_flags_t*/, 
  //            ndrange_t, 
  //            uint num_events_in_wait_list, clk_event_t *in_wait_list,
  //            clk_event_t *event_ret,
  //            void * (local void*) /*block_literal ptr*/, 
  //            uint *localbuf_size, uint localbuf_size_len,
  //            ExtendedExecutionContext * pEEC)
  //
  // This function creates LLVM types for ALL 4 above enqueue_kernel callbacks
  FunctionType*  ResolveWICall::getEnqueueKernelType(const TInternalCallType type ){
    std::vector<Type*> params;
    // queue_t
    params.push_back(getQueueType());
    // int /*kernel_enqueue_flags_t*/
    params.push_back(getKernelEnqueueFlagsType());
    // ndrange_t
    params.push_back(getNDRangeType());
    // events
    if (type == ICT_ENQUEUE_KERNEL_EVENTS || type == ICT_ENQUEUE_KERNEL_EVENTS_LOCALMEM) {
      // uint num_events_in_wait_list
      params.push_back(IntegerType::get(*m_pLLVMContext, 32));
      // clk_event_t *in_wait_list
      params.push_back(PointerType::get(getClkEventType(), 0));
      // clk_event_t *event_ret
      params.push_back(PointerType::get(getClkEventType(), 0));
    }
    // void * /*block_literal ptr*/
    if(type == ICT_ENQUEUE_KERNEL_BASIC || type == ICT_ENQUEUE_KERNEL_EVENTS)
      params.push_back(getBlockNoArgumentsType());
    else if (type == ICT_ENQUEUE_KERNEL_LOCALMEM || type == ICT_ENQUEUE_KERNEL_EVENTS_LOCALMEM)
      params.push_back(getBlockLocalMemType());
    else
      assert(0); // should not be here

    // local memory
    if(type == ICT_ENQUEUE_KERNEL_LOCALMEM || type == ICT_ENQUEUE_KERNEL_EVENTS_LOCALMEM) {
      // uint * localbuf_size
      params.push_back(PointerType::get(getLocalMemBufType(), 0));
      // uint localbuf_size_len
      params.push_back(IntegerType::get(*m_pLLVMContext, 32));
    }
    // ExtendedExecutionContext *
    params.push_back(getExtendedExecContextType());
    // create function type
    return FunctionType::get(
      getEnqueueKernelRetType(), // return type
      params, 
      false);
  }

  FunctionType* ResolveWICall::getEnqueueMarkerFunctionType()
  {
    std::vector<Type*> params;
    params.push_back(getQueueType()); //queue_t queue
    params.push_back(IntegerType::get(*m_pLLVMContext, 32)); //uint num_events_in_wait_list
    params.push_back(PointerType::get(getClkEventType(), 0)); //const clk_event_t *event_wait_list
    params.push_back(PointerType::get(getClkEventType(), 0)); // clk_event_t *event_ret
    params.push_back(getExtendedExecContextType()); // arg Extended Execution
    return FunctionType::get(
      IntegerType::get(*m_pLLVMContext, 32), // return type
      params,
      false);
  }
  
  void ResolveWICall::addExtExecFunctionDeclaration(const TInternalCallType type)
  {
    // check declaration exists
    if(m_ExtExecDecls.find(type) != m_ExtExecDecls.end())
      return;
    // create declaration
    Function::Create(
      getExtExecFunctionType(type), // function type
      Function::ExternalLinkage, 
      getExtExecCallbackName(type), // function name
      m_pModule);
    // mark declaration is done
    m_ExtExecDecls.insert(type);
  }
  std::string ResolveWICall::getExtExecCallbackName(const TInternalCallType type) const
  {
    if(type == ICT_GET_DEFAULT_QUEUE)
      return "ocl20_get_default_queue";
    else if(type == ICT_ENQUEUE_KERNEL_BASIC)
      return "ocl20_enqueue_kernel_basic";
    else if(type == ICT_ENQUEUE_KERNEL_LOCALMEM)
      return "ocl20_enqueue_kernel_localmem";
    else if(type == ICT_ENQUEUE_KERNEL_EVENTS)
      return "ocl20_enqueue_kernel_events";
    else if(type == ICT_ENQUEUE_KERNEL_EVENTS_LOCALMEM)
      return "ocl20_enqueue_kernel_events_localmem";
    else if(type == ICT_ENQUEUE_MARKER)
      return "ocl20_enqueue_marker";
    else 
      assert(0);
    return "";
  }
  FunctionType* ResolveWICall::getExtExecFunctionType(const TInternalCallType type)
  {
    if(type == ICT_GET_DEFAULT_QUEUE)
      return getDefaultQueueFunctionType();
    else if(type == ICT_ENQUEUE_KERNEL_BASIC ||
            type == ICT_ENQUEUE_KERNEL_LOCALMEM ||
            type == ICT_ENQUEUE_KERNEL_EVENTS ||
            type == ICT_ENQUEUE_KERNEL_EVENTS_LOCALMEM)
      return getEnqueueKernelType(type);
    else if(type == ICT_ENQUEUE_MARKER)
      return getEnqueueMarkerFunctionType();
    else 
      assert(0);
    return NULL;
  }
} // namespace intel
