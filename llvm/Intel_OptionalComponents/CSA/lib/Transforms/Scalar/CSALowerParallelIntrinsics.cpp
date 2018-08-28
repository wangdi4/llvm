//===- CSALowerParallelIntrinsics.cpp - Lower section intrinsics into metadata
//-*- C++ -*-===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
///===---------------------------------------------------------------------===//
/// \file
///
/// This file provides implementation of CSALowerParallelIntrinsics.
///
///===---------------------------------------------------------------------===//

#include "Intel_CSA/CSAIRPasses.h"
#include "Intel_CSA/Transforms/Scalar/CSALowerParallelIntrinsics.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/MDBuilder.h"
#include "llvm/Pass.h"
#include "llvm/Support/GenericDomTreeConstruction.h"
#include <deque>
#include <queue>

using namespace llvm;

namespace llvm {
// Forward declarations.
class CSALowerParallelIntrinsicsImpl;
} // end namespace llvm


namespace {
/// \brief The class defines a section defined by a pair of
/// llvm.csa.parallel_section_entry/exit calls.
class Section {
  /// \brief A call to llvm.csa.parallel_section_entry
  /// for this section.
  IntrinsicInst *EntryCall;

  /// \brief A call to llvm.csa.parallel_section_exit
  /// for this section.
  IntrinsicInst *ExitCall;

  /// \brief A list of memory accessing instructions located inside
  /// the section.
  ///
  /// We may keep them unordered, because they are processed uniformly.
  /// Note that instructions belonging to the subsections are not present
  /// in this list, thus, one has to look into the subsections to
  /// find all memory accesses located in this section.
  SmallVector<Instruction *, 128> MemoryInstructions;

  /// \brief A pointer to the parent section.
  ///
  /// If SPMDization has done its job, then we may have the following IR:
  ///    %0 = region.entry <- parallel region entry
  ///    %1 = region.entry <- SPMD region entry
  ///    %2 = section.entry(%1)
  ///    loop_worker1:
  ///        %3 = section.entry(%0)
  ///        section.exit(%3)
  ///    br loop_worker1
  ///    section.exit(%2)
  ///    %4 = section.entry(%1)
  ///    loop_worker2:
  ///        %5 = section.entry(%0)
  ///        section.exit(%5)
  ///    br loop_worker2
  ///    section.exit(%4)
  ///    region.exit(%1) <- SPMD region exit
  ///    region.exit(%0) <- parallel region exit
  ///
  /// Section %2 will be a parent for section %3, and section %4
  /// will be a parent for section %5.
  Section *Parent;

  /// \brief A set of subsections included into this section.
  ///
  /// Keep the enclosed sections in order, so that we assign
  /// the alias scopes deterministically.
  SmallSetVector<Section *, 32> EnclosedSections;

  /// \brief Unique integer section identifier.
  unsigned ID;

  /// \brief True - if the section has been collected, false - otherwise.
  ///
  /// The attribute is set to false initially, and updated in
  /// collectSectionRecursively().
  bool IsCollected;

  /// \brief An alias scope defined by this section.
  MDNode *Scope;

  /// \brief A list of alias scopes defined by sections that are
  /// "parallel" to this section.
  ///
  /// Each section defines its own alias scope.  If there is a set
  /// of sections that are "parallel" to this one (i.e. memory accesses
  /// from these sections do not alias with memory accesses in this
  /// section), then the NoaliasScopes is a composite metadata node
  /// listing Scope nodes of these other sections.
  MDNode *NoaliasScopes;

public:
  /// \brief Constructor with the section call \p II and
  /// the \p Context.
  Section(IntrinsicInst *II, CSALowerParallelIntrinsicsImpl &Context);
  /// \brief Constructor with just a \p Context.  It is used to create
  /// a fake top level section that encloses all sections in the current
  /// Function that do not have the Parent.
  Section(CSALowerParallelIntrinsicsImpl &Context);

  IntrinsicInst *getEntryCall() const {
    return EntryCall;
  }

