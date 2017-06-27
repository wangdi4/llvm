//==---- SPIRMaterializer.h - SPIR 1.2 materializer  -----------*- C++ -*---=
//
// Copyright (C) 2012-2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------===
/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/


#ifndef __SPIR_MATERIALIZER_H__
#define __SPIR_MATERIALIZER_H__

namespace llvm {
  class Module;
}

namespace intel {
/// \brief Adjusts the given module to be processed by the BE.
///
/// More concretely:
/// - replaces SPIR artifacts with Intel-implementation specific stuff.
/// - updates LLVM IR to version supported by back-end compiler
int MaterializeSPIR(llvm::Module &M);
}

#endif //__SPIR_MATERIALIZER_H__
