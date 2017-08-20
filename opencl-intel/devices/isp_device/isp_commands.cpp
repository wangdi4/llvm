// Copyright (c) 2014 Intel Corporation
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
//  isp_commands.cpp
//  Implementation of the ISP commands classes
///////////////////////////////////////////////////////////

#include "stdafx.h"

#include "isp_commands.h"
#include "isp_task_dispatcher.h"
#include "isp_program_service.h"
#include "isp_logger.h"
#include "isp_utils.h"

#include <cl_shared_ptr.hpp>
#include <task_executor.h>

#include <cl_sys_info.h>
#include <cl_utils.h>

using namespace Intel::OpenCL::ISPDevice;
using namespace Intel::OpenCL::TaskExecutor;
using namespace Intel::OpenCL::Utils;

void ISPCommandBase::NotifyCommandStatusChanged(cl_int status, cl_dev_err_code errorCode)
{
    if (CL_COMPLETE == status)
    {
        // TODO: add ITask->Release ?
        m_bCompleted = true;
    }

    cl_ulong timer = 0;
    // if profiling enabled for the command get timer
    if (m_pCmdDesc->profiling)
    {
        timer = HostTime();
    }
    // notify framework about status change
    m_pFrameworkCallBacks->clDevCmdStatusChanged(m_pCmdDesc->id, m_pCmdDesc->data, status, (cl_int)errorCode, timer);
}

NDRange::NDRange(IOCLDevLogDescriptor* logDesc, cl_int logHandle,cl_dev_cmd_desc* pCmd,
                 IOCLFrameworkCallbacks* frameworkCallbacks, CameraShim* pCameraShim,
                 ISPExtensionManager* pIspExtensionManager) :
ISPCommandBase(logDesc, logHandle, pCmd, frameworkCallbacks), m_pCameraShim(pCameraShim),
m_pIspExtensionManager(pIspExtensionManager)
{
    cl_dev_cmd_param_kernel* cmdParams = reinterpret_cast<cl_dev_cmd_param_kernel*>(m_pCmdDesc->params);
    m_pIspKernel = reinterpret_cast<const ISPKernel*>(cmdParams->kernel);
}

//------------------------------------------------
// NDRange Command
cl_dev_err_code NDRange::Create(ISPDeviceQueue* pDeviceQueue, cl_dev_cmd_desc* pCmd, SharedPtr<ITaskBase>* ppTask)
{
    assert(nullptr != pDeviceQueue && "Invalid create command parameter");
    assert(nullptr != pCmd && "Invalid create command parameter");
    assert(nullptr != ppTask && "Invalid create command parameter");

    NDRange* pCommand = new NDRange(
                                pDeviceQueue->GetLogDescriptor(),
                                pDeviceQueue->GetLogHandle(),
                                pCmd,
                                pDeviceQueue->GetFrameworkCallbacks(),
                                pDeviceQueue->GetCameraHandle(),
                                pDeviceQueue->GetExtensionManager());
    if (nullptr == pCommand)
    {
        return CL_DEV_OUT_OF_MEMORY;
    }

    *ppTask = pCommand;

    return CL_DEV_SUCCESS;
}

bool NDRange::Execute()
{
    NotifyCommandStatusChanged(CL_RUNNING, CL_DEV_SUCCESS);

    IspDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("Executing NDRange command id %d with kernel name %s"), m_pCmdDesc->id, m_pIspKernel->GetKernelName().c_str());

    bool succeed = true;

    ExtractNDRangeKernelArguments();

    // execute pre-work
    if (CAMERA_STATE_PREVIEW_RUNNING == m_pIspKernel->GetRequiredState())
    {
        succeed = ExecuteStartPreview();
    }
    else if (CAMERA_STATE_PREVIEW_STOPPED == m_pIspKernel->GetRequiredState())
    {
        succeed = ExecuteStopPreview();
    }

    if (!succeed)
    {
        NotifyCommandStatusChanged(CL_COMPLETE, CL_DEV_ERROR_FAIL);
        return false;
    }

    // execute actual command
    enum cameraCommand cameraCmd = m_pIspKernel->GetCameraCommand();
    switch (cameraCmd)
    {
    case CAMERA_NO_COMMAND:
        // standalone kernel from binary
        succeed = ExecuteStandaloneCustomKernel();
        break;

    case CAMERA_AUTO_FOCUS:
        succeed = ExecuteAutoFocus();
        break;

    case CAMERA_TAKE_PICTURE:
    case CAMERA_TAKE_PICTURE_WITH_AF:
        succeed = ExecuteTakePicture(CAMERA_TAKE_PICTURE_WITH_AF == cameraCmd);
        break;

    case CAMERA_SET_PICTURE_SIZE:
        succeed = ExecuteSetPictureSize();
        break;

    case CAMERA_SET_PREVIEW_SIZE:
        succeed = ExecuteSetPreviewSize();
        break;

    case CAMERA_COPY_PREVIEW_BUFFER:
        succeed = ExecuteCopyPreviewBuffer();
        break;

    case CAMERA_START_PREVIEW:
    case CAMERA_STOP_PREVIEW:
        // work will be done in pre/post-work
        break;

    default:
        assert(false && "Unknown command");
        return false;
    }

    if (!succeed)
    {
        NotifyCommandStatusChanged(CL_COMPLETE, CL_DEV_ERROR_FAIL);
        return false;
    }

    // execute post-work
    if (CAMERA_STATE_PREVIEW_RUNNING == m_pIspKernel->GetStateAfterExecution())
    {
        succeed = ExecuteStartPreview();
    }
    else if (CAMERA_STATE_PREVIEW_STOPPED == m_pIspKernel->GetStateAfterExecution())
    {
        succeed = ExecuteStopPreview();
    }

    if (!succeed)
    {
        NotifyCommandStatusChanged(CL_COMPLETE, CL_DEV_ERROR_FAIL);
        return false;
    }

    IspDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("Done executing NDRange command id %d"), m_pCmdDesc->id);

    NotifyCommandStatusChanged(CL_COMPLETE, CL_DEV_SUCCESS);
    return true;
}

