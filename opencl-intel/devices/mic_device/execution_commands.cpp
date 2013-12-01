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

ExecutionCommand::~ExecutionCommand()
{
    // Unregister the completion barrier
    unregisterBarrier(m_startEvent);
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
    do
    {
        COIEVENT barrier;
        unsigned int numDependecies = 0;
        m_pCommandList->getLastDependentBarrier(&barrier, &numDependecies, true);
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

        err = init();
        if (err != CL_DEV_SUCCESS)
        {
            break;
        }

        // TODO: if necessary run it on initialization
        if (m_pCmd->profiling)
        {
            init_profiling_mode();
        }

        registerProfilingContext(true);

#ifdef ENABLE_MIC_TRACER
        // Set command type for the tracer.
        m_commandTracer.set_command_type(commandNameStr);
        // Set start coi execution time for the tracer.
        m_commandTracer.set_current_time_coi_enqueue_command_time_start();
#endif
        size_t tCoiBuffsArrSize = m_coiBuffsArr.size();

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

        // Use completion event for in order queue's only
        COIEVENT* pEvent = m_pCommandList->isInOrderCommandList() ? &m_endEvent.cmdEvent: NULL;

        COIRESULT result = COIPipelineRunFunction(pipe,
                                func,
                                tCoiBuffsArrSize,
                                (tCoiBuffsArrSize > 0 ? &(m_coiBuffsArr[0]) : NULL),
                                (tCoiBuffsArrSize > 0 ? &(m_accessFlagsArr[0]) : NULL),
                                numDependecies, pBarrier,
                                m_pDispatchData, m_uiDispatchDataSize,
                                NULL, 0,                                                // We don't have nothing to receive from device
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

void ExecutionCommand::AddMemoryObject( MICDevMemoryObject *memObj, bool isConstAccess )
{
    // Set the COIBUFFER of coiBuffsArr[currCoiBuffIndex] to be the COIBUFFER that hold the data of the buffer.
    m_coiBuffsArr.push_back(memObj->clDevMemObjGetCoiBufferHandler());
            
    // TODO - Change the flag according to the information about the argument (Not implemented yet by the BE)            
    if ( isConstAccess )
    {
        m_accessFlagsArr.push_back(COI_SINK_READ);
    }
    else
    {
        // Now check the memObj flags (The flags that the memObj created with)
        cl_mem_flags mem_flags = memObj->clDevMemObjGetMemoryFlags();

        if (CL_MEM_READ_ONLY == (CL_MEM_READ_ONLY & mem_flags))
        {
            m_accessFlagsArr.push_back(COI_SINK_READ);
        }
        else
        {
            m_accessFlagsArr.push_back(COI_SINK_WRITE);
        }
    }
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

    if (CL_DEV_SUCCESS == m_lastError)
    {
      m_lastError = CL_DEV_SUCCESS;
    }

    if (m_pCmd->profiling)
    {
        assert(m_cmdRunningTime > 0 && m_cmdCompletionTime > 0 && "When profiling On, both RUNNING and COMPLETED must be set");

        cl_ulong overhead = m_pCommandList->getOverheadData()->execution_overhead;
        
        assert(overhead != PerformanceDataStore::NOT_MEASURED);

        if (PerformanceDataStore::NOT_MEASURED != overhead)
        {
            cl_ulong time = m_cmdCompletionTime - m_cmdRunningTime;
            time = (time <= overhead) ? 1 : time - overhead;
            m_cmdRunningTime = m_cmdCompletionTime - time;
        }
    }

#ifdef ENABLE_MIC_TRACER
    m_commandTracer.set_opencl_running_time_start( m_cmdRunningTime );
    m_commandTracer.set_opencl_running_time_end( m_cmdCompletionTime );
#endif

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
    ExecutionCommand(pCommandList, pFrameworkCallBacks, pCmd)
{
}

cl_dev_err_code NDRange::Create(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd, SharedPtr<Command>& pOutCommand)
{
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

    const ICLDevBackendKernel_* pKernel = ProgramService::GetBackendKernel(cmdParams->kernel);
    assert(pKernel);
    const ICLDevBackendKernelProporties* pKernelProps = pKernel->GetKernelProporties();
    assert(pKernelProps);

    size_t    stLocMemSize = 0;

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

cl_dev_err_code NDRange::init()
{
    // Optimized for 64bit machines, not supported on 32 bit addresses
    if ( sizeof(void*) != sizeof(uint64_t) )
    {
        assert( 0 && " Currently implemented only for 64bit address machines");
        return CL_DEV_NOT_SUPPORTED;
    }

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
    // array of directive_pack for preExeDirectives
    do
    {
        ProgramService* program_service = m_pCommandList->getProgramService();

        // Get command params
        const cl_dev_cmd_param_kernel* cmdParams = (cl_dev_cmd_param_kernel*)m_pCmd->params;
        // Get Kernel from params
#ifndef NDRANGE_UNIT_TEST
        const ICLDevBackendKernel_* pKernel = program_service->GetBackendKernel(cmdParams->kernel);
#else
        program_service = NULL;
        const ICLDevBackendKernel_* pKernel = (ICLDevBackendKernel_*)cmdParams->kernel;
#endif

        m_uiDispatchDataSize = cmdParams->arg_size;
        // Check if we can arguments to the device, assumption on host and device we have same address size
        if ( m_uiDispatchDataSize > COI_PIPELINE_MAX_IN_MISC_DATA_LEN )
        {
            assert (0 && "Argument buffer is too large");
            m_uiDispatchDataSize = 0;
            returnError = CL_DEV_OUT_OF_MEMORY;
            break;
        }
        m_pDispatchData = cmdParams->arg_values;

        // Get the amount of buffers in kernel args and info of their offset in blob and their index in pArgs
        unsigned int numOfBuffersInKernelArgs = pKernel->GetMemoryObjectArgumentCount();
        const unsigned int* pBufferArgsInx = pKernel->GetMemoryObjectArgumentIndexes();

        // Reserve also for SVM buffers
        m_coiBuffsArr.reserve(numOfBuffersInKernelArgs + cmdParams->uiNonArgSvmBuffersCount);
        m_accessFlagsArr.reserve(numOfBuffersInKernelArgs + cmdParams->uiNonArgSvmBuffersCount);

        // Now fill the parameters
        ndrange_dispatcher_data* dispatcherData = (ndrange_dispatcher_data*)m_pDispatchData;

        // Fill NDRange work data
        dispatcherData->AssignWorkData(cmdParams);

        // Get device side kernel address and set kernel directive
        dispatcherData->kernelAddress = ProgramService::GetDeviceSideKernel(cmdParams->kernel);

        if (0 == dispatcherData->kernelAddress)
        {
            returnError = CL_DEV_INVALID_KERNEL;
            break;
        }

        char* pKernelArgs = (char*)cmdParams->arg_values + sizeof(ndrange_dispatcher_data);

        const cl_kernel_argument*   pBEParams = pKernel->GetKernelParams();
        for (unsigned int i = 0; i < numOfBuffersInKernelArgs; ++i)
        {
            const cl_kernel_argument&             paramDesc = pBEParams[pBufferArgsInx[i]];
            
            MICDevMemoryObject *memObj = *(MICDevMemoryObject**)(pKernelArgs+paramDesc.offset_in_bytes);
            if ( NULL == memObj )
                continue;

            AddMemoryObject(memObj, CL_KRNL_ARG_PTR_CONST == paramDesc.type);
        }

        // set unique command identifier
        dispatcherData->commandIdentifier = (uint64_t)m_pCmd->id;
        // set device side command queue pointer
        dispatcherData->deviceQueuePtr = m_pCommandList->getDeviceQueueAddress();

#ifdef __NEW_BE_API__
        // Initialize BE parameters here, call to new API
#endif

        // If the CommandList is OOO
        if ( !m_pCommandList->isInOrderCommandList() )
        {
            if ( m_pCmd->profiling )
            {
                assert(m_pCommandList->isProfilingEnabled() && "Profiling is set for command, but list is not supporting it");
                registerBarrier(m_startEvent);
            }
            // Register completion barrier
            registerBarrier(m_endEvent);

        }
        dispatcherData->startEvent = m_startEvent;
        dispatcherData->endEvent = m_endEvent;
    }
    while (0);

    m_lastError = returnError;

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    if ( (NULL != m_pCommandList->GetGPAInfo()) && m_pCommandList->GetGPAInfo()->bUseGPA )
    {
        __itt_task_end(m_pCommandList->GetGPAInfo()->pDeviceDomain);
    }
#endif

    return returnError;
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

cl_dev_err_code FillMemObject::init()
{
    cl_dev_err_code returnError = CL_DEV_SUCCESS;

    cl_dev_cmd_param_fill*            cmdParams = (cl_dev_cmd_param_fill*)m_pCmd->params;
    MICDevMemoryObject*               pMicMemObj;
    fill_mem_obj_dispatcher_data      fillMemObjDispatcherData;

    // The do .... while (0) is a pattern when there are many failures points instead of goto operation use do ... while (0) with break commands.
    do {
        returnError = cmdParams->memObj->clDevMemObjGetDescriptor(CL_DEVICE_TYPE_ACCELERATOR, 0, (cl_dev_memobj_handle*)&pMicMemObj);
        if (CL_DEV_FAILED(returnError))
        {
          break;
        }

        // TODO: use tbb::scalable allocator
        m_pDispatchData = &m_fillDispatchData;
        m_uiDispatchDataSize = sizeof(m_fillDispatchData);

        const cl_mem_obj_descriptor& pMemObj = pMicMemObj->clDevMemObjGetDescriptorRaw();

        m_fillDispatchData.deviceQueuePtr = m_pCommandList->getDeviceQueueAddress();
        m_fillDispatchData.commandIdentifier = (uint64_t)m_pCmd->id;

        // copy the dimension value
        assert(pMemObj.dim_count == cmdParams->dim_count);
        m_fillDispatchData.dim_count = cmdParams->dim_count;
        m_fillDispatchData.from_offset = MemoryAllocator::CalculateOffset(m_fillDispatchData.dim_count, cmdParams->offset, pMemObj.pitch, pMemObj.uiElementSize);

        // Set region
        memcpy(m_fillDispatchData.vRegion, cmdParams->region, sizeof(m_fillDispatchData.vRegion));
        m_fillDispatchData.vRegion[0] = cmdParams->region[0] * pMemObj.uiElementSize;

        // Set pitch
        memcpy(m_fillDispatchData.vFromPitch, pMemObj.pitch, sizeof(m_fillDispatchData.vFromPitch));

        // Set pattern
        assert(cmdParams->pattern_size <= MAX_PATTERN_SIZE && "Pattern size is too large");
        if (cmdParams->pattern_size > MAX_PATTERN_SIZE)
        {
            returnError = CL_DEV_INVALID_VALUE;
            break;
        }
        memcpy(m_fillDispatchData.pattern, cmdParams->pattern, cmdParams->pattern_size);
        m_fillDispatchData.pattern_size = cmdParams->pattern_size;

        // Optimization which send the COI buffer as COI_SINK_WRITE_ENTIRE if the user like to over-write the whole buffer.
        COI_ACCESS_FLAGS dstBuffAccessFlag = COI_SINK_WRITE_ENTIRE;
        for (unsigned int i = 0; i < m_fillDispatchData.dim_count; i++)
        {
            if ((cmdParams->offset[i] != 0) || ((pMemObj.memObjType == CL_MEM_OBJECT_BUFFER) && (cmdParams->region[i] != pMemObj.dimensions.buffer_size)) ||
              ((pMemObj.memObjType != CL_MEM_OBJECT_BUFFER) && (cmdParams->region[i] != pMemObj.dimensions.dim[i])))
            {
                dstBuffAccessFlag = COI_SINK_WRITE;
                break;
            }
        }

        // Add the destination buffer and set its directive as pre exe directive
        m_coiBuffsArr.push_back(pMicMemObj->clDevMemObjGetCoiBufferHandler());
        m_accessFlagsArr.push_back(dstBuffAccessFlag);

        // Register start barrier
        // If it is OutOfOrderCommandList, add BARRIER directive to postExeDirectives
        if ( !m_pCommandList->isInOrderCommandList() )
        {
            // Register start barrier
            if ( m_pCmd->profiling )
            {
                assert(m_pCommandList->isProfilingEnabled() && "Profiling is set for command, but list is not supporting it");
                registerBarrier(m_startEvent);
            }
            // Register completion barrier
            registerBarrier(m_endEvent);
        }

        m_fillDispatchData.startEvent = m_startEvent;
        m_fillDispatchData.endEvent = m_endEvent;

    } while (0);

    m_lastError = returnError;

    return returnError;
}
