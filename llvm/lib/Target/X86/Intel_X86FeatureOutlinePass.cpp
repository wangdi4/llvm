//===----------------------- FeatureOutlinePass.cpp -----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.-
//
//===----------------------------------------------------------------------===//
//
// This file implemented the FeatureOutliner pass.
// It outlines basic blocks that contain assume(has_feature()) calls into
// separate functions, s.t. each new function is labeled with the correct
// "target-features" metadata.
//
//===----------------------------------------------------------------------===//

#include "X86Subtarget.h"
#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetSubtargetInfo.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/CodeExtractor.h"

using namespace llvm;

class X86Subtarget;

#define DEBUG_TYPE "feature-outline"

namespace {

class FeatureOutliner : public ModulePass {
  public:
    static char ID;

    FeatureOutliner(const TargetMachine *TM = nullptr) : 
      ModulePass(ID), TM(TM) {
      initializeFeatureOutlinerPass(*PassRegistry::getPassRegistry());
    }
    bool runOnModule(Module &M) override;
    const char *getPassName() const override {return "Feature Outliner";}
  
  private:
    // Returns the subtarget features assumed in basicblock BB.
    // Features provided in Available are assumed to already be 
    // available in this basic block and are filtered out.
    uint64_t getAssumedFeatures(BasicBlock *BB, uint64_t Available);

    // Appends the features described by Features to the 
    // target-feature string Str. Str is an in/out parameter.
    // Returns true if the returned-string is non-empty.
    bool appendFeatureString(std::string &Str, uint64_t Features);

    static std::map<uint64_t, uint64_t> FeatureMapping;
    // TODO: There doesn't seem to be a good way to map a subtarget feature back
    // into a string (!). Fix that later, for now, have the mapping here.
    static std::map<uint64_t, StringRef> FeatureStrings;
    const TargetMachine *TM;

    typedef std::vector<BasicBlock*> Region;

  };
}

char FeatureOutliner::ID = 0;
INITIALIZE_TM_PASS(FeatureOutliner, "featureoutliner",
                   "Outline Subtarget Features", false, false)

// Entries set to 0 are either unneccessary, or it's unclear
// what to map them to.
std::map<uint64_t, uint64_t> FeatureOutliner::FeatureMapping = { 
  {0x00000001ULL, 0}, // generic
  {0x00000002ULL, 0}, // fpu
  {0x00000004ULL, X86::FeatureCMOV},
  {0x00000008ULL, X86::FeatureMMX},
  {0x00000010ULL, 0}, // fxsave
  {0x00000020ULL, X86::FeatureSSE1},
  {0x00000040ULL, X86::FeatureSSE2},
  {0x00000080ULL, X86::FeatureSSE3},
  {0x00000100ULL, X86::FeatureSSSE3},
  {0x00000200ULL, X86::FeatureSSE41},
  {0x00000400ULL, X86::FeatureSSE42},
  {0x00000800ULL, 0}, // mobve
  {0x00001000ULL, X86::FeaturePOPCNT},
  {0x00002000ULL, X86::FeaturePCLMUL},
  {0x00004000ULL, X86::FeatureAES},
  {0x00008000ULL, X86::FeatureF16C},
  {0x00010000ULL, X86::FeatureAVX},
  {0x00020000ULL, X86::FeatureRDRAND},
  {0x00040000ULL, X86::FeatureFMA},
  {0x00080000ULL, X86::FeatureBMI | X86::FeatureBMI2},
  {0x00100000ULL, X86::FeatureLZCNT},
  {0x00200000ULL, X86::FeatureHLE},
  {0x00400000ULL, X86::FeatureRTM},
  {0x00800000ULL, X86::FeatureAVX2},
  {0x01000000ULL, X86::FeatureDQI},
  {0x04000000ULL, 0}, // kncni
  {0x08000000ULL, X86::FeatureAVX512},
  {0x10000000ULL, X86::FeatureADX},
  {0x20000000ULL, X86::FeatureRDSEED},
  {0x40000000ULL, 0}, // avx512ifma52
  {0x100000000ULL, X86::FeatureERI},
  {0x200000000ULL, X86::FeaturePFI},
  {0x400000000ULL, X86::FeatureCDI},
  {0x800000000ULL, X86::FeatureSHA},
  {0x1000000000ULL, 0}, // mpx
  {0x2000000000ULL, X86::FeatureBWI},
  {0x4000000000ULL, X86::FeatureVLX},
  {0x8000000000ULL, 0} // avx512vbmi
};

