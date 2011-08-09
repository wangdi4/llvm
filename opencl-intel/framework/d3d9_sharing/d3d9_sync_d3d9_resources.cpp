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

#include <cassert>
#include "d3d9_sync_d3d9_resources.h"
#include "CL\cl_d3d9.h"

namespace Intel { namespace OpenCL { namespace Framework
{
    /**
     * @fn  const char* SyncD3D9Resources::GetCommandName() const
     */

    const char* SyncD3D9Resources::GetCommandName() const
    {
        if (CL_COMMAND_ACQUIRE_D3D9_OBJECTS_INTEL == GetCommandType())
        {
            return "CL_COMMAND_ACQUIRE_D3D9_OBJECTS_INTEL";
        }
        assert(CL_COMMAND_RELEASE_D3D9_OBJECTS_INTEL == GetCommandType());
        return "CL_COMMAND_RELEASE_D3D9_OBJECTS_INTEL";
    }

    /**
     * @fn  cl_err_code SyncD3D9Resources::Execute()
     */

    cl_err_code SyncD3D9Resources::Execute()
    {
        if (CL_COMMAND_ACQUIRE_D3D9_OBJECTS_INTEL == GetCommandType())
        {
            for (unsigned int i = 0; i < GetNumMemObjs(); i++)
            {
                dynamic_cast<D3D9Resource&>(GetMemoryObject(i)).AcquireD3D9();
            }
        }
        else
        {
            for (unsigned int i = 0; i < GetNumMemObjs(); i++)
            {
                dynamic_cast<D3D9Resource&>(GetMemoryObject(i)).ReleaseD3D9();
            }
        }
        return RuntimeCommand::Execute();
    }

}}}
