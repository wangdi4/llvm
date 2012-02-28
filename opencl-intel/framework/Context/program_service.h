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

#include <cl_types.h>

namespace Intel { namespace OpenCL { namespace Framework {

    typedef void (CL_CALLBACK *pfnNotifyBuildDone)(cl_program, void *);

	class FissionableDevice;
	class Program;
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
		* Function: 	ProgramService
		* Description:	The Program Service class constructor
		* Author:		Sagi Shahar
		* Date:			January 2012
		******************************************************************************************/		
		ProgramService(Context* pContext);

        /******************************************************************************************
		* Function: 	~ProgramService
		* Description:	The Program Service class destructor
		* Author:		Sagi Shahar
		* Date:			January 2012
		******************************************************************************************/	
		~ProgramService();

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
        cl_err_code CompileProgram(Program*             program, 
                                   cl_uint              num_devices, 
                                   const cl_device_id*  device_list, 
                                   cl_uint              num_input_headers, 
                                   Program**            input_headers, 
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
        cl_err_code LinkProgram(Program*            program, 
                                cl_uint             num_devices, 
                                const cl_device_id* device_list, 
                                cl_uint             num_input_programs, 
                                Program**           input_programs, 
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
        cl_err_code BuildProgram(Program*               program, 
                                 cl_uint                num_devices, 
                                 const cl_device_id*    device_list, 
                                 const char*            options, 
                                 pfnNotifyBuildDone     pfn_notify, 
                                 void*                  user_data);

    protected:
        Context*    m_pContext;
    };


    class BuildTask : public Intel::OpenCL::TaskExecutor::ITask, public Intel::OpenCL::Framework::BuildEvent
    {
    public:

        BuildTask(cl_context context);

	    virtual bool	Execute() = 0;

	    virtual long	Release();

        virtual void    DoneWithDependencies(OclEvent* pEvent); 

        unsigned int    Launch();

    protected:

        ~BuildTask();
    };



    class CompileTask : public BuildTask
    {
    public:

        CompileTask(cl_context              context,
                    cl_device_id            deviceID,
                    const FrontEndCompiler* pFECompiler,
                    const char*             szSource,
                    unsigned int            uiNumHeaders,
                    const char**            pszHeaders,
                    char**            pszHeadersNames,
                    const char*             szOptions,
                    Program*                pProg);

	    virtual bool	Execute();

    protected:

        ~CompileTask();

        cl_device_id            m_deviceID;
        const FrontEndCompiler* m_pFECompiler;

        const char*             m_szSource;
        unsigned int            m_uiNumHeaders;
        const char**            m_pszHeaders;
        const char**            m_pszHeadersNames;
        const char*             m_szOptions;

        Program*                m_pProg;
    };


    class LinkTask : public BuildTask
    {
    public:

        LinkTask(cl_context              context,
                 cl_device_id            deviceID,
                 const FrontEndCompiler* pFECompiler,
                 IOCLDeviceAgent*  pDeviceAgent,
                 Program**               ppBinaries,
                 unsigned int            uiNumBinaries,
                 const char*             szOptions,
                 Program*                pProg);

	    virtual bool	Execute();

    protected:

        ~LinkTask();

        cl_device_id            m_deviceID;
        const FrontEndCompiler*       m_pFECompiler;
        IOCLDeviceAgent*        m_pDeviceAgent;

        Program**               m_ppPrograms;
        unsigned int            m_uiNumPrograms;
        const char*             m_szOptions;

        Program*                m_pProg;
    };


    class PostBuildTask : public BuildTask
    {
    public:

        PostBuildTask(cl_context          context,
                      cl_uint             num_devices,
                      const cl_device_id* deviceID,
                      unsigned int        uiNumHeaders,
                      Program**           ppHeaders,
                      char**        	  pszHeadersNames,
                      unsigned int        uiNumBinaries,
                      Program**           ppBinaries,
                      Program*            pProg,
                      const char*         szOptions,
                      pfnNotifyBuildDone  pfn_notify,
                      void*               user_data);

	    virtual bool	Execute();

    protected:

        ~PostBuildTask();

        cl_uint             m_num_devices;
        const cl_device_id* m_deviceID;

        unsigned int        m_uiNumHeaders;
        Program**           m_ppHeaders;
        char**              m_pszHeadersNames;

        unsigned int        m_uiNumBinaries;
        Program**           m_ppBinaries;

        Program*            m_pProg;

        const char*         m_szOptions;

        pfnNotifyBuildDone  m_pfn_notify; 
        void*               m_user_data;
    };

}}} // namespace
