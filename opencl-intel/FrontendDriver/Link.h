// INTEL CONFIDENTIAL
//
// Copyright 2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#pragma once

#include "frontend_api.h"

#include "llvm/IR/Module.h"
#include "llvm/Option/ArgList.h"
#include "llvm/Option/OptTable.h"

#include <list>
#include <memory>

namespace Intel {
namespace OpenCL {
namespace ClangFE {

// Derived from llvm::opt::OptTable because it has protected constructor
class OpenCLLinkOptTable : public llvm::opt::GenericOptTable {
public:
  OpenCLLinkOptTable();
};

//
// Options parser for the Link function
//
class ClangLinkOptions {
public:
  ClangLinkOptions(const char *pszOptions);
  ClangLinkOptions() = delete;

  // Validates the user supplied OpenCL link options
  // Params:
  //    pszUnknownOptions - optional outbound pointer to the space separated
  //    unrecognized options
  //    uiUnknownOptionsSize - size of the pszUnknownOptions buffer
  // Returns:
  //    true if the options verification was successful, false otherwise
  bool checkOptions(char *pszUnknownOptions, size_t uiUnknownOptionsSize);

  bool hasArg(int id) const;

private:
  OpenCLLinkOptTable m_optTbl;
  std::unique_ptr<llvm::opt::InputArgList> m_pArgs;
  unsigned m_missingArgIndex;
  unsigned m_missingArgCount;
  llvm::SmallVector<const char *, 4> m_Args;
};

class ClangFECompilerLinkTask {
public:
  ClangFECompilerLinkTask(
      Intel::OpenCL::FECompilerAPI::FELinkProgramsDescriptor *pProgDesc)
      : m_pProgDesc(pProgDesc) {}

  int Link(IOCLFEBinaryResult **pBinaryResult);

private:
  Intel::OpenCL::FECompilerAPI::FELinkProgramsDescriptor *m_pProgDesc;
};

struct OCLFELinkKernelNames : public FECompilerAPI::IOCLFELinkKernelNames {
public:
  const char *GetAllKernelNames() override { return m_allKernelNames.c_str(); }
  void SetAllKernelNames(std::string names) { m_allKernelNames = names; }
  void Release() override { delete this; }

private:
  std::string m_allKernelNames;
};

} // namespace ClangFE
} // namespace OpenCL
} // namespace Intel