void NDRange::ExtractNDRangeKernelArguments()
{
    // Get kernel argument prototype
    cl_uint argsCount = m_pIspKernel->GetKernelArgsCount();
    const cl_kernel_argument* argsPrototype = m_pIspKernel->GetKernelArgsPrototype();

    // Get arguments buffer
    cl_dev_cmd_param_kernel* cmdParams = reinterpret_cast<cl_dev_cmd_param_kernel*>(m_pCmdDesc->params);
    unsigned char* argsValuesBase = reinterpret_cast<unsigned char*>(cmdParams->arg_values);

    // Add direct pointer to each argument value
    for (cl_uint i = 0; i < argsCount; ++i)
    {
        m_vArgs.push_back(argsValuesBase + argsPrototype[i].offset_in_bytes);
    }
}

bool NDRange::ExecuteStartPreview()
{
    // start preview, if already started do nothing
    status_t ret = m_pCameraShim->preview_start();
    if (0 != ret)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Failed to start preview in camera"));
        return false;
    }

    return true;
}
bool NDRange::ExecuteStopPreview()
{
    // stop preview, if already stopped do nothing
    status_t ret = m_pCameraShim->preview_stop();
    if (0 != ret)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Failed to stop preview in camera"));
        return false;
    }

    // clear extension mode preview
    if (CL_FAILED(m_pIspExtensionManager->ClearRunning()))
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Failed to clear extension mode kernels from preview"));
        return false;
    }

    return true;
}
bool NDRange::ExecuteAutoFocus()
{
    // TODO: why not blocking ?
    status_t ret = m_pCameraShim->focus(false);
    if (0 != ret)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Failed to send focus command to camera"));
        return false;
    }

    return true;
}
bool NDRange::ExecuteTakePicture(bool withAutoFocus)
{
    // TODO: why we need this ?
    status_t ret = m_pCameraShim->enable_shutter_sound(true);
    if (0 != ret)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Could not enable shutter sound in camera"));
        return false;
    }

    assert(1 == m_vArgs.size() && "Invalid number of arguments for command");

    IOCLDevMemoryObject* pMemObj = *((IOCLDevMemoryObject**)(m_vArgs[0]));
    assert(nullptr != pMemObj && "Invalid handle to memory object");

    cl_mem_obj_descriptor* pMemObjDesc = nullptr;
    pMemObj->clDevMemObjGetDescriptor(CL_DEVICE_TYPE_CUSTOM, 0, (cl_dev_memobj_handle*) &pMemObjDesc);
    assert(nullptr != pMemObjDesc && "Couldn't get the memory object descriptor");

    ret = m_pCameraShim->take_picture(pMemObjDesc->pData, pMemObjDesc->dimensions.buffer_size, withAutoFocus);
    if (0 != ret)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Could not take picture in camera"));
        return false;
    }

    return true;
}
bool NDRange::ExecuteSetPictureSize()
{
    assert(2 == m_vArgs.size() && "Invalid number of arguments for command");
    cl_uint width = *((cl_uint*)(m_vArgs[0]));
    cl_uint height = *((cl_uint*)(m_vArgs[1]));
    // TODO: change to UNSIGNED int
    status_t ret = m_pCameraShim->set_picture_size(width, height);
    if (0 != ret)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Could not set picture size in camera"));
        return false;
    }

    return true;
}
bool NDRange::ExecuteSetPreviewSize()
{
    assert(2 == m_vArgs.size() && "Invalid number of arguments for command");
    cl_uint width = *((cl_uint*)(m_vArgs[0]));
    cl_uint height = *((cl_uint*)(m_vArgs[1]));
    // TODO: change to UNSIGNED int
    status_t ret = m_pCameraShim->set_preview_size(width, height);
    if (0 != ret)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Could not set preview size in camera"));
        return false;
    }

    return true;
}
bool NDRange::ExecuteCopyPreviewBuffer()
{
    Frame currentFrame;
    status_t ret = m_pCameraShim->preview_grabbuffer(currentFrame);
    if (0 != ret)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Could not grab preview buffer"));
        return false;
    }
    size_t bytesToCopy = currentFrame.size;

    assert(1 == m_vArgs.size() && "Invalid number of arguments for command");

    IOCLDevMemoryObject* pMemObj = *((IOCLDevMemoryObject**)(m_vArgs[0]));
    assert(nullptr != pMemObj && "Invalid handle to memory object");

    cl_mem_obj_descriptor* pMemObjDesc = nullptr;
    pMemObj->clDevMemObjGetDescriptor(CL_DEVICE_TYPE_CUSTOM, 0, (cl_dev_memobj_handle*) &pMemObjDesc);
    assert(nullptr != pMemObjDesc && "Couldn't get the memory object descriptor");

    if (pMemObjDesc->dimensions.buffer_size < bytesToCopy)
    {
        // not a fatal error for the command
        bytesToCopy = pMemObjDesc->dimensions.buffer_size;
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Mem object too small, can only copy partial preview buffer"));
    }

    IspDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("MEMCPY_S(%p, %d, %p, %d)"), pMemObjDesc->pData, pMemObjDesc->dimensions.buffer_size, currentFrame.img_data, bytesToCopy);
    MEMCPY_S(pMemObjDesc->pData, pMemObjDesc->dimensions.buffer_size, currentFrame.img_data, bytesToCopy);

    ret = m_pCameraShim->preview_releasebuffer(currentFrame);
    if(0 != ret)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Could not release preview buffer"));
        return false;
    }

    return true;
}
bool NDRange::ExecuteStandaloneCustomKernel()
{
    // TODO: maybe make things simple here ?
    // Standalone custom kernel execution:
    //  1 - Upload firmware to ISP
    //  2 - Map kernel arguments to ISP space:
    //    2.a - Allocate required buffer on ISP
    //    2.b - Inside arguments buffer, map each pointer argument to ISP space
    //    2.c - Map the whole arguments buffer to ISP space
    //  3 - Set the mapped arguments buffer for the loaded firmware
    //  4 - Run the firmware on ISP
    //  5 - Unmap the kernel arguments buffer from ISP space:
    //    5.a - Unmap the whole arguments buffer from ISP space
    //    5.b - Inside mapped arguments buffer, unmap each pointer argument
    //    5.c - Free the allocated buffer on ISP
    //  6 - Unload the firmware from ISP

    void* pAllocatedArgsBuffer = nullptr;
    isp_ptr pMappedArgsBuffer = nullptr;

    // firmware is already allocated on ISP, just need to upload it
    fw_info firmwareInfo = m_pIspKernel->GetBlob();
    status_t ret = m_pCameraShim->acc_upload_fw_standalone(firmwareInfo);
    if (0 != ret)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Could not upload firmware to ISP"));
        return false;
    }

    // get arguments buffer size
    cl_dev_cmd_param_kernel* cmdParams = reinterpret_cast<cl_dev_cmd_param_kernel*>(m_pCmdDesc->params);
    const size_t stArgsBufferSizeActual = cmdParams->arg_size;

    const size_t stArgsBufferSizePrototype = m_pIspKernel->GetKernelArgsBufferSize();
    assert(stArgsBufferSizeActual == stArgsBufferSizePrototype && "Prototype is different from actual arguments size");

    // TODO: handle extreme case of 0 args
    // TODO: handle failures

    // allocate required buffer on ISP
    pAllocatedArgsBuffer = m_pCameraShim->host_alloc(stArgsBufferSizeActual);
    if (nullptr == pAllocatedArgsBuffer)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Could not allocate arguments buffer on ISP"));
        return false;
    }

    // copy actual arguments values to allocated buffer on ISP
    unsigned char* argsValuesBase = reinterpret_cast<unsigned char*>(cmdParams->arg_values);
    MEMCPY_S(pAllocatedArgsBuffer, stArgsBufferSizePrototype, argsValuesBase, stArgsBufferSizeActual);

    // map pointer arguments to ISP space
    if (!MapArgumentsToISP(pAllocatedArgsBuffer))
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Could not map pointer arguments"));
        return false;
    }

    // map whole argument buffer to ISP space
    ret = m_pCameraShim->acc_map(pAllocatedArgsBuffer, pMappedArgsBuffer);
    if (0 != ret)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Could not map arguments buffer"));
        return false;
    }

    // send the mapped arguments buffer to ISP
    ret = m_pCameraShim->acc_sendarg(firmwareInfo, pMappedArgsBuffer);
    if (0 != ret)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Could not send arguments buffer to ISP"));
        return false;
    }

    // run !
    ret = m_pCameraShim->acc_start_and_wait_standalone(firmwareInfo);
    if (0 != ret)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Could not start ISP with custom firmware"));
        return false;
    }

    // unload the firmware
    ret = m_pCameraShim->acc_unload(firmwareInfo);
    if (0 != ret)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Could not unload firmware"));
        return false;
    }

    // unmap whole arguments buffer from ISP space
    ret = m_pCameraShim->acc_unmap(pMappedArgsBuffer);
    if (0 != ret)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Could not unmap arguments buffer"));
        return false;
    }

    // unmap pointer arguments from ISP space
    if (!UnmapArgumentsFromISP(pAllocatedArgsBuffer))
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Could not unmap pointer arguments"));
        return false;
    }

    // free the allocated area in ISP space
    ret = m_pCameraShim->host_free(pAllocatedArgsBuffer);
    if (0 != ret)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Could not free arguments buffer"));
        return false;
    }

    return true;
}

