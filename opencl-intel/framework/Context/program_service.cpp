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
#include "framework_proxy.h"
#include "events_manager.h"
#include "cl_device_api.h"

#include "Device.h"


using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::TaskExecutor;


BuildTask::BuildTask(cl_context context) : BuildEvent(context)
{
}

BuildTask::~BuildTask()
{
}

long BuildTask::Release()
{
    RemovePendency(NULL);
    return 0;
}

void BuildTask::NotifyReady(OclEvent* pEvent)
{
    Launch();
    BuildEvent::NotifyReady(pEvent);
}

unsigned int BuildTask::Launch()
{
    return TaskExecutor::GetTaskExecutor()->Execute(this);
}

CompileTask::CompileTask(cl_context              context,
                         cl_device_id            deviceID,
                         const FrontEndCompiler* pFECompiler,
                         const char*             szSource,
                         unsigned int            uiNumHeaders,
                         const char**            pszHeaders,
                         const char**            pszHeadersNames,
                         const char*             szOptions,
                         Program*                pProg) : 
BuildTask(context), 
m_deviceID(deviceID), m_pFECompiler(pFECompiler), m_szSource(szSource), m_uiNumHeaders(uiNumHeaders),
m_pszHeaders(pszHeaders), m_pszHeadersNames(pszHeadersNames), m_szOptions(szOptions), m_pProg(pProg)
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
        m_pProg->SetStateInternal(m_deviceID, DEVICE_PROGRAM_COMPILE_FAILED);
        SetComplete(CL_BUILD_SUCCESS);
        return true;
    }

    //Else compile succeeded
    m_pProg->SetBinaryInternal(m_deviceID, uiBinarySize, pBinary);
    delete[] pBinary;
    SetComplete(CL_BUILD_SUCCESS);

	return true;
}

LinkTask::LinkTask(cl_context               context, 
                   cl_device_id             deviceID, 
                   const FrontEndCompiler*  pFECompiler, 
                   IOCLDeviceAgent*   pDeviceAgent, 
                   Program**                ppPrograms, 
                   unsigned int             uiNumPrograms, 
                   const char*              szOptions, 
                   Program*                 pProg) :
BuildTask(context), 
m_deviceID(deviceID), m_pFECompiler(pFECompiler), m_pDeviceAgent(pDeviceAgent), m_ppPrograms(ppPrograms), 
m_uiNumPrograms(uiNumPrograms), m_szOptions(szOptions), m_pProg(pProg)
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

    if (m_pProg->GetStateInternal(m_deviceID) == DEVICE_PROGRAM_COMPILE_FAILED)
	{
        SetComplete(CL_BUILD_SUCCESS);
		return true;
	}

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
                               &szLinkLog);

    delete[] ppBinaries;
    delete[] puiBinariesSizes;

    if (NULL != szLinkLog)
    {
        m_pProg->SetBuildLogInternal(m_deviceID, szLinkLog);
        delete[] szLinkLog;
    }

    if (0 == uiBinarySize)
    {
        assert( NULL == pBinary);
        //Build failed
        m_pProg->SetStateInternal(m_deviceID, DEVICE_PROGRAM_LINK_FAILED);
        SetComplete(CL_BUILD_SUCCESS);
        return true;
    }

    //Else link succeeded
    m_pProg->SetBinaryInternal(m_deviceID, uiBinarySize, pBinary);

    // TODO: check for -create-library
    cl_dev_program      programHandle;
    cl_build_status     build_status;

    cl_prog_container_header*	pHeader = (cl_prog_container_header*)pBinary;
    pHeader->description.bin_type = CL_PROG_BIN_EXECUTABLE_LLVM;

    cl_dev_err_code err = m_pDeviceAgent->clDevCreateProgram(uiBinarySize, pBinary, CL_DEV_BINARY_COMPILER, &programHandle);
    if (CL_DEV_SUCCESS != err)
    {
        //Build failed
        delete[] pBinary;
        m_pProg->SetStateInternal(m_deviceID, DEVICE_PROGRAM_LINK_FAILED);
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
        SetComplete(CL_BUILD_SUCCESS);
		return true;
	}

    SetComplete(CL_BUILD_SUCCESS);
	return true;
}

PostBuildTask::PostBuildTask(cl_context context, 
                             cl_uint num_devices, 
                             const cl_device_id *deviceID, 
                             unsigned int uiNumHeaders, 
                             Program **ppHeaders, 
                             unsigned int uiNumBinaries, 
                             Program **ppBinaries, 
                             Program *pProg,
                             const char* szOptions,
                             pfnNotifyBuildDone pfn_notify, 
                             void *user_data) :
BuildTask(context),
m_num_devices(num_devices), m_deviceID(deviceID), m_uiNumHeaders(uiNumHeaders), m_ppHeaders(ppHeaders),
m_uiNumBinaries(uiNumBinaries), m_ppBinaries(ppBinaries), m_pProg(pProg), m_szOptions(szOptions),
m_pfn_notify(pfn_notify), m_user_data(user_data)
{
}