  IntrinsicInst *getExitCall() const {
    return ExitCall;
  }

  bool isTopLevel() const {
    return !Parent;
  }

  bool isCollected() const {
    return IsCollected;
  }

  /// \brief Returns true, if the given instruction \p I is a memory
  /// access instruction that must be annotated with alias.scope/noalias
  /// metadata.
  bool instMustBeAnnotated(Instruction *I) const {
    if (!I->mayReadOrWriteMemory())
      return false;

    IntrinsicInst *II = dyn_cast<IntrinsicInst>(I);

    // Avoid marking CSA intrinsics, because it is not needed
    // and just increases the size of the memory access lists.
    // We will remove the section/region entry/exit calls later,
    // and marking SPMD intrinsics does not add any value.
    if (II &&
        (II->getIntrinsicID() == Intrinsic::csa_parallel_section_entry ||
         II->getIntrinsicID() == Intrinsic::csa_parallel_section_exit ||
         II->getIntrinsicID() == Intrinsic::csa_parallel_region_entry ||
         II->getIntrinsicID() == Intrinsic::csa_parallel_region_exit ||
         II->getIntrinsicID() == Intrinsic::csa_spmdization_entry ||
         II->getIntrinsicID() == Intrinsic::csa_spmdization_exit))
      return false;

    return true;
  }

  /// \brief Add the given memory access instruction \p I into the list
  /// of memory accesses for this section.
  void addMemAccess(Instruction *I) {
    MemoryInstructions.push_back(I);
  }

  /// \brief Add the given section \p S as a subsection of this section.
  ///
  /// The given \p S section is added into the list of enclosed sections
  /// of this section, and this section becomes a Parent for the given
  /// \p S section.
  void addSubsection(Section *S) {
    assert(EnclosedSections.count(S) == 0 &&
           "Trying to add a subsection multiple times.");
    EnclosedSections.insert(S);
    assert(!S->Parent && "A subsection already has a parent.");
    S->Parent = this;
  }

  /// \brief Given the pass \p Context, walks all instructions enclosed
  /// in this section and populates the subsections list and the memory
  /// accesses list.  The \p Padding is used internally for aligning
  /// debug dumps.
  ///
  /// The method starts from this section and walks all the instructions
  /// enclosed in it.  If a subsection (a call to
  /// llvm.csa.parallel_section_entry) is met during the walk, the method
  /// processes this subsection recursively, and then adds the subsection
  /// into the list of enclosed subsections.  If an instMustBeAnnotated()
  /// instruction is met during the walk of this section, then it is added
  /// into the list of memory acceses for this section.  Note that the memory
  /// access instructions belonging to the subsections are not added into
  /// the list of memory accesses for this section.
  /// Depending on the order of processing, a subsection may be processed
  /// before this section.  In this case, when during the walk of this section
  /// we meet the subsection, we identify the processed subsection by
  /// checking IsCollected and do not process it again - we just add it
  /// into the list of enclosed sections.
  /// When this method is run for all sections, they all have IsCollected set.
  void collectSectionRecursively(CSALowerParallelIntrinsicsImpl &Context,
                                 unsigned Padding = 0);

