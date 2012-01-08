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

#include "GraphicsApiMemoryObject.h"
#include "ocl_event.h"
#include "memobj_event.h"

namespace Intel { namespace OpenCL { namespace Framework
{
    /**
     * @fn  GraphicsApiMemoryObject::~GraphicsApiMemoryObject()
     */

    GraphicsApiMemoryObject::~GraphicsApiMemoryObject()
    {
        OclEvent* pOldEvent = m_pAcquireEvent.exchange(NULL);
        if ( NULL != pOldEvent )
        {
            pOldEvent->RemovePendency(this);
        }
    }

    /**
     * @fn  cl_err_code GraphicsApiMemoryObject::UpdateHostPtr(cl_mem_flags clMemFlags,
     *      void* pHostPtr)
     */

    cl_err_code GraphicsApiMemoryObject::UpdateHostPtr(cl_mem_flags clMemFlags, void* pHostPtr)
    {
        return CL_SUCCESS;
    }

    /**
     * @fn  cl_err_code GraphicsApiMemoryObject::UpdateLocation(FissionableDevice* pDevice)
     */

    cl_err_code GraphicsApiMemoryObject::UpdateLocation(FissionableDevice* pDevice)
    {
        return CL_SUCCESS;
    }

    /**
     * @fn  bool GraphicsApiMemoryObject::IsSharedWith(FissionableDevice* pDevice)
     */

    bool GraphicsApiMemoryObject::IsSharedWith(FissionableDevice* pDevice)
    {
        return true;
    }

    /**
     * @fn  void GraphicsApiMemoryObject::GetLayout(size_t* dimensions, size_t* rowPitch,
     *      size_t* slicePitch) const
     */

    void GraphicsApiMemoryObject::GetLayout(size_t* dimensions, size_t* rowPitch, size_t* slicePitch) const
    {
        if (NULL != m_pChildObject)
        {
            m_pChildObject->GetLayout(dimensions, rowPitch, slicePitch);
        }
    }

    /**
     * @fn  cl_err_code GraphicsApiMemoryObject::CheckBoundsRect(const size_t* pszOrigin,
     *      const size_t* pszRegion, size_t szRowPitch, size_t szSlicePitch) const
     */

    cl_err_code GraphicsApiMemoryObject::CheckBoundsRect(const size_t* pszOrigin, const size_t* pszRegion,
        size_t szRowPitch, size_t szSlicePitch) const
    {
        if (NULL == m_pChildObject)
        {
            return CL_INVALID_VALUE;
        }
        return m_pChildObject->CheckBoundsRect(pszOrigin, pszRegion, szRowPitch, szSlicePitch);
    }

    /**
     * @fn  void* GraphicsApiMemoryObject::GetBackingStore(const size_t* pszOrigin) const
     */

    void* GraphicsApiMemoryObject::GetBackingStoreData(const size_t* pszOrigin) const
    {
        if (NULL == m_pChildObject)
        {
            return NULL;
        }
        return m_pChildObject->GetBackingStoreData(pszOrigin);
    }

    /**
     * @fn  cl_err_code GraphicsApiMemoryObject::CreateDeviceResource(FissionableDevice* pDevice)
     */

    cl_err_code GraphicsApiMemoryObject::CreateDeviceResource(FissionableDevice* pDevice)
    {
        return CL_SUCCESS;
    }

        /**
     * @fn  bool GraphicsApiMemoryObject::IsSupportedByDevice(FissionableDevice* pDevice)
     */

    bool GraphicsApiMemoryObject::IsSupportedByDevice(FissionableDevice* pDevice)
    {
        return true;
    }

    /**
     * @fn  cl_err_code GraphicsApiMemoryObject::MemObjCreateDevMappedRegion(const FissionableDevice* pDevice,
     *      cl_dev_cmd_param_map* cmd_param_map)
     */

