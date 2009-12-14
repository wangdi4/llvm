// Copyright (c) 2008-2009 Intel Corporation
// All rights reserved.
// 
// WARRANTY DISCLAIMER
// 
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING ANY WAY OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

///////////////////////////////////////////////////////////
//  lrb_agent.h
//  Implementation of the Class LrbAgent
//  Created on:      16-June-2009
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////
#if !defined(__LRB_AGENT_H__)
#define __LRB_AGENT_H__

#include "cl_device_api.h"
#include "lrb_xn_wrapper.h"


namespace Intel { namespace OpenCL { namespace LRBAgent {

    extern const char* LRB_STRING;

    // Forward declarations
    class LrbMemoryManager;
    class LrbCommandExecuter;
    class LrbProgramService;
    class LrbCommunicator;

    /**********************************************************************************************
     * Class name:    LrbAgent
     *
     * Description:    
     *      Represents a Lrb OpenCL device agent.
     *      This class implements all OpenCL device API for the LRB device execution. 
     *      The object is the LRB host library and use XN API to communicate with a LRB Native executable.
     *      The native executable is the Agent portion ruining on the LRB HW.
     *      This class is singleton an OpenCL host application process.
     *
     * Author:        Arnon Peleg
     * Date:          June 2009
    /**********************************************************************************************/    
    class LrbAgent
    {
    public:
        static LrbAgent* GetInstance() 
        {
            if (NULL == m_pLrbInstance)
	        {
		        m_pLrbInstance = new LrbAgent();
	        }
	        return m_pLrbInstance;
        };

        static void Destroy();
        cl_int      Initialize(cl_uint uiDevId, cl_dev_call_backs *devCallbacks, cl_dev_log_descriptor *logDesc);
        cl_int      Release();
                
        //
        // Device entry points, static functions to be registered on the devCallbacks object
        //
        static cl_int clDevGetDeviceInfo        ( cl_device_info param, size_t val_size, void* paramVal, size_t* param_val_size_ret);
        static cl_int clDevCreateCommandList    (cl_dev_cmd_list_props props, cl_dev_cmd_list* list);
        static cl_int clDevRetainCommandList    (cl_dev_cmd_list list);
        static cl_int clDevReleaseCommandList   (cl_dev_cmd_list list);
        static cl_int clDevFlushCommandList     (cl_dev_cmd_list list);
        static cl_int clDevCommandListExecute   (cl_dev_cmd_list list, cl_dev_cmd_desc** cmds, cl_uint count);
        static cl_int clDevGetSupportedImageFormats(cl_dev_mem_flags flags, cl_dev_mem_object_type imageType, cl_uint numEntries, cl_image_format* formats, cl_uint* numEntriesRet);
        static cl_int clDevCreateMemoryObject   (cl_dev_mem_flags flags, const cl_image_format* format, cl_uint dim_count, const size_t* dim_size, void* buffer_ptr, const size_t* pitch, cl_dev_host_ptr_flags host_flags, cl_dev_mem* memObj);
        static cl_int clDevDeleteMemoryObject   ( cl_dev_mem memObj );
        static cl_int clDevCreateMappedRegion   ( cl_dev_cmd_param_map* pMapParams);
        static cl_int clDevReleaseMappedRegion  ( cl_dev_cmd_param_map* pMapParams );
        static cl_int clDevCheckProgramBinary   ( size_t binSize, const void* b);
        static cl_int clDevCreateProgram        (size_t binSize, const void* bin, cl_dev_binary_prop prop, cl_dev_program* prog);
        static cl_int clDevBuildProgram         (cl_dev_program prog, const cl_char* options, void* userData);
        static cl_int clDevReleaseProgram       (cl_dev_program prog);
        static cl_int clDevUnloadCompiler       ();
        static cl_int clDevGetProgramBinary     (cl_dev_program prog, size_t size, void* binary, size_t* sizeRet);
        static cl_int clDevGetBuildLog          (cl_dev_program prog, size_t size, char* log, size_t* size_ret);
        static cl_int clDevGetSupportedBinaries ( cl_uint count, cl_prog_binary_desc* types, size_t* sizeRet );
        static cl_int clDevGetKernelId          (cl_dev_program prog, const char* name, cl_dev_kernel* kernelId);
        static cl_int clDevGetProgramKernels    (cl_dev_program prog, cl_uint numKernels, cl_dev_kernel* kernels, cl_uint* numKernelsRet);
        static cl_int clDevGetKernelInfo        (cl_dev_kernel kernel, cl_dev_kernel_info param, size_t valueSize, void* value, size_t* valueSizeRet);
        static void   clDevCloseDevice          (void);
   
    private:
        LrbAgent();
        ~LrbAgent();

        // Copy functions are not implemented and are private.
        LrbAgent(const LrbAgent&);           // copy constructor
        LrbAgent& operator=(const LrbAgent&);// assignment operator

        static LrbAgent*    m_pLrbInstance;


        // Private members of this agent
        cl_dev_call_backs       m_clRuntimeCallBacks; // Pointer to runtime callbacks to use when Cmd/Build is done

        // Agent main components
        XNWrapper               m_xnWrapper;        // This object is used for all accesses to LRB HW device
        LrbMemoryManager*       m_pMemManager;      // Memory management component
        LrbCommandExecuter*     m_pCommandExecuter; // Responsible to execute flushed commands
        LrbCommunicator*        m_pCommunicator;    // Responsible to handle command done events
        LrbProgramService*      m_pProgServices;    // Responsible to program ops such as build program or create kernel

        // 

        /*
        cl_uint                    m_uiCpuId; 
        OclDynamicLib            m_dlRunTime;
        */


        // Private functions Handlers
        cl_int  SetLogger(cl_uint uiDevId, cl_dev_log_descriptor *logDesc);
    };

}}};    // Intel::OpenCL::LRBAgent
#endif  // !defined(__LRB_AGENT_H__)
