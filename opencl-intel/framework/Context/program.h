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

namespace Intel { namespace OpenCL { namespace Framework {
	class Kernel;
	class Context;

	class Program : public OCLObject<_cl_program_int>, IDeviceFissionObserver
	{

	public:
		Program(Context * pContext, ocl_entry_points * pOclEntryPoints);

		// return the context to which the program belongs
		const Context * GetContext() const { return m_pContext; }

		// create new kernel object
		virtual cl_err_code CreateKernel(const char * pscKernelName, Kernel ** ppKernel);

		// create all kernels from the program object
		virtual cl_err_code CreateAllKernels(cl_uint uiNumKernels, cl_kernel * pclKernels, cl_uint * puiNumKernelsRet);

		// get the kernels associated to the program
		virtual cl_err_code GetKernels(cl_uint uiNumKernels, Kernel ** ppKernels, cl_uint * puiNumKernelsRet);

		// remove kernel from program
		virtual cl_err_code RemoveKernel(cl_kernel clKernel);

		//Query details about the build
		cl_err_code GetBuildInfo(cl_device_id clDevice, 
			cl_program_build_info clParamName, 
			size_t szParamValueSize, 
			void * pParamValue, 
			size_t * pszParamValueSizeRet);

		// build the program for the given set of devices
		cl_err_code Build(cl_uint	uiNumDevices,
			const cl_device_id *	pclDeviceList,
			const char *			pcOptions,
			pfnNotifyBuildDone      pfn,
			void *					pUserData);

		// Implement the common queries. Specific queries like binaries or source go to implementing classes
		virtual cl_err_code GetInfo(cl_int param_name, size_t param_value_size, void * param_value, size_t * param_value_size_ret) const;

        // Called by the context when a device is fissioned
        // Left virtual as implementation is different for programs created from source and ones created from binaries
        virtual cl_err_code NotifyDeviceFissioned(FissionableDevice* parent, size_t count, FissionableDevice** children) = 0;

	protected:
		virtual ~Program();

		DeviceProgram* GetDeviceProgram(cl_device_id clDeviceId);
        DeviceProgram* InternalGetDeviceProgram(cl_device_id clDeviceId);

		Context*       m_pContext;
		DeviceProgram** m_ppDevicePrograms;
		cl_uint         m_szNumAssociatedDevices;

        mutable Utils::OclReaderWriterLock m_deviceProgramLock;

		OCLObjectsMap<_cl_kernel_int>	m_pKernels;			// associated kernels

	};
}}}
