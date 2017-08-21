//===- CSAMemopOrdering.cpp - CSA Memory Operation Ordering ----*- C++ -*--===//
//
//===----------------------------------------------------------------------===//
//
// This file implements a machine function pass for the CSA target that
// ensures that memory operations occur in the correct order.
//
//===----------------------------------------------------------------------===//

#include "CSA.h"
#include "CSATargetMachine.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Analysis/AliasSetTracker.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/MachineSSAUpdater.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "MachineCDG.h"

using namespace llvm;

// Flag for controlling code that deals with memory ordering.
enum OrderMemopsMode {
  // No extra code added at all for ordering.  Often incorrect.
  none = 0,

  // Linear ordering of all memops.  Dumb but should be correct.
  linear = 1,

  //  Stores inside a basic block are totally ordered.
  //  Loads ordered between the stores, but
  //  unordered with respect to each other.
  //  No reordering across basic blocks.
  wavefront = 2,
};

static cl::opt<OrderMemopsMode>
OrderMemopsType("csa-order-memops-type",
                cl::Hidden,
                cl::desc("CSA Specific: Order memory operations"),
                cl::values(clEnumVal(none,
                                     "No memory ordering. Possibly incorrect"),
                           clEnumVal(linear,
                                     "Linear ordering. Dumb but correct"),
                           clEnumVal(wavefront,
                                     "Totally ordered stores, parallel loads between stores.")),
                cl::init(OrderMemopsMode::wavefront));

//  Boolean flag.  If it is set to 0, we force "none" for memory
//  ordering.  Otherwise, we just obey the OrderMemopsType variable.
static cl::opt<int>
OrderMemops("csa-order-memops",
            cl::Hidden, cl::ZeroOrMore,
            cl::desc("CSA Specific: Disable ordering of memory operations (by setting to 0)"),
            cl::init(1));

static cl::opt<bool>
KillReadChains("csa-kill-readchains",
            cl::Hidden,
            cl::desc("CSA-specific: kill ordering chains which only link reads"),
            cl::init(false));

static cl::opt<bool>
ParallelOrderMemops("csa-parallel-memops",
            cl::Hidden,
            cl::desc("CSA-specific: use parallel builtins to generate parallel memop ordering"),
            cl::init(true));

// The register class we are going to use for all the memory-op
// dependencies.  Technically they could be I0, but I don't know how
// happy LLVM will be with that.
const TargetRegisterClass* MemopRC = &CSA::I1RegClass;


// These values are used for tuning LLVM datastructures; correctness is not at
// stake if they are off.
// Width of vectors we are using for memory op calculations.
#define MEMDEP_VEC_WIDTH 8
// A guess at the number of memops per alias set per function.
#define MEMDEP_OPS_PER_SET 32

#define DEBUG_TYPE "csa-memop-ordering"

// TODO: memop ordering statistics?
//STATISTIC(NumInlineAsmExpansions, "Number of asm()s expanded into MIs");
//STATISTIC(NumInlineAsmInstrs,     "Number of machine instructions resulting from asm()s");

namespace {

  /* This wraps AliasSetTracker, and gives a less sophisticated interface. Its
   * advantage is that it can handle PseudoSourceValues, such as frame index
   * pointers, which do not appear in IR and are not represented by Values. The
   * intended usage is as follows:
   * 1. Populate all memory ops with add()
   * 2. Query only the total number of alias sets, or the alias set number for
   *    a given MachineMemOperand. You cannot get an underlying AliasSet from
   *    MachineAliasSetTracker.
   *
   * It is illegal to query a memory op which you have not previously add()ed.
   *
   * */
  class MachineAliasSetTracker {
    public:
      MachineAliasSetTracker(AliasAnalysis &aa, MachineFrameInfo *mfi) :
        AST(aa), MFI(mfi), isMerged(false), pseudosCounter(1) { }

      /* Use 'add' to populate the tracker with pointers. */
      void add(MachineMemOperand *mop);
      /* Query the number of effective alias sets. */
      unsigned getNumAliasSets();
      /* Query the opaque ID of the set associated with a given mem op */
      unsigned getAliasSetNumForMemop(const MachineMemOperand *mop);
      void dump() const { print(dbgs()); }
      void print(raw_ostream &OS) const;


    private:
      AliasSetTracker AST;
      MachineFrameInfo *MFI;
      bool isMerged;
      std::map<const PseudoSourceValue*, unsigned> pseudos;
      unsigned pseudosCounter;
  };

  class CSAMemopOrdering : public MachineFunctionPass {
    bool runOnMachineFunction(MachineFunction &MF) override;
    const TargetMachine *TM;
    const CSASubtarget *STI;
    const MCInstrInfo  *MII;

  public:
    static char ID;
    CSAMemopOrdering() : MachineFunctionPass(ID) { }
    StringRef getPassName() const override {
      return "CSA Memory Operation Ordering";
    }

    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.addRequired<ControlDependenceGraph>();
      AU.addRequired<AAResultsWrapperPass>();
      AU.addRequired<MachineDominatorTree>();
      AU.addRequired<MachinePostDominatorTree>();
      AU.setPreservesAll();
      MachineFunctionPass::getAnalysisUsage(AU);
    }

    // Very generic helper copied from what's in MachineDominatorTree. This
    // should be in the upstream MachinePostDominatorTree, probably. (Is there
    // some way to re-use it for PDT that I couldn't figure out?)
    static bool postDominates(const MachineInstr* A, const MachineInstr* B, MachinePostDominatorTree *PDT) {
      const MachineBasicBlock *BA = A->getParent();
      const MachineBasicBlock *BB = B->getParent();
      if (BA != BB)
        return PDT->dominates(BA, BB);

      // Loop through the basic block until we find A or B.
      MachineBasicBlock::const_iterator I = BA->begin();
      for (; &*I != A && &*I != B; ++I)
        /*empty*/ ;

      // A post-dominates B if B is found first in the basic block.
      return &*I == B;
    }

  private:
    AliasAnalysis *AA;
    MachineAliasSetTracker *AS;
    MachineBasicBlock *entryBB;
    ControlDependenceGraph *CDG;
    const CSAInstrInfo *TII;
    MachineRegisterInfo *MRI;
    MachineDominatorTree *DT;
    MachinePostDominatorTree *PDT;

    struct parSectionInfo {
      unsigned sectionToken;
      MachineInstr* entry;
      MachineInstr* exit;
      bool operator<(const parSectionInfo &other) const { return entry < other.entry; }
      bool containsInstr(const MachineInstr *inst,
                         MachineDominatorTree *DT,
                         MachinePostDominatorTree *PDT) const {
        return DT->dominates(entry, inst) && postDominates(exit, inst, PDT) &&
          !DT->dominates(exit, inst) && !postDominates(entry, inst, PDT);
      }
    };

