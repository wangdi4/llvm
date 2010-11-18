// Copyright (c) 2006-2009 Intel Corporation
// All rights reserved.
// 
// WARRANTY DISCLAIMER
// 
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING ANY WAY OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

///////////////////////////////////////////////////////////
//  lrb_agnet.cpp
///////////////////////////////////////////////////////////
#include "lrb_agent.h"

#include "lrb_memory_manager.h"
#include "lrb_command_executer.h"
#include "lrb_program_service.h"
#include "lrb_communicator.h"

#include <cl_logger.h>
#include <cl_types.h>
#include <stdio.h>

using namespace Intel::OpenCL::LRBAgent;

/************************************************************************
 * Device info definitions.
 ************************************************************************/
const cl_uint   LRB_MAX_WORK_ITEM_DIMENSIONS = 3;
const size_t    LRB_MAX_WORK_GROUP_SIZE =1024; // TODO: Set it to a relevant number
const size_t    LRB_MAX_WORK_ITEM_SIZES[LRB_MAX_WORK_ITEM_DIMENSIONS] = 
{
    LRB_MAX_WORK_GROUP_SIZE,    // In each DIM the max is the max size of work group, in that case other dims should be 1,1
    LRB_MAX_WORK_GROUP_SIZE,
    LRB_MAX_WORK_GROUP_SIZE
};


/************************************************************************
 * 
 ************************************************************************/
char* clDevErr2Txt(cl_dev_err_code clDevErrCode)
{
    switch(clDevErrCode)
    {
    case (CL_DEV_ERROR_FAIL):           return "CL_DEV_ERROR_FAIL";
    case (CL_DEV_INVALID_VALUE):        return "CL_DEV_INVALID_VALUE";
    case (CL_DEV_INVALID_PROPERTIES):   return "CL_DEV_INVALID_PROPERTIES";
    case (CL_DEV_OUT_OF_MEMORY):        return "CL_DEV_OUT_OF_MEMORY";
    case (CL_DEV_INVALID_COMMAND_LIST): return "CL_DEV_INVALID_COMMAND_LIST";
    case (CL_DEV_INVALID_COMMAND_TYPE): return "CL_DEV_INVALID_COMMAND_TYPE";
    case (CL_DEV_INVALID_MEM_OBJECT):   return "CL_DEV_INVALID_MEM_OBJECT";
    case (CL_DEV_INVALID_KERNEL):       return "CL_DEV_INVALID_KERNEL";
    case (CL_DEV_INVALID_OPERATION):    return "CL_DEV_INVALID_OPERATION";
    case (CL_DEV_INVALID_WRK_DIM):      return "CL_DEV_INVALID_WRK_DIM";
    case (CL_DEV_INVALID_WG_SIZE):      return "CL_DEV_INVALID_WG_SIZE";
    case (CL_DEV_INVALID_GLB_OFFSET):   return "CL_DEV_INVALID_GLB_OFFSET";
    case (CL_DEV_INVALID_WRK_ITEM_SIZE):return "CL_DEV_INVALID_WRK_ITEM_SIZE";
    case (CL_DEV_INVALID_IMG_FORMAT):   return "CL_DEV_INVALID_IMG_FORMAT";
    case (CL_DEV_INVALID_IMG_SIZE):     return "CL_DEV_INVALID_IMG_SIZE";
    case (CL_DEV_OBJECT_ALLOC_FAIL):    return "CL_DEV_INVALID_COMMAND_LIST";
    case (CL_DEV_INVALID_BINARY):       return "CL_DEV_INVALID_BINARY";
    case (CL_DEV_INVALID_BUILD_OPTIONS):return "CL_DEV_INVALID_BUILD_OPTIONS";
    case (CL_DEV_INVALID_PROGRAM):      return "CL_DEV_INVALID_PROGRAM";
    case (CL_DEV_STILL_BUILDING):       return "CL_DEV_STILL_BUILDING";
    case (CL_DEV_INVALID_KERNEL_NAME):  return "CL_DEV_INVALID_KERNEL_NAME";    
    default:                            return "Unknown Error Code";

    }
}

// Static members initialization
LrbAgent* LrbAgent::m_pLrbInstance = NULL;

/******************************************************************
 *
 ******************************************************************/
LrbAgent::LrbAgent()
{
}

/******************************************************************
 *
 ******************************************************************/
LrbAgent::~LrbAgent()
{
}

