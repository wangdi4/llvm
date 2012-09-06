/*****************************************************************************\

Copyright (c) Intel Corporation (2010-2012).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  ExecutionContext.h

\*****************************************************************************/

#ifndef __EXECUTION_CONTEXT_H__
#define __EXECUTION_CONTEXT_H__

#include "cpu_dev_limits.h"
#include "cl_dev_backend_api.h"
#include <cstddef>
#include <set>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

  /// @brief  Work Info structure, contains these information:
  ///         uiWorkDim     - work dimension
  ///         GlobalOffset  - global offset (for each dimension)
  ///         GlobalSize    - global size (for each dimension)
  ///         LocalSize     - local size (for each dimension)
  ///         WGNumber      - number of work groups (for each dimension)
  struct sWorkInfo
  {
      unsigned int    uiWorkDim;
      size_t          GlobalOffset[MAX_WORK_DIM];
      size_t          GlobalSize[MAX_WORK_DIM];
      size_t          LocalSize[MAX_WORK_DIM];
      size_t          WGNumber[MAX_WORK_DIM];
  };

  /// @brief Callback Context class contains implementation for some built-ins.
  ///         An instance of this class should be passed to kernel
  ///         as implicit parameter at execution time.
  class CallbackContext {
  public:
    // Constructor
    CallbackContext() : m_pPrinter(NULL){
      Reset();
    }

    // Destructor
    ~CallbackContext() {}

    // Reset Callback context
    void Reset() {
      m_bIsFirst.clear();
    }

    // Returns true if copy procedure to be done, false if not
    bool SetAndCheckAsyncCopy(unsigned int uiKey) {
        // if uiKey is not in set
        if(!m_bIsFirst.count(uiKey)) {
            // add uiKey to set
            m_bIsFirst.insert(uiKey);
            // return copy needs to be done
            return true;
        }
        else {
            // uiKey is in set. No need to do copying
            return false;
        }
    }

    // Returns true if barrier() should be applied to make WI switch
    // TODO: This function always returns false! Do we really need it?!
    bool ResetAsyncCopy(unsigned int uiKey) {
        if(!m_bIsFirst.count(uiKey)) {
            // repetitive work_group_events() call is treated as no-op 
            return false;
        }
        // erase element from set
        m_bIsFirst.erase(uiKey);
        return false;
    }

    // Initialize Device backend buffer printer (is not needed for CPU)
    void SetDevicePrinter(ICLDevBackendBufferPrinter* printer) {
      m_pPrinter = printer;
    }

    // Get Device backend buffer printer
    ICLDevBackendBufferPrinter* GetDevicePrinter() {
      return m_pPrinter;
    }

  private:
    // Set for checking if async_wg_copy built-ins in executed workitem should 
    // perform copying or it was already done
    // set stores event numbers (unsigned int) produced by async_wg_copy functions
    // if event is present in this set it means coping has already been done and
    // no need to perform copying within executed workgroup
    std::set<unsigned int> m_bIsFirst;

    // for printer service - not owned by this class
    ICLDevBackendBufferPrinter* m_pPrinter;
  };
}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {

#endif // __EXECUTION_CONTEXT_H__
