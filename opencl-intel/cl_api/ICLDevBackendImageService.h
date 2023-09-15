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

#ifndef ICLDevBackendImageService_H
#define ICLDevBackendImageService_H

#include "ICLDevBackendProgram.h"
#include "cl_device_api.h"
#include "cl_types.h"
// #include "cl.h"
// #include "cl_image_supported_types.h"

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

/**
 * This interface class is responsible for the image callback services, i.e.,
 * giving the callback module to the device, and supplying the API for image
 *creation/deletion functions in the BE
 ***/

class ICLDevBackendImageService {
public:
  virtual ~ICLDevBackendImageService() {}

  /**
 returns the size of the auxilary structure for
 **/
  virtual size_t GetAuxilarySize() const = 0;

  /**
   * Endues the image object with the auxilary data, and the proper callback
   *functions assigned, according to the architecture with which this service
   *has been initialized pImageObject - mem_object descriptor. General
   *descriptor of image object auxObject - Pointer to auxiliary data structure
   *to fill in. It is initialized with callbacks and set to
   *pImageObject->imageAuxData
   **/
  virtual cl_dev_err_code CreateImageObject(cl_mem_obj_descriptor *pImageObject,
                                            void *auxObject) const = 0;

  /**
   *  Releases the image object, and returns the auxilary object to the device
   *for releasing.
   **/

  virtual cl_dev_err_code DeleteImageObject(cl_mem_obj_descriptor *pImageObject,
                                            void **auxObject) const = 0;

  const virtual cl_image_format *
  GetSupportedImageFormats(unsigned int *numFormats,
                           cl_mem_object_type imageType,
                           cl_mem_flags flags) = 0;

  /**
   * Releases the Image Service
   */
  virtual void Release() = 0;
};

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel

#endif // ICLDevBackendImageService_H
