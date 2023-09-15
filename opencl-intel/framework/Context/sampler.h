// INTEL CONFIDENTIAL
//
// Copyright 2006 Intel Corporation.
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

#include "Logger.h"
#include "cl_framework.h"
#include "cl_object.h"
#include "cl_shared_ptr.h"
#include <map>

namespace Intel {
namespace OpenCL {
namespace Framework {

class Context;

/*******************************************************************************
 * Class name:  Sampler
 *
 * Inherit:    OCLObject
 * Description:  represents a sampler object
 ******************************************************************************/
class Sampler : public OCLObject<_cl_sampler_int> {
public:
  PREPARE_SHARED_PTR(Sampler)

  static SharedPtr<Sampler> Allocate(_cl_context_int *context) {
    return SharedPtr<Sampler>(new Sampler(context));
  }

  /*****************************************************************************
   * Function:   Sampler
   * Description:  The Sampler class constructor
   * Arguments:
   ****************************************************************************/
  Sampler(_cl_context_int *context);

  Sampler(const Sampler &) = delete;
  Sampler &operator=(const Sampler &) = delete;

  // get image info
  cl_err_code GetInfo(cl_int iParamName, size_t szParamValueSize,
                      void *pParamValue,
                      size_t *pszParamValueSizeRet) const override;

  virtual cl_err_code Initialize(SharedPtr<Context> pContext,
                                 cl_bool bNormalizedCoords,
                                 cl_addressing_mode clAddressingMode,
                                 cl_filter_mode clFilterMode);

  ConstSharedPtr<Context> GetContext() const { return m_pContext; }

  SharedPtr<Context> GetContext() { return m_pContext; }

  cl_uint GetValue() const { return m_clSamlerProps; }

  void SetProperties(std::vector<cl_sampler_properties> &samplerPropsArray);

protected:
  /*****************************************************************************
   * Function:   ~Sampler
   * Description:  The Sampler class destructor
   * Arguments:
   ****************************************************************************/
  virtual ~Sampler();

  SharedPtr<Context> m_pContext; // the context to which the sampler belongs

  cl_addressing_mode m_clAddressingMode = 0;
  cl_filter_mode m_clFilterMode = 0;
  cl_bool m_bNormalizedCoords = false;

  cl_uint m_clSamlerProps = 0;
  std::vector<cl_sampler_properties> m_clSamplerPropArrays;

  DECLARE_LOGGER_CLIENT;
};

} // namespace Framework
} // namespace OpenCL
} // namespace Intel
