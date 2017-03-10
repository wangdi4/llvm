//===- CSAMemopOrdering.cpp - CSA Memory Operation Ordering ----*- C++ -*--===//
//
//===----------------------------------------------------------------------===//
//
// This file implements an interesting feature of the CSA Target: expansion of
// INLINEASM MachineInstrs into functional MachineInstrs.
//
//===----------------------------------------------------------------------===//

#include "CSA.h"
#include "CSATargetMachine.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/DenseMap.h"
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
            cl::Hidden,
            cl::desc("CSA Specific: Disable ordering of memory operations (by setting to 0)"),
            cl::init(1));

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
      AU.setPreservesAll();
      MachineFunctionPass::getAnalysisUsage(AU);
    }

  private:
    AliasAnalysis *AA;
    AliasSetTracker *AS;
    MachineBasicBlock *entryBB;
    ControlDependenceGraph *CDG;
    const CSAInstrInfo *TII;
    MachineRegisterInfo *MRI;

    typedef DenseMap<const AliasSet*, unsigned> AliasSetVReg;
    typedef DenseMap<const AliasSet*, std::unique_ptr<MachineSSAUpdater> > AliasSetUpdater;
    typedef DenseMap<const AliasSet*, SmallPtrSet<MachineOperand*, MEMDEP_OPS_PER_SET> > AliasSetUses;

    void addMemoryOrderingConstraints(MachineFunction* thisMF);

    // Helper methods:

    // Create a new OLD/OST instruction, to replace an existing LD /
    // ST instruction.
    //  issued_reg is the register to define as the extra output
    //  ready_reg is the register which is the extra input
    //  If non-NULL, ready_op_num will have the operand number which uses the
    //  ready_reg stored to it.
    MachineInstr* convert_memop_ins(MachineInstr* memop,
                                    unsigned new_opcode,
                                    unsigned issued_reg,
                                    unsigned ready_reg,
                                    unsigned *ready_op_num);

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
                                         AliasSetUpdater& depchain_updater,
                                         AliasSetUses& depchain_uses,
                                         AliasSetVReg& depchain_start);

    // Wavefront version.   Same conceptual functionality as linear version,
    // but more optimized.
    //
    // Only serializes stores in a block, but allows loads to occur in
    // parallel between stores.
    void convert_block_memops_wavefront(MachineBasicBlock& BB,
                                         AliasSetUpdater& depchain_updater,
                                         AliasSetUses& depchain_uses,
                                         AliasSetVReg& depchain_start);

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
                                      unsigned input_mem_reg);
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
        AS->add(const_cast<Value*>(op->getValue()),
            op->getSize(), op->getAAInfo());
      }
    }
  }
  DEBUG(errs() << "AliasSets for function " << MF.getName() << ":\n");
  DEBUG(AS->dump());

  // This step should run before the main dataflow conversion because
  // it introduces extra dependencies through virtual registers than
  // the dataflow conversion must also deal with.
  addMemoryOrderingConstraints(&MF);

  return true;
}

void CSAMemopOrdering::addMemoryOrderingConstraints(MachineFunction *thisMF) {

  const unsigned MemTokenMOVOpcode = TII->getMemTokenMOVOpcode();

  // For each alias set, maintain 3 items:
  // depchain_start:   a map from the set to the first vreg in its chain
  // depchain_updater: a map from the set to a MachineSSAUpdater
  // depchain_uses:    a map from the set to a collection of all of its uses
  AliasSetVReg    depchain_start;
  AliasSetUpdater depchain_updater;
  AliasSetUses    depchain_uses;

  // Initialize the above maps.
  for (const AliasSet &set : AS->getAliasSets()) {
    // Create the start of the chain for each alias set.
    depchain_start[&set] = MRI->createVirtualRegister(MemopRC);
    BuildMI(*entryBB,
        entryBB->getFirstNonPHI(),
        DebugLoc(),
        TII->get(MemTokenMOVOpcode),
        depchain_start[&set]).addImm(1);
    // Initialize an SSAUpdater for each set.
    depchain_updater[&set] = std::unique_ptr<MachineSSAUpdater>(new MachineSSAUpdater(*thisMF));
    depchain_updater[&set]->Initialize(depchain_start[&set]);
    depchain_updater[&set]->AddAvailableValue(entryBB, depchain_start[&set]);
  }

  DEBUG(errs() << "Before addMemoryOrderingConstraints");
  for (MachineBasicBlock &BB : *thisMF) {

    // Link all the memory ops in BB together.
    // Return the name of the last output register (which could be
    // mem_in_reg).
    switch (OrderMemopsType) {
    case OrderMemopsMode::wavefront:
      {
        convert_block_memops_wavefront(BB, depchain_updater, depchain_uses, depchain_start);
      }
      break;
    case OrderMemopsMode::linear:
      {
        convert_block_memops_linear(BB, depchain_updater, depchain_uses, depchain_start);
      }
      break;

      // We should never get here.
    case OrderMemopsMode::none:
    default:
      assert(0 && "Only linear and wavefront memory ordering implemented now.");

    }

    DEBUG(errs() << "After memop conversion of function: " << BB << "\n");
  }

  // Create a mov to consume the end of all of each chain. We'll need one in
  // each terminating basic block. (We are still thinking control flow here.)
  // Note that using RI1 register class should keep this on the SXU.  Even
  // though we allocate a separate virtual register for each one, LLVM in the
  // end is free to re-use the same physical register since the values are dead
  // after each def.
  for (MachineBasicBlock &BB : *thisMF) {
    if (!BB.isReturnBlock())
      continue;

    for (const AliasSet &set : AS->getAliasSets()) {
      unsigned depchain_end = MRI->createVirtualRegister(&CSA::RI1RegClass);
      unsigned prev_chain_reg = depchain_start[&set];
      MachineInstr* endMov = BuildMI(BB, BB.getFirstTerminator(), DebugLoc(), TII->get(MemTokenMOVOpcode), depchain_end).addReg(prev_chain_reg);
      depchain_uses[&set].insert(endMov->operands_end()-1);
    }
  }

  // Finally, use the updater for each set to fully rewrite to SSA. This
  // includes generating PHI nodes.
  for (const AliasSet &set : AS->getAliasSets()) {
    for (MachineOperand *op : depchain_uses[&set]) {
      // There is an exception here: RewriteUse is not smart enough to find new
      // values added by "AddAvailableValue" before a use in the same basic
      // block. (I.e., it can only find them if they are in a basic block which
      // is a strict dominator.) This is only an issue when the use is in the
      // function's entry block. Fortunately, in this case, we can be sure that
      // depchain_reg contained the right value to use.
      if(op->getParent()->getParent() == entryBB)
        continue;

      depchain_updater[&set]->RewriteUse(*op);
    }
  }
}