  /// \brief Process this section (and its subsections recursively):
  /// mark the memory accessing instructions with metadata and mark
  /// the enclosing loop as CSALoopTag::Parallel.
  ///
  /// This method is called once for the fake top level section that
  /// encloses all orthan sections after the collection is done.
  /// It walks the sections tree such that the whole list of subsections
  /// is processed before their parent.  For any section the list of its
  /// subsections define the sets of memory accesses that do not alias, e.g.
  ///     SF
  ///      |->S1
  ///      |   |->S2
  ///      |   |->S3
  ///      |   |->S4
  ///      |
  ///      |->S5
  ///          |->S6
  ///          |->S7
  ///
  /// SF is a fake top level section enclosing S1 and S5.  Memory accesses
  /// from S2, S3 and S4 do not alias between each other (but the memory
  /// accesses inside, for example, S2 may alias between each other).
  /// When we process the list of subsections of S1, namely, S2, S3 and S4
  /// we create an anonymous alias domain AD1 for S1, then we create
  /// alias scopes AS2, AS3 and AS4 for S2, S3 and S4.  For every memory
  /// access instruction in S2/S3/S4 we set alias.scope metadata to
  /// AS2/AS3/AS4.  For every section S2/S3/S4 we create metadata nodes
  /// listing the alias scopes of the other sibling sections, e.g.
  /// for S2 -> {AS3, AS4}, for S3 -> {AS2, AS4} and for S4 -> {AS2, AS3}.
  /// For every memory access instruction in S2/S3/S4 we set noalias metadata
  /// to {AS3, AS4}/{AS2, AS4}/{AS2, AS3}.  This metadata represents
  /// the noaliasing information henceforth.  The same is done for S6 and S7.
  ///
  /// When we process S1 and S5 as subsections of SF, we create the alias
  /// scope and noalias metadata the same way, but the memory access
  /// instructions that need to be annotated are collected from the
  /// subsections as well, i.e.
  ///     InstructionsForAnnotation(S1) := MemoryInstructions(S1) U
  ///                                      MemoryInstructions(S2) U
  ///                                      MemoryInstructions(S3) U
  ///                                      MemoryInstructions(S4)
  ///     InstructionsForAnnotation(S5) := MemoryInstructions(S6) U
  ///                                      MemoryInstructions(S7)
  ///
  /// The last step is to mark the Loops containing the sections as
  /// CSALoopTag::Parallel.  Please refer to the method body
  /// for a comprehensive comment explaining how we look for Loops
  /// that need to be marked.
  void processSectionRecursively(CSALowerParallelIntrinsicsImpl &Context);
};

} // end anonymous namespace

namespace llvm {
  // Legacy wrapper pass to provide the CSALowerParallelIntrinsics object.
  class CSALowerParallelIntrinsicsWrapper : public FunctionPass {
    std::unique_ptr<CSALowerParallelIntrinsics> Result;

  public:
    static char ID;
    CSALowerParallelIntrinsics Impl;

    bool runOnFunction(Function &) override;

    CSALowerParallelIntrinsicsWrapper() : FunctionPass(ID) {
      initializeCSALowerParallelIntrinsicsWrapperPass(
          *PassRegistry::getPassRegistry());
    }

    void getAnalysisUsage(AnalysisUsage &) const override;

    StringRef getPassName() const override {
      return "CSA: Lower parallel intrinsics";
    }
  };

  class CSALowerParallelIntrinsicsImpl {
    friend Section;

    DenseMap<IntrinsicInst *, Section *> CallToSectionMap;

    /// \brief F - a reference to the current Function.
    Function &F;
    /// \brief DT - a reference to DominatorTree structure.
    DominatorTree &DT;
    /// \brief LI - a reference to LoopInfo structure.
    LoopInfo &LI;

    /// \brief Unique integer identifier of a section.
    unsigned SectionID;

    /// \brief Delete all CSA intrinsic calls in the current Function.
    void deleteIntrinsicCalls(ArrayRef<Section *> Sections);

    /// \brief Map the given CSA intrinsic call \p I to the given
    /// Section \p S created for this call.
    void setSectionForInst(IntrinsicInst *I, Section *S) {
      CallToSectionMap[I] = S;
    }

    /// \brief Return Section corresponding to the given CSA intrinsic
    /// call \p I.
    Section *getSectionFromInst(IntrinsicInst *I) const {
      auto MI = CallToSectionMap.find(I);

      assert(MI != CallToSectionMap.end() &&
             "Unmapped llvm.csa.parallel_section_entry.");

      return MI->second;
    }

  public:
    /// \brief Constructor initializing the pass's context
    /// with the given anlysis' results.
    CSALowerParallelIntrinsicsImpl(
        Function &F, DominatorTree &DT, LoopInfo &LI) :
      F(F), DT(DT), LI(LI), SectionID(0) {}

