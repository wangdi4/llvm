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
// VPBasicBlocks also contain the Blcok Predicate recipe within their recipe
// list, in order for the code generator to emit code for it.
//
// Connecting Predicates within a Region
// -------------------------------------
// Each VPBasicBlock gets its own Block Predicate which has one or more inputs.
// Each of the inputs is the predecessor block's block predicate.
//
// Predicate Propagation across the HCFG
// -------------------------------------
// The Entry Block of a Region shares the same predicate as the Region.
// Once a Region is about to be predicated by predicateRegion(), the
// Region->getPredicateRecipe() is set as the Region->getEntry()'s predicate.
// The top level region (VPlan's entry block) is assigned Block Predicate
// with an AllOnes recipe as its only input edge.


#include "VPlan.h"
#include "VPlanPredicator.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"
#include "llvm/ADT/GraphTraits.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/Support/FileSystem.h"
#include <sstream>

using namespace llvm;

static cl::opt<bool>
VPlanPredicatorReport("vplan-predicator-report", cl::init(false),
											cl::Hidden,
											cl::desc("VPlan Predicator report for testing"));
static cl::opt<bool>
DumpVPlanDot("dump-vplan-dot", cl::init(false), cl::Hidden,
						 cl::desc("Dump the vplan dot file"));



// Return the condition instruction of VPBB
// FIXME: This code is TEMPORARY and WRONG!
//        This should be provided by consulting condition recipe direclty.
static CmpInst *getBBCondition(VPBasicBlock *BB) {
	assert(BB);
	// Walk down the recipes looking for the condition recipe
	for (VPBasicBlock::iterator it = BB->begin(), ite = BB->end();
			 it != ite; ++it) {
		VPRecipeBase *recipe = &*it;
		if (VPOneByOneRecipeBase *OBORecipe
				= dyn_cast<VPOneByOneRecipeBase>(recipe)) {
			// FIXME: This code is wrong.
			//        We should be using the condition recipe directly.
			for (BasicBlock::iterator bbit = OBORecipe->begin(),
						 bbite = OBORecipe->end(); bbit != bbite; ++bbit) {
				Instruction *Instr = &*bbit;
				if (CmpInst *CI = dyn_cast<CmpInst>(Instr)) {
					return CI;
				}
			}
		}
	}
	return nullptr;
}

// Returns the first recipe in BB or NULL if empty.
static VPRecipeBase *getFirstRecipeSafe(VPBasicBlock *BB) {
	VPRecipeBase *FirstRecipe = nullptr;
	if (! BB->getRecipes().empty()) {
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
		if (! isBackEdge(PredBlock, SuccBlock)) {
			Succs.push_back(SuccBlock);
		}
	}
}

// Helper function generating a unique name for a predicate
static std::string genPredicateName(const std::string &Name,
																		VPBlockBase *Block) {
	VPBlockBase *ParentBlock = Block->getParent();
	std::string ParentBlockName;
	if (ParentBlock) {
		ParentBlockName = ParentBlock->getName();
	} else {
		ParentBlockName = "VPlan";
	}
	return "[" + ParentBlockName + "." + Block->getName() + "]" + Name;
}

