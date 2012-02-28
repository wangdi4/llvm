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
//  clang_driver.cpp
///////////////////////////////////////////////////////////

#include "stdafx.h"

#include "clang/Basic/Diagnostic.h"
#include "clang/Driver/Arg.h"
#include "clang/Driver/ArgList.h"
#include "clang/Driver/CC1Options.h"
#include "clang/Driver/DriverDiagnostic.h"
#include "clang/Driver/OptTable.h"
#include "clang/CodeGen/CodeGenAction.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/FrontendDiagnostic.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Frontend/TextDiagnosticBuffer.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/FrontendTool/Utils.h"
#include "llvm/Linker.h"
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/ADT/OwningPtr.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/Bitcode/BitstreamWriter.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/SystemUtils.h"
#include "llvm/Support/IRReader.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/Path.h"

#include "clang_driver.h"

#include <Logger.h>
#include <cl_sys_info.h>
#include <cl_cpu_detect.h>

#include <string>
#include <list>
#include <vector>
#include <cctype>
#include <algorithm>
using namespace Intel::OpenCL::ClangFE;
using namespace llvm;
using namespace clang;
using namespace clang::frontend;
using namespace Intel::OpenCL::Utils;

using namespace std;

#if defined (_WIN32)
#define GET_CURR_WORKING_DIR(len, buff) GetCurrentDirectoryA(len, buff)
#define PASS_PCH
#else
#define GET_CURR_WORKING_DIR(len, buff) getcwd(buff, len)
#endif

#define MAX_STR_BUFF	1024

// Declare logger client
DECLARE_LOGGER_CLIENT;

OclMutex ClangFECompilerCompileTask::s_serializingMutex;

void LLVMErrorHandler(void *UserData, const std::string &Message) {
  DiagnosticsEngine &Diags = *static_cast<DiagnosticsEngine*>(UserData);

  Diags.Report(diag::err_fe_error_backend) << Message;

  // We cannot recover from llvm errors.
  exit(1);
}

// Tokenize a string into tokens separated by any char in 'delims'. 
// Support quoting to allow some tokens to contain delimiters, with possible
// escape characters to support quotes inside quotes.
// To disable quoting or escaping, set relevant chars to '\x00'.
//
static vector<string> quoted_tokenize(string str, string delims, char quote, char escape)
{
    vector<string> ret;
    string::size_type ptr = str.find_first_not_of(delims);

    if (ptr == string::npos)
        return ret;

    // A state machine, with the following state vars:
    //
    // ptr        - points to the current char in the string
    // is_escaped - is the current char escaped (i.e. was the
    //              previous char = escape, inside a quote)
    // in_quote   - are we in a quote now (i.e. a quote character
    //              appeared without a maching closing quote)
    // tok        - accumulates the current token. once an unquoted
    //              delimiter or end of string is encountered, tok
    //              is added to the return vector and re-initialized
    //
    bool is_escaped = false;
    bool in_quote = false;
    string tok;

    do
    {
        char c = str.at(ptr);

        if (c == quote)
        {
            if (in_quote)
            {
                if (is_escaped)
                    tok += c;
                else
                    in_quote = false;
            }
            else
                in_quote = true;

            is_escaped = false;
        }
        else if (c == escape)
        {
            if (in_quote)
            {
                if (is_escaped)
                {
                    tok += c;
                    is_escaped = false;
                }
                else
                    is_escaped = true;
            }
            else
            {
                tok += c;
                is_escaped = false;
            }
        }
        else if (delims.find(c) != string::npos)
        {
            if (in_quote)
                tok += c;
            else
            {
                ret.push_back(tok);
                tok.clear();
                ptr = str.find_first_not_of(delims, ptr);

                if (ptr == string::npos)
                    break;
                else
                    --ptr; // will be increased at end of iteration
            }

            is_escaped = false;
        }
        else
            tok += c;

        if (ptr == str.size() - 1)
            ret.push_back(tok);
    }
    while (++ptr < str.size());

    return ret;
}


