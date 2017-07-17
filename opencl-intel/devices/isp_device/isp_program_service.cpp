// Copyright (c) 2014 Intel Corporation
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

///////////////////////////////////////////////////////////
//  isp_program_service.cpp
//  Implementation of the Class ISPProgramService
//  Class Object is responsible on programs and kernels binaries
///////////////////////////////////////////////////////////

#include "stdafx.h"

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

#include "isp_program_service.h"
#include "isp_logger.h"
#include "isp_utils.h"

//#include "css/sh_css_firmware.h"
//#include "css/sh_css_types.h"
#include "css/css_types.h"

#include <cl_sys_defines.h>

using namespace Intel::OpenCL::ISPDevice;

// TODO: find if this value relevant to CSS. Can we change it ?
#define KERNEL_NAME_MAX_LENGTH 32

#define ISP_FIRMWARE_FILE_EXTENSION "bin"

ISPProgramService::ISPProgramService(cl_int devId, IOCLDevLogDescriptor *logDesc, CameraShim* pCameraShim) :
    m_iDevId(devId), m_pLogDescriptor(logDesc), m_pCameraShim(pCameraShim), m_pBuiltInKernelRegistry(nullptr)
{
    if (nullptr != logDesc)
    {
        cl_int ret = m_pLogDescriptor->clLogCreateClient(m_iDevId, "ISP Device: Program Service", &m_iLogHandle);
        if (CL_DEV_FAILED(ret))
        {
            m_iLogHandle = 0;
        }
    }

    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("ISPDevice: Program Service - Constructed"));
}

ISPProgramService::~ISPProgramService()
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("ISPDevice: Program Service - Destructed"));

    if (0 != m_iLogHandle)
    {
        m_pLogDescriptor->clLogReleaseClient(m_iLogHandle);
    }

    if (nullptr != m_pBuiltInKernelRegistry)
    {
        delete m_pBuiltInKernelRegistry;
    }
}

cl_dev_err_code ISPProgramService::Init()
{
    m_pBuiltInKernelRegistry = new BuiltInKernelRegistry(m_pCameraShim);
    if (nullptr == m_pBuiltInKernelRegistry)
    {
        return CL_DEV_OUT_OF_MEMORY;
    }

    // Add built-in commands
    cl_dev_err_code ret = m_pBuiltInKernelRegistry->AddCameraCommands();
    if (CL_DEV_FAILED(ret))
    {
        return CL_DEV_ERROR_FAIL;
    }
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Camera built-in commands added"));
/*
    // Add built-in firmwares
    // TODO: Put in a config file or so
    std::string dirToSearch = "/etc/firmware/";
    ret = FindFirmwares(dirToSearch);
    if (CL_DEV_FAILED(ret))
    {
        return CL_DEV_ERROR_FAIL;
    }
    //FindFirmwares("/system/etc/firmware/");
    dirToSearch = "/data/kernels/";
    ret = FindFirmwares(dirToSearch);
    if (CL_DEV_FAILED(ret))
    {
        return CL_DEV_ERROR_FAIL;
    }*/

    std::string dirToSearch = "/data/kernels/";
    ret = FindFirmwares(dirToSearch);
    if (CL_DEV_FAILED(ret))
    {
        return CL_DEV_ERROR_FAIL;
    }

    return CL_DEV_SUCCESS;
}

cl_dev_err_code ISPProgramService::FindFirmwares(std::string dir)
{
    std::vector< std::string > fileList;

    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Looking for firmwares in: "), dir.c_str());

    int numFilesFound = SearchFilesInDirectory(dir, ISP_FIRMWARE_FILE_EXTENSION, fileList);
    if (0 == numFilesFound)
    {
        // no firmwares found - skip
        IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s%s"), TEXT("No firmwares were found in "), dir.c_str());
        return CL_DEV_SUCCESS;
    }

    size_t headerSize = sizeof(struct ::sh_css_fw_info) + KERNEL_NAME_MAX_LENGTH;

    std::vector< std::string >::const_iterator filename = fileList.begin();
    for ( ; filename != fileList.end(); ++filename)
    {
        IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s%s"), TEXT("Trying to read file: "), filename->c_str());

        // open the file
        std::ifstream file(filename->c_str(), std::ifstream::in | std::ifstream::binary | std::ifstream::ate);
        if (!file.is_open())
        {
            IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s%s"), TEXT("Cannot open file - skipping: "), filename->c_str());
            continue;
        }

        // check file size
        size_t fileSize = file.tellg();
        if (fileSize < headerSize)
        {
            IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s%s"), TEXT("Firmware file is too small - skipping: "), filename->c_str());
            file.close();
            continue;
        }

        // allocate memory for the file
        char * fileContent = new char[fileSize];
        if (nullptr == fileContent)
        {
            IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s%s"), TEXT("Cannot allocate memory for file - skipping: "), filename->c_str());
            file.close();
            continue;
        }

        // read the file and close it
        file.seekg(0, std::ifstream::beg);
        file.read(fileContent, fileSize);
        file.close();

        // check if valid binary // TODO
        /*if (CL_DEV_FAILED(CheckProgramBinary(fileSize, fileContent)))
        {
            IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s%s"), TEXT("Not a valid firmware file - skipping: "), filename->c_str());
            delete[] fileContent;
            continue;
        }*/

        // add the firmware to our built-ins
        if (CL_DEV_FAILED(m_pBuiltInKernelRegistry->AddFirmware(fileContent, fileSize)))
        {
            IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s%s"), TEXT("Cannot add firmware - skipping: "), filename->c_str());
            delete[] fileContent;
            continue;
        }

        delete[] fileContent;
        IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s%s"), TEXT("Added firmware: "), filename->c_str());
    }

    return CL_DEV_SUCCESS;
}


