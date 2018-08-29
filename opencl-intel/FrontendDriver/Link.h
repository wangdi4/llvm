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

using namespace llvm;

// Derived from llvm::opt::OptTable because it has protected constructor
class OpenCLLinkOptTable : public llvm::opt::OptTable {
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
  llvm::SmallVector<const char*, 4> m_Args;
};

//
// Wrapper around the 'opencl.compiler.options' metadata
// (we can't use the MetaData Utils since it would add additional dependency
// on MetaData component which is not shared with VPG yet )
//
class SpirOptions {
public:
  SpirOptions(llvm::Module *pModule);

  bool getDebugInfoFlag() const { return m_bDebugInfo; }
  bool getProfiling() const { return m_bProfiling; }
  bool getDisableOpt() const { return m_bDisableOpt; }
  bool getFastRelaxedMath() const { return m_bFastRelaxedMath; }
  bool getDenormsAreZero() const { return m_bDenormsAreZero; }

  void setDebugInfoFlag(bool value) { setBoolFlag(value, "-g", m_bDebugInfo); }

  void setProfiling(bool value) {
    setBoolFlag(value, "-profiling", m_bProfiling);
  }

  void setDisableOpt(bool value) {
    setBoolFlag(value, "-cl-opt-disable", m_bDisableOpt);
  }

  void setFastRelaxedMath(bool value) {
    setBoolFlag(value, "-cl-fast-relaxed-math", m_bFastRelaxedMath);
  }

  void setDenormsAreZero(bool value) {
    setBoolFlag(value, "-cl-denorms-are-zero", m_bDenormsAreZero);
  }

  void addOption(const std::string &option);

  std::list<std::string>::const_iterator beginOptions() const {
    return m_options.begin();
  }

  std::list<std::string>::const_iterator endOptions() const {
    return m_options.end();
  }

  void save(llvm::Module *pModule);

private:
  void setBoolFlag(bool value, const char *flag, bool &dest);

  bool m_bDebugInfo;
  bool m_bProfiling;
  bool m_bDisableOpt;
  bool m_bFastRelaxedMath;
  bool m_bDenormsAreZero;
  std::list<std::string> m_options;
};

//
// Helper class for 'opencl.compiler.options' metadata conflict resolution
// during link
//
class MetadataLinker {
public:
  MetadataLinker(const ClangLinkOptions &linkOptions, llvm::Module *pModule);

  void Link(llvm::Module *pModule);

  void Save(llvm::Module *pModule);

private:
  // effective flags stored in the module
  SpirOptions m_effectiveOptions;
  // flags supplied by link options
  bool m_bEnableLinkOptions;
  bool m_bCreateLibrary;
  bool m_bDenormsAreZeroLinkOpt;
  bool m_bNoSignedZerosLinkOpt;
  bool m_bUnsafeMathLinkOpt;
  bool m_bFiniteMathOnlyLinkOpt;
  bool m_bFastRelaxedMathLinkOpt;
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

} // ClangFE
} // OpenCL
} // Intel
