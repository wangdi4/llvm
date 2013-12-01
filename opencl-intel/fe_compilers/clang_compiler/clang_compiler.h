// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the CLANG_COMPILER_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// CLANG_COMPILER_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.

#pragma once

#include <cl_synch_objects.h>
#include <frontend_api.h>
#include "clang_device_info.h"
#include "cl_config.h"

#ifdef OCLFRONTEND_PLUGINS 
#include "plugin_manager.h"
#endif


namespace Intel { namespace OpenCL { namespace ClangFE {

	class ClangFECompiler : public Intel::OpenCL::FECompilerAPI::IOCLFECompiler
	{
	public:
		ClangFECompiler(const void* pszDeviceInfo);

		  // IOCLFECompiler
        int CompileProgram(Intel::OpenCL::FECompilerAPI::FECompileProgramDescriptor* pProgDesc, 
                           Intel::OpenCL::FECompilerAPI::IOCLFEBinaryResult* *pBinaryResult);

        int LinkPrograms(Intel::OpenCL::FECompilerAPI::FELinkProgramsDescriptor* pProgDesc, 
                         Intel::OpenCL::FECompilerAPI::IOCLFEBinaryResult* *pBinaryResult);

        int GetKernelArgInfo(const void*        pBin,
                             const char*        szKernelName,
                             Intel::OpenCL::FECompilerAPI::FEKernelArgInfo*   *pArgInfo);

        bool CheckCompileOptions(const char*  szOptions,
                                 char**       szUnrecognizedOptions);

        bool CheckLinkOptions(const char*  szOptions,
                              char**       szUnrecognizedOptions);

		void Release()
		{
			delete this;
		}

        static void ShutDown();
        
	protected:
		virtual ~ClangFECompiler();

		CLANG_DEV_INFO	m_sDeviceInfo;

		// Static members
		static Intel::OpenCL::Utils::AtomicCounter	s_llvmReferenceCount;
        static volatile bool                        m_bLllvmActive;
        Intel::OpenCL::Utils::BasicCLConfigWrapper  m_config;
        
    #ifdef OCLFRONTEND_PLUGINS 
    mutable Intel::OpenCL::PluginManager m_pluginManager;
    #endif //OCLFRONTEND_PLUGINS
	};

}}}