// ClangFECompilerCompileTask calls implementation
ClangFECompilerCompileTask::ClangFECompilerCompileTask(Intel::OpenCL::FECompilerAPI::FECompileProgramDescriptor* pProgDesc, const char* pszDeviceExtensions)
: m_pProgDesc(pProgDesc), m_pszDeviceExtensions(pszDeviceExtensions), m_pOutIR(NULL), m_stOutIRSize(0), m_pLogString(NULL), m_stLogSize(0)
{
}

ClangFECompilerCompileTask::~ClangFECompilerCompileTask()
{
	if ( NULL != m_pOutIR )
	{
		delete []m_pOutIR;
	}

	if ( NULL != m_pLogString )
	{
		delete []m_pLogString;
	}
}

void ClangFECompilerCompileTask::PrepareArgumentList(ArgListType &list, ArgListType &ignored, const char *buildOpts)
{
	// Reset options
	OptDebugInfo = false;
	Opt_Disable = false;
	Denorms_Are_Zeros = false;
	Fast_Relaxed_Math = false;
	
    if (!buildOpts)
        buildOpts = "";

    // Parse the build options - handle the ones we understand and pass the
    // rest into 'ignored'. Use " quoting to accept token with whitespace, but
    // don't use escaping (since we only need path tokens).
    //
    vector<string> opts = quoted_tokenize(buildOpts, " \t", '"', '\x00');
    vector<string>::const_iterator opt_i = opts.begin();
    while (opt_i != opts.end()) {
        if (*opt_i == "-g") {
            list.push_back(*opt_i);
            OptDebugInfo = true;
        } 
        else if (*opt_i == "-w") {
            list.push_back(*opt_i);
        }
        else if (opt_i->find("-D") == 0 || opt_i->find("-I") == 0) {
            if (opt_i->length() == 2) {
                // Definition is separated from the flag, so grab it from the
                // next token
                //
                string flag = *opt_i;
                if (++opt_i != opts.end()) {
                    list.push_back(flag);
                    list.push_back(*opt_i);
                }
                else {
                    ignored.push_back(flag);
                    continue;
                }
            }
            else {
                // Definition is attached to the flag, so pass it as is
                //
                list.push_back(*opt_i);
            }
        }
        else if (*opt_i == "-s") {
            // Expect the file name as the next token
            //
            if (++opt_i != opts.end()) {
                m_source_filename = *opt_i;
                // Normalize path to contain forward slashes
                replace(
                    m_source_filename.begin(), 
                    m_source_filename.end(), 
                    '\\', '/');

                // On Windows only, normalize the filename to lowercase, since
                // LLVM saves buffer names in a case-sensitive manner, while
                // other Windows tools don't.
                //
#ifdef _WIN32
                transform(
                    m_source_filename.begin(),
                    m_source_filename.end(),
                    m_source_filename.begin(),
                    ::tolower);
#endif
            }
        }
        else if (*opt_i == "-Werror") {
            list.push_back(*opt_i);
        }
        else if (   *opt_i == "-cl-mad-enable" ||
                    *opt_i == "-cl-strict-aliasing" || 
                    *opt_i == "-cl-no-signed-zeros" || 
                    *opt_i == "-cl-unsafe-math-optimizations" || 
                    *opt_i == "-cl-single-precision-constant" || 
                    opt_i->find("-dump-opt-llvm=") == 0) {
            // nop
        }
        else if (*opt_i == "-cl-opt-disable") {
            Opt_Disable = true;
        }
        else if (*opt_i == "-cl-denorms-are-zero") {
            Denorms_Are_Zeros = true;
        }
        else if (*opt_i == "-cl-finite-math-only") {
            list.push_back("-D");
            list.push_back("__FINITE_MATH_ONLY__=1");
        }
        else if (*opt_i == "-cl-fast-relaxed-math") {
            list.push_back("-D");
            list.push_back("__FAST_RELAXED_MATH__=1");
            Fast_Relaxed_Math = true;
        }
        else {
            ignored.push_back(*opt_i);
        }

        ++opt_i;
    }

	// Add standard OpenCL options

	list.push_back("-x");
	list.push_back("cl");
	list.push_back("-S");
	list.push_back("-emit-llvm-bc");

	// Add CPU type
	unsigned int uFeatures = CPUDetect::GetInstance()->GetCPUFeatureSupport();
	std::string CPUType = "pentium4";
	if ( uFeatures & CFS_SSE42 )
	{
		CPUType = "corei7";
	}
	else if ( uFeatures & CFS_SSE41 )
	{
		CPUType = "penryn";
	}
	else if ( uFeatures & CFS_SSSE3 )
	{
		CPUType = "core2";
	}
	else if ( uFeatures & CFS_SSE3 )
	{
		CPUType = "prescott";
	}

	list.push_back("-target-cpu");
	list.push_back(CPUType);

	char	szBinaryPath[MAX_STR_BUFF];
	char	szCurrDirrPath[MAX_STR_BUFF];

	// Retrieve local relatively to binary directory
	GetModuleDirectory(szBinaryPath, MAX_STR_BUFF);
#ifndef PASS_PCH
    char	szOclIncPath[MAX_STR_BUFF];
	char	szOclPchPath[MAX_STR_BUFF];

	SPRINTF_S(szOclIncPath, MAX_STR_BUFF, "%sfe_include", szBinaryPath);
	SPRINTF_S(szOclPchPath, MAX_STR_BUFF, "%sopencl_.pch", szBinaryPath);

	list.push_back("-I");
	list.push_back(szOclIncPath);

	list.push_back("-include-pch");
	list.push_back(szOclPchPath);
#else
    list.push_back("-include-pch");
	list.push_back("OpenCL_.pch");
#endif

    list.push_back("-fno-validate-pch");

	// Add current directory
	GET_CURR_WORKING_DIR(MAX_STR_BUFF, szCurrDirrPath);
	list.push_back("-I");
	list.push_back(szCurrDirrPath);

	//Add OpenCL predefined macros
	list.push_back("-D");
	list.push_back("__OPENCL_VERSION__=110");
	list.push_back("-D");
	list.push_back("CL_VERSION_1_0=100");
	list.push_back("-D");
	list.push_back("CL_VERSION_1_1=110");
	list.push_back("-D");
	list.push_back("__ENDIAN_LITTLE__=1");
	list.push_back("-D");
	list.push_back("__ROUNDING_MODE__=rte");	
	list.push_back("-D");
	list.push_back("__IMAGE_SUPPORT__=1");	

	// Add extension defines
	std::string extStr = m_pszDeviceExtensions;
	while(extStr != "")
	{
        std::string subExtStr;
        std::string::size_type pos = extStr.find(" ", 0);
		if(pos == string::npos)
		{
            subExtStr = extStr;
			extStr.clear();
		}
		else
		{
			subExtStr = extStr.substr(0, pos);
			extStr = extStr.substr(pos + 1);
		}

		list.push_back("-D");
        list.push_back(subExtStr);

        //list.push_back("-target-feature");
        //list.push_back('+' + subExtStr);
	}

	// Don't optimize in the frontend
	list.push_back("-O0");
#if defined(__linux__)	
	list.push_back("-triple");
	list.push_back("x86_64-unknown-linux-gnu");
#endif

  list.push_back("-cl-kernel-arg-info");
}

