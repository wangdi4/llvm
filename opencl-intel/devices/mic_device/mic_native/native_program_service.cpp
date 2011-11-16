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
//  ProgramService.cpp
//  Implementation of the Class ProgramService
//  Class Object is responsible on programs and kernels binaries
//  Intercats with the compiler backend
///////////////////////////////////////////////////////////

#include "pragmas.h"
#include "native_program_service.h"
#include "native_common_macros.h"
#include "thread_local_storage.h"
#include <cl_dev_backend_api.h>

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <assert.h>
#include <math.h>
#include <string.h>

#include <sink/COIPipeline_sink.h>

using namespace Intel::OpenCL::MICDeviceNative;
using namespace Intel::OpenCL::DeviceBackend;

// init singleton
ProgramService* ProgramService::m_gProgramService = NULL;

void ProgramService::createProgramService( void )
{
    m_gProgramService = new ProgramService();
    assert( m_gProgramService && "SINK: Cannot create ProgramService" );
}

void ProgramService::releaseProgramService( void )
{
    if (m_gProgramService)
    {
        delete m_gProgramService;
        m_gProgramService = NULL;
    }
}

ProgramService::ProgramService()
{
    LoadBackendServices();

    NATIVE_PRINTF("Program Service - Created\n");
}

ProgramService::~ProgramService()
{
    // delete all programs
    TProgId2Map::iterator prog_it     = m_ProgId2Map.begin();
    TProgId2Map::iterator prog_it_end = m_ProgId2Map.end();

    for(; prog_it != prog_it_end; ++prog_it)
    {
        RemoveProgramEntry( prog_it->second );
    }

    m_ProgId2Map.clear();
    ReleaseBackendServices();

    NATIVE_PRINTF("Program Service - Destructed\n");
}

/****************************************************************************************************************
 BEGIN MICNativeBackendExecMemoryAllocator
********************************************************************************************************************/
void* MICNativeBackendExecMemoryAllocator::AllocateExecutable(size_t size, size_t alignment)
{
    TlsAccessor tls;
    ProgramMemoryManager* mem = tls.get_program_memory_manager();

    assert( NULL != mem && "SINK: Execution memory allocator called while TLS is not set" );

    if (NULL == mem)
    {
        NATIVE_PRINTF("SINK: Execution memory allocator called while TLS is not set\n");
        return NULL;
    }

    void* buf;
    bool ok = mem->allocate( size, &buf );
    assert( ok && "SINK: Try to allocate more executable memory for program than reserved" );

    if (!ok)
    {
        NATIVE_PRINTF("SINK: Try to allocate more executable memory for program than reserved\n");
        return NULL;
    }

    return buf;
}

void MICNativeBackendExecMemoryAllocator::FreeExecutable(void* ptr)
{
    TlsAccessor tls;
    ProgramMemoryManager* mem = tls.get_program_memory_manager();

    assert( NULL != mem && "SINK: Execution memory de-allocator called while TLS is not set" );

    if (NULL == mem)
    {
        NATIVE_PRINTF("SINK: Execution memory de-allocator called while TLS is not set\n");
        return;
    }

    bool ok = mem->free( ptr );
    assert( ok && "SINK: Try to free no-allocated executable memory for program" );

    if (!ok)
    {
        NATIVE_PRINTF("SINK: Try to free no-allocated executable memory for program: %p\n", ptr );
    }

}

/****************************************************************************************************************
 END   MICNativeBackendExecMemoryAllocator
********************************************************************************************************************/

/****************************************************************************************************************
 BEGIN MICNativeBackendPrintfFiller
********************************************************************************************************************/
void MICNativeBackendPrintfFiller::print( const char* buf )
{
}

/****************************************************************************************************************
 END MICNativeBackendPrintfFiller
********************************************************************************************************************/

/****************************************************************************************************************
 BEGIN MICNativeBackendOptions
********************************************************************************************************************/

