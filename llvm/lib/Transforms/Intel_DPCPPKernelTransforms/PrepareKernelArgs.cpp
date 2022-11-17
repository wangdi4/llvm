//===- PrepareKernelArgs.cpp - Prepare DPC++ kernel arguments -------------===//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/PrepareKernelArgs.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/ValueHandle.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/ImplicitArgsUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/TypeAlignment.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"

using namespace llvm;

#define DEBUG_TYPE "dpcpp-kernel-prepare-args"

extern bool EnableTLSGlobals;

namespace {

uint64_t STACK_PADDING_BUFFER = DEV_MAXIMUM_ALIGN * 1;

class PrepareKernelArgsLegacy : public ModulePass {

public:
  /// Pass identification, replacement for typeid.
  static char ID;

  PrepareKernelArgsLegacy(bool UseTLSGlobals = false);

  StringRef getPassName() const override { return "PrepareKernelArgsLegacy"; }

  bool runOnModule(Module &M) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<AssumptionCacheTracker>();
    AU.addRequired<ImplicitArgsAnalysisLegacy>();
  }

private:
  PrepareKernelArgsPass Impl;
  bool UseTLSGlobals;
};

} // namespace

INITIALIZE_PASS_BEGIN(PrepareKernelArgsLegacy, DEBUG_TYPE,
                      "Change the way arguments are passed to kernels", false,
                      false)
INITIALIZE_PASS_DEPENDENCY(AssumptionCacheTracker)
INITIALIZE_PASS_DEPENDENCY(ImplicitArgsAnalysisLegacy)
INITIALIZE_PASS_END(PrepareKernelArgsLegacy, DEBUG_TYPE,
                    "Change the way arguments are passed to kernels", false,
                    false)

char PrepareKernelArgsLegacy::ID = 0;

PrepareKernelArgsLegacy::PrepareKernelArgsLegacy(bool UseTLSGlobals)
    : ModulePass(ID), UseTLSGlobals(UseTLSGlobals) {
  initializePrepareKernelArgsLegacyPass(*PassRegistry::getPassRegistry());
}

bool PrepareKernelArgsLegacy::runOnModule(Module &M) {
  auto GetAC = [&](Function &F) {
    return &getAnalysis<AssumptionCacheTracker>().getAssumptionCache(F);
  };
  ImplicitArgsInfo *IAInfo =
      &getAnalysis<ImplicitArgsAnalysisLegacy>().getResult();
  return Impl.runImpl(M, UseTLSGlobals, GetAC, IAInfo);
}

ModulePass *llvm::createPrepareKernelArgsLegacyPass(bool UseTLSGlobals) {
  return new PrepareKernelArgsLegacy(UseTLSGlobals);
}

PreservedAnalyses PrepareKernelArgsPass::run(Module &M,
                                             ModuleAnalysisManager &AM) {
  FunctionAnalysisManager &FAM =
      AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto GetAC = [&](Function &F) {
    return &FAM.getResult<AssumptionAnalysis>(F);
  };
  ImplicitArgsInfo *IAInfo = &AM.getResult<ImplicitArgsAnalysis>(M);
  if (!runImpl(M, UseTLSGlobals, GetAC, IAInfo))
    return PreservedAnalyses::all();
  return PreservedAnalyses::none();
}

bool PrepareKernelArgsPass::runImpl(
    Module &M, bool UseTLSGlobals,
    function_ref<AssumptionCache *(Function &F)> GetAC,
    ImplicitArgsInfo *IAInfo) {
  this->M = &M;
  this->UseTLSGlobals = UseTLSGlobals | EnableTLSGlobals;
  this->IAInfo = IAInfo;
  LLVMContext &C = M.getContext();
  SizetTy = IntegerType::get(C, M.getDataLayout().getPointerSizeInBits());
  I32Ty = Type::getInt32Ty(C);
  I8Ty = Type::getInt8Ty(C);

  // Get all kernels (original scalar kernels and vectorized kernels).
  auto kernelsFuncSet = CompilationUtils::getAllKernels(*this->M);

  // Handle all kernels.
  for (auto *F : kernelsFuncSet)
    runOnFunction(F, GetAC(*F));

  return !kernelsFuncSet.empty();
}