int ClangFECompilerCompileTask::Compile()
{
	LOG_INFO(TEXT("%s"), TEXT("enter"));

	OclAutoMutex CS(&s_serializingMutex);

	ArgListType ArgList;
	ArgListType IgnoredArgs;

	PrepareArgumentList(ArgList, IgnoredArgs, m_pProgDesc->pszOptions);

	const char **argArray = new const char *[ArgList.size()];
	ArgListType::iterator iter = ArgList.begin();

	for(unsigned int i=0; i<ArgList.size(); i++)
	{
		argArray[i] = iter->c_str();
		iter++;
	}

	SmallVector<char, 4096>	Log;
	llvm::raw_svector_ostream errStream(Log);

	while(!IgnoredArgs.empty())
	{
		errStream << "warning: ignoring build option: \"";
		errStream << IgnoredArgs.front();
		errStream << "\"\n";
		IgnoredArgs.pop_front();
	}

	llvm::OwningPtr<CompilerInstance> Clang(new CompilerInstance());
    llvm::IntrusiveRefCntPtr<DiagnosticIDs> DiagID(new DiagnosticIDs());

	// Buffer diagnostics from argument parsing so that we can output them using a
	//well formed diagnostic object.
	TextDiagnosticBuffer *DiagsBuffer = new TextDiagnosticBuffer;
	DiagnosticsEngine Diags(DiagID, DiagsBuffer);
	CompilerInvocation::CreateFromArgs(Clang->getInvocation(), argArray, argArray + ArgList.size(),
                                     Diags);

	// Create the actual diagnostics engine.
	Clang->SetErrorStream(&errStream);

	Clang->createDiagnostics((int)ArgList.size(), const_cast<char**>(argArray));
	if (!Clang->hasDiagnostics())
	{
		LOG_ERROR(TEXT("Failed to create diagnostics"), "");
		delete []argArray;
		return CL_OUT_OF_HOST_MEMORY;
	}

	// don't write anything on the screen
	Clang->getDiagnosticOpts().ShowCarets = 0;

	// Set an error handler, so that any LLVM backend diagnostics go through our
	// error handler.
	llvm::remove_fatal_error_handler();
	llvm::install_fatal_error_handler(LLVMErrorHandler,
                                  static_cast<void*>(&Clang->getDiagnostics()));

	DiagsBuffer->FlushDiagnostics(Clang->getDiagnostics());

	llvm::MemoryBuffer *SB = llvm::MemoryBuffer::getMemBuffer(
        m_pProgDesc->pProgramSource, 
        m_source_filename);
	Clang->SetInputBuffer(SB);

    for (unsigned int i = 0; i < m_pProgDesc->uiNumInputHeaders; ++i)
    {
        llvm::MemoryBuffer *header = llvm::MemoryBuffer::getMemBufferCopy(m_pProgDesc->pInputHeaders[i]);
        Clang->AddInMemoryHeader(header, m_pProgDesc->pszInputHeadersNames[i]);
    }

	//prepare output buffer
	SmallVector<char, 4096>	IRbinary;
	llvm::raw_svector_ostream *IRStream = new llvm::raw_svector_ostream(IRbinary);
	Clang->SetOutputStream(IRStream);
    
    bool Success = true;

#ifdef PASS_PCH
	//prepare pch buffer
	HMODULE hMod = NULL;
	HRSRC hRes = NULL;
	HGLOBAL hBytes = NULL;
	char *pData = NULL;
	size_t dResSize = NULL;
	llvm::MemoryBuffer *pchBuff = NULL;

	// Get the handle to the current module
	GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | 
					  GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, 
					  (LPCWSTR)L"clang_compiler",
					  &hMod);

	// Locate the resource
	if( NULL != hMod )
	{
		hRes = FindResource(hMod, L"#101", L"PCH");
	}
    else
    {
        LOG_ERROR(TEXT("%s"), "hMod is NULL");
        Success = false;
    }

	// Load the resource
	if( NULL != hRes )
	{
		hBytes = LoadResource(hMod, hRes);
	}
    else
    {
        LOG_ERROR(TEXT("%s"), "hRes is NULL");
        Success = false;
    }
	
	// Get the base address to the resource. This call doesn't really lock it
	if( NULL != hBytes )
	{
		pData = (char *)LockResource(hBytes);
	}
    else
    {
        LOG_ERROR(TEXT("%s"), "hBytes is NULL");
        Success = false;
    }
	
	// Get the buffer size
	if( NULL != pData )
	{
		dResSize = SizeofResource(hMod, hRes);
	}
    else
    {
        LOG_ERROR(TEXT("%s"), "pData is NULL");
        Success = false;
    }

	if( dResSize > 0 )
	{	
		pchBuff = llvm::MemoryBuffer::getMemBufferCopy(StringRef(pData, dResSize));
	}
    else
    {
        LOG_ERROR(TEXT("%s"), "dResSize <= 0");
        Success = false;
    }

	if( NULL != pchBuff )
	{
		Clang->SetPchBuffer(pchBuff);
	}
    else
    {
        LOG_ERROR(TEXT("%s"), "pchBuff is NULL");
        Success = false;
    }
