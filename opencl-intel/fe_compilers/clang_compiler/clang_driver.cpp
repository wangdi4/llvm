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
#include "clang/Frontend/CodeGenAction.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/FrontendDiagnostic.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Frontend/TextDiagnosticBuffer.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "llvm/LLVMContext.h"
#include "llvm/ADT/OwningPtr.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/System/DynamicLibrary.h"
#include "llvm/Target/TargetSelect.h"
#include "llvm/System/Threading.h"


#include "clang_driver.h"
#include "clang_compiler.h"
#include "frontend_api.h"
#include "cl_sys_info.h"
#include "cl_cpu_detect.h"
#include "cl_types.h"
#include "logger.h"

using namespace Intel::OpenCL::ClangFE;
using namespace llvm;
using namespace clang;
using namespace clang::frontend;
using namespace Intel::OpenCL::Utils;


#include <string>
#include <list>

#pragma comment (lib, "cl_logger.lib")
#pragma comment (lib, "cl_sys_utils.lib")

#define MAX_STR_BUFF	1024
#define __DOUBLE_ENABLED__

// copied from cpu_device.cpp (Guy)
#ifdef __DOUBLE_ENABLED__
static const char OCL_SUPPORTED_EXTENSIONS[] = "cl_khr_fp64 cl_khr_global_int32_base_atomics "\
												"cl_khr_global_int32_extended_atomics cl_khr_local_int32_base_atomics "\
												"cl_khr_local_int32_extended_atomics cl_khr_gl_sharing cl_khr_byte_addressable_store "\
												"cl_intel_printf cl_intel_overloading";
#else
static const char OCL_SUPPORTED_EXTENSIONS[] = "cl_khr_global_int32_base_atomics "\
											   "cl_khr_global_int32_extended_atomics cl_khr_local_int32_base_atomics "\
											   "cl_khr_local_int32_extended_atomics cl_khr_gl_sharing cl_khr_byte_addressable_store "\
												"cl_intel_printf cl_intel_overloading";
#endif

// Declare logger client
DECLARE_LOGGER_CLIENT;

OclMutex CompileTask::s_serializingMutex;

void LLVMErrorHandler(void *UserData, const std::string &Message) {
  Diagnostic &Diags = *static_cast<Diagnostic*>(UserData);

  Diags.Report(diag::err_fe_error_backend) << Message;

  // We cannot recover from llvm errors.
  exit(1);
}

int Intel::OpenCL::ClangFE::InitClangDriver()
{
	INIT_LOGGER_CLIENT(L"ClangCompiler", LL_DEBUG);

	INIT_LOGGER_CLIENT(L"ClangCompiler", LL_DEBUG);
	
	LOG_INFO("Initialize ClangCompiler - start");

	llvm::InitializeAllTargets();
	llvm::InitializeAllAsmPrinters();

	LOG_INFO("Initialize ClangCompiler - Finish");
	return 0;
}

int Intel::OpenCL::ClangFE::CloseClangDriver()
{
	llvm::llvm_shutdown();

	LOG_INFO("Close ClangCompiler - done");

	RELEASE_LOGGER_CLIENT;

	return 0;
}