/****************************************************************************************************************
 CheckProgramBinary
    Description
        Performs syntax validation of the intermediate or binary to be built by the device during later stages.
        Call backend compiler to do the check
    Input
        bin_size                Size of the binary buffer
        bin                     A pointer to binary buffer that holds program container defined by cl_prog_container.
    Output
        NONE
    Returns
        CL_DEV_SUCCESS          The function is executed successfully.
        CL_DEV_INVALID_VALUE    If bin_size is 0 or bin is NULL.
        CL_DEV_INVALID_BINARY   If the binary is not supported by the device or program container content is invalid.
********************************************************************************************************************/
cl_dev_err_code ISPProgramService::CheckProgramBinary(size_t IN binSize, const void* IN bin)
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("CheckProgramBinary enter"));

    if (sizeof(struct sh_css_fw_info) > binSize)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Invalid program binary"));
        return CL_DEV_INVALID_VALUE;
    }

    const sh_css_fw_info* pCssHeader = (struct sh_css_fw_info*) bin;

    if (sizeof(struct sh_css_fw_info) != pCssHeader->header_size)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("Invalid header size: %d"), pCssHeader->header_size);
        return CL_DEV_INVALID_VALUE;
    }

    if (sh_css_isp_firmware != pCssHeader->type)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("Invalid firmware type: %d"), pCssHeader->type);
        return CL_DEV_INVALID_VALUE;
    }

    if (SH_CSS_MAX_NUM_FRAME_FORMATS < pCssHeader->info.isp.num_output_formats)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("Too many output formats: %d"), pCssHeader->info.isp.num_output_formats);
        return CL_DEV_INVALID_VALUE;
    }

    for (int i = 0; i < pCssHeader->info.isp.num_output_formats; i++)
    {
        if (N_SH_CSS_FRAME_FORMAT <= pCssHeader->info.isp.output_formats[i])
        {
            IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("Invalid output format (%d) at %d"), pCssHeader->info.isp.output_formats[i], i);
            return CL_DEV_INVALID_VALUE;
        }
    }

    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Program passed binary validation"));

    IspDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("binary ISP address: %p"), pCssHeader->info.isp.isp_addresses);
    IspDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("binary main entry: %p"), pCssHeader->info.isp.main_entry);
    IspDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("binary input frame: %p"), pCssHeader->info.isp.in_frame);
    IspDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("binary output frame: %p"), pCssHeader->info.isp.out_frame);
    IspDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("binary input data: %p"), pCssHeader->info.isp.in_data);
    IspDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("binary output data: %p"), pCssHeader->info.isp.out_data);

    return CL_DEV_SUCCESS;
}

/*******************************************************************************************************************
CreateProgram
    Description
        Creates a device specific program entity (no build is performed).
    Input
        bin_size                        Size of the binary buffer
        bin                             A pointer to binary buffer that holds program container defined by cl_prog_container.
        prop                            Specifies the origin of the input binary. The values is defined by cl_dev_binary_prop.
    Output
        prog                            A handle to created program object.
    Returns
        CL_DEV_SUCCESS                  The function is executed successfully.
        CL_DEV_INVALID_BINARY           If the back-end compiler failed to process binary.
        CL_DEV_OUT_OF_MEMORY            If the device failed to allocate memory for the program
***********************************************************************************************************************/
cl_dev_err_code ISPProgramService::CreateProgram(size_t IN binSize,
                                              const void* IN bin,
                                              cl_dev_binary_prop IN prop,
                                              cl_dev_program* OUT prog)
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("CreateProgram enter"));

    // Input parameters validation
    if (0 == binSize || nullptr == bin)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Invalid binSize or bin parameters"));
        return CL_DEV_INVALID_VALUE;
    }

    if (nullptr == prog)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Invalid prog parameter"));
        return CL_DEV_INVALID_VALUE;
    }

    cl_dev_err_code ret = CheckProgramBinary(binSize, bin);
    if (CL_DEV_FAILED(ret))
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Invalid binary"));
        return ret;
    }

    ISPProgram* pIspProgram = new ISPProgram(bin, binSize);
    if (nullptr == pIspProgram)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Can't allocate program entry"));
        return CL_DEV_OUT_OF_MEMORY;
    }

    ret = pIspProgram->AllocateOnISP(m_pCameraShim);
    if (CL_DEV_FAILED(ret))
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Can't copy program to ISP address space"));
        delete pIspProgram;
        return ret;
    }

    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Program was successfully created"));
    *prog = (cl_dev_program*) pIspProgram;

    return CL_DEV_SUCCESS;
}

/*******************************************************************************************************************
CreateBuiltInKernelProgram
    Description
        Create a program from build-in kernels
    Input
        szBuiltInNames                  Built-in kernels names that will be used to create the program
    Output
        prog                            A handle to created program object.
    Returns
        CL_DEV_SUCCESS                  The function is executed successfully.
        CL_DEV_OUT_OF_MEMORY            If the device failed to allocate memory for the program.
        CL_DEV_INVALID_KERNEL_NAME      If one of the kernel names was not found in the device built-in kernels.
***********************************************************************************************************************/
cl_dev_err_code ISPProgramService::CreateBuiltInKernelProgram(const char* IN szBuiltInNames,
                                                           cl_dev_program* OUT prog)
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("CreateBuiltInKernelProgram enter"));

    ISPProgram* pIspProgram = new ISPProgram(m_pBuiltInKernelRegistry, szBuiltInNames);
    if(nullptr == pIspProgram)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Can't allocate program entry"));
        return CL_DEV_OUT_OF_MEMORY;
    }

    cl_dev_err_code ret = pIspProgram->Build();
    if (CL_DEV_INVALID_KERNEL_NAME == ret)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Invalid built-in name"));
        delete pIspProgram;
        return ret;
    }
    if (CL_DEV_FAILED(ret))
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Creating program with built-in kernels has failed"));
        delete pIspProgram;
        return ret;
    }

    // program successfully built
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Program with built-in kernels was successfully created and built"));
    pIspProgram->SetBuildStatus(CL_BUILD_SUCCESS);
    *prog = (cl_dev_program*) pIspProgram;

    return CL_DEV_SUCCESS;
}

