// VPlan Predicator
// ----------------
// Generate predicate recipes for each block (VPBB or Region) of the HCFG
// and attach them to:
//  i.  the blocks themselves (to allow propagation across the HCFG), and
//  ii. the recipe list of the VPBasicBlocks
//
// Predicate Types
// ---------------
// The predicate recipes contain all the information needed to generate the
// mask conditions during code generation.
// The edge predicates point to the condition and denote whether the mask
// should be an IfTrue or IfFalse.
// Operations onto the masks are performed by the Block Predicate
// (VPBlockPredicateRecipe) which operates on one or more input edge predicates
// appended to it.
//
// VPBlockPredicateRecipe (Block Predicate)
// ----------------------------------------
// All blocks (VPBBs or VPRegions) carry a Block Predicate which is the
// input/output mask for the whole SESE block.
// It can be retrieved with Block->getPredicateRecipe().
// VPBasicBlocks also contain the Block Predicate recipe within their recipe
// list, in order for the code generator to emit code for it.
//
// Connecting Predicates within a Region
// -------------------------------------
// Each VPBasicBlock gets its own Block Predicate which has one or more inputs.
// Each of the inputs is the predecessor block's block predicate.
//
// Predicate Propagation across the HCFG
// -------------------------------------
// A Region's PredicateRecipe pointer points to the incoming block's predicate
// recipe.
// Before a Region gets predicated by predicateRegion(), the Region's
// Block Predicate should have already been set.
// The top level region (VPlan's entry block) is assigned an AllOnes recipe.

#include "VPlanPredicator.h"
#include "VPlan.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/GraphTraits.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace vpo;

static cl::opt<bool>
    VPlanPredicatorReport("vplan-predicator-report", cl::init(false),
                          cl::Hidden,
                          cl::desc("VPlan Predicator report for testing"));
static cl::opt<bool> DumpVPlanDot("dump-vplan-dot", cl::init(false), cl::Hidden,
                                  cl::desc("Dump the vplan dot file"));
static cl::opt<bool>
    PredicateOutermostLoop("vplan-predicate-outermost", cl::init(true),
                           cl::Hidden,
                           cl::desc("Start predication at the outermost loop"));

// Returns the first recipe in BB or NULL if empty.
static VPRecipeBase *getFirstRecipeSafe(VPBasicBlock *BB) {
  VPRecipeBase *FirstRecipe = nullptr;
  if (!BB->getRecipes().empty()) {
    FirstRecipe = &BB->front();
  }
  return FirstRecipe;
}

// Emit the Predicate recipe to BLOCK if it is a BB
static void emitRecipeIfBB(VPPredicateRecipeBase *Predicate,
                           VPBlockBase *Block) {
  if (VPBasicBlock *BB = dyn_cast<VPBasicBlock>(Block)) {
    BB->addRecipe(Predicate, getFirstRecipeSafe(BB));
  }
}

// Get the PredBlock's successors, skipping the Back-Edges
void VPlanPredicator::getSuccessorsNoBE(VPBlockBase *PredBlock,
                                        SmallVector<VPBlockBase *, 2> &Succs) {
  for (VPBlockBase *SuccBlock : PredBlock->getSuccessors()) {
    if (!IntelVPlanUtils::isBackEdge(PredBlock, SuccBlock)) {
      Succs.push_back(SuccBlock);
    }
  }
}