#endif
    
    // Execute the frontend actions.
    if (Success)
    {
        try {
            Success = ExecuteCompilerInvocation(Clang.get());
        } catch (const std::exception&) {
            Success = false;
            LOG_ERROR(TEXT("CompileTask::Execute() - caught an exception during compilation"), "");
        }
    }    

	// Our error handler depends on the Diagnostics object, which we're
	// potentially about to delete. Uninstall the handler now so that any
	// later errors use the default handling behavior instead.
	llvm::remove_fatal_error_handler();

	//Clang.take();

	IRStream->flush();

  delete IRStream;
	errStream.flush();
	if ( !Log.empty() )
	{
		m_pLogString = new char[Log.size()+1];
		if ( m_pLogString != NULL )
		{
			MEMCPY_S(m_pLogString, Log.size(), Log.begin(), Log.size());
			m_pLogString[Log.size()] = '\0';
		}
	}
	
	if (!Success)
	{
		return CL_BUILD_PROGRAM_FAILURE;
	}

	size_t	stTotSize = 0;
	if ( !IRbinary.empty() )
	{
		stTotSize = IRbinary.size()+
			sizeof(cl_prog_container_header)+sizeof(cl_llvm_prog_header);
		m_pOutIR = new char[stTotSize];
		if ( NULL == m_pOutIR )
		{
			LOG_ERROR(TEXT("%s"), TEXT("Failed to allocate memory for buffer"));
			delete []argArray;
			return CL_OUT_OF_HOST_MEMORY;
		}
	}

	if ( NULL != m_pOutIR )
	{
		m_stOutIRSize = stTotSize;
		cl_prog_container_header*	pHeader = (cl_prog_container_header*)m_pOutIR;
		memcpy(pHeader->mask, _CL_CONTAINER_MASK_, 4);
		pHeader->container_size = IRbinary.size()+sizeof(cl_llvm_prog_header);
		pHeader->container_type = CL_PROG_CNT_PRIVATE;
		pHeader->description.bin_type = CL_PROG_BIN_COMPILED_LLVM;
		pHeader->description.bin_ver_major = 1;
		pHeader->description.bin_ver_minor = 1;
		// Fill options
		cl_llvm_prog_header *pProgHeader = (cl_llvm_prog_header*)(m_pOutIR+sizeof(cl_prog_container_header));
		pProgHeader->bDebugInfo = OptDebugInfo;
		pProgHeader->bDisableOpt = Opt_Disable;
		pProgHeader->bDenormsAreZero = Denorms_Are_Zeros;
		pProgHeader->bFastRelaxedMath = Fast_Relaxed_Math;
        pProgHeader->bEnableLinkOptions = true; // enable link options is true for all compiled objects
		void *pIR = (void*)(pProgHeader+1);
		// Copy IR
		MEMCPY_S(pIR, IRbinary.size(), IRbinary.begin(), IRbinary.size());
	}

	IRbinary.clear();
	LOG_INFO(TEXT("%s"), TEXT("Finished"));

	delete []argArray;

	return CL_SUCCESS;
}


