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
#include "clang_driver.h"
#include "elf_binary.h"
#include "cache_binary_handler.h"
#include "mic_dev_limits.h"
#include "common_clang.h"
#include "os_inc.h"

#include <Logger.h>
#include <cl_sys_info.h>
#include <cl_cpu_detect.h>
#include <cl_autoptr_ex.h>

#include <string>
#include <list>
#include <vector>
#include <cctype>
#include <algorithm>

using namespace Intel::OpenCL::ELFUtils;
using namespace Intel::OpenCL::ClangFE;
using namespace Intel::OpenCL::Utils;
using namespace std;

typedef auto_ptr_ex<IOCLFEBinaryResult,ReleaseDP<IOCLFEBinaryResult> > IOCLFEBinaryResultPtr;

#if defined (_WIN32)
#define GET_CURR_WORKING_DIR(len, buff) GetCurrentDirectoryA(len, buff)
#define PASS_PCH
#else
#define GET_CURR_WORKING_DIR(len, buff) getcwd(buff, len)
#endif

#define MAX_STR_BUFF    1024

// Declare logger client
DECLARE_LOGGER_CLIENT;

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

std::string GetCurrentDir()
{
    char szCurrDirrPath[MAX_STR_BUFF];
    GET_CURR_WORKING_DIR(MAX_STR_BUFF, szCurrDirrPath);

    std::stringstream ss;

    ss << "\"" << szCurrDirrPath << "\"";

    return ss.str();
}

int ClangFECompilerCompileTask::Compile(IOCLFEBinaryResult* *pBinaryResult)
{
    LOG_INFO(TEXT("%s"), TEXT("enter"));

    bool bProfiling   = std::string(m_pProgDesc->pszOptions).find("-profiling")            != std::string::npos;
    bool bRelaxedMath = std::string(m_pProgDesc->pszOptions).find("-cl-fast-relaxed-math") != std::string::npos;

    // Force the -profiling option if such was not supplied by user
    std::stringstream options;
    options << m_pProgDesc->pszOptions;

    if (m_sDeviceInfo.bEnableSourceLevelProfiling && !bProfiling)
    {
        options << " -profiling";
    }

    // Passing -cl-fast-relaxed-math option if specifed in the environment variable or in the config
    const bool useRelaxedMath = m_config.UseRelaxedMath();

    if (useRelaxedMath && !bRelaxedMath)
    {
        options << " -cl-fast-relaxed-math";
    }

    options << " -pch-cpu";
    std::stringstream optionsEx;
    // Add current directory
    optionsEx << " -I" << GetCurrentDir();

    if(m_sDeviceInfo.bImageSupport)
    {
        optionsEx << " -D__IMAGE_SUPPORT__=1";
    }

    IOCLFEBinaryResultPtr spBinaryResult;

    int res = ::Compile(m_pProgDesc->pProgramSource,
                    m_pProgDesc->pInputHeaders,
                    m_pProgDesc->uiNumInputHeaders,
                    m_pProgDesc->pszInputHeadersNames,
                    0,
                    0,
                    options.str().c_str(),   // pszOptions
                    optionsEx.str().c_str(), // pszOptionsEx
                    m_sDeviceInfo.sExtensionStrings,
                    GetOpenCLVersionStr(m_config.GetOpenCLVersion()),
                    spBinaryResult.getOutPtr());

#ifdef OCLFRONTEND_PLUGINS
    if (getenv("OCLBACKEND_PLUGINS") && NULL == getenv("OCL_DISABLE_SOURCE_RECORDER"))
    {
        Intel::OpenCL::Frontend::CompileData compileData;
        Intel::OpenCL::Frontend::SourceFile sourceFile = createSourceFile( m_pProgDesc->pProgramSource,
                                                                           m_pProgDesc->pszOptions,
                                                                           0,
                                                                           spBinaryResult.get());
        compileData.sourceFile(sourceFile);
        for (unsigned headerCount=0 ; headerCount < m_pProgDesc->uiNumInputHeaders ; headerCount++)
        {
            compileData.addIncludeFile(createSourceFile(
                m_pProgDesc->pszInputHeadersNames[headerCount],
                "", //include files comes without compliation flags
                headerCount+1));
        }
        g_pluginManager.OnCompile(&compileData);
    }
#endif //OCLFRONTEND_PLUGINS

    if( pBinaryResult )
    {
        *pBinaryResult = spBinaryResult.release();
    }
    return res;
}

//
// ClangFECompilerLinkTask calls implementation
//
int ClangFECompilerLinkTask::Link(IOCLFEBinaryResult* *pBinaryResult)
{
    std::vector<void*> m_Binaries;
    std::vector<size_t> m_BinariesSizes;

    for(unsigned int i = 0; i < m_pProgDesc->uiNumBinaries; ++i)
    {
        if(CacheBinaryReader::IsValidCacheObject(m_pProgDesc->pBinaryContainers[i], m_pProgDesc->puiBinariesSizes[i]))
        {
            CacheBinaryReader reader(m_pProgDesc->pBinaryContainers[i], m_pProgDesc->puiBinariesSizes[i]);
            m_Binaries.push_back((void*)reader.GetSectionData(g_irSectionName));
            m_BinariesSizes.push_back(reader.GetSectionSize(g_irSectionName));
        }
        else if(OCLElfBinaryReader::IsValidOpenCLBinary((const char*)m_pProgDesc->pBinaryContainers[i], m_pProgDesc->puiBinariesSizes[i]))
        {
            OCLElfBinaryReader reader((const char*)m_pProgDesc->pBinaryContainers[i], m_pProgDesc->puiBinariesSizes[i]);
            char* pBinaryData = NULL;
            size_t uiBinaryDataSize = 0;
            reader.GetIR(pBinaryData,uiBinaryDataSize);
            m_Binaries.push_back(pBinaryData);
            m_BinariesSizes.push_back(uiBinaryDataSize);
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

        for(unsigned int i = 0; i < m_Binaries.size(); ++i)
        {
            linkData.addInputBuffer(m_Binaries[i], m_BinariesSizes[i]);
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
    char* pIRBuffer = NULL;
    size_t uiIRBufferSize = 0;

    if(CacheBinaryReader::IsValidCacheObject(pBin, uiBinarySize))
    {
        CacheBinaryReader reader(pBin, uiBinarySize);
        pIRBuffer = (char*)reader.GetSectionData(g_irSectionName);
        uiIRBufferSize = reader.GetSectionSize(g_irSectionName);
    }
    else if(OCLElfBinaryReader::IsValidOpenCLBinary((const char*)pBin, uiBinarySize))
    {
        OCLElfBinaryReader reader((const char*)pBin, uiBinarySize);
        reader.GetIR(pIRBuffer,uiIRBufferSize);
    }
    else
    {
        pIRBuffer = const_cast<char*>((const char*)pBin);
        uiIRBufferSize = uiBinarySize;
    }
    return ::GetKernelArgInfo((const void*)pIRBuffer, uiIRBufferSize,  szKernelName, ppResult);
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
