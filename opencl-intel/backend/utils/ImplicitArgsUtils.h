// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
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

#ifndef __IMPLICIT_ARGS_UTILS_H__
#define __IMPLICIT_ARGS_UTILS_H__

#include "ImplicitArgProperties.h"
#include "ImplicitArgument.h"
#include <cassert>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

  class CallbackContext;
  class ExtendedExecutionContext;
  namespace NDInfo {
  // Keep these values in same order as structure so they can be used as indices
  // for GEP accesses
  enum _NDInfo {
    WORK_DIM,
    GLOBAL_OFFSET,
    GLOBAL_SIZE,
    LOCAL_SIZE,
    WG_NUMBER,
    RUNTIME_INTERFACE,
    BLOCK2KERNEL_MAPPER,
    LAST
  };
  static const char* getRecordName(unsigned RecordID) {
    const char* Names[LAST] = {
      "WorkDim",
      "GlobalOffset_",
      "GlobalSize_",
      "LocalSize_",
      "NumGroups_",
      "RuntimeInterface",
      "Block2KernelMapper",
    };
    return Names[RecordID];
  }
  }
  enum TInternalCallType {
    ICT_NONE,
    ICT_GET_BASE_GLOBAL_ID,
    ICT_GET_SPECIAL_BUFFER,
    ICT_GET_WORK_DIM,
    ICT_GET_GLOBAL_SIZE,
    ICT_GET_LOCAL_SIZE,
    ICT_GET_ENQUEUED_LOCAL_SIZE,
    ICT_GET_NUM_GROUPS,
    ICT_GET_GROUP_ID,
    ICT_GET_GLOBAL_OFFSET,
    ICT_PRINTF,
    ICT_PREFETCH,
    // int enqueue_kernel_varargs()
    ICT_ENQUEUE_KERNEL_LOCALMEM,
    // int enqueue_kernel_events_varargs()
    ICT_ENQUEUE_KERNEL_EVENTS_LOCALMEM,
    ICT_NUMBER
 };
  static unsigned InternalCall2NDInfo(unsigned InternalCall) {
    assert(InternalCall < ICT_NUMBER);
    switch (InternalCall) {
    case ICT_GET_GLOBAL_OFFSET:
      return NDInfo::GLOBAL_OFFSET;
    case ICT_GET_GLOBAL_SIZE:
      return NDInfo::GLOBAL_SIZE;
    case ICT_GET_LOCAL_SIZE:
    case ICT_GET_ENQUEUED_LOCAL_SIZE:
      return NDInfo::LOCAL_SIZE;
    case ICT_GET_NUM_GROUPS:
      return NDInfo::WG_NUMBER;
    }
    assert(false && "Unhandled case");
    return NDInfo::LAST;
  }

  struct sWorkInfo;

  /// @brief  ImplicitArgsUtils class used to provide helper utilies for handling
  ///         implicit arguments.
  class ImplicitArgsUtils {
  
  public:
    enum IMPLICIT_ARGS {
      IA_SLM_BUFFER,
      IA_WORK_GROUP_INFO,
      IA_WORK_GROUP_ID,
      IA_GLOBAL_BASE_ID,
      IA_BARRIER_BUFFER,
      IA_RUNTIME_HANDLE,
      IA_NUMBER
    };
    static const unsigned int NUMBER_IMPLICIT_ARGS = IA_NUMBER;

    /// @brief Returns the implicit argument properties of given argument index
    /// @param arg     The implicit argument index
    /// @returns The implicit argument properties
    static const ImplicitArgProperties& getImplicitArgProps(unsigned int arg);

    /// @brief  Initialize properties on implicit arguments in run time
    /// @param  sizeOfPtr     Size of pointer, depends on target machine
    /// @returns none
    static void initImplicitArgProps(unsigned int sizeOfPtr);
    
    /// @brief Indicates that the properties were initialized
    static bool m_initialized;

    /// @brief Constructor
    ImplicitArgsUtils() {}
    /// @brief Destructor
    ~ImplicitArgsUtils() {}

    /// @brief Creates implicit arguments based on the implicit arguments properties
    /// @param pDest          A buffer that should hold the values of the implicit arguments
    void createImplicitArgs(char* pDest);

    static const char* getArgName(unsigned Idx);
  
    /// @brief Initialized the work item local IDs
    static void initWILocalIds(size_t dim, const size_t* pLocalSizes, const unsigned int packetWidth, size_t* pWIids);
    // getAdjustedAlignemnt - Returns a value which is >= 'offset' for the offset
    // of the implicit args from the start of the kernel uniform args. Used to
    // ensure the implicit args is located at a  correctly aligned address.
    // offset - the offset that possibly needs to be adjusted
    // ST - sizeof(size_t)
    static size_t getAdjustedAlignment(size_t offset, size_t ST);

  private:
    /// static list of implicit argument properties 
    static ImplicitArgProperties m_implicitArgProps[NUMBER_IMPLICIT_ARGS];
  
    /// list of implicit arguments
    ImplicitArgument m_implicitArgs[NUMBER_IMPLICIT_ARGS];
  };

}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {

#endif // __IMPLICIT_ARGS_UTILS_H__
