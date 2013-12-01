/////////////////////////////////////////////////////////////////////////
// ExtendedExecutionContext.h:
/////////////////////////////////////////////////////////////////////////
// INTEL CONFIDENTIAL
// Copyright 2007-2013 Intel Corporation All Rights Reserved.
//
// The source code contained or described herein and all documents related 
// to the source code ("Material") are owned by Intel Corporation or its 
// suppliers or licensors. Title to the Material remains with Intel Corporation
// or its suppliers and licensors. The Material may contain trade secrets and 
// proprietary and confidential information of Intel Corporation and its 
// suppliers and licensors, and is protected by worldwide copyright and trade 
// secret laws and treaty provisions. No part of the Material may be used, copied, 
// reproduced, modified, published, uploaded, posted, transmitted, distributed, 
// or disclosed in any way without Intel?s prior express written permission. 
//
// No license under any patent, copyright, trade secret or other intellectual
// property right is granted to or conferred upon you by disclosure or delivery 
// of the Materials, either expressly, by implication, inducement, estoppel or 
// otherwise. Any license under such intellectual property rights must be express
// and approved by Intel in writing.
//
// Unless otherwise agreed by Intel in writing, you may not remove or alter this notice 
// or any other notice embedded in Materials by Intel or Intel?s suppliers or licensors 
// in any way.
/////////////////////////////////////////////////////////////////////////

#ifndef __EXTENEDED_EXECUTION_CONTEXT_H__
#define __EXTENEDED_EXECUTION_CONTEXT_H__

#include "cl_device_api.h"
#include "IDeviceCommandManager.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {
  class IBlockToKernelMapper;

  /// Extended Execution Context
  class ExtendedExecutionContext
  {
  public:
    /// @brief ctor with args
    /// @param pDCM - ptr to IDeviceCommandManager interface
    /// @param pMapper - ptr to IBlockToKernelMapper interface. read-only
    ExtendedExecutionContext( IDeviceCommandManager *pDCM,
                              const IBlockToKernelMapper  *pMapper)
      : m_pDeviceCommandManager(pDCM), 
        m_pBlockToKernelMapper(pMapper) 
    {}
    
    /// obtain IDeviceCommandManager interface
    IDeviceCommandManager *GetDeviceCommandManager() const {
      return m_pDeviceCommandManager;
    }
    
    /// obtain IBlockToKernelMapper interface
    const IBlockToKernelMapper *GetBlockToKernelMapper() const {
      return m_pBlockToKernelMapper;
    }
  private:
    /// reference to IDeviceCommandManager. Class does not own it
    IDeviceCommandManager * m_pDeviceCommandManager;
    /// reference to BlockToKernelMapper object. Class does not own it
    const IBlockToKernelMapper *  m_pBlockToKernelMapper;
  };

}}}

#endif // __EXTENEDED_EXECUTION_CONTEXT_H__
