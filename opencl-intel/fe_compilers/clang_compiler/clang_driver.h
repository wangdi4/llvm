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

#include "clang_device_info.h"
#include <frontend_api.h>
#include <cl_synch_objects.h>
#include "cl_config.h"

#include <string>
#include <list>

namespace Intel { namespace OpenCL { namespace ClangFE {

    struct ARG_INFO
    {
        char* name;
        char* typeName;
        cl_kernel_arg_address_qualifier adressQualifier;
        cl_kernel_arg_access_qualifier accessQualifier;
        cl_kernel_arg_type_qualifier typeQualifier;
    };

    typedef std::list<std::string> ArgListType;		

    class ClangFETask
    {
    protected:
		static Intel::OpenCL::Utils::OclMutex		s_serializingMutex;
    };

    class ClangFECompilerCompileTask : public Intel::OpenCL::FECompilerAPI::IOCLFEBinaryResult, ClangFETask
	{
	public:
		ClangFECompilerCompileTask(Intel::OpenCL::FECompilerAPI::FECompileProgramDescriptor* pProgDesc, 
																Intel::OpenCL::ClangFE::CLANG_DEV_INFO pszDeviceInfo,
                                                                const Intel::OpenCL::Utils::BasicCLConfigWrapper& config);
		
		int Compile();

		// IOCLFEBinaryResult
		size_t	GetIRSize() {return m_stOutIRSize;}
		const void*	GetIR() { return m_pOutIR;}
		const char*	GetErrorLog() {return m_pLogString;}
		long Release() { delete this; return 0; }

	protected:
        virtual ~ClangFECompilerCompileTask();

        void PrepareArgumentList(ArgListType &list, ArgListType &BEArgList, const char *buildOpts);

        Intel::OpenCL::FECompilerAPI::FECompileProgramDescriptor* m_pProgDesc;
        Intel::OpenCL::ClangFE::CLANG_DEV_INFO	m_sDeviceInfo;

        int  CLSTDSet;
        bool OptDebugInfo;
        bool OptProfiling;
        bool Opt_Disable;
        bool Denorms_Are_Zeros;
        bool Fast_Relaxed_Math;
        std::string m_source_filename;

		char*	m_pOutIR;				// Output IR
		size_t	m_stOutIRSize;
		char*	m_pLogString;			// Output log
		size_t	m_stLogSize;
        const Intel::OpenCL::Utils::BasicCLConfigWrapper& m_config;
    private:
      // private copy constructor to prevent wrong assignment
      ClangFECompilerCompileTask(const ClangFECompilerCompileTask&);
      // private operator= constructor to prevent wrong assignment
      ClangFECompilerCompileTask& 
        operator= (ClangFECompilerCompileTask const &) {return *this;}
	};


    class ClangFECompilerLinkTask : public Intel::OpenCL::FECompilerAPI::IOCLFEBinaryResult, ClangFETask
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
        virtual ~ClangFECompilerLinkTask();

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
        bool bProfilingFlag;
        bool bDenormsAreZeroFlag;
        bool bDisableOptFlag;
        bool bFastRelaxedMathFlag;
        bool bEnableLinkOptionsFlag;
    private:
      // private copy constructor to prevent wrong assignment
      ClangFECompilerLinkTask(const ClangFECompilerLinkTask&) {}
      // private operator= constructor to prevent wrong assignment
      ClangFECompilerLinkTask& 
        operator= (ClangFECompilerLinkTask const &) {return *this;}
	};

    class ClangFECompilerGetKernelArgInfoTask : public Intel::OpenCL::FECompilerAPI::FEKernelArgInfo, ClangFETask
    {
    public:
        ClangFECompilerGetKernelArgInfoTask();

        int GetKernelArgInfo(const void*    pBin,
                             const char*    szKernelName);

        unsigned int getNumArgs() const { return m_numArgs; }
        const char* getArgName(unsigned int index) const { return m_argsInfo[index].name; }
        const char* getArgTypeName(unsigned int index) const { return m_argsInfo[index].typeName; }
        cl_kernel_arg_address_qualifier getArgAdressQualifier(unsigned int index) const { return m_argsInfo[index].adressQualifier; }
        cl_kernel_arg_access_qualifier getArgAccessQualifier(unsigned int index) const { return m_argsInfo[index].accessQualifier; }
        cl_kernel_arg_type_qualifier getArgTypeQualifier(unsigned int index) const { return m_argsInfo[index].typeQualifier; }

        long Release() { delete this; return 0;}
    protected:
        virtual ~ClangFECompilerGetKernelArgInfoTask();

        unsigned int    m_numArgs;
        ARG_INFO*       m_argsInfo;
    private:
      // private copy constructor to prevent wrong assignment
      ClangFECompilerGetKernelArgInfoTask(const ClangFECompilerGetKernelArgInfoTask&)
      : m_numArgs(0), m_argsInfo(NULL) {}
      // private operator= constructor to prevent wrong assignment
      ClangFECompilerGetKernelArgInfoTask& 
        operator= (ClangFECompilerGetKernelArgInfoTask const &) {return *this;}
    };

    // ClangFECompilerCheckCompileOptions
    // Input: szOptions - a string representing the compile options
    // Output: szUnrecognizedOptions - a new string containing the unrecognized options separated by spaces
    // Returns: 'true' if the compile options are legal and 'false' otherwise
    bool ClangFECompilerCheckCompileOptions(const char*  szOptions,
                                            char**       szUnrecognizedOptions,
                                            const Intel::OpenCL::Utils::BasicCLConfigWrapper& config);

    // ClangFECompilerCheckLinkOptions
    // Input: szOptions - a string representing the link options
    // Output: szUnrecognizedOptions - a new string containing the unrecognized options separated by spaces
    // Returns: 'true' if the link options are legal and 'false' otherwise
    bool ClangFECompilerCheckLinkOptions(const char*  szOptions,
                                         char**       szUnrecognizedOptions);

    bool ParseCompileOptions(const char*  szOptions,
                             char**       szUnrecognizedOptions,
                             const Intel::OpenCL::Utils::BasicCLConfigWrapper& config,
                             ArgListType* pList               = NULL,
                             ArgListType* BEArgList           = NULL,
                             int*         pbCLStdSet          = NULL,
                             bool*        pbOptDebugInfo      = NULL,
                             bool*        pbOptProfiling      = NULL,
                             bool*        pbOptDisable        = NULL,
                             bool*        pbDenormsAreZeros   = NULL,
                             bool*        pbFastRelaxedMath   = NULL,
                             std::string* pszFileName         = NULL,
                             std::string* pszTriple           = NULL);

    bool ParseLinkOptions(const char* szOptions,
                          char**      szUnrecognizedOptions,
                          bool*       pbCreateLibrary     = NULL,
                          bool*       pbEnableLinkOptions = NULL,
                          bool*       pbDenormsAreZero    = NULL,
                          bool*       pbNoSignedZeroes    = NULL,
                          bool*       pbUnsafeMath        = NULL,
                          bool*       pbFiniteMath        = NULL,
                          bool*       pbFastRelaxedMath   = NULL);

}}}
