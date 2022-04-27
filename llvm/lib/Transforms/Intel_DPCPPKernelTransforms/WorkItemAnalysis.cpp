//===- WorkItemAnalysis.cpp - Work item dependency analysis -----*- C++ -*-===//
//
// Copyright (C) 2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/WorkItemAnalysis.h"
#include "NameMangleAPI.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/BuiltinLibInfoAnalysis.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/SoaAllocaAnalysis.h"
#include <stack>

#define DEBUG_TYPE "dpcpp-kernel-work-item-analysis"

using namespace llvm;
using namespace DPCPPKernelCompilationUtils;

/// WorkItemAnalysis follows pointer arithmetic and index arithmetic when
/// calculating dependency properties. If a part of the index is lost due to a
/// transformation, it is acceptable.
/// This constant decides how many bits need to be preserved before we give up
/// on the analysis.
static constexpr unsigned MinIndexBitwidthToPreserve = 16;

/// Define shorter names for dependencies, for clarity of the conversion map.
static constexpr WorkItemInfo::Dependency UNI = WorkItemInfo::UNIFORM;
static constexpr WorkItemInfo::Dependency SEQ = WorkItemInfo::CONSECUTIVE;
static constexpr WorkItemInfo::Dependency PTR = WorkItemInfo::PTR_CONSECUTIVE;
static constexpr WorkItemInfo::Dependency STR = WorkItemInfo::STRIDED;
static constexpr WorkItemInfo::Dependency RND = WorkItemInfo::RANDOM;

/// Depdency maps which defines output dependency according to 2 input deps.
static constexpr WorkItemInfo::Dependency
    AddConversion[WorkItemInfo::NumDeps][WorkItemInfo::NumDeps] = {
        /*         UNI, SEQ, PTR, STR, RND */
        /* UNI */ {UNI, SEQ, PTR, STR, RND},
        /* SEQ */ {SEQ, STR, STR, STR, RND},
        /* PTR */ {PTR, STR, STR, STR, RND},
        /* STR */ {STR, STR, STR, STR, RND},
        /* RND */ {RND, RND, RND, RND, RND}};

static constexpr WorkItemInfo::Dependency
    SubConversion[WorkItemInfo::NumDeps][WorkItemInfo::NumDeps] = {
        /*         UNI, SEQ, PTR, STR, RND */
        /* UNI */ {UNI, STR, RND, RND, RND},
        /* SEQ */ {SEQ, RND, RND, RND, RND},
        /* PTR */ {PTR, RND, RND, RND, RND},
        /* STR */ {STR, RND, RND, RND, RND},
        /* RND */ {RND, RND, RND, RND, RND}};

static constexpr WorkItemInfo::Dependency
    MulConversion[WorkItemInfo::NumDeps][WorkItemInfo::NumDeps] = {
        /*         UNI, SEQ, PTR, STR, RND */
        /* UNI */ {UNI, STR, STR, STR, RND},
        /* SEQ */ {STR, RND, RND, RND, RND},
        /* PTR */ {STR, RND, RND, RND, RND},
        /* STR */ {STR, RND, RND, RND, RND},
        /* RND */ {RND, RND, RND, RND, RND}};

static constexpr WorkItemInfo::Dependency
    SelectConversion[WorkItemInfo::NumDeps][WorkItemInfo::NumDeps] = {
        /*         UNI, SEQ, PTR, STR, RND */
        /* UNI */ {UNI, STR, STR, STR, RND},
        /* SEQ */ {STR, SEQ, STR, STR, RND},
        /* PTR */ {STR, STR, PTR, STR, RND},
        /* STR */ {STR, STR, STR, STR, RND},
        /* RND */ {RND, RND, RND, RND, RND}};

static constexpr WorkItemInfo::Dependency
    GEPConversion[WorkItemInfo::NumDeps][WorkItemInfo::NumDeps] = {
        /*         UNI, SEQ, PTR, STR, RND */
        /* UNI */ {UNI, PTR, RND, RND, RND},
        /* SEQ */ {RND, RND, RND, RND, RND},
        /* PTR */ {PTR, RND, RND, RND, RND},
        /* STR */ {RND, RND, RND, RND, RND},
        /* RND */ {RND, RND, RND, RND, RND}};

static constexpr WorkItemInfo::Dependency
    GEPConversionForIndirection[WorkItemInfo::NumDeps][WorkItemInfo::NumDeps] =
        {
            /*         UNI, SEQ, PTR, STR, RND */
            /* UNI */ {UNI, PTR, RND, RND, RND},
            /* SEQ */ {RND, RND, RND, RND, RND},
            /* PTR */ {STR, RND, RND, RND, RND},
            /* STR */ {RND, RND, RND, RND, RND},
            /* RND */ {RND, RND, RND, RND, RND}};

INITIALIZE_PASS_BEGIN(WorkItemAnalysisLegacy, DEBUG_TYPE,
                      "Work item dependency analysis", false, true)
INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfoAnalysisLegacy)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(PostDominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(SoaAllocaAnalysisLegacy)
INITIALIZE_PASS_END(WorkItemAnalysisLegacy, DEBUG_TYPE,
                    "Work item dependency analysis", false, true)

char WorkItemAnalysisLegacy::ID = 0;

WorkItemAnalysisLegacy::WorkItemAnalysisLegacy(unsigned VectorizeDim)
    : FunctionPass(ID), VectorizeDim(VectorizeDim) {
  initializeWorkItemAnalysisLegacyPass(*PassRegistry::getPassRegistry());
}

bool WorkItemAnalysisLegacy::runOnFunction(Function &F) {
  auto *RTS = getAnalysis<BuiltinLibInfoAnalysisLegacy>()
                  .getResult()
                  .getRuntimeService();
  auto *DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  auto *PDT = &getAnalysis<PostDominatorTreeWrapperPass>().getPostDomTree();
  auto *LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  auto *SA = &getAnalysis<SoaAllocaAnalysisLegacy>().getResult();
  WIInfo.reset(new WorkItemInfo(F, RTS, DT, PDT, LI, SA));
  WIInfo->compute(VectorizeDim);
  return false;
}

void WorkItemAnalysisLegacy::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<BuiltinLibInfoAnalysisLegacy>();
  AU.addRequiredTransitive<DominatorTreeWrapperPass>();
  AU.addRequiredTransitive<PostDominatorTreeWrapperPass>();
  AU.addRequiredTransitive<LoopInfoWrapperPass>();
  AU.addRequired<SoaAllocaAnalysisLegacy>();
  AU.setPreservesAll();
}

FunctionPass *llvm::createWorkItemAnalysisLegacyPass(unsigned VectorizeDim) {
  return new WorkItemAnalysisLegacy();
}

AnalysisKey WorkItemAnalysis::Key;

WorkItemInfo WorkItemAnalysis::run(Function &F, FunctionAnalysisManager &AM) {
  auto *RTS = AM.getResult<ModuleAnalysisManagerFunctionProxy>(F)
                  .getCachedResult<BuiltinLibInfoAnalysis>(*F.getParent())
                  ->getRuntimeService();
  auto *DT = &AM.getResult<DominatorTreeAnalysis>(F);
  auto *PDT = &AM.getResult<PostDominatorTreeAnalysis>(F);
  auto *LI = &AM.getResult<LoopAnalysis>(F);
  auto *SA = &AM.getResult<SoaAllocaAnalysis>(F);
  WorkItemInfo WIInfo(F, RTS, DT, PDT, LI, SA);
  WIInfo.compute(/*Dim*/ 0);
  return WIInfo;
}

void WorkItemInfo::compute(unsigned Dim) {
  VectorizeDim = Dim;

  Changed[0].clear();
  Changed[1].clear();
  ChangedNew = &Changed[0];
  Deps.clear();
  DivBlocks.clear();
  DivPhiBlocks.clear();
  FullJoin = nullptr;
  InfluenceRegion.clear();
  PartialJoins.clear();
  DivergePartialJoins.clear();
  SchedulingConstraints.clear();

  // Compute the first iteration of the work-item dependency according to
  // ordering instructions. This ordering is generally good (as it usually
  // correlates well with dominance).
  for (auto &I : instructions(F))
    calculateDep(&I);

  // Recursively check if work-item dependency changes and if so re-calculates
  // it and marks the users for re-checking.
  // This procedure is guaranteed to converge since work-item dependency can
  // only become less uniform (uniform->consecutive->ptr->stride->random).
  updateDeps();

  LLVM_DEBUG(dbgs() << F.getName() << "\n");
  LLVM_DEBUG(for (auto &I
                  : instructions(F)) dbgs()
             << "WIA " << Deps[&I] << " " << I << " \n");

  // Concatenate predicated regions, when possible, to guarantee that predicated
  // regions appear one after the other will still appear one after the other
  // after linearization.
  // This is done in order to avoid cases where the linearizer accidently adds a
  // non-conditional branch from a dedicated to a non-dedicated node.
  // The concatenation is done to support non-nested divergent branches that
  // appear one after the other in the same nesting level.
  // This code cannot get into an infinite loop because an influence region of a
  // divergent branch span from the branch to its immediate post dominator.
  for (auto &SC : SchedulingConstraints) {
    auto &Dst = SC.second;
    // As long as the post dom's terminator starts a maximal divergent region.
    for (;;) {
      const auto It = SchedulingConstraints.find(Dst.back());
      if (It == SchedulingConstraints.end())
        break;
      const auto &Src = It->second;
      Dst.insert(Dst.end(), Src.begin(), Src.end());
    }
  }
}

void WorkItemInfo::setDepend(const Instruction *From, const Instruction *To) {
  assert(From && To && "Bad instruction");
  auto It = Deps.find(From);
  assert(It != Deps.end() && "Can't find them in Deps");

  const auto *FromBB = From->getParent();
  const auto *ToBB = To->getParent();

  if (DivBlocks.contains(FromBB))
    DivBlocks.insert(ToBB);
  if (DivPhiBlocks.contains(FromBB))
    DivPhiBlocks.insert(ToBB);
  LLVM_DEBUG(dbgs() << "setDepend Deps[" << *To << "] = Deps[" << *From
                    << "]: " << It->second << "\n");
  Deps[To] = It->second;
}

WorkItemInfo::Dependency WorkItemInfo::whichDepend(const Value *V) {
  assert(ChangedNew->empty() && "set should be empty before query");
  assert(V && "Bad value");

  auto It = Deps.find(V);
  auto Dep = (It != Deps.end())    ? It->second
             : isa<Instruction>(V) ? RANDOM
                                   : UNIFORM;

  LLVM_DEBUG(dbgs() << "whichDepend " << Dep << " " << *V << "\n");
  return Dep;
}

void WorkItemInfo::print(raw_ostream &OS) const {
  OS << "WorkItemAnalysis for function " << F.getName() << ":\n";
  for (const auto &I : instructions(F)) {
    auto It = Deps.find(&I);
    assert(It != Deps.end() &&
           "Expect all instructions to have WorkItem dependency at this point");
    OS.indent(2) << "";
    switch (It->second) {
    case UNIFORM:
      OS << "UNI";
      break;
    case CONSECUTIVE:
      OS << "SEQ";
      break;
    case PTR_CONSECUTIVE:
      OS << "PTR";
      break;
    case STRIDED:
      OS << "STR";
      break;
    case RANDOM:
      OS << "RND";
      break;
    default:
      llvm_unreachable("Unknown WorkItem dependency");
    }
    OS << " " << I << "\n";
  }
}

