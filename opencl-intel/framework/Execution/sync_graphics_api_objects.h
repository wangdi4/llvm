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

#include "enqueue_commands.h"
#include "GraphicsApiMemoryObject.h"

namespace Intel { namespace OpenCL { namespace Framework
{
    /**
     * @class   SyncGraphicsApiObjects
     *
     * @brief   Common base class for synchronization of graphics API objects, such as OpenGL and
     * 			Direct3D
     *
     * @author  Aharon
     * @date    7/13/2011
     *
     * @sa  Intel::OpenCL::Framework::RuntimeCommand
     */

    class SyncGraphicsApiObjects : public RuntimeCommand
    {
        const bool m_bIsAcquireCmd;
        const cl_command_type m_cmdType;
        const size_t m_uiMemObjNum;
        GraphicsApiMemoryObject** m_pMemObjects;

    protected:
        
        /**
         * @fn  SyncGraphicsApiObjects::SyncGraphicsApiObjects(cl_command_type cmdType,
         *      unsigned int uiMemObjNum, IOclCommandQueueBase* cmdQueue,
         *      ocl_entry_points * pOclEntryPoints, GraphicsApiMemoryObject** pMemObjects,
         *      bool bIsAcquireCmd);
         *
         * @brief   Constructor.
         *
         * @author  Aharon
         * @date    7/13/2011
         */

        SyncGraphicsApiObjects(cl_command_type cmdType, size_t uiMemObjNum,
            IOclCommandQueueBase* cmdQueue, ocl_entry_points * pOclEntryPoints,
            GraphicsApiMemoryObject** pMemObjects, bool bIsAcquireCmd);

        public:

        /**
         * @fn  SyncGraphicsApiObjects::~SyncGraphicsApiObjects();
         *
         * @brief   Finaliser.
         *
         * @author  Aharon
         * @date    7/13/2011
         */

        virtual ~SyncGraphicsApiObjects();

        /**
         * @fn  unsigned int SyncGraphicsApiObjects::GetNumMemObjs() const
         *
         * @brief   Gets the number of memory objects.
         *
         * @author  Aharon
         * @date    7/13/2011
         *
         * @return  The number of memory objects.
         */

        size_t GetNumMemObjs() const { return m_uiMemObjNum; }

        // inherited methods:

        virtual cl_command_type GetCommandType() const  { return m_cmdType; }

        virtual ECommandExecutionType GetExecutionType() const { return RUNTIME_EXECUTION_TYPE; }

        virtual cl_err_code Init();

        cl_err_code CommandDone();

        virtual bool isControlCommand()	const { return false; }

        /**
         * @fn  const GraphicsApiMemoryObject& SyncGraphicsApiObjects::GetMemoryObject(size_t Index) const
         *
         * @brief   Gets a memory object.
         *
         * @author  Aharon
         * @date    7/13/2011
         *
         * @param   Index   Zero-based index of the memory object.
         *
         * @return  The memory object.
         */

        const GraphicsApiMemoryObject& GetMemoryObject(size_t Index) const { return *m_pMemObjects[Index]; }

        /**
         * @fn  GraphicsApiMemoryObject& SyncGraphicsApiObjects::GetMemoryObject(size_t Index)
         *
         * @brief   Gets a memory object.
         *
         * @author  Aharon
         * @date    7/13/2011
         *
         * @param   Index   Zero-based index of the memory object.
         *
         * @return  The memory object.
         */

        GraphicsApiMemoryObject& GetMemoryObject(size_t Index) { return *m_pMemObjects[Index]; }
	private:
		SyncGraphicsApiObjects(const SyncGraphicsApiObjects& s);
		SyncGraphicsApiObjects& operator=(const SyncGraphicsApiObjects& s);
    };

}}}
