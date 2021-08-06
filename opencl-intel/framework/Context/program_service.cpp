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

#include "program_service.h"
#include "Context.h"
#include "fe_compiler.h"
#include "program.h"
#include "program_builtin_kernels.h"
#include "framework_proxy.h"
#include "events_manager.h"
#include "cl_device_api.h"
#include "cl_shared_ptr.hpp"
#include "Device.h"
#include "cl_user_logger.h"
#include "ElfWriter.h"
#include "cl_autoptr_ex.h"
#include <cl_local_array.h>
#include "program_with_il.h"
#include <cl_synch_objects.h>
#include "llvm/Support/Compiler.h" // LLVM_FALLTHROUGH

#include <string>

using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::Utils;

//
// ElfWriterDP- ElfWriter delete policy for autoptr.
//
struct ElfWriterDP
{
    static void Delete(CLElfLib::CElfWriter* pElfWriter)
    {
        CLElfLib::CElfWriter::Delete(pElfWriter);
    }
};
typedef auto_ptr_ex<CLElfLib::CElfWriter, ElfWriterDP> ElfWriterPtr;

// In this file we use CompileTask, LinkTask and PostBuildTask as building blocks to create a general build tree.
//
// for a full BuildProgram we use CompileTask -> LinkTask -> DeviceBuildTask -> PostBuildTask
// for a CompileProgram we use CompileTask -> PostBuildTask
// for a LinkProgram we use LinkTask -> DeviceBuildTask -> PostBuildTask
//
// where CompileTask and LinkTask and DeviceBuildTask are created for each device and dependent
// only on the previouse task for the same device.
// PostBuildTask is created once for all the devices and dependent on the previous tasks on all the devices.
//
// Since we need the PostBuildTask to be executed even when a build fail in order to clean up all the used resources.
// All the tasks return CL_BUILD_SUCCESS even on error and use program state to notify error.

BuildTask::BuildTask(_cl_context_int* context,
                        const SharedPtr<Program>& pProg,
                        const ConstSharedPtr<FrontEndCompiler>& pFECompiler) : BuildEvent(context), m_pProg(pProg), m_pFECompiler(pFECompiler)
{
}

BuildTask::~BuildTask()
{
}

long BuildTask::Release()
{
    return 0;
}

void BuildTask::DoneWithDependencies(const SharedPtr<OclEvent>& pEvent)
{
    Launch();
    BuildEvent::DoneWithDependencies(pEvent);
}

bool BuildTask::Launch()
{
    return FrameworkProxy::Instance()->ExecuteImmediate(SharedPtr<BuildTask>(this));
}

void BuildTask::SetComplete(cl_int returnCode)
{
    // We must assing NULL to shared pointers in order to remove reference count and avoid
    // a raise condition during program release
    m_pProg = NULL;
    m_pFECompiler = NULL;
    BuildEvent::SetComplete(returnCode);
}

Intel::OpenCL::Utils::OclMutex CompileTask::m_compileMtx;
Intel::OpenCL::Utils::OclMutex LinkTask::m_linkMtx;
Intel::OpenCL::Utils::OclMutex DeviceBuildTask::m_deviceBuildMtx;

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

CompileTask::CompileTask(_cl_context_int *context,
                         const SharedPtr<Program> &pProg,
                         const ConstSharedPtr<FrontEndCompiler> &pFECompiler,
                         DeviceProgram *pDeviceProgram,
                         unsigned int uiNumHeaders, const char **pszHeaders,
                         const char **pszHeadersNames, const char *szOptions)
    : BuildTask(context, pProg, pFECompiler), m_pDeviceProgram(pDeviceProgram),
      m_uiNumHeaders(uiNumHeaders), m_pszHeaders(pszHeaders),
      m_pszHeadersNames(pszHeadersNames), m_sOptions(szOptions) {}

CompileTask::~CompileTask()
{
}

bool CompileTask::Execute()
{
    auto_ptr_ex<char, ArrayDP<char> > pOutBinary;
    auto_ptr_ex<char, ArrayDP<char> > szOutCompileLog;
    size_t uiOutBinarySize = 0;

    m_pDeviceProgram->SetBuildLogInternal("Compilation started\n");
    m_pDeviceProgram->SetStateInternal(DEVICE_PROGRAM_FE_COMPILING);

    // check if program contains already compiled data
    if (m_pDeviceProgram->GetBinaryTypeInternal() == CL_PROGRAM_BINARY_TYPE_COMPILED_OBJECT ||
        m_pDeviceProgram->GetBinaryTypeInternal() == CL_PROGRAM_BINARY_TYPE_LIBRARY)
    {
        // we have spir binary / SPV-IR, no need for FE compilation
        m_pDeviceProgram->SetBuildLogInternal("Compilation done\n");
        SetComplete(CL_BUILD_SUCCESS);
        return true;
    }

    bool IsSPIR = m_pDeviceProgram->GetBinaryTypeInternal() ==
                  CL_PROGRAM_BINARY_TYPE_INTERMEDIATE;
    const char* szSource = m_pProg->GetSourceInternal();
    if (NULL == szSource && !IsSPIR)
    {
        // not spir and no source
        m_pDeviceProgram->SetBuildLogInternal("Compilation failed\n");
        m_pDeviceProgram->SetStateInternal(DEVICE_PROGRAM_COMPILE_FAILED);
        SetComplete(CL_BUILD_SUCCESS);
        return true;
    }

    SharedPtr<ProgramWithIL> pIL = m_pProg.DynamicCast<ProgramWithIL>();

    {
        // The frontend compiler is not thread safe
        OclAutoMutex lockCompile(&m_compileMtx);
        if(pIL)
        {
            unsigned int binarySize = pIL->GetSize();
            assert(szSource != NULL && "Invalid source code");
            m_pFECompiler->ParseSpirv(szSource,
                                      binarySize,
                                      m_sOptions.c_str(),
                                      pIL->GetSpecConstCount(),
                                      pIL->GetSpecConstIds(),
                                      pIL->GetSpecConstValues(),
                                      pOutBinary.getOutPtr(),
                                      &uiOutBinarySize,
                                      szOutCompileLog.getOutPtr());
        }
        else
        {
            if (IsSPIR)
            {
                m_pFECompiler->MaterializeSPIR(
                    m_pDeviceProgram->GetBinaryInternal(),
                    m_pDeviceProgram->GetBinarySizeInternal(), pOutBinary.getOutPtr(),
                    &uiOutBinarySize, szOutCompileLog.getOutPtr());
            }
            else
            {
                assert(szSource != NULL && "Invalid source code");
                m_pFECompiler->CompileProgram(szSource,
                                              m_uiNumHeaders,
                                              m_pszHeaders,
                                              m_pszHeadersNames,
                                              m_sOptions.c_str(),
                                              m_pProg->GetContext()->IsFPGAEmulator(),
                                              m_pProg->GetContext()->IsEyeQEmulator(),
                                              pOutBinary.getOutPtr(),
                                              &uiOutBinarySize,
                                              szOutCompileLog.getOutPtr());
            }
        }
    }

    if (NULL != szOutCompileLog.get())
    {
        m_pDeviceProgram->SetBuildLogInternal(szOutCompileLog.get());
    }

    if (0 == uiOutBinarySize)
    {
        assert( NULL == pOutBinary.get());
        //Build failed
        m_pDeviceProgram->SetBuildLogInternal("Compilation failed\n");
        m_pDeviceProgram->SetStateInternal(DEVICE_PROGRAM_COMPILE_FAILED);
        SetComplete(CL_BUILD_SUCCESS);
        return true;
    }

    //compile succeeded
    ElfWriterPtr pElfWriter(CLElfLib::CElfWriter::Create( CLElfLib::EH_TYPE_OPENCL_OBJECTS,
                                                          CLElfLib::EH_MACHINE_NONE,
                                                          CLElfLib::EH_FLAG_NONE ));
    CLElfLib::SSectionNode sectionNode;
    sectionNode.Name = ".ocl.ir";
    sectionNode.pData = pOutBinary.get();
    sectionNode.DataSize = uiOutBinarySize;
    sectionNode.Flags = 0;
    sectionNode.Type = CLElfLib::SH_TYPE_OPENCL_LLVM_BINARY;

    if( pElfWriter->AddSection( &sectionNode ) != CLElfLib::SUCCESS)
    {
        //Build failed
        m_pDeviceProgram->SetBuildLogInternal("Compilation failed\n");
        m_pDeviceProgram->SetStateInternal(DEVICE_PROGRAM_COMPILE_FAILED);
        SetComplete(CL_BUILD_SUCCESS);
        return true;
    }

    auto_ptr_ex<char, ArrayDP<char> > pBinary;
    unsigned int uiBinarySize = 0;

    if( pElfWriter->ResolveBinary( *pBinary.getOutPtr(), uiBinarySize ) != CLElfLib::SUCCESS )
    {
        //Build failed
        m_pDeviceProgram->SetBuildLogInternal("Compilation failed\n");
        m_pDeviceProgram->SetStateInternal(DEVICE_PROGRAM_COMPILE_FAILED);
        SetComplete(CL_BUILD_SUCCESS);
        return true;
    }

    pBinary.reset(new char[uiBinarySize]);
    if( NULL == pBinary.get())
    {
        //Build failed
        m_pDeviceProgram->SetBuildLogInternal("Compilation failed\n");
        m_pDeviceProgram->SetStateInternal(DEVICE_PROGRAM_COMPILE_FAILED);
        SetComplete(CL_BUILD_SUCCESS);
        return true;
    }

    if(pElfWriter->ResolveBinary( *pBinary.getOutPtr(), uiBinarySize ) != CLElfLib::SUCCESS )
    {
        //Build failed
        m_pDeviceProgram->SetBuildLogInternal("Compilation failed\n");
        m_pDeviceProgram->SetStateInternal(DEVICE_PROGRAM_COMPILE_FAILED);
        SetComplete(CL_BUILD_SUCCESS);
        return true;
    }

    // Compilation is done, set used compile options so that program will not be
    // recompiled if same options are passed. Also this is necessary to
    // propagate "-cl-opt-disable" and "-g" options to build task which is
    // actually peforming all backend optimizations depending on provided
    // options.
    m_pDeviceProgram->SetBuildOptionsInternal(m_sOptions.c_str());
    m_pDeviceProgram->SetBuildLogInternal("Compilation done\n");
    m_pDeviceProgram->SetBinaryInternal(uiBinarySize, pBinary.get(), CL_PROGRAM_BINARY_TYPE_COMPILED_OBJECT);
    SetComplete(CL_BUILD_SUCCESS);
    return true;
}