// ICLDevBackendOptions interface
// BUGBUG: DK remove this!
#define CL_DEV_BACKEND_OPTION_PRINTF     2

bool MICNativeBackendOptions::GetBooleanValue( int optionId, bool defaultValue) const
{
    return defaultValue;
}

int MICNativeBackendOptions::GetIntValue( int optionId, int defaultValue) const
{
    return defaultValue;
}

const char* MICNativeBackendOptions::GetStringValue( int optionId, const char* defaultValue)const
{
    return defaultValue;
}

bool MICNativeBackendOptions::GetValue( int optionId, void* Value, size_t* pSize) const
{
    if (NULL == Value || NULL == pSize || sizeof(void*) != *pSize)
    {
        return false;
    }

    switch (optionId)
    {
        case CL_DEV_BACKEND_OPTION_JIT_ALLOCATOR:
            *(void**)Value = (void*)(&m_allocator);
            return false;

        case CL_DEV_BACKEND_OPTION_PRINTF:
            *(void**)Value = NULL;
            return false;

        default:
            return false;
    }
}

/****************************************************************************************************************
 END MICNativeBackendOptions
********************************************************************************************************************/

bool ProgramService::LoadBackendServices(void)
{
    cl_dev_err_code err;

    err = InitDeviceBackend( NULL );

    assert( CL_DEV_SUCCEEDED(err) && "SINK: InitDeviceBackend(NULL)" );

    // load Backend Compiler
    ICLDevBackendServiceFactory* be_factory = GetDeviceBackendFactory();

    if (NULL == be_factory)
    {
        assert( false && "SINK: MIC Device cannot create BE Factory" );
        return false;
    }

    err = be_factory->GetExecutionService( &m_BE_Executor.m_options, &m_BE_Executor.pExecutionService );

    if (CL_DEV_FAILED( err ))
    {
        assert( false && "SINK: MIC Device cannot create Backend Execution Service" );
        return false;
    }

    err = be_factory->GetSerializationService( &m_BE_Executor.m_options, &m_BE_Executor.pSerializationService );

    if (CL_DEV_FAILED( err ))
    {
        assert( false && "SINK: MIC Device  cannot create Backend Serialization Service" );

        m_BE_Executor.pExecutionService->Release();
        m_BE_Executor.pExecutionService = NULL;
        return false;
    }

    return true;
}

void ProgramService::ReleaseBackendServices(void)
{
    if (m_BE_Executor.pSerializationService)
    {
        m_BE_Executor.pSerializationService->Release();
        m_BE_Executor.pSerializationService = NULL;
    }

    if (m_BE_Executor.pExecutionService)
    {
        m_BE_Executor.pExecutionService->Release();
        m_BE_Executor.pExecutionService = NULL;
    }

    TerminateDeviceBackend();
}

//
// get Backend Options
//
size_t ProgramService::getBackendTargetDescriptionSize( void ) const
{
    return GetExecutionService()->GetTargetMachineDescriptionSize();
}

size_t ProgramService::getBackendTargetDescription( size_t buffer_size, void* buffer ) const
{
    ICLDevBackendExecutionService* service = GetExecutionService();

    size_t required_size = service->GetTargetMachineDescriptionSize();

    if (required_size > buffer_size)
    {
        NATIVE_PRINTF( "TargetMachineDescriptionSize is %u bytes, while passed only %u bytes\n",
                        (unsigned int)required_size, (unsigned int)buffer_size );

        return 0;
    }

    cl_dev_err_code err = service->GetTargetMachineDescription( buffer, buffer_size );

    if (CL_DEV_FAILED( err ))
    {
        NATIVE_PRINTF( "GetTargetMachineDescription() returned %u error\n", (unsigned int)err );
        return 0;
    }

    return required_size;
}

