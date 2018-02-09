//==---- Link.h --- OpenCL front-end compiler ---------------------*- C++ -*---=
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===-----------------------------------------------------------------------===
#pragma once

#include "frontend_api.h"

namespace Intel {
namespace OpenCL {
namespace ClangFE {


class ClangFECompilerLinkTask {
public:
  ClangFECompilerLinkTask(
      Intel::OpenCL::FECompilerAPI::FELinkProgramsDescriptor *pProgDesc)
      : m_pProgDesc(pProgDesc) {}

  int Link(IOCLFEBinaryResult **pBinaryResult);

private:
  Intel::OpenCL::FECompilerAPI::FELinkProgramsDescriptor *m_pProgDesc;
};

// ClangFECompilerCheckLinkOptions
// Input: szOptions - a string representing the link options
// Output: szUnrecognizedOptions - a new string containing the unrecognized
// options separated by spaces Returns: 'true' if the link options are legal and
// 'false' otherwise
bool ClangFECompilerCheckLinkOptions(const char *szOptions,
                                     char *szUnrecognizedOptions,
                                     size_t uiUnrecognizedOptionsSize);

} // ClangFE
} // OpenCL
} // Intel
