//==---- GetKernelArgInfo.h ----- OpenCL front-end compiler -------*- C++ -*---=
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===-----------------------------------------------------------------------===
#pragma once

namespace Intel {
namespace OpenCL {
namespace ClangFE {

struct IOCLFEKernelArgInfo;

class ClangFECompilerGetKernelArgInfoTask {
public:
  ClangFECompilerGetKernelArgInfoTask() {}

  int GetKernelArgInfo(const void *pBin, size_t uiBinarySize,
                       const char *szKernelName,
                       IOCLFEKernelArgInfo **ppResult);
};

} // ClangFE
} // OpenCL
} // Intel