Function *PrepareKernelArgsPass::createWrapper(Function *F) {
  // Create new function's argument type list
  SmallVector<Type *, 3> NewArgsVec;
  // The new function receives the following arguments:
  // i8* pBuffer
  NewArgsVec.push_back(PointerType::get(I8Ty, 0));
  // GID argument
  NewArgsVec.push_back(IAInfo->getArgType(ImplicitArgsUtils::IA_WORK_GROUP_ID));
  // Runtime context
  NewArgsVec.push_back(
      IAInfo->getArgType(ImplicitArgsUtils::IA_RUNTIME_HANDLE));
  // Create new functions return type
  FunctionType *FTy =
      FunctionType::get(F->getReturnType(), NewArgsVec, /*isVarArg*/ false);

  // Create a new function
  Function *NewF = Function::Create(FTy, F->getLinkage(), F->getName(), M);
  NewF->setCallingConv(F->getCallingConv());
  NewF->copyMetadata(F, 0);
  NewF->setDSOLocal(F->isDSOLocal());

  // Add comdat to newF and drop from F.
  NewF->setComdat(F->getComdat());
  F->setComdat(nullptr);

  // Copy attributes from the old kernel to the new one.
  auto FnAttrs = F->getAttributes().getAttributes(AttributeList::FunctionIndex);
  AttrBuilder B(F->getContext(), std::move(FnAttrs));
  NewF->addFnAttrs(B);

  return NewF;
}