    /// \brief Pass entry point.
    bool run();
  };
} // end namespace llvm

#define DEBUG_TYPE "csa-lower-parallel-intrinsic"

static cl::opt<bool> EnableLowering{
  "csa-lower-parallel-intrinsics", cl::Hidden,
  cl::desc("Replace region/section entry/exit calls with scoped AA metadata, "
           "and mark the loops as Parallel."),
  cl::init(false)};

char CSALowerParallelIntrinsicsWrapper::ID = 0;
INITIALIZE_PASS_BEGIN(CSALowerParallelIntrinsicsWrapper, DEBUG_TYPE,
                      "CSA: Lower parallel intrinsics", false, false)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_END(CSALowerParallelIntrinsicsWrapper, DEBUG_TYPE,
                    "CSA: Lower parallel intrinsics", false, false)

void CSALowerParallelIntrinsicsWrapper::getAnalysisUsage(
    AnalysisUsage &AU) const {
  AU.addRequired<DominatorTreeWrapperPass>();
  AU.addRequired<LoopInfoWrapperPass>();
  AU.setPreservesAll();
  FunctionPass::getAnalysisUsage(AU);
}

Pass *llvm::createCSALowerParallelIntrinsicsWrapperPass() {
  return new CSALowerParallelIntrinsicsWrapper();
}

PreservedAnalyses CSALowerParallelIntrinsics::run(Function &F,
                                                  FunctionAnalysisManager &AM)
{
  auto &DT = AM.getResult<DominatorTreeAnalysis>(F);
  auto &LI = AM.getResult<LoopAnalysis>(F);

  (void)CSALowerParallelIntrinsicsImpl(F, DT, LI).run();

  return PreservedAnalyses::all();
}

bool CSALowerParallelIntrinsicsWrapper::runOnFunction(Function &F)
{
  if (skipFunction(F))
    return false;

  auto &DT = getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  auto &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();

  return CSALowerParallelIntrinsicsImpl(F, DT, LI).run();
}

bool CSALowerParallelIntrinsicsImpl::run() {
  if (!EnableLowering) {
    return false;
  }

  LLVM_DEBUG(dbgs() << "Initial collection of sections.\n");

  // Keep an ordered list of sections so that the processing order
  // is defined by the instructions ordering.
  SmallSetVector<Section *, 128> AllSections;

  for (auto &I : instructions(F)) {
    if (IntrinsicInst *EntryCall = dyn_cast<IntrinsicInst>(&I))
      if (EntryCall->getIntrinsicID() ==
          Intrinsic::csa_parallel_section_entry) {
        // Create new section, record it in AllSections and
        // set up mapping between the intrinsic call and the new section
        // object.
        Section *S = new Section(EntryCall, *this);
        AllSections.insert(S);
        setSectionForInst(EntryCall, S);
      }
  }

  if (AllSections.empty()) {
    LLVM_DEBUG(dbgs() << "No sections found.\n");
    return false;
  }

  LLVM_DEBUG(dbgs() << "Reconstructing sections hierarchy.\n");

  for (auto *S : AllSections) {
    S->collectSectionRecursively(*this);
  }

  LLVM_DEBUG(dbgs() << "Replace intrinsics with metadata.\n");

  // Create a fake section that encloses all top-level sections.
  Section *Top = new Section(*this);

  for (auto *S : AllSections) {
    if (S->isTopLevel())
      Top->addSubsection(S);
  }

  // Add metadata for the memory accesses based on the collected
  // sections.
  Top->processSectionRecursively(*this);

  // Delete intrinsics in the Function.
  deleteIntrinsicCalls(AllSections.getArrayRef());

  // Release memory.
  for (auto *S : AllSections)
    delete S;

  delete Top;

  return true;
}

