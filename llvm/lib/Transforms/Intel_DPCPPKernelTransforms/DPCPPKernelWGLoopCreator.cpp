//==DPCPPKernelWGLoopCreator.cpp - Create WG loops in DPCPP kernels -*- C++-==//
//
// Copyright (C) 2020-2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelWGLoopCreator.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelBarrierUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Passes.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"

#include <sstream>

using namespace llvm;

#define DEBUG_TYPE "dpcpp-kernel-wgloop-creator"

namespace {

class DPCPPKernelWGLoopCreatorLegacy : public ModulePass {
  DPCPPKernelWGLoopCreatorPass Impl;

public:
  static char ID;

  DPCPPKernelWGLoopCreatorLegacy() : ModulePass(ID) {}

  llvm::StringRef getPassName() const override { return "WGLoopCreatorLegacy"; }

  bool runOnModule(Module &M) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override;
};

} // namespace

char DPCPPKernelWGLoopCreatorLegacy::ID = 0;

INITIALIZE_PASS_BEGIN(DPCPPKernelWGLoopCreatorLegacy, DEBUG_TYPE,
                      "Create a loop over SYCL lambda", false, false)
INITIALIZE_PASS_DEPENDENCY(UnifyFunctionExitNodesLegacyPass)
INITIALIZE_PASS_END(DPCPPKernelWGLoopCreatorLegacy, DEBUG_TYPE,
                    "Create a loop over SYCL lambda", false, false)

bool DPCPPKernelWGLoopCreatorLegacy::runOnModule(Module &M) {
  FuncSet FSet = DPCPPKernelCompilationUtils::getAllKernels(M);
  MapFunctionToReturnInst FuncReturn;
  for (auto *F : FSet) {
    BasicBlock *SingleRetBB =
        getAnalysis<UnifyFunctionExitNodesLegacyPass>(*F).getReturnBlock();
    assert(SingleRetBB && "Expect a valid ret block");
    FuncReturn[F] = cast<ReturnInst>(SingleRetBB->getTerminator());
  }
  Impl.setFuncReturn(FuncReturn);
  return Impl.runImpl(M);
}

void DPCPPKernelWGLoopCreatorLegacy::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<UnifyFunctionExitNodesLegacyPass>();
}

llvm::ModulePass *llvm::createDPCPPKernelWGLoopCreatorLegacyPass() {
  return new DPCPPKernelWGLoopCreatorLegacy();
}

DPCPPKernelWGLoopCreatorPass::DPCPPKernelWGLoopCreatorPass()
    : Context(nullptr), ScalarRet(nullptr), VectorRet(nullptr), IndTy(nullptr),
      ConstZero(nullptr), ConstOne(nullptr), ConstPacket(nullptr),
      Func(nullptr), NumDim(0), ScalarEntry(nullptr), NewEntry(nullptr),
      VectorFunc(nullptr), PacketWidth(0), VectorizedDim(0) {}

PreservedAnalyses DPCPPKernelWGLoopCreatorPass::run(Module &M,
                                                    ModuleAnalysisManager &AM) {
  (void)AM;
  FuncSet FSet = DPCPPKernelCompilationUtils::getAllKernels(M);
  for (auto *F : FSet) {
    for (BasicBlock &BB : *F) {
      if (auto *RI = dyn_cast<ReturnInst>(BB.getTerminator())) {
        FuncReturn[F] = RI;
        break;
      }
    }
  }
  if (!runImpl(M))
    return PreservedAnalyses::all();
  return PreservedAnalyses::none();
}

