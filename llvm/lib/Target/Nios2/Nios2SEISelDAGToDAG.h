//===-- Nios2SEISelDAGToDAG.h - A Dag to Dag Inst Selector for Nios2SE -----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Subclass of Nios2DAGToDAGISel specialized for nios232.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_NIOS2_NIOS2SEISELDAGTODAG_H
#define LLVM_LIB_TARGET_NIOS2_NIOS2SEISELDAGTODAG_H

#include "Nios2ISelDAGToDAG.h"

namespace llvm {

class Nios2SEDAGToDAGISel : public Nios2DAGToDAGISel {

public:
  explicit Nios2SEDAGToDAGISel(Nios2TargetMachine &TM, CodeGenOpt::Level OL)
      : Nios2DAGToDAGISel(TM, OL) {}

private:

  bool runOnMachineFunction(MachineFunction &MF) override;

  bool trySelect(SDNode *Node) override;

  void processFunctionAfterISel(MachineFunction &MF) override;

  // Insert instructions to initialize the global base register in the
  // first MBB of the function.
//  void initGlobalBaseReg(MachineFunction &MF);

  std::pair<SDNode *, SDNode *> selectMULT(SDNode *N, unsigned Opc,
                                           const SDLoc &DL, EVT Ty, bool HasLo,
                                           bool HasHi);

  void selectAddESubE(unsigned MOp, SDValue InFlag, SDValue CmpLHS,
                      const SDLoc &DL, SDNode *Node) const;
};

FunctionPass *createNios2SEISelDag(Nios2TargetMachine &TM,
                                  CodeGenOpt::Level OptLevel);

}

#endif