Section::Section(CSALowerParallelIntrinsicsImpl &Context) :
  EntryCall(nullptr), ExitCall(nullptr), Parent(nullptr),
  ID(Context.SectionID++), IsCollected(false),
  Scope(nullptr), NoaliasScopes(nullptr) {

  // The fake top-level section will not have entry/exit instructions
  // in the debug output.  For the real sections the entry/exit
  // instructions will be printed in the other constructor.
  LLVM_DEBUG(dbgs() << "Section " << ID << " created:\n");
}

Section::Section(IntrinsicInst *II, CSALowerParallelIntrinsicsImpl &Context) :
  Section(Context) {

  EntryCall = II;

  LLVM_DEBUG(dbgs() << "Entry: " << *EntryCall << "\n");

  for (auto *I : EntryCall->users()) {
    assert(!ExitCall && "Multiple users of llvm.csa.parallel.section.entry");
    ExitCall = dyn_cast<IntrinsicInst>(I);
    assert(ExitCall &&
           ExitCall->getIntrinsicID() ==
           Intrinsic::csa_parallel_section_exit &&
           "Invalid use of llvm.csa.parallel.section.entry.");

    assert(Context.DT.dominates(EntryCall->getParent(),
                                ExitCall->getParent()) &&
           "Section entry does not dominate section exit.");
  }

  LLVM_DEBUG(dbgs() << "Exit: " << *ExitCall << "\n");
}

void Section::collectSectionRecursively(
    CSALowerParallelIntrinsicsImpl &Context, unsigned Padding) {
  if (isCollected())
    return;

  LLVM_DEBUG(dbgs() << std::string(Padding, ' ') <<
             "Processing section " << ID << " {\n");
  LLVM_DEBUG(dbgs() << std::string(Padding + 1, ' ') <<
             "Entry: " << *EntryCall << "\n");

  // Use BasicBlocks set to avoid loops during the walk.
  DenseSet<BasicBlock *> VisitedBlocks;

  // Working list of BasicBlocks to be walked to find
  // all instructions dominated by the EntryCall and post-dominated
  // by the ExitCall.  Note that the subsection's blocks are not
  // tracked as visited (they are tracked by the recursive subsection
  // walks themselves).
  std::queue<BasicBlock *> BlocksToVisit;

  auto *StartBlock = EntryCall->getParent();
  bool MatchFound = false;

  BlocksToVisit.push(StartBlock);

  while (!BlocksToVisit.empty()) {
    auto *BB = BlocksToVisit.front();
    BlocksToVisit.pop();
    VisitedBlocks.insert(BB);

    Instruction *NextInst =
      (EntryCall->getParent() != BB) ?
      &BB->front() :
      ((EntryCall == &BB->back()) ? nullptr : EntryCall->getNextNode());

    while (NextInst && NextInst != ExitCall) {
      IntrinsicInst *I = dyn_cast<IntrinsicInst>(NextInst);

      if (I &&
          I->getIntrinsicID() == Intrinsic::csa_parallel_section_entry) {
        // If we reached the EntryCall again, then the CFG between
        // the section entry/exit calls is broken.
        assert(EntryCall != I &&
               "Invalid structure of sections.");

        // This is a beginning of a subsection.
        Section *SS = Context.getSectionFromInst(I);

        if (SS->isCollected())
          LLVM_DEBUG(dbgs() << std::string(Padding, ' ') <<
                     "Precomputed section " << SS->ID << "\n");

        SS->collectSectionRecursively(Context, Padding + 1);
        addSubsection(SS);

        // Continue walking instructions after the subsection's
        // exit call.
        IntrinsicInst *SubSectionExitCall = SS->ExitCall;

        if (BB != SubSectionExitCall->getParent()) {
          BB = SubSectionExitCall->getParent();
          VisitedBlocks.insert(BB);
        }
        NextInst = SubSectionExitCall;
      } else if (I &&
                 I->getIntrinsicID() ==
                 Intrinsic::csa_parallel_section_exit) {
        assert(false &&
               "Wrong structure of section entry/exit calls.");
      } else if (instMustBeAnnotated(NextInst)) {
        addMemAccess(NextInst);
        LLVM_DEBUG(dbgs() << std::string(Padding, ' ') <<
                   "Mem: " << *NextInst << "\n");
      }

      NextInst = NextInst->getNextNode();
    }

    if (NextInst == ExitCall) {
      // We completed collecting the section.
      MatchFound = true;
    }
    else {
      for (auto *SB : successors(BB)) {
        if (VisitedBlocks.find(SB) == VisitedBlocks.end())
          BlocksToVisit.push(SB);
      }
    }
  }

  assert(MatchFound && "No matching section exit.");
  (void)MatchFound;

  IsCollected = true;

  LLVM_DEBUG(dbgs() << std::string(Padding + 1, ' ') <<
             "Exit: " << *ExitCall << "\n");
  LLVM_DEBUG(dbgs() << std::string(Padding, ' ') << "}\n");
}

