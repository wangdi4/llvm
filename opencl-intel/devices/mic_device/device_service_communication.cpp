// Copyright (c) 2006-2013 Intel Corporation
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

#include "device_service_communication.h"
#include "cl_sys_defines.h"
#include "cl_sys_info.h"
#include "mic_dev_limits.h"
#include "mic_common_macros.h"
#include "mic_sys_info.h"
#include "mic_device.h"
#include "mic_device_interface.h"
#include "mic_sys_info_internal.h"
#include "profiling_notification.h"
#include "Logger.h"

#include <source/COIEngine_source.h>
#include <source/COIProcess_source.h>
#include <source/COIEvent_source.h>
#include <source/COIBuffer_source.h>

#include "stdafx.h"

#ifndef WIN32
#include <libgen.h>
#endif
#include <string.h>
#include <assert.h>

#ifndef WIN32
#include <dlfcn.h> 
#else
#define RTLD_NOW 2
#endif

#define KILOBYTE 1024
#define MEGABYTE (1024*KILOBYTE)

using namespace std;

using namespace Intel::OpenCL::MICDevice;
using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::TaskExecutor;

extern char **environ;

static bool ReRegisterAtExitAfterCOI_Init = true;

// Device side functions used as entry points
// MUST be parallel to the enum DEVICE_SIDE_FUNCTION !!!!
const char* const DeviceServiceCommunication::m_device_function_names[DeviceServiceCommunication::DEVICE_SIDE_FUNCTION_COUNT] =
{
    "get_backend_target_description_size",  // GET_BACKEND_TARGET_DESCRIPTION_SIZE
    "get_backend_target_description",       // GET_BACKEND_TARGET_DESCRIPTION
    "copy_program_to_device",               // COPY_PROGRAM_TO_DEVICE
    "create_built_in_program",              // CREATE_BUILT_IN_PROGRAM
    "remove_program_from_device",           // REMOVE_PROGRAM_FROM_DEVICE
    "execute_command_ndrange",              // EXECUTE_NDRANGE
    "execute_command_nativekernel",         // EXECUTE_NATIVE_KERNEL
    "init_device",                          // INIT THE NATIVE PROCESS (Call it only once, after process creation)
    "release_device",                       // CLEAN SOME RESOURCES OF THE NATIVE PROCESS (Call it before closing the process)
    "init_commands_queue",                  // INIT COMMANDS QUEUE ON DEVICE
    "release_commands_queue",               // RELEASE COMMANDS QUEUE ON DEVICE
    "execute_device_utility",               // EXECUTE UTILITY FUNCTION ON DEVICE (as part of some user queue thread)
    "execute_command_fill_mem_object"       // FILL MEM OBJECT
#ifdef ENABLE_MIC_TRACER
    ,"get_trace_size",
    "get_trace"
#endif
};


DeviceServiceCommunication::DeviceServiceCommunication(unsigned int uiMicId) 
    : m_uiMicId(uiMicId), m_process(nullptr), m_pipe(nullptr), m_uiNumActiveThreads(0), m_initCompleted(false)
{
    memset(m_device_functions, 0, sizeof(m_device_functions));
}

DeviceServiceCommunication::~DeviceServiceCommunication()
{
    freeDevice(!MICDevice::isDeviceLibraryUnloaded());
}

cl_dev_err_code DeviceServiceCommunication::deviceSeviceCommunicationFactory(unsigned int uiMicId, 
                                                                             DeviceServiceCommunication** ppDeviceServiceCom)
{
    // find the first unused device index
    DeviceServiceCommunication* tDeviceServiceComm = new DeviceServiceCommunication(uiMicId);
    if (nullptr == tDeviceServiceComm)
    {
        return CL_DEV_OUT_OF_MEMORY;
    }

    // create a thread that will initialize the device process and open a service pipeline.
    int err = tDeviceServiceComm->Start();
    if (THREAD_RESULT_SUCCESS != err)
    {
      delete tDeviceServiceComm;
      return CL_DEV_ERROR_FAIL;
    }

    *ppDeviceServiceCom = tDeviceServiceComm;
    return CL_DEV_SUCCESS;
}