// ClangFECompilerLinkTask calls implementation
ClangFECompilerLinkTask::ClangFECompilerLinkTask(Intel::OpenCL::FECompilerAPI::FELinkProgramsDescriptor *pProgDesc)
: m_pProgDesc(pProgDesc), m_pOutIR(NULL), m_stOutIRSize(0), m_pLogString(NULL), m_stLogSize(0)
{
}

ClangFECompilerLinkTask::~ClangFECompilerLinkTask()
{
    if ( NULL != m_pOutIR )
	{
		delete []m_pOutIR;
	}

	if ( NULL != m_pLogString )
	{
		delete []m_pLogString;
	}
}

int ClangFECompilerLinkTask::Link()
{
    if (0 == m_pProgDesc->uiNumBinaries)
    {
        return CL_SUCCESS;
    }

    ParseOptions(m_pProgDesc->pszOptions);
    ResolveFlags();

    if (1 == m_pProgDesc->uiNumBinaries)
    {
        m_stOutIRSize = m_pProgDesc->puiBinariesSizes[0];

        m_pOutIR = new char[m_pProgDesc->puiBinariesSizes[0]];
        if ( NULL == m_pOutIR )
		{
			LOG_ERROR(TEXT("%s"), TEXT("Failed to allocate memory for buffer"));
            m_stOutIRSize = 0;
			return CL_OUT_OF_HOST_MEMORY;
		}
    
        MEMCPY_S(m_pOutIR, m_stOutIRSize, m_pProgDesc->pBinaryContainers[0], m_stOutIRSize);

        cl_prog_container_header*	pHeader = (cl_prog_container_header*)m_pOutIR;
        pHeader->description.bin_type = CL_PROG_BIN_LINKED_LLVM;

        cl_llvm_prog_header* pProgHeader = (cl_llvm_prog_header*)((char*)pHeader + sizeof(cl_prog_container_header));

        pProgHeader->bDebugInfo = bDebugInfoFlag;
        pProgHeader->bDenormsAreZero = bDenormsAreZeroFlag;
        pProgHeader->bDisableOpt = bDisableOptFlag;
        pProgHeader->bFastRelaxedMath = bFastRelaxedMathFlag;
        pProgHeader->bEnableLinkOptions = bEnableLinkOptionsFlag;

        return CL_SUCCESS;
    }

    // We have more the one binary so we need to link
    
    SMDiagnostic Err;
    string ErrorMessage;
    LLVMContext &Context = getGlobalContext();

    // Initialize the module with the first binary
    cl_prog_container_header* pHeader = (cl_prog_container_header*) m_pProgDesc->pBinaryContainers[0];

    cl_llvm_prog_header* pProgHeader = (cl_llvm_prog_header*)((char*)pHeader + sizeof(cl_prog_container_header));

    size_t uiBinarySize = pHeader->container_size - sizeof(cl_llvm_prog_header);

    char* pBinary = (char*)pHeader + 
                    sizeof(cl_prog_container_header) + // Skip the container
                    sizeof(cl_llvm_prog_header); //Skip the build flags for now

    MemoryBuffer *pBinBuff = MemoryBuffer::getMemBufferCopy(StringRef(pBinary, uiBinarySize));
           
    std::auto_ptr<Module> composite(ParseIR(pBinBuff, Err, Context));

    if (composite.get() == 0) 
    {
        ErrorMessage = Err.getMessage();
        if ( !ErrorMessage.empty() )
	    {
            m_pLogString = new char[ErrorMessage.length() + 1];
		    if ( m_pLogString != NULL )
		    {
                MEMCPY_S(m_pLogString, ErrorMessage.length(), ErrorMessage.c_str(), ErrorMessage.length());
			    m_pLogString[ErrorMessage.length()] = '\0';
		    }
	    }

        m_stOutIRSize = 0;
        m_pOutIR = NULL;

        return CL_LINK_PROGRAM_FAILURE;
    }

    // Now go over the rest of the binaries and add them
    for (unsigned int i = 1; i < m_pProgDesc->uiNumBinaries; ++i)
    {
        pHeader = (cl_prog_container_header*) m_pProgDesc->pBinaryContainers[i];

        pProgHeader = (cl_llvm_prog_header*)((char*)pHeader + sizeof(cl_prog_container_header));

        uiBinarySize = pHeader->container_size - sizeof(cl_llvm_prog_header);

        char* pBinary = (char*)pHeader + 
                        sizeof(cl_prog_container_header) + // Skip the container
                        sizeof(cl_llvm_prog_header); //Skip the build flags for now

        MemoryBuffer *pBinBuff = MemoryBuffer::getMemBufferCopy(StringRef(pBinary, uiBinarySize));
               
        std::auto_ptr<Module> M(ParseIR(pBinBuff, Err, Context));
        if (M.get() == 0) 
        {
            ErrorMessage = Err.getMessage();
            if ( !ErrorMessage.empty() )
	        {
                m_pLogString = new char[ErrorMessage.length() + 1];
		        if ( m_pLogString != NULL )
		        {
                    MEMCPY_S(m_pLogString, ErrorMessage.length(), ErrorMessage.c_str(), ErrorMessage.length());
			        m_pLogString[ErrorMessage.length()] = '\0';
		        }
	        }

            m_stOutIRSize = 0;
            m_pOutIR = NULL;

            return CL_LINK_PROGRAM_FAILURE;
        }

        if( Linker::LinkModules(composite.get(), M.get(), Linker::PreserveSource, &ErrorMessage))
        {
            // apparently LinkModules returns true on failure and false on success
            if ( !ErrorMessage.empty() )
	        {
                m_pLogString = new char[ErrorMessage.length() + 1];
		        if ( m_pLogString != NULL )
		        {
                    MEMCPY_S(m_pLogString, ErrorMessage.length(), ErrorMessage.c_str(), ErrorMessage.length());
			        m_pLogString[ErrorMessage.length()] = '\0';
		        }
	        }

            m_stOutIRSize = 0;
            m_pOutIR = NULL;

            return CL_LINK_PROGRAM_FAILURE;
        }
    }


    std::vector<unsigned char> Buffer;
    BitstreamWriter Stream(Buffer);

    Buffer.reserve(256*1024);

    WriteBitcodeToStream( composite.get(), Stream );

    m_stOutIRSize = sizeof(cl_prog_container_header) + 
                    sizeof(cl_llvm_prog_header) + 
                    Buffer.size();

    m_pOutIR = new char[m_stOutIRSize];
    if ( NULL == m_pOutIR )
    {
	    LOG_ERROR(TEXT("%s"), TEXT("Failed to allocate memory for buffer"));
        m_stOutIRSize = 0;
	    return CL_OUT_OF_HOST_MEMORY;
    }

    pHeader = (cl_prog_container_header*) m_pOutIR;

    pProgHeader = (cl_llvm_prog_header*)((char*)pHeader + sizeof(cl_prog_container_header));

    pBinary = (char*)pProgHeader + sizeof(cl_llvm_prog_header);

    MEMCPY_S(pBinary, Buffer.size(), &Buffer.front(), Buffer.size());

    memcpy(pHeader->mask, _CL_CONTAINER_MASK_, 4);
	pHeader->container_size = Buffer.size() + sizeof(cl_llvm_prog_header);
	pHeader->container_type = CL_PROG_CNT_PRIVATE;
	pHeader->description.bin_type = CL_PROG_BIN_LINKED_LLVM;
	pHeader->description.bin_ver_major = 1;
	pHeader->description.bin_ver_minor = 1;

    pProgHeader->bDebugInfo = bDebugInfoFlag;
    pProgHeader->bDenormsAreZero = bDenormsAreZeroFlag;
    pProgHeader->bDisableOpt = bDisableOptFlag;
    pProgHeader->bFastRelaxedMath = bFastRelaxedMathFlag;
    pProgHeader->bEnableLinkOptions = bEnableLinkOptionsFlag;

    return CL_SUCCESS;
}

