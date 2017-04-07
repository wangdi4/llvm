//===- CSAMemopOrdering.cpp - CSA Memory Operation Ordering ----*- C++ -*--===//
//
//===----------------------------------------------------------------------===//
//
// This file implements explicit ordering of memory operations in preparation
// for dataflow conversion. The purpose of this is to preserve the ordering
// expected by a sequential programmer in the absence of a program counter.
//
// This is a MachineFunctionPass which proceeds roughly as follows:
// 1. Build alias sets for all memory operations in the function.
// 2. For each alias set, establish a "chain" of vregs which will be connected
//    by ordered operations. Each vreg in a chain represents a new ordering
//    state for that alias set.
// 3. For every instruction, determine its effect on our idea of the current
//    ordering state for that alias set:
//    a) If it has no effect, nothing needs to be done.
//    b) If it merely "uses" the previous ordering state, alter the instruction
//       to consume the latest value off of the dependence chain.
//    c) If it creates a new ordering state, it is made to depend on all users
//       of the previous state, and a new value is created on the dependence
//       chain. (Jim Sukha called the set of users a "wavefront", and I've kept
//       this terminology for now.)
// 4. Finally, the last vreg on each chain is given a use at every exit point
//    in the function, so that the function cannot return until all ordered ops
//    have executed.
//
//===----------------------------------------------------------------------===//

#include "CSA.h"
#include "CSATargetMachine.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Analysis/AliasSetTracker.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/MachineSSAUpdater.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "MachineCDG.h"

using namespace llvm;

// If this is set to 0, we disable memory ordering entirely.
static cl::opt<int>
OrderMemops("csa-order-memops",
            cl::Hidden,
            cl::desc("CSA Specific: Disable ordering of memory operations (by setting to 0)"),
            cl::init(true));

// These values are used for tuning LLVM datastructures; correctness is not at
// stake if they are off.
// Width of vectors we are using for memory op calculations.
#define MEMDEP_VEC_WIDTH 8
// A guess at the number of memops per alias set per function.
#define MEMDEP_OPS_PER_SET 32
// A guess at the number of phi nodes needed for SSA rewriting.
#define MEMDEP_PHIS 4

#define DEBUG_TYPE "csa-memop-ordering"

// TODO: memop ordering statistics?
//STATISTIC(NumInlineAsmExpansions, "Number of asm()s expanded into MIs");
//STATISTIC(NumInlineAsmInstrs,     "Number of machine instructions resulting from asm()s");

namespace {

  class CSAMemopOrdering : public MachineFunctionPass {
    bool runOnMachineFunction(MachineFunction &MF) override;
    const TargetMachine *TM;
    const CSASubtarget *STI;
    const MCInstrInfo  *MII;
    const MachineLoopInfo *MLI;

  public:
    static char ID;
    CSAMemopOrdering() : MachineFunctionPass(ID) { }
    StringRef getPassName() const override {
      return "CSA Memory Operation Ordering";
    }

    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.addRequired<ControlDependenceGraph>();
      AU.addRequired<AAResultsWrapperPass>();
      AU.addRequired<MachineLoopInfo>();
      AU.addRequired<MachineDominatorTree>();
      AU.addRequired<MachinePostDominatorTree>();
      AU.setPreservesAll();
      MachineFunctionPass::getAnalysisUsage(AU);
    }

  private:
    AliasAnalysis *AA;
    AliasSetTracker *AS;
    MachineBasicBlock *entryBB;
    ControlDependenceGraph *CDG;
    MachinePostDominatorTree *PDT;
    MachineDominatorTree *DT;
    const CSAInstrInfo *TII;
    MachineRegisterInfo *MRI;

    struct Depchain;
    struct WavefrontOp {
      unsigned start;
      std::unique_ptr<MachineSSAUpdater> updater;
      std::unique_ptr<SmallVector<MachineInstr*, MEMDEP_PHIS> > phis;
      MachineInstr *instr;
      Depchain *ch;
      bool updated;

      void updateWaveOp(MachineOperand *op, MachineDominatorTree *DT);
    };
    typedef DenseMap<unsigned, struct WavefrontOp> Wavefront;