// FIXME: This should change once Matt introduces the recipes
//        that point to other recipes so that we point to the one
//        closest to the condition.
VPVectorizeBooleanRecipe *
VPlanPredicator::getConditionRecipe(VPConditionBitRecipeBase *CBR) {
  // When we have LiveIn/Uniform CBR, we remove it
  // and replace it with a VPVectorizeBoolean
  if (VPConditionBitRecipeWithScalar *CBRWS =
          dyn_cast<VPConditionBitRecipeWithScalar>(CBR)) {
    VPVectorizeBooleanRecipe *VBR = nullptr;
    // If we have not generated the CBRWS recipe, generate it.
    auto it = CBRtoVBRMap.find(CBRWS);
    if (it == CBRtoVBRMap.end()) {
      Value *CI = CBRWS->getScalarCondition();
      VPConditionBitRecipeWithScalar *InsertBefore = CBRWS;
      assert(CI && "CBRWS not generated properly");
      VBR = PlanUtils.createVectorizeBooleanRecipe(CI);
      // Remember that we have already generated this VBR for CBR
      CBRtoVBRMap[CBRWS] = VBR;
      VPBasicBlock *BB = CBRWS->getParent();
      if (!BB) {
        // Live-Ins need special treatment as they are not in a BB.
        // We emit the VecBooleanRecipe at the outer loop preheader.
        assert(isa<VPLiveInConditionBitRecipe>(CBRWS));
        assert(std::distance(VPLI->begin(), VPLI->end()) == 1 &&
               "Multiple outer loops?");
        const VPLoop *Loop = *VPLI->begin();
        VPBlockBase *PreheaderBlock = Loop->getLoopPreheader();
        assert(isa<VPBasicBlock>(PreheaderBlock) && "Preheader must be a BB");
        BB = cast<VPBasicBlock>(PreheaderBlock);
        InsertBefore = nullptr;
      }
      assert(BB && "Need BB to insert the condition to");
      BB->addRecipe(VBR, InsertBefore);
    }
    // Reuse the one we have already generated.
    else {
      VBR = it->second;
    }
    return VBR;
  }
  // FIXME:
  else {
    assert(0 && "FIXME");
    return nullptr;
  }
}

// Return/Generate the incoming predicate.
// TODO: Do you mean single predecessor?
// If a single successor, then return predBB's block predicate
// Else generate the ifTrue/ifFalse predicates and return them
VPPredicateRecipeBase *
VPlanPredicator::genOrUseIncomingPredicate(VPBlockBase *CurrBlock,
                                           VPBlockBase *PredBlock) {
  VPPredicateRecipeBase *IncomingPredicate = nullptr;

  // Get the predecessor's successors skipping the Back-Edges
  SmallVector<VPBlockBase *, 2> PredSuccessorsNoBE;
  getSuccessorsNoBE(PredBlock, PredSuccessorsNoBE);

  // 1. Generate Edge Predicates if needed.
  //    If there is an unconditional branch to the currBB, then we don't
  //    create edge predicates. We use the predecessor's block predicate
  //    instead. VPRegionBlock's should always hit here.
  if (PredSuccessorsNoBE.size() == 1) {
    IncomingPredicate = PredBlock->getPredicateRecipe();
  }
  // If the predecessor block contains a condition
  else if (PredSuccessorsNoBE.size() == 2) {
    assert(isa<VPBasicBlock>(PredBlock) &&
           "Only BBs can have more than one successsor");
    VPBasicBlock *PredBB = cast<VPBasicBlock>(PredBlock);
    VPConditionBitRecipeBase *CBR = PredBB->getConditionBitRecipe();
    VPVectorizeBooleanRecipe *VBR = getConditionRecipe(CBR);
    assert(VBR);
    // CurrBB is the True successor of PredBB
    if (PredBlock->getSuccessors()[0] == CurrBlock) {
      // FIXME: We should be using the creation utils instead
      VPIfTruePredicateRecipe *IfTrueRecipe =
          PlanUtils.createIfTruePredicateRecipe(
              VBR, PredBB->getPredicateRecipe());
      emitRecipeIfBB(IfTrueRecipe, CurrBlock);
      IncomingPredicate = IfTrueRecipe;
    }
    // TODO: Else? A block cannot have the same successor twice (except when we
    // support switch statementes)
    // CurrBB is the False successor of PredBB
    if (PredBlock->getSuccessors()[1] == CurrBlock) {
      VPIfFalsePredicateRecipe *IfFalseRecipe =
          PlanUtils.createIfFalsePredicateRecipe(
              VBR, PredBB->getPredicateRecipe());
      emitRecipeIfBB(IfFalseRecipe, CurrBlock);
      IncomingPredicate = IfFalseRecipe;
    }
  } else {
    assert(0 && "Unreachable: Inconsistent predecessors / successors");
  }
  return IncomingPredicate;
}

