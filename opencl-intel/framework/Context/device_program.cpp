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

#include "device_program.h"
#include "framework_proxy.h"
#include "Device.h"
#include "events_manager.h"
#include "fe_compiler.h"
#include "cl_sys_defines.h"
#include "ElfReader.h"
#include "cl_autoptr_ex.h"

using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;

//
// ElfReaderDP- ElfReader delete policy for autoptr.
//
struct ElfReaderDP
{
    static void Delete(CLElfLib::CElfReader* pElfReader)
    {
        CLElfLib::CElfReader::Delete(pElfReader);
    }
};
typedef auto_ptr_ex<CLElfLib::CElfReader, ElfReaderDP> ElfReaderPtr;

DeviceProgram::DeviceProgram() : m_state(DEVICE_PROGRAM_INVALID),
m_bBuiltFromSource(false), m_bFECompilerSuccess(false), m_bIsClone(false), m_pDevice(nullptr),
m_deviceHandle(0), m_programHandle(0), m_parentProgramHandle(0), m_parentProgramContext(0),
m_uiBuildLogSize(0), m_szBuildLog(nullptr), m_emptyString('\0'), m_szBuildOptions(nullptr),
m_pBinaryBits(nullptr), m_uiBinaryBitsSize(0), m_clBinaryBitsType(CL_PROGRAM_BINARY_TYPE_NONE),
m_currentAccesses(0)
{
}

DeviceProgram::DeviceProgram(const Intel::OpenCL::Framework::DeviceProgram &dp) :
m_state(DEVICE_PROGRAM_INVALID), m_bBuiltFromSource(false),
m_bFECompilerSuccess(false), m_bIsClone(true), m_pDevice(nullptr), m_deviceHandle(0),
m_programHandle(0), m_parentProgramHandle(0), m_emptyString('\0'), m_szBuildOptions(nullptr),
m_pBinaryBits(nullptr), m_uiBinaryBitsSize(0), m_clBinaryBitsType(CL_PROGRAM_BINARY_TYPE_NONE),
m_currentAccesses(0)
{
    SetDevice(dp.m_pDevice);
    SetHandle(dp.m_parentProgramHandle);
    SetContext(dp.m_parentProgramContext);
    m_bBuiltFromSource   = dp.m_bBuiltFromSource;
    m_bFECompilerSuccess = dp.m_bFECompilerSuccess;
    //Todo: in the future it's a good idea to copy a completed binary from my source, or to add myself as an observer for its completion
    // Currently, force a real re-build of the program even if we're copying a built program
    // Thus, no use for m_bIsClone currently
    m_bIsClone = false;
}

DeviceProgram::~DeviceProgram()
{
    if (m_pBinaryBits)
    {
        delete[] m_pBinaryBits;
        m_pBinaryBits = nullptr;
        m_uiBinaryBitsSize = 0;
    }
    if (m_szBuildOptions)
    {
        delete[] m_szBuildOptions;
        m_szBuildOptions = nullptr;
    }
    if (m_szBuildLog != nullptr)
    {
        delete[] m_szBuildLog;
        m_szBuildLog  = nullptr;
        m_uiBuildLogSize = 0;
    }
    if (m_pDevice)
    {
        if (0 != m_programHandle)
        {
            m_pDevice->GetDeviceAgent()->clDevReleaseProgram(m_programHandle);
        }
    }
}

void DeviceProgram::SetDevice(const SharedPtr<FissionableDevice>& pDevice)
{
    m_pDevice = pDevice;
    //Must not give NULL ptr
    assert(m_pDevice);
    m_deviceHandle = m_pDevice->GetHandle();
}

