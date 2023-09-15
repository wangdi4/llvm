// INTEL CONFIDENTIAL
//
// Copyright 2010 Intel Corporation.
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

#ifndef __CPU_SERIALIZATION_SERVICE
#define __CPU_SERIALIZATION_SERVICE

#include "IAbstractBackendFactory.h"
#include "cl_dev_backend_api.h"
#include <map>
#include <string>

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

class CPUSerializationService : public ICLDevBackendSerializationService {
public:
  CPUSerializationService(const ICLDevBackendOptions *pBackendOptions);

  // Program Functions
  virtual cl_dev_err_code
  GetSerializationBlobSize(cl_serialization_type serializationType,
                           const ICLDevBackendProgram_ *pProgram,
                           size_t *pSize) const override;

  virtual cl_dev_err_code
  SerializeProgram(cl_serialization_type serializationType,
                   const ICLDevBackendProgram_ *pProgram, void *pBlob,
                   size_t blobSize) const override;

  virtual cl_dev_err_code ReloadProgram(cl_serialization_type serializationType,
                                        ICLDevBackendProgram_ *pProgram,
                                        const void *pBlob, size_t blobSize,
                                        unsigned int binaryVersion) const;

  virtual cl_dev_err_code
  DeSerializeProgram(cl_serialization_type serializationType,
                     ICLDevBackendProgram_ **ppProgram, const void *pBlob,
                     size_t blobSize,
                     unsigned int binaryVersion) const override;

  virtual void ReleaseProgram(ICLDevBackendProgram_ *pProgram) const override;

  virtual void Release() override;

private:
  IAbstractBackendFactory *m_pBackendFactory;
};

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel

#endif // __CPU_SERIALIZATION_SERVICE