PostBuildTask::~PostBuildTask()
{
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
}

bool PostBuildTask::Execute()
{
    cl_program program = m_pProg->GetHandle();

    for (unsigned int i = 0; i < m_uiNumHeaders; ++i)
    {
        m_ppHeaders[i]->RemovePendency(m_pProg);
    }

    for (unsigned int binID = 0; binID < m_uiNumBinaries; ++binID)
    {
        for (unsigned int i = 0; i < m_num_devices; ++i)
        {
            m_ppBinaries[binID]->Unacquire(m_deviceID[i]);
        }

        m_ppBinaries[binID]->RemovePendency(m_pProg);
    }

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

        default:
            break;
        }

        m_pProg->Unacquire(devID);
    }

    if (m_pfn_notify)
    {
        m_pfn_notify(program, m_user_data);
    }

    SetComplete(CL_BUILD_SUCCESS);

	return true;
}


ProgramService::ProgramService(Context* pContext) : m_pContext(pContext)
{
}

ProgramService::~ProgramService()
{
}

cl_err_code ProgramService::CompileProgram(Program *program, 
                                           cl_uint num_devices, 
                                           cl_device_id *device_list, 
                                           cl_uint num_input_headers, 
                                           Program** input_headers, 
                                           const char **header_include_names, 
                                           const char *options, 
                                           pfnNotifyBuildDone pfn_notify, 
                                           void *user_data)
{
    ////TODO: Acquire all relevent programs and check them

    //cl_context context = m_pContext->GetHandle();

    //const char* szProgramSource = program->GetSourceInternal();
    //const char** pszInputHeaders = new const char*[num_input_headers];

    //for (unsigned int i = 0; i < num_input_headers; ++i)
    //{
    //    pszInputHeaders[i] = input_headers[i]->GetSourceInternal();
    //}

    //BuildTask** ppCompileTasks = new BuildTask*[num_devices];

    //for (unsigned int i = 0; i < num_devices; ++i)
    //{
    //    cl_device_id deviceID = device_list[i];
    //    FissionableDevice* pDevice;
    //    m_pContext->GetDevice(deviceID, &pDevice);
    //    const FrontEndCompiler* feCompiler = pDevice->GetRootDevice()->GetFrontEndCompiler();

    //    ppCompileTasks[i] = new CompileTask(context, deviceID, feCompiler, szProgramSource, num_input_headers,
    //                                               pszInputHeaders, header_include_names, options, program);
    //}

    //PostBuildTask* pPostBuildTask = new PostBuildTask(context, num_devices, device_list, num_input_headers,
    //                                                  input_headers, 0, NULL, program, szBuildOptions,
    //                                                  pfn_notify, user_data);
    //pPostBuildTask->AddDependentOnMulti(num_devices, ppCompileTasks);

    //for (unsigned int i = 0; i < num_devices; ++i)
    //{
    //    ppCompileTasks[i]->Launch();
    //}

    //if (NULL == pfn_notify)
    //{
    //    pPostBuildTask->Wait();
    //}

    return CL_COMPILE_PROGRAM_FAILURE;
}

cl_err_code ProgramService::LinkProgram(Program *program, 
                                        cl_uint num_devices, 
                                        cl_device_id *device_list, 
                                        cl_uint num_input_programs, 
                                        Program** input_programs, 
                                        const char *options, 
                                        pfnNotifyBuildDone pfn_notify, 
                                        void *user_data)
{
    ////TODO: Acquire all relevent programs and check them

    //cl_context context = m_pContext->GetHandle();

    //BuildTask** ppLinkTasks = new BuildTask*[num_devices];

    //for (unsigned int i = 0; i < num_devices; ++i)
    //{
    //    cl_device_id deviceID = device_list[i];
    //    FissionableDevice* pDevice;
    //    m_pContext->GetDevice(deviceID, &pDevice);
    //    const FrontEndCompiler* feCompiler = pDevice->GetRootDevice()->GetFrontEndCompiler();
    //    IOCLDeviceAgent* pDeviceAgent = pDevice->GetDeviceAgent();

    //    ppLinkTasks[i] = new LinkTask(context, deviceID, feCompiler, pDeviceAgent, input_programs, num_input_programs, options, program);
    //}

    //PostBuildTask* pPostBuildTask = new PostBuildTask(context, num_devices, device_list, 0, NULL, 
    //                                                  num_input_programs, input_programs, program, szBuildOptions,
    //                                                  pfn_notify, user_data);
    //pPostBuildTask->AddDependentOnMulti(num_devices, ppLinkTasks);

    //for (unsigned int i = 0; i < num_devices; ++i)
    //{
    //    ppLinkTasks[i]->Launch();
    //}

    //if (NULL == pfn_notify)
    //{
    //    pPostBuildTask->Wait();
    //}

    return CL_LINK_PROGRAM_FAILURE;
}