void CompileTask::Cancel()
{
    SetComplete(CL_BUILD_ERROR);
}

LinkTask::LinkTask(_cl_context_int*             context,
                   const SharedPtr<Program>&    pProg,
                   const ConstSharedPtr<FrontEndCompiler>&  pFECompiler,
                   DeviceProgram*               pDeviceProgram,
                   SharedPtr<Program>*          ppPrograms,
                   unsigned int                 uiNumPrograms,
                   const char*                  szOptions) :
BuildTask(context, pProg, pFECompiler),
m_pDeviceProgram(pDeviceProgram), m_ppPrograms(ppPrograms),
m_uiNumPrograms(uiNumPrograms), m_sOptions(szOptions)
{
}

LinkTask::~LinkTask()
{
}

bool LinkTask::Execute()
{
    auto_ptr_ex<char, ArrayDP<char> > pOutBinary;
    size_t uiOutBinarySize = 0;
    std::vector<char> linkLog;
    bool bIsLibrary = false;

    // if previous task failed don't continue execution
    if (m_pDeviceProgram->GetStateInternal() == DEVICE_PROGRAM_COMPILE_FAILED)
    {
        SetComplete(CL_BUILD_SUCCESS);
        return true;
    }

    m_pDeviceProgram->SetBuildLogInternal("Linking started\n");
    m_pDeviceProgram->SetStateInternal(DEVICE_PROGRAM_FE_LINKING);

    bool useInputPrograms = true;
    if (0 == m_uiNumPrograms)
    {
        m_uiNumPrograms = 1;
        useInputPrograms = false;
    }

    clLocalArray<const void*> arrBinaries(m_uiNumPrograms);
    clLocalArray<size_t> arrBinariesSizes(m_uiNumPrograms);

    if (useInputPrograms)
    {
        // user provided input programs
        for (unsigned int i = 0; i < m_uiNumPrograms; ++i)
        {
            arrBinaries[i] = m_ppPrograms[i]->GetBinaryInternal(m_pDeviceProgram->GetDeviceId());
            arrBinariesSizes[i] = m_ppPrograms[i]->GetBinarySizeInternal(m_pDeviceProgram->GetDeviceId());
        }
    }
    else
    {
        // link the binary that is already in our deviceProgram
        arrBinaries[0] = m_pDeviceProgram->GetBinaryInternal();
        arrBinariesSizes[0] = m_pDeviceProgram->GetBinarySizeInternal();
    }

    {
        // The frontend compiler is not thread safe
        OclAutoMutex lockLink(&m_linkMtx);

        m_pFECompiler->LinkProgram(arrBinaries,
                                   m_uiNumPrograms,
                                   arrBinariesSizes,
                                   m_sOptions.c_str(),
                                   pOutBinary.getOutPtr(),
                                   &uiOutBinarySize,
                                   linkLog,
                                   &bIsLibrary);
    }

    if (0 == uiOutBinarySize)
    {
        assert( NULL == pOutBinary.get());
        //Build failed
        m_pDeviceProgram->SetStateInternal(DEVICE_PROGRAM_LINK_FAILED);

        if (!linkLog.empty())
        {
            m_pDeviceProgram->SetBuildLogInternal(&linkLog[0]);
        }

        m_pDeviceProgram->SetBuildLogInternal("Linking failed\n");

        SetComplete(CL_BUILD_SUCCESS);
        return true;
    }

    //Else link succeeded

    ElfWriterPtr pElfWriter(CLElfLib::CElfWriter::Create( bIsLibrary? CLElfLib::EH_TYPE_OPENCL_LIBRARY : CLElfLib::EH_TYPE_OPENCL_LINKED_OBJECTS,
                                                          CLElfLib::EH_MACHINE_NONE,
                                                          CLElfLib::EH_FLAG_NONE ));
    CLElfLib::SSectionNode sectionNode;
    sectionNode.Name = ".ocl.ir";
    sectionNode.pData = pOutBinary.get();
    sectionNode.DataSize = uiOutBinarySize;
    sectionNode.Flags = 0;
    sectionNode.Type = CLElfLib::SH_TYPE_OPENCL_LLVM_BINARY;

    if( pElfWriter->AddSection( &sectionNode ) != CLElfLib::SUCCESS)
    {
        //Build failed
        m_pDeviceProgram->SetStateInternal(DEVICE_PROGRAM_LINK_FAILED);
        m_pDeviceProgram->SetBuildLogInternal("Linking failed\n");
        SetComplete(CL_BUILD_SUCCESS);
        return true;
    }

    auto_ptr_ex<char, ArrayDP<char> > pBinary;
    unsigned int uiBinarySize = 0;

    if( pElfWriter->ResolveBinary( *pBinary.getOutPtr(), uiBinarySize ) != CLElfLib::SUCCESS )
    {
        //Build failed
        m_pDeviceProgram->SetStateInternal(DEVICE_PROGRAM_LINK_FAILED);
        m_pDeviceProgram->SetBuildLogInternal("Linking failed\n");
        SetComplete(CL_BUILD_SUCCESS);
        return true;
    }

    pBinary.reset(new char[uiBinarySize]);
    if( NULL == pBinary.get())
    {
        //Build failed
        m_pDeviceProgram->SetStateInternal(DEVICE_PROGRAM_LINK_FAILED);
        m_pDeviceProgram->SetBuildLogInternal("Linking failed\n");
        SetComplete(CL_BUILD_SUCCESS);
        return true;
    }

    if(pElfWriter->ResolveBinary( *pBinary.getOutPtr(), uiBinarySize ) != CLElfLib::SUCCESS )
    {
        //Build failed
        m_pDeviceProgram->SetStateInternal(DEVICE_PROGRAM_LINK_FAILED);
        m_pDeviceProgram->SetBuildLogInternal("Linking failed\n");
        SetComplete(CL_BUILD_SUCCESS);
        return true;
    }

    // Update binary
    if ( bIsLibrary )
    {
        m_pDeviceProgram->SetBinaryInternal(uiBinarySize, pBinary.get(), CL_PROGRAM_BINARY_TYPE_LIBRARY);
    }
    else // executable
    {
        m_pDeviceProgram->SetBinaryInternal(uiBinarySize, pBinary.get(), CL_PROGRAM_BINARY_TYPE_EXECUTABLE);
    }

    m_pDeviceProgram->SetBuildLogInternal("Linking done\n");
    SetComplete(CL_BUILD_SUCCESS);
    return true;
}

