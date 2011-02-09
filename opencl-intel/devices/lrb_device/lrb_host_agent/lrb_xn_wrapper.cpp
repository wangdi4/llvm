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
//  lrb_xn_wrapper.cpp
///////////////////////////////////////////////////////////
#include "lrb_xn_wrapper.h"
#include "lrb_agent_common.h"
#include "lrb_program_service.h"
#include "lrb_memory_manager.h"
#include "lrb_commands_list.h"

#include <cl_types.h>
#include <cl_env.h>
#include <host/XN0_host.h>
#include <stdio.h>

using namespace Intel::OpenCL::LRBAgent;
using namespace Intel::OpenCL::Utils;

/******************************************************************
 * Conversion tables -
 * The tables below convert between OCL Device API types and XN API Type
 ******************************************************************/
XN_MAP xnMapFlagsTable[] =
{
    XN_MAP_READ,            // Stub, 0 is not expected.
    XN_MAP_READ,        	// CL_DEV_MEM_READ       = 1
    XN_MAP_READ_WRITE,     	// CL_DEV_MEM_WRITE      = 2 Write is read/write since we need to write back also parts that were not changed
    XN_MAP_READ_WRITE   	// CL_DEV_MEM_READ_WRITE = 3
};

/******************************************************************
 *
 ******************************************************************/
XNWrapper::XNWrapper():
    m_xnContext(0),
    m_xnLibrary(0)
{
}

/******************************************************************
 *
 ******************************************************************/
XNWrapper::~XNWrapper()
{
}


/******************************************************************
 *
 ******************************************************************/
cl_int XNWrapper::Initialize()
{
    // 
    // The code below initialize Native SDK executable.
    //
    return InitLrbNative();
}

/******************************************************************
 * 
 ******************************************************************/
cl_int XNWrapper::Release()
{
    return ReleaseLrbNative();
}

/******************************************************************
 * This function initiate the native executable, load it and query
 * for constant data that is required by the agent.
 ******************************************************************/
cl_int XNWrapper::InitLrbNative()
{
    XNERROR result = XN_SUCCESS;

    // Detect the Larrabee devices which are installed on the system.
    uint32_t uiNumberOfEngines = XN0EngineGetCount();
    printf("HOST: Found %d Larrabee device(s)\n", uiNumberOfEngines);

    // 2) Create a context with a run schedule on the first Larrabee device .
    // TODO: Change context not to use run schedule
    XNENGINE engine;
    XN0EngineGetHandle( 0, &engine );
    printf("HOST: Loading Larrabee executable...\n");

    // TODO: Use flush instead of runSchedule
    XN_RUN_SCHEDULE runSchedule;
    runSchedule.frequencyInHz = 1;
    runSchedule.executionQuantumInUsecs = 500000;
    result = XN0ContextCreate(engine, &runSchedule, &m_xnContext);
    if(XN_SUCCESS != result)
    {
        return result;
    }

    // Load the program/library into the context
    std::string libPath;
    result = (XNERROR)GetEnvVar(libPath, "MTV_LOCAL_BIN_DIR");
    char libName[MAX_PATH];
    sprintf_s( libName, "%s\\lrb_native_agent.dll", libPath.c_str());

    // this loads the library into the context and executes it's main function
    // The main function is initiate the device and wait until is signaled to terminate
    result = XN0ContextLoadLib( m_xnContext, libName, &m_xnLibrary );
    if(XN_SUCCESS != result)
    {
        return result;
    }

    //
    // Initialize the communicator.
    // 
    XN0MessageCreateCommunicator( COMMAND_STATUS_CHANGE_COMMUNICATOR, XN_MESSAGE_MAX_SIZE, &m_xnCommunicator );
    m_uiMessageSize = sizeof(CommandDoneMessage);

    return result;
}

/******************************************************************
 * This function releases all LRB Native executable related objects.
 * 
 ******************************************************************/
