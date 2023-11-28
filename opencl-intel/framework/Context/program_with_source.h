// INTEL CONFIDENTIAL
//
// Copyright 2012 Intel Corporation.
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
#include "observer.h"
#include "program.h"

namespace Intel {
namespace OpenCL {
namespace Framework {

class ProgramWithSource : public Program {

  PREPARE_SHARED_PTR(ProgramWithSource)

public:
  static SharedPtr<ProgramWithSource>
  Allocate(SharedPtr<Context> pContext, cl_uint uiNumStrings,
           const char **pSources, const size_t *pszLengths, cl_int *piRet) {
    return SharedPtr<ProgramWithSource>(new ProgramWithSource(
        pContext, uiNumStrings, pSources, pszLengths, piRet));
  }

  ProgramWithSource(const ProgramWithSource &) = delete;
  ProgramWithSource &operator=(const ProgramWithSource &) = delete;

  cl_err_code GetInfo(cl_int param_name, size_t param_value_size,
                      void *param_value,
                      size_t *param_value_size_ret) const override;

  // Returns a read only pointer to internal source, used for build stages by
  // program service
  virtual const char *GetSourceInternal() override {
    return m_SourceString.data();
  };

protected:
  ProgramWithSource(SharedPtr<Context> pContext, cl_uint uiNumStrings,
                    const char **pSources, const size_t *pszLengths,
                    cl_int *piRet);

  virtual ~ProgramWithSource();

  bool CopySourceStrings(cl_uint uiNumStrings, const char **pSources,
                         const size_t *pszLengths);

private:
  std::string m_SourceString;
};
} // namespace Framework
} // namespace OpenCL
} // namespace Intel