    struct parRegionInfo {
      const MachineInstr* entry;
      const MachineInstr* exit;
      unsigned regionToken;
      std::set<parSectionInfo> sections;
      std::map<unsigned, MachineInstr*> aliasSetCaptures;
      std::map<unsigned, MachineInstr*> aliasSetReleases;
      bool operator<(const parRegionInfo &other) const { return entry < other.entry; }
      const parSectionInfo* containsInstr(MachineInstr *inst,
                         MachineDominatorTree *DT,
                         MachinePostDominatorTree *PDT) const {
        auto secIter = std::find_if(sections.begin(), sections.end(),
            [inst,DT,PDT](const parSectionInfo& ps) {
            return ps.containsInstr(inst, DT, PDT);
            });
        if (secIter != sections.end())
          return &*secIter;
        else
          return nullptr;
      }
      bool isAliasSetCapture(MachineInstr *inst,
                             unsigned* aliasSet = nullptr) {
        for(const auto &mapEntry : aliasSetCaptures) {
          if (mapEntry.second == inst) {
            if (aliasSet)
              *aliasSet = mapEntry.first;
            return true;
          }
        }
        return false;
      }
      bool isAliasSetRelease(MachineInstr *inst,
                             unsigned* aliasSet = nullptr) {
        for(const auto &mapEntry : aliasSetReleases) {
          if (mapEntry.second == inst) {
            if (aliasSet)
              *aliasSet = mapEntry.first;
            return true;
          }
        }
        return false;
      }
    };

    std::vector<parRegionInfo> parRegions;

    unsigned getAliasSetForPassthrough(MachineInstr *inst) {
      unsigned aliasSet;
      for(parRegionInfo &pr : parRegions) {
        if (pr.isAliasSetCapture(inst, &aliasSet))
          return aliasSet;
        if (pr.isAliasSetRelease(inst, &aliasSet))
          return aliasSet;
      }
      assert(false && "Unknown chaining passthrough");
    }

    void dumpParRegions(void) {
      errs() << "Dumping " << parRegions.size() << " known parallel regions and their sections:\n";
      unsigned count = 0;
      for(const parRegionInfo& pl : parRegions) {
        errs() << "\tRegion " << ++count << " with " << pl.sections.size() << " parallel sections:\n";
        for(const parSectionInfo& ps : pl.sections) {
          errs() << "\t\tSection starting with instr " << *(ps.entry);
          errs() << "\t\t\tending with instr " << *(ps.exit);
        }
      }
      if (!count) {
        errs() << "(No known parallel regions.)\n";
      }
      errs() << "\n";
    }

    // For each chain, maintain 4 items:
    // start:    the first vreg in its chain
    // updater:  a MachineSSAUpdater
    // uses:     a collection of all of its uses
    // readonly: a bool: are all memops reads?
    struct depchain {
      unsigned start;
      std::unique_ptr<MachineSSAUpdater> updater;
      SmallPtrSet<MachineOperand*, MEMDEP_OPS_PER_SET> uses;
      bool readonly;
    };

    typedef DenseMap<unsigned, depchain> AliasSetDepchain;
    typedef DenseMap<unsigned, unsigned> AliasSetVReg;

    AliasSetDepchain depchains;

    void addMemoryOrderingConstraints(MachineFunction* thisMF);

    // Helper methods:

    // Update a memory instruction by setting ordering operands in-place.
    //  issued_reg is the register to define as the extra output
    //  ready_reg is the register which is the extra input
    //  If non-NULL, ready_op_num will have the operand number which uses the
    //  ready_reg stored to it.
    void order_memop_ins(
      MachineInstr& memop,
      unsigned issued_reg, unsigned ready_reg,
      unsigned *ready_op_num
    );

    // The set of memory operations which were ordered. This excludes
    // passthrough MOVs.
    std::set<MachineInstr*> orderedMemops;

    // Generate a MOV "passthrough" instruction corresponding to each region's
    // entry/exit. The normal serial ordering functions should treat these like
    // a store in terms of ordering. These instructions serve to capture the
    // ordering token on entry/exit, which is helpful when generating the
    // fork/join code for parallel sections.
    void generate_region_capture_release_movs(MachineBasicBlock& BB);

    // Create a dependency chain in virtual registers through the
    // basic block BB.
    //
    //   mem_in_reg is the virtual register number being used as
    //   input, i.e., the "source" for all the memory ops in this
    //   block.
    //
    //   This function returns the virtual register that is the "sink"
    //   of all the memory operations in this block.  The returned
    //   register might be the same as the source "mem_in_reg" if
    //   there are no memory operations in this block.
    //
    // This method also converts the LD/ST instructions into OLD/OST
    // instructions, as they are encountered.
    //
    // linear version of this function links all memory operations in
    // the block together in a single chain.
    //
    void convert_block_memops_linear(MachineBasicBlock& BB,
        AliasSetDepchain& depchains);

    // Wavefront version.   Same conceptual functionality as linear version,
    // but more optimized.
    //
    // Only serializes stores in a block, but allows loads to occur in
    // parallel between stores.
    void convert_block_memops_wavefront(MachineBasicBlock& BB,
        AliasSetDepchain& depchains);

    // Merge all the .i1 registers stored in "current_wavefront" into
    // a single output register.
    // Returns the output register, or "input_mem_reg" if
    // current_wavefront is empty.
    //
    // Note that this method has several side-effects:
    //  (a) It inserts the merge instructions after
    //      instruction MI in BB, or before the last terminator in the
    //      block if MI == NULL, and
    //  (b) It clears current_wavefront.
    unsigned merge_dependency_signals(MachineBasicBlock& BB,
                                      MachineInstr* MI,
                                      SmallVector<unsigned, MEMDEP_VEC_WIDTH>* current_wavefront,
                                      unsigned input_mem_reg,
                                      SmallVector<MachineOperand*, MEMDEP_VEC_WIDTH>* newOps = nullptr);

    // Determine whether a given instruction should be assigned ordering.
    // This is the case if it has a memory operand and if its last use and last def
    // are both %ign.
    bool should_assign_ordering(const MachineInstr& MI) const;

    // Determine if a val is trivially derived from an ancestor vreg,
    // accounting for PHI, MOV0, and MERGE1 dataflow. COPY/COPYN transforms are
    // not currently accounted for. if 'mergeOnly' is true, then only merge
    // trees will be explored. If given a pointer to a vector of users "flow",
    // it's filled with the dataflow path found. Note that there may be
    // multiple paths.
    bool isRegDerivedFromReg(MachineOperand* val, MachineOperand* ancestor, bool mergeOnly, SmallVector<MachineOperand*, MEMDEP_VEC_WIDTH> *flow) const {
      std::set<MachineOperand*> visited;
      return isRegDerivedFromReg(val, ancestor, mergeOnly, flow, &visited);
    }
    bool isRegDerivedFromReg(MachineOperand* val, MachineOperand* ancestor, bool mergeOnly) const {
      SmallVector<MachineOperand*, MEMDEP_VEC_WIDTH> unused;
      return isRegDerivedFromReg(val, ancestor, mergeOnly, &unused);
    }
    bool isRegDerivedFromReg(MachineOperand* val, MachineOperand* ancestor) const {
      return isRegDerivedFromReg(val, ancestor, false);
    }
    bool isRegDerivedFromReg(MachineOperand* val, MachineOperand* ancestor, bool mergeOnly, SmallVector<MachineOperand*, MEMDEP_VEC_WIDTH> *flow, std::set<MachineOperand*> *visited) const;