// Generate and attach an empty Block Predicate onto CurrBlock
void VPlanPredicator::genAndAttachEmptyBlockPredicate(VPBlockBase *CurrBlock) {
  // Only Basic Blocks have block predicates attached to them, as they are the
  // ones capable of generating code.
  if (isa<VPBasicBlock>(CurrBlock)) {
    VPBlockPredicateRecipe *blockPredicate =
        PlanUtils.createBlockPredicateRecipe();
    emitRecipeIfBB(blockPredicate, CurrBlock);
    CurrBlock->setPredicateRecipe(blockPredicate);
  }
  // A Region will simply point to its incoming predicate recipe.
  // This gets taken care of later.
  else {
    CurrBlock->setPredicateRecipe(nullptr);
  }
}

// Helper for appending a Recipe to Block.
// It hides the fact that we only BBs get to have a BlockPredicate.
// Regions simply point to the input's recipe.
static void appendPredicateToBlock(VPBlockBase *Block,
                                   VPPredicateRecipeBase *Recipe) {
  if (isa<VPBasicBlock>(Block)) {
    VPBlockPredicateRecipe *BP =
        dyn_cast<VPBlockPredicateRecipe>(Block->getPredicateRecipe());
    assert(BP);
    BP->appendIncomingPredicate(Recipe);
  } else {
    assert(Block->getPredicateRecipe() == nullptr && "Overwriting ?");
    Block->setPredicateRecipe(Recipe);
  }
}

// Generate all predicates needed for CurrBB
void VPlanPredicator::propagatePredicatesAcrossBlocks(VPBlockBase *CurrBlock,
                                                      VPRegionBlock *Region) {
  // Skip entry blocks
  if (CurrBlock == Region->getEntry()) {
    return;
  }

  // For each input block, get the predicate and append it to BP
  for (VPBlockBase *PredBlock : CurrBlock->getPredecessors()) {
    // Skip back-edges
    if (IntelVPlanUtils::isBackEdge(PredBlock, CurrBlock)) {
      continue;
    }

    VPPredicateRecipeBase *IncomingPredicate =
        genOrUseIncomingPredicate(CurrBlock, PredBlock);

    // // Create a block predicate and append the inputs to it
    // VPBlockPredicateRecipe *BlockPredicate
    //   = dyn_cast<VPBlockPredicateRecipe>(CurrBlock->getPredicateRecipe());
    // assert(blockPredicate);
    // BlockPredicate->appendIncomingPredicate(IncomingPredicate);
    appendPredicateToBlock(CurrBlock, IncomingPredicate);
  }
}

// Dump predicates for LIT testing.
void VPlanPredicator::genLitReport(VPRegionBlock *Region) {
  if (!VPlanPredicatorReport) {
    return;
  }
  raw_ostream &OS = outs();
  OS << Region->getName() << ":\n";
  VPBlockBase *EntryBlock = Region->getEntry();
  ReversePostOrderTraversal<VPBlockBase *> RPOT(EntryBlock);
  for (auto it = RPOT.begin(); it != RPOT.end(); ++it) {
    VPBlockBase *Block = *it;
    // If it is a BB, print all predicate recipes within it.
    if (const VPBasicBlock *BasicBlock = dyn_cast<VPBasicBlock>(Block)) {
      OS << "  " << BasicBlock->getName() << ":\n";
      for (const VPRecipeBase &Recipe : BasicBlock->getRecipes()) {
        if (const VPPredicateRecipeBase *Predicate =
                dyn_cast<VPPredicateRecipeBase>(&Recipe)) {
          OS << "    ";
          Predicate->print(OS);
          OS << "\n";
        }
      }
    }
    // If it is a region, then print the predicate recipe attached to it.
    else if (const VPRegionBlock *Region = dyn_cast<VPRegionBlock>(Block)) {
      OS << "  " << Region->getName() << ":\n";
      VPPredicateRecipeBase *Predicate = Region->getPredicateRecipe();
      OS << "    ";
      Predicate->print(OS);
      OS << "\n";
    } else {
      llvm_unreachable("Unsupported block type");
    }
  }
  outs() << "\n";
}