    // For each chain, maintain 4 items:
    // start:    the first vreg in its chain
    // updater:  a MachineSSAUpdater
    // uses:     a collection of all of its uses
    // wave:     a collection of ops which together constitute the next chain value
    struct Depchain {
      unsigned start;
      std::unique_ptr<MachineSSAUpdater> updater;
      SmallPtrSet<MachineOperand*, MEMDEP_OPS_PER_SET> uses;
      Wavefront wave;

      // Codegen helper for starting the chain off
      void emitStart(MachineBasicBlock *BB, const CSAInstrInfo* TII,
          MachineRegisterInfo *MRI, SmallVectorImpl< MachineInstr * > *phis) {
        const unsigned movToken = TII->getMemTokenMOVOpcode();
        start = MRI->createVirtualRegister(TII->getMemTokenRC());
        BuildMI(*BB, BB->getFirstNonPHI(), DebugLoc(),
            TII->get(movToken), start).addImm(1);
        updater.reset(new MachineSSAUpdater(*BB->getParent(), phis));
        updater->Initialize(start);
        updater->AddAvailableValue(BB, start);
      }

      // Codegen helper for finishing the chain
      void emitEnd(MachineBasicBlock *BB, unsigned finalValue, const
          CSAInstrInfo* TII, MachineRegisterInfo *MRI) {
        // This register will be dead a soon as we write to it. It's an "RI1"
        // register, which means it must be on the SXU.
        unsigned outRegister = MRI->createVirtualRegister(&CSA::RI1RegClass);
        const unsigned movToken = TII->getMemTokenMOVOpcode();
        MachineInstr* endMov = BuildMI(*BB, BB->getFirstTerminator(), DebugLoc(), TII->get(movToken), outRegister).addReg(finalValue);
        if (finalValue == start)
          uses.insert(endMov->operands_end()-1);
      }
    };

    typedef DenseMap<const AliasSet*, Depchain> AliasSetDepchain;
    typedef DenseMap<const AliasSet*, unsigned> AliasSetVReg;

    // Establish types of ordering effects that an instruction may have.
    enum OrderingEffect {
      NoEffect,     // This instruction has no ordering relationship.
      UsesState,    // This instruction uses the current ordering state.
      CreatesState, // This instruction consumes the current state and creates
                    // a new current ordering state.
    };

    void generateBlockConstraints(MachineBasicBlock& BB, AliasSetDepchain& depchains);
    OrderingEffect determineOrderingEffect(MachineInstr &MI);

    // Helper methods:

    // Create a new OLD/OST/ATM* instruction, to replace an existing LD/ST/ATM*
    // instruction.
    //  issued_reg is the register to define as the extra output
    //  ready_reg is the register which is the extra input
    //  ch is the ordering chain identified for this instruction
    MachineInstr* emitOrderedInstr(MachineInstr* memop,
                                    unsigned issued_reg,
                                    unsigned ready_reg,
                                    Depchain *ch);

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
    unsigned mergeWavefront(MachineBasicBlock& BB,
                                      MachineInstr* MI,
                                      Wavefront* current_wavefront,
                                      unsigned input_mem_reg);

    // Return true if the specified loop has been annotated as parallel in the
    // source code.
    bool isParallelLoop(MachineLoop* loop) const;

    // Return true if the specified basic block contains a .csa_parallel_loop
    // pseudo-op.
    bool findParallelLoopPseudoOp(const MachineBasicBlock& BB) const;

    // Traverse the PHI nodes that were inserted by the ssa updater.
    // For PHI nodes that belong to loops that have been annotated as
    // parallel, mark the incoming PHI edges that represent memory-order backedges
    // by inserting a CSA_PARALLEL_MEMDEP psuedo-op between the definition of
    // the edge and the PHI.
    void markParallelLoopBackedges(MachineFunction *thisMF,
                                   const SmallVectorImpl<MachineInstr*>& inserted_PHIs);
  };
}

char CSAMemopOrdering::ID = 0;

MachineFunctionPass *llvm::createCSAMemopOrderingPass() {
  return new CSAMemopOrdering();
}