cl_int XNWrapper::ReleaseLrbNative()
{
    XNERROR result;
    if( 0 != m_xnLibrary)
    {
        printf("HOST: Waiting for the Larrabee process to exit...\n");
        int exitCode = -1;
        // this will unload the library, but not until after the main function has finished, so keep trying until success.
        while (1)
        {
            result = XN0ContextUnloadLib(m_xnLibrary, 1000, &exitCode);
            if (XN_TIME_OUT_REACHED != result)
            {
                break;
            }
        };

        printf("HOST: Received an exit code of %d from the Larrabee process.\n", exitCode );
    }

    // Free the context.
    if(0 != m_xnContext)
    {
        result = XN0ContextDestroy(m_xnContext);
    }

    // Free communicator
    if(0 != m_xnCommunicator)
    {        
        result = XN0MessageDestroyCommunicator( m_xnCommunicator );
    }
    return (cl_int)result;
}


/******************************************************************
 * 
 ******************************************************************/
cl_int XNWrapper::GetDeviceInfo()
{
    return -1;
}

/************************************************************************
 * 
 ************************************************************************/
cl_int XNWrapper::ExecuteCommands( CommandEntry** ppCmds, cl_uint uiCmdsCount, bool bIsInOrder)
{
    // 1. Calculate the size of the commands buffer and the Buffer list
    uint32_t uiBufSize = CalculateBufferSize( ppCmds, uiCmdsCount );
    uint32_t uiXnBufArraySize = CalculateXnBufArraySize( ppCmds, uiCmdsCount);
    
    // 2. Create host temp commands buffer and buffer list;
    uint8_t* pCmdBuffer = (uint8_t*)malloc(uiBufSize*sizeof(uint8_t));
    XNBUFFER* pXnBufArray = (XNBUFFER*)malloc(uiXnBufArraySize*sizeof(XNBUFFER));

    GenerateOffloadData(ppCmds, uiCmdsCount, pCmdBuffer, pXnBufArray+1);
    
    XNFUNCTION xnFunc;
    XNERROR  result = XN0ContextGetLibFunctionHandle(m_xnLibrary, "NativeExecuteCmds", &xnFunc);
    if ( XN_SUCCESS == result)
    {
        //
        // 3. Create buffer list, buffers handles are extracted dependents on the command
        //    xnBuffersList[0] is always preserved for the commands buffer.
        //
        XN_BUFFER_LIST xnBufferList;
        xnBufferList.numBufs = uiXnBufArraySize;
        xnBufferList.pBufArray = pXnBufArray;

        //
        // 4. Create XN_USAGE_IMMUTABLE xnbuffer with the host temp buffer as input
        //
        XN_BUFFER_DESC xnBufferDesc;
        xnBufferDesc.CPUAccessFlags = 0; // No real meaning since buffer is immutable
        xnBufferDesc.size = uiBufSize;
        xnBufferDesc.Usage = XN_USAGE_IMMUTABLE ; // Initialize on creation and is read once by the device on build
        result = XN0ContextCreateBuffer(m_xnContext,&xnBufferDesc,pCmdBuffer, &(xnBufferList.pBufArray[0]));
        if( XN_SUCCESS != result)
        {
            // Error case
            free(pCmdBuffer);
            free(pXnBufArray);
            return CL_ERR_FAILURE;
        }

        //
        // Create misc data
        //
        execute_cmds_params MiscData;
        MiscData.uiIsListInOrder = (bIsInOrder ? 0x1 : 0x0);
        MiscData.uiNumCmds = uiCmdsCount;

        //
        // 5. execute RunFunction
        //
        result = XN0ContextRunFunction(xnFunc,xnBufferList, &MiscData, sizeof(execute_cmds_params));
        if( XN_SUCCESS != result)
        {
            // Error case
            free(pCmdBuffer);
            free(pXnBufArray);
            return CL_ERR_FAILURE;
        }
    } 

    free(pCmdBuffer);
    free(pXnBufArray);
    
    // TODO: What about the new buffer xnBufferList.pBufArray[0] - need to destroy it !!!

    return CL_SUCCESS;
}


/************************************************************************
 * Free resources that are related to the command
 ************************************************************************/