void WorkItemInfo::calculateDep(const Value *V) {
  assert(V && "Bad Value");

  // Not an instruction, must be a constant or an argument. Could this vector
  // type be of a constant which is not uniform?
  const auto *I = cast<Instruction>(V);

  // We only caulculate dependency on unset instructions if all their operands
  // were already given dependency. This is good for compile time since these
  // instructions will be visited again after the operands dependency is set.
  // An exception are phi nodes since they can be the ancestor of themselves in
  // the def-use chain. Note that in this case we force the phi to have the pre
  // header value already calculated.
  if (!hasDependency(I)) {
    unsigned UnsetOpNum = count_if(
        I->operands(), [&](const Value *Op) { return !hasDependency(Op); });
    if (isa<PHINode>(I)) {
      if (UnsetOpNum == I->getNumOperands()) {
        LLVM_DEBUG(dbgs() << "Skip PhiNode with all incoming values unset: "
                          << *I << "\n");
        return;
      }
    } else if (UnsetOpNum) {
      LLVM_DEBUG(dbgs() << "Skip non-PhiNode inst with unset operands: " << *I
                        << "\n");
      return;
    }
  }

  // Our initial value.
  Dependency Dep = hasDependency(I) ? getDependency(I) : UNIFORM;
  if (RANDOM == Dep) {
    LLVM_DEBUG(dbgs() << "Skip random dep: " << *I << "\n");
    return;
  }

  // LLVM does not have compile time polymorphism.
  // TODO: to make things faster we may want to sort the list below according to
  // the order of their probability of appearance.
  if (const auto *BI = dyn_cast<BinaryOperator>(I))
    Dep = calculateDep(BI);
  else if (const auto *UI = dyn_cast<UnaryOperator>(I))
    Dep = calculateDep(UI);
  else if (const auto *CI = dyn_cast<CallInst>(I))
    Dep = calculateDep(CI);
  else if (const auto *CI = dyn_cast<CmpInst>(I))
    Dep = calculateDepSimple(CI);
  else if (const auto *EEI = dyn_cast<ExtractElementInst>(I))
    Dep = calculateDepSimple(EEI);
  else if (const auto *GEP = dyn_cast<GetElementPtrInst>(I))
    Dep = calculateDep(GEP);
  else if (const auto *IEI = dyn_cast<InsertElementInst>(I))
    Dep = calculateDepSimple(IEI);
  else if (const auto *IVI = dyn_cast<InsertValueInst>(I))
    Dep = calculateDepSimple(IVI);
  else if (const auto *Phi = dyn_cast<PHINode>(I))
    Dep = calculateDep(Phi);
  else if (const auto *SVI = dyn_cast<ShuffleVectorInst>(I))
    Dep = calculateDepSimple(SVI);
  else if (const auto *SI = dyn_cast<StoreInst>(I))
    Dep = calculateDepSimple(SI);
  else if (I->isTerminator())
    Dep = calculateDepTerminator(I);
  else if (const auto *SI = dyn_cast<SelectInst>(I))
    Dep = calculateDep(SI);
  else if (const auto *AI = dyn_cast<AllocaInst>(I))
    Dep = calculateDep(AI);
  else if (const auto *CI = dyn_cast<CastInst>(I))
    Dep = calculateDep(CI);
  else if (const auto *EVI = dyn_cast<ExtractValueInst>(I))
    Dep = calculateDepSimple(EVI);
  else if (const auto *LI = dyn_cast<LoadInst>(I))
    Dep = calculateDepSimple(LI);
  else if (const auto *VAI = dyn_cast<VAArgInst>(I))
    Dep = calculateDep(VAI);

  updateDepMap(I, Dep);
}

WorkItemInfo::Dependency WorkItemInfo::calculateDep(const AllocaInst *I) {
  // Check if alloca instruction can be converted to SOA-alloca.
  return (SA->isSoaAllocaScalarRelated(I)) ? PTR_CONSECUTIVE : RANDOM;
}

