// clang_compiler.cpp : Defines the exported functions for the DLL application.
//
#include "stdafx.h"
#include "clang_compiler.h"
#include "clang_driver_cc.h"
#include "translation_controller.h"

#include <Logger.h>
#include <cl_sys_defines.h>
#include <cl_device_api.h>
#include <cl_shutdown.h>

#include "llvm/Support/Threading.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/ManagedStatic.h"

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
#else
#define DLL_EXPORT
#endif

using namespace Intel::OpenCL::FECompilerAPI;

extern DECLARE_LOGGER_CLIENT;

namespace TC
{
    CTranslationController* g_pTranslationController = NULL;
}

namespace llvm
{
    extern bool DisablePrettyStackTrace;
}

void ClangCompilerInitialize()
{
    TC::g_pTranslationController = TC::CTranslationController::Create();
    llvm::llvm_start_multithreaded();
    llvm::InitializeAllTargets();
    llvm::InitializeAllAsmPrinters();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllTargetMCs();
}

void ClangCompilerTerminate()
{
    llvm::llvm_shutdown();
}

// ClangFECompiler class implementation
ClangFECompiler::ClangFECompiler(const void* pszDeviceInfo)
{
    CLANG_DEV_INFO *pDevInfo = (CLANG_DEV_INFO *)pszDeviceInfo;

    memset(&m_sDeviceInfo, 0, sizeof(CLANG_DEV_INFO));

    m_sDeviceInfo.sExtensionStrings = STRDUP(pDevInfo->sExtensionStrings);
    m_sDeviceInfo.bImageSupport     = pDevInfo->bImageSupport;
    m_sDeviceInfo.bDoubleSupport    = pDevInfo->bDoubleSupport;
    m_sDeviceInfo.bEnableSourceLevelProfiling = pDevInfo->bEnableSourceLevelProfiling;
    m_config.Initialize(GetConfigFilePath());

    llvm::DisablePrettyStackTrace = m_config.DisableStackDump();
}

ClangFECompiler::~ClangFECompiler()
{
    if ( NULL != m_sDeviceInfo.sExtensionStrings )
    {
        free((void *)m_sDeviceInfo.sExtensionStrings);
    }

    if( TC::g_pTranslationController )
    {
        TC::CTranslationController::Delete( TC::g_pTranslationController );
    }
}

int ClangFECompiler::CompileProgram(FECompileProgramDescriptor* pProgDesc, IOCLFEBinaryResult* *pBinaryResult)
{
    assert(NULL != pProgDesc);
    assert(NULL != pBinaryResult);

    std::auto_ptr<ClangFECompilerCompileTask> pTask( new ClangFECompilerCompileTask(pProgDesc, m_sDeviceInfo, m_config) );

    int res = pTask->Compile();

    if(pBinaryResult)
        *pBinaryResult = pTask.release();

    return res;
}

int ClangFECompiler::LinkPrograms(Intel::OpenCL::FECompilerAPI::FELinkProgramsDescriptor* pProgDesc,
                         IOCLFEBinaryResult* *pBinaryResult)
{
    assert(NULL != pProgDesc);
    assert(NULL != pBinaryResult);

    return ClangFECompilerLinkTask(pProgDesc).Link(pBinaryResult);
}

int ClangFECompiler::GetKernelArgInfo(const void*             pBin,
                                      size_t                  uiBinarySize,
                                      const char*             szKernelName,
                                      IOCLFEKernelArgInfo*  *pArgInfo)
{
    assert(NULL!=pBin);
    assert(NULL!=szKernelName);
    assert(NULL!=pArgInfo);

    return ClangFECompilerGetKernelArgInfoTask().GetKernelArgInfo(pBin, uiBinarySize, szKernelName, pArgInfo);
}

bool ClangFECompiler::CheckCompileOptions(const char* szOptions, char* szUnrecognizedOptions, size_t uiUnrecognizedOptionsSize)
{
    return ClangFECompilerCheckCompileOptions(szOptions, szUnrecognizedOptions, uiUnrecognizedOptionsSize, m_config);
}

bool ClangFECompiler::CheckLinkOptions(const char* szOptions, char* szUnrecognizedOptions, size_t uiUnrecognizedOptionsSize)
{
    return ClangFECompilerCheckLinkOptions(szOptions, szUnrecognizedOptions, uiUnrecognizedOptionsSize);
}

namespace Intel { namespace OpenCL { namespace Utils {
FrameworkUserLogger* g_pUserLogger = NULL;
}}}

extern "C" DLL_EXPORT int CreateFrontEndInstance(const void* pDeviceInfo, size_t devInfoSize, IOCLFECompiler* *pFECompiler)
{
    assert(NULL != pFECompiler);
    assert(devInfoSize == sizeof(CLANG_DEV_INFO));

    try
    {
        *pFECompiler = new ClangFECompiler(pDeviceInfo);
        return CL_SUCCESS;
    }
    catch( std::bad_alloc& )
    {
        LOG_ERROR(TEXT("%S"), TEXT("Can't allocate compiler instance"));
        return CL_OUT_OF_HOST_MEMORY;
    }
}