/*****************************************************************************\

Copyright (c) Intel Corporation (2010).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  Executable.h

\*****************************************************************************/

#pragma once

#include "cl_device_api.h"
#include "cl_dev_backend_api.h"
#include "cpu_dev_limits.h"
#include "ImplicitArgument.h"
#include <cassert>
#include <set>
#include <vector>
#include <algorithm>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

  class Binary;

  // Base executable object which knows how to create an internal context
  // The Execute() method is still not implemented
  class Executable : public ICLDevBackendExecutable_
  {
  public:
    Executable(const Binary* pBin);

    virtual ~Executable();

    // Initialize context to with specific number of WorkItems 
    virtual cl_dev_err_code Init(void* *pLocalMemoryBuffers, void* pWGStackFrame, unsigned int uiWICount);

    virtual cl_dev_err_code Execute(const size_t* IN pGroupId,
        const size_t* IN pLocalOffset, 
        const size_t* IN pItemsToProcess);

    // Prepares current thread for the executable execution
    virtual cl_dev_err_code PrepareThread() = 0;

    // Restores Thread state as it was before the execution
    virtual cl_dev_err_code RestoreThreadState() = 0;

    // Releases the context object
    void Release();

    // Returns the executable object which generated this context
    const ICLDevBackendBinary* GetBinary() const {
      return (const ICLDevBackendBinary*)m_pBinary;
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
            assert( false && "not exist event associated with input key" );
        }
        // erase element from set
        m_bIsFirst.erase(uiKey);
        return false;
    }

  protected:
    // TODO : add getter instead
    friend class ImplicitArgsUtils;

    const Binary*   m_pBinary;
    char*           m_pParameters;
    std::vector<ImplicitArgument> m_implicitArgs;
    size_t          m_stParamSize;
    unsigned int    m_CurrWI; // place holder no need to initialize
    size_t          m_GlobalId[CPU_MAX_WI_DIM_POW_OF_2];
    
    unsigned int    m_uiMXCSRstate;   // Stores thread CSR state
    unsigned int    m_uiCSRMask;      // Mask to be applied to set the execution flags  
    unsigned int    m_uiCSRFlags;     // Flags to be set during execution
    bool            m_DAZ;            // Denormals as Zero flag

    // Set for checking if async_wg_copy built-ins in executed workitem should 
    // perform copying or it was already done
    // set stores event numbers (unsigned int) produced by async_wg_copy functions
    // if event is present in this set it means coping has already been done and
    // no need to perform copying within executed workgroup
    std::set<unsigned int> m_bIsFirst;
  };

}}}