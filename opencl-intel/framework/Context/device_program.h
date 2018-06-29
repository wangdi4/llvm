// INTEL CONFIDENTIAL
//
// Copyright 2006-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#pragma once

#include <cl_types.h>
#include <Logger.h>
#include <cl_synch_objects.h>
#include <cl_device_api.h>
#include <observer.h>
#include <build_event.h>
#include <vector>
#include <string>

namespace Intel { namespace OpenCL { namespace Framework {
    class Device;
    class FissionableDevice;

    enum EDeviceProgramState
    {
        DEVICE_PROGRAM_INVALID,                 // Object was just created
        DEVICE_PROGRAM_BUILTIN_KERNELS,         // Program based on the built-in kernels
        DEVICE_PROGRAM_SOURCE,                  // Source loaded
        DEVICE_PROGRAM_FE_COMPILING,            // Currently compiling with FE compiler
        DEVICE_PROGRAM_COMPILED,                // Compiled IR
        DEVICE_PROGRAM_COMPILE_FAILED,          // Compilation failed
        DEVICE_PROGRAM_FE_LINKING,              // Currently linking with FE compiler
        DEVICE_PROGRAM_LINKED,                  // Linked IR
        DEVICE_PROGRAM_LINK_FAILED,             // Linking failed
        DEVICE_PROGRAM_LOADED_IR,               // Loaded IR
        DEVICE_PROGRAM_SPIRV,                   // Compiled SPIRV
        DEVICE_PROGRAM_BE_BUILDING,             // Currently building with BE compiler
        DEVICE_PROGRAM_BUILD_DONE,              // Build complete, executable code ready
        DEVICE_PROGRAM_CUSTOM_BINARY,           // Program contains device specific binary
        DEVICE_PROGRAM_BUILD_FAILED,            // Build failed
        DEVICE_PROGRAM_CREATING_AUTORUN         // Currently creating instances of autorun kernels
    };

    class DeviceProgram
    {
    public:
        DeviceProgram();
        DeviceProgram(const DeviceProgram& dp);
        virtual ~DeviceProgram();

        void           SetDevice(const SharedPtr<FissionableDevice>& pDevice);
        const SharedPtr<FissionableDevice>&  GetDevice()   const { return m_pDevice; }
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

        bool IsBinaryAvailable(cl_program_binary_type requestedType) const;

        cl_err_code GetNumKernels(cl_uint* pszNumKernels);
        // Returns an array of NULL-terminated strings, one for each
        cl_err_code GetKernelNames(char** ppNames, size_t* pszNameSizes, size_t szNumNames);
        cl_err_code GetAutorunKernelsNames(std::vector<std::string> &vsNames);

        // Returns true if the object can be safely worked on and false otherwise
        bool Acquire();
        // Notifies that we're done working with this object
        void Unacquire() { m_currentAccesses--; }

        ///////////////////////////////////////////////////////////
        // Get/Set function used by program service for building //
        ///////////////////////////////////////////////////////////

        // Returns a read only pointer to internal binary
        const char*   GetBinaryInternal() const { return m_pBinaryBits; };

        // Returns internal binary size
        size_t        GetBinarySizeInternal() const { return m_uiBinaryBitsSize; };

        // Returns internal binary type
        cl_program_binary_type   GetBinaryTypeInternal() const { return m_clBinaryBitsType; };

        // Set internal binary type
        // Returns CL_SUCCESS if nothing unexpected happened
        cl_err_code   SetBinaryTypeInternal(cl_program_binary_type clBinaryType);

        // Set the program binary for a specific device
        // Creates a copy of the input
        // Returns CL_SUCCESS if nothing unexpected happened
        cl_err_code   SetBinaryInternal(size_t uiBinarySize, const void* pBinary, cl_program_binary_type clBinaryType);

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

        bool CheckProgramBinary(size_t uiBinSize, const void* pBinary, cl_prog_binary_type* pBinaryType = nullptr);

    protected:
        // Current program state
        EDeviceProgramState m_state;
        bool                m_bBuiltFromSource;
        bool                m_bFECompilerSuccess;
        bool                m_bIsClone;

        // Associated device members
        SharedPtr<FissionableDevice>  m_pDevice;
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
        cl_program_binary_type m_clBinaryBitsType;

        // Ensure the object is multi-thread safe
        mutable Intel::OpenCL::Utils::AtomicCounter m_currentAccesses;

    private:
        DeviceProgram& operator=(const DeviceProgram&);
    };
}}}