cl_err_code DeviceProgram::SetBinary(size_t uiBinarySize, const unsigned char* pBinary, cl_int* piBinaryStatus)
{
    cl_prog_binary_type uiBinaryType;
    // Check if binary format is known by the runtime and device
    if (!CheckProgramBinary(uiBinarySize, pBinary, &uiBinaryType))
    {
        // Binary format is not supported by both runtime and device
        if (piBinaryStatus)
        {
            *piBinaryStatus = CL_INVALID_BINARY;
        }
        return CL_INVALID_BINARY;
    }

    cl_program_binary_type clBinaryType = CL_PROGRAM_BINARY_TYPE_NONE;

    switch( uiBinaryType)
    {
    case CL_PROG_BIN_COMPILED_SPIR:
        clBinaryType = CL_PROGRAM_BINARY_TYPE_INTERMEDIATE;
        break;
    case CL_PROG_BIN_COMPILED_LLVM:
        clBinaryType = CL_PROGRAM_BINARY_TYPE_COMPILED_OBJECT;
        break;
    case CL_PROG_BIN_LINKED_LLVM:
        clBinaryType = CL_PROGRAM_BINARY_TYPE_LIBRARY;
        break;
    case CL_PROG_BIN_EXECUTABLE_LLVM:
        clBinaryType = CL_PROGRAM_BINARY_TYPE_EXECUTABLE;
        break;
    default:
        if (piBinaryStatus)
        {
            *piBinaryStatus = CL_INVALID_BINARY;
        }
        return CL_INVALID_BINARY;
    }

    if (piBinaryStatus)
    {
        *piBinaryStatus = CL_SUCCESS;
    }

    // if binary is valid binary create program binary object and add it to the program object
    return SetBinaryInternal(uiBinarySize, pBinary, clBinaryType);
}

cl_err_code DeviceProgram::SetBinaryInternal(size_t uiBinarySize, const void *pBinary, cl_program_binary_type clBinaryType)
{
    if (m_uiBinaryBitsSize > 0)
    {
        assert(m_pBinaryBits);
        delete[] m_pBinaryBits;
    }

    m_uiBinaryBitsSize = uiBinarySize;
    m_pBinaryBits      = new char[uiBinarySize];

    if (!m_pBinaryBits)
    {
        m_uiBinaryBitsSize = 0;
        m_state = DEVICE_PROGRAM_INVALID;
        return CL_OUT_OF_HOST_MEMORY;
    }

    MEMCPY_S(m_pBinaryBits, m_uiBinaryBitsSize, pBinary, m_uiBinaryBitsSize);

    SetBinaryTypeInternal(clBinaryType);

    return CL_SUCCESS;
}

cl_err_code DeviceProgram::SetBinaryTypeInternal(cl_program_binary_type clBinaryType)
{
    m_clBinaryBitsType = clBinaryType;
    return CL_SUCCESS;
}

cl_err_code DeviceProgram::ClearBuildLogInternal()
{
    if (m_szBuildLog)
    {
        delete[] m_szBuildLog;
        m_szBuildLog = nullptr;
    }

    return CL_SUCCESS;
}

cl_err_code DeviceProgram::SetBuildLogInternal(const char* szBuildLog)
{
    size_t uiLogSize = strlen(szBuildLog) + 1;

    if (m_szBuildLog)
    {
        size_t uiNewBuildLogSize = m_uiBuildLogSize + uiLogSize - 1;   //no need for two NULL termination

        char* szNewBuildLog = new char[uiNewBuildLogSize];
        if (!szNewBuildLog)
        {
            return CL_OUT_OF_HOST_MEMORY;
        }

        STRCPY_S(szNewBuildLog, uiNewBuildLogSize, m_szBuildLog);
        STRCAT_S(szNewBuildLog, uiNewBuildLogSize, szBuildLog);

        m_uiBuildLogSize = uiNewBuildLogSize;
        delete[] m_szBuildLog;
        m_szBuildLog = szNewBuildLog;

        return CL_SUCCESS;
    }

    m_szBuildLog = new char[uiLogSize];
    if (!m_szBuildLog)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    STRCPY_S(m_szBuildLog, uiLogSize, szBuildLog);
    m_uiBuildLogSize = uiLogSize;

    return CL_SUCCESS;
}

cl_err_code DeviceProgram::SetBuildOptionsInternal(const char *szBuildOptions)
{
    if (m_szBuildOptions)
    {
        delete[] m_szBuildOptions;
        m_szBuildOptions = nullptr;
    }

    if (szBuildOptions)
    {
        size_t uiOptionLength = strlen(szBuildOptions) + 1;
        m_szBuildOptions = new char[uiOptionLength];
        if (!m_szBuildOptions)
        {
            return CL_OUT_OF_HOST_MEMORY;
        }
        MEMCPY_S(m_szBuildOptions, uiOptionLength, szBuildOptions, uiOptionLength);
    }

    return CL_SUCCESS;
}

const char* DeviceProgram::GetBuildOptionsInternal()
{
    return m_szBuildOptions;
}