WorkItemInfo::Dependency WorkItemInfo::calculateDep(const BinaryOperator *I) {
  // Calculate the dependency type for each of the operands.
  Value *Op0 = I->getOperand(0);
  Value *Op1 = I->getOperand(1);

  Dependency Dep0 = getDependency(Op0);
  Dependency Dep1 = getDependency(Op1);

  // For whatever binary operation, uniform returns uniform.
  if (UNIFORM == Dep0 && UNIFORM == Dep1)
    return UNIFORM;

  // FIXME: assumes that the X value does not cross the +/- border - risky!
  // The pattern (and (X, C)), where C preserves the lower k bits of the value,
  // is often used for truncating numbers in 64bit. We assume that the index
  // properties are not hurt by this.
  if (I->getOpcode() == Instruction::And) {
    auto *C0 = dyn_cast<ConstantInt>(I->getOperand(0));
    auto *C1 = dyn_cast<ConstantInt>(I->getOperand(1));
    // Use any of the constants. Instcombine places constants on Op1, so try Op1
    // first.
    if (C1 || C0) {
      ConstantInt *C = C1 ? C1 : C0;
      Dependency Dep = C1 ? Dep0 : Dep1;
      // Cannot look at bit pattern of huge integers.
      if (C->getBitWidth() < 65) {
        uint64_t V = C->getZExtValue();
        uint64_t PtrMask = (1 << MinIndexBitwidthToPreserve) - 1;
        // Zero all bits above the lower k bits that we are interested in.
        V &= PtrMask;
        // Make sure that all of the remaining bits are active.
        if (V == PtrMask)
          return Dep;
      }
    }
  }

  // FIXME: assumes that the X value does not cross +/- border - risky!
  // The pattern (ashr (shl X, C)C) is used for truncating of numbers in 64bit.
  // The constant C must leave at least 32bits of the original number.
  if (I->getOpcode() == Instruction::AShr) {
    BinaryOperator *SHL = dyn_cast<BinaryOperator>(I->getOperand(0));
    // We also allow add of uniform value between the ashr and shl instructions,
    // since instcombine creates this pattern when adding a constant.
    // The shl forces all low bits to be zero, so there can be no carry to the
    // high bits due to the addition. Addition with uniform preserves
    // WorkItem-dep.
    if (SHL && SHL->getOpcode() == Instruction::Add) {
      Value *AddedV = SHL->getOperand(1);
      if (getDependency(AddedV) == UNIFORM)
        SHL = dyn_cast<BinaryOperator>(SHL->getOperand(0));
    }

    if (SHL && SHL->getOpcode() == Instruction::Shl) {
      auto *CAshr = dyn_cast<ConstantInt>(I->getOperand(1));
      auto *CShl = dyn_cast<ConstantInt>(SHL->getOperand(1));
      const auto *AshrTy = cast<IntegerType>(I->getType());
      if (CAshr && CShl && CAshr->getZExtValue() == CShl->getZExtValue()) {
        // If word_width - shift_width >= 32 bits, return dep of original X.
        if ((AshrTy->getBitWidth() - CShl->getZExtValue()) >=
            MinIndexBitwidthToPreserve)
          return getDependency(SHL->getOperand(0));
      }
    }
  }

  switch (I->getOpcode()) {
  // Addition simply adds the stride value, except for ptr_consecutive which
  // is promoted to strided.
  // Another exception is when we subtract the tid: 1 - X which turns the tid
  // order to random.
  case Instruction::Add:
  case Instruction::FAdd:
    return AddConversion[Dep0][Dep1];
  case Instruction::Sub:
  case Instruction::FSub:
    return SubConversion[Dep0][Dep1];
  case Instruction::Mul:
  case Instruction::FMul:
  case Instruction::Shl:
    // If one of the sides is uniform, then we can adopt the other side
    // (strided * uniform is still strided). Stride size is K, where K is the
    // uniform input. An exception to this is ptr_consecutive, which is
    // promoted to strided.
    if (UNIFORM == Dep0 || UNIFORM == Dep1)
      return MulConversion[Dep0][Dep1];
    LLVM_FALLTHROUGH;
  default:
    // TODO Support more arithmetic if needed.
    return RANDOM;
  }
}

WorkItemInfo::Dependency WorkItemInfo::calculateDep(const CallInst *I) {
  // TODO: This function requires much more work, to be correct:
  //   Some functions (dot_prod, cross_prod) privide "measurable" behavior
  //   (Uniform->Strided).
  //   This information should also be obtained from RuntimeService somehow.

  // Check if call is TID-generator.
  bool IsTidGen;
  bool Err;
  unsigned Dim;
  std::tie(IsTidGen, Err, Dim) = RTService->isTIDGenerator(I);
  assert(!Err && "TIDGen inst receives non-constant input. Cannot vectorize!");
  // All WorkItem's are consecutive along the dimension.
  if (IsTidGen && Dim == VectorizeDim)
    return CONSECUTIVE;

  auto *Callee = I->getCalledFunction();
  assert(Callee && "Unexpected indirect function invocation");

  // For functions defined in this module, it is unsafe to assume anything.
  if (!Callee->isDeclaration())
    return RANDOM;

  // Check if the function is in the table of functions.
  StringRef CalleeName = Callee->getName();
  // WG functions must be packetized (although their results may be uniform)
  if (isWorkGroupBuiltin(CalleeName))
    return RANDOM;

  // FIXME remove code in following block that is for volcano.
  std::string ScalarFuncName = CalleeName.str();
  bool MaskedMemOp;
  {
    // If it is a fake builtin then we might need to demangle it before
    // demangling the fake part.
    auto IsFakeBuiltin = [](std::string &Name) {
      return Name.find("_f_v.") != std::string::npos;
    };
    auto IsMangledCall = [](std::string &Name) {
      return Name.find("maskedf_") != std::string::npos;
    };
    auto IsMangledLoad = [](std::string &Name) {
      return Name.find("masked_load_align") != std::string::npos;
    };
    auto IsMangledStore = [](std::string &Name) {
      return Name.find("masked_store_align") != std::string::npos;
    };

    // Check if the function is in the table of functions.
    StringRef ImageFunctions[] = {"read_imagei", "read_imageui", "write_imagei",
                                  "write_imageui"};

    auto Demangle = [&](const std::string &Name, bool Masked) {
      if (NameMangleAPI::isMangledName(Name.c_str())) {
        StringRef Stripped = NameMangleAPI::stripName(Name.c_str());
        if (Stripped.startswith("mask_")) {
          for (auto It = std::begin(ImageFunctions),
                    E = std::end(ImageFunctions);
               It != E; ++It) {
            // in the case of image functions, the Name should be unchanged,
            // since Masked functions are 'real functions' to be looked in the
            // reflection module
            if (Stripped.endswith(*It))
              return Name;
          }
        }
        if (!Masked)
          return std::string(Stripped);
      }

      if (!Masked)
        return Name;

      assert(Name.find("maskedf_") != Name.npos && "not a mangled function");
      // Format:
      // masked_83_function
      size_t Start =
          Name.find("maskedf_") + std::string("maskedf_").length() + 1;
      size_t Orig = Name.find("_", Start);
      assert(Orig != std::string::npos && "unable to find Original Name");
      return Name.substr(Orig + 1);
    };

    auto DemangleFakeBuiltin = [&](std::string &Name) {
      assert(IsFakeBuiltin(Name) && "not a mangled fake builtin function");
      // Format:
      // _f_v.function
      size_t Start = Name.find("_f_v.");
      // when DemangleFakeBuiltin is called fake_builtin_pefix should be at
      // Start
      assert(Start == 0);
      Start += std::string("_f_v.").length();
      return Name.substr(Start);
    };

    if (IsFakeBuiltin(ScalarFuncName)) {
      std::string FakeFuncName = ScalarFuncName;
      if (IsMangledCall(FakeFuncName))
        FakeFuncName = Demangle(FakeFuncName, true);
      ScalarFuncName = DemangleFakeBuiltin(FakeFuncName);
    }

    bool IsMangled = IsMangledCall(ScalarFuncName);
    MaskedMemOp =
        (IsMangledLoad(ScalarFuncName) || IsMangledStore(ScalarFuncName));
    // First remove any name-mangling (for example, masking), from the function
    // name
    if (IsMangled)
      ScalarFuncName = Demangle(ScalarFuncName, true);
  }

  // Check with the runtime whether we can say that the output of the call is
  // uniform in case all its operands are uniform.
  // Note that for OpenCL the runtime will say it is true for: get_global_id,
  // get_local_id, since on dimension 0 the isTIDGenerator should answer true,
  // and we will say the value is Consecutive. So here we cover dimension 1,2
  // which are uniform.
  bool UniformByOps = RTService->hasNoSideEffect(ScalarFuncName);
  // Look for the function in the builtin functions hash.
  if (!MaskedMemOp && !IsTidGen && !UniformByOps)
    return RANDOM;

  // Iterate over all input dependencies. If all are uniform, propagate it,
  // otherwise, return RANDOM.
  bool IsAllUniform = all_of(I->args(), [&](const Value *Arg) {
    return UNIFORM == getDependency(Arg);
  });

  // FIXME remove following code that is for volcano.
  // An allones and allzeros branches are uniform branch.
  if (CalleeName.contains("__ocl_allOne") ||
      CalleeName.contains("__ocl_allZero"))
    return UNIFORM;

  return IsAllUniform ? UNIFORM : RANDOM;
}