std::vector<Value *> PrepareKernelArgsPass::createArgumentLoads(
    IRBuilder<> &Builder, Function *WrappedKernel, Argument *ArgsBuffer,
    Argument *WGId, Argument *RuntimeContext) {
  // Get old function's arguments list in the OpenCL level from its metadata
  std::vector<KernelArgument> Arguments;
  std::vector<unsigned int> memoryArguments;
  CompilationUtils::parseKernelArguments(M, WrappedKernel, UseTLSGlobals,
                                         Arguments, memoryArguments);

  std::vector<Value *> Params;
  Function::arg_iterator CallIt = WrappedKernel->arg_begin();
  DPCPPKernelMetadataAPI::KernelInternalMetadataAPI KIMD(WrappedKernel);

  const DataLayout &DL = M->getDataLayout();
  // TODO :  get common code from the following 2 for loops into a function
  // Handle explicit arguments
  Type *ArgsBufferElementTy =
      ArgsBuffer->getType()->getScalarType()->getPointerElementType();
  for (unsigned ArgNo = 0; ArgNo < Arguments.size(); ++ArgNo) {
    KernelArgument KArg = Arguments[ArgNo];
    //  %0 = getelementptr i8* %pBuffer, i32 CurrOffset
    Value *GEP = Builder.CreateGEP(ArgsBufferElementTy, ArgsBuffer,
                                   ConstantInt::get(I32Ty, KArg.OffsetInBytes));

    Value *Arg;

    if (KArg.Ty == KRNL_ARG_COMPOSITE || KArg.Ty == KRNL_ARG_VECTOR_BY_REF) {
      // If this is a struct argument, then the struct itself is passed by value
      // inside ArgsBuffer and the original kernel signature was: foo(...,
      // MyStruct* byval myStruct, ...) meaning GEP already points to the
      // structure and we do not need to load it we just need to have a bitcast
      // from i8* to MyStruct* and pass the pointer (!!!) to foo

      // %myStruct = bitcast i8* to MyStruct*
      // foo(..., %myStruct, ...)

      Value *PointerCast = Builder.CreatePointerCast(GEP, CallIt->getType());
      Arg = PointerCast;
    } else if (KArg.Ty == KRNL_ARG_PTR_LOCAL) {
      // The argument is actually the size of the buffer
      Value *PointerCast =
          Builder.CreatePointerCast(GEP, PointerType::get(SizetTy, 0));
      LoadInst *BufferSize = Builder.CreateAlignedLoad(
          SizetTy, PointerCast, MaybeAlign(DL.getABITypeAlign(SizetTy)), false);
      // TODO: when buffer size is 0, we might want to set dummy address for
      // debugging!
      const auto AllocaAddrSpace = DL.getAllocaAddrSpace();

      // Set alignment of buffer to type size.
      unsigned Alignment = 16; // Cacheline
      Type *EltTy = CallIt->getType()->getPointerElementType();
      // If the kernel was vectorized, choose an alignment that is good for the
      // *vectorized* type. This can be good for unaligned loads on targets that
      // support instructions such as MOVUPS
      unsigned VecSize =
          KIMD.VectorizedWidth.hasValue() ? KIMD.VectorizedWidth.get() : 1;
      if (VecSize != 1 && VectorType::isValidElementType(EltTy))
        EltTy = FixedVectorType::get(EltTy, VecSize);
      Alignment = NextPowerOf2(DL.getTypeAllocSize(EltTy) - 1);
      // We can't use overload without explicit alloca addrspace because the
      // BB does not have a parent yet.
      AllocaInst *Allocation =
          new AllocaInst(I8Ty, AllocaAddrSpace, BufferSize, Align(Alignment));
      Builder.Insert(Allocation);
      Arg = Builder.CreatePointerCast(Allocation, CallIt->getType());
    } else if (KArg.Ty == KRNL_ARG_PTR_BLOCK_LITERAL) {
      Arg = Builder.CreateAddrSpaceCast(GEP, CallIt->getType());
    } else {
      // Otherwise this is some other type, lets say int4, then int4 itself is
      // passed by value inside ArgsBuffer and the original kernel signature
      // was: foo(..., int4 vec, ...) meaning GEP points to int4 and we just
      // need to have a bitcast from i8* to int4* load the int4 and pass the
      // loaded value (!!!) to foo

      // %pVec = bitcast i8* %0 to int4 *
      // %vec = load int4 * %pVec {, align <alignment> }
      // foo(..., vec, ...)

      auto *DestTy = PointerType::get(CallIt->getType(), 0);
      Value *PointerCast = Builder.CreatePointerCast(GEP, DestTy);
      size_t A = TypeAlignment::getAlignment(KArg);
      MaybeAlign MA = A > 0
                          ? MaybeAlign(A)
                          : DL.getABITypeAlign(CallIt->getType());
      LoadInst *LI = Builder.CreateAlignedLoad(CallIt->getType(),
                                               PointerCast, MA, false);
      Arg = LI;
    }

    // Here we mark the load instructions from the struct that are the actual
    // parameters for the original kernel's restricted formal parameters This
    // info is used later on in OpenCLAliasAnalysis to overcome the fact that
    // inlining does not maintain the restrict information.
    Instruction *ArgInst = cast<Instruction>(Arg);
    if (WrappedKernel->getAttributes().hasAttributeAtIndex(ArgNo + 1,
                                                          Attribute::NoAlias)) {
      ArgInst->setMetadata("restrict", MDNode::get(M->getContext(), 0));
    }

    // TODO: Maybe get arg name from metadata?
    SmallString<16> NameStorage;
    Arg->setName((Twine("explicit_") + Twine(ArgNo)).toStringRef(NameStorage));
    Params.push_back(Arg);

    ++CallIt;
  }

  // Offset to after last explicit argument + adjusted alignment
  // Believe it or not, the conformance has a test kernel with 0 args...
  size_t CurrOffset = 0;
  if (!Arguments.empty()) {
    CurrOffset = Arguments.back().OffsetInBytes +
                 TypeAlignment::getSize(Arguments.back());
    CurrOffset = ImplicitArgsUtils::getAdjustedAlignment(
        CurrOffset, DL.getPointerABIAlignment(DL.getAllocaAddrSpace()).value());
  }
  // Handle implicit arguments
  // Set to the Work Group Info implicit arg, as soon as it is known. Used for
  // computing other arg values
  Value *WGInfo = 0;
  // LocalSize for each dimension. Used several times below.
  SmallVector<Value *, 4> LocalSize;
  unsigned PtrSizeInBytes = M->getDataLayout().getPointerSize();
  ImplicitArgsUtils::initImplicitArgProps(PtrSizeInBytes);
  for (unsigned int I = 0; I < ImplicitArgsUtils::NUM_IMPLICIT_ARGS; ++I) {
    Value *Arg = nullptr;
    if (!UseTLSGlobals)
      assert(CallIt->getType() == IAInfo->getArgType(I) &&
             "Mismatch in arg found in function and expected arg type");
    assert((I == ImplicitArgsUtils::IA_SLM_BUFFER ||
            I == ImplicitArgsUtils::IA_WORK_GROUP_ID ||
            I == ImplicitArgsUtils::IA_RUNTIME_HANDLE ||
            I == ImplicitArgsUtils::IA_GLOBAL_BASE_ID ||
            I == ImplicitArgsUtils::IA_BARRIER_BUFFER ||
            I == ImplicitArgsUtils::IA_WORK_GROUP_INFO) &&
           "Invalid implicit argument index!");
    switch (I) {
    case ImplicitArgsUtils::IA_SLM_BUFFER: {
      uint64_t SLMSizeInBytes =
          KIMD.LocalBufferSize.hasValue() ? KIMD.LocalBufferSize.get() : 0;
      // TODO: when SLMSizeInBytes equal 0, we might want to set dummy
      // address for debugging!
      if (SLMSizeInBytes == 0) { // no need to create of pad this buffer.
        Arg = Constant::getNullValue(PointerType::get(I8Ty, 3));
      } else {
        // add stack padding before and after this alloca, to allow unmasked
        // wide loads inside the vectorizer.
        Type *SLMType =
            ArrayType::get(I8Ty, SLMSizeInBytes + STACK_PADDING_BUFFER * 2);
        const auto AllocaAddrSpace = DL.getAllocaAddrSpace();
        // Set alignment of implicit local buffer to max alignment.
        // TODO: we should choose the min required alignment size
        // move argument up over the lower side padding.
        AllocaInst *slmBuffer =
            new AllocaInst(SLMType, AllocaAddrSpace, nullptr,
                           Align(TypeAlignment::MAX_ALIGNMENT));
        Builder.Insert(slmBuffer);
        Value *CastBuf =
            Builder.CreatePointerCast(slmBuffer, PointerType::get(I8Ty, 3));
        Arg = Builder.CreateGEP(I8Ty, CastBuf,
                                ConstantInt::get(I32Ty, STACK_PADDING_BUFFER));
      }
    } break;
    case ImplicitArgsUtils::IA_WORK_GROUP_ID:
      // WGID is passed by value as an argument to the wrapper
      assert(IAInfo->getArgType(I) == WGId->getType() && "Unmatching types");
      Arg = WGId;
      break;
    case ImplicitArgsUtils::IA_RUNTIME_HANDLE:
      // Runtime Context is passed by value as an argument to the wrapper
      assert(IAInfo->getArgType(I) == RuntimeContext->getType() &&
             "Unmatching types");
      Arg = RuntimeContext;
      break;
    case ImplicitArgsUtils::IA_GLOBAL_BASE_ID: {
      assert(WGInfo && "WGInfo should have already been initialized");
      // Obtain values of Local Size for each dimension
      assert(LocalSize.empty() &&
             "Assuming that we are computing Local Sizes here");
      for (unsigned Dim = 0; Dim < MAX_WORK_DIM; ++Dim)
        LocalSize.push_back(
            IAInfo->GenerateGetEnqueuedLocalSize(WGInfo, false, Dim, Builder));

      // Obtain values of NDRange Offsets for each dimension
      SmallVector<Value *, 4> GlobalOffsets;
      for (unsigned Dim = 0; Dim < MAX_WORK_DIM; ++Dim) {
        GlobalOffsets.push_back(
            IAInfo->GenerateGetGlobalOffset(WGInfo, Dim, Builder));
      }
      // Obtain values of group ID for each dimension
      SmallVector<Value *, 4> GroupIDs;
      for (unsigned Dim = 0; Dim < MAX_WORK_DIM; ++Dim)
        GroupIDs.push_back(IAInfo->GenerateGetGroupID(WGId, Dim, Builder));
      // Compute the required value:
      // GlobalBaseId[i] = GroupId[i]*LocalSize[i]+GlobalOffset[i]
      SmallVector<Value *, 4> Computes;
      for (unsigned Dim = 0; Dim < MAX_WORK_DIM; ++Dim) {
        Value *V = Builder.CreateBinOp(Instruction::Mul, LocalSize[Dim],
                                       GroupIDs[Dim]);
        V = Builder.CreateBinOp(Instruction::Add, V, GlobalOffsets[Dim]);
        Computes.push_back(V);
      }
      // Collect all values to single array
      Value *U = UndefValue::get(IAInfo->getArgType(I));
      for (unsigned Dim = 0; Dim < MAX_WORK_DIM; ++Dim) {
        U = Builder.CreateInsertValue(U, Computes[Dim],
                                      ArrayRef<unsigned>(Dim));
      }
      Arg = U;
    } break;
    case ImplicitArgsUtils::IA_BARRIER_BUFFER: {
      // We obtain the number of bytes needed per item from the Metadata
      // which is set by the Barrier pass
      uint64_t SizeInBytes =
          KIMD.BarrierBufferSize.hasValue() ? KIMD.BarrierBufferSize.get() : 0;
      // BarrierBufferSize := BytesNeededPerWI
      //                      * ((LocalSize(0) + VF - 1) / VF) * VF
      //                      * LocalSize(1) * LocalSize(2)
      Value *BarrierBufferSize = ConstantInt::get(SizetTy, SizeInBytes);
      assert(WGInfo && "Work Group Info was not initialized");
      assert(!LocalSize.empty() &&
             "Local group sizes are assumed to be computed already");

      // If sub-group emulation OR vectorization happens, we may store a <VF
      // x Ty> vector to special buffer in the last iteration of barrier
      // loop even if there are only part of WIs active (WG_Size % SG_Size
      // != 0). So we need to round the special buffer size on vec / emu dim
      // to multiple of vec / emu size. We don't check whether the
      // Work-Group is vectorized with tail here, in such cases, this action
      // may waste a little memory.
      Value *LocalSizeProd = LocalSize[0];
      unsigned VF =
          KIMD.VectorizedWidth.hasValue() ? KIMD.VectorizedWidth.get() : 1;
      if (VF > 1) {
        Value *VFValue = ConstantInt::get(SizetTy, VF);
        Value *VFMinus1 = ConstantInt::get(SizetTy, VF - 1);
        Value *Sum = Builder.CreateAdd(LocalSizeProd, VFMinus1, "",
                                       /*HasNUW*/ true, /*HasNSW*/ true);
        LocalSizeProd = Builder.CreateAnd(Sum, Builder.CreateNeg(VFValue),
                                          "RoundUpToMultipleVF");
      }
      for (unsigned Dim = 1; Dim < MAX_WORK_DIM; ++Dim)
        LocalSizeProd = Builder.CreateMul(LocalSizeProd, LocalSize[Dim], "",
                                          /*HasNUW*/ true, /*HasNSW*/ true);
      LocalSizeProd->setName("LocalSizeProd");
      BarrierBufferSize = Builder.CreateMul(BarrierBufferSize, LocalSizeProd,
                                            "BarrierBufferSize",
                                            /*HasNUW*/ true, /*HasNSW*/ true);

      // alloca i8, %BarrierBufferSize
      const auto AllocaAddrSpace = DL.getAllocaAddrSpace();
      // TODO: we should choose the min required alignment size
      AllocaInst *BarrierBuffer =
          new AllocaInst(I8Ty, AllocaAddrSpace, BarrierBufferSize,
                         Align(TypeAlignment::MAX_ALIGNMENT));
      Builder.Insert(BarrierBuffer);
      Arg = BarrierBuffer;
      LLVM_DEBUG(CompilationUtils::insertPrintf("SPECIAL BUFFER: ", Builder,
                                                BarrierBuffer));
      LLVM_DEBUG(CompilationUtils::insertPrintf(
          "SPECIAL BUFFER SIZE: ", Builder, BarrierBufferSize));
    } break;
    case ImplicitArgsUtils::IA_WORK_GROUP_INFO: {
      // These values are pointers that just need to be loaded from the
      // UniformKernelArgs structure and passed on to the kernel
      const ImplicitArgProperties &ImplicitArgProp =
          ImplicitArgsUtils::getImplicitArgProps(I);
      // %0 = getelementptr i8* %pBuffer, i32 CurrOffset
      Value *GEP = Builder.CreateGEP(ArgsBufferElementTy, ArgsBuffer,
                                     ConstantInt::get(I32Ty, CurrOffset));
      Arg = Builder.CreatePointerCast(GEP, IAInfo->getArgType(I));
      WGInfo = Arg;
      // Advance the ArgsBuffer offset based on the size
      CurrOffset += ImplicitArgProp.Size;
    } break;
    }

    if (UseTLSGlobals) {
      assert(Arg && "No value was created for this TLS global!");
      GlobalVariable *GV = CompilationUtils::getTLSGlobal(M, I);
      Builder.CreateAlignedStore(Arg, GV, DL.getABITypeAlign(Arg->getType()));
    } else {
      assert(Arg && "No value was created for this implicit argument!");
      Arg->setName(ImplicitArgsUtils::getArgName(I));
      Params.push_back(Arg);
      ++CallIt;
    }
  }

  return Params;
}

