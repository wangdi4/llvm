#include "native_common_macros.h"
#include "native_globals.h"
#include "native_program_service.h"
#include "native_thread_pool.h"

#include <sink/COIBuffer_sink.h>
#include <common/COIEvent_common.h>

#include <sink/COIPipeline_sink.h>
#include <sink/COIProcess_sink.h>

using namespace Intel::OpenCL::MICDeviceNative;

// Declare global variables
Intel::OpenCL::MICDevice::mic_exec_env_options  Intel::OpenCL::MICDeviceNative::gMicExecEnvOptions;
#ifdef USE_ITT
ocl_gpa_data                                    Intel::OpenCL::MICDeviceNative::gMicGPAData;
#endif

// Module local variable
static volatile unsigned int resume_server_execution = 0;

// Initialize the device thread pool. Call it immediately after process creation.
COINATIVELIBEXPORT
void init_device(uint32_t in_BufferCount,
		     void**           in_ppBufferPointers,
				 uint64_t*        in_pBufferLengths,
				 void*            in_pMiscData,
				 uint16_t         in_MiscDataLength,
				 void*            in_pReturnValue,
				 uint16_t         in_ReturnValueLength)
{
	assert(in_MiscDataLength == sizeof(mic_exec_env_options));
	assert(in_ReturnValueLength == sizeof(cl_dev_err_code));

	cl_dev_err_code* pErr = (cl_dev_err_code*)in_pReturnValue;
	*pErr = CL_DEV_SUCCESS;
	
	// The mic_exec_env_options input.
	mic_exec_env_options* tEnvOptions = (mic_exec_env_options*)in_pMiscData;
	assert(tEnvOptions);
	if (tEnvOptions->stop_at_load)
	{
		printf("********* DEVICE STOPPED PLEASE ATTACH TO PID = %d ************\n", getpid());
		fflush(stdout);
		while (resume_server_execution == 0) {};
	}

	gMicExecEnvOptions = *tEnvOptions;
#ifdef USE_ITT
    memset(&gMicGPAData, sizeof(ocl_gpa_data), 0);
    if ( gMicExecEnvOptions.enable_itt )
    {
      gMicGPAData.bUseGPA = true;
      if ( NULL == gMicGPAData.pDeviceDomain )
      {
        gMicGPAData.pDeviceDomain   = __itt_domain_create("com.intel.opencl.device.mic");
        gMicGPAData.pNDRangeHandle  = __itt_string_handle_create("NDRange");
      }
    }
#endif

  unsigned int num_of_worker_threads = gMicExecEnvOptions.threads_per_core*gMicExecEnvOptions.num_of_cores;
	assert((num_of_worker_threads > 0) && (num_of_worker_threads < MIC_NATIVE_MAX_WORKER_THREADS));

    if ((num_of_worker_threads <= 0) || (num_of_worker_threads >= MIC_NATIVE_MAX_WORKER_THREADS))
    {
        *pErr = CL_DEV_ERROR_FAIL;
        return;
    }

	*pErr = ProgramService::createProgramService();
	if ( CL_DEV_FAILED(*pErr) )
	{
		return;
	}

	// Create thread pool singleton instance.
	ThreadPool* pThreadPool = ThreadPool::getInstance();
	if (NULL == pThreadPool)
	{
		ProgramService::releaseProgramService();
		*pErr = CL_DEV_OUT_OF_MEMORY;
		return;
	}

	// Initialize the thread pool with "numOfWorkers" workers.
	if (false == pThreadPool->init( gMicExecEnvOptions.use_affinity, 
                                    gMicExecEnvOptions.num_of_cores, gMicExecEnvOptions.threads_per_core,
                                    gMicExecEnvOptions.ignore_core_0, gMicExecEnvOptions.ignore_last_core ))
	{
		ProgramService::releaseProgramService();
		ThreadPool::releaseSingletonInstance();
		*pErr = CL_DEV_ERROR_FAIL;
		return;
	}
	
}

// release the device thread pool. Call it before process destruction.
COINATIVELIBEXPORT
void release_device(uint32_t         in_BufferCount,
					 void**           in_ppBufferPointers,
					 uint64_t*        in_pBufferLengths,
					 void*            in_pMiscData,
					 uint16_t         in_MiscDataLength,
					 void*            in_pReturnValue,
					 uint16_t         in_ReturnValueLength)
{
	// Release the thread pool singleton.
	ThreadPool::releaseSingletonInstance();
	ProgramService::releaseProgramService();    
}

// Execute device utility function
COINATIVELIBEXPORT
void execute_device_utility( uint32_t         in_BufferCount,
        					 void**           in_ppBufferPointers,
        					 uint64_t*        in_pBufferLengths,
        					 void*            in_pMiscData,
        					 uint16_t         in_MiscDataLength,
        					 void*            in_pReturnValue,
        					 uint16_t         in_ReturnValueLength)
{
    //
    // Execute some management function on device as part of some queue
    //
    assert( ((NULL != in_pMiscData) && (sizeof(utility_function_options) == in_MiscDataLength)) 
                                            && "Wrong params to execute_device_utility" );

    if ((NULL == in_pMiscData) || (sizeof(utility_function_options) != in_MiscDataLength))
    {
        return;
    }

    utility_function_options* options = (utility_function_options*)in_pMiscData;

    switch (options->request)
    {
        case UTILITY_MEASURE_OVERHEAD:
            break;

        default:;
    }
}