bool DPCPPKernelWGLoopCreatorPass::runImpl(Module &M) {
  Context = &M.getContext();
  IndTy = DPCPPKernelLoopUtils::getIntTy(&M);
  ConstZero = ConstantInt::get(IndTy, 0);
  ConstOne = ConstantInt::get(IndTy, 1);
  auto Kernels = DPCPPKernelCompilationUtils::getKernels(M);
  for (auto *F : Kernels) {
    // No need to check if NoBarrierPath Value exists, it is guaranteed that
    // KernelAnalysisPass ran before WGLoopCreator pass.
    assert(F->hasFnAttribute(NO_BARRIER_PATH_ATTRNAME) &&
           "DPCPPKernelWGLoopCreator: Expect " NO_BARRIER_PATH_ATTRNAME
           " attribute!");
    StringRef Value =
        F->getFnAttribute(NO_BARRIER_PATH_ATTRNAME).getValueAsString();
    assert((Value == "true" || Value == "false") &&
           "DPCPPKernelWGLoopCreator: unexpected " NO_BARRIER_PATH_ATTRNAME
           " value!");
    // Kernel that should be handled in barrier path, skip it.
    if (Value == "false")
      continue;

    unsigned int VectWidth = 0;
    // Set the vectorized function
    Function *VectKernel = DPCPPKernelCompilationUtils::getFnAttributeFunction(
        M, *F, "vectorized_kernel");
    // Need to check if Vectorized Kernel Value exists, it is not guaranteed
    // that Vectorized is running in all scenarios.
    if (VectKernel) {
      // Set the vectorized width
      assert(
          F->hasFnAttribute("vectorized_width") &&
          "WGLoopCreator: vectorized_width has to be set (but may be empty)!");
      bool Res = to_integer(
          VectKernel->getFnAttribute("vectorized_width").getValueAsString(),
          VectWidth);
      // silence warning to avoid an extra call
      (void)Res;
      assert(Res &&
             "WGLoopCreator: vectorized_width has to have a numeric value");

      // save the relevant information from the vectorized kernel
      // prior to erasing this information.
      F->removeFnAttr("vectorized_kernel");
      F->addFnAttr("vectorized_kernel", "");
      F->removeFnAttr("vectorized_width");
      F->addFnAttr("vectorized_width", utostr(VectWidth));
    }

    LLVM_DEBUG(dbgs() << "vectWidth for " << F->getName() << ": " << VectWidth
                      << "\n";);
    processFunction(F, VectKernel, VectWidth);
  }
  return !Kernels.empty();
}

// if the vector kernel exists then we create the following code:
//
// max_vector =
// if(vectorLoopSize != 0)
//   vector loops
// if (scalarLoopSize != 0)
//   scalar loops
// return
LoopRegion DPCPPKernelWGLoopCreatorPass::createVectorAndRemainderLoops() {
  // Collect get**id and return instructions in the vector kernel.
  VectorRet = getFunctionData(VectorFunc, GidCallsVec, LidCallsVec);

  // Inline the vector kernel into the scalar kernel.
  BasicBlock *VecEntry = inlineVectorFunction(ScalarEntry);

  // Obtain boundaries for the vector loops and scalar remainder loops.
  LoopBoundaries Dim0Boundaries = getVectorLoopBoundaries(
      InitGIDs[VectorizedDim], LoopSizes[VectorizedDim]);

  ValueVec InitGIDs = DPCPPKernelWGLoopCreatorPass::InitGIDs;   // hard copy.
  ValueVec LoopSizes = DPCPPKernelWGLoopCreatorPass::LoopSizes; // hard copy.

  // Create vector loops.
  LoopSizes[VectorizedDim] = Dim0Boundaries.VectorLoopSize;
  LoopRegion VectorBlocks = addWGLoops(VecEntry, true, VectorRet, GidCallsVec,
                                       LidCallsVec, InitGIDs, LoopSizes);

  // Create scalar loops.
  InitGIDs[VectorizedDim] = Dim0Boundaries.MaxVector;
  LoopSizes[VectorizedDim] = Dim0Boundaries.ScalarLoopSize;
  LoopRegion ScalarBlocks =
      addWGLoops(ScalarEntry, false, ScalarRet, GidCallsSc, LidCallsSc,
                 InitGIDs, LoopSizes);

  // Create blocks to jump over the loops.
  BasicBlock *LoopsEntry =
      BasicBlock::Create(*Context, "vect_if", Func, VectorBlocks.PreHeader);
  BasicBlock *ScalarIf =
      BasicBlock::Create(*Context, "scalarIf", Func, ScalarBlocks.PreHeader);

  BasicBlock *RetBlock = BasicBlock::Create(*Context, "ret", Func);

  // Execute the vector loops if(vectorLoopSize != 0).
  Instruction *Vectcmp =
      new ICmpInst(*LoopsEntry, CmpInst::ICMP_NE, Dim0Boundaries.VectorLoopSize,
                   ConstZero, "");
  BranchInst::Create(VectorBlocks.PreHeader, ScalarIf, Vectcmp, LoopsEntry);
  BranchInst::Create(ScalarIf, VectorBlocks.Exit);

  // execute the scalar loops if(scalarLoopSize != 0)
  Instruction *ScalarCmp =
      new ICmpInst(*ScalarIf, CmpInst::ICMP_NE, Dim0Boundaries.ScalarLoopSize,
                   ConstZero, "");
  BranchInst::Create(ScalarBlocks.PreHeader, RetBlock, ScalarCmp, ScalarIf);
  BranchInst::Create(RetBlock, ScalarBlocks.Exit);
  return LoopRegion(LoopsEntry, RetBlock);
}