//
// Deserialize program and create kernels
//
void ProgramService::add_program(
                      size_t prog_blob_size, void* prog_blob,  // serialized prog
                      const COPY_PROGRAM_TO_DEVICE_INPUT_STRUCT* prog_info,
                      COPY_PROGRAM_TO_DEVICE_OUTPUT_STRUCT*      fill_kernel_info )
{
    // initialize output
    fill_kernel_info->filled_kernels = 0;

    TProgramEntry* prog_entry = new TProgramEntry;

    if (NULL == prog_entry)
    {
        NATIVE_PRINTF("ProgramService::add_program: Cannot allocate TProgramEntry" );
        return;
    }

    // 1. Reserve execution memory
    if (prog_info->required_executable_size)
    {
        prog_entry->exec_memory_manager = new ProgramMemoryManager();
        if (! prog_entry->exec_memory_manager->reserveExecutableMemory(prog_info->required_executable_size, 1))
        {
            NATIVE_PRINTF("ProgramService::add_program: Cannot reserve %lu executable bytes for passed program\n",prog_info->required_executable_size );

            delete prog_entry;
            return;
        }
    }
    else
    {
        prog_entry->exec_memory_manager = NULL;
    }

    // setup TLS with execution memory allocator
    TlsContainer tls;
    tls.set_program_memory_manager( prog_entry->exec_memory_manager );

    // 2. Deserialize program
    cl_dev_err_code be_err = GetSerializationService()->DeSerializeProgram( &(prog_entry->pProgram) , prog_blob, prog_blob_size );

    if (CL_DEV_FAILED(be_err))
    {
        prog_entry->pProgram = NULL;

        NATIVE_PRINTF("ProgramService::add_program: Cannot deserialize program with blob size %#lX bytes\n", prog_blob_size );

        RemoveProgramEntry( prog_entry );
        return;
    }

    // 3. Create kernels list and fill output data
    int kernels_count = prog_entry->pProgram->GetKernelsCount();
    assert( kernels_count >= 0 && "ProgramService::add_program: Cannot restore kernels from deserialized program" );
    assert( kernels_count == prog_info->number_of_kernels && "ProgramService::add_program: Backend restored kernel count differ from serialized" );

    bool error_found = false;

    for (int i = 0; i < kernels_count; ++i)
    {
        TKernelEntry* k_entry = new TKernelEntry;
        assert( k_entry && "ProgramService::add_program: No memory to allocate kernel entry struct" );

        if (NULL == k_entry)
        {
            NATIVE_PRINTF("ProgramService::add_program: Cannot allocate TKernelEntry" );
            error_found = true;
            break;
        }

        k_entry->program_entry = prog_entry;
        be_err = prog_entry->pProgram->GetKernel(i, &(k_entry->kernel) );
        assert( CL_DEV_SUCCEEDED(be_err) && "ProgramService::add_program: Cannot get kernel from deserialized program" );

        // add kernel entry to the prog list
        prog_entry->kernels.push_back( k_entry );

        // add kernel info to the return struct
        fill_kernel_info->device_kernel_info_pts[i].kernel_id        = (uint64_t)k_entry->kernel->GetKernelID();
        fill_kernel_info->device_kernel_info_pts[i].device_info_ptr  = (uint64_t)(size_t)k_entry;
    }

    if (error_found)
    {
        RemoveProgramEntry( prog_entry );
        return;
    }

    // 4. Add program to the program map
    m_muProgMap.Lock();
    m_ProgId2Map[ prog_info->uid_program_on_device ] = prog_entry;
    m_muProgMap.Unlock();

    // 5. Set number of filled kernels
    fill_kernel_info->filled_kernels = kernels_count;
}

void ProgramService::remove_program( uint64_t be_program_id )
{
    TProgramEntry* prog_entry;

    m_muProgMap.Lock();

    TProgId2Map::iterator it = m_ProgId2Map.find( be_program_id );
    if (it == m_ProgId2Map.end())
    {
        m_muProgMap.Unlock();

        // no program to remove
        NATIVE_PRINTF("ProgramService::remove_program: Program with backend ID %lu not found\n", be_program_id );
        return;
    }

    prog_entry = it->second;
    m_ProgId2Map.erase( it );

    m_muProgMap.Unlock();

    RemoveProgramEntry( prog_entry );
}

