/*****************************************************************************\

Copyright (c) Intel Corporation (2011).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  OpenCLMICArgsBuffer.h

\*****************************************************************************/
#ifndef OPENCL_MIC_ARGS_BUFFER_H
#define OPENCL_MIC_ARGS_BUFFER_H

#include "IRunResult.h"
#include "IBufferContainerList.h"
#include "DeviceCommunicationService.h"
#include "MICNative/common.h"
#include "source/COIBuffer_source.h"
#include "source/COIProcess_source.h"
#include "source/COIPipeline_source.h"

#include "cl_types.h"
#include "CL/cl.h"
#include "cl_device_api.h"
#include "cl_dev_backend_api.h"

namespace Validation
{
    /// @brief Fills ocl_backend's cl_mem_obj_descriptor structure from DataManager's
    /// BufferDesc and pointer to data
    /// @param [OUT] mem_desc output structure
    /// @param [IN] buffer_desc DataManager's buffer descriptor
    /// @param [IN] pData pointer to data stored in Buffer
    void FillMemObjDescriptor( cl_mem_obj_descriptor& mem_desc, 
        const BufferDesc& buffer_desc, void* pData);
    
    /// @brief Fills ocl_backend's cl_mem_obj_descriptor structure from DataManager's
    /// ImageDesc and pointer to data
    /// @param [OUT] mem_desc output structure
    /// @param [IN] image_desc DataManager's image descriptor
    /// @param [IN] pData pointer to data stored in image
    void FillMemObjDescriptor( cl_mem_obj_descriptor& mem_desc, 
        const ImageDesc& image_desc, void* pData);

  /// @brief This class is responsible for handling the arguments buffer need for creation of kernel binary
  class OpenCLMICArgsBuffer
  {

  public:

    /// @brief Constructor
    /// @param [IN] pKernelArgs Kernel arguments description
    /// @param [IN] kernelNumArgs Number of kernel arguments
    /// @param [IN] input Input buffers for the test program
      OpenCLMICArgsBuffer(const cl_kernel_argument * pKernelArgs,
          cl_uint kernelNumArgs,
          IBufferContainerList * input,
          DispatcherData* dispatcher,
          COIBuffersWrapper& coiBuffers,
          const COIPROCESS& deviceProcess);

    /// @brief Destructor
    ~OpenCLMICArgsBuffer(void);

    /// @brief Copies output from the kernel arguments buffer into run result output buffers
    /// @param [OUT] runResult This method updates runResult execution time
    /// @param [IN] input Input buffers for the test program, used for creation of the run result output buffers
    /// @param [IN] kernelName name of the kernel to copy the results for
    void CopyOutput(IRunResult * runResult, IBufferContainerList * input, const char* kernelName);

    /// @brief Returns the kernel arguments buffer
    /// @return Kernel arguments buffer
    uint8_t* GetArgsBuffer();

    /// @brief Returns the kernel arguments buffer size
    /// @return Kernel arguments buffer size in bytes
    size_t GetArgsBufferSize();

    std::vector<BufferDirective> GetDirectivePacks()
    {
        return m_directives;
    }

  private:
    /// @brief Calculates the kernel arguments buffer size
    /// @return Size in bytes of the kernel arguments buffer
    size_t CalcArgsBufferSize();

    /// @brief Copies input buffers content into the kernel arguments buffer
    /// Allocates memory for kernel arguments which represent buffers
    /// @param [IN] input Input buffers for the test program, used for creation of the run result output buffers
    void FillArgsBuffer(IBufferContainerList * input,
        COIBuffersWrapper& coiBuffers,
        const COIPROCESS& deviceProcess);

  private:

    // Kernel arguments buffer
    uint8_t* m_pArgsBuffer;
    // Size in bytes of kernel arguments buffer
    size_t m_argsBufferSize;

    // Kernel arguments description buffer
    const cl_kernel_argument * m_pKernelArgs;
    // Number of kernel arguments
    cl_uint m_kernelNumArgs;

    DispatcherData* m_dispatcher;
    std::vector<BufferDirective> m_directives;

  };
}


#endif // OPENCL_MIC_ARGS_BUFFER_H