    // Enumerate parallel regions and sections.
    void findParallelRegions(MachineFunction* MF);
    // Helper for findParallelRegions. Needs cleanup.
    const MachineInstr* isInstrPostDomByIntrinsic(MachineInstr *inst,
                                                 unsigned opcode,
                                                 unsigned token) const;

    // Wipe out all of the intrinsics.
    void eraseParallelIntrinsics(MachineFunction *MF);

    unsigned isParallelRegion(MachineInstr *regionEntry, const MachineInstr** entry = nullptr, const MachineInstr** exit = nullptr) const;

    // Find ordering edges which are crossing parallel sections in the same
    // parallel region. This function also currently is in charge of removing
    // said edges.
    void relaxSectionOrderingEdges(MachineFunction *thisMF);
    void relaxSectionOrderingEdges(parRegionInfo &parReg, MachineFunction *thisMF);

    std::pair<MachineOperand*,MachineOperand*> isCrossSectionDependence(parRegionInfo* region, MachineInstr* i1, MachineInstr* i2);

  };
}

char CSAMemopOrdering::ID = 0;

MachineFunctionPass *llvm::createCSAMemopOrderingPass() {
  return new CSAMemopOrdering();
}

bool CSAMemopOrdering::runOnMachineFunction(MachineFunction &MF) {
  if (!OrderMemops || (OrderMemopsType <= OrderMemopsMode::none)) {
    return false;
  }

  TII = static_cast<const CSAInstrInfo*>(MF.getSubtarget().getInstrInfo());
  MRI = &MF.getRegInfo();

  AA = &getAnalysis<AAResultsWrapperPass>().getAAResults();
  CDG = &getAnalysis<ControlDependenceGraph>();

  DT = &getAnalysis<MachineDominatorTree>();
  PDT = &getAnalysis<MachinePostDominatorTree>();

  // Find the entry block. (Surely there's an easier way to do this?)
  ControlDependenceNode *rootN = CDG->getRoot();
  if(!(rootN && *rootN->begin())) return false;
  entryBB = (*rootN->begin())->getBlock();
  assert(entryBB && "Couldn't determine this function's entry block");

  // Create the AliasSetTracker and populate with all basic blocks.
  MachineAliasSetTracker AST(*AA, &MF.getFrameInfo());
  AS = &AST;
  for (MachineBasicBlock &MB : MF) {
    for (MachineInstr &MI : MB) {
      for (MachineMemOperand *op : MI.memoperands()) {
        AS->add(op);
      }
    }
  }
  DEBUG(errs() << "AliasSets for function " << MF.getName() << ":\n");
  DEBUG(AS->dump());

  // If generating parallel memops for regions/sections, discover the
  // regions/sections. This is similar to an analysis; it fills in some
  // structures for later use. Otherwise, find and remove all parallel
  // instrinsics.
  if (ParallelOrderMemops) {
    findParallelRegions(&MF);
    DEBUG(dumpParRegions());
  } else {
    eraseParallelIntrinsics(&MF);
  }

  // This step should run before the main dataflow conversion because
  // it introduces extra dependencies through virtual registers than
  // the dataflow conversion must also deal with. It is effectively
  // implementing the program's sequential semantics with respect to memory
  // operations.
  addMemoryOrderingConstraints(&MF);

  // If generating parallel memops for regions/sections, do the code generation
  // here and then remove the intrinsics. If "addMemoryOrderingConstraints" is
  // implementing sequential semantics, then "relaxSectionOrderingEdges" can be
  // thought of as implementing "fork/join" parallelism for executions of
  // sections within regions.
  if (ParallelOrderMemops) {
    relaxSectionOrderingEdges(&MF);
    eraseParallelIntrinsics(&MF);
  }

  return true;
}

void CSAMemopOrdering::addMemoryOrderingConstraints(MachineFunction *thisMF) {

  const unsigned MemTokenMOVOpcode = TII->getMemTokenMOVOpcode();

  SmallVector<MachineInstr*, 16> inserted_PHIs;  // Phi nodes from ssa updaters

  // Reset the set tracking all ordered memops.
  orderedMemops.clear();

  // Initialize the maps.
  depchains.clear();
  for (unsigned set=0, e=AS->getNumAliasSets(); set<e; ++set) {
    // Create the start of the chain for each alias set.
    depchains[set].start = MRI->createVirtualRegister(MemopRC);
    BuildMI(*entryBB,
        entryBB->getFirstNonPHI(),
        DebugLoc(),
        TII->get(MemTokenMOVOpcode),
        depchains[set].start).addImm(1);
    // Initialize an SSAUpdater for each set.
    depchains[set].updater.reset(new MachineSSAUpdater(*thisMF, &inserted_PHIs));
    depchains[set].updater->Initialize(depchains[set].start);
    depchains[set].updater->AddAvailableValue(entryBB, depchains[set].start);
    depchains[set].readonly = KillReadChains;
  }

  // An extra loop over all memops to determine which alias sets consist only
  // of reads.
  for (MachineBasicBlock &BB : *thisMF) {
    for (MachineInstr &MI : BB) {

      // If this instruction doesn't have memory operands, we're done.
      if (MI.memoperands_empty())
        continue;

      assert(MI.hasOneMemOperand() && "Can't handle multiple-memop ordering");
      const MachineMemOperand *mOp = *MI.memoperands_begin();

      // Use AliasAnalysis to determine which ordering chain we should be on.
      unsigned as = AS->getAliasSetNumForMemop(mOp);

      // Update the chain's readonly status.
      depchains[as].readonly &= TII->isLoad(&MI);
    }
  }

  DEBUG(errs() << "Before addMemoryOrderingConstraints");
  for (MachineBasicBlock &BB : *thisMF) {

    // Generate region entry/exit MOVs for capture.
    generate_region_capture_release_movs(BB);

    // Link all the memory ops in BB together.
    // Return the name of the last output register (which could be
    // mem_in_reg).
    switch (OrderMemopsType) {
    case OrderMemopsMode::wavefront:
      {
        convert_block_memops_wavefront(BB, depchains);
      }
      break;
    case OrderMemopsMode::linear:
      {
        convert_block_memops_linear(BB, depchains);
      }
      break;

      // We should never get here.
    case OrderMemopsMode::none:
    default:
      assert(0 && "Only linear and wavefront memory ordering implemented now.");

    }

    DEBUG(errs() << "After memop conversion of basic block: " << BB << "\n");
  }

  // Finally, use the updater for each set to fully rewrite to SSA. This
  // includes generating PHI nodes.
  for (unsigned set=0, e=AS->getNumAliasSets(); set<e; ++set) {
    for (MachineOperand *op : depchains[set].uses) {
      // There is an exception here: RewriteUse is not smart enough to find new
      // values added by "AddAvailableValue" before a use in the same basic
      // block. (I.e., it can only find them if they are in a basic block which
      // is a strict dominator.) This is only an issue when the use is in the
      // function's entry block. Fortunately, in this case, we can be sure that
      // depchain_reg contained the right value to use.
      if(op->getParent()->getParent() == entryBB)
        continue;

      depchains[set].updater->RewriteUse(*op);
    }
  }
}