WorkItemInfo::Dependency WorkItemInfo::calculateDep(const CastInst *I) {
  auto *Op0 = I->getOperand(0);
  Dependency Dep0 = getDependency(Op0);
  // Independent remains independent.
  if (UNIFORM == Dep0)
    return Dep0;

  switch (I->getOpcode()) {
  case Instruction::SExt:
  case Instruction::FPTrunc:
  case Instruction::FPExt:
  case Instruction::PtrToInt:
  case Instruction::IntToPtr:
  case Instruction::AddrSpaceCast:
  case Instruction::UIToFP:
  case Instruction::SIToFP:
  case Instruction::FPToUI:
  case Instruction::FPToSI:
    return Dep0;
  case Instruction::BitCast:
  case Instruction::ZExt:
    return RANDOM;
  case Instruction::Trunc: {
    const Type *DstTy = I->getDestTy();
    const auto *IntTy = dyn_cast<IntegerType>(DstTy);
    return (IntTy && IntTy->getBitWidth() >= MinIndexBitwidthToPreserve)
               ? Dep0
               : RANDOM;
  }
  default:
    llvm_unreachable("unsupported opcode");
  }
}

WorkItemInfo::Dependency
WorkItemInfo::calculateDep(const GetElementPtrInst *I) {
  // Iterate over the all indices arguments except for the last.
  // Here we assume the pointer is the first operand.
  unsigned Num = I->getNumIndices();
  for (unsigned Idx = 1; Idx < Num; ++Idx)
    if (getDependency(I->getOperand(Idx)) != UNIFORM)
      return RANDOM;

  const Value *OpPtr = I->getOperand(0);
  Dependency DepPtr = getDependency(OpPtr);
  Dependency DepLastInd = getDependency(I->getOperand(Num));

  // SOA alloca related pointer will be turned into SOA format.
  // Thus, it is allowed to assume: PTR + UNI = PTR.
  bool IsIndirectGEP =
      OpPtr->getType() != I->getType() && !SA->isSoaAllocaScalarRelated(OpPtr);

  return IsIndirectGEP ? GEPConversionForIndirection[DepPtr][DepLastInd]
                       : GEPConversion[DepPtr][DepLastInd];
}

WorkItemInfo::Dependency WorkItemInfo::calculateDep(const PHINode *I) {
  SmallVector<Dependency, 4> Deps;
  for (unsigned Idx = 0, N = I->getNumIncomingValues(); Idx < N; ++Idx) {
    // For PHI we ignore unset incoming values, so cases like loop with
    // consecutive variable that is increased by uniform will be considered
    // consecutive.
    Value *Op = I->getIncomingValue(Idx);
    if (hasDependency(Op))
      Deps.push_back(getDependency(Op));
  }
  assert(!Deps.empty() &&
         "Should not reach here with all incoming values are unset");

  Dependency TotalDep = Deps[0];
  for (size_t Idx = 0, E = Deps.size(); Idx < E; ++Idx)
    TotalDep = SelectConversion[TotalDep][Deps[Idx]];
  return TotalDep;
}

