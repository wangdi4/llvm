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
#include <string>
#include <vector>

namespace Intel {
namespace OpenCL {
namespace ClangFE {

struct CachedArgInfo {
  std::string typeName;
  std::string name;
  cl_kernel_arg_address_qualifier adressQualifier;
  cl_kernel_arg_access_qualifier accessQualifier;
  cl_kernel_arg_type_qualifier typeQualifier;
  cl_bool hostAccessible;
  cl_int localMemSize;
};

class OCLFEKernelArgInfo : public IOCLFEKernelArgInfo {
public:
  size_t getNumArgs() const override { return m_argsInfo.size(); }
  const char *getArgName(size_t index) const override {
    return m_argsInfo[index].name.c_str();
  }
  const char *getArgTypeName(size_t index) const override {
    return m_argsInfo[index].typeName.c_str();
  }
  cl_kernel_arg_address_qualifier
  getArgAdressQualifier(size_t index) const override {
    return m_argsInfo[index].adressQualifier;
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
  void Release() override { m_argsInfo.clear(); }
  void addInfo(const CachedArgInfo &info) {
      m_argsInfo.push_back(info);
  }

private:
  std::vector<CachedArgInfo> m_argsInfo;
};

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