void  ProgramService::RemoveProgramEntry( TProgramEntry* prog_entry )
{
    TKernelList::iterator it     = prog_entry->kernels.begin();
    TKernelList::iterator it_end = prog_entry->kernels.end();

    for(; it != it_end; ++it)
    {
        delete *it;
    }

    prog_entry->kernels.clear();

    if (prog_entry->pProgram)
    {
        // setup TLS with execution memory allocator
        TlsContainer tls;
        tls.set_program_memory_manager( prog_entry->exec_memory_manager );

        prog_entry->pProgram->Release();
    }

    if (prog_entry->exec_memory_manager)
    {
        delete prog_entry->exec_memory_manager;
    }

    delete prog_entry;
}

// get kernel
// called by worker and pipeline threads
bool ProgramService::get_kernel(
                 uint64_t device_info_ptr,
                 const ICLDevBackendKernel_** kernel,
                 ProgramMemoryManager**       program_exec_memory_manager ) const
{
    if (0 == device_info_ptr)
    {
        return false;
    }

    TKernelEntry* k_entry = (TKernelEntry*)(size_t)device_info_ptr;

    // try to touch
    if (TKernelEntry::marker_value != k_entry->marker)
    {
        // error
        assert( false && "SINK: ProgramService::get_kernel: wrong device kernel info pointer passed" );
        NATIVE_PRINTF("ProgramService::get_kernel: wrong device kernel info pointer passed: %lu\n", device_info_ptr );
        return false;
    }

    if (kernel)
    {
        *kernel = k_entry->kernel;
    }

    if (program_exec_memory_manager)
    {
        *program_exec_memory_manager = k_entry->program_entry->exec_memory_manager;
    }

    return true;
}

cl_dev_err_code ProgramService::create_binary( const ICLDevBackendKernel_* pKernel,
		                           char* pLockedParams,
								   uint64_t argSize,
								   cl_work_description_type* pWorkDesc,
								   ICLDevBackendBinary_** ppOutBinary ) const
{
	return GetExecutionService()->CreateBinary(pKernel, pLockedParams, argSize, pWorkDesc, ppOutBinary);
}


////////////////////////////////////////////////////////////////////////////////
//
// Interface to the HOST
//
////////////////////////////////////////////////////////////////////////////////

//
// get_backend_target_description_size
//  Receives:
//      p-return-value   : filled data length as uint64_t
//
//  On any error return 0 as filled data length
//
COINATIVELIBEXPORT
void get_backend_target_description_size(
              uint32_t         in_BufferCount,
              void**           in_ppBufferPointers,
              uint64_t*        in_pBufferLengths,
              void*            in_pMiscData,
              uint16_t         in_MiscDataLength,
              void*            in_pReturnValue,
              uint16_t         in_ReturnValueLength)
{
    assert( 0 == in_BufferCount && "SINK: get_backend_target_description_size() should receive 0 buffers" );
    assert( sizeof(uint64_t) == in_ReturnValueLength && "SINK: get_backend_target_description_size() should receive return-value as uint64_t" );

    ProgramService& program_service = ProgramService::getInstance();

    size_t filled_size = program_service.getBackendTargetDescriptionSize();

    *((uint64_t*)in_pReturnValue) = (uint64_t)filled_size;
}