void DeviceServiceCommunication::freeDevice(bool releaseCoiObjects)
{
    COIRESULT result = COI_ERROR;

    waitForInitialization();

    if (releaseCoiObjects)
    {
#ifdef ENABLE_MIC_TRACER
        // Get device trace size
        uint64_t devTraceSize = 0;
        // Get device trace size
        bool res = runServiceFunction(GET_TRACE_SIZE, 0, nullptr, sizeof(uint64_t), &devTraceSize, 0, nullptr, nullptr);
        assert(res);
        if (devTraceSize > 0)
        {
            COIBUFFER traceCoiBuffer;
            COIRESULT coiRes = COIBufferCreate( devTraceSize, 
                                                COI_BUFFER_NORMAL, 0, 
                                                nullptr, 
                                                1, &m_process,
                                                &traceCoiBuffer);
            assert(COI_SUCCESS == coiRes);

            COI_ACCESS_FLAGS accessFlag[1] = { COI_SINK_WRITE_ENTIRE };
            // Get device trace
            res = runServiceFunction(GET_TRACE, 0, nullptr, 0, nullptr, 1, &traceCoiBuffer, accessFlag);
            assert(res);

            COIMAPINSTANCE mapInstance;
            void* devTrace = nullptr;
            coiRes = COIBufferMap( traceCoiBuffer,
                                   0, devTraceSize, 
                                   COI_MAP_READ_ONLY, 
                                   0, nullptr, nullptr,
                                   &mapInstance, 
                                   &devTrace );
            assert(COI_SUCCESS == coiRes);

            // Get device 0 freq.
            unsigned long long freq = MICSysInfo::getInstance().getMaxClockFrequency(0);

            // Write to file device trace
            MICDevice::m_tracer->draw_device_to_file(devTrace, devTraceSize, freq);

            coiRes = COIBufferUnmap( mapInstance, 0, nullptr, nullptr);
            assert(COI_SUCCESS == coiRes);
            coiRes = COIBufferDestroy( traceCoiBuffer );
            assert(COI_SUCCESS == coiRes);
        }

        // Write to file host trace
        MICDevice::m_tracer->draw_host_to_file(MICSysInfo::getInstance().getMicDeviceConfig());
#endif
        ProfilingNotification::getInstance().unregisterProfilingNotification(m_process);
        // Run release device function on device side
        runServiceFunction(RELEASE_DEVICE, 0, nullptr, 0, nullptr, 0, nullptr, nullptr);

        //close service pipeline
        if (m_pipe)
        {
            result = COIPipelineDestroy(m_pipe);
//            assert(result == COI_SUCCESS && "COIPipelineDestroy failed for service pipeline");
            m_pipe = nullptr;
        }

        // close the process, wait for main function to finish indefinitely.
        if (m_process)
        {
            result = COIProcessDestroy(m_process, -1, false, nullptr, nullptr);
//            assert(result == COI_SUCCESS && "COIProcessDestroy failed");
            m_process = nullptr;
        }
    }
}

COIPROCESS DeviceServiceCommunication::getDeviceProcessHandle() 
{
    waitForInitialization();
    return m_process;
}

COIFUNCTION DeviceServiceCommunication::getDeviceFunction( DEVICE_SIDE_FUNCTION id ) 
{
    waitForInitialization();
    if ( id >= LAST_DEVICE_SIDE_FUNCTION )
    {
        assert( false && "Too large Device Entry point Function ID" );
        return nullptr;
    }
    assert( 0 != m_device_functions[id] && "Getting reference to Device Entry point that does not exists" );
    return (nullptr == m_process) ? nullptr :  m_device_functions[id];
}

