//===----------------------------------------------------------------------===//
//
//   Copyright (C) 2017-2019 Intel Corporation. All rights reserved.
//
//   The information and source code contained herein is the exclusive
//   property of Intel Corporation. and may not be disclosed, examined
//   or reproduced in whole or in part without explicit written authorization
//   from the company.
//
//===----------------------------------------------------------------------===//
//
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
// 1. The edge predicates (VPEdgePredicateRecipe, VPIfTruePredicateRecipe,
//    VPIfFalsePredicateRecipe) point to the condition and denote whether
//    the mask should be an IfTrue or IfFalse.
// 2. Operations onto the masks are performed by the VPBlockPredicateRecipe
//    which operates on one or more input edge predicates appended to it.
//
// VPBlockPredicateRecipeBase (Block Predicate)
// --------------------------------------------
// Both VPBBs or VPRegions carry a VPBlockPredicateRecipeBase derived predicate
// which is the input/output mask for the whole SESE block.
// It can be retrieved with Block->getPredicateRecipe().
//
// o VPBasicBlocks point to a VPBlockPredicate and *not* to a
//   VPEdgePredicateRecipeBase derived class.
//   VPBasicBlocks also contain the Block Predicate recipe within their recipe
//   list, in order for the code generator to emit code for it.
//
// o VPRegions, on the other hand, point to either a VPBlockPredicate
//   or a VPEdgePredicateRecipeBase derived class.
//
// Connecting Predicates within a Region
// -------------------------------------
// Each VPBasicBlock gets its own Block Predicate which has one or more inputs.
// Each of the inputs is the predecessor block's block predicate.
//
// Predicate Placement and Propagation across the HCFG
// ---------------------------------------------------
// - Regions cannot cannot generate code themselves (only BBs do). So their
//   PredicateRecipe pointer points to the incoming block's predicate
//   recipe.
// - Before a Region gets predicated by predicateRegionRec(), the Region's
//   Block Predicate should have already been set.
// - The top level region (VPlan's entry block) is assigned an all-ones
//   predicate (nullptr).
// - Edge Predicates are emitted in the same BB as the condition they point to.

#include "IntelVPlanPredicator.h"
#include "IntelVPlan.h"
#include "IntelVPlanDominatorTree.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/GraphTraits.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "VPlanPredicator"

using namespace llvm;
using namespace llvm::vpo;

static cl::opt<bool>
    VPlanPredicatorReport("vplan-predicator-report", cl::init(false),
                          cl::Hidden,
                          cl::desc("VPlan Predicator report for testing"));
static cl::opt<bool> DumpVPlanDot("dump-vplan-dot", cl::init(false), cl::Hidden,
                                  cl::desc("Dump the vplan dot file"));
static cl::opt<bool> DisablePredicatorOpts(
    "disable-predicator-opts", cl::init(false), cl::Hidden,
    cl::desc("Disable optimizations in VPlan Predicator."));

cl::opt<bool> VPlanLoopCFU(
    "vplan-loop-cfu", cl::init(true), cl::Hidden,
    cl::desc("Perform inner loop control flow uniformity transformation"));

// Returns the first recipe in BB or NULL if empty.
static VPRecipeBase *getFirstRecipeSafe(VPBasicBlock *BB) {
  VPRecipeBase *FirstRecipe = nullptr;
  if (!BB->getRecipes().empty()) {
    FirstRecipe = &BB->front();
  }
  return FirstRecipe;
}

// Count PredBlock's successors, skipping back-edges
int VPlanPredicator::countSuccessorsNoBE(VPBlockBase *PredBlock, bool& HasBE) {
  HasBE = false;
  int cnt = 0;
  for (VPBlockBase *SuccBlock : PredBlock->getSuccessors()) {
    if (!VPBlockUtils::isBackEdge(PredBlock, SuccBlock, VPLI))
      cnt++;
    else
      HasBE = true;
  }
  return cnt;
}

