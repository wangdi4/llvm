/*****************************************************************************\

Copyright (c) Intel Corporation (2010).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  plugin_interface.h

\*****************************************************************************/

/*
*
* File plugin_interface.h
* IPlugin is an interface that each plugin should implement
*
*/
#pragma once

#ifdef __cplusplus
extern "C" 
{
#endif

struct _cl_prog_container_header;
struct _cl_work_description_type;

#ifdef __cplusplus
}
#endif

namespace llvm
{
    class Function;
}


namespace Intel { namespace OpenCL { namespace DeviceBackend {

    class ICLDevBackendKernel_;
    class ICLDevBackendProgram_;

    /**
     * ICLDevBackendPlugin is an interface that each plugin should implement 
     */
    class ICLDevBackendPlugin
    {
    public:
        virtual ~ICLDevBackendPlugin(){}

        virtual void OnCreateBinary(const ICLDevBackendKernel_* pKernel, 
                                    const _cl_work_description_type* pWorkDesc, 
                                    size_t bufSize, 
                                    void* pArgsBuffer) =0;

        virtual void OnCreateKernel(const ICLDevBackendProgram_* pProgram,
                                    const ICLDevBackendKernel_* pKernel,
                                    const llvm::Function* pFunction) = 0;

        virtual void OnCreateProgram(const _cl_prog_container_header* pContainer, 
                                     const ICLDevBackendProgram_* pProgram) =0;

        virtual void OnReleaseProgram(const ICLDevBackendProgram_* pProgram) =0;
    };

}}}

#ifdef __cplusplus
extern "C" {
#endif 

typedef Intel::OpenCL::DeviceBackend::ICLDevBackendPlugin* (*PLUGIN_CREATE_FUNCPTR)(void);
typedef void (*PLUGIN_RELEASE_FUNCPTR)(Intel::OpenCL::DeviceBackend::ICLDevBackendPlugin* );
#ifdef __cplusplus
}
#endif 