void
CSAMemopOrdering::generate_region_capture_release_movs(MachineBasicBlock& BB) {
  for (MachineInstr& MI : BB) {
    if (MI.getOpcode() == CSA::CSA_PARALLEL_REGION_ENTRY ||
        MI.getOpcode() == CSA::CSA_PARALLEL_REGION_EXIT) {

      MachineOperand *regionTokenOp = &MI.getOperand(0);
      for(parRegionInfo&pr : parRegions){
        MachineOperand *entryToken = &MRI->getUniqueVRegDef(pr.regionToken)->getOperand(0);
        if (isRegDerivedFromReg(regionTokenOp, entryToken)) {
          for (unsigned aliasSet=0, e=AS->getNumAliasSets(); aliasSet<e; ++aliasSet) {
            MachineInstr *mv = BuildMI(*MI.getParent(),
                &MI,
                MI.getDebugLoc(),
                TII->get(CSA::MOV0),
                CSA::IGN).addReg(CSA::IGN);
            if (MI.getOpcode() == CSA::CSA_PARALLEL_REGION_ENTRY) {
              pr.aliasSetCaptures[aliasSet] = mv;
            }
            else {
              pr.aliasSetReleases[aliasSet] = mv;
            }
          }
        }
      }
    }
  }
}

void
CSAMemopOrdering::convert_block_memops_wavefront(MachineBasicBlock& BB,
    AliasSetDepchain& depchains) {
  DEBUG(errs() << "Wavefront memory ordering for block " << BB << "\n");

  // Save the latest evolution of each alias set's memory chain here.
  AliasSetVReg depchain_reg;
  // Also save a wavefront of load output signals per alias set.
  DenseMap<unsigned, SmallVector<unsigned, MEMDEP_VEC_WIDTH> > wavefront;

  for (MachineInstr& MI : BB) {
    DEBUG(errs() << "Found instruction: " << MI << "\n");

    // If this instruction shouldn't be ordered, we're done.
    if (not should_assign_ordering(MI))
      continue;

    unsigned as;
    if (not MI.memoperands_empty()) {
      assert(MI.hasOneMemOperand() && "Can't handle multiple-memop ordering");
      const MachineMemOperand *mOp = *MI.memoperands_begin();

      // Use AliasAnalysis to determine which ordering chain we should be on.
      as = AS->getAliasSetNumForMemop(mOp);
    } else {
      // This is not a real memory operand, but a MOV passthrough for parallel
      // section handling.
      as = getAliasSetForPassthrough(&MI);
    }

    // If this chain consists only of readonly access, then it is unnecessary.
    // This is a stronger requirement than is necessary.
    if (depchains[as].readonly) continue;

    // Create a new vreg which will be written to as the next link of the
    // chain.
    unsigned next_mem_reg = MRI->createVirtualRegister(MemopRC);

    // If the previous link is the first into this block, we use the vreg which
    // the updater was initialized with. The use will be saved, and then the
    // updater will fix it up later. Otherwise, just use an output created in
    // this block, which won't need updating.
    if (!depchain_reg[as])
      depchain_reg[as] = depchains[as].start;

    bool is_load = TII->isLoad(&MI);
    if (is_load) {
      // Just a load. Build up the set of load outputs that we depend on.
      wavefront[as].push_back(next_mem_reg);
    }
    else {
      // This is a store or atomic instruction.  If there were any loads in the
      // last interval, merge all their outputs into one output, and change the
      // latest source.
      depchain_reg[as] = merge_dependency_signals(BB,
          &MI,
          &wavefront[as],
          depchain_reg[as]);

      // Update the SSA updater.
      depchains[as].updater->AddAvailableValue(&BB, depchain_reg[as]);

      // We have merged/consumed all pending load outputs.
      assert(wavefront[as].size() == 0);
    }

    unsigned newOpNum;
    order_memop_ins(MI, next_mem_reg, depchain_reg[as], &newOpNum);

    // If the instruction uses a value coming into the block, then it will need
    // to be fixed by MachineSSAUpdater later. Save the operand to the list to
    // do this later.
    MachineOperand *newOp = &MI.getOperand(newOpNum);
    assert(newOp && newOp->isReg() && newOp->isUse());
    if (newOp->getReg() == depchains[as].start) {
      depchains[as].uses.insert(newOp);
    }

    if (!is_load) {
      // Advance the chain.
      depchain_reg[as] = next_mem_reg;

      // Update the SSA updater.
      depchains[as].updater->AddAvailableValue(&BB, next_mem_reg);
    }
  }

  for (auto &pair : wavefront) {
    unsigned as = pair.first;
    unsigned next_mem_reg = depchain_reg[as];
    assert(next_mem_reg);

    // Sink any loads at the end of the block to the end of the block.
    next_mem_reg = merge_dependency_signals(BB,
        NULL,
        &pair.second,
        depchain_reg[as]);
    depchain_reg[as] = next_mem_reg;

    // Update the SSA updater.
    depchains[as].updater->AddAvailableValue(&BB, next_mem_reg);
  }

  if (BB.isReturnBlock()) {
    const unsigned MemTokenMOVOpcode = TII->getMemTokenMOVOpcode();
    for (unsigned aliasSet=0, e=AS->getNumAliasSets(); aliasSet<e; ++aliasSet) {
      unsigned depchain_end = MRI->createVirtualRegister(&CSA::RI1RegClass);
      unsigned prev_chain_reg = depchain_reg[aliasSet] ? depchain_reg[aliasSet] : depchains[aliasSet].start;
      MachineInstr* endMov = BuildMI(BB, BB.getFirstTerminator(), DebugLoc(), TII->get(MemTokenMOVOpcode), depchain_end).addReg(prev_chain_reg);
      if (prev_chain_reg == depchains[aliasSet].start) {
        depchains[aliasSet].uses.insert(endMov->operands_end()-1);
      }
    }
  }
}

