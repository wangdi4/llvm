#include "device_service_communication.h"
#include "cl_sys_defines.h"
#include "cl_sys_info.h"
#include "mic_dev_limits.h"
#include "mic_common_macros.h"
#include "mic_sys_info.h"

#include <source/COIEngine_source.h>
#include <source/COIProcess_source.h>
#include <source/COIEvent_source.h>

#include <libgen.h>
#include <assert.h>

using namespace Intel::OpenCL::MICDevice;
using namespace Intel::OpenCL::Utils;

extern bool gSafeReleaseOfCoiObjects;

// Device side functions used as entry points
// MUST be parallel to the enum DEVICE_SIDE_FUNCTION !!!!
const char* const DeviceServiceCommunication::m_device_function_names[DeviceServiceCommunication::DEVICE_SIDE_FUNCTION_COUNT] =
{
    "get_backend_target_description_size",  // GET_BACKEND_TARGET_DESCRIPTION_SIZE
    "get_backend_target_description",       // GET_BACKEND_TARGET_DESCRIPTION
    "copy_program_to_device",               // COPY_PROGRAM_TO_DEVICE
    "remove_program_from_device",           // REMOVE_PROGRAM_FROM_DEVICE

	"execute_NDRange"						// EXECUTE_NDRANGE
};

DeviceServiceCommunication::DeviceServiceCommunication(unsigned int uiMicId) : m_uiMicId(uiMicId), m_process(NULL), m_pipe(NULL), m_initDone(false)
{
    pthread_mutex_init(&m_mutex, NULL);
    pthread_cond_init(&m_cond, NULL);
}

DeviceServiceCommunication::~DeviceServiceCommunication()
{
    freeDevice(gSafeReleaseOfCoiObjects);
}

cl_dev_err_code DeviceServiceCommunication::deviceSeviceCommunicationFactory(unsigned int uiMicId, DeviceServiceCommunication** ppDeviceServiceCom)
{
    // find the first unused device index
    DeviceServiceCommunication* tDeviceServiceComm = new DeviceServiceCommunication(uiMicId);
	if (NULL == tDeviceServiceComm)
	{
		return CL_DEV_OUT_OF_MEMORY;
	}

    pthread_attr_t tattr;
    pthread_attr_init(&tattr);
    pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);

    // create a thread that will initialize the device process and open a service pipeline.
    int err = pthread_create(&tDeviceServiceComm->m_initializerThread, &tattr, initEntryPoint, tDeviceServiceComm);
	if (0 != err)
	{
		delete tDeviceServiceComm;
		return CL_DEV_ERROR_FAIL;
	}

    pthread_attr_destroy(&tattr);

    *ppDeviceServiceCom = tDeviceServiceComm;
    return CL_DEV_SUCCESS;
}

void DeviceServiceCommunication::freeDevice(bool releaseCoiObjects)
{
    COIRESULT result = COI_ERROR;

    waitForInitThread();

	if (releaseCoiObjects)
	{
		//close service pipeline
		if (m_pipe)
		{
			result = COIPipelineDestroy(m_pipe);
//			assert(result == COI_SUCCESS && "COIPipelineDestroy failed for service pipeline");
			m_pipe = NULL;
		}

		// close the process, wait for main function to finish indefinitely.
		if (m_process)
		{
			result = COIProcessDestroy(m_process, -1, false, NULL, NULL);
//			assert(result == COI_SUCCESS && "COIProcessDestroy failed");
			m_process = NULL;
		}
	}

    pthread_cond_destroy(&m_cond);
    pthread_mutex_destroy(&m_mutex);
}

COIPROCESS DeviceServiceCommunication::getDeviceProcessHandle() const
{
    waitForInitThread();
    return m_process;
}

COIFUNCTION DeviceServiceCommunication::getDeviceFunction( DEVICE_SIDE_FUNCTION id ) const
{
    waitForInitThread();
    assert( id < LAST_DEVICE_SIDE_FUNCTION && "Too large Device Entry point Function ID" );
    assert( 0 != m_device_functions[id] && "Getting reference to Device Entry point that does not exists" );
	return (NULL == m_process) ? NULL :  m_device_functions[id];
}

bool DeviceServiceCommunication::runServiceFunction(
                            DEVICE_SIDE_FUNCTION func,
                            size_t input_data_size, void* input_data,
                            size_t output_data_size, void* output_data,
                            unsigned int numBuffers, const COIBUFFER* buffers, const COI_ACCESS_FLAGS* bufferAccessFlags)
{
    COIRESULT   result = COI_ERROR;
    COIEVENT  barrier;

    if (LAST_DEVICE_SIDE_FUNCTION <= func)
    {
        assert( false && "Unknown function in passed to runServiceFunction()" );
        return false;
    }

    if ((0 == input_data_size) || (NULL == input_data))
    {
        input_data_size = 0;
        input_data = NULL;
    }

    if ((0 == output_data_size) || (NULL == output_data))
    {
        output_data_size = 0;
        output_data = NULL;
    }

    if ((0 == numBuffers) || (NULL == buffers) || (NULL == bufferAccessFlags))
    {
        numBuffers = 0;
        buffers = NULL;
        bufferAccessFlags = NULL;
    }

    waitForInitThread();

	if (NULL == m_pipe)
	{
		return false;
	}

    // Run func on device with no dependencies, assign a barrier in order to wait until the function execution complete.
    result = COIPipelineRunFunction(m_pipe, getDeviceFunction(func),
                                    numBuffers, buffers, bufferAccessFlags,
                                    0, NULL,                    // dependecies
                                    input_data, input_data_size,
                                    output_data, output_data_size,
                                    &barrier);
    if (result != COI_SUCCESS)
    {
        return false;
    }
    // Wait until the function execution completed on the sink side.
    result = COIEventWait(1, &barrier, -1, false, NULL, NULL);
    if ((result != COI_SUCCESS) && (result != COI_EVENT_CANCELED))
    {
        return false;
    }
    return true;
}

