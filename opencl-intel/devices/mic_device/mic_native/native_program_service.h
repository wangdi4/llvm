
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
#include "native_synch_objects.h"
#include "program_memory_manager.h"
#include "mic_device_interface.h"
#include "cl_dev_backend_api.h"
#include "ICLDevBackendSerializationService.h"

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

// kernel printf filler required by Device Backend
class MICNativeBackendPrintfFiller //: public ICLDevBackendPrintf
{
public:
    void  print( const char* buf );
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
    MICNativeBackendPrintfFiller        m_printf;
};

// ProgramService class
class ProgramService
{

public:
    // main API.

    // return size of filled area in buffer
    size_t getBackendTargetDescription( size_t buffer_size, void* buffer ) const;
    size_t getBackendTargetDescriptionSize( void ) const;

    // add/remove program
    void add_program( size_t prog_blob_size, void* prog_blob,  // serialized prog
                      const COPY_PROGRAM_TO_DEVICE_INPUT_STRUCT* prog_info,
                      COPY_PROGRAM_TO_DEVICE_OUTPUT_STRUCT*      fill_kernel_info );

    void remove_program( uint64_t be_program_id );

    // get kernel
    // called by worker and pipeline threads
    bool get_kernel( uint64_t device_info_ptr,
                     const ICLDevBackendKernel_** kernel,
                     ProgramMemoryManager**       program_exec_memory_manager ) const;


	// create binary according to input parameters
	cl_dev_err_code create_binary( const ICLDevBackendKernel_* pKernel, 
		                           char* pLockedParams, 
								   uint64_t argSize,
								   cl_work_description_type* pWorkDesc,
								   ICLDevBackendBinary_** ppOutBinary ) const;

    // singleton
    static ProgramService& getInstance( void )
    {
        assert( m_gProgramService && "SINK Native ProgramService not initialized" );
        return *m_gProgramService;
    };

    static void createProgramService( void );
    static void releaseProgramService( void );

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

    struct TProgramEntry;
    struct TKernelEntry
    {
        static const uint64_t       marker_value = 0xBEAFF00D;
        uint64_t                    marker; // set it to the marker_value at contructor

        const ICLDevBackendKernel_* kernel;
        TProgramEntry*              program_entry; // back link to the owning program entry

        // constructor
        TKernelEntry( void ) : marker( marker_value ) {};
    };

    typedef std::list<TKernelEntry*> TKernelList;

    struct TProgramEntry
    {
        ICLDevBackendProgram_*   pProgram;
        ProgramMemoryManager*   exec_memory_manager;
        TKernelList             kernels;
    };

    typedef std::map<uint64_t, TProgramEntry*> TProgId2Map;

    BackendInterfaces              m_BE_Executor;

    OclMutexNative                 m_muProgMap;
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
