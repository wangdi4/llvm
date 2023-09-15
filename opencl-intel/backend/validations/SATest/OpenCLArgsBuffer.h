// INTEL CONFIDENTIAL
//
// Copyright 2011 Intel Corporation.
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

#ifndef OPENCL_ARGS_BUFFER_H
#define OPENCL_ARGS_BUFFER_H

#include "CL/cl.h"
#include "IBufferContainerList.h"
#include "IRunResult.h"
#include "cl_dev_backend_api.h"
#include "cl_device_api.h"
#include "cl_types.h"

namespace Validation {

/// @brief Fills ocl_backend's cl_mem_obj_descriptor structure from
/// DataManager's BufferDesc and pointer to data
/// @param [OUT] mem_desc output structure
/// @param [IN] buffer_desc DataManager's buffer descriptor
/// @param [IN] pData pointer to data stored in Buffer
void FillMemObjDescriptor(cl_mem_obj_descriptor &mem_desc,
                          const BufferDesc &buffer_desc, void *pData);

/// @brief Fills ocl_backend's cl_mem_obj_descriptor structure from
/// DataManager's ImageDesc and pointer to data
/// @param [OUT] mem_desc output structure
/// @param [IN] image_desc DataManager's image descriptor
/// @param [IN] pData pointer to data stored in image
void FillMemObjDescriptor(
    cl_mem_obj_descriptor &mem_desc, const ImageDesc &image_desc, void *pData,
    const Intel::OpenCL::DeviceBackend::ICLDevBackendImageService
        *pImageService);

/// @brief This class is responsible for handling the arguments buffer need for
/// creation of kernel binary
class OpenCLArgsBuffer {
public:
  /// @brief Constructor
  /// @param [IN] pKernelArgs Kernel arguments description
  /// @param [IN] kernelNumArgs Number of kernel arguments
  /// @param [IN] input Input buffers for the test program
  /// @param {IN] isCheckOOBAccess if true, check for out of bounds access
  OpenCLArgsBuffer(const llvm::KernelArgument *pKernelArgs,
                   cl_uint kernelNumArgs, IBufferContainerList *input,
                   const Intel::OpenCL::DeviceBackend::ICLDevBackendImageService
                       *pImageService,
                   bool isCheckOOBAccess);

  /// @brief Destructor
  ~OpenCLArgsBuffer(void);

  /// @brief Copies output from the kernel arguments buffer into run result
  /// output buffers
  /// @param [OUT] output BufferContainer will be filled with output data.
  /// @param [IN] input Input buffers for the test program, used for creation of
  /// the run result output buffers
  void CopyOutput(IBufferContainerList &output,
                  const IBufferContainerList *input);

  /// @brief Returns the kernel arguments buffer
  /// @return Kernel arguments buffer
  uint8_t *GetArgsBuffer();

  /// @brief Returns the kernel arguments buffer size
  /// @return Kernel arguments buffer size in bytes
  size_t GetArgsBufferSize();

private:
  /// @brief Calculates the kernel arguments buffer size
  /// @return Size in bytes of the kernel arguments buffer
  size_t CalcArgsBufferSize();

  /// @brief Copies input buffers content into the kernel arguments buffer
  /// Allocates memory for kernel arguments which represent buffers
  /// @param [IN] input Input buffers for the test program, used for creation of
  /// the run result output buffers
  void FillArgsBuffer(IBufferContainerList *input);

  /// @brief Destroys kernel arguments which represent buffers
  /// Destroys memory allocated for kernel arguments which represent buffers
  void DestroyArgsBuffer();

private:
  // Kernel arguments buffer
  uint8_t *m_pArgsBuffer;
  // Size in bytes of kernel arguments buffer
  size_t m_argsBufferSize;

  // Kernel arguments description buffer
  const llvm::KernelArgument *m_pKernelArgs;
  // Number of kernel arguments
  cl_uint m_kernelNumArgs;
  // Image service to use for image aux data initialization
  const Intel::OpenCL::DeviceBackend::ICLDevBackendImageService
      *m_pImageService;

  // will this buffer self-check for out of bounds access?
  bool m_isCheckOOBAccess;
};
} // namespace Validation

#endif // OPENCL_ARGS_BUFFER_H
