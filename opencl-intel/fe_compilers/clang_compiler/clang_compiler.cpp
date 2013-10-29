// clang_compiler.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "clang_compiler.h"
#include "clang_driver.h"
#ifdef OCLFRONTEND_PLUGINS 
#include "compile_data.h"
#include "link_data.h"
#include "source_file.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/LLVMContext.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/IRReader.h"
#include "llvm/Module.h"
#include "llvm/Metadata.h"
#endif //OCLFRONTEND_PLUGINS

#include <Logger.h>
#include <cl_sys_defines.h>
#include <cl_device_api.h>
#include <cl_shutdown.h>

#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/ManagedStatic.h>

#if defined (_WIN32)
#include<windows.h>
#endif
#include <string.h>
#include <memory>
#include <sstream>
#include <ctime>

using namespace Intel::OpenCL::ClangFE;
using namespace Intel::OpenCL::Utils;

#if defined (_WIN32)
#define DLL_EXPORT _declspec(dllexport)
// LLVM libraries
#pragma comment (lib, "LLVMAnalysis.lib")
#pragma comment (lib, "LLVMBitReader.lib")
#pragma comment (lib, "LLVMBitWriter.lib")
#pragma comment (lib, "LLVMCodeGen.lib")
#pragma comment (lib, "LLVMSupport.lib")
#pragma comment (lib, "LLVMTarget.lib")
#pragma comment (lib, "LLVMTransformUtils.lib")
#pragma comment (lib, "LLVMCore.lib")
#pragma comment (lib, "LLVMX86Info.lib")
#pragma comment (lib, "LLVMX86CodeGen.lib")
#pragma comment (lib, "LLVMX86AsmParser.lib")
#pragma comment (lib, "LLVMX86AsmPrinter.lib")
#pragma comment (lib, "LLVMX86Disassembler.lib")
#pragma comment (lib, "LLVMMC.lib")
#pragma comment (lib, "LLVMSelectionDAG.lib")
#pragma comment (lib, "LLVMAsmPrinter.lib")
#pragma comment (lib, "LLVMScalarOpts.lib")
#pragma comment (lib, "LLVMipo.lib")
#pragma comment (lib, "LLVMipa.lib")
#pragma comment (lib, "LLVMInstCombine.lib")

// Clang libraries
#pragma comment (lib, "clangAST.lib")
#pragma comment (lib, "clangBasic.lib")
#pragma comment (lib, "clangCodeGen.lib")
#pragma comment (lib, "clangLex.lib")
#pragma comment (lib, "clangFrontend.lib")
#pragma comment (lib, "clangParse.lib")
#pragma comment (lib, "clangSema.lib")
#else
#define DLL_EXPORT
#endif

using namespace Intel::OpenCL::FECompilerAPI;

volatile bool ClangFECompiler::m_bLllvmActive = false;


extern DECLARE_LOGGER_CLIENT;

USE_SHUTDOWN_HANDLER(ClangFECompiler::ShutDown);

int InitClangDriver()
{
	INIT_LOGGER_CLIENT("ClangCompiler", LL_DEBUG);
	LOG_INFO(TEXT("%s"), TEXT("Initialize ClangCompiler - start"));

	llvm::InitializeAllTargets();
	llvm::InitializeAllAsmPrinters();
	llvm::InitializeAllAsmParsers();
  llvm::InitializeAllTargetMCs();

	LOG_INFO(TEXT("%s"), TEXT("Initialize ClangCompiler - Finish"));
	return 0;
}

int CloseClangDriver()
{
	llvm::llvm_shutdown();

	LOG_INFO(TEXT("%s"), TEXT("Close ClangCompiler - done"));

	RELEASE_LOGGER_CLIENT;

	return 0;
}

Intel::OpenCL::Utils::AtomicCounter Intel::OpenCL::ClangFE::ClangFECompiler::s_llvmReferenceCount(0);

