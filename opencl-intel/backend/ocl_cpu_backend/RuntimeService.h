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

File Name:  RuntimeService.h

\*****************************************************************************/
#ifndef __RUNTIME_SERVICE_H__
#define __RUNTIME_SERVICE_H__

#include "RefcountThreadSafe.h"
#include "IBlockToKernelMapper.h"
#include <assert.h>

/*
    Backend Runtime service
    Reference-counted container for objects shared between Program and Kernels
    The motivation for this container is lifetime of kernel and parent program 
    Program can be deleted while kernels are alive in memory. So pointers from
    kernel to program may be invalidated.
    Example: Contain OCL20 BlockToKernelMap used to resolve block to kernels
    Feel free to add another objects to share between kernel and program
*/

namespace Intel { namespace OpenCL { namespace DeviceBackend {
  
  /// Runtime service class
  class RuntimeServiceImpl{
  public:
    /// ctor
    RuntimeServiceImpl() : m_pBlockToKernelMapper(NULL) {}
    
    /// dtor
    virtual ~RuntimeServiceImpl(){
      delete m_pBlockToKernelMapper;
    }
    
    /// getter for IBlockToKernelMapper
    IBlockToKernelMapper * GetBlockToKernelMapper() const {
      return m_pBlockToKernelMapper;
    }
    
    /// setter IBlockToKernelMapper
    void SetBlockToKernelMapper(IBlockToKernelMapper * p){
      assert(p && "IBlockToKernelMapper is NULL");
      if (m_pBlockToKernelMapper)
        delete m_pBlockToKernelMapper;
      m_pBlockToKernelMapper = p;
    }

  private:
    /// IBlockToKernelMapper object. This class owns it and is responsible for deleting
    IBlockToKernelMapper * m_pBlockToKernelMapper;
  };

  /// RuntimeService declaration
  /// implementation of intel::RefCount<> is thread-safe
  typedef intel::RefCountThreadSafe<RuntimeServiceImpl> RuntimeServiceSharedPtr;

}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {

#endif // __RUNTIME_SERVICE_H__