cl_err_code DeviceProgram::SetStateInternal(EDeviceProgramState state)
{
    //TODO: maybe add state machine
    m_state = state;

    return CL_SUCCESS;
}

bool DeviceProgram::Acquire()
{
    if (0 == m_currentAccesses++)
    {
        return true;
    }
    m_currentAccesses--;
    return false;
}

cl_build_status DeviceProgram::GetBuildStatus() const
{
    switch (m_state)
    {
    default:
    case DEVICE_PROGRAM_INVALID:
        return CL_BUILD_ERROR;

    case DEVICE_PROGRAM_SOURCE:
    case DEVICE_PROGRAM_LOADED_IR:
    case DEVICE_PROGRAM_CUSTOM_BINARY:
    case DEVICE_PROGRAM_SPIRV:
        return CL_BUILD_NONE;

    case DEVICE_PROGRAM_FE_COMPILING:
    case DEVICE_PROGRAM_FE_LINKING:
    case DEVICE_PROGRAM_BE_BUILDING:
        return CL_BUILD_IN_PROGRESS;

    case DEVICE_PROGRAM_COMPILED:
    case DEVICE_PROGRAM_LINKED:
    case DEVICE_PROGRAM_BUILD_DONE:
    case DEVICE_PROGRAM_BUILTIN_KERNELS:
        return CL_BUILD_SUCCESS;
    }

    return CL_BUILD_ERROR;
}