CallInst *PrepareKernelArgsPass::createWrapperBody(Function *Wrapper,
                                                   Function *WrappedKernel) {
  // Set new function's argument name
  Function::arg_iterator DestI = Wrapper->arg_begin();
  DestI->setName("UniformArgs");
  DestI->addAttr(Attribute::NoAlias);
  Argument *ArgsBuffer = &*(DestI++);
  DestI->setName("pWGId");
  DestI->addAttr(Attribute::NoAlias);
  Argument *WGId = &*(DestI++);
  DestI->setName("RuntimeHandle");
  DestI->addAttr(Attribute::NoAlias);
  Argument *RuntimeContext = &*(DestI++);
  assert(DestI == Wrapper->arg_end() && "Expected to be past last arg");

  // Create wrapper function
  LLVMContext &C = M->getContext();
  BasicBlock *BB = BasicBlock::Create(C, "wrapper_entry", Wrapper);
  IRBuilder<> Builder(BB);
  std::vector<Value *> Params = createArgumentLoads(
      Builder, WrappedKernel, ArgsBuffer, WGId, RuntimeContext);

  CallInst *CI = Builder.CreateCall(WrappedKernel, ArrayRef<Value *>(Params));
  CI->setCallingConv(WrappedKernel->getCallingConv());

  return CI;
}

