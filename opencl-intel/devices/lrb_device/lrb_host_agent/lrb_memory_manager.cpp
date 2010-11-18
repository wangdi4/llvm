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
//  lrb_memory_manager.cpp
///////////////////////////////////////////////////////////
#include "lrb_memory_manager.h"
#include "lrb_xn_wrapper.h"
#include "lrb_commands_list.h"
#include "lrb_command_executer.h"

#include <cl_logger.h>
#include <cl_types.h>
#include <cl_table.h>

using namespace Intel::OpenCL::LRBAgent;

/************************************************************************
 * Conversion tables -
 * the table below convert API inputs to enumeration values that can be used
 * by this implementation.
 ************************************************************************/

// clMemObjectTypeTable convert dim number to mem object type.
cl_uint clMemObjectTypeTable[] =
{
    0,                      //  nothing for dim = 0
    CL_MEM_OBJECT_BUFFER,   // Dim = 1
    CL_MEM_OBJECT_IMAGE2D,  // Dim = 2
    CL_MEM_OBJECT_IMAGE3D   // dim = 3
};



/******************************************************************
 *
 ******************************************************************/
LrbMemoryManager::LrbMemoryManager(cl_uint uiDevId, XNWrapper* pXnWrapper, fn_clDevCmdStatusChanged* pclDevCmdStatusChanged):
    m_uiDevId(uiDevId),
    m_pXnWrapper(pXnWrapper),
    m_pCmdExecuter(NULL),
    m_pclDevCmdStatusChanged(pclDevCmdStatusChanged),
    m_pMemTransactor(NULL)

{
    m_pMemoryObjTable = new ClTable;
}

/******************************************************************
 *
 ******************************************************************/
LrbMemoryManager::~LrbMemoryManager()
{
    delete m_pMemoryObjTable;
    m_pMemoryObjTable = NULL;

    if( NULL != m_pMemTransactor)
    {
        delete m_pMemTransactor;
        m_pMemTransactor = NULL;
    }
}

/******************************************************************
 *
 ******************************************************************/
cl_int LrbMemoryManager::CreateMemObject(cl_dev_mem_flags clFlags, const cl_image_format* pclFormat, cl_uint uiDimsCount, const size_t* pszDims, void* pMemBuf, const size_t* pszPitch, cl_dev_host_ptr_flags clHostFlags, cl_dev_mem* pMemObj)
{
    //
    // 1. TODO: Validate arguments
    //

    // 
    // 2. Calculate buffer size
    //
    cl_uint uiImageType = clMemObjectTypeTable[uiDimsCount];
    size_t szBufferSize = CalcBufferSize(uiImageType, pclFormat, pszDims, pszPitch);

    //
    // 4. Create buffer
    //
    void* bufHndl = NULL;
    void* pInData = (( CL_DEV_HOST_PTR_DATA_AVAIL &  clHostFlags ) ? pMemBuf : NULL) ;
    m_pXnWrapper->CreateBuffer(clFlags, szBufferSize, pMemBuf, &bufHndl);

    //
    // 5. Create memory object
    // TODO:  1. Fill description that is needed for image copy...
    //        2. Create/Use host memory if needed according to flags.
    //
    LrbMemObject* pLrbMemObj = new LrbMemObject;
    cl_dev_mem	  hMemObj = new _cl_dev_mem;
    pLrbMemObj->bufHndl = bufHndl;
    pLrbMemObj->pHostBuf = NULL; // This buffer is created on Map/Unmap
    
    hMemObj->objHandle = (void*)m_pMemoryObjTable->Insert(pLrbMemObj);
    hMemObj->allocId = m_uiDevId;
    pLrbMemObj->hMemObj = hMemObj;
    *pMemObj = hMemObj;

    return CL_DEV_SUCCESS;
}

/************************************************************************
 * Returns the device handle of the memory object
 ************************************************************************/
void* LrbMemoryManager::GetMemObjectHndl( cl_dev_mem memObj )
{
    unsigned long key = (unsigned long)memObj->objHandle;
    LrbMemObject* pObj = (LrbMemObject*)m_pMemoryObjTable->Get(key);
    if ( NULL == pObj )
    {
        return NULL;
    }
    return pObj->bufHndl;
}

/************************************************************************
 * 
 ************************************************************************/
cl_int LrbMemoryManager::DeleteMemObject(cl_dev_mem memObj)
{
    unsigned long key = (unsigned long)memObj->objHandle;
    LrbMemObject* pObj = (LrbMemObject*)m_pMemoryObjTable->Get(key);
    if ( NULL == pObj )
    {
        return CL_DEV_INVALID_VALUE;
    }
    // Delete resources
    m_pXnWrapper->DeleteBuffer(pObj->bufHndl);
    delete pObj->hMemObj;
    delete pObj;
    m_pMemoryObjTable->Remove(key);

    return CL_SUCCESS;
}


/************************************************************************
 * Returns the size in bytes of memory needed to store a memory object as defined 
 * by the input parameters.
 ************************************************************************/
