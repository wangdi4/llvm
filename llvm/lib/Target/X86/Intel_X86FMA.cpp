//====-- Intel_X86FMA.cpp - Fused Multiply Add optimization ---------------====
//
//      Copyright (c) 2016 Intel Corporation.
//      All rights reserved.
//
//        INTEL CORPORATION PROPRIETARY INFORMATION
//
// This software is supplied under the terms of a license
// agreement or nondisclosure agreement with Intel Corp.
// and may not be copied or disclosed except in accordance
// with the terms of that agreement.
//
// static char cvs_id[] = "$Id$";
//
// This file defines the pass which finds the best representations of
// the original expression trees consisting of MUL/ADD/SUB/FMA/UnarySub
// operations and performs transformations.
//
//  External interfaces:
//      FunctionPass *llvm::createX86GlobalFMAPass();
//      bool X86GlobalFMA::runOnMachineFunction(MachineFunction &MFunc);
//

#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "Intel_X86FMACommon.h"
using namespace llvm;

#define DEBUG_TYPE "x86-global-fma"

namespace {
// This internal switch can be used to turn off the Global FMA optimization.
// Currently the optimization is turned OFF by default.
static cl::opt<bool> DoFMAOpt("do-x86-global-fma",
                              cl::desc("Enable the global FMA opt."),
                              cl::init(false), cl::Hidden);
// This internal switch regulates the amount of debug messages printed
// by the Global FMA optimization.
static cl::opt<bool> DebugFMAOpt("debug-x86-global-fma",
                                 cl::desc("Control FMA debug printings."),
                                 cl::init(false), cl::Hidden);

// This function was created to make it possible to generate DEBUG output
// with desired level of details. DEBUG_WITH_TYPE() macro does not let to do
// it as it is exclusive, i.e. you can specify only 1 level of detailization,
// while this optimization may want to have several levels of dump details.
raw_ostream &fmadbgs() {
  return (!DebugFMAOpt) ? nulls() : dbgs();
}

// This class holds all pre-computed/efficient FMA patterns.
class FMAPatterns {
  private:
    // Represents a set of FMA patterns that all have the same SHAPE.
    struct FMAPatternsSet {
      const uint64_t  *Dags;
      unsigned         NumDags;

      // Initializes a set of patterns using the given reference to an array
      // \p Dags and the size of the array \p NumDags.
      FMAPatternsSet(const uint64_t *Dags, unsigned NumDags) :
        Dags(Dags), NumDags(NumDags) { }
    };

    // All FMA patterns are stored as a vector of references to groups of Dags
    // where each of the groups has the same SHAPE.
    // It is also supposed that the groups of Dags are sorted by the SHAPE.
    std::vector<FMAPatternsSet *> Dags;

    // Returns the number of shape (i.e. the number of Dag/pattern sets).
    unsigned getNumShapes() { return Dags.size(); }

  public:
    FMAPatterns() { };
    ~FMAPatterns(void) {
      for (auto D : Dags)
        delete D;
    }

    // Initialize the patterns storage.
    // Currently it is assumed that there is only one set of patterns for
    // the target CPU. It may be changed in future, for example, there may
    // be 2 separate pattern sets: for AVX and for AVX2. In such cases
    // the init() method may get some input arguments and become more
    // complex.
    void init() {
      // All the code that initializes the patterns storage is in the
      // following included header file.
#       include "X86GenMAPatterns.inc"
    }
};

// This class does all the optimization work, it goes through the functions,
// searches for the optimizable expressions and replaces then with more
// efficient equivalents.
class X86GlobalFMA : public MachineFunctionPass {
public:
  X86GlobalFMA() : MachineFunctionPass(ID),
                   MF(nullptr), TII(nullptr), Patterns(nullptr) { }
  ~X86GlobalFMA() {
    delete Patterns;
  }

  const char *getPassName() const override {return "X86 GlobalFMA";}

  bool runOnMachineFunction(MachineFunction &MFunc) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    MachineFunctionPass::getAnalysisUsage(AU);
  }

private:
  // Pass identification, replacement for typeid.
  static char ID;

  // A reference to the function being currently compiled.
  MachineFunction *MF;

  // This field is used to get information about available target operations.
  const TargetInstrInfo *TII;

  // A storage with pre-computed/efficient FMA patterns.
  FMAPatterns *Patterns;

  // Do the FMA optimization in one basic block.
  // Return true iff any changes in the IR were made.
  bool optBasicBlock(MachineBasicBlock &MBB);

  // Do the FMA optimizations in one parsed basic block.
  // Return true iff any changes in the IR were made.
  bool optParsedBasicBlock(/* FMABasicBlock *BB */);
};

char X86GlobalFMA::ID = 0;