void PrepareKernelArgsPass::replaceFunctionPointers(Function *Wrapper,
                                                    Function *WrappedKernel) {
  // BIs like enqueue_kernel and kernel query have a function pointer to a
  // block invoke kernel as an argument.
  // Replace these arguments by a pointer to the wrapper function.
  IRBuilder<> Builder(WrappedKernel->getContext());
  for (auto &EEF : *M) {
    if (!EEF.isDeclaration())
      continue;

    StringRef EEFName = EEF.getName();
    if (!(EEFName.startswith("__ocl20_enqueue_kernel_") ||
          EEFName.equals("__ocl20_get_kernel_wg_size") ||
          EEFName.equals("__ocl20_get_kernel_preferred_wg_size_multiple")))
      continue;

    unsigned BlockInvokeIdx = (EEFName.startswith("__ocl20_enqueue_kernel_"))
                                  ? (EEFName.contains("_events") ? 6 : 3)
                                  : 0;

    for (auto *U : EEF.users()) {
      if (auto *EECall = dyn_cast<CallInst>(U)) {
        Value *BlockInvoke =
            EECall->getArgOperand(BlockInvokeIdx)->stripPointerCasts();
        if (BlockInvoke != WrappedKernel)
          continue;
        auto *Int8PtrTy =
            PointerType::get(IntegerType::getInt8Ty(M->getContext()),
                             CompilationUtils::ADDRESS_SPACE_GENERIC);
        Builder.SetInsertPoint(EECall);
        auto *NewCast = Builder.CreatePointerCast(Wrapper, Int8PtrTy);
        EECall->setArgOperand(BlockInvokeIdx, NewCast);
      }
    }
  }

  for (User *U : WrappedKernel->users()) {
    // Replace bitcast operator user, e.g. in global value.
    if (auto *Op = dyn_cast<BitCastOperator>(U)) {
      auto *WrapperOp = ConstantExpr::getBitCast(Wrapper, Op->getDestTy());
      Op->replaceAllUsesWith(WrapperOp);
    } else if (auto *Op = dyn_cast<SelectInst>(U)) {
      // WrappedKernel is used as select instruction operand
      auto *WrapperOp =
          ConstantExpr::getBitCast(Wrapper, Op->getOperand(1)->getType());
      if (Op->getOperand(1)->getName() == WrappedKernel->getName())
        Op->setOperand(1, WrapperOp);
      else
        Op->setOperand(2, WrapperOp);
    }
  }
}