WorkItemInfo::Dependency WorkItemInfo::calculateDep(const SelectInst *I) {
  auto *Op0 = I->getOperand(0); // mask

  // If mask is non-uniform, the select output can be a combination so we know
  // nothing about it.
  if (UNIFORM != getDependency(Op0))
    return RANDOM;

  Dependency Dep1 = getDependency(I->getOperand(1));
  Dependency Dep2 = getDependency(I->getOperand(2));
  // In cast of constant scalar select we can choose according to the mask.
  if (auto *C = dyn_cast<ConstantInt>(Op0))
    return C->getZExtValue() ? Dep1 : Dep2;
  // Select the 'weaker" dep, but if only one dep is PTR_CONSECUTIVE, it must be
  // promoted to STRIDED (as this data may propagate to Load/Store instructions.
  return SelectConversion[Dep1][Dep2];
}

WorkItemInfo::Dependency WorkItemInfo::calculateDep(const UnaryOperator *I) {
  return getDependency(I->getOperand(0));
}

WorkItemInfo::Dependency WorkItemInfo::calculateDep(const VAArgInst *I) {
  assert(false && "Are we supporting this ??");
  return RANDOM;
}

WorkItemInfo::Dependency
WorkItemInfo::calculateDepTerminator(const Instruction *I) {
  // Simply check that all operands are uniform, if so return uniform, else
  // random. Instruction has no return value. Just need to know if this inst is
  // uniform or not, because we may want to avoid predication if the control
  // flows in the function are uniform...
  switch (I->getOpcode()) {
  case Instruction::Br: {
    const auto *Br = cast<BranchInst>(I);
    // Unconditional branch is non TID-dependent.
    if (!Br->isConditional())
      return UNIFORM;
    return getDependency(Br->getCondition()) == UNIFORM ? UNIFORM : RANDOM;
  }
  case Instruction::Ret:
    // Return instruction is unconditional.
    return UNIFORM;
  case Instruction::IndirectBr:
    // TODO: Define the dependency requirements of IndirectBr.
  case Instruction::Switch:
    // TODO: Should this depend only on the condition, like branch?
  default:
    return RANDOM;
  }
}

WorkItemInfo::Dependency
WorkItemInfo::calculateDepSimple(const Instruction *I) {
  for (const Value *V : I->operands())
    if (getDependency(V) != UNIFORM)
      return RANDOM;
  return UNIFORM;
}

void WorkItemInfo::calcInfoForBranch(const Instruction *I) {
  DomTreeNode *PostDomNode =
      PDT->getNode(const_cast<BasicBlock *>(I->getParent()));

  // If we are in an infinite loop then there is no post-dominant.
  // In this case, we mark everything reachable from the divergent branch as its
  // influence region (conservative).
  if (PostDomNode) {
    // Because I is a conditional branch then it is not the last basic block
    // and therefore getIDom does not return null.
    assert(PostDomNode->getIDom() && "Post dominator cannot be null");
    FullJoin = PostDomNode->getIDom()->getBlock();
  } else {
    FullJoin = nullptr;
  }

  bool UpdatedFullJoin = true;
  std::vector<BasicBlock *> SchedConstraints;
  SmallPtrSet<BasicBlock *, 4> FullJoinLoopLatches;
  DenseSet<BasicBlock *> LeftSet;
  DenseSet<BasicBlock *> RightSet;

  // Iterate until we do not need to re-calculate the full join.
  while (UpdatedFullJoin) {
    UpdatedFullJoin = false;

    InfluenceRegion.clear();
    PartialJoins.clear();
    DivergePartialJoins.clear();
    SchedConstraints.clear();
    FullJoinLoopLatches.clear();
    LeftSet.clear();
    RightSet.clear();

    // Adding the root of the predicated region for the scheduling constraints.
    SchedConstraints.push_back(const_cast<BasicBlock *>(I->getParent()));

    Loop *FullJoinLoop = LI->getLoopFor(FullJoin);
    if (FullJoinLoop) {
      for (BasicBlock *PredBB : predecessors(FullJoinLoop->getHeader()))
        if (FullJoinLoop->contains(PredBB))
          FullJoinLoopLatches.insert(PredBB);
    }

    std::stack<BasicBlock *> WorkSet;

    for (unsigned Idx = 0; Idx < 2; ++Idx) { // I->getNumSuccessors() == 2
      BasicBlock *SuccBB = I->getSuccessor(Idx);
      if (SuccBB != FullJoin) {
        WorkSet.push(SuccBB);
        while (!WorkSet.empty()) {
          BasicBlock *CurrBB = WorkSet.top();
          WorkSet.pop();
          DivBlocks.insert(CurrBB); // Mark block as divergent.
          Loop *CurrLoop = LI->getLoopFor(CurrBB);

          // If full join is in the CurrLoop and we reachaed the latch node of
          // this loop, then we should abort, update the full join by finding
          // its first post-dominator which is outside CurrLoop and then
          // re-calculate new info for this branch.
          if (FullJoinLoop && FullJoinLoop == CurrLoop &&
              FullJoinLoopLatches.contains(CurrBB)) {
            UpdatedFullJoin = true;
            break;
          }

          if (Idx == 1 && LeftSet.contains(CurrBB))
            PartialJoins.insert(CurrBB);

          auto &BBSet = (Idx == 0) ? LeftSet : RightSet;
          BBSet.insert(CurrBB);

          if (InfluenceRegion.insert(CurrBB))
            SchedConstraints.push_back(CurrBB);

          for (BasicBlock *Succ : successors(CurrBB))
            if (Succ != FullJoin && !BBSet.contains(Succ))
              WorkSet.push(Succ);
        }
      }

      if (UpdatedFullJoin)
        break;
    }

    // If we need to update FullJoin, it means that during computing the
    // influence region we have reached the latch node of the loop where the
    // full join is located in. In this case, in order to be sound, we should
    // move the full join to the full join's first post-dominator outside it
    // loop and start the computation from the beginning. This process can also
    // be calculated incrementally by continuing the calculation from the
    // current full join.
    if (UpdatedFullJoin) {
      BasicBlock *NextFullJoin = FullJoin;
      Loop *NextFullJoinLoop = nullptr;
      // Find the first full join's post-dominator outside the post dominator's
      // loop.
      do {
        DomTreeNode *PostDomNode = PDT->getNode(NextFullJoin);
        // UpdatedFullJoin is true, so we are not in an infinite loop and
        // therefore, getNode does not return null.
        assert(PostDomNode && "getNode should not return null");
        // If the post dom is outside the loop then it cannot be the last block
        // and therefore, getIDom does not return null.
        auto *ImmDom = PostDomNode->getIDom();
        assert(ImmDom && "getIDom should not return null");
        NextFullJoin = ImmDom->getBlock();
        NextFullJoinLoop = LI->getLoopFor(NextFullJoin);
      } while (NextFullJoinLoop == FullJoinLoop);

      FullJoin = NextFullJoin;
      FullJoinLoop = NextFullJoinLoop;
    }
  }

  SchedConstraints.push_back(FullJoin);
  SchedulingConstraints[*(SchedConstraints.begin())] = SchedConstraints;
}

