// Copyright 2013 Intel Corporation.
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

#ifndef __RUNTIME_SERVICE_H__
#define __RUNTIME_SERVICE_H__

#include "IBlockToKernelMapper.h"
#include "RefcountThreadSafe.h"
#include <assert.h>
#include <mutex>

/*
    Backend Runtime service
    Reference-counted container for objects shared between Program and Kernels
    The motivation for this container is lifetime of kernel and parent program
    Program can be deleted while kernels are alive in memory. So pointers from
    kernel to program may be invalidated.
    Example: Contain OCL20 BlockToKernelMap used to resolve block to kernels
    Feel free to add another objects to share between kernel and program
*/

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

/// Runtime service class
class RuntimeServiceImpl {
public:
  /// ctor
  RuntimeServiceImpl() : m_pBlockToKernelMapper(NULL) {}

  /// dtor
  virtual ~RuntimeServiceImpl() {
    std::lock_guard<std::mutex> locked(m_BlockToKernelMapperMutex);
    delete m_pBlockToKernelMapper;
  }

  /// getter for IBlockToKernelMapper
  IBlockToKernelMapper *GetBlockToKernelMapper() const {
    std::lock_guard<std::mutex> locked(m_BlockToKernelMapperMutex);
    return m_pBlockToKernelMapper;
  }

  /// setter IBlockToKernelMapper
  void SetBlockToKernelMapper(IBlockToKernelMapper *p) {
    assert(p && "IBlockToKernelMapper is NULL");
    std::lock_guard<std::mutex> locked(m_BlockToKernelMapperMutex);
    delete m_pBlockToKernelMapper;
    m_pBlockToKernelMapper = p;
  }

private:
  /// IBlockToKernelMapper object. This class owns it and is responsible for
  /// deleting
  IBlockToKernelMapper *m_pBlockToKernelMapper;
  mutable std::mutex m_BlockToKernelMapperMutex;

  /// hide copy ctor
  RuntimeServiceImpl(const RuntimeServiceImpl &s);
  /// hide assignment
  void operator=(RuntimeServiceImpl &);
};

/// RuntimeService declaration
/// implementation of intel::RefCount<> is thread-safe
typedef intel::RefCountThreadSafe<RuntimeServiceImpl> RuntimeServiceSharedPtr;

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel

#endif // __RUNTIME_SERVICE_H__
