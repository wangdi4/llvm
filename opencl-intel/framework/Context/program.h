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
///////////////////////////////////////////////////////////////////////////////////////////////////
//  program.h
//  Implementation of the Program class
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <cl_types.h>
#include <cl_object.h>
#include <cl_objects_map.h>
#include <observer.h>
#include <Logger.h>
#include <cl_synch_objects.h>
#include <map>
#include "device_program.h"
#include "kernel.h"

namespace Intel { namespace OpenCL { namespace Framework {
	class Context;

	class Program : public OCLObject<_cl_program_int>
	{

	public:
		Program(SharedPtr<Context> pContext);

        PREPARE_SHARED_PTR(Program)

		Program(SharedPtr<Context> pContext, ocl_entry_points * pOclEntryPoints);

		// return the context to which the program belongs
        const SharedPtr<Context>& GetContext() const { return m_pContext; }

		// create new kernel object
		virtual cl_err_code CreateKernel(const char * pscKernelName, SharedPtr<Kernel>* ppKernel);

		// create all kernels from the program object
		virtual cl_err_code CreateAllKernels(cl_uint uiNumKernels, cl_kernel * pclKernels, cl_uint * puiNumKernelsRet);

		// get the kernels associated to the program
		virtual cl_err_code GetKernels(cl_uint uiNumKernels, SharedPtr<Kernel>* ppKernels, cl_uint * puiNumKernelsRet);

		// remove kernel from program
		virtual cl_err_code RemoveKernel(cl_kernel clKernel);

		//Query details about the build
		cl_err_code GetBuildInfo(cl_device_id clDevice, 
			cl_program_build_info clParamName, 
			size_t szParamValueSize, 
			void * pParamValue, 
			size_t * pszParamValueSizeRet);

		// Implement the common queries. Specific queries like binaries or source go to implementing classes
		virtual cl_err_code GetInfo(cl_int param_name, size_t param_value_size, void * param_value, size_t * param_value_size_ret) const;

        // Returns a read only pointer to internal binary, used for build stages by program service
        const char*     GetBinaryInternal(cl_device_id clDevice);

        // Returns internal binary size, used for build stages by program service
        size_t          GetBinarySizeInternal(cl_device_id clDevice);

        // Sets the binary for a specific device, used for build stages by program service
        cl_err_code     SetBinaryInternal(cl_device_id clDevice, size_t uiBinarySize, const void* pBinary);

        // Clears the current build log, called in the beginning of each build sequence
		cl_err_code   ClearBuildLogInternal(cl_device_id clDevice);

        // Set the program build log for a specific device, used for build stages by program service
		cl_err_code   SetBuildLogInternal(cl_device_id clDevice, const char* szBuildLog);

        // Returns a read only pointer to internal source, used for build stages by program service
        virtual const char*     GetSourceInternal() { return NULL; };

        // set the program's build options
		// Creates a copy of the input
		// Returns CL_SUCCESS if nothing unexpected happened
        cl_err_code   SetBuildOptionsInternal(cl_device_id clDevice, const char* szBuildOptions);

        // get the latest program's build options for a specific device
        const char*   GetBuildOptionsInternal(cl_device_id clDevice);

        // Set the program state
        cl_err_code   SetStateInternal(cl_device_id clDevice, EDeviceProgramState state);

        // Get the program state
        EDeviceProgramState   GetStateInternal(cl_device_id clDevice);

        // Sets device handle, used for build stages by program service
        cl_err_code     SetDeviceHandleInternal(cl_device_id clDevice, cl_dev_program programHandle);

        // Get the number of associated devices
        cl_uint         GetNumDevices();

        // Fill in ppDeviceID with associated devices, assuming ppDeviceID has at least the number of cells returned from GetNumDevices
        cl_err_code     GetDevices(cl_device_id* pDeviceID);

        // Get the number of kernels
        cl_uint         GetNumKernels();

        // Returns true if the object can be safely worked on for the specified device and false otherwise
		bool Acquire(cl_device_id clDevice);

		// Notifies that we're done working with this object
		void Unacquire(cl_device_id clDevice);

		typedef std::map<cl_device_id, DeviceProgram*> tDeviceProgramMap;

		// Create map from cl_device_id that exist in the context and have or its parent (recursively) have built program (DeviceProgram*), to its related DeviceProgram*
		void SetContextDevicesToProgramMappingInternal();

		// Find the DeviceProgram related to devID and set in pOutID the ID of this DeviceProgram device.
		// If not found return false, otherwise return true
		bool GetMyRelatedProgramDeviceIDInternal(const cl_device_id devID, cl_int* pOutID);

	protected:
		virtual ~Program();

		DeviceProgram*  GetDeviceProgram(cl_device_id clDeviceId);
        DeviceProgram*  InternalGetDeviceProgram(cl_device_id clDeviceId);

		SharedPtr<Context>        m_pContext;
		DeviceProgram** m_ppDevicePrograms;
		cl_uint         m_szNumAssociatedDevices;

		OCLObjectsMap<_cl_kernel_int>	m_pKernels;			// associated kernels

		tDeviceProgramMap m_deviceToProgram;

	private:

		// Mutex for m_deviceToProgram 
		mutable Intel::OpenCL::Utils::OclMutex m_deviceProgramMapMutex;
	};
}}}