bool CSAMemopOrdering::runOnMachineFunction(MachineFunction &MF) {

  if (!OrderMemops) {
    return false;
  }

  TII = static_cast<const CSAInstrInfo*>(MF.getSubtarget().getInstrInfo());
  MRI = &MF.getRegInfo();

  AA = &getAnalysis<AAResultsWrapperPass>().getAAResults();
  CDG = &getAnalysis<ControlDependenceGraph>();
  DT = &getAnalysis<MachineDominatorTree>();
  PDT = &getAnalysis<MachinePostDominatorTree>();

  MLI = &getAnalysis<MachineLoopInfo>();

  // Find the entry block. (Surely there's an easier way to do this?)
  ControlDependenceNode *rootN = CDG->getRoot();
  if(!(rootN && *rootN->begin())) return false;
  entryBB = (*rootN->begin())->getBlock();
  assert(entryBB && "Couldn't determine this function's entry block");

  // Create the AliasSetTracker and populate with all basic blocks.
  AliasSetTracker AST(*AA);
  AS = &AST;
  for (MachineBasicBlock &MB : MF) {
    for (MachineInstr &MI : MB) {
      for (MachineMemOperand *op : MI.memoperands()) {
        assert(op->getValue() && "I don't understand this memory operand!");
        AS->add(const_cast<Value*>(op->getValue()),
            op->getSize(), op->getAAInfo());
      }
    }
  }
  DEBUG(errs() << "AliasSets for function " << MF.getName() << ":\n");
  DEBUG(AS->dump());

  AliasSetDepchain depchains;
  SmallVector<MachineInstr*, 16> inserted_PHIs;  // Phi nodes from ssa updaters

  // Create the start of the chain for each alias set.
  for (const AliasSet &set : AS->getAliasSets())
    depchains[&set].emitStart(entryBB, TII, MRI, &inserted_PHIs);

  // Visit basic blocks after their children in the post-dominance tree,
  // emitting memop dependencies for all alias sets at once.
  typedef po_iterator<MachinePostDominatorTree*> ppi;
  for (ppi DTN = ppi::begin(PDT), END = ppi::end(PDT); DTN != END; ++DTN) {
    MachineBasicBlock *BB = DTN->getBlock();
    assert(BB && "post-dominance tree has a node with null BasicBlocK?");
    generateBlockConstraints(*BB, depchains);
  }

  // Finally, use the updater for each set to fully rewrite to SSA. This
  // includes generating PHI nodes.
  for (const AliasSet &set : AS->getAliasSets()) {
    Depchain *ch = &depchains[&set];
    for (MachineOperand *op : ch->uses) {
      // There is an exception here: RewriteUse is not smart enough to find new
      // values added by "AddAvailableValue" before a use in the same basic
      // block. (I.e., it can only find them if they are in a basic block which
      // is a strict dominator.) This is only an issue when the use is in the
      // function's entry block. Fortunately, in this case, we can be sure that
      // depchain_reg contained the right value to use.
      if(op->getParent()->getParent() == entryBB)
        continue;

      ch->updater->RewriteUse(*op);
    }
  }

  // Traverse the PHI nodes that were inserted by the ssa updater.
  // For PHI nodes that belong to loops that have been annotated as
  // parallel, mark the incoming PHI edges that represent memory-order backedges
  // by inserting a CSA_PARALLEL_MEMDEP psuedo-op between the definition of
  // the edge and the PHI.
  markParallelLoopBackedges(&MF, inserted_PHIs);

  return true;
}

void CSAMemopOrdering::WavefrontOp::updateWaveOp(MachineOperand *op, MachineDominatorTree *DT) {
  assert(op->isReg());

  // If the use dominates the def then this is the easy case: nothing needs to
  // be done.
  if (DT->dominates(instr, op->getParent())) {
    updated = true;
    return;
  }

  // Otherwise we need to account for the fact that we want either the
  // wavefront value OR some predecessor in the main dependence chain,
  // depending on control flow. (Note that such a predecessor always exists
  // because the chain starts in the entry basic block.)
  updater->RewriteUse(*op);

  if (op->getReg() == ch->start ||
      op->getReg() == start)
    ch->uses.insert(op);

  // Search the inserted PHIs for chain values which need updating.
  for (MachineInstr *phi : *phis) {
    for (MachineOperand &phiOp : phi->operands()) {
      if (phiOp.isReg()) {
        unsigned phiReg = phiOp.getReg();
        if (phiReg == ch->start || phiReg == start) {
          ch->uses.insert(&phiOp);
        }
      }
    }
  }

  updated = true;
}