bool NDRange::MapArgumentsToISP(void* pArgsBuffer)
{
    // iterate over all arguments values and replace all pointers with mapped pointers from ISP space

    assert(sizeof(void*) == sizeof(isp_ptr) && "Need to change implementation !");

    cl_uint uiArgsNum = m_pIspKernel->GetKernelArgsCount();
    const cl_kernel_argument* argsPrototype = m_pIspKernel->GetKernelArgsPrototype();
    unsigned char* pArgsBufferBase = reinterpret_cast<unsigned char*>(pArgsBuffer);

    for (cl_uint i = 0; i < uiArgsNum; ++i)
    {
        switch (argsPrototype[i].type)
        {
        case CL_KRNL_ARG_PTR_GLOBAL:
        case CL_KRNL_ARG_PTR_CONST:
            {
                IOCLDevMemoryObject* pMemObj = *((IOCLDevMemoryObject**)(m_vArgs[i]));
                if (nullptr == pMemObj)
                {
                    continue;
                }

                cl_mem_obj_descriptor* pMemObjDesc;
                cl_dev_err_code err = pMemObj->clDevMemObjGetDescriptor(CL_DEVICE_TYPE_CUSTOM, 0, (cl_dev_memobj_handle*) &pMemObjDesc);
                if (CL_FAILED(err))
                {
                    IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Could not get memory object descriptor"));
                    return false;
                }

                assert(nullptr != pMemObjDesc->pData && "Passing NULL data object for execution");

                isp_ptr pMappedData = nullptr;
                status_t ret = m_pCameraShim->acc_map(pMemObjDesc->pData, pMappedData);
                if (0 != ret)
                {
                    IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Could not map parameter"));
                    return false;
                }

                *(isp_ptr*)(pArgsBufferBase + argsPrototype[i].offset_in_bytes) = pMappedData;

                break;
            }

        case CL_KRNL_ARG_INT:
        case CL_KRNL_ARG_UINT:
        case CL_KRNL_ARG_FLOAT:
        case CL_KRNL_ARG_DOUBLE:
        case CL_KRNL_ARG_VECTOR:
        case CL_KRNL_ARG_COMPOSITE:
            // do nothing for non-pointers
            break;

        case CL_KRNL_ARG_VECTOR_BY_REF:
        case CL_KRNL_ARG_SAMPLER:
        case CL_KRNL_ARG_PTR_LOCAL:

        case CL_KRNL_ARG_PTR_IMG_2D:
        case CL_KRNL_ARG_PTR_IMG_2D_DEPTH:
        case CL_KRNL_ARG_PTR_IMG_3D:
        case CL_KRNL_ARG_PTR_IMG_2D_ARR:
        case CL_KRNL_ARG_PTR_IMG_2D_ARR_DEPTH:
        case CL_KRNL_ARG_PTR_IMG_1D:
        case CL_KRNL_ARG_PTR_IMG_1D_ARR:
        case CL_KRNL_ARG_PTR_IMG_1D_BUF:
        case CL_KRNL_ARG_PTR_BLOCK_LITERAL:
        default:
            assert(false && "TODO: Unsupported type");
            break;
        }
    }

    return true;
}
bool NDRange::UnmapArgumentsFromISP(void* pMappedArgsBuffer)
{
    // iterate over all arguments values and unmap all pointers from ISP space

    cl_uint uiArgsNum = m_pIspKernel->GetKernelArgsCount();
    const cl_kernel_argument* argsPrototype = m_pIspKernel->GetKernelArgsPrototype();
    unsigned char* pArgsBufferBase = reinterpret_cast<unsigned char*>(pMappedArgsBuffer);

    for (cl_uint i = 0; i < uiArgsNum; ++i)
    {
        switch (argsPrototype[i].type)
        {
        case CL_KRNL_ARG_PTR_GLOBAL:
        case CL_KRNL_ARG_PTR_CONST:
            {
                isp_ptr pMappedData = *(isp_ptr*)(pArgsBufferBase + argsPrototype[i].offset_in_bytes);
                if (nullptr == pMappedData)
                {
                    continue;
                }
                status_t ret = m_pCameraShim->acc_unmap(pMappedData);
                if (0 != ret)
                {
                    IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Could not unmap parameter"));
                    return false;
                }

                break;
            }

        case CL_KRNL_ARG_INT:
        case CL_KRNL_ARG_UINT:
        case CL_KRNL_ARG_FLOAT:
        case CL_KRNL_ARG_DOUBLE:
        case CL_KRNL_ARG_VECTOR:
        case CL_KRNL_ARG_COMPOSITE:
            // do nothing for non-pointers
            break;

        case CL_KRNL_ARG_VECTOR_BY_REF:
        case CL_KRNL_ARG_SAMPLER:
        case CL_KRNL_ARG_PTR_LOCAL:

        case CL_KRNL_ARG_PTR_IMG_2D:
        case CL_KRNL_ARG_PTR_IMG_2D_DEPTH:
        case CL_KRNL_ARG_PTR_IMG_3D:
        case CL_KRNL_ARG_PTR_IMG_2D_ARR:
        case CL_KRNL_ARG_PTR_IMG_2D_ARR_DEPTH:
        case CL_KRNL_ARG_PTR_IMG_1D:
        case CL_KRNL_ARG_PTR_IMG_1D_ARR:
        case CL_KRNL_ARG_PTR_IMG_1D_BUF:
        case CL_KRNL_ARG_PTR_BLOCK_LITERAL:
        default:
            assert(false && "TODO: Unsupported type");
            break;
        }
    }

    return true;
}