//
// get_backend_target_description
//  Receives:
//      Number of buffers: 1 - write-only buffer to contain output data
//      p-return-value   : filled data length as uint64_t
//
//  On any error return 0 as filled data length
//
COINATIVELIBEXPORT
void get_backend_target_description(
              uint32_t         in_BufferCount,
              void**           in_ppBufferPointers,
              uint64_t*        in_pBufferLengths,
              void*            in_pMiscData,
              uint16_t         in_MiscDataLength,
              void*            in_pReturnValue,
              uint16_t         in_ReturnValueLength)
{
    assert( 1 == in_BufferCount && "SINK: get_backend_target_description() should receive 1 buffer" );
    assert( sizeof(uint64_t) == in_ReturnValueLength && "SINK: get_backend_target_description() should receive return-value as uint64_t" );

    size_t filled_size = 0;
    ProgramService& program_service = ProgramService::getInstance();

    filled_size = program_service.getBackendTargetDescription( (size_t)in_pBufferLengths[0], in_ppBufferPointers[0] );

    *((uint64_t*)in_pReturnValue) = (uint64_t)filled_size;
}

//
// copy_program_to_device
//  Receives:
//   Buffers:
//       buffer1 - normal buffer with serialized program [IN]
//       buffer2 - normal buffer with COPY_PROGRAM_TO_DEVICE_OUTPUT_STRUCT [OUT]
//   MiscData
//       input - COPY_PROGRAM_TO_DEVICE_INPUT_STRUCT
//       output - none
//
COINATIVELIBEXPORT
void copy_program_to_device(
              uint32_t         in_BufferCount,
              void**           in_ppBufferPointers,
              uint64_t*        in_pBufferLengths,
              void*            in_pMiscData,
              uint16_t         in_MiscDataLength,
              void*            in_pReturnValue,
              uint16_t         in_ReturnValueLength)
{
    assert( 2 == in_BufferCount && "SINK: copy_program_to_device() should receive 2 buffers" );
    assert( sizeof(COPY_PROGRAM_TO_DEVICE_INPUT_STRUCT) == in_MiscDataLength &&
                    "SINK: copy_program_to_device() should receive input misc data  as COPY_PROGRAM_TO_DEVICE_INPUT_STRUCT" );

    COPY_PROGRAM_TO_DEVICE_INPUT_STRUCT* input = (COPY_PROGRAM_TO_DEVICE_INPUT_STRUCT*)in_pMiscData;

    // calculate required size of COPY_PROGRAM_TO_DEVICE_OUTPUT_STRUCT
    assert( COPY_PROGRAM_TO_DEVICE_OUTPUT_STRUCT_SIZE( input->number_of_kernels ) == in_pBufferLengths[1] &&
                    "SINK: copy_program_to_device() should COPY_PROGRAM_TO_DEVICE_OUTPUT_STRUCT of right length" );

    COPY_PROGRAM_TO_DEVICE_OUTPUT_STRUCT* output = (COPY_PROGRAM_TO_DEVICE_OUTPUT_STRUCT*)in_ppBufferPointers[1];

    // check that serialized program size is valid
    size_t blob_size = in_pBufferLengths[0];
    assert( blob_size > 0 && "SINK: copy_program_to_device() should get non-zero serialized program" );

    ProgramService& program_service = ProgramService::getInstance();

    program_service.add_program( blob_size, in_ppBufferPointers[0], input, output );
}

//
// remove_program_from_device
//  Receives:
//      inpuit misc   : program backend ID as uint64_t
//
COINATIVELIBEXPORT
void remove_program_from_device(
              uint32_t         in_BufferCount,
              void**           in_ppBufferPointers,
              uint64_t*        in_pBufferLengths,
              void*            in_pMiscData,
              uint16_t         in_MiscDataLength,
              void*            in_pReturnValue,
              uint16_t         in_ReturnValueLength)
{
    assert( 0 == in_BufferCount && "SINK: remove_program_from_device() should receive 0 buffers" );
    assert( sizeof(uint64_t) == in_MiscDataLength && "SINK: remove_program_from_device() should receive program backend ID as uint64_t misc input" );

    uint64_t be_program_id = *((uint64_t*)in_pMiscData);

    ProgramService& program_service = ProgramService::getInstance();

    program_service.remove_program( be_program_id );
}



