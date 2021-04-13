//===------------------ VectorVariantLowering.h -*- C++ -*-----------------===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef BACKEND_VECTORIZER_VECTORVARIANT_VECTORVARIANTLOWERING_H
#define BACKEND_VECTORIZER_VECTORVARIANT_VECTORVARIANTLOWERING_H

#include "OCLPassSupport.h"
#include "VecConfig.h"

using namespace llvm;

namespace intel {

class VectorVariantLowering : public ModulePass {
public:
  static char ID;

  VectorVariantLowering(const Intel::OpenCL::Utils::CPUDetect *CPUId = nullptr);

protected:
  bool runOnModule(Module &M) override;

private:
  const Intel::OpenCL::Utils::CPUDetect *CPUId;
};

} // namespace intel

#endif // BACKEND_VECTORIZER_VECTORVARIANT_VECTORVARIANTLOWERING_H