//------------------------------------------------
// ReadWriteMemoryObject Command
cl_dev_err_code ReadWriteMemoryObject::Create(ISPDeviceQueue* pDeviceQueue, cl_dev_cmd_desc* pCmd, SharedPtr<ITaskBase>* ppTask)
{
    assert(nullptr != pDeviceQueue && "Invalid create command parameter");
    assert(nullptr != pCmd && "Invalid create command parameter");
    assert(nullptr != ppTask && "Invalid create command parameter");

    ReadWriteMemoryObject* pCommand = new ReadWriteMemoryObject(
                                pDeviceQueue->GetLogDescriptor(),
                                pDeviceQueue->GetLogHandle(),
                                pCmd,
                                pDeviceQueue->GetFrameworkCallbacks());
    if (nullptr == pCommand)
    {
        return CL_DEV_OUT_OF_MEMORY;
    }

    *ppTask = pCommand;

    return CL_DEV_SUCCESS;
}

bool ReadWriteMemoryObject::Execute()
{
    NotifyCommandStatusChanged(CL_RUNNING, CL_DEV_SUCCESS);

    IspDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("Executing ReadWriteMemoryObject command id %d"), m_pCmdDesc->id);

    cl_dev_cmd_param_rw* cmdParams = reinterpret_cast<cl_dev_cmd_param_rw*>(m_pCmdDesc->params);

    cl_mem_obj_descriptor* pMemObj = nullptr;
    cl_dev_err_code ret = cmdParams->memObj->clDevMemObjGetDescriptor(CL_DEVICE_TYPE_CUSTOM, 0, (cl_dev_memobj_handle*) &pMemObj);
    assert(CL_DEV_SUCCEEDED(ret) && nullptr != pMemObj && "Failed to get memory object descriptor");

    // Memory object pointer
    void* pMemObjPtr = CalculateOffsetPointer(pMemObj->pData, cmdParams->dim_count, cmdParams->origin, cmdParams->memobj_pitch, pMemObj->uiElementSize);
    // Memory object pitch
    size_t* pMemObjPitch = cmdParams->memobj_pitch;

    // Host pointer
    // TODO: check if this calculation is correct
    void* pHostPtr = CalculateOffsetPointer(cmdParams->ptr, pMemObj->dim_count, cmdParams->ptr_origin, cmdParams->pitch, 1);
    // Host pitch
    size_t* pHostPitch = cmdParams->pitch;

    // Dimensions
    SMemCpyParams sCpyParam;
    sCpyParam.uiDimCount = cmdParams->dim_count;
    // Region
    MEMCPY_S(sCpyParam.vRegion, sizeof(sCpyParam.vRegion), cmdParams->region, sizeof(sCpyParam.vRegion));
    sCpyParam.vRegion[0] = sCpyParam.vRegion[0] * pMemObj->uiElementSize;

    if (CL_DEV_CMD_READ == m_pCmdDesc->type)
    {
        sCpyParam.pSrc = (cl_char*) pMemObjPtr;
        MEMCPY_S(sCpyParam.vSrcPitch, sizeof(sCpyParam.vSrcPitch), pMemObjPitch, sizeof(sCpyParam.vSrcPitch));
        sCpyParam.pDst = (cl_char*) pHostPtr;
        MEMCPY_S(sCpyParam.vDstPitch, sizeof(sCpyParam.vDstPitch), pHostPitch, sizeof(sCpyParam.vDstPitch));
    }
    else // command is write (CL_DEV_CMD_WRITE)
    {
        sCpyParam.pSrc = (cl_char*) pHostPtr;
        MEMCPY_S(sCpyParam.vSrcPitch, sizeof(sCpyParam.vSrcPitch), pHostPitch, sizeof(sCpyParam.vSrcPitch));
        sCpyParam.pDst = (cl_char*) pMemObjPtr;
        MEMCPY_S(sCpyParam.vDstPitch, sizeof(sCpyParam.vDstPitch), pMemObjPitch, sizeof(sCpyParam.vDstPitch));
    }

    IspDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("ReadWriteMemoryObject command: from %p to %p"), sCpyParam.pSrc, sCpyParam.pDst);

    // Execute copy routine
    clCopyMemoryRegion(&sCpyParam);

    IspDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("Done executing ReadWriteMemoryObject command id %d"), m_pCmdDesc->id);

    NotifyCommandStatusChanged(CL_COMPLETE, CL_DEV_SUCCESS);
    return true;
}


