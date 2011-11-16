
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
#include <string>

#include "cl_device_api.h"
#include "handle_allocator.h"
#include "cl_synch_objects.h"
#include "mic_config.h"
#include "cl_dev_backend_api.h"
#include "backend_wrapper.h"

using namespace Intel::OpenCL::DeviceBackend;

namespace Intel { namespace OpenCL { namespace MICDevice {
    // forward declaration
    struct COPY_PROGRAM_TO_DEVICE_INPUT_STRUCT;
    struct COPY_PROGRAM_TO_DEVICE_OUTPUT_STRUCT;
}}}

namespace Intel { namespace OpenCL { namespace MICDevice {

// forward declarations
class DeviceServiceCommunication;

// class required by Device Backend to specify options
class MICBackendOptions : public ICLDevBackendOptions
{
public:
    MICBackendOptions( DeviceServiceCommunication& dev_service );

    void init( bool bUseVectorizer, bool bUseVtune );

    // ICLDevBackendOptions interface
    bool GetBooleanValue( int optionId, bool defaultValue) const;
    int GetIntValue( int optionId, int defaultValue) const;
    const char* GetStringValue( int optionId, const char* defaultValue)const;
    bool GetValue( int optionId, void* Value, size_t* pSize) const;

private:
    DeviceServiceCommunication& m_dev_service;
    bool                        m_bUseVectorizer;
    bool                        m_bUseVtune;

    int getTargetDescriptionSize( void ) const;
    bool getTargetDescription( void* Value, size_t* pSize) const;
};


class ProgramService
{

public:
    ProgramService(cl_int devId, IOCLFrameworkCallbacks *devCallbacks, IOCLDevLogDescriptor *logDesc, MICDeviceConfig *config,
                    DeviceServiceCommunication& dev_service );
    virtual ~ProgramService();

    bool Init( void );

    cl_dev_err_code CheckProgramBinary (size_t IN bin_size, const void* IN bin);
    cl_dev_err_code CreateProgram( size_t IN binSize,
                                        const void* IN bin,
                                        cl_dev_binary_prop IN prop,
                                        cl_dev_program* OUT prog
                                       );
    cl_dev_err_code BuildProgram( cl_dev_program IN prog,
                                        const char* IN options,
                                        void* IN userData
                                       );
    cl_dev_err_code ReleaseProgram( cl_dev_program IN prog );
    cl_dev_err_code UnloadCompiler();
    cl_dev_err_code GetProgramBinary( cl_dev_program IN prog,
                                        size_t IN size,
                                        void* OUT binary,
                                        size_t* OUT sizeRet
                                        );

    cl_dev_err_code GetBuildLog( cl_dev_program IN prog,
                                        size_t IN size,
                                        char* OUT log,
                                        size_t* OUT sizeRet
                                      );
    cl_dev_err_code GetSupportedBinaries( size_t IN size,
                                        cl_prog_binary_desc* OUT types,
                                        size_t* OUT sizeRet
                                           );

    cl_dev_err_code GetKernelId( cl_dev_program IN prog, const char* IN name, cl_dev_kernel* OUT kernelId );

    cl_dev_err_code GetProgramKernels( cl_dev_program IN prog, cl_uint IN num_kernels, cl_dev_kernel* OUT kernels,
                         cl_uint* OUT numKernelsRet );

    cl_dev_err_code GetKernelInfo( cl_dev_kernel IN kernel, cl_dev_kernel_info IN param, size_t IN valueSize,
                    void* OUT value, size_t* OUT valueSizeRet ) const;

    // internal methods to be used for MIC DA

    // get Backend kernel object
    const ICLDevBackendKernel_* GetBackendKernel( cl_dev_kernel kernel ) const;

    // increment reference counter of program and return device pointer
    uint64_t AcquireKernelOnDevice( cl_dev_kernel kernel );

