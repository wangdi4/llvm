//==---- BinaryResult.h --- OpenCL front-end compiler -------------*- C++ -*---=
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===-----------------------------------------------------------------------===

#pragma once

#include "common_clang.h"
#include "llvm/ADT/SmallVector.h"
#include <string>

namespace Intel {
namespace OpenCL {
namespace ClangFE {

class OCLFEBinaryResult : public IOCLFEBinaryResult {
public:
  size_t GetIRSize() const override { return m_IRBuffer.size(); }
  const void *GetIR() const override { return m_IRBuffer.data(); }
  const char *GetIRName() const override { return m_IRName.c_str(); }
  IR_TYPE GetIRType() const override { return m_type; }
  const char *GetErrorLog() const override { return m_log.c_str(); }
  void Release() override { delete this; }

// OCLFEBinaryResult
  OCLFEBinaryResult() : m_type(IR_TYPE_UNKNOWN) {}
  llvm::SmallVectorImpl<char> &getIRBufferRef() { return m_IRBuffer; }
  std::string &getLogRef() { return m_log; }
  void setLog(const std::string &log) { m_log = log; }
  void setIRName(const std::string &name) { m_IRName = name; }
  void setIRType(Intel::OpenCL::ClangFE::IR_TYPE type) { m_type = type; }
  void setResult(int result) { m_result = result; }
  int getResult(void) const { return m_result; }

private:
  llvm::SmallVector<char, 4096> m_IRBuffer;
  std::string m_log;
  std::string m_IRName;
  Intel::OpenCL::ClangFE::IR_TYPE m_type;
  int m_result;
};

} // namespace ClangFE
} // namespace OpenCL
} // namespace Intel
