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

#include <cl_dev_backend_api.h>
#include "mic_device.h"
#include "execution_commands.h"
#include "command_list.h"
#include "memory_allocator.h"

#include <source/COIBuffer_source.h>

using namespace Intel::OpenCL::MICDevice;
using namespace Intel::OpenCL::DeviceBackend;

//
//  ExecutionCommand Object
//

ExecutionCommand::ExecutionCommand(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd) :
    Command(pCommandList, pFrameworkCallBacks, pCmd)
{
}

//
// StartUp device services before ExecutionCommand launch if profiling mode was required
//
void ExecutionCommand::init_profiling_mode()
{
    PerformanceDataStore* overhead_data = m_pCommandList->getOverheadData();
    if (PerformanceDataStore::NOT_MEASURED != overhead_data->execution_overhead)
    {
        return;
    }
    
    OclAutoMutex( &(overhead_data->lock) );
    if (PerformanceDataStore::NOT_MEASURED != overhead_data->execution_overhead)
    {
        return;
    }

    // Get this queue COIPIPELINE handle
    utility_function_options options;
    bool                     ok;
    
    // measure execution overhead on device
    options.request = UTILITY_MEASURE_OVERHEAD;

    cl_ulong time_sum = 0;

    registerProfilingContext();

    for (unsigned int i = 0; i < MIC_DEVICE_EXECUTION_OVERHEAD_LOOP_COUNT; ++i)
    {
        m_cmdRunningTime = 0;
        m_cmdCompletionTime = 0;

        ok = m_pCommandList->runQueueServiceFunction( DeviceServiceCommunication::EXECUTE_DEVICE_UTILITY,
                                                      sizeof(options), &options, // input
                                                      0, NULL,                   // ouput
                                                      0, NULL, NULL              // buffers 
                                                     );
        assert( ok );
        if (! ok)
        {
            break;
        }

        time_sum += (m_cmdCompletionTime - m_cmdRunningTime);
    }

    unregisterProfilingContext();

    m_cmdRunningTime = 0;
    m_cmdCompletionTime = 0;
    
    if (!ok)
    {
        overhead_data->execution_overhead = 0;
        return;
    }

    overhead_data->execution_overhead = time_sum / MIC_DEVICE_EXECUTION_OVERHEAD_LOOP_COUNT;
}