// ClangFECompiler class implementation
ClangFECompiler::ClangFECompiler(const void* pszDeviceInfo)
{
     m_bLllvmActive = true;

	long prev = s_llvmReferenceCount++;
	if ( 0 == prev )
	{
		InitClangDriver();
	}

	CLANG_DEV_INFO *pDevInfo = (CLANG_DEV_INFO *)pszDeviceInfo;

	memset(&m_sDeviceInfo, 0, sizeof(CLANG_DEV_INFO));

	m_sDeviceInfo.sExtensionStrings = STRDUP(pDevInfo->sExtensionStrings);
	m_sDeviceInfo.bImageSupport     = pDevInfo->bImageSupport;
	m_sDeviceInfo.bDoubleSupport    = pDevInfo->bDoubleSupport;
	m_sDeviceInfo.bEnableSourceLevelProfiling = pDevInfo->bEnableSourceLevelProfiling;
    m_config.Initialize(GetConfigFilePath());
}

ClangFECompiler::~ClangFECompiler()
{
	if ( NULL != m_sDeviceInfo.sExtensionStrings )
	{
		free((void *)m_sDeviceInfo.sExtensionStrings);
	}
	long prev = s_llvmReferenceCount--;
	if ( 1 == prev )
	{
		CloseClangDriver();
        m_bLllvmActive = false;
	}
}

void ClangFECompiler::ShutDown()
{
    while (m_bLllvmActive)
    {
        hw_pause();
    }
}


#ifdef OCLFRONTEND_PLUGINS
//
//Creates a source file object from a given contents string, and a serial identifier.
//
static Intel::OpenCL::Frontend::SourceFile createSourceFile(
  const char* contents,
  const char* options,
  unsigned serial,
  IOCLFEBinaryResult* pBinary = NULL)
{
  //composing a file name based on the current time
  std::stringstream fileName;
  std::string strContents(contents);
  Intel::OpenCL::Frontend::BinaryBuffer buffer;
  if (pBinary){
   //the length (in bytes) of the bytecode headers. We leave that out of the
    //hashcode computation, since the header(s) might change during the
    //the compilation process.
    const size_t bufferLen = sizeof(_cl_prog_container_header) + sizeof(cl_llvm_prog_header);
    buffer.binary = (char*)pBinary->GetIR() + bufferLen;
    buffer.size   = pBinary->GetIRSize() - bufferLen;
    llvm::SMDiagnostic err;
    llvm::LLVMContext  ctxt;
    //retrieving the name of the first kernel
    llvm::StringRef inData((const char*)buffer.binary, buffer.size);
    llvm::MemoryBuffer* pBuffer = llvm::MemoryBuffer::getMemBuffer(inData, "", false);
    std::auto_ptr<llvm::Module> pModule(llvm::ParseIR(pBuffer, err, ctxt));
    assert(pModule.get() && "NULL module");
    llvm::NamedMDNode* metadata = pModule->getNamedMetadata("opencl.kernels");
    llvm::MDNode *elt = metadata->getOperand(0);
    llvm::Function* pKernel =
      llvm::dyn_cast<llvm::Function>(elt->getOperand(0)->stripPointerCasts());
    fileName <<  pKernel->getName().str();
  }
  fileName  << serial << ".cl";
  Intel::OpenCL::Frontend::SourceFile ret = Intel::OpenCL::Frontend::SourceFile(
    std::string(fileName.str()),
    std::string(strContents),
    std::string(options) );
  if (pBinary)
    ret.setBinaryBuffer(buffer); 
  return ret;
}
#endif //OCLFRONTEND_PLUGINS

