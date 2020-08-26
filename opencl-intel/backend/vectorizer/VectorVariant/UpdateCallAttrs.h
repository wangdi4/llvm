//===--------------------- UpdateCallAttrs.h -*- C++ -*--------------------===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef BACKEND_VECTORIZER_VECTORVARIANT_UPDATECALLATTRS_H
#define BACKEND_VECTORIZER_VECTORVARIANT_UPDATECALLATTRS_H

#include "OCLPassSupport.h"

using namespace llvm;

namespace intel {

class UpdateCallAttrs : public ModulePass {
public:
  static char ID;

  UpdateCallAttrs();

protected:
  bool runOnModule(Module &M) override;
};

} // namespace intel

#endif // BACKEND_VECTORIZER_VECTORVARIANT_UPDATECALLATTRS_H