void LinkTask::Cancel()
{
    SetComplete(CL_BUILD_ERROR);
}

DeviceBuildTask::DeviceBuildTask(_cl_context_int*           context,
                                const SharedPtr<Program>&   pProg,
                                DeviceProgram*              pDeviceProgram,
                                const char*                 szOptions) :
BuildTask(context, pProg, NULL),
m_pDeviceProgram(pDeviceProgram), m_sOptions(szOptions)
{
    if (m_pProg->GetContext()->IsEyeQEmulator())
    {
        static const std::string unsafe_math_strs[] = {
            "-cl-mad-enable",
            "-cl-no-signed-zeros",
            "-cl-unsafe-math-optimizations",
            "-cl-finite-math-only",
            "-cl-fast-relaxed-math"
        };
        for (unsigned int i = 0;
             i < sizeof(unsafe_math_strs)/sizeof(unsafe_math_strs[0]);
             ++i)
        {
            std::string const &unsafe_math_str = unsafe_math_strs[i];
            for (std::size_t unsafe_math_str_pos = m_sOptions.find(unsafe_math_str) ;
                 unsafe_math_str_pos != std::string::npos ;
                 unsafe_math_str_pos = m_sOptions.find(unsafe_math_str))
            {
                m_sOptions.erase(unsafe_math_str_pos, unsafe_math_str.length());
            }
        }
        static const std::string denorms_are_zero = "-cl-denorms-are-zero";
        if (m_sOptions.find(denorms_are_zero) == std::string::npos)
        {
            m_sOptions += " ";
            m_sOptions += denorms_are_zero;
        }
    }
}

DeviceBuildTask::~DeviceBuildTask()
{
}

bool DeviceBuildTask::Execute()
{
    const char*         pBinary         = NULL;
    size_t              uiBinarySize    = 0;
    cl_dev_program      programHandle   = NULL;
    IOCLDeviceAgent*    pDeviceAgent    = NULL;
    cl_build_status     build_status;

    EDeviceProgramState state = m_pDeviceProgram->GetStateInternal();
    // If previous stages failed don't contue execution
    if ( (DEVICE_PROGRAM_COMPILE_FAILED == state) || (DEVICE_PROGRAM_LINK_FAILED == state) )
    {
        SetComplete(CL_BUILD_SUCCESS);
        return true;
    }

    // If we are building library no need for device build
    if (m_pDeviceProgram->GetBinaryTypeInternal() != CL_PROGRAM_BINARY_TYPE_EXECUTABLE)
    {
        SetComplete(CL_BUILD_SUCCESS);
        return true;
    }

    m_pDeviceProgram->SetBuildLogInternal("Device build started\n");
    m_pDeviceProgram->SetStateInternal(DEVICE_PROGRAM_BE_BUILDING);

    pDeviceAgent = m_pDeviceProgram->GetDevice()->GetDeviceAgent();
    uiBinarySize = m_pDeviceProgram->GetBinarySizeInternal();
    pBinary      = m_pDeviceProgram->GetBinaryInternal();

    cl_dev_err_code err = pDeviceAgent->clDevCheckProgramBinary(uiBinarySize, pBinary);
    if (CL_DEV_SUCCESS != err)
    {
        //Build failed
        m_pDeviceProgram->SetStateInternal(DEVICE_PROGRAM_BUILD_FAILED);
        m_pDeviceProgram->SetBuildLogInternal("Binary is not supported by the device\n");
        SetComplete(CL_BUILD_SUCCESS);
        return true;
    }

    {
        // The backend compiler is not thread safe, furthermore we need
        // to guard not only clDevBuildProgram but clDevCreatePRogram also.
        // Likely they shared some internal state.
        // Sporadic assertions in vectorizer happens without the lock.
        OclAutoMutex lockBuild(&m_deviceBuildMtx);

        err = pDeviceAgent->clDevCreateProgram(uiBinarySize, pBinary, CL_DEV_BINARY_COMPILER, &programHandle);
        if (CL_DEV_SUCCESS != err)
        {
            //Build failed
            m_pDeviceProgram->SetStateInternal(DEVICE_PROGRAM_BUILD_FAILED);
            m_pDeviceProgram->SetBuildLogInternal("Failed to create device program\n");
            SetComplete(CL_BUILD_SUCCESS);
            return true;
        }

        m_pDeviceProgram->SetDeviceHandleInternal(programHandle);

        std::string OptionsLog =
            std::string("Options used by backend compiler: ") + m_sOptions +
            std::string("\n");
        m_pDeviceProgram->SetBuildLogInternal(OptionsLog.c_str());

        err = pDeviceAgent->clDevBuildProgram(programHandle, m_sOptions.c_str(), &build_status);
        if (CL_DEV_SUCCESS != err)
        {
            m_pDeviceProgram->SetStateInternal(DEVICE_PROGRAM_BUILD_FAILED);
            m_pDeviceProgram->SetBuildLogInternal("Failed to build device program\n");
            SetComplete(CL_BUILD_SUCCESS);
            return true;
        }
    }

    assert( (CL_BUILD_ERROR == build_status || CL_BUILD_SUCCESS == build_status) && "Unknown build status returned by the device agent" );

    if (CL_BUILD_ERROR == build_status)
    {
        m_pDeviceProgram->SetStateInternal(DEVICE_PROGRAM_BUILD_FAILED);
        m_pDeviceProgram->SetBuildLogInternal("Failed to build device program\n");
        SetComplete(CL_BUILD_SUCCESS);
        return true;
    }

    m_pDeviceProgram->SetBuildLogInternal("Device build done\n");
    SetComplete(CL_BUILD_SUCCESS);
    return true;
}

void DeviceBuildTask::Cancel()
{
    SetComplete(CL_BUILD_ERROR);
}

PostBuildTask::PostBuildTask(_cl_context_int*           context,
                             const SharedPtr<Program>&  pProg,
                             cl_uint                    num_devices,
                             DeviceProgram**            ppDevicePrograms,
                             unsigned int               uiNumHeaders,
                             SharedPtr<Program>*        ppHeaders,
                             char**                     pszHeadersNames,
                             unsigned int               uiNumBinaries,
                             SharedPtr<Program>*        ppBinaries,
                             pfnNotifyBuildDone         pfn_notify,
                             void*                      user_data) :
BuildTask(context, pProg, NULL),
m_num_devices(num_devices), m_ppDevicePrograms(ppDevicePrograms), m_uiNumHeaders(uiNumHeaders), m_ppHeaders(ppHeaders),
m_pszHeadersNames(pszHeadersNames), m_uiNumBinaries(uiNumBinaries), m_ppBinaries(ppBinaries),
m_pfn_notify(pfn_notify), m_user_data(user_data)
{
}

PostBuildTask::~PostBuildTask()
{
}