void CSAMemopOrdering::convert_block_memops_linear(MachineBasicBlock& BB,
    AliasSetDepchain& depchains) {
  // Save the latest evolution of each alias set's memory chain here.
  AliasSetVReg depchain_reg;

  for (MachineInstr& MI : BB) {
    DEBUG(errs() << "Found instruction: " << MI << "\n");

    // If this instruction shouldn't be ordered, we're done.
    if (not should_assign_ordering(MI)) continue;

    assert(MI.hasOneMemOperand() && "Can't handle multiple-memop ordering");
    const MachineMemOperand *mOp = *MI.memoperands_begin();

    // Use AliasAnalysis to determine which ordering chain we should be on.
    unsigned as = AS->getAliasSetNumForMemop(mOp);

    // If this chain consists only of readonly access, then it is unnecessary.
    // This is a stronger requirement than is necessary.
    if (depchains[as].readonly) continue;

    // Create a new vreg which will be written to as the next link of the
    // chain.
    unsigned next_mem_reg = MRI->createVirtualRegister(MemopRC);

    // If the previous link is the first into this block, we use the vreg which
    // the updater was initialized with. The use will be saved, and then the
    // updater will fix it up later. Otherwise, just use an output created in
    // this block, which won't need updating.
    if (!depchain_reg[as])
      depchain_reg[as] = depchains[as].start;

    // Hook this instruction into the chain, connecting the previous and next
    // values. The operand number using the old value is saved to newOpNum.
    unsigned newOpNum;
    order_memop_ins(MI, next_mem_reg, depchain_reg[as], &newOpNum);

    // If the instruction uses a value coming into the block, then it will need
    // to be fixed by MachineSSAUpdater later. Save the operand to the list to
    // do this later.
    MachineOperand *newOp = &MI.getOperand(newOpNum);
    assert(newOp && newOp->isReg() && newOp->isUse());
    if (newOp->getReg() == depchains[as].start) {
      depchains[as].uses.insert(newOp);
    }

    // Advance the chain.
    depchain_reg[as] = next_mem_reg;

    // Update the SSA updater, advising it on the latest value in this
    // evolution coming out of this BB.
    depchains[as].updater->AddAvailableValue(&BB, next_mem_reg);
  }
}

unsigned CSAMemopOrdering::merge_dependency_signals(MachineBasicBlock& BB,
    MachineInstr* MI,
    SmallVector<unsigned, MEMDEP_VEC_WIDTH>* current_wavefront,
    unsigned input_mem_reg,
    SmallVector<MachineOperand*, MEMDEP_VEC_WIDTH>* newOps) {

  if (current_wavefront->size() > 0) {
    DEBUG(errs() << "Merging dependency signals from " << current_wavefront->size() << " register " << "\n");

    SmallVector<unsigned, MEMDEP_VEC_WIDTH> original_wavefront;
    for(unsigned in : *current_wavefront)
      original_wavefront.push_back(in);

    // BFS-like algorithm for merging the registers together.
    // Merge consecutive pairs of dependency signals together,
    // and push the output into "next_level".
    SmallVector<unsigned, MEMDEP_VEC_WIDTH> tmp_buffer;
    SmallVector<unsigned, MEMDEP_VEC_WIDTH>* current_level;
    SmallVector<unsigned, MEMDEP_VEC_WIDTH>* next_level;

    current_level = current_wavefront;
    next_level = &tmp_buffer;

    while (current_level->size() > 1) {
      assert(next_level->size() == 0);
      for (unsigned i = 0; i < current_level->size(); i+=2) {
        // Merge current_level[i] and current_level[i+1] into
        // next_level[i/2]
        if ((i+1) < current_level->size()) {

          // Even case: we have a pair to merge.  Create a virtual
          // register + instruction to do the merge.
          unsigned next_out_reg = MRI->createVirtualRegister(MemopRC);
          MachineInstr* new_inst;
          if (MI) {
            new_inst = BuildMI(*MI->getParent(),
                               MI,
                               MI->getDebugLoc(),
                               TII->get(CSA::MERGE1),
                               next_out_reg).addImm(0).addReg((*current_level)[i]).addReg((*current_level)[i+1]);
          }
          else {
            // Adding a merge at the end of the block.
            new_inst = BuildMI(BB,
                               BB.getFirstTerminator(),
                               DebugLoc(),
                               TII->get(CSA::MERGE1),
                               next_out_reg).addImm(0).addReg((*current_level)[i]).addReg((*current_level)[i+1]);
          }
          if (newOps) {
            MachineOperand& op1 = new_inst->getOperand(2);
            MachineOperand& op2 = new_inst->getOperand(3);
            auto op1it = std::find(original_wavefront.begin(), original_wavefront.end(), op1.getReg());
            auto op2it = std::find(original_wavefront.begin(), original_wavefront.end(), op2.getReg());
            if (op1it != original_wavefront.end())
              newOps->push_back(&op1);
            if (op2it != original_wavefront.end())
              newOps->push_back(&op2);
          }
          DEBUG(errs() << "Inserted dependecy merge instruction " << *new_inst << "\n");
          next_level->push_back(next_out_reg);
        }
        else {
          // In an odd case, just pass register through to next level.
          next_level->push_back((*current_level)[i]);
        }
      }

      // Swap next and current.
      std::swap(current_level, next_level);
      next_level->clear();

      DEBUG(errs() << "Current level size is now " << current_level->size() << "\n");
      DEBUG(errs() << "Next level size is now " << next_level->size() << "\n");
    }

    assert(current_level->size() == 1);
    unsigned ans = (*current_level)[0];

    // Clear both vectors, just to be certain.
    current_level->clear();
    next_level->clear();

    return ans;
  }
  else {
    return input_mem_reg;
  }
}

void CSAMemopOrdering::order_memop_ins(
  MachineInstr& MI,
  unsigned issued_reg, unsigned ready_reg,
  unsigned *ready_op_num = nullptr
) {
  using namespace std;

  DEBUG(errs() << "We want convert this instruction.\n");
  for (unsigned i = 0; i < MI.getNumOperands(); ++i) {
    MachineOperand& MO = MI.getOperand(i);
    DEBUG(errs() << "  Operand " << i << ": " << MO << "\n");
  }
  assert(should_assign_ordering(MI));

  // Just update the last use and last def
  prev(end(MI.defs()))->ChangeToRegister(issued_reg, true);
  prev(end(MI.uses()))->ChangeToRegister(ready_reg, false);

  // ...and if requested, save the ready_op for the caller.
  if(ready_op_num != nullptr)
    *ready_op_num = MI.getNumOperands() - 1;

  DEBUG(errs() << "   Updated instruction: " << MI << "\n");

  for (unsigned i = 0; i < MI.getNumOperands(); ++i) {
    MachineOperand& MO = MI.getOperand(i);
    DEBUG(errs() << "  Operand " << i << ": " << MO << "\n");
  }

  // Finally, add this to the list of ordered instructions.
  if (!MI.memoperands_empty())
    orderedMemops.insert(&MI);
}

