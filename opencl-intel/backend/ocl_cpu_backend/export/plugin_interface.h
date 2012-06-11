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

#include <cstdlib>

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


namespace Intel { namespace OpenCL {
 
namespace DeviceBackend {

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

}//DeviceBackend

namespace Frontend {
    class LinkData;
    class CompileData;
    /*
    * interface for Front end pluging to implement
    */
    struct ICLFrontendPlugin{
        //
        //invoked when a program is being linked
        virtual void OnLink(const LinkData* linkData) = 0;
        //
        //invoked when a program is being compiled
        virtual void OnCompile(const CompileData* compileData) = 0;

        virtual ~ICLFrontendPlugin(){};
    };

}//Forntend

//Serves as a query system for BE/FE plugin
struct IPlugin{
    //When implemented in derived classes, should return a pointer to a backend
    //pluging. Note: NULL valude is not legal.
    virtual DeviceBackend::ICLDevBackendPlugin* getBackendPlugin() = 0;
    //When implemented in derived classes, should return a pointer to a frontend
    //pluging. Note: NULL valude is not legal.
    virtual Frontend::ICLFrontendPlugin* getFrontendPlugin() = 0;
    virtual ~IPlugin() {};
};

}}

#ifdef __cplusplus
extern "C" {
#endif 

//
//BE factory/release methods
//
typedef Intel::OpenCL::IPlugin* (*PLUGIN_CREATE_FUNCPTR)(void);
typedef void (*PLUGIN_RELEASE_FUNCPTR)(Intel::OpenCL::IPlugin*);

#ifdef __cplusplus
}
#endif 
