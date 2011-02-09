///////////////////////////////////////////////////////////
// Copyright (c) 2008-2009 Intel Corporation
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
//  lrb_native_executer.h
//  Created on:      Aug-2009
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////
#if !defined(__LRB_NATIVE_EXECUTER_H__)
#define __LRB_NATIVE_EXECUTER_H__

#include "lrb/XN0_lrb.h"
#include "cl_backend_api.h"

namespace Intel { namespace OpenCL { namespace LRBAgent {


    /************************************************************************
     *  Class name:    LrbKernelTask
     *   
     *  Description:    
     *      This object is used in 3 different dependent tasks that are used to implement 
     *      an OCL kernel execution on LRB Native, using XNTask API.
     *      We use 3 different tasks to prevent sync points inside the execution phase.
     *      Leaving the sync overhead to the XNTask platform. We suffer from overhead of 2
     *      tasks, but in very large task set this overhead is minimal.
     *             
     *  Author:        Arnon Peleg
     *  Date:          Sep. 2009
     ************************************************************************/
    class LrbKernelTask
    {
    public:
        LrbKernelTask(): m_pExecutable(NULL)    {};
        ~LrbKernelTask() { if (NULL != m_pExecutable) delete m_pExecutable; };

        XNERROR Execute( void** INOUT ppPrevNewCommandHndl, bool bIsInOrder );

        CLBackendExecutable*    m_pExecutable;            
        XNCOMMUNICATOR*         m_pXnCommunicator;
        uint32_t                m_uiCmdAddr;
        uint8_t                 m_uiWorkDim;
        uint32_t                m_uiGlbWrkSize[3];

        // 
        // Execution functions for each task
        //
        static void TaskBegin    ( void *pLrbKernelTask);
        static void TaskCompleted( void *pLrbKernelTask);
        static void	TaskExecute  ( void *pLrbKernelTask, const uint32_t uiTaskIndex, const uint32_t uiTaskSetSize);
    };

    /************************************************************************
    * 
    ************************************************************************/
    class CopyTask
    {
    public:
        CopyTask();
        ~CopyTask();

        XNERROR Execute() { return XN_SUCCESS; }

        // Copy Data
        void*       m_pSrcMemObj;      
        void*       m_pDstMemObj;      
        uint8_t     m_uiSrcDimCount;   
        uint8_t     m_uiDstDimCount;   
        uint32_t    m_uiSrcOrigins[3]; 
        uint32_t    m_uiDstOrigins[3]; 
        uint32_t    m_uiRegion[3];

        XNCOMMUNICATOR*         m_pXnCommunicator;
        uint32_t                uiCmdAddr;
    };

    /**********************************************************************************************
     * Class name:    LrbNativeExecuter
     *
     * Description:    
     *      The executer role is to enqueue commands for processing and execute them.
     *      The type of commands are hard coded. Currently there are 2 commands:
     *          1. Kernel - execute kernel using the interface as defined by the back-end complier.
     *          2. Copy buffers commands - copy buffers/images on the device it self.
     *      The execution sequence is in-order.
     *      
     * Author:        Arnon Peleg
     * Date:          Aug. 2009
     **********************************************************************************************/    
    class LrbNativeExecuter
    {
    public:
        LrbNativeExecuter( XNCOMMUNICATOR* pXnCommunicator );
        ~LrbNativeExecuter();

        XNERROR EnqueueKernelCmd( lrb_cmd_desc* pCmdDesc, XNBUFFER* pXNBuffers, void** INOUT ppPrevNewCommandHndl, bool bIsInOrder);
        XNERROR EnqueueCopyCmd  ( lrb_cmd_desc* pCmdDesc, XNBUFFER* pXNBuffers, void** INOUT ppPrevNewCommandHndl, bool bIsInOrder);

    private:

        // Private members of this class


        // XN objects
        XNCOMMUNICATOR* m_pXnCommunicator;

        // Private functions Handlers
        void FillMemObjectPointers( CLBackendKernel* pKernel, uint8_t* pKernelArgs, XNBUFFER* pXNBuffers );

        // Private classes to be used
    };

}}};    // Intel::OpenCL::LRBAgent
#endif  // !defined(__LRB_NATIVE_EXECUTER_H__)
