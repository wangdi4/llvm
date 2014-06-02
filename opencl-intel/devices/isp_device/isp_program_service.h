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
//  isp_program_service.h
//  Implementation of the Class ISPProgramService
//  Class Object is responsible on programs and kernels binaries
///////////////////////////////////////////////////////////

#pragma once

#include <cl_device_api.h>

#include "res/Ilibshim.h"


#include <string>
#include <vector>
#include <map>

namespace Intel { namespace OpenCL { namespace ISPDevice {

enum cameraCommand {
    CAMERA_NO_COMMAND = 0,
    CAMERA_AUTO_FOCUS,
    CAMERA_TAKE_PICTURE,
    CAMERA_TAKE_PICTURE_WITH_AF,
    CAMERA_SET_PICTURE_SIZE,
    CAMERA_SET_PREVIEW_SIZE,
    CAMERA_START_PREVIEW,
    CAMERA_STOP_PREVIEW,
    CAMERA_COPY_PREVIEW_BUFFER,
    //CAMERA_BEGIN_PIPELINE, // TODO: check if this is relevant anymore
    //CAMERA_END_PIPELINE,

    LAST_CAMERA_COMMAND
};

enum cameraState {
    CAMERA_STATE_INDIFFERENT = 0,
    CAMERA_STATE_PREVIEW_RUNNING,
    CAMERA_STATE_PREVIEW_STOPPED,

    NUM_CAMERA_STATES
};

class ISPKernel
{
public:
    // Construct kernel from binary
    ISPKernel(const char* kernelName, fw_info blob);
    // Construct a camera command built-in kernel
    ISPKernel(enum cameraCommand cmd);

    // Initialize kernel & check arguments types
    cl_dev_err_code Build(std::string& log);

    // kernel info
    const cl_kernel_argument*   GetKernelArgsPrototype() const;
    const cl_uint*              GetKernelMemoryObjArgsIndices() const;
    cl_uint     GetKernelArgsCount() const { return m_argsPrototype.size(); }
    size_t      GetKernelArgsBufferSize() const { return m_argsBufferSize; }
    std::string GetKernelName() const { return m_kernelName; }
    std::string GetKernelAttributes() const { return ""; }
    cl_uint     GetKernelMemoryObjArgsCount() const { return m_memObjArgsIndices.size(); }
    cl_ulong    GetKernelImplicitLocalSize() const { return 0; }
    //const size_t* ISPKernel::GetRequiredWorkGroupSize() const { return m_reqdWGSize[0] ? m_reqdWGSize : NULL; }
    cl_bool IsNonUniformWGSizeSupported() const { return CL_FALSE; }
    size_t GetOptimalWGSize() const { return m_optWGSize; }

    fw_info GetBlob() const { return m_blob; }

    enum cameraCommand  GetCameraCommand() const { return m_command; }
    enum cameraState    GetRequiredState() const { return m_requiredState; }
    enum cameraState    GetStateAfterExecution() const { return m_stateAfterExecution; }

private:
    cl_dev_err_code BuildFromBinary(std::string& log);
    cl_dev_err_code BuildFromCommand(std::string& log);

    void AddKernelArgPrototype(cl_kernel_arg_type type, unsigned int size, unsigned int access, unsigned int offset_in_bytes);

    const char* CommandToString(enum cameraCommand cmd);
    const char* KernelTypeToString(cl_kernel_arg_type type);
    void LogArgumentsPrototypes(std::string& log);

    // Name of the kernel function
    std::string             m_kernelName;

    // TODO: change references from libshim to a more class-specific (i.e CameraShim)

    // NOT OWNED by the kernel, used by libshim, contains pointer to and size of the blob
    fw_info                 m_blob;                 // used by libshim, contains pointer to and size of the blob
    struct sh_css_fw_info*  m_cssHeader;            // header of the firmware, as defined in CSS

    enum cameraCommand      m_command;              // the camera command
    enum cameraState        m_requiredState;        // the required camera state for this command
    enum cameraState        m_stateAfterExecution;  // the state of the camera after command has finished

    // kernel args prototype
    std::vector<cl_kernel_argument> m_argsPrototype;
    std::vector<cl_uint>            m_memObjArgsIndices;
    size_t                          m_argsBufferSize;

    // TODO: do we need this
    size_t m_optWGSize;
    size_t m_reqdWGSize[MAX_WORK_DIM];
    size_t m_hintWGSize[MAX_WORK_DIM];
};

class BuiltInKernelRegistry
{
public:
    BuiltInKernelRegistry(CameraShim* shim);
    ~BuiltInKernelRegistry();

    // Add built-in commands
    cl_dev_err_code AddCameraCommands();
    // Add built-in firmwares
    cl_dev_err_code AddFirmware(const void* pBinary, size_t size);

    // get a pointer to a built-in kernel
    cl_dev_err_code GetBuiltInKernel(std::string& builtinName, ISPKernel** pKernel_ret);

    //cl_dev_err_code Init();
    //cl_dev_err_code AddFirmware(const char* name, const char* path, const char* kernelName);

    //cl_dev_err_code CreateBuiltInKernel(const char* name, ISPKernel** kernelRet);

private:
    CameraShim* m_pCameraShim;

    typedef std::map<std::string, ISPKernel*> KernelMap_t;
    KernelMap_t m_builtInKernels;