cl_int XNWrapper::UnmapCommand( CommandEntry* pPoppedEntry)
{
    if ( pPoppedEntry->uiNumOfBufHndls > 0 )
    {
        XNBUFFER* xnBufArr = (XNBUFFER*)pPoppedEntry->pMappedBufHndls;
        for ( cl_uint ui = 0; ui < pPoppedEntry->uiNumOfBufHndls; ui++)
        {
            XN0ContextRemoveBuffer(m_xnContext, xnBufArr[ui]);
        }
        delete[] pPoppedEntry->pMappedBufHndls;
        pPoppedEntry->pMappedBufHndls = 0;
        pPoppedEntry->uiNumOfBufHndls = 0;
    }
    return CL_SUCCESS;
}


/************************************************************************
 * Create an XN buffer to represent memory object on the device.
 * An handle to the buffer is returned in ppBufHndl.
 * 
 ************************************************************************/
cl_int XNWrapper::CreateBuffer(cl_dev_mem_flags clAccessFlags, size_t szBufSize, void* pInData, void** ppBufHndl)
{
    //
    // 
    // The buffer by default use XN_USAGE_DEFAULT, which means Read/Write access by LRB.
    // Anyway, the host cannot read this buffer unless it use a staging buffer as in MAP operation.
    // There is no problem to initialize the buffer on create with pInData
    // TODO: Set Usage according to input clAccessFlags (currently use defaults)
    //    
    XNBUFFER xnBuffer;
    XN_BUFFER_DESC xnBufferDesc;
    xnBufferDesc.CPUAccessFlags = XN_CPU_ACCESS_WRITE ; 
    xnBufferDesc.size = szBufSize;
    xnBufferDesc.Usage = XN_USAGE_DYNAMIC;
    XNERROR result = XN0ContextCreateBuffer(m_xnContext,&xnBufferDesc,pInData,&xnBuffer);
    *ppBufHndl = (void*)xnBuffer;

    return  (cl_int)result;
}

/************************************************************************
 * Destroy the buffer identified buy the pBufHndl
 ************************************************************************/
cl_int XNWrapper::DeleteBuffer( void* pBufHndl )
{
    XNERROR result = XN0ContextDestroyBuffer((XNBUFFER)pBufHndl);
    return  (cl_int)result;
}

/************************************************************************
 * Map a buffer identified by pBufHndl to the host pointer ppOutData
 * Currently this function supports only CL_DEV_MEM_READ or CL_DEV_MEM_WRITE
 * A temp staging memory is used, the map buffer will always upload the data
 * to enable write-back of part of the buffer.
 *                                       	
 ************************************************************************/
cl_int XNWrapper::MapBuffer(void* pBufHndl, cl_dev_mem_flags clAccessFlags, void** ppOutData)
{
    XNERROR result = XN_SUCCESS;
    XNBUFFER xnBuffer = (XNBUFFER)pBufHndl;

    //
    // Create staging buffer and copy to
    //
    XN_BUFFER_DESC xnBufferDesc;
    result = XN0BufferGetDesc(xnBuffer, &xnBufferDesc) ;
    if ( XN_SUCCESS != result)
    {
        return result;
    }
    XNBUFFER xnStagingBuffer;
    XN_BUFFER_DESC xnStagingBufferDesc;

    xnStagingBufferDesc.size = xnBufferDesc.size;
    xnStagingBufferDesc.CPUAccessFlags = XN_CPU_ACCESS_READ | XN_CPU_ACCESS_WRITE;
    xnStagingBufferDesc.Usage = XN_USAGE_STAGING; 
    result = XN0ContextCreateBuffer(m_xnContext,&xnStagingBufferDesc,NULL,&xnStagingBuffer);
    if ( XN_SUCCESS != result)
    {
        return result;
    }

    //
    // Copy original buffer to host accessible buffer
    //
    result = XN0ContextCopyBuffer(m_xnContext, xnStagingBuffer, xnBuffer);

    //
    // Map buffer
    //
    result = XN0BufferMap( xnStagingBuffer, xnMapFlagsTable[clAccessFlags], 0, ppOutData);
    if ( XN_SUCCESS != result)
    {
        // Delete buffer
        XN0ContextDestroyBuffer(xnStagingBuffer);
        return result;
    }

    BufferEntry entry = { xnStagingBuffer, xnMapFlagsTable[clAccessFlags] };
    m_mappedBuffer[xnBuffer] = entry;

    return  (cl_int)result;
}