// Return/Generate the incoming predicate.
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
	//    instead.
	if (PredSuccessorsNoBE.size() == 1) {
		IncomingPredicate = PredBlock->getPredicateRecipe();
	}
	// If the predecessor block contains a condition
	else if (PredSuccessorsNoBE.size() == 2) {
		VPBasicBlock *PredBB = dyn_cast<VPBasicBlock>(PredBlock);
		assert(PredBB && "Only BBs can have more than one successsors");
		// FIXME: This is a HACK: getBBCondition() should use a utils function instead
		CmpInst *CI = getBBCondition(PredBB);
		// currBB is the True successor of PredBB
		if (PredBlock->getSuccessors()[0] == CurrBlock) {
			// FIXME: We should be using the creation utils instead
			VPIfTruePredicateRecipe *IfTrueRecipe
				= new VPIfTruePredicateRecipe(CI, PredBB->getPredicateRecipe());
			IfTrueRecipe->setName(genPredicateName("IfTrue", PredBB));
			emitRecipeIfBB(IfTrueRecipe, CurrBlock);
			IncomingPredicate = IfTrueRecipe;
		}

		// currBB is the False successor of PredBB
		if (PredBlock->getSuccessors()[1] == CurrBlock) {
			// FIXME: We should be using the creation utils instead
			VPIfFalsePredicateRecipe *IfFalseRecipe
				= new VPIfFalsePredicateRecipe(CI, PredBB->getPredicateRecipe());
			IfFalseRecipe->setName(genPredicateName("IfFalse", PredBB));
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
	// Create the BP
	VPBlockPredicateRecipe *blockPredicate = new VPBlockPredicateRecipe();
	blockPredicate->setName(genPredicateName("BP", CurrBlock));
	emitRecipeIfBB(blockPredicate, CurrBlock);
	CurrBlock->setPredicateRecipe(blockPredicate);
}

// Returns TRUE if the edge FromBlock->ToBlock is a back-edge
bool VPlanPredicator::isBackEdge(VPBlockBase *FromBlock,
                                 VPBlockBase *ToBlock) {
	assert(FromBlock->getParent() == ToBlock->getParent());
	// A back-edge has to be within a loop region
	VPLoop *Loop = dyn_cast<VPLoop>(FromBlock->getParent());
	if (! Loop) {
		return false;
	}
	// A back-edge is latch->header
	return (Loop->contains(FromBlock) && Loop->contains(ToBlock)
					&& Loop->isLoopLatch(FromBlock)
					&& (ToBlock == Loop->getHeader()));
}

// Generate all predicates needed for currBB
void VPlanPredicator::propagateInputPredicates(VPBlockBase *CurrBlock,
                                               VPRegionBlock *Region) {
	// Skip entry blocks
	if (CurrBlock == Region->getEntry()) {
		return;
	}

	// For each input block, get the predicate and append it to BP
	for (VPBlockBase *PredBlock : CurrBlock->getPredecessors()) {
		// Skip back-edges
		if (isBackEdge(PredBlock, CurrBlock)) {
			continue;
		}

		VPPredicateRecipeBase *IncomingPredicate
			= genOrUseIncomingPredicate(CurrBlock, PredBlock);

		// Create a block predicate and append the inputs to it
		VPBlockPredicateRecipe *blockPredicate
			= dyn_cast<VPBlockPredicateRecipe>(CurrBlock->getPredicateRecipe());
		assert(blockPredicate);
		blockPredicate->appendIncomingPredicate(IncomingPredicate);
	}
}

// Return the predicate's name. If it is a BP, then expand its inputs too.
static std::string getExpandedName(VPPredicateRecipeBase *predicate) {
	std::string outString;
	std::stringstream ss(outString);

	ss << predicate->getName();
	if (VPBlockPredicateRecipe* blockPredicate
			= dyn_cast<VPBlockPredicateRecipe>(predicate)) {
		for (VPPredicateRecipeBase *incomingPredicate
					 : blockPredicate->getIncomingPredicates()) {
			ss << "\n";
			ss << "  " << incomingPredicate->getName()
				 << "->" << predicate->getName();
		}
	}
	return ss.str();
}

// Helper for genLitReport()
static void dumpBlockPredicate(VPBlockBase *Block) {
		VPPredicateRecipeBase *Predicate = Block->getPredicateRecipe();
		outs() << Block->getName()
					 << ": " << getExpandedName(Predicate) << "\n";
}

// Dump predicates for LIT testing.
void VPlanPredicator::genLitReport(VPRegionBlock *Region) {
	if (! VPlanPredicatorReport) {
		return;
	}

	dumpBlockPredicate(Region);
	VPBlockBase *EntryBlock = Region->getEntry();
	for (auto it = df_iterator<VPBlockBase *>::begin(EntryBlock),
				 ite = df_iterator<VPBlockBase *>::end(EntryBlock);
			 it != ite; ++it) {
		dumpBlockPredicate(*it);
	}
	outs() << "\n";
}

// Generate all predicates for LOOP
void VPlanPredicator::predicateRegion(VPRegionBlock *Region) {
	VPPredicateRecipeBase *RegionRecipe = Region->getPredicateRecipe();
	assert(RegionRecipe && "Must have been assigned an input predicate");
	VPBlockBase *EntryBlock = dyn_cast<VPBasicBlock>(Region->getEntry());

	// 1. Generate the block predicates in any order.
	for (auto it = df_iterator<VPBlockBase *>::begin(EntryBlock),
				 ite = df_iterator<VPBlockBase *>::end(EntryBlock);
			 it != ite; ++it) {
		VPBlockBase *Block = *it;
		if (Block != Region->getEntry()) {
			genAndAttachEmptyBlockPredicate(Block);
		}
	}

	// 2. Propagate the Region's predicate to its Entry Block
	Region->getEntry()->setPredicateRecipe(RegionRecipe);
	emitRecipeIfBB(RegionRecipe, EntryBlock);

	// 3. Generate edge predicates and append them to the block predicate
	//    The visitng order does not matter.
	for (auto it = df_iterator<VPBlockBase *>::begin(EntryBlock),
				 ite = df_iterator<VPBlockBase *>::end(EntryBlock);
			 it != ite; ++it) {
		propagateInputPredicates(*it, Region);
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
static void dumpVplanDot(IntelVPlan *Plan) {
	if (DumpVPlanDot) {
		std::error_code EC;
		raw_fd_ostream file("/tmp/vplan.dot", EC, sys::fs::F_RW);
		VPlanPrinter printer(file, *Plan);
		printer.dump();
		file.close();
	}
}

// Entry point. The driver function for the predicator.
void VPlanPredicator::predicate(void) {
	dumpVplanDot(Plan);	// FIXME: Only for debugging. Can be removed.

	VPBlockBase *EntryBlock = Plan->getEntry();
	std::vector<VPRegionBlock *> RegionsWorklist;
	appendRegionsToWorklist(EntryBlock, RegionsWorklist);
	assert(RegionsWorklist.size() == 1 && isa<VPRegionBlock>(EntryBlock)
				 && "We expect a single entry block for Plan.");

	// The plan's entry block should have an "AllOnes" predicate.
	VPPredicateRecipeBase *AllOnes
		= VPAllOnesPredicateRecipe::getPredicateRecipe();
	AllOnes->setName(genPredicateName("AllOnes", EntryBlock));
	genAndAttachEmptyBlockPredicate(EntryBlock);
	VPBlockPredicateRecipe *EntryBlockBP
		= dyn_cast<VPBlockPredicateRecipe>(EntryBlock->getPredicateRecipe());
	EntryBlockBP->appendIncomingPredicate(AllOnes);

	// Iterate until there are no Regions left.
	while (! RegionsWorklist.empty()) {
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
}
