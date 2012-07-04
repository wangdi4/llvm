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

#include "GraphicsApiMemoryObject.h"
#include "ocl_event.h"
#include "memobj_event.h"

using namespace std;
using namespace Intel::OpenCL::Utils;

namespace Intel { namespace OpenCL { namespace Framework
{
    /**
     * @fn  GraphicsApiMemoryObject::~GraphicsApiMemoryObject()
     */

    GraphicsApiMemoryObject::~GraphicsApiMemoryObject()
    {
		for(t_AcquiredObjects::iterator it=m_lstAcquiredObjectDescriptors.begin(); it!=m_lstAcquiredObjectDescriptors.end(); it++)
        {
            (*it).first->RemovePendency(this);
			(*it).second->Release();
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
     * @fn  OclEvent* LockOnDevice( IN const FissionableDevice* dev, IN MemObjUsage usage )
     */

    OclEvent* GraphicsApiMemoryObject::LockOnDevice( IN const FissionableDevice* dev, IN MemObjUsage usage )
    {
		OclAutoMutex mu(&m_muAcquireRelease);
        if ( m_lstAcquiredObjectDescriptors.end() == m_itCurrentAcquriedObject )
        {
            return NULL;
        }
		return m_itCurrentAcquriedObject->second->LockOnDevice(dev, usage);
    }

    /**
     * @fn  void UnLockOnDevice( IN const FissionableDevice* dev, IN MemObjUsage usage )
     */

    void GraphicsApiMemoryObject::UnLockOnDevice( IN const FissionableDevice* dev, IN MemObjUsage usage )
    {
		OclAutoMutex mu(&m_muAcquireRelease);
        if ( m_lstAcquiredObjectDescriptors.end() == m_itCurrentAcquriedObject )
        {
            return;
        }
        return m_itCurrentAcquriedObject->second->UnLockOnDevice(dev, usage);
    }

    /**
     * @fn  void GraphicsApiMemoryObject::GetLayout(size_t* dimensions, size_t* rowPitch,
     *      size_t* slicePitch) const
     */

    void GraphicsApiMemoryObject::GetLayout(size_t* dimensions, size_t* rowPitch, size_t* slicePitch) const
    {
		OclAutoMutex mu(&m_muAcquireRelease);
        if ( m_lstAcquiredObjectDescriptors.end() != m_itCurrentAcquriedObject )
        {
            m_itCurrentAcquriedObject->second->GetLayout(dimensions, rowPitch, slicePitch);
        }
    }

    /**
     * @fn  cl_err_code GraphicsApiMemoryObject::CheckBoundsRect(const size_t* pszOrigin,
     *      const size_t* pszRegion, size_t szRowPitch, size_t szSlicePitch) const
     */

    cl_err_code GraphicsApiMemoryObject::CheckBoundsRect(const size_t* pszOrigin, const size_t* pszRegion,
        size_t szRowPitch, size_t szSlicePitch) const
    {
		OclAutoMutex mu(&m_muAcquireRelease);
        if ( m_lstAcquiredObjectDescriptors.end() == m_itCurrentAcquriedObject )
        {
            return CL_INVALID_VALUE;
        }
        return m_itCurrentAcquriedObject->second->CheckBoundsRect(pszOrigin, pszRegion, szRowPitch, szSlicePitch);
    }

    /**
     * @fn  void* GraphicsApiMemoryObject::GetBackingStoreData(const size_t* pszOrigin) const
     */

    void* GraphicsApiMemoryObject::GetBackingStoreData(const size_t* pszOrigin) const
    {
		OclAutoMutex mu(&m_muAcquireRelease);
        if ( m_lstAcquiredObjectDescriptors.end() == m_itCurrentAcquriedObject )
        {
            return NULL;
        }
        return m_itCurrentAcquriedObject->second->GetBackingStoreData(pszOrigin);
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
		OclAutoMutex mu(&m_muAcquireRelease);
        if ( m_lstAcquiredObjectDescriptors.end() == m_itCurrentAcquriedObject )
        {
            return CL_INVALID_OPERATION;
        }
        return m_itCurrentAcquriedObject->second->MemObjCreateDevMappedRegion(pDevice, cmd_param_map, pHostMapDataPtr);
    }

    /**
     * @fn  cl_err_code GraphicsApiMemoryObject::MemObjReleaseDevMappedRegion(const FissionableDevice* pDevice,
     *      cl_dev_cmd_param_map* cmd_param_map)
     */

    cl_err_code	GraphicsApiMemoryObject::MemObjReleaseDevMappedRegion(const FissionableDevice* pDevice,
        cl_dev_cmd_param_map* cmd_param_map, void* pHostMapDataPtr)
    {
		OclAutoMutex mu(&m_muAcquireRelease);
        if ( m_lstAcquiredObjectDescriptors.end() == m_itCurrentAcquriedObject )
        {
            return CL_INVALID_OPERATION;
        }
        return m_itCurrentAcquriedObject->second->MemObjReleaseDevMappedRegion(pDevice, cmd_param_map, pHostMapDataPtr);
    }

	bool GraphicsApiMemoryObject::IsSynchDataWithHostRequired( cl_dev_cmd_param_map* IN pMapInfo, void* IN pHostMapDataPtr ) const
	{
		OclAutoMutex mu(&m_muAcquireRelease);
        if ( m_lstAcquiredObjectDescriptors.end() == m_itCurrentAcquriedObject )
        {
            return false;
        }
		return m_itCurrentAcquriedObject->second->IsSynchDataWithHostRequired( pMapInfo, pHostMapDataPtr );
	}

    cl_err_code GraphicsApiMemoryObject::SynchDataToHost( cl_dev_cmd_param_map* IN pMapInfo, void* IN pHostMapDataPtr )
    {
		OclAutoMutex mu(&m_muAcquireRelease);
        if ( m_lstAcquiredObjectDescriptors.end() == m_itCurrentAcquriedObject )
        {
            return CL_INVALID_OPERATION;
        }
        return m_itCurrentAcquriedObject->second->SynchDataToHost( pMapInfo, pHostMapDataPtr );
    }

    cl_err_code GraphicsApiMemoryObject::SynchDataFromHost( cl_dev_cmd_param_map* IN pMapInfo, void* IN pHostMapDataPtr )
    {
		OclAutoMutex mu(&m_muAcquireRelease);
        if ( m_lstAcquiredObjectDescriptors.end() == m_itCurrentAcquriedObject )
        {
            return CL_INVALID_OPERATION;
        }
        return m_itCurrentAcquriedObject->second->SynchDataFromHost( pMapInfo, pHostMapDataPtr );
    }

    /**
     * @fn  cl_err_code GraphicsApiMemoryObject::SetAcquireCmdEvent(OclEvent* pEvent)
     */

    cl_err_code GraphicsApiMemoryObject::SetAcquireCmdEvent(OclEvent* pEvent)
    {
		OclAutoMutex mu(&m_muAcquireRelease);

		if ( NULL != pEvent )
		{
			pEvent->AddPendency(this);
			m_lstAcquiredObjectDescriptors.push_back(t_AcquiredObjects::value_type(pEvent, CL_GFX_OBJECT_NOT_ACQUIRED));
			if ( m_lstAcquiredObjectDescriptors.end() == m_itCurrentAcquriedObject )
			{
				m_itCurrentAcquriedObject = m_lstAcquiredObjectDescriptors.begin();
			}
		} else
		{
			assert(!m_lstAcquiredObjectDescriptors.empty() && "On Release the Aquired Event list must be NOT empty");

			MemoryObject* pMemObj = m_lstAcquiredObjectDescriptors.front().second;
			if ( CL_GFX_OBJECT_NOT_ACQUIRED ==pMemObj )
			{
				// Nothing to do with NON acquried objects
				return CL_SUCCESS;
			}

			if ( (CL_GFX_OBJECT_NOT_READY != pMemObj) && (CL_GFX_OBJECT_FAIL_IN_ACQUIRE!=pMemObj) )
			{
				pMemObj->Release();				// Relase allocated child object
			}
			
			m_lstAcquiredObjectDescriptors.front().first->RemovePendency(this);	// Remove pendency from acquired command
			m_lstAcquiredObjectDescriptors.pop_front();
			m_itCurrentAcquriedObject = m_lstAcquiredObjectDescriptors.begin();
		}

		return CL_SUCCESS;
    }

    /**
     * @fn  cl_err_code GraphicsApiMemoryObject::GetDeviceDescriptor(FissionableDevice* pDevice,
     *      IOCLDevMemoryObject* *ppDevObject, OclEvent** ppEvent)
     */

    cl_err_code GraphicsApiMemoryObject::GetDeviceDescriptor(FissionableDevice* pDevice, IOCLDevMemoryObject* *ppDevObject, OclEvent** ppEvent)
    {
		OclAutoMutex mu(&m_muAcquireRelease);

        if ( m_lstAcquiredObjectDescriptors.empty() )
        {
            // Trying to get device descriptor before acquire operation was enqueued
            return CL_INVALID_OPERATION;
        }

		// Need to check if retriving curren acquried object descriptor or not
		if ( --m_lstAcquiredObjectDescriptors.end() == m_itCurrentAcquriedObject) 
		{
			if (CL_GFX_OBJECT_NOT_READY == m_itCurrentAcquriedObject->second) 
			{
				// Here the acquire operation is not finished and we need to create child object
				cl_err_code err = CreateChildObject();
				if ( CL_FAILED(err) )
				{
					return err;
				}
			}

			if ( CL_GFX_OBJECT_FAIL_IN_ACQUIRE == m_itCurrentAcquriedObject->second )
			{
				return CL_OUT_OF_RESOURCES;
			}
			
			MemoryObject* pCurrentChild = m_itCurrentAcquriedObject->second;
			if ( NULL != pCurrentChild )
			{
				return pCurrentChild->GetDeviceDescriptor(pDevice, ppDevObject, ppEvent);
			}
		}

		// Retrieving descriptor of the object that still was not aquired

	    // Now we need to create event that will updated on acquire completion
		assert(NULL!=ppEvent);
		OclEvent* pNewEvent = new MemoryObjectEvent(ppDevObject, this, pDevice);
		if ( NULL == pNewEvent )
		{
			return CL_OUT_OF_HOST_MEMORY;
		}

		// Link to the acquire event
		pNewEvent->AddDependentOn(m_lstAcquiredObjectDescriptors.back().first);
		*ppEvent = pNewEvent;
		// Event is born with user RefCount == 1, we should release it
		pNewEvent->Release();

	    return CL_NOT_READY;
	
    }

	cl_err_code GraphicsApiMemoryObject::UpdateDeviceDescriptor(FissionableDevice* IN pDevice, IOCLDevMemoryObject* OUT *ppDevObject)
	{
		OclAutoMutex mu(&m_muAcquireRelease);

        if ( m_lstAcquiredObjectDescriptors.end() == m_itCurrentAcquriedObject )
        {
            // Trying to get device descriptor before acquire operation was enqueued
            return CL_INVALID_OPERATION;
        }

        if ( CL_GFX_OBJECT_NOT_READY == m_itCurrentAcquriedObject->second )
        {
            // Here the acquire operation is not finished and we need to create child object
            cl_err_code err = CreateChildObject();
            if ( CL_FAILED(err) )
            {
                return err;
            }
        }

		MemoryObject* pCurrentChild = m_itCurrentAcquriedObject->second;
        if ( NULL != pCurrentChild )
        {
            return pCurrentChild->GetDeviceDescriptor(pDevice, ppDevObject, NULL);
        }

        if ( CL_GFX_OBJECT_FAIL_IN_ACQUIRE == m_itCurrentAcquriedObject->second )
        {
			return CL_OUT_OF_RESOURCES;
        }

		assert (0 && "We should not get to this line. After acquire completed, it should be a valid object or an error code");
		return CL_INVALID_OPERATION;
	}

}}}