std::map<uint64_t, StringRef> FeatureOutliner::FeatureStrings = { 
  {X86::FeatureCMOV,  "cmov"},
  {X86::FeatureMMX,   "mmx"},
  {X86::FeatureSSE1,  "sse"},
  {X86::FeatureSSE2,  "sse2"},
  {X86::FeatureSSE3,  "sse3"},
  {X86::FeatureSSSE3, "ssse3"},
  {X86::FeatureSSE41, "sse4.1"},
  {X86::FeatureSSE42, "sse4.2"},
  {X86::FeaturePOPCNT,"popcnt"},
  {X86::FeaturePCLMUL,"pclmul"},
  {X86::FeatureAES,   "aes"},
  {X86::FeatureF16C,  "f16c"},
  {X86::FeatureAVX,   "avx"},
  {X86::FeatureRDRAND,"rdrnd"},
  {X86::FeatureFMA,   "fma"},
  {X86::FeatureBMI,   "bmi"},
  {X86::FeatureBMI2,  "bmi2"},
  {X86::FeatureLZCNT, "lzcnt"},
  {X86::FeatureHLE,   "hle"},
  {X86::FeatureRTM,   "rtm"},
  {X86::FeatureAVX2,  "avx2"},
  {X86::FeatureDQI,   "avx512dq"},
  {X86::FeatureAVX512,"avx512f"},
  {X86::FeatureADX,   "adx"},
  {X86::FeatureRDSEED,"rdseed"},
  {X86::FeatureERI,   "avx512er"},
  {X86::FeaturePFI,   "avx512pf"},
  {X86::FeatureCDI,   "avx512cd"},
  {X86::FeatureSHA,   "sha"},
  {X86::FeatureBWI,   "avx512bw"},
  {X86::FeatureVLX,   "avx512vl"},
};

ModulePass *llvm::createFeatureOutlinerPass(const TargetMachine *TM) {
  return new FeatureOutliner(TM);
}

bool FeatureOutliner::appendFeatureString(
    std::string &Str, uint64_t Features) {
  // Go over all set bits in the Features bitset, and
  // append the right feature string to the provided string
  while (Features) {
    uint64_t CurrFeature = Features & ~(Features - 1);
    Features &= Features - 1;
    StringRef FeatureString = FeatureStrings[CurrFeature];
    if (Str == "")
      Str += "+";
    else
      Str += ",+";
    Str += FeatureString;
  }

  return (Str != "");
}

uint64_t FeatureOutliner::getAssumedFeatures(BasicBlock *BB, 
                                             uint64_t Available) {
  // TODO: Should probably check we don't have a has_feature that
  // gets used in a different way (e.g. assume(has_feature() & foo)),
  // and assert on it. We don't expect that to happen, but...
  uint64_t NewFeatures = 0;
  for (auto I = BB->begin(), E = BB->end(); I != E; ++I) {
    IntrinsicInst *Assume = dyn_cast<IntrinsicInst>(I);
    if (!Assume || Assume->getIntrinsicID() != Intrinsic::assume)
      continue;

    IntrinsicInst *Feature = dyn_cast<IntrinsicInst>(Assume->getOperand(0));
    if (!Feature || Feature->getIntrinsicID() != Intrinsic::has_feature)
      continue;

    ConstantInt *FeatureSet = 
      dyn_cast<ConstantInt>(Feature->getOperand(0));
    assert(FeatureSet && "Can only assume constant features");
    // If the feature is already available, filter it out.
    // TODO: Fix this to support multiple-features. Basically, need another FFS.
    if (FeatureSet)
      NewFeatures |= FeatureMapping[FeatureSet->getZExtValue()] & ~Available;
  }

  return NewFeatures;
}

bool FeatureOutliner::runOnModule(Module &M) {
  bool Changed = false;

  // TODO: What is the right CC to use?
  CallingConv::ID CC = CallingConv::X86_FastCall;

  // We are going to be adding functions. So prepare
  // the list of functions we want to work on in advance.
  std::vector<Function*> FunctionList;
  for (auto F = M.begin(), FE = M.end(); F != FE; ++F) {
    if (F->isDeclaration())
      continue;
    FunctionList.push_back(F);
  }

  for (auto FI = FunctionList.begin(), FE = FunctionList.end(); 
       FI != FE; ++FI) {
    Function *F = *FI;
    // Maps each basic block to the new features the region it
    // dominates needs to support. We need iteration over this
    // to be deterministic, so we use a MapVector instead of DenseMap
    MapVector<BasicBlock*, uint64_t> BBFeatureMap;
    // Maps each basic block to the set of basic blocks it dominates
    // (i.e regions). Note that those regions are single-entry but
    // may have multiple exits.
    DenseMap<BasicBlock*, Region*> RegionMap;

    // Used to keep track of the allocated regions. Due to the way
    // we traverse the dom-tree, the top-most regions come first.
    SmallVector<Region*, 8> RegionList;

    const TargetSubtargetInfo *TSI = TM->getSubtargetImpl(*F);
    uint64_t FunctionFeatures = TSI->getFeatureBits();

    StringRef CPUString =  F->getFnAttribute("target-cpu").getValueAsString();

    // Walk the dominator tree top-down, and create regions
    DominatorTree DT;
    DT.recalculate(*F);
    DomTreeNode *Root = DT.getRootNode();
    for (df_iterator<DomTreeNode*> I = df_begin(Root), E = df_end(Root);
          I != E; ++I) {
      uint64_t Available = 0;
      BasicBlock *BB = I->getBlock();
      // For the root, the available features are those available
      // for the whole function. For everything else, it's the features
      // known to be available in its immediate dominator;
      DomTreeNode *Parent = I->getIDom();
      Available = FunctionFeatures;
      if (Parent)
        Available |= BBFeatureMap[Parent->getBlock()];

      // TODO: This doesn't actually handle hierarchy correctly, since
      // only explicitly turned on features are assumed to be available.
      // So if a parent block assumes AVX and a child block assumes SSE,
      // we will not know that AVX implies SSE, and outline the SSE block
      // separately, with "+avx, +sse".
      uint64_t Features = getAssumedFeatures(BB, Available);
      BBFeatureMap[BB] = Features;

      // If this is the root block, or it has new features,
      // create a new region, otherwise add this block to
      // the parent's region.
      Region *BBRegion;
      if (Features || !Parent) {
        BBRegion = new Region();
        RegionList.push_back(BBRegion);
      } else {
        BBRegion = RegionMap[Parent->getBlock()];
      }
       
      BBRegion->push_back(BB);
      RegionMap[BB] = BBRegion;
    }

    // Walk the region list, in reverse order.
    // We need to outline from the bottom-up, otherwise we may 
    // get malformed regions.
    for (auto R = RegionList.rbegin(), RE = RegionList.rend();
         R != RE; ++R) {
      BasicBlock *BB = (**R)[0];
      uint64_t Features = BBFeatureMap[BB];
      if (!Features)
        continue;

      std::string FeatureString = 
        F->getFnAttribute("target-features").getValueAsString();
      appendFeatureString(FeatureString, Features);
      // If the entry block adds features, outlining doesn't make sense,
      // just mark the function itself to have these features.
      // TODO: Generalize this under the constraint that just walking up the
      // dom tree is wrong, because even if B post-dominates A, there may be
      // cases where control simply doesn't reach B.
      // TODO: Also handle the same constraint within a single BB (a user
      // function call preceding the assume)
      if (BB == &F->getEntryBlock()) {
        F->addFnAttr("target-features", FeatureString);
        Changed = true;
        continue;
      }

      CodeExtractor CE(*RegionMap[BB], &DT);

      // TODO: Fix CodeExtractor to preserve function metadata!
      // For now, we manually append the right target-cpu and target-features.
      if (Function *NewF = CE.extractCodeRegion()) {
        if (CPUString != "")
          NewF->addFnAttr("target-cpu", CPUString);
        NewF->addFnAttr("target-features", FeatureString);

        // Adjust the calling convention for both the funciton and the
        // call site.
        NewF->setCallingConv(CC);
        assert(NewF->hasOneUse() && "New function shold have one use");
        User *U = NewF->user_back();
        CallInst *NewCall = cast<CallInst>(U);
        NewCall->setCallingConv(CC);

        // Add the newly created blocks to its parent region.
        // Strictly speaking, this is only necessary if there are CFG
        // edges from one of them into the region.
        DomTreeNode *NewNode = DT.getNode(NewCall->getParent());
        DomTreeNode *IDomNode = NewNode->getIDom();
        BasicBlock *IDomBB = IDomNode->getBlock();

        for (df_iterator<DomTreeNode*> I = df_begin(NewNode), 
             E = df_end(NewNode); I != E; ++I) {
          BasicBlock *NewBlock = I->getBlock();
          RegionMap[IDomBB]->push_back(NewBlock);
        }

        Changed = true;
      }
    }

    // Clean up all of the regions
    for (auto &R : RegionList)
      delete R;
  }

  return Changed;
}


