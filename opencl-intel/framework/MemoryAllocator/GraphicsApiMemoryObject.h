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

#include "MemoryObject.h"

#include <list>

namespace Intel { namespace OpenCL { namespace Framework
{    

    /**
     * @class   GraphicsApiMemoryObject
     *
     * @brief   Common base class for Graphics API MemoryObjects, like OpenGL and Direct3D
     *
     * @author  Aharon
     * @date    7/13/2011
     *
     * @sa  Intel::OpenCL::Framework::MemoryObject
     */

    class GraphicsApiMemoryObject : public MemoryObject
    {

    public:

        /**
         * @fn  GraphicsApiMemoryObject::GraphicsApiMemoryObject(SharedPtr<Context> pContext,
         *      ocl_entry_points* pOclEntryPoints)
         *
         * @brief   Constructor.
         *
         * @author  Aharon
         * @date    7/13/2011
         */

        GraphicsApiMemoryObject(SharedPtr<Context> pContext) :
          MemoryObject(pContext), m_itCurrentAcquriedObject(m_lstAcquiredObjectDescriptors.end()) { }

    public:

        PREPARE_SHARED_PTR(GraphicsApiMemoryObject)

        /**
         * @fn  virtual GraphicsApiMemoryObject::~GraphicsApiMemoryObject();
         *
         * @brief   Finaliser.
         *
         * @author  Aharon
         * @date    7/13/2011
         */

        virtual ~GraphicsApiMemoryObject();

        cl_err_code SetAcquireCmdEvent(SharedPtr<OclEvent> pEvent); // Set Event of Acquire command that belongs to the object.

        cl_err_code ClearAcquireCmdEvent(); // Clear Event of Acquire command that belongs to the object.

        // inherited methods:
        virtual cl_err_code LockOnDevice( IN const SharedPtr<FissionableDevice>& dev, IN MemObjUsage usage, OUT MemObjUsage* pOutUsageLocked, OUT SharedPtr<OclEvent>& pOutEvent );
        virtual cl_err_code UnLockOnDevice( IN const SharedPtr<FissionableDevice>& dev, IN MemObjUsage usage );

        virtual cl_err_code UpdateHostPtr(cl_mem_flags clMemFlags, void* pHostPtr);        
		
        virtual cl_err_code CheckBoundsRect(const size_t* pszOrigin, const size_t* pszRegion,
            size_t szRowPitch, size_t szSlicePitch) const;

        virtual void* GetBackingStoreData(const size_t* pszOrigin = NULL) const;

        virtual cl_err_code CreateDeviceResource(const SharedPtr<FissionableDevice>& pDevice);

        virtual cl_err_code GetDeviceDescriptor(const SharedPtr<FissionableDevice>& pDevice,
            IOCLDevMemoryObject** ppDevObject, SharedPtr<OclEvent>* ppEvent);

		virtual cl_err_code UpdateDeviceDescriptor(const SharedPtr<FissionableDevice>& IN pDevice,
			IOCLDevMemoryObject* OUT *ppDevObject);

        virtual bool IsSupportedByDevice(const SharedPtr<FissionableDevice>& pDevice);

        // In the case when Backing Store region is different from Host Map pointer provided by user
        // we need to synchronize user area with device area after/before each map/unmap command
        //
		virtual bool        IsSynchDataWithHostRequired( cl_dev_cmd_param_map* IN pMapInfo, void* IN pHostMapDataPtr ) const;
        virtual cl_err_code SynchDataToHost(   cl_dev_cmd_param_map* IN pMapInfo, void* IN pHostMapDataPtr );
        virtual cl_err_code SynchDataFromHost( cl_dev_cmd_param_map* IN pMapInfo, void* IN pHostMapDataPtr );

    protected:

        // This function is responsible for creating a supporting child object
        virtual cl_err_code CreateChildObject() = 0;

        // inherited methods:

        virtual	cl_err_code	MemObjCreateDevMappedRegion(const SharedPtr<FissionableDevice> &,
            cl_dev_cmd_param_map* cmd_param_map, void** pHostMapDataPtr);

        virtual	cl_err_code	MemObjReleaseDevMappedRegion(const SharedPtr<FissionableDevice>&,
            cl_dev_cmd_param_map* cmd_param_map, void* pHostMapDataPtr);


        mutable Intel::OpenCL::Utils::OclSpinMutex m_muAcquireRelease;
		// This list hold the ordered events which represent acquire commands and appropriate child object,
		// the latest event located in the tail.
		// Scenario:
		// clEnqueueAcquireGL...()
		// clEnqueuNDRange()
		// clEnqueueReleaseGL...()
		// clEnqueueAcquireGL...()
		// clEnqueuNDRange()
		// clEnqueueReleaseGL...()
		// ...
		// clFinish()

        enum ClGfxObjectState {
            CL_GFX_OBJECT_VALID,
            CL_GFX_OBJECT_NOT_READY,
            CL_GFX_OBJECT_NOT_ACQUIRED,
            CL_GFX_OBJECT_FAIL_IN_ACQUIRE
        };

        class AcquiredObject : public SharedPtr<MemoryObject>
        {
        public:

            AcquiredObject(ClGfxObjectState state, const SharedPtr<MemoryObject>& pObj = SharedPtr<MemoryObject>()) : SharedPtr<MemoryObject>(pObj), m_state(state)
            {
                assert(Invarient());
            }

            AcquiredObject(const SharedPtr<MemoryObject>& pObj) : SharedPtr<MemoryObject>(pObj), m_state(CL_GFX_OBJECT_VALID)
            {
                assert(Invarient());
            }

            operator ClGfxObjectState() { return m_state; }

        private:

            bool Invarient() { return !(CL_GFX_OBJECT_VALID == m_state && NULL == m_ptr) && !(CL_GFX_OBJECT_VALID != m_state && NULL != m_ptr); }

            ClGfxObjectState m_state;
        };

        // The list is protectd by m_muAcquireRelease
		typedef std::list< std::pair<SharedPtr<OclEvent>, AcquiredObject > > t_AcquiredObjects;
		t_AcquiredObjects m_lstAcquiredObjectDescriptors;
		t_AcquiredObjects::iterator m_itCurrentAcquriedObject;

	};

}}}
