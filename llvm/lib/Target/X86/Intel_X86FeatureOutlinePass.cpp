//===----------------------- FeatureOutlinePass.cpp -----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is Intel-private, and has not been upstreamed to LLVM.org
//
//===----------------------------------------------------------------------===//
//
// This file implements the FeatureOutliner pass.
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

// This is turned off by default so as not to affect existing LIT tests
// It needs to be turned on by tests that require it, and by the standard
// compilation flow.
static cl::opt<bool>
ConsistentVectorABI("consistent-vector-abi", cl::init(false),
  cl::Hidden,
  cl::desc("Passing vectors implies having the subtarget feature"));

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
    typedef std::vector<BasicBlock*> Region;

    // Returns the subtarget features assumed in basicblock BB.
    // Features provided in Available are assumed to already be 
    // available in this basic block and are filtered out.
    FeatureBitset getAssumedFeatures(BasicBlock *BB, 
                                     const FeatureBitset& Available);

    // Appends the features described by Features to the 
    // target-feature string Str. Str is an in/out parameter.
    // Returns true if the returned-string is non-empty.
    bool appendFeatureString(std::string &Str, const FeatureBitset &Features);

    // Maps the externally used feature flags into the internal representation.
    // (Why not expose the internal enum to the user? Because, hopefully,
    // soon it will not be an enum, but a larger-than-64-bit bitset.)
    FeatureBitset translateExternalFeatures(uint64_t Features) const;

    // Returns the subtarget feature implied by the vector type
    FeatureBitset impliedFeature(const VectorType *VT) const;

    // Spills vector values that live in-between regions where the vector
    // type is native (e.g. situations where we have an AVX-block that produces
    // a ymm variable, and an AVX-block that consumes the ymm variable, but the
    // intervening blocks only have SSE).
    void spillLiveVectors(DenseMap<BasicBlock*, Region*> &RegionMap, 
      Region &BBRegion, MapVector<BasicBlock*, FeatureBitset> &BBFeatureMap,
      DominatorTree *DT, const FeatureBitset &NewFeatures) const;

    // Maps the user-visible feature enum into the internal representation
    static std::map<uint64_t, FeatureBitset> FeatureMapping;

    // TODO: There doesn't seem to be a good way to map a subtarget feature back
    // into a string (!). Fix that later, for now, have the mapping here.
    static std::map<uint64_t, StringRef> FeatureStrings;
    const TargetMachine *TM;
  };
}

char FeatureOutliner::ID = 0;
INITIALIZE_TM_PASS(FeatureOutliner, "featureoutliner",
                   "Outline Subtarget Features", false, false)

