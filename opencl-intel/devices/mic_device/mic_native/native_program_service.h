
// Copyright (c) 2006-2008 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

///////////////////////////////////////////////////////////
//  ProgramService.h
//  Implementation of the Class ProgramService
//  Class Object is responsible on programs and kernels binaries
//  Intercats with the compiler backend
///////////////////////////////////////////////////////////

#pragma once

#include <map>
#include <list>
#include <string>

#include "pragmas.h"
#include "cl_synch_objects.h"
#include "thread_local_storage.h"
#include "program_memory_manager.h"
#include "mic_device_interface.h"
#include "cl_dev_backend_api.h"
#include "ICLDevBackendSerializationService.h"

#ifdef USE_ITT
#include <ocl_itt.h>
#endif

using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::UtilsNative;
using namespace Intel::OpenCL::DeviceBackend;
using namespace Intel::OpenCL::MICDevice;

namespace Intel { namespace OpenCL { namespace MICDeviceNative {

// execution memory allocator required by Device Backend
class MICNativeBackendExecMemoryAllocator : public ICLDevBackendJITAllocator
{
public:
    void* AllocateExecutable(size_t size, size_t alignment);
    void FreeExecutable(void* ptr);
};

// class required by Device Backend to specify options
class MICNativeBackendOptions : public ICLDevBackendOptions
{
public:

    // ICLDevBackendOptions interface
    bool GetBooleanValue( int optionId, bool defaultValue) const;
    int GetIntValue( int optionId, int defaultValue) const;
    const char* GetStringValue( int optionId, const char* defaultValue)const;
    bool GetValue( int optionId, void* Value, size_t* pSize) const;

private:
    MICNativeBackendExecMemoryAllocator m_allocator;
};

// ProgramService class
class ProgramService
{
public:
    // return size of filled area in buffer
    size_t getBackendTargetDescription( size_t buffer_size, void* buffer ) const;
    size_t getBackendTargetDescriptionSize( void ) const;

    // add/remove program
    cl_dev_err_code add_program( size_t prog_blob_size, void* prog_blob,  // serialized prog
                    const COPY_PROGRAM_TO_DEVICE_INPUT_STRUCT* prog_info,
                    COPY_PROGRAM_TO_DEVICE_OUTPUT_STRUCT*      fill_kernel_info );

    void remove_program( uint64_t be_program_id );

    // singleton
    static ProgramService& getInstance( void )
    {
        assert( m_gProgramService && "SINK Native ProgramService not initialized" );
        return *m_gProgramService;
    };

    static cl_dev_err_code createProgramService( void );
    static void releaseProgramService( void );

    // called by worker and pipeline threads
    static  const ICLDevBackendKernel_* GetKernelPointer( uint64_t device_ptr) { return ((TKernelEntry*)((size_t)device_ptr))->kernel; }

#ifdef USE_ITT
    static __itt_string_handle* get_itt_kernel_name(uint64_t device_info_ptr);
    static __itt_domain*        get_itt_kernel_domain(uint64_t device_info_ptr);
#endif

private:
    ProgramService();
    ~ProgramService();

    // compiler interfaces. Initialized on the first access
    struct BackendInterfaces {
        ICLDevBackendExecutionService*      pExecutionService;
        ICLDevBackendSerializationService*  pSerializationService;
        MICNativeBackendOptions             m_options;

        // constructor
        BackendInterfaces():  pExecutionService(NULL),
                              pSerializationService(NULL)
                              {};
    };

    struct TKernelEntry
    {
        const ICLDevBackendKernel_* kernel;         // The pointer is not required to be deleted
#ifdef USE_ITT
        __itt_string_handle*  pIttKernelName;   // The value is not required to freed by the ITT
        __itt_domain*         pIttKernelDomain;
#endif

      // constructor
      TKernelEntry( void ) : kernel(NULL)
#ifdef USE_ITT
      , pIttKernelName(NULL)
#endif
      {}
    };

    struct TProgramEntry
    {
        ICLDevBackendProgram_*  pProgram;
        ProgramMemoryManager*   exec_memory_manager;
        TKernelEntry*           pKernels;
    };

    typedef std::map<uint64_t, TProgramEntry*> TProgId2Map;

    BackendInterfaces              m_BE_Executor;

    OclMutex                       m_muProgMap;
    TProgId2Map                    m_ProgId2Map;


    ICLDevBackendExecutionService*      GetExecutionService(void) const
    {
        return m_BE_Executor.pExecutionService;
    }

    ICLDevBackendSerializationService*  GetSerializationService(void) const
    {
        return m_BE_Executor.pSerializationService;
    }

    bool  LoadBackendServices(void);
    void  ReleaseBackendServices(void);

    void  RemoveProgramEntry( TProgramEntry* prog_entry );


    // singleton
    static ProgramService*         m_gProgramService;

};

}}}
