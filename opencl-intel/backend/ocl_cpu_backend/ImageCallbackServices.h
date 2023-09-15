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

#pragma once

#include "CompilerConfig.h"
#include "cl_cpu_detect.h"
#include "cl_dev_backend_api.h"
#include "cl_types.h"

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

/**
 * This interface class is responsible for the image callback services, i.e.,
 * giving the callback module to the device, and supplying the API for image
 *creation/deletion functions in the BE
 ***/

class ImageCallbackService : public ICLDevBackendImageService {
public:
  /* Initializes the service with the proper options, getting the architecture
   */
  ImageCallbackService(const ICompilerConfig &config, bool isCpu);

  /**
  returns the size of the auxilary structure for
  **/
  size_t GetAuxilarySize() const override { return sizeof(image_aux_data); }

  /**
   * Endues the image object with the auxilary data, and the proper callback
   *functions assigned, according to the architecture with which this service
   *has been initialized pImageObject - mem_object descriptor. General
   *descriptor of image object auxObject - Pointer to auxiliary data structure
   *to fill in. It is initialized with callbacks and set to
   *pImageObject->imageAuxData
   **/
  cl_dev_err_code CreateImageObject(cl_mem_obj_descriptor *pImageObject,
                                    void *auxObject) const override;

  /**
   *  Releases the auxilary data from the image object
   **/

  cl_dev_err_code DeleteImageObject(cl_mem_obj_descriptor *pImageObject,
                                    void **auxObject) const override;

  /**
   *  Returns an array of supported image formats
   ***/

  const cl_image_format *GetSupportedImageFormats(unsigned int *numFormats,
                                                  cl_mem_object_type imageType,
                                                  cl_mem_flags flags) override;

  void Release() override;

private:
  /*
   * Initializes the given pointer to 'trap' function.
   */
  void InitializeToTrap(void *arr[], size_t) const;
  void InitializeToTrap(void *&) const;

  Intel::OpenCL::Utils::CPUDetect *m_CpuId;
};

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