/*******************************************************************************************************************
BuildProgram
    Description
        Builds (compiles & links) a program executable from the program intermediate or binary.
    Input
        options                         A pointer to a string that describes the build options to be used for building the program executable.
                                        The list of supported options is described in section 5.4.3 in OCL spec. document.
    Output
        prog                            A handle to created program object.
        buildStatus                     Build status of thr program.
    Returns
        CL_DEV_SUCCESS                  The function is executed successfully.
        CL_DEV_INVALID_BUILD_OPTIONS    If build options for back-end compiler specified by options are invalid.
        CL_DEV_INVALID_BINARY           If the back-end compiler failed to process binary.
        CL_DEV_OUT_OF_MEMORY            If the device failed to allocate memory for the program
***********************************************************************************************************************/
cl_dev_err_code ISPProgramService::BuildProgram(cl_dev_program OUT prog,
                                                const char* IN options,
                                                cl_build_status* OUT buildStatus)
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("BuildProgram enter"));

    // TODO: options is ignored

    if (nullptr == prog)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Invalid program parameter"));
        return CL_DEV_INVALID_PROGRAM;
    }

    cl_dev_err_code ret = CL_DEV_SUCCESS;

    ISPProgram* pIspProgram = (ISPProgram*)prog;

    if (CL_BUILD_SUCCESS == pIspProgram->GetBuildStatus())
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Program build was already complete"));
        return CL_DEV_BUILD_ALREADY_COMPLETE;
    }

    assert(CL_BUILD_IN_PROGRESS != pIspProgram->GetBuildStatus() &&
        "Program build is already in progress, framework should have cought this");

    if(CL_BUILD_NONE != pIspProgram->GetBuildStatus())
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("Invalid build status(%d), should be CL_BUILD_NONE(%d)"),
            pIspProgram->GetBuildStatus(), CL_BUILD_NONE);
        return CL_DEV_INVALID_OPERATION;
    }

    // one more binary check
    // TODO: is this needed?
    ret = CheckProgramBinary(pIspProgram->GetBinarySize(), pIspProgram->GetProgramBinary());
    if (CL_DEV_FAILED(ret))
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Invalid program binary"));
        return CL_DEV_INVALID_BINARY;
    }

    pIspProgram->SetBuildStatus(CL_BUILD_IN_PROGRESS);

    ret = pIspProgram->Build();
    if (CL_DEV_FAILED(ret))
    {
        IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Building ISP program failed"));
        pIspProgram->SetBuildStatus(CL_BUILD_ERROR);
    }
    else
    {
        // build succeed
        IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("ISP Program with %d kernels was successfully built"), pIspProgram->GetKernelsCount());
        pIspProgram->SetBuildStatus(CL_BUILD_SUCCESS);
    }

    if(nullptr != buildStatus)
    {
        *buildStatus = pIspProgram->GetBuildStatus();
    }

    return CL_DEV_SUCCESS;
}

/********************************************************************************************************************
ReleaseProgram
    Description
        Deletes previously created program object and releases all related resources.
    Input
        prog                            A handle to program object to be deleted
    Output
        NONE
    Returns
        CL_DEV_SUCCESS                  The function is executed successfully.
        CL_DEV_INVALID_PROGRAM          Invalid program object was specified.
********************************************************************************************************************/
cl_dev_err_code ISPProgramService::ReleaseProgram(cl_dev_program IN prog)
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("ReleaseProgram enter"));

    if (nullptr == prog)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Invalid program parameter"));
        return CL_DEV_INVALID_PROGRAM;
    }

    ISPProgram* pToDelete = (ISPProgram*) prog;
    // resources deallocation will be done by the program destructor
    delete pToDelete;

    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Program is released"));

    return CL_DEV_SUCCESS;
}

/********************************************************************************************************************
GetProgramBinary
    Description
        Returns the compiled program binary.
    Input
        prog                    A handle to created program object.
        size                    Size in bytes of the buffer passed to the function.
    Output
        binary                  A pointer to buffer wherein program binary will be stored.
        size_ret                The actual size in bytes of the returned buffer. When size is equal to 0 and binary is NULL,
                                 returns size in bytes of a program binary. If NULL the parameter is ignored.
    Returns
        CL_DEV_SUCCESS          The function is executed successfully.
        CL_DEV_INVALID_PROGRAM  If program is not valid program object.
        CL_DEV_INVALID_VALUE    If size is not enough to store the binary or binary is NULL and size is not 0.
********************************************************************************************************************/
cl_dev_err_code ISPProgramService::GetProgramBinary(cl_dev_program IN prog,
                                                 size_t IN size,
                                                 void* OUT binary,
                                                 size_t* OUT sizeRet)
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("GetProgramBinary enter"));

    if (nullptr == prog)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Invalid program parameter"));
        return CL_DEV_INVALID_PROGRAM;
    }

    ISPProgram* pIspProgram = (ISPProgram*) prog;
    fw_info blobInfo = pIspProgram->GetBlob();
    if (nullptr == blobInfo.data)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Program doesn't contain any binary"));
        return CL_DEV_INVALID_VALUE;
    }

    if (nullptr != sizeRet)
    {
        *sizeRet = blobInfo.size;
    }

    if ((0 == size) && (nullptr == binary))
    {
        IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Size of the actual binary is returned. No binary copy was required"));
        return CL_DEV_SUCCESS;
    }

    assert((size > 0 && nullptr != binary) && "Invalid size or binary pointer was specifed, framework should have cought this");

    if (size < blobInfo.size)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("The input size is not enough for copying the binary"));
        return CL_DEV_INVALID_VALUE;
    }

    MEMCPY_S((char*) binary, size, blobInfo.data, blobInfo.size);
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Program binary was copied successfully"));
    return CL_DEV_SUCCESS;
}

/********************************************************************************************************************
GetProgramBinary
    Description
        Returns the build log of the program.
    Input
        prog                    A handle to created program object.
        size                    Size in bytes of the buffer passed to the function.
    Output
        log                     A pointer to buffer wherein program build log will be copied to.
        size_ret                The actual size in bytes of the returned buffer. When size is equal to 0 and binary is NULL,
                                 returns size in bytes of a program binary. If NULL the parameter is ignored.
    Returns
        CL_DEV_SUCCESS          The function is executed successfully.
        CL_DEV_INVALID_PROGRAM  If program is not valid program object.
        CL_DEV_INVALID_VALUE    If size is not enough to store the binary or binary is NULL and size is not 0.
********************************************************************************************************************/
cl_dev_err_code ISPProgramService::GetBuildLog(cl_dev_program IN prog, size_t IN size, char* OUT log, size_t* OUT sizeRet)
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("GetBuildLog enter"));

    if (nullptr == prog)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Invalid program parameter"));
        return CL_DEV_INVALID_PROGRAM;
    }

    ISPProgram* pIspProgram = (ISPProgram*) prog;
    if (CL_BUILD_NONE == pIspProgram->GetBuildStatus())
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Program has not been built yet"));
        return CL_DEV_INVALID_OPERATION;
    }

    assert(CL_BUILD_IN_PROGRESS != pIspProgram->GetBuildStatus() &&
        "Program build is already in progress, framework should have cought this");

    const char* pLog = pIspProgram->GetBuildLog();
    size_t stLogSize = pIspProgram->GetBuildLogSize();

    if (nullptr != sizeRet)
    {
        *sizeRet = stLogSize;
    }

    if ((nullptr == log) || (size < stLogSize))
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Invalid input buffer or buffer size is not enough"));
        return CL_DEV_INVALID_VALUE;
    }

    MEMCPY_S(log, size, pLog, stLogSize);
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Program build log was copied successfully"));

    return CL_DEV_SUCCESS;
}

