#if INTEL_FEATURE_SW_ADVANCED
//===--------------------- Intel_IPPredOpt.cpp ----------------------------===//
//
// Copyright (C) 2021-2023 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the optimization that hoists condition checks
// across function calls to avoid unnecessary computations.
//
// Ex:
//   Before:
//     if (contains(this)) {
//       if (this->field0) {
//         if (this->field2->vtable->function5 == foo) {
//           do_some_real_thing_1();
//         } else if (this->field2->vtable->function5 == bar) {
//           do_some_real_thing_2();
//         }
//       }
//     }
//
// Let us assume "contains" is not a small function and doesn't have any
// side effects. If value of this->field2->vtable->function5 is neither
// "bar" nor "foo", then there is no need to compute value of "contains()" call.
// We could transform the code like below to avoid unnecessary computations
// if we can prove that there are no side effects with "contains()" call.
//
//   After:
//     if (this->field0 && (this->field2->vtable->function5 == foo ||
//         this->field2->vtable->function5 == bar)) {
//       if (contains(this)) {
//         if (this->field2->vtable->function5 == foo) {
//           do_some_real_thing_1();
//         } else if (this->field2->vtable->function5 == bar) {
//           do_some_real_thing_2();
//         }
//       }
//     }
//
// This is implemented as Module Pass even though the transformation
// is not global since it is required to do analysis across functions.
//

#include "llvm/Transforms/IPO/Intel_IPPredOpt.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/Intel_Andersens.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/Analysis/TypeMetadataUtils.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/IPO.h"

using namespace llvm;

#define DEBUG_TYPE "ippredopt"

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// This option is mainly used by LIT tests.
//
static cl::opt<bool> IPPredDumpTargetFunctions(
    "ippred-dump-target-functions", cl::init(false), cl::ReallyHidden,
    cl::desc("Dump target functions for virtual function calls"));

#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

// Main class to implement the transformation.
class IPPredOptImpl {

public:
  IPPredOptImpl(Module &M, WholeProgramInfo &WPInfo) : M(M), WPInfo(WPInfo){};
  ~IPPredOptImpl(){};
  bool run(void);

private:
  Module &M;
  WholeProgramInfo &WPInfo;
  DenseMap<Metadata *, SmallSet<std::pair<GlobalVariable *, uint64_t>, 4>>
      TypeIdMap;

  void buildTypeIdMap();
  bool getVirtualPossibleTargets(CallBase &CB,
                                 SetVector<Function *> &TargetFunctions);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void dumpTargetFunctions(void);
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
};

// Build type identification map for Vtables.
void IPPredOptImpl::buildTypeIdMap() {
  SmallVector<MDNode *, 2> Types;
  for (GlobalVariable &GV : M.globals()) {
    Types.clear();
    GV.getMetadata(LLVMContext::MD_type, Types);
    if (GV.isDeclaration() || Types.empty())
      continue;

    for (MDNode *Type : Types) {
      Metadata *TypeID = Type->getOperand(1).get();

      uint64_t Offset =
          cast<ConstantInt>(
              cast<ConstantAsMetadata>(Type->getOperand(0))->getValue())
              ->getZExtValue();

      TypeIdMap[TypeID].insert(std::make_pair(&GV, Offset));
    }
  }
}