//------------------------------------------------
// CopyMemoryObject Command
cl_dev_err_code CopyMemoryObject::Create(ISPDeviceQueue* pDeviceQueue, cl_dev_cmd_desc* pCmd, SharedPtr<ITaskBase>* ppTask)
{
    assert(nullptr != pDeviceQueue && "Invalid create command parameter");
    assert(nullptr != pCmd && "Invalid create command parameter");
    assert(nullptr != ppTask && "Invalid create command parameter");

    CopyMemoryObject* pCommand = new CopyMemoryObject(
                                pDeviceQueue->GetLogDescriptor(),
                                pDeviceQueue->GetLogHandle(),
                                pCmd,
                                pDeviceQueue->GetFrameworkCallbacks());
    if (nullptr == pCommand)
    {
        return CL_DEV_OUT_OF_MEMORY;
    }

    *ppTask = pCommand;

    return CL_DEV_SUCCESS;
}

bool CopyMemoryObject::Execute()
{
    NotifyCommandStatusChanged(CL_RUNNING, CL_DEV_SUCCESS);

    IspDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("Executing CopyMemoryObject command id %d"), m_pCmdDesc->id);

    cl_dev_cmd_param_copy*  cmdParams = reinterpret_cast<cl_dev_cmd_param_copy*>(m_pCmdDesc->params);
    cl_mem_obj_descriptor*  pSrcMemObj;
    cl_mem_obj_descriptor*  pDstMemObj;

    cl_dev_err_code ret;
    ret = cmdParams->srcMemObj->clDevMemObjGetDescriptor(CL_DEVICE_TYPE_CUSTOM, 0, (cl_dev_memobj_handle*) &pSrcMemObj);
    assert(CL_DEV_SUCCEEDED(ret) && nullptr != pSrcMemObj && "Failed to get source memory object descriptor");
    ret = cmdParams->dstMemObj->clDevMemObjGetDescriptor(CL_DEVICE_TYPE_CUSTOM, 0, (cl_dev_memobj_handle*) &pDstMemObj);
    assert(CL_DEV_SUCCEEDED(ret) && nullptr != pDstMemObj && "Failed to get destination memory object descriptor");

    size_t stSrcElementSize = pSrcMemObj->uiElementSize;
    size_t stDstElementSize = pDstMemObj->uiElementSize;

    // Objects have to have same element size or buffer<->image
    if ((stDstElementSize != stSrcElementSize) &&
        (1 != stDstElementSize) && (1 != stSrcElementSize))
    {
        NotifyCommandStatusChanged(CL_COMPLETE, CL_DEV_INVALID_COMMAND_PARAM);
        return false;
    }

    // Options for different dimensions are
    //  - Copy a 2D image object to a 2D slice of a 3D image object.
    //  - Copy a 2D slice of a 3D image object to a 2D image object.
    //  - Copy 2D to 2D
    //  - Copy 3D to 3D
    //  - Copy 2D image to buffer
    //  - Copy 3D image to buffer
    //  - Buffer to image

    SMemCpyParams sCpyParam;

    MEMCPY_S(sCpyParam.vSrcPitch, sizeof(sCpyParam.vSrcPitch), cmdParams->src_pitch[0] ? cmdParams->src_pitch : pSrcMemObj->pitch, sizeof(sCpyParam.vSrcPitch));
    MEMCPY_S(sCpyParam.vDstPitch, sizeof(sCpyParam.vDstPitch), cmdParams->dst_pitch[0] ? cmdParams->dst_pitch : pDstMemObj->pitch, sizeof(sCpyParam.vDstPitch));

    sCpyParam.pSrc = (cl_char*) CalculateOffsetPointer(pSrcMemObj->pData, cmdParams->src_dim_count, cmdParams->src_origin, sCpyParam.vSrcPitch, pSrcMemObj->uiElementSize);
    sCpyParam.pDst = (cl_char*) CalculateOffsetPointer(pDstMemObj->pData, cmdParams->dst_dim_count, cmdParams->dst_origin, sCpyParam.vDstPitch, pDstMemObj->uiElementSize);

    IspDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("CopyMemoryObject command: from %p to %p"), sCpyParam.pSrc, sCpyParam.pDst);

    sCpyParam.uiDimCount = cmdParams->src_dim_count;

    // check if src vs dst have different dimensions
    if (cmdParams->dst_dim_count != cmdParams->src_dim_count)
    {
        if (CL_MEM_OBJECT_BUFFER == pSrcMemObj->memObjType)
        {
            // buffer to image
            // we will consider the buffer as an image with the same dims and element size as destination
            stSrcElementSize = stDstElementSize;
            sCpyParam.uiDimCount = cmdParams->dst_dim_count;
            sCpyParam.vSrcPitch[0] = cmdParams->region[0] * stDstElementSize;
            sCpyParam.vSrcPitch[1] = sCpyParam.vSrcPitch[0] * cmdParams->region[1];
        }
        else if(CL_MEM_OBJECT_BUFFER == pDstMemObj->memObjType)
        {
            // image to buffer
            // we will consider the buffer as an image with height=1
            sCpyParam.uiDimCount = cmdParams->src_dim_count;
            sCpyParam.vDstPitch[0] = cmdParams->region[0] * stSrcElementSize;
            sCpyParam.vDstPitch[1] = sCpyParam.vDstPitch[0] * cmdParams->region[1];
        }
        else
        {
            // Image to Image with different dims
            sCpyParam.uiDimCount = min(cmdParams->src_dim_count, cmdParams->dst_dim_count);
        }
    }

    // TODO: row_pitch = region[0] ?
    // If row_pitch (or input_row_pitch) is set to 0, the appropriate row pitch is calculated
    // based on the size of each element in bytes multiplied by width.
    MEMCPY_S(sCpyParam.vRegion, sizeof(sCpyParam.vRegion), cmdParams->region, sizeof(sCpyParam.vRegion));
    sCpyParam.vRegion[0] *= stSrcElementSize;

    // Execute copy routine
    clCopyMemoryRegion(&sCpyParam);

    IspDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("Done executing CopyMemoryObject command id %d"), m_pCmdDesc->id);

    NotifyCommandStatusChanged(CL_COMPLETE, CL_DEV_SUCCESS);

    return true;
}


