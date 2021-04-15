//===- ResolveWICall.cpp - Resolve DPC++ kernel work-item call ------------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/ResolveWICall.h"
#include "ImplicitArgsUtils.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Passes.h"
#include <algorithm>

using namespace llvm;

#define DEBUG_TYPE "dpcpp-kernel-resolve-wi-call"

// Add command line to specify work groups size as uniform.
static cl::opt<bool> OptUniformWGSize(
    "dpcpp-uniform-wg-size", cl::init(false), cl::Hidden,
    cl::desc("The flag speficies work groups size as uniform"));

namespace {

/// Legacy ResolveWICall pass.
class ResolveWICallLegacy : public ModulePass {
public:
  static char ID;

  ResolveWICallLegacy(bool IsUniformWG = false, bool UseTLSGlobals = false);

  llvm::StringRef getPassName() const override { return "ResolveWICallLegacy"; }

  bool runOnModule(Module &M) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<CallGraphWrapperPass>();
    AU.addRequired<ImplicitArgsAnalysisLegacy>();
    AU.addPreserved<CallGraphWrapperPass>();
    AU.addPreserved<ImplicitArgsAnalysisLegacy>();
  }

private:
  ResolveWICallPass Impl;
  /// true if a module is compiled with the support of the non-uniform
  /// work-group size.
  bool UniformLocalSize;
  /// Use TLS globals instead of implicit arguments.
  bool UseTLSGlobals;
};

} // namespace

INITIALIZE_PASS_BEGIN(ResolveWICallLegacy, DEBUG_TYPE,
                      "Resolve work-item built-in calls", false, false)
INITIALIZE_PASS_DEPENDENCY(CallGraphWrapperPass)
INITIALIZE_PASS_DEPENDENCY(ImplicitArgsAnalysisLegacy)
INITIALIZE_PASS_END(ResolveWICallLegacy, DEBUG_TYPE,
                    "Resolve work-item built-in calls", false, false)

char ResolveWICallLegacy::ID = 0;

ResolveWICallLegacy::ResolveWICallLegacy(bool IsUniformWG, bool UseTLSGlobals)
    : ModulePass(ID), UniformLocalSize(IsUniformWG),
      UseTLSGlobals(UseTLSGlobals) {
  initializeImplicitArgsAnalysisLegacyPass(*PassRegistry::getPassRegistry());
}

bool ResolveWICallLegacy::runOnModule(Module &M) {
  CallGraph *CG = &getAnalysis<CallGraphWrapperPass>().getCallGraph();
  ImplicitArgsInfo *IAInfo =
      &getAnalysis<ImplicitArgsAnalysisLegacy>().getResult();
  return Impl.runImpl(M, UniformLocalSize, UseTLSGlobals, IAInfo, CG);
}

ModulePass *llvm::createResolveWICallLegacyPass(bool IsUniformWGSize,
                                                bool UseTLSGlobals) {
  return new ResolveWICallLegacy(IsUniformWGSize, UseTLSGlobals);
}

PreservedAnalyses ResolveWICallPass::run(Module &M, ModuleAnalysisManager &AM) {
  CallGraph *CG = &AM.getResult<CallGraphAnalysis>(M);
  ImplicitArgsInfo *IAInfo = &AM.getResult<ImplicitArgsAnalysis>(M);
  if (!runImpl(M, false, false, IAInfo, CG))
    return PreservedAnalyses::all();
  PreservedAnalyses PA;
  PA.preserve<CallGraphAnalysis>();
  PA.preserve<ImplicitArgsAnalysis>();
  return PA;
}

bool ResolveWICallPass::runImpl(Module &M, bool IsUniformWG, bool UseTLSGlobals,
                                ImplicitArgsInfo *IAInfo, CallGraph *CG) {
  this->M = &M;
  Ctx = &M.getContext();
  this->IAInfo = IAInfo;
  this->CG = CG;

  PrefetchDecl = false;
  UniformLocalSize = IsUniformWG | OptUniformWGSize;
  this->UseTLSGlobals = UseTLSGlobals;

  // extended execution flags
  ExtExecDecls.clear();

  OclVersion = DPCPPKernelCompilationUtils::fetchCLVersionFromMetadata(M);
  // Run on all defined function in the module
  for (Function &F : M) {
    if (F.isDeclaration()) {
      // Function is not defined inside module
      continue;
    }

    if (DPCPPKernelCompilationUtils::isGlobalCtorDtorOrCPPFunc(&F))
      continue;

    clearPerFunctionCache();
    runOnFunction(&F);
  }

  return true;
}