/************************************************************************************************************
GetSupportedBinaries (optional)
    Description
        Returns the list of supported binaries.
    Input
        count                   Size of the buffer passed to the function in terms of cl_prog_binary_desc.
    Output
        types                   A pointer to buffer wherein binary types will be stored.
        count_ret               The actual size ofthe buffer returned by the function in terms of cl_prog_binary_desc.
                                When count is equal to 0 and types is NULL, function returns a size of the list.
                                If NULL the parameter is ignored.
    Returns
        CL_DEV_SUCCESS          The function is executed successfully.
        CL_DEV_INVALID_PROGRAM  If program is not valid program object.
        CL_DEV_INVALID_VALUE    If count is not enough to store the binary or types is NULL and count is not 0.
***************************************************************************************************************/
cl_dev_err_code ISPProgramService::GetSupportedBinaries(size_t IN count, cl_prog_binary_desc* OUT types, size_t* OUT sizeRet)
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("GetSupportedBinaries enter"));

    // only custom binary is supported now
    cl_prog_binary_desc supportedBinType;
    supportedBinType.bin_type = CL_PROG_BIN_CUSTOM;
    supportedBinType.bin_ver_major = 0;
    supportedBinType.bin_ver_minor = 0;

    cl_prog_binary_desc* pSupportedBinaries = &supportedBinType;
    size_t stSupportedBinariesSize = sizeof(supportedBinType);

    if (nullptr != sizeRet)
    {
        *sizeRet = stSupportedBinariesSize;
    }

    if ((nullptr == types) || (count < stSupportedBinariesSize))
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Invalid input buffer or buffer size is not enough"));
        return CL_DEV_INVALID_VALUE;
    }

    MEMCPY_S(types, count, pSupportedBinaries, stSupportedBinariesSize);
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Supported binaries descriptor was copied successfully"));

    return CL_DEV_SUCCESS;
}

/************************************************************************************************************
GetKernelId
    Description
        Returns kernel id from its name.
    Input
        prog                        A handle to created program object.
        name                        Name of the kernel.
    Output
        kernelId                    The returned kernel id.
    Returns
        CL_DEV_SUCCESS              The function is executed successfully.
        CL_DEV_INVALID_PROGRAM      If program is not valid program object, or if it wasn't built yet.
        CL_DEV_INVALID_VALUE        If name or kernelId is NULL.
        CL_DEV_INVALID_KERNEL_NAME  If no kernel belongs to prog has the name found.
***************************************************************************************************************/
cl_dev_err_code ISPProgramService::GetKernelId(cl_dev_program IN prog, const char* IN name, cl_dev_kernel* OUT kernelId)
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("GetKernelId enter"));

    if (nullptr == prog)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Invalid program parameter"));
        return CL_DEV_INVALID_PROGRAM;
    }

    if((nullptr == name) || (nullptr == kernelId))
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Invalid kernel name or output parameter"));
        return CL_DEV_INVALID_VALUE;
    }

    ISPProgram* pIspProgram = (ISPProgram*) prog;

    if(CL_BUILD_SUCCESS != pIspProgram->GetBuildStatus())
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Program was not built"));
        return CL_DEV_INVALID_PROGRAM;
    }

    ISPKernel* pKernel = pIspProgram->GetKernel(name);
    if(nullptr == pKernel)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s%s"), TEXT("Invalid kernel name "), name);
        return CL_DEV_INVALID_KERNEL_NAME;
    }

    *kernelId = (cl_dev_kernel) pKernel;

    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("Kernel id for kernel %s returned successfully"), name);

    return CL_DEV_SUCCESS;
}

/************************************************************************************************************
GetProgramKernels
    Description
        Returns all kernels of a program.
    Input
        prog                        A handle to created program object.
        num_kernels                 Number of kernels that kernels buffer can contain.
    Output
        kernels                     A buffer were the output kernels will be stored.
        num_kernels_ret             Number of kernels that the program contain.
    Returns
        CL_DEV_SUCCESS              The function is executed successfully.
        CL_DEV_INVALID_PROGRAM      If program is not valid program object, or if it wasn't built yet.
        CL_DEV_INVALID_VALUE        If kernels size is not enough to store the output kernels or kernels is NULL.
***************************************************************************************************************/
cl_dev_err_code ISPProgramService::GetProgramKernels(cl_dev_program IN prog, cl_uint IN num_kernels, cl_dev_kernel* OUT kernels, cl_uint* OUT num_kernels_ret)
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("GetProgramKernels enter"));

    if (nullptr == prog)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Invalid program parameter"));
        return CL_DEV_INVALID_PROGRAM;
    }

    ISPProgram* pIspProgram = (ISPProgram*) prog;

    if(CL_BUILD_SUCCESS != pIspProgram->GetBuildStatus())
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Program was not built"));
        return CL_DEV_INVALID_PROGRAM;
    }

    cl_uint numAvailableKernels = pIspProgram->GetKernelsCount();

    if (nullptr != num_kernels_ret)
    {
        *num_kernels_ret = numAvailableKernels;
    }

    if (nullptr != kernels)
    {
        if (num_kernels < numAvailableKernels)
        {
            IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Input buffer size is not enough"));
            return CL_DEV_INVALID_VALUE;
        }

        // Retrieve kernels from program and store internally
        for(unsigned int i = 0; i < numAvailableKernels; ++i)
        {
            kernels[i] = (cl_dev_kernel) pIspProgram->GetKernel(i);
        }

        IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Kernels were copied successfully"));
    }

    return CL_DEV_SUCCESS;
}