bool CSAMemopOrdering::isRegDerivedFromReg(MachineOperand* val, MachineOperand* ancestor, bool mergeOnly, SmallVector<MachineOperand*, MEMDEP_VEC_WIDTH> *flow, std::set<MachineOperand*> *visited) const {
  assert(val && ancestor);
  assert(flow);
  assert(visited);
  assert(val->isReg() && ancestor->isReg());
  if(val->getReg() == ancestor->getReg()) {
    return true;
  }

  if (visited->count(val))
    // refuse to recurse further.
    return false;

  visited->insert(val);

  assert(MRI->hasOneDef(val->getReg()) && "Expecting an SSA vreg");
  for(MachineInstr &defMI : MRI->def_instructions(val->getReg())) {
    // If not a PHI, then we're not going to trace backward any further.
    if(!mergeOnly && defMI.isPHI()) {
      unsigned numPhiOperands = defMI.getNumOperands();
      for(unsigned i=1; i<numPhiOperands; i+= 2) {
        MachineOperand &phiData = defMI.getOperand(i);
        if(phiData.isReg()) {
          MachineOperand* newAncestor = &phiData;
          // Return true if any incoming PHI values are themselves descendants.
          if(isRegDerivedFromReg(newAncestor, ancestor, mergeOnly, flow, visited)) {
            flow->push_back(&phiData);
            return true;
          }
        }
      }
    } else if (!mergeOnly && defMI.getOpcode() == CSA::MOV0) {
      MachineOperand &movData = defMI.getOperand(1);
      if(movData.isReg()) {
        MachineOperand* newAncestor = &movData;
        // Return true if the MOV source is a descendant.
        if (isRegDerivedFromReg(newAncestor, ancestor, mergeOnly, flow, visited)) {
          flow->push_back(&movData);
          return true;
        }
      }
    } else if (defMI.getOpcode() == CSA::MERGE1) {
      unsigned numMergeOperands = defMI.getNumOperands();
      for(unsigned i=2; i<numMergeOperands; ++i) {
        MachineOperand &mergeData = defMI.getOperand(i);
        if(mergeData.isReg()) {
          MachineOperand* newAncestor = &mergeData;
          // Return true if any incoming merge values are themselves descendants.
          if (isRegDerivedFromReg(newAncestor, ancestor, mergeOnly, flow, visited)) {
            flow->push_back(&mergeData);
            return true;
          }
        }
      }
    }
  }

  visited->erase(val);
  return false;
}

void CSAMemopOrdering::findParallelRegions(MachineFunction *MF){
  // Find and index parallel intrinsics.
  for(MachineBasicBlock& mb : *MF) {
    for(MachineInstr& mi : mb) {
      if (mi.getOpcode() == CSA::CSA_PARALLEL_REGION_ENTRY) {
        parRegionInfo newRegion;
        const MachineInstr *entry, *exit;
        unsigned regionToken = isParallelRegion(&mi, &entry, &exit);
        if(regionToken) {
          assert(entry && exit);
          assert(MRI->hasOneDef(regionToken));
          MachineOperand& regionTokenOp = *MRI->def_begin(regionToken);
          newRegion.regionToken = regionToken;
          newRegion.entry = entry;
          newRegion.exit = exit;

          for(MachineBasicBlock &mbb : *MF) {
            for(MachineInstr &mi : mbb) {
              if (!DT->dominates(entry, &mi) || !postDominates(exit, &mi, PDT))
                continue;

              if (mi.getOpcode() == CSA::CSA_PARALLEL_SECTION_ENTRY &&
                  isRegDerivedFromReg(&mi.getOperand(1), &regionTokenOp)) {
                parSectionInfo newSection;
                newSection.entry = &mi;
                newSection.sectionToken = mi.getOperand(0).getReg();

                bool foundExit = false;
                for (MachineBasicBlock& mb2 : *MF) {
                  for (MachineInstr& mi2 : mb2) {
                    if (mi2.getOpcode() == CSA::CSA_PARALLEL_SECTION_EXIT) {
                      if (DT->dominates(&mi, &mi2) && postDominates(&mi2, &mi, PDT)) {
                        if (isRegDerivedFromReg(&mi.getOperand(0), &mi2.getOperand(0))) {
                          newSection.exit = &mi2;
                          foundExit = true;
                          break;
                        }
                      }
                    }
                  }
                }
                if(foundExit) {
                  newRegion.sections.insert(newSection);
                } else {
                  DEBUG(errs() << "WARNING: couldn't find parallel section exit\n");
                }
              }
            }
          }

          parRegions.push_back(newRegion);
        }
        else {
          DEBUG(errs() << "WARNING: no single-entry/single-exit region for " << mi;);
        }
      }
    }
  }
}

void CSAMemopOrdering::eraseParallelIntrinsics(MachineFunction *MF){
  bool needDeadPHIRemoval = false;
  std::set<MachineInstr*> toErase;
  for(MachineBasicBlock &mbb : *MF)
    for(MachineInstr &mi : mbb)
      if (mi.getOpcode() == CSA::CSA_PARALLEL_SECTION_ENTRY ||
          mi.getOpcode() == CSA::CSA_PARALLEL_SECTION_EXIT  ||
          mi.getOpcode() == CSA::CSA_PARALLEL_REGION_ENTRY  ||
          mi.getOpcode() == CSA::CSA_PARALLEL_REGION_EXIT) {
        toErase.insert(&mi);
        // Any token users should also go away.
        for (MachineInstr &tokenUser :
            MRI->use_nodbg_instructions(mi.getOperand(0).getReg()))
          toErase.insert(&tokenUser);
      }

  for(MachineInstr* mi : toErase) {
    mi->eraseFromParentAndMarkDBGValuesForRemoval();
    needDeadPHIRemoval = true;
  }

  // We've removed all of the intrinsics, but their tokens may have been
  // flowing through PHI nodes. Look for dead PHI nodes and remove them.
  while (needDeadPHIRemoval) {
    needDeadPHIRemoval= false;
    toErase.clear();
    for(MachineBasicBlock &mbb: *MF)
      for(MachineInstr &mi : mbb)
        if (mi.isPHI() && mi.getOperand(0).isReg() &&
            MRI->use_nodbg_empty(mi.getOperand(0).getReg()))
          toErase.insert(&mi);
    for(MachineInstr *mi : toErase) {
      mi->eraseFromParentAndMarkDBGValuesForRemoval();
      needDeadPHIRemoval = true;
    }
  }
}