// TODO: This function is not used.
// Get the PredBlock's successors, skipping the Back-Edges
void VPlanPredicator::getSuccessorsNoBE(VPBlockBase *PredBlock,
                                        SmallVector<VPBlockBase *, 2> &Succs) {
  for (VPBlockBase *SuccBlock : PredBlock->getSuccessors()) {
    if (!VPBlockUtils::isBackEdge(PredBlock, SuccBlock, VPLI)) {
      Succs.push_back(SuccBlock);
    }
  }
}

VPPredicateRecipeBase *VPlanPredicator::genEdgeRecipe(VPBasicBlock *PredBB,
                                                      VPPredicateRecipeBase *R,
                                                      BasicBlock *From,
                                                      BasicBlock *To) {
  VPEdgePredicateRecipe *EdgeRecipe = new VPEdgePredicateRecipe(R, From, To);
  EdgeRecipe->setName(VPlanUtils::createUniqueName("AuxEdgeForMaskSetting"));
  PredBB->addRecipe(EdgeRecipe);
  return EdgeRecipe;
}

// Generate an Edge Predicate Recipe if required inside PredBB and return it.
// Returns NULL if no recipe was created.
VPPredicateRecipeBase *VPlanPredicator::genEdgeRecipe(VPBasicBlock *PredBB,
                                                      EdgeType ET) {
  VPValue *CBV = PredBB->getCondBit();
  // TODO: Add this assertion back when HIR fake condition bits are removed
  // assert(CBV->getValue() && "Even Live-Ins should have a value");
  assert(CBV && "Broken getConditionRecipe() ?");

  // CurrBB is the True successor of PredBB
  if (ET == TRUE_EDGE) {
    VPIfTruePredicateRecipe *IfTrueRecipe =
        new VPIfTruePredicateRecipe(CBV, PredBB->getPredicateRecipe(),
                                    PredBB->getCBlock(), PredBB->getTBlock());
    IfTrueRecipe->setName(VPlanUtils::createUniqueName("IfT"));
    // Emit IfTrueRecipe into PredBB
    PredBB->addRecipe(IfTrueRecipe);
    return IfTrueRecipe;
  }
  // CurrBB is the False successor of PredBB
  else if (ET == FALSE_EDGE) {
    VPIfFalsePredicateRecipe *IfFalseRecipe =
        new VPIfFalsePredicateRecipe(CBV, PredBB->getPredicateRecipe(),
                                     PredBB->getCBlock(), PredBB->getFBlock());
    IfFalseRecipe->setName(VPlanUtils::createUniqueName("IfF"));
    PredBB->addRecipe(IfFalseRecipe);
    return IfFalseRecipe;
  }
  llvm_unreachable("Support for switch statements ?");
  return nullptr;
}

// Helper for appending a Recipe to Block.
// It hides the fact that we only BBs get to have a BlockPredicate.
// Regions simply point to the input's recipe.
static void appendPredicateToBlock(VPBlockBase *Block,
                                   VPPredicateRecipeBase *Recipe) {

  LLVM_DEBUG(errs() << "Appending " << Recipe->getName() << " to "
                    << Block->getName() << "\n");

  if (isa<VPBasicBlock>(Block)) {
    assert(isa<VPBlockPredicateRecipe>(Block->getPredicateRecipe()) &&
           "Expected VPBlockPredicateRecipe");
    VPBlockPredicateRecipe *BP =
        cast<VPBlockPredicateRecipe>(Block->getPredicateRecipe());
    VPValueUtils::appendIncomingToBlockPred(BP, Recipe);
  } else {
    // Regions accept any Block Predicate (not just VPBlockPredicateRecipe)
    assert(isa<VPRegionBlock>(Block) && "Expected VPRegionBlock.");
    assert(!Block->getPredicateRecipe() && "Overwriting ?");
    // TODO: PlanUtils
    Block->setPredicateRecipe(Recipe);
  }
}

