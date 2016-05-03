//===-- LPULicAllocation.h - LPU LIC allocation structures ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the machine instruction level if-conversion pass.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LPU_LPULICALLOCATION_H
#define LLVM_LIB_TARGET_LPU_LPULICALLOCATION_H

#include <set>

namespace llvm {

  class MachineBasicBlock;
  class MachineInstr;

  class LPULicAllocation {
  public:
    LPULicAllocation() {
    }

    // make live ranges LIC allocatable by inserting PICKs/SWITCHes as needed
    bool makeLiveRangesLicAllocatable(MachineInstr *MI, MachineBasicBlock* BB, 
                                      std::set<unsigned> &LiveRangesSet);


    //allocate LICs to live ranges that are LIC allocatable
    bool allocateLicsInLoop(MachineInstr *MI, MachineBasicBlock* BB);


  };
}


#endif
