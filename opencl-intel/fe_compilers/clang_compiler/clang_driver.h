// Copyright (c) 2006-2009 Intel Corporation
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
//  clang_driver.h
///////////////////////////////////////////////////////////

#pragma once

#include <frontend_api.h>
#include <cl_synch_objects.h>

#include <string>
#include <list>

namespace Intel { namespace OpenCL { namespace ClangFE {

	class ClangFECompilerCompileTask : public Intel::OpenCL::FECompilerAPI::IOCLFEBinaryResult
	{
	public:
		ClangFECompilerCompileTask(Intel::OpenCL::FECompilerAPI::FECompileProgramDescriptor* pProgDesc, const char* pszDeviceExtensions);
		
		int Compile();

		// IOCLFEBinaryResult
		size_t	GetIRSize() {return m_stOutIRSize;}
		const void*	GetIR() { return m_pOutIR;}
		const char* GetErrorLog() {return m_pLogString;}
		long Release() { delete this; return 0; }

	protected:
        ~ClangFECompilerCompileTask();
        
        typedef std::list<std::string> ArgListType;		

        void PrepareArgumentList(ArgListType &list, ArgListType &ignored, const char *buildOpts);

		Intel::OpenCL::FECompilerAPI::FECompileProgramDescriptor* m_pProgDesc;
		const char* m_pszDeviceExtensions;

		bool OptDebugInfo;
		bool Opt_Disable;
		bool Denorms_Are_Zeros;
		bool Fast_Relaxed_Math;
        std::string m_source_filename;

		char*	m_pOutIR;				// Output IR
		size_t	m_stOutIRSize;
		char*	m_pLogString;			// Output log
		size_t	m_stLogSize;

		// Static members
		static Intel::OpenCL::Utils::OclMutex		s_serializingMutex;
	};


    class ClangFECompilerLinkTask : public Intel::OpenCL::FECompilerAPI::IOCLFEBinaryResult
	{
	public:
		ClangFECompilerLinkTask(Intel::OpenCL::FECompilerAPI::FELinkProgramsDescriptor* pProgDesc);
		
		int Link();

		// IOCLFEBinaryResult
		size_t	GetIRSize() {return m_stOutIRSize;}
		const void*	GetIR() { return m_pOutIR;}
		const char* GetErrorLog() {return m_pLogString;}
		long Release() { delete this; return 0;}
        bool IsLibrary() {return bCreateLibrary;}

	protected:
        ~ClangFECompilerLinkTask();

        void ParseOptions(const char *buildOpts);
        void ResolveFlags();

		Intel::OpenCL::FECompilerAPI::FELinkProgramsDescriptor* m_pProgDesc;

		char*	m_pOutIR;				// Output IR
		size_t	m_stOutIRSize;
		char*	m_pLogString;			// Output log
		size_t	m_stLogSize;

        bool bCreateLibrary;
        bool bEnableLinkOptions;

        bool bDenormsAreZero;
        bool bNoSignedZeroes;
        bool bUnsafeMath;
        bool bFiniteMath;
        bool bFastRelaxedMath;

        bool bDebugInfoFlag;
        bool bDenormsAreZeroFlag;
        bool bDisableOptFlag;
        bool bFastRelaxedMathFlag;
        bool bEnableLinkOptionsFlag;
	};
}}}
