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

#include "mic_dev_limits.h"
#include "ElfWriter.h"
#include "os_inc.h"
#include "tc_common.h"
#include "translator.h"
#include "translation_controller.h"
#include "clang_driver_cc.h"
#include "cache_binary_handler.h"
#include "resource.h"
#include "options.h"

#include <Logger.h>
#include <cl_sys_info.h>
#include <cl_cpu_detect.h>
#include <cl_autoptr_ex.h>

#include <llvm/Support/MemoryBuffer.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/OwningPtr.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Constants.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/IR/Metadata.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/SourceMgr.h>

#include <string>
#include <list>
#include <vector>
#include <cctype>
#include <algorithm>

using namespace Intel::OpenCL::ELFUtils;
using namespace Intel::OpenCL::ClangFE;
using namespace Intel::OpenCL::Utils;
using namespace TC;
using namespace std;

typedef auto_ptr_ex<IOCLFEBinaryResult,ReleaseDP<IOCLFEBinaryResult> > IOCLFEBinaryResultPtr;

//===----------------------------------------------------------------------===//
// ElfWriterDP- ElfWriter delete policy for autoptr.
//
struct ElfWriterDP
{
    static void Delete(CLElfLib::CElfWriter* pElfWriter)
    {
        CLElfLib::CElfWriter::Delete(pElfWriter);
    }
};
typedef auto_ptr_ex<CLElfLib::CElfWriter, ElfWriterDP> ElfWriterPtr;

#ifdef OCLFRONTEND_PLUGINS
#include "compile_data.h"
#include "link_data.h"
#include "source_file.h"
#include "plugin_manager.h"
Intel::OpenCL::PluginManager g_pluginManager;

//
//Creates a source file object from a given contents string, and a serial identifier.
//
static Intel::OpenCL::Frontend::SourceFile createSourceFile(
  const char* contents,
  const char* options,
  unsigned serial,
  Intel::OpenCL::ClangFE::IOCLFEBinaryResult* pResult = NULL)
{
    //composing a file name based on the current time
    std::stringstream fileName;
    std::string strContents(contents);

    if (pResult)
    {
        fileName <<  pResult->GetIRName();
    }

    fileName  << serial << ".cl";
    Intel::OpenCL::Frontend::SourceFile ret = Intel::OpenCL::Frontend::SourceFile(
        std::string(fileName.str()),
        std::string(strContents),
        std::string(options) );

    if (pResult)
    {
        Intel::OpenCL::Frontend::BinaryBuffer buffer(pResult->GetIR(), pResult->GetIRSize());
        ret.setBinaryBuffer(buffer);
    }
    return ret;
}
#endif //OCLFRONTEND_PLUGINS

#if defined (_WIN32)
#define GET_CURR_WORKING_DIR(len, buff) GetCurrentDirectoryA(len, buff)
#define PASS_PCH
#else
#define GET_CURR_WORKING_DIR(len, buff) getcwd(buff, len)
#endif

#define MAX_STR_BUFF    1024

// Declare logger client
DECLARE_LOGGER_CLIENT;

const char* GetOpenCLVersionStr(OPENCL_VERSION ver)
{
    switch(ver)
    {
    case OPENCL_VERSION_1_2: return "120";
    case OPENCL_VERSION_2_0: return "200";
    default:
        throw "Unknown OpenCL version";
    }
}

OclMutex ClangFETask::s_serializingMutex;

int ClangFECompilerCompileTask::StoreOutput(TC::STB_TranslateOutputArgs* pOutputArgs, TC::TB_DATA_FORMAT llvmBinaryType)
{
    assert(pOutputArgs);
    try
    {
        if ( pOutputArgs->pErrorString )
        {
            m_sLogString = pOutputArgs->pErrorString;
        }

        const char*  BufferStart = (const char*) pOutputArgs->pOutput;
        size_t BufferSize = pOutputArgs->OutputSize;
        llvm::SmallVector<char, 4096>   SPIRbinary;
        llvm::raw_svector_ostream SPIRstream(SPIRbinary);
        llvm::OwningPtr<llvm::LLVMContext> context( new llvm::LLVMContext() );

        if ( pOutputArgs->pOutput && pOutputArgs->OutputSize )
        {
            m_pOutIR = new char[BufferSize];
            if ( NULL == m_pOutIR )
            {
                LOG_ERROR(TEXT("%s"), TEXT("Failed to allocate memory for buffer"));
                return CL_OUT_OF_HOST_MEMORY;
            }
        }

        if ( NULL != m_pOutIR )
        {
          m_stOutIRSize = BufferSize;
          void *pIR = (void*)m_pOutIR;
          // Copy IR
          MEMCPY_S(pIR, BufferSize, BufferStart, BufferSize);
        }
    }
    catch( std::bad_alloc )
    {
        return CL_OUT_OF_HOST_MEMORY;
    }
    return CL_SUCCESS;
}

void ClangFECompilerCompileTask::ClearOutput( TC::STB_TranslateOutputArgs* pOutputArgs )
{
    try{
        if ( pOutputArgs->pErrorString )
        {
          m_sLogString = pOutputArgs->pErrorString;
        }
    }
    catch( std::bad_alloc)
    {
    }

    m_stOutIRSize = 0;
    m_pOutIR = NULL;
}

int ClangFECompilerCompileTask::Compile()
{
  LOG_INFO(TEXT("%s"), TEXT("enter"));

  OclAutoMutex CS(&s_serializingMutex);

  try
  {   // create a new scope to make sure the mutex will be released last
    // Prepare argument list

    bool clStd20 = std::string(m_pProgDesc->pszOptions).find("-cl-std=CL2.0") != std::string::npos;
    int rcid = clStd20 ? IDR_PCH2 : IDR_PCH1;
    size_t uiPCHSize = 0;
    const char* pPCHBuff = ResourceManager::instance().get_resource(rcid, "PCH", false, uiPCHSize, 0);

    std::stringstream optionsEx;
    // Add standard OpenCL options
    optionsEx << " -fno-validate-pch";

    // Add current directory
    char szCurrDirrPath[MAX_STR_BUFF];
    GET_CURR_WORKING_DIR(MAX_STR_BUFF, szCurrDirrPath);

    optionsEx << " -I" << szCurrDirrPath;

    if(m_sDeviceInfo.bImageSupport)
    {
        optionsEx << " -D__IMAGE_SUPPORT__=1";
    }

    if (m_sDeviceInfo.bEnableSourceLevelProfiling)
    {
        optionsEx << " -profiling";
    }

    // Parse options
    CompileOptionsParser optionsParser(m_sDeviceInfo.sExtensionStrings, GetOpenCLVersionStr(m_config.GetOpenCLVersion()));
    optionsParser.processOptions(m_pProgDesc->pszOptions, optionsEx.str().c_str());
    m_source_filename = optionsParser.getSourceName();

    // Create the input ELF binary
    ElfWriterPtr pElfWriter( CLElfLib::CElfWriter::Create( CLElfLib::EH_TYPE_OPENCL_SOURCE,
                                                           CLElfLib::EH_MACHINE_NONE,
                                                           0 ) );
    if( !pElfWriter.get() )
    {
        throw std::bad_alloc();
    }

    CLElfLib::SSectionNode sectionNode;

    // create main section
    sectionNode.Name = "CLMain";
    sectionNode.pData = (char *)m_pProgDesc->pProgramSource;
    sectionNode.DataSize = strlen( m_pProgDesc->pProgramSource );
    sectionNode.Flags = 0;
    sectionNode.Type = CLElfLib::SH_TYPE_OPENCL_SOURCE;

    // add main program's source
    if( pElfWriter->AddSection( &sectionNode ) != CLElfLib::SUCCESS)
    {
        throw std::bad_alloc();
    }

    // add each header
    for( UINT i = 0; i < m_pProgDesc->uiNumInputHeaders; i++ )
    {
        sectionNode.Name = m_pProgDesc->pszInputHeadersNames[i];
        sectionNode.pData = (char *)m_pProgDesc->pInputHeaders[i];
        sectionNode.DataSize = strlen( m_pProgDesc->pInputHeaders[i] );
        sectionNode.Type  = CLElfLib::SH_TYPE_OPENCL_HEADER;
        sectionNode.Flags = 0;

        if( pElfWriter->AddSection( &sectionNode ) != CLElfLib::SUCCESS )
        {
            throw std::bad_alloc();
        }
    }

    sectionNode.Name = "CLPCH";
    sectionNode.pData = const_cast<char*>(pPCHBuff);
    sectionNode.DataSize = uiPCHSize;
    sectionNode.Flags = 0;
    sectionNode.Type = CLElfLib::SH_TYPE_OPENCL_PCH;

    if( pElfWriter->AddSection( &sectionNode ) != CLElfLib::SUCCESS )
    {
        throw std::bad_alloc();
    }

    char *pCompileData = NULL;
    cl_uint CompileDataSize = 0;

    if( pElfWriter->ResolveBinary( pCompileData, CompileDataSize ) != CLElfLib::SUCCESS )
    {
        throw std::bad_alloc();
    }

    pCompileData = new char[CompileDataSize];
    std::auto_ptr<char> spCompileData(pCompileData);

    // Get resolved compiled data size and allocate
    if( pElfWriter->ResolveBinary( pCompileData, CompileDataSize ) != CLElfLib::SUCCESS )
    {
        throw std::bad_alloc();
    }

    // create unique data for new thread
    TC::STB_TranslationCode code = { TC::TB_DATA_FORMAT_ELF, TC::TB_DATA_FORMAT_LLVM_BINARY };
    TC::STC_TranslateArgs TranslateArgs;
    std::string sOptions = optionsParser.getEffectiveOptionsAsString();

    TranslateArgs.pInput = pCompileData;
    TranslateArgs.InputSize = CompileDataSize;
    TranslateArgs.ChainType = TC::TC_CHAIN_COMPILE;
    TranslateArgs.Code = code;
    TranslateArgs.Options = sOptions.c_str();
    TranslateArgs.pTask = this;

    int retVal = TC::CTranslationController::ProcessTranslation( &TranslateArgs);
    LOG_INFO(TEXT("%s"), TEXT("Finished"));
    return retVal ;
  }
  catch( std::bad_alloc& )
  {
      return CL_OUT_OF_HOST_MEMORY;
  }
}

