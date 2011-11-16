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

#pragma once

#include "sync_graphics_api_objects.h"
#include "d3d9_resource.h"

namespace Intel { namespace OpenCL { namespace Framework
{
    /**
     * @class   SyncD3D9Resources
     *
     * @brief   Synchronize Direct3D 9 resources. 
     *
     * @author  Aharon
     * @date    7/12/2011
     *
     * @sa  RuntimeCommand
     */

    class SyncD3D9Resources : public SyncGraphicsApiObjects
    {
       
    public:

        /**
         * @fn  SyncD3D9Resources::SyncD3D9Resources(IOclCommandQueueBase* const cmdQueue,
         *      ocl_entry_points* const pOclEntryPoints, D3D9Resource** const pMemObjects,
         *      size_t szNumMemObjects, cl_command_type cmdType)
         *
         * @brief   Constructor.
         *
         * @author  Aharon
         * @date    7/12/2011
         *
         * @param   cmdQueue
         * @param   pOclEntryPoints
         * @param   pMemObjects             the D3D9Resouce objects to synchronize.
         * @param   szNumMemObjects         the number of D3D9Resouce objects to synchronize.
         * @param   cmdType                 the type of the command.
         */

        SyncD3D9Resources(IOclCommandQueueBase* const cmdQueue,
            ocl_entry_points* const pOclEntryPoints, D3D9Resource** const pMemObjects,
            size_t szNumMemObjects, cl_command_type cmdType) :
        SyncGraphicsApiObjects(cmdType, szNumMemObjects, cmdQueue, pOclEntryPoints,
            (GraphicsApiMemoryObject**)pMemObjects, cmdType == CL_COMMAND_ACQUIRE_DX9_OBJECTS_INTEL)
        { }

        // overridden methods:

        virtual const char* GetCommandName() const;

        virtual cl_err_code Execute();

    };

}}}