void
CSAMemopOrdering::convert_block_memops_wavefront(MachineBasicBlock& BB,
    AliasSetUpdater& depchain_updater, AliasSetUses &depchain_uses,
    AliasSetVReg &depchain_start)
{
  DEBUG(errs() << "Wavefront memory ordering for block " << BB << "\n");

  // Save the latest evolution of each alias set's memory chain here.
  AliasSetVReg depchain_reg;
  // Also save a wavefront of load output signals per alias set.
  DenseMap<AliasSet*, SmallVector<unsigned, MEMDEP_VEC_WIDTH> > wavefront;

  MachineBasicBlock::iterator iterMI = BB.begin();
  while (iterMI != BB.end()) {
    MachineInstr* MI = &*iterMI;
    DEBUG(errs() << "Found instruction: " << *MI << "\n");

    unsigned current_opcode = MI->getOpcode();
    unsigned converted_opcode = TII->get_ordered_opcode_for_LDST(current_opcode);

    // If this is not an ordered instruction, we're done.
    if (current_opcode == converted_opcode) {
      ++iterMI;
      continue;
    }

    assert(MI->hasOneMemOperand() && "Can't handle multiple-memop ordering");
    const MachineMemOperand *mOp = *MI->memoperands_begin();

    // Use AliasAnalysis to determine which ordering chain we should be on.
    AliasSet *as =
      AS->getAliasSetForPointerIfExists(const_cast<Value*>(mOp->getValue()),
          mOp->getSize(), mOp->getAAInfo());

    // Create a new vreg which will be written to as the next link of the
    // chain.
    unsigned next_mem_reg = MRI->createVirtualRegister(MemopRC);

    // If the previous link is the first into this block, we use the vreg which
    // the updater was initialized with. The use will be saved, and then the
    // updater will fix it up later. Otherwise, just use an output created in
    // this block, which won't need updating.
    if (!depchain_reg[as])
      depchain_reg[as] = depchain_start[as];

    bool is_load = TII->isLoad(MI);
    if (is_load) {
      // Just a load. Build up the set of load outputs that we depend on.
      wavefront[as].push_back(next_mem_reg);
    }
    else {
      // This is a store or atomic instruction.  If there were any loads in the
      // last interval, merge all their outputs into one output, and change the
      // latest source.
      depchain_reg[as] = merge_dependency_signals(BB,
          MI,
          &wavefront[as],
          depchain_reg[as]);

      // Update the SSA updater.
      depchain_updater[as]->AddAvailableValue(&BB, depchain_reg[as]);

      // We have merged/consumed all pending load outputs.
      assert(wavefront[as].size() == 0);
    }

    unsigned newOpNum;
    MachineInstr *newInst = convert_memop_ins(MI, converted_opcode,
        next_mem_reg, depchain_reg[as], &newOpNum);

    // If the instruction uses a value coming into the block, then it will need
    // to be fixed by MachineSSAUpdater later. Save the operand to the list to
    // do this later.
    MachineOperand *newOp = &newInst->getOperand(newOpNum);
    assert(newOp && newOp->isReg() && newOp->isUse());
    if (newOp->getReg() == depchain_start[as]) {
      depchain_uses[as].insert(newOp);
    }

    if (!is_load) {
      // Advance the chain.
      depchain_reg[as] = next_mem_reg;

      // Update the SSA updater.
      depchain_updater[as]->AddAvailableValue(&BB, next_mem_reg);
    }

    // Finally, erase the old instruction.
    iterMI = BB.erase(iterMI);
  }

  for (auto &pair : wavefront) {
    AliasSet *as = pair.first;
    assert(depchain_reg[as]);

    // Sink any loads at the end of the block to the end of the block.
    depchain_reg[as] = merge_dependency_signals(BB,
        NULL,
        &pair.second,
        depchain_reg[as]);

    // Update the SSA updater.
    depchain_updater[as]->AddAvailableValue(&BB, depchain_reg[as]);
  }
}

