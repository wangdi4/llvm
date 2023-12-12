//===- PrepareKernelArgs.cpp - Prepare DPC++ kernel arguments -------------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/SYCLTransforms/PrepareKernelArgs.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/ValueHandle.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/ImplicitArgsUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/MetadataAPI.h"
#include "llvm/Transforms/SYCLTransforms/Utils/TypeAlignment.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"

using namespace llvm;

#define DEBUG_TYPE "sycl-kernel-prepare-args"

namespace {

uint64_t STACK_PADDING_BUFFER = DEV_MAXIMUM_ALIGN * 1;

} // namespace

PreservedAnalyses PrepareKernelArgsPass::run(Module &M,
                                             ModuleAnalysisManager &AM) {
  FunctionAnalysisManager &FAM =
      AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto GetAC = [&](Function &F) {
    return &FAM.getResult<AssumptionAnalysis>(F);
  };
  ImplicitArgsInfo *IAInfo = &AM.getResult<ImplicitArgsAnalysis>(M);
  if (!runImpl(M, GetAC, IAInfo))
    return PreservedAnalyses::all();
  return PreservedAnalyses::none();
}

static void
collectEnqueueKernelAndQueryFuncs(Module &M,
                                  SmallVectorImpl<Function *> &Funcs) {
  for (Function &EEF : M) {
    if (EEF.isDeclaration()) {
      StringRef EEFName = EEF.getName();
      if (EEFName.startswith("__ocl20_enqueue_kernel_") ||
          EEFName.equals("__ocl20_get_kernel_wg_size") ||
          EEFName.equals("__ocl20_get_kernel_preferred_wg_size_multiple"))
        Funcs.push_back(&EEF);
    }
  }
}

bool PrepareKernelArgsPass::runImpl(
    Module &M, function_ref<AssumptionCache *(Function &F)> GetAC,
    ImplicitArgsInfo *IAInfo) {
  this->M = &M;
  this->IAInfo = IAInfo;
  LLVMContext &C = M.getContext();
  SizetTy = IntegerType::get(C, M.getDataLayout().getPointerSizeInBits());
  I32Ty = Type::getInt32Ty(C);
  I8Ty = Type::getInt8Ty(C);

  HasTLSGlobals = CompilationUtils::hasTLSGlobals(M);

  // Get all kernels (original scalar kernels and vectorized kernels).
  auto kernelsFuncSet = CompilationUtils::getAllKernels(*this->M);

  collectEnqueueKernelAndQueryFuncs(*this->M, EnqueueKernelAndQueryFuncs);

  // Handle all kernels.
  for (auto *F : kernelsFuncSet)
    runOnFunction(F, GetAC(*F));

  return !kernelsFuncSet.empty();
}