bool DeviceServiceCommunication::runServiceFunction(
                            DEVICE_SIDE_FUNCTION func,
                            size_t input_data_size, const void* input_data,
                            size_t output_data_size, void* output_data,
                            unsigned int numBuffers, const COIBUFFER* buffers, 
                            const COI_ACCESS_FLAGS* bufferAccessFlags,
                            COIPIPELINE use_pipeline )
{
    COIRESULT   result = COI_ERROR;
    COIEVENT  barrier;

    if (LAST_DEVICE_SIDE_FUNCTION <= func)
    {
        assert( false && "Unknown function in passed to runServiceFunction()" );
        return false;
    }

    if ((0 == input_data_size) || (nullptr == input_data))
    {
        input_data_size = 0;
        input_data = nullptr;
    }

    if ((0 == output_data_size) || (nullptr == output_data))
    {
        output_data_size = 0;
        output_data = nullptr;
    }

    if ((0 == numBuffers) || (nullptr == buffers) || (nullptr == bufferAccessFlags))
    {
        numBuffers = 0;
        buffers = nullptr;
        bufferAccessFlags = nullptr;
    }

    waitForInitialization();

    COIPIPELINE pipe = (nullptr != use_pipeline) ? use_pipeline : m_pipe;

    if (nullptr == pipe)
    {
        return false;
    }

    // Run func on device with no dependencies, assign a barrier in order to wait until the function execution complete.
    result = COIPipelineRunFunction(pipe, getDeviceFunction(func),
                                    numBuffers, buffers, bufferAccessFlags,
                                    0, nullptr,                    // dependencies
                                    input_data, input_data_size,
                                    output_data, output_data_size,
                                    &barrier);
    if (result != COI_SUCCESS)
    {
        return false;
    }
    // Wait until the function execution completed on the sink side.
    result = COIEventWait(1, &barrier, -1, false, nullptr, nullptr);
    if ((result != COI_SUCCESS) && (result != COI_EVENT_CANCELED))
    {
        return false;
    }
    return true;
}


void DeviceServiceCommunication::getVTuneEnvVars(vector<char*>& additionalEnvVars)
{
    unsigned int count = 0;
    string tStr;
    string hostPrefix = "__OCL_MIC_INTEL_";
    string stripPrefix = "__OCL_MIC_";
    while(environ[count] != nullptr)
    {
        tStr = environ[count];
        if ((string::npos != tStr.find(hostPrefix)) && (tStr.size() > hostPrefix.size()))
        {
            additionalEnvVars.push_back(environ[count] + stripPrefix.size());
        }
        count ++;
    }
}

static inline bool isCOIPoolRetValOk( COIRESULT result )
{
    return ((COI_SUCCESS            == result) ||
            (COI_RESOURCE_EXHAUSTED == result) ||
            (COI_OUT_OF_MEMORY      == result));
}