DPCPPKernelWGLoopCreatorPass::LoopBoundaries
DPCPPKernelWGLoopCreatorPass::getVectorLoopBoundaries(Value *InitVal,
                                                      Value *DimSize) {
  // computes constant log packetWidth
  assert(PacketWidth && ((PacketWidth & (PacketWidth - 1)) == 0) &&
         "packet width is not power of 2");
  unsigned LogPacket = Log2_32(PacketWidth);
  Constant *LogPacketConst = ConstantInt::get(IndTy, LogPacket);

  // vector loops size can be derived by shifting size with log packet bits.
  Value *VectorLoopSize = BinaryOperator::Create(
      Instruction::AShr, DimSize, LogPacketConst, "vector.size", NewEntry);
  Value *NumVectorWI =
      BinaryOperator::Create(Instruction::Shl, VectorLoopSize, LogPacketConst,
                             "num.vector.wi", NewEntry);
  Value *MaxVector = BinaryOperator::Create(
      Instruction::Add, NumVectorWI, InitVal, "max.vector.gid", NewEntry);
  Value *ScalarLoopSize = BinaryOperator::Create(
      Instruction::Sub, DimSize, NumVectorWI, "scalar.size", NewEntry);
  return LoopBoundaries(VectorLoopSize, ScalarLoopSize, MaxVector);
}

BasicBlock *DPCPPKernelWGLoopCreatorPass::inlineVectorFunction(BasicBlock *BB) {
  // Create denseMap of function arguments
  ValueToValueMapTy ValueMap;
  assert(Func->getFunctionType() == VectorFunc->getFunctionType() &&
         "vector and scalar functtion type mismatch");
  Function *Fn = BB->getParent();
  Function::const_arg_iterator VArgIt = VectorFunc->arg_begin();
  Function::const_arg_iterator VArgE = VectorFunc->arg_end();
  Function::arg_iterator ArgIt = Fn->arg_begin();
  for (; VArgIt != VArgE; ++ArgIt, ++VArgIt) {
    ValueMap[&*VArgIt] = &*ArgIt;
  }

  // create a list for return values
  SmallVector<ReturnInst *, 2> Returns;

  // Map debug information metadata entries in the cloned code from the vector
  // DISubprogram to the scalar DISubprogram.
  DISubprogram *SSP = Func->getSubprogram();
  DISubprogram *VSP = VectorFunc->getSubprogram();
  if (SSP && VSP) {
    auto &MD = ValueMap.MD();
    MD[VSP].reset(SSP);
  }

  // Do actual cloning work
  // TODO: replace manual inlining by llvm::InlineFunction()
  CloneFunctionInto(Fn, VectorFunc, ValueMap,
                    CloneFunctionChangeType::LocalChangesOnly, Returns,
                    "vector_func");

  // The CloneFunctionInto() above will move all function metadata from the
  // vector function to the scalar function. Because the scalar function
  // already has debug info metadata, it ends up with two DISubprograms
  // assigned. The easiest way to assign the correct one is to erase both of
  // them by resetting the original scalar subprogram.
  Func->setSubprogram(SSP);

  // Restore sycl_kernel attribute which was overwriten by CloneFunctionInto.
  Func->addFnAttr("sycl_kernel");

  for (auto &VBB : *VectorFunc) {
    BasicBlock *ClonedBB = dyn_cast<BasicBlock>(ValueMap[&VBB]);
    ClonedBB->moveBefore(BB);
  }

  for (unsigned Dim = 0; Dim < NumDim; ++Dim) {
    for (unsigned i = 0, e = GidCallsVec[Dim].size(); i < e; ++i) {
      Instruction *inst = dyn_cast<Instruction>(ValueMap[GidCallsVec[Dim][i]]);
      assert(inst && "Only Instructions are expected here!");
      GidCallsVec[Dim][i] = inst;
    }
    for (unsigned i = 0, e = LidCallsVec[Dim].size(); i < e; ++i) {
      Instruction *inst = dyn_cast<Instruction>(ValueMap[LidCallsVec[Dim][i]]);
      assert(inst && "Only Instructions are expected here!");
      LidCallsVec[Dim][i] = inst;
    }
  }
  VectorRet = cast<ReturnInst>(ValueMap[VectorRet]);
  // Get hold of the entry to the vector section in the vectorized function...
  BasicBlock *VectorEntryBlock =
      dyn_cast<BasicBlock>(ValueMap[&*(VectorFunc->begin())]);
  assert(!VectorFunc->getNumUses() && "vector kernel should have no use");
  VectorFunc->eraseFromParent();
  return VectorEntryBlock;
}

