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

#pragma once

#include "sync_graphics_api_objects.h"
#include "d3d9_resource.h"

namespace Intel { namespace OpenCL { namespace Framework
{
/**
 * @class   SyncD3DResources
 *
 * @brief   Synchronize Direct3D 9 resources. 
 *
 * @param RESOURCE_TYPE the super-type of all Direct 3D resources that can be created by this context
 * @param DEV_TYPE the type of the Direct 3D device
 *
 * @author  Aharon
 * @date    7/12/2011
 *
 * @sa  RuntimeCommand
 */

template<typename RESOURCE_TYPE, typename DEV_TYPE>
class SyncD3DResources : public SyncGraphicsApiObjects
{
   
public:

    /**
     * @brief   Constructor.
     *
     * @author  Aharon
     * @date    7/12/2011
     *
     * @param   cmdQueue
     * @param   pMemObjects             the D3D9Resouce objects to synchronize.
     * @param   szNumMemObjects         the number of D3D9Resouce objects to synchronize.
     * @param   cmdType                 the type of the command.
     * @param   d3d9Definitions          a ID3DSharingDefinitions of the version of the extension used
     */

    SyncD3DResources(SharedPtr<IOclCommandQueueBase> cmdQueue, SharedPtr<GraphicsApiMemoryObject>* const pMemObjects, size_t szNumMemObjects, cl_command_type cmdType,
        const ID3DSharingDefinitions& d3d9Definitions) :
    SyncGraphicsApiObjects(cmdType, szNumMemObjects, cmdQueue, pMemObjects, cmdType == d3d9Definitions.GetCommandAcquireDevice()),
        m_d3dDefinitions(d3d9Definitions)
    { }

    // overridden methods:

    virtual const char* GetCommandName() const;

    virtual cl_err_code Execute();

private:

    const ID3DSharingDefinitions& m_d3dDefinitions;

};

template<typename RESOURCE_TYPE, typename DEV_TYPE>
const char* SyncD3DResources<RESOURCE_TYPE, DEV_TYPE>::GetCommandName() const
{
    if (m_d3dDefinitions.GetCommandAcquireDevice() == GetCommandType())
    {
        return "CL_COMMAND_ACQUIRE_DX9_OBJECTS";
    }
    assert(m_d3dDefinitions.GetCommandReleaseDevice() == GetCommandType());
    return "CL_COMMAND_RELEASE_DX9_OBJECTS";
}

template<typename RESOURCE_TYPE, typename DEV_TYPE>
cl_err_code SyncD3DResources<RESOURCE_TYPE, DEV_TYPE>::Execute()
{
    if (m_d3dDefinitions.GetCommandAcquireDevice() == GetCommandType())
    {
        for (unsigned int i = 0; i < GetNumMemObjs(); i++)
        {
            if (!dynamic_cast<D3DResource<RESOURCE_TYPE, DEV_TYPE>&>(GetMemoryObject(i)).AcquireD3D())
			{
				m_returnCode = CL_MAP_FAILURE;
			}
        }
    }
    else
    {
        for (unsigned int i = 0; i < GetNumMemObjs(); i++)
        {
            dynamic_cast<D3DResource<RESOURCE_TYPE, DEV_TYPE>&>(GetMemoryObject(i)).ReleaseD3D();
        }
    }
    return RuntimeCommand::Execute();
}

}}}
