// Copyright (c) 2006-2012 Intel Corporation
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

#include "sync_graphics_api_objects.h"
#include "command_queue.h"

namespace Intel { namespace OpenCL { namespace Framework
{
    /**
     * @fn  SyncGraphicsApiObjects::SyncGraphicsApiObjects(cl_command_type cmdType,
     *      unsigned int uiMemObjNum, SharedPtr<IOclCommandQueueBase> cmdQueue,
     *      ocl_entry_points * pOclEntryPoints, SharedPtr<GraphicsApiMemoryObject>* pMemObjects)
     */

    SyncGraphicsApiObjects::SyncGraphicsApiObjects(cl_command_type cmdType, size_t uiMemObjNum,
        SharedPtr<IOclCommandQueueBase> cmdQueue, SharedPtr<GraphicsApiMemoryObject>* pMemObjects,bool bIsAcquireCmd) :
      RuntimeCommand(cmdQueue), m_bIsAcquireCmd(bIsAcquireCmd), m_cmdType(cmdType), m_uiMemObjNum(uiMemObjNum)
    {
        m_pMemObjects = new SharedPtr<GraphicsApiMemoryObject>[uiMemObjNum];
        for (size_t i = 0; i < uiMemObjNum; i++)
        {
            m_pMemObjects[i] = pMemObjects[i];
        }
    }

    /**
     * @fn  SyncGraphicsApiObjects::~SyncGraphicsApiObjects()
     */

    SyncGraphicsApiObjects::~SyncGraphicsApiObjects()
    {
        delete[] m_pMemObjects;
    }

    /**
     * @fn  cl_err_code SyncGraphicsApiObjects::Init()
     */

    cl_err_code SyncGraphicsApiObjects::Init()
    {
        if (m_bIsAcquireCmd)
        {
            for (unsigned int i=0; i<m_uiMemObjNum; ++i)
            {
                m_pMemObjects[i]->SetAcquireCmdEvent(m_Event);
            }
        }
        return CL_SUCCESS;
    }

    /**
     * @fn  cl_err_code SyncGraphicsApiObjects::CommandDone()
     */

    cl_err_code SyncGraphicsApiObjects::CommandDone()
    {
        if (m_bIsAcquireCmd)
        {
            // if Acquire command succeed - do nothing

            if (GetReturnCode() != CL_SUCCESS)
            {
                // if Acquire command failed, we need to undo what we did in Init
                for (unsigned int i = 0; i < m_uiMemObjNum; i++)
                {
                    m_pMemObjects[i]->ClearAcquireCmdEvent();
                }
            }
        }
        else
        {
            // not m_bIsAcquireCmd e.g. release

            if (GetReturnCode() == CL_SUCCESS)
            {
                for (unsigned int i = 0; i < m_uiMemObjNum; i++)
                {
                    m_pMemObjects[i]->SetAcquireCmdEvent(NULL);
                }
            }
            // if release command failed - do nothing
        }
        return CL_SUCCESS;
    }

}}}
