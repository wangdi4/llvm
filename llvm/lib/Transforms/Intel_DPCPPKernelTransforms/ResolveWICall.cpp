//===- ResolveWICall.cpp - Resolve DPC++ kernel work-item call ------------===//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/ResolveWICall.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/ImplicitArgsUtils.h"
#include <algorithm>

using namespace llvm;

#define DEBUG_TYPE "dpcpp-kernel-resolve-wi-call"

// Add command line to specify work groups size as uniform.
static cl::opt<bool> OptUniformWGSize(
    "dpcpp-uniform-wg-size", cl::init(false), cl::Hidden,
    cl::desc("The flag speficies work groups size as uniform"));

extern bool EnableTLSGlobals;

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
    AU.addPreserved<ImplicitArgsAnalysisLegacy>();
  }

private:
  ResolveWICallPass Impl;
  /// true if a module is compiled with the support of the non-uniform
  /// work-group size.
  bool IsUniformWG;
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
    : ModulePass(ID), IsUniformWG(IsUniformWG), UseTLSGlobals(UseTLSGlobals) {
  initializeResolveWICallLegacyPass(*PassRegistry::getPassRegistry());
}

bool ResolveWICallLegacy::runOnModule(Module &M) {
  CallGraph *CG = &getAnalysis<CallGraphWrapperPass>().getCallGraph();
  ImplicitArgsInfo *IAInfo =
      &getAnalysis<ImplicitArgsAnalysisLegacy>().getResult();
  return Impl.runImpl(M, IsUniformWG, UseTLSGlobals, IAInfo, CG);
}

ModulePass *llvm::createResolveWICallLegacyPass(bool IsUniformWGSize,
                                                bool UseTLSGlobals) {
  return new ResolveWICallLegacy(IsUniformWGSize, UseTLSGlobals);
}

PreservedAnalyses ResolveWICallPass::run(Module &M, ModuleAnalysisManager &AM) {
  CallGraph *CG = &AM.getResult<CallGraphAnalysis>(M);
  ImplicitArgsInfo *IAInfo = &AM.getResult<ImplicitArgsAnalysis>(M);
  if (!runImpl(M, IsUniformWG, UseTLSGlobals, IAInfo, CG))
    return PreservedAnalyses::all();
  PreservedAnalyses PA;
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
  this->IsUniformWG = IsUniformWG | OptUniformWGSize;
  this->UseTLSGlobals = UseTLSGlobals | EnableTLSGlobals;

  // extended execution flags
  ExtExecDecls.clear();

  OclVersion = CompilationUtils::fetchCLVersionFromMetadata(M);
  // Run on all defined function in the module
  for (Function &F : M) {
    if (F.isDeclaration()) {
      // Function is not defined inside module
      continue;
    }

    if (CompilationUtils::isGlobalCtorDtorOrCPPFunc(&F))
      continue;

    clearPerFunctionCache();
    runOnFunction(&F);
  }

  return true;
}

static LoadInst *createLoadForTLSGlobal(IRBuilder<> &Builder, Module *M,
                                        ImplicitArgsUtils::IMPLICIT_ARGS Arg) {
  auto TLSG = CompilationUtils::getTLSGlobal(M, Arg);
  assert(TLSG && "TLS Global cannot be nullptr");
  return Builder.CreateLoad(TLSG->getValueType(), TLSG);
}

