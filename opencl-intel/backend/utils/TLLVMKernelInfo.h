/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef TLLVM_KERNEL_INFO_H
#define TLLVM_KERNEL_INFO_H

#include <cstddef> 

namespace Intel { namespace OpenCL { namespace DeviceBackend {

  struct TLLVMKernelInfo
  {
    std::size_t stTotalImplSize;
  };

  struct TKernelInfo
  {
    std::size_t kernelExecutionLength;
    bool hasBarrier;
  };

}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {

#endif // TLLVM_KERNEL_INFO_H