bool PostBuildTask::Execute()
{
    // Unacquire input binaries
    for (unsigned int binIndex = 0; binIndex < m_uiNumBinaries; ++binIndex)
    {
        for (unsigned int devIndex = 0; devIndex < m_num_devices; ++devIndex)
        {
            cl_device_id deviceId = m_ppDevicePrograms[devIndex]->GetDeviceId();
            m_ppBinaries[binIndex]->Unacquire(deviceId);
        }
    }

    bool bBuildFailed = false;

    for (unsigned int i = 0; i < m_num_devices; ++i)
    {
        EDeviceProgramState currState = m_ppDevicePrograms[i]->GetStateInternal();

        switch (currState)
        {
        case DEVICE_PROGRAM_FE_COMPILING:
            m_ppDevicePrograms[i]->SetStateInternal(DEVICE_PROGRAM_COMPILED);
            break;

        case DEVICE_PROGRAM_FE_LINKING:
            m_ppDevicePrograms[i]->SetStateInternal(DEVICE_PROGRAM_LINKED);
            break;

        case DEVICE_PROGRAM_BE_BUILDING:
        case DEVICE_PROGRAM_CREATING_AUTORUN:
            m_ppDevicePrograms[i]->SetStateInternal(DEVICE_PROGRAM_BUILD_DONE);
            break;

        case DEVICE_PROGRAM_BUILD_DONE:
            // PostBuildTask was only called for cleanup
            break;

        default:
            bBuildFailed = true;
            break;
        }

        m_ppDevicePrograms[i]->Unacquire();
    }

    m_pProg->SetContextDevicesToProgramMappingInternal();

    if (m_ppDevicePrograms)
    {
        delete[] m_ppDevicePrograms;
        m_ppDevicePrograms = NULL;
    }

    if (m_ppHeaders)
    {
        delete[] m_ppHeaders;
        m_ppHeaders = NULL;
    }

    for (unsigned int i = 0; i < m_uiNumHeaders; ++i)
    {
        delete[] m_pszHeadersNames[i];
    }
    if (m_pszHeadersNames)
    {
        delete[] m_pszHeadersNames;
        m_pszHeadersNames = NULL;
    }

    if (m_ppBinaries)
    {
        delete[] m_ppBinaries;
        m_ppBinaries = NULL;
    }

    if (m_pfn_notify)
    {
        if (NULL != g_pUserLogger && g_pUserLogger->IsApiLoggingEnabled())
        {
            std::stringstream stream;
            stream << "BuildProgram callback(" << m_pProg->GetHandle() << ", " << m_user_data << ")" << std::endl;
            g_pUserLogger->PrintString(stream.str());
        }
        m_pfn_notify(m_pProg->GetHandle(), m_user_data);
    }

    if (bBuildFailed)
    {
        SetComplete(CL_BUILD_PROGRAM_FAILURE);
    }
    else
    {
        SetComplete(CL_BUILD_SUCCESS);
    }

    return true;
}

void PostBuildTask::Cancel()
{
    SetComplete(CL_BUILD_PROGRAM_FAILURE);
}

ProgramService::~ProgramService()
{
}

