// INTEL CONFIDENTIAL
//
// Copyright 2006-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

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