/************************************************************************
 * If buffer is mapped for Read/Write so we need to write back after un-mapping
 *
 ************************************************************************/
cl_int XNWrapper::UnmapBuffer( void* pBufHndl )
{
    XNERROR result = XN_SUCCESS;
    XNBUFFER xnBuffer = (XNBUFFER)pBufHndl;
    if ( m_mappedBuffer.count(xnBuffer) > 0)
    {
        // There is a use of staging buffer; 
        // Should be always unless xnBuffer wasn't mapped before.
        map<XNBUFFER, BufferEntry>::iterator pos  = m_mappedBuffer.find(xnBuffer);
        XNBUFFER xnStagingBuffer = pos->second.xnMappedBuf;
        XN0BufferUnmap(xnStagingBuffer);
        if( XN_MAP_READ != pos->second.xnMapAccess )
        {
            // Buffer is not read only, so maybe is was written, there is a need to write back.
            result = XN0ContextCopyBuffer(m_xnContext, xnBuffer, xnStagingBuffer);
        }
        // delete temp buffer
        XN0ContextDestroyBuffer(xnStagingBuffer);
        m_mappedBuffer.erase(pos);
    }
    else
    {
        result = XN_INVALID_HANDLE;
    }
    return (cl_int)result;
}

/************************************************************************
 * This is a blocking function that waits until a new message is recieved 
 * from the communicator.
 * A message is always in the format of CommandDoneMessage. The user is
 * expected to allocate the message and the content is filled when a message
 * is received.
 * If succeeded, CL_SUCCESS is returned.
 *
 ************************************************************************/
cl_int XNWrapper::ReceiveMessageSync( CommandDoneMessage* pReceivedMessage)
{
    // Block call to receive message
    cl_int result = XN0MessageReceive( m_xnCommunicator, pReceivedMessage, m_uiMessageSize, NULL );
    return result;
}

/************************************************************************
 * This function is called when the communicator is done.
 *
 ************************************************************************/
cl_int XNWrapper::NotifyProcessDone()
{
    
    CommandDoneMessage receivedMessage;
    receivedMessage.cmdHndl = 0;
    cl_int result = XN0MessageSend( m_xnCommunicator, &receivedMessage, m_uiMessageSize);
    return result;
}

/************************************************************************
 * This function should load the program binaries to device.
 * The actual operation is to write the data into an XNBUFFER and this buffer
 * will be in used on build RunFunction
 *
 ************************************************************************/
cl_int XNWrapper::LoadProgram(size_t szBinSize, const void* pBinData, cl_ulong* pulProgHndl, cl_ulong* pulBuildOutHndl)
{
    XNBUFFER xnBuffer;
    XN_BUFFER_DESC xnBufferDesc;
    xnBufferDesc.CPUAccessFlags = 0; // No real meaning since buffer is immutable
    xnBufferDesc.size = (uint32_t)szBinSize;
    xnBufferDesc.Usage = XN_USAGE_IMMUTABLE ; // Initialize on creation and is read once by the device on build
    XNERROR result = XN0ContextCreateBuffer(m_xnContext,&xnBufferDesc,pBinData,&xnBuffer);
    if( XN_SUCCESS == result )
    {
        *pulProgHndl = xnBuffer;

        // Allocate input buffer
        xnBufferDesc.CPUAccessFlags = 0 ; // No real meaning since buffer is immutable
        xnBufferDesc.size = BUILD_OUTPUT_MAX_SISE;
        xnBufferDesc.Usage = XN_USAGE_DEFAULT ;
        result = XN0ContextCreateBuffer(m_xnContext,&xnBufferDesc,NULL,&xnBuffer);
        if( XN_SUCCESS == result )
        {
            *pulBuildOutHndl = xnBuffer;
        }
    }
    return  (cl_int)result;
}

/************************************************************************
 * Function: BuildProgram
 * Build data is offloaded to the LRB device using the XN0ContextRunFunction.
 * The native function that is used is NativeBuildProgram
 *
 * Build inputs:
 *  BufferList:
 *      Buf 1: Program binaries (ulProgramBinHndl)
 *      Buf 2: Output buffer    (ulBuildOutHndl)
 *  Misc Data:
 *      - uint32 ulProgId
 *      - Build options
 *
 ************************************************************************/