void PrepareKernelArgsPass::createDummyRetWrappedKernel(Function *Wrapper,
                                                        Function *F) {
  DebugLoc Loc;
  for (auto &BB : *Wrapper)
    if (auto *RI = dyn_cast<ReturnInst>(BB.getTerminator()))
      Loc = RI->getDebugLoc();

  auto *BB = BasicBlock::Create(F->getContext(), "", F);
  auto *RI = ReturnInst::Create(F->getContext(), BB);
  // FIXME this debug info doesn't make sense. We can remove this after removing
  // kernel wrapper [CMPLRLLVM-15348].
  RI->setDebugLoc(Loc);
}

/// This is modified from lib/Transforms/Utils/InlineFunction.cpp.
/// For byval argument, make the implicit memcpy explicit by adding it.
static Value *HandleByValArgument(Type *ByValType, Value *Arg,
                                  Instruction *TheCall,
                                  const Function *CalledFunc,
                                  unsigned ByValAlignment) {
  assert(cast<PointerType>(Arg->getType())
             ->isOpaqueOrPointeeTypeMatches(ByValType));
  Function *Caller = TheCall->getFunction();
  const DataLayout &DL = Caller->getParent()->getDataLayout();

  // Create the alloca.  If we have DataLayout, use nice alignment.
  Align Alignment(DL.getPrefTypeAlignment(ByValType));

  // If the byval had an alignment specified, we *must* use at least that
  // alignment, as it is required by the byval argument (and uses of the
  // pointer inside the callee).
  Alignment = std::max(Alignment, MaybeAlign(ByValAlignment).valueOrOne());

  Value *NewAlloca = new AllocaInst(ByValType, DL.getAllocaAddrSpace(), nullptr,
                                    Alignment, Arg->getName() + Twine(".ptr"),
                                    &*Caller->begin()->begin());

  // If the byval was in a different address space, add a cast.
  PointerType *ArgTy = cast<PointerType>(Arg->getType());
  if (DL.getAllocaAddrSpace() != ArgTy->getAddressSpace()) {
    NewAlloca = new AddrSpaceCastInst(
        NewAlloca, ArgTy, "",
        cast<Instruction>(NewAlloca)->getNextNonDebugInstruction());
  }

  // Uses of the argument in the function should use our new alloca
  // instead.
  return NewAlloca;
}