cl_err_code DeviceProgram::GetBuildInfo(cl_program_build_info clParamName, size_t uiParamValueSize, void * pParamValue, size_t * puiParamValueSizeRet) const
{
    size_t uiParamSize = 0;
    void * pValue = nullptr;
    cl_build_status clBuildStatus;
    cl_program_binary_type clBinaryType;
    char emptyString = '\0';

    switch (clParamName)
    {
    case CL_PROGRAM_BUILD_STATUS:
        uiParamSize = sizeof(cl_build_status);
        clBuildStatus = GetBuildStatus();
        pValue = &clBuildStatus;
        break;

    case CL_PROGRAM_BINARY_TYPE:
        uiParamSize = sizeof(cl_program_binary_type);
        clBinaryType = GetBinaryTypeInternal();
        pValue = &clBinaryType;
        break;

    case CL_PROGRAM_BUILD_OPTIONS:
        if (nullptr != m_szBuildOptions)
        {
            uiParamSize = strlen(m_szBuildOptions) + 1;
            pValue = m_szBuildOptions;
            break;
        }
        uiParamSize = 1;
        pValue        = &emptyString;
        break;

    case CL_PROGRAM_BUILD_LOG:
        switch (m_state)
        {
        default:
        case DEVICE_PROGRAM_INVALID:
        case DEVICE_PROGRAM_SOURCE:
        case DEVICE_PROGRAM_LOADED_IR:
        case DEVICE_PROGRAM_CUSTOM_BINARY:
        case DEVICE_PROGRAM_BUILTIN_KERNELS:
        case DEVICE_PROGRAM_FE_COMPILING:
        case DEVICE_PROGRAM_FE_LINKING:
        case DEVICE_PROGRAM_BE_BUILDING:
            uiParamSize = 1;
            pValue        = &emptyString;
            break;

        case DEVICE_PROGRAM_COMPILED:
        case DEVICE_PROGRAM_LINKED:
        case DEVICE_PROGRAM_COMPILE_FAILED:
        case DEVICE_PROGRAM_LINK_FAILED:
            if (m_szBuildLog)
            {
                uiParamSize = m_uiBuildLogSize;
                pValue      = m_szBuildLog;
            }
            else
            {
                uiParamSize = 1;
                pValue      = &emptyString;
            }
            break;

        case DEVICE_PROGRAM_BUILD_DONE:
        case DEVICE_PROGRAM_BUILD_FAILED:
            {
                cl_dev_err_code clDevErr = CL_DEV_SUCCESS;
                // still need to append the FE build log
                // First of all calculate the size
                clDevErr = m_pDevice->GetDeviceAgent()->clDevGetBuildLog(m_programHandle, 0, nullptr, &uiParamSize);
                if CL_DEV_FAILED(clDevErr)
                {
                    if (CL_DEV_INVALID_PROGRAM == clDevErr)
                    {
                        return CL_INVALID_PROGRAM;
                    } else {
                        return CL_INVALID_VALUE;
                    }
                }
                if ( nullptr != m_szBuildLog )
                {
                    uiParamSize += m_uiBuildLogSize;
                    // Now we have reserved place for two '\0's. Remove one.
                    uiParamSize--;
                }
                if (nullptr != pParamValue && uiParamSize > uiParamValueSize)
                {
                    return CL_INVALID_VALUE;
                }

                // if pParamValue == NULL return param value size
                if (nullptr != puiParamValueSizeRet)
                {
                    *puiParamValueSizeRet = uiParamSize;
                }

                // get the actual log
                if (nullptr != pParamValue)
                {
                    if ( nullptr != m_szBuildLog )
                    {
                        //Copy the FE log minus the terminating NULL
                        MEMCPY_S(pParamValue, uiParamValueSize, m_szBuildLog, m_uiBuildLogSize - 1);
                        // and let the device write the rest of the log
                        uiParamSize -= (m_uiBuildLogSize - 1);

                        clDevErr = m_pDevice->GetDeviceAgent()->clDevGetBuildLog(m_programHandle, uiParamSize, ((char*)pParamValue) + m_uiBuildLogSize - 1, nullptr);
                    }
                    else
                    {
                        clDevErr = m_pDevice->GetDeviceAgent()->clDevGetBuildLog(m_programHandle, uiParamSize, (char*)pParamValue, nullptr);
                    }
                }
                if CL_DEV_FAILED(clDevErr)
                {
                    if (CL_DEV_INVALID_PROGRAM == clDevErr)
                    {
                        return CL_INVALID_PROGRAM;
                    } else {
                        return CL_INVALID_VALUE;
                    }
                }
                return CL_SUCCESS;
                break;
            }
        }

        break;

    case CL_PROGRAM_BUILD_GLOBAL_VARIABLE_TOTAL_SIZE:
        {
            if (sizeof(size_t) > uiParamValueSize)
            {
                return CL_INVALID_VALUE;
            }
            cl_dev_err_code clDevErr = m_pDevice->GetDeviceAgent()->clDevGetGlobalVariableTotalSize(m_programHandle, (size_t*)pParamValue);
            if CL_DEV_FAILED(clDevErr)
            {
                if (CL_DEV_INVALID_PROGRAM == clDevErr)
                {
                    return CL_INVALID_PROGRAM;
                } else {
                    return CL_INVALID_VALUE;
                }
            }
            if (nullptr != puiParamValueSizeRet)
            {
                *puiParamValueSizeRet = sizeof(size_t);
            }
            return CL_SUCCESS;
            break;
        }

    default:
        return CL_INVALID_VALUE;
    }

    if (nullptr != pParamValue && uiParamSize > uiParamValueSize)
    {
        return CL_INVALID_VALUE;
    }

    // if pParamValue == NULL return only param value size
    if (nullptr != puiParamValueSizeRet)
    {
        *puiParamValueSizeRet = uiParamSize;
    }

    if (nullptr != pParamValue && uiParamSize > 0)
    {
        MEMCPY_S(pParamValue, uiParamValueSize, pValue, uiParamSize);
    }

    return CL_SUCCESS;
}

cl_err_code DeviceProgram::GetBinary(size_t uiBinSize, void * pBin, size_t * puiBinSizeRet)
{
    if (nullptr == pBin && nullptr == puiBinSizeRet)
    {
        return CL_INVALID_VALUE;
    }

    if (uiBinSize > 0 && nullptr == pBin)
    {
        return CL_INVALID_VALUE;
    }

    switch(m_state)
    {
    case DEVICE_PROGRAM_BUILD_DONE:
        //Return the resultant compiled binaries
        return m_pDevice->GetDeviceAgent()->clDevGetProgramBinary(m_programHandle, uiBinSize, pBin, puiBinSizeRet);

    case DEVICE_PROGRAM_COMPILED:
    case DEVICE_PROGRAM_LINKED:
    case DEVICE_PROGRAM_LOADED_IR:
    case DEVICE_PROGRAM_CUSTOM_BINARY:
        if ( nullptr == pBin)
        {
            assert(m_uiBinaryBitsSize <= CL_MAX_UINT32);
            *puiBinSizeRet = (cl_uint)m_uiBinaryBitsSize;
            return CL_SUCCESS;
        }
        if (uiBinSize < m_uiBinaryBitsSize)
        {
            return CL_INVALID_VALUE;
        }
        MEMCPY_S(pBin, uiBinSize, m_pBinaryBits, m_uiBinaryBitsSize);
        return CL_SUCCESS;

    case DEVICE_PROGRAM_SOURCE:
        // Program source loaded but hasn't been built yet, so no binary to return.
        // Return success and zero binary size to be consistent with GEN.
        *puiBinSizeRet = 0;
        return CL_SUCCESS;

    default:
        if ( nullptr == pBin)    // When query for binary size and it's not available, we should return 0
        {
            *puiBinSizeRet = 0;
            return CL_SUCCESS;
        }
        // In every other case, we have nothing intelligent to return
        // Todo: see what I should return here
        return CL_INVALID_OPERATION;
    }
}