/************************************************************************************************************
GetKernelInfo
    Description
        Returns specific kernel info per param.
    Input
        kernel                      A handle to created kernel object.
        param                       The kernel info requested.
        valueSize                   The size of the input buffer which will be used to store the output kernel info.
    Output
        value                       A buffer which will be used to store the output kernel info.
        valueSizeRet                The actual size of the buffer needed to store the kernel info.
    Returns
        CL_DEV_SUCCESS              The function is executed successfully.
        CL_DEV_INVALID_KERNEL       If kernel is not valid kernel object.
        CL_DEV_INVALID_VALUE        If kernels size is not enough to store the output kernels or kernels is NULL.
***************************************************************************************************************/
cl_dev_err_code ISPProgramService::GetKernelInfo(cl_dev_kernel IN kernel, cl_dev_kernel_info IN param, size_t IN valueSize, void* OUT value, size_t* OUT valueSizeRet)
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("GetKernelInfo enter"));

    if (nullptr == kernel)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Invalid kernel parameter"));
        return CL_DEV_INVALID_KERNEL;
    }

    ISPKernel* pIspKernel = (ISPKernel*) kernel;

    cl_bool boolValue;
    size_t stValue;
    cl_ulong ulValue;
    cl_dev_dispatch_buffer_prop dispatchProperties;

    const void* pValue = nullptr;
    size_t stValueSize = 0;

    // TODO: verify these values
    switch (param)
    {
        case CL_DEV_KERNEL_NAME:
            pValue = pIspKernel->GetKernelName().c_str();
            stValueSize = pIspKernel->GetKernelName().size() + 1;
            break;

        case CL_DEV_KERNEL_PROTOTYPE:
            pValue = pIspKernel->GetKernelArgsPrototype();
            stValueSize = pIspKernel->GetKernelArgsCount() * sizeof(cl_kernel_argument);
            break;

        case CL_DEV_KERNEL_WG_SIZE_REQUIRED:
            pValue = pIspKernel->GetRequiredWGSize();
            stValueSize = sizeof(size_t) * MAX_WORK_DIM;
            break;

        case CL_DEV_KERNEL_MAX_WG_SIZE:
        case CL_DEV_KERNEL_WG_SIZE:
            stValue = pIspKernel->GetOptimalWGSize();
            pValue = &stValue;
            stValueSize = sizeof(size_t);
            break;

        case CL_DEV_KERNEL_ATTRIBUTES:
            pValue = pIspKernel->GetKernelAttributes().c_str();
            stValueSize = pIspKernel->GetKernelAttributes().size() + 1;
            break;

        case CL_DEV_KERNEL_DISPATCH_BUFFER_PROPERTIES:
            dispatchProperties.size = pIspKernel->GetKernelArgsBufferSize();
            dispatchProperties.argumentOffset = 0;
            dispatchProperties.alignment = 1;
            pValue = &dispatchProperties;
            stValueSize = sizeof(cl_dev_dispatch_buffer_prop);
            break;

        case CL_DEV_KERNEL_MEMORY_OBJECT_INDEXES:
            pValue = pIspKernel->GetKernelMemoryObjArgsIndices();
            stValueSize = pIspKernel->GetKernelMemoryObjArgsCount() * sizeof(cl_uint);
            break;

        case CL_DEV_KERNEL_IMPLICIT_LOCAL_SIZE:
            ulValue = pIspKernel->GetKernelImplicitLocalSize();
            pValue = &ulValue;
            stValueSize = sizeof(cl_ulong);
            break;

        case CL_DEV_KERNEL_NON_UNIFORM_WG_SIZE_SUPPORT:
            boolValue = pIspKernel->IsNonUniformWGSizeSupported();
            pValue = &boolValue;
            stValueSize = sizeof(cl_bool);
            break;

        case CL_DEV_KERNEL_PRIVATE_SIZE:
        //    ulValue = 0;
        //    pValue = &ulValue;
        //    stValueSize = sizeof(cl_ulong);
        //    break;

        case CL_DEV_KERNEL_ARG_INFO:

            return CL_DEV_NOT_SUPPORTED;

        default:
            IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Invalid kernel info parameter"));
            return CL_DEV_INVALID_VALUE;
    }

    if (nullptr != valueSizeRet)
    {
        *valueSizeRet = stValueSize;
    }

    if (nullptr != value && valueSize < stValueSize)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Invalid input buffer or buffer size is not enough"));
        return CL_DEV_INVALID_VALUE;
    }

    if (nullptr != value && 0 != stValueSize)
    {
        MEMCPY_S(value, valueSize, pValue, stValueSize);
    }

    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Kernel info copied successfully"));

    return CL_DEV_SUCCESS;
}

/************************************************************************************************************
GetSupportedImageFormats
    Description
        Returns image formats that are suported by the device.
    Input
        flags                       Memory flags of the requsted image formats.
        imageType                   Image type (buffer/1D/2D etc.)
        numEntries                  Number of entries that the output buffer can hold.
    Output
        formats                     Output buffer containing the supported image formats.
        numEntriesRet               The actual number of the entries needed to store the output buffer.
    Returns
        CL_DEV_SUCCESS              The function is executed successfully.
***************************************************************************************************************/
cl_dev_err_code ISPProgramService::GetSupportedImageFormats(cl_mem_flags IN flags, cl_mem_object_type IN imageType,
                cl_uint IN numEntries, cl_image_format* OUT formats, cl_uint* OUT numEntriesRet)
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("GetSupportedImageFormats enter"));
    if (nullptr != numEntriesRet)
    {
        numEntriesRet = 0; // Currently, no images are supported
    }

    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("No image formats were copied. Currently, images are not supported."));

    return CL_DEV_SUCCESS;
}


// Construct kernel from binary
ISPKernel::ISPKernel(const char* kernelName, fw_info blob) :
                m_kernelName(kernelName), m_blob(blob),
                m_command(CAMERA_NO_COMMAND), m_argsBufferSize(0)
{
}

// Construct a camera command built-in kernel
ISPKernel::ISPKernel(enum cameraCommand cmd) :
    m_command(cmd), m_argsBufferSize(0)
{
    m_blob.data = nullptr;
    m_blob.size = 0;
}

cl_dev_err_code ISPKernel::Build(std::string& log)
{
    if (((nullptr == m_blob.data || 0 == m_blob.size) && CAMERA_NO_COMMAND == m_command) ||
        ((nullptr != m_blob.data || 0 != m_blob.size) && CAMERA_NO_COMMAND != m_command))
    {
        return CL_DEV_INVALID_VALUE;
    }

    // Currently ISP kernels run as a single work-item
    // TODO: Set correct values
    m_optWGSize = 1;
    for(int i = 0; i < MAX_WORK_DIM; i++)
    {
        m_reqdWGSize[i] = 0;
        m_hintWGSize[i] = 1;
    }

    if (CAMERA_NO_COMMAND == m_command)
    {
        return BuildFromBinary(log);
    }
    // else: is a command
    return BuildFromCommand(log);
}