// For now the strategy is very simple: non-memory ops have no effect. Loads
// consume the previous state, but do not establish a new ordering point.
// Stores consume the previous state and create a new ordering point.
CSAMemopOrdering::OrderingEffect CSAMemopOrdering::determineOrderingEffect(MachineInstr &MI) {
  unsigned opcode = MI.getOpcode();
  unsigned convertedOpcode = TII->get_ordered_opcode_for_LDST(opcode);
  if (opcode == convertedOpcode)
    return NoEffect;

  if (TII->isLoad(&MI))
    return UsesState;

  return CreatesState;
}

void CSAMemopOrdering::generateBlockConstraints(MachineBasicBlock& BB, AliasSetDepchain& depchains) {

  // Track the vreg of the token through this block. Initially it's the vreg
  // that the SSAUpdater is tracking, but may get updated to a redefinition
  // in this block.
  AliasSetVReg latestInBlock;
  for (const AliasSet &set : AS->getAliasSets())
    latestInBlock[&set] = depchains[&set].start;

  MachineBasicBlock::iterator mi_it = BB.begin();
  while (mi_it != BB.end()) {
    MachineInstr &MI = *mi_it;

    OrderingEffect effect = determineOrderingEffect(MI);

    // If this instruction is not ordered, we're done.
    if (effect == NoEffect) {
      ++mi_it;
      continue;
    }

    const MachineMemOperand *mOp = *MI.memoperands_begin();

    // Use AliasAnalysis to determine which ordering chain we should be on.
    AliasSet *as =
      AS->getAliasSetForPointerIfExists(const_cast<Value*>(mOp->getValue()),
          mOp->getSize(), mOp->getAAInfo());
    Depchain *ch = &depchains[as];

    if (effect == UsesState) {
      //We must wait on the dep chain, but we don't directly create a new value
      //with our own output; instead, we build up a wave of ops which depend on
      //the previous value. The outputs of all of these will be merged by the
      //next op which establishes new ordering state, creating the next chain
      //value from the merge result. So for now, all we need to do is add our
      //output to the wavefront.
      unsigned issued = MRI->createVirtualRegister(TII->getMemTokenRC());
      unsigned ready = latestInBlock[as];

      // Create an SSA updater for this op's output and describe its initial
      // evolution. The important part here is that we initialize the SSA
      // updater with the op's INPUT, so that if this op doesn't dominate a
      // use, this input will be used instead of the output.
      SmallVector<MachineInstr*, MEMDEP_PHIS> *insertedPhis;
      insertedPhis = new SmallVector<MachineInstr*, MEMDEP_PHIS>();
      MachineSSAUpdater *updater = new MachineSSAUpdater(*BB.getParent(), insertedPhis);
      updater->Initialize(ch->start);
      updater->AddAvailableValue(entryBB, ch->start);
      updater->AddAvailableValue(&BB, issued);

      // Add to the wavefront, passing the updater along.
      ch->wave[issued] = {
        ready,
        std::unique_ptr<MachineSSAUpdater>(updater),
        std::unique_ptr<SmallVector<MachineInstr*, MEMDEP_PHIS> >(insertedPhis),
        emitOrderedInstr(&MI, issued, latestInBlock[as], ch),
        ch,
        false,
      };
    } else {
      // Otherwise we need to merge the wavefront and establish a new chain
      // value. Do the merge. This may be effectively a no-op.
      latestInBlock[as] = mergeWavefront(BB, &MI, &ch->wave, latestInBlock[as]);
      ch->updater->AddAvailableValue(&BB, latestInBlock[as]);

      unsigned issued = MRI->createVirtualRegister(TII->getMemTokenRC());
      emitOrderedInstr(&MI, issued, latestInBlock[as], ch);
      ch->updater->AddAvailableValue(&BB, issued);
      latestInBlock[as] = issued;
    }

    // Finally, remove the old (unordered) duplicate instruction.
    mi_it = BB.erase(mi_it);
  }

  // Create a mov to consume the end of each chain. We'll need one in each
  // terminating basic block. (We are still thinking control flow here.) Note
  // that using RI1 register class should keep this on the SXU.  Even though
  // we allocate a separate virtual register for each one, LLVM in the end is
  // free to re-use the same physical register since the values are dead
  // after each def.
  if (BB.isReturnBlock()) {
    for (const AliasSet &set : AS->getAliasSets()) {
      Depchain *ch = &depchains[&set];

      // Do another merge. This may be effectively a no-op.
      latestInBlock[&set] = mergeWavefront(BB, NULL, &ch->wave, latestInBlock[&set]);
      ch->updater->AddAvailableValue(&BB, latestInBlock[&set]);

      // Emit code at the end of the block to consume the chain, making the
      // exit of the function wait for it.
      ch->emitEnd(&BB, latestInBlock[&set], TII, MRI);
    }
  }
}