/******************************************************************
 * Destroy static function is used to free the Lrb instance.
 *
 ******************************************************************/
void LrbAgent::Destroy()
{
    if(NULL != m_pLrbInstance)
    {
        m_pLrbInstance->Release();
        delete m_pLrbInstance;
        m_pLrbInstance = NULL;
    }
}

/******************************************************************
 *
 ******************************************************************/
cl_int LrbAgent::Initialize(cl_uint uiDevId, cl_dev_call_backs *devCallbacks, cl_dev_log_descriptor *clLogDesc)
{
    cl_int errCode = CL_SUCCESS;
    
    memcpy(&m_clRuntimeCallBacks, devCallbacks, sizeof(cl_dev_call_backs));
    // Use runtime logger for debug printing. 
    errCode = SetLogger(uiDevId, clLogDesc);
    if(CL_FAILED(errCode))
    {
        return errCode;
    }

    // 
    // The code below initialize Native SDK executable.
    //
    m_xnWrapper.Initialize();
    
    m_pMemManager = new LrbMemoryManager(uiDevId, &m_xnWrapper, m_clRuntimeCallBacks.pclDevCmdStatusChanged);
    m_pCommandExecuter = new LrbCommandExecuter(&m_xnWrapper, m_pMemManager);
    m_pProgServices = new LrbProgramService(&m_xnWrapper, m_clRuntimeCallBacks.pclDevBuildStatusUpdate);
    m_pCommunicator = new LrbCommunicator(&m_xnWrapper, m_pCommandExecuter, m_pProgServices, m_clRuntimeCallBacks.pclDevCmdStatusChanged);
    m_pMemManager->SetCmdDoneListener(m_pCommandExecuter);
    m_xnWrapper.SetPrgramService(m_pProgServices);
    m_xnWrapper.SetMemoryManager(m_pMemManager); 

    // Start working threads
    m_pCommunicator->Start();
    m_pMemManager->StartMemoryTransactor();

    return errCode;
}

/******************************************************************
 * Release frees object resources including unloading Native code on LRB
 ******************************************************************/
cl_int LrbAgent::Release()
{
    delete m_pMemManager;
    delete m_pCommandExecuter;
    delete m_pCommunicator;
    m_xnWrapper.Release();
    return 0;
}

/******************************************************************
 *
 ******************************************************************/
cl_int LrbAgent::SetLogger(cl_uint uiDevId, cl_dev_log_descriptor *logDesc)
{
    // Do nothing
    // TODO: Add logger
    return CL_SUCCESS;
}

///////////////////////////////////////////////////////////////////
// Device entry points:
// THe following function implement the device entry point callbacks.
//
///////////////////////////////////////////////////////////////////

/******************************************************************
 *
 *  clDevGetDeviceInfo
 *  Description
 *      This function return device specific information defined by cl_device_info enumeration as specified in OCL spec. table 4.3.
 *  Input
 *      param                    An enumeration that identifies the device information being queried. It can be one of
 *                              the following values as specified in OCL spec. table 4.3
 *      valSize                Specifies the size in bytes of memory pointed to by paramValue. This size in
 *                              bytes must be >= size of return type
 *  Output
 *      pParamVal                A pointer to memory location where appropriate values for a given param as specified in OCL spec. table 4.3 will be returned. If pParamVal is NULL, it is ignored
 *      paramValSize_ret        Returns the actual size in bytes of data being queried by pParamVal. If paramValSize_ret is NULL, it is ignored
 *  Returns
 *      CL_DEV_SUCCESS            If functions is executed successfully.
 *      CL_DEV_INVALID_VALUE    If param_name is not one of the supported values or if size in bytes specified by paramValSize is < size of return type as specified in OCL spec. table 4.3 and pParamVal is not a NULL value
 *
 ******************************************************************/