cl_dev_err_code ISPKernel::BuildFromBinary(std::string& log)
{
    if (nullptr == m_blob.data || 0 == m_blob.size)
    {
        return CL_DEV_INVALID_VALUE;
    }

    m_requiredState = CAMERA_STATE_PREVIEW_STOPPED;
    m_stateAfterExecution = CAMERA_STATE_INDIFFERENT;

    log.append("Building kernel " + m_kernelName + "\n");

    unsigned int currentOffset = 0; //no alignment required

    // TODO: hard-coded arg types for now...
    if(m_kernelName.compare("negative") == 0 ||
       m_kernelName.compare("nv12convert") == 0 ||
       m_kernelName.compare("HDRish") == 0)
    {
        AddKernelArgPrototype(CL_KRNL_ARG_PTR_GLOBAL, sizeof(cl_mem), 0, currentOffset);
        currentOffset += sizeof(cl_mem);
        AddKernelArgPrototype(CL_KRNL_ARG_PTR_GLOBAL, sizeof(cl_mem), 0, currentOffset);
        currentOffset += sizeof(cl_mem);
        AddKernelArgPrototype(CL_KRNL_ARG_UINT, sizeof(cl_uint), 0, currentOffset);
        currentOffset += sizeof(cl_uint);
        AddKernelArgPrototype(CL_KRNL_ARG_UINT, sizeof(cl_uint), 0, currentOffset);

        LogArgumentsPrototypes(log);

        return CL_DEV_SUCCESS;
    }

    assert(false && "TODO: currently ISP device agent support hard-coded arg types only");

    // TODO: CSS doesn't store number of argument or their types
    // so for now we assume kernel arguments are global pointers
    // TODO: remove using sizeof ! this will be problem if host is 64bit and ISP is 32bit
    m_cssHeader = (struct sh_css_fw_info*) m_blob.data;

    int dmemParams = m_cssHeader->info.isp.memory_interface[SH_CSS_ISP_DMEM0].parameters.size / sizeof(void*);

    for (int i = 0; i < dmemParams; i++)
    {
        // TODO: We only support memory objects in global memory right now
        // We cannot know the type of arguments,
        // because the CSS header does not store that information
        // We will assume memory objects in global memory
        AddKernelArgPrototype(CL_KRNL_ARG_PTR_GLOBAL, sizeof(cl_mem), 0, currentOffset);
        currentOffset += sizeof(cl_mem);
    }

    m_requiredState = CAMERA_STATE_INDIFFERENT;
    m_stateAfterExecution = CAMERA_STATE_INDIFFERENT;

    LogArgumentsPrototypes(log);

    return CL_DEV_SUCCESS;
}

cl_dev_err_code ISPKernel::BuildFromCommand(std::string& log)
{
    const char * name = CommandToString(m_command);
    if (nullptr == name)
    {
        return CL_DEV_INVALID_VALUE;
    }
    m_kernelName = name;

    log.append("Building kernel " + m_kernelName + "\n");

    m_requiredState = CAMERA_STATE_INDIFFERENT;
    m_stateAfterExecution = CAMERA_STATE_INDIFFERENT;

    switch (m_command)
    {
        case (CAMERA_TAKE_PICTURE):
            // Intentional fall through
        case (CAMERA_TAKE_PICTURE_WITH_AF):
            m_stateAfterExecution = CAMERA_STATE_PREVIEW_STOPPED;       // Take picture command stops preview
            // Intentional fall through
        case (CAMERA_COPY_PREVIEW_BUFFER):
            AddKernelArgPrototype(CL_KRNL_ARG_PTR_GLOBAL, sizeof(cl_mem), 0, 0); // Image buffer
            // Intentional fall through
        case (CAMERA_AUTO_FOCUS):
            m_requiredState = CAMERA_STATE_PREVIEW_RUNNING;             // Take picture needs preview
            break;

        case (CAMERA_SET_PREVIEW_SIZE):
            // Intentional fall through
        case (CAMERA_SET_PICTURE_SIZE):
            AddKernelArgPrototype(CL_KRNL_ARG_UINT, sizeof(cl_uint), 0, 0);                  // width
            AddKernelArgPrototype(CL_KRNL_ARG_UINT, sizeof(cl_uint), 0, sizeof(cl_uint));    // height
            m_requiredState = CAMERA_STATE_PREVIEW_STOPPED;             // preview needs to be stopped
            break;

        case (CAMERA_START_PREVIEW):
            m_stateAfterExecution = CAMERA_STATE_PREVIEW_RUNNING;
            break;

        case (CAMERA_STOP_PREVIEW):
            m_stateAfterExecution = CAMERA_STATE_PREVIEW_STOPPED;
            break;

        case (CAMERA_BEGIN_PIPELINE):
            AddKernelArgPrototype(CL_KRNL_ARG_PTR_GLOBAL, sizeof(cl_mem), 0, 0); // Image buffer
            break;

        case (CAMERA_END_PIPELINE):
            break;

        default:
            log.append("Invalid command: " + m_kernelName + "\n");
            log.append("Build failed\n");
            return CL_DEV_INVALID_VALUE;
    }

    LogArgumentsPrototypes(log);

    return CL_DEV_SUCCESS;
}

void ISPKernel::AddKernelArgPrototype(cl_kernel_arg_type type, unsigned int size, unsigned int access, unsigned int offset_in_bytes)
{
    assert(((type <= CL_KRNL_ARG_VECTOR) || (CL_KRNL_ARG_PTR_GLOBAL == type)) && "TODO: unsupported arg type");

    cl_kernel_argument arg;
    arg.type = type;
    arg.size_in_bytes = size;
    arg.access = access;
    arg.offset_in_bytes = offset_in_bytes;

    m_argsPrototype.push_back(arg);

    // the last call to AddKernelArgPrototype always decide the args buffer size
    m_argsBufferSize = offset_in_bytes + size;

    // if memory object - store it's index
    if (CL_KRNL_ARG_PTR_GLOBAL == type)
    {
        m_memObjArgsIndices.push_back(m_argsPrototype.size() - 1);
    }
}

const cl_kernel_argument* ISPKernel::GetKernelArgsPrototype() const
{
    if (m_argsPrototype.empty())
    {
        return nullptr;
    }

    return &(m_argsPrototype[0]);
}

const cl_uint* ISPKernel::GetKernelMemoryObjArgsIndices() const
{
    if (m_memObjArgsIndices.empty())
    {
        return nullptr;
    }

    return &(m_memObjArgsIndices[0]);
}

const char* ISPKernel::CommandToString(enum cameraCommand cmd)
{
    assert(CAMERA_NO_COMMAND != cmd && "CommandToString should not be called on non-commands!");

    switch(cmd)
    {
        case (CAMERA_AUTO_FOCUS): return "CAMERA_AUTO_FOCUS";
        case (CAMERA_TAKE_PICTURE): return "CAMERA_TAKE_PICTURE";
        case (CAMERA_TAKE_PICTURE_WITH_AF): return "CAMERA_TAKE_PICTURE_WITH_AF";
        case (CAMERA_SET_PICTURE_SIZE): return "CAMERA_SET_PICTURE_SIZE";
        case (CAMERA_SET_PREVIEW_SIZE): return "CAMERA_SET_PREVIEW_SIZE";
        case (CAMERA_START_PREVIEW): return "CAMERA_START_PREVIEW";
        case (CAMERA_STOP_PREVIEW): return "CAMERA_STOP_PREVIEW";
        case (CAMERA_COPY_PREVIEW_BUFFER): return "CAMERA_COPY_PREVIEW_BUFFER";
        case (CAMERA_BEGIN_PIPELINE): return "CAMERA_BEGIN_PIPELINE";
        case (CAMERA_END_PIPELINE): return "CAMERA_END_PIPELINE";

        default:
            // Unknown command
            return nullptr;
    }
}

