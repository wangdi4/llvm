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

#pragma once

namespace llvm {
class Module;
}

namespace Intel {
namespace OpenCL {
namespace FECompilerAPI {
struct FESPIRProgramDescriptor;
}
namespace ClangFE {

struct IOCLFEBinaryResult;

// Updates SPIR 1.2 to current LLVM IR version.
class ClangFECompilerMaterializeSPIRTask {
public:
  ClangFECompilerMaterializeSPIRTask(
      Intel::OpenCL::FECompilerAPI::FESPIRProgramDescriptor *pProgDesc)
      : m_pProgDesc(pProgDesc) {}

  /// \brief Updates SPIR 1.2 to consumable by the compiler back-end LLVM IR.
  /// \return error code of clCompileProgram API
  int MaterializeSPIR(IOCLFEBinaryResult **pBinaryResult);

  /// \brief Adjusts the given module to be processed by the BE.
  ///
  /// More concretely:
  /// - replaces SPIR artifacts with Intel-implementation specific stuff.
  /// - updates LLVM IR to version supported by back-end compiler
  static int MaterializeSPIR(llvm::Module &M);

private:
  Intel::OpenCL::FECompilerAPI::FESPIRProgramDescriptor *m_pProgDesc;
};

} // namespace ClangFE
} // namespace OpenCL
} // namespace Intel