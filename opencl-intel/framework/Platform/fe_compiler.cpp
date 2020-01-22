// INTEL CONFIDENTIAL
//
// Copyright 2006-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "fe_compiler.h"
#include "observer.h"
#include "cl_sys_info.h"
#include "common_clang.h"

#include <cl_sys_defines.h>
#include <task_executor.h>

#ifdef _WIN32
#include <Windows.h>
#else // _WIN32
#include <malloc.h>
#endif// _WIN32

using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::FECompilerAPI;
using namespace Intel::OpenCL::TaskExecutor;
using namespace Intel::OpenCL::ClangFE;

#ifdef _WIN32
static std::string GetDriverStorePathToLibrary()
{
    std::string dllPath;
    dllPath.resize(MAX_PATH);
#ifdef _M_X64
    LPCTSTR crt_name = "IntelOpenCL64.dll";
#else //(_M_X64)
    LPCTSTR crt_name = "IntelOpenCL32.dll";
#endif //(_M_X64)
    DWORD Length = GetModuleFileNameA( GetModuleHandle(crt_name), &dllPath[0], MAX_PATH );

    for( DWORD l = Length; l > 0; l-- )
    {
        if( dllPath[ l - 1 ] == '\\' )
        {
            dllPath[ l ] = '\0';
            break;
        }
    }

    return dllPath;
}
#endif // _WIN32

FrontEndCompiler::FrontEndCompiler() :
        OCLObject<_cl_object>(nullptr, "FrontEndCompiler"),
        m_pfnCreateInstance(nullptr),
        m_pszModuleName(nullptr),
        m_pFECompiler(nullptr),
        m_pLoggerClient(nullptr)
{
}

FrontEndCompiler::~FrontEndCompiler()
{
    FreeResources();
}

cl_err_code FrontEndCompiler::Initialize(const char * psModuleName, const void *pDeviceInfo, size_t stDevInfoSize)
{
    FreeResources();

    INIT_LOGGER_CLIENT(TEXT("FrontEndCompiler"), LL_DEBUG);

    if ( !m_dlModule.Load(GetFullModuleNameForLoad(psModuleName)) )
    {
#ifdef _WIN32
        const char* path = (GetDriverStorePathToLibrary() + psModuleName).c_str();
        if ( !m_dlModule.Load(GetFullModuleNameForLoad(path)) )
        {
            LOG_ERROR(TEXT("Can't find frontend library neither %s nor %s)"), psModuleName, path);
            return CL_COMPILER_NOT_AVAILABLE;
        }
#else
        LOG_ERROR(TEXT("Can't find frontend library %s)"), psModuleName);
        return CL_COMPILER_NOT_AVAILABLE;
#endif
    }

    m_pfnCreateInstance = (fnCreateFECompilerInstance*)m_dlModule.GetFunctionPtrByName("CreateFrontEndInstance");
    if ( nullptr == m_pfnCreateInstance )
    {
        LOG_ERROR(TEXT("%s"), TEXT("Can't find entry function"));
        return CL_COMPILER_NOT_AVAILABLE;
    }

    m_pszModuleName = STRDUP(psModuleName);

    cl_err_code err = m_pfnCreateInstance(pDeviceInfo, stDevInfoSize, &m_pFECompiler, g_pUserLogger);
    if ( CL_FAILED(err) )
    {
        LOG_ERROR(TEXT("FECompiler::CreateInstance() failed with %x"), err);
    }

    return err;
}

void FrontEndCompiler::FreeResources()
{
    RELEASE_LOGGER_CLIENT;

    if ( nullptr != m_pFECompiler )
    {
        if (!m_bTerminate)
        {
            m_pFECompiler->Release();
        }
        m_pFECompiler = nullptr;
    }

    if ( nullptr != m_pszModuleName )
    {
        free((void*)m_pszModuleName);
        m_pszModuleName = nullptr;
        m_dlModule.Close();
        m_pfnCreateInstance = nullptr;
    }
}

cl_err_code FrontEndCompiler::ProcessResults(cl_err_code Error,
                                             IOCLFEBinaryResult *Result,
                                             char **Binary, size_t *BinarySize,
                                             char **CompileLog) const {
  if (CL_OUT_OF_HOST_MEMORY == Error) {
    LOG_ERROR(TEXT("Front-End compilation failed = %x"), Error);
    if (Result)
      Result->Release();

    return CL_OUT_OF_HOST_MEMORY;
  }

  try {
    if (const char *ErrLog = Result->GetErrorLog()) {
      *CompileLog = new char[strlen(ErrLog) + 1];
      MEMCPY_S(*CompileLog, strlen(ErrLog) + 1, ErrLog, strlen(ErrLog) + 1);
    }

    *BinarySize = Result->GetIRSize();

    if (*BinarySize) {
      assert(Result->GetIR() && "LLVM IR is expected");
      *Binary = new char[*BinarySize];
      MEMCPY_S(*Binary, *BinarySize, Result->GetIR(), *BinarySize);
    }
  } catch (std::bad_alloc& e) {
    Result->Release();
    return CL_OUT_OF_HOST_MEMORY;
  }

  Result->Release();
  return CL_SUCCESS;
}

cl_err_code FrontEndCompiler::ParseSpirv(const char*    szProgramBinary,
                                         unsigned int   uiProgramBinarySize,
                                         const char*    szOptions,
                                         OUT char**     ppBinary,
                                         OUT size_t*    puiBinarySize,
                                         OUT char**     pszCompileLog) const
{
    LOG_DEBUG(TEXT("Enter ParseSpirv(szProgramBinary=%d, uiProgramBinarySize=%d, szOptions=%d, ppBinary=%d, puiBinarySize=%d, pszCompileLog=%d)"),
        szProgramBinary, uiProgramBinarySize, szOptions, ppBinary, puiBinarySize, pszCompileLog);

    IOCLFEBinaryResult* pResult;
    int err = CL_SUCCESS;

    assert(sizeof(_CL_SPIRV_MAGIC_NUMBER_) < uiProgramBinarySize &&
           _CL_SPIRV_MAGIC_NUMBER_ == ((unsigned int*)szProgramBinary)[0] && "The binary is not SPIRV program.");

    FESPIRVProgramDescriptor spirvDesc;

    spirvDesc.pSPIRVContainer = szProgramBinary;
    spirvDesc.uiSPIRVContainerSize = uiProgramBinarySize;
    spirvDesc.pszOptions = szOptions;

    err = m_pFECompiler->ParseSPIRV(&spirvDesc, &pResult);

    return ProcessResults(err, pResult, ppBinary, puiBinarySize, pszCompileLog);
}

cl_err_code FrontEndCompiler::MaterializeSPIR(const char *szProgramBinary,
                                              unsigned int uiProgramBinarySize,
                                              OUT char **ppBinary,
                                              OUT size_t *puiBinarySize,
                                              OUT char **pszCompileLog) const {
  LOG_DEBUG(
      TEXT("Enter MaterializeSPIR(szProgramBinary=%d, uiProgramBinarySize=%d, "
           "ppBinary=%d, puiBinarySize=%d, pszCompileLog=%d)"),
      szProgramBinary, uiProgramBinarySize, ppBinary, puiBinarySize,
      pszCompileLog);

  IOCLFEBinaryResult *pResult;
  int err = CL_SUCCESS;

  FESPIRProgramDescriptor spirvDesc;

  spirvDesc.pSPIRContainer = szProgramBinary;
  spirvDesc.uiSPIRContainerSize = uiProgramBinarySize;

  err = m_pFECompiler->MaterializeSPIR(&spirvDesc, &pResult);

  return ProcessResults(err, pResult, ppBinary, puiBinarySize, pszCompileLog);
}

cl_err_code FrontEndCompiler::CompileProgram(const char*    szProgramSource,
                                             unsigned int   uiNumInputHeaders,
                                             const char**   pszInputHeaders,
                                             const char**   pszInputHeadersNames,
                                             const char*    szOptions,
                                             bool           bFpgaEmulator,
                                             bool           bEyeQEmulator,
                                             OUT char**     ppBinary,
                                             OUT size_t*    puiBinarySize,
                                             OUT char**     pszCompileLog) const
{
    LOG_DEBUG(TEXT("Enter CompileProgram(szProgramSource=%d, uiNumInputHeaders=%d, pszInputHeaders=%d, pszInputHeadersNames=%d, szOptions=%d, ppBinary=%d, puiBinarySize=%d, pszCompileLog=%d)"),
        szProgramSource, uiNumInputHeaders, pszInputHeaders, pszInputHeadersNames, szOptions, ppBinary, puiBinarySize, pszCompileLog);

    IOCLFEBinaryResult*         pResult;
    FECompileProgramDescriptor  compileDesc;

    compileDesc.pProgramSource = szProgramSource;
    compileDesc.uiNumInputHeaders = uiNumInputHeaders;
    compileDesc.pInputHeaders = pszInputHeaders;
    compileDesc.pszInputHeadersNames = pszInputHeadersNames;
    compileDesc.pszOptions = szOptions;
    compileDesc.bFpgaEmulator = bFpgaEmulator;
    compileDesc.bEyeQEmulator = bEyeQEmulator;

    int err = m_pFECompiler->CompileProgram(&compileDesc, &pResult);

    return ProcessResults(err, pResult, ppBinary, puiBinarySize, pszCompileLog);
}