size_t LrbMemoryManager::CalcBufferSize(cl_uint uiImageType, const cl_image_format* pclFormat, const size_t* pszDims, const size_t* pszPitch)
{
    size_t szBufferSize = 0;
    switch (uiImageType)
    {
    case CL_MEM_OBJECT_BUFFER:
        szBufferSize = pszDims[0];
        break;
    case CL_MEM_OBJECT_IMAGE2D:
    case CL_MEM_OBJECT_IMAGE3D:
    default:
        assert( 0 && "Only BUFFER obj is supported");
    	break;
    }
    return szBufferSize;
}

/************************************************************************
 * This command insert a Read/Write command that is ready for processing
 * After adding a memory transaction, the user is expected to signal the 
 * memory transactor to process this command in its own thread
 ************************************************************************/
cl_int LrbMemoryManager::QueueMemoryTransaction( CommandEntry* pBlockingCmd )
{
    return m_pMemTransactor->AddTransaction(pBlockingCmd);
}

/************************************************************************
 * The function does the actual Read/Write operation on a memory object
 * according to the data in the command entry.
 *
 ************************************************************************/
cl_int LrbMemoryManager::ProcessMemoryTransaction( CommandEntry* pCommand )
{
    cl_dev_cmd_desc* pclCmd = pCommand->pclCmd;
    //
    // 1. Process/Handle command. First notify running
    //
    // TODO: (1) add profiling handling.
    m_pclDevCmdStatusChanged(pclCmd->id, pclCmd->data, CL_RUNNING, CL_SUCCESS, 0 /* time=0*/ );
    cl_dev_cmd_type cmdType = pclCmd->type;
    switch (cmdType)
    {
    case CL_DEV_CMD_READ:
        ReadMemoryCommand(pclCmd);
        break;
    case CL_DEV_CMD_WRITE:
        WriteMemoryCommand(pclCmd);
        break;
    case CL_DEV_CMD_MAP:
        printf("==== Map command is not implemented yet... \n");
        break;
    case CL_DEV_CMD_UNMAP:
        printf("==== Un-Map command is not implemented yet... \n");        
        break;
    default:
        // All others are errors
        assert(0);
    }
    //
    // 2, Notify done to the runtime and the executer
    //
    // TODO: (1) Add error result to command (2) Add profiling handling.
    m_pclDevCmdStatusChanged(pclCmd->id, pclCmd->data, CL_COMPLETE, CL_SUCCESS, 0 /* time=0*/ );
    pCommand->eState = CMD_COMPELETED;
    return m_pCmdExecuter->SignalCommandDone(pCommand->pCmdList, true);
}

/************************************************************************
 * This function create transactor and starts its thread
 ************************************************************************/
cl_int LrbMemoryManager::StartMemoryTransactor()
{
    if( NULL == m_pMemTransactor)
    {
        m_pMemTransactor = new LrbMemTransactor(this);
    }
    return m_pMemTransactor->Start();
}

/************************************************************************
 * 
 ************************************************************************/
cl_int LrbMemoryManager::ReadMemoryCommand(cl_dev_cmd_desc* pclCmd)
{
    cl_int result = CL_SUCCESS;
    cl_dev_cmd_param_rw* pCmdParams = (cl_dev_cmd_param_rw*)pclCmd->params;
    unsigned long key = (unsigned long)pCmdParams->memObj->objHandle;
    LrbMemObject* pObj = (LrbMemObject*)m_pMemoryObjTable->Get(key);

    // Map buffer for read
    char* pOutData;
    result = m_pXnWrapper->MapBuffer(pObj->bufHndl, CL_DEV_MEM_READ, (void**)&pOutData);
    // Actual read
    if( CL_SUCCESS == result)
    {
        // Safe to read data to host...
        switch(pCmdParams->dim_count)
        {
        case 1: // Buffer
            memcpy(pCmdParams->ptr, pOutData+pCmdParams->origin[0], pCmdParams->region[0]);
            break;
        case 2: // 2D image
        case 3: // 3D image
        default:
            assert ( 0 && "2D/3D images are not supported");
        }
        // Unmap buffer - allow LRB to continue using it.
        result = m_pXnWrapper->UnmapBuffer(pObj->bufHndl);
    }
    return result;
}

/************************************************************************
 * 
 ************************************************************************/
cl_int LrbMemoryManager::WriteMemoryCommand(cl_dev_cmd_desc* pclCmd)
{
    cl_int result = CL_SUCCESS;
    cl_dev_cmd_param_rw* pCmdParams = (cl_dev_cmd_param_rw*)pclCmd->params;
    unsigned long key = (unsigned long)pCmdParams->memObj->objHandle;
    LrbMemObject* pObj = (LrbMemObject*)m_pMemoryObjTable->Get(key);

    // Map buffer for write
    char* pInData;
    result = m_pXnWrapper->MapBuffer(pObj->bufHndl, CL_DEV_MEM_WRITE, (void**)&pInData);
    // Actual write
    if( CL_SUCCESS == result)
    {
        // Safe to read data to host...
        switch(pCmdParams->dim_count)
        {
        case 1: // Buffer
            memcpy(pInData+pCmdParams->origin[0], pCmdParams->ptr, pCmdParams->region[0]);
            break;
        case 2: // 2D image
        case 3: // 3D image
        default:
            assert ( 0 && "2D/3D images are not supported");
        }
        // Unmap buffer - allow LRB to continue using it.
        result = m_pXnWrapper->UnmapBuffer(pObj->bufHndl);
    }
    return result;

}