//------------------------------------------------
// MapMemoryObject Command
cl_dev_err_code MapMemoryObject::Create(ISPDeviceQueue* pDeviceQueue, cl_dev_cmd_desc* pCmd, SharedPtr<ITaskBase>* ppTask)
{
    assert(nullptr != pDeviceQueue && "Invalid create command parameter");
    assert(nullptr != pCmd && "Invalid create command parameter");
    assert(nullptr != ppTask && "Invalid create command parameter");

    MapMemoryObject* pCommand = new MapMemoryObject(
                                pDeviceQueue->GetLogDescriptor(),
                                pDeviceQueue->GetLogHandle(),
                                pCmd,
                                pDeviceQueue->GetFrameworkCallbacks());
    if (nullptr == pCommand)
    {
        return CL_DEV_OUT_OF_MEMORY;
    }

    *ppTask = pCommand;

    return CL_DEV_SUCCESS;
}

bool MapMemoryObject::Execute()
{
    NotifyCommandStatusChanged(CL_RUNNING, CL_DEV_SUCCESS);
    IspDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("Executing MapMemoryObject command id %d"), m_pCmdDesc->id);

    // memory shared with host - nothing to do

    IspDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("Done executing MapMemoryObject command id %d"), m_pCmdDesc->id);
    NotifyCommandStatusChanged(CL_COMPLETE, CL_DEV_SUCCESS);

    return true;
}


//------------------------------------------------
// UnmapMemoryObject Command
cl_dev_err_code UnmapMemoryObject::Create(ISPDeviceQueue* pDeviceQueue, cl_dev_cmd_desc* pCmd, SharedPtr<ITaskBase>* ppTask)
{
    assert(nullptr != pDeviceQueue && "Invalid create command parameter");
    assert(nullptr != pCmd && "Invalid create command parameter");
    assert(nullptr != ppTask && "Invalid create command parameter");

    UnmapMemoryObject* pCommand = new UnmapMemoryObject(
                                pDeviceQueue->GetLogDescriptor(),
                                pDeviceQueue->GetLogHandle(),
                                pCmd,
                                pDeviceQueue->GetFrameworkCallbacks());
    if (nullptr == pCommand)
    {
        return CL_DEV_OUT_OF_MEMORY;
    }

    *ppTask = pCommand;

    return CL_DEV_SUCCESS;
}

bool UnmapMemoryObject::Execute()
{
    NotifyCommandStatusChanged(CL_RUNNING, CL_DEV_SUCCESS);
    IspDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("Executing UnmapMemoryObject command id %d"), m_pCmdDesc->id);

    // memory shared with host - nothing to do

    IspDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("Done executing UnmapMemoryObject command id %d"), m_pCmdDesc->id);
    NotifyCommandStatusChanged(CL_COMPLETE, CL_DEV_SUCCESS);

    return true;
}


//------------------------------------------------
// FillMemoryObject Command
cl_dev_err_code FillMemoryObject::Create(ISPDeviceQueue* pDeviceQueue, cl_dev_cmd_desc* pCmd, SharedPtr<ITaskBase>* ppTask)
{
    assert(nullptr != pDeviceQueue && "Invalid create command parameter");
    assert(nullptr != pCmd && "Invalid create command parameter");
    assert(nullptr != ppTask && "Invalid create command parameter");

    FillMemoryObject* pCommand = new FillMemoryObject(
                                pDeviceQueue->GetLogDescriptor(),
                                pDeviceQueue->GetLogHandle(),
                                pCmd,
                                pDeviceQueue->GetFrameworkCallbacks());
    if (nullptr == pCommand)
    {
        return CL_DEV_OUT_OF_MEMORY;
    }

    *ppTask = pCommand;

    return CL_DEV_SUCCESS;
}