cl_err_code FrontEndCompiler::LinkProgram(
    const void **ppBinaries, unsigned int uiNumInputBinaries,
    const size_t *puiBinariesSizes, const char *szOptions, OUT char **ppBinary,
    OUT size_t *puiBinarySize, OUT std::vector<char> &linkLog,
    OUT bool *pbIsLibrary) const {
  LOG_DEBUG(
      TEXT("Enter CompileProgram(ppBinaries=%d, uiNumInputBinaries=%d, "
           "puiBinariesSizes=%d, szOptions=%d, ppBinary=%d, puiBinarySize=%d)"),
      ppBinaries, uiNumInputBinaries, puiBinariesSizes, szOptions, ppBinary,
      puiBinarySize);

  IOCLFEBinaryResult *Result;
  FELinkProgramsDescriptor linkDesc;

  linkDesc.pBinaryContainers = ppBinaries;
  linkDesc.uiNumBinaries = uiNumInputBinaries;
  linkDesc.puiBinariesSizes = puiBinariesSizes;
  linkDesc.pszOptions = szOptions;

  int Error = m_pFECompiler->LinkPrograms(&linkDesc, &Result);

  if (CL_OUT_OF_HOST_MEMORY == Error) {
    LOG_ERROR(TEXT("Front-End compilation failed = %x"), Error);
    if (Result)
      Result->Release();

    return CL_OUT_OF_HOST_MEMORY;
  }

  try {
    if (const char *ErrLog = Result->GetErrorLog()) {
      linkLog.resize(strlen(ErrLog) + 1);
      MEMCPY_S(&linkLog[0], strlen(ErrLog) + 1, ErrLog, strlen(ErrLog) + 1);
    }

    *puiBinarySize = Result->GetIRSize();

    if (*puiBinarySize) {
      assert(Result->GetIR() && "LLVM IR is expected");
      *ppBinary = new char[*puiBinarySize];
      MEMCPY_S(*ppBinary, *puiBinarySize, Result->GetIR(), *puiBinarySize);
    }
  } catch (std::bad_alloc& e) {
    Result->Release();
    return CL_OUT_OF_HOST_MEMORY;
  }

  if (pbIsLibrary) {
    *pbIsLibrary =
        Result->GetIRType() == Intel::OpenCL::ClangFE::IR_TYPE_LIBRARY;
  }

  Result->Release();

  return CL_SUCCESS;
}

bool FrontEndCompiler::CheckCompileOptions(const char* szOptions, char* szUnrecognizedOptions, size_t uiUnrecognizedOptionsSize) const
{
    return m_pFECompiler->CheckCompileOptions(szOptions, szUnrecognizedOptions, uiUnrecognizedOptionsSize);
}

bool FrontEndCompiler::CheckLinkOptions(const char* szOptions, char* szUnrecognizedOptions, size_t uiUnrecongnizedOptionsSize) const
{
  return m_pFECompiler->CheckLinkOptions(szOptions, szUnrecognizedOptions, uiUnrecongnizedOptionsSize);
}

cl_err_code FrontEndCompiler::GetKernelArgInfo(const void*        pBin,
                                               size_t             uiBinarySize,
                                               const char*        szKernelName,
                                               IOCLFEKernelArgInfo*   *ppArgInfo) const
{
    LOG_DEBUG(TEXT("Enter GetKernelArgInfo(pBin=%p, szKernelName=<%s>, ppArgInfo=%p)"), (void*)pBin, szKernelName, (void*)ppArgInfo);

    if ( (nullptr == pBin) || (nullptr == szKernelName) || (nullptr==ppArgInfo) )
    {
        return CL_INVALID_VALUE;
    }

    int err = m_pFECompiler->GetKernelArgInfo(pBin, uiBinarySize, szKernelName, ppArgInfo);

    LOG_DEBUG(TEXT("Exit GetKernelArgInfo(pBin=%p, szKernelName=<%s>, ppArgInfo=%p, err=%d)"), (void*)pBin, szKernelName, (void*)ppArgInfo, err);

    return err;
}