cl_err_code ProgramService::CompileProgram(const SharedPtr<Program>&    program,
                                           cl_uint                      num_devices,
                                           const cl_device_id *         device_list,
                                           cl_uint                      num_input_headers,
                                           SharedPtr<Program>*          input_headers,
                                           const char **                header_include_names,
                                           const char *                 options,
                                           pfnNotifyBuildDone           pfn_notify,
                                           void *                       user_data)
{
    if (program->GetNumKernels() > 0)
    {
        return CL_INVALID_OPERATION;
    }

    if ((0 == num_devices) && (NULL != device_list))
    {
        return CL_INVALID_VALUE;
    }

    if ((0 < num_devices) && (NULL == device_list))
    {
        return CL_INVALID_VALUE;
    }

    _cl_context_int* context = (_cl_context_int*)m_pContext->GetHandle();
    cl_uint uiNumDevices = program->GetNumDevices();

    if (0 == uiNumDevices)
    {
        return CL_INVALID_DEVICE;
    }
    if (num_devices > 0)
    {
        uiNumDevices = num_devices;
    }

    std::string buildOptions;
    if (NULL != options)
    {
        buildOptions = options;
    }

    clLocalArray<SharedPtr<BuildTask> > arrCompileTasks(uiNumDevices);

    for (unsigned int i = 0; i < uiNumDevices; ++i)
    {
        arrCompileTasks[i] = NULL;
    }

    // The memory will be used by compiler tasks for all devices
    // it will be released by PostBuild task
    const char** pszHeaders = NULL;
    char** pszHeadersNames = NULL;

    if (0 < num_input_headers)
    {
        pszHeaders = new const char*[num_input_headers];
        if (NULL == pszHeaders)
        {
            return CL_OUT_OF_HOST_MEMORY;
        }

        pszHeadersNames = new char*[num_input_headers];
        if (!pszHeadersNames)
        {
            delete[] pszHeaders;
            return CL_OUT_OF_HOST_MEMORY;
        }
    }

    // Get the sources for all the headers
    for (cl_uint i = 0; i < num_input_headers; ++i)
    {
        pszHeaders[i] = input_headers[i]->GetSourceInternal();
        if (NULL == pszHeaders[i])
        {
            for (cl_uint j = 0; j < i; ++j)
            {
                delete[] pszHeadersNames[j];
            }
            delete[] pszHeaders;
            delete[] pszHeadersNames;

            return CL_INVALID_OPERATION;
        }

        size_t len = strlen(header_include_names[i]) + 1;
        pszHeadersNames[i] = new char[len];
        if (NULL == pszHeadersNames[i])
        {
            for (cl_uint j = 0; j < i; ++j)
            {
                delete[] pszHeadersNames[j];
            }
            delete[] pszHeaders;
            delete[] pszHeadersNames;

            return CL_OUT_OF_HOST_MEMORY;
        }

        STRCPY_S(pszHeadersNames[i], len, header_include_names[i]);
    }

    // This will be released in PostBuildTask
    DeviceProgram** ppDevicePrograms = new DeviceProgram*[uiNumDevices];
    if (NULL == ppDevicePrograms)
    {
        // Release allocated memory of the headers
        for (cl_uint j = 0; j < num_input_headers; j++)
        {
            delete[] pszHeadersNames[j];
        }
        delete[] pszHeaders;
        delete[] pszHeadersNames;

        return CL_OUT_OF_HOST_MEMORY;
    }

    if (num_devices > 0)
    {
        // Retrive device programs for specified devices
        for ( cl_uint i = 0; i < uiNumDevices; ++i)
        {
            ppDevicePrograms[i] = program->GetDeviceProgram(device_list[i]);
        }
    }
    else // Build for all devices
    {
        std::vector<unique_ptr<DeviceProgram>>& pAllDevicePrograms = program->GetProgramsForAllDevices();
        for ( cl_uint i = 0; i < uiNumDevices; ++i)
        {
            ppDevicePrograms[i] = pAllDevicePrograms[i].get();
        }
    }

    // Acquire the program devices
    for (cl_uint i = 0; i < uiNumDevices; ++i)
    {
        if (!ppDevicePrograms[i]->Acquire())
        {
            // Acquire failed, release all accesses already acquired
            for (cl_uint j = 0; j < i; j++)
            {
                ppDevicePrograms[j]->Unacquire();
            }

            // Release allocated memory of the headers
            for (cl_uint j = 0; j < num_input_headers; j++)
            {
                delete[] pszHeadersNames[j];
            }
            delete[] pszHeaders;
            delete[] pszHeadersNames;
            delete[] ppDevicePrograms;

            return CL_INVALID_OPERATION;
        }
    }

    clLocalArray<ConstSharedPtr<FrontEndCompiler> > arrFeCompilers(uiNumDevices);

    // Check if the compile options are legal for all the devices
    for (unsigned int i = 0; i < uiNumDevices; ++i)
    {
        SharedPtr<Device> pDevice = ppDevicePrograms[i]->GetDevice()->GetRootDevice();

        arrFeCompilers[i] = pDevice->GetFrontEndCompiler();
        char* szUnrecognizedOptions = new char[buildOptions.size() + 1];
        if (!arrFeCompilers[i]->CheckCompileOptions(buildOptions.c_str(), szUnrecognizedOptions, buildOptions.size()+1))
        {
            for (cl_uint j = 0; j < uiNumDevices; ++j)
            {
                ppDevicePrograms[i]->SetBuildLogInternal("Compilation failed\n");
                ppDevicePrograms[i]->SetBuildLogInternal("Unrecognized build options: ");
                ppDevicePrograms[i]->SetBuildLogInternal(szUnrecognizedOptions);
                ppDevicePrograms[i]->SetBuildLogInternal("\n");

                ppDevicePrograms[i]->SetStateInternal(DEVICE_PROGRAM_COMPILE_FAILED);

                ppDevicePrograms[i]->Unacquire();
            }

            for (cl_uint j = 0; j < num_input_headers; ++j)
            {
              delete[] pszHeadersNames[j];
            }
            delete[] pszHeaders;
            delete[] pszHeadersNames;
            delete[] szUnrecognizedOptions;
            delete[] ppDevicePrograms;

            return CL_INVALID_COMPILER_OPTIONS;
        }
        delete[] szUnrecognizedOptions;
    }

    bool bNeedToBuild = false;

    for (unsigned int i = 0; i < uiNumDevices; ++i)
    {
        switch (ppDevicePrograms[i]->GetStateInternal())
        {
        case DEVICE_PROGRAM_COMPILED:
            {
                const char* szLastBuildOptions = ppDevicePrograms[i]->GetBuildOptionsInternal();

                //We're already compiled. If compile options changed, compile again. Otherwise, report success immediately.
                if (((NULL == szLastBuildOptions) && (NULL == options)) ||
                    ((NULL != szLastBuildOptions) && (NULL != options) && (0 == strcmp(szLastBuildOptions, options))))
                {
                    //No changes in compile options, so effectively the compilation can succeed vacuously
                    break;
                }
                //Else: some changes to compile options. Need to compile.
                //Intentional fall through.
            }
            LLVM_FALLTHROUGH;
        case DEVICE_PROGRAM_BUILD_DONE:
        case DEVICE_PROGRAM_BUILD_FAILED:
            {
                // The spec doesn't forbid compilation of built program
                // Intentional fall through.
            }
        case DEVICE_PROGRAM_LOADED_IR:
            {
                // SPIR extension allows compiling binary
                // Intentional fall through.
            }
        case DEVICE_PROGRAM_SOURCE:
        case DEVICE_PROGRAM_SPIRV:
            {
                // Building from source
                bNeedToBuild = true;
                ppDevicePrograms[i]->SetStateInternal(DEVICE_PROGRAM_FE_COMPILING);
                arrCompileTasks[i] = CompileTask::Allocate(
                    context, program, arrFeCompilers[i], ppDevicePrograms[i],
                    num_input_headers, pszHeaders,
                    const_cast<const char **>(pszHeadersNames),
                    buildOptions.c_str());
                if (NULL == arrCompileTasks[i].GetPtr()) {
                  ppDevicePrograms[i]->SetStateInternal(
                      DEVICE_PROGRAM_BUILD_FAILED);
                }
                break;
            }

        case DEVICE_PROGRAM_LINKED:
        case DEVICE_PROGRAM_CUSTOM_BINARY:
            {
                // Linked and custom binary programs already contains compiled binary
                break;
            }

        case DEVICE_PROGRAM_FE_COMPILING:
        case DEVICE_PROGRAM_FE_LINKING:
        case DEVICE_PROGRAM_BE_BUILDING:
            // If we succeeded in acquiring the program, this should not happen
        default:
            assert(false);
        }
    }

    SharedPtr<PostBuildTask> pPostBuildTask = PostBuildTask::Allocate(context, program, uiNumDevices, ppDevicePrograms,
                                                                    num_input_headers, input_headers, pszHeadersNames,
                                                                    0, NULL,
                                                                    pfn_notify, user_data);
    if (NULL == pPostBuildTask.GetPtr()) {
      delete[] ppDevicePrograms;
      return CL_OUT_OF_HOST_MEMORY;
    }

    for (unsigned int i = 0; i < uiNumDevices; ++i)
    {
      if (NULL != arrCompileTasks[i].GetPtr()) {
        pPostBuildTask->AddDependentOn(arrCompileTasks[i]);
      }
    }

    for (unsigned int i = 0; i < uiNumDevices; ++i)
    {
      if (NULL != arrCompileTasks[i].GetPtr()) {
        arrCompileTasks[i]->Launch();
      }
    }

    // If no build required, launch post build task
    if (!bNeedToBuild)
    {
        pPostBuildTask->Launch();
    }

    if (NULL == pfn_notify)
    {
        pPostBuildTask->Wait();
        if (CL_BUILD_SUCCESS != pPostBuildTask->GetReturnCode())
        {
            return CL_COMPILE_PROGRAM_FAILURE;
        }
    }

    return CL_SUCCESS;
}