cl_dev_err_code ExecutionCommand::executeInt(DeviceServiceCommunication::DEVICE_SIDE_FUNCTION funcId, char* commandNameStr)
{
    cl_dev_err_code err = CL_DEV_SUCCESS;
    // the COIBUFFERs to dispatch
    vector<COIBUFFER> coiBuffsArr;
    // the access flags of the COIBUFFERs array
    vector<COI_ACCESS_FLAGS> accessFlagsArr;
    do
    {
        COIEVENT barrier;
        unsigned int numDependecies = 0;
        m_pCommandSynchHandler->getLastDependentBarrier(m_pCommandList, &barrier, &numDependecies, true);
        COIEVENT* pBarrier = (numDependecies == 0) ? NULL : &barrier;

        assert( (numDependecies <= 1) && "Previous command list dependencies may not be more than 1" );

        if (numDependecies > 1)
        {
            m_lastError = CL_DEV_NOT_SUPPORTED;
            break;
        }

        // Get this queue COIPIPELINE handle
        COIPIPELINE pipe = m_pCommandList->getPipelineHandle();

        //Get COIFUNCTION handle according to func name (ask from DeviceServiceCommunication dictionary)
        COIFUNCTION func = m_pCommandList->getDeviceFunction( funcId );

        // Set command type for the tracer.
        m_commandTracer.set_command_type(commandNameStr);

        err = init(coiBuffsArr, accessFlagsArr);
        if (err != CL_DEV_SUCCESS)
        {
            break;
        }

        if (m_pCmd->profiling)
        {
            init_profiling_mode();
        }

        registerProfilingContext(true);

        // Set start coi execution time for the tracer.
        m_commandTracer.set_current_time_coi_enqueue_command_time_start();

        size_t tCoiBuffsArrSize = coiBuffsArr.size();

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    if ( (NULL != m_pCommandList->GetGPAInfo()) && m_pCommandList->GetGPAInfo()->bUseGPA )
    {
      static __thread __itt_string_handle* pTaskName = NULL;
      if ( NULL == pTaskName )
      {
        pTaskName = __itt_string_handle_create("ExecutionCommand::executeInt()->PrepareData");
      }
      __itt_task_begin(m_pCommandList->GetGPAInfo()->pDeviceDomain, __itt_null, __itt_null, pTaskName);
    }
#endif
    void* pDataPtr = m_dispatcherDatahandler.getDispatcherDataPtrForCoiRunFunc();
    uint16_t pDataSize = m_dispatcherDatahandler.getDispatcherDataSizeForCoiRunFunc();
    void* pMiscPtr = m_miscDatahandler.getMiscDataPtrForCoiRunFunc();
    uint16_t pMiscSize = m_miscDatahandler.getMiscDataSizeForCoiRunFunc();
    COIEVENT* pEvent = m_pCommandSynchHandler->registerBarrier(m_completionBarrier, this);
        /* Run the function pointed by 'func' on the device with 'numBuffersToDispatch' buffers and with dependency on 'barrier' (Can be NULL) and signal m_completionBarrier.cmdEvent when finish.
           'm_pCommandSynchHandler->registerCompletionBarrier(&m_completionBarrier.cmdEvent))' can return NULL, in case of Out of order CommandList */
#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
  if ( (NULL != m_pCommandList->GetGPAInfo()) && m_pCommandList->GetGPAInfo()->bUseGPA )
  {
    __itt_task_end(m_pCommandList->GetGPAInfo()->pDeviceDomain);
  }
#endif
#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    if ( (NULL != m_pCommandList->GetGPAInfo()) && m_pCommandList->GetGPAInfo()->bUseGPA )
    {
      static __thread __itt_string_handle* pTaskName = NULL;
      if ( NULL == pTaskName )
      {
        pTaskName = __itt_string_handle_create("ExecutionCommand::executeInt()->COIPipelineRunFunction()");
      }
      __itt_task_begin(m_pCommandList->GetGPAInfo()->pDeviceDomain, __itt_null, __itt_null, pTaskName);
    }
#endif
        COIRESULT result = COIPipelineRunFunction(pipe,
                                                  func,
                                                  tCoiBuffsArrSize, (tCoiBuffsArrSize > 0 ? &(coiBuffsArr[0]) : NULL), (tCoiBuffsArrSize > 0 ? &(accessFlagsArr[0]) : NULL),
                                                  numDependecies, pBarrier,
                                                  pDataPtr, pDataSize,
                                                  pMiscPtr, pMiscSize,
                                                  pEvent);
#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
  if ( (NULL != m_pCommandList->GetGPAInfo()) && m_pCommandList->GetGPAInfo()->bUseGPA )
  {
    __itt_task_end(m_pCommandList->GetGPAInfo()->pDeviceDomain);
  }
#endif

  unregisterProfilingContext();


        if (result != COI_SUCCESS)
        {
            assert( (result == COI_SUCCESS) && "COIPipelineRunFunction() returned error for kernel invoke" );
            m_lastError = CL_DEV_ERROR_FAIL;
            break;
        }
    }
    while (0);

    return executePostDispatchProcess(true);
}