cl_int LrbAgent::clDevGetDeviceInfo(
    cl_device_info  clParam, 
    size_t          szValSize, 
    void*           pParamVal,
    size_t*         szParamValSizeRet
    )
{
    size_t  internalRetunedValueSize = szValSize;
    size_t  *pinternalRetunedValueSize;

    //if both pParamVal and paramValSize is NULL return error
    if(NULL == pParamVal && NULL == szParamValSizeRet)
    {
        return CL_DEV_INVALID_VALUE;
    }
    //if paramValSize_ret is NULL it should be ignored
    if(szParamValSizeRet)
    {
        pinternalRetunedValueSize = szParamValSizeRet;
    }
    else
    {
        pinternalRetunedValueSize = &internalRetunedValueSize;
    }
    
    //
    // Switch all device info parameters
    //
    switch (clParam)
    {
    case( CL_DEVICE_TYPE):
        {
            *pinternalRetunedValueSize = sizeof(cl_device_type);
            if(pParamVal)
            {
                if (*pinternalRetunedValueSize > szValSize) return CL_DEV_INVALID_VALUE;
                *(cl_device_type*)pParamVal = (cl_device_type)CL_DEVICE_TYPE_GPU;
            }
            break;
        }
    case( CL_DEVICE_MAX_MEM_ALLOC_SIZE):
        {
            *pinternalRetunedValueSize = sizeof(cl_ulong);
            if(pParamVal)
            {
                if (*pinternalRetunedValueSize > szValSize) return CL_DEV_INVALID_VALUE;
                *(cl_ulong*)pParamVal = 128*1024*1024; // For now: 128 MB, TODO: Set something that actually means
            }
            break;
        }
    case( CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS):
        {
            *pinternalRetunedValueSize = sizeof(cl_uint);
            if(pParamVal)
            {
                if (*pinternalRetunedValueSize > szValSize) return CL_DEV_INVALID_VALUE;
                *(cl_uint*)pParamVal = LRB_MAX_WORK_ITEM_DIMENSIONS;
            }
            break;

        }
    case( CL_DEVICE_MAX_WORK_GROUP_SIZE):
        {
            *pinternalRetunedValueSize = sizeof(size_t);
            if (pParamVal)
            {
                if (*pinternalRetunedValueSize > szValSize) return CL_DEV_INVALID_VALUE;
                *(size_t*)pParamVal = LRB_MAX_WORK_GROUP_SIZE; 
            }
            break;
        }
    case( CL_DEVICE_MAX_WORK_ITEM_SIZES):
        {
            *pinternalRetunedValueSize = LRB_MAX_WORK_ITEM_DIMENSIONS * sizeof(size_t);
            if (pParamVal)
            {
                if (*pinternalRetunedValueSize > szValSize) return CL_DEV_INVALID_VALUE;
                memcpy(pParamVal, LRB_MAX_WORK_ITEM_SIZES, *pinternalRetunedValueSize);            
            }
            break;
        }
    case( CL_DEVICE_VENDOR_ID):
    case( CL_DEVICE_MAX_COMPUTE_UNITS):
    case( CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR):
    case( CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT):
    case( CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT):
    case( CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT):
    case( CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG): 
    case( CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE):
    case( CL_DEVICE_IMAGE_SUPPORT):
    case( CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE):
    case( CL_DEVICE_SINGLE_FP_CONFIG):    
    case( CL_DEVICE_IMAGE2D_MAX_WIDTH): 
    case( CL_DEVICE_IMAGE2D_MAX_HEIGHT):
    case( CL_DEVICE_IMAGE3D_MAX_WIDTH): 
    case( CL_DEVICE_IMAGE3D_MAX_HEIGHT):
    case( CL_DEVICE_IMAGE3D_MAX_DEPTH):
    case( CL_DEVICE_MAX_PARAMETER_SIZE):
    case( CL_DEVICE_MAX_SAMPLERS):
    case( CL_DEVICE_MAX_READ_IMAGE_ARGS):
    case( CL_DEVICE_MAX_WRITE_IMAGE_ARGS):
    case( CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE):
    case( CL_DEVICE_MAX_CONSTANT_ARGS ):
    case( CL_DEVICE_MEM_BASE_ADDR_ALIGN):
    case( CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE):
    case( CL_DEVICE_GLOBAL_MEM_CACHE_SIZE):
    case( CL_DEVICE_LOCAL_MEM_SIZE):
    case( CL_DEVICE_MAX_CLOCK_FREQUENCY):
    case( CL_DEVICE_ADDRESS_BITS):
    case( CL_DEVICE_PROFILING_TIMER_RESOLUTION):
    case( CL_DEVICE_GLOBAL_MEM_CACHE_TYPE):
    case( CL_DEVICE_GLOBAL_MEM_SIZE):
    case( CL_DEVICE_ENDIAN_LITTLE):
    case( CL_DEVICE_ERROR_CORRECTION_SUPPORT):
    case( CL_DEVICE_LOCAL_MEM_TYPE):
    case( CL_DEVICE_AVAILABLE):
    case( CL_DEVICE_EXECUTION_CAPABILITIES):
    case( CL_DEVICE_QUEUE_PROPERTIES ):
    case( CL_DEVICE_COMPILER_AVAILABLE):
    case( CL_DEVICE_NAME):
    case( CL_DEVICE_VENDOR):
    case( CL_DEVICE_PROFILE):
    case( CL_DEVICE_VERSION):
    case( CL_DEVICE_EXTENSIONS):
    case( CL_DRIVER_VERSION ):
        // For now, all above fall trough here
        break;

    default:
        return CL_DEV_INVALID_VALUE;
    };
    return CL_DEV_SUCCESS;

}