//
// Create Device memory cache for OpenCL buffers
// 
// 1. If 2M paged buffers are disabled - disable 2M paged pool on device, otherwise disable 4K paged pool on device
//    Disabled pool doesn't mean pool will not be created - itr means that pool will not be created upfront and will be allocated 
//    on demand only and if demand cannot be fulfilled without swapping out existing buffers from the same pool
//
// 2. Pool is created with following params:
//    2.1 Immediately allocate INIT size
//    2.2 Prefer pool on-demand extension until FINI size
//    2.3 Prefer swapping out existing buffers after pool reached FINI size
//
bool DeviceServiceCommunication::setupBufferMemoryPools( const MICDeviceConfig& tMicConfig )
{
    COIRESULT result = COI_ERROR;

    size_t initial      = tMicConfig.Device_Initial2MBPoolSizeInMB() * MEGABYTE; // MiB
    size_t final        = tMicConfig.Device_Final2MBPoolSizeInMB() * MEGABYTE;     // MiB;
    size_t max_possible = MIC_MAX_GLOBAL_MEM_SIZE( m_uiMicId );

    if ((0 == final) || (final > max_possible))
    {
        final = max_possible;
    }

    if (initial > final)
    {
        initial = final;   
    }

    // decide on 4K vs 2M
    size_t initial_4kb_pool_size = 0;
    size_t final_4kb_pool_size = 0;
    size_t initial_2mb_pool_size = 0;
    size_t final_2mb_pool_size   = 0;

    if (0 == tMicConfig.Device_2MB_BufferMinSizeInKB())
    {
        // 2M pagess disabled - use 4K instead
        initial_4kb_pool_size = initial;
        final_4kb_pool_size   = final;
    }
    else
    {
        initial_2mb_pool_size = initial;
        final_2mb_pool_size   = final;
    }

    if (initial > 0)
    {
        result = COIProcessSetCacheSize( m_process, 
                                         initial_2mb_pool_size, COI_CACHE_MODE_ONDEMAND_SYNC | COI_CACHE_ACTION_GROW_NOW,
                                         initial_4kb_pool_size, COI_CACHE_MODE_ONDEMAND_SYNC | COI_CACHE_ACTION_GROW_NOW,
                                         0, nullptr, nullptr );

        if (! isCOIPoolRetValOk(result))
        {
            assert(false && "Device memory inital reservation (COIProcessSetCacheSize) failed");            
            return false;
        }        
    }

    if (final > initial)
    {
        result = COIProcessSetCacheSize( m_process, 
                                         final_2mb_pool_size, COI_CACHE_MODE_ONDEMAND_SYNC | COI_CACHE_ACTION_NONE,
                                         final_4kb_pool_size, COI_CACHE_MODE_ONDEMAND_SYNC | COI_CACHE_ACTION_NONE,
                                         0, nullptr, nullptr );

        if (! isCOIPoolRetValOk(result))
        {
            assert(false && "Device memory final limit set (COIProcessSetCacheSize) failed");            
            return false;
        }        
    }

    return true;
    
}