void ClangFECompilerLinkTask::ParseOptions(const char *buildOpts)
{
    // Reset options
	bCreateLibrary = false;
    bEnableLinkOptions = false;

    bDenormsAreZero = false;
    bNoSignedZeroes = false;
    bUnsafeMath = false;
    bFiniteMath = false;
    bFastRelaxedMath = false;
	
    if (!buildOpts)
        buildOpts = "";

    // Parse the build options - handle the ones we understand and ignore the rest.
    //
    vector<string> opts = quoted_tokenize(buildOpts, " \t", '"', '\x00');
    vector<string>::const_iterator opt_i = opts.begin();

    while (opt_i != opts.end()) {
        if (opt_i->find("-D") == 0 || opt_i->find("-I") == 0) 
        {
            if (opt_i->length() == 2) 
            {
                // Definition is separated from the flag, so, we need to discard
                // the next token
                //
                if (++opt_i == opts.end()) 
                {
                    continue;
                }
            }
        }
        else if (*opt_i == "-s") 
        {
            // Expect the file name as the next token and discard it
            //
            if (++opt_i == opts.end()) 
            {
                continue;
            }
        }
        else if (*opt_i == "-cl-denorms-are-zero") 
        {
            bDenormsAreZero = true;
        }
        else if (*opt_i == "-cl-no-signed-zeroes") 
        {
            bNoSignedZeroes = true;
        }
        else if (*opt_i == "-cl-unsafe-math-optimizations") 
        {
            bUnsafeMath = true;
        }
        else if (*opt_i == "-cl-finite-math-only") 
        {
            bFiniteMath = true;
        }
        else if (*opt_i == "-cl-fast-relaxed-math") 
        {
            bFastRelaxedMath = true;
        }
        else if (*opt_i == "-create-library") 
        {
            bCreateLibrary = true;
        }
        else if (*opt_i == "-enable-link-options") 
        {
            bEnableLinkOptions = true;
        }

        ++opt_i;
    }
}