void Section::processSectionRecursively(
    llvm::CSALowerParallelIntrinsicsImpl &Context) {

  LLVM_DEBUG(dbgs() << "Processing section " << ID << ".\n");

  // Process the enclosed sections first.
  for (auto *SS : EnclosedSections) {
    if (!SS->EnclosedSections.empty())
      SS->processSectionRecursively(Context);
  }

  if (EnclosedSections.size() > 1) {
    // Create alias scope for each enclosed section,
    // and assign the alias.scope/noalias metadata
    // to the memory accessing operations inside
    // the enclosing sections.

    // Create a new domain using the current section's name.
    MDBuilder MDB(Context.F.getContext());

    // If this is a fake top-level section, then get the domain
    // name from the Function name.
    StringRef DomainName =
      ExitCall ?
      ExitCall->getOperand(0)->getName() :
      Context.F.getName();

    MDNode *NewDomain = MDB.createAnonymousAliasScopeDomain(DomainName);

    // Create a new alias.scope for each subsection.
    for (auto *SS : EnclosedSections) {
      StringRef ScopeName = SS->ExitCall->getOperand(0)->getName();
      SS->Scope = MDB.createAnonymousAliasScope(NewDomain, ScopeName);
    }

    // For each subsection create a list of scopes created for all the other
    // subsections.
    for (auto *SS : EnclosedSections) {
      for (auto *SSS: EnclosedSections) {
        if (SS != SSS) {
          if (!SS->NoaliasScopes)
            SS->NoaliasScopes =
              MDNode::get(Context.F.getContext(), SSS->Scope);
          else
            SS->NoaliasScopes =
              MDNode::concatenate(SS->NoaliasScopes,
                                  MDNode::get(Context.F.getContext(),
                                              SSS->Scope));
        }
      }
    }

    // Assign alias.scope/noalias metadata to the memory accessing
    // instructions inside the subsections recursively (i.e. the memory
    // accessing instructions in the subsections' subsections must be
    // also marked).
    for (auto *SS : EnclosedSections) {
      MDNode *AliasScope = SS->Scope;
      MDNode *Noalias = SS->NoaliasScopes;

      std::function<void(Section *)> markSectionRecursively =
        [&markSectionRecursively, &Context, AliasScope, Noalias](Section *S) {
        for (auto *I : S->MemoryInstructions) {
          I->setMetadata(LLVMContext::MD_alias_scope,
                         MDNode::concatenate(
                             I->getMetadata(LLVMContext::MD_alias_scope),
                             MDNode::get(Context.F.getContext(), AliasScope)));
          I->setMetadata(LLVMContext::MD_noalias,
                         MDNode::concatenate(
                             I->getMetadata(LLVMContext::MD_noalias),
                             Noalias));
        }

        for (auto *SS : S->EnclosedSections)
          markSectionRecursively(SS);
      };

      markSectionRecursively(SS);
    }

    LLVM_DEBUG(dbgs() <<
               "Memory accesses annotated for subsections of section "
               << ID << ".\n");
  }
  else {
    LLVM_DEBUG(dbgs() << "No memory accesses in section " << ID << ".\n");
  }

  // Setup loop metadata for loops containing the subsections
  // of the current section.  We have to be careful to mark the right loops.
  //
  // Let's consider a parallel loop:
  //     region.entry
  //     loop:
  //         section.entry
  //         section.exit
  //     br loop
  //     region.exit
  //
  // If we completely unroll a loop, then we will get:
  //     region.entry
  //         section.entry
  //         section.exit
  //         section.entry
  //         section.exit
  //     region.exit
  //
  // When we process these sections as subsections of some enclosing section
  // (probably the fake top level section), we may figure out to which Loop
  // they belong to.  But we cannot mark this Loop as parallel,
  // if the corresponding region entry/exit calls are located inside the same
  // loop - this would mean that the original loop was completely unrolled.
  //
  // Another interesting example is loop unswitching transformation:
  //     region.entry
  //     loop:
  //         section.entry
  //         if () THEN_CODE else ELSE_CODE
  //         section.exit
  //     br loop
  //     region.exit
  //
  // After transformation:
  //     region.entry
  //     if () {
  //         loop1:
  //             section.entry
  //             THEN_CODE
  //             section.exit
  //         br loop1
  //     } else {
  //         loop2:
  //             section.entry
  //             ELSE_CODE
  //             section.exit
  //         br loop2
  //     }
  //     region.exit
  //
  // The two sections above will be subsections of some enclosing section,
  // but they belong to two different loops, both of which must be marked
  // as parallel.
  //
  // It is a question whether we may have a Loop like this:
  //
  //     %1 = region.entry
  //     %2 = region.entry
  //     loop:
  //         %3 = section.entry(%1)
  //             section.entry(%2)
  //             section.exit
  //         section.exit(%3)
  //         %4 = section.entry(%1)
  //             section.entry(%2)
  //             section.exit
  //         section.exit(%4)
  //     br loop
  //
  // In this case, the loop may be marked as parallel twice, first, when
  // we process section %3, and then, when we process section %4.
  // I am not trying to verify that, and just avoid marking loops twice
  // during processing multiple subsections.
  //

  DenseSet<Loop *> MarkedLoops;

  for_each(EnclosedSections,
           [&MarkedLoops,&Context] (Section *SS) {
             auto *SectionBB = SS->EntryCall->getParent();
             auto *RegionEntryCall =
               dyn_cast<IntrinsicInst>(SS->EntryCall->getOperand(0));

             assert(RegionEntryCall &&
                    RegionEntryCall->getIntrinsicID() ==
                    Intrinsic::csa_parallel_region_entry &&
                    "Invalid operand of llvm.csa.parallel_section_entry.");

             auto *L = Context.LI.getLoopFor(SectionBB);
             if (!L)
               return;

             if (L == Context.LI.getLoopFor(RegionEntryCall->getParent()))
               return;

             if (MarkedLoops.find(L) != MarkedLoops.end())
               return;

             // Mark the Loop as parallel.
             auto *LoopID = L->getLoopID();
             MDString *MDKey =
               MDString::get(Context.F.getContext(), CSALoopTag::Parallel);
             MDTuple *KeyValue = MDTuple::get(Context.F.getContext(), {MDKey});
             SmallVector<Metadata *, 4> Ops;
             Ops.push_back(nullptr);
             if (LoopID)
               std::copy(std::next(LoopID->op_begin()), LoopID->op_end(),
                         std::back_inserter(Ops));
             Ops.push_back(KeyValue);
             MDTuple *NewLoopID = MDTuple::get(Context.F.getContext(), Ops);
             NewLoopID->replaceOperandWith(0, NewLoopID);
             L->setLoopID(NewLoopID);

             LLVM_DEBUG(dbgs() << "Loop " << L->getName() <<
                        " marked parallel by section " << SS->ID << ".\n");

             MarkedLoops.insert(L);
           });
}