unsigned CSAMemopOrdering::mergeWavefront(MachineBasicBlock& BB,
                                                  MachineInstr* MI,
                                                  Wavefront* wavefront,
                                                  unsigned input_mem_reg) {

  SmallVector<unsigned, MEMDEP_VEC_WIDTH> current_wavefront;
  for (auto &pair : *wavefront) {
    current_wavefront.push_back(pair.first);
  }

  if (current_wavefront.size() > 0) {
    DEBUG(errs() << "Merging dependency signals from " << current_wavefront.size() << " register " << "\n");

    // BFS-like algorithm for merging the registers together.
    // Merge consecutive pairs of dependency signals together,
    // and push the output into "next_level".
    SmallVector<unsigned, MEMDEP_VEC_WIDTH> tmp_buffer;
    SmallVector<unsigned, MEMDEP_VEC_WIDTH>* current_level;
    SmallVector<unsigned, MEMDEP_VEC_WIDTH>* next_level;

    current_level = &current_wavefront;
    next_level = &tmp_buffer;

    while (current_level->size() > 1) {
      assert(next_level->size() == 0);
      for (unsigned i = 0; i < current_level->size(); i+=2) {
        // Merge current_level[i] and current_level[i+1] into
        // next_level[i/2]
        if ((i+1) < current_level->size()) {

          // Even case: we have a pair to merge.  Create a virtual
          // register + instruction to do the merge.
          unsigned next_out_reg = MRI->createVirtualRegister(TII->getMemTokenRC());

          // We have vregs we're using in the op's block, but their defs may
          // not dominate the merge. Get some help from MachineSSAUpdater.
          unsigned in1, in2;
          in1 = (*current_level)[i];
          in2 = (*current_level)[i+1];

          MachineInstr* new_inst;
          // Create the merge instruction, paying special attention to MI. If
          // MI is NULL, then this means that we're doing an end-of-block
          // merge.
          new_inst = BuildMI(MI ? *MI->getParent() : BB,
                             MI ? MI : BB.getFirstTerminator(),
                             MI ? MI->getDebugLoc() : DebugLoc(),
                             TII->get(CSA::MERGE1),
                             next_out_reg).addImm(0).addReg(in1).addReg(in2);

          if (wavefront->count(in1)) {
            struct WavefrontOp *l1 = &(*wavefront)[in1];
            MachineOperand *op1 = new_inst->operands_end()-2;
            l1->updateWaveOp(op1, DT);
          }

          if (wavefront->count(in2)) {
            struct WavefrontOp *l2 = &(*wavefront)[in2];
            MachineOperand *op2 = new_inst->operands_end()-1;
            l2->updateWaveOp(op2, DT);
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

    unsigned merged_reg = (*current_level)[0];
    if (wavefront->count(merged_reg) > 0) {
      struct WavefrontOp *l = &(*wavefront)[merged_reg];
      if (!l->updated) {
        // Our input was just a single memop output which potentially still
        // needs SSA updating. Create a use here, in this block, in order to
        // use the rewriter. In terms of ordering, there's no cost to this, but
        // we're emitting a useless "MOV". Hopefully RedundantMovElim will take
        // care of it.
        MachineInstr *new_inst;
        unsigned new_merged = MRI->createVirtualRegister(TII->getMemTokenRC());
        unsigned copyOpCode = TII->getMoveOpcode(TII->getMemTokenRC());

        // Create the copy instruction, paying special attention to MI. If MI
        // is NULL, then this means that we're doing an end-of-block merge.
        new_inst = BuildMI(MI ? *MI->getParent() : BB,
                           MI ? MI : BB.getFirstTerminator(),
                           MI ? MI->getDebugLoc() : DebugLoc(),
                           TII->get(copyOpCode),
                           new_merged).addReg(merged_reg);

        MachineOperand *op = new_inst->operands_end()-1;
        l->updateWaveOp(op, DT);
        merged_reg = new_merged;
      }

    }

    // Clear both vectors, just to be certain.
    current_level->clear();
    next_level->clear();
    // Also clear the wavefront.
    wavefront->clear();

    return merged_reg;
  }
  else {
    return input_mem_reg;
  }
}

MachineInstr* CSAMemopOrdering::emitOrderedInstr(MachineInstr* MI,
                                                 unsigned issued_reg,
                                                 unsigned ready_reg,
                                                 Depchain *ch) {
  MachineInstr* new_inst = NULL;
  DEBUG(errs() << "We want convert this instruction.\n");
  for (unsigned i = 0; i < MI->getNumOperands(); ++i) {
    MachineOperand& MO = MI->getOperand(i);
    DEBUG(errs() << "  Operand " << i << ": " << MO << "\n");
  }

  unsigned opcode = MI->getOpcode();
  unsigned new_opcode = TII->get_ordered_opcode_for_LDST(opcode);
  assert(opcode != new_opcode && "I don't know how to convert this instruction");

  // Alternative implementation would be:
  //  1. Build an "copy" of the existing instruction,
  //  2. Remove the operands from the clonsed instruction.
  //  3. Add new ones, in the right order.
  //
  // This operation doesn't work, because the cloned instruction gets created
  // with too few operands.
  //
  // MachineInstr* new_inst = thisMF->CloneMachineInstr(MI);
  // BB->insert(iterMI, new_inst);
  // new_inst->setDesc(TII->get(new_opcode));
  // int k = MI->getNumOperands() - 1;
  // while (k >= 0) {
  //   new_inst->RemoveOperand(k);
  //   k--;
  // }
  new_inst = BuildMI(*MI->getParent(),
                     MI,
                     MI->getDebugLoc(),
                     TII->get(new_opcode));

  unsigned opidx = 0;
  // Create dummy operands for this instruction.
  MachineOperand issued_op = MachineOperand::CreateReg(issued_reg, true);
  MachineOperand ready_op = MachineOperand::CreateReg(ready_reg, false);


  // Figure out how many "def" operands we have in this instruction.
  // This code assumes that normal loads have exactly one definition,
  // and normal stores have no definitions.
  unsigned expected_def_operands = 0;
  if (TII->isLoad(MI)) {
    expected_def_operands = 1;
  } else if (TII->isStore(MI)) {
    expected_def_operands = 0;
  } else if (TII->isAtomic(MI)) {
    expected_def_operands = 1;
  }
  else {
    assert(false && "Converting unknown type of instruction to ordered memory op");
  }

  // We should have at least as many definitions as expected operands.
  assert(MI->getNumOperands() >= expected_def_operands);

  // 1. Add all the defs to the new instruction first.
  while(opidx < expected_def_operands) {
    MachineOperand& MO = MI->getOperand(opidx);
    // Sanity-check: if we have registers operands, then they had
    // better be definitions.
    if (MO.isReg()) {
      assert(MO.isDef());
    }
    new_inst->addOperand(MO);
    opidx++;
  }

  // 2. Add issued flag.
  new_inst->addOperand(issued_op);
  // Then add the remaining operands.
  while (opidx < MI->getNumOperands()) {
    MachineOperand& MO = MI->getOperand(opidx);
    // In the remaining operands, there should not be any register
    // definitions.
    if (MO.isReg()) {
      assert(!MO.isDef());
    }
    new_inst->addOperand(MO);
    opidx++;
  }
  // 3. Finally, add the ready flag...
  new_inst->addOperand(ready_op);

  // ...and note the use of the ready flag for the SSA updater if this was an
  // out-of-block chain use.
  if (ready_op.getReg() == ch->start)
    ch->uses.insert(new_inst->operands_end()-1);

  // 4. Now copy over remaining state in MI:
  //      Flags
  //      MemRefs.
  //
  // Ideally, we'd be able to just call this function instead,
  // but with a different opcode that reserves more space for
  // operands.
  //   MachineInstr(MachineFunction &, const MachineInstr &);
  new_inst->setFlags(MI->getFlags());
  new_inst->setMemRefs(MI->memoperands_begin(),
                       MI->memoperands_end());

  DEBUG(errs() << "   Convert to ins: " << *new_inst << "\n");

  for (unsigned i = 0; i < new_inst->getNumOperands(); ++i) {
    MachineOperand& MO = new_inst->getOperand(i);
    DEBUG(errs() << "  Operand " << i << ": " << MO << "\n");
  }

  DEBUG(errs() << "   Original ins modified: " << *MI << "\n");

  return new_inst;
}

bool CSAMemopOrdering::isParallelLoop(MachineLoop* loop) const
{
  // Look for a ".csa_parallel_loop" pseudo-opcode in the block immediately
  // before the loop.  Note that the pseudo-opcode is not *within* the loop
  // (unless it is referring to a nested loop).

  // If the loop has a preheader, then looptop is that preheader; otherwise
  // looptop is the regular loop header.
  MachineBasicBlock* looptop = loop->getLoopPreheader();
  if (looptop) {
    // There is a preheader at the top of the loop. The preheader is not
    // technically part of the loop, so it might contain the parallel pseudo
    // op. If so, then this is a parallel loop and we're done. Most of the
    // time, however, the pseudo-op (if any) is in the predecessor of the
    // preheader, so we have to keep going.
    if (findParallelLoopPseudoOp(*looptop)) {
      DEBUG(errs() << "%%% Loop is parallel\n");
      return true;
    }
  }
  else {
    // There is a no preheader at the top of the loop; use the header of the
    // loop as the top.  Since the header is part of the loop (unlike the
    // preheader), we do not check the header for a parallel pseudo op.
    looptop = loop->getHeader();
  }

  // If ALL predicessors to the loop contain the parallel pseudo-op, then
  // this is a parallel loop.  If NONE of the predecessors contain the
  // psuedo op, then this is not a parallel loop.  If SOME BUT NOT ALL
  // predecessors contain the pseudo op, then our algorithm is broken.
  int numPredecessors = 0;
  int numPseudoOps = 0;
  for (MachineBasicBlock* loopPredecessor : looptop->predecessors()) {
    ++numPredecessors;
    if (findParallelLoopPseudoOp(*loopPredecessor))
      ++numPseudoOps;
  }
  assert((0 == numPseudoOps || numPredecessors == numPseudoOps) &&
         "Expected all-or-none of the predecessors to have pseuodo-op");

  // Return true if all of the predicessors contain the parallel pseudo-op.
  return (numPseudoOps > 0);
}

bool CSAMemopOrdering::findParallelLoopPseudoOp(const MachineBasicBlock& BB) const {
  // Return true if the specified basic block contains a .csa_parallel_loop pseudo-op.

  // Find a machine instruction with opcode CSA::CSA_PARALLEL_LOOP in this
  // basic block.
  auto MIiter = std::find_if(BB.begin(), BB.end(),
                             [](const MachineInstr& MI) {
                               return MI.getOpcode() == CSA::CSA_PARALLEL_LOOP;
                             });

  if (MIiter != BB.end()) {
    // We may eventually want to remove the pseudo-op once we've processed it.
    // BB.erase(MIiter);
    return true;  // Found the parallel pseudo-op
  }

  return false; // Didn't find the parallel pseudo-op
}

void CSAMemopOrdering::markParallelLoopBackedges(MachineFunction *thisMF,
                                                 const SmallVectorImpl<MachineInstr*>& inserted_PHIs)
{
  DEBUG(errs() << "%%% Before markParallelLoopBackedges\n");
  DEBUG(thisMF->dump());

  // Loop through the newly inserted PHI nodes, looking for the ones that are
  // in the header of a parallel loop.
  DEBUG(errs() << "%%% Inserted PHIs\n");
  for (MachineInstr* PHI : inserted_PHIs) {
    DEBUG(errs() << "    " << *PHI);

    // Get the parallel loop in which the PHI was defined, if any
    const MachineBasicBlock* BB = PHI->getParent();
    MachineLoop* phiLoop = MLI->getLoopFor(BB);
    if (! phiLoop) {
      DEBUG(errs() << "        Not in a loop\n");
      continue;  // Ignore PHIs not in a loop
    }
    else if (phiLoop->getHeader() != PHI->getParent()) {
      DEBUG(errs() << "        In a loop, but not in loop header block\n");
      continue;
    }
    else if (! isParallelLoop(phiLoop)) {
      DEBUG(errs() << "        In serial " << *phiLoop);
      continue;
    }
    else {
      DEBUG(errs() << "        In parallel " << *phiLoop);
    }

    // A PHI machine instruction has 5 or more arguments as follows:
    //  Operand 0: output
    //  Operand 1: input register 1
    //  Operand 2: Last basic block on  control flow breanch for register 1 definition
    //  Operand 3: second input register
    //  Operand 2: Last basic block on  control flow breanch for register 2 definition
    //  ...
    //  Operand 2*N-1: input register N
    //  Operand 2*N:   Last basic block on  control flow breanch for register N definition

    // We are interested in edges that come from the same loop as the PHI. Those edges are the back
    // edges that we want to label.  Because these PHIs were inserted as a result of memory
    // ordering, it no more than one edge should come from the same loop as the PHI; if more than
    // one edge *does* come from the same loop as the PHI, we'll ignore this PHI, for now.
    MachineOperand*    backedgeOperand = nullptr;
    auto operandIter = PHI->operands_begin();
    auto operandEnd  = PHI->operands_end();
    ++operandIter;   // Skip output operand
    while (operandIter != operandEnd) {
      MachineOperand& regOperand = *operandIter++;
      assert(regOperand.isReg() && regOperand.getReg());

      MachineOperand& bbOperand  = *operandIter++;
      assert(bbOperand.isMBB());

      MachineLoop* fromLoop = MLI->getLoopFor(bbOperand.getMBB());

      if (fromLoop == phiLoop) {
        if (backedgeOperand != nullptr) {
          DEBUG(errs() <<
                "%%% Ignored PHI where more than one input is from the same loop as the PHI: " <<
                *PHI);
          continue;
        }

        backedgeOperand = &regOperand;
      }
    }

    if (backedgeOperand == nullptr)
      continue;  // No back edges from same loop were detected

    // Found a back edge from same loop as the PHI. Get register
    unsigned backedgeReg = backedgeOperand->getReg();

    // Mark the back edge by inserting a .csa_parallel_memdep psuedo-op before it.
    // The new instruction is inserted on the output of the operation that originally defined
    // backedgeReg; consumers of backedgeReg are not changed. The new instruction is inserted into
    // the same BB as the operation that originally defined it, so that the SSA form does not need
    // to be adjusted.
    assert(MRI->hasOneDef(backedgeReg));
    MachineOperand&    backedgeDef      = *MRI->def_begin(backedgeReg);
    MachineInstr*      backedgeDefInstr = backedgeDef.getParent();
    MachineBasicBlock* backedgeDefBB    = backedgeDefInstr->getParent();
    unsigned           newMemdepReg     = MRI->createVirtualRegister(TII->getMemTokenRC());
    MRI->def_begin(backedgeReg)->ChangeToRegister(newMemdepReg, true); // Replace definition
    BuildMI(*backedgeDefBB, backedgeDefBB->getFirstTerminator(), DebugLoc(),
            TII->get(CSA::CSA_PARALLEL_MEMDEP), backedgeReg).addReg(newMemdepReg);

    // Sanity check: all channels are 0 or 1 bit wide
  } // end for each PHI

  DEBUG(errs() << "%%% After markParallelLoopBackedges\n");
  DEBUG(thisMF->dump());
}