cl_int XNWrapper::BuildProgram(cl_ulong IN ulProgramBinHndl, cl_ulong IN ulBuildOutHndl, uint32_t ulProgId, const cl_char* IN options)
{
    //
    // For build we use run function
    // 
    XNBUFFER xnBufArray[2] = { ulProgramBinHndl, ulBuildOutHndl};
    XN_BUFFER_LIST xnBufferList;
    xnBufferList.numBufs = 2;
    xnBufferList.pBufArray = xnBufArray;

    // Get a handle to function from the Larrabee binary 
    XNFUNCTION xnFunc;
    XNERROR result = XN0ContextGetLibFunctionHandle(m_xnLibrary, "NativeBuildProgram", &xnFunc);
    if ( XN_SUCCESS == result)
    {
        //
        // Create misc data
        //
        uint16_t miscDataHeaderLen = sizeof(uint32_t);
        uint16_t optionsLen = (NULL == options) ? 0 : strlen((const char*)options);
        uint16_t miscDataLen = miscDataHeaderLen+ optionsLen;
        int8_t* pMiscData = (int8_t*)malloc(miscDataLen);
        memcpy(pMiscData, &ulProgId, miscDataHeaderLen);
        if(NULL != options)
        {
            memcpy(pMiscData+miscDataHeaderLen, options, optionsLen);
        }

        // Init build sequence
        result = XN0ContextRunFunction(xnFunc,xnBufferList, pMiscData, miscDataLen);
        free(pMiscData);
    } 
    return result;
}


/************************************************************************
 * Calculate the size of the required buffer to holds the set of data in ppCmds
 ************************************************************************/
uint32_t XNWrapper::CalculateBufferSize( CommandEntry** ppCmds, cl_uint uiCmdsCount )
{
    uint32_t uiTotalSize = 0;
    uint32_t uiCmdSize = 0;
    for ( cl_uint ui = 0; ui < uiCmdsCount; ui++)
    {
        cl_dev_cmd_desc* pCmd = ppCmds[ui]->pclCmd;
        uiCmdSize = sizeof(lrb_cmd_desc);
        switch (pCmd->type)
        {
        case CL_DEV_CMD_EXEC_TASK:  // Fall through
        case CL_DEV_CMD_EXEC_KERNEL:
            uiCmdSize += sizeof(lrb_cmd_param_kernel);
            uiCmdSize += ((cl_dev_cmd_param_kernel*)(pCmd->params))->arg_size;
            break;
        case CL_DEV_CMD_EXEC_NATIVE:
            assert( 0 && "Native command is not supported yet");
            // TODO: add support for native command.
            break;
        case CL_DEV_CMD_COPY:
            uiCmdSize += sizeof(lrb_cmd_param_copy);
            break;
        default:  // All others are not handled
            assert( 0 && "Command is not supposed to been offloaded");            
            break;
        }
        uiTotalSize += uiCmdSize;
    }
    return uiTotalSize;
}


/************************************************************************
* For each command, if needed, translate XNBUFFERs arguments into the buffer list
************************************************************************/
uint32_t XNWrapper::CalculateXnBufArraySize ( CommandEntry** ppCmds, cl_uint uiCmdsCount)
{
    //
    // 1. Find the size of the buffer array.
    //
    cl_uint uiTotalBuffers = 1; // First cell saves for offloaded data    
    for ( cl_uint ui = 0; ui < uiCmdsCount; ui++)
    {
        cl_dev_cmd_desc* pCmd = ppCmds[ui]->pclCmd;
        switch (pCmd->type)
        {
        case CL_DEV_CMD_EXEC_TASK:  
        case CL_DEV_CMD_EXEC_KERNEL:
            {
                // Get number of buffers
                cl_dev_kernel kernelId = ((cl_dev_cmd_param_kernel*)(pCmd->params))->kernel;            
                cl_uint uiNumBufferInKernel = 0;
                cl_uint uiArgsListSize = 0;
                cl_kernel_argument* pArgsList;
                m_pLrbProgramService->GetKernelParams(kernelId, &pArgsList, (size_t*)&uiArgsListSize);
                // Find number of buffers;
                for ( cl_uint idx=0 ; idx < uiArgsListSize; idx++ )
                {
                    cl_kernel_arg_type argType = pArgsList[idx].type;
                    if( 
                        CL_KRNL_ARG_PTR_GLOBAL == argType ||
                        CL_KRNL_ARG_PTR_CONST  == argType ||
                        CL_KRNL_ARG_PTR_IMG_2D == argType ||
                        CL_KRNL_ARG_PTR_IMG_3D == argType
                        )
                    {
                        // It's a memory object
                        uiTotalBuffers++;
                    }
                }
                break;
            }
        case CL_DEV_CMD_EXEC_NATIVE:
            assert( 0 && "Native command is not supported yet");
            // TODO: add support for native command.
            break;
        case CL_DEV_CMD_COPY:
            // There always 2 buffers
            uiTotalBuffers += 2;
            break;
        default:  // All others are not handled
            assert( 0 && "Command is not supposed to been offloaded");            
            break;
        }
    }
    return uiTotalBuffers;
}