const MachineInstr* CSAMemopOrdering::isInstrPostDomByIntrinsic(MachineInstr* entryInst, unsigned op,
                                                unsigned token) const
{
  auto *cur = PDT->getNode(entryInst->getParent());
  assert(MRI->hasOneDef(token));
  MachineOperand& tokenOp = *MRI->def_begin(token);
  for( ; cur; cur = cur->getIDom()) {
    MachineBasicBlock *dom = cur->getBlock();

    for(MachineInstr &inst : *dom) {
      if (inst.getOpcode()==op && postDominates(&inst, entryInst, PDT)) {
        DEBUG(errs() << "Post-dom'ing inst is " << inst);
        assert(inst.getNumOperands()==1 && inst.getOperand(0).isReg());
        MachineOperand& candidateTokenOp = inst.getOperand(0);
        if (isRegDerivedFromReg(&candidateTokenOp, &tokenOp)) {
          DEBUG(errs() << "\t=> Yes! Post-dominator " << dom->getName() << " (BB "
              << dom->getNumber() << " says so. Token vreg=" << token << ".\n");
          return &inst;
        } else {
          DEBUG(errs() << "\t=> There's an intrinsic, but wrong token.\n");
        }
      }
    }
  }

  DEBUG(errs() << "\t<= No parallel annotation found.\n");
  return nullptr;
}

unsigned CSAMemopOrdering::isParallelRegion(MachineInstr* regionEntry, const MachineInstr** entry, const MachineInstr** exit) const
{
  unsigned regionToken;
  bool isEntry = regionEntry->getOpcode() == CSA::CSA_PARALLEL_REGION_ENTRY;
  regionToken = regionEntry->getOperand(0).getReg();
  DEBUG(errs() << "\tIs this a begin intrinsic? " <<
      (isEntry ?  "yes" : "no") << "\n");
  if (isEntry) {
    const MachineInstr* postDom = isInstrPostDomByIntrinsic(regionEntry, CSA::CSA_PARALLEL_REGION_EXIT, regionToken);
    DEBUG(errs() << "\tIs it post-dominated by an end intrinsic? " <<
        (postDom ? "yes" : "no") << "\n");
    if (postDom) {
      if(entry)
        *entry = regionEntry;
      if(exit)
        *exit = postDom;
      return regionToken;
    }
  }

  return 0;
}

std::pair<MachineOperand*,MachineOperand*>
CSAMemopOrdering::isCrossSectionDependence(parRegionInfo* region, MachineInstr* i1, MachineInstr* i2) {
  // All def -> use pairs to define a potential dependence "edge".
  MachineOperand *def = std::prev(std::end(i1->defs()));
  MachineOperand *use = std::prev(std::end(i2->uses()));

  SmallVector<MachineOperand*, MEMDEP_VEC_WIDTH> flow;
  // Is there a connecting edge?
  if (!isRegDerivedFromReg(use, def, false, &flow))
    // No edge. Uninteresting.
    return {nullptr,nullptr};

  // Consider pairs of instructions which are both in this region.
  const parSectionInfo* sA = region->containsInstr(i1, DT, PDT);
  const parSectionInfo* sB = region->containsInstr(i2, DT, PDT);
  if (!sA || !sB)
    // One or both of these is not in the parallel region.
    return {nullptr,nullptr};

  // Both are in a section in the region. (I.e., they're both in the same
  // or two different sections in the region.)
  //
  // Interesting edges cross a section boundary. This is obviously the
  // case if the two sections involved are different. It's also the case
  // if the two sections are actually the same section, but the edge is
  // going through instructions which are not in the section. (I.e., the
  // edge "leaves" the section and comes back in.) Walk the intermediate
  // users involved in the dataflow of the edge looking for ones which
  // cross from one section to another.

  MachineOperand *incoming = nullptr;
  MachineOperand *outgoing = nullptr;
  flow.push_back(use);
  for(MachineOperand* fuser : flow) {
    if (!outgoing && !sA->containsInstr(fuser->getParent(), DT, PDT)) {
      assert(MRI->hasOneDef(fuser->getReg()) && "cannot understand dataflow with multiple/no defs");
      outgoing = &*std::begin(MRI->def_operands(fuser->getReg()));
    }
    if (outgoing && sB->containsInstr(fuser->getParent(), DT, PDT)) {
      incoming = fuser;
      return {incoming,outgoing};
    }
  }

  assert(!incoming && !outgoing && "discovered section exit but not re-entry?");
  assert(sA==sB && "section-to-section edge must be a dependency");
  return {nullptr,nullptr};
}