cl_err_code ProgramService::LinkProgram(const SharedPtr<Program>&   program,
                                        cl_uint                     num_devices,
                                        const cl_device_id *        device_list,
                                        cl_uint                     num_input_programs,
                                        SharedPtr<Program>*         input_programs,
                                        const char *                options,
                                        pfnNotifyBuildDone          pfn_notify,
                                        void *                      user_data)
{
    if (program->GetNumKernels() > 0)
    {
        return CL_INVALID_OPERATION;
    }

    if ((0 == num_devices) && (NULL != device_list))
    {
        return CL_INVALID_VALUE;
    }

    if ((0 < num_devices) && (NULL == device_list))
    {
        return CL_INVALID_VALUE;
    }

    _cl_context_int* context = (_cl_context_int*)m_pContext->GetHandle();
    cl_uint uiNumDevices = program->GetNumDevices();

    if (0 == uiNumDevices)
    {
        return CL_INVALID_DEVICE;
    }
    if (num_devices > 0)
    {
        uiNumDevices = num_devices;
    }

    std::string buildOptions;
    if (NULL != options)
    {
        buildOptions = options;
    }

    clLocalArray<SharedPtr<BuildTask> > arrLinkTasks(uiNumDevices);
    clLocalArray<SharedPtr<BuildTask> > arrDeviceBuildTasks(uiNumDevices);

    for (unsigned int i = 0; i < uiNumDevices; ++i)
    {
        arrLinkTasks[i] = NULL;
        arrDeviceBuildTasks[i] = NULL;
    }

    // This will be released in PostBuildTask
    DeviceProgram** ppDevicePrograms = new DeviceProgram*[uiNumDevices];
    if (NULL == ppDevicePrograms)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }
    if (num_devices > 0)
    {
        // Retrive device programs for specified devices
        for ( cl_uint i = 0; i < uiNumDevices; ++i)
        {
            ppDevicePrograms[i] = program->GetDeviceProgram(device_list[i]);
        }
    }
    else // Build for all devices
    {
        std::vector<unique_ptr<DeviceProgram>>& pAllDevicePrograms = program->GetProgramsForAllDevices();
        for ( cl_uint i = 0; i < uiNumDevices; ++i)
        {
            ppDevicePrograms[i] = pAllDevicePrograms[i].get();
        }
    }

    // Check that all libraries have valid binary
    clLocalArray<bool> arrBuildForDevice(uiNumDevices);

    for (unsigned int devIndex = 0; devIndex < uiNumDevices; ++devIndex)
    {
        unsigned int uiFoundBinaries = 0;

        for (unsigned int libIndex = 0; libIndex < num_input_programs; ++libIndex)
        {
            cl_device_id clDeviceID = ppDevicePrograms[devIndex]->GetDeviceId();
            cl_program_binary_type clBinaryType = input_programs[libIndex]->GetBinaryTypeInternal( clDeviceID );
            if ((CL_PROGRAM_BINARY_TYPE_COMPILED_OBJECT == clBinaryType) ||
                (CL_PROGRAM_BINARY_TYPE_INTERMEDIATE  == clBinaryType) ||
                (CL_PROGRAM_BINARY_TYPE_LIBRARY == clBinaryType) )
            {
                ++uiFoundBinaries;
            }
        }

        // according to the its all or nothing, either we have binaries for all the devices or none of them
        if (0 == uiFoundBinaries)
        {
            arrBuildForDevice[devIndex] = false;
            break;
        }

        if (num_input_programs == uiFoundBinaries)
        {
            arrBuildForDevice[devIndex] = true;
            continue;
        }
        delete[] ppDevicePrograms;
        delete[] input_programs;
        return CL_INVALID_OPERATION;
    }

    // Acquire the program devices
    for (cl_uint i = 0; i < uiNumDevices; ++i)
    {
        if (arrBuildForDevice[i])
        {
            if (!ppDevicePrograms[i]->Acquire())
            {
                // Acquire failed, release all accesses already acquired
                for (cl_uint j = 0; j < i; j++)
                {
                    ppDevicePrograms[j]->Unacquire();
                }

                delete[] ppDevicePrograms;
                delete[] input_programs;

                return CL_INVALID_OPERATION;
            }
        }
    }

    clLocalArray<ConstSharedPtr<FrontEndCompiler> > arrFeCompilers(uiNumDevices);

    // Check if the link options are legal for all the devices
    for (unsigned int i = 0; i < uiNumDevices; ++i)
    {
        if ( DEVICE_PROGRAM_CUSTOM_BINARY == ppDevicePrograms[i]->GetStateInternal() )
        {
            // For custom binaries no need call to FE compiler, no compiler or linker services
            continue;
        }
        SharedPtr<Device> pDevice = ppDevicePrograms[i]->GetDevice()->GetRootDevice();

        arrFeCompilers[i] = pDevice->GetFrontEndCompiler();
        char* szUnrecognizedOptions = new char[buildOptions.size() + 1];
        if (!arrFeCompilers[i]->CheckLinkOptions(buildOptions.c_str(), szUnrecognizedOptions, buildOptions.size() + 1))
        {
            for (cl_uint j = 0; j < uiNumDevices; ++j)
            {
                ppDevicePrograms[i]->SetBuildLogInternal("Linking failed\n");
                ppDevicePrograms[i]->SetBuildLogInternal("Unrecognized link options: ");
                ppDevicePrograms[i]->SetBuildLogInternal(szUnrecognizedOptions);
                ppDevicePrograms[i]->SetBuildLogInternal("\n");

                ppDevicePrograms[i]->SetStateInternal(DEVICE_PROGRAM_LINK_FAILED);

                ppDevicePrograms[i]->Unacquire();
            }

            delete[] szUnrecognizedOptions;
            delete[] ppDevicePrograms;
            delete[] input_programs;

            return CL_INVALID_LINKER_OPTIONS;
        }
        delete[] szUnrecognizedOptions;
    }

    bool bNeedToBuild = false;

    for (unsigned int i = 0; i < uiNumDevices; ++i)
    {
        switch (ppDevicePrograms[i]->GetStateInternal())
        {
        case DEVICE_PROGRAM_INVALID:
            {
                // program is created specially for linking purposes so no action should have been done yet
                if (arrBuildForDevice[i])
                {
                    bNeedToBuild = true;

                    ppDevicePrograms[i]->SetStateInternal(DEVICE_PROGRAM_FE_LINKING);
                    arrLinkTasks[i] = LinkTask::Allocate(context, program, arrFeCompilers[i], ppDevicePrograms[i], input_programs, num_input_programs,
                                                buildOptions.c_str());
                    if (NULL == arrLinkTasks[i].GetPtr()) {
                      ppDevicePrograms[i]->SetStateInternal(
                          DEVICE_PROGRAM_BUILD_FAILED);
                    }
                }
            } //Intentional fall through.
            LLVM_FALLTHROUGH;
        case DEVICE_PROGRAM_LINKED:
        case DEVICE_PROGRAM_CUSTOM_BINARY:
            {
                // program was linked, we need to build it on device
                if (arrBuildForDevice[i])
                {
                    bNeedToBuild = true;
                    ppDevicePrograms[i]->SetStateInternal(DEVICE_PROGRAM_BE_BUILDING);

                    // Propagate "-cl-opt-disable" and "-g" options used for
                    // compilation to build task because build task is actually
                    // performing all backend optimizations depending on
                    // provided options. This propagation is necessary for
                    // scenario when SYCL compiler is used with CPU backend
                    // because SYCL compiler doesn't add metadata for these
                    // options. In case of OpenCL options are propagated using
                    // opencl.compiler.options metadata.
                    std::string compileOptions;
                    for (unsigned int libIndex = 0;
                         libIndex < num_input_programs; ++libIndex) {
                      if (const char *opts =
                              input_programs[libIndex]->GetBuildOptionsInternal(
                                  ppDevicePrograms[i]->GetDeviceId())) {
                        if (std::string(opts).find("-cl-opt-disable") !=
                                std::string::npos &&
                            compileOptions.find("-cl-opt-disable") ==
                                std::string::npos)
                          compileOptions.append(" -cl-opt-disable");
                        if (std::string(opts).find("-g") != std::string::npos &&
                            compileOptions.find("-g") == std::string::npos)
                          compileOptions.append(" -g");
                      }
                    }

                    std::string mergedOptions = compileOptions + buildOptions;
                    arrDeviceBuildTasks[i] = DeviceBuildTask::Allocate(
                        context, program, ppDevicePrograms[i],
                        mergedOptions.c_str());
                    if (NULL == arrDeviceBuildTasks[i].GetPtr()) {
                      ppDevicePrograms[i]->SetStateInternal(
                          DEVICE_PROGRAM_BUILD_FAILED);
                    } else if (NULL != arrLinkTasks[i].GetPtr()) {
                      arrDeviceBuildTasks[i]->AddDependentOn(arrLinkTasks[i]);
                    }

                    ppDevicePrograms[i]->ClearBuildLogInternal();
                    ppDevicePrograms[i]->SetBuildOptionsInternal(options);
                }
            }
            continue;

        case DEVICE_PROGRAM_BUILD_DONE:
        case DEVICE_PROGRAM_BUILD_FAILED:
        case DEVICE_PROGRAM_SOURCE:
        case DEVICE_PROGRAM_COMPILED:
        case DEVICE_PROGRAM_LOADED_IR:
        case DEVICE_PROGRAM_FE_COMPILING:
        case DEVICE_PROGRAM_FE_LINKING:
        case DEVICE_PROGRAM_BE_BUILDING:
        default:
            // Program for link should never be in one of these states when calling LinkProgram
            assert(false);
        }
    }

    SharedPtr<PostBuildTask> pPostBuildTask = PostBuildTask::Allocate(context, program, uiNumDevices, ppDevicePrograms,
                                                                    0, NULL, NULL,
                                                                    num_input_programs, input_programs,
                                                                    pfn_notify, user_data);

    if (NULL == pPostBuildTask.GetPtr()) {
      delete[] ppDevicePrograms;
      delete[] input_programs;
      return CL_OUT_OF_HOST_MEMORY;
    }

    cl_bool isFPGAEmulator = program->GetContext()->IsFPGAEmulator();
    SharedPtr<CreateAutorunKernelsTask> pCreateAutorunKernelsTask;

    if (isFPGAEmulator)
    {
        try
        {
            pCreateAutorunKernelsTask = CreateAutorunKernelsTask::Allocate(
                context, program);
        }
        catch (const std::bad_alloc& e)
        {
            delete[] ppDevicePrograms;
            return CL_OUT_OF_HOST_MEMORY;
        }
    }

    for (unsigned int i = 0; i < uiNumDevices; ++i)
    {
      if (NULL != arrDeviceBuildTasks[i].GetPtr()) {
        if (isFPGAEmulator) {
          pCreateAutorunKernelsTask->AddDependentOn(arrDeviceBuildTasks[i]);
        } else {
          pPostBuildTask->AddDependentOn(arrDeviceBuildTasks[i]);
        }
      }
    }

    if (isFPGAEmulator)
    {
        pPostBuildTask->AddDependentOn(pCreateAutorunKernelsTask);
    }

    // launch the required task for each device
    for (unsigned int i = 0; i < uiNumDevices; ++i)
    {
      if (NULL != arrLinkTasks[i].GetPtr()) {
        arrLinkTasks[i]->Launch();
      } else if (NULL != arrDeviceBuildTasks[i].GetPtr()) {
        arrDeviceBuildTasks[i]->Launch();
      }
    }

    // If no build required, launch post build task
    if (!bNeedToBuild)
    {
        if (isFPGAEmulator)
        {
            pCreateAutorunKernelsTask->Launch();
        }
        else
        {
            pPostBuildTask->Launch();
        }
    }

    if (NULL == pfn_notify)
    {
        pPostBuildTask->Wait();
        if (CL_BUILD_SUCCESS != pPostBuildTask->GetReturnCode())
        {
            return CL_LINK_PROGRAM_FAILURE;
        }
    }

    return CL_SUCCESS;
}