bool FillMemoryObject::Execute()
{
    NotifyCommandStatusChanged(CL_RUNNING, CL_DEV_SUCCESS);
    IspDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("Executing FillMemoryObject command id %d"), m_pCmdDesc->id);

    cl_dev_cmd_param_fill*  cmdParams = reinterpret_cast<cl_dev_cmd_param_fill*>(m_pCmdDesc->params);
    cl_mem_obj_descriptor*  pMemObj;

    cl_dev_err_code ret;
    ret = cmdParams->memObj->clDevMemObjGetDescriptor(CL_DEVICE_TYPE_CUSTOM, 0, (cl_dev_memobj_handle*) &pMemObj);
    assert(CL_DEV_SUCCEEDED(ret) && nullptr != pMemObj && "Failed to get memory object descriptor");

    // we will fill the requested region row by row

    // row size (width) in bytes.
    const size_t rowSize =  cmdParams->region[0] * pMemObj->uiElementSize;

    // each row has his height and depth in the image
    size_t requestedDepthStart = 0;
    size_t requestedDepthEnd = 1;
    size_t requestedHeightStart = 0;
    size_t requestedHeightEnd = 1;
    if(cmdParams->dim_count > 2)
    {
        requestedDepthStart = cmdParams->offset[2];
        requestedDepthEnd = requestedDepthStart + cmdParams->region[2];
    }
    if(cmdParams->dim_count > 1)
    {
        requestedHeightStart = cmdParams->offset[1];
        requestedHeightEnd = requestedHeightStart + cmdParams->region[1];
    }

    // create one requested row and fill it with pattern/color
    cl_uchar* tempFilledRow = new cl_uchar[ rowSize ];
    if (nullptr == tempFilledRow)
    {
        NotifyCommandStatusChanged(CL_COMPLETE, CL_DEV_OUT_OF_MEMORY);
        return false;
    }
    // fill the row with pattern
    CopyPattern(cmdParams->pattern, cmdParams->pattern_size, tempFilledRow, rowSize);

    // iterate on the requested region and fill it row by row
    size_t offset[MAX_WORK_DIM];
    offset[0] = cmdParams->offset[0];
    for(size_t currentDepth = requestedDepthStart; currentDepth < requestedDepthEnd; ++currentDepth)
    {
        offset[2] = currentDepth;
        for(size_t currentHeight = requestedHeightStart; currentHeight < requestedHeightEnd; ++currentHeight)
        {
            offset[1] = currentHeight;
            void* dst = CalculateOffsetPointer(pMemObj->pData, cmdParams->dim_count, offset, pMemObj->pitch, pMemObj->uiElementSize);
            MEMCPY_S(dst, rowSize, tempFilledRow, rowSize);
        }
    }

    delete[] tempFilledRow;

    IspDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("Done executing FillMemoryObject command id %d"), m_pCmdDesc->id);
    NotifyCommandStatusChanged(CL_COMPLETE, CL_DEV_SUCCESS);

    return true;
}



//------------------------------------------------
// MigrateMemoryObject Command
cl_dev_err_code MigrateMemoryObject::Create(ISPDeviceQueue* pDeviceQueue, cl_dev_cmd_desc* pCmd, SharedPtr<ITaskBase>* ppTask)
{
    assert(nullptr != pDeviceQueue && "Invalid create command parameter");
    assert(nullptr != pCmd && "Invalid create command parameter");
    assert(nullptr != ppTask && "Invalid create command parameter");

    MigrateMemoryObject* pCommand = new MigrateMemoryObject(
                                pDeviceQueue->GetLogDescriptor(),
                                pDeviceQueue->GetLogHandle(),
                                pCmd,
                                pDeviceQueue->GetFrameworkCallbacks());
    if (nullptr == pCommand)
    {
        return CL_DEV_OUT_OF_MEMORY;
    }

    *ppTask = pCommand;

    return CL_DEV_SUCCESS;
}

bool MigrateMemoryObject::Execute()
{
    NotifyCommandStatusChanged(CL_RUNNING, CL_DEV_SUCCESS);
    IspDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("Executing MigrateMemoryObject command id %d"), m_pCmdDesc->id);

    // memory is already shared across devices so nothing to do here

    IspDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("Done executing MigrateMemoryObject command id %d"), m_pCmdDesc->id);
    NotifyCommandStatusChanged(CL_COMPLETE, CL_DEV_SUCCESS);
    return true;
}


//------------------------------------------------
// CommandFailureNotification
cl_dev_err_code CommandFailureNotification::Create(ISPDeviceQueue* pDeviceQueue, cl_dev_cmd_desc* pCmd, SharedPtr<ITaskBase>* ppTask)
{
    assert(nullptr != pDeviceQueue && "Invalid create command parameter");
    assert(nullptr != pCmd && "Invalid create command parameter");
    assert(nullptr != ppTask && "Invalid create command parameter");

    CommandFailureNotification* pCommand = new CommandFailureNotification(
                                pDeviceQueue->GetLogDescriptor(),
                                pDeviceQueue->GetLogHandle(),
                                pCmd,
                                pDeviceQueue->GetFrameworkCallbacks());
    if (nullptr == pCommand)
    {
        return CL_DEV_OUT_OF_MEMORY;
    }

    *ppTask = pCommand;

    return CL_DEV_SUCCESS;
}

//------------------------------------------------
// extension mode
// PipeLineNDRange
cl_dev_err_code PipelineNDRange::Create(ISPDeviceQueue* pDeviceQueue, std::vector<cl_dev_cmd_desc*>& commands, SharedPtr<ITaskBase>* ppTask)
{
    assert(nullptr != pDeviceQueue && "Invalid create command parameter");
    assert(nullptr != ppTask && "Invalid create command parameter");

    PipelineNDRange* pCommand = new PipelineNDRange(
                                pDeviceQueue->GetLogDescriptor(),
                                pDeviceQueue->GetLogHandle(),
                                commands,
                                pDeviceQueue->GetFrameworkCallbacks(),
                                pDeviceQueue->GetCameraHandle(),
                                pDeviceQueue->GetExtensionManager());
    if (nullptr == pCommand)
    {
        return CL_DEV_OUT_OF_MEMORY;
    }

    *ppTask = pCommand;

    return CL_DEV_SUCCESS;
}