void WorkItemInfo::findDivergePartialJoins(const Instruction *I) {
  DenseSet<BasicBlock *> LeftSet;
  DenseSet<BasicBlock *> RightSet;

  for (BasicBlock *PartialJoin : PartialJoins) {
    LeftSet.clear();
    RightSet.clear();
    std::stack<BasicBlock *> WorkSet;

    // If this partial join does not contains phi nodes then goto the next node.
    auto FirstInstItr = PartialJoin->begin();
    if (!isa<PHINode>(&*FirstInstItr))
      continue;

    for (unsigned Idx = 0; Idx < 2; ++Idx) { // I->getNumSuccessors() == 2
      BasicBlock *SuccBB = I->getSuccessor(Idx);
      if (SuccBB == PartialJoin)
        continue;
      WorkSet.push(SuccBB);
      while (!WorkSet.empty()) {
        BasicBlock *CurrBB = WorkSet.top();
        WorkSet.pop();
        auto &BBSet = (Idx == 0) ? LeftSet : RightSet;
        BBSet.insert(CurrBB);
        for (BasicBlock *Succ : successors(CurrBB))
          if (Succ != PartialJoin && !BBSet.contains(Succ))
            WorkSet.push(Succ);
      }
    }

    bool ReachLeft = false;
    bool ReachRight = false;
    for (BasicBlock *PredBB : predecessors(PartialJoin)) {
      bool IsLeft = LeftSet.contains(PredBB);
      bool IsRight = RightSet.contains(PredBB);
      // If we saw a path from the left successor of cbr to a predecessor and
      // now we see a path from the right successor to a different one.
      // Or the other way around ...
      if ((IsRight && ReachLeft) || (IsLeft && ReachRight)) {
        DivergePartialJoins.insert(PartialJoin);
        break;
      }
      ReachLeft |= IsLeft;
      ReachRight |= IsRight;
    }
  }
}

WorkItemInfo::Dependency WorkItemInfo::getDependency(const Value *V) {
  return Deps.insert({V, UNIFORM}).first->second;
}

bool WorkItemInfo::hasDependency(const Value *V) {
  if (!isa<Instruction>(V))
    return true;
  return Deps.count(V);
}

void WorkItemInfo::markDependentPhiRandom() {
  auto PhiHasEqualIncomingValues = [](PHINode *Phi) {
    for (unsigned Idx = 1, E = Phi->getNumIncomingValues(); Idx < E; ++Idx)
      if (Phi->getIncomingValue(0) != Phi->getIncomingValue(Idx))
        return false;
    return true;
  };

  // Full join.
  // Note that a branch can have null immediate post-dominator when a function
  // has multiple exits in llvm-IR.
  if (FullJoin) {
    DivPhiBlocks.insert(FullJoin);
    for (auto &I : *FullJoin) {
      auto *Phi = dyn_cast<PHINode>(&I);
      if (!Phi)
        break;
      if (!PhiHasEqualIncomingValues(Phi))
        updateDepMap(Phi, RANDOM);
    }
  }

  // Partial joins.
  for (BasicBlock *BB : DivergePartialJoins) {
    DivPhiBlocks.insert(BB);
    for (auto &I : *BB) {
      auto *Phi = dyn_cast<PHINode>(&I);
      if (!Phi)
        break;
      if (!PhiHasEqualIncomingValues(Phi))
        updateDepMap(Phi, RANDOM);
    }
  }
}

void WorkItemInfo::updateDepMap(const Instruction *I, Dependency Dep) {
  // Return if the value is not changed.
  if (hasDependency(I) && Dep == getDependency(I))
    return;
  LLVM_DEBUG(dbgs() << "updateDepMap " << Dep << " " << *I << "\n");
  // Save the new value of this instruction.
  Deps[I] = Dep;

  // Register for update all of the dependent values of this updated
  // instruction.
  for (const User *U : I->users())
    ChangedNew->insert(U);

  if (!I->isTerminator() || Dep == UNIFORM)
    return;

  // Divergent branch, trigger updates due to control-dependence.
  const auto *Br = dyn_cast<BranchInst>(I);
  if (!Br || !Br->isConditional())
    return;

  DivBranchesQueue.push(Br);
  // Due to data structures sharing, every divergent branch should be handled
  // separately. Therefore, we use a queue to guarantee that newly random
  // branches, discovered during branch divergent propagation, are propagated
  // only termination of the previous divergent branch propagation.
  if (DivBranchesQueue.size() == 1) {
    do {
      updateCfDependency(DivBranchesQueue.front());
      DivBranchesQueue.pop();
    } while (!DivBranchesQueue.empty());
  }
}