/************************************************************************
 * Both pCmdBuffer pXnBufList are pre-allocated memory with enough data
 * to hold the content that this function will insert into them.
 ************************************************************************/
cl_int XNWrapper::GenerateOffloadData( CommandEntry** ppCmds, cl_uint uiCmdsCount, uint8_t* pCmdBuffer, XNBUFFER* pXnBufArray )
{
    uint8_t*  pCurrentPos = pCmdBuffer;
    XNBUFFER* pCurrentBuf = pXnBufArray;

    uint32_t uiCmdSize = 0;
    uint32_t uiNumCmdBufs = 0;    

    for ( cl_uint ui = 0; ui < uiCmdsCount; ui++)
    {
        CommandEntry* pCurrentCmd = ppCmds[ui];
        switch (pCurrentCmd->pclCmd->type)
        {
        case CL_DEV_CMD_EXEC_TASK:  
        case CL_DEV_CMD_EXEC_KERNEL:
            FillKernelCmdData(pCurrentPos, pCurrentBuf, pCurrentCmd, &uiCmdSize, &uiNumCmdBufs);
            break;
        case CL_DEV_CMD_EXEC_NATIVE:
            assert( 0 && "Native command is not supported yet");
            // TODO: add support for native command.
            break;
        case CL_DEV_CMD_COPY:
            FillCopyCmdData(pCurrentPos, pCurrentBuf, pCurrentCmd, &uiCmdSize, &uiNumCmdBufs);
            break;
        default:  // All others are not handled
            assert( 0 && "Command is not supposed to been offloaded");            
            uiCmdSize = 0;
            uiNumCmdBufs = 0;
            break;
        }
        pCurrentPos += uiCmdSize;
        pCurrentBuf += uiNumCmdBufs;
    }
    return CL_SUCCESS;
}

/************************************************************************
 * 
 ************************************************************************/
