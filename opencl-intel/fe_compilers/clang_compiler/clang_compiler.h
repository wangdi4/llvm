// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the CLANG_COMPILER_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// CLANG_COMPILER_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.

#pragma once

#include <cl_synch_objects.h>
#include <frontend_api.h>


namespace Intel { namespace OpenCL { namespace ClangFE {

	class ClangFECompiler : public Intel::OpenCL::FECompilerAPI::IOCLFECompiler
	{
	public:
		ClangFECompiler(const char* pszDeviceExtensions);

		// IOCLFECompiler
		int BuildProgram(Intel::OpenCL::FECompilerAPI::FEBuildProgramDescriptor* pProgDesc,
									Intel::OpenCL::FECompilerAPI::IOCLFEBuildProgramResult* *pBuildResult);
		void Release()
		{
			delete this;
		}
	protected:
		~ClangFECompiler();

		char*	m_pszDeviceExtensions; // A string for device supported extensions

		// Static members
		static Intel::OpenCL::Utils::AtomicCounter	s_llvmReferenceCount;
	};

}}}