// Return whether the edge FromBlock -> ToBlock is TRUE_EDGE are FALSE_EDGE
VPlanPredicator::EdgeType
VPlanPredicator::getEdgeTypeBetween(VPBlockBase *FromBlock,
                                    VPBlockBase *ToBlock) {
  unsigned Cnt = 0;
  for (VPBlockBase *SuccBlock : FromBlock->getSuccessors()) {
    if (SuccBlock == ToBlock)
      return (Cnt == 0) ? TRUE_EDGE : FALSE_EDGE;
    Cnt++;
  }
  llvm_unreachable("Broken FromSuccessorsNoBE[] ?");
  return EDGE_TYPE_UNINIT;
}

// Get the Incoming predicate from PredBlock
static VPBlockBase *getTakePredicateFrom(VPBlockBase *PredBlock,
                                         VPBasicBlock *PredBasicBlock,
                                         bool HasBackEdge) {
  if (HasBackEdge) {
    // If the PredBlock belongs to an inner loop, the predicate of the
    // edge between the PredBlock and CurrentBlock is a predicate of the
    // Entry block of the loop.
    assert(PredBasicBlock && "Only BBs have multiple exits");
    return cast<VPLoopRegion>(PredBasicBlock->getParent())->getEntry();
  } else
    return PredBlock;
}

// Generate all predicates needed for CurrBB
void VPlanPredicator::propagatePredicatesAcrossBlocks(VPBlockBase *CurrBlock,
                                                      VPRegionBlock *Region) {
  // Skip entry blocks
  if (CurrBlock == Region->getEntry()) {
    return;
  }

  // For each predecessor, get the predicate and append it to BP
  for (VPBlockBase *PredBlock : CurrBlock->getPredecessors()) {
    // Skip back-edges
    if (VPBlockUtils::isBackEdge(PredBlock, CurrBlock, VPLI)) {
      continue;
    }

    VPBasicBlock *PredBasicBlock = dyn_cast<VPBasicBlock>(PredBlock);
    VPBasicBlock *CurrBasicBlock = dyn_cast<VPBasicBlock>(CurrBlock);

    VPPredicateRecipeBase *IncomingPredicate = nullptr;
    bool HasBackEdge = false;
    int NumPredSuccsNoBE = countSuccessorsNoBE(PredBlock, HasBackEdge);

    // If there is an unconditional branch to the currBB, then we don't create
    // edge predicates. We use the predecessor's block predicate instead.
    // VPRegionBlocks should always hit here.
    if (NumPredSuccsNoBE == 1) {

      VPBlockBase *TakePredicateFrom =
          getTakePredicateFrom(PredBlock, PredBasicBlock, HasBackEdge);
      IncomingPredicate = TakePredicateFrom->getPredicateRecipe();

      if (PredBasicBlock && CurrBasicBlock && PredBasicBlock->getCBlock()) {
        // Even if we have an unconditional branch, the branch should
        // be in CBlock and TBlock
        BasicBlock *FromBB = PredBasicBlock->getCBlock();
        EdgeType ET = getEdgeTypeBetween(PredBlock, CurrBlock);
        
        BasicBlock *ToBB = (ET == TRUE_EDGE) ? PredBasicBlock->getTBlock() :
          PredBasicBlock->getFBlock();
        // TODO: Passing TakePRedicateFrom and IncommingPredicate seem to be
        // redundant.
        genEdgeRecipe(cast<VPBasicBlock>(TakePredicateFrom), IncomingPredicate,
                      FromBB, ToBB);
      }
    }
    else if (NumPredSuccsNoBE == 2) {
      // Emit Edge recipes into PredBlock if required
      assert(PredBasicBlock && "Only BBs have multiple exits");
      EdgeType ET = getEdgeTypeBetween(PredBlock, CurrBlock);

      // TODO: We have to generate edge predicates even when Region is uniform
      // because CG uses them to vectorize Phi nodes. When we change the way Phi
      // nodes are vectorized, we should also revisit this part.
      VPPredicateRecipeBase *EdgeRecipe =
          genEdgeRecipe(cast<VPBasicBlock>(PredBlock), ET);

      // If Region is uniform, we just propagate predecessors's block predicate.
      // Edge predicates are not used since the edge condition is uniform. We
      // aren't predicating any instruciton.
      if (Region->isDivergent()) 
        IncomingPredicate = EdgeRecipe;
      else
        IncomingPredicate =
            getTakePredicateFrom(PredBlock, PredBasicBlock, HasBackEdge)
                ->getPredicateRecipe();
    }
    else {
      llvm_unreachable("FIXME: switch statement ?");
    }
    assert(IncomingPredicate && "Wrong traversal ?");
    appendPredicateToBlock(CurrBlock, IncomingPredicate);
  }
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// Dump predicates for LIT testing.
void VPlanPredicator::genLitReport(VPRegionBlock *Region) {
  if (!VPlanPredicatorReport) {
    return;
  }
  raw_ostream &OS = outs();
  OS << "Predicator report\n";
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
          Predicate->print(OS, Twine()); // TODO: Twine
          OS << "\n";
        }
      }
    }
    // If it is a region, then print the predicate recipe attached to it.
    else if (const VPRegionBlock *Region = dyn_cast<VPRegionBlock>(Block)) {
      OS << "  " << Region->getName() << ":\n";
      VPPredicateRecipeBase *Predicate = Region->getPredicateRecipe();
      if (Predicate) {
        OS << "    ";
        Predicate->print(OS, Twine()); // TODO: Twine
        OS << "\n";
      }
    } else {
      llvm_unreachable("Unsupported block type");
    }
  }
  outs() << "End of the Predicator report.\n";
}
#endif // !NDEBUG || LLVM_ENABLE_DUMP

