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
//  lrb_native_agent.h
//  Created on:      June-2009
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////
#if !defined(__LRB_NATIVE_AGENT_H__)
#define __LRB_NATIVE_AGENT_H__

#include "lrb/XN0_lrb.h"

namespace Intel { namespace OpenCL { namespace LRBAgent {
    
    class LrbNativeExecuter;

    /**********************************************************************************************
     * Class name:    LrbNativeAgent
     *
     * Description:    
     *      The class represent the LRB OpenCL device agent on the native executable.
     *      The object holds the state that is used for task executable.
     *      
     * Author:        Arnon Peleg
     * Date:          June 2009
    /**********************************************************************************************/    
    class LrbNativeAgent
    {
    public:
        static LrbNativeAgent* GetInstance() 
        {
            if (NULL == m_pLrbInstance)
	        {
		        m_pLrbInstance = new LrbNativeAgent();
	        }
	        return m_pLrbInstance;
        };
        static void Destroy();

        XNERROR     Initialize();
        XNERROR     Release();
        XNERROR     WaitForCompletion();                
        
        XNERROR     ExecuteCommands(XN_BUFFER_LIST xnBufs, void* pMiscData, uint16_t uiMiscDataLen);
        XNERROR     BuildProgram(XN_BUFFER_LIST xnBufs, void* pMiscData, uint16_t uiMiscDataLen);
   
    private:
        LrbNativeAgent();
        ~LrbNativeAgent();

        // Copy functions are not implemented and are private.
        LrbNativeAgent(const LrbNativeAgent&);           // copy constructor
        LrbNativeAgent& operator=(const LrbNativeAgent&);// assignment operator

        static LrbNativeAgent*    m_pLrbInstance;

        // Private members of this agent
        BOOL bIsInitilized;
        LrbNativeExecuter* m_pExecuter;

        // XN objects
        XNCOMMUNICATOR m_xnCommunicator;

        // Private functions Handlers
        void    ReleaseLrbNativeAgent();
        XNERROR InitLrbNativeAgent();
    };

}}};    // Intel::OpenCL::LRBAgent
#endif  // !defined(__LRB_NATIVE_AGENT_H__)
