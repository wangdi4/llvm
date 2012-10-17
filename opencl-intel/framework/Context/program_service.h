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
//  program_service.h
//  Implementation of the Program service class
//  Created on:      12-Jan-2012
//  Original author: Sagi Shahar
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "task_executor.h"
#include "build_event.h"
#include "fe_compiler.h"
#include "program.h"
#include <cl_types.h>
#include "cl_shared_ptr.h"

namespace Intel { namespace OpenCL { namespace Framework {    

    typedef void (CL_CALLBACK *pfnNotifyBuildDone)(cl_program, void *);

	class FissionableDevice;
    class FrontEndCompiler;
    class Context;

    /**********************************************************************************************
	* Class name:	ProgramService
	*
	* Description:	represents a program service
	* Author:		Sagi Shahar
	* Date:			January 2012
	**********************************************************************************************/		
	class ProgramService
	{
	public:

        /******************************************************************************************
		* Function: 	~ProgramService
		* Description:	The Program Service class destructor
		* Author:		Sagi Shahar
		* Date:			January 2012
		******************************************************************************************/	
		~ProgramService();

        /******************************************************************************************
		* Function: 	SetContext
		* Description:	set the Context of this ProgramService
		* Author:		Aharon Abramson
        * Arguments:    pContext a SharedPtr<Context> pointing to the Context to be set
		* Date:			March 2012
		******************************************************************************************/	
        void SetContext(Context* pContext)
        {
            m_pContext = pContext;
        }

        /******************************************************************************************
		* Function: 	CompileProgram  
		* Description:	Compile program from a set of source and headers
		* Arguments:	program - The program to be compiled
        *               num_devices - the number of devices in device_list
        *               device_list - a list of devices on which to compile the program
        *               num_input_headers - the number of headers in input_headers
        *               input_headers - a list of programs to be used as input headers
        *               header_include_names - a list of headers names corresponding to input_headers
        *               options - compile options
        *               pfn_notify - the notify function from the user
        *               user_data - user data to be passed to pfn_notify
		* Return value:	CL_SUCCESS - The operation succeeded
		* Author:		Sagi Shahar
		* Date:			January 2012
		******************************************************************************************/
        cl_err_code CompileProgram(const SharedPtr<Program>&   program, 
                                   cl_uint              num_devices, 
                                   const cl_device_id*  device_list, 
                                   cl_uint              num_input_headers, 
                                   SharedPtr<Program>*  input_headers, 
                                   const char**         header_include_names, 
                                   const char*          options, 
                                   pfnNotifyBuildDone   pfn_notify, 
                                   void*                user_data);

        /******************************************************************************************
		* Function: 	LinkProgram  
		* Description:	Link program from a set of binaries
		* Arguments:	program - The output linked program
        *               num_devices - the number of devices in device_list
        *               device_list - a list of devices on which to compile the program
        *               num_input_programs - the number of binaries in input_programs
        *               input_programs - a list of programs to be linked
        *               options - link options
        *               pfn_notify - the notify function from the user
        *               user_data - user data to be passed to pfn_notify
		* Return value:	CL_SUCCESS - The operation succeeded
		* Author:		Sagi Shahar
		* Date:			January 2012
		******************************************************************************************/
        cl_err_code LinkProgram(const SharedPtr<Program>& program, 
                                cl_uint             num_devices, 
                                const cl_device_id* device_list, 
                                cl_uint             num_input_programs, 
                                SharedPtr<Program>* input_programs, 
                                const char*         options, 
                                pfnNotifyBuildDone  pfn_notify, 
                                void*               user_data);

        /******************************************************************************************
		* Function: 	BuildProgram  
		* Description:	build (compile and link) program
		* Arguments:	program - The program to be built
        *               num_devices - the number of devices in device_list
        *               device_list - a list of devices on which to compile the program
        *               options - build options
        *               pfn_notify - the notify function from the user
        *               user_data - user data to be passed to pfn_notify
		* Return value:	CL_SUCCESS - The operation succeeded
		* Author:		Sagi Shahar
		* Date:			January 2012
		******************************************************************************************/
        cl_err_code BuildProgram(const SharedPtr<Program>& program, 
                                 cl_uint                num_devices, 
                                 const cl_device_id*    device_list, 
                                 const char*            options, 
                                 pfnNotifyBuildDone     pfn_notify, 
                                 void*                  user_data);

    protected:
        Context*    m_pContext; // since ProgramService is aggregated by Context, this can be a regular pointer
    };


    class BuildTask : public Intel::OpenCL::Framework::BuildEvent, public Intel::OpenCL::TaskExecutor::ITask
    {
    public:

        PREPARE_SHARED_PTR(BuildTask);
        BuildTask(_cl_context_int* context,
					const SharedPtr<Program>&      			pProg,
					const ConstSharedPtr<FrontEndCompiler>& pFECompiler);

	    virtual bool	Execute() = 0;

	    virtual long	Release();

        virtual void    DoneWithDependencies(SharedPtr<OclEvent> pEvent); 

        unsigned int    Launch();
		
		virtual	void	SetComplete(cl_int returnCode); 

    protected:

        BuildTask(cl_context context);

        ~BuildTask();

        SharedPtr<Program>					m_pProg;
        ConstSharedPtr<FrontEndCompiler>	m_pFECompiler;

    private:

        class BuildTaskSharedPtr : public SmartPtr<Intel::OpenCL::TaskExecutor::ITaskBase>, public SharedPtr<BuildTask>
        {
        public:

            BuildTaskSharedPtr(BuildTask* ptr) : SmartPtr<Intel::OpenCL::TaskExecutor::ITaskBase>(ptr), SharedPtr<BuildTask>(ptr) { }
        };
		
    };