/******************************************************************
 * clDevCreateCommandList
 *  
 *
 ******************************************************************/
cl_int LrbAgent::clDevCreateCommandList(cl_dev_cmd_list_props props, cl_dev_cmd_list* pCmdList)
{
    if (NULL == pCmdList)
    {
        return CL_DEV_INVALID_VALUE;
    }
    return m_pLrbInstance->m_pCommandExecuter->CreateCommandList(props, pCmdList);
}
/******************************************************************
 * clDevRetainCommandList
 *  
 *
 ******************************************************************/
cl_int LrbAgent::clDevRetainCommandList(cl_dev_cmd_list clList)
{
    return m_pLrbInstance->m_pCommandExecuter->RetainCommandList(clList);
}
/******************************************************************
 * clDevReleaseCommandList
 *  
 *
 ******************************************************************/
cl_int LrbAgent::clDevReleaseCommandList(cl_dev_cmd_list clList)
{
    return m_pLrbInstance->m_pCommandExecuter->ReleaseCommandList(clList);
}

/******************************************************************
* clDevFlushCommandList
*  
*
******************************************************************/
cl_int LrbAgent::clDevFlushCommandList(cl_dev_cmd_list clList)
{
    return m_pLrbInstance->m_pCommandExecuter->FlushCommandList(clList);
}

/******************************************************************
 * clDevCommandListExecute
 *  Call TaskDispatcher to execute command list
 *
 ******************************************************************/
cl_int LrbAgent::clDevCommandListExecute(cl_dev_cmd_list list, cl_dev_cmd_desc** ppCmds, cl_uint uiCount)
{
    if (NULL == ppCmds)
    {
        return CL_DEV_INVALID_VALUE;
    }
    return m_pLrbInstance->m_pCommandExecuter->ExecuteCommands(list, ppCmds, uiCount);
}

/******************************************************************
 * clDevGetSupportedImageFormats
 *  Call Memory Allocator to get supported image formats
 *
 ******************************************************************/
cl_int LrbAgent::clDevGetSupportedImageFormats(cl_dev_mem_flags flags, cl_dev_mem_object_type imageType, cl_uint numEntries, cl_image_format* formats, cl_uint* numEntriesRet)
{
    return CL_SUCCESS;
    
}
/******************************************************************
 * clDevCreateMemoryObject
 *  Allocate memory objects.
 *
 ******************************************************************/
cl_int LrbAgent::clDevCreateMemoryObject(cl_dev_mem_flags clFlags, const cl_image_format* cpClFormat, cl_uint uiDimCount, const size_t* cpszDimSize, void* pData, const size_t* cpPitchs, cl_dev_host_ptr_flags clHostPtrFlags, cl_dev_mem* pMemObj)
{
    //
    // TODO: validate parameters,
    //
    cl_int result = m_pLrbInstance->m_pMemManager->CreateMemObject(clFlags, cpClFormat, uiDimCount, cpszDimSize, pData, cpPitchs, clHostPtrFlags, pMemObj);
    return result;
}
/******************************************************************
 * clDevDeleteMemoryObject
 *      Call Memory Allocator to delete memory object
 *
 ******************************************************************/
cl_int LrbAgent::clDevDeleteMemoryObject( cl_dev_mem clMemObj )
{
    cl_int result = m_pLrbInstance->m_pMemManager->DeleteMemObject(clMemObj);
    return result;
}
/******************************************************************
 * clDevCreateMappedRegion
 *      Call Memory Allocator to craete mapped region
 *
 ******************************************************************/