void WorkItemInfo::updateDeps() {
  // As long as we have values to update.
  while (!ChangedNew->empty()) {
    // swap between changedSet pointers - recheck the newChanged(now old)
    auto *ChangedOld = ChangedNew;
    ChangedNew = (ChangedNew == &Changed[0]) ? &Changed[1] : &Changed[0];
    // Clear the ChangedNew set so it will be filled with the users of
    // instruction which their work-item dependency changed during the current
    // iteration.
    ChangedNew->clear();
    // Update all changed values.
    for (const auto *V : *ChangedOld) {
      // Remove first instruction.
      // Calculate its new dependency value.
      calculateDep(V);
    }
  }
}

void WorkItemInfo::updateCfDependency(const Instruction *I) {
  assert(I->isTerminator() && "Expect a terminator instruction");
  assert(isa<BranchInst>(I) && dyn_cast<BranchInst>(I)->isConditional() &&
         "Branch has to be a conditional branch");
  assert(I->getNumSuccessors() == 2 &&
         "Supports only for conditional branches with two successors");

  auto *BB = const_cast<BasicBlock *>(I->getParent());

  // If the root block is marked as divergent then we should not add scheduling
  // constraints for this region because it is part of a larger region that is
  // going to be predicated.
  // If we will add every predicated region then we might bet a conflict at the
  // linearizer that caused by commoning.
  bool ShouldUpdateConstraints = !isDivergentBlock(BB);

  calcInfoForBranch(I);

  findDivergePartialJoins(I);

  // Mark each phi node in a join or a partial join as divergent.
  markDependentPhiRandom();

  // Walk through all the instructions in the influence-region.
  for (auto *DefBB : InfluenceRegion) {
    // A node in the influence region of a divergent branch may not have an
    // incoming edge from a non-divergent block (unless its the immediate
    // post dominator which contains a random terminator).
    // Therefore, in case such an edge exists we need to mark the terminator of
    // the immediate dominator of the edge successor as random.
    // The only exception that this is allowed is in loops and because loops has
    // preheaders with non-conditional branches then the following will work for
    // these as well.
    for (auto *PredBB : predecessors(DefBB)) {
      if (isDivergentBlock(PredBB) || PredBB == BB)
        continue;
      // Because DefBB is divergent and PredBB is not, the idom of DefBB should
      // also be a dom of PredBB and therefore, such a dominator exists.
      assert(DT->getNode(DefBB) && DT->getNode(DefBB)->getIDom() &&
             "dominator cannot be null");
      auto *ImmDom = DT->getNode(DefBB)->getIDom()->getBlock();
      assert(ImmDom && "ImmDom cannot be null");

      Instruction *T = ImmDom->getTerminator();
      auto *Br = cast<BranchInst>(T);

      // FIXME remove code in the following block that is for volcano.
      bool BranchIsAllOnes;
      bool BranchIsAllZeroes;
      {
        // Return the terminator of BB if it is a branch conditional on
        // allones. Otherwise returns NULL.
        auto GetAllOnesBranch = [](BasicBlock *BB) -> BranchInst * {
          Instruction *Term = BB->getTerminator();
          assert(Term && "terminator cannot be null");
          BranchInst *Br = dyn_cast<BranchInst>(Term);
          if (!Br)
            return nullptr;

          if (Br->isConditional()) {
            CallInst *CondCall = dyn_cast<CallInst>(Br->getCondition());
            if (CondCall && CondCall->getCalledFunction())
              if (CondCall->getCalledFunction()->getName().contains(
                      "__ocl_allOne"))
                return Br;
          }
          return nullptr;
        };

        // allones and allzeros branches are uniform.
        BranchIsAllOnes = (GetAllOnesBranch(Br->getParent()) != nullptr);
        CallInst *CI = Br->isConditional()
                           ? dyn_cast<CallInst>(Br->getCondition())
                           : nullptr;
        BranchIsAllZeroes =
            CI && CI->getCalledFunction() &&
            CI->getCalledFunction()->getName().contains("__ocl_allZero");
      }

      if (Br->isConditional() && !BranchIsAllOnes && !BranchIsAllZeroes) {
        updateDepMap(T, RANDOM);
        // This region is going to be part of a larger region that is going to
        // be predicated.
        ShouldUpdateConstraints = false;
      }
      break;
    }

    for (auto &DefI : *DefBB) {
      // If DefI is random then its randomness will propagate to its usages in
      // the regular way and no control flow into propagation is needed.
      if (hasDependency(&DefI) && getDependency(&DefI) == RANDOM)
        continue;

      // Look at the users.
      for (User *U : DefI.users()) {
        auto *UserI = dyn_cast<Instruction>(U);
        if (!UserI)
          continue;
        BasicBlock *UserBB = UserI->getParent();
        if (UserBB == DefBB) {
          // Local def-use, not related to control-dependence.
          continue;
        }

        if (UserBB == FullJoin || PartialJoins.contains(UserBB)) {
          // We can check whether the (partial) join is a loop exit and change
          // the algorithm. This might increase accuracy in case there are gotos
          // but seems like redundant computation for our case.
          // For now we'll mark a usage in every join/partial join as random.
          // We might changed it in the future.
          updateDepMap(UserI, RANDOM);
        } else {
          // Mark each usage not in the influence region as random.
          if (!InfluenceRegion.contains(UserBB))
            updateDepMap(UserI, RANDOM);
        }
      }
    }
  }

  if (!ShouldUpdateConstraints)
    SchedulingConstraints.erase(BB);

  InfluenceRegion.clear();
}