// Entries set to 0 are either unneccessary, or it's unclear
// what to map them to.
std::map<uint64_t, FeatureBitset> FeatureOutliner::FeatureMapping = { 
  {0x00000001ULL, {}}, // generic
  {0x00000002ULL, {}}, // fpu
  {0x00000004ULL, {X86::FeatureCMOV}},
  {0x00000008ULL, {X86::FeatureMMX}},
  {0x00000010ULL, {}}, // fxsave
  {0x00000020ULL, {X86::FeatureSSE1}},
  {0x00000040ULL, {X86::FeatureSSE2}},
  {0x00000080ULL, {X86::FeatureSSE3}},
  {0x00000100ULL, {X86::FeatureSSSE3}},
  {0x00000200ULL, {X86::FeatureSSE41}},
  {0x00000400ULL, {X86::FeatureSSE42}},
  {0x00000800ULL, {}}, // mobve
  {0x00001000ULL, {X86::FeaturePOPCNT}},
  {0x00002000ULL, {X86::FeaturePCLMUL}},
  {0x00004000ULL, {X86::FeatureAES}},
  {0x00008000ULL, {X86::FeatureF16C}},
  {0x00010000ULL, {X86::FeatureAVX}},
  {0x00020000ULL, {X86::FeatureRDRAND}},
  {0x00040000ULL, {X86::FeatureFMA}},
  {0x00080000ULL, {X86::FeatureBMI, X86::FeatureBMI2}},
  {0x00100000ULL, {X86::FeatureLZCNT}},
  {0x00200000ULL, {X86::FeatureHLE}},
  {0x00400000ULL, {X86::FeatureRTM}},
  {0x00800000ULL, {X86::FeatureAVX2}},
  {0x01000000ULL, {X86::FeatureDQI}},
  {0x04000000ULL, {}}, // kncni
  {0x08000000ULL, {X86::FeatureAVX512}},
  {0x10000000ULL, {X86::FeatureADX}},
  {0x20000000ULL, {X86::FeatureRDSEED}},
  {0x40000000ULL, {}}, // avx512ifma52
  {0x100000000ULL, {X86::FeatureERI}},
  {0x200000000ULL, {X86::FeaturePFI}},
  {0x400000000ULL, {X86::FeatureCDI}},
  {0x800000000ULL, {X86::FeatureSHA}},
  {0x1000000000ULL, {}}, // mpx
  {0x2000000000ULL, {X86::FeatureBWI}},
  {0x4000000000ULL, {X86::FeatureVLX}},
  {0x8000000000ULL, {}} // avx512vbmi
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

void FeatureOutliner::spillLiveVectors(
       DenseMap<BasicBlock*, Region*> &RegionMap, Region &BBRegion,
       MapVector<BasicBlock*, FeatureBitset> &BBFeatureMap,
       DominatorTree *DT, const FeatureBitset &NewFeatures) const {
  // Walk over all the values that are live-in to the region.
  // If any vector values enter the region, make sure they are spilled
  // and reloaded. This will involve splitting the entry block of the region
  // and adding the spills in the first part, and the reloads in the second,
  // where only the second block is actually part of the region.
  
  // Maps each "external" vector value to all its internal users.
  std::map<Instruction*, std::vector<Instruction*>> UseMap;

  BasicBlock *RegionHeader = BBRegion.front();

  for (BasicBlock *BB : BBRegion) {
    for (Instruction &I : *BB) {
      // PHIs in the region header are considered external, not internal,
      // so skip then.
      if (BB == RegionHeader && isa<PHINode>(I))
        continue;

      for (Value *Op : I.operands()) {
        VectorType *VT = dyn_cast<VectorType>(Op->getType());
        if (!VT)
          continue;

        // We only care if the feature implied by the vector type
        // was missing in the parent region but is present now.
        if ((NewFeatures & impliedFeature(VT)) == 0)
          continue;

        // We only care about instructions
        Instruction *Source = dyn_cast<Instruction>(Op);
        if (!Source)
          continue;

        // If the source basic block does not dominate the user,
        // then this isn't really an input into the user's region.
        // Rather, this is the output from a more inner region that
        // feeds into a phi in the outer region. In this case,
        // it will be returned by reference (the code extractor takes
        // care of this), so no need to do anything.
        if (!DT->dominates(Source, &I))
          continue;

        BasicBlock *SourceBB = Source->getParent();

        // As above, PHIs in the header are considered external
        if (RegionMap.lookup(SourceBB) != &BBRegion || 
            (SourceBB == RegionHeader && isa<PHINode>(Source)))
          UseMap[Source].push_back(&I);
      }
    }
  }

  // If we didn't find any instructions, nothing to do.
  if (UseMap.empty())
    return;

  // We have at least one value that needs to be spilled.
  // Split the region header and use the new block for spilling.
  // Unfortunately, splitBlock does "the wrong thing" since the
  // new block is below the old one, not above. So switch them
  // around.

  // Note that this means that if a loop header is in the region,
  // but the pre-header is out, we'll split the loop header.
  // (And this will happen, because we don't propagate markers
  // upwards in the DT).
  // This means we'll have a function call in a loop, instead
  // of hoisting the entire loop out.
  // TODO: Fix the above.
  BasicBlock *PreHeader = RegionHeader;
  RegionHeader = SplitBlock(RegionHeader, RegionHeader->getFirstNonPHI(), DT);
  
  // Update the regions:
  // The old header should belong to the region "just above"
  // The new header should replace the old header in the current region.
  BBRegion[0] = RegionHeader;
  RegionMap[RegionHeader] = &BBRegion;
  BasicBlock *IDom = DT->getNode(PreHeader)->getIDom()->getBlock();
  Region *IDomRegion = RegionMap[IDom];
  IDomRegion->push_back(PreHeader);
  RegionMap[PreHeader] = IDomRegion;
  
  // TODO: This does the right thing, but the commented out code doesn't.
  // My bug? A compiler bug? A bug in the implementation of MapVector?
  auto PreHeaderFeatures = BBFeatureMap[PreHeader];
  BBFeatureMap[RegionHeader] = PreHeaderFeatures;
  //BBFeatureMap[RegionHeader] = (uint64_t)BBFeatureMap[PreHeader];

  // Now do the actual spilling.
  Function *F = RegionHeader->getParent();
  BasicBlock *FuncEntry = &F->getEntryBlock();

  for (auto I : UseMap) {
    Instruction *Used = I.first;
    Type *VT = Used->getType();
    // Spill in the pre-header, reload in the header, and replace all the
    // uses in the region with the re-loaded value.
    AllocaInst *SpillLoc = new AllocaInst(VT, "spillVec", 
      &*(FuncEntry->getFirstInsertionPt()));
    new StoreInst(Used, SpillLoc, &*(PreHeader->getFirstInsertionPt()));
    LoadInst *Reload = new LoadInst(SpillLoc, "reloadVec", 
      &*(RegionHeader->getFirstInsertionPt()));
    for (Instruction *Use : I.second)
      Use->replaceUsesOfWith(Used, Reload);
  }
}

FeatureBitset FeatureOutliner::impliedFeature(const VectorType *VT) const {
  // We want 128-bit wide vectors to imply SSE, 256-bit wide 
  // vectors to imply AVX, and 512-bit wide vectors to imply AVX512.
  switch (VT->getBitWidth()) {
  case 128:
    return FeatureBitset({X86::FeatureSSE1});
  case 256:
    return FeatureBitset({X86::FeatureAVX});
  case 512:
    return FeatureBitset({X86::FeatureAVX512});
  default:
    return {};
  }
}

FeatureBitset FeatureOutliner::translateExternalFeatures(uint64_t Features) const {
  // Go over all set bits in the Features bitset, and map each set
  // bit to the corresponding internal bit.
  FeatureBitset Internal = {};
  while (Features) {
    uint64_t CurrFeature = Features & ~(Features - 1);
    Features &= Features - 1;
    Internal |= FeatureMapping[CurrFeature];
  }
  return Internal;
}

bool FeatureOutliner::appendFeatureString(
    std::string &Str, const FeatureBitset &Features) {
  // Go over all set bits in the Features bitset, and
  // append the right feature string to the provided string
  if (Features.none())
    return false;
    
  for (unsigned i = 0; i < Features.size(); ++i) {
    if (!Features[i])
      continue;

    StringRef FeatureString = FeatureStrings[i];
    if (Str == "")
      Str += "+";
    else
      Str += ",+";
    Str += FeatureString;
  }

  return (Str != "");
}

FeatureBitset FeatureOutliner::getAssumedFeatures(BasicBlock *BB, 
    const FeatureBitset& Available) {
  // TODO: Should probably check we don't have a has_feature that
  // gets used in a different way (e.g. assume(has_feature() & foo)),
  // and assert on it. We don't expect that to happen, but...
  FeatureBitset NewFeatures = {};

  // If this is the entry block of a function that has vector parameters,
  // or return a vector, then the whole function can use the instruction
  // set implied by the vector width.

  Function *F = BB->getParent();
  if (ConsistentVectorABI && (&F->getEntryBlock() == BB)) {
    for (auto const &Arg : F->args())
      if (VectorType *VT = dyn_cast<VectorType>(Arg.getType()))
        NewFeatures |= impliedFeature(VT);
    if (VectorType *VT = dyn_cast<VectorType>(F->getReturnType()))
        NewFeatures |= impliedFeature(VT);
  }

  // FIXME: This is a bit different from the ICC classic behavior.
  // In ICC classic, a call is a block terminator. So, if we have
  // <sse code>
  // foo() /* this may call exit() */
  // <avx code>
  // ICC classic will not propagate the avx marker above foo().
  // The code below will.
  // The way to handle it is to break basic blocks at calls... :-\

  for (auto I = BB->begin(), E = BB->end(); I != E; ++I) {
    // Intrinsics are calls, so if it's not a call, we don't care
    CallInst *Call = dyn_cast<CallInst>(I);
    if (!Call)
      continue;
    
    // If we have a call to a function that has vector parameters,
    // or returns a vector then the block can use the instruction
    // set implied by the vector width
    if (ConsistentVectorABI) {
      for (Value* Arg : Call->arg_operands())
        if (VectorType *VT = dyn_cast<VectorType>(Arg->getType()))
          NewFeatures |= impliedFeature(VT);

      if (VectorType *VT = dyn_cast<VectorType>(Call->getType()))
          NewFeatures |= impliedFeature(VT);
    }

    IntrinsicInst *Assume = dyn_cast<IntrinsicInst>(I);
    if (!Assume || Assume->getIntrinsicID() != Intrinsic::assume)
      continue;

    IntrinsicInst *Feature = dyn_cast<IntrinsicInst>(Assume->getOperand(0));
    if (!Feature || Feature->getIntrinsicID() != Intrinsic::has_feature)
      continue;

    ConstantInt *FeatureSet = 
      dyn_cast<ConstantInt>(Feature->getOperand(0));
    assert(FeatureSet && "Can only assume constant features");
    if (FeatureSet)
      NewFeatures |= translateExternalFeatures(FeatureSet->getZExtValue());
  }

  // If the features are already available, filter them out.
  return NewFeatures & ~Available;
}

bool FeatureOutliner::runOnModule(Module &M) {
  bool Changed = false;

  // TODO: What is the right CC to use?
  CallingConv::ID CC = CallingConv::X86_FastCall;
  SubtargetFeatures SF;

  // We are going to be adding functions. So prepare
  // the list of functions we want to work on in advance.
  std::vector<Function*> FunctionList;
  for (auto F = M.begin(), FE = M.end(); F != FE; ++F) {
    if (F->isDeclaration())
      continue;
    FunctionList.push_back(&*F);
  }

  for (auto FI = FunctionList.begin(), FE = FunctionList.end(); 
       FI != FE; ++FI) {
    Function *F = *FI;
    // Maps each basic block to the new features the region it
    // dominates needs to support. We need iteration over this
    // to be deterministic, so we use a MapVector instead of DenseMap
    MapVector<BasicBlock*, FeatureBitset> BBFeatureMap;
    // Maps each basic block to the features *available* in this
    // block. This is necessary because a new feature may imply a
    // bunch of other features, and we need to know that when looking
    // at the blocks below.
    DenseMap<BasicBlock*, FeatureBitset> AvailableFeatureMap;

    // Maps each basic block to the set of basic blocks it dominates
    // (i.e regions). Note that those regions are single-entry but
    // may have multiple exits.
    DenseMap<BasicBlock*, Region*> RegionMap;

    // Used to keep track of the allocated regions. Due to the way
    // we traverse the dom-tree, the top-most regions come first.
    SmallVector<Region*, 8> RegionList;

    const TargetSubtargetInfo *TSI = TM->getSubtargetImpl(*F);
    StringRef CPUString =  F->getFnAttribute("target-cpu").getValueAsString();

    // Walk the dominator tree top-down, and create regions
    DominatorTree DT;
    DT.recalculate(*F);
    DomTreeNode *Root = DT.getRootNode();
    for (df_iterator<DomTreeNode*> I = df_begin(Root), E = df_end(Root);
          I != E; ++I) {
      FeatureBitset Available = {};
      BasicBlock *BB = I->getBlock();
      // For the root, the available features are those available
      // for the whole function. For everything else, it's the features
      // known to be available in its immediate dominator;
      DomTreeNode *Parent = I->getIDom();

      if (Parent)
        Available = AvailableFeatureMap[Parent->getBlock()];
      else
        Available = TSI->getFeatureBits();

      FeatureBitset NewFeatures = getAssumedFeatures(BB, Available);
      BBFeatureMap[BB] = NewFeatures;

      // If this is the root block, or it has new features,
      // create a new region, otherwise add this block to
      // the parent's region.
      Region *BBRegion;
      if (NewFeatures.any() || !Parent) {
        BBRegion = new Region();
        RegionList.push_back(BBRegion);
      } else {
        BBRegion = RegionMap[Parent->getBlock()];
      }

      // Update the available features.
      for (unsigned i = 0; i < NewFeatures.size(); ++i) {
          if (!NewFeatures[i])
            continue;

        // If we don't have this feature available yet, turn it
        // and everything implied by it on. The check is needed because
        // NewFeatures may contain two features that imply each other.
        // In this case, just blindly calling ToggleFeature may turn
        // one of them off.
        if (!Available[i]) {
          Available = SF.ToggleFeature(Available, 
                        FeatureStrings[i],
                        TM->getMCSubtargetInfo()->getProcFeatures());
        }
      }
      AvailableFeatureMap[BB] = Available;
       
      BBRegion->push_back(BB);
      RegionMap[BB] = BBRegion;
    }

    // If we have any live vector values that cross region boundaries,
    // where the intervening code does not natively support them
    // we will run into trouble. They will end up as a vector arguments
    // passed between functions compiled for different ABIs.
    // Make sure this doesn't happen by spilling such values before
    // entering a region that will cause an ABI change (so the vector will
    // be passed by reference)
    for (auto R : RegionList) {
      FeatureBitset Features = BBFeatureMap[(*R)[0]];
      spillLiveVectors(RegionMap, *R, BBFeatureMap, &DT, Features);
    }

    // Walk the region list, in reverse order.
    // We need to outline from the bottom-up, otherwise we may 
    // get malformed regions.
    for (auto R = RegionList.rbegin(), RE = RegionList.rend(); R != RE; ++R) {
      BasicBlock *BB = (**R)[0];
      FeatureBitset& Features = BBFeatureMap[BB];
      if (!Features.any())
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

        // Adjust the calling convention for both the function and the
        // call site.
        NewF->setCallingConv(CC);
        assert(NewF->hasOneUse() && "New function should have one use");
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


