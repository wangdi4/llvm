//===----------------- SGSizeCollectorIndirect.h -*- C++ -*----------------===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef BACKEND_VECTORIZER_VECTORVARIANT_SGSIZECOLLECTORINDIRECT_H
#define BACKEND_VECTORIZER_VECTORVARIANT_SGSIZECOLLECTORINDIRECT_H

#include "OCLPassSupport.h"
#include "SGSizeCollector.h"

using namespace llvm;

namespace intel {

class SGSizeCollectorIndirectImpl : public SGSizeCollectorImpl {
public:
  SGSizeCollectorIndirectImpl(const Intel::OpenCL::Utils::CPUDetect *CPUId);

  bool runImpl(Module &M);
};

class SGSizeCollectorIndirect : public ModulePass {
public:
  static char ID;

  SGSizeCollectorIndirect(const Intel::OpenCL::Utils::CPUDetect *CPUId =
                              Intel::OpenCL::Utils::CPUDetect::GetInstance());

protected:
  bool runOnModule(Module &M) override;

private:
  SGSizeCollectorIndirectImpl Impl;
};

} // namespace intel

#endif // BACKEND_VECTORIZER_VECTORVARIANT_SGSIZECOLLECTORINDIRECT_H
