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


///////////////////////////////////////////////////////////////////////////////////////////////////
//  program_service.cpp
//  Implementation of the Program service class
//  Created on:      12-Jan-2012
//  Original author: Sagi Shahar
///////////////////////////////////////////////////////////////////////////////////////////////////

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


using namespace Intel::OpenCL::Framework;

// In this file we use CompileTask, LinkTask and PostBuildTask as building blocks to create a general build tree.
// for a full build we use CompileTask -> LinkTask -> PostBuildTask where CompileTask and LinkTask are created 
// for each device and dependent only on the previouse task for the same device. PostBuildTask is created once
// for all the devices and dependent on the prevous tasks on all the devices.
// When compiling a program we dont' use the LinkTask and when Linking a program, we don't use the CompileTask.
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
    return FrameworkProxy::Instance()->Execute(SharedPtr<BuildTask>(this));
}

void BuildTask::SetComplete(cl_int returnCode)
{
	// We must assing NULL to shared pointers in order to remove reference count and avoid
	// a raise condition during program release
	m_pProg = NULL;
	m_pFECompiler = NULL;
	BuildEvent::SetComplete(returnCode);
}


///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

CompileTask::CompileTask(_cl_context_int*        context,
                         cl_device_id            deviceID,
                         const ConstSharedPtr<FrontEndCompiler>& pFECompiler,
                         const char*             szSource,
                         unsigned int            uiNumHeaders,
                         const char**            pszHeaders,
                         char**                  pszHeadersNames,
                         const char*             szOptions,
                         const SharedPtr<Program>&  pProg) : 
BuildTask(context, pProg, pFECompiler), 
m_deviceID(deviceID), m_szSource(szSource), m_uiNumHeaders(uiNumHeaders),
m_pszHeaders(pszHeaders), m_pszHeadersNames((const char**)pszHeadersNames), m_szOptions(szOptions)
{
}

CompileTask::~CompileTask()
{
}

bool CompileTask::Execute()
{
    char* pBinary = NULL;
    size_t uiBinarySize = 0;
    char* szCompileLog = NULL;

    m_pProg->SetBuildLogInternal(m_deviceID, "Compilation started\n");
    m_pProg->SetStateInternal(m_deviceID, DEVICE_PROGRAM_FE_COMPILING);

    m_pFECompiler->CompileProgram(m_szSource, 
                                  m_uiNumHeaders, 
                                  m_pszHeaders, 
                                  m_pszHeadersNames, 
                                  m_szOptions, 
                                  &pBinary, 
                                  &uiBinarySize, 
                                  &szCompileLog);

    
    if (NULL != szCompileLog)
    {
        m_pProg->SetBuildLogInternal(m_deviceID, szCompileLog);
        delete[] szCompileLog;
    }

    if (0 == uiBinarySize)
    {
        assert( NULL == pBinary);
        //Build failed
        m_pProg->SetBuildLogInternal(m_deviceID, "Compilation failed\n");
        m_pProg->SetStateInternal(m_deviceID, DEVICE_PROGRAM_COMPILE_FAILED);
        SetComplete(CL_BUILD_SUCCESS);
        return true;
    }

    m_pProg->SetBuildLogInternal(m_deviceID, "Compilation done\n");

    //Else compile succeeded
    m_pProg->SetBinaryInternal(m_deviceID, uiBinarySize, pBinary);
    delete[] pBinary;
    SetComplete(CL_BUILD_SUCCESS);

	return true;
}

void CompileTask::Cancel()
{
    SetComplete(CL_BUILD_ERROR);
}

LinkTask::LinkTask(_cl_context_int*               context, 
                   cl_device_id             deviceID, 
                   const ConstSharedPtr<FrontEndCompiler>&  pFECompiler, 
                   IOCLDeviceAgent*   pDeviceAgent, 
                   SharedPtr<Program>*                ppPrograms, 
                   unsigned int             uiNumPrograms, 
                   const char*              szOptions, 
                   const SharedPtr<Program>&                 pProg) :
BuildTask(context, pProg, pFECompiler), 
m_deviceID(deviceID), m_pDeviceAgent(pDeviceAgent), m_ppPrograms(ppPrograms), 
m_uiNumPrograms(uiNumPrograms), m_szOptions(szOptions)
{
}

LinkTask::~LinkTask()
{
}