// This is modified from lib/Transforms/Utils/InlineFunction.cpp.
static void HandleByValArgumentInit(Type *ByValType, Value *Dst, Value *Src,
                                    Module *M, BasicBlock *InsertBlock) {
  IRBuilder<> Builder(InsertBlock, InsertBlock->begin());

  Value *Size =
      Builder.getInt64(M->getDataLayout().getTypeStoreSize(ByValType));

  // Always generate a memcpy of alignment 1 here because we don't know
  // the alignment of the src pointer.  Other optimizations can infer
  // better alignment.
  Builder.CreateMemCpy(Dst, /*DstAlign*/ Align(1), Src,
                       /*SrcAlign*/ Align(1), Size);
}

// We can't use InlineFunction because InlineFunction does optimizations, which
// are not desired for O0 mode.
static void inlineWrappedKernel(CallInst *CI, AssumptionCache *AC) {
  Function *Caller = CI->getFunction();
  Function *CalledFunc = CI->getCalledFunction();

  BasicBlock *FirstNewBlock = &CalledFunc->getEntryBlock();
  Caller->getBasicBlockList().splice(Caller->end(),
                                     CalledFunc->getBasicBlockList());

  ValueToValueMapTy VMap;
  struct ByValInit {
    Value *Dst;
    Value *Src;
    Type *Ty;
  };
  // Keep a list of pair (dst, src) to emit byval initializations.
  SmallVector<ByValInit, 4> ByValInits;
  unsigned ArgNo = 0;
  auto CIArgIt = CI->arg_begin();
  for (auto It = CalledFunc->arg_begin(), E = CalledFunc->arg_end(); It != E;
       ++It, ++CIArgIt, ++ArgNo) {
    // make a copy of byval argument.
    Value *ActualArg = *CIArgIt;
    if (CI->isByValArgument(ArgNo)) {
      ActualArg =
          HandleByValArgument(CI->getParamByValType(ArgNo), ActualArg, CI,
                              CalledFunc, CalledFunc->getParamAlignment(ArgNo));
      if (ActualArg != *CIArgIt)
        ByValInits.push_back(
            {ActualArg, (Value *)*CIArgIt, CI->getParamByValType(ArgNo)});
    }

    It->replaceAllUsesWith(ActualArg);
  }

  // Unregister assumptions.
  for (BasicBlock &NewBlock :
       make_range(FirstNewBlock->getIterator(), Caller->end())) {
    for (Instruction &I : NewBlock)
      if (auto *Assume = dyn_cast<AssumeInst>(&I))
        AC->unregisterAssumption(Assume);
  }

  // Reduce the strength of inlined tail calls.
  // TODO this shall not be necessary. Currently there is test fail if we don't
  // do this, but it is likely a tailcallelim or dse bug.
  auto CallSiteTailKind = CI->getTailCallKind();
  for (auto It = FirstNewBlock->getIterator(), E = Caller->end(); It != E;
       ++It) {
    for (auto &I : *It) {
      if (auto *ClonedCI = dyn_cast<CallInst>(&I)) {
        auto TCK = ClonedCI->getTailCallKind();
        if (TCK != CallInst::TCK_NoTail)
          TCK = std::min(TCK, CallSiteTailKind);
        ClonedCI->setTailCallKind(TCK);
      }
    }
  }

  BasicBlock *Entry = &Caller->getEntryBlock();
  BranchInst::Create(FirstNewBlock, Entry);

  // Move alloca to beginning of caller's entry block.
  auto InsertPoint = Entry->begin();
  for (auto I = FirstNewBlock->begin(), E = FirstNewBlock->end(); I != E;) {
    auto *AI = dyn_cast<AllocaInst>(I++);
    if (!AI)
      continue;
    Entry->getInstList().splice(InsertPoint, FirstNewBlock->getInstList(),
                                AI->getIterator(), I);
  }

  // Inject byval arguments initialization.
  for (ByValInit &Init : ByValInits)
    HandleByValArgumentInit(Init.Ty, Init.Dst, Init.Src, Caller->getParent(),
                            &*FirstNewBlock);

  MergeBlockIntoPredecessor(FirstNewBlock);

  CI->eraseFromParent();

  // CalledFunc's basic blocks are moved to Caller.
  // This ends up with DISubprogram being attached to both functions, so we
  // need to reset CalledFunc's DISubprogram.
  Caller->setSubprogram(CalledFunc->getSubprogram());
  CalledFunc->setSubprogram(nullptr);
}

bool PrepareKernelArgsPass::runOnFunction(Function *F, AssumptionCache *AC) {
  const std::string FName = F->getName().str();

  // Create wrapper function
  Function *Wrapper = createWrapper(F);

  // Change name of old function
  F->setName("__" + Twine(F->getName()) + "_separated_args");
  CallInst *CI = createWrapperBody(Wrapper, F);

  // Set original kernel name to wrapper.
  Wrapper->setName(FName);

  // Replace function pointers to the original function (occures in case of
  // a call of a device execution built-in) by ones to the wrapper
  replaceFunctionPointers(Wrapper, F);

  DPCPPKernelMetadataAPI::KernelInternalMetadataAPI KIMD(F);
  KIMD.KernelWrapper.set(Wrapper);
  // TODO move stats from original kernel to the wrapper

  inlineWrappedKernel(CI, AC);

  // Make wrapped kernel only contain a ret instruction.
  // We can't delete wrapped kernel for now, in order to avoid its declaration
  // being removed by StripDeadPrototypes or GlobalDCE.
  createDummyRetWrappedKernel(Wrapper, F);

  return true;
}