void CSAMemopOrdering::relaxSectionOrderingEdges(parRegionInfo &parReg, MachineFunction *thisMF) {

  std::set< std::pair<unsigned, std::pair<MachineOperand*,MachineOperand*> > > edgesToRelax;

  DEBUG(errs() << "Looking for section-ordering edges in region " << *parReg.entry);

  // Iterate over all pairs of ordered instructions, identifying those which
  // implement ordering between sections in the same region.
  for(MachineInstr *iA : orderedMemops) {
    for(MachineInstr *iB : orderedMemops) {

      MachineOperand *inOp, *outOp;
      std::tie(inOp,outOp) = isCrossSectionDependence(&parReg, iA, iB);
      if (inOp) {
        assert(inOp && outOp && "Found only one of edge exiting/entering operands?");
        DEBUG(errs() << "\tSection-ordering edge from " << *iA <<
            "\t      to ---->    " << *iB);
        DEBUG(errs() << "\t edge section inOp: " << *inOp << "; outOp: " << *outOp << "\n");

        unsigned aliasSet = AS->getAliasSetNumForMemop(*iA->memoperands_begin());
        edgesToRelax.insert({aliasSet, {inOp, outOp}});
      }
    }
  }

  // For each alias set [chain] and each region, keep track of the
  // section-exiting edges which need to be "join"ed together via MERGEs.
  std::map<unsigned, std::set<unsigned> > sectionMergees;

  // Iterate over the edges and replace the edge source with one coming from
  // the ordering "fork" point.
  for (auto edge : edgesToRelax) {
    unsigned aliasSet = edge.first;
    MachineOperand *incoming, *outgoing;
    std::tie(incoming,outgoing) = edge.second;

    unsigned chV = parReg.aliasSetCaptures[aliasSet]->getOperand(0).getReg();
    unsigned incomingReg = incoming->getReg();

    // Replace the incoming value with a use of the value when crossing
    // the region entry intrinsic, as was determined by the appropriate
    // SSAUpdater.
    incoming->setReg(chV);

    // On the other end, the chain output should go somewhere, namely, to
    // a merge after the sections. (The end of the region.) Do this in a
    // separate step, since we'll want to merge all of the
    // section-exiting chain edges together at once. (We don't
    // necessarily know all of them yet.)
    sectionMergees[aliasSet].insert(outgoing->getReg());

    // Also arrange to reconnect the incoming edge to the implicit dataflow
    // graph by adding it to the region join merge.
    sectionMergees[aliasSet].insert(incomingReg);
  }

  // Finally, merge all of the outputs for each alias set. (This is the
  // "join".) The code generation here is a little more complicated because
  // the merge inputs may not dominate the merge point.
  for (const auto &m : sectionMergees) {
    unsigned aliasSet = m.first;
    const auto &waveSet = m.second;
    SmallVector<unsigned, MEMDEP_VEC_WIDTH> wave(waveSet.begin(), waveSet.end());

    // We will use these values repeatedly to set up MachineSSAUpdaters.
    unsigned chEntering = parReg.aliasSetCaptures[aliasSet]->getOperand(1).getReg();
    assert(parReg.entry && "missing region entry instruction");
    MachineBasicBlock* enteringBlock = const_cast<MachineBasicBlock*>(parReg.entry->getParent());

    // Generate a MERGE1 tree joining the parallel sections' chains back into
    // a a single chain for exiting the parallel region.
    MachineInstr* insertBefore = parReg.aliasSetReleases[aliasSet];
    assert(insertBefore && "missing region release" );
    // If we are not already merging the original outgoing edge, arrange for
    // it to be added to the merge tree tool. Otherwise, it may be left as a
    // disconnected LIC.
    unsigned outgoingChainVReg = insertBefore->getOperand(1).getReg();
    // Include the old passthrough value in the merge.
    if (!waveSet.count(outgoingChainVReg))
      wave.push_back(outgoingChainVReg);

    DEBUG(errs() << "Merging " << wave.size() << " for this alias set:\n");
    for (unsigned wE : wave) {
      DEBUG(errs() << "\t" << PrintReg(wE) << "\n");
      (void)wE;
    }
    SmallVector<MachineOperand*, MEMDEP_VEC_WIDTH> newOps;
    unsigned merged = merge_dependency_signals(*insertBefore->getParent(), insertBefore, &wave, chEntering, &newOps);
    MachineInstr *mergeDef = MRI->getUniqueVRegDef(merged);

    // Go back and fix up SSA form for the MERGE inputs, if necessary. This
    // looks nasty, but it basically amounts to doing a RewriteUse on each
    // logical MERGE input.
    for(MachineOperand* mop : newOps) {
      unsigned mergeIn = mop->getReg();
      MachineInstr *inDef = MRI->getUniqueVRegDef(mergeIn);
      assert(inDef && mergeDef);
      assert(waveSet.count(mergeIn) || mergeIn == outgoingChainVReg);
      if (!DT->dominates(inDef, mergeDef)) {
        MachineSSAUpdater mergeUpdater(*thisMF);
        mergeUpdater.Initialize(chEntering);
        mergeUpdater.AddAvailableValue(enteringBlock, chEntering);
        mergeUpdater.AddAvailableValue(inDef->getParent(), mergeIn);
        mergeUpdater.RewriteUse(*mop);
      }
    }

    // Splice the merged output back into the ordering chain after the
    // region. This is easy because of the passthrough MOVs.
    // We can safely make this passthrough MOV dead.
    parReg.aliasSetReleases[aliasSet]->getOperand(1).setReg(merged);
  }
}

void CSAMemopOrdering::relaxSectionOrderingEdges(MachineFunction *thisMF) {
  for (parRegionInfo &pl : parRegions) {
    relaxSectionOrderingEdges(pl, thisMF);
  }
}

bool CSAMemopOrdering::should_assign_ordering(const MachineInstr& MI) const {
  using namespace std;
  return (not MI.memoperands_empty() or MI.getOpcode() == CSA::MOV0)
    and begin(MI.defs()) != end(MI.defs()) and begin(MI.uses()) != end(MI.uses())
    and prev(end(MI.defs()))->isReg() and prev(end(MI.defs()))->getReg() == CSA::IGN
    and prev(end(MI.uses()))->isReg() and prev(end(MI.uses()))->getReg() == CSA::IGN;
}

void MachineAliasSetTracker::add(MachineMemOperand *mop) {
  if (isMerged)
    return;

  /* Handle the "normal" case where we have a Value by adding the value into
   * the real AliasSetTracker. */
  Value *v = const_cast<Value*>(mop->getValue());
  if (v) {
    AST.add(v, mop->getSize(), mop->getAAInfo());
    return;
  }

  /* Otherwise, we there is no Value, and the pointer is something like a frame
   * object. (This is the only case I've seen so far, but there are other types
   * of PseudoSourceValues.) */
  const PseudoSourceValue *pv = mop->getPseudoValue();

  /* Ask if the PseudoValue IS aliased with a Value. If it's a
   * FixedStackPseudoSourceValue, then this will consult MFI. If the answer is
   * "no", then we consider the pv to be in its own alias set and can avoid
   * giving up (by merging all of the alias sets). Note that we can't track
   * this with an actual AliasSet. "isAliased" reports whether any Values may
   * alias, so this also assumes that PseudoValues cannot alias one another. */
  if (pv && !pv->isAliased(MFI)) {
    DEBUG(errs() << "found a non-aliasing pv.\n");
    if (!pseudos[pv])
      pseudos[pv] = pseudosCounter++;
    return;
  }

  /* If we find a memop that has no Value and no PseudoValue, or if we find
   * that any PseudoValue is not in its own alias set, then we give up and
   * consider ourselves to only have one all-encompassing alias set. */
  DEBUG(errs() << "found a pv which may be aliased. smushing into one alias set.\n");
  isMerged = true;
}

unsigned MachineAliasSetTracker::getNumAliasSets(void) {
  if (isMerged)
    return 1;

  unsigned size = 0;
  for (const AliasSet &s : AST.getAliasSets()) {
    (void)s;
    size++;
  }
  size+=pseudos.size();

  return size;
}

unsigned MachineAliasSetTracker::getAliasSetNumForMemop(const MachineMemOperand *mop) {
  if (isMerged)
    return 0;

  Value *v = const_cast<Value*>(mop->getValue());
  if (mop->getValue()) {
    AliasSet *as = AST.getAliasSetForPointerIfExists(v, mop->getSize(), mop->getAAInfo());
    assert(as && "Memop must be added to MachineAliasSetTracker before querying");
    unsigned pos = 0;
    for (const AliasSet &s : AST.getAliasSets()) {
      if (&s == as)
        return pos;
      pos++;
    }
  }

  const PseudoSourceValue *pv = mop->getPseudoValue();
  unsigned pos = 0;
  for (const AliasSet &s : AST.getAliasSets()) {
    (void)s;
    pos++;
  }
  assert(pseudos[pv] && "Memop must be added to MachineAliasSetTracker before querying");
  return pos + pseudos[pv] - 1;
}

void MachineAliasSetTracker::print(raw_ostream &OS) const {
  if (isMerged) {
    OS << "[Merged]\n\n";
    return;
  }

  OS << "Non-aliasing PseudoValues: " << pseudosCounter-1 << "\n";
  OS << "Values in AliasSetTracker:\n";
  AST.print(OS);
}
