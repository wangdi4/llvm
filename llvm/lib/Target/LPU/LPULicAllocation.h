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
#include "LPUTargetMachine.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Target/TargetRegisterInfo.h"

namespace llvm {

  class MachineBasicBlock;
  class MachineInstr;

  class LPULicAllocation {
  public:

    enum LPULicAllocType {
      LicAllocDisabled = 0,  // Disable the pass.
      LicAllocEnabled  = 1,  // Enable the full pass.
      LicAllocDryRun   = 2,  // Analysis only, but don't modify the graph. 
      LicAllocUnknown  = 3,  // Last option.
    };

    LPULicAllocation(int lic_alloc_flag) : m_lic_alloc_flag(lic_alloc_flag % LicAllocUnknown) { }

    bool pass_enabled() const {
      return (m_lic_alloc_flag > LicAllocDisabled);
    }


    //allocate LICs to live ranges that are LIC allocatable
    bool allocateLicsInBlock(MachineBasicBlock* BB);

  private:

    // Integer flag that we can specify at the command line to control
    // the LIC allocation step.
    //
    // Currently, the allowed options are defined by LPULicAllocType
    int m_lic_alloc_flag;


    // Flag for controlling whether we do replacement of registers in
    // PHIs.
    //
    // TBD(jsukha): It is 1 for now, until we understand how to deal
    // with registers that appear in PHI nodes.
    const int SKIP_PHI_FLAG = 1;

    // Method checking whether we will actually replace registers with
    // LICs or not, based on the user's input flag.
    bool do_LIC_replacement() const {
      return (m_lic_alloc_flag == LicAllocEnabled);
    }


    // Other interesting helper methods.

    // Count the number of uses of a particular virtual register
    // "Reg".
    // 
    // If "skip_phis" is true, then this function returns 0 if this
    // register is used in any PHI instruction.
    // TBD(jsukha): For now, we are skipping register uses in PHIs
    // because there are problems in later stages.   
    // 
    // The MachineBasicBlock BB is the basic block is one we expect
    // to have most of the uses, but some of the uses can be
    // outside this block.
    // 
    // TBD(jsukha): Right now, this method internally differentiates
    // between uses inside BB and outside, but it only returns a total
    // value.  We may want to change the interface to eliminate this
    // distinction later.
    int count_reg_uses(MachineRegisterInfo* MRI,
                       unsigned Reg,
                       MachineBasicBlock* BB,
                       bool skip_phis);

    
    // Replace the uses of a particular virtual register "Reg" with
    // the list of LICs defined in "replacement_LICs".
    //
    // The uses replaced should match the ones counted by a
    // corresponding call to count_reg_uses.
    //
    //  Use j of "Reg" is replaced by replacement_LICs[j], if j <
    //  replacement_LICs[j].size(), or replacement_LICs[0] if j is out
    //  of bounds.
    //
    // (This interface covers the two common cases, namely (1) we
    //  provide a new unique LIC for every use, or (2) we just point
    //  all uses to the same LIC, assuming that some other mechanism
    //  will create the necessary copies.)
    //
    int replace_reg_uses_with_LICs(MachineRegisterInfo* MRI,
                                   const TargetRegisterInfo& TRI,
                                   unsigned Reg,
                                   std::vector<unsigned>& replacement_LICs,
                                   bool skip_phis);

    // Generates N copies of the source register/LIC "src" (with the
    // specified register class "new_LIC_RC") connected using a
    // complete D-ary tree.
    // 
    // This method allocates LICs for each of nodes in the tree.
    //
    // After this method is complete, replacement_LICs is a vector
    // leaves of the tree that is modified to store the LICs at the N
    // leaves of the tree.
    //
    // TBD(jsukha): This method should also insert the relevant copy
    // instructions.
    void generate_LIC_copies(LPUMachineFunctionInfo* LMFI,
                             unsigned src,
                             const TargetRegisterClass* new_LIC_RC,
                             std::vector<unsigned>* replacement_LICs,
                             int N);

  };
}


#endif