cl_err_code ProgramService::BuildProgram(Program *program, cl_uint num_devices, const cl_device_id *device_list, const char *options, pfnNotifyBuildDone pfn_notify, void *user_data)
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

    cl_context context = m_pContext->GetHandle();
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
    if (!pDevices)
    {
        delete[] szBuildOptions;
        return CL_OUT_OF_HOST_MEMORY;
    }

    OclEvent** ppCompileTasks = new OclEvent*[uiNumDevices];
    if (!ppCompileTasks)
    {
        delete[] szBuildOptions;
        delete[] pDevices;
        return CL_OUT_OF_HOST_MEMORY;
    }

    OclEvent** ppLinkTasks = new OclEvent*[uiNumDevices];
    if (!ppLinkTasks)
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
                //Release all accesses already acquired
				for (cl_uint j = i - 1; j >= 0; --j)
				{
                    program->Unacquire(device_list[j]);
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
                    for (cl_uint j = i - 1; j >= 0; --j)
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
    
    bool bNeedToBuild = false;

    const char* szProgramSource = program->GetSourceInternal();   

    for (unsigned int i = 0; i < uiNumDevices; ++i)
    {
        ppCompileTasks[i] = NULL;
        ppLinkTasks[i] = NULL;

        cl_device_id deviceID = pDevices[i];
        FissionableDevice* pDevice;
        m_pContext->GetDevice(deviceID, &pDevice);

        const FrontEndCompiler* feCompiler = pDevice->GetRootDevice()->GetFrontEndCompiler();
        if (NULL == feCompiler)
        {
            // No FE compiler assigned, need to allocate one
            FrameworkProxy::Instance()->GetPlatformModule()->InitFECompiler(pDevice->GetRootDevice());
            feCompiler = pDevice->GetRootDevice()->GetFrontEndCompiler();
        }

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
		        //Intetional fall through.
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
                //Intetional fall through.
            }
	    case DEVICE_PROGRAM_SOURCE:	
            {
		        // Building from source
                bNeedToBuild = true;
                program->SetStateInternal(deviceID, DEVICE_PROGRAM_FE_COMPILING);
                ppCompileTasks[i] = new CompileTask(context, deviceID, feCompiler, szProgramSource, 0, NULL, NULL, 
                                                    szBuildOptions, program);

                if (!ppCompileTasks[i])
                {
                    program->SetStateInternal(deviceID, DEVICE_PROGRAM_BUILD_FAILED);
                }
                //Intetional fall through.
            }
	    case DEVICE_PROGRAM_COMPILED:
        case DEVICE_PROGRAM_LINKED:
        case DEVICE_PROGRAM_LOADED_IR:
            {
		        // Building from bin
                bNeedToBuild = true;
                program->SetStateInternal(deviceID, DEVICE_PROGRAM_FE_LINKING);
                ppLinkTasks[i] = new LinkTask(context, deviceID, feCompiler, pDeviceAgent, NULL, 0, 
                                              szBuildOptions, program);
                if (!ppLinkTasks[i])
                {
                    program->SetStateInternal(deviceID, DEVICE_PROGRAM_BUILD_FAILED);
                }

                ppLinkTasks[i]->AddDependentOn(ppCompileTasks[i]);

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

    PostBuildTask* pPostBuildTask = new PostBuildTask(context, uiNumDevices, pDevices, 0, NULL, 
                                                      0, NULL, program, szBuildOptions, 
                                                      pfn_notify, user_data);
    pPostBuildTask->AddDependentOnMulti(uiNumDevices, ppLinkTasks);


    for (unsigned int i = 0; i < uiNumDevices; ++i)
    {
        if (ppCompileTasks[i])
        {
            BuildTask* pBuildTask = dynamic_cast<BuildTask*>(ppCompileTasks[i]);
            assert(pBuildTask);
            pBuildTask->Launch();
        }
    }

    // if building from binary we need to launch linkTask explicitly
    for (unsigned int i = 0; i < uiNumDevices; ++i)
    {
        if (!ppCompileTasks[i])
        {
            if (ppLinkTasks[i])
            {
                BuildTask* pBuildTask = dynamic_cast<BuildTask*>(ppLinkTasks[i]);
                assert(pBuildTask);
                pBuildTask->Launch();
            }
        }
    }

    // If no build required, launch post build task
    if (!bNeedToBuild)
    {
        BuildTask* pBuildTask = dynamic_cast<BuildTask*>(pPostBuildTask);
        assert(pBuildTask);
        pBuildTask->Launch();
    }

    if (NULL == pfn_notify)
    {
        pPostBuildTask->Wait();
    }

    return CL_SUCCESS;
}