int ClangFECompilerLinkTask::Link(IOCLFEBinaryResult* *pBinaryResult)
{
    std::vector<void*> m_Binaries;
    std::vector<size_t> m_BinariesSizes;

    for(unsigned int i = 0; i < m_pProgDesc->uiNumBinaries; ++i)
    {
        CacheBinaryReader cacheReader(m_pProgDesc->pBinaryContainers[i], m_pProgDesc->puiBinariesSizes[i]);

        if(cacheReader.IsCachedObject())
        {
            m_Binaries.push_back((void*)cacheReader.GetSectionData(g_irSectionName));
            m_BinariesSizes.push_back(cacheReader.GetSectionSize(g_irSectionName));
        }
        else
        {
            m_Binaries.push_back((void*)m_pProgDesc->pBinaryContainers[i]);
            m_BinariesSizes.push_back(m_pProgDesc->puiBinariesSizes[i]);
        }
    }

    IOCLFEBinaryResultPtr spBinaryResult;

    int res = ::Link((const void**)m_Binaries.data(),
                  m_pProgDesc->uiNumBinaries,
                  m_BinariesSizes.data(),
                  m_pProgDesc->pszOptions,
                  spBinaryResult.getOutPtr());

#ifdef OCLFRONTEND_PLUGINS
    if (getenv("OCLBACKEND_PLUGINS") && NULL == getenv("OCL_DISABLE_SOURCE_RECORDER"))
    {
        Intel::OpenCL::Frontend::LinkData linkData;

        for(unsigned int i = 0; i < m_pProgDesc->uiNumBinaries; ++i)
        {
            linkData.addInputBuffer(m_pProgDesc->pBinaryContainers[i], m_pProgDesc->puiBinariesSizes[i]);
        }
        linkData.setOptions(m_pProgDesc->pszOptions);
        linkData.setBinaryResult(spBinaryResult.get());
        g_pluginManager.OnLink(&linkData);
    }
#endif //OCLFRONTEND_PLUGINS

    if( pBinaryResult )
    {
        *pBinaryResult = spBinaryResult.release();
    }
    return res;
}

int ClangFECompilerGetKernelArgInfoTask::GetKernelArgInfo(const void *pBin,
                                                          size_t      uiBinarySize,
                                                          const char *szKernelName,
                                                          IOCLFEKernelArgInfo** ppResult)
{
    CacheBinaryReader cacheReader(pBin, uiBinarySize);

    if(cacheReader.IsCachedObject())
    {
        pBin = (void*)cacheReader.GetSectionData(g_irSectionName);
        uiBinarySize = cacheReader.GetSectionSize(g_irSectionName);
    }
    return ::GetKernelArgInfo(pBin, uiBinarySize,  szKernelName, ppResult);
}

bool Intel::OpenCL::ClangFE::ClangFECompilerCheckCompileOptions(const char*  szOptions,
                                        char*        szUnrecognizedOptions,
                                        size_t uiUnrecognizedOptionsSize,
                                        const Intel::OpenCL::Utils::BasicCLConfigWrapper& )
{
    return ::CheckCompileOptions(szOptions,szUnrecognizedOptions,uiUnrecognizedOptionsSize);
}

bool Intel::OpenCL::ClangFE::ClangFECompilerCheckLinkOptions(const char* szOptions, char* szUnrecognizedOptions, size_t uiUnrecognizedOptionsSize)
{
    return ::CheckLinkOptions(szOptions, szUnrecognizedOptions, uiUnrecognizedOptionsSize);
}
