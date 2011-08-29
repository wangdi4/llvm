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

#include "MemoryObject.h"

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

        Intel::OpenCL::Utils::AtomicPointer<OclEvent> m_pAcquireEvent;
        Intel::OpenCL::Utils::AtomicCounter m_clAcquireState;

    protected:

        Intel::OpenCL::Utils::AtomicPointer<MemoryObject> m_pChildObject;
        // This object is the actual object that handle interaction with devices
        Intel::OpenCL::Utils::OclMutex m_muAcquireRelease;

    public:

        /**
         * @fn  GraphicsApiMemoryObject::GraphicsApiMemoryObject(Context* pContext,
         *      ocl_entry_points* pOclEntryPoints)
         *
         * @brief   Constructor.
         *
         * @author  Aharon
         * @date    7/13/2011
         */

        GraphicsApiMemoryObject(Context* pContext, ocl_entry_points* pOclEntryPoints, cl_mem_object_type clObjType) :
          MemoryObject(pContext, pOclEntryPoints), m_pAcquireEvent(NULL),
	  m_clAcquireState(CL_SUCCESS), m_pChildObject(NULL) { }

        /**
         * @fn  virtual GraphicsApiMemoryObject::~GraphicsApiMemoryObject();
         *
         * @brief   Finaliser.
         *
         * @author  Aharon
         * @date    7/13/2011
         */

        virtual ~GraphicsApiMemoryObject();

        cl_err_code SetAcquireCmdEvent(OclEvent* pEvent); // Set Event of Acquire command that belongs to the object.

        // inherited methods:

        virtual cl_err_code UpdateHostPtr(cl_mem_flags clMemFlags, void* pHostPtr);

        virtual cl_err_code UpdateLocation(FissionableDevice* pDevice);

        virtual bool IsSharedWith(FissionableDevice* pDevice);

        virtual void GetLayout(size_t* dimensions, size_t* rowPitch, size_t* slicePitch) const;

        virtual cl_err_code CheckBoundsRect(const size_t* pszOrigin, const size_t* pszRegion,
            size_t szRowPitch, size_t szSlicePitch) const;

        virtual void* GetBackingStoreData(const size_t* pszOrigin = NULL) const;

        virtual cl_err_code CreateDeviceResource(FissionableDevice* pDevice);

        virtual cl_err_code GetDeviceDescriptor(FissionableDevice* pDevice,
            IOCLDevMemoryObject** ppDevObject, OclEvent** ppEvent);

        virtual bool IsSupportedByDevice(FissionableDevice* pDevice);

        virtual cl_err_code NotifyDeviceFissioned(FissionableDevice* parent, size_t count,
            FissionableDevice** children);

        // In the case when Backing Store region is different from Host Map pointer provided by user
        // we need to synchronize user area with device area after/before each map/unmap command
        //
        virtual cl_err_code SynchDataToHost(   cl_dev_cmd_param_map* IN pMapInfo, void* IN pHostMapDataPtr );
        virtual cl_err_code SynchDataFromHost( cl_dev_cmd_param_map* IN pMapInfo, void* IN pHostMapDataPtr );

    protected:

        /**
         * @fn  long GraphicsApiMemoryObject::GetAcquireState() const
         *
         * @brief   Gets the acquire state.
         *
         * @author  Aharon
         * @date    7/13/2011
         *
         * @return  The acquire state.
         */

        long GetAcquireState() const { return m_clAcquireState; }

        /**
         * @fn  void GraphicsApiMemoryObject::SetAcquireState(long lVal)
         *
         * @brief   Sets an acquire state.
         *
         * @author  Aharon
         * @date    7/13/2011
         *
         * @param   lVal    The value.
         */

        void SetAcquireState(long lVal) { m_clAcquireState = lVal; }

        // This function is responsible for creating a supporting child object
        virtual cl_err_code CreateChildObject() = 0;

        // inherited methods:

        virtual	cl_err_code	MemObjCreateDevMappedRegion(const FissionableDevice*,
            cl_dev_cmd_param_map* cmd_param_map, void** pHostMapDataPtr);

        virtual	cl_err_code	MemObjReleaseDevMappedRegion(const FissionableDevice*,
            cl_dev_cmd_param_map* cmd_param_map, void* pHostMapDataPtr);

    };

}}}