PipelineNDRange::PipelineNDRange(IOCLDevLogDescriptor* logDesc, cl_int logHandle, std::vector<cl_dev_cmd_desc*>& commands,
                IOCLFrameworkCallbacks* frameworkCallbacks, CameraShim* pCameraShim, ISPExtensionManager* pIspExtensionManager)  :
                ISPCommandBase(logDesc, logHandle, nullptr, frameworkCallbacks), m_vCommandsDesc(commands),
                m_pCameraShim(pCameraShim), m_pIspExtensionManager(pIspExtensionManager)
{
    assert(m_vCommandsDesc.size() >= 2 && "Pipeline command should at least contain BEGIN_PIPELINE and END_PIPELINE");

    m_itrBeginPipelineCmdDesc = m_vCommandsDesc.begin();
    m_itrEndPipelineCmdDesc = m_vCommandsDesc.end();
    --m_itrEndPipelineCmdDesc;

    cl_dev_cmd_param_kernel* cmdParams;
    const ISPKernel* pIspKernel;

    cmdParams = reinterpret_cast<cl_dev_cmd_param_kernel*>((*m_itrBeginPipelineCmdDesc)->params);
    pIspKernel = reinterpret_cast<const ISPKernel*>(cmdParams->kernel);
    assert(pIspKernel->GetCameraCommand() == CAMERA_BEGIN_PIPELINE && "First command in pipeline should be BEGIN_PIPELINE");

    cmdParams = reinterpret_cast<cl_dev_cmd_param_kernel*>((*m_itrEndPipelineCmdDesc)->params);
    pIspKernel = reinterpret_cast<const ISPKernel*>(cmdParams->kernel);
    assert(pIspKernel->GetCameraCommand() == CAMERA_END_PIPELINE && "Last command in pipeline should be END_PIPELINE");

    // extract the pipeline stages (between BEGIN and END)
    std::vector<cl_dev_cmd_desc*>::iterator pStageCmdDesc = m_itrBeginPipelineCmdDesc;
    ++pStageCmdDesc;

    for ( ;pStageCmdDesc != m_itrEndPipelineCmdDesc; ++pStageCmdDesc)
    {
        m_vPipelineStages.push_back(*pStageCmdDesc);
    }
}

bool PipelineNDRange::Execute()
{
    NotifyCommandStatusChanged(CL_RUNNING, CL_DEV_SUCCESS);

    // iterate over the pipeline stages (between BEGIN and END)
    for (std::vector<cl_dev_cmd_desc*>::iterator pPipelineStage = m_vPipelineStages.begin();
         pPipelineStage != m_vPipelineStages.end(); ++pPipelineStage)
    {
        cl_dev_cmd_param_kernel* cmdParams = reinterpret_cast<cl_dev_cmd_param_kernel*>((*pPipelineStage)->params);
        const ISPKernel* pIspKernel = reinterpret_cast<const ISPKernel*>(cmdParams->kernel);
        void* argsValuesBase = cmdParams->arg_values;

        if (CL_FAILED(m_pIspExtensionManager->RequestExtension(pIspKernel, argsValuesBase)))
        {
            m_pIspExtensionManager->ClearRequested();
            //TODO: fail
        }
    }

    if (CL_FAILED(m_pIspExtensionManager->CreatePipelineFromRequested()))
    {
        //TODO: fail
    }

    cl_dev_cmd_param_kernel* cmdParams = reinterpret_cast<cl_dev_cmd_param_kernel*>((*m_itrBeginPipelineCmdDesc)->params);

    IOCLDevMemoryObject* pMemObj = *((IOCLDevMemoryObject**)(cmdParams->arg_values));
    if (nullptr == pMemObj)
    {
        //TODO: fail
    }

    cl_mem_obj_descriptor* pMemObjDesc = nullptr;
    pMemObj->clDevMemObjGetDescriptor(CL_DEVICE_TYPE_CUSTOM, 0, (cl_dev_memobj_handle*) &pMemObjDesc);
    assert(nullptr != pMemObjDesc && "Couldn't get the memory object descriptor");

    //TODO
    m_pCameraShim->preview_start();

    Frame currentFrame;
    status_t ret = m_pCameraShim->preview_grabbuffer(currentFrame);
    if (0 != ret)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Could not grab preview buffer"));
        return false;
    }
    size_t bytesToCopy = currentFrame.size;

    if (pMemObjDesc->dimensions.buffer_size < bytesToCopy)
    {
        // not a fatal error for the command
        bytesToCopy = pMemObjDesc->dimensions.buffer_size;
        // TODO
        //IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Mem object too small, can only copy partial preview buffer"));
    }

    IspDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("MEMCPY_S(%p, %d, %p, %d)"), pMemObjDesc->pData, pMemObjDesc->dimensions.buffer_size, currentFrame.img_data, bytesToCopy);
    MEMCPY_S(pMemObjDesc->pData, pMemObjDesc->dimensions.buffer_size, currentFrame.img_data, bytesToCopy);

    ret = m_pCameraShim->preview_releasebuffer(currentFrame);
    if(0 != ret)
    {
        // TODO
        //IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Could not release preview buffer"));
        //return false;
    }

    NotifyCommandStatusChanged(CL_COMPLETE, CL_DEV_SUCCESS);
    return true;
}

void PipelineNDRange::NotifyCommandStatusChanged(cl_int status, cl_dev_err_code errorCode)
{
    if (CL_COMPLETE == status)
    {
        // TODO: add ITask->Release ?
        m_bCompleted = true;
    }

    cl_ulong timer = 0;

    // notify framework about status change
    for (std::vector<cl_dev_cmd_desc*>::iterator pCmdDesc = m_vCommandsDesc.begin();
         pCmdDesc != m_vCommandsDesc.end();
         ++pCmdDesc)
    {
        // if profiling enabled for the command get timer
        if ((*pCmdDesc)->profiling)
        {
            timer = HostTime();
        }
        m_pFrameworkCallBacks->clDevCmdStatusChanged((*pCmdDesc)->id, (*pCmdDesc)->data, status, (cl_int)errorCode, timer);
    }
}