cl_int XNWrapper::FillKernelCmdData( uint8_t* pCmdBuffer, XNBUFFER* pXnBufArray, CommandEntry* pCurrentCmd, uint32_t* puiCmdSize, uint32_t* puiNumBufs)
{
    uint32_t uiTotalCmdSize = 0;
    uint32_t uiCmdSize = 0;
    cl_uint  uiTotalBuffers = 0; // First cell saves for offloaded data    


    lrb_cmd_desc lrbCmdDesc;
    cl_dev_cmd_desc* pCmd = pCurrentCmd->pclCmd;
    lrbCmdDesc.uiCmdAddr =  (uint32_t)pCurrentCmd;
    lrbCmdDesc.bProfiling = ((pCmd->profiling) ? 0x1 : 0x0);
    lrbCmdDesc.pParams = 0; // initialize to 0, will be set on host
    lrbCmdDesc.uiCmdType = LRB_CMD_KERNEL;
    lrbCmdDesc.uiParamSize = sizeof(lrb_cmd_param_kernel);
    uiCmdSize = sizeof(lrb_cmd_desc);
    memcpy(pCmdBuffer, &lrbCmdDesc, uiCmdSize);
    // Use pointer to the allocated buffer to enable setting of command size and num buffes at the end.
    lrb_cmd_desc* AllocatedlrbCmdDesc = (lrb_cmd_desc*)pCmdBuffer; 
    pCmdBuffer += uiCmdSize;
    uiTotalCmdSize = uiCmdSize;

    // Copy params
    lrb_cmd_param_kernel lrbKernelParams;
    cl_dev_cmd_param_kernel* pclParamKernel = (cl_dev_cmd_param_kernel*)(pCmd->params);
    lrbKernelParams.uiKernelAddr = (uint32_t)pclParamKernel->kernel; // TODO: Check that kernel is an addr.
    lrbKernelParams.uiWorkDim = pclParamKernel->work_dim;
    for( int i=0 ; i<3; i++)
    {
        lrbKernelParams.uiLclWrkSize[i] = pclParamKernel->lcl_wrk_size[i];
        lrbKernelParams.uiGlbWrkOffs[i] = pclParamKernel->glb_wrk_offs[i];
        lrbKernelParams.uiGlbWrkSize[i] = pclParamKernel->glb_wrk_size[i];
    }
    // Set args size below, but we know the size, so let's move on and return later to write the lrbKernelParams 
    uint8_t* pLrbKernelParams = pCmdBuffer;
    uiCmdSize = sizeof(lrb_cmd_param_kernel);
    pCmdBuffer += uiCmdSize;
    uiTotalCmdSize += uiCmdSize;

    // Copy args value, if arg is mem object pointer, use sizeof (void*) to save place for pointer.
    // 
    uiCmdSize = 0;
    cl_uint uiArgsListSize = 0;
    cl_kernel_argument* pArgsList;
    uint8_t* pHostArgs = (uint8_t*)pclParamKernel->arg_values;    
    m_pLrbProgramService->GetKernelParams((cl_dev_kernel)lrbKernelParams.uiKernelAddr, &pArgsList, (size_t*)&uiArgsListSize);
    // Copy each parameter by its own
    for ( cl_uint idx=0; idx < uiArgsListSize; idx++ )
    {
        cl_kernel_arg_type argType = pArgsList[idx].type;
        if( 
            CL_KRNL_ARG_PTR_GLOBAL == argType ||
            CL_KRNL_ARG_PTR_CONST  == argType ||
            CL_KRNL_ARG_PTR_IMG_2D == argType ||
            CL_KRNL_ARG_PTR_IMG_3D == argType
            )
        {
            // It's a memory object
            // 1. Set cmd buffer
            uint32_t uiArgSize = sizeof(void*);
            cl_dev_mem* pArg = (cl_dev_mem*)pHostArgs;
            memset(pCmdBuffer, 0, uiArgSize);
            pHostArgs  += sizeof(cl_dev_mem);
            pCmdBuffer += uiArgSize;
            uiCmdSize  += uiArgSize;
            // 2. Set XNBUFFER list
            pXnBufArray[uiTotalBuffers] = (XNBUFFER)(m_pMemoryManager->GetMemObjectHndl(*pArg));
            uiTotalBuffers++;
        }
        else
        {
            // Just copy
            uint32_t uiArgSize = pArgsList[idx].size_in_bytes;
            memcpy(pCmdBuffer, pHostArgs, uiArgSize);
            pHostArgs  += uiArgSize;
            pCmdBuffer += uiArgSize;
            uiCmdSize  += uiArgSize;
        }
    }
    
    // Don't forget, now we need to write params with the new arg that we have
    lrbKernelParams.uiArgsSize = uiCmdSize;
    lrbKernelParams.pArgsValues = 0;
    memcpy( pLrbKernelParams, &lrbKernelParams, sizeof(lrb_cmd_param_kernel));
    

    uiTotalCmdSize += uiCmdSize;

    // Set values
    *puiCmdSize = uiTotalCmdSize;
    *puiNumBufs = uiTotalBuffers;

    AllocatedlrbCmdDesc->uiNumMemObjUsed = uiTotalBuffers;
    AllocatedlrbCmdDesc->uiCmdSize = uiTotalCmdSize;

    // Before returning, need to use XN0ContextAddBuffer to map the buffer on the host for use
    pCurrentCmd->pMappedBufHndls = (void*)(new XNBUFFER[uiTotalBuffers]);
    pCurrentCmd->uiNumOfBufHndls = uiTotalBuffers;
    memcpy(pCurrentCmd->pMappedBufHndls, pXnBufArray, uiTotalBuffers*sizeof(XNBUFFER));

    // XN0ContextAddBuffer - map on host before RunFunction
    for (cl_uint idx = 0; idx < uiTotalBuffers; idx++ )
    {
        XN0ContextAddBuffer(m_xnContext, pXnBufArray[idx]);
    }
    return CL_SUCCESS;
}