bool LinkTask::Execute()
{
    char* pBinary = NULL;
    size_t uiBinarySize = 0;
    char* szLinkLog = NULL;
    bool bIsLibrary = false;

    if (m_pProg->GetStateInternal(m_deviceID) == DEVICE_PROGRAM_COMPILE_FAILED)
	  {
        SetComplete(CL_BUILD_SUCCESS);
		    return true;
	  }

    m_pProg->SetBuildLogInternal(m_deviceID, "Linking started\n");
    m_pProg->SetStateInternal(m_deviceID, DEVICE_PROGRAM_FE_LINKING);

    const void** ppBinaries = NULL;
    size_t* puiBinariesSizes = NULL;

    if (0 == m_uiNumPrograms)
    {
        m_uiNumPrograms = 1;

        ppBinaries = new const void*[1];
		    if (NULL == ppBinaries)
		    {
			    m_pProg->SetStateInternal(m_deviceID, DEVICE_PROGRAM_LINK_FAILED);
				m_pProg->SetBuildLogInternal(m_deviceID, "Out of memory encountered\n");
				SetComplete(CL_BUILD_SUCCESS);
				return true;
		    }

        puiBinariesSizes = new size_t[1];
        if (NULL == puiBinariesSizes)
        {
			    delete[] ppBinaries;
				m_pProg->SetStateInternal(m_deviceID, DEVICE_PROGRAM_LINK_FAILED);
				m_pProg->SetBuildLogInternal(m_deviceID, "Out of memory encountered\n");
				SetComplete(CL_BUILD_SUCCESS);
				return true;
        }

        ppBinaries[0] = m_pProg->GetBinaryInternal(m_deviceID);
        puiBinariesSizes[0] = m_pProg->GetBinarySizeInternal(m_deviceID);
    }
    else
    {
		ppBinaries = new const void*[m_uiNumPrograms];
		if (NULL == ppBinaries)
		{
			m_pProg->SetStateInternal(m_deviceID, DEVICE_PROGRAM_LINK_FAILED);
			m_pProg->SetBuildLogInternal(m_deviceID, "Out of memory encountered\n");
			SetComplete(CL_BUILD_SUCCESS);
			return true;
		}

        puiBinariesSizes = new size_t[m_uiNumPrograms];
        if (NULL == puiBinariesSizes)
        {
			delete[] ppBinaries;
            m_pProg->SetStateInternal(m_deviceID, DEVICE_PROGRAM_LINK_FAILED);
            m_pProg->SetBuildLogInternal(m_deviceID, "Out of memory encountered\n");
            SetComplete(CL_BUILD_SUCCESS);
            return true;
        }

        for (unsigned int i = 0; i < m_uiNumPrograms; ++i)
        {
            ppBinaries[i] = m_ppPrograms[i]->GetBinaryInternal(m_deviceID);
            puiBinariesSizes[i] = m_ppPrograms[i]->GetBinarySizeInternal(m_deviceID);
        }
    }

    m_pFECompiler->LinkProgram(ppBinaries, 
                               m_uiNumPrograms, 
                               puiBinariesSizes, 
                               m_szOptions, 
                               &pBinary, 
                               &uiBinarySize, 
                               &szLinkLog,
                               &bIsLibrary);

    delete[] ppBinaries;
    delete[] puiBinariesSizes;

    if (0 == uiBinarySize)
    {
        assert( NULL == pBinary);
        //Build failed
        m_pProg->SetStateInternal(m_deviceID, DEVICE_PROGRAM_LINK_FAILED);

        if (NULL != szLinkLog)
        {
            m_pProg->SetBuildLogInternal(m_deviceID, szLinkLog);
            delete[] szLinkLog;
        }

        m_pProg->SetBuildLogInternal(m_deviceID, "Linking failed\n");

        SetComplete(CL_BUILD_SUCCESS);
        return true;
    }

    //Else link succeeded

    // If we are creating library, no need for BE build
    if (bIsLibrary)
    {
        
        m_pProg->SetBinaryInternal(m_deviceID, uiBinarySize, pBinary);
        m_pProg->SetBuildLogInternal(m_deviceID, "Linking done\n");
        SetComplete(CL_BUILD_SUCCESS);
        delete[] pBinary;
        return true;
    }

    cl_dev_program      programHandle;
    cl_build_status     build_status;

    cl_prog_container_header*	pHeader = (cl_prog_container_header*)pBinary;
    pHeader->description.bin_type = CL_PROG_BIN_EXECUTABLE_LLVM;

    m_pProg->SetBinaryInternal(m_deviceID, uiBinarySize, pBinary);

    cl_dev_err_code err = m_pDeviceAgent->clDevCheckProgramBinary(uiBinarySize, pBinary);
    if (CL_DEV_SUCCESS != err)
    {
        //Build failed
        delete[] pBinary;
        m_pProg->SetStateInternal(m_deviceID, DEVICE_PROGRAM_LINK_FAILED);
        m_pProg->SetBuildLogInternal(m_deviceID, "Linking failed\n");
        SetComplete(CL_BUILD_SUCCESS);
        return true;
    }

    err = m_pDeviceAgent->clDevCreateProgram(uiBinarySize, pBinary, CL_DEV_BINARY_COMPILER, &programHandle);
    if (CL_DEV_SUCCESS != err)
    {
        //Build failed
        delete[] pBinary;
        m_pProg->SetStateInternal(m_deviceID, DEVICE_PROGRAM_LINK_FAILED);
        m_pProg->SetBuildLogInternal(m_deviceID, "Linking failed\n");
        SetComplete(CL_BUILD_SUCCESS);
        return true;
    }
	
	  m_pProg->SetDeviceHandleInternal(m_deviceID, programHandle);

    delete[] pBinary;

    m_pProg->SetStateInternal(m_deviceID, DEVICE_PROGRAM_BE_BUILDING);

    err = m_pDeviceAgent->clDevBuildProgram(programHandle, m_szOptions, &build_status);

    assert(CL_BUILD_ERROR == build_status || CL_BUILD_SUCCESS == build_status);

	  if (CL_BUILD_ERROR == build_status)
	  {
        m_pProg->SetStateInternal(m_deviceID, DEVICE_PROGRAM_BUILD_FAILED);
        m_pProg->SetBuildLogInternal(m_deviceID, "Linking failed\n");
        SetComplete(CL_BUILD_SUCCESS);
		    return true;
	  }
  
    m_pProg->SetBuildLogInternal(m_deviceID, "Linking done\n");
    SetComplete(CL_BUILD_SUCCESS);
	  return true;
}

void LinkTask::Cancel()
{
    SetComplete(CL_BUILD_ERROR);
}

PostBuildTask::PostBuildTask(_cl_context_int* context, 
                             cl_uint num_devices, 
                             const cl_device_id *deviceID, 
                             unsigned int uiNumHeaders, 
                             SharedPtr<Program>*ppHeaders, 
                             char** pszHeadersNames,
                             unsigned int uiNumBinaries, 
                             SharedPtr<Program>* ppBinaries, 
                             const SharedPtr<Program>& pProg,
                             const char* szOptions,
                             pfnNotifyBuildDone pfn_notify, 
                             void *user_data) :
