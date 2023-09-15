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
#include "cl_shared_ptr.hpp"
#include "program.h"

#include <cstdint>
#include <vector>

namespace Intel {
namespace OpenCL {
namespace Framework {

class ProgramWithIL : public Program {
  std::vector<char> m_pIL;

  // Info about specialiazation constants found in m_pIL.
  // This info is used to validate arguments given by the user
  // to clSetProgramSpecializationConstant API.
  using SpecConstInfoTy = std::pair<uint32_t, uint32_t>;
  std::vector<SpecConstInfoTy> m_SpecConstInfo;

  bool m_bHasSpecConstInfoFromIL = false;

  // Specialization constant provided by the user
  std::vector<uint32_t> m_SpecIds;
  std::vector<uint64_t> m_SpecVals;

public:
  PREPARE_SHARED_PTR(ProgramWithIL)

  static SharedPtr<ProgramWithIL> Allocate(SharedPtr<Context> pContext,
                                           const unsigned char *pIL,
                                           size_t length, cl_int *piRet) {
    return SharedPtr<ProgramWithIL>(
        new ProgramWithIL(pContext, pIL, length, piRet));
  }

  virtual const char *GetSourceInternal() override { return m_pIL.data(); }

  virtual unsigned int GetSize() { return m_pIL.size(); }

  bool IsSpecConstInfoCached() { return m_bHasSpecConstInfoFromIL; }

  void SpecConstInfoIsCached() { m_bHasSpecConstInfoFromIL = true; }

  unsigned int GetSpecConstCount() const { return m_SpecIds.size(); }

  const uint32_t *GetSpecConstIds() const { return m_SpecIds.data(); }

  const uint64_t *GetSpecConstValues() const { return m_SpecVals.data(); }

  std::vector<SpecConstInfoTy> &GetSpecConstInfoRef() {
    return m_SpecConstInfo;
  }

  cl_int AddSpecConst(cl_uint spec_id, size_t spec_size,
                      const void *spec_value);

  cl_err_code GetInfo(cl_int param_name, size_t param_value_size,
                      void *param_value,
                      size_t *param_value_size_ret) const override;

protected:
  ProgramWithIL(SharedPtr<Context> pContext, const unsigned char *pIL,
                size_t length, cl_int *piRet);

  virtual ~ProgramWithIL();
};

} // namespace Framework
} // namespace OpenCL
} // namespace Intel
