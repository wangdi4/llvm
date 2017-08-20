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
#include "native_globals.h"

#include <cl_dev_backend_api.h>
#include <builtin_kernels.h>

#ifdef __INCLUDE_MKL__
#include <mkl_builtins.h>
#endif

#include "mic_tracer.h"

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
ProgramService* ProgramService::m_gProgramService = nullptr;

cl_dev_err_code ProgramService::createProgramService( void )
{
    m_gProgramService = new ProgramService();
    assert( m_gProgramService && "SINK: Cannot create ProgramService" );
    if ( nullptr == m_gProgramService )
    {
    	return CL_DEV_OUT_OF_MEMORY;
    }
    
    return CL_DEV_SUCCESS;
}

void ProgramService::releaseProgramService( void )
{
    if (m_gProgramService)
    {
        delete m_gProgramService;
        m_gProgramService = nullptr;
    }
}

ProgramService::ProgramService()
{
    LoadBackendServices();

    NATIVE_PRINTF("Program Service - Created\n");
}

ProgramService::~ProgramService()
{
    ReleaseBackendServices();

    NATIVE_PRINTF("Program Service - Destructed\n");
}

/****************************************************************************************************************
 BEGIN MICNativeBackendExecMemoryAllocator
********************************************************************************************************************/
void* MICNativeBackendExecMemoryAllocator::AllocateExecutable(size_t size, size_t alignment)
{
    TlsAccessor tlsAccessor;
	ProgramServiceTls tls(&tlsAccessor);
    ProgramMemoryManager* mem = (ProgramMemoryManager*)tls.getTls(ProgramServiceTls::PROGRAM_MEMORY_MANAGER);

    assert( nullptr != mem && "SINK: Execution memory allocator called while TLS is not set" );

    if (nullptr == mem)
    {
        NATIVE_PRINTF("SINK: Execution memory allocator called while TLS is not set\n");
        return nullptr;
    }

    void* buf;
    bool ok = mem->allocate( size, &buf );
    assert( ok && "SINK: Try to allocate more executable memory for program than reserved" );

    if (!ok)
    {
        NATIVE_PRINTF("SINK: Try to allocate more executable memory for program than reserved\n");
        return nullptr;
    }

    return buf;
}

