//===--------------------- SGSizeCollector.h -*- C++ -*--------------------===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef BACKEND_VECTORIZER_VECTORVARIANT_SGSIZECOLLECTOR_H
#define BACKEND_VECTORIZER_VECTORVARIANT_SGSIZECOLLECTOR_H

#include "OCLPassSupport.h"
#include "VecConfig.h"

#include "llvm/Analysis/Intel_VectorVariant.h"

using namespace llvm;

namespace intel {

class SGSizeCollectorImpl {
public:
  SGSizeCollectorImpl(const Intel::OpenCL::Utils::CPUDetect *CPUId);

  bool runImpl(Module &M);

protected:
  bool hasVecLength(Function *F, int &VecLength);
  VectorVariant::ISAClass getCPUIdISA();

private:
  const Intel::OpenCL::Utils::CPUDetect *CPUId;
};

class SGSizeCollector : public ModulePass {
public:
  static char ID;

  SGSizeCollector(const Intel::OpenCL::Utils::CPUDetect *CPUId =
                      Intel::OpenCL::Utils::CPUDetect::GetInstance());

protected:
  bool runOnModule(Module &M) override;

private:
  SGSizeCollectorImpl Impl;
};

} // namespace intel

#endif // BACKEND_VECTORIZER_VECTORVARIANT_SGSIZECOLLECTOR_H