cl_int LrbAgent::clDevCreateMappedRegion( cl_dev_cmd_param_map* pclMapParams)
{
    return CL_SUCCESS;
}
/******************************************************************
 * clDevReleaseMappedRegion
 *      Call Memory Allocator to release mapped region
 *
 ******************************************************************/
cl_int LrbAgent::clDevReleaseMappedRegion( cl_dev_cmd_param_map* pclMapParams )
{
    return CL_SUCCESS;
}

/******************************************************************
 * clDevCheckProgramBinary
 *      Call Program Serice to check binaries
 *
 ******************************************************************/
cl_int LrbAgent::clDevCheckProgramBinary( size_t szBinSize, const void* cpBin )
{
    return CL_SUCCESS;
}

/******************************************************************
 * clDevCreateProgram    
 *      Call programService to create program
 *
 ******************************************************************/
cl_int LrbAgent::clDevCreateProgram(size_t binSize, const void* bin, cl_dev_binary_prop prop, cl_dev_program* prog)
{
    cl_int result = m_pLrbInstance->m_pProgServices->CreateProgram(binSize, bin, prop, prog);
    return result;
}

/******************************************************************
 * clDevBuildProgram
 *      Call programService to build program
 *
 ******************************************************************/
cl_int LrbAgent::clDevBuildProgram(cl_dev_program prog, const cl_char* options, void* userData)
{
    cl_int result = m_pLrbInstance->m_pProgServices->BuildProgram(prog, options, userData);
    return result;
}

/******************************************************************
 * clDevReleaseProgram
 *      Call programService to release program
 *
 ******************************************************************/
cl_int LrbAgent::clDevReleaseProgram(cl_dev_program prog)
{
    cl_int result = m_pLrbInstance->m_pProgServices->ReleaseProgram(prog);
    return result;
}

/******************************************************************
 * clDevUnloadCompiler
 *      Call programService to unload the backend compiler
 *
 ******************************************************************/
cl_int LrbAgent::clDevUnloadCompiler()
{
    return CL_SUCCESS;
}
/******************************************************************
 * clDevGetProgramBinary
 *      Call programService to get the program binary
 *
 ******************************************************************/
cl_int LrbAgent::clDevGetProgramBinary(cl_dev_program prog, size_t size, void* binary, size_t* sizeRet)
{
    return CL_SUCCESS;
}
/******************************************************************
 * clDevGetBuildLog
 *      Call programService to get the build log
 *
 ******************************************************************/
cl_int LrbAgent::clDevGetBuildLog(cl_dev_program prog, size_t size, char* log, size_t* size_ret)
{
    return CL_SUCCESS;
}
/******************************************************************
 * clDevGetSupportedBinaries
 *      Call programService to get supported binary description
 *
 ******************************************************************/
cl_int LrbAgent::clDevGetSupportedBinaries( cl_uint uCount, cl_prog_binary_desc* clTypes, size_t* szSizeRet )
{
    return CL_SUCCESS;
}
/******************************************************************
 * clDevGetKernelId
 *      Call programService to get kernel id from its name
 *
 ******************************************************************/
cl_int LrbAgent::clDevGetKernelId(cl_dev_program prog, const char* name, cl_dev_kernel* kernelId)
{
    cl_int result = m_pLrbInstance->m_pProgServices->GetKernelId(prog, name, kernelId);
    return result;
}
/******************************************************************
 * clDevGetProgramKernels
 *      Call programService to get kernels from the program
 *
 ******************************************************************/
cl_int LrbAgent::clDevGetProgramKernels(cl_dev_program prog, cl_uint numKernels, cl_dev_kernel* kernels, cl_uint* numKernelsRet)
{
    cl_int result = m_pLrbInstance->m_pProgServices->GetProgramKernels(prog, numKernels, kernels, numKernelsRet);
    return result;
}
/******************************************************************
 * clDevGetKernelInfo
 *      Call programService to get kernel info
 *
 ******************************************************************/
cl_int LrbAgent::clDevGetKernelInfo(cl_dev_kernel kernel, cl_dev_kernel_info param, size_t valueSize, void* value, size_t* valueSizeRet)
{
    cl_int result = m_pLrbInstance->m_pProgServices->GetKernelInfo(kernel, param, valueSize, value, valueSizeRet);
    return result;
}

/******************************************************************
 * clDevCloseDevice
 *      Close device
 *
 ******************************************************************/
void LrbAgent::clDevCloseDevice(void)
{
    // Unload the device instance
    LrbAgent::Destroy();
}