RETURN_TYPE_ENTRY_POINT DeviceServiceCommunication::Run()
{
    COIRESULT result = COI_ERROR;
    cl_dev_err_code err = CL_DEV_SUCCESS;
    char nativeDirName[MAX_PATH] = {0};
    char fileNameBuffer[MAX_PATH] = {0};

    MICSysInfo& info = MICSysInfo::getInstance();

    // Get a handle to MIC engine number m_engineId
    COIENGINE engine = info.getCOIEngineHandle( m_uiMicId );

    // The following call creates a process on the sink.
    GetModuleDirectory( (char*)nativeDirName, sizeof(nativeDirName) );
    STRCAT_S((char*)nativeDirName, sizeof(nativeDirName), MIC_NATIVE_SUBDIR_NAME );

    STRCAT_S((char*)fileNameBuffer, sizeof(fileNameBuffer), nativeDirName );
    STRCAT_S((char*)fileNameBuffer, sizeof(fileNameBuffer), "/" );
    STRCAT_S((char*)fileNameBuffer, sizeof(fileNameBuffer), MIC_NATIVE_SERVER_EXE );

    do
    {
        const MICDeviceConfig& tMicConfig = MICSysInfo::getInstance().getMicDeviceConfig();
        // Get the amount of compute units in the device
        unsigned int numOfWorkers   = info.getNumOfComputeUnits(m_uiMicId);
        unsigned int numOfCores     = info.getNumOfCores(m_uiMicId);
        unsigned int threadsPerCore = numOfWorkers / numOfCores;

        if (threadsPerCore > MIC_NATIVE_MAX_THREADS_PER_CORE)
        {
            threadsPerCore = MIC_NATIVE_MAX_THREADS_PER_CORE;
        }

        mic_exec_env_options mic_device_options;
        memset( &mic_device_options, 0, sizeof(mic_device_options) );
        
        mic_device_options.stop_at_load                        = tMicConfig.Device_StopAtLoad();
        mic_device_options.use_affinity                        = tMicConfig.Device_UseAffinity();
        mic_device_options.threads_per_core                    = tMicConfig.Device_ThreadsPerCore();
        mic_device_options.num_of_cores                        = tMicConfig.Device_NumCores();
        mic_device_options.ignore_core_0                       = tMicConfig.Device_IgnoreCore0();
        mic_device_options.ignore_last_core                    = tMicConfig.Device_IgnoreLastCore();
        mic_device_options.use_TBB_grain_size                  = tMicConfig.Device_TbbGrainSize();
        mic_device_options.kernel_safe_mode                    = tMicConfig.Device_safeKernelExecution();
        mic_device_options.use_vtune                           = tMicConfig.UseVTune();
        mic_device_options.enable_itt                          = tMicConfig.UseITT();
        mic_device_options.trap_workers                        = tMicConfig.Device_TbbTrapWorkers();
        mic_device_options.min_buffer_size_parallel_fill       = tMicConfig.Device_ParallelFillBufferFromSize();
        mic_device_options.max_tasks_per_worker_fill_buffer    = tMicConfig.Device_ParallelFillMaxTaskPerWorker();
        mic_device_options.max_workers_fill_buffer             = tMicConfig.Device_ParallelFillMaxWorkers();
        mic_device_options.logger_enable                       = Logger::GetInstance().IsActive();
#ifdef __INCLUDE_MKL__
        mic_device_options.enable_mkl                          = tMicConfig.UseMKL();
#endif        
        string tbb_scheduler = tMicConfig.Device_TbbScheduler();
     
        if (tbb_scheduler == "affinity")
        {
            mic_device_options.tbb_scheduler = TE_CMD_LIST_PREFERRED_SCHEDULING_PRESERVE_TASK_AFFINITY;
        }
        else if ( tbb_scheduler == "dynamic" )
        {
            mic_device_options.tbb_scheduler = TE_CMD_LIST_PREFERRED_SCHEDULING_DYNAMIC;
        }
        else if ( tbb_scheduler == "opencl" )
        {
                mic_device_options.tbb_scheduler = TE_CMD_LIST_PREFERED_SCHEDULING_UNEVEN_OPENCL;
        }
        else // default
        {
            mic_device_options.tbb_scheduler = TE_CMD_LIST_PREFERED_SCHEDULING_UNEVEN_OPENCL;
        }

        string block_optimization = tMicConfig.Device_TbbBlockOptimization();
        
        if (block_optimization == "rows")
        {
            mic_device_options.tbb_block_optimization = TASK_SET_OPTIMIZE_BY_ROW;
        }
        else if (block_optimization == "columns")
        {
            mic_device_options.tbb_block_optimization = TASK_SET_OPTIMIZE_BY_COLUMN;
        }
        else if (block_optimization == "tiles")
        {
            mic_device_options.tbb_block_optimization = TASK_SET_OPTIMIZE_BY_TILE;
        }
        else
        {
            mic_device_options.tbb_block_optimization = TASK_SET_OPTIMIZE_DEFAULT;
        }
        
        memset(mic_device_options.mic_cpu_arch_str, 0, MIC_CPU_ARCH_STR_SIZE);
        MEMCPY_S(mic_device_options.mic_cpu_arch_str, MIC_CPU_ARCH_STR_SIZE, get_mic_cpu_arch(), sizeof(get_mic_cpu_arch()));

        if ((0 == mic_device_options.threads_per_core) || (mic_device_options.threads_per_core > threadsPerCore))
        {
            mic_device_options.threads_per_core = threadsPerCore;
        }

        if ((0 == mic_device_options.num_of_cores) || (mic_device_options.num_of_cores > numOfCores))
        {
            mic_device_options.num_of_cores = numOfCores;
        }

        mic_device_options.min_work_groups_number   = 
            mic_device_options.num_of_cores * mic_device_options.threads_per_core;

        vector<char*> additionalEnvVars;
        // If USE VTUNE need to send some env variables to sink side
        if (mic_device_options.use_vtune)
        {
          getVTuneEnvVars(additionalEnvVars);
        }

#ifdef __MIC_DA_OMP__
        // Set the environment variables that define OMP_SCHEDULE and KMP_AFFINITY
        char* cStr = nullptr;
        if (tMicConfig.Device_OmpSchedule().size() > 0)
        {
            string strSched;
            strSched.append("OMP_SCHEDULE=");
            strSched.append(tMicConfig.Device_OmpSchedule());
            cStr = new char[strSched.size() + 1];
            memset((void*)cStr, 0, strSched.size() + 1);
            memcpy((void*)cStr, (void*)strSched.c_str(), strSched.size());
            additionalEnvVars.push_back(cStr);
        }
        if (tMicConfig.Device_OmpKmpAffinity().size() > 0)
        {
            cStr = nullptr;
            string strKmpAff;
            strKmpAff.append("KMP_AFFINITY=");
            strKmpAff.append(tMicConfig.Device_OmpKmpAffinity());
            cStr = new char[strKmpAff.size() + 1];
            memset((void*)cStr, 0, strKmpAff.size() + 1);
            memcpy((void*)cStr, (void*)strKmpAff.c_str(), strKmpAff.size());
            additionalEnvVars.push_back(cStr);
        }
        if (tMicConfig.Device_OmpKmpBlockTime().size() > 0)
        {
            cStr = nullptr;
            string strKmpBlock;
            strKmpBlock.append("KMP_BLOCKTIME=");
            strKmpBlock.append(tMicConfig.Device_OmpKmpBlockTime());
            cStr = new char[strKmpBlock.size() + 1];
            memset((void*)cStr, 0, strKmpBlock.size() + 1);
            memcpy((void*)cStr, (void*)strKmpBlock.c_str(), strKmpBlock.size());
            additionalEnvVars.push_back(cStr);
        }
        //TODO delete those char* allocations after COIProcessCreateFromFile when I will fix getVTuneEnvVars()
#endif //__MIC_DA_OMP__

        if (additionalEnvVars.size() > 0)
        {
            additionalEnvVars.push_back(nullptr);
        }

        // create a process on device and run it's main() function
        result = COIProcessCreateFromFile(engine, (char*)fileNameBuffer,
                                         0, nullptr,                                                                                // argc, argv
                                         false, additionalEnvVars.size() == 0 ? nullptr : (const char**)&(additionalEnvVars[0]),    // duplicate env, additional env vars
                                         MIC_DEV_IO_PROXY_TO_HOST, nullptr,                                                         // I/O proxy required + host root
                                         MIC_DEV_INITIAL_BUFFER_PREALLOCATION,                                                   // reserve inital buffer space
                                         nativeDirName,                                                                          // a path to locate dynamic libraries dependencies for the sink application
                                         &m_process);
        assert(result == COI_SUCCESS && "COIProcessCreateFromFile failed");
        if (COI_SUCCESS != result)
        {
            break;
        }

        if (ReRegisterAtExitAfterCOI_Init)
        {
            ReRegisterAtExitAfterCOI_Init = false;
            UseShutdownHandler::ReRegisterAtExit();
        }

        // load additional DLLs required by this specific device
        const char * const * string_arr = nullptr;
        unsigned int dlls_count = info.getRequiredDeviceDLLs(m_uiMicId, &string_arr);

        if ((0 < dlls_count) && (nullptr != string_arr))
        {
            for (unsigned int i = 0; i < dlls_count; ++i)
            {
                if (nullptr != string_arr[i])
                {
                    COILIBRARY lib_handle = nullptr;
                    result = COIProcessLoadLibraryFromFile(
                                        m_process,             // in_Process
                                        string_arr[i],                          // in_FileName
                                        nullptr,                                   // in_so-name if not exists in file
                                        nativeDirName,                          // in_LibrarySearchPath
                                        RTLD_NOW,                                //Bitmask of the flags that will be passed in as the dlopen()
                                        &lib_handle );

                    assert( ((COI_SUCCESS == result) || (COI_ALREADY_EXISTS == result))
                            && "Cannot load device DLL" );
                    if ((COI_SUCCESS != result) && (COI_ALREADY_EXISTS == result))
                    {
                        break;
                    }
                }
            }
        }
        if ((COI_SUCCESS != result) && (COI_ALREADY_EXISTS == result))
        {
            break;
        }
        result = COI_SUCCESS;

        // We'll need a pipeline to run service functions
        result = COIPipelineCreate(m_process, nullptr, nullptr, &m_pipe);
        assert(result == COI_SUCCESS && "COIPipelineCreate failed for service pipeline");
        if (COI_SUCCESS != result)
        {
            break;
        }

#ifdef _DEBUG
        bool failed = false;
        // In debug get one by one to report missing names
        for(size_t i=0; i<DEVICE_SIDE_FUNCTION_COUNT; ++i)
        {
            // Get list of entry points
            result = COIProcessGetFunctionHandles(m_process,
                                1,
                                (const char**)(m_device_function_names+i),
                                m_device_functions+i );
            if (COI_SUCCESS != result)
            {
                failed = true;
                printf("Failed to retrieve function <%s>\n", m_device_function_names[i]); fflush(0);
            }
        }
        assert( (!failed) && "Failed to retrieve function names");
#else
        // Get list of entry points
        result = COIProcessGetFunctionHandles(m_process,
                                              DEVICE_SIDE_FUNCTION_COUNT,
                                              (const char**)m_device_function_names,
                                              m_device_functions );
        assert(result == COI_SUCCESS && "COIProcessGetFunctionHandles failed to find all device entry points");
        if (COI_SUCCESS != result)
        {
            break;
        }
#endif
        // Run init device function on device side
        // Run func on device with no dependencies, assign a barrier in order to wait until the function execution complete.
        COIEVENT barrier;
        mic_init_return retVal;
        result = COIPipelineRunFunction(m_pipe, m_device_functions[INIT_DEVICE],
                                        0, nullptr, nullptr,
                                        0, nullptr,                                      // dependecies
                                        &mic_device_options, sizeof(mic_device_options),
                                        &retVal, sizeof(mic_init_return),
                                        &barrier);
        assert(result == COI_SUCCESS);
        if (result != COI_SUCCESS)
        {
            break;
        }
        // Wait until the function execution completed on the sink side.
        result = COIEventWait(1, &barrier, -1, false, nullptr, nullptr);
        assert(result == COI_SUCCESS);
        if ((result != COI_SUCCESS) && (result != COI_EVENT_CANCELED))
        {
            break;
        }
        err = retVal.initError;
        assert(CL_DEV_SUCCESS == err);
        if (CL_DEV_FAILED(err))
        {
            break;
        }

        if (false == ProfilingNotification::getInstance().registerProfilingNotification(m_process))
        {
            err = CL_DEV_ERROR_FAIL;
            break;
        }
        m_uiNumActiveThreads = retVal.uiNumActiveThreads;

        if (!setupBufferMemoryPools(tMicConfig))
        {
            err = CL_DEV_ERROR_FAIL;
            break;
        }
        
    }
    while (0);

    if ((COI_SUCCESS != result) || (CL_DEV_FAILED(err)))
    {
        ProfilingNotification::getInstance().unregisterProfilingNotification(m_process);

        if (nullptr != m_pipe)
        {
            COIPipelineDestroy(m_pipe);
            m_pipe = nullptr;
        }

        if (nullptr != m_process)
        {
            COIProcessDestroy(m_process, -1, false, nullptr, nullptr);
            m_process = nullptr;
        }
    }
    
    m_initCompleted = true;

    return 0;
}
