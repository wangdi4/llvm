// Copyright (c) 2006-2007 Intel Corporation
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

namespace Intel { namespace OpenCL { namespace Framework
{
    /**
     * @fn  SyncGraphicsApiObjects::SyncGraphicsApiObjects(cl_command_type cmdType,
     *      unsigned int uiMemObjNum, IOclCommandQueueBase* cmdQueue,
     *      ocl_entry_points * pOclEntryPoints, GraphicsApiMemoryObject** pMemObjects)
     */

    SyncGraphicsApiObjects::SyncGraphicsApiObjects(cl_command_type cmdType, size_t uiMemObjNum,
        IOclCommandQueueBase* cmdQueue, ocl_entry_points * pOclEntryPoints,
        GraphicsApiMemoryObject** pMemObjects,bool bIsAcquireCmd) :
      RuntimeCommand(cmdQueue, pOclEntryPoints), m_bIsAcquireCmd(bIsAcquireCmd), m_cmdType(cmdType), m_uiMemObjNum(uiMemObjNum)
    {
        m_pMemObjects = new GraphicsApiMemoryObject*[uiMemObjNum];
        MEMCPY_S(m_pMemObjects, sizeof(GraphicsApiMemoryObject*)*uiMemObjNum, pMemObjects, sizeof(GraphicsApiMemoryObject*)*uiMemObjNum);
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
        for (unsigned int i=0; i<m_uiMemObjNum; ++i)
        {
            if (m_bIsAcquireCmd)
            {
                m_pMemObjects[i]->SetAcquireCmdEvent(&m_Event);
            }
            m_pMemObjects[i]->AddPendency(this);            
        }
        return CL_SUCCESS;
    }

    /**
     * @fn  cl_err_code SyncGraphicsApiObjects::CommandDone()
     */

    cl_err_code SyncGraphicsApiObjects::CommandDone()
    {
        for (unsigned int i = 0; i < m_uiMemObjNum; i++)
        {
			if (!m_bIsAcquireCmd || GetReturnCode() != CL_SUCCESS)  // if it m_bIsAcquireCmd and we have a failure, we need to undo what we did in Init
			{
                m_pMemObjects[i]->SetAcquireCmdEvent(NULL);
			}
            m_pMemObjects[i]->RemovePendency(this);
        }
        return CL_SUCCESS;
    }

}}}
