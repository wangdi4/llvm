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

    class Device;
    class FissionableDevice;

	enum EDeviceProgramState
	{
		DEVICE_PROGRAM_INVALID,         // Object was just created
		DEVICE_PROGRAM_SOURCE,		    // Source loaded
		DEVICE_PROGRAM_FE_COMPILING,    // Currently compiling with FE compiler
		DEVICE_PROGRAM_COMPILED,        // Compiled IR
        DEVICE_PROGRAM_COMPILE_FAILED,  // Compilation failed
        DEVICE_PROGRAM_FE_LINKING,      // Currently linking with FE compiler
        DEVICE_PROGRAM_LINKED,          // Linked IR
        DEVICE_PROGRAM_LINK_FAILED,     // Linking failed
        DEVICE_PROGRAM_LOADED_IR,       // Loaded IR
		DEVICE_PROGRAM_BE_BUILDING,	    // Currently building with BE compiler
		DEVICE_PROGRAM_BUILD_DONE,      // Build complete, executable code ready
        DEVICE_PROGRAM_BUILD_FAILED     // Build failed
	};

	class DeviceProgram : public OCLObjectBase
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


        // Attempts to attach the given binary to the device associated with this program
		// Creates a copy of the input
		// Returns CL_SUCCESS if nothing unexpected happened -- note that iBinaryStatus can still be CL_INVALID_BINARY
		cl_err_code   SetBinary(size_t uiBinarySize, const unsigned char* pBinary, cl_int* piBinaryStatus);

		// Attempts to return a binary associated with this program
		// If the program was built, the resulting binary
		// Otherwise, if IR was supplied, return that
		// Otherwise, return error
		cl_err_code   GetBinary(size_t uiBinSize, void * pBin, size_t * puiBinSizeRet);

		cl_err_code GetBuildInfo (cl_program_build_info clParamName, 
			                     size_t                uiParamValueSize, 
			                     void *                pParamValue, 
			                     size_t *              puiParamValueSizeRet) const;

		cl_build_status GetBuildStatus() const;

		cl_err_code GetNumKernels(cl_uint* pszNumKernels);
		// Returns an array of NULL-terminated strings, one for each 
		cl_err_code GetKernelNames(char** ppNames, size_t* pszNameSizes, size_t szNumNames);

		// Returns true if the object can be safely worked on and false otherwise
		bool Acquire();
		// Notifies that we're done working with this object
		void Unacquire() { m_currentAccesses--; }

        ///////////////////////////////////////////////////////////
        // Get/Set function used by program service for building //
        ///////////////////////////////////////////////////////////

        // Returns a read only pointer to internal binary
        const char*   GetBinaryInternal() { return m_pBinaryBits; };

        // Returns internal binary size
        size_t        GetBinarySizeInternal() { return m_uiBinaryBitsSize; };

        // Set the program binary for a specific device
        // Creates a copy of the input
		// Returns CL_SUCCESS if nothing unexpected happened
		cl_err_code   SetBinaryInternal(size_t uiBinarySize, const void* pBinary);


        // Clears the current build log, called in the beginning of each build sequence
		// Returns CL_SUCCESS if nothing unexpected happened
		cl_err_code   ClearBuildLogInternal();

        // Set the program build log for a specific device
        // If build log already exists input is concatenated
        // Creates a copy of the input
		// Returns CL_SUCCESS if nothing unexpected happened
		cl_err_code   SetBuildLogInternal(const char* szBuildLog);

        // set the program's build options
		// Creates a copy of the input
		// Returns CL_SUCCESS if nothing unexpected happened
        cl_err_code   SetBuildOptionsInternal(const char* szBuildOptions);

        // get the latest program's build options
        const char*   GetBuildOptionsInternal();

        // Set the program state
        cl_err_code   SetStateInternal(EDeviceProgramState state);

        // Get the program state
        EDeviceProgramState   GetStateInternal() { return m_state; };

        // Set device handle
        cl_err_code   SetDeviceHandleInternal(cl_dev_program programHandle);

    protected:

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

		// FE build log container
		size_t              m_uiBuildLogSize;
		char*               m_szBuildLog;
		char                m_emptyString;

        // Build options
        char*               m_szBuildOptions;

		// Binary-related members
		char*               m_pBinaryBits;
		size_t              m_uiBinaryBitsSize;
		
		// Ensure the object is multi-thread safe
		mutable Intel::OpenCL::Utils::AtomicCounter m_currentAccesses;
	};
}}}