const char* ISPKernel::KernelTypeToString(cl_kernel_arg_type type)
{
    switch(type)
    {
        case (CL_KRNL_ARG_INT): return "signed integer";
        case (CL_KRNL_ARG_UINT): return "unsigned integer";
        case (CL_KRNL_ARG_FLOAT): return "float";
        case (CL_KRNL_ARG_DOUBLE): return "double";
        case (CL_KRNL_ARG_VECTOR): return "vector";
        case (CL_KRNL_ARG_VECTOR_BY_REF): return "byval pointer to a vector";
        case (CL_KRNL_ARG_SAMPLER): return "sampler";
        case (CL_KRNL_ARG_COMPOSITE): return "user defined struct";
        case (CL_KRNL_ARG_PTR_LOCAL): return "local pointer";
        case (CL_KRNL_ARG_PTR_GLOBAL): return "global pointer";
        case (CL_KRNL_ARG_PTR_CONST): return "pointer to constant";
        case (CL_KRNL_ARG_PTR_IMG_2D): return "pointer to 2D image";
        case (CL_KRNL_ARG_PTR_IMG_2D_DEPTH): return "pointer to 2D image depth";
        case (CL_KRNL_ARG_PTR_IMG_3D): return "pointer to 3D image";
        case (CL_KRNL_ARG_PTR_IMG_2D_ARR): return "pointer to a 2D image array";
        case (CL_KRNL_ARG_PTR_IMG_2D_ARR_DEPTH): return "pointer to a 2D image depth array";
        case (CL_KRNL_ARG_PTR_IMG_1D): return "pointer to 1D image";
        case (CL_KRNL_ARG_PTR_IMG_1D_ARR): return "pointer to 1D image array";
        case (CL_KRNL_ARG_PTR_IMG_1D_BUF): return "pointer to 1D image buffer";
        case (CL_KRNL_ARG_PTR_BLOCK_LITERAL): return "pointer to Block Literal structure";

        default:
            return "unknown type";
    }
}

void ISPKernel::LogArgumentsPrototypes(std::string& log)
{
    for (int i = 0; i < m_argsPrototype.size(); ++i)
    {
        std::stringstream ss;
        ss << "Kernel arguemnt " << i << ":";
        ss << " type = " << KernelTypeToString(m_argsPrototype[i].type);
        ss << " size = " << m_argsPrototype[i].size_in_bytes;
        ss << "\n";
        log.append(ss.str());
    }
}


BuiltInKernelRegistry::BuiltInKernelRegistry(CameraShim* shim) :
    m_pCameraShim(shim)
{
}

BuiltInKernelRegistry::~BuiltInKernelRegistry()
{
    // release the built-in kernels
    KernelMap_t::iterator it = m_builtInKernels.begin();
    for( ; it != m_builtInKernels.end(); ++it)
    {
        delete it->second;
    }

    // release the fw info that was allocated on ISP
    std::vector<fw_info>::iterator blob = m_blobs.begin();
    for ( ; blob != m_blobs.end(); ++blob)
    {
        m_pCameraShim->host_free(blob->data);
    }
}

cl_dev_err_code BuiltInKernelRegistry::AddCameraCommands()
{
    cl_dev_err_code ret = CL_DEV_SUCCESS;

    // TODO: move this to class as member
    std::string buildLog;

    for (int i = CAMERA_NO_COMMAND+1; i < LAST_CAMERA_COMMAND; i++)
    {
        ISPKernel* pKernel = new ISPKernel(static_cast<enum cameraCommand>(i));
        if(nullptr == pKernel)
        {
            return CL_DEV_OUT_OF_MEMORY;
        }

        ret = pKernel->Build(buildLog);
        if (CL_DEV_FAILED(ret))
        {
            delete pKernel;
            return CL_DEV_ERROR_FAIL;
        }

        m_builtInKernels[pKernel->GetKernelName()] = pKernel;
    }

    return ret;
}

cl_dev_err_code BuiltInKernelRegistry::AddFirmware(const void* pBinary, size_t size)
{
    cl_dev_err_code ret = CL_DEV_SUCCESS;

    // To get the kernels in the binary we will create temporary ISPProgram
    // ISPProgram will allocate itself on ISP and initialize the kernels
    // ISPProgram will not take ownership on the input binary
    // ISPProgram is owner of the FW blob and the kernels
    // but we will take ownership on them using ISPProgram::TakeBlobAndKernels()

    ISPProgram program(pBinary, size);
    if ( CL_DEV_FAILED(program.AllocateOnISP(m_pCameraShim)) ||
         CL_DEV_FAILED(program.Build()) )
    {
        return CL_DEV_ERROR_FAIL;
    }

    fw_info blob;
    blob.data = nullptr;
    blob.size = 0;

    cl_uint numKernels = program.GetKernelsCount();
    if (0 == numKernels)
    {
        return CL_DEV_ERROR_FAIL;
    }

    ISPKernel** ppKernels = new ISPKernel*[numKernels];
    if (nullptr == ppKernels)
    {
        return CL_DEV_ERROR_FAIL;
    }

    if (CL_DEV_FAILED(program.TakeBlobAndKernels(&blob, numKernels, ppKernels)))
    {
        delete[] ppKernels;
        return CL_DEV_ERROR_FAIL;
    }

    m_blobs.push_back(blob);
    for (cl_uint i = 0; i < numKernels; i++)
    {
        m_builtInKernels[ ppKernels[i]->GetKernelName() ] = ppKernels[i];
    }

    delete[] ppKernels;

    return CL_DEV_SUCCESS;
}

cl_dev_err_code BuiltInKernelRegistry::GetBuiltInKernel(std::string& builtinName, ISPKernel** pKernel_ret)
{
    if (builtinName.empty() || nullptr == pKernel_ret)
    {
        return CL_DEV_INVALID_VALUE;
    }

    KernelMap_t::const_iterator it;
    it = m_builtInKernels.find(builtinName);
    if (m_builtInKernels.end() == it)
    {
        return CL_DEV_INVALID_KERNEL_NAME;
    }

    ISPKernel* pFoundKernel = it->second;
    *pKernel_ret = pFoundKernel;

    return CL_DEV_SUCCESS;
}


// Construct ISP program from binary
ISPProgram::ISPProgram(const void* pBinary, size_t size) :
    m_binary(pBinary), m_binarySize(size),
    m_isBuiltIn(false), m_pBuiltInKernelRegistry(nullptr),
    m_buildStatus(CL_BUILD_NONE)
{
    m_blob.size = 0;
    m_blob.data = nullptr;
}