void CSAMemopOrdering::convert_block_memops_linear(MachineBasicBlock& BB,
    AliasSetUpdater& depchain_updater, AliasSetUses &depchain_uses,
    AliasSetVReg& depchain_start)
{
  // Save the latest evolution of each alias set's memory chain here.
  AliasSetVReg depchain_reg;

  MachineBasicBlock::iterator iterMI = BB.begin();
  while (iterMI != BB.end()) {
    MachineInstr* MI = &*iterMI;
    DEBUG(errs() << "Found instruction: " << *MI << "\n");

    unsigned current_opcode = MI->getOpcode();
    unsigned converted_opcode = TII->get_ordered_opcode_for_LDST(current_opcode);

    // If this is not an ordered instruction, we're done.
    if (current_opcode == converted_opcode) {
      ++iterMI;
      continue;
    }

    assert(MI->hasOneMemOperand() && "Can't handle multiple-memop ordering");
    const MachineMemOperand *mOp = *MI->memoperands_begin();

    // Use AliasAnalysis to determine which ordering chain we should be on.
    AliasSet *as =
      AS->getAliasSetForPointerIfExists(const_cast<Value*>(mOp->getValue()),
          mOp->getSize(), mOp->getAAInfo());

    // Create a new vreg which will be written to as the next link of the
    // chain.
    unsigned next_mem_reg = MRI->createVirtualRegister(MemopRC);

    // If the previous link is the first into this block, we use the vreg which
    // the updater was initialized with. The use will be saved, and then the
    // updater will fix it up later. Otherwise, just use an output created in
    // this block, which won't need updating.
    if (!depchain_reg[as])
      depchain_reg[as] = depchain_start[as];

    // Hook this instruction into the chain, connecting the previous and next
    // values. The operand number using the old value is saved to newOpNum.
    unsigned newOpNum;
    MachineInstr *newInst = convert_memop_ins(MI, converted_opcode,
        next_mem_reg, depchain_reg[as], &newOpNum);

    // If the instruction uses a value coming into the block, then it will need
    // to be fixed by MachineSSAUpdater later. Save the operand to the list to
    // do this later.
    MachineOperand *newOp = &newInst->getOperand(newOpNum);
    assert(newOp && newOp->isReg() && newOp->isUse());
    if (newOp->getReg() == depchain_start[as]) {
      depchain_uses[as].insert(newOp);
    }

    // Advance the chain.
    depchain_reg[as] = next_mem_reg;

    // Update the SSA updater, advising it on the latest value in this
    // evolution coming out of this BB.
    depchain_updater[as]->AddAvailableValue(&BB, next_mem_reg);

    // Finally, erase the old instruction.
    iterMI = BB.erase(iterMI);
  }
}

unsigned CSAMemopOrdering::merge_dependency_signals(MachineBasicBlock& BB,
                                                  MachineInstr* MI,
                                                  SmallVector<unsigned, MEMDEP_VEC_WIDTH>* current_wavefront,
                                                  unsigned input_mem_reg) {

  if (current_wavefront->size() > 0) {
    DEBUG(errs() << "Merging dependency signals from " << current_wavefront->size() << " register " << "\n");

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
          DEBUG(errs() << "Inserted dependecy merge instruction " << *new_inst << "\n");
          next_level->push_back(next_out_reg);
        }
        else {
          // In an odd case, just pass register through to next level.
          next_level->push_back((*current_level)[i]);
        }
      }

      // Swap next and current.
      SmallVector<unsigned, MEMDEP_VEC_WIDTH>* tmp = current_level;
      current_level = next_level;
      next_level = tmp;
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

MachineInstr* CSAMemopOrdering::convert_memop_ins(MachineInstr* MI,
                                                unsigned new_opcode,
                                                unsigned issued_reg,
                                                unsigned ready_reg,
                                                unsigned *ready_op_num = nullptr) {
  MachineInstr* new_inst = NULL;
  DEBUG(errs() << "We want convert this instruction.\n");
  for (unsigned i = 0; i < MI->getNumOperands(); ++i) {
    MachineOperand& MO = MI->getOperand(i);
    DEBUG(errs() << "  Operand " << i << ": " << MO << "\n");
  }

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

  // ...and if requested, save the ready_op for the caller.
  if(ready_op_num != nullptr)
    *ready_op_num = new_inst->getNumOperands() - 1;

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

