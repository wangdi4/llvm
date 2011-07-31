#include "device_service_communication.h"
#include "cl_sys_defines.h"
#include "cl_sys_info.h"
#include "mic_dev_limits.h"
#include "mic_common_macros.h"

#include <source/COIEngine_source.h>
#include <source/COIProcess_source.h>
#include <source/COIBarrier_source.h>
#include <assert.h>

using namespace Intel::OpenCL::MICDevice;
using namespace Intel::OpenCL::Utils;

// Device side functions used as entry points
// MUST be parallel to the enum DEVICE_SIDE_FUNCTION !!!!
const char* const DeviceServiceCommunication::m_device_function_names[DeviceServiceCommunication::DEVICE_SIDE_FUNCTION_COUNT] =
{
    "get_backend_target_description_size",  // GET_BACKEND_TARGET_DESCRIPTION_SIZE
    "get_backend_target_description",       // GET_BACKEND_TARGET_DESCRIPTION
    "copy_program_to_device",               // COPY_PROGRAM_TO_DEVICE
    "remove_program_from_device",           // REMOVE_PROGRAM_FROM_DEVICE

    "hello_world"                           // EXECUTE_IN_ORDER
};

DeviceServiceCommunication::device_service_communication_pack DeviceServiceCommunication::pDeviceServiceCommPack;

DeviceServiceCommunication::DeviceServiceCommunication() : m_engineId(0), m_process(NULL), m_pipe(NULL), m_initDone(false)
{
    pthread_mutex_init(&m_mutex, NULL);
    pthread_cond_init(&m_cond, NULL);
}

DeviceServiceCommunication::~DeviceServiceCommunication()
{
    freeDevice();
}

void DeviceServiceCommunication::loadingInit()
{
    pDeviceServiceCommPack.deviceServiceCommArr = NULL;
    pDeviceServiceCommPack.numEngines = 0;
    pDeviceServiceCommPack.numFreeEngines = 0;
    pthread_mutex_init(&pDeviceServiceCommPack.lock, NULL);
}

void DeviceServiceCommunication::unloadRelease()
{
    if (pDeviceServiceCommPack.deviceServiceCommArr)
    {
        volatile DeviceServiceCommunication* tDeviceServiceComm = NULL;
        for (unsigned int i = 0; i < pDeviceServiceCommPack.numEngines; i++)
        {
            if (pDeviceServiceCommPack.deviceServiceCommArr[i])
            {
                tDeviceServiceComm = pDeviceServiceCommPack.deviceServiceCommArr[i];
                delete(tDeviceServiceComm);
            }
        }
        free((void*)pDeviceServiceCommPack.deviceServiceCommArr);
    }
    pthread_mutex_destroy(&pDeviceServiceCommPack.lock);
}

bool DeviceServiceCommunication::devcieSeviceCommunicationFactory(DeviceServiceCommunication** ppDeviceServiceCom)
{
    // If first call, shall initialte the data structure
    if (pDeviceServiceCommPack.deviceServiceCommArr == NULL)
    {
        pthread_mutex_lock(&pDeviceServiceCommPack.lock);
        if (pDeviceServiceCommPack.deviceServiceCommArr == NULL)
        {
            COIRESULT result = COI_ERROR;
            // get amount of KNF devices available
            result = COIEngineGetCount(COI_ISA_KNF, &pDeviceServiceCommPack.numEngines);
            if (result != COI_SUCCESS)
            {
                pthread_mutex_unlock(&pDeviceServiceCommPack.lock);
                return false;
            }
            // If numEngines is not greater than 0 there is a problem
            if (pDeviceServiceCommPack.numEngines == 0)
            {
                pthread_mutex_unlock(&pDeviceServiceCommPack.lock);
                return false;
            }
            pDeviceServiceCommPack.numFreeEngines = pDeviceServiceCommPack.numEngines;
            // allocate memory and init by null pointers for DeviceServiceCommunication objects
            volatile DeviceServiceCommunication* volatile * tDeviceServiceCommArr = (volatile DeviceServiceCommunication* volatile *)malloc(sizeof(DeviceServiceCommunication*) * pDeviceServiceCommPack.numEngines);
            assert(tDeviceServiceCommArr && "Malloc operation failed");
            memset((void*)tDeviceServiceCommArr, 0, sizeof(DeviceServiceCommunication*) * pDeviceServiceCommPack.numEngines);
            ATOMIC_ASSIGN( pDeviceServiceCommPack.deviceServiceCommArr, tDeviceServiceCommArr );
        }
        pthread_mutex_unlock(&pDeviceServiceCommPack.lock);
    }
    int tCurrFreeEngines = __sync_fetch_and_sub(&pDeviceServiceCommPack.numFreeEngines, 1);
    // If there is no available engine
    if (tCurrFreeEngines <= 0)
    {
        __sync_fetch_and_add(&pDeviceServiceCommPack.numFreeEngines, 1);
        return false;
    }
    // find the first unused device index
    DeviceServiceCommunication* tDeviceServiceComm = new DeviceServiceCommunication();
    assert(tDeviceServiceComm && "new operation failed");
    int tEngineId = -1;
    for (unsigned int i = 0; i < pDeviceServiceCommPack.numEngines; i++)
    {
        if ((pDeviceServiceCommPack.deviceServiceCommArr[i] == NULL) &&
            (__sync_bool_compare_and_swap(&pDeviceServiceCommPack.deviceServiceCommArr[i], NULL, tDeviceServiceComm)))
        {
            tEngineId = i;
            break;
        }
    }
    assert(tEngineId >= 0);

    tDeviceServiceComm->setEngineId(tEngineId);

    pthread_attr_t tattr;
    pthread_attr_init(&tattr);
    pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);

    // create a thread that will initialize the device process and open a service pipeline.
    int err = pthread_create(&tDeviceServiceComm->m_initializerThread, &tattr, initEntryPoint, tDeviceServiceComm);
    assert(err == 0 && "thread creation failed");

    pthread_attr_destroy(&tattr);

    *ppDeviceServiceCom = tDeviceServiceComm;

    return true;
}