// Construct ISP program from built-in kernels
ISPProgram::ISPProgram(BuiltInKernelRegistry* registry, const char* szBuiltInKernelList) :
    m_binary(nullptr), m_binarySize(0),
    m_isBuiltIn(true), m_pBuiltInKernelRegistry(registry),
    m_buildStatus(CL_BUILD_NONE)
{
    m_blob.size = 0;
    m_blob.data = nullptr;

    if (nullptr != szBuiltInKernelList)
    {
        m_strBuiltInsList = szBuiltInKernelList;
    }
}

ISPProgram::~ISPProgram()
{
    // release the kernels
    KernelVector_t::iterator it = m_vecKernels.begin();
    for ( ; it != m_vecKernels.end(); ++it)
    {
        delete *it;
    }

    // release the fw info from ISP
    if (nullptr != m_blob.data)
    {
        assert(0 == m_blob.size && "Binary size should be larger than zero when we have a valid binary pointer!");
        m_pCameraShim->host_free(m_blob.data);
    }
}

// Allocate and copy the program binary to ISP address space
cl_dev_err_code ISPProgram::AllocateOnISP(CameraShim* shim)
{
    if (0 == m_binary || 0 == m_binarySize || m_isBuiltIn || nullptr == shim)
    {
        return CL_DEV_INVALID_VALUE;
    }

    m_pCameraShim = shim;

    fw_info blob;
    blob.size = m_binarySize;
    blob.data = m_pCameraShim->host_alloc(blob.size);
    if (nullptr == blob.data ||
        0 != MEMCPY_S(blob.data, blob.size, m_binary, m_binarySize))
    {
        return CL_DEV_ERROR_FAIL;
    }

    m_blob = blob;

    return CL_DEV_SUCCESS;
}

// Initialize kernels in program
cl_dev_err_code ISPProgram::Build()
{
    if (m_isBuiltIn)
    {
        return BuildFromBuiltIns();
    }
    // else program was built from binary
    return BuildFromBinary();
}

cl_dev_err_code ISPProgram::BuildFromBinary()
{
    //--------------------------------
    // TODO: Currently only one kernel per program
    if (nullptr == m_binary || m_isBuiltIn)
    {
        return CL_DEV_INVALID_VALUE;
    }

    m_buildLog.append("Build started\n");

    //char * kernelName = "nv12convert";
    //char kernelName[KERNEL_NAME_MAX_LENGTH + 1];
    // TODO: need to learn about CSS for this
    //SPRINTF_S(kernelName, sizeof(kernelName), "%s", (char*)m_binary + sizeof(struct sh_css_fw_info));

    char kernelName[KERNEL_NAME_MAX_LENGTH + 1];
    // TODO: nasty hack
    SPRINTF_S(kernelName, sizeof(kernelName), "%s", (char*)m_binary + 0x3d8);

    m_buildLog.append("Found kernel " + std::string(kernelName) + "\n");

    ISPKernel* pKernel = new ISPKernel(kernelName, m_blob);
    if (nullptr == pKernel)
    {
        return CL_DEV_OUT_OF_MEMORY;
    }
    m_vecKernels.push_back(pKernel);
    m_mapKernels[kernelName] = pKernel;
    //--------------------------------

    KernelVector_t::const_iterator it = m_vecKernels.begin();
    for ( ; it != m_vecKernels.end(); ++it)
    {
        cl_dev_err_code ret = (*it)->Build(m_buildLog);
        if (CL_DEV_FAILED(ret))
        {
            return ret;
        }
    }

    m_buildLog.append("Build finished successfully\n");

    return CL_DEV_SUCCESS;
}

cl_dev_err_code ISPProgram::BuildFromBuiltIns()
{
    if (m_strBuiltInsList.empty() || !m_isBuiltIn || nullptr == m_pBuiltInKernelRegistry)
    {
        return CL_DEV_INVALID_OPERATION;
    }

    cl_dev_err_code ret = CL_DEV_SUCCESS;

    char delim = ';';
    std::string builtInName;

    m_buildLog.append("Build started\n");

    std::stringstream stream(m_strBuiltInsList);
    // TODO: this code can build program combined of camera commands and binary kernels
    //       check if this cause issues elsewhere
    while (std::getline(stream, builtInName, delim))
    {
        ISPKernel* pBuiltinKernel = nullptr;
        ret = m_pBuiltInKernelRegistry->GetBuiltInKernel(builtInName, &pBuiltinKernel);
        if (CL_DEV_FAILED(ret))
        {
            m_buildLog.append("Cannot add built-in kernel " + builtInName + "\n");
            m_buildLog.append("Build failed\n");
            return ret;
        }
        assert(nullptr != pBuiltinKernel && "GetBuiltInKernel returned NULL kernel even in success!");

        // ISPKernel is not the owner of any internal pointer so copy c'tor is enough
        ISPKernel* pKernel = new ISPKernel(*pBuiltinKernel);
        if (nullptr == pKernel)
        {
            return CL_DEV_OUT_OF_MEMORY;
        }

        m_vecKernels.push_back(pKernel);
        m_mapKernels[builtInName] = pKernel;

        m_buildLog.append("Added built-in kernel " + builtInName + "\n");
    }

    m_buildLog.append("Build finished successfully\n");

    return ret;
}

cl_dev_err_code ISPProgram::TakeBlobAndKernels(fw_info* blob_ret, cl_uint numKernels, ISPKernel** kernels_ret)
{
    if (nullptr == blob_ret || nullptr == m_blob.data || 0 == m_blob.size)
    {
        return CL_DEV_INVALID_VALUE;
    }
    if (nullptr == kernels_ret || numKernels != GetKernelsCount())
    {
        return CL_DEV_INVALID_VALUE;
    }

    // take the blob
    *blob_ret = m_blob;
    m_blob.data = nullptr;
    m_blob.size = 0;

    // take the kernels
    KernelVector_t::iterator it = m_vecKernels.begin();
    for ( int i = 0; it != m_vecKernels.end(); ++it, ++i)
    {
        kernels_ret[i] = *it;
    }

    // this will not deallocate the kernels
    m_vecKernels.clear();
    m_mapKernels.clear();

    return CL_DEV_SUCCESS;
}

ISPKernel* ISPProgram::GetKernel(const char * name) const
{
    if(m_mapKernels.size() == 0)
    {
        return nullptr;
    }

    KernelMap_t::const_iterator iter;
    iter = m_mapKernels.find(name);
    if(m_mapKernels.end() == iter)
    {
        return nullptr;
    }

    return iter->second;
}

ISPKernel* ISPProgram::GetKernel(int id) const
{
    if(m_vecKernels.size() <= id)
    {
        return nullptr;
    }

    return m_vecKernels[id];
}
