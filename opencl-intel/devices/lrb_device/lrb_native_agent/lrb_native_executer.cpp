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
//  lrb_native_executer.cpp
///////////////////////////////////////////////////////////
#include "lrb_native_executer.h"

#include <lrb/XN0_lrb.h>


using namespace Intel::OpenCL::LRBAgent;


/************************************************************************
 * 
 ************************************************************************/
LrbNativeExecuter::LrbNativeExecuter( XNCOMMUNICATOR* pXnCommunicator ):
    m_pXnCommunicator(pXnCommunicator)
{
    // Init XNTasks
    XN0TaskInit(XN0SysGetHardwareThreadCount(), NULL, 0);

}

/************************************************************************
 * 
 ************************************************************************/
LrbNativeExecuter::~LrbNativeExecuter()
{
    XN0TaskShutdown();
}

/************************************************************************
 * Enqueues Kernel for execution. 
 * Each command is mapped to 3 dependent tasks.
 ************************************************************************/
XNERROR LrbNativeExecuter::EnqueueKernelCmd( lrb_cmd_desc* pCmdDesc, XNBUFFER* pXNBuffers, void** INOUT ppPrevNewCommandHndl, bool bIsInOrder)
{
    // Prepare execution
    LrbKernelTask* pTask = new LrbKernelTask;
    pTask->m_pXnCommunicator = m_pXnCommunicator;
    pTask->m_uiCmdAddr=pCmdDesc->uiCmdAddr;

    lrb_cmd_param_kernel* pParams =  (lrb_cmd_param_kernel*)((uint8_t*)pCmdDesc + sizeof(lrb_cmd_desc));
    CLBackendKernel* pKernel = (CLBackendKernel*)pParams->uiKernelAddr;
    pParams->pArgsValues = (uint32_t*)((uint8_t*)pParams + sizeof(lrb_cmd_param_kernel));
    FillMemObjectPointers( pKernel, (uint8_t*)(pParams->pArgsValues), pXNBuffers );
   
    pKernel->CreateExecutable(pParams->pArgsValues, pParams->uiArgsSize, pParams->uiWorkDim, pParams->uiGlbWrkSize,
                                pParams->uiLclWrkSize, &pTask->m_pExecutable);

    // Init values
    pTask->m_uiGlbWrkSize[2]  = 1;
    pTask->m_uiGlbWrkSize[1]  = 1;
    pTask->m_uiGlbWrkSize[0]  = 1;

    pTask->m_uiWorkDim = pParams->uiWorkDim;
    switch(pTask->m_uiWorkDim)
    {
    case 3:     pTask->m_uiGlbWrkSize[2] = pParams->uiGlbWrkSize[2];
    case 2:     pTask->m_uiGlbWrkSize[1] = pParams->uiGlbWrkSize[1];
    case 1:     pTask->m_uiGlbWrkSize[0] = pParams->uiGlbWrkSize[0];
    }

    // Execute Task
    return pTask->Execute( ppPrevNewCommandHndl, bIsInOrder );
}

/************************************************************************
 * 
 ************************************************************************/
XNERROR LrbNativeExecuter::EnqueueCopyCmd( lrb_cmd_desc* pCmdDesc, XNBUFFER* pXNBuffers, void** INOUT ppPrevNewCommandHndl, bool bIsInOrder)
{
    return XN_SUCCESS;
}


/************************************************************************
 * 
 ************************************************************************/
void LrbNativeExecuter::FillMemObjectPointers( CLBackendKernel* pKernel, uint8_t* pKernelArgs, XNBUFFER* pXNBuffers )
{
    size_t szNumParams = 0;
    pKernel->GetKernelParams( NULL, &szNumParams );
    // Create argument list buffer
    cl_kernel_argument* KernelPrototype = new cl_kernel_argument[szNumParams];
    pKernel->GetKernelParams(KernelPrototype, &szNumParams);

    for( uint32_t ui=0; ui<szNumParams; ui++)
    {
        cl_kernel_arg_type argType = KernelPrototype[ui].type;
        if( 
            CL_KRNL_ARG_PTR_GLOBAL == argType ||
            CL_KRNL_ARG_PTR_CONST  == argType ||
            CL_KRNL_ARG_PTR_IMG_2D == argType ||
            CL_KRNL_ARG_PTR_IMG_3D == argType
            )
        {
            // Set buffer pointer.
            XN_FORMAT_DESC xnBinFormatDesc;
            XN0BufferGetInternalFormat(*pXNBuffers,&xnBinFormatDesc);
            memcpy(pKernelArgs, &(xnBinFormatDesc.pBufData), sizeof(void*));
            pKernelArgs += sizeof(void*);
            pXNBuffers++;
        } 
        else
        {
            // Skip
            pKernelArgs += KernelPrototype[ui].size_in_bytes;
        }
    }
    delete[] KernelPrototype;
}