    class CompileTask : public BuildTask
    {
    public:

        PREPARE_SHARED_PTR(CompileTask);                

        static SharedPtr<CompileTask> Allocate(
            _cl_context_int*        context,
            cl_device_id            deviceID,
            const ConstSharedPtr<FrontEndCompiler>& pFECompiler,
            const char*             szSource,
            unsigned int            uiNumHeaders,
            const char**            pszHeaders,
            char**					pszHeadersNames,
            const char*             szOptions,
			const SharedPtr<Program>&      pProg)
        {
            return SharedPtr<CompileTask>(new CompileTask(context, deviceID,
                pFECompiler, szSource, uiNumHeaders, pszHeaders, pszHeadersNames, szOptions, pProg));
        }

	    virtual bool	Execute();
		bool	SetAsSyncPoint() {assert(0&&"Should not be called");return false;}
		bool	IsCompleted() const {assert(0&&"Should not be called");return true;}
		bool	CompleteAndCheckSyncPoint() {return false;}

    protected:

        CompileTask(_cl_context_int*        context,
                    cl_device_id            deviceID,
					const ConstSharedPtr<FrontEndCompiler>& pFECompiler,
                    const char*             szSource,
                    unsigned int            uiNumHeaders,
                    const char**            pszHeaders,
                    char**					pszHeadersNames,
                    const char*             szOptions,
                    const SharedPtr<Program>&      pProg);

        ~CompileTask();

        cl_device_id            m_deviceID;

        const char*             m_szSource;
        unsigned int            m_uiNumHeaders;
        const char**            m_pszHeaders;
        const char**            m_pszHeadersNames;
        const char*             m_szOptions;
    };


    class LinkTask : public BuildTask
    {
    public:

        PREPARE_SHARED_PTR(LinkTask);        

        static SharedPtr<LinkTask> Allocate(
            _cl_context_int*         context,
            cl_device_id            deviceID,
            const ConstSharedPtr<FrontEndCompiler>& pFECompiler,
            IOCLDeviceAgent*		 pDeviceAgent,
            SharedPtr<Program>*      ppBinaries,
            unsigned int             uiNumBinaries,
            const char*              szOptions,
            const SharedPtr<Program>& pProg)
        {
            return SharedPtr<LinkTask>(new LinkTask(context, deviceID, pFECompiler, pDeviceAgent, ppBinaries, uiNumBinaries, szOptions, pProg));
        }

	    virtual bool	Execute();
		bool	SetAsSyncPoint() {assert(0&&"Should not be called");return false;}
		bool	IsCompleted() const {assert(0&&"Should not be called");return true;}
		bool	CompleteAndCheckSyncPoint() {return false;}

    protected:

        LinkTask(_cl_context_int*         context,
                 cl_device_id            deviceID,
                 const ConstSharedPtr<FrontEndCompiler>& pFECompiler,
                 IOCLDeviceAgent*		 pDeviceAgent,
                 SharedPtr<Program>*               ppBinaries,
                 unsigned int            uiNumBinaries,
                 const char*             szOptions,
                 const SharedPtr<Program>&  pProg);

        ~LinkTask();

        cl_device_id            m_deviceID;
        IOCLDeviceAgent*        m_pDeviceAgent;

        SharedPtr<Program>* 	m_ppPrograms;
        unsigned int            m_uiNumPrograms;
        const char*             m_szOptions;
    };


    class PostBuildTask : public BuildTask
    {
    public:        

        PREPARE_SHARED_PTR(PostBuildTask);        

        static SharedPtr<PostBuildTask> Allocate(
            _cl_context_int*    context,
            cl_uint             num_devices,
            const cl_device_id* deviceID,
            unsigned int        uiNumHeaders,
            SharedPtr<Program>*           ppHeaders,
            char**        	  pszHeadersNames,
            unsigned int        uiNumBinaries,
            SharedPtr<Program>*           ppBinaries,
            const SharedPtr<Program>&     pProg,
            const char*         szOptions,
            pfnNotifyBuildDone  pfn_notify,
            void*               user_data)
        {
            return SharedPtr<PostBuildTask>(new PostBuildTask(context, num_devices, deviceID, uiNumHeaders, ppHeaders, pszHeadersNames, uiNumBinaries,
                ppBinaries, pProg, szOptions, pfn_notify, user_data));
        }

	    virtual bool	Execute();
		bool	SetAsSyncPoint() {assert(0&&"Should not be called");return false;}
		bool	IsCompleted() const {assert(0&&"Should not be called");return true;}
		bool	CompleteAndCheckSyncPoint() {return false;}

    protected:

        PostBuildTask(_cl_context_int*    context,
                      cl_uint             num_devices,
                      const cl_device_id* deviceID,
                      unsigned int        uiNumHeaders,
                      SharedPtr<Program>* ppHeaders,
                      char**        	  pszHeadersNames,
                      unsigned int        uiNumBinaries,
                      SharedPtr<Program>* ppBinaries,
                      const SharedPtr<Program>& pProg,
                      const char*         szOptions,
                      pfnNotifyBuildDone  pfn_notify,
                      void*               user_data);

        ~PostBuildTask();

        cl_uint             m_num_devices;
        const cl_device_id* m_deviceID;

        unsigned int        m_uiNumHeaders;
        SharedPtr<Program>* m_ppHeaders;
        char**              m_pszHeadersNames;

        unsigned int        m_uiNumBinaries;
        SharedPtr<Program>* m_ppBinaries;

		const char*         m_szOptions;

        pfnNotifyBuildDone  m_pfn_notify; 
        void*               m_user_data;
	private:
		PostBuildTask(const PostBuildTask&);
		PostBuildTask& operator=(const PostBuildTask&);
    };

}}} // namespace