cl_err_code ProgramService::BuildProgram(const SharedPtr<Program>& program, cl_uint num_devices, const cl_device_id *device_list, const char *options, pfnNotifyBuildDone pfn_notify, void *user_data)
{
    if (program->GetNumKernels() > 0)
    {
        return CL_INVALID_OPERATION;
    }

    if (NULL != program.DynamicCast<ProgramWithBuiltInKernels>().GetPtr()) {
      return CL_INVALID_OPERATION;
    }

    if ((0 == num_devices) && (NULL != device_list))
    {
        return CL_INVALID_VALUE;
    }

    if ((0 < num_devices) && (NULL == device_list))
    {
        return CL_INVALID_VALUE;
    }

    _cl_context_int* context = (_cl_context_int*)m_pContext->GetHandle();
    cl_uint uiNumDevices = program->GetNumDevices();

    if (0 == uiNumDevices)
    {
        return CL_INVALID_DEVICE;
    }
    if (num_devices > 0)
    {
        uiNumDevices = num_devices;
    }

    std::string buildOptions;
    if (NULL != options)
    {
        buildOptions = options;
    }

    clLocalArray<SharedPtr<BuildTask> > arrCompileTasks(uiNumDevices);
    clLocalArray<SharedPtr<BuildTask> > arrLinkTasks(uiNumDevices);
    clLocalArray<SharedPtr<BuildTask> > arrDeviceBuildTasks(uiNumDevices);

    // this will be released in PostBuildTask
    DeviceProgram** ppDevicePrograms = new DeviceProgram*[uiNumDevices];
    if (NULL == ppDevicePrograms)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }
    if (num_devices > 0)
    {
        // Retrive device programs for specified devices
        for ( cl_uint i = 0; i < uiNumDevices; ++i)
        {
            ppDevicePrograms[i] = program->GetDeviceProgram(device_list[i]);
        }
    }
    else // Build for all devices
    {
        std::vector<unique_ptr<DeviceProgram>>& pAllDevicePrograms = program->GetProgramsForAllDevices();
        for ( cl_uint i = 0; i < uiNumDevices; ++i)
        {
            ppDevicePrograms[i] = pAllDevicePrograms[i].get();
        }
    }

    // Device programs may be null. CL_INVALID_DEVICE will be returned according to spec.
    for (cl_uint i = 0; i < uiNumDevices; ++i)
    {
        if (NULL == ppDevicePrograms[i])
        {
            return CL_INVALID_DEVICE;
        }
    }

    // Acquire the program devices
    for (cl_uint i = 0; i < uiNumDevices; ++i)
    {
        if (!ppDevicePrograms[i]->Acquire())
        {
            // Acquire failed, release all accesses already acquired
            for (cl_uint j = 0; j < i; j++)
            {
                ppDevicePrograms[j]->Unacquire();
            }

            delete[] ppDevicePrograms;

            return CL_INVALID_OPERATION;
        }
    }

    clLocalArray<ConstSharedPtr<FrontEndCompiler> > arrFeCompilers(uiNumDevices);

    // Check if the build options are legal for all the devices
    for (unsigned int i = 0; i < uiNumDevices; ++i)
    {
        if ( DEVICE_PROGRAM_CUSTOM_BINARY == ppDevicePrograms[i]->GetStateInternal() )
        {
            // For custom binaries no need call to FE compiler, no compiler or linker services
            continue;
        }

        SharedPtr<Device> pDevice = ppDevicePrograms[i]->GetDevice()->GetRootDevice();

        arrFeCompilers[i] = pDevice->GetFrontEndCompiler();
        char* szUnrecognizedOptions = new char[buildOptions.size() + 1];
        if (!arrFeCompilers[i]->CheckCompileOptions(buildOptions.c_str(), szUnrecognizedOptions, buildOptions.size()+1))
        {
            for (cl_uint j = 0; j < uiNumDevices; ++j)
            {
                ppDevicePrograms[j]->SetBuildLogInternal("Compilation failed\n");
                ppDevicePrograms[j]->SetBuildLogInternal("Unrecognized build options: ");
                ppDevicePrograms[j]->SetBuildLogInternal(szUnrecognizedOptions);
                ppDevicePrograms[j]->SetBuildLogInternal("\n");

                ppDevicePrograms[j]->SetStateInternal(DEVICE_PROGRAM_COMPILE_FAILED);

                ppDevicePrograms[j]->Unacquire();
            }

            delete[] szUnrecognizedOptions;
            delete[] ppDevicePrograms;

            return CL_INVALID_BUILD_OPTIONS;
        }
        delete[] szUnrecognizedOptions;
    }

    bool bNeedToBuild = false;

    for (unsigned int i = 0; i < uiNumDevices; ++i)
    {
        switch (ppDevicePrograms[i]->GetStateInternal())
        {
        case DEVICE_PROGRAM_BUILD_DONE:
            {
                const char* szLastBuildOptions = ppDevicePrograms[i]->GetBuildOptionsInternal();

                //We're already built. If build options changed, build again. Otherwise, report success immediately.
                if (((NULL == szLastBuildOptions) && (NULL == options)) ||
                    ((NULL != szLastBuildOptions) && (NULL != options) && (0 == strcmp(szLastBuildOptions, options))))
                {
                    //No changes in build options, so effectively the build can succeed vacuously
                    break;
                }
                //Else: some changes to build options. Need to build.
                //Intentional fall through.
            }
            LLVM_FALLTHROUGH;
        case DEVICE_PROGRAM_BUILD_FAILED:
            {
                // Possibly retrying a failed build - legal
                if (nullptr == program->GetSourceInternal())
                {
                    //invalid binaries are hopeless
                    //remember build options even if build will fail
                    ppDevicePrograms[i]->SetBuildOptionsInternal(options);
                    break;
                }
                //Intentional fall through.
            }
            LLVM_FALLTHROUGH;
        case DEVICE_PROGRAM_LOADED_IR:
        case DEVICE_PROGRAM_SOURCE:
        case DEVICE_PROGRAM_SPIRV:
            {
                // Building from source
                bNeedToBuild = true;
                ppDevicePrograms[i]->SetStateInternal(DEVICE_PROGRAM_FE_COMPILING);
                arrCompileTasks[i] = CompileTask::Allocate(context, program, arrFeCompilers[i], ppDevicePrograms[i],
                                                    0, NULL, NULL, buildOptions.c_str());

                if (NULL == arrCompileTasks[i].GetPtr()) {
                  ppDevicePrograms[i]->SetStateInternal(
                      DEVICE_PROGRAM_BUILD_FAILED);
                }
                //Intentional fall through.
            }
            LLVM_FALLTHROUGH;
        case DEVICE_PROGRAM_COMPILED:
            {
                // Building from compiled object
                bNeedToBuild = true;
                ppDevicePrograms[i]->SetStateInternal(DEVICE_PROGRAM_FE_LINKING);
                arrLinkTasks[i] = LinkTask::Allocate(context, program, arrFeCompilers[i], ppDevicePrograms[i],
                                                NULL, 0, buildOptions.c_str());
                if (NULL == arrLinkTasks[i].GetPtr()) {
                  ppDevicePrograms[i]->SetStateInternal(
                      DEVICE_PROGRAM_BUILD_FAILED);
                } else if (NULL != arrCompileTasks[i].GetPtr()) {
                  arrLinkTasks[i]->AddDependentOn(arrCompileTasks[i]);
                }
                //Intentional fall through.
            }
            LLVM_FALLTHROUGH;
        case DEVICE_PROGRAM_LINKED:
        case DEVICE_PROGRAM_CUSTOM_BINARY:
            {
                // Building from linked or custom binary
                bNeedToBuild = true;
                ppDevicePrograms[i]->SetStateInternal(DEVICE_PROGRAM_BE_BUILDING);
                arrDeviceBuildTasks[i] = DeviceBuildTask::Allocate(context, program, ppDevicePrograms[i], buildOptions.c_str());
                if (NULL == arrDeviceBuildTasks[i].GetPtr()) {
                  ppDevicePrograms[i]->SetStateInternal(
                      DEVICE_PROGRAM_BUILD_FAILED);
                } else if (NULL != arrLinkTasks[i].GetPtr()) {
                  arrDeviceBuildTasks[i]->AddDependentOn(arrLinkTasks[i]);
                }

                ppDevicePrograms[i]->ClearBuildLogInternal();
                ppDevicePrograms[i]->SetBuildOptionsInternal(options);
            }
            break;

        default:
        case DEVICE_PROGRAM_FE_COMPILING:
        case DEVICE_PROGRAM_FE_LINKING:
        case DEVICE_PROGRAM_BE_BUILDING:
            // If we succeeded in acquiring the program, this should not happen
            assert(false);
        }
    }

    SharedPtr<PostBuildTask> pPostBuildTask = PostBuildTask::Allocate(context, program, uiNumDevices, ppDevicePrograms, 0, NULL, NULL,
                                                                      0, NULL,
                                                                      pfn_notify, user_data);

    if (NULL == pPostBuildTask.GetPtr()) {
      delete[] ppDevicePrograms;
      return CL_OUT_OF_HOST_MEMORY;
    }

    cl_bool isFPGAEmulator = program->GetContext()->IsFPGAEmulator();
    SharedPtr<CreateAutorunKernelsTask> pCreateAutorunKernelsTask;

    if (isFPGAEmulator)
    {
        try
        {
            pCreateAutorunKernelsTask = CreateAutorunKernelsTask::Allocate(
                context, program);
        }
        catch (const std::bad_alloc& e)
        {
            delete[] ppDevicePrograms;
            return CL_OUT_OF_HOST_MEMORY;
        }
    }

    for (unsigned int i = 0; i < uiNumDevices; ++i)
    {
      if (NULL != arrDeviceBuildTasks[i].GetPtr()) {
        if (isFPGAEmulator) {
          pCreateAutorunKernelsTask->AddDependentOn(arrDeviceBuildTasks[i]);
        } else {
          pPostBuildTask->AddDependentOn(arrDeviceBuildTasks[i]);
        }
      }
    }

    if (isFPGAEmulator)
    {
        pPostBuildTask->AddDependentOn(pCreateAutorunKernelsTask);
    }

    // launch the required task for each device
    for (unsigned int i = 0; i < uiNumDevices; ++i)
    {
      if (NULL != arrCompileTasks[i].GetPtr()) {
        arrCompileTasks[i]->Launch();
      } else if (NULL != arrLinkTasks[i].GetPtr()) {
        arrLinkTasks[i]->Launch();
      } else if (NULL != arrDeviceBuildTasks[i].GetPtr()) {
        arrDeviceBuildTasks[i]->Launch();
      }
    }

    // If no build required, launch post build task
    if (!bNeedToBuild)
    {
        if (isFPGAEmulator)
        {
            pCreateAutorunKernelsTask->Launch();
        }
        else
        {
            pPostBuildTask->Launch();
        }
    }

    if (NULL == pfn_notify)
    {
        pPostBuildTask->Wait();
        if (CL_BUILD_SUCCESS != pPostBuildTask->GetReturnCode())
        {
            return CL_BUILD_PROGRAM_FAILURE;
        }
    }

    return CL_SUCCESS;
}

