//=== HIRVerifier.h - Verifies internal structure of HLNodes --*-- C++ --*-===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the HIR verifier that checks internal
// structure of HLNodes and attached DDRefs
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_IR_INTEL_LOOPIR_HIRVERIFIER_H
#define LLVM_IR_INTEL_LOOPIR_HIRVERIFIER_H

namespace llvm {

namespace loopopt {

class HLNode;

namespace HIRVerifier {

/// \brief Verifies a specific node integrity.
template <bool Recursive> void verifyNode(const HLNode *N);

/// \brief Verifies all nodes of HIR.
void verifyAll();
}

} // End namespace loopopt

} // End namespace llvm

#endif