void ClangFECompilerLinkTask::ResolveFlags()
{
    bDebugInfoFlag = false;
    bDenormsAreZeroFlag = false;
    bDisableOptFlag = false;
    bFastRelaxedMathFlag = false;
    bEnableLinkOptionsFlag = false;

    cl_uint uiNumBinaries = m_pProgDesc->uiNumBinaries;
    const char** ppBinaries = (const char**)m_pProgDesc->pBinaryContainers;
    cl_llvm_prog_header* pProgHeader = NULL;

    if (bCreateLibrary && bEnableLinkOptions)
    {
        bEnableLinkOptionsFlag = true;
    }

    // Check denorms are zero
    bDenormsAreZeroFlag = true;

    for (cl_uint i = 0; i < uiNumBinaries; ++i)
    {
        pProgHeader = (cl_llvm_prog_header*)(ppBinaries[i] + sizeof(cl_prog_container_header));
        if (pProgHeader->bDenormsAreZero)
        {
            // If the flag is true we can move on to the next one
            continue;
        }

        // else, see if it can be override
        if ( pProgHeader->bEnableLinkOptions && bDenormsAreZero )
        {
            continue;
        }

        // else, don't use the flag
        bDenormsAreZeroFlag = false;
        break;
    }

    
    // Check fase relaxed math
    bFastRelaxedMathFlag = true;

    for (cl_uint i = 0; i < uiNumBinaries; ++i)
    {
        pProgHeader = (cl_llvm_prog_header*)(ppBinaries[i] + sizeof(cl_prog_container_header));
        if (pProgHeader->bFastRelaxedMath)
        {
            // If the flag is true we can move on to the next one
            continue;
        }

        // else, see if it can be override
        if ( pProgHeader->bEnableLinkOptions && bFastRelaxedMath )
        {
            continue;
        }

        // else, don't use the flag
        bFastRelaxedMathFlag = false;
        break;
    }

    
    // Check disable optimization
    bDisableOptFlag = false;

    for (cl_uint i = 0; i < uiNumBinaries; ++i)
    {
        pProgHeader = (cl_llvm_prog_header*)(ppBinaries[i] + sizeof(cl_prog_container_header));
        if (pProgHeader->bDisableOpt)
        {
            // If the flag is true we enable it for everyone
            bDisableOptFlag = true;
            break;
        }
    }
    

    // Check debug info
    bDebugInfoFlag = true;

    for (cl_uint i = 0; i < uiNumBinaries; ++i)
    {
        pProgHeader = (cl_llvm_prog_header*)(ppBinaries[i] + sizeof(cl_prog_container_header));
        if (pProgHeader->bDebugInfo)
        {
            // If the flag is true we can move on to the next one
            continue;
        }

        // else, don't use the flag
        bDebugInfoFlag = false;
        break;
    }
}