// Get possible target functions using type identification map.
bool IPPredOptImpl::getVirtualPossibleTargets(
    CallBase &CB, SetVector<Function *> &TargetFunctions) {
  assert(!CB.getCalledFunction() && "Expected indirect call");

  LLVM_DEBUG(dbgs() << "Collecting possible targets for: " << CB << "\n");
  const Instruction *PrevI = CB.getPrevNode();
  if (!PrevI || !isa<LoadInst>(PrevI)) {
    LLVM_DEBUG(dbgs() << "    No LoadInst Found: "
                      << "\n");
    return false;
  }
  auto *LI = cast<LoadInst>(PrevI);
  PrevI = PrevI->getPrevNode();
  if (PrevI && isa<GetElementPtrInst>(PrevI))
    PrevI = PrevI->getPrevNode();

  if (!PrevI || !isa<IntrinsicInst>(PrevI)) {
    LLVM_DEBUG(dbgs() << "    No Assume call Found: "
                      << "\n");
    return false;
  }
  auto *AI = cast<IntrinsicInst>(PrevI);
  if (AI->getIntrinsicID() != Intrinsic::assume) {
    LLVM_DEBUG(dbgs() << "    No Assume intrinsic Found: "
                      << "\n");
    return false;
  }
  PrevI = PrevI->getPrevNode();
  if (!PrevI || !isa<IntrinsicInst>(PrevI)) {
    LLVM_DEBUG(dbgs() << "    No typetest call Found: "
                      << "\n");
    return false;
  }
  auto *TI = cast<CallInst>(PrevI);
  if (TI->getIntrinsicID() != Intrinsic::type_test) {
    LLVM_DEBUG(dbgs() << "    No typetest intrinsic Found: "
                      << "\n");
    return false;
  }

  auto *TypeId = cast<MetadataAsValue>(TI->getArgOperand(1))->getMetadata();
  const Value *Object = LI->getPointerOperand();
  auto DL = LI->getFunction()->getParent()->getDataLayout();
  APInt ObjectOffset(DL.getTypeSizeInBits(Object->getType()), 0);
  Object->stripAndAccumulateConstantOffsets(DL, ObjectOffset,
                                            /* AllowNonInbounds */ true);

  for (auto &VTableInfo : TypeIdMap[TypeId]) {
    GlobalVariable *VTable = VTableInfo.first;
    uint64_t VTableOffset = VTableInfo.second;

    Function *Caller = CB.getFunction();
    LLVM_DEBUG(dbgs() << "    VTable: " << *VTable << "\n");
    LLVM_DEBUG(dbgs() << "    VTableOffset: " << VTableOffset << "\n");
    LLVM_DEBUG(dbgs() << "    ObjectOffset: " << ObjectOffset << "\n");
    Constant *Ptr = getPointerAtOffset(
        VTable->getInitializer(), VTableOffset + ObjectOffset.getZExtValue(),
        *Caller->getParent());
    if (!Ptr) {
      LLVM_DEBUG(dbgs() << "    Can't find pointer in vtable: "
                        << "\n");
      return false;
    }

    auto TargetFunc = dyn_cast<Function>(Ptr->stripPointerCasts());
    if (!TargetFunc) {
      LLVM_DEBUG(dbgs() << "    vtable entry is not function pointer: "
                        << "\n");
      return false;
    }

    TargetFunctions.insert(TargetFunc);
    LLVM_DEBUG(dbgs() << "    Adding target function: " << TargetFunc->getName()
                      << "\n");
  }

  return true;
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
void IPPredOptImpl::dumpTargetFunctions(void) {
  for (Function &F : M) {
    if (F.isDeclaration() || F.isIntrinsic())
      continue;
    for (auto &I : instructions(&F)) {
      auto CB = dyn_cast<CallBase>(&I);
      if (!CB || CB->getCalledFunction())
        continue;

      dbgs() << F.getName() << "  --  " << *CB << "\n";
      SetVector<Function *> TargetFunctions;
      if (!getVirtualPossibleTargets(*CB, TargetFunctions) ||
          TargetFunctions.empty()) {
        dbgs() << " Can't find possible targets \n";
        continue;
      }
      for (auto TF : TargetFunctions)
        dbgs() << "        " << TF->getName() << "\n";
    }
  }
}
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

bool IPPredOptImpl::run(void) {

  LLVM_DEBUG(dbgs() << "  IP Pred Opt: Started\n");
  auto TTIAVX2 = TargetTransformInfo::AdvancedOptLevel::AO_TargetHasIntelAVX2;
  if (!WPInfo.isWholeProgramSafe() || !WPInfo.isAdvancedOptEnabled(TTIAVX2)) {
    LLVM_DEBUG(dbgs() << "    Failed: Whole Program or target\n");
    return false;
  }

  buildTypeIdMap();

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  if (IPPredDumpTargetFunctions)
    dumpTargetFunctions();
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  // TODO: Add more code here
  return false;
}

namespace {

struct IPPredOptLegacyPass : public ModulePass {
public:
  static char ID; // Pass identification, replacement for typeid
  IPPredOptLegacyPass(void) : ModulePass(ID) {
    initializeIPPredOptLegacyPassPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<WholeProgramWrapperPass>();
    AU.addPreserved<WholeProgramWrapperPass>();
    AU.addPreserved<AndersensAAWrapperPass>();
    AU.addPreserved<GlobalsAAWrapperPass>();
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M))
      return false;
    auto WPInfo = getAnalysis<WholeProgramWrapperPass>().getResult();
    IPPredOptImpl IPPredOptI(M, WPInfo);
    return IPPredOptI.run();
  }
};

} // namespace

char IPPredOptLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(IPPredOptLegacyPass, "ippredopt", "ippredopt", false,
                      false)
INITIALIZE_PASS_DEPENDENCY(WholeProgramWrapperPass)
INITIALIZE_PASS_END(IPPredOptLegacyPass, "ippredopt", "ippredopt", false, false)

ModulePass *llvm::createIPPredOptLegacyPass(void) {
  return new IPPredOptLegacyPass();
}

IPPredOptPass::IPPredOptPass(void) {}

PreservedAnalyses IPPredOptPass::run(Module &M, ModuleAnalysisManager &AM) {
  auto &WPInfo = AM.getResult<WholeProgramAnalysis>(M);

  IPPredOptImpl IPPredOptI(M, WPInfo);
  if (!IPPredOptI.run())
    return PreservedAnalyses::all();
  auto PA = PreservedAnalyses();
  PA.preserve<AndersensAA>();
  PA.preserve<GlobalsAA>();
  PA.preserve<WholeProgramAnalysis>();
  return PA;
}

#endif // INTEL_FEATURE_SW_ADVANCED