void CompileTask::PrepareArgumentList(ArgListType &list, ArgListType &ignored, const char *buildOpts)
{
	// Reset options
	OptDebugInfo = false;
	Opt_Disable = false;
	Denorms_Are_Zeros = false;
	Fast_Relaxed_Math = false;
	
	// Parse user build options
	// Check for known options, and pass the rest to clang
	if ( NULL != m_pTask->pszOptions)
	{
		char *sOptions = strdup(buildOpts);
		char *opt;
		char *cont;
		
		// Now tokenize the received string - it should be passed
		// as an array of char*
		opt = strtok_s( sOptions, " \t", &cont);

		while(opt)
		{
			if(!strncmp(opt, "-g", 2))
			{
				list.push_back("-g");
				OptDebugInfo = true;
			}
			else if(!strncmp(opt, "-w", 2))
			{
				list.push_back("-w");
			}
			else if(!strncmp(opt, "-D", 2) || !strncmp(opt, "-I", 2))
			{
				if(opt[2] == 0)
				{
					char* flag = opt;
					opt = strtok_s( NULL, " \t", &cont);
					if(!opt)
					{
						ignored.push_back(flag);
						continue;
					}
					list.push_back(flag);
				}
				list.push_back(opt);
			}
			else if(!strncmp(opt, "-Werror", 7))
			{
				list.push_back("-Werror");
			}
			else if(!strncmp(opt, "-cl-mad-enable", 14))
			{
			}
			else if(!strncmp(opt, "-dump-opt-llvm=", 15))
			{
				// this option is parsed by the backend - disregard it
			}
			else if(!strncmp(opt, "-cl-opt-disable", 15))
			{
				Opt_Disable = true;
			}
			else if(!strncmp(opt, "-cl-strict-aliasing", 19))
			{
			}
			else if(!strncmp(opt, "-cl-no-signed-zeros", 19))
			{
			}
			else if(!strncmp(opt, "-cl-denorms-are-zero", 20))
			{
				Denorms_Are_Zeros = true;
			}
			else if(!strncmp(opt, "-cl-finite-math-only", 20))
			{
				list.push_back("-D");
				list.push_back("__FINITE_MATH_ONLY__=1");
			}
			else if(!strncmp(opt, "-cl-fast-relaxed-math", 21))
			{
				list.push_back("-D");
				list.push_back("__FAST_RELAXED_MATH__=1");
				Fast_Relaxed_Math = true;
			}
			else if(!strncmp(opt, "-cl-unsafe-math-optimizations", 29))
			{
			}
			else if(!strncmp(opt, "-cl-single-precision-constant", 29))
			{
			}
			else
			{
				// command line option ignored!
				ignored.push_back(opt);
			}

			opt = strtok_s( NULL, " \t", &cont);
		}

		free(sOptions);
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
	char	szOclIncPath[MAX_STR_BUFF];
	char	szOclPchPath[MAX_STR_BUFF];
	char	szCurrDirrPath[MAX_STR_BUFF];

	// Retrieve local relatively to binary directory
	GetModuleDirectory(szBinaryPath, MAX_STR_BUFF);
#ifndef WIN32
	sprintf_s(szOclIncPath, MAX_STR_BUFF, "%sfe_include", szBinaryPath);
	sprintf_s(szOclPchPath, MAX_STR_BUFF, "%sopencl_.pch", szBinaryPath);

	list.push_back("-I");
	list.push_back(szOclIncPath);

	list.push_back("-isysroot");
	list.push_back(szBinaryPath);

	list.push_back("-include-pch");
	list.push_back(szOclPchPath);
#endif

	// Add current directory
	GetCurrentDirectoryA(MAX_STR_BUFF, szCurrDirrPath);
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
	std::string extStr = OCL_SUPPORTED_EXTENSIONS;
	while(extStr != "")
	{
		list.push_back("-D");
		std::string::size_type pos = extStr.find(" ", 0);
		if(pos == string::npos)
		{
			list.push_back(extStr);
			extStr.clear();
		}
		else
		{
			list.push_back(extStr.substr(0, pos));
			extStr = extStr.substr(pos + 1);
		}	
	}

	// Don't optimize in the frontend
	list.push_back("-O0");
}

void CompileTask::Execute()
{
	LOG_INFO("CompileTask::Execute() - Started");

	CompilerInstance Clang;

	OclAutoMutex CS(&s_serializingMutex);

	Clang.setLLVMContext(new llvm::LLVMContext);

	TextDiagnosticBuffer DiagsBuffer;
	Diagnostic Diags(&DiagsBuffer);

	ArgListType ArgList;
	ArgListType IgnoredArgs;

	PrepareArgumentList(ArgList, IgnoredArgs, m_pTask->pszOptions);

	const char **argArray = new const char *[ArgList.size()];
	ArgListType::iterator iter = ArgList.begin();

	for(int i=0; i<ArgList.size(); i++)
	{
		argArray[i] = iter->c_str();
		iter++;
	}

	CompilerInvocation::CreateFromArgs(Clang.getInvocation(), argArray, argArray + ArgList.size(),
                                     Diags);

	SmallVector<char, 4096>	Log;
	llvm::raw_svector_ostream errStream(Log);

	while(!IgnoredArgs.empty())
	{
		errStream << "warning: ignoring build option: \"";
		errStream << IgnoredArgs.front();
		errStream << "\"\n";
		IgnoredArgs.pop_front();
	}

	Clang.SetErrorStream(&errStream);

	Clang.createDiagnostics(ArgList.size(), const_cast<char**>(argArray));
	if (!Clang.hasDiagnostics())
	{
		LOG_ERROR("CompileTask::Execute() - Failed to create diagnostics");
		m_pTask->pCallBack(m_pTask->pData, NULL, 0, CL_OUT_OF_HOST_MEMORY, NULL);
		return;
	}

	// don't write anything on the screen
	Clang.getDiagnosticOpts().ShowCarets = 0;

	//if(!llvm::llvm_is_multithreaded())
	//{
		// Set an error handler, so that any LLVM backend diagnostics go through our
		// error handler.
		// TODO: this solution is wrong. Need to redesign it. (Guy)
		llvm::llvm_remove_error_handler();
		llvm::llvm_install_error_handler(LLVMErrorHandler,
			static_cast<void*>(&Clang.getDiagnostics()));

		//llvm::llvm_start_multithreaded();
	//}

	DiagsBuffer.FlushDiagnostics(Clang.getDiagnostics());

	size_t stTotalSize = 0;
	for(int i=0; i<m_pTask->uiLineCount; ++i)
	{
		stTotalSize += m_pTask->pLengths[i];
	}

	// Prepare input buffer
	llvm::MemoryBuffer *SB = llvm::MemoryBuffer::getNewUninitMemBuffer(stTotalSize);
	if ( NULL == SB )
	{
		LOG_ERROR("CompileTask::Execute() - Failed to create buffer");
		m_pTask->pCallBack(m_pTask->pData, NULL, 0, CL_OUT_OF_HOST_MEMORY, NULL);
		return;
	}
	// Copy sources to the new buffer
	char*	pBegin = (char*)SB->getBufferStart();
	for(int i=0; i<m_pTask->uiLineCount; ++i)
	{
		memcpy_s(pBegin, stTotalSize, m_pTask->ppsLineArray[i], m_pTask->pLengths[i]);
		stTotalSize -= m_pTask->pLengths[i];
		pBegin += m_pTask->pLengths[i];
	}

	Clang.SetInputBuffer(SB);

	//prepare output buffer
	SmallVector<char, 4096>	IRbinary;
	Clang.SetOutputStream(new llvm::raw_svector_ostream(IRbinary));

#ifdef WIN32
	//prepare pch buffer
	HMODULE hMod = NULL;
	HRSRC hRes = NULL;
	HGLOBAL hBytes = NULL;
	char *pData = NULL;
	size_t dResSize = NULL;
	llvm::MemoryBuffer* pchBuff = NULL;

	// Get the handle to the current module
	GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | 
					  GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, 
					  (LPCSTR)"clang_compiler",
					  &hMod);

	// Locate the resource
	if( NULL != hMod )
	{
		hRes = FindResource(hMod, "#101", "PCH");
	}

	// Load the resource
	if( NULL != hRes )
	{
		hBytes = LoadResource(hMod, hRes);
	}
	
	// Get the base address to the resource. This call doesn't really lock it
	if( NULL != hBytes )
	{
		pData = (char *)LockResource(hBytes);
	}
	
	// Get the buffer size
	if( NULL != pData )
	{
		dResSize = SizeofResource(hMod, hRes);
	}

	if( dResSize > 0 )
	{	
		pchBuff = llvm::MemoryBuffer::getMemBufferCopy(pData, pData + dResSize);
	}

	if( NULL != pchBuff )
	{
		Clang.SetPchBuffer(pchBuff);
	}
#endif

	// If there were errors in processing arguments, don't do anything else.
	bool Success = false;
	if (!Clang.getDiagnostics().getNumErrors()) {
		// Create and execute the frontend action.
		llvm::OwningPtr<FrontendAction> Act(new EmitBCAction());
		if (Act)
			Success = Clang.ExecuteAction(*Act);
	}

	// Create output buffer
	char*	pOutBuff = NULL;
	// Create Log buffer
	char*	pLogBuff = NULL;

	errStream.flush();
	if ( !Log.empty() )
	{
		pLogBuff = new char[Log.size()+1];
		if ( pLogBuff != NULL )
		{
			memcpy_s(pLogBuff, Log.size(), Log.begin(), Log.size());
			pLogBuff[Log.size()] = '\0';
		}
	}

	size_t	stTotSize = 0;
	if ( !IRbinary.empty() )
	{
		stTotSize = IRbinary.size()+
			sizeof(cl_prog_container_header)+sizeof(cl_llvm_prog_header);
		pOutBuff = new char[stTotSize];
		if ( NULL == pOutBuff )
		{
			LOG_ERROR("CompileTask::Execute() - Failed to allocate memory for buffer");
			m_pTask->pCallBack(m_pTask->pData, NULL, 0, CL_OUT_OF_HOST_MEMORY, NULL);
			return;
		}
	}

	if ( NULL != pOutBuff )
	{
		cl_prog_container_header*	pHeader = (cl_prog_container_header*)pOutBuff;
		memcpy(pHeader->mask, _CL_CONTAINER_MASK_, 4);
		pHeader->container_size = IRbinary.size()+sizeof(cl_llvm_prog_header);
		pHeader->container_type = CL_PROG_CNT_PRIVATE;
		pHeader->description.bin_type = CL_PROG_BIN_LLVM;
		pHeader->description.bin_ver_major = 1;
		pHeader->description.bin_ver_minor = 1;
		// Fill options
		cl_llvm_prog_header *pProgHeader = (cl_llvm_prog_header*)(pOutBuff+sizeof(cl_prog_container_header));
		pProgHeader->bDebugInfo = OptDebugInfo;
		pProgHeader->bDisableOpt = Opt_Disable;
		pProgHeader->bDemorsAreZero = Denorms_Are_Zeros;
		pProgHeader->bFastRelaxedMath = Fast_Relaxed_Math;
		void *pIR = (void*)(pProgHeader+1);
		// Copy IR
		memcpy_s(pIR, IRbinary.size(), IRbinary.begin(), IRbinary.size());
	}

	LOG_INFO("CompileTask::Execute() - Finished");
	m_pTask->pCallBack(m_pTask->pData, pOutBuff, stTotSize, CL_SUCCESS, pLogBuff);
	// Release log and binary
	if ( NULL != pLogBuff )
	{
		delete []pLogBuff;
	}
	delete []pOutBuff;
#ifdef WIN32
	if( NULL != pchBuff )
	{
		delete pchBuff;
	}
#endif
	IRbinary.clear();

	return;
}