void* DeviceServiceCommunication::initEntryPoint(void* arg)
{
    COIRESULT result = COI_ERROR;
    char fileNameBuffer[MAX_PATH] = {0};
    DeviceServiceCommunication* pDevServiceComm = (DeviceServiceCommunication*)arg;

    MICSysInfo& info = MICSysInfo::getInstance();

    // Get a handle to KNF engine number m_engineId
    COIENGINE engine = info.getCOIEngineHandle( pDevServiceComm->m_uiMicId );

    // The following call creates a process on the sink.
    GetModuleDirectory( (char*)fileNameBuffer, sizeof(fileNameBuffer) );
    STRCAT_S((char*)fileNameBuffer, sizeof(fileNameBuffer), MIC_NATIVE_SERVER_EXE );

	char tBuff[PATH_MAX];
    GetModulePathName((void*)(ptrdiff_t)initEntryPoint, tBuff, PATH_MAX-1);
    const char* device_dir = dirname(tBuff);

	do
	{

		// create a process on device and run it's main() function
		result = COIProcessCreateFromFile(engine, (char*)fileNameBuffer,
										 0, NULL,													// argc, argv
										 false, NULL,												// duplicate env, additional env vars
										 MIC_DEV_IO_PROXY_TO_HOST, NULL,							// I/O proxy required + host root
                                         MIC_AVAILABLE_PROCESS_MEMORY(pDevServiceComm->m_uiMicId),  // reserve buffer space
										 device_dir,												// a path to locate dynamic libraries dependencies for the sink application
										 &pDevServiceComm->m_process);
		assert(result == COI_SUCCESS && "COIProcessCreateFromFile failed");
		if (COI_SUCCESS != result)
		{
			break;
		}

		// load additional DLLs required by this specific device
		const char * const * string_arr = NULL;
		unsigned int dlls_count = info.getRequiredDeviceDLLs(pDevServiceComm->m_uiMicId, &string_arr);

		if ((0 < dlls_count) && (NULL != string_arr))
		{
			for (unsigned int i = 0; i < dlls_count; ++i)
			{
				if (NULL != string_arr[i])
				{
					COILIBRARY lib_handle;
					result = COIProcessLoadLibraryFromFile(
										pDevServiceComm->m_process,             // in_Process
										string_arr[i],                          // in_FileName
										NULL,                                   // in_so-name if not exists in file
										device_dir,                             // in_LibrarySearchPath
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
		result = COIPipelineCreate(pDevServiceComm->m_process, NULL, NULL, &pDevServiceComm->m_pipe);
		assert(result == COI_SUCCESS && "COIPipelineCreate failed for service pipeline");
		if (COI_SUCCESS != result)
		{
			break;
		}

		// Get list of entry points
		result = COIProcessGetFunctionHandles(pDevServiceComm->m_process,
											  DEVICE_SIDE_FUNCTION_COUNT,
											  (const char**)m_device_function_names,
											  pDevServiceComm->m_device_functions );
		assert(result == COI_SUCCESS && "COIProcessGetFunctionHandles failed to find all device entry points");
		if (COI_SUCCESS != result)
		{
			break;
		}
	}
	while (0);
	if (COI_SUCCESS != result)
	{
		if (NULL != pDevServiceComm->m_pipe)
		{
			COIPipelineDestroy(pDevServiceComm->m_pipe);
			ATOMIC_ASSIGN(pDevServiceComm->m_pipe, NULL);
		}

		if (NULL != pDevServiceComm->m_process)
		{
			COIProcessDestroy(pDevServiceComm->m_process, -1, false, NULL, NULL);
			ATOMIC_ASSIGN(pDevServiceComm->m_process, NULL);
		}
	}
    // Release the blocked threads that are waiting for me
    pthread_mutex_lock(&pDevServiceComm->m_mutex);
    ATOMIC_ASSIGN( pDevServiceComm->m_initDone, true );
    pthread_cond_broadcast(&pDevServiceComm->m_cond);
    pthread_mutex_unlock(&pDevServiceComm->m_mutex);

    return NULL;
}

inline void DeviceServiceCommunication::waitForInitThread() const
{
    // if initializer thread finished, can return.
    if (m_initDone == false)
    {
        pthread_mutex_lock(&m_mutex);
        while (m_initDone == false)
        {
            pthread_cond_wait(&m_cond, &m_mutex);
        }
        pthread_mutex_unlock(&m_mutex);
    }
}