// Generate all predicates within Region. We stay at the same level in
// the HCFG and do not attempt to predicate the internals of other regions.
void VPlanPredicator::predicateRegion(VPRegionBlock *Region) {
  VPPredicateRecipeBase *RegionRecipe = Region->getPredicateRecipe();
  assert(RegionRecipe && "Must have been assigned an input predicate");
  assert(isa<VPBasicBlock>(Region->getEntry()) &&
         "Region entry is not a VPBasicBlock");
  VPBlockBase *EntryBlock = cast<VPBasicBlock>(Region->getEntry());

  // 1. Generate the block predicates for all Basic Blocks in any order
  //    VPRegions' predicate will be set to nullptr.
  for (auto BlockIt = df_iterator<VPBlockBase *>::begin(EntryBlock),
            BlockItE = df_iterator<VPBlockBase *>::end(EntryBlock);
       BlockIt != BlockItE; ++BlockIt) {
    genAndAttachEmptyBlockPredicate(*BlockIt);
  }

  // 2. Propagate the Region's Block Predicate inputs to the entry block
  // VPBlockPredicateRecipe *RegionBP
  //   = dyn_cast<VPBlockPredicateRecipe>(RegionRecipe);
  // assert(RegionBP && "Each region should have a BP attached to it.");
  assert(EntryBlock->getPredicateRecipe() &&
         isa<VPBlockPredicateRecipe>(EntryBlock->getPredicateRecipe()) &&
         "Should have been emitted by Step 1.");
  VPBlockPredicateRecipe *EntryBP =
      cast<VPBlockPredicateRecipe>(EntryBlock->getPredicateRecipe());
  // for (VPPredicateRecipeBase *Incoming : RegionBP->getIncomingPredicates()) {
  EntryBP->appendIncomingPredicate(RegionRecipe);
  // }

  // 3. Generate edge predicates and append them to the block predicate
  //    RPO is necessary since nested VPRegions' predicate is null and it has to
  //    be set before it's propagated
  //for (auto it = df_iterator<VPBlockBase *>::begin(EntryBlock),
  //          ite = df_iterator<VPBlockBase *>::end(EntryBlock);
  //     it != ite; ++it) {
  // TODO: If we have to use RPOT here, we should reuse it in step1
  ReversePostOrderTraversal<VPBlockBase *> RPOT(EntryBlock);
  for (VPBlockBase *VPB : make_range(RPOT.begin(), RPOT.end())) {

    propagatePredicatesAcrossBlocks(VPB, Region);
  }
}

// Helper function for traversing the Hierarchical Control Flow
// Look for region blocks within entryBlock and append them to regionWL
static void appendRegionsToWorklist(VPBlockBase *EntryBlock,
                                    std::vector<VPRegionBlock *> &RegionWL) {
  for (auto it = df_iterator<VPBlockBase *>::begin(EntryBlock),
            ite = df_iterator<VPBlockBase *>::end(EntryBlock);
       it != ite; ++it) {
    VPBlockBase *block = *it;
    if (VPRegionBlock *Region = dyn_cast<VPRegionBlock>(block)) {
      RegionWL.push_back(Region);
    }
  }
}

// FIXME: Only for debugging. Can be removed.
static void dumpVplanDot(IntelVPlan *Plan,
                         const char *dotFile = "/tmp/vplan.dot") {
  if (DumpVPlanDot) {
    std::error_code EC;
    raw_fd_ostream file(dotFile, EC, sys::fs::F_RW);
    VPlanPrinter printer(file, *Plan);
    printer.dump();
    file.close();
  }
}