bool DeviceProgram::IsBinaryAvailable(cl_program_binary_type requestedType) const
{
    cl_program_binary_type binaryType = CL_PROGRAM_BINARY_TYPE_NONE;

    if (CL_BUILD_SUCCESS == GetBuildStatus() &&
        CL_SUCCESS == GetBuildInfo(CL_PROGRAM_BINARY_TYPE,
                                sizeof(binaryType),
                                &binaryType,
                                nullptr) &&
        binaryType == requestedType)
    {
        return true;
    }
    return false;
}

cl_err_code DeviceProgram::GetNumKernels(cl_uint* pszNumKernels)
{
    assert(pszNumKernels);
    return m_pDevice->GetDeviceAgent()->clDevGetProgramKernels(m_programHandle, 0, nullptr, pszNumKernels);
}

cl_err_code DeviceProgram::GetKernelNames(char **ppNames, size_t *pszNameSizes, size_t szNumNames)
{
    cl_uint         numKernels;
    cl_err_code    errRet     = CL_SUCCESS;
    cl_dev_kernel* devKernels = new cl_dev_kernel[szNumNames];

    if (nullptr == devKernels)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }
    if (!pszNameSizes)
    {
        delete[] devKernels;
        return CL_INVALID_VALUE;
    }
    assert(szNumNames <= CL_MAX_UINT32);
    errRet = m_pDevice->GetDeviceAgent()->clDevGetProgramKernels(m_programHandle, (cl_uint)szNumNames, devKernels, &numKernels);
    if (CL_FAILED(errRet))
    {
        delete[] devKernels;
        return errRet;
    }
    assert(numKernels == szNumNames);

    if (nullptr == ppNames)
    {
        for (size_t i = 0; i < numKernels; ++i)
        {
            errRet = m_pDevice->GetDeviceAgent()->clDevGetKernelInfo(devKernels[i], CL_DEV_KERNEL_NAME, 0, nullptr, 0, nullptr, pszNameSizes + i);
            if (CL_FAILED(errRet))
            {
                delete[] devKernels;
                return errRet;
            }
        }
        delete[] devKernels;
        return CL_SUCCESS;
    }

    for (size_t i = 0; i < numKernels; ++i)
    {
        size_t kernelNameSize;
        errRet = m_pDevice->GetDeviceAgent()->clDevGetKernelInfo(devKernels[i], CL_DEV_KERNEL_NAME, 0, nullptr, pszNameSizes[i], ppNames[i], &kernelNameSize);
        if (CL_FAILED(errRet))
        {
            delete[] devKernels;
            return errRet;
        }
        assert(kernelNameSize == pszNameSizes[i]);
    }

    delete[] devKernels;

    return CL_SUCCESS;
}