CreateAutorunKernelsTask::CreateAutorunKernelsTask(
    _cl_context_int* context, const SharedPtr<Program>& pProg)
    : BuildTask(context, pProg, nullptr)
{
}

bool CreateAutorunKernelsTask::Execute()
{
    auto& devicePrograms = m_pProg->GetProgramsForAllDevices();

    // used to save original states of device programs
    for (const auto& program: devicePrograms)
    {
        EDeviceProgramState state = program->GetStateInternal();
        cl_program_binary_type bin_type = program->GetBinaryTypeInternal();
        // If previous stages failed don't continue execution
        if ((DEVICE_PROGRAM_COMPILE_FAILED == state) ||
            (DEVICE_PROGRAM_LINK_FAILED == state) ||
            (DEVICE_PROGRAM_BUILD_FAILED == state) ||
            (bin_type != CL_PROGRAM_BINARY_TYPE_EXECUTABLE))
        {
            SetComplete(CL_BUILD_SUCCESS);
            return true;
        }
    }

    for (const auto& program: devicePrograms)
    {
        program->SetStateInternal(DEVICE_PROGRAM_CREATING_AUTORUN);
    }

    cl_err_code error = m_pProg->CreateAutorunKernels(0, nullptr, nullptr);
    if (CL_FAILED(error))
    {
        for (const auto& program: devicePrograms)
        {
            program->SetStateInternal(DEVICE_PROGRAM_BUILD_FAILED);
        }
    }

    SetComplete(CL_BUILD_SUCCESS);
    return true;
}

void CreateAutorunKernelsTask::Cancel()
{
    SetComplete(CL_BUILD_ERROR);
}

CreateAutorunKernelsTask::~CreateAutorunKernelsTask()
{
}

cl_err_code ProgramService::SetSpecializationConstant(const SharedPtr<Program>& pProgram,
                                                      cl_uint uiSpecId,
                                                      size_t szSpecSize,
                                                      const void* pSpecValue)
{
    SharedPtr<ProgramWithIL> pIL = pProgram.DynamicCast<ProgramWithIL>();
    // Retrieve information about specialization constants from IL/SPIRV
    // just once and cache it the ProgramWithIL object.
    if (!pIL->IsSpecConstInfoCached()) {
        // We need a frontend compiler to read SPIR-V. The program is
        // associated with one or more devices. Each device in turn is
        // associated with a frontend compiler.
        // In the current implementaion we have single frontend compiler for
        // all supported devices. So we can pick any device returned by
        // GetProgramsForAllDevices().
        auto& AllDevPrograms = pProgram->GetProgramsForAllDevices();
        assert(!AllDevPrograms.empty() &&
               "No device program is associated with the program");
        unique_ptr<DeviceProgram>& pDevProgram = AllDevPrograms[0];
        SharedPtr<Device> pDevice = pDevProgram->GetDevice()->GetRootDevice();
        assert(pDevice && "No device is associated with the device program");
        SharedPtr<FrontEndCompiler> pFECompiler = pDevice->GetFrontEndCompiler();
        assert(pFECompiler && "No FE compiler is associated with the device");
        pFECompiler->GetSpecConstInfo(pIL->GetSourceInternal(),
                                      pIL->GetSize(),
                                      pIL->GetSpecConstInfoRef());
        pIL->SpecConstInfoIsCached();
    }
    return pIL->AddSpecConst(uiSpecId, szSpecSize, pSpecValue);
}