/************************************************************************
 * 
 ************************************************************************/
cl_int XNWrapper::FillCopyCmdData( uint8_t* pCmdBuffer, XNBUFFER* pXnBufArray, CommandEntry* pCurrentCmd, uint32_t* puiCmdSize, uint32_t* puiNumBufs)
{
    uint32_t uiTotalCmdSize = 0;
    uint32_t uiCmdSize = 0;

    lrb_cmd_desc lrbCmdDesc;
    cl_dev_cmd_desc* pCmd = pCurrentCmd->pclCmd;
    lrbCmdDesc.uiCmdAddr =  (uint32_t)pCurrentCmd;
    lrbCmdDesc.bProfiling = ((pCmd->profiling) ? 0x1 : 0x0);
    lrbCmdDesc.pParams = 0; // initialize to 0, will be set on host
    lrbCmdDesc.uiCmdType = LRB_CMD_CPY;
    lrbCmdDesc.uiParamSize = sizeof(lrb_cmd_param_copy);
    uiCmdSize = sizeof(lrb_cmd_desc);
    memcpy(pCmdBuffer, &lrbCmdDesc, uiCmdSize);
    lrb_cmd_desc* AllocatedlrbCmdDesc = (lrb_cmd_desc*)pCmdBuffer;
    pCmdBuffer += uiCmdSize;
    uiTotalCmdSize = uiCmdSize;

    // Copy params
    lrb_cmd_param_copy lrbCopyParams;
    cl_dev_cmd_param_copy* pclCopyParam = (cl_dev_cmd_param_copy*)(pCmd->params);

    lrbCopyParams.uiDstDimCount = pclCopyParam->dst_dim_count;
    lrbCopyParams.uiSrcDimCount = pclCopyParam->src_dim_count;
    // TODO: move XNBUFFERS to buffers list.
    lrbCopyParams.pDstMemObj   = pclCopyParam->dstMemObj->objHandle;
    lrbCopyParams.pSrcMemObj   = pclCopyParam->srcMemObj->objHandle;
    for( int i=0 ; i<3; i++)
    {
        lrbCopyParams.uiDstOrigins[i] = pclCopyParam->dst_origin[i];
        lrbCopyParams.uiSrcOrigins[i] = pclCopyParam->src_origin[i];
        lrbCopyParams.uiRegion[i]     = pclCopyParam->region[i];
    }

    uiCmdSize = sizeof(lrb_cmd_param_copy);
    memcpy(pCmdBuffer, &lrbCopyParams, uiCmdSize);
    pCmdBuffer += uiCmdSize;
    uiTotalCmdSize += uiCmdSize;

    // Set values
    *puiCmdSize = uiTotalCmdSize;
    *puiNumBufs = 2;

    uint16_t uiNumBufferInCmd = 2;
    AllocatedlrbCmdDesc->uiNumMemObjUsed = uiNumBufferInCmd;
    AllocatedlrbCmdDesc->uiCmdSize = uiTotalCmdSize;

    // Before returning, need to use XN0ContextAddBuffer to map the buffer on the host for use
    pCurrentCmd->pMappedBufHndls = (void*)(new XNBUFFER[uiNumBufferInCmd]);
    pCurrentCmd->uiNumOfBufHndls = uiNumBufferInCmd;
    memcpy(pCurrentCmd->pMappedBufHndls, pXnBufArray, uiNumBufferInCmd*sizeof(XNBUFFER));

    // XN0ContextAddBuffer - map on host before RunFunction
    XN0ContextAddBuffer(m_xnContext, pXnBufArray[0]);
    XN0ContextAddBuffer(m_xnContext, pXnBufArray[1]);

    return CL_SUCCESS;
}