void ExecutionCommand::fireCallBack(void* arg)
{
#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    if ( (NULL != m_pCommandList->GetGPAInfo()) && m_pCommandList->GetGPAInfo()->bUseGPA )
    {
      static __thread __itt_string_handle* pTaskName = NULL;
      if ( NULL == pTaskName )
      {
        pTaskName = __itt_string_handle_create("ExecutionCommand::fireCallBack()");
      }
      __itt_task_begin(m_pCommandList->GetGPAInfo()->pDeviceDomain, __itt_null, __itt_null, pTaskName);
    }
#endif

    misc_data miscData;
    m_miscDatahandler.readMiscData(&miscData);
    if (CL_DEV_SUCCESS == m_lastError)
    {
        m_lastError = miscData.errCode;
    }

    if (m_pCmd->profiling)
    {
        assert(m_cmdRunningTime > 0 && m_cmdCompletionTime > 0);
        
        cl_ulong overhead = m_pCommandList->getOverheadData()->execution_overhead;
        
        assert(overhead != PerformanceDataStore::NOT_MEASURED);

        if (PerformanceDataStore::NOT_MEASURED != overhead)
        {
            cl_ulong time = m_cmdCompletionTime - m_cmdRunningTime;
            time = (time <= overhead) ? 1 : time - overhead;
            m_cmdRunningTime = m_cmdCompletionTime - time;
        }
    }

    m_commandTracer.set_opencl_running_time_start( m_cmdRunningTime );
    m_commandTracer.set_opencl_running_time_end( m_cmdCompletionTime );

    // It is safe now to release the resources of m_dispatcherDatahandler, m_miscDatahandler and m_startBarrier (m_startBarrier in case of OOO Queue)
    // It is NOT safe to release m_completionBarrier because it can be use by other thread during the release.
    m_dispatcherDatahandler.release();
    m_miscDatahandler.release();

    m_pCommandSynchHandler->unregisterBarrier(m_startBarrier);

    // Call parent fireCallBack
    Command::fireCallBack(arg);
#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
  if ( (NULL != m_pCommandList->GetGPAInfo()) && m_pCommandList->GetGPAInfo()->bUseGPA )
  {
    __itt_task_end(m_pCommandList->GetGPAInfo()->pDeviceDomain);
  }
#endif
}

//
//  NDRange Object
//

NDRange::NDRange(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd) : 
    ExecutionCommand(pCommandList, pFrameworkCallBacks, pCmd), m_kernel_locked(false)
{
}

NDRange::~NDRange()
{
    releaseResources(!MICDevice::isDeviceLibraryUnloaded());
}

cl_dev_err_code NDRange::Create(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd, SharedPtr<Command>& pOutCommand)
{
    // run CheckCommandParams only in Debug mode
    assert( CL_DEV_SUCCESS == CheckCommandParams(pCommandList, pCmd) && "Wrong params" );

    return verifyCreation(new NDRange(pCommandList, pFrameworkCallBacks, pCmd), pOutCommand);
}

cl_dev_err_code NDRange::CheckCommandParams(CommandList* pCommandList, cl_dev_cmd_desc* cmd)
{
    if ( (CL_DEV_CMD_EXEC_KERNEL != cmd->type) && (CL_DEV_CMD_EXEC_TASK != cmd->type) )
    {
        return CL_DEV_INVALID_COMMAND_TYPE;
    }

    if ( sizeof(cl_dev_cmd_param_kernel) != cmd->param_size )
    {
        return CL_DEV_INVALID_COMMAND_PARAM;
    }

    cl_dev_cmd_param_kernel *cmdParams = (cl_dev_cmd_param_kernel*)(cmd->params);

    ProgramService* program_service     = pCommandList->getProgramService();
    const ICLDevBackendKernel_* pKernel = program_service->GetBackendKernel(cmdParams->kernel);
    assert(pKernel);
    const ICLDevBackendKernelProporties* pKernelProps = pKernel->GetKernelProporties();
    assert(pKernelProps);

    size_t    stLocMemSize = 0;

    // Check kernel parameters
    cl_uint                     uiNumArgs = pKernel->GetKernelParamsCount();
    const cl_kernel_argument*   pArgs = pKernel->GetKernelParams();

    cl_char*    pCurrParamPtr = (cl_char*)cmdParams->arg_values;
    size_t        stOffset = 0;
    // Check kernel parameters and memory buffers
    for(unsigned int i=0; i<uiNumArgs; ++i)
    {
        // Argument is buffer object or local memory size
        if ( CL_KRNL_ARG_PTR_GLOBAL <= pArgs[i].type )
        {
            stOffset += sizeof(void*);
        }
        else if (CL_KRNL_ARG_PTR_LOCAL == pArgs[i].type)
        {
            size_t origSize = ((size_t)*(((void**)(pCurrParamPtr+stOffset))));
            size_t locSize = ADJUST_SIZE_TO_MAXIMUM_ALIGN(origSize);
            stLocMemSize += locSize;
            stOffset += sizeof(void*);
        }
        else if (CL_KRNL_ARG_VECTOR == pArgs[i].type)
        {
            unsigned int uiSize = pArgs[i].size_in_bytes;
            uiSize = (uiSize & 0xFFFF) * (uiSize >> 16);
            stOffset += uiSize;
        }
        else if (CL_KRNL_ARG_SAMPLER == pArgs[i].type)
        {
            stOffset += sizeof(cl_int);
        }
        else
        {
            stOffset += pArgs[i].size_in_bytes;
        }
    }
    // Check parameters array size
    if ( stOffset != cmdParams->arg_size )
    {
        return CL_DEV_INVALID_COMMAND_PARAM;
    }

    // Check implicit memory sizes
    stLocMemSize += pKernelProps->GetImplicitLocalMemoryBufferSize();


    // Check if local memory size is enough for kernel
    if ( MIC_DEV_LCL_MEM_SIZE < stLocMemSize )
    {
        return CL_DEV_INVALID_COMMAND_PARAM;
    }

    // Check Work-Group / Work-Item information
    if ( CL_DEV_CMD_EXEC_KERNEL == cmd->type )
    {
        // Check WG dimensions
        size_t    stWGSize = 1;

        if ( MAX_WORK_DIM < cmdParams->work_dim )
        {
            return CL_DEV_INVALID_WRK_DIM;
        }

        const size_t    *pReqdWGSize = pKernelProps->GetRequiredWorkGroupSize();
        for(unsigned int i=0; i<cmdParams->work_dim; ++i)
        {
            if ( ((0 != cmdParams->lcl_wrk_size[i]) && (MIC_DEV_MAX_WI_SIZE < cmdParams->lcl_wrk_size[i])) ||
                ( pReqdWGSize && (pReqdWGSize[i] != cmdParams->lcl_wrk_size[i]))
                )
            {
                return CL_DEV_INVALID_WRK_ITEM_SIZE;
            }

            stWGSize *= cmdParams->lcl_wrk_size[i];
        }

        if ( MIC_MAX_WORK_GROUP_SIZE < stWGSize )
        {
            return CL_DEV_INVALID_WG_SIZE;
        }
    } else
    {
        // For Task one dimension is required
        if ( 1 != cmdParams->work_dim )
        {
            return CL_DEV_INVALID_WRK_DIM;
        }
        // Work Group size should be 1
        if ( 1 != cmdParams->lcl_wrk_size[0] )
        {
            return CL_DEV_INVALID_WRK_ITEM_SIZE;
        }
        // Work-Group size should be 1
        if ( 1 != cmdParams->glb_wrk_size[0] )
        {
            return CL_DEV_INVALID_WRK_DIM;
        }
    }

    return CL_DEV_SUCCESS;
}

cl_dev_err_code NDRange::init(vector<COIBUFFER>& outCoiBuffsArr, vector<COI_ACCESS_FLAGS>& outAccessFlagArr)
{
#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    if ( (NULL != m_pCommandList->GetGPAInfo()) && m_pCommandList->GetGPAInfo()->bUseGPA )
    {
      static __thread __itt_string_handle* pTaskName = NULL;
      if ( NULL == pTaskName )
      {
        pTaskName = __itt_string_handle_create("NDRange::init()");
      }
      __itt_task_begin(m_pCommandList->GetGPAInfo()->pDeviceDomain, __itt_null, __itt_null, pTaskName);
    }
#endif

    cl_dev_err_code returnError = CL_DEV_SUCCESS;
    // array of directive_pack for preExeDirectives
    directive_pack* preExeDirectives = NULL;
    // array of directive_pack for preExeDirectives
    directive_pack* postExeDirectives = NULL;
    do
    {
        ProgramService* program_service = m_pCommandList->getProgramService();

        // Get command params
        cl_dev_cmd_param_kernel *cmdParams = (cl_dev_cmd_param_kernel*)m_pCmd->params;
        // Get Kernel from params
#ifndef NDRANGE_UNIT_TEST
        const ICLDevBackendKernel_* pKernel = program_service->GetBackendKernel(cmdParams->kernel);
#else
        program_service = NULL;
        const ICLDevBackendKernel_* pKernel = (ICLDevBackendKernel_*)cmdParams->kernel;
#endif

        // Get kernel params
        const cl_kernel_argument*   pBEParams = pKernel->GetKernelParams();
        const char*                 pKernelParams = (const char*)cmdParams->arg_values;

        // Define the directives and function arguments to dispatch to the device side. 
        // (Will be store at the first COIBUFFER or at misc data if its size is less than COI_PIPELINE_MAX_IN_MISC_DATA_LEN)
        ndrange_dispatcher_data dispatcherData;

#ifndef NDRANGE_UNIT_TEST
        // Get device side kernel address and set kernel directive (Also increase the reference counter of the Program.
        // TODO: Is it long operation? Why we can't just have a remote address
        dispatcherData.kernelDirective.kernelAddress = program_service->AcquireKernelOnDevice(cmdParams->kernel);
        m_kernel_locked = (0 != dispatcherData.kernelDirective.kernelAddress);
#else
        // Only for unit test
        dispatcherData.kernelDirective.kernelAddress = pKernel->GetKernelID();
#endif

        if (0 == dispatcherData.kernelDirective.kernelAddress)
        {
            returnError = CL_DEV_INVALID_KERNEL;
            break;
        }
        // set unique command identifier
        dispatcherData.commandIdentifier = m_pCmd->id;

        // TODO: Why need this info, the kernel on device knows its parameters. args size must match
        // set kernel args blob size in bytes for dispatcherData
        dispatcherData.kernelArgSize = cmdParams->arg_size;
        // TODO: The type of the queue is known, why add it?
        // set isInOrderQueue flag in dispatcherData
        dispatcherData.isInOrderQueue = m_pCommandList->isInOrderCommandList();
        // Filling the workDesc structure in dispatcherData
        dispatcherData.workDesc.setParams(cmdParams->work_dim, cmdParams->glb_wrk_offs, cmdParams->glb_wrk_size, cmdParams->lcl_wrk_size);

        // directives counter.
        unsigned int numPreDirectives = 0;
        unsigned int numPostDirectives = 0;

        // TODO: SVM non-args support required
        unsigned int numOfBuffersInKernelArgs = pKernel->GetMemoryObjectArgumentCount();
        
        // TODO: Remove this code once dynamic directives build will be removed
        unsigned int numOfActualBuffers = 0;
        const unsigned int* pBufferArgsInx = pKernel->GetMemoryObjectArgumentIndexes();
        for (unsigned int i = 0; i < numOfBuffersInKernelArgs; ++i)
        {
            const cl_kernel_argument&             paramDesc = pBEParams[pBufferArgsInx[i]];

            MICDevMemoryObject *memObj = *(MICDevMemoryObject**)(((char*)cmdParams->arg_values)+paramDesc.offset_in_bytes);
            if ( NULL == memObj )
                continue;
            // real buffer passed
            ++numOfActualBuffers;
        }

        numPreDirectives += numOfActualBuffers;

        // If the CommandList is OOO
        if (false == dispatcherData.isInOrderQueue)
        {
            // We should add additional directive for pre and post execution (BARRIER directive)
            numPreDirectives ++;
            numPostDirectives ++;
        }

        // The the amount of pre and post exe directives in 'dispatcherData'
        dispatcherData.preExeDirectivesCount = numPreDirectives;
        dispatcherData.postExeDirectivesCount = numPostDirectives;
        // calculate and set the offset parameters in 'dispatcherData'
        dispatcherData.calcAndSetOffsets();

        outCoiBuffsArr.reserve(numOfActualBuffers + AMOUNT_OF_OPTIONAL_DISPATCH_BUFFERS);
        outAccessFlagArr.reserve(numOfActualBuffers + AMOUNT_OF_OPTIONAL_DISPATCH_BUFFERS);

        // TODO: the directives are locally used, why not use alloca()?
        if (numPreDirectives > 0)
        {
            preExeDirectives = new directive_pack[numPreDirectives];
            if (NULL == preExeDirectives)
            {
                returnError = CL_DEV_OUT_OF_MEMORY;
                break;
            }
        }
        if (numPostDirectives > 0)
        {
            postExeDirectives = new directive_pack[numPostDirectives];
            if (NULL == postExeDirectives)
            {
                returnError = CL_DEV_OUT_OF_MEMORY;
                break;
            }
        }

        unsigned int currPreDirectiveIndex = 0;
        unsigned int currPostDirectiveIndex = 0;

        // Set the buffer arguments of the kernel as directives and COIBUFFERS
        assert(numPreDirectives >= numOfActualBuffers);
        //const unsigned int* pBufferArgsInx = pKernel->GetMemoryObjectArgumentIndexes();
        for (unsigned int i = 0; i < numOfBuffersInKernelArgs; ++i)
        {
            const cl_kernel_argument&             paramDesc = pBEParams[pBufferArgsInx[i]];
            
            MICDevMemoryObject *memObj = *(MICDevMemoryObject**)(((char*)cmdParams->arg_values)+paramDesc.offset_in_bytes);
            if ( NULL == memObj )
                continue;

            // Set the COIBUFFER of coiBuffsArr[currCoiBuffIndex] to be the COIBUFFER that hold the data of the buffer.
            outCoiBuffsArr.push_back(memObj->clDevMemObjGetCoiBufferHandler());
            
            // TODO - Change the flag according to the information about the argument (Not implemented yet by the BE)            
            if ( CL_KRNL_ARG_PTR_CONST == paramDesc.type )
            {
                outAccessFlagArr.push_back(COI_SINK_READ);
            }
            else
            {
                // Now check the memObj flags (The flags that the memObj created with)
                cl_mem_flags mem_flags = memObj->clDevMemObjGetMemoryFlags();

                if (CL_MEM_READ_ONLY == (CL_MEM_READ_ONLY & mem_flags))
                {
                    outAccessFlagArr.push_back(COI_SINK_READ);
                }
                else
                {
                    outAccessFlagArr.push_back(COI_SINK_WRITE);
                }
            }
            assert(outCoiBuffsArr.size() == outAccessFlagArr.size());
            // Set this directive settings
            preExeDirectives[currPreDirectiveIndex].id = BUFFER;
            preExeDirectives[currPreDirectiveIndex].bufferDirective.bufferIndex = outCoiBuffsArr.size() - 1;
            preExeDirectives[currPreDirectiveIndex].bufferDirective.offset_in_blob = paramDesc.offset_in_bytes;
            preExeDirectives[currPreDirectiveIndex].bufferDirective.mem_obj_desc = memObj->clDevMemObjGetDescriptorRaw();

            currPreDirectiveIndex ++;
        }

        // TODO: do we need these barriers also in in-order queue?
        // Register start barrier
        m_pCommandSynchHandler->registerBarrier(m_startBarrier, this);
        // Register completion barrier
        m_pCommandSynchHandler->registerBarrier(m_completionBarrier, this);
        // If it is OutOfOrderCommandList, add BARRIER directive to postExeDirectives
        if (false == dispatcherData.isInOrderQueue)
        {
            preExeDirectives[currPreDirectiveIndex].id = BARRIER;
            preExeDirectives[currPreDirectiveIndex].barrierDirective.barrier = m_startBarrier.cmdEvent;
            currPreDirectiveIndex ++;

            postExeDirectives[currPostDirectiveIndex].id = BARRIER;
            postExeDirectives[currPostDirectiveIndex].barrierDirective.barrier = m_completionBarrier.cmdEvent;
            currPostDirectiveIndex ++;
        }

        // Get device side process in order to create COIBUFFERs for this process.
        COIPROCESS tProcess = m_pCommandList->getDeviceProcess();

        // initialize the miscDataHandler
        // TODO: What is this? Do we need this?
        returnError = m_miscDatahandler.init(!dispatcherData.isInOrderQueue, &tProcess);
        if (CL_DEV_FAILED(returnError))
        {
            break;
        }
        // register misc_data
        m_miscDatahandler.registerMiscData(outCoiBuffsArr, outAccessFlagArr);

        returnError = m_dispatcherDatahandler.init(dispatcherData, preExeDirectives, postExeDirectives, pKernelParams, &tProcess);
        if (CL_DEV_FAILED(returnError))
        {
            break;
        }
        m_dispatcherDatahandler.registerDispatcherData(outCoiBuffsArr, outAccessFlagArr);

        assert(outCoiBuffsArr.size() == outAccessFlagArr.size());
    }
    while (0);

    if (preExeDirectives)
    {
        delete [] preExeDirectives;
    }
    if (postExeDirectives)
    {
        delete [] postExeDirectives;
    }

    if (CL_DEV_FAILED(returnError))
    {
        outCoiBuffsArr.clear();
        outAccessFlagArr.clear();
    }
    m_lastError = returnError;

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    if ( (NULL != m_pCommandList->GetGPAInfo()) && m_pCommandList->GetGPAInfo()->bUseGPA )
    {
    __itt_task_end(m_pCommandList->GetGPAInfo()->pDeviceDomain);
    }
#endif

    return returnError;
}

inline void NDRange::releaseKernel( void )
{
    if (m_kernel_locked)
    {
        // Decrement the reference counter of this kernel program.
        m_pCommandList->getProgramService()->releaseKernelOnDevice(((cl_dev_cmd_param_kernel*)m_pCmd->params)->kernel);        
        m_kernel_locked = false;
    }
}

void NDRange::fireCallBack(void* arg)
{
    // Do release kernel here and not only in NDRange distructor in order to avoid races with 
    // clReleaseProgram that may be called during notifyCommandStatusChanged() call
    releaseKernel();

    return ExecutionCommand::fireCallBack(arg);
}

void NDRange::releaseResources(bool releaseCoiObjects)
{
    // release kernel for the case of running error
    releaseKernel();
}

//
//  Fill Memory Object
//

FillMemObject::FillMemObject(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd) : ExecutionCommand(pCommandList, pFrameworkCallBacks, pCmd)
{
}

cl_dev_err_code FillMemObject::Create(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd, SharedPtr<Command>& pOutCommand)
{
    return verifyCreation(new FillMemObject(pCommandList, pFrameworkCallBacks, pCmd), pOutCommand);
}

cl_dev_err_code FillMemObject::init(vector<COIBUFFER>& outCoiBuffsArr, vector<COI_ACCESS_FLAGS>& outAccessFlagArr)
{
    cl_dev_err_code returnError = CL_DEV_SUCCESS;
    // directive_pack for preExeDirective
    directive_pack preExeDirective[2];
    // directive_pack for preExeDirective
    directive_pack postExeDirective;

    cl_dev_cmd_param_fill*            cmdParams = (cl_dev_cmd_param_fill*)m_pCmd->params;
    MICDevMemoryObject*                pMicMemObj;
    fill_mem_obj_dispatcher_data    fillMemObjDispatcherData;

    // The do .... while (0) is a pattern when there are many failures points instead of goto operation use do ... while (0) with break commands.
    do {
        returnError = cmdParams->memObj->clDevMemObjGetDescriptor(CL_DEVICE_TYPE_ACCELERATOR, 0, (cl_dev_memobj_handle*)&pMicMemObj);
        if (CL_DEV_FAILED(returnError))
        {
            break;
        }

        const cl_mem_obj_descriptor& pMemObj = pMicMemObj->clDevMemObjGetDescriptorRaw();

        fillMemObjDispatcherData.commandIdentifier = m_pCmd->id;
        fillMemObjDispatcherData.isInOrderQueue = m_pCommandList->isInOrderCommandList();

        // copy the dimension value
        assert(pMemObj.dim_count == cmdParams->dim_count);
        fillMemObjDispatcherData.dim_count = cmdParams->dim_count;
        fillMemObjDispatcherData.from_offset = MemoryAllocator::CalculateOffset(fillMemObjDispatcherData.dim_count, cmdParams->offset, pMemObj.pitch, pMemObj.uiElementSize);

        // Set region
        memcpy(fillMemObjDispatcherData.vRegion, cmdParams->region, sizeof(fillMemObjDispatcherData.vRegion));
        fillMemObjDispatcherData.vRegion[0] = cmdParams->region[0] * pMemObj.uiElementSize;

        // Set pitch
        memcpy(fillMemObjDispatcherData.vFromPitch, pMemObj.pitch, sizeof(fillMemObjDispatcherData.vFromPitch));

        // Set pattern
        assert(cmdParams->pattern_size <= MAX_PATTERN_SIZE);
        if (cmdParams->pattern_size > MAX_PATTERN_SIZE)
        {
            returnError = CL_DEV_INVALID_VALUE;
            break;
        }
        memcpy(fillMemObjDispatcherData.pattern, cmdParams->pattern, cmdParams->pattern_size);
        fillMemObjDispatcherData.pattern_size = cmdParams->pattern_size;

        // Optimization which send the COI buffer as COI_SINK_WRITE_ENTIRE if the user like to over-write the whole buffer.
        COI_ACCESS_FLAGS dstBuffAccessFlag = COI_SINK_WRITE_ENTIRE;
        for (unsigned int i = 0; i < fillMemObjDispatcherData.dim_count; i++)
        {
            if ((cmdParams->offset[i] != 0) || ((pMemObj.memObjType == CL_MEM_OBJECT_BUFFER) && (cmdParams->region[i] != pMemObj.dimensions.buffer_size)) ||
                ((pMemObj.memObjType != CL_MEM_OBJECT_BUFFER) && (cmdParams->region[i] != pMemObj.dimensions.dim[i])))
            {
                dstBuffAccessFlag = COI_SINK_WRITE;
                break;
            }
        }

        // Add the destination buffer and set its directive as pre exe directive
        outCoiBuffsArr.push_back(pMicMemObj->clDevMemObjGetCoiBufferHandler());
        outAccessFlagArr.push_back(dstBuffAccessFlag);
        memset(&preExeDirective, 0, sizeof(preExeDirective));
        preExeDirective[0].id = BUFFER;
        preExeDirective[0].bufferDirective.bufferIndex = 0;
        fillMemObjDispatcherData.preExeDirectivesCount = 1;

        fillMemObjDispatcherData.postExeDirectivesCount = 0;

        // Register start barrier
        m_pCommandSynchHandler->registerBarrier(m_startBarrier, this);
        // Register completion barrier
        m_pCommandSynchHandler->registerBarrier(m_completionBarrier, this);
        // If it is OutOfOrderCommandList, add BARRIER directive to postExeDirectives
        if (false == fillMemObjDispatcherData.isInOrderQueue)
        {
            preExeDirective[1].id = BARRIER;
            preExeDirective[0].barrierDirective.barrier = m_startBarrier.cmdEvent;
            fillMemObjDispatcherData.preExeDirectivesCount ++;

            // Set the post exe directive
            postExeDirective.id = BARRIER;
            postExeDirective.barrierDirective.barrier = m_completionBarrier.cmdEvent;
            fillMemObjDispatcherData.postExeDirectivesCount = 1;
        }

        // calculate and set the offset parameters in 'dispatcherData'
        fillMemObjDispatcherData.calcAndSetOffsets();

        // Get device side process in order to create COIBUFFERs for this process (Only in case of OOO queue).
        COIPROCESS tProcess = m_pCommandList->getDeviceProcess();
        // initialize the miscDataHandler
        returnError = m_miscDatahandler.init(!fillMemObjDispatcherData.isInOrderQueue, &tProcess);
        if (CL_DEV_FAILED(returnError))
        {
            break;
        }
        // register misc_data
        m_miscDatahandler.registerMiscData(outCoiBuffsArr, outAccessFlagArr);

        returnError = m_dispatcherDatahandler.init(fillMemObjDispatcherData, preExeDirective, &postExeDirective, NULL, &tProcess);
        if (CL_DEV_FAILED(returnError))
        {
            break;
        }
        m_dispatcherDatahandler.registerDispatcherData(outCoiBuffsArr, outAccessFlagArr);

        assert(outCoiBuffsArr.size() == outAccessFlagArr.size());

    } while (0);

    if (CL_DEV_FAILED(returnError))
    {
        outCoiBuffsArr.clear();
        outAccessFlagArr.clear();
    }
    m_lastError = returnError;

    return returnError;
}

