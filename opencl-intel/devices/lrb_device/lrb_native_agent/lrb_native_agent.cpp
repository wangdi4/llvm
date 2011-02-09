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
//  lrb_native_agnet.cpp
///////////////////////////////////////////////////////////
#include "lrb_native_agent.h"
#include "lrb_agent_common.h"
#include "cl_backend_api.h"
#include "lrb_native_executer.h"

#include <stdio.h>
#include "cl_types.h"

using namespace Intel::OpenCL::LRBAgent;

// Static members initialization
LrbNativeAgent* LrbNativeAgent::m_pLrbInstance = NULL;

/******************************************************************
 *
 ******************************************************************/
LrbNativeAgent::LrbNativeAgent():
    bIsInitilized(0),
    m_pExecuter(NULL)
{
    printf("LRB: Create Lrb Native Agent\n");
}

/******************************************************************
 *
 ******************************************************************/
LrbNativeAgent::~LrbNativeAgent()
{
    printf("LRB: Free Lrb Native Agent\n");
}

/******************************************************************
 * Destroy static function is used to free the Lrb native instance.
 *
 ******************************************************************/
void LrbNativeAgent::Destroy()
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
XNERROR LrbNativeAgent::Initialize()
{
    printf("LRB: Initialize Lrb Native Agent\n");
    
    // 
    // The code below initialize Native SDK executable.
    //
    return InitLrbNativeAgent();
}

/******************************************************************
 * Release frees object reources including unloading Native code on LRB
 ******************************************************************/
XNERROR LrbNativeAgent::Release()
{
    
    ReleaseLrbNativeAgent();
    if (m_pExecuter != NULL)
    {
        delete m_pExecuter;
    }

    //
    // Clear XN API code
    //
    XN0MessageDestroyCommunicator( m_xnCommunicator );

    printf("LRB: Release Lrb Native Agent\n");
    return XN_SUCCESS;
}

/******************************************************************
 * This function initiate the native executable, load it and querey
 * for constant data that is required by the agent.
 ******************************************************************/
XNERROR LrbNativeAgent::InitLrbNativeAgent()
{
    // Init XN API components.
    XNERROR errCode = XN0MessageCreateCommunicator( COMMAND_STATUS_CHANGE_COMMUNICATOR, XN_MESSAGE_MAX_SIZE, &m_xnCommunicator );
    m_pExecuter = new LrbNativeExecuter(&m_xnCommunicator);
    if( XN_SUCCESS == errCode)
    {
        bIsInitilized = 0x1;
    }
    return errCode;
}

/******************************************************************
 * This function releases all LRB Native executable related objects.
 * 
 ******************************************************************/
void LrbNativeAgent::ReleaseLrbNativeAgent()
{

}

/******************************************************************
 * This function waits for termination message on the device.
 * It acks the host that it is done
 ******************************************************************/
XNERROR LrbNativeAgent::WaitForCompletion()
{
    printf( "LRB: Waiting to receive done notification from the host..\n" );
    CommandDoneMessage doneMessage;
    do 
    {
        // Receive the message from the Host process.
        XN0MessageReceive( m_xnCommunicator, &doneMessage, sizeof(doneMessage), NULL );
    } while ( 0 != doneMessage.cmdHndl);
    
    printf( "LRB: Received notification to end execution\n");
    // Send the results beck to the Host process.
    XNERROR result = XN0MessageSend( m_xnCommunicator, &doneMessage, sizeof(doneMessage) );
    return result;
}

/************************************************************************
 * This function build the program and sends output data to host
 * Input & Output data syntax as as follows:
 *  1. xnBufs[0] - Program binary in cl_prog_container structure
 *  2. xnBufs[1] - place holder for build output. The output stream includes kernels data
 *  3. pMiscData - includes (1) program host Id that need to be used on message send back.
 *                          (2) build options
 *
 *  --------------------------------------
 *  | uint32 uiHostProgId | char* options |
 *  --------------------------------------
 *
 ************************************************************************/
