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
///////////////////////////////////////////////////////////////////////////////////////////////////
//  device_program.h
//  A per-device instance of a Program object
//  Created on:      28-Jul-2010
//  Original author: Doron Singer
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <cl_types.h>
#include <Logger.h>
#include <cl_synch_objects.h>
#include <cl_device_api.h>
#include <observer.h>
#include <build_event.h>
#include "ocl_object_base.h"

namespace Intel { namespace OpenCL { namespace Framework {

	typedef void (CL_CALLBACK *pfnNotifyBuildDone)(cl_program, void *);

    class Device;
    class FissionableDevice;

	enum EDeviceProgramState
	{
		DEVICE_PROGRAM_INVALID,      // Object was just created or build failed
		DEVICE_PROGRAM_SOURCE,		 // Source loaded
		DEVICE_PROGRAM_FE_BUILDING,	 // Currently building with BE compiler
		DEVICE_PROGRAM_EXTERNAL_BIN, // Binaries Loaded
		DEVICE_PROGRAM_INTERNAL_IR,  // IR Built from supplied source
		DEVICE_PROGRAM_BE_BUILDING,	 // Currently building with BE compiler
		DEVICE_PROGRAM_MACHINE_CODE // Build complete, executable code ready
	};

	class DeviceProgram : public OCLObjectBase, IFrontendBuildDoneObserver, IBuildDoneObserver
	{
	public:
		DeviceProgram();
        DeviceProgram(const DeviceProgram& dp);
		virtual ~DeviceProgram();

		void           SetDevice(FissionableDevice* pDevice);
		const FissionableDevice*  GetDevice()   const { return m_pDevice; }
		cl_device_id   GetDeviceId()            const { return m_deviceHandle; }
		cl_dev_program GetDeviceProgramHandle() const { return m_programHandle; }
		void           SetHandle(cl_program handle)   { m_parentProgramHandle = handle; }
		void           SetContext(cl_context context) { m_parentProgramContext = context;}

		// Sets the source-related data members of this program
		// No copying is done, as program source is immutable in OCL1.0/1 and Program instance will copy it for us
		void          SetSource(cl_uint uiNumStrings, size_t* pszLengths, const char** pSourceStrings);

		// Attempts to attach the given binary to the device associated with this program
		// Creates a copy of the input
		// Returns CL_SUCCESS if nothing unexpected happened -- note that iBinaryStatus can still be CL_INVALID_BINARY
		cl_err_code   SetBinary(size_t szBinarySize, const unsigned char* pBinary, cl_int* iBinaryStatus);

		// Attempts to return a binary associated with this program
		// If the program was built, the resulting binary
		// Otherwise, if IR was supplied, return that
		// Otherwise, return error
		cl_err_code   GetBinary(size_t uiBinSize, void * pBin, size_t * puiBinSizeRet);

		// Builds the program for the associated device
		cl_err_code Build( const char * pcOptions, pfnNotifyBuildDone pfn, void* pUserData);

		cl_err_code GetBuildInfo(cl_program_build_info clParamName, 
			                     size_t                szParamValueSize, 
			                     void *                pParamValue, 
			                     size_t *              pzsParamValueSizeRet);

		cl_build_status GetBuildStatus() const;

		cl_err_code GetNumKernels(cl_uint* pszNumKernels);
		// Returns an array of NULL-terminated strings, one for each 
		cl_err_code GetKernelNames(char** ppNames, size_t* pszNameSizes, size_t szNumNames);

		// Returns true if the object can be safely worked on and false otherwise
		bool Acquire();
		// Notifies that we're done working with this object
		void Unacquire() { m_currentAccesses--; }

		//observer interfaces
		virtual cl_err_code NotifyFEBuildDone(cl_device_id device, size_t szBinSize, const void * pBinData, const char *pBuildLog);
		virtual cl_err_code NotifyBuildDone(cl_device_id device, cl_build_status build_status);


	protected:

		cl_err_code FeBuild();
		cl_err_code BeBuild();

		cl_err_code CopyBinary(size_t szBinarySize, const unsigned char* pBinary);
		cl_err_code CopyBuildOptions(const char* pcBuildOptions);

		// Current program state
		EDeviceProgramState m_state;
		bool                m_bBuiltFromSource;
		bool                m_bFECompilerSuccess;
        bool                m_bIsClone;

		// Associated device members
		FissionableDevice*  m_pDevice;
		cl_device_id        m_deviceHandle;
		cl_dev_program      m_programHandle;
		cl_program          m_parentProgramHandle;
		cl_context          m_parentProgramContext;

		// Build-specific members
		void*               m_pUserData;
		pfnNotifyBuildDone  m_pfn;
		char*               m_pBuildOptions;
		// Events used for waiting on builds
		BuildEvent*         m_pFeBuildEvent;
		BuildEvent*         m_pBeBuildEvent;

		// Source-related members
		// Will be zeros and NULLs for programs created from binaries
		cl_uint             m_uiNumStrings;
		size_t*             m_pszStringLengths;
		const char**        m_pSourceStrings;

		// FE build log container
		size_t              m_szBuildLog;
		char*               m_pBuildLog;
		char                m_emptyString;

		// Binary-related members
		char*               m_pBinaryBits;
		size_t              m_szBinaryBitsSize;
		
		// Ensure the object is multi-thread safe
		Intel::OpenCL::Utils::AtomicCounter m_currentAccesses;
	};
}}}