BuildTask(context, pProg, ConstSharedPtr<FrontEndCompiler>()),
m_num_devices(num_devices), m_deviceID(deviceID), m_uiNumHeaders(uiNumHeaders), m_ppHeaders(ppHeaders),
m_pszHeadersNames(pszHeadersNames), m_uiNumBinaries(uiNumBinaries), m_ppBinaries(ppBinaries), 
m_szOptions(szOptions), m_pfn_notify(pfn_notify), m_user_data(user_data)
{
}

PostBuildTask::~PostBuildTask()
{
}

bool PostBuildTask::Execute()
{
    cl_program program = m_pProg->GetHandle();

    for (unsigned int i = 0; i < m_uiNumHeaders; ++i)
    {
        delete[] m_pszHeadersNames[i];
    }

    for (unsigned int binID = 0; binID < m_uiNumBinaries; ++binID)
    {
        for (unsigned int i = 0; i < m_num_devices; ++i)
        {
            m_ppBinaries[binID]->Unacquire(m_deviceID[i]);
        }
    }

    bool bBuildFailed = false;

    for (unsigned int i = 0; i < m_num_devices; ++i)
    {
        cl_device_id devID = m_deviceID[i];
        EDeviceProgramState currState = m_pProg->GetStateInternal(devID);

        switch (currState)
        {
        case DEVICE_PROGRAM_FE_COMPILING:
            m_pProg->SetStateInternal(devID, DEVICE_PROGRAM_COMPILED);
            break;

        case DEVICE_PROGRAM_FE_LINKING:
            m_pProg->SetStateInternal(devID, DEVICE_PROGRAM_LINKED);
            break;

        case DEVICE_PROGRAM_BE_BUILDING:
            m_pProg->SetStateInternal(devID, DEVICE_PROGRAM_BUILD_DONE);
            break;

        case DEVICE_PROGRAM_BUILD_DONE:
            // PostBuildTask was only called for cleanup
            break;

        default:
            bBuildFailed = true;
            break;
        }

        m_pProg->Unacquire(devID);
    }

	m_pProg->SetContextDevicesToProgramMappingInternal();

    if (m_pfn_notify)
    {
        m_pfn_notify(program, m_user_data);
    }

    if (m_deviceID)
    {
        delete[] m_deviceID;
        m_deviceID = NULL;
    }

    if (m_ppHeaders)
    {
        delete[] m_ppHeaders;
        m_ppHeaders = NULL;
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

    if (m_szOptions)
    {
        delete[] m_szOptions;
        m_szOptions = NULL;
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

cl_err_code ProgramService::CompileProgram(const SharedPtr<Program>& program, 
                                           cl_uint num_devices, 
                                           const cl_device_id *device_list, 
                                           cl_uint num_input_headers, 
                                           SharedPtr<Program>* input_headers, 
                                           const char **header_include_names, 
                                           const char *options, 
                                           pfnNotifyBuildDone pfn_notify, 
                                           void *user_data)
{
    const char* szProgramSource = program->GetSourceInternal(); 
    if (NULL == szProgramSource)
    {
        return CL_INVALID_OPERATION;
    }

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

    char* szBuildOptions = NULL;

    if (options)
    {
        szBuildOptions = new char[strlen(options) + 1];
        if (NULL == szBuildOptions)
        {
            return CL_OUT_OF_HOST_MEMORY;
        }
        STRCPY_S(szBuildOptions, strlen(options) + 1, options);
    }
    else
    {
        // initialize szBuildOptions to empty string
        szBuildOptions = new char[1];
        if (NULL == szBuildOptions)
        {
            return CL_OUT_OF_HOST_MEMORY;
        }
        szBuildOptions[0] = '\0';
    }

    cl_device_id* pDevices = new cl_device_id[uiNumDevices];
    if (NULL == pDevices)
    {
        delete[] szBuildOptions;
        return CL_OUT_OF_HOST_MEMORY;
    }

    SharedPtr<OclEvent>* ppCompileTasks = new SharedPtr<OclEvent>[uiNumDevices];
    if (NULL == ppCompileTasks)
    {
        delete[] szBuildOptions;
        delete[] pDevices;
        return CL_OUT_OF_HOST_MEMORY;
    }

    for (unsigned int i = 0; i < uiNumDevices; ++i)
    {
        pDevices[i] = NULL;
        ppCompileTasks[i] = NULL;
    }

    // Get the sources for all the headers
    const char** pszHeaders = NULL;
    char** pszHeadersNames = NULL;

    if (0 < num_input_headers)
    {
        pszHeaders = new const char*[num_input_headers];
        if (NULL == pszHeaders)
        {
            delete[] szBuildOptions;
            delete[] ppCompileTasks;
			delete[] pDevices;
			return CL_OUT_OF_HOST_MEMORY;
        }

        pszHeadersNames = new char*[num_input_headers];
        if (!pszHeadersNames)
        {
            delete[] szBuildOptions;
            delete[] ppCompileTasks;
            delete[] pszHeaders;
			delete[] pDevices;
			return CL_OUT_OF_HOST_MEMORY;
        }
    }

    for (unsigned int i = 0; i < num_input_headers; ++i)
    {
        pszHeaders[i] = input_headers[i]->GetSourceInternal();
        if (NULL == pszHeaders[i])
        {
            delete[] szBuildOptions;
            delete[] ppCompileTasks;
            
            for (cl_uint j = 0; j < i; ++j)
            {
                delete[] pszHeadersNames[j];
            }

            delete[] pszHeaders;
            delete[] pszHeadersNames;
			delete[] pDevices;
            return CL_INVALID_OPERATION;
        }

        size_t len = strlen(header_include_names[i]) + 1;
        pszHeadersNames[i] = new char[len];
        if (NULL == pszHeadersNames[i])
        {
            delete[] szBuildOptions;
            delete[] ppCompileTasks;
            
            for (cl_uint j = 0; j < i; ++j)
            {
                delete[] pszHeadersNames[j];
            }

            delete[] pszHeaders;
            delete[] pszHeadersNames;
			delete[] pDevices;
            return CL_OUT_OF_HOST_MEMORY;
        }

        STRCPY_S(pszHeadersNames[i], len, header_include_names[i]);
    }


    if (num_devices > 0)
    {
        //Phase one: Acquire the program for all requested devices 
        uiNumDevices = num_devices;

        for (cl_uint i = 0; i < uiNumDevices; ++i)
        {
            cl_device_id deviceID = device_list[i];
            if (!program->Acquire(deviceID))
            {
                if (0 < i)
                {
                    //Release all accesses already acquired
                    for (int j = (int)i - 1; j >= 0; --j)
                    {
                        program->Unacquire(device_list[j]);
                    }
                }

                delete[] szBuildOptions;
                delete[] pDevices;
                delete[] ppCompileTasks;
  
                for (cl_uint j = 0; j < num_input_headers; ++j)
                {
                    delete[] pszHeadersNames[j];
                }

                delete[] pszHeaders;
                delete[] pszHeadersNames;

                return CL_INVALID_OPERATION;
            }

            pDevices[i] = deviceID;
        }
    }
    else //build for all devices
    {
        //Phase one: Acquire the program for all associated devices
        program->GetDevices(pDevices);

        for (cl_uint i = 0; i < uiNumDevices; ++i)
        {
            cl_device_id deviceID = pDevices[i];
            if (!program->Acquire(deviceID))
            {
                if (0 < i)
                {
                    //Release all accesses already acquired
                    for (int j = (int)i - 1; j >= 0; --j)
                    {
                        program->Unacquire(pDevices[j]);
                    }
                }

                delete[] szBuildOptions;
                delete[] pDevices;
                delete[] ppCompileTasks;
  
                for (cl_uint j = 0; j < num_input_headers; ++j)
                {
                    delete[] pszHeadersNames[j];
                }

                delete[] pszHeaders;
                delete[] pszHeadersNames;

                return CL_INVALID_OPERATION;
            }
        }
    }

    // Check if the compile options are legal for all the devices
    ConstSharedPtr<FrontEndCompiler>* pfeCompilers = new ConstSharedPtr<FrontEndCompiler>[uiNumDevices];
    if (!pfeCompilers)
    {
        for (cl_uint i = 0; i < uiNumDevices; ++i)
        {
            program->Unacquire(pDevices[i]);
        }

        delete[] szBuildOptions;
        delete[] pDevices;
        delete[] ppCompileTasks;

        for (cl_uint i = 0; i < num_input_headers; ++i)
        {
            delete[] pszHeadersNames[i];
        }

        delete[] pszHeaders;
        delete[] pszHeadersNames;

        return CL_OUT_OF_HOST_MEMORY;
    }

    for (unsigned int i = 0; i < uiNumDevices; ++i)
    {
        cl_device_id deviceID = pDevices[i];
        SharedPtr<FissionableDevice> pDevice = m_pContext->GetDevice(deviceID);

        pfeCompilers[i] = pDevice->GetRootDevice()->GetFrontEndCompiler();
        if (NULL == pfeCompilers[i])
        {
          // No FE compiler assigned, need to allocate one
          FrameworkProxy::Instance()->GetPlatformModule()->InitFECompiler(pDevice->GetRootDevice());
          pfeCompilers[i] = pDevice->GetRootDevice()->GetFrontEndCompiler();
        }

        char* szUnrecognizedOptions = new char[strlen(szBuildOptions) + 1];
        if (!pfeCompilers[i]->CheckCompileOptions(szBuildOptions, &szUnrecognizedOptions))
        {
            for (cl_uint j = 0; j < uiNumDevices; ++j)
            {
                program->SetBuildLogInternal(pDevices[j], "Compilation failed\n");
                program->SetBuildLogInternal(pDevices[j], "Unrecognized build options: ");
                program->SetBuildLogInternal(pDevices[j], szUnrecognizedOptions);
                program->SetBuildLogInternal(pDevices[j], "\n");

                program->SetStateInternal(pDevices[j], DEVICE_PROGRAM_COMPILE_FAILED);

                program->Unacquire(pDevices[j]);
            }

            delete[] szBuildOptions;
            delete[] pDevices;
            delete[] ppCompileTasks;

            for (cl_uint j = 0; j < num_input_headers; ++j)
            {
              delete[] pszHeadersNames[j];
            }

            delete[] pszHeaders;
            delete[] pszHeadersNames;
			      delete[] pfeCompilers;
            delete[] szUnrecognizedOptions;
            return CL_INVALID_COMPILER_OPTIONS;
        }
        delete[] szUnrecognizedOptions;
    }


    bool bNeedToBuild = false;  

    for (unsigned int i = 0; i < uiNumDevices; ++i)
    {
        cl_device_id deviceID = pDevices[i];
        ConstSharedPtr<FrontEndCompiler> feCompiler = pfeCompilers[i];

        switch (program->GetStateInternal(deviceID))
        {
        case DEVICE_PROGRAM_COMPILED:
            {
                const char* szLastBuildOptions = program->GetBuildOptionsInternal(deviceID);

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
        case DEVICE_PROGRAM_BUILD_DONE:
        case DEVICE_PROGRAM_BUILD_FAILED:
            {
                // The spec doesn't forbid compilation of built program
                // Intentional fall through.
            }
        case DEVICE_PROGRAM_SOURCE:	
            {
                // Building from source
                bNeedToBuild = true;
                program->SetStateInternal(deviceID, DEVICE_PROGRAM_FE_COMPILING);

                ppCompileTasks[i] = CompileTask::Allocate(context, deviceID, feCompiler, szProgramSource, num_input_headers,
                                                    pszHeaders, pszHeadersNames, szBuildOptions, program);

                if (NULL == ppCompileTasks[i])
                {
                    program->SetStateInternal(deviceID, DEVICE_PROGRAM_BUILD_FAILED);
                }
                
                break;
            }

        case DEVICE_PROGRAM_LINKED:
        case DEVICE_PROGRAM_LOADED_IR:
            // Linked and loaded IR programs shouldn't have source so this should not happen
            assert(false);
        default:
        case DEVICE_PROGRAM_FE_COMPILING:
        case DEVICE_PROGRAM_FE_LINKING:
        case DEVICE_PROGRAM_BE_BUILDING:
            // If we succeeded in acquiring the program, this should not happen 
            assert(false);
        }
    }

    // delete FE compilers array, we don't need it anymore
    delete[] pfeCompilers;

    SharedPtr<PostBuildTask> pPostBuildTask = PostBuildTask::Allocate(context, uiNumDevices, pDevices, num_input_headers, 
                                                                      input_headers, pszHeadersNames, 0, NULL, program, 
                                                                      szBuildOptions, pfn_notify, user_data); 
    if (NULL == pPostBuildTask)
    {
        delete[] ppCompileTasks;
		delete[] pszHeaders;
        return CL_OUT_OF_HOST_MEMORY;
    }

    for (unsigned int i = 0; i < uiNumDevices; ++i)
    {
        if (NULL != ppCompileTasks[i])
        {
            pPostBuildTask->AddDependentOn(ppCompileTasks[i]);
        }
    }

    for (unsigned int i = 0; i < uiNumDevices; ++i)
    {
        if (ppCompileTasks[i])
        {
            SharedPtr<BuildTask> pBuildTask = ppCompileTasks[i].DynamicCast<BuildTask>();
            assert(pBuildTask);
            pBuildTask->Launch();
        }
    }

    // delete compile task array, we don't need it anymore
    delete[] ppCompileTasks;

    // If no build required, launch post build task
    if (!bNeedToBuild)
    {
        SharedPtr<BuildTask> pBuildTask = pPostBuildTask.DynamicCast<BuildTask>();
        assert(pBuildTask);
        pBuildTask->Launch();
    }

    if (NULL == pfn_notify)
    {
        cl_int ret = CL_COMPILE_PROGRAM_FAILURE;

        pPostBuildTask->Wait();
        ret = pPostBuildTask->GetReturnCode();

        if (CL_BUILD_SUCCESS != ret)
        {
			delete[] pszHeaders;
            return CL_COMPILE_PROGRAM_FAILURE;
        }
    }
	delete[] pszHeaders;
    return CL_SUCCESS;
}

cl_err_code ProgramService::LinkProgram(const SharedPtr<Program>& program, 
                                        cl_uint num_devices, 
                                        const cl_device_id *device_list, 
                                        cl_uint num_input_programs, 
                                        SharedPtr<Program>* input_programs, 
                                        const char *options, 
                                        pfnNotifyBuildDone pfn_notify, 
                                        void *user_data)
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

    char* szBuildOptions = NULL;

    if (options)
    {
        szBuildOptions = new char[strlen(options) + 1];
        if (!szBuildOptions)
        {
            return CL_OUT_OF_HOST_MEMORY;
        }
        STRCPY_S(szBuildOptions, strlen(options) + 1, options);
    }
    else
    {
        // initialize szBuildOptions to empty string
        szBuildOptions = new char[1];
        if (!szBuildOptions)
        {
            return CL_OUT_OF_HOST_MEMORY;
        }
        szBuildOptions[0] = '\0';
    }

    cl_device_id* pDevices = new cl_device_id[uiNumDevices];
    if (NULL == pDevices)
    {
        delete[] szBuildOptions;
        return CL_OUT_OF_HOST_MEMORY;
    }

	vector<SharedPtr<OclEvent> > ppLinkTasks(uiNumDevices);

    for (unsigned int i = 0; i < uiNumDevices; ++i)
    {
        pDevices[i] = NULL;
        ppLinkTasks[i] = NULL;
    }

    // Phase one: Get a list of build devices
    if (num_devices > 0)
    {
        uiNumDevices = num_devices;

        for (cl_uint i = 0; i < uiNumDevices; ++i)
        {
            pDevices[i] = device_list[i];
        }
    }
    else //build for all devices
    {     
        // Get all the devices associated with program
        program->GetDevices(pDevices);
    }

    // check that all libraries has valid binary
    vector<bool> pbBuildForDevice(uiNumDevices);

    for (unsigned int devID = 0; devID < uiNumDevices; ++devID)
    {
        unsigned int uiFoundBinaries = 0;

        for (unsigned int libID = 0; libID < num_input_programs; ++libID)
        {
            const char* pBin = input_programs[libID]->GetBinaryInternal(pDevices[devID]);
            cl_prog_container_header* pHeader = (cl_prog_container_header*)pBin;

            if (pHeader)
            {
                cl_prog_binary_type binType = pHeader->description.bin_type;

                if ((CL_PROG_BIN_COMPILED_LLVM == binType) ||
                    (CL_PROG_BIN_LINKED_LLVM == binType) ||
                    (CL_PROG_BIN_COMPILED_SPIR == binType) ||
                    (CL_PROG_BIN_LINKED_SPIR == binType))
                {
                    ++uiFoundBinaries;
                }
            }
        }

        // according to the its all or nothing, either we have binaries for all the devices or none of them
        if (0 == uiFoundBinaries)
        {
            pbBuildForDevice[devID] = false;
            break;
        }

        if (num_input_programs == uiFoundBinaries)
        {
            pbBuildForDevice[devID] = true;
            continue;
        }
        delete[] szBuildOptions;
        delete[] pDevices;
        delete[] input_programs;
        return CL_INVALID_OPERATION;
    }

    // aquire the program for the relevant devices
    for (unsigned int devID = 0; devID < uiNumDevices; ++devID)
    {
        if (pbBuildForDevice[devID])
        {
            if (!program->Acquire(pDevices[devID]))
            {
                if (0 < devID)
                {
                    //Release all accesses already acquired
                    for (int j = (int)devID - 1; j >= 0; --j)
                    {
                        program->Unacquire(pDevices[j]);
                    }
                }
				delete[] szBuildOptions;
				delete[] pDevices;
                return CL_INVALID_OPERATION;
            }
        }
    }

    // // Check if the link options are legal for all the devices
	vector<ConstSharedPtr<FrontEndCompiler> > pfeCompilers(uiNumDevices);
    for (unsigned int i = 0; i < uiNumDevices; ++i)
    {
      cl_device_id deviceID = pDevices[i];
      SharedPtr<FissionableDevice> pDevice = m_pContext->GetDevice(deviceID);

      pfeCompilers[i] = pDevice->GetRootDevice()->GetFrontEndCompiler();
      if (NULL == pfeCompilers[i])
      {
        // No FE compiler assigned, need to allocate one
        FrameworkProxy::Instance()->GetPlatformModule()->InitFECompiler(pDevice->GetRootDevice());
        pfeCompilers[i] = pDevice->GetRootDevice()->GetFrontEndCompiler();
      }

      char* szUnrecognizedOptions = NULL;
      if (!pfeCompilers[i]->CheckLinkOptions(szBuildOptions, &szUnrecognizedOptions))
      {
        for (cl_uint j = 0; j < uiNumDevices; ++j)
        {
          program->Unacquire(pDevices[j]);
        }
		delete[] szBuildOptions;
		delete[] pDevices;
        return CL_INVALID_LINKER_OPTIONS;
      }
    }

    bool bNeedToBuild = false;

    for (unsigned int i = 0; i < uiNumDevices; ++i)
    {
        cl_device_id deviceID = pDevices[i];
        SharedPtr<FissionableDevice> pDevice = m_pContext->GetDevice(deviceID);

        ConstSharedPtr<FrontEndCompiler> feCompiler = pfeCompilers[i];
        IOCLDeviceAgent* pDeviceAgent = pDevice->GetDeviceAgent();

        switch (program->GetStateInternal(deviceID))
        {
        case DEVICE_PROGRAM_INVALID:
            {
                // program is created specially for linking purposes so no action should have been done yet
                if (pbBuildForDevice[i])
                {
                    bNeedToBuild = true;

                    program->SetStateInternal(deviceID, DEVICE_PROGRAM_FE_LINKING);
                    ppLinkTasks[i] = LinkTask::Allocate(context, deviceID, feCompiler, pDeviceAgent, input_programs, num_input_programs, 
                                                  szBuildOptions, program);
                    if (NULL == ppLinkTasks[i])
                    {
                        program->SetStateInternal(deviceID, DEVICE_PROGRAM_BUILD_FAILED);
                    }

                    program->ClearBuildLogInternal(deviceID);
                    program->SetBuildOptionsInternal(deviceID, options);
                }  

				continue;
            }
        case DEVICE_PROGRAM_BUILD_DONE:
        case DEVICE_PROGRAM_BUILD_FAILED:
        case DEVICE_PROGRAM_SOURCE:	
        case DEVICE_PROGRAM_COMPILED:
        case DEVICE_PROGRAM_LINKED:
        case DEVICE_PROGRAM_LOADED_IR:
        case DEVICE_PROGRAM_FE_COMPILING:
        case DEVICE_PROGRAM_FE_LINKING:
        case DEVICE_PROGRAM_BE_BUILDING:
        default:
            // Program for link should never be in one of these states when calling LinkProgram
            assert(false);
        }
    }

	SharedPtr<PostBuildTask> pPostBuildTask = PostBuildTask::Allocate(context, uiNumDevices, pDevices, 0, NULL, NULL,
                                                      num_input_programs, input_programs, program, szBuildOptions, 
                                                      pfn_notify, user_data);

    if (NULL == pPostBuildTask)
    {
		delete[] szBuildOptions;
		delete[] pDevices;
        return CL_OUT_OF_HOST_MEMORY;
    }

    for (unsigned int i = 0; i < uiNumDevices; ++i)
    {
        if (NULL != ppLinkTasks[i])
        {
            pPostBuildTask->AddDependentOn(ppLinkTasks[i]);
        }
    }

    for (unsigned int i = 0; i < uiNumDevices; ++i)
    {
        if (ppLinkTasks[i])
        {
            SharedPtr<BuildTask> pBuildTask = ppLinkTasks[i].DynamicCast<BuildTask>();
            assert(pBuildTask);
            pBuildTask->Launch();
        }
    }

    // If no build required, launch post build task
    if (!bNeedToBuild)
    {
        SharedPtr<BuildTask> pBuildTask = pPostBuildTask.DynamicCast<BuildTask>();
        assert(pBuildTask);
        pBuildTask->Launch();
    }

    if (NULL == pfn_notify)
    {
        cl_int ret = CL_LINK_PROGRAM_FAILURE;

        pPostBuildTask->Wait();
        ret = pPostBuildTask->GetReturnCode();

        if (CL_BUILD_SUCCESS != ret)
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

    if ( NULL != program.DynamicCast<ProgramWithBuiltInKernels>())
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

    char* szBuildOptions = NULL;

    if (options)
    {
        szBuildOptions = new char[strlen(options) + 1];
        if (!szBuildOptions)
        {
            return CL_OUT_OF_HOST_MEMORY;
        }
        STRCPY_S(szBuildOptions, strlen(options) + 1, options);
    }
    else
    {
        // initialize szBuildOptions to empty string
        szBuildOptions = new char[1];
        if (!szBuildOptions)
        {
            return CL_OUT_OF_HOST_MEMORY;
        }
        szBuildOptions[0] = '\0';
    }

    cl_device_id* pDevices = new cl_device_id[uiNumDevices];
    if (NULL == pDevices)
    {
        delete[] szBuildOptions;
        return CL_OUT_OF_HOST_MEMORY;
    }

    SharedPtr<OclEvent>* ppCompileTasks = new SharedPtr<OclEvent>[uiNumDevices];
    if (NULL == ppCompileTasks)
    {
        delete[] szBuildOptions;
        delete[] pDevices;
        return CL_OUT_OF_HOST_MEMORY;
    }

    SharedPtr<OclEvent>* ppLinkTasks = new SharedPtr<OclEvent>[uiNumDevices];
    if (NULL == ppLinkTasks)
    {
        delete[] szBuildOptions;
        delete[] pDevices;
        delete[] ppCompileTasks;
        return CL_OUT_OF_HOST_MEMORY;
    }

    for (unsigned int i = 0; i < uiNumDevices; ++i)
    {
        pDevices[i] = NULL;
        ppCompileTasks[i] = NULL;
        ppLinkTasks[i] = NULL;
    }


    if (num_devices > 0)
    {
        //Phase one: Acquire the program for all requested devices 
        uiNumDevices = num_devices;

        for (cl_uint i = 0; i < uiNumDevices; ++i)
        {
            cl_device_id deviceID = device_list[i];
            if (!program->Acquire(deviceID))
            {
                if (0 < i)
                {
                    //Release all accesses already acquired
                    for (int j = (int)i - 1; j >= 0; --j)
                    {
                        program->Unacquire(device_list[j]);
                    }
                }

                delete[] szBuildOptions;
                delete[] pDevices;
                delete[] ppCompileTasks;
                delete[] ppLinkTasks;
                return CL_INVALID_OPERATION;
            }

            pDevices[i] = deviceID;
        }
    }
    else //build for all devices
    {
        //Phase one: Acquire the program for all associated devices
        program->GetDevices(pDevices);

        for (cl_uint i = 0; i < uiNumDevices; ++i)
        {
            cl_device_id deviceID = pDevices[i];
            if (!program->Acquire(deviceID))
            {
                if (0 < i)
                {
                    //Release all accesses already acquired
                    for (int j = (int)i - 1; j >= 0; --j)
                    {
                        program->Unacquire(device_list[j]);
                    }
                }

                delete[] szBuildOptions;
                delete[] pDevices;
                delete[] ppCompileTasks;
                delete[] ppLinkTasks;
                return CL_INVALID_OPERATION;
            }
        }
    }

    // Check if the build options are legal for all the devices
    ConstSharedPtr<FrontEndCompiler>* pfeCompilers = new ConstSharedPtr<FrontEndCompiler>[uiNumDevices];
    if (!pfeCompilers)
    {
        for (cl_uint i = 0; i < uiNumDevices; ++i)
        {
            program->Unacquire(pDevices[i]);
        }

        delete[] szBuildOptions;
        delete[] pDevices;
        delete[] ppCompileTasks;
        delete[] ppLinkTasks;

        return CL_OUT_OF_HOST_MEMORY;
    }

    for (unsigned int i = 0; i < uiNumDevices; ++i)
    {
      cl_device_id deviceID = pDevices[i];
      SharedPtr<FissionableDevice> pDevice = m_pContext->GetDevice(deviceID);

      pfeCompilers[i] = pDevice->GetRootDevice()->GetFrontEndCompiler();
      if (NULL == pfeCompilers[i])
      {
        // No FE compiler assigned, need to allocate one
        FrameworkProxy::Instance()->GetPlatformModule()->InitFECompiler(pDevice->GetRootDevice());
        pfeCompilers[i] = pDevice->GetRootDevice()->GetFrontEndCompiler();
      }

      char* szUnrecognizedOptions = new char[strlen(szBuildOptions) + 1];
      if (!pfeCompilers[i]->CheckCompileOptions(szBuildOptions, &szUnrecognizedOptions))
      {
          for (cl_uint j = 0; j < uiNumDevices; ++j)
          {
            program->SetBuildLogInternal(pDevices[j], "Compilation failed\n");
            program->SetBuildLogInternal(pDevices[j], "Unrecognized build options: ");
            program->SetBuildLogInternal(pDevices[j], szUnrecognizedOptions);
            program->SetBuildLogInternal(pDevices[j], "\n");

            program->SetStateInternal(pDevices[j], DEVICE_PROGRAM_COMPILE_FAILED);

            program->Unacquire(pDevices[j]);
          }

          delete[] szBuildOptions;
          delete[] pDevices;
          delete[] ppCompileTasks;
          delete[] ppLinkTasks;
          delete[] szUnrecognizedOptions;
		      delete[] pfeCompilers;
          return CL_INVALID_BUILD_OPTIONS;
      }
      delete[] szUnrecognizedOptions;
    }

    bool bNeedToBuild = false;

    const char* szProgramSource = program->GetSourceInternal();   

    for (unsigned int i = 0; i < uiNumDevices; ++i)
    {
        cl_device_id deviceID = pDevices[i];
        SharedPtr<FissionableDevice> pDevice = m_pContext->GetDevice(deviceID);

        ConstSharedPtr<FrontEndCompiler> feCompiler = pfeCompilers[i];
        IOCLDeviceAgent* pDeviceAgent = pDevice->GetDeviceAgent();

        switch (program->GetStateInternal(deviceID))
        {
        case DEVICE_PROGRAM_BUILD_DONE:
            {
                const char* szLastBuildOptions = program->GetBuildOptionsInternal(deviceID);

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
        case DEVICE_PROGRAM_BUILD_FAILED:
            {
                // Possibly retrying a failed build - legal
                if (!szProgramSource)
                {
                    //invalid binaries are hopeless
                    //remember build options even if build will fail
                    program->SetBuildOptionsInternal(deviceID, options);
                    break;
                }
                //Intentional fall through.
            }
        case DEVICE_PROGRAM_SOURCE:	
            {
                // Building from source
                bNeedToBuild = true;
                program->SetStateInternal(deviceID, DEVICE_PROGRAM_FE_COMPILING);
                ppCompileTasks[i] = CompileTask::Allocate(context, deviceID, feCompiler, szProgramSource, 0, NULL, NULL, 
                                                    szBuildOptions, program);

                if (NULL == ppCompileTasks[i])
                {
                    program->SetStateInternal(deviceID, DEVICE_PROGRAM_BUILD_FAILED);
                }
                //Intentional fall through.
            }
        case DEVICE_PROGRAM_COMPILED:
        case DEVICE_PROGRAM_LINKED:
        case DEVICE_PROGRAM_LOADED_IR:
            {
                // Building from bin
                bNeedToBuild = true;
                program->SetStateInternal(deviceID, DEVICE_PROGRAM_FE_LINKING);
                ppLinkTasks[i] = LinkTask::Allocate(context, deviceID, feCompiler, pDeviceAgent, NULL, 0, 
                                              szBuildOptions, program);
                if (NULL == ppLinkTasks[i])
                {
                    program->SetStateInternal(deviceID, DEVICE_PROGRAM_BUILD_FAILED);
                }
                else if (NULL != ppCompileTasks[i])
                {
                    ppLinkTasks[i]->AddDependentOn(ppCompileTasks[i]);
                }

                program->ClearBuildLogInternal(deviceID);
                program->SetBuildOptionsInternal(deviceID, options);
                break;
            }
        default:
        case DEVICE_PROGRAM_FE_COMPILING:
        case DEVICE_PROGRAM_FE_LINKING:
        case DEVICE_PROGRAM_BE_BUILDING:
            // If we succeeded in acquiring the program, this should not happen 
            assert(false);
        }
    }

    // delete FE compilers array, we don't need it anymore
    delete[] pfeCompilers;

    SharedPtr<PostBuildTask> pPostBuildTask = PostBuildTask::Allocate(context, uiNumDevices, pDevices, 0, NULL, NULL,
                                                                      0, NULL, program, szBuildOptions, 
                                                                      pfn_notify, user_data);

    if (NULL == pPostBuildTask)
    {
		delete[] ppCompileTasks;
		delete[] ppLinkTasks;
        return CL_OUT_OF_HOST_MEMORY;
    }

    for (unsigned int i = 0; i < uiNumDevices; ++i)
    {
        if (NULL != ppLinkTasks[i])
        {
            pPostBuildTask->AddDependentOn(ppLinkTasks[i]);
        }
    }

    for (unsigned int i = 0; i < uiNumDevices; ++i)
    {
        if (ppCompileTasks[i])
        {
            SharedPtr<BuildTask> pBuildTask = ppCompileTasks[i].DynamicCast<BuildTask>();
            assert(pBuildTask);
            pBuildTask->Launch();
        }
    }

    // if building from binary we need to launch linkTask explicitly
    for (unsigned int i = 0; i < uiNumDevices; ++i)
    {
        if (NULL == ppCompileTasks[i])
        {
            if (ppLinkTasks[i])
            {
                SharedPtr<BuildTask> pBuildTask = ppLinkTasks[i].DynamicCast<BuildTask>();
                assert(pBuildTask);
                pBuildTask->Launch();
            }
        }
    }

    // delete compile and link task arrays, we don't need them anymore
    delete[] ppCompileTasks;
    delete[] ppLinkTasks;

    // If no build required, launch post build task
    if (!bNeedToBuild)
    {
        SharedPtr<BuildTask> pBuildTask = pPostBuildTask.DynamicCast<BuildTask>();
        assert(pBuildTask);
        pBuildTask->Launch();
    }

    if (NULL == pfn_notify)
    {
        cl_int ret = CL_BUILD_PROGRAM_FAILURE;

        pPostBuildTask->Wait();
        ret = pPostBuildTask->GetReturnCode();

        if (CL_BUILD_SUCCESS != ret)
        {
            return CL_BUILD_PROGRAM_FAILURE;
        }
    }

    return CL_SUCCESS;
}