/// Loop over all of the basic blocks, performing the FMA optimization for
/// each block separately.
bool X86GlobalFMA::runOnMachineFunction(MachineFunction &MFunc) {
  if (!DoFMAOpt)
    return false;

  MF = &MFunc;
  const X86Subtarget &ST = MF->getSubtarget<X86Subtarget>();
  TII = ST.getInstrInfo();

  // SubTarget must support FMA ISA.
  if (!ST.hasFMA())
    return false;

  // Compilation options must allow FP contraction and FP expression
  // re-association.
  const TargetOptions &Options = MF->getTarget().Options;
  if (Options.AllowFPOpFusion != FPOpFusion::Fast || !Options.UnsafeFPMath)
    return false;

  // Even though the compilation switches allow the Global FMA optimization it
  // still may be unsafe to do it as some of MUL/ADD/SUB/etc machine
  // instructions could be generated for LLVM IR operations with unset
  // 'fast-math' attributes. Such LLVM IR operations may be added to the
  // currently compiled function by the inlining optimization controlled by
  // -flto switch.
  // The 'fast-math' attributes get lost after LLVM IR to MachineInstr
  // translation. So, it is generally incorrect to do any unsafe algebra
  // transformations at the MachineInstr IR level.
  // FIXME: The ideal solution for this problem would be to have 'fast-math'
  // attributes defined for individual MachineInstr operations.
  // The currently used solution is rather temporary.
  //
  // Check the LLVM IR function. If there are some instructions in it with
  // attributes not allowing unsafe algebra, then exit.
  const Function *F = MF->getFunction();
  // If LLVM IR is not available, then just conservatively exit.
  if (!F)
    return false;
  for (auto &I : instructions(F)) {
    // isa<FPMathOperator>(&I) returns true for any operation having FP result.
    // In particular, it returns true for FP loads, which never have
    // the Fast-Math attributes set. Thus this opcode check is needed to
    // avoid mess with FP loads and other FMA opt unrelated operations.
    unsigned Opcode = I.getOpcode();
    bool CheckedOp = Opcode == Instruction::FAdd ||
                     Opcode == Instruction::FSub ||
                     Opcode == Instruction::FMul ||
                     Opcode == Instruction::FDiv ||
                     Opcode == Instruction::FRem ||
                     Opcode == Instruction::FCmp ||
                     Opcode == Instruction::Call;
    if (CheckedOp && isa<FPMathOperator>(&I) && !I.hasUnsafeAlgebra()) {
      DEBUG(fmadbgs() << "Exit because found mixed fast-math settings.\n");
      return false;
    }
  }

  // The patterns storage initialization code is not cheap, so it is better
  // to call it only when FMA instructions have a chance to be generated.
  // Also, if the patterns storage is already created/initialized once, it
  // does not make sense to re-initialize it again.
  // This place may need to be updated if/when the patterns storage
  // initialization gets dependant on the target CPU settings. For example,
  // if the patterns are initialized one way for AVX, another way for AVX2,
  // and there are functions with different target CPU setttings.
  if (Patterns == nullptr) {
    Patterns = new FMAPatterns();
    Patterns->init();
  }

  bool EverMadeChangeInFunc = false;

  // Process all basic blocks.
  for (MachineFunction::iterator I = MF->begin(), E = MF->end(); I != E; ++I)
    if (optBasicBlock(*I))
      EverMadeChangeInFunc = true;

  DEBUG(dbgs() << "********** X86 Global FMA **********\n");
  if (EverMadeChangeInFunc) {
    DEBUG(MF->print(dbgs()));
  }

  return EverMadeChangeInFunc;
}

/// Loop over all of the instructions in the basic block, optimizing
/// MUL/ADD/FMA expressions. Return true iff any changes in the machine
/// operation were done.
bool X86GlobalFMA::optBasicBlock(MachineBasicBlock &MBB) {
  DEBUG(fmadbgs() << "\n**** RUN FMA OPT FOR ANOTHER BASIC BLOCK ****\n");

  // Save the dump of the basic block, we may want to print it after the basic
  // block is changed by this optimization.
  std::string LogBBStr = "";
  raw_string_ostream LogBB(LogBBStr);
  DEBUG(LogBB << "Basic block before Global FMA opt:\n" << MBB << "\n");

  // Find MUL/ADD/SUB/FMA/etc operations in the input machine instructions
  // and create some FMA internal structures for them.
  // TODO: not implemented yet.

  // Run the FMA optimization and dump the debug messages if the optimization
  // produced any changes in IR.
  bool EverMadeChangeInBB = optParsedBasicBlock();
  if (EverMadeChangeInBB) {
    DEBUG(fmadbgs() << LogBB.str());
    DEBUG(fmadbgs() << "\nBasic block after Global FMA opt:\n" << MBB << "\n");
  }
  return EverMadeChangeInBB;
}

bool X86GlobalFMA::optParsedBasicBlock(/* FMABasicBlock *BB */) {
  // Not implemented yet.
  return false;
}

} // End anonymous namespace.

FunctionPass *llvm::createX86GlobalFMAPass() {
  return new X86GlobalFMA();
}