/************************************************************************
 * 
 ************************************************************************/
XNERROR LrbKernelTask::Execute( void** INOUT ppPrevNewCommandHndl, bool bIsInOrder )
{
    XNERROR result = XN_SUCCESS;
    //
    // 3 steps of tasks
    //
    XNTASK  xnBeginTask, xnExecuteTask, xnCompleteTask;
    XNTASK* pxnPrevTask = (XNTASK*)ppPrevNewCommandHndl;

    if ( *pxnPrevTask == 0 ) 
    {
        // No dependency
        result = XN0TaskCreate(XN_TASK_PRIORITY_MEDIUM,(XNTaskFunction)TaskBegin, this, NULL, 0, &xnBeginTask);
    }
    else
    {
        result = XN0TaskCreate(XN_TASK_PRIORITY_MEDIUM,(XNTaskFunction)TaskBegin, this, pxnPrevTask, 1, &xnBeginTask);
    }

    // Create task set in the size of the task group
    uint32_t uiTaskSetSize = m_uiGlbWrkSize[0] * m_uiGlbWrkSize[1] * m_uiGlbWrkSize[2];
    result = XN0TaskCreateSet( uiTaskSetSize, XN_TASK_PRIORITY_MEDIUM, (XNTaskSetFunction)TaskExecute, this, &xnBeginTask, 1, &xnExecuteTask);

    // enqueue completed task 
    result = XN0TaskCreate(XN_TASK_PRIORITY_MEDIUM,(XNTaskFunction)TaskCompleted, this, &xnExecuteTask, 1, &xnCompleteTask);

    // Free local references.
    XN0TaskDecRef(xnBeginTask, NULL);
    XN0TaskDecRef(xnExecuteTask, NULL);

    if( *pxnPrevTask != 0 )
    {
        // Free it...
        XN0TaskDecRef(*pxnPrevTask, NULL);
    }
    if( ! bIsInOrder )
    {
        XN0TaskDecRef(xnCompleteTask, NULL);
    }
    else
    {
        // needed by next command
        *ppPrevNewCommandHndl = (void*)xnCompleteTask;
    }
    return result;
}


/************************************************************************
 * 
 ************************************************************************/
void LrbKernelTask::TaskBegin( void *pLrbKernelTask)
{
    LrbKernelTask* pThis = (LrbKernelTask*)pLrbKernelTask;
    CommandDoneMessage message;
    message.cmdHndl = pThis->m_uiCmdAddr;
    message.status = STS_RUNNING;
    XNERROR result = XN0MessageSend(*(pThis->m_pXnCommunicator),&message,sizeof(message));   
    if( XN_SUCCESS != result )
    {
        // TODO - handle errors
        return;
    }
}

/************************************************************************
 * 
 ************************************************************************/
void LrbKernelTask::TaskCompleted( void *pLrbKernelTask)
{
    // At the end - send status STS_COMPLETE
    LrbKernelTask* pThis = (LrbKernelTask*)pLrbKernelTask;
    CommandDoneMessage message;
    message.cmdHndl = pThis->m_uiCmdAddr;
    message.status = STS_COMPLETE;
    XN0MessageSend(*(pThis->m_pXnCommunicator),&message,sizeof(message));   
    delete pThis;
    // TODO - handle errors from XN0MessageSend
}

/************************************************************************
 * The executing kernel per task
 ************************************************************************/
void LrbKernelTask::TaskExecute( void *pLrbKernelTask, const uint32_t uiTaskIndex, const uint32_t uiTaskSetSize)
{
    LrbKernelTask* pThis = (LrbKernelTask*)pLrbKernelTask;
    // TODO: Extract the global id 3 dim value from the task index.
    // Current status:  (1) The tasks handles only buffers, means 1 dimension. GlobalId[0] =  uiTaskIndex;
    //                  (2) This implementation do not use local id, it is always 0,0,0. 
    // TODO: Add support to localIds (WG size > 1).
    size_t pszGlobalId[] = { 0, 0, 0};
    size_t pszLocalId[] =  { 0, 0, 0};
    pszGlobalId[0] = uiTaskIndex;
    pThis->m_pExecutable->Execute(NULL, 0, pszGlobalId, pszLocalId, NULL);
    // TODO - handle errors from Execute
}