    cl_err_code	GraphicsApiMemoryObject::MemObjCreateDevMappedRegion(const FissionableDevice* pDevice,
        cl_dev_cmd_param_map* cmd_param_map, void** pHostMapDataPtr)
    {
        return m_pChildObject->MemObjCreateDevMappedRegion(pDevice, cmd_param_map, pHostMapDataPtr);
    }

    /**
     * @fn  cl_err_code GraphicsApiMemoryObject::MemObjReleaseDevMappedRegion(const FissionableDevice* pDevice,
     *      cl_dev_cmd_param_map* cmd_param_map)
     */

    cl_err_code	GraphicsApiMemoryObject::MemObjReleaseDevMappedRegion(const FissionableDevice* pDevice,
        cl_dev_cmd_param_map* cmd_param_map, void* pHostMapDataPtr)
    {
        return m_pChildObject->MemObjReleaseDevMappedRegion(pDevice, cmd_param_map, pHostMapDataPtr);
    }

    cl_err_code GraphicsApiMemoryObject::SynchDataToHost( cl_dev_cmd_param_map* IN pMapInfo, void* IN pHostMapDataPtr )
    {
        return m_pChildObject->SynchDataToHost( pMapInfo, pHostMapDataPtr );
    }

    cl_err_code GraphicsApiMemoryObject::SynchDataFromHost( cl_dev_cmd_param_map* IN pMapInfo, void* IN pHostMapDataPtr )
    {
        return m_pChildObject->SynchDataFromHost( pMapInfo, pHostMapDataPtr );
    }


    /**
     * @fn  cl_err_code GraphicsApiMemoryObject::NotifyDeviceFissioned(FissionableDevice* parent, size_t count,
     *      FissionableDevice** children)
     */

    cl_err_code GraphicsApiMemoryObject::NotifyDeviceFissioned(FissionableDevice* parent, size_t count,
        FissionableDevice** children)
    {
        return CL_SUCCESS;
    }

    /**
     * @fn  cl_err_code GraphicsApiMemoryObject::SetAcquireCmdEvent(OclEvent* pEvent)
     */

    cl_err_code GraphicsApiMemoryObject::SetAcquireCmdEvent(OclEvent* pEvent)
    {
		if ( NULL != pEvent )
		{
			pEvent->AddPendency(this);
		}
		OclEvent* pOldEvent = m_pAcquireEvent.exchange(pEvent);        
        if ( NULL != pOldEvent )
        {
            pOldEvent->RemovePendency(this);            
        }
		return CL_SUCCESS;
    }

    /**
     * @fn  cl_err_code GraphicsApiMemoryObject::GetDeviceDescriptor(FissionableDevice* pDevice,
     *      IOCLDevMemoryObject* *ppDevObject, OclEvent** ppEvent)
     */

    cl_err_code GraphicsApiMemoryObject::GetDeviceDescriptor(FissionableDevice* pDevice, IOCLDevMemoryObject* *ppDevObject, OclEvent** ppEvent)
    {
        if ( NULL == m_pAcquireEvent )
        {
            // Trying to get device descriptor before acquire operation was enqueued
            return CL_INVALID_OPERATION;
        }

        if ( CL_NOT_READY == m_clAcquireState )
        {
            // Here the acquire operation is not finished and we need to create child object
            cl_err_code err = CreateChildObject();
            if ( CL_FAILED(err) )
            {
                return err;
            }
        }

        if ( NULL != m_pChildObject )
        {
            return m_pChildObject->GetDeviceDescriptor(pDevice, ppDevObject, ppEvent);
        }

        if ( CL_FAILED(m_clAcquireState) )
        {
            return m_clAcquireState;
        }


        // Now we need to create event that will updated on acquire completion
        assert(NULL!=ppEvent);
        OclEvent* pNewEvent = new MemoryObjectEvent(ppDevObject, this, pDevice);
        if ( NULL == pNewEvent )
        {
            return CL_OUT_OF_HOST_MEMORY;
        }
        pNewEvent->AddDependentOn(m_pAcquireEvent);
        *ppEvent = pNewEvent;
        pNewEvent->Release();

        return CL_NOT_READY;
    }

}}}