ReturnInst *DPCPPKernelWGLoopCreatorPass::getFunctionData(Function *F,
                                                          InstVecVec &Gids,
                                                          InstVecVec &Lids) {
  std::string GID = DPCPPKernelCompilationUtils::mangledGetGID();
  std::string LID = DPCPPKernelCompilationUtils::mangledGetLID();
  DPCPPKernelLoopUtils::collectTIDCallInst(GID, Gids, F);
  DPCPPKernelLoopUtils::collectTIDCallInst(LID, Lids, F);

  return FuncReturn[F];
}

void DPCPPKernelWGLoopCreatorPass::getLoopsBoundaries(Function *F) {
  LoopSizes.clear();
  InitGIDs.clear();
  BaseGIDs.clear();
  for (unsigned Dim = 0; Dim < NumDim; ++Dim) {
    CallInst *BaseGID = DPCPPKernelLoopUtils::getWICall(
        F->getParent(), DPCPPKernelCompilationUtils::nameGetBaseGID(), IndTy,
        Dim, NewEntry);
    InitGIDs.push_back(BaseGID);
    BaseGIDs.push_back(BaseGID);
    CallInst *LocalSize = DPCPPKernelLoopUtils::getWICall(
        F->getParent(), DPCPPKernelCompilationUtils::mangledGetLocalSize(),
        IndTy, Dim, NewEntry);
    LoopSizes.push_back(LocalSize);
  }
}

unsigned int DPCPPKernelWGLoopCreatorPass::resolveDimension(unsigned int Dim) {
  if (Dim == 0)
    return VectorizedDim;
  else if (Dim > VectorizedDim)
    return Dim;
  else
    return Dim - 1;
}

void DPCPPKernelWGLoopCreatorPass::computeDimStr(unsigned Dim, bool IsVector) {
  std::stringstream DimStream;
  DimStream << "dim_" << Dim << "_";
  if (IsVector)
    DimStream << "vector_";
  DimStr = DimStream.str();
}

LoopRegion DPCPPKernelWGLoopCreatorPass::addWGLoops(
    BasicBlock *KernelEntry, bool IsVector, ReturnInst *Ret, InstVecVec &GIDs,
    InstVecVec &LIDs, ValueVec &InitGIDs, SmallVector<Value *, 4> &LoopSizes) {

  assert(KernelEntry && Ret && "uninitialized parameters");
  // Move allocas in the entry kernel entry block to the new entry block.
  DPCPPKernelCompilationUtils::moveAllocaToEntry(KernelEntry, NewEntry);

  // Inital head and latch are the kernel entry and return block respectively.
  BasicBlock *Head = KernelEntry;
  BasicBlock *Latch = Ret->getParent();

  // Erase original return instruction.
  Ret->eraseFromParent();

  // In case of vector kernel the tid generators are incremented by the packet
  // width. In case of scalar loop increment by 1.
  Value *Dim0IncBy = IsVector ? ConstPacket : ConstOne;
  for (unsigned Dim = 0; Dim < NumDim; ++Dim) {
    unsigned ResolvedDim = resolveDimension(Dim);
    computeDimStr(ResolvedDim, IsVector);
    Value *IncBy = ResolvedDim == VectorizedDim ? Dim0IncBy : ConstOne;
    // Create the loop
    LoopRegion Blocks = DPCPPKernelLoopUtils::createLoop(
        Head, Latch, ConstZero, ConstOne, LoopSizes[ResolvedDim], DimStr,
        *Context);
    // Modify get***id accordingly.
    Value *InitGID = InitGIDs[ResolvedDim];
    if (GIDs[ResolvedDim].size())
      replaceTIDsWithPHI(GIDs[ResolvedDim], InitGID, IncBy, Head,
                         Blocks.PreHeader, Latch);
    if (LIDs[ResolvedDim].size()) {
      Value *InitLID =
          BinaryOperator::Create(Instruction::Sub, InitGID, BaseGIDs[Dim],
                                 DimStr + "init_lid", NewEntry);
      replaceTIDsWithPHI(LIDs[ResolvedDim], InitLID, IncBy, Head,
                         Blocks.PreHeader, Latch);
    }
    // head, latch for the next loop are the pre header and exit
    // block respectively.
    Head = Blocks.PreHeader;
    Latch = Blocks.Exit;
  }
  return LoopRegion(Head, Latch);
}