// Generate all predicates within Region and recursively predicate nested
// regions.
void VPlanPredicator::predicateRegionRec(VPRegionBlock *Region) {

  LLVM_DEBUG(dbgs() << "Predicating Region: " << Region->getName() << "\n");

  assert(isa<VPBasicBlock>(Region->getEntry()) &&
         "Region entry is not a VPBasicBlock");
  VPBasicBlock *EntryBlock = cast<VPBasicBlock>(Region->getEntry());

  // RPOT is reused in steps 1 and 3.
  ReversePostOrderTraversal<VPBlockBase *> RPOT(EntryBlock);

  // 1. Generate the block predicates for all VPBasicBlocks.
  for (VPBlockBase *Block : make_range(RPOT.begin(), RPOT.end())) {

    if (auto VPBB = dyn_cast<VPBasicBlock>(Block)) {
      VPBlockPredicateRecipe *BP = new VPBlockPredicateRecipe();
      BP->setName(VPlanUtils::createUniqueName("BP"));
      VPBB->addRecipe(BP, getFirstRecipeSafe(VPBB));
      VPBB->setPredicateRecipe(BP);
    } else if (auto SubRegion = dyn_cast<VPRegionBlock>(Block)) {
      LLVM_DEBUG(dbgs() << "Found subregion: " << SubRegion->getName() << "\n");

      assert(!SubRegion->getPredicateRecipe() && "Region predicate must be "
                                                 "nullptr. Input predicate "
                                                 "hasn't been propagated yet.");
      (void) SubRegion;
    } else {
      llvm_unreachable(
          "VPBlockBase is neither a VPBasicBlock nor a VPRegionBlock.");
    }
  }

  // 2. Propagate the Region's Block Predicate inputs to the entry block.
  assert(EntryBlock->getPredicateRecipe() &&
         isa<VPBlockPredicateRecipe>(EntryBlock->getPredicateRecipe()) &&
         "Should have been emitted by Step 1.");
  VPBlockPredicateRecipe *EntryBP =
      cast<VPBlockPredicateRecipe>(EntryBlock->getPredicateRecipe());
  VPPredicateRecipeBase *RegionInputPred = Region->getPredicateRecipe();
  VPValueUtils::appendIncomingToBlockPred(EntryBP, RegionInputPred);

  // 3. Generate edge predicates and append them to the block predicate RPO is
  //    necessary since nested VPRegions' predicate is null and it has to be set
  //    before it's propagated.
  for (VPBlockBase *VPB : make_range(RPOT.begin(), RPOT.end())) {
    propagatePredicatesAcrossBlocks(VPB, Region);
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  // We generate only one LIT report per region. If predicator optimizations are
  // disabled, we generate the report here. Otherwise, the report is generated
  // in 'optimizeRegionRec'.
  if (DisablePredicatorOpts)
    genLitReport(Region);
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  // Predicate subregions
  for (VPBlockBase *Block : make_range(RPOT.begin(), RPOT.end())) {
    if (auto SubRegion = dyn_cast<VPRegionBlock>(Block)) {
      predicateRegionRec(SubRegion);
    }
  }
}

// This optimization propagates the predicate in an IDom block to its IPostDom
// block, stating from Region's Entry. This propagation will replace operations
// that join multiple predicates in IPostDom block (e.g., BP5 = BP3 || BP4) by
// just the predicate in IDom block (e.g., BP5 = BP2)
static void optimizeImmediatePostdomBlocks(VPRegionBlock *Region) {
  Region->computeDT();
  Region->computePDT();

  // Go over the nodes of the Domination Tree.
  // For each node get the (children) nodes that it dominates.
  // If the child node post-dominates the original node, then we propagate
  // the predicate from the node to the childe node.
  
  // We iterate through the DomTree and check against all children.
  VPDominatorTree *RegionDT = Region->getDT();
  VPPostDominatorTree *RegionPDT = Region->getPDT();
  // Walk down the nodes of the dominator graph
  for (auto DTNode :
       make_range(df_iterator<VPDomTreeNode *>::begin(RegionDT->getRootNode()),
                  df_iterator<VPDomTreeNode *>::end(RegionDT->getRootNode()))) {
      VPBlockBase *BB = DTNode->getBlock();
      VPPredicateRecipeBase *BBPred = BB->getPredicateRecipe();
      // For all children nodes that are dominated by DTNode
      for (auto ChildDTNode : DTNode->getChildren()) {
          VPBlockBase *ChildBB = ChildDTNode->getBlock();
          auto ChildPDTNode = RegionPDT->getNode(ChildBB);
          auto PDTNode = RegionPDT->getNode(BB);
          // If the child node imm post-dominates the node
          if (PDTNode->getIDom() == ChildPDTNode) {
              VPPredicateRecipeBase *ChildBBPredBase =
                  ChildBB->getPredicateRecipe();
              // Skip if both parent and child is all-ones
              if (!ChildBBPredBase && !BBPred)
                continue;
              // Check if only child is all-ones
              assert((!BBPred || ChildBBPredBase) &&
                     "Propagating non all-ones to all-ones!");

              // Regions need special treatment. The reason is that their BP
              // is the actual BP of a predecessor block. Therefore editing
              // the region's BP (e.g., by appendIncoming()) will cause the
              // remote BP to change, which is not valid.
              if (isa<VPRegionBlock>(ChildBB)) {
                ChildBB->setPredicateRecipe(BBPred);
              } else {
                assert(isa<VPBlockPredicateRecipe>(ChildBBPredBase) &&
                       "Expected VPBlockPredicateRecipe");
                VPBlockPredicateRecipe *ChildBBPred =
                    cast<VPBlockPredicateRecipe>(ChildBBPredBase);
                VPValueUtils::clearIncomingsFromBlockPred(ChildBBPred);
                VPValueUtils::appendIncomingToBlockPred(ChildBBPred, BBPred);
              }
          }
      }
  }
}
// It propagates all-ones predicates within Region (and its sub-regions) and
// optimizes operations on predicates that involve an all-ones predicate.
// TODO: Use Def-Use chains when available.
static void optimizeAllOnesPredicates(
    VPRegionBlock *Region,
    SmallPtrSetImpl<VPPredicateRecipeBase *> &AllOnesPreds) {

  for (auto Block : make_range(df_iterator<VPRegionBlock *>::begin(Region),
                               df_iterator<VPRegionBlock *>::end(Region))) {

    // Region's predicate is used just to propagate predecessor's predicate
    // inside the Region and to its successor. It doesn't need to be
    // optimized.
    if (auto VPBB = dyn_cast<VPBasicBlock>(Block)) {
      // A. Optimize BlockPredicate.
      assert(isa<VPBlockPredicateRecipe>(Block->getPredicateRecipe()) &&
             "Expected BlockPredicate.");
      VPBlockPredicateRecipe *BlockPred =
          cast<VPBlockPredicateRecipe>(Block->getPredicateRecipe());

      // Remove all AllOnesPreds predicates from Incomings
      SmallVectorImpl<VPPredicateRecipeBase *> &BPIncomings =
          BlockPred->getIncomingPredicates();
      BPIncomings.erase(std::remove_if(BPIncomings.begin(), BPIncomings.end(),
                                       [&](VPPredicateRecipeBase *Incoming) {
                                         return AllOnesPreds.count(Incoming);
                                       }),
                        BPIncomings.end());

      if (BPIncomings.size() == 0) {
        LLVM_DEBUG(dbgs() << BlockPred->getName() << " became all-ones.\n");

        // BlockPred is empty so it's all-ones. Set Block's predicate to null.
        Block->setPredicateRecipe(nullptr);

        // Insert BlockPred in AllOnesPreds. Please, note that we cannot erase
        // BlockPred from VPBB's list of recipes at this point because
        // sub-regions may use this predicate.
        AllOnesPreds.insert(BlockPred);
      }

      // B. Optimize EdgePredicates in this VPBB. Remove operations with
      // all-ones predicates.
      for (auto &Recipe : make_range(VPBB->begin(), VPBB->end())) {
        VPRecipeBase *RecipePtr = &Recipe;
        if (auto EdgePred = dyn_cast<VPEdgePredicateRecipeBase>(RecipePtr)) {
          if (AllOnesPreds.count(EdgePred->getPredecessorPredicate())) {
            LLVM_DEBUG(dbgs() << "Setting " << EdgePred->getName()
                              << "'s PredecessorPredicate ("
                              << EdgePred->getPredecessorPredicate()->getName()
                              << ") to nullptr\n");
            EdgePred->setPredecessorPredicate(nullptr);
          }
        }
      }
    }
  }
}

// Set of optimizations that need to happen in pre-order of the recursion,
// before we optimize Region's sub-regions.
static void
optimizeRegionPreOrder(VPRegionBlock *Region,
                       SmallPtrSetImpl<VPPredicateRecipeBase *> &AllOnesPreds) {

  // Pre-order 1. Propagate predicates to IPostDoms
  optimizeImmediatePostdomBlocks(Region);

  // Pre-order 2. Propagate/optimize away all-ones predicates.
  optimizeAllOnesPredicates(Region, AllOnesPreds);
}

// Set of optimizations that need to happen in post-order of the recursion,
// after all the Region's sub-regions have been optimized.
static void optimizeRegionPostOrder(
    VPPredicateRecipeBase *IncomingAllOnesPred,
    SmallPtrSetImpl<VPPredicateRecipeBase *> &AllOnesPreds) {

  // Post-order 1. Destroy Region-local VPBlockPredicates from VPBB's list of
  // recipes. These predicates are no longer used after previous recursive visit
  // of sub-regions. Do not destroy IncomingAllOnesPred from AllOnesPreds. This
  // predicate is destroyed in the parent region.
  if (IncomingAllOnesPred)
    AllOnesPreds.erase(IncomingAllOnesPred);

  for (auto PredRecipe : AllOnesPreds) {
    VPBasicBlock *ParentVPBB = PredRecipe->getParent();
    LLVM_DEBUG(dbgs() << "Destroying recipe " << PredRecipe->getName()
                      << " from " << ParentVPBB->getName() << "\n");

    assert(ParentVPBB && "Expected parent for this recipe.");
    // Erase destroys the actual recipe opbject.
    ParentVPBB->eraseRecipe(PredRecipe);
  }
}

// Optimize predicates within Region. IncomingAllOnesPred holds an incoming
// predicate that became all-ones and we have to propagate inside the
// region. If incoming predicate didn't become all-ones, IncomingAllOnesPred is
// null.
// TODO: Simplify/revisit IncomingAllOnesPred after removing VPAllOnesPredicate.
void VPlanPredicator::optimizeRegionRec(
    VPRegionBlock *Region, VPPredicateRecipeBase *IncomingAllOnesPred) {

  LLVM_DEBUG(dbgs() << "Optimizing " << Region->getName()
                    << ". IncomingAllOnesPred = "
                    << (IncomingAllOnesPred ? IncomingAllOnesPred->getName()
                                            : "nullptr")
                    << "\n");

  // It contains all the predicates that are/become all-ones at this HCFG level.
  SmallPtrSet<VPPredicateRecipeBase *, 16> AllOnesPreds;
  if (IncomingAllOnesPred) {
    AllOnesPreds.insert(IncomingAllOnesPred);
  }

  // Pre-order optimizations
  optimizeRegionPreOrder(Region, AllOnesPreds);

  // Recursively optimize sub-regions within Region
  for (auto Block : make_range(df_iterator<VPRegionBlock *>::begin(Region),
                               df_iterator<VPRegionBlock *>::end(Region))) {

    if (VPRegionBlock *SubRegion = dyn_cast<VPRegionBlock>(Block)) {
      // If Region's predicate is/became all-ones, propagate such all-ones
      // predicate inside the region and set Region's predicate to nullptr.
      VPPredicateRecipeBase *SRPred = SubRegion->getPredicateRecipe();
      VPPredicateRecipeBase *SRIncomingAllOnesPred = nullptr;
      if (AllOnesPreds.count(SRPred)) {
        SRIncomingAllOnesPred = SRPred;
        SubRegion->setPredicateRecipe(nullptr);
      };

      optimizeRegionRec(SubRegion, SRIncomingAllOnesPred);
    }
  }

  // Post-order optimizations
  optimizeRegionPostOrder(IncomingAllOnesPred, AllOnesPreds);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  genLitReport(Region);
#endif // !NDEBUG || LLVM_ENABLE_DUMP
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
// FIXME: Only for debugging. Can be removed.
static void dumpVplanDot(VPlan *Plan,
                         const char *dotFile = "/tmp/vplan.dot") {
  if (DumpVPlanDot) {
    std::error_code EC;
    raw_fd_ostream file(dotFile, EC, sys::fs::FA_Write);
    file << *Plan;
    file.close();
  }
}
#endif // !NDEBUG || LLVM_ENABLE_DUMP

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
    if (VPlanUtils::isBackEdge(From, To)) {
      errs() << "Saw backedge\n";
      return false;
    }
    
    return Visited.insert(To).second;
  }

  void finishPostorder(VPBlockBase *VB) {};
};
}
#endif