    // decrement reference counter of program
    void releaseKernelOnDevice( cl_dev_kernel kernel );


private:
    friend class ProgramBuildTask;

    DeviceServiceCommunication&             m_DevService;

    // compiler interfaces. Initialized on the first access
    struct CompilerInterfaces {
        ICLDevBackendCompilationService*    volatile pCompilationService;
        ICLDevBackendSerializationService*  volatile pSerializationService;
        MICBackendOptions                   MICOptions;
        OclMutex                            creationLock;
        OpenCLBackendWrapper                be_wrapper;

        // constructor
        CompilerInterfaces(DeviceServiceCommunication& dev_service,
                           const char* be_dll_name ) :
                               pCompilationService(NULL),
                               pSerializationService(NULL),
                               MICOptions(dev_service),
                               be_wrapper(be_dll_name) {};

    };
    CompilerInterfaces                      m_BE_Compiler;

    struct TProgramEntry;

    // kernel info struct
    struct TKernelEntry
    {
        static const uint64_t       marker_value = 0xBEAFF00D;
        uint64_t                    marker; // set it to the marker_value at contructor

        const ICLDevBackendKernel_* pKernel;
        TProgramEntry*              pProgEntry;
        uint64_t                    uDevKernelEntry; // pointer to the kernel struct on device

        // constructor
        TKernelEntry( void ) : marker( marker_value ) {};
    };

    // 2 parallel maps point tothe same set of structs
    typedef std::map<std::string, TKernelEntry*>            TKernelName2Entry;
    typedef std::map<unsigned long long int, TKernelEntry*> TKernelId2Entry;

    // program info struct
    struct    TProgramEntry
    {
        static const uint64_t       marker_value = 0xF00DBEAF;
        uint64_t                    marker; // set it to the marker_value at contructor

        ICLDevBackendProgram_*      pProgram;
        uint64_t                    uid_program_on_device;
        volatile cl_build_status    clBuildStatus;
        bool                        copy_to_device_ok;
        TKernelName2Entry           mapName2Kernels;
        TKernelId2Entry             mapId2Kernels;
        AtomicCounter               outanding_usages_count;
        cl_int                      m_iDevId;   // created for current device

        // constructor
        TProgramEntry( cl_int dev_id ) : marker( marker_value ), m_iDevId(dev_id)  {};
    };

    typedef std::list<TProgramEntry*>    TProgramList;

    cl_int                         m_iDevId;
    IOCLDevLogDescriptor*          m_pLogDescriptor;
    cl_int                         m_iLogHandle;
    TProgramList                   m_Programs;
    OclMutex                       m_muPrograms;
    IOCLFrameworkCallbacks*        m_pCallBacks;

    MICDeviceConfig               *m_pMICConfig;


    ICLDevBackendCompilationService* GetCompilationService(void);
    ICLDevBackendSerializationService* GetSerializationService(void);

    bool                             LoadBackendServices(void);
    void                             ReleaseBackendServices(void);

    void    DeleteProgramEntry(TProgramEntry* pEntry);
    bool    BuildKernelData(TProgramEntry* pEntry);

    cl_dev_kernel kernel_entry_2_cl_dev_kernel( const TKernelEntry* e ) const;
    TKernelEntry* cl_dev_kernel_2_kernel_entry( cl_dev_kernel k ) const;

    cl_dev_program program_entry_2_cl_dev_program( const TProgramEntry* e ) const;
    TProgramEntry* cl_dev_program_2_program_entry( cl_dev_program p ) const;

    bool    CopyProgramToDevice(
                const ICLDevBackendProgram_* pProgram,
                size_t input_size, COPY_PROGRAM_TO_DEVICE_INPUT_STRUCT*  input,
                size_t output_size,COPY_PROGRAM_TO_DEVICE_OUTPUT_STRUCT* output );

    bool    RemoveProgramFromDevice( const TProgramEntry* pEntry );
};

}}}
