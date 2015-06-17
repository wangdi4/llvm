//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This header file declares the base class for HIR transformation passes.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_VPO_VPOPASSES_H
#define LLVM_TRANSFORMS_VPO_VPOPASSES_H

namespace llvm {

class FunctionPass;

// Create VPO Driver pass
FunctionPass *createVPODriverPass();
}

#endif // LLVM_TRANSFORMS_VPO_VPOPASSES_H