Function *ResolveWICallPass::runOnFunction(Function *F) {
  this->F = F;
  Value *SpecialBuf = nullptr;
  IRBuilder<> Builder(F->getContext());
  if (UseTLSGlobals) {
    Builder.SetInsertPoint(dyn_cast<Instruction>(F->getEntryBlock().begin()));
    WorkInfo = createLoadForTLSGlobal(Builder, M,
                                      ImplicitArgsUtils::IA_WORK_GROUP_INFO);
    WGId =
        createLoadForTLSGlobal(Builder, M, ImplicitArgsUtils::IA_WORK_GROUP_ID);
    BaseGlbId = createLoadForTLSGlobal(Builder, M,
                                       ImplicitArgsUtils::IA_GLOBAL_BASE_ID);
    SpecialBuf = createLoadForTLSGlobal(Builder, M,
                                        ImplicitArgsUtils::IA_BARRIER_BUFFER);
    RuntimeHandle = createLoadForTLSGlobal(
        Builder, M, ImplicitArgsUtils::IA_RUNTIME_HANDLE);
  } else {
    CompilationUtils::getImplicitArgs(F, nullptr, &WorkInfo, &WGId, &BaseGlbId,
                                      &SpecialBuf, &RuntimeHandle);
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
        addExternFunctionDeclaration(CalledFuncType,
                                     getOrCreatePrintfFuncType(),
                                     CompilationUtils::nameOpenCLPrintf());
      NewRes = updatePrintf(Builder, CI);
      assert(NewRes && "Expected updatePrintf to succeed");
      break;
    case ICT_ENQUEUE_KERNEL_EVENTS_LOCALMEM:
    case ICT_ENQUEUE_KERNEL_LOCALMEM: {
      // TODO: It seems there is no need to handle these functions
      // separately, since they are not variadics anymore. Move their
      // implementation to built-in library
      std::string CallbackName = CalledFuncType == ICT_ENQUEUE_KERNEL_LOCALMEM
                                     ? "__ocl20_enqueue_kernel_localmem"
                                     : "__ocl20_enqueue_kernel_events_localmem";
      if (!ExtExecDecls.count(CalledFuncType)) {
        FunctionType *FT = getOrCreateEnqueueKernelFuncType(CalledFuncType);
        addExternFunctionDeclaration(CalledFuncType, FT, CallbackName);
      }
      // Copy original function operands
      SmallVector<Value *, 16> ExtExecArgs(CI->args());
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
  bool IsUserWIFunction = false;
  switch (CallType) {
  case ICT_GET_GLOBAL_OFFSET:
    return IAInfo->GenerateGetFromWorkInfo(
        NDInfo::internalCall2NDInfo(CallType), WorkInfo, CI->getArgOperand(0),
        Builder);
  case ICT_GET_GLOBAL_SIZE:
  case ICT_GET_NUM_GROUPS:
    return IAInfo->GenerateGetFromWorkInfo(
        NDInfo::internalCall2NDInfo(CallType, IsUserWIFunction), WorkInfo,
        CI->getArgOperand(0), Builder);
  case ICT_GET_LOCAL_SIZE:
    return IAInfo->GenerateGetLocalSize(IsUniformWG, WorkInfo, WGId,
                                        IsUserWIFunction, CI->getArgOperand(0),
                                        Builder);
  case ICT_GET_ENQUEUED_LOCAL_SIZE:
    return IAInfo->GenerateGetEnqueuedLocalSize(WorkInfo, IsUserWIFunction,
                                                CI->getArgOperand(0), Builder);
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

// This function creates printf argument buffer and stores argument size/value
// into the buffer.
//
// Memory layout of argument buffer that is passed to __opencl_printf builtin:
// =================================================================
// | Byte count | Name       | Introduction                        |
// =================================================================
// | 4          | BufferSize | Size of the buffer in bytes         |
// |===============================================================| _
// | 4          | Size       | Size of arg 1 element and DummyB    |  |
// |---------------------------------------------------------------|  | Second
// | A few      | DummyB     | Space to ensure ArgValue is aligned |  | Argument
// |---------------------------------------------------------------|  |
// | ArgSize    | ArgValue   | Value of arg 1                      | _|
// |===============================================================| _
// | 0-3        | DummyA     | Space to ensure Size is aligned  |  |
// |---------------------------------------------------------------|  |
// | 4          | Size       | Size of arg 2 element and DummyB    |  | Third
// |---------------------------------------------------------------|  | Argument
// | A few      | DummyB     | Space to ensure ArgValue is aligned |  |
// |---------------------------------------------------------------|  |
// | ArgSize    | ArgValue   | Value of arg 2                      | _|
// |===============================================================|
// | ...                                                           |
// | ...                                                           |
// | ...                                                           |
// |===============================================================| _
// | 0-3        | DummyA     | Space to ensure Size is aligned  |  |
// |---------------------------------------------------------------|  |
// | 4          | Size       | Size of arg N element and DummyB    |  | Last
// |---------------------------------------------------------------|  | Argument
// | A few      | DummyB     | Space to ensure ArgValue is aligned |  |
// |---------------------------------------------------------------|  |
// | ArgSize    | ArgValue   | Value of arg N                      | _|
// |===============================================================|
// =================================================================
//
// Notes:
// * The first argument (index 0), which is format string, isn't stored to
//   argument buffer. Therefore, argument index starts with '1' and 'second'.
// * 'Size' contains 'DummyB' size, which is used to align argument buffer.
//   Memory layout of 'Size':
//   ===================================
//   |0               |16              | Bit
//   |---------------- ----------------|
//   |Arg element size|  DummyB size   |
//   ===================================
// * For vector argument, its element size is store in 'Size'.
//   'DummyB' ensures that 'ArgValue' is aligned to its element size.
//   __opencl_printf builtin uses format field to identify number of elements.
Value *ResolveWICallPass::updatePrintf(IRBuilder<> &Builder, CallInst *CI) {
  assert(RuntimeHandle && "Context pointer RuntimeHandle created as expected");
  const DataLayout &DL = M->getDataLayout();
  // Types used in several places.
  IntegerType *I32Ty = IntegerType::getInt32Ty(*Ctx);
  PointerType *I32PtrTy = PointerType::getUnqual(I32Ty);
  unsigned I32TySize = 4;

  // Find out the buffer size required to store all the arguments.
  // Note: CallInst->getNumOperands() returns the number of operands in
  // the instruction, including its destination as #0. Since this is
  // a printf call and we're interested in all the arguments after the
  // format string, we start with #1.
  assert(CI->arg_size() > 0 &&
         "Expect printf to have a format string");
  SmallVector<unsigned, 16> ArgEltSizes;
  SmallVector<unsigned, 16> Sizes;
  // The first 4 bytes stores total size which is used for out-of-bound check
  // in __opencl_printf builtin implementation.
  unsigned TotalArgSize = I32TySize;
  auto AlignSizeTo = [](unsigned Size, unsigned Align) {
    return (Size + Align - 1) & ~(Align - 1);
  };
  for (unsigned I = 1, E = CI->arg_size(); I != E; ++I) {
    auto *ArgTy = CI->getArgOperand(I)->getType();

    // Offset to store argument size. For vector type, use element type size.
    const unsigned SizeOffset = AlignSizeTo(TotalArgSize, I32TySize);
    const unsigned DummyOffset = SizeOffset + I32TySize;
    unsigned ArgEltSize;
    if (auto *VTy = dyn_cast<VectorType>(ArgTy))
      ArgEltSize = DL.getTypeAllocSize(VTy->getElementType());
    else
      ArgEltSize = DL.getTypeAllocSize(ArgTy);
    ArgEltSizes.push_back(ArgEltSize);
    assert(ArgEltSize <= std::numeric_limits<uint16_t>::max() &&
           "arg element size too large");

    // Offset to store current argument value.
    const unsigned ArgValueOffset = AlignSizeTo(DummyOffset, ArgEltSize);

    // Compute DummyB size.
    unsigned DummySize = ArgValueOffset - DummyOffset;
    assert(DummySize <= std::numeric_limits<uint16_t>::max() &&
           "dummy size too large");

    // Combine argument size and DummyB size.
    Sizes.push_back(ArgEltSize | (DummySize << 16));

    TotalArgSize = ArgValueOffset + DL.getTypeAllocSize(ArgTy);
  }

  // Create the alloca instruction for allocating the buffer on the stack.
  // For the special case where printf got no vararg arguments: printf("hello"),
  // only TotalArgSize (equals I32TySize) will be stored to the buffer.
  auto *BufArrType = ArrayType::get(IntegerType::getInt8Ty(*Ctx), TotalArgSize);
  // Alloca buffer to store size and arguments. This buffer will be parsed by
  // __opencl_printf builtin.
  auto *BufAI =
      new AllocaInst(BufArrType, DL.getAllocaAddrSpace(), "temp_arg_buf",
                     &*CI->getFunction()->getEntryBlock().begin());
  BufAI->setAlignment(Align(I32TySize));

  // Generate instructions to store sizes and operands into the argument buffer.
  Builder.SetInsertPoint(CI);
  unsigned BufPointerOffset = 0;
  SmallVector<Value *, 2> IndexArgs(2);
  IndexArgs[0] = getConstZeroInt32Value();

  auto CreateGEPCastStore = [&](unsigned Offset, Type *DestTy, StringRef Name,
                                Value *V, Optional<Align> Alignment = None) {
    IndexArgs[1] = ConstantInt::get(I32Ty, Offset);
    auto *GEP = Builder.CreateInBoundsGEP(BufArrType, BufAI, IndexArgs);
    auto *Cast = Builder.CreatePointerCast(GEP, DestTy, Name);
    if (Alignment)
      Builder.CreateAlignedStore(V, Cast, *Alignment);
    else
      Builder.CreateStore(V, Cast);
    return GEP;
  };

  // Store total size.
  // Get a pointer to the buffer, in order to pass it to __opencl_printf
  // function.
  auto *PtrToBuf =
      CreateGEPCastStore(BufPointerOffset, I32PtrTy, "arg_buf_size",
                         ConstantInt::get(I32Ty, TotalArgSize));
  BufPointerOffset += I32TySize;

  for (unsigned I = 1, E = CI->arg_size(); I != E; ++I) {
    Value *Arg = CI->getArgOperand(I);
    auto *ArgTy = Arg->getType();

    // Store current argument size.
    const unsigned SizeOffset = AlignSizeTo(BufPointerOffset, I32TySize);
    CreateGEPCastStore(SizeOffset, I32PtrTy, "arg_size",
                       ConstantInt::get(I32Ty, Sizes[I - 1]));

    // Store current argument.
    // Compute the address into which this argument will be placed.
    const unsigned DummyOffset = SizeOffset + I32TySize;
    const unsigned ArgValueOffset =
        AlignSizeTo(DummyOffset, ArgEltSizes[I - 1]);
    CreateGEPCastStore(ArgValueOffset, PointerType::getUnqual(ArgTy), "arg_val",
                       Arg, Align(1));

    // This Argument occupied some space in the buffer.
    // Advance the buffer pointer offset by its size to know where the next
    // Argument should be placed.
    BufPointerOffset = ArgValueOffset + DL.getTypeAllocSize(ArgTy);
  }

  // Finally create the call to __opencl_printf.
  Function *F = M->getFunction(CompilationUtils::nameOpenCLPrintf());
  assert(F && "Expect builtin printf to be declared before use");

  SmallVector<Value *, 4> Params;
  Params.push_back(CI->getArgOperand(0));
  Params.push_back(PtrToBuf);
  Value *RuntimeInterface = getOrCreateRuntimeInterface();
  Params.push_back(RuntimeInterface);
  Params.push_back(RuntimeHandle);
  auto *Res = Builder.CreateCall(F, Params, "translated_opencl_printf_call");
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
  Function *Prefetch = M->getFunction("__lprefetch");
  assert(Prefetch && "Missing '__lprefetch' function");
  CallInst::Create(Prefetch, ArrayRef<Value *>(Params), "", CI);
}

FunctionType *ResolveWICallPass::getOrCreatePrintfFuncType() {
  // The prototype of __opencl_printf is:
  // int __opencl_printf(__constant char *format, char *args, void *Callback,
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
  Function::Create(NewType, Function::ExternalLinkage, "__lprefetch", M);

  PrefetchDecl = true;
}

TInternalCallType ResolveWICallPass::getCallFunctionType(StringRef FuncName) {
  if (FuncName == CompilationUtils::nameGetBaseGID())
    return ICT_GET_BASE_GLOBAL_ID;
  if (CompilationUtils::isGetSpecialBuffer(FuncName))
    return ICT_GET_SPECIAL_BUFFER;
  if (CompilationUtils::isGetWorkDim(FuncName))
    return ICT_GET_WORK_DIM;
  if (CompilationUtils::isGetGlobalSize(FuncName))
    return ICT_GET_GLOBAL_SIZE;
  if (CompilationUtils::isGetNumGroups(FuncName))
    return ICT_GET_NUM_GROUPS;
  if (CompilationUtils::isGetGroupId(FuncName))
    return ICT_GET_GROUP_ID;
  if (CompilationUtils::isGlobalOffset(FuncName))
    return ICT_GET_GLOBAL_OFFSET;
  // special built-ins that need to update.
  if (CompilationUtils::isPrintf(FuncName))
    return ICT_PRINTF;
  if (CompilationUtils::isPrefetch(FuncName))
    return ICT_PREFETCH;

  // OpenCL 2.0/3.0 built-ins to resolve.
  if (OclVersion >= CompilationUtils::OclVersion::CL_VER_2_0) {
    if (CompilationUtils::isEnqueueKernelLocalMem(FuncName))
      return ICT_ENQUEUE_KERNEL_LOCALMEM;
    if (CompilationUtils::isEnqueueKernelEventsLocalMem(FuncName))
      return ICT_ENQUEUE_KERNEL_EVENTS_LOCALMEM;
    if (CompilationUtils::isGetLocalSize(FuncName))
      return ICT_GET_LOCAL_SIZE;
    if (CompilationUtils::isGetEnqueuedLocalSize(FuncName))
      return ICT_GET_ENQUEUED_LOCAL_SIZE;
  } else {
    // built-ins which behavior is different in OpenCL versions older than 2.0.
    if (CompilationUtils::isGetLocalSize(FuncName))
      return ICT_GET_ENQUEUED_LOCAL_SIZE;
  }
  return ICT_NONE;
}

// address space generated by clang for queue_t, clk_event_t, ndrange_t types.
const int EXTEXEC_OPAQUE_TYPES_ADDRESS_SPACE =
    CompilationUtils::ADDRESS_SPACE_GLOBAL;
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
                          CompilationUtils::ADDRESS_SPACE_GENERIC);
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

// The prototype of __ocl20_enqueue_kernel_events is:
// int __ocl20_enqueue_kernel_events_localmem(
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