static LoadInst *createLoadForTLSGlobal(IRBuilder<> &Builder, Module *M,
                                        ImplicitArgsUtils::IMPLICIT_ARGS Arg) {
  auto TLSG = DPCPPKernelCompilationUtils::getTLSGlobal(M, Arg);
  assert(TLSG && "TLS Global cannot be nullptr");
  return Builder.CreateLoad(TLSG->getValueType(), TLSG);
}

Function *ResolveWICallPass::runOnFunction(Function *F) {
  this->F = F;
  Value *SpecialBuf = nullptr;
  if (UseTLSGlobals) {
    IRBuilder<> B(dyn_cast<Instruction>(F->getEntryBlock().begin()));
    WorkInfo =
        createLoadForTLSGlobal(B, M, ImplicitArgsUtils::IA_WORK_GROUP_INFO);
    WGId = createLoadForTLSGlobal(B, M, ImplicitArgsUtils::IA_WORK_GROUP_ID);
    BaseGlbId =
        createLoadForTLSGlobal(B, M, ImplicitArgsUtils::IA_GLOBAL_BASE_ID);
    SpecialBuf =
        createLoadForTLSGlobal(B, M, ImplicitArgsUtils::IA_BARRIER_BUFFER);
    RuntimeHandle =
        createLoadForTLSGlobal(B, M, ImplicitArgsUtils::IA_RUNTIME_HANDLE);
  } else {
    DPCPPKernelCompilationUtils::getImplicitArgs(
        F, nullptr, &WorkInfo, &WGId, &BaseGlbId, &SpecialBuf, &RuntimeHandle);
  }

  std::vector<Instruction *> ToRemoveInsts;
  std::vector<CallInst *> ToHandleCalls;
  for (auto &N : *(*CG)[F]) {
    auto *CI = cast<CallInst>(*N.first);
    if (!CI->getCalledFunction())
      continue; // skip indirect function calls

    ToHandleCalls.push_back(CI);
  }
  for (CallInst *CI : ToHandleCalls) {
    assert(CI->getCalledFunction() &&
           "Unexpected indirect function invocation");
    StringRef CalledFuncName = CI->getCalledFunction()->getName();
    TInternalCallType CalledFuncType = getCallFunctionType(CalledFuncName);

    Value *NewRes = nullptr;
    switch (CalledFuncType) {

    case ICT_GET_SPECIAL_BUFFER:
      NewRes = SpecialBuf;
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
      NewRes = updateGetFunction(CI, CalledFuncType);
      assert(NewRes && "Expected updateGetFunction to succeed");
      break;
    case ICT_PRINTF:
      if (!ExtExecDecls.count(ICT_PRINTF))
        addExternFunctionDeclaration(
            CalledFuncType, getOrCreatePrintfFuncType(), "opencl_printf");
      NewRes = updatePrintf(CI);
      assert(NewRes && "Expected updatePrintf to succeed");
      break;
    case ICT_ENQUEUE_KERNEL_EVENTS_LOCALMEM:
    case ICT_ENQUEUE_KERNEL_LOCALMEM: {
      // TODO: It seems there is no need to handle these functions
      // separately, since they are not variadics anymore. Move their
      // implementation to built-in library
      std::string CallbackName = CalledFuncType == ICT_ENQUEUE_KERNEL_LOCALMEM
                                     ? "ocl20_enqueue_kernel_localmem"
                                     : "ocl20_enqueue_kernel_events_localmem";
      if (!ExtExecDecls.count(CalledFuncType)) {
        FunctionType *FT = getOrCreateEnqueueKernelFuncType(CalledFuncType);
        addExternFunctionDeclaration(CalledFuncType, FT, CallbackName);
      }
      // Copy original function operands
      SmallVector<Value *, 16> ExtExecArgs(CI->arg_operands());
      // Add the RuntimeInterface arg
      ExtExecArgs.push_back(getOrCreateRuntimeInterface());
      // Add the Block2KernelMapper arg
      ExtExecArgs.push_back(getOrCreateBlock2KernelMapper());
      // Add the RuntimeHandle arg if needed
      ExtExecArgs.push_back(RuntimeHandle);
      NewRes = updateEnqueueKernelFunction(ExtExecArgs, CallbackName, CI);
      assert(NewRes && "Expected non-NULL results");
    } break;
    case ICT_PREFETCH:
      addPrefetchDeclaration();
      // Substitute extern operand with function parameter
      updatePrefetch(CI);
      // prefetch* function returns void, no need to replace its usages!
      break;
    default:
      continue;
    }

    if (NewRes) {
      // Replace CI usages with new calculation
      CI->replaceAllUsesWith(NewRes);
    }
    // Add CI it to toRemoveInstruction container
    ToRemoveInsts.push_back(CI);
  }

  // Remove all instructions in ToRemoveInsts.
  for (Instruction *I : ToRemoveInsts)
    I->eraseFromParent();

  return F;
}

