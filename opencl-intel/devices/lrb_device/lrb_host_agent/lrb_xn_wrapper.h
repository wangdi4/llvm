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
//  lrb_xn_wrapper.h
//  Implementation of the Class XNWrapper
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////
#if !defined(__LRB_XN_WRAPPER_H__)
#define __LRB_XN_WRAPPER_H__

#include "cl_device_api.h"
#include "host/XN0_host.h"
#include <map>


namespace Intel { namespace OpenCL { namespace LRBAgent {

    // Forward declarations
    struct CommandDoneMessage;
    struct CommandEntry;
    class  LrbProgramService;
    class  LrbMemoryManager;

    struct BufferEntry
    {
        XNBUFFER xnMappedBuf;
        XN_MAP   xnMapAccess;
    };

    /**********************************************************************************************
    * Class name:    XNWrapper
    *
    * Description:    
    *      This object is used to wrap all XN API calls. 
    *      A wrapper represents a LRB context and is used to flush commands to device, to read/write 
    *      buffers, etc.
    *
    * Author:        Arnon Peleg
    * Date:          July 2009
    /**********************************************************************************************/    
    class XNWrapper
    {
    public:
        XNWrapper();
        ~XNWrapper();
        cl_int Initialize();
        cl_int Release();
        void   SetPrgramService (LrbProgramService* pProgramService)    { m_pLrbProgramService = pProgramService; };
        void   SetMemoryManager (LrbMemoryManager* pMemoryManager)      { m_pMemoryManager = pMemoryManager; };


        //
        // Calls that are used
        //
        cl_int GetDeviceInfo    ();
        cl_int ExecuteCommands  ( CommandEntry** pCmds, cl_uint uiCmdsCount, bool bIsInOrder = true);
        cl_int UnmapCommand     ( CommandEntry* pPoppedEntry);
        cl_int CreateBuffer     ( cl_dev_mem_flags clAccessFlags, size_t clSize, void* pInData, void** ppBufHndl);
        cl_int DeleteBuffer     ( void* pBufHndl );
        cl_int MapBuffer        ( void* pBufHndl, cl_dev_mem_flags clAccessFlags, void** ppOutData);
        cl_int UnmapBuffer      ( void* pBufHndl );

        //
        // Program build calls
        //
        cl_int LoadProgram(size_t szBinSize, const void* pBinData, uint64_t* pulProgHndl, uint64_t* pulBuildOutHndl);
        cl_int BuildProgram( uint64_t ulProgramBinHndl, uint64_t ulBuildOutHndl, uint32_t ulProgId, const cl_char* options );
        cl_int ReleaseProgeam(uint32_t ulProgId);
        
        // Communicator calls
        cl_int ReceiveMessageSync ( CommandDoneMessage* pReceivedMessage);
        cl_int NotifyProcessDone();

    private:

        // Copy functions are not implemented and are private.
        XNWrapper(const XNWrapper&);           // copy constructor
        XNWrapper& operator=(const XNWrapper&);// assignment operator

        // Private members of this agent
        unsigned int                    m_uiMessageSize;
        std::map<XNBUFFER, BufferEntry> m_mappedBuffer;         // Includes all mapped buffer
        LrbProgramService*              m_pLrbProgramService;   // Used to get kernel info before offloading
        LrbMemoryManager*               m_pMemoryManager;       // Used to get the XNBUFFER value for kernel's parameters.

        // XN objects
        XNCONTEXT               m_xnContext;
        XNLIBRARY               m_xnLibrary;
        XNCOMMUNICATOR          m_xnCommunicator;

        // Private functions Handlers
        cl_int   InitLrbNative          ();
        cl_int   ReleaseLrbNative       ();
        uint32_t CalculateXnBufArraySize( CommandEntry** ppCmds, cl_uint uiCmdsCount);
        uint32_t CalculateBufferSize    ( CommandEntry** ppCmds, cl_uint uiCmdsCount );
        cl_int   GenerateOffloadData    ( CommandEntry** ppCmds, cl_uint uiCmdsCount, uint8_t* pCmdBuffer, XNBUFFER* pXnBufArray );
        cl_int   FillKernelCmdData      ( uint8_t* pCmdBuffer, XNBUFFER* pXnBufArray, CommandEntry* pCurrentCmd, 
                                                uint32_t* puiCmdSize, uint32_t* puiNumBufs);
        cl_int   FillCopyCmdData        ( uint8_t* pCmdBuffer, XNBUFFER* pXnBufArray, CommandEntry* pCurrentCmd, 
                                                uint32_t* puiCmdSize, uint32_t* puiNumBufs);
    };

}}};    // Intel::OpenCL::LRBAgent
#endif  // !defined(__LRB_AGENT_H__)