int ClangFECompiler::CompileProgram(FECompileProgramDescriptor* pProgDesc, IOCLFEBinaryResult* *pBinaryResult)
{
	assert(NULL != pProgDesc);
	assert(NULL != pBinaryResult);
	// Create new compile task
	ClangFECompilerCompileTask* pCompileTask = new ClangFECompilerCompileTask(pProgDesc, m_sDeviceInfo, m_config);
	if ( NULL == pCompileTask )
	{
		*pBinaryResult = NULL;
		return CL_OUT_OF_HOST_MEMORY;
	}
  cl_err_code ret = pCompileTask->Compile();
	*pBinaryResult = pCompileTask;
#ifdef OCLFRONTEND_PLUGINS
  if (getenv("OCLBACKEND_PLUGINS") && NULL == getenv("OCL_DISABLE_SOURCE_RECORDER")){
    Intel::OpenCL::Frontend::CompileData compileData;
    Intel::OpenCL::Frontend::SourceFile sourceFile = createSourceFile(
      pProgDesc->pProgramSource,
      pProgDesc->pszOptions,
      0,
      *pBinaryResult
    );
    compileData.sourceFile(sourceFile);
    for (unsigned headerCount=0 ; headerCount < pProgDesc->uiNumInputHeaders ; headerCount++)
      compileData.addIncludeFile(createSourceFile(
        pProgDesc->pszInputHeadersNames[headerCount],
        "", //include files comes without compliation flags
        headerCount+1
      ));
    m_pluginManager.OnCompile(&compileData);
  }
#endif //OCLFRONTEND_PLUGINS
	return ret;
}

int ClangFECompiler::LinkPrograms(Intel::OpenCL::FECompilerAPI::FELinkProgramsDescriptor* pProgDesc, 
                         Intel::OpenCL::FECompilerAPI::IOCLFEBinaryResult* *pBinaryResult)
{
  assert(NULL != pProgDesc);
	assert(NULL != pBinaryResult);

	// Create new link task
	ClangFECompilerLinkTask* pLinkTask = new ClangFECompilerLinkTask(pProgDesc);
	if ( NULL == pLinkTask )
	{
		*pBinaryResult = NULL;
		return CL_OUT_OF_HOST_MEMORY;
	}

    cl_err_code ret = pLinkTask->Link();
	*pBinaryResult = pLinkTask;
	return ret;
}

int ClangFECompiler::GetKernelArgInfo(const void*             pBin, 
                                      const char*             szKernelName, 
                                      FEKernelArgInfo*  *pArgInfo)
{
    assert( (NULL!=pBin) && (NULL!=szKernelName) && (NULL!=pArgInfo) && "Invlaid arguments");

	// Create new GetKernelArgInfo task
	ClangFECompilerGetKernelArgInfoTask* pGetKernelArgInfoTask = new ClangFECompilerGetKernelArgInfoTask();
	if ( NULL == pGetKernelArgInfoTask )
	{
		*pArgInfo = NULL;
		return CL_OUT_OF_HOST_MEMORY;
	}

    cl_err_code ret = pGetKernelArgInfoTask->GetKernelArgInfo(pBin, szKernelName);
	*pArgInfo = pGetKernelArgInfoTask;
	return ret;
}

bool ClangFECompiler::CheckCompileOptions(const char* szOptions, char** szUnrecognizedOptions)
{
    return ClangFECompilerCheckCompileOptions(szOptions, szUnrecognizedOptions, m_config);
}

bool ClangFECompiler::CheckLinkOptions(const char* szOptions, char** szUnrecognizedOptions)
{
    return ClangFECompilerCheckLinkOptions(szOptions, szUnrecognizedOptions);
}

extern "C" DLL_EXPORT int CreateFrontEndInstance(const void* pDeviceInfo, size_t devInfoSize, IOCLFECompiler* *pFECompiler)
{
	assert(NULL != pFECompiler);
  assert(devInfoSize == sizeof(CLANG_DEV_INFO));

	IOCLFECompiler* pNewCompiler = new ClangFECompiler(pDeviceInfo);
	if ( NULL == pNewCompiler )
	{
		LOG_ERROR(TEXT("%S"), TEXT("Cann't allocate compiler instance"));
		return CL_OUT_OF_HOST_MEMORY;
	}
	
	*pFECompiler = pNewCompiler;
	return CL_SUCCESS;
}
