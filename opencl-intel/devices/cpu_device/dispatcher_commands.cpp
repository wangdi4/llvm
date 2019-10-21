// INTEL CONFIDENTIAL
//
// Copyright 2006-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

///////////////////////////////////////////////////////////////////////////
//  dispatcher_commands.cpp
//  Implementation of the execution of internal task dispatcher commands
/////////////////////////////////////////////////////////////////////////////

#include "dispatcher_commands.h"
#include "task_dispatcher.h"
#include "cpu_logger.h"
#include "cpu_dev_limits.h"
#include "cl_heap.h"
#include "cl_shared_ptr.hpp"
#include "cl_user_logger.h"
#include <builtin_kernels.h>
#include <cl_dev_backend_api.h>
#include <cl_sys_defines.h>
#include <ocl_itt.h>

#if defined (_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#define getR(color) ((color >> 16) & 0xFF)
#define getG(color) ((color >> 8) & 0xFF)
#define getB(color) (color & 0xFF)

// This string size is calculated as 6x(the maximal number of digits assuming size_t is 64 bit) + 8
#define GPA_RANGE_STRING_SIZE 130

#define COLOR(R,G,B)    (((R)<<16) + ((G)<<8) + (B))
using namespace Intel::OpenCL::CPUDevice;
using namespace Intel::OpenCL;
unsigned int NDRange::RGBTable[COLOR_TABLE_SIZE] = {
    COLOR(204,0,102),    COLOR(153,51,153),    COLOR(102,51,204),    COLOR(51,0,253),
    COLOR(102,153,255),    COLOR(0,204,204),    COLOR(153,255,204),    COLOR(0,153,102),
    COLOR(0,51,0),        COLOR(204,255,51),    COLOR(255,255,0),    COLOR(204,51,0),
    COLOR(255,153,255),    COLOR(204,102,255),    COLOR(51,0,51),        COLOR(51,0,153),
    COLOR(0,102,204),    COLOR(51,255,255),    COLOR(0,204,153),    COLOR(0,204,102),
    COLOR(102,204,51),    COLOR(153,204,0),    COLOR(255,255,102),    COLOR(255,102,0),
    COLOR(255,153,204),    COLOR(204,51,204),    COLOR(153,51,255),    COLOR(51,51,102),
    COLOR(0,0,204),        COLOR(153,255,255),    COLOR(51,255,204),    COLOR(51,255,153),
    COLOR(102,255,51),    COLOR(51,102,0),    COLOR(255,255,204),    COLOR(255,102,51),
    COLOR(102,0,51),    COLOR(153,0,102),    COLOR(102,0,102),    COLOR(102,102,255),
    COLOR(0,0,153),        COLOR(51,204,255),    COLOR(0,51,51),        COLOR(153,204,153),
    COLOR(0,204,51),    COLOR(102,153,51),    COLOR(153,153,102),    COLOR(204,102,51),
    COLOR(153,0,51),    COLOR(255,0,102),    COLOR(204,0,204),    COLOR(102,0,204),
    COLOR(51,51,255),    COLOR(0,51,102),    COLOR(0,153,153),    COLOR(0,255,153),
    COLOR(0,153,51),    COLOR(0,153,0),        COLOR(204,255,0),    COLOR(255,204,0),        
    COLOR(204,204,204),    COLOR(153,153,153),    COLOR(102,102,102),    COLOR(51,51,51)};

AtomicCounter NDRange::RGBTableCounter;

using namespace Intel::OpenCL::BuiltInKernels;
using Intel::OpenCL::Utils::g_pUserLogger;

/**
 * Debug prints flag. Required for (weird) platforms like Linux, where our logger does not work.
 */
//#define _DEBUG_PRINT

#if defined(__INCLUDE_MKL__) && defined(__OMP2TBB__)
extern "C" void __omp2tbb_set_thread_max_concurency(int max_concurency);
#endif

///////////////////////////////////////////////////////////////////////////
// Base dispatcher command
DispatcherCommand::DispatcherCommand(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd) :
m_pTaskDispatcher(pTD), m_pCmd(pCmd), m_bCompleted(false)
{
    assert(pTD && "Expected non NULL TaskDispatcher");
    assert(pCmd && "Expected non NULL command descriptor");
    m_pLogDescriptor = pTD->m_pLogDescriptor;
    m_iLogHandle = pTD->m_iLogHandle;
    m_pGPAData = pTD->m_pGPAData;
#if defined(USE_ITT)
    m_ittID = __itt_null;
    if ((nullptr != m_pGPAData) && (m_pGPAData->bUseGPA))
    {
        // unique ID to pass all tasks, and markers.
        m_ittID = __itt_id_make(&m_ittID, (unsigned long long)this);
        __itt_id_create(m_pGPAData->pDeviceDomain, m_ittID);
    }
#endif
}

DispatcherCommand::~DispatcherCommand()
{
#if defined(USE_ITT)
    if ((nullptr != m_pGPAData) && (m_pGPAData->bUseGPA))
    {
        __itt_id_destroy(m_pGPAData->pDeviceDomain, m_ittID);
    }
#endif
}

void DispatcherCommand::NotifyCommandStatusChanged(cl_dev_cmd_desc* cmd, unsigned uStatus, int iErr)
{
    if ( CL_COMPLETE == uStatus )
    {
        void* pTaskPtr = cmd->device_agent_data;
        // If the ITask pointer still exists we need reduce reference count
        // and release ITask object
        if ( nullptr != pTaskPtr )
        {
            ITaskBase* pTask = static_cast<ITaskBase*>(pTaskPtr);
            pTask->Release();
        }
        m_bCompleted = true;
    }
    m_pTaskDispatcher->NotifyCommandStatusChange(cmd, uStatus, iErr);
}

cl_dev_err_code DispatcherCommand::ExtractNDRangeParams(void* pTargetTaskParam, 
                                                        const cl_kernel_argument*   pParams,
                                                        const unsigned int* pMemObjectIndx,
                                                        unsigned int uiMemObjCount,
                                                        std::vector<cl_mem_obj_descriptor*>* devMemObjects,
                                                        std::vector<char>* kernelParamsVec)
{
    char* pLockedParams = (char*)pTargetTaskParam;

#ifdef OCLDEVICE_PLUGINS
    // save copy of kernel params, because *pLockedParams will be changed below
    char* pKernelParams = &(*kernelParamsVec)[0];
    size_t sizeParams = (*kernelParamsVec).size();
    MEMCPY_S(pKernelParams, sizeParams, pLockedParams, sizeParams);
#endif

    // Lock required memory objects, in place operation
    for(unsigned int i=0; i<uiMemObjCount; ++i)
    {
        const cl_kernel_argument&   param       = pParams[pMemObjectIndx[i]];
        size_t                      stOffset    = param.offset_in_bytes;
        IOCLDevMemoryObject*        memObj     = *(IOCLDevMemoryObject**)(pLockedParams+stOffset);

        assert( ((CL_KRNL_ARG_PTR_CONST == param.type) || (CL_KRNL_ARG_PTR_GLOBAL == param.type) || (nullptr != memObj)) && "NULL is not allowed for non buffer arguments");
        if (nullptr != memObj)
        {
            char* loc = pLockedParams+stOffset;
            cl_mem_obj_descriptor* objHandle;
            memObj->clDevMemObjGetDescriptor(CL_DEVICE_TYPE_CPU, 0, (void**)&objHandle );
#ifdef OCLDEVICE_PLUGINS
            *(cl_mem_obj_descriptor**)(pKernelParams+stOffset) = objHandle;
#endif
            if ( objHandle->memObjType == CL_MEM_OBJECT_BUFFER ||
                 objHandle->memObjType == CL_MEM_OBJECT_PIPE )
            {
                *(void**)loc = objHandle->pData;
            }
            else
            {
                *(void**)loc = objHandle->imageAuxData;
            }
            if ( nullptr != devMemObjects )
            {
                devMemObjects->push_back(objHandle);
            }
        }
    };

    return CL_DEV_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////
// CommandBaseClass
template <class ITaskClass>
    bool CommandBaseClass<ITaskClass>::SetAsSyncPoint()
{
    long prev = m_aIsSyncPoint.exchange(1);
    return 1 == prev;
}

template <class ITaskClass>
    bool CommandBaseClass<ITaskClass>::CompleteAndCheckSyncPoint()
{
    // The queue some how need to be signaled stop processing of the job list.
    // On other side RT should be aware that the command was done.
    // And it should be done by single instruction, otherwise there is a race
    long prev = m_aIsSyncPoint.exchange(1);
    return 1 == prev;
}

///////////////////////////////////////////////////////////////////////////
// OCL Read/Write buffer execution

cl_dev_err_code ReadWriteMemObject::Create(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, SharedPtr<ITaskBase>* pTask, const SharedPtr<ITaskList>& pList)
{
    ReadWriteMemObject* pCommand = new ReadWriteMemObject(pTD, pCmd);
    if (nullptr == pCommand)
    {
        return CL_DEV_OUT_OF_MEMORY;
    }

    // run CheckCommandParams only in Debug mode
    assert( CL_DEV_SUCCESS == pCommand->CheckCommandParams(pCmd) && "Wrong params" );

    assert(pTask);
    *pTask = pCommand;

    return CL_DEV_SUCCESS;
}

ReadWriteMemObject::ReadWriteMemObject(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd) :
    CommandBaseClass<ITask>(pTD, pCmd)
{
}

cl_dev_err_code ReadWriteMemObject::CheckCommandParams(cl_dev_cmd_desc* cmd)
{
    if ( (CL_DEV_CMD_READ != cmd->type) && (CL_DEV_CMD_WRITE != cmd->type) )
    {
        return CL_DEV_INVALID_COMMAND_TYPE;
    }

    if ( sizeof(cl_dev_cmd_param_rw) != cmd->param_size )
    {
        return CL_DEV_INVALID_COMMAND_PARAM;
    }

    cl_dev_cmd_param_rw *cmdParams = (cl_dev_cmd_param_rw*)(cmd->params);

    for (unsigned int i=0; i< cmdParams->dim_count-1; i++)
    {
        if(0 == cmdParams->pitch[i])
        {
            return CL_DEV_INVALID_VALUE;
        }
    }

    // Check Region
    return CL_DEV_SUCCESS;
}

bool ReadWriteMemObject::Execute()
{
    cl_dev_cmd_param_rw*    cmdParams = (cl_dev_cmd_param_rw*)m_pCmd->params;
    cl_mem_obj_descriptor*    pMemObj;
    SMemCpyParams            sCpyParam;

    NotifyCommandStatusChanged(m_pCmd, CL_RUNNING, CL_DEV_SUCCESS);

    cmdParams->memObj->clDevMemObjGetDescriptor(CL_DEVICE_TYPE_CPU, 0, (cl_dev_memobj_handle*)&pMemObj);

    void*    pObjPtr;
    size_t* pObjPitchPtr = cmdParams->memobj_pitch[0] ? cmdParams->memobj_pitch : pMemObj->pitch;

#ifdef _DEBUG_PRINT
    printf("--> ReadWriteMemObject(start), cmdid:%p\n", m_pCmd->id);
#endif


    // Request access on default device

    sCpyParam.uiDimCount = cmdParams->dim_count;
    pObjPtr = MemoryAllocator::CalculateOffsetPointer(pMemObj->pData, sCpyParam.uiDimCount, cmdParams->origin, pObjPitchPtr, pMemObj->uiElementSize);

    // Set Source/Destination
    MEMCPY_S(sCpyParam.vRegion, sizeof(sCpyParam.vRegion), cmdParams->region, sizeof(sCpyParam.vRegion));
    sCpyParam.vRegion[0] = cmdParams->region[0] * pMemObj->uiElementSize;

    // In case the pointer parameter (Destination for CMD_READ and Source for CMD_WRITE) has pitch properties,
    // we need to consider that too.
    size_t ptrOffset =    cmdParams->ptr_origin[2] * cmdParams->pitch[1] + \
                        cmdParams->ptr_origin[1] * cmdParams->pitch[0] + \
                        cmdParams->ptr_origin[0];

    if ( CL_DEV_CMD_READ == m_pCmd->type )
    {
        sCpyParam.pSrc = (cl_char*)pObjPtr;
        MEMCPY_S(sCpyParam.vSrcPitch, sizeof(sCpyParam.vSrcPitch), pObjPitchPtr, sizeof(sCpyParam.vSrcPitch));
        sCpyParam.pDst = (cl_char*)((size_t)cmdParams->ptr + ptrOffset);
        MEMCPY_S(sCpyParam.vDstPitch, sizeof(sCpyParam.vDstPitch), cmdParams->pitch, sizeof(sCpyParam.vDstPitch));
    }
    else
    {
        sCpyParam.pSrc = (cl_char*)((size_t)cmdParams->ptr + ptrOffset);
        MEMCPY_S(sCpyParam.vSrcPitch, sizeof(sCpyParam.vSrcPitch), cmdParams->pitch, sizeof(sCpyParam.vSrcPitch));
        sCpyParam.pDst = (cl_char*)pObjPtr;
        MEMCPY_S(sCpyParam.vDstPitch, sizeof(sCpyParam.vDstPitch), pObjPitchPtr, sizeof(sCpyParam.vDstPitch));
    }

    // Execute copy routine
#if defined(USE_ITT)
    if ((nullptr != m_pGPAData) && (m_pGPAData->bUseGPA))
    {

#if defined(USE_GPA)
        __itt_set_track(nullptr);
#endif
        __itt_task_begin(m_pGPAData->pDeviceDomain, m_ittID, __itt_null, ( CL_DEV_CMD_READ == m_pCmd->type ? m_pGPAData->pReadHandle : m_pGPAData->pWriteHandle ));

#if defined(USE_GPA)
        TAL_SetNamedTaskColor((CL_DEV_CMD_READ == m_pCmd->type ? "Read" : "Write"), 255, 0, 0);
#endif

        // Copy dimensions to 64 bit, for uniformity.
        cl_ulong copyParams64[MAX_WORK_DIM];
        copyParams64[0] = sCpyParam.vRegion[0];
        copyParams64[1] = cmdParams->region[1];
        copyParams64[2] = cmdParams->region[2];

        __itt_metadata_add(m_pGPAData->pDeviceDomain, m_ittID, m_pGPAData->pSizeHandle, __itt_metadata_u64, cmdParams->dim_count, copyParams64);
    }
#endif // ITT

    clCopyMemoryRegion(&sCpyParam);

#if defined(USE_ITT)
    if ((nullptr != m_pGPAData) && (m_pGPAData->bUseGPA))
    {
#if defined(USE_GPA)
        __itt_set_track(nullptr);
#endif
        __itt_task_end(m_pGPAData->pDeviceDomain);
    }
#endif // ITT

#ifdef _DEBUG_PRINT
    printf("--> ReadWriteMemObject(end), cmdid:%p(%d)\n", m_pCmd->id, CL_DEV_SUCCESS);
#endif
    NotifyCommandStatusChanged(m_pCmd, CL_COMPLETE, CL_DEV_SUCCESS);

    return true;
}

///////////////////////////////////////////////////////////////////////////
// OCL Copy memory object execution
cl_dev_err_code CopyMemObject::Create(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, SharedPtr<ITaskBase>* pTask, const SharedPtr<ITaskList>& pList)
{
    CopyMemObject* pCommand = new CopyMemObject(pTD, pCmd);
    if (nullptr == pCommand)
    {
        return CL_DEV_OUT_OF_MEMORY;
    }
#ifdef _DEBUG
    cl_dev_err_code rc;
    rc = pCommand->CheckCommandParams(pCmd);
    if( CL_DEV_FAILED(rc))
    {
        delete pCommand;
        return rc;
    }
#endif

    assert(pTask);
    *pTask = static_cast<ITaskBase*>(pCommand);

    return CL_DEV_SUCCESS;
}

CopyMemObject::CopyMemObject(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd) :
    CommandBaseClass<ITask>(pTD, pCmd)
{
}

cl_dev_err_code CopyMemObject::CheckCommandParams(cl_dev_cmd_desc* cmd)
{
    if(CL_DEV_CMD_COPY != cmd->type)
    {
        return CL_DEV_INVALID_COMMAND_TYPE;
    }

    if ( sizeof(cl_dev_cmd_param_copy) != cmd->param_size )
    {
        return CL_DEV_INVALID_COMMAND_PARAM;
    }

    return CL_DEV_SUCCESS;
}

bool CopyMemObject::Execute()
{
    cl_dev_cmd_param_copy*    cmdParams = (cl_dev_cmd_param_copy*)m_pCmd->params;
    cl_mem_obj_descriptor*    pSrcMemObj;;
    cl_mem_obj_descriptor*    pDstMemObj;
    SMemCpyParams            sCpyParam;

    cmdParams->srcMemObj->clDevMemObjGetDescriptor(CL_DEVICE_TYPE_CPU, 0, (cl_dev_memobj_handle*)&pSrcMemObj);
    cmdParams->dstMemObj->clDevMemObjGetDescriptor(CL_DEVICE_TYPE_CPU, 0, (cl_dev_memobj_handle*)&pDstMemObj);

    size_t  uiSrcElementSize = pSrcMemObj->uiElementSize;
    size_t    uiDstElementSize = pDstMemObj->uiElementSize;

    // Objects has to have same element size or buffer<->image
    if( (uiDstElementSize != uiSrcElementSize) &&
        (1 != uiDstElementSize) && (1 != uiSrcElementSize) )
    {
#ifdef _DEBUG_PRINT
        printf("--> CopyMemObject(fail,3), cmdid:%p(%d)\n", m_pCmd->id, CL_DEV_INVALID_COMMAND_PARAM);
#endif
        NotifyCommandStatusChanged(m_pCmd, CL_COMPLETE, (cl_int)CL_DEV_INVALID_COMMAND_PARAM);
        return true;
    }

    // No we can notify that we are running
    NotifyCommandStatusChanged(m_pCmd, CL_RUNNING, CL_DEV_SUCCESS);

    //Options for different dimensions are
    //Copy a 2D image object to a 2D slice of a 3D image object.
    //Copy a 2D slice of a 3D image object to a 2D image object.
    //Copy 2D to 2D
    //Copy 3D to 3D
    //Copy 2D image to buffer
    //Copy 3D image to buffer
    //Buffer to image
    MEMCPY_S(sCpyParam.vSrcPitch,  sizeof(sCpyParam.vSrcPitch), cmdParams->src_pitch[0] ? cmdParams->src_pitch : pSrcMemObj->pitch, sizeof(sCpyParam.vSrcPitch));
    MEMCPY_S(sCpyParam.vDstPitch, sizeof(sCpyParam.vDstPitch), cmdParams->dst_pitch[0] ? cmdParams->dst_pitch : pDstMemObj->pitch, sizeof(sCpyParam.vDstPitch));

    sCpyParam.pSrc = (cl_char*)MemoryAllocator::CalculateOffsetPointer(pSrcMemObj->pData, cmdParams->src_dim_count, cmdParams->src_origin, sCpyParam.vSrcPitch, pSrcMemObj->uiElementSize);
    sCpyParam.pDst = (cl_char*)MemoryAllocator::CalculateOffsetPointer(pDstMemObj->pData, cmdParams->dst_dim_count, cmdParams->dst_origin, sCpyParam.vDstPitch, pDstMemObj->uiElementSize);

    sCpyParam.uiDimCount = min(cmdParams->src_dim_count, cmdParams->dst_dim_count);
    //Buffer to image
    if (CL_MEM_OBJECT_BUFFER == pSrcMemObj->memObjType && CL_MEM_OBJECT_BUFFER != pDstMemObj->memObjType)
    {
        uiSrcElementSize = uiDstElementSize;
        sCpyParam.uiDimCount = cmdParams->dst_dim_count;
        sCpyParam.vSrcPitch[0] = cmdParams->region[0] * uiDstElementSize;
        sCpyParam.vSrcPitch[1] = sCpyParam.vSrcPitch[0] * cmdParams->region[1];
    }
    else if (CL_MEM_OBJECT_BUFFER == pDstMemObj->memObjType && CL_MEM_OBJECT_BUFFER != pSrcMemObj->memObjType)
    {
        //When destination is buffer the memcpy will be done as if the buffer is an image with height=1
        sCpyParam.uiDimCount = cmdParams->src_dim_count;
        sCpyParam.vDstPitch[0] = cmdParams->region[0] * uiSrcElementSize;
        sCpyParam.vDstPitch[1] = sCpyParam.vDstPitch[0] * cmdParams->region[1];
    }

    //If row_pitch (or input_row_pitch) is set to 0, the appropriate row pitch is calculated
    //based on the size of each element in bytes multiplied by width.
    MEMCPY_S(sCpyParam.vRegion, sizeof(sCpyParam.vRegion), cmdParams->region, sizeof(sCpyParam.vRegion));
    sCpyParam.vRegion[0] *= uiSrcElementSize;

#if defined(USE_ITT)
    // Execute copy routine
    if ((nullptr != m_pGPAData) && (m_pGPAData->bUseGPA))
    {
#if defined(USE_GPA)
        __itt_set_track(nullptr);
#endif
        __itt_task_begin(m_pGPAData->pDeviceDomain, m_ittID, __itt_null, m_pGPAData->pCopyHandle);
#if defined(USE_GPA)
        TAL_SetNamedTaskColor("Copy", 255, 0, 0);
#endif

        // Copy dimensions to 64 bit, for uniformity.
        cl_ulong copyParams64[MAX_WORK_DIM];
        copyParams64[0] = sCpyParam.vRegion[0];
        copyParams64[1] = cmdParams->region[1];
        copyParams64[2] = cmdParams->region[2];

        __itt_metadata_add(m_pGPAData->pDeviceDomain, m_ittID, m_pGPAData->pSizeHandle, __itt_metadata_u64 , cmdParams->src_dim_count, copyParams64);
    }
#endif // ITT
    
    // Execute copy routine
    clCopyMemoryRegion(&sCpyParam);

#if defined(USE_ITT)
    if ((nullptr != m_pGPAData) && (m_pGPAData->bUseGPA))
    {
#if defined(USE_GPA)
        __itt_set_track(nullptr);
#endif
        __itt_task_end(m_pGPAData->pDeviceDomain);
    } 
#endif

#ifdef _DEBUG_PRINT
    printf("--> CopyMemObject(end), cmd_id:%d(%d)\n", m_pCmd->id, CL_DEV_SUCCESS);
#endif
    NotifyCommandStatusChanged(m_pCmd, CL_COMPLETE, CL_DEV_SUCCESS);

    return true;
}

///////////////////////////////////////////////////////////////////////////
// OCL Native function execution
cl_dev_err_code NativeFunction::Create(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, SharedPtr<ITaskBase>* pTask, const SharedPtr<ITaskList>& pList)
{
    NativeFunction* pCommand = new NativeFunction(pTD, pCmd);
    if (nullptr == pCommand)
    {
        return CL_DEV_OUT_OF_MEMORY;
    }
#ifdef _DEBUG
    cl_dev_err_code rc;
    rc = pCommand->CheckCommandParams(pCmd);
    if( CL_DEV_FAILED(rc))
    {
        delete pCommand;
        return rc;
    }
#endif

    cl_dev_cmd_param_native *cmdParams = (cl_dev_cmd_param_native*)pCmd->params;

    // Create temporal buffer for execution
    char*    pArgV = new char[cmdParams->args];
    if ( nullptr == pArgV )
    {
#ifdef _DEBUG
        CpuErrLog(pCommand->m_pLogDescriptor, pCommand->m_iLogHandle, TEXT("%s"), TEXT("Can't allocate memory for parameters"));
#endif
        delete pCommand;
        return CL_DEV_OUT_OF_MEMORY;
    }
    pCommand->m_pArgV = pArgV;
    MEMCPY_S(pArgV, cmdParams->args, cmdParams->argv, cmdParams->args);

    assert(pTask);
    *pTask = static_cast<ITaskBase*>(pCommand);
    return CL_DEV_SUCCESS;
}

NativeFunction::NativeFunction(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd) :
    CommandBaseClass<ITask>(pTD, pCmd),
    m_pArgV(nullptr)
{
}

cl_dev_err_code    NativeFunction::CheckCommandParams(cl_dev_cmd_desc* cmd)
{
    if ( CL_DEV_CMD_EXEC_NATIVE != cmd->type )
    {
        return CL_DEV_INVALID_COMMAND_TYPE;
    }

    if ( sizeof(cl_dev_cmd_param_native) != cmd->param_size )
    {
        return CL_DEV_INVALID_COMMAND_PARAM;
    }

    cl_dev_cmd_param_native *cmdParams = (cl_dev_cmd_param_native*)(cmd->params);

    if( nullptr == cmdParams->func_ptr )
    {
        return CL_DEV_INVALID_COMMAND_PARAM;
    }

    return CL_DEV_SUCCESS;
}

bool NativeFunction::Execute()
{
    cl_dev_cmd_param_native *cmdParams = (cl_dev_cmd_param_native*)m_pCmd->params;

    NotifyCommandStatusChanged(m_pCmd, CL_RUNNING, CL_DEV_SUCCESS);

    for(unsigned int i=0; (i<cmdParams->mem_num); ++i )
    {
        IOCLDevMemoryObject *memObj = *((IOCLDevMemoryObject**)((((char*)cmdParams->argv)+cmdParams->mem_offset[i])));
        
        cl_mem_obj_descriptor* memObjDesc;
        memObj->clDevMemObjGetDescriptor(CL_DEVICE_TYPE_CPU, 0, (cl_dev_memobj_handle*)&memObjDesc);

        void*    *pMemPtr = (void**)((cl_char*)m_pArgV+cmdParams->mem_offset[i]);
        *pMemPtr = memObjDesc->pData;
    }

    // Notify start execution
    NotifyCommandStatusChanged(m_pCmd, CL_RUNNING, CL_DEV_SUCCESS);

    // Execute native function
    fn_clNativeKernel *func = (fn_clNativeKernel*)cmdParams->func_ptr;
    func(m_pArgV);

    // Free memory allocated to execution parameters
    delete []m_pArgV;

    NotifyCommandStatusChanged(m_pCmd, CL_COMPLETE, CL_DEV_SUCCESS);
    return true;
}

///////////////////////////////////////////////////////////////////////////
// OCL Map buffer execution
//////////////////////////////////////////////////////////////////////////
cl_dev_err_code MapMemObject::Create(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, SharedPtr<ITaskBase>* pTask, const SharedPtr<ITaskList>& pList)
{
    MapMemObject* pCommand = new MapMemObject(pTD, pCmd);
    if (nullptr == pCommand)
    {
        return CL_DEV_OUT_OF_MEMORY;
    }
#ifdef _DEBUG
    cl_dev_err_code rc;
    rc = pCommand->CheckCommandParams(pCmd);
    if( CL_DEV_FAILED(rc))
    {
        delete pCommand;
        return rc;
    }
#endif

    assert(pTask);
    *pTask = static_cast<ITaskBase*>(pCommand);
    return CL_DEV_SUCCESS;
}

MapMemObject::MapMemObject(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd) :
    CommandBaseClass<ITask>(pTD, pCmd)
{
}

cl_dev_err_code MapMemObject::CheckCommandParams(cl_dev_cmd_desc* cmd)
{
    if (CL_DEV_CMD_MAP != cmd->type)
    {
        return CL_DEV_INVALID_COMMAND_TYPE;
    }

    if ( sizeof(cl_dev_cmd_param_map) != cmd->param_size )
    {
        return CL_DEV_INVALID_COMMAND_PARAM;
    }

    return CL_DEV_SUCCESS;
}

bool MapMemObject::Execute()
{
    NotifyCommandStatusChanged(m_pCmd, CL_RUNNING, CL_DEV_SUCCESS);

#ifdef _DEBUG_PRINT
    printf("--> MapMemObject(start), cmdid:%p\n", m_pCmd->id);
#endif

#if defined(USE_ITT)
    // Write Map task to ITT trace
    if ((nullptr != m_pGPAData) && (m_pGPAData->bUseGPA))
    {
#if defined(USE_GPA)
        __itt_set_track(nullptr);
#endif
        __itt_task_begin(m_pGPAData->pDeviceDomain, m_ittID, __itt_null, m_pGPAData->pMapHandle);
#if defined(USE_GPA)
        TAL_SetNamedTaskColor("Map", 255, 0, 0);
#endif
    }

    if ((nullptr != m_pGPAData) && (m_pGPAData->bUseGPA))
    {
#if defined(USE_GPA)
        __itt_set_track(nullptr);
#endif
        __itt_task_end(m_pGPAData->pDeviceDomain);
    } 
#endif

#ifdef _DEBUG_PRINT
    printf("--> MapMemObject(end), cmdid:%p\n", m_pCmd->id);
#endif

    NotifyCommandStatusChanged(m_pCmd, CL_COMPLETE, CL_DEV_SUCCESS);

    return true;
}

///////////////////////////////////////////////////////////////////////////
// OCL Unmap buffer execution
//////////////////////////////////////////////////////////////////////////
cl_dev_err_code UnmapMemObject::Create(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, SharedPtr<ITaskBase>* pTask, const SharedPtr<ITaskList>& pList)
{
    UnmapMemObject* pCommand = new UnmapMemObject(pTD, pCmd);
    if (nullptr == pCommand)
    {
        return CL_DEV_OUT_OF_MEMORY;
    }
#ifdef _DEBUG
    cl_dev_err_code rc;
    rc = pCommand->CheckCommandParams(pCmd);
    if( CL_DEV_FAILED(rc))
    {
        delete pCommand;
        return rc;
    }
#endif

    assert(pTask);
    *pTask = static_cast<ITaskBase*>(pCommand);
    return CL_DEV_SUCCESS;
}

UnmapMemObject::UnmapMemObject(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd) :
    CommandBaseClass<ITask>(pTD, pCmd)
{
}

cl_dev_err_code UnmapMemObject::CheckCommandParams(cl_dev_cmd_desc* cmd)
{
    if (CL_DEV_CMD_UNMAP != cmd->type)
    {
        return CL_DEV_INVALID_COMMAND_TYPE;
    }

    if ( sizeof(cl_dev_cmd_param_map) != cmd->param_size )
    {
        return CL_DEV_INVALID_COMMAND_PARAM;
    }

    return CL_DEV_SUCCESS;
}

bool UnmapMemObject::Execute()
{
    NotifyCommandStatusChanged(m_pCmd, CL_RUNNING, CL_DEV_SUCCESS);

#ifdef _DEBUG_PRINT
    printf("--> UnmapMemObject(start), cmdid:%p\n", m_pCmd->id);
#endif

#ifdef _DEBUG_PRINT
    printf("--> UnmapMemObject(end), cmdid:%p\n", m_pCmd->id);
#endif

    NotifyCommandStatusChanged(m_pCmd, CL_COMPLETE, CL_DEV_SUCCESS);

    return true;
}

///////////////////////////////////////////////////////////////////////////
// OCL Kernel execution
Intel::OpenCL::Utils::AtomicCounter    NDRange::s_lGlbNDRangeId;

cl_dev_err_code NDRange::Create(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, SharedPtr<ITaskBase>* pTask, const SharedPtr<ITaskList>& pList)
{
#ifdef __INCLUDE_MKL__
    // First to check if the required NDRange is one of the built-in kernels
    const ProgramService::KernelMapEntry* pKernelEntry = (const ProgramService::KernelMapEntry*)(((cl_dev_cmd_param_kernel*)pCmd->params)->kernel);
    const ICLDevBackendKernel_* pKernel = pKernelEntry->pBEKernel;

    const ICLDevBackendKernelProporties* pProperties = pKernel->GetKernelProporties();
    
    assert( nullptr != pProperties && "Kernel properties always shall exist");

    // Built-in kernel currently returns -1 for Execution lenght properties.
    if ( (size_t)-1 == pProperties->GetKernelExecutionLength() )
    {
        return NativeKernelTask::Create(pTD, pList, pCmd, pTask);
    }
#endif
    pCmd->id = (cl_dev_cmd_id)((long)pCmd->id & ~(1L << (sizeof(long) * 8 - 1)));    // device NDRange IDs have their MSB set, while in host NDRange IDs they're reset
    NDRange* pCommand = new NDRange(pTD, pCmd, pList.GetPtr(), nullptr);

    assert(pTask && "Invalid task parameter");
    *pTask = static_cast<ITaskBase*>(pCommand);

    return CL_DEV_SUCCESS;
}

// Declare local storage variable
THREAD_LOCAL ICLDevBackendKernelRunner::ICLDevExecutionState NDRange::m_tExecState = {0};

NDRange::NDRange(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, ITaskList* pList, KernelCommand* parent) :
    CommandBaseClass<ITaskSet>(pTD, pCmd),
    KernelCommand(pList, parent, this),
    m_lastError(CL_DEV_SUCCESS),
    m_numThreads(0), m_bEnablePredictablePartitioning(false),
    m_lNDRangeId(-1)
{
#ifdef _DEBUG
    m_lFinish.exchange(0);
    m_lAttaching.exchange(0);
    m_lExecuting.exchange(0);
#endif
    m_pImplicitArgs = nullptr;
    m_numThreads = pList->GetDeviceConcurency();
}

int NDRange::Init(size_t region[], unsigned int &dimCount, size_t numberOfThreads)
{
    cl_dev_cmd_param_kernel* cmdParams = (cl_dev_cmd_param_kernel*)m_pCmd->params;


#ifdef _DEBUG_PRINT
    printf("--> Init(start):%s, id(%d)\n", pKernel->GetKernelName(), (int)m_pCmd->id);
#endif

    if ( CL_DEV_FAILED(m_lastError) )
    {
        return m_lastError;
    }

    char* pLockedParams = (char*)cmdParams->arg_values;

    NotifyCommandStatusChanged(m_pCmd, CL_RUNNING, CL_DEV_SUCCESS);

#ifdef USE_ITT
    // Start execution task
    if ((nullptr != m_pGPAData) && (m_pGPAData->bUseGPA))
    {
        cl_dev_cmd_param_kernel *cmdParams = (cl_dev_cmd_param_kernel*)m_pCmd->params;

        __itt_task_begin(m_pGPAData->pDeviceDomain, m_ittID, __itt_null,
            ((const ProgramService::KernelMapEntry*)cmdParams->kernel)->ittTaskNameHandle);
    }
#endif

    const ICLDevBackendKernel_* pKernel = ((const ProgramService::KernelMapEntry*)cmdParams->kernel)->pBEKernel;
    const cl_kernel_argument*   pParams = pKernel->GetKernelParams();
    const ICLDevBackendKernelProporties* pProperties = pKernel->GetKernelProporties();
    m_needSerializeWGs = pProperties->NeedSerializeWGs();

    std::vector<cl_mem_obj_descriptor*> devMemObjects;
    unsigned int uiMemArgCount = pKernel->GetMemoryObjectArgumentCount();
    devMemObjects.reserve(uiMemArgCount + cmdParams->uiNonArgSvmBuffersCount +
                          cmdParams->uiNonArgUsmBuffersCount);

    // Fill with SVM buffers
    devMemObjects.resize(cmdParams->uiNonArgSvmBuffersCount +
                         cmdParams->uiNonArgUsmBuffersCount);
    for(cl_uint i=0;i<cmdParams->uiNonArgSvmBuffersCount;++i)
    {
        cmdParams->ppNonArgSvmBuffers[i]->clDevMemObjGetDescriptor(CL_DEVICE_TYPE_CPU, 0, (void**)&devMemObjects[i] );
    }
    // Fill with USM buffers
    unsigned int offset = cmdParams->uiNonArgSvmBuffersCount;
    for (cl_uint i = 0; i < cmdParams->uiNonArgUsmBuffersCount; ++i)
        cmdParams->ppNonArgUsmBuffers[i]->clDevMemObjGetDescriptor(
            CL_DEVICE_TYPE_CPU, 0, (void**)&devMemObjects[offset + i]);

    std::vector<char> kernelParamsVec;

    if ( uiMemArgCount > 0 )
    {
#ifdef OCLDEVICE_PLUGINS
        kernelParamsVec.resize(cmdParams->arg_size);
#endif
        cl_dev_err_code clRet = ExtractNDRangeParams(pLockedParams, pParams, pKernel->GetMemoryObjectArgumentIndexes(), uiMemArgCount,
                                                     &devMemObjects, &kernelParamsVec);
        if ( CL_DEV_FAILED(clRet) )
        {
            m_lastError = clRet;
            NotifyCommandStatusChanged(m_pCmd, CL_COMPLETE, clRet);
            return clRet;
        }
    }
    m_pKernelArgs = pLockedParams;
    m_pImplicitArgs = (cl_uniform_kernel_args*)(pLockedParams+pKernel->GetExplicitArgumentBufferSize());
    m_pImplicitArgs->WorkDim = cmdParams->work_dim;
    m_pImplicitArgs->RuntimeInterface = static_cast<IDeviceCommandManager*>(this);
    // Copy global_offset, global_size, uniform local_work_size, and non-uniform local_work_size
    for(int i = 0; i < MAX_WORK_DIM; ++i)
    {
        m_pImplicitArgs->GlobalOffset[i] = cmdParams->glb_wrk_offs[i];
        m_pImplicitArgs->LocalSize[UNIFORM_WG_SIZE_INDEX][i]    = cmdParams->lcl_wrk_size[UNIFORM_WG_SIZE_INDEX][i];
        m_pImplicitArgs->LocalSize[NONUNIFORM_WG_SIZE_INDEX][i] = cmdParams->lcl_wrk_size[NONUNIFORM_WG_SIZE_INDEX][i];
        m_pImplicitArgs->GlobalSize[i]   = cmdParams->glb_wrk_size[i];
    }

    m_pImplicitArgs->minWorkGroupNum = m_numThreads;

    m_pRunner = pKernel->GetKernelRunner();
#ifdef OCLDEVICE_PLUGINS
    unsigned int memObjCount = (unsigned int)kernelParamsVec.size();
    const cl_mem_obj_descriptor** memArgs = memObjCount > 0 ? (const cl_mem_obj_descriptor**)&kernelParamsVec[0] : nullptr;
#else
    unsigned int memObjCount = 0;
    const cl_mem_obj_descriptor** memArgs = nullptr;
#endif
    bool zero_enqueue = false;
    for (unsigned int i = 0; i < cmdParams->work_dim; ++i)
    {
        if(!cmdParams->glb_wrk_size[i])
        {
            zero_enqueue = true;
            break;
        }
    }
    if(!zero_enqueue)
    {
        m_pRunner->PrepareKernelArguments(pLockedParams, memArgs, memObjCount, numberOfThreads);
    }

    // if logger is enabled, always print local work size from BE
    if (nullptr != g_pUserLogger && g_pUserLogger->IsApiLoggingEnabled())
    {
        vector<size_t> dims(m_pImplicitArgs->LocalSize[0], &m_pImplicitArgs->LocalSize[0][cmdParams->work_dim]);
        g_pUserLogger->SetLocalWorkSize4ArgValues(m_pCmd->id, dims);
    }

    const size_t*    pWGSize = m_pImplicitArgs->WGCount;
    assert(pWGSize && "pWGSize must be non zero pointer");
    unsigned int i;
    for (i = 0; i < cmdParams->work_dim; ++i)
    {
      region[i] = pWGSize[i];
    }
    for (; i<MAX_WORK_DIM; ++i)
    {
        region[i] = 1;
    }

    dimCount = cmdParams->work_dim;

    //TODO: might want to revisit these restrictions in the future
    //TODO: Use direct access to ITEDevice and expose sub device INFO.
    m_bEnablePredictablePartitioning = ( (1 == dimCount) && (region[0] == m_numThreads) && m_pTaskDispatcher->isPredictablePartitioningAllowed() );
    if ( m_bEnablePredictablePartitioning )
    {
        m_bWGExecuted.init(m_numThreads, false);
    }

#ifdef _DEBUG_PRINT
    printf("--> Init(done):%s\n", pKernel->GetKernelName());
#endif
    m_lNDRangeId = s_lGlbNDRangeId++;
    return CL_DEV_SUCCESS;
}

size_t NDRange::PreferredSequentialItemsPerThread() const
{
    size_t preferredSize = 1;
    if (m_needSerializeWGs)
    {
        assert(m_pImplicitArgs != nullptr && "Init should be called first.");
        const size_t* pWGSize = m_pImplicitArgs->WGCount;
        for (unsigned int i = 0; i < m_pImplicitArgs->WorkDim; ++i)
        {
            // The whole NDRange are splitted to several tasks using grainsize.
            // The important fact that 3D-range are handled as three independent
            // 1D-ranges with the same grainsize computed here.
            // So, to ensure that 3D-range will not be splitted we need to
            // ensure that no one from independent 1D-ranges will not be
            // splitted. That is why it is enough to use maximum number of
            // work-groups among all dimensions instead of total count of
            // work-groups in NDRange.
            preferredSize =
                (preferredSize < pWGSize[i]) ? pWGSize[i] : preferredSize;
        }
    }

    return preferredSize;
}

bool NDRange::Finish(FINISH_REASON reason)
{
    StopExecutionProfiling();
    NotifyCommandStatusChanged(m_pCmd, CL_ENDED_RUNNING, m_lastError);

    // Need to notify all kernel children and wait for their completion
    WaitForChildrenCompletion();

    // regular stuff:
#ifdef _DEBUG
    long lVal = (m_lExecuting.test_and_set(0, 0) | m_lAttaching.test_and_set(0, 0));
    assert(lVal == 0);
    m_lFinish.exchange(1);
#endif

#ifdef _DEBUG
    lVal = (m_lExecuting.test_and_set(0, 0) | m_lAttaching.test_and_set(0, 0));
    assert(lVal == 0);
#endif

    NotifyCommandStatusChanged(m_pCmd, CL_COMPLETE, m_lastError);

#ifdef _DEBUG_PRINT
    cl_dev_cmd_param_kernel *cmdParams = (cl_dev_cmd_param_kernel*)m_pCmd->params;
    const ICLDevBackendKernel_* pKernel = ((const ProgramService::KernelMapEntry*)cmdParams->kernel)->pBEKernel;
    printf("--> Finish(done):%s\n", pKernel->GetKernelName());
#endif
#ifdef _DEBUG
    m_lFinish.exchange(0);
#endif

#if defined(USE_ITT)
    if ((nullptr != m_pGPAData) && (m_pGPAData->bUseGPA))
    {
        __itt_task_end(m_pGPAData->pDeviceDomain);
    }
#endif // ITT

    return true;
}

void* NDRange::AttachToThread(void* pWgContextBase, size_t uiNumberOfWorkGroups, size_t firstWGID[], size_t lastWGID[])
{
#ifdef _DEBUG_PRINT
    printf("AttachToThread %d, WrkId(%d), CmdId(%d)\n", (int)GetCurrentThreadId(), (int)uiWorkerId, (int)m_pCmd->id);
#endif

#ifdef _DEBUG
    long lVal = m_lFinish.test_and_set(0, 0);
    if ( lVal == 1 )
    {
        assert(0);
    }
    ++m_lAttaching ;
#endif

    m_pRunner->PrepareThreadState(m_tExecState);

#ifdef USE_ITT
    // Start execution task
    if ((nullptr != m_pGPAData) && (m_pGPAData->bUseGPA))
    {
        unsigned int uiWorkGroupSize = 1;
        const size_t*    pWGSize = m_pImplicitArgs->WGCount;
        cl_dev_cmd_param_kernel *cmdParams = (cl_dev_cmd_param_kernel*)m_pCmd->params;

        __itt_task_begin(m_pGPAData->pDeviceDomain, m_ittID, __itt_null,
                ((const ProgramService::KernelMapEntry*)cmdParams->kernel)->ittTaskNameHandle);

        for (unsigned int i=0 ; i<cmdParams->work_dim ; ++i)
        {
            uiWorkGroupSize *= (unsigned int)pWGSize[i];
        }
        __itt_metadata_add(m_pGPAData->pDeviceDomain, m_ittID, m_pGPAData->pWorkGroupSizeHandle, __itt_metadata_u32 , 1, &uiWorkGroupSize);

        cl_ulong numOfWGs64 = uiNumberOfWorkGroups;
        __itt_metadata_add(m_pGPAData->pDeviceDomain, m_ittID, m_pGPAData->pNumberOfWorkGroupsHandle, __itt_metadata_u64 , 1, &numOfWGs64);

        // Make sure all values are 64 bit.
        cl_ulong firstWGID64[MAX_WORK_DIM];
        cl_ulong lastWGID64[MAX_WORK_DIM];
        for (int i=0 ; i < MAX_WORK_DIM ; ++i)
        {
            firstWGID64[i] = firstWGID[i];
            lastWGID64[i]  = lastWGID[i];
        }

        // Do not use string metadata, it is VERY slow.
        __itt_metadata_add(m_pGPAData->pDeviceDomain, m_ittID, m_pGPAData->pStartPos, __itt_metadata_u64 , cmdParams->work_dim, firstWGID64);
        __itt_metadata_add(m_pGPAData->pDeviceDomain, m_ittID, m_pGPAData->pEndPos, __itt_metadata_u64 , cmdParams->work_dim, lastWGID64);
    }
#endif // ITT

#ifdef _DEBUG
    -- m_lAttaching;
#endif

    return pWgContextBase;
}

void NDRange::DetachFromThread(void* pWgContext)
{
    // End execution task
#if defined(USE_ITT)
    if ((nullptr != m_pGPAData) && (m_pGPAData->bUseGPA))
    {
        __itt_task_end(m_pGPAData->pDeviceDomain);
    }
#endif // ITT
    
    m_pRunner->RestoreThreadState(m_tExecState);
}

bool NDRange::ExecuteIteration(size_t x, size_t y, size_t z, void* pWgCtx)
{
#ifdef _DEBUG
    long lVal = m_lFinish.test_and_set(0, 0);
    assert(lVal == 0);
    ++ m_lExecuting;
#endif

    // We always start from (0,0,0) and process whole WG
    // No Need in parameters now
#ifdef _DEBUG
    const size_t* pWGCount = m_pImplicitArgs->WGCount;

    cl_dev_cmd_param_kernel *cmdParams = (cl_dev_cmd_param_kernel*)m_pCmd->params;
    size_t tDimArr[3] = {x, y, z};
    for (unsigned int i=0; i<cmdParams->work_dim;++i)
    {
        size_t val = pWGCount[i];
        assert(tDimArr[i]<val);
    }

    lVal = m_lFinish.test_and_set(0, 0);
    assert(lVal == 0);
#endif

    // Execute WG
    size_t groupId[MAX_WORK_DIM] = {x, y, z};
#ifndef _WIN32  //Don't support this feature on Windows at the moment   
                //Optionally override the iteration to be executed if an affinity permutation is defined
    if (m_bEnablePredictablePartitioning)
    {
        assert((0 == y) && (0 == z) && "predicted partitioning work on 1D only");
        ITaskExecutor* pTaskExecutor = reinterpret_cast<ITaskExecutor*>(pWgCtx);
        unsigned int myGroupID = pTaskExecutor->GetPosition();
        if (0 == m_bWGExecuted.bitTestAndSet(myGroupID))
        {
            groupId[0] = myGroupID;
        }
        else // find an available work group
        {
            for (unsigned int ui = 0; ui < m_numThreads; ++ui)
            {
                if (0 == m_bWGExecuted.bitTestAndSet(ui))
                {
                    groupId[0] = ui;
                    break;
                }
            }
        }
    }
#endif
    CommandSubmitionLists childKernelsForWG;

    m_pRunner->RunGroup(m_pKernelArgs, groupId, &childKernelsForWG);

    if ( (nullptr != childKernelsForWG.waitingChildrenForWorkGroup) || (nullptr != childKernelsForWG.waitingChildrenForKernelLocalHead) )
    {
        SubmitCommands(&childKernelsForWG);
    }

#ifdef _DEBUG
    -- m_lExecuting;
#endif
    return true;
}

KernelCommand* NDRange::AllocateChildCommand(ITaskList* pList, const Intel::OpenCL::DeviceBackend::ICLDevBackendKernel_* pKernel,
        const void* pBlockLiteral, size_t stBlockSize, const size_t* pLocalSizes, size_t stLocalSizeCount,
        const _ndrange_t* pNDRange) const
{
    KernelCommand* pNewCommand = new DeviceNDRange(m_pTaskDispatcher, pList, const_cast<NDRange*>(this), pKernel, pBlockLiteral, stBlockSize, pLocalSizes, stLocalSizeCount, pNDRange);

    return pNewCommand;
}

queue_t NDRange::GetDefaultQueueForDevice() const
{
    if ( nullptr == m_pTaskDispatcher )
    {
        return 0 != m_parent ? m_parent->GetDefaultQueueForDevice() : nullptr;
    }
    return m_pTaskDispatcher->GetDefaultQueue();
}

///////////////////////////////////////////////////////////////////////////
// OCL Fill buffer/image execution

cl_dev_err_code FillMemObject::Create(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, SharedPtr<ITaskBase>* pTask, const SharedPtr<ITaskList>& pList)
{
    FillMemObject* pCommand = new FillMemObject(pTD, pCmd);
    if (nullptr == pCommand)
    {
        return CL_DEV_OUT_OF_MEMORY;
    }
#ifdef _DEBUG
    cl_dev_err_code rc;
    rc = pCommand->CheckCommandParams(pCmd);
    assert(CL_DEV_SUCCESS == rc);
#endif

    assert(pTask);
    *pTask = static_cast<ITaskBase*>(pCommand);

    return CL_DEV_SUCCESS;
}


FillMemObject::FillMemObject(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd) :
    CommandBaseClass<ITask>(pTD, pCmd)
{
}

cl_dev_err_code FillMemObject::CheckCommandParams(cl_dev_cmd_desc* cmd)
{
    if ( (CL_DEV_CMD_FILL_BUFFER != cmd->type) && (CL_DEV_CMD_FILL_IMAGE != cmd->type) )
    {
        return CL_DEV_INVALID_COMMAND_TYPE;
    }

    if ( sizeof(cl_dev_cmd_param_fill) != cmd->param_size )
    {
        return CL_DEV_INVALID_COMMAND_PARAM;
    }

    cl_dev_cmd_param_fill *cmdParams = (cl_dev_cmd_param_fill*)(cmd->params);

    for (unsigned int i=0; i< cmdParams->dim_count-1; i++)
    {
        // offset may be 0, but region must be bigger than 0 for used dimmensions.
        if(0 == cmdParams->region[i])
        {
            return CL_DEV_INVALID_VALUE;
        }
    }

    // TODO: Check Region
    return CL_DEV_SUCCESS;
}

bool FillMemObject::Execute()
{
    const cl_dev_cmd_param_fill* cmdParams = (cl_dev_cmd_param_fill*)m_pCmd->params;
    cl_mem_obj_descriptor*  pMemObj;

    NotifyCommandStatusChanged(m_pCmd, CL_RUNNING, CL_DEV_SUCCESS);

    cmdParams->memObj->clDevMemObjGetDescriptor(CL_DEVICE_TYPE_CPU, 0, (cl_dev_memobj_handle*)&pMemObj);

#ifdef _DEBUG_PRINT
    printf("--> FillMemObject(start), cmdid:%p\n", m_pCmd->id);
#endif

    size_t depthStart = 0;
    size_t depthEnd = 1;
    if (cmdParams->dim_count > 2)
    {
        depthStart = cmdParams->offset[2];
        depthEnd = depthStart + cmdParams->region[2];
    }

    size_t heightStart = 0;
    size_t heightEnd = 1;
    if (cmdParams->dim_count > 1)
    {
        heightStart = cmdParams->offset[1];
        heightEnd = heightStart + cmdParams->region[1];
    }

    // width, unlike height and depth, is in bytes.
    const size_t width =  cmdParams->region[0] * pMemObj->uiElementSize;

    // prepare copy buffer:
    char* fillBuf = (char*)malloc(width);
    if (nullptr == fillBuf) return false;

    CopyPattern(cmdParams->pattern, cmdParams->pattern_size, fillBuf, width);

#ifdef _DEBUG_PRINT
    fprintf(stderr, "Going to fill [%lu,%lu,%lu] to [%lu,%lu,%lu]- with buffer of len %lu bytes\n", cmdParams->offset[0], heightStart, depthStart,
        cmdParams->offset[0] + cmdParams->region[0], heightEnd, depthEnd, width);
    fprintf(stderr, "dim_count is %u ; pitches: %lu, %lu ; element size: %u ; color: ", cmdParams->dim_count, pMemObj->pitch[0], pMemObj->pitch[1], pMemObj->uiElementSize);
    for (size_t i=0 ; i < cmdParams->pattern_size ; ++i)
    {
        fprintf(stderr, "%02x ", cmdParams->pattern[i]);
    }
    fprintf(stderr, "\n");
#endif
    
    size_t offset[MAX_WORK_DIM];
    offset[0] = cmdParams->offset[0];
    for (size_t depth = depthStart ; depth < depthEnd ; ++depth)
    {
        offset[2] = depth;
        for (size_t height = heightStart ; height < heightEnd ; ++height)
        {
            offset[1] = height;
             void* dst = MemoryAllocator::CalculateOffsetPointer(pMemObj->pData, cmdParams->dim_count,
                    offset, pMemObj->pitch, pMemObj->uiElementSize);
            memcpy(dst, fillBuf, width);
        }
    }

    free(fillBuf);

#ifdef _DEBUG_PRINT
    printf("--> FillMemObject(end), cmdid:%p(%d)\n", m_pCmd->id, CL_DEV_SUCCESS);
#endif
    NotifyCommandStatusChanged(m_pCmd, CL_COMPLETE, CL_DEV_SUCCESS);
    return true;
}

///////////////////////////////////////////////////////////////////////////
// OCL Migrate buffer/image execution

cl_dev_err_code MigrateMemObject::Create(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, SharedPtr<ITaskBase>* pTask, const SharedPtr<ITaskList>& pList)
{
    MigrateMemObject* pCommand = new MigrateMemObject(pTD, pCmd);
    if (nullptr == pCommand)
    {
        return CL_DEV_OUT_OF_MEMORY;
    }
#ifdef _DEBUG
    cl_dev_err_code rc;
    rc = pCommand->CheckCommandParams(pCmd);
    assert(CL_DEV_SUCCESS == rc);
#endif

    assert(pTask);
    *pTask = static_cast<ITaskBase*>(pCommand);

    return CL_DEV_SUCCESS;
}

MigrateMemObject::MigrateMemObject(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd) :
    CommandBaseClass<ITask>(pTD, pCmd)
{
}

cl_dev_err_code MigrateMemObject::CheckCommandParams(cl_dev_cmd_desc* cmd)
{
    if ( sizeof(cl_dev_cmd_param_migrate) != cmd->param_size )
    {
        return CL_DEV_INVALID_COMMAND_PARAM;
    }

    cl_dev_cmd_param_migrate *cmdParams = (cl_dev_cmd_param_migrate*)(cmd->params);

    if (0 == cmdParams->mem_num)
    {
        return CL_DEV_INVALID_VALUE;
    }

    for (unsigned int i=0; i< cmdParams->mem_num; ++i)
    {
        if(nullptr == cmdParams->memObjs[i])
        {
            return CL_DEV_INVALID_VALUE;
        }
    }

    if (0 != (cmdParams->flags & ~(CL_MIGRATE_MEM_OBJECT_HOST | CL_MIGRATE_MEM_OBJECT_CONTENT_UNDEFINED)))
    {
        return CL_DEV_INVALID_VALUE;
    }
    
    return CL_DEV_SUCCESS;
}

bool MigrateMemObject::Execute()
{
    //cl_dev_cmd_param_migrate* cmdParams = (cl_dev_cmd_param_migrate*)m_pCmd->params;

    NotifyCommandStatusChanged(m_pCmd, CL_RUNNING, CL_DEV_SUCCESS);

    // TODO: Numa implementation

    NotifyCommandStatusChanged(m_pCmd, CL_COMPLETE, CL_DEV_SUCCESS);
    return true;
}

///////////////////////////////////////////////////////////////////////////
// OCL Migrate USM buffer execution

cl_dev_err_code MigrateUSMMemObject::Create(TaskDispatcher* pTD,
                                            cl_dev_cmd_desc* pCmd,
                                            SharedPtr<ITaskBase>* pTask,
                                            const SharedPtr<ITaskList>& pList)
{
    MigrateUSMMemObject* pCommand = new MigrateUSMMemObject(pTD, pCmd);
    if (nullptr == pCommand)
    {
        return CL_DEV_OUT_OF_MEMORY;
    }
#ifdef _DEBUG
    cl_dev_err_code rc;
    rc = pCommand->CheckCommandParams(pCmd);
    assert(CL_DEV_SUCCESS == rc);
#endif

    assert(pTask);
    *pTask = static_cast<ITaskBase*>(pCommand);

    return CL_DEV_SUCCESS;
}

MigrateUSMMemObject::MigrateUSMMemObject(TaskDispatcher* pTD,
    cl_dev_cmd_desc* pCmd) : CommandBaseClass<ITask>(pTD, pCmd)
{
}

cl_dev_err_code MigrateUSMMemObject::CheckCommandParams(cl_dev_cmd_desc* cmd)
{
    if ( sizeof(cl_dev_cmd_param_migrate_usm) != cmd->param_size )
    {
        return CL_DEV_INVALID_COMMAND_PARAM;
    }

    cl_dev_cmd_param_migrate_usm *cmdParams =
        (cl_dev_cmd_param_migrate_usm*)(cmd->params);

    if(nullptr == cmdParams->memObj)
    {
        return CL_DEV_INVALID_VALUE;
    }

    if (0 != (cmdParams->flags & ~(CL_MIGRATE_MEM_OBJECT_HOST |
                                   CL_MIGRATE_MEM_OBJECT_CONTENT_UNDEFINED)))
    {
        return CL_DEV_INVALID_VALUE;
    }

    return CL_DEV_SUCCESS;
}

bool MigrateUSMMemObject::Execute()
{
    NotifyCommandStatusChanged(m_pCmd, CL_RUNNING, CL_DEV_SUCCESS);

    NotifyCommandStatusChanged(m_pCmd, CL_COMPLETE, CL_DEV_SUCCESS);

    return true;
}

// DeviceNDRange:

AtomicCounter DeviceNDRange::sm_cmdIdCnt;

void DeviceNDRange::InitBlockCmdDesc(const Intel::OpenCL::DeviceBackend::ICLDevBackendKernel_* pKernel,
        const void* pBlockLiteral, size_t stBlockSize,
        const size_t* pLocalSizes, size_t stLocalSizeCount,
        const _ndrange_t* pNDRange)
{
    m_kernelMapEntry.pBEKernel = pKernel;
#ifdef USE_ITT
    m_kernelMapEntry.ittTaskNameHandle = nullptr; //TODO: Need to see how to integrate ITT task here
#endif
    m_paramKernel.kernel = &m_kernelMapEntry;
    m_paramKernel.work_dim = pNDRange->workDimension;
    for (cl_uint i = 0; i < m_paramKernel.work_dim; ++i)
    {
        m_paramKernel.glb_wrk_offs[i] = pNDRange->globalWorkOffset[i];
        m_paramKernel.glb_wrk_size[i] = pNDRange->globalWorkSize[i];
        m_paramKernel.lcl_wrk_size[UNIFORM_WG_SIZE_INDEX][i] = pNDRange->localWorkSize[i];
        // work-group size may be 0
        const size_t nonUniLocalSize = pNDRange->localWorkSize[i] == 0 ?
                                                                     0 :
                                                                     pNDRange->globalWorkSize[i] % pNDRange->localWorkSize[i];
        // if the remainder is 0 set non-unifrom size to uniform value
        m_paramKernel.lcl_wrk_size[NONUNIFORM_WG_SIZE_INDEX][i] = nonUniLocalSize == 0 ?
                                                                  pNDRange->localWorkSize[i] :
                                                                  nonUniLocalSize;
    }

    size_t const exp_arg_size = pKernel->GetExplicitArgumentBufferSize();
    size_t const sizeof_loc_sizes_buf = stLocalSizeCount * sizeof(size_t);
    // Align block's size to sizes of local buffers or the implicit arguments
    size_t const local_arg_offset = ALIGN_UP(stBlockSize, sizeof(size_t));
    assert( (exp_arg_size == (local_arg_offset + sizeof_loc_sizes_buf) ) &&
            "Explicit arguments buffer size is not as expected" );

    // We should also allocate space for the implicit arguments
    size_t const total_size = exp_arg_size + sizeof(cl_uniform_kernel_args);

    char* pAllocatedContext = (char*)ALIGNED_MALLOC(total_size,
                                                    pKernel->GetArgumentBufferRequiredAlignment());
    m_paramKernel.uiNonArgSvmBuffersCount = 0;
    m_paramKernel.uiNonArgUsmBuffersCount = 0;
    m_paramKernel.arg_size = total_size;
    m_paramKernel.arg_values = pAllocatedContext;

    // Copy the block literal (w\ all captured variables)
    MEMCPY_S(pAllocatedContext, exp_arg_size, pBlockLiteral, stBlockSize);
    // Copy local buffer sizes
    MEMCPY_S(pAllocatedContext + local_arg_offset, exp_arg_size - local_arg_offset, pLocalSizes, sizeof_loc_sizes_buf);

    m_cmdDesc.type = CL_DEV_CMD_EXEC_KERNEL;
    // device NDRange IDs have their MSB set, while in host NDRange IDs they're reset
    m_cmdDesc.id = (cl_dev_cmd_id)(DeviceNDRange::GetNextCmdId() | (1L << (sizeof(long) * 8 - 1)));
    m_cmdDesc.data = this;
    m_cmdDesc.device_agent_data = nullptr;
    m_cmdDesc.profiling = GetList()->IsProfilingEnabled();
    m_cmdDesc.params = &m_paramKernel;
    m_cmdDesc.param_size = sizeof(m_paramKernel);
}

void DeviceNDRange::NotifyCommandStatusChanged(cl_dev_cmd_desc* cmd, unsigned uStatus, int iErr)
{
    switch (uStatus)
    {
    case CL_RUNNING:
        StartExecutionProfiling();
        break;
    case CL_ENDED_RUNNING:
        // do nothing
        break;
    case CL_COMPLETE:
        SignalComplete( (cl_dev_err_code)iErr );
        GetParent()->ChildCompleted(GetError());
        break;
    default:
        assert(0 && "Invalid execution status");
    }
    // no need to call DispatcherCommand::NotifyCommandStatusChanged because we don't need to return to RT
}

#ifdef __INCLUDE_MKL__
/////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
cl_dev_err_code NativeKernelTask::Create(TaskDispatcher* pTD, const SharedPtr<ITaskList>& pList, cl_dev_cmd_desc* pCmd, SharedPtr<ITaskBase>* pTask)
{
    NativeKernelTask* pCommand = new NativeKernelTask(pTD, pList, pCmd);
    if (nullptr == pCommand)
    {
        return CL_DEV_OUT_OF_MEMORY;
    }

    assert(pTask);
    *pTask = static_cast<ITaskBase*>(pCommand);

    return CL_DEV_SUCCESS;
}

NativeKernelTask::NativeKernelTask(TaskDispatcher* pTD, const SharedPtr<ITaskList>& pList, cl_dev_cmd_desc* pCmd) :
    CommandBaseClass<ITask>(pTD, pCmd), m_pList(pList.GetPtr())
{
    cl_dev_cmd_param_kernel* pCmd_params = (cl_dev_cmd_param_kernel*)(pCmd->params);
    const ProgramService::KernelMapEntry* pEntry = ((const ProgramService::KernelMapEntry*)pCmd_params->kernel);
    const ICLDevBackendKernel_* pKernel = pEntry->pBEKernel;
    
    m_pBIKernel = static_cast<const IBuiltInKernel*>(pKernel);
}

cl_dev_err_code NativeKernelTask::CheckCommandParams(cl_dev_cmd_desc* cmd)
{
    return CL_DEV_SUCCESS;
}

bool NativeKernelTask::Execute()
{
    cl_dev_cmd_param_kernel* pCmd_params = (cl_dev_cmd_param_kernel*)(m_pCmd->params);
    const ProgramService::KernelMapEntry* pEntry = ((const ProgramService::KernelMapEntry*)pCmd_params->kernel);
    const ICLDevBackendKernel_* pKernel = pEntry->pBEKernel;
    const cl_kernel_argument*   pParams = pKernel->GetKernelParams();

    NotifyCommandStatusChanged(m_pCmd, CL_RUNNING, CL_DEV_SUCCESS);

#if defined(USE_ITT)
    // Start execution task
    if ((nullptr != m_pGPAData) && (m_pGPAData->bUseGPA))
    {
#if defined(USE_GPA)
        __itt_set_track(nullptr);
#endif

        __itt_task_begin(m_pGPAData->pDeviceDomain, m_ittID, __itt_null, pEntry->ittTaskNameHandle);
    }
#endif

    cl_dev_err_code err = ExtractNDRangeParams(pCmd_params->arg_values, pParams,
                                                   pKernel->GetMemoryObjectArgumentIndexes(),
                                                   pKernel->GetMemoryObjectArgumentCount(),
                                                   nullptr);
    if ( CL_DEV_FAILED(err) )
    {
        NotifyCommandStatusChanged(m_pCmd, CL_COMPLETE, err);
        return false;
    }

    cl_uniform_kernel_args* pUnifromArgs = (cl_uniform_kernel_args*)((char*)pCmd_params->arg_values + pKernel->GetExplicitArgumentBufferSize());
    pUnifromArgs->WorkDim = pCmd_params->work_dim;
    memcpy(pUnifromArgs->GlobalSize, pCmd_params->glb_wrk_size, pCmd_params->work_dim*sizeof(size_t));
#ifndef __OMP2TBB__
    cl_dev_err_code res = m_pBIKernel->Execute(pCmd_params->arg_values, m_pTaskDispatcher->getOmpExecutionThread());
#else
    cl_dev_err_code res = m_pBIKernel->Execute(m_pList, pCmd_params->arg_values);
#endif

#if defined(USE_ITT)
    if ((nullptr != m_pGPAData) && (m_pGPAData->bUseGPA))
    {
#if defined(USE_GPA)
        __itt_set_track(nullptr);
#endif
        __itt_task_end(m_pGPAData->pDeviceDomain);
    }
#endif // ITT

    NotifyCommandStatusChanged(m_pCmd, CL_COMPLETE, res);

    // Return success even if BuiltIn kernel execution failed
    return true;
}
#endif

/////////////////////////////////////////////////////////////////////////
// OCL advise USM mem command


cl_dev_err_code AdviseUSMMemObject::Create(TaskDispatcher* pTD,
                                           cl_dev_cmd_desc* pCmd,
                                           SharedPtr<ITaskBase>* pTask,
                                           const SharedPtr<ITaskList>& pList)
{
    AdviseUSMMemObject* pCommand = new AdviseUSMMemObject(pTD, pCmd);

#ifdef _DEBUG
    cl_dev_err_code rc;
    rc = pCommand->CheckCommandParams(pCmd);
    assert(CL_DEV_SUCCESS == rc);
#endif

    assert(pTask);
    *pTask = static_cast<ITaskBase*>(pCommand);

    return CL_DEV_SUCCESS;
}

AdviseUSMMemObject::AdviseUSMMemObject(TaskDispatcher* pTD,
                                       cl_dev_cmd_desc* pCmd) :
    CommandBaseClass<ITask>(pTD, pCmd)
{
}

cl_dev_err_code AdviseUSMMemObject::CheckCommandParams(cl_dev_cmd_desc* cmd)
{
    if (sizeof(cl_dev_cmd_param_advise_usm) != cmd->param_size)
        return CL_DEV_INVALID_COMMAND_PARAM;

    cl_dev_cmd_param_advise_usm *cmdParams =
        (cl_dev_cmd_param_advise_usm*)(cmd->params);

    if (nullptr == cmdParams->memObj)
        return CL_DEV_INVALID_VALUE;

    return CL_DEV_SUCCESS;
}

bool AdviseUSMMemObject::Execute()
{
    bool ret = true;

    NotifyCommandStatusChanged(m_pCmd, CL_RUNNING, CL_DEV_SUCCESS);

    NotifyCommandStatusChanged(m_pCmd, CL_COMPLETE, CL_DEV_SUCCESS);

    return ret;
}