void CSALowerParallelIntrinsicsImpl::deleteIntrinsicCalls(
    ArrayRef<Section *> Sections) {

  // First, delete all section exit calls.
  for (auto *S : Sections)
    S->getExitCall()->eraseFromParent();

  // Collect all region entry calls and delete all region exit calls.
  SmallSetVector<IntrinsicInst *, 64> RegionEntryCalls;

  for (auto *S : Sections) {
    auto *RegionEntryCall =
      dyn_cast<IntrinsicInst>(S->getEntryCall()->getOperand(0));

    assert(RegionEntryCall &&
           RegionEntryCall->getIntrinsicID() ==
           Intrinsic::csa_parallel_region_entry &&
           "Invalid operand of llvm.csa.parallel_section_entry.");

    RegionEntryCalls.insert(RegionEntryCall);

    SmallSetVector<IntrinsicInst *, 2> RegionExitCalls;

    // TODO (vzakhari 5/13/2018): Jump Threading transformation may
    //       duplicate calls to llvm.csa.parallel_region_entry, e.g.:
    //       B0:
    //           BR B1
    //
    //       B1:
    //           %0 = llvm.csa.parallel_region_entry()
    //           BR loop_header
    //
    //       may be tranformed into:
    //       B0:
    //           %0.0 = llvm.csa.parallel_region_entry()
    //           BR loop_header
    //
    //       B1:
    //           %0.1 = llvm.csa.parallel_region_entry()
    //           BR loop_header
    //
    //       loop_header:
    //           %0 = PHI %0.0, %0.1
    //
    // The transformation is legal, so we may actually support such cases
    // in this pass (see waveront_bug.c example in gerrit review for this
    // change-set).
    for (auto *I : RegionEntryCall->users()) {
      IntrinsicInst *RegionExitCall = dyn_cast<IntrinsicInst>(I);

      assert(RegionExitCall &&
             (RegionExitCall->getIntrinsicID() ==
              Intrinsic::csa_parallel_region_exit ||
              RegionExitCall->getIntrinsicID() ==
              Intrinsic::csa_parallel_section_entry) &&
             "Invalid use of region entry call.");

      // We cannot delete the exit call just yet, because
      // this will invalidate the iterator.
      if (RegionExitCall->getIntrinsicID() ==
          Intrinsic::csa_parallel_region_exit)
        RegionExitCalls.insert(RegionExitCall);
    }

    // Now we can delete the region exit call.
    for (auto *I : RegionExitCalls)
      I->eraseFromParent();
  }

  // Delete all section entry calls.
  for (auto *S : Sections) {
    S->getEntryCall()->eraseFromParent();
  }

  // Delete all region entry calls.
  for (auto *I : RegionEntryCalls)
    I->eraseFromParent();

  // Delete all SPMDization entry/exit calls.
  SmallVector<IntrinsicInst *, 16> SPMDEntryCalls;
  SmallVector<IntrinsicInst *, 16> SPMDExitCalls;

  for (auto &I : instructions(F))
    if (auto *EntryCall = dyn_cast<IntrinsicInst>(&I))
      if (EntryCall->getIntrinsicID() == Intrinsic::csa_spmdization_entry) {
        LLVM_DEBUG(dbgs() << "SPMD entry intrinsic found: "
                   << *EntryCall << "\n");

        for (auto *II : EntryCall->users()) {
          auto *ExitCall = dyn_cast<IntrinsicInst>(II);

          assert(ExitCall &&
                 ExitCall->getIntrinsicID() ==
                 Intrinsic::csa_spmdization_exit &&
                 "Invalid use of SPMD entry.");

          SPMDExitCalls.push_back(ExitCall);
        }

        SPMDEntryCalls.push_back(EntryCall);
      }

  if (SPMDEntryCalls.empty())
    return;

  for (auto *CI : SPMDExitCalls)
    CI->eraseFromParent();
  for (auto *CI : SPMDEntryCalls)
    CI->eraseFromParent();

  LLVM_DEBUG(dbgs() <<
             "CSA spmdization/region/section entry/exit calls removed.\n");
}