#if 0
// TODO - if we need to skip backedges
typedef SmallPtrSet<VPBlockBase *, 8> VPBlockSet;

namespace llvm {
// Specialize po_iterator_storage to skip backedges
template<> class po_iterator_storage<VPBlockSet, true> {
  VPBlockSet &Visited;
public:
  po_iterator_storage(VPBlockSet &VBS) : Visited(VBS) {}
  po_iterator_storage(const po_iterator_storage &S) : Visited(S.Visited) {}

  // These functions are defined below.
  bool insertEdge(VPBlockBase * From, VPBlockBase *To) {
    if (IntelVPlanUtils::isBackEdge(From, To)) {
      errs() << "Saw backedge\n";
      return false;
    }
    
    return Visited.insert(To).second;
  }

  void finishPostorder(VPBlockBase *VB) {};
};
}
#endif

// Linearize the CFG
void VPlanPredicator::linearize(VPBlockBase *EntryBlock) {
  typedef std::vector<VPBlockBase *>::reverse_iterator rpo_iterator;
  ReversePostOrderTraversal<VPBlockBase *> RPOT(EntryBlock);

  VPBlockBase *PrevBlock = nullptr;
  for (rpo_iterator I = RPOT.begin(); I != RPOT.end(); ++I) {
    VPBlockBase *CurrBlock = (*I);

    // dbgs() << "LV: VPBlock in RPO " << CurrBlock->getName() << '\n';

    CurrBlock->clearEdges();
    if (PrevBlock) {
      VPlanUtils::addEdge(PrevBlock, CurrBlock);
    }
    if (VPRegionBlock *Region = dyn_cast<VPRegionBlock>(CurrBlock)) {
      linearize(Region->getEntry());
    }
    PrevBlock = CurrBlock;
  }
}

// Entry point. The driver function for the predicator.
void VPlanPredicator::predicate(void) {
  dumpVplanDot(
      Plan,
      "/tmp/vplan.before.dot"); // FIXME: Only for debugging. Can be removed.

  VPBlockBase *EntryBlock;
  if (PredicateOutermostLoop) {
    assert(std::distance(VPLI->begin(), VPLI->end()) == 1 &&
           "more than 1 loop?");
    EntryBlock = (*VPLI->begin())->getLoopPreheader()->getParent();
  } else {
    EntryBlock = Plan->getEntry();
  }
  std::vector<VPRegionBlock *> RegionsWorklist;
  appendRegionsToWorklist(EntryBlock, RegionsWorklist);
  assert(RegionsWorklist.size() == 1 && isa<VPRegionBlock>(EntryBlock) &&
         "We expect a single entry block for Plan.");

  // The plan's entry block should have an "AllOnes" predicate.
  VPPredicateRecipeBase *AllOnes =
      PlanUtils.createAllOnesPredicateRecipe();
  genAndAttachEmptyBlockPredicate(EntryBlock);
  appendPredicateToBlock(EntryBlock, AllOnes);

  // Iterate until there are no Regions left.
  while (!RegionsWorklist.empty()) {
    VPRegionBlock *Region = RegionsWorklist.back();
    RegionsWorklist.pop_back();
    // Predicate the blocks within Region at the same level as the
    // Region entry. It will not dive deeper into the HCFG.
    predicateRegion(Region);
    // For LIT testing
    genLitReport(Region);
    // Add Regions within REGION into the worklist.
    // NOTE: This could be done from within predicateRegion() to save
    //       a second pass through the Region's blocks.
    appendRegionsToWorklist(Region->getEntry(), RegionsWorklist);
  }

  dumpVplanDot(Plan, "/tmp/vplan.after.dot"); // For debugging

  linearize(Plan->getEntry());

  dumpVplanDot(Plan, "/tmp/vplan.after.linearized.dot"); // For debugging
}