    // Owned by the builtin registry, used by libshim, contains pointer to and size of the blob
    std::vector<fw_info> m_blobs;
};

class ISPProgram
{
public:
    // Construct ISP program from binary
    ISPProgram(const void* pBinary, size_t size);
    // Construct ISP program from built-in kernels
    ISPProgram(BuiltInKernelRegistry* registry, const char* szBuiltInKernelList);

    ~ISPProgram();

    // Allocate and copy the program binary to ISP adress space
    cl_dev_err_code AllocateOnISP(CameraShim* shim);
    // Build kernels in program
    cl_dev_err_code Build();

    // take the blob and kernels ownership from the program
    cl_dev_err_code TakeBlobAndKernels(fw_info* blob_ret, cl_uint numKernels, ISPKernel** kernels_ret);

    // Get kernel by name
    ISPKernel* GetKernel(const char * name) const;
    // Get kernel by id
    ISPKernel* GetKernel(int id) const;
    // Get number of kernels
    cl_uint GetKernelsCount() const { return m_vecKernels.size(); }
    // Get build status
    cl_build_status GetBuildStatus() const { return m_buildStatus; }
    // Set build status
    void SetBuildStatus(cl_build_status status) { m_buildStatus = status; }
    // Get pointer to program binary
    const void* GetProgramBinary() const { return m_binary; }
    // Get binary size
    size_t GetBinarySize() const { return m_binarySize; }
    // Get pointer to program build log
    const char* GetBuildLog() const { return m_buildLog.c_str(); }
    // Get build log size
    size_t GetBuildLogSize() const { return m_buildLog.size() + 1; }
    // Get firmware info blob
    fw_info GetBlob() const { return m_blob; }

private:
    // Initialize kernels in program from binary
    cl_dev_err_code BuildFromBinary();
    // Initialize kernels in program from builtins
    cl_dev_err_code BuildFromBuiltIns();

    // Pointer to binary that is owned by the framework
    const void*     m_binary;
    // Size of the binary as given by the framework
    size_t          m_binarySize;

    // Owned by the program, used by libshim, contains pointer to and size of the blob
    fw_info         m_blob;
    CameraShim*     m_pCameraShim;

    // whether the program contains built-in kernels
    cl_bool         m_isBuiltIn;
    // list of the built-in kernels names as specified by user
    std::string     m_strBuiltInsList;
    BuiltInKernelRegistry* m_pBuiltInKernelRegistry;

    cl_build_status m_buildStatus;
    std::string     m_buildLog;

    // two data structures to store kernels by id and by name
    typedef std::vector<ISPKernel*> KernelVector_t;
    KernelVector_t  m_vecKernels;
    typedef std::map<std::string, ISPKernel*> KernelMap_t;
    KernelMap_t     m_mapKernels;

};

class ISPProgramService
{
public:
    ISPProgramService(cl_int devId, IOCLDevLogDescriptor* logDesc, CameraShim* pCameraShim);
    virtual ~ISPProgramService();

    cl_dev_err_code Init();

    cl_dev_err_code CheckProgramBinary(size_t IN bin_size, const void* IN bin);
    cl_dev_err_code CreateProgram(size_t IN binSize, const void* IN bin, cl_dev_binary_prop IN prop, cl_dev_program* OUT prog);
    cl_dev_err_code CreateBuiltInKernelProgram(const char* IN szBuiltInNames, cl_dev_program* OUT prog);
    cl_dev_err_code BuildProgram(cl_dev_program OUT prog, const char* IN options, cl_build_status* OUT buildStatus);
    cl_dev_err_code ReleaseProgram(cl_dev_program IN prog);
    cl_dev_err_code GetProgramBinary(cl_dev_program IN prog, size_t IN size, void* OUT binary, size_t* OUT sizeRet);
    cl_dev_err_code GetBuildLog(cl_dev_program IN prog, size_t IN size, char* OUT log, size_t* OUT sizeRet);
    cl_dev_err_code GetSupportedBinaries(size_t IN count, cl_prog_binary_desc* OUT types, size_t* OUT sizeRet);
    cl_dev_err_code GetKernelId(cl_dev_program IN prog, const char* IN name, cl_dev_kernel* OUT kernelId);
    cl_dev_err_code GetProgramKernels(cl_dev_program IN prog, cl_uint IN num_kernels, cl_dev_kernel* OUT kernels, cl_uint* OUT num_kernels_ret);
    cl_dev_err_code GetKernelInfo(cl_dev_kernel IN kernel, cl_dev_kernel_info IN param, size_t IN valueSize, void* OUT value, size_t* OUT valueSizeRet);
    cl_dev_err_code GetSupportedImageFormats(cl_mem_flags IN flags, cl_mem_object_type IN imageType,
                                             cl_uint IN numEntries, cl_image_format* OUT formats, cl_uint* OUT numEntriesRet);

protected:
    // Search for firmwares in specified directory
    cl_dev_err_code FindFirmwares(std::string dir);


    cl_int                  m_iDevId;
    IOCLDevLogDescriptor*   m_pLogDescriptor;
    cl_int                  m_iLogHandle;
    CameraShim*             m_pCameraShim;

    BuiltInKernelRegistry*  m_pBuiltInKernelRegistry;
};

}}} // namespace Intel { namespace OpenCL { namespace ISPDevice {