// Linearize the CFG within Region.
// TODO: Predication and linearization need RPOT for every region.
// This traversal is expensive. Since predication is not adding new
// blocks, we should be able to compute RPOT once in predication and
// reuse it here.
void VPlanPredicator::linearizeRegionRec(VPRegionBlock *Region) {

  // 1. Linearize CFG within Region only if it's divergent.
  if (Region->isDivergent()) {

    ReversePostOrderTraversal<VPBlockBase *> RPOT(Region->getEntry());
    VPBlockBase *PrevBlock = nullptr;

    // TODO: RPO is not providing the right topological order (if->else->then
    // instead of if->then->else) but currently it works for the Q1 test cases.
    // This problem could have improved with the fix for adding VPBB
    // predecessors in the same order as LLVM BB.
    for (VPBlockBase *CurrBlock : make_range(RPOT.begin(), RPOT.end())) {
      // dbgs() << "LV: VPBlock in RPO " << CurrBlock->getName() << '\n';

      // We have to preserve the right order of successors when a
      // ConditionBit is kept after linearization. Currently, only loop
      // latches' CBRs are preserved. For that reason, we keep intact loop
      // latches' successors or loop header's predecessors.
      // Current implementation doesn't work if a loop latch has a switch.
      assert((!PrevBlock || !VPBlockUtils::blockIsLoopLatch(PrevBlock, VPLI) ||
              PrevBlock->getNumSuccessors() < 3) &&
             "Linearization doesn't support switches in loop latches");

      if (PrevBlock && !VPLI->isLoopHeader(CurrBlock) &&
          !VPBlockUtils::blockIsLoopLatch(PrevBlock, VPLI)) {

        LLVM_DEBUG(dbgs() << "Linearizing: " << PrevBlock->getName() << "->"
                          << CurrBlock->getName() << "\n");

        PrevBlock->clearSuccessors();
        CurrBlock->clearPredecessors();
        VPBlockUtils::connectBlocks(PrevBlock, CurrBlock);
      }

      PrevBlock = CurrBlock;
    }
  }

  // 2. Recurse inside Region
  for (auto *Block : make_range(df_iterator<VPRegionBlock *>::begin(Region),
                                df_iterator<VPRegionBlock *>::end(Region))) {
    if (VPRegionBlock *SubRegion = dyn_cast<VPRegionBlock>(Block)) {
      linearizeRegionRec(SubRegion);
    }
  }
}