cl_err_code DeviceProgram::GetAutorunKernelsNames(
    std::vector<std::string> &vsNames)
{
    cl_uint        numKernels = 0;
    cl_err_code    errRet     = CL_SUCCESS;

    try
    {
        auto devAgent = m_pDevice->GetDeviceAgent();
        errRet = devAgent->clDevGetProgramKernels(m_programHandle, 0, nullptr,
            &numKernels);
        if (CL_FAILED(errRet))
        {
            return errRet;
        }

        if (numKernels > 0)
        {
            std::vector<cl_dev_kernel*> devKernels(numKernels);

            errRet = devAgent->clDevGetProgramKernels(m_programHandle,
                numKernels, (cl_dev_kernel*)&devKernels.front(), nullptr);
            if (CL_FAILED(errRet))
            {
                return errRet;
            }

            for (size_t i = 0; i < numKernels; ++i)
            {
                cl_bool isAutorun = CL_FALSE;
                errRet = devAgent->clDevGetKernelInfo(devKernels[i],
                    CL_DEV_KERNEL_IS_AUTORUN, 0, nullptr, sizeof(cl_bool),
                    &isAutorun, nullptr);
                if (CL_FAILED(errRet))
                {
                    return errRet;
                }

                if (isAutorun)
                {
                    std::string name;
                    size_t size;
                    errRet = devAgent->clDevGetKernelInfo(devKernels[i],
                        CL_DEV_KERNEL_NAME, 0, nullptr, 0, nullptr, &size);
                    if (CL_FAILED(errRet))
                    {
                        return errRet;
                    }

                    name.resize(size);
                    errRet = devAgent->clDevGetKernelInfo(devKernels[i],
                        CL_DEV_KERNEL_NAME, 0, nullptr, sizeof(char) * size,
                        &name[0], nullptr);
                    if (CL_FAILED(errRet))
                    {
                        return errRet;
                    }
                    vsNames.push_back(name);
                }
            }
        }
    }
    catch (const std::bad_alloc &e)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    return CL_SUCCESS;
}

cl_err_code DeviceProgram::SetDeviceHandleInternal(cl_dev_program programHandle)
{
    if (m_pDevice)
    {
        if (0 != m_programHandle)
        {
            m_pDevice->GetDeviceAgent()->clDevReleaseProgram(m_programHandle);
        }
    }
    m_programHandle = programHandle;
    return CL_SUCCESS;
}

bool DeviceProgram::CheckProgramBinary(size_t uiBinSize, const void *pBinary, cl_prog_binary_type* pBinaryType)
{
    //check if it is Binary object
    if( CLElfLib::CElfReader::IsValidElf64((const char*)pBinary, uiBinSize))
    {
        if( pBinaryType )
        {
            ElfReaderPtr pReader(CLElfLib::CElfReader::Create((const char*)pBinary, uiBinSize));
            switch( pReader->GetElfHeader()->Type)
            {
               case CLElfLib::EH_TYPE_OPENCL_OBJECTS   :
                   *pBinaryType = CL_PROG_BIN_COMPILED_LLVM;
                   break;
               case CLElfLib::EH_TYPE_OPENCL_LIBRARY   :
                   *pBinaryType = CL_PROG_BIN_LINKED_LLVM;
                   break;
               case CLElfLib::EH_TYPE_OPENCL_EXECUTABLE:
                   *pBinaryType = CL_PROG_BIN_EXECUTABLE_LLVM;
                   return CL_DEV_SUCCEEDED(m_pDevice->GetDeviceAgent()->clDevCheckProgramBinary(uiBinSize, pBinary));
               case CLElfLib::EH_TYPE_OPENCL_LINKED_OBJECTS:
                   *pBinaryType = CL_PROG_BIN_EXECUTABLE_LLVM;
                   break;
               case CLElfLib::EH_TYPE_NONE             :
               case CLElfLib::EH_TYPE_RELOCATABLE      :
               case CLElfLib::EH_TYPE_EXECUTABLE       :
               case CLElfLib::EH_TYPE_DYNAMIC          :
               case CLElfLib::EH_TYPE_CORE             :
               case CLElfLib::EH_TYPE_OPENCL_SOURCE    :
               default:
                   return false;
            }
        }
        return true;
    }

    if (sizeof(_CL_LLVM_BITCODE_MASK_) > uiBinSize )
    {
        return false;
    }

    //check if it is LLVM IR object
    if ( !memcmp(_CL_LLVM_BITCODE_MASK_, pBinary, sizeof(_CL_LLVM_BITCODE_MASK_) - 1) )
    {
        if( pBinaryType )
            *pBinaryType = CL_PROG_BIN_COMPILED_SPIR;

        return CL_DEV_SUCCEEDED(m_pDevice->GetDeviceAgent()->clDevCheckProgramBinary(uiBinSize, pBinary));
    }

    //check if it is SPIRV object
    if (sizeof(_CL_SPIRV_MAGIC_NUMBER_) < uiBinSize && _CL_SPIRV_MAGIC_NUMBER_ == ((unsigned int*)pBinary)[0])
    {
        if( pBinaryType )
            *pBinaryType = CL_PROG_BIN_COMPILED_SPIRV;

        return true;
    }

    return false;
}