Value *ResolveWICallPass::updateGetFunction(CallInst *CI,
                                            TInternalCallType CallType) {
  assert(CI && "Invalid CallInst");
  if (CallType == ICT_GET_WORK_DIM) {
    IRBuilder<> B(CI);
    return IAInfo->GenerateGetFromWorkInfo(NDInfo::WORK_DIM, WorkInfo, B);
  }
  BasicBlock *BB = CI->getParent();
  Value *pResult = nullptr; // Object that holds the resolved value

  uint64_t OverflowValue = 0;
  switch (CallType) {
  case ICT_GET_BASE_GLOBAL_ID:
  case ICT_GET_GROUP_ID:
  case ICT_GET_GLOBAL_OFFSET:
    break;
  case ICT_GET_NUM_GROUPS:
  case ICT_GET_LOCAL_SIZE:
  case ICT_GET_ENQUEUED_LOCAL_SIZE:
  case ICT_GET_GLOBAL_SIZE:
    OverflowValue = 1;
    break;
  default:
    llvm_unreachable("Unhandled internal call type!");
  }

  // check if the function argument is constant
  IntegerType *I32Ty = IntegerType::get(*Ctx, 32);
  Constant *ConstOverflow = ConstantInt::get(CI->getType(), OverflowValue);
  ConstantInt *Val = dyn_cast<ConstantInt>(CI->getArgOperand(0));

  if (nullptr != Val) {
    // in case of constant argument we can check it "offline" if it's inbound
    unsigned int indexValue = (unsigned int)*Val->getValue().getRawData();

    if (indexValue >= MAX_WORK_DIM) {
      // return overflow result (OCL SPEC requirement)
      return ConstOverflow;
    }
    return updateGetFunctionInBound(CI, CallType, CI);
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
  BasicBlock *getWIProperties =
      BasicBlock::Create(*Ctx, "get.wi.properties", BB->getParent());
  BasicBlock *splitContinue =
      BB->splitBasicBlock(BasicBlock::iterator(CI), "split.continue");

  // A.change the old basic block to the detailed entry
  // Entry:1. remove the unconditional jump instruction
  BB->getTerminator()->eraseFromParent();

  // Entry:2. add the entry tail code (as described up)
  ConstantInt *MaxWorkDimI32 = ConstantInt::get(I32Ty, MAX_WORK_DIM);
  ICmpInst *CheckIndex = new ICmpInst(ICmpInst::ICMP_ULT, CI->getArgOperand(0),
                                      MaxWorkDimI32, "check.index.inbound");
  CheckIndex->setDebugLoc(CI->getDebugLoc());
  BB->getInstList().push_back(CheckIndex);
  BranchInst *CheckIndexBI = BranchInst::Create(getWIProperties, splitContinue, CheckIndex, BB);
  CheckIndexBI->setDebugLoc(CI->getDebugLoc());

  // B.Build the get.wi.properties block
  // Now retrieve address of the DIM count

  BranchInst *SplitContinueBI = BranchInst::Create(splitContinue, getWIProperties);
  SplitContinueBI->setDebugLoc(CI->getDebugLoc());
  Instruction *InsertBefore = getWIProperties->getTerminator();
  pResult = updateGetFunctionInBound(CI, CallType, InsertBefore);

  // C.Create Phi node at the first of the spiltted BB
  PHINode *AttrResult =
      PHINode::Create(CI->getType(), 2, "", splitContinue->getFirstNonPHI());
  AttrResult->addIncoming(pResult, getWIProperties);
  AttrResult->addIncoming(ConstOverflow, BB);

  return AttrResult;
}

Value *ResolveWICallPass::updateGetFunctionInBound(CallInst *CI,
                                                   TInternalCallType CallType,
                                                   Instruction *InsertBefore) {
  IRBuilder<> Builder(InsertBefore);
  std::string Name;
  switch (CallType) {
  case ICT_GET_GLOBAL_OFFSET:
  case ICT_GET_GLOBAL_SIZE:
  case ICT_GET_NUM_GROUPS:
    return IAInfo->GenerateGetFromWorkInfo(
        NDInfo::internalCall2NDInfo(CallType), WorkInfo, CI->getArgOperand(0),
        Builder);
  case ICT_GET_LOCAL_SIZE:
    return IAInfo->GenerateGetLocalSize(UniformLocalSize, WorkInfo, WGId,
                                        CI->getArgOperand(0), Builder);
  case ICT_GET_ENQUEUED_LOCAL_SIZE:
    return IAInfo->GenerateGetEnqueuedLocalSize(WorkInfo, CI->getArgOperand(0),
                                                Builder);
  case ICT_GET_BASE_GLOBAL_ID:
    return IAInfo->GenerateGetBaseGlobalID(BaseGlbId, CI->getArgOperand(0),
                                           Builder);
  case ICT_GET_GROUP_ID:
    return IAInfo->GenerateGetGroupID(WGId, CI->getArgOperand(0), Builder);
  default:
    break;
  }
  assert(false && "Unexpected ID function");
  return 0;
}

Value *ResolveWICallPass::updatePrintf(CallInst *CI) {

  assert(RuntimeHandle && "Context pointer RuntimeHandle created as expected");
  const DataLayout &DL = M->getDataLayout();

  // Find out the buffer size required to store all the arguments.
  // Note: CallInst->getNumOperands() returns the number of operands in
  // the instruction, including its destination as #0. Since this is
  // a printf call and we're interested in all the arguments after the
  // format string, we start with #2.
  assert(CI->getNumArgOperands() > 0 &&
         "Expect printf to have a format string");
  unsigned TotalArgSize = 0;
  for (unsigned NumArg = 1; NumArg < CI->getNumArgOperands(); ++NumArg) {
    Value *arg = CI->getArgOperand(NumArg);
    unsigned argsize = DL.getTypeAllocSize(arg->getType());
    TotalArgSize += argsize;
  }

  // Types used in several places.
  IntegerType *I32Type = IntegerType::get(*Ctx, 32);
  IntegerType *I8Type = IntegerType::get(*Ctx, 8);

  // Create the alloca instruction for allocating the buffer on the stack.
  // Also, handle the special case where printf got no vararg arguments:
  // printf("hello");
  // Since we have to pass something into the 'args' argument of
  // opencl_printf, and 'alloca' with size 0 is undefined behavior, we
  // just allocate a dummy buffer of size 1. opencl_printf won't look at
  // it anyway.
  ArrayType *BufArrType;
  if (CI->getNumArgOperands() == 1) {
    BufArrType = ArrayType::get(I8Type, 1);
  } else {
    BufArrType = ArrayType::get(I8Type, TotalArgSize);
  }
  // TODO: add comment
  AllocaInst *BufAI =
      new AllocaInst(BufArrType, DL.getAllocaAddrSpace(), "temp_arg_buf",
                     &*CI->getParent()->getParent()->getEntryBlock().begin());

  // Generate instructions to store the operands into the argument buffer.
  unsigned BufPointerOffset = 0;
  for (unsigned NumArg = 1; NumArg < CI->getNumArgOperands(); ++NumArg) {
    std::vector<Value *> IndexArgs;
    IndexArgs.push_back(getConstZeroInt32Value());
    IndexArgs.push_back(ConstantInt::get(I32Type, BufPointerOffset));

    // getelementptr to compute the address into which this argument will
    // be placed.
    GetElementPtrInst *GEPInst = GetElementPtrInst::CreateInBounds(
        BufAI, ArrayRef<Value *>(IndexArgs), "", CI);

    Value *Arg = CI->getArgOperand(NumArg);
    Type *Argtype = Arg->getType();

    // bitcast from generic i8* address to a pointer to the Argument's type.
    CastInst *CastI = CastInst::CreatePointerCast(
        GEPInst, PointerType::getUnqual(Argtype), "", CI);

    // store Argument into address. Alignment forced to 1 to make vector.
    // stores safe.
    (void)new StoreInst(Arg, CastI, false, Align(1), CI);

    // This Argument occupied some space in the buffer.
    // Advance the buffer pointer offset by its size to know where the next
    // Argument should be placed.
    unsigned Argsize = DL.getTypeAllocSize(Arg->getType());
    BufPointerOffset += Argsize;
  }

  // Create a pointer to the buffer, in order to pass it to the function.
  std::vector<Value *> IndexArgs;
  IndexArgs.push_back(getConstZeroInt32Value());
  IndexArgs.push_back(getConstZeroInt32Value());

  GetElementPtrInst *PtrToBuf = GetElementPtrInst::CreateInBounds(
      BufAI, ArrayRef<Value *>(IndexArgs), "", CI);

  // Finally create the call to opencl_printf.
  Function *F = M->getFunction("opencl_printf");
  assert(F && "Expect builtin printf to be declared before use");

  SmallVector<Value *, 16> Params;
  Params.push_back(CI->getArgOperand(0));
  Params.push_back(PtrToBuf);
  Value *RuntimeInterface = getOrCreateRuntimeInterface();
  Params.push_back(RuntimeInterface);
  Params.push_back(RuntimeHandle);
  CallInst *Res =
      CallInst::Create(F, Params, "translated_opencl_printf_call", CI);
  Res->setDebugLoc(CI->getDebugLoc());
  return Res;
}

void ResolveWICallPass::updatePrefetch(CallInst *CI) {
  unsigned int SizeT = getPointerSize();

  // Create new call instruction with extended parameters.
  SmallVector<Value *, 4> Params;
  // push original parameters.
  // Need bitcast to a general pointer.
  CastInst *BCPtr = CastInst::CreatePointerCast(
      CI->getArgOperand(0), PointerType::get(IntegerType::get(*Ctx, 8), 0), "",
      CI);
  Params.push_back(BCPtr);
  // Put number of elements.
  Params.push_back(CI->getArgOperand(1));
  // Distinguish element size.
  PointerType *PTy = dyn_cast<PointerType>(CI->getArgOperand(0)->getType());
  assert(PTy && "Must be a pointer");
  Type *PT = PTy->getElementType();

  assert(PT->getPrimitiveSizeInBits() &&
         "Not primitive type, not valid calculation");
  unsigned int Size = M->getDataLayout().getPrefTypeAlignment(PT);

  Params.push_back(ConstantInt::get(IntegerType::get(*Ctx, SizeT), Size));
  Function *Prefetch = M->getFunction("lprefetch");
  assert(Prefetch && "Missing 'lprefetch' function");
  CallInst::Create(Prefetch, ArrayRef<Value *>(Params), "", CI);
}

FunctionType *ResolveWICallPass::getOrCreatePrintfFuncType() {
  // The prototype of opencl_printf is:
  // int opencl_printf(__constant char *format, char *args, void *Callback,
  // void *RuntimeHandle)
  std::vector<Type *> Params;
  // The 'format' string is in constant address space (address space 2).
  Params.push_back(PointerType::get(IntegerType::get(*Ctx, 8), 2));
  Params.push_back(PointerType::get(IntegerType::get(*Ctx, 8), 0));
  Params.push_back(
      IAInfo->getWorkGroupInfoMemberType(NDInfo::RUNTIME_INTERFACE));
  Params.push_back(RuntimeHandle->getType());

  return FunctionType::get(Type::getInt32Ty(*Ctx), Params, false);
}

void ResolveWICallPass::addPrefetchDeclaration() {
  if (PrefetchDecl) {
    // Prefetch declaration already added.
    return;
  }

  unsigned int SizeT = getPointerSize();

  std::vector<Type *> Params;
  // Source Pointer
  Params.push_back(PointerType::get(IntegerType::get(*Ctx, 8), 0));
  // Number of elements
  Params.push_back(IntegerType::get(*Ctx, SizeT));
  // Element size
  Params.push_back(IntegerType::get(*Ctx, SizeT));
  FunctionType *NewType =
      FunctionType::get(Type::getVoidTy(*Ctx), Params, false);
  Function::Create(NewType, Function::ExternalLinkage, "lprefetch", M);

  PrefetchDecl = true;
}

TInternalCallType ResolveWICallPass::getCallFunctionType(StringRef FuncName) {
  if (FuncName == DPCPPKernelCompilationUtils::nameGetBaseGID())
    return ICT_GET_BASE_GLOBAL_ID;
  if (DPCPPKernelCompilationUtils::isGetSpecialBuffer(FuncName))
    return ICT_GET_SPECIAL_BUFFER;
  if (DPCPPKernelCompilationUtils::isGetWorkDim(FuncName))
    return ICT_GET_WORK_DIM;
  if (DPCPPKernelCompilationUtils::isGetGlobalSize(FuncName))
    return ICT_GET_GLOBAL_SIZE;
  if (DPCPPKernelCompilationUtils::isGetNumGroups(FuncName))
    return ICT_GET_NUM_GROUPS;
  if (DPCPPKernelCompilationUtils::isGetGroupId(FuncName))
    return ICT_GET_GROUP_ID;
  if (DPCPPKernelCompilationUtils::isGlobalOffset(FuncName))
    return ICT_GET_GLOBAL_OFFSET;
  // special built-ins that need to update.
  if (DPCPPKernelCompilationUtils::isPrintf(FuncName))
    return ICT_PRINTF;
  if (DPCPPKernelCompilationUtils::isPrefetch(FuncName))
    return ICT_PREFETCH;

  // OpenCL2.0 built-ins to resolve.
  if (OclVersion == DPCPPKernelCompilationUtils::OclVersion::CL_VER_2_0) {
    if (DPCPPKernelCompilationUtils::isEnqueueKernelLocalMem(FuncName))
      return ICT_ENQUEUE_KERNEL_LOCALMEM;
    if (DPCPPKernelCompilationUtils::isEnqueueKernelEventsLocalMem(FuncName))
      return ICT_ENQUEUE_KERNEL_EVENTS_LOCALMEM;
    if (DPCPPKernelCompilationUtils::isGetLocalSize(FuncName))
      return ICT_GET_LOCAL_SIZE;
    if (DPCPPKernelCompilationUtils::isGetEnqueuedLocalSize(FuncName))
      return ICT_GET_ENQUEUED_LOCAL_SIZE;
  } else {
    // built-ins which behavior is different in OpenCL versions older than 2.0.
    if (DPCPPKernelCompilationUtils::isGetLocalSize(FuncName))
      return ICT_GET_ENQUEUED_LOCAL_SIZE;
  }
  return ICT_NONE;
}

// address space generated by clang for queue_t, clk_event_t, ndrange_t types.
const int EXTEXEC_OPAQUE_TYPES_ADDRESS_SPACE =
    DPCPPKernelCompilationUtils::ADDRESS_SPACE_GLOBAL;
Type *ResolveWICallPass::getQueueType() const {
  return PointerType::getInt8PtrTy(*Ctx, EXTEXEC_OPAQUE_TYPES_ADDRESS_SPACE);
}

Type *ResolveWICallPass::getClkEventType() const {
  return PointerType::getInt8PtrTy(*Ctx, EXTEXEC_OPAQUE_TYPES_ADDRESS_SPACE);
}

Type *ResolveWICallPass::getKernelEnqueueFlagsType() const {
  return IntegerType::get(*Ctx, 32);
}

Type *ResolveWICallPass::getNDRangeType() const {
  return PointerType::getInt8PtrTy(*Ctx, EXTEXEC_OPAQUE_TYPES_ADDRESS_SPACE);
}

Type *ResolveWICallPass::getBlockLocalMemType() const {
  // void (^block)(local void *, ...), - OpenCL
  // i8 addrspace(4)*  - LLVM representation
  return PointerType::get(Type::getInt8Ty(*Ctx),
                          DPCPPKernelCompilationUtils::ADDRESS_SPACE_GENERIC);
}

Type *ResolveWICallPass::getEnqueueKernelRetType() const {
  return IntegerType::get(*Ctx, ENQUEUE_KERNEL_RETURN_BITS);
}

ConstantInt *ResolveWICallPass::getConstZeroInt32Value() const {
  return ConstantInt::get(Type::getInt32Ty(*Ctx), 0);
}

Type *ResolveWICallPass::getLocalMemBufType() const {
  unsigned SizeT = getPointerSize();
  return IntegerType::get(*Ctx, SizeT);
}

static bool isPointerToStructType(Type *Ty) {
  // check it is pointer.
  if (!Ty->isPointerTy())
    return false;

  Type *PtrTy = cast<PointerType>(Ty)->getElementType();
  // pointer type is struct.
  if (!PtrTy->isStructTy())
    return false;
  return true;
}

Value *ResolveWICallPass::updateEnqueueKernelFunction(
    SmallVectorImpl<Value *> &NewParams, const StringRef FuncName,
    CallInst *CI) {
  // Bitcast types from built-in argument type to type of callback function's
  // argument.
  // Handles scenario when types like ndrange_t.3, clk_event.2, queue_t.5, etc
  // are generated by ParseBitcodeFile.
  // Appending .#number to type name happens when the LLVM context is reused by
  // BE for loading bytecode.
  // We handle here ONLY pointer to struct and double pointer to struct.
  Function *CbkF = M->getFunction(FuncName);
  assert(CbkF && "Missing callback function");
  Function::arg_iterator ArgIt = CbkF->arg_begin();
  for (auto It = NewParams.begin(), E = NewParams.end(); It != E;
       ++It, ++ArgIt) {
    Value *&NewParam = *It;
    Type *NewParamTy = NewParam->getType();
    Type *ExpectedArgTy = ArgIt->getType();
    // check types are equal - no bitcast needed.
    if (NewParamTy == ArgIt->getType())
      continue;
    // check it is pointer.
    if (!NewParamTy->isPointerTy())
      llvm_unreachable("Unsupported type of argument");
    Type *PtrTy = cast<PointerType>(NewParamTy)->getElementType();
    // pointer type is struct.
    if (PtrTy->isStructTy()) {
      *It = CastInst::CreatePointerCast(NewParam, ExpectedArgTy, "", CI);
      continue;
    }
    // check pointer is to pointer.
    if (!PtrTy->isPointerTy())
      llvm_unreachable("Unsupported type of argument");
    // double pointer points to structure.
    Type *PPtrTy = cast<PointerType>(PtrTy)->getElementType();
    if (PPtrTy->isStructTy()) {
      NewParam = CastInst::CreatePointerCast(NewParam, ExpectedArgTy, "", CI);
      continue;
    }
    llvm_unreachable("Unsupported type of argument");
  }
  CallInst *NewCI =
      CallInst::Create(M->getFunction(FuncName), NewParams, "", CI);

  // If return value type does not match return type, bitcast to return type.
  Value *Ret = NewCI;
  if (CI->getType() != NewCI->getType()) {
    if (isPointerToStructType(CI->getType()))
      Ret = CastInst::CreatePointerCast(NewCI, CI->getType(), "", CI);
    else
      llvm_unreachable("Unsupported type of Return value");
  }
  return Ret;
}

void ResolveWICallPass::clearPerFunctionCache() {
  F = 0;
  RuntimeInterface = 0;
  Block2KernelMapper = 0;
}

Value *ResolveWICallPass::getOrCreateBlock2KernelMapper() {
  IRBuilder<> Builder(&*F->getEntryBlock().begin());
  if (UseTLSGlobals)
    Builder.SetInsertPoint(dyn_cast<Instruction>(WorkInfo)->getNextNode());
  if (!Block2KernelMapper)
    Block2KernelMapper = IAInfo->GenerateGetFromWorkInfo(
        NDInfo::BLOCK2KERNEL_MAPPER, WorkInfo, Builder);
  return Block2KernelMapper;
}

Value *ResolveWICallPass::getOrCreateRuntimeInterface() {
  IRBuilder<> Builder(&*F->getEntryBlock().begin());
  if (UseTLSGlobals)
    Builder.SetInsertPoint(dyn_cast<Instruction>(WorkInfo)->getNextNode());
  if (!RuntimeInterface)
    RuntimeInterface = IAInfo->GenerateGetFromWorkInfo(
        NDInfo::RUNTIME_INTERFACE, WorkInfo, Builder);
  return RuntimeInterface;
}

// The prototype of ocl20_enqueue_kernel_events is:
// int ocl20_enqueue_kernel_events_localmem(
//    queue_t*, int /*kernel_enqueue_flags_t*/,
//    ndrange_t,
//    uint num_events_in_wait_list,
//    clk_event_t *in_wait_list, clk_event_t *event_ret,
//    void * /*block_invoke*/,
//    void * /*block_literal*/,
//    uint localbuf_size_len, uint *localbuf_size,
//    ExtendedExecutionContext * pEEC)
//
// This function creates LLVM types for ALL 2 above enqueue_kernel callbacks
FunctionType *
ResolveWICallPass::getOrCreateEnqueueKernelFuncType(unsigned FuncType) {
  assert(FuncType == ICT_ENQUEUE_KERNEL_LOCALMEM ||
         FuncType == ICT_ENQUEUE_KERNEL_EVENTS_LOCALMEM);
  SmallVector<Type *, 16> Params;
  // queue_t
  Params.push_back(getQueueType());
  // int /*kernel_enqueue_flags_t*/
  Params.push_back(getKernelEnqueueFlagsType());
  // ndrange_t
  Params.push_back(getNDRangeType());
  // events
  if (FuncType == ICT_ENQUEUE_KERNEL_EVENTS_LOCALMEM) {
    // uint num_events_in_wait_list
    Params.push_back(IntegerType::get(*Ctx, 32));
    // clk_event_t *in_wait_list
    Params.push_back(PointerType::get(getClkEventType(), 0));
    // clk_event_t *event_ret
    Params.push_back(PointerType::get(getClkEventType(), 0));
  }
  Params.push_back(getBlockLocalMemType()); // block invoke function pointer
  Params.push_back(getBlockLocalMemType()); // block literal pointer

  // local memory
  // uint localbuf_size_len
  Params.push_back(IntegerType::get(*Ctx, 32));
  // uint * localbuf_size
  Params.push_back(PointerType::get(getLocalMemBufType(), 0));
  Params.push_back(
      IAInfo->getWorkGroupInfoMemberType(NDInfo::RUNTIME_INTERFACE));
  Params.push_back(
      IAInfo->getWorkGroupInfoMemberType(NDInfo::BLOCK2KERNEL_MAPPER));
  Params.push_back(RuntimeHandle->getType());
  // create function type
  return FunctionType::get(getEnqueueKernelRetType(), Params, false);
}

void ResolveWICallPass::addExternFunctionDeclaration(unsigned Ty,
                                                     FunctionType *FTy,
                                                     StringRef FuncName) {
  // check declaration exists.
  if (ExtExecDecls.find(Ty) != ExtExecDecls.end())
    return;
  // create declaration.
  Function::Create(FTy, Function::ExternalLinkage, FuncName, M);
  // mark declaration is done.
  ExtExecDecls.insert(Ty);
}

unsigned ResolveWICallPass::getPointerSize() const {
  unsigned PointerSizeInBits = M->getDataLayout().getPointerSizeInBits(0);
  assert((32 == PointerSizeInBits || 64 == PointerSizeInBits) &&
         "Unsopported pointer size");
  return PointerSizeInBits;
}