#include "IntelVPlanLoopCFU.h"

// Entry point. The driver function for the predicator.
void VPlanPredicator::predicate(void) {
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  dumpVplanDot(Plan, "/tmp/vplan.before.dot"); // For debugging
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  // Get the outermost VPLoopRegion where predication will start from
  // TODO: We should have a better way to do this. A pointer in VPlan, for
  // example.
  assert(VPLI->size() == 1 && "more than 1 loop?");
  VPBlockBase *PH = (*VPLI->begin())->getLoopPreheader();
  assert(PH && "Unexpected null pre-header!");
  VPLoopRegion *EntryLoopR = cast<VPLoopRegion>(PH->getParent());

  // The plan's entry loop region must have no predicate (all-ones).
  assert(!EntryLoopR->getPredicateRecipe() &&
         "Entry loop region must have no predicate.");

  // Transform inner loop control to become uniform.
  if (VPlanLoopCFU)
    handleInnerLoopBackedges(EntryLoopR->getVPLoop());

  // Predicate the blocks within Region and recursively predicate nested
  // regions.
  predicateRegionRec(EntryLoopR);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  dumpVplanDot(Plan, "/tmp/vplan.after.dot"); // For debugging
#endif // !NDEBUG || LLVM_ENABLE_DUMP

  // Optimize predicates within Region and recursively optimize predicates in
  // nested regions.
  if (!DisablePredicatorOpts) {
    optimizeRegionRec(EntryLoopR, nullptr /* IncomingAllOnesPred */);
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
    dumpVplanDot(Plan, "/tmp/vplan.optimized.dot"); // For debugging
#endif // !NDEBUG || LLVM_ENABLE_DUMP
  }
  const VPLoop *VPL = EntryLoopR->getVPLoop();
  SmallVector<VPBlockBase *, 8> Exits;
  VPL->getExitBlocks(Exits);
  // FIXME: Current linearization doesn't work correctly for multi exit loops,
  // thus disable it by now.
  if (Exits.size() > 1)
    return;

  linearizeRegionRec(EntryLoopR);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  dumpVplanDot(Plan, "/tmp/vplan.after.linearized.dot"); // For debugging
#endif // !NDEBUG || LLVM_ENABLE_DUMP
}