void DeviceServiceCommunication::freeDevice()
{
    COIRESULT result = COI_ERROR;

    waitForInitThread();

    //close service pipeline
    if (m_pipe)
    {
        result = COIPipelineDestroy(m_pipe);
        assert(result == COI_SUCCESS && "COIPipelineDestroy failed for service pipeline");
        m_pipe = NULL;
    }

    // close the process, wait for main function to finish indefinitely.
    if (m_process)
    {
        result = COIProcessDestroy(m_process, -1, false, NULL, NULL);
        assert(result == COI_SUCCESS && "COIProcessDestroy failed");
        m_process = NULL;
    }

    pthread_cond_destroy(&m_cond);
    pthread_mutex_destroy(&m_mutex);

    // set me as NULL pointer in deviceServiceCommArr
    pDeviceServiceCommPack.deviceServiceCommArr[m_engineId] = NULL;

    __sync_fetch_and_add(&pDeviceServiceCommPack.numFreeEngines, 1);
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
    return m_device_functions[id];
}

bool DeviceServiceCommunication::runServiceFunction(
                            DEVICE_SIDE_FUNCTION func,
                            size_t input_data_size, void* input_data,
                            size_t output_data_size, void* output_data,
                            unsigned int numBuffers, const COIBUFFER* buffers, const COI_ACCESS_FLAGS* bufferAccessFlags)
{
    COIRESULT   result = COI_ERROR;
    COIBARRIER  barrier;

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
    result = COIBarrierWait(1, &barrier, -1, false, NULL, NULL);
    if ((result != COI_SUCCESS) && (result != COI_BARRIER_CANCELED))
    {
        return false;
    }
    return true;
}

void* DeviceServiceCommunication::initEntryPoint(void* arg)
{
    COIRESULT result = COI_ERROR;
    COIENGINE engine;
    char fileNameBuffer[MAX_PATH] = {0};
    DeviceServiceCommunication* pDevServiceComm = (DeviceServiceCommunication*)arg;

    // Get a handle to KNF engine number m_engineId
    result = COIEngineGetHandle(COI_ISA_KNF, pDevServiceComm->m_engineId, &engine);
    assert(result == COI_SUCCESS && "COIEngineGetHandle failed");

    // The following call creates a process on the sink.
    GetModuleDirectory( (char*)fileNameBuffer, sizeof(fileNameBuffer) );
    STRCAT_S((char*)fileNameBuffer, sizeof(fileNameBuffer), MIC_NATIVE_SERVER_EXE );

    // create a process on device and run it's main() function
    result = COIProcessCreateFromFile(engine, (char*)fileNameBuffer,
                                     0, NULL,                               // argc, argv
                                     false, NULL,                           // duplicate env, additional env vars
                                     MIC_DEV_IO_PROXY_TO_HOST, NULL,        // I/O proxy required + host root
                                     MIC_DEV_MAX_ALLOCATED_BUFFERS_SIZE,    // reserve buffer space
                                     &pDevServiceComm->m_process);
    assert(result == COI_SUCCESS && "COIProcessCreateFromFile failed");

    // We'll need a pipeline to run service functions
    result = COIPipelineCreate(pDevServiceComm->m_process, NULL, NULL, &pDevServiceComm->m_pipe);
    assert(result == COI_SUCCESS && "COIPipelineCreate failed for service pipeline");

    // Get list of entry points
    result = COIProcessGetFunctionHandles(pDevServiceComm->m_process,
                                          DEVICE_SIDE_FUNCTION_COUNT,
                                          (const char**)m_device_function_names,
                                          pDevServiceComm->m_device_functions );
    assert(result == COI_SUCCESS && "COIProcessGetFunctionHandles failed to find all device entry points");

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

void DeviceServiceCommunication::setEngineId(unsigned int engineId)
{
    m_engineId = engineId;
}

