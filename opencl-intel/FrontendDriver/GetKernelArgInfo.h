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

#include "cl_device_api.h"
#include "frontend_api.h"
#include <string>
#include <vector>

namespace Intel {
namespace OpenCL {
namespace ClangFE {

class OCLFEKernelArgInfo : public IOCLFEKernelArgInfo {
public:
  ~OCLFEKernelArgInfo() { Release(); }

  size_t getNumArgs() const override { return m_argsInfo.size(); }
  const char *getArgName(size_t index) const override {
    return m_argsInfo[index].name;
  }
  const char *getArgTypeName(size_t index) const override {
    return m_argsInfo[index].typeName;
  }
  cl_kernel_arg_address_qualifier
  getArgAdressQualifier(size_t index) const override {
    return m_argsInfo[index].addressQualifier;
  }
  cl_kernel_arg_access_qualifier
  getArgAccessQualifier(size_t index) const override {
    return m_argsInfo[index].accessQualifier;
  }
  cl_kernel_arg_type_qualifier
  getArgTypeQualifier(size_t index) const override {
    return m_argsInfo[index].typeQualifier;
  }
  cl_bool getArgHostAccessible(size_t index) const override {
    return m_argsInfo[index].hostAccessible;
  }
  cl_int getArgLocalMemSize(size_t index) const override {
    return m_argsInfo[index].localMemSize;
  }
  void Release() override {
    for (auto &argInfo : m_argsInfo) {
      free(argInfo.name);
      free(argInfo.typeName);
    }
    m_argsInfo.clear();
  }
  void addInfo(const cl_kernel_argument_info &info) {
    m_argsInfo.push_back(info);
  }

private:
  std::vector<cl_kernel_argument_info> m_argsInfo;
};

class ClangFECompilerGetKernelArgInfoTask {
public:
  ClangFECompilerGetKernelArgInfoTask() {}

  int GetKernelArgInfo(const void *pBin, size_t uiBinarySize,
                       const char *szKernelName,
                       IOCLFEKernelArgInfo **ppResult);
};

} // namespace ClangFE
} // namespace OpenCL
} // namespace Intel
