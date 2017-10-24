/*****************************************************************************\

Copyright (c) Intel Corporation (2013).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  IBlockToKernelMapper.h

\*****************************************************************************/
#ifndef __IBLOCK_TO_KERNEL_MAPPER_H__
#define __IBLOCK_TO_KERNEL_MAPPER_H__

/*
  Interface for mapping enqueued Block to ICLDevBackendKernel object
*/
namespace Intel { namespace OpenCL { namespace DeviceBackend {
  class ICLDevBackendKernel_;

  /// interface for mapping OCL20 block at runtime to ICLDevBackendKernel object
  class IBlockToKernelMapper {
  public:
    /// @brief map key to ICLDevBackendKernel object
    /// @param key - unique block id. For CPU supposed to be block function entry point
    /// @return pointer to constant ICLDevBackendKernel_  object. 
    virtual const ICLDevBackendKernel_ * Map(const void * key) const = 0;

    /// @brief dtor
    virtual ~IBlockToKernelMapper() {}
  };

}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {

#endif // __IBLOCK_TO_KERNEL_MAPPER_H__