Function *PrepareKernelArgsPass::createWrapper(Function *F) {
  // Create new function's argument type list
  SmallVector<Type *, 3> NewArgsVec;
  ArgsBufferValueTy = I8Ty;
  // The new function receives the following arguments:
  // i8* pBuffer
  NewArgsVec.push_back(PointerType::get(ArgsBufferValueTy, 0));
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

/// Return the type that has larger alloc size.
static Type *getMaxSizeType(const DataLayout &DL, Type *OldTy, Type *NewTy) {
  return (!OldTy || DL.getTypeAllocSize(NewTy) > DL.getTypeAllocSize(OldTy))
             ? NewTy
             : OldTy;
}

std::pair<Value *, Value *> PrepareKernelArgsPass::createHeapMemLoadCmpInst(
    IRBuilder<> &Builder, Argument *NonUniformArgsBuffer) {
  PointerType *SizetPtrTy = PointerType::get(SizetTy, 0);
  Value *HeapMemLoc = Builder.CreateGEP(SizetPtrTy, NonUniformArgsBuffer,
                                        ConstantInt::get(I32Ty, MAX_WORK_DIM));
  Value *HeapMem =
      Builder.Insert(new LoadInst(SizetPtrTy, HeapMemLoc, "", false, Align()));
  HeapMem->setName("pHeapMem");

  Value *HeapMemNullCmp = Builder.CreateCmp(
      CmpInst::ICMP_EQ, HeapMem, Constant::getNullValue(HeapMem->getType()));
  return std::make_pair(HeapMem, HeapMemNullCmp);
}

static Value *updateAddrWithAlignment(Value *OriginalAddr, size_t Alignment,
                                      IRBuilder<> &Builder) {
  // ((ADDR + Alignment - 1) & (~ (Alignment - 1))
  // Create add instruction
  Value *AddInst = Builder.CreateAdd(
      OriginalAddr, ConstantInt::get(OriginalAddr->getType(), Alignment - 1));
  // Create bitwise and instruction
  return Builder.CreateAnd(
      AddInst, ConstantInt::get(AddInst->getType(), ~(Alignment - 1)));
}

// If HeapMem is nullptr, alloca a new array on stack.
// If HeapMem is not nullptr, use heap memory to store data.
Value *PrepareKernelArgsPass::allocaArrayForLocalPrivateBuffer(
    IRBuilder<> &Builder, std::pair<Value *, Value *> HeapMem,
    const DataLayout &DL, Value *BufferSize, unsigned Alignment,
    Value *HeapCurrentOffset) {
  Value *HeapMemPtr = HeapMem.first;
  Value *HeapMemNullCmp = HeapMem.second;
  assert(HeapMemPtr && HeapMemNullCmp &&
         "HeapMem and HeapMemNullCmp should be created.");
  Value *AllocaSize = Builder.CreateSelect(HeapMemNullCmp, BufferSize,
                                           ConstantInt::get(SizetTy, 0));
  LLVM_DEBUG(CompilationUtils::insertPrintf(
      "LOCAL OR SPECIAL BUFFER USE HEAP MEMORY: ", Builder,
      {HeapMemNullCmp, AllocaSize}, {"CMP", "ALLOCA_SIZE"}));

  const auto AllocaAddrSpace = DL.getAllocaAddrSpace();
  AllocaInst *Allocation =
      new AllocaInst(I8Ty, AllocaAddrSpace, AllocaSize, Align(Alignment));
  Builder.Insert(Allocation);
  Value *HeapLocation = Builder.CreateGEP(I8Ty, HeapMemPtr, HeapCurrentOffset);
  Value *LocalArgBuffer =
      Builder.CreateSelect(HeapMemNullCmp, Allocation, HeapLocation);
  return LocalArgBuffer;
}

static bool hasLocalOrSpecialBuffer(
    std::vector<KernelArgument> &Arguments,
    SYCLKernelMetadataAPI::KernelInternalMetadataAPI *KIMD) {
  for (const auto &KArg : Arguments)
    if (KArg.Ty == KRNL_ARG_PTR_LOCAL)
      return true;

  uint64_t LocalBufferSize =
      KIMD->LocalBufferSize.hasValue() ? KIMD->LocalBufferSize.get() : 0;
  uint64_t BarrierBufferSize =
      KIMD->BarrierBufferSize.hasValue() ? KIMD->BarrierBufferSize.get() : 0;
  return LocalBufferSize > 0 || BarrierBufferSize > 0;
}

std::vector<Value *> PrepareKernelArgsPass::createArgumentLoads(
    IRBuilder<> &Builder, Function *WrappedKernel, Argument *UniformArgsBuffer,
    Argument *NonUniformArgsBuffer, Argument *RuntimeContext) {
  // Get old function's arguments list in the OpenCL level from its metadata
  std::vector<KernelArgument> Arguments;
  std::vector<unsigned int> MemoryArguments;
  CompilationUtils::parseKernelArguments(M, WrappedKernel, HasTLSGlobals,
                                         Arguments, MemoryArguments);

  std::vector<Value *> Params;
  Function::arg_iterator CallIt = WrappedKernel->arg_begin();
  SYCLKernelMetadataAPI::KernelInternalMetadataAPI KIMD(WrappedKernel);

  const DataLayout &DL = M->getDataLayout();
  Value *WGId = NonUniformArgsBuffer;

  // The first is heap memory pointer
  // The sconed is cmp result of heap memory pointer and null
  std::pair<Value *, Value *> HeapMem(nullptr, nullptr);
  if (hasLocalOrSpecialBuffer(Arguments, &KIMD))
    HeapMem = createHeapMemLoadCmpInst(Builder, NonUniformArgsBuffer);

  // HeapMem : [ local buffer | local argument buffer | special buffer ]
  uint64_t LocalBufferSizeInBytes =
      KIMD.LocalBufferSize.hasValue() ? KIMD.LocalBufferSize.get() : 0;
  size_t SizeWithMaxAlign = (LocalBufferSizeInBytes + DEV_MAXIMUM_ALIGN - 1) &
                            ~(DEV_MAXIMUM_ALIGN - 1);
  // add stack padding before and after this alloca, to allow unmasked
  // wide loads inside the vectorizer.
  size_t LocalBufferSizePadding = SizeWithMaxAlign + STACK_PADDING_BUFFER * 2;
  Value *HeapCurrentOffset =
      LocalBufferSizeInBytes == 0
          ? ConstantInt::get(SizetTy, 0)
          : ConstantInt::get(SizetTy, LocalBufferSizePadding);
  bool HasLocalArg = false;

  // TODO :  get common code from the following 2 for loops into a function
  // Handle explicit arguments
  for (unsigned ArgNo = 0; ArgNo < Arguments.size(); ++ArgNo) {
    KernelArgument KArg = Arguments[ArgNo];
    //  %0 = getelementptr i8* %pBuffer, i32 CurrOffset
    Value *GEP = Builder.CreateGEP(ArgsBufferValueTy, UniformArgsBuffer,
                                   ConstantInt::get(I32Ty, KArg.OffsetInBytes));

    Value *Arg;

    if (KArg.Ty == KRNL_ARG_COMPOSITE || KArg.Ty == KRNL_ARG_VECTOR_BY_REF) {
      // If this is a struct argument, then the struct itself is passed by value
      // inside UniformArgsBuffer and the original kernel signature was:
      // foo(..., MyStruct* byval myStruct, ...) meaning GEP already points to
      // the structure and we do not need to load it we just need to have a
      // bitcast from i8* to MyStruct* and pass the pointer (!!!) to foo

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

      // Set alignment of buffer to type size.
      size_t Alignment = DEV_MAXIMUM_ALIGN;
      Type *EltTy = nullptr;
      // If the kernel was vectorized, choose an alignment that is good for the
      // *vectorized* type. This can be good for unaligned loads on targets that
      // support instructions such as MOVUPS
      unsigned VecSize =
          KIMD.VectorizedWidth.hasValue() ? KIMD.VectorizedWidth.get() : 1;
      // Nonspirv doesn't have argument type metadata. Get type/align from IR.
      if (auto A = CallIt->getParamAlign()) {
        Alignment =
            NextPowerOf2((VecSize == 3 ? 4 : VecSize) * A.value().value() - 1);
      } else {
        SmallVector<User *, 8> WorkList{CallIt->users()};
        SmallPtrSet<Value *, 4> Def{&*CallIt};
        while (!WorkList.empty()) {
          User *U = WorkList.pop_back_val();
          if (isa<AddrSpaceCastInst>(U) || isa<SelectInst>(U) ||
              isa<PHINode>(U)) {
            Def.insert(U);
            WorkList.append(U->user_begin(), U->user_end());
            continue;
          }
          if (auto *Op = getLoadStorePointerOperand(U); Op && Def.contains(Op))
            EltTy = getMaxSizeType(DL, EltTy, getLoadStoreType(U));
          else if (auto *GEP = dyn_cast<GetElementPtrInst>(U))
            EltTy = getMaxSizeType(DL, EltTy, GEP->getSourceElementType());
        }
      }
      if (EltTy) {
        if (VecSize != 1 && VectorType::isValidElementType(EltTy))
          EltTy = FixedVectorType::get(EltTy, VecSize);
        Alignment = NextPowerOf2(DL.getTypeAllocSize(EltTy) - 1);
      }
      if (HasLocalArg)
        // Local buffer's size has been align with DEV_MAXIMUM_ALIGN.
        // So if this is the first local buffer arg, need not alignment when
        // using heap memory
        HeapCurrentOffset =
            updateAddrWithAlignment(HeapCurrentOffset, Alignment, Builder);
      Value *LocalArgBuffer = allocaArrayForLocalPrivateBuffer(
          Builder, HeapMem, DL, BufferSize, Alignment, HeapCurrentOffset);
      Arg = Builder.CreatePointerCast(LocalArgBuffer, CallIt->getType());
      HeapCurrentOffset = Builder.CreateAdd(HeapCurrentOffset, BufferSize);
      HasLocalArg = true;
    } else if (KArg.Ty == KRNL_ARG_PTR_BLOCK_LITERAL) {
      Arg = Builder.CreateAddrSpaceCast(GEP, CallIt->getType());
    } else {
      // Otherwise this is some other type, lets say int4, then int4 itself is
      // passed by value inside UniformArgsBuffer and the original kernel
      // signature was: foo(..., int4 vec, ...) meaning GEP points to int4 and
      // we just need to have a bitcast from i8* to int4* load the int4 and pass
      // the loaded value (!!!) to foo

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
    if (!HasTLSGlobals)
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
      if (LocalBufferSizeInBytes == 0) {
        // no need to create of pad this buffer.
        Arg = Constant::getNullValue(PointerType::get(I8Ty, 3));
      } else {
        Value *HeapOffset = ConstantInt::get(SizetTy, 0);
        // Set alignment of implicit local buffer to max alignment.
        // TODO: we should choose the min required alignment size
        // move argument up over the lower side padding.
        Value *SLMBuffer = allocaArrayForLocalPrivateBuffer(
            Builder, HeapMem, DL,
            ConstantInt::get(SizetTy, LocalBufferSizePadding),
            TypeAlignment::MAX_ALIGNMENT, HeapOffset);
        Value *CastBuf =
            Builder.CreatePointerCast(SLMBuffer, PointerType::get(I8Ty, 3));
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
      const auto AllocaAddrSpace = DL.getAllocaAddrSpace();
      // We obtain the number of bytes needed per item from the Metadata
      // which is set by the Barrier pass
      uint64_t SizeInBytes =
          KIMD.BarrierBufferSize.hasValue() ? KIMD.BarrierBufferSize.get() : 0;
      if (SizeInBytes == 0) {
        Arg = Constant::getNullValue(PointerType::get(I8Ty, AllocaAddrSpace));
        break;
      }
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
      if (HasLocalArg)
        HeapCurrentOffset = updateAddrWithAlignment(
            HeapCurrentOffset, TypeAlignment::MAX_ALIGNMENT, Builder);
      Value *BarrierBuffer = allocaArrayForLocalPrivateBuffer(
          Builder, HeapMem, DL, BarrierBufferSize, TypeAlignment::MAX_ALIGNMENT,
          HeapCurrentOffset);
      Arg = BarrierBuffer;
      LLVM_DEBUG(CompilationUtils::insertPrintf(
          "SPECIAL BUFFER: ", Builder, {BarrierBuffer, BarrierBufferSize},
          {"ADDR", "SIZE"}));
    } break;
    case ImplicitArgsUtils::IA_WORK_GROUP_INFO: {
      // These values are pointers that just need to be loaded from the
      // UniformKernelArgs structure and passed on to the kernel
      const ImplicitArgProperties &ImplicitArgProp =
          ImplicitArgsUtils::getImplicitArgProps(I);
      // %0 = getelementptr i8* %pBuffer, i32 CurrOffset
      Value *GEP = Builder.CreateGEP(ArgsBufferValueTy, UniformArgsBuffer,
                                     ConstantInt::get(I32Ty, CurrOffset));
      Arg = Builder.CreatePointerCast(GEP, IAInfo->getArgType(I));
      WGInfo = Arg;
      // Advance the ArgsBuffer offset based on the size
      CurrOffset += ImplicitArgProp.Size;
    } break;
    }

    if (HasTLSGlobals) {
      assert(Arg && "No value was created for this TLS global!");
      auto *C = dyn_cast<Constant>(Arg);
      if (!(C && C->isNullValue() &&
            (I == ImplicitArgsUtils::IA_SLM_BUFFER ||
             I == ImplicitArgsUtils::IA_BARRIER_BUFFER))) {
        GlobalVariable *GV = CompilationUtils::getTLSGlobal(M, I);
        Builder.CreateAlignedStore(Arg, GV, DL.getABITypeAlign(Arg->getType()));
      }
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
  if (WrappedKernel->use_empty())
    return;

  // BIs like enqueue_kernel and kernel query have a function pointer to a
  // block invoke kernel as an argument.
  // Replace these arguments by a pointer to the wrapper function.
  IRBuilder<> Builder(WrappedKernel->getContext());
  for (Function *EEF : EnqueueKernelAndQueryFuncs) {
    StringRef EEFName = EEF->getName();
    unsigned BlockInvokeIdx = (EEFName.startswith("__ocl20_enqueue_kernel_"))
                                  ? (EEFName.contains("_events") ? 6 : 3)
                                  : 0;

    for (auto *U : EEF->users()) {
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

  ValueToValueMapTy VMap;
  VMap[WrappedKernel] = Wrapper;
  ValueMapper VMapper(VMap, RF_NoModuleLevelChanges | RF_IgnoreMissingLocals);
  SmallVector<User *, 16> Users{WrappedKernel->users()};
  for (User *U : Users) {
    // Skip the CallInst that was created in createWrapperBody.
    if (auto *CI = dyn_cast<CallInst>(U); CI && CI->getFunction() == Wrapper)
      continue;

    if (auto *I = dyn_cast<Instruction>(U))
      VMapper.remapInstruction(*I);
    else if (auto *GV = dyn_cast<GlobalVariable>(U))
      GV->setInitializer(VMapper.mapConstant(*GV->getInitializer()));
    else if (auto *C = dyn_cast<Constant>(U))
      C->replaceAllUsesWith(VMapper.mapConstant(*C));
    else
      llvm_unreachable("unhandled wrapped kernel user");
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
                                  Align ByValAlignment) {
  Function *Caller = TheCall->getFunction();
  const DataLayout &DL = Caller->getParent()->getDataLayout();

  // Create the alloca.  If we have DataLayout, use nice alignment.
  Align Alignment(DL.getPrefTypeAlign(ByValType));

  // If the byval had an alignment specified, we *must* use at least that
  // alignment, as it is required by the byval argument (and uses of the
  // pointer inside the callee).
  Alignment = std::max(Alignment, ByValAlignment);

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
  Caller->splice(Caller->end(), CalledFunc);

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
      ActualArg = HandleByValArgument(
          CI->getParamByValType(ArgNo), ActualArg, CI, CalledFunc,
          CalledFunc->getParamAlign(ArgNo).valueOrOne());
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

  SYCLKernelMetadataAPI::KernelInternalMetadataAPI KIMD(F);
  KIMD.KernelWrapper.set(Wrapper);
  // TODO move stats from original kernel to the wrapper

  inlineWrappedKernel(CI, AC);

  // Make wrapped kernel only contain a ret instruction.
  // We can't delete wrapped kernel for now, in order to avoid its declaration
  // being removed by StripDeadPrototypes or GlobalDCE.
  createDummyRetWrappedKernel(Wrapper, F);

  return true;
}