void MICNativeBackendExecMemoryAllocator::FreeExecutable(void* ptr)
{
    TlsAccessor tlsAccessor;
	ProgramServiceTls tls(&tlsAccessor);
    ProgramMemoryManager* mem = (ProgramMemoryManager*)tls.getTls(ProgramServiceTls::PROGRAM_MEMORY_MANAGER);

    assert( nullptr != mem && "SINK: Execution memory de-allocator called while TLS is not set" );

    if (nullptr == mem)
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
 BEGIN MICNativeBackendOptions
********************************************************************************************************************/

// ICLDevBackendOptions interface
// BUGBUG: DK remove this!
#define CL_DEV_BACKEND_OPTION_PRINTF     20000

bool MICNativeBackendOptions::GetBooleanValue( int optionId, bool defaultValue) const
{
    return (CL_DEV_BACKEND_OPTION_USE_VTUNE == optionId) ? gMicExecEnvOptions.use_vtune : defaultValue;
}

int MICNativeBackendOptions::GetIntValue( int optionId, int defaultValue) const
{
    return defaultValue;
}

const char* MICNativeBackendOptions::GetStringValue( int optionId, const char* defaultValue)const
{
    switch (optionId)
    {
        case CL_DEV_BACKEND_OPTION_DEVICE:
            return "mic";

        case CL_DEV_BACKEND_OPTION_SUBDEVICE:
            return gMicExecEnvOptions.mic_cpu_arch_str;

        //case CL_DEV_BACKEND_OPTION_DUMPFILE:
        //    assert(false && "OPTION NOT SUPPORTED ON DEVICE SIDE");

        default:
            return defaultValue;
    }
}

bool MICNativeBackendOptions::GetValue( int optionId, void* Value, size_t* pSize) const
{
    if (nullptr == Value || nullptr == pSize || sizeof(void*) != *pSize)
    {
        return false;
    }

    switch (optionId)
    {
        case CL_DEV_BACKEND_OPTION_JIT_ALLOCATOR:
            *(void**)Value = (void*)(&m_allocator);
            return true;

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

    err = InitDeviceBackend( nullptr );

    assert( CL_DEV_SUCCEEDED(err) && "SINK: InitDeviceBackend(NULL)" );

    // load Backend Compiler
    ICLDevBackendServiceFactory* be_factory = GetDeviceBackendFactory();

    if (nullptr == be_factory)
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
        m_BE_Executor.pExecutionService = nullptr;
        return false;
    }

    return true;
}

void ProgramService::ReleaseBackendServices(void)
{
    if (m_BE_Executor.pSerializationService)
    {
        m_BE_Executor.pSerializationService->Release();
        m_BE_Executor.pSerializationService = nullptr;
    }

    if (m_BE_Executor.pExecutionService)
    {
        m_BE_Executor.pExecutionService->Release();
        m_BE_Executor.pExecutionService = nullptr;
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

cl_dev_err_code ProgramService::fill_program_info(TProgramEntry* prog_entry, COPY_PROGRAM_TO_DEVICE_OUTPUT_STRUCT* fill_kernel_info)
{
    // 3. Create kernels list and fill output data
    int kernels_count = prog_entry->pProgram->GetKernelsCount();
    assert( kernels_count >= 0 && "ProgramService::add_program: Cannot restore kernels from deserialized program" );

    prog_entry->pKernels = new TKernelEntry[kernels_count];
    if ( nullptr == prog_entry->pKernels )
    {
        NATIVE_PRINTF("ProgramService::add_program: Cannot allocate TKernelEntry[kernels_count], kernels_count=%d", kernels_count);
        return CL_DEV_OUT_OF_MEMORY;
    }

    for (int i = 0; i < kernels_count; ++i)
    {
        cl_dev_err_code be_err = prog_entry->pProgram->GetKernel(i, &(prog_entry->pKernels[i].kernel) );
        assert( CL_DEV_SUCCEEDED(be_err) && "ProgramService::add_program: Cannot get kernel from de-serialized program" );
        if ( CL_DEV_FAILED(be_err) )
        {
          break;
        }

#ifdef USE_ITT
        if ( gMicGPAData.bUseGPA)
        {
            static const char kernelDomainNamePrefix[] = "com.intel.opencl.device.mic.";
            static const size_t maxKernelDomainLenght = 256;

            const char* kernelName = prog_entry->pKernels[i].kernel->GetKernelName();

            prog_entry->pKernels[i].pIttKernelName = __itt_string_handle_create(kernelName);

            // Create private domain per kernel
            if ( strlen(kernelDomainNamePrefix)+strlen(kernelName) < maxKernelDomainLenght)
            {
              char kernelDomainName[maxKernelDomainLenght];
              sprintf(kernelDomainName, "%s%s", kernelDomainNamePrefix, kernelName);
              prog_entry->pKernels[i].pIttKernelDomain = __itt_domain_create(kernelDomainName);
            }
            else
            {
              prog_entry->pKernels[i].pIttKernelDomain = nullptr;
            }
        }
#endif
        // add kernel info to the return struct
        fill_kernel_info->device_kernel_info_pts[i].device_info_ptr  = (uint64_t)(size_t)&prog_entry->pKernels[i];
    }

    // 5. Set number of filled kernels
    fill_kernel_info->uid_program_on_device = (uint64_t)prog_entry;
    fill_kernel_info->filled_kernels = kernels_count;
    
    return CL_DEV_SUCCESS;
}

//
// Deserialize program and create kernels
//
cl_dev_err_code ProgramService::add_program(
                      size_t prog_blob_size, void* prog_blob,  // serialized prog
                      const COPY_PROGRAM_TO_DEVICE_INPUT_STRUCT* prog_info,
                      COPY_PROGRAM_TO_DEVICE_OUTPUT_STRUCT*      fill_kernel_info )
{
    // initialize output
    fill_kernel_info->filled_kernels = 0;

    TProgramEntry* prog_entry = new TProgramEntry;

    if (nullptr == prog_entry)
    {
        NATIVE_PRINTF("ProgramService::add_program: Cannot allocate TProgramEntry" );
        return CL_DEV_OUT_OF_MEMORY;
    }

    // 1. Reserve execution memory
    if (prog_info->required_executable_size || prog_blob_size)
    {
        size_t required_exec_size = MAX( prog_info->required_executable_size, prog_blob_size );
        //required_exec_size = ALIGN_UP(required_exec_size, PAGE_4K_SIZE)
        prog_entry->exec_memory_manager = new ProgramMemoryManager();
        if (! prog_entry->exec_memory_manager->reserveExecutableMemory(required_exec_size, 1))
        {
            NATIVE_PRINTF("ProgramService::add_program: Cannot reserve %lu executable bytes for passed program\n",required_exec_size );
            delete prog_entry;
            return CL_DEV_OUT_OF_MEMORY;
        }
    }
    else
    {
        prog_entry->exec_memory_manager = nullptr;
    }

    // setup TLS with execution memory allocator, which is required by DeSerialize
    TlsAccessor tlsAccessor;
    ProgramServiceTls tls(&tlsAccessor);
    tls.setTls(ProgramServiceTls::PROGRAM_MEMORY_MANAGER, prog_entry->exec_memory_manager);

    // 2. Deserialize program
    cl_dev_err_code be_err = GetSerializationService()->DeSerializeProgram(SERIALIZE_OFFLOAD_IMAGE, &(prog_entry->pProgram) , prog_blob, prog_blob_size );

    if (CL_DEV_FAILED(be_err))
    {
        prog_entry->pProgram = nullptr;

        NATIVE_PRINTF("ProgramService::add_program: Cannot deserialize program with blob size %#lX bytes\n", prog_blob_size );

        RemoveProgramEntry( prog_entry );
        return be_err;
    }

    // clean TLS
    tls.setTls(ProgramServiceTls::PROGRAM_MEMORY_MANAGER, nullptr);

    be_err = fill_program_info(prog_entry, fill_kernel_info);

    if ( CL_DEV_FAILED(be_err) )
    {
        RemoveProgramEntry( prog_entry );
        return be_err;
    }

    assert( fill_kernel_info->filled_kernels == prog_info->number_of_kernels && "ProgramService::add_program: Backend restored kernel count differ from serialized" );

    return CL_DEV_SUCCESS;
}

cl_dev_err_code ProgramService::add_builtin_program(const char* szBuiltInNames, COPY_PROGRAM_TO_DEVICE_OUTPUT_STRUCT* fill_kernel_info)
{
    NATIVE_PRINTF("add_builtin_program: %s\n", szBuiltInNames);
    // initialize output
    fill_kernel_info->filled_kernels = 0;

    TProgramEntry* prog_entry = new TProgramEntry;
    if (nullptr == prog_entry)
    {
        NATIVE_PRINTF("ProgramService::add_program: Cannot allocate TProgramEntry" );
        return CL_DEV_OUT_OF_MEMORY;
    }

    ICLDevBackendProgram_* pProg;
    cl_dev_err_code err = BuiltInKernels::BuiltInKernelRegistry::GetInstance()->CreateBuiltInProgram(szBuiltInNames, &pProg);
    if ( CL_DEV_FAILED(err) )
    {
        NATIVE_PRINTF("CreateBuiltInProgram failed with %x", err);
        delete prog_entry;
        return err;
    }

    prog_entry->exec_memory_manager = nullptr;
    prog_entry->pProgram = pProg;
    err = fill_program_info(prog_entry, fill_kernel_info);
    if ( CL_DEV_FAILED(err) )
    {
        RemoveProgramEntry( prog_entry );
        delete pProg;
        fill_kernel_info->filled_kernels = 0;
    }

    NATIVE_PRINTF("add_builtin_program - DONE\n");
    return err;
}

void ProgramService::remove_program( uint64_t be_program_id )
{
    TProgramEntry* prog_entry = (TProgramEntry*)be_program_id;

    RemoveProgramEntry( prog_entry );
}

void  ProgramService::RemoveProgramEntry( TProgramEntry* prog_entry )
{
    if ( nullptr != prog_entry->exec_memory_manager)
    {
        if ( nullptr != prog_entry->pProgram )
        {
            // setup TLS with execution memory allocator
            TlsAccessor tlsAccessor;
            ProgramServiceTls tls(&tlsAccessor);
            tls.setTls(ProgramServiceTls::PROGRAM_MEMORY_MANAGER, prog_entry->exec_memory_manager);

            GetSerializationService()->ReleaseProgram(prog_entry->pProgram);

            // clean TLS
            tls.setTls(ProgramServiceTls::PROGRAM_MEMORY_MANAGER, nullptr);
            prog_entry->pProgram = nullptr;
        }
        delete prog_entry->exec_memory_manager;
        prog_entry->exec_memory_manager = nullptr;
    }
    else
    {
        if ( nullptr != prog_entry->pProgram )
        {
            delete prog_entry->pProgram;
            prog_entry->pProgram = nullptr;
        }
    }

    if ( nullptr != prog_entry->pKernels )
    {
        delete []prog_entry->pKernels;
        prog_entry->pKernels = nullptr;
    }

    delete prog_entry;
}

#ifdef USE_ITT
__itt_string_handle* ProgramService::get_itt_kernel_name(uint64_t device_info_ptr)
{
    assert( 0 != device_info_ptr && "Invalid arguments passed");
    if ( 0 == device_info_ptr )
    {
        return nullptr;
    }

    TKernelEntry* k_entry = (TKernelEntry*)(size_t)device_info_ptr;
    return k_entry->pIttKernelName;
}

__itt_domain* ProgramService::get_itt_kernel_domain(uint64_t device_info_ptr)
{
    assert( 0 != device_info_ptr && "Invalid arguments passed");
    if ( 0 == device_info_ptr )
    {
        return nullptr;
    }

    TKernelEntry* k_entry = (TKernelEntry*)(size_t)device_info_ptr;
    return k_entry->pIttKernelDomain;
}
#endif

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

#ifdef ENABLE_MIC_TRACER
    CommandTracer cmdTracer;
    cmdTracer.set_command_id(input->uid_program_on_device);
    cmdTracer.set_current_time_build_deserialize_time_start();
#endif
    // check that serialized program size is valid
    size_t blob_size = in_pBufferLengths[0];
    assert( blob_size > 0 && "SINK: copy_program_to_device() should get non-zero serialized program" );

    ProgramService& program_service = ProgramService::getInstance();

    cl_dev_err_code err = program_service.add_program( blob_size, in_ppBufferPointers[0], input, output );
    if ( in_ReturnValueLength > 0 )
    {
        *((cl_dev_err_code*)in_pReturnValue) = err;
    }

#ifdef ENABLE_MIC_TRACER
    cmdTracer.set_current_time_build_deserialize_time_end();
#endif
}

// Create a program from a list of built-in kernel
COINATIVELIBEXPORT
void create_built_in_program(
              uint32_t         in_BufferCount,
              void**           in_ppBufferPointers,
              uint64_t*        in_pBufferLengths,
              void*            in_pMiscData,
              uint16_t         in_MiscDataLength,
              void*            in_pReturnValue,
              uint16_t         in_ReturnValueLength)
{
    assert ( in_BufferCount == 0 && "This function doesn't expect any buffer");
    assert ( in_MiscDataLength > 0 && "This function expects input MISC data");
    assert ( in_ReturnValueLength > 0 && "This function should return value");

    COPY_PROGRAM_TO_DEVICE_OUTPUT_STRUCT* output = (COPY_PROGRAM_TO_DEVICE_OUTPUT_STRUCT*)in_pReturnValue;

    ProgramService& program_service = ProgramService::getInstance();
    program_service.add_builtin_program((const char*)in_pMiscData, output);
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