void DPCPPKernelWGLoopCreatorPass::replaceTIDsWithPHI(
    InstVec &TIDs, Value *InitVal, Value *IncBy, BasicBlock *Head,
    BasicBlock *PreHead, BasicBlock *Latch) {
  assert(TIDs.size() && "unexpected emty tid vector");
  std::string Str = "dim0_";
  PHINode *DimTID =
      PHINode::Create(IndTy, 2, Str + "_tid", Head->getFirstNonPHI());
  BinaryOperator *IncTID = BinaryOperator::Create(
      Instruction::Add, DimTID, IncBy, Str + "inc_tid", Latch->getTerminator());
  IncTID->setHasNoSignedWrap();
  IncTID->setHasNoUnsignedWrap();
  DimTID->addIncoming(InitVal, PreHead);
  DimTID->addIncoming(IncTID, Latch);
  for (auto *TID : TIDs) {
    TID->replaceAllUsesWith(DimTID);
    TID->eraseFromParent();
  }
}

void DPCPPKernelWGLoopCreatorPass::processFunction(Function *F,
                                                   Function *VectorFuncParam,
                                                   unsigned PacketWidthParam) {
  LLVM_DEBUG(dbgs() << "Creating loop for " << F->getName() << "\n";);
  LLVM_DEBUG(if (VectorFunc) {
    dbgs() << "Vector function name: " << VectorFunc->getName() << "\n";
  });

  // For DPC++ we always vectorize along dimension 0.
  VectorizedDim = 0;

  // Update member fields with the current kernel.
  Func = F;
  VectorFunc = VectorFuncParam;
  PacketWidth = PacketWidthParam;
  Context = &(F->getContext());
  // generate constants
  ConstPacket = ConstantInt::get(IndTy, PacketWidth);

  // Collect get**id and return instructions from the kernels.
  ScalarRet = getFunctionData(F, GidCallsSc, LidCallsSc);

  // 3 is the max dimenstions allowed.
  // TODO: port deducing of actual #of dimensions.
  // In the header we can hint the compiler on how many loops
  // we actually need.
  NumDim = 3;

  // Mark scalar kernel entry and create new entry block for boundaries
  // calculation.
  ScalarEntry = &F->getEntryBlock();
  ScalarEntry->setName("scalar_kernel_entry");
  NewEntry = BasicBlock::Create(*Context, "", F, ScalarEntry);

  // Obtain loops boundaries from early exit call.
  getLoopsBoundaries(F);

  LoopRegion WGLoopRegion =
      VectorFunc && NumDim
          ? createVectorAndRemainderLoops()
          : addWGLoops(ScalarEntry, false, ScalarRet, GidCallsSc, LidCallsSc,
                       InitGIDs, LoopSizes);
  assert(WGLoopRegion.PreHeader && WGLoopRegion.Exit &&
         "loops entry,exit not initialized");

  // Connect the new entry block with the WG loops.
  BranchInst::Create(WGLoopRegion.PreHeader, NewEntry);

  // Create return block and connect it to WG loops exit.
  // We must create separate block for the return since the it might be
  // that there are no WG loops (NumDim=0) and WGLoopRegion.Exit
  // is not empty.
  BasicBlock *NewRet = BasicBlock::Create(*Context, "", F);
  BranchInst::Create(NewRet, WGLoopRegion.Exit);
  ReturnInst::Create(*Context, NewRet);

  // Finally, remove noinline attr
  if (!F->hasFnAttribute(llvm::Attribute::OptimizeNone))
    F->removeFnAttr(llvm::Attribute::NoInline);

  LLVM_DEBUG(dbgs() << "Created loop for " << F->getName() << "\n";);
}