XNERROR LrbNativeAgent::BuildProgram(XN_BUFFER_LIST xnBufs, void* pMiscData, uint16_t uiMiscDataLen)
{
    cl_int result = CL_DEV_SUCCESS;

    // Wait until init is done to build
    // bIsInitilized is set only once on initilization and therefore no need to protect the access
    // Wait to bIsInitilized to change from 0 - 1
    while( 0 == bIsInitilized);
    
    // Get program addr
    uint32_t uiHostProgId;
    size_t sizeOfuint32 = sizeof(uint32_t);
    memcpy(&uiHostProgId, pMiscData, sizeOfuint32);

    //
    // Create program object
    //
    XNBUFFER xnBinaryBuffer = xnBufs.pBufArray[0];
    XN_FORMAT_DESC xnBinFormatDesc;
    XN0BufferGetInternalFormat(xnBinaryBuffer,&xnBinFormatDesc);
    cl_prog_container clProgContainer;
    clProgContainer.container_size = (size_t)xnBinFormatDesc.bufLen;
    clProgContainer.container = xnBinFormatDesc.pBufData;

    CLBackendProgram* pProgram = NULL;
    result = CLBackendProgram::CreateProgram(&clProgContainer, &pProgram);
    if(CL_DEV_FAILED(result))
    {
        return XN_ERROR;
    }

    //
    // Build program
    //
    char pOptions[XN_OPTIONS_MAX_SISE] = { 0 }; 
    if ( uiMiscDataLen > sizeOfuint32)
    {
        // There are options
        memcpy(pOptions,(uint8_t*)pMiscData+sizeOfuint32, uiMiscDataLen-sizeOfuint32 );
    }
    result = pProgram->BuildProgram(pOptions);
    if(CL_DEV_FAILED(result))
    {
        return XN_ERROR;
    }

    //
    // Create build output array data
    //
    XNBUFFER xnOutputBuffer = xnBufs.pBufArray[1];
    XN_FORMAT_DESC xnOutputFormatDesc;
    XN0BufferGetInternalFormat(xnOutputBuffer,&xnOutputFormatDesc);
    
    result = pProgram->GenerateOutputData(xnOutputFormatDesc.pBufData, xnOutputFormatDesc.bufLen);
    if(CL_DEV_FAILED(result))
    {
        return XN_ERROR;
    }

    //
    // Create a return message and send it 
    //
    CommandDoneMessage buildDoneMessage;
    buildDoneMessage.cmdHndl = uiHostProgId;
    buildDoneMessage.status = STS_BUILD_DONE;
    XNERROR xnResult = XN0MessageSend(m_xnCommunicator,&buildDoneMessage,sizeof(buildDoneMessage));
    if( XN_SUCCESS != result )
    {
        // TODO: Clean up buffers/ re-send
        return xnResult;
    }
    return XN_SUCCESS;
}

/************************************************************************
 * 
 ************************************************************************/
XNERROR LrbNativeAgent::ExecuteCommands(XN_BUFFER_LIST xnBufs, void* pMiscData, uint16_t uiMiscDataLen)
{
    XNBUFFER xnBinaryBuffer = xnBufs.pBufArray[0];
    XN_FORMAT_DESC xnBinFormatDesc;
    XN0BufferGetInternalFormat(xnBinaryBuffer,&xnBinFormatDesc);
    
    void* pCommandsBuffer = xnBinFormatDesc.pBufData;
    XNBUFFER* pXNBuffers = xnBufs.pBufArray+1;

    execute_cmds_params* cmdsParams = (execute_cmds_params*)pMiscData;

    //
    // For each command in the pCommandsBuffer do the following:
    //  1. Get command description
    //  2. Process the command with references to the buffers
    //  3. If the cmds are In-Order each command as register as dependent on the previous command.
    bool bIsInOrder = ( 1== cmdsParams->uiIsListInOrder);
    void* pPrevCommandHndl = NULL;
    for ( uint16_t ui = 0; ui < cmdsParams->uiNumCmds; ui++ )
    {
        if( ui == cmdsParams->uiNumCmds-1 )
        {
            // Last command, no dependencies - clear bIsInOrder flag
            bIsInOrder = false;
        }
        lrb_cmd_desc* pCmdDesc = (lrb_cmd_desc*)pCommandsBuffer;
        if( LRB_CMD_KERNEL == pCmdDesc->uiCmdType)
        {
            m_pExecuter->EnqueueKernelCmd(pCmdDesc, pXNBuffers, &pPrevCommandHndl, bIsInOrder);
        }
        else if ( LRB_CMD_CPY == pCmdDesc->uiCmdType )
        {
            m_pExecuter->EnqueueCopyCmd(pCmdDesc, pXNBuffers, &pPrevCommandHndl, bIsInOrder);
        }
        pCommandsBuffer = (void*)((uint8_t*)pCommandsBuffer + pCmdDesc->uiCmdSize);
        pXNBuffers += pCmdDesc->uiNumMemObjUsed;
    }    
    return XN_SUCCESS;
}
