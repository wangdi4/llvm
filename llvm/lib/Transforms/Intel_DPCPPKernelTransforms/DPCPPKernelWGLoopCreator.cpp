//==DPCPPKernelWGLoopCreator.cpp - Create WG loops in DPCPP kernels -*- C++-==//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
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
#include "llvm/Support/MathExtras.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelBarrierUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"

#include <sstream>

using namespace llvm;

#define DEBUG_TYPE "DPCPPKernelWGLoopCreatorLegacyPass"

PreservedAnalyses DPCPPKernelWGLoopCreatorPass::run(Module &M,
                                                    ModuleAnalysisManager &AM) {
  assert(false && "New PM piping not implemented");
  return PreservedAnalyses::none();
}

INITIALIZE_PASS_BEGIN(DPCPPKernelWGLoopCreatorLegacyPass,
                      "dpcpp-kernel-wgloop-creator",
                      "Create a loop over SYCL lambda", false, false)
INITIALIZE_PASS_DEPENDENCY(UnifyFunctionExitNodes)
INITIALIZE_PASS_END(DPCPPKernelWGLoopCreatorLegacyPass,
                    "dpcpp-kernel-wgloop-creator",
                    "Create a loop over SYCL lambda", false, false)
namespace llvm {

DPCPPKernelWGLoopCreatorLegacyPass::DPCPPKernelWGLoopCreatorLegacyPass()
    : ModulePass(ID), Context(nullptr), ScalarRet(nullptr), VectorRet(nullptr),
      IndTy(nullptr), ConstZero(nullptr), ConstOne(nullptr),
      ConstPacket(nullptr), Func(nullptr), NumDim(0), ScalarEntry(nullptr),
      NewEntry(nullptr), VectorFunc(nullptr), PacketWidth(0), VectorizedDim(0) {
  initializeDPCPPKernelWGLoopCreatorLegacyPassPass(
      *PassRegistry::getPassRegistry());
}

DPCPPKernelWGLoopCreatorLegacyPass::~DPCPPKernelWGLoopCreatorLegacyPass() {}

bool DPCPPKernelWGLoopCreatorLegacyPass::runOnModule(Module &M) {
  return processModule(M);
}

void DPCPPKernelWGLoopCreatorLegacyPass::getAnalysisUsage(
    AnalysisUsage &AU) const {
  AU.addRequired<UnifyFunctionExitNodes>();
}

char DPCPPKernelWGLoopCreatorLegacyPass::ID = 0;

llvm::ModulePass *createDPCPPKernelWGLoopCreatorPass() {
  return new DPCPPKernelWGLoopCreatorLegacyPass();
}

bool DPCPPKernelWGLoopCreatorLegacyPass::processModule(Module &M) {
  SmallVector<Function *, 8> WorkList;

  Context = &M.getContext();
  IndTy = DPCPPKernelLoopUtils::getIntTy(&M);
  ConstZero = ConstantInt::get(IndTy, 0);
  ConstOne = ConstantInt::get(IndTy, 1);

  for (auto &F : M) {
    if (F.hasFnAttribute("sycl_kernel"))
      WorkList.push_back(&F);
  }

  for (auto *F : WorkList) {
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
    Function *VectKernel = nullptr;
    // Set the vectorized function
    if (F->hasFnAttribute("vectorized_kernel")) {
      VectKernel = M.getFunction(
          F->getFnAttribute("vectorized_kernel").getValueAsString());
    }
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

  if (WorkList.empty())
    return false;
  return true;
}

// if the vector kernel exists then we create the following code:
//
// max_vector =
// if(vectorLoopSize != 0)
//   vector loops
// if (scalarLoopSize != 0)
//   scalar loops
// return
LoopRegion DPCPPKernelWGLoopCreatorLegacyPass::createVectorAndRemainderLoops() {
  // Collect get**id and return instructions in the vector kernel.
  VectorRet = getFunctionData(VectorFunc, LidCallsVec);

  // Inline the vector kernel into the scalar kernel.
  BasicBlock *VecEntry = inlineVectorFunction(ScalarEntry);

  // Obtain boundaries for the vector loops and scalar remainder loops.
  LoopBoundaries Dim0Boundaries = getVectorLoopBoundaries(
      InitGIDs[VectorizedDim], LoopSizes[VectorizedDim]);

  ValueVec InitGIDs =
      DPCPPKernelWGLoopCreatorLegacyPass::InitGIDs; // hard copy.
  ValueVec LoopSizes =
      DPCPPKernelWGLoopCreatorLegacyPass::LoopSizes; // hard copy.

  // Create vector loops.
  LoopSizes[VectorizedDim] = Dim0Boundaries.VectorLoopSize;
  LoopRegion VectorBlocks =
      addWGLoops(VecEntry, true, VectorRet, LidCallsVec, InitGIDs, LoopSizes);

  // Create scalar loops.
  InitGIDs[VectorizedDim] = Dim0Boundaries.MaxVector;
  LoopSizes[VectorizedDim] = Dim0Boundaries.ScalarLoopSize;
  LoopRegion ScalarBlocks = addWGLoops(ScalarEntry, false, ScalarRet,
                                       LidCallsSc, InitGIDs, LoopSizes);

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

DPCPPKernelWGLoopCreatorLegacyPass::LoopBoundaries
DPCPPKernelWGLoopCreatorLegacyPass::getVectorLoopBoundaries(Value *InitVal,
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

BasicBlock *
DPCPPKernelWGLoopCreatorLegacyPass::inlineVectorFunction(BasicBlock *BB) {
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
  CloneFunctionInto(Fn, VectorFunc, ValueMap, /*ModuleLevelChanges*/ true,
                    Returns, "vector_func");

  // The CloneFunctionInto() above will move all function metadata from the
  // vector function to the scalar function. Because the scalar function
  // already has debug info metadata, it ends up with two DISubprograms
  // assigned. The easiest way to assign the correct one is to erase both of
  // them by resetting the original scalar subprogram.
  Func->setSubprogram(SSP);

  for (auto &VBB : *VectorFunc) {
    BasicBlock *ClonedBB = dyn_cast<BasicBlock>(ValueMap[&VBB]);
    ClonedBB->moveBefore(BB);
  }

  for (unsigned Dim = 0; Dim < NumDim; ++Dim) {
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

ReturnInst *
DPCPPKernelWGLoopCreatorLegacyPass::getFunctionData(Function *F,
                                                    InstVecVec &Lids) {
  std::string LID = "__builtin_get_local_id";

  DPCPPKernelLoopUtils::collectTIDCallInst(LID.c_str(), Lids, F);

  BasicBlock *SingleRetBB =
      getAnalysis<UnifyFunctionExitNodes>(*F).getReturnBlock();
  return cast<ReturnInst>(SingleRetBB->getTerminator());
}

void DPCPPKernelWGLoopCreatorLegacyPass::getLoopsBoundaries(Function *F) {
  LoopSizes.clear();
  InitGIDs.clear();
  for (unsigned Dim = 0; Dim < NumDim; ++Dim) {
    InitGIDs.push_back(ConstZero);
  }
  // This must be consistent with header file local size positions!
  LoopSizes.push_back(F->getArg(3));
  LoopSizes.push_back(F->getArg(4));
  LoopSizes.push_back(F->getArg(5));
}

unsigned int
DPCPPKernelWGLoopCreatorLegacyPass::resolveDimension(unsigned int Dim) {
  if (Dim == 0)
    return VectorizedDim;
  else if (Dim > VectorizedDim)
    return Dim;
  else
    return Dim - 1;
}

void DPCPPKernelWGLoopCreatorLegacyPass::computeDimStr(unsigned Dim,
                                                       bool IsVector) {
  std::stringstream DimStream;
  DimStream << "dim_" << Dim << "_";
  if (IsVector)
    DimStream << "vector_";
  DimStr = DimStream.str();
}

LoopRegion DPCPPKernelWGLoopCreatorLegacyPass::addWGLoops(
    BasicBlock *KernelEntry, bool IsVector, ReturnInst *Ret, InstVecVec &LIDs,
    ValueVec &InitGIDs, SmallVector<Value *, 4> &LoopSizes) {

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
    if (LIDs[ResolvedDim].size()) {
      // TODO: Do I need base GID computations?
      replaceTIDsWithPHI(LIDs[ResolvedDim], InitGID, IncBy, Head,
                         Blocks.PreHeader, Latch);
    }
    // head, latch for the next loop are the pre header and exit
    // block respectively.
    Head = Blocks.PreHeader;
    Latch = Blocks.Exit;
  }
  return LoopRegion(Head, Latch);
}

void DPCPPKernelWGLoopCreatorLegacyPass::replaceTIDsWithPHI(
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

void DPCPPKernelWGLoopCreatorLegacyPass::processFunction(
    Function *F, Function *VectorFuncParam, unsigned PacketWidthParam) {
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
  ScalarRet = getFunctionData(F, LidCallsSc);

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

  LoopRegion WGLoopRegion = VectorFunc && NumDim
                                ? createVectorAndRemainderLoops()
                                : addWGLoops(ScalarEntry, false, ScalarRet,
                                             LidCallsSc, InitGIDs, LoopSizes);
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

} // namespace llvm
