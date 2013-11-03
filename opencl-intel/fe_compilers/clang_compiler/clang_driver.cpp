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
#include "llvm/IR/Constants.h"
#include "llvm/Linker.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/PassManager.h"
#include "llvm/IR/DataLayout.h"
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
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/Path.h"
#include "llvm/Transforms/IPO.h"
#include "mic_dev_limits.h"

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

#define MAX_STR_BUFF    1024

// Declare logger client
DECLARE_LOGGER_CLIENT;

OclMutex ClangFETask::s_serializingMutex;

// We use this ID in order to give each source a unique file name to prevent
// Multiple compile units with the same ID later
static int g_uiProgID = 1;

const std::string APFLevelOptionName("-auto-prefetch-level=");

void LLVMErrorHandler(void *UserData, const std::string &Message, bool gen_crash_diag) {
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
ClangFECompilerCompileTask::ClangFECompilerCompileTask(Intel::OpenCL::FECompilerAPI::FECompileProgramDescriptor* pProgDesc, 
																												Intel::OpenCL::ClangFE::CLANG_DEV_INFO sDeviceInfo,
                                                                                                                const Intel::OpenCL::Utils::BasicCLConfigWrapper& config)
: m_pProgDesc(pProgDesc), m_sDeviceInfo(sDeviceInfo), m_pOutIR(NULL), m_stOutIRSize(0), m_pLogString(NULL), m_stLogSize(0), m_config(config)
{
  Twine t(g_uiProgID++);
  m_source_filename = t.str();
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

void ClangFECompilerCompileTask::PrepareArgumentList(ArgListType &list, ArgListType &BEArgList, const char *buildOpts)
{
    CLSTDSet = 0;
    std::string triple;

    ParseCompileOptions(buildOpts,
                        NULL,
                        m_config,
                        &list,
                        &BEArgList,
                        &CLSTDSet,
                        &OptDebugInfo,
                        &OptProfiling,
                        &Opt_Disable,
                        &Denorms_Are_Zeros,
                        &Fast_Relaxed_Math,
                        &m_source_filename,
                        &triple);

    // Add standard OpenCL options

  if(!CLSTDSet) {
    list.push_back("-cl-std=CL1.2");
    list.push_back("-D");
    list.push_back("__OPENCL_C_VERSION__=120");
  }

    list.push_back("-x");
    list.push_back("cl");
    list.push_back("-S");
    list.push_back("-emit-llvm-bc");


    char    szBinaryPath[MAX_STR_BUFF];
    char    szCurrDirrPath[MAX_STR_BUFF];

    // Retrieve local relatively to binary directory
    GetModuleDirectory(szBinaryPath, MAX_STR_BUFF);

    const char* pchFileName;
    if ( CLSTDSet >= 200 ) {
      pchFileName = "opencl20_.pch";
    } else {
      pchFileName = "opencl_.pch";
    }

#ifndef PASS_PCH
    char szOclIncPath[MAX_STR_BUFF];
    char  szOclPchPath[MAX_STR_BUFF];

  SPRINTF_S(szOclIncPath, MAX_STR_BUFF, "%sfe_include", szBinaryPath);
  SPRINTF_S(szOclPchPath, MAX_STR_BUFF, "%s%s", szBinaryPath, pchFileName);

  list.push_back("-I");
  list.push_back(szOclIncPath);

  list.push_back("-include-pch");
  list.push_back(szOclPchPath);
#else
  list.push_back("-include-pch");
  list.push_back(pchFileName);
#endif

    list.push_back("-fno-validate-pch");

    // Add current directory
    GET_CURR_WORKING_DIR(MAX_STR_BUFF, szCurrDirrPath);
    list.push_back("-I");
    list.push_back(szCurrDirrPath);

	//Add OpenCL predefined macros
	list.push_back("-D");
	list.push_back("__OPENCL_VERSION__=120");
	list.push_back("-D");
	list.push_back("CL_VERSION_1_0=100");
	list.push_back("-D");
	list.push_back("CL_VERSION_1_1=110");
	list.push_back("-D");
	list.push_back("CL_VERSION_1_2=120");
	list.push_back("-D");
	list.push_back("CL_VERSION_2_0=200");
	list.push_back("-D");
	list.push_back("__ENDIAN_LITTLE__=1");
	list.push_back("-D");
	list.push_back("__ROUNDING_MODE__=rte");	
	if(m_sDeviceInfo.bImageSupport) {
		list.push_back("-D");
		list.push_back("__IMAGE_SUPPORT__=1");	
	}
	if (!OptProfiling && m_sDeviceInfo.bEnableSourceLevelProfiling) {
        OptProfiling = true;

        if (!OptDebugInfo)
        {
            list.push_back("-g");

            list.push_back("-main-file-name");
            list.push_back(m_source_filename.c_str());
        }
        
    }

    // Add extension defines
    std::string extStr = m_sDeviceInfo.sExtensionStrings;
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
    list.push_back("-triple");
    if(triple.empty()) {
#if defined(_WIN64) || defined(__x86_64__) || defined(_M_AMD64) || defined (_M_X64)
        list.push_back("spir64-unknown-unknown");
#elif defined(_WIN32) || defined(i386) || defined(__i386__) || defined(__x86__) || defined(__ANDROID__)
        list.push_back("spir-unknown-unknown");
#else
  #error "Can't define target triple: unknown architecture."
#endif
    } else {
        list.push_back(triple);
    }
}

int ClangFECompilerCompileTask::Compile()
{
  LOG_INFO(TEXT("%s"), TEXT("enter"));

  OclAutoMutex CS(&s_serializingMutex);
  {   // create a new scope to make sure the mutex will be released last

  // Prepare argument list
  ArgListType ArgList;
  ArgListType BEArgList;

  PrepareArgumentList(ArgList, BEArgList, m_pProgDesc->pszOptions);

  const char **argArray = new const char *[ArgList.size()];
  ArgListType::iterator iter = ArgList.begin();

  for(unsigned int i=0; i<ArgList.size(); i++)
  {
    argArray[i] = iter->c_str();
    iter++;
  }

  // Prepare input Buffer
  llvm::MemoryBuffer *inputBuffer = llvm::MemoryBuffer::getMemBuffer(
        m_pProgDesc->pProgramSource,
        m_source_filename);

  // Prepare output buffer
  SmallVector<char, 4096>   IRbinary;
  llvm::raw_svector_ostream IRStream(IRbinary);

  // Prepare error buffer
  SmallVector<char, 4096> Log;
  llvm::raw_svector_ostream errStream(Log);

  // Prepare our diagnostic client.
  llvm::IntrusiveRefCntPtr<DiagnosticIDs> DiagID = new DiagnosticIDs();
  llvm::IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts = new DiagnosticOptions();
  DiagOpts->ShowCarets = 0;
  TextDiagnosticPrinter *DiagsPrinter = new TextDiagnosticPrinter(errStream, DiagOpts.getPtr());
  llvm::IntrusiveRefCntPtr<DiagnosticsEngine> Diags = 
    new DiagnosticsEngine(DiagID, &*DiagOpts, DiagsPrinter);

  // Prepare Clang
  llvm::OwningPtr<CompilerInstance> Clang(new CompilerInstance());

  // Set our buffers
  Clang->SetInputBuffer(inputBuffer);
  Clang->SetOutputStream(&IRStream);
  Clang->setDiagnostics(Diags.getPtr());

  // Create compiler invocation from user args
  CompilerInvocation::CreateFromArgs(Clang->getInvocation(), argArray, argArray + ArgList.size(),
                                     *Diags);

  // Set an error handler, so that any LLVM backend diagnostics go through our
  // error handler.
  llvm::remove_fatal_error_handler();
  llvm::install_fatal_error_handler(LLVMErrorHandler,
                                    static_cast<void*>(&Clang->getDiagnostics()));


  for (unsigned int i = 0; i < m_pProgDesc->uiNumInputHeaders; ++i)
  {
      llvm::MemoryBuffer *header = llvm::MemoryBuffer::getMemBufferCopy(m_pProgDesc->pInputHeaders[i], m_pProgDesc->pszInputHeadersNames[i]);
      Clang->AddInMemoryHeader(header, m_pProgDesc->pszInputHeadersNames[i]);
  }

  bool Success = true;

#ifdef PASS_PCH
  //prepare pch buffer
  HMODULE hMod = NULL;
  HRSRC hRes = NULL;
  HGLOBAL hBytes = NULL;
  char *pData = NULL;
  size_t dResSize = NULL;
  llvm::MemoryBuffer *pchBuff = NULL;

#if defined (_WIN32)
#if defined (_M_X64)
  static const char* sFEModuleName = "clang_compiler64";
#else
  static const char* sFEModuleName = "clang_compiler32";
#endif
#else
  static const char* sFEModuleName = "clang_compiler";
#endif

  // Get the handle to the current module
  GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | 
                    GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, 
                    sFEModuleName,
                    &hMod);

  // Locate the resource
  if( NULL != hMod )
  {
    if ( CLSTDSet >= 200 ) {
      hRes = FindResource(hMod, "#102", "PCH");
    } else {
      hRes = FindResource(hMod, "#101", "PCH");
    }
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

    IRStream.flush();
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
    
    // Add module level SPIR related stuff
    string ErrorMessage;
    const char*  BufferStart = (const char*)IRbinary.begin();
    size_t BufferSize = IRbinary.size();
    llvm::OwningPtr<MemoryBuffer> pBinBuff (MemoryBuffer::getMemBufferCopy(StringRef((const char *)(IRbinary.begin()), IRbinary.size())));
    llvm::SmallVector<char, 4096>   SPIRbinary;
    llvm::raw_svector_ostream SPIRstream(SPIRbinary);
    {
        LLVMContext ctx;
        llvm::Module *M = ParseBitcodeFile(pBinBuff.get(), ctx, &ErrorMessage);
        llvm::OwningPtr<PassManager> Passes(new PassManager());
        Passes->add(new DataLayout(M)); // Use correct DataLayout
        Passes->add(createSPIRMetadataAdderPass(BEArgList, CLSTDSet));
        Passes->run(*M);
        WriteBitcodeToFile(M, SPIRstream);
        SPIRstream.flush();
        BufferStart = (const char*)SPIRbinary.begin();
        BufferSize  = SPIRbinary.size();
    }
    size_t stTotSize = 0;
    if ( BufferSize > 0 )
    {
      stTotSize = BufferSize +
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
      cl_prog_container_header* pHeader = (cl_prog_container_header*)m_pOutIR;
      MEMCPY_S(pHeader->mask, 4, _CL_CONTAINER_MASK_, 4);
      pHeader->container_size = BufferSize+sizeof(cl_llvm_prog_header);
      pHeader->container_type = CL_PROG_CNT_PRIVATE;
      pHeader->description.bin_type = CL_PROG_BIN_COMPILED_LLVM;
      pHeader->description.bin_ver_major = 1;
      pHeader->description.bin_ver_minor = 1;
      // Fill options
      cl_llvm_prog_header *pProgHeader = (cl_llvm_prog_header*)(m_pOutIR+sizeof(cl_prog_container_header));
      pProgHeader->bDebugInfo = OptDebugInfo;
      pProgHeader->bProfiling = OptProfiling;
      pProgHeader->bDisableOpt = Opt_Disable;
      pProgHeader->bDenormsAreZero = Denorms_Are_Zeros;
      pProgHeader->bFastRelaxedMath = Fast_Relaxed_Math;
      pProgHeader->bEnableLinkOptions = true; // enable link options is true for all compiled objects
      void *pIR = (void*)(pProgHeader+1);
      // Copy IR
      MEMCPY_S(pIR, BufferSize, BufferStart, BufferSize);
    }

    LOG_INFO(TEXT("%s"), TEXT("Finished"));

    delete []argArray;

    return CL_SUCCESS;
  }
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
    OclAutoMutex CS(&s_serializingMutex);
    char* pBinary = NULL;

    {   // create a new scope to make sure the mutex will be released last

    if (0 == m_pProgDesc->uiNumBinaries)
    {
        return CL_SUCCESS;
    }

    ParseOptions(m_pProgDesc->pszOptions);
    ResolveFlags();

    if (1 == m_pProgDesc->uiNumBinaries)
    {
        //Don't need actual linker...
        bool isSPIR = (((const char*)(m_pProgDesc->pBinaryContainers[0]))[0] == 'B') &&
          (((const char*)(m_pProgDesc->pBinaryContainers[0]))[1] == 'C');

        m_stOutIRSize = m_pProgDesc->puiBinariesSizes[0];
        if (isSPIR) m_stOutIRSize += sizeof(cl_llvm_prog_header) + sizeof(cl_prog_container_header);

        m_pOutIR = new char[m_stOutIRSize];
        if ( NULL == m_pOutIR )
        {
            LOG_ERROR(TEXT("%s"), TEXT("Failed to allocate memory for buffer"));
            m_stOutIRSize = 0;
            return CL_OUT_OF_HOST_MEMORY;
        }

        if (isSPIR) 
        {
            cl_prog_container_header*   pHeader = (cl_prog_container_header*)m_pOutIR;
            MEMCPY_S(pHeader->mask, 4, _CL_CONTAINER_MASK_, 4);
            pHeader->container_size = m_pProgDesc->puiBinariesSizes[0]+sizeof(cl_llvm_prog_header);
            pHeader->container_type = CL_PROG_CNT_PRIVATE;
            pHeader->description.bin_type = CL_PROG_BIN_COMPILED_LLVM;
            pHeader->description.bin_ver_major = 1;
            pHeader->description.bin_ver_minor = 1;
            // Fill options
            cl_llvm_prog_header *pProgHeader = (cl_llvm_prog_header*)(m_pOutIR+sizeof(cl_prog_container_header));
            pProgHeader->bDebugInfo = false;
            pProgHeader->bProfiling = false;
            pProgHeader->bDisableOpt = false;
            pProgHeader->bDenormsAreZero = false;
            pProgHeader->bFastRelaxedMath = false;
            pProgHeader->bEnableLinkOptions = false;
            void *pIR = (void*)(pProgHeader+1);

            string ErrorMessage;
            MemoryBuffer *pBinBuff = MemoryBuffer::getMemBufferCopy(StringRef((const char *)(m_pProgDesc->pBinaryContainers[0]), m_pProgDesc->puiBinariesSizes[0]));   
            llvm::Module *M = ParseBitcodeFile(pBinBuff, getGlobalContext(), &ErrorMessage);

            if (NULL == M) 
            {
                LOG_ERROR(TEXT("%s"), TEXT("Cannot parse SPIR binary"));
                m_stOutIRSize = 0;
                return CL_LINK_PROGRAM_FAILURE;
            }
            
            if (llvm::NamedMDNode *Opts = M->getNamedMetadata("opencl.compiler.options")){
                if (llvm::MDNode *OptsMD = Opts->getOperand(0)) {
                    for (unsigned i=0; i<OptsMD->getNumOperands(); ++i){
                        MDString *S = dyn_cast_or_null<MDString>(OptsMD->getOperand(i));
                        assert(S && "Null String value in compiler options");
                        StringRef opt = S->getString();
                        pProgHeader->bDisableOpt |= (opt == "-cl-opt-disable");
                        pProgHeader->bDenormsAreZero |= (opt == "-cl-denorms-are-zeros");
                        pProgHeader->bFastRelaxedMath |= ( opt == "-cl-fast-relaxed-math");
                        pProgHeader->bEnableLinkOptions |= ( opt == "-enable-link-options");
                    }
                }
            }

            MEMCPY_S(pIR, m_pProgDesc->puiBinariesSizes[0],
                m_pProgDesc->pBinaryContainers[0], m_pProgDesc->puiBinariesSizes[0]);
        } 
        else 
        {
            MEMCPY_S(m_pOutIR, m_stOutIRSize, m_pProgDesc->pBinaryContainers[0], m_stOutIRSize);
        }

        cl_prog_container_header*   pHeader = (cl_prog_container_header*)m_pOutIR;
        pHeader->description.bin_type = CL_PROG_BIN_LINKED_LLVM;

        cl_llvm_prog_header* pProgHeader = (cl_llvm_prog_header*)((char*)pHeader + sizeof(cl_prog_container_header));

        pProgHeader->bDebugInfo = bDebugInfoFlag;
        pProgHeader->bProfiling = bProfilingFlag;
        pProgHeader->bDenormsAreZero = bDenormsAreZeroFlag;
        pProgHeader->bDisableOpt = bDisableOptFlag;
        pProgHeader->bFastRelaxedMath = bFastRelaxedMathFlag;
        pProgHeader->bEnableLinkOptions = bEnableLinkOptionsFlag;

        return CL_SUCCESS;
    }

    // We have more the one binary so we need to link
    
    string ErrorMessage;
    LLVMContext &Context = getGlobalContext();
    size_t uiBinarySize = 0;

    // Initialize the module with the first binary
    cl_prog_container_header* pHeader = (cl_prog_container_header*) m_pProgDesc->pBinaryContainers[0];

    cl_llvm_prog_header* pProgHeader = (cl_llvm_prog_header*)((char*)pHeader + sizeof(cl_prog_container_header));

    if ( llvm::isRawBitcode((const unsigned char*)m_pProgDesc->pBinaryContainers[0], NULL) ) {
        uiBinarySize = m_pProgDesc->puiBinariesSizes[0];
        pBinary = (char*)m_pProgDesc->pBinaryContainers[0];

    } else {
        uiBinarySize = pHeader->container_size - sizeof(cl_llvm_prog_header);
        pBinary = (char*)pHeader + 
                  sizeof(cl_prog_container_header) + // Skip the container
                  sizeof(cl_llvm_prog_header); //Skip the build flags for now

    }
    MemoryBuffer *pBinBuff = MemoryBuffer::getMemBufferCopy(StringRef(pBinary, uiBinarySize));   
    std::auto_ptr<llvm::Module> composite(ParseBitcodeFile(pBinBuff, Context, &ErrorMessage));


    if (composite.get() == 0) 
    {
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
        if ( llvm::isRawBitcode((const unsigned char*)m_pProgDesc->pBinaryContainers[i], NULL) ) {
            uiBinarySize = m_pProgDesc->puiBinariesSizes[i];
            pBinary = (char*)m_pProgDesc->pBinaryContainers[i];
        } else {
            pHeader = (cl_prog_container_header*) m_pProgDesc->pBinaryContainers[i];
            pProgHeader = (cl_llvm_prog_header*)((char*)pHeader + sizeof(cl_prog_container_header));

            uiBinarySize = pHeader->container_size - sizeof(cl_llvm_prog_header);

            pBinary = (char*)pHeader + 
                      sizeof(cl_prog_container_header) + // Skip the container
                      sizeof(cl_llvm_prog_header); //Skip the build flags for now
        }

        MemoryBuffer *pBinBuff = MemoryBuffer::getMemBufferCopy(StringRef(pBinary, uiBinarySize));
               
        std::auto_ptr<llvm::Module> M(ParseBitcodeFile(pBinBuff, Context, &ErrorMessage));
        if (M.get() == 0) 
        {
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

        if( Linker::LinkModules(composite.get(), M.get(), Linker::DestroySource, &ErrorMessage))
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

    llvm::SmallVector<char, 1024> Buffer;
    llvm::raw_svector_ostream OS(Buffer);

    Buffer.reserve(256*1024);

    WriteBitcodeToFile(composite.get(), OS);
    OS.flush();

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

    MEMCPY_S(pHeader->mask, 4, _CL_CONTAINER_MASK_, 4);
      pHeader->container_size = Buffer.size() + sizeof(cl_llvm_prog_header);
      pHeader->container_type = CL_PROG_CNT_PRIVATE;
      pHeader->description.bin_type = CL_PROG_BIN_LINKED_LLVM;
      pHeader->description.bin_ver_major = 1;
      pHeader->description.bin_ver_minor = 1;

    pProgHeader->bDebugInfo = bDebugInfoFlag;
    pProgHeader->bProfiling = bProfilingFlag;
    pProgHeader->bDenormsAreZero = bDenormsAreZeroFlag;
    pProgHeader->bDisableOpt = bDisableOptFlag;
    pProgHeader->bFastRelaxedMath = bFastRelaxedMathFlag;
    pProgHeader->bEnableLinkOptions = bEnableLinkOptionsFlag;

    return CL_SUCCESS;

  }
}

void ClangFECompilerLinkTask::ParseOptions(const char *buildOpts)
{
    ParseLinkOptions(buildOpts,
                     NULL,
                     &bCreateLibrary,
                     &bEnableLinkOptions,
                     &bDenormsAreZero,
                     &bNoSignedZeroes,
                     &bUnsafeMath,
                     &bFiniteMath,
                     &bFastRelaxedMath);
}


void ClangFECompilerLinkTask::ResolveFlags()
{
    bDebugInfoFlag = false;
    bProfilingFlag = false;
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

    
    // Check fast relaxed math
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
    

    // Check debug info and profiling flags
    bDebugInfoFlag = true;
    bProfilingFlag = true;

    for (cl_uint i = 0; i < uiNumBinaries; ++i)
    {
        pProgHeader = (cl_llvm_prog_header*)(ppBinaries[i] + sizeof(cl_prog_container_header));
        if (!pProgHeader->bDebugInfo) {
            bDebugInfoFlag = false;
        }

        if (!pProgHeader->bProfiling) {
            bProfilingFlag = false;
        }
    }
}

ClangFECompilerGetKernelArgInfoTask::ClangFECompilerGetKernelArgInfoTask()
: m_numArgs(0), m_argsInfo(NULL)
{
}

ClangFECompilerGetKernelArgInfoTask::~ClangFECompilerGetKernelArgInfoTask()
{
    if (NULL != m_argsInfo)
    {
        for (unsigned int i = 0; i < m_numArgs; ++i)
        {
            if (m_argsInfo[i].name)
            {
                delete[] m_argsInfo[i].name;
                m_argsInfo[i].name = NULL;
            }

            if (m_argsInfo[i].typeName)
            {
                delete[] m_argsInfo[i].typeName;
                m_argsInfo[i].typeName = NULL;
            }
        }

        delete[] m_argsInfo;
        m_argsInfo = NULL;
    }
}

int ClangFECompilerGetKernelArgInfoTask::GetKernelArgInfo(const void *pBin, const char *szKernelName)
{
    OclAutoMutex CS(&s_serializingMutex);

    {   // create a new scope to make sure the mutex will be released last

    SMDiagnostic Err;
    string ErrorMessage;
    LLVMContext Context;

    cl_prog_container_header* pHeader = (cl_prog_container_header*) pBin;

    size_t uiBinarySize = pHeader->container_size - sizeof(cl_llvm_prog_header);

    char* pBinary = (char*)pHeader + 
                    sizeof(cl_prog_container_header) + // Skip the container
                    sizeof(cl_llvm_prog_header); //Skip the build flags

    MemoryBuffer *pBinBuff = MemoryBuffer::getMemBufferCopy(StringRef(pBinary, uiBinarySize));

    llvm::Module* pModule = ParseIR(pBinBuff, Err, Context);

    if (NULL == pModule) 
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    NamedMDNode *pKernels = pModule->getNamedMetadata("opencl.kernels");
    if (!pKernels)
    {
        assert(false && "couldn't find any kernels in the metadata");
        delete pModule;
        return CL_FE_INTERNAL_ERROR_OHNO;
    }

    unsigned int uiNumKernels = pKernels->getNumOperands();

    MDNode* pKernel = NULL;
    bool bFoundKernel = false;

    // go over the kernels and search for ours
    for (unsigned int i = 0; i < uiNumKernels; ++i)
    {
        pKernel = pKernels->getOperand(i);
        StringRef szCurrKernelName = pKernel->getOperand(0)->getName();

        if (0 == szCurrKernelName.compare(szKernelName))
        {
            // We found our kernel
            bFoundKernel = true;
            break;
        }
    }

    if (!bFoundKernel)
    {
        assert(false && "couldn't find our kernel in the metadata");
        delete pModule;
        return CL_FE_INTERNAL_ERROR_OHNO;
    }
      
    MDNode* pAddressQualifiers = NULL;
    MDNode* pAccessQualifiers = NULL;
    MDNode* pTypeNames = NULL;
    MDNode* pTypeQualifiers = NULL;
    MDNode* pArgNames = NULL;
       
    for (unsigned int i = 0; i < pKernel->getNumOperands(); ++i)
    {
        MDNode* pTemp = dyn_cast<MDNode>(pKernel->getOperand(i));
        if (NULL == pTemp){
            continue;
        }

        MDString* pName = dyn_cast<MDString>(pTemp->getOperand(0));
        if (NULL == pName) {
            continue;
        }

        if (0 == pName->getString().compare("kernel_arg_addr_space")) {
            pAddressQualifiers = pTemp;
            continue;
        }

        if (0 == pName->getString().compare("kernel_arg_access_qual")) {
            pAccessQualifiers = pTemp;
            continue;
        }

        if (0 == pName->getString().compare("kernel_arg_type")) {
            pTypeNames = pTemp;
            continue;
        }

        if (0 == pName->getString().compare("kernel_arg_type_qual")) {
            pTypeQualifiers = pTemp;
            continue;
        }

        if (0 == pName->getString().compare("kernel_arg_name")) {
            pArgNames = pTemp;
            continue;
        }
    }    

    // all of the above must be valid
	if ( !( pAddressQualifiers && pAccessQualifiers && pTypeNames && pTypeQualifiers && pArgNames ) ) {
        assert(pAddressQualifiers && "pAddressQualifiers is NULL");
        assert(pAccessQualifiers && "pAccessQualifiers is NULL");
        assert(pTypeNames && "pTypeNames is NULL");
        assert(pTypeQualifiers && "pTypeQualifiers is NULL");
        assert(pArgNames && "pArgNames is NULL");
        delete pModule;
        return CL_FE_INTERNAL_ERROR_OHNO;
	}

    m_numArgs = pAddressQualifiers->getNumOperands() - 1;

    m_argsInfo = new ARG_INFO[m_numArgs];
    if (!m_argsInfo)
    {
        delete pModule;
        return CL_OUT_OF_HOST_MEMORY;
    }
        
    for (unsigned int i = 1; i < m_numArgs + 1; ++i)
    {
        // Since the arg info in the metadata have a string field before the operands
        // Now we have an off by one that we need to compensate for
        ARG_INFO &argInfo = m_argsInfo[i - 1];

        // Adress qualifier
        ConstantInt* pAddressQualifier = dyn_cast<ConstantInt>(pAddressQualifiers->getOperand(i));
        assert(pAddressQualifier && "pAddressQualifier is not a valid ConstantInt*");

        uint64_t uiAddressQualifier = pAddressQualifier->getZExtValue();
        switch( uiAddressQualifier )
        {
        case 0:
            argInfo.adressQualifier = CL_KERNEL_ARG_ADDRESS_PRIVATE;
            break;
        case 1:
            argInfo.adressQualifier = CL_KERNEL_ARG_ADDRESS_GLOBAL;
            break;
        case 2:
            argInfo.adressQualifier = CL_KERNEL_ARG_ADDRESS_CONSTANT;
            break;
        case 3:
            argInfo.adressQualifier = CL_KERNEL_ARG_ADDRESS_LOCAL;
            break;
        }

        // Access qualifier
        llvm::MDString* pAccessQualifier = dyn_cast<llvm::MDString>(pAccessQualifiers->getOperand(i));
        assert(pAccessQualifier && "pAccessQualifier is not a valid MDString");

        if (!pAccessQualifier->getString().compare("none")) {
            argInfo.accessQualifier = CL_KERNEL_ARG_ACCESS_NONE;
        } else if (!pAccessQualifier->getString().compare("read_only")) {
            argInfo.accessQualifier = CL_KERNEL_ARG_ACCESS_READ_ONLY;
        } else if (!pAccessQualifier->getString().compare("write_only")) {
            argInfo.accessQualifier = CL_KERNEL_ARG_ACCESS_WRITE_ONLY;
        } else {
            argInfo.accessQualifier = CL_KERNEL_ARG_ACCESS_READ_WRITE;
        }

        // Type qualifier
        llvm::MDString* pTypeQualifier = dyn_cast<llvm::MDString>(pTypeQualifiers->getOperand(i));
        assert(pTypeQualifier && "pTypeQualifier is not a valid ConstantInt*");
        argInfo.typeQualifier = 0;
        if (pTypeQualifier->getString().find("const") != llvm::StringRef::npos)
        {
            argInfo.typeQualifier |= CL_KERNEL_ARG_TYPE_CONST;
        }
        if (pTypeQualifier->getString().find("restrict") != llvm::StringRef::npos)
        {
            argInfo.typeQualifier |= CL_KERNEL_ARG_TYPE_RESTRICT;
        }
        if (pTypeQualifier->getString().find("volatile") != llvm::StringRef::npos)
        {
            argInfo.typeQualifier |= CL_KERNEL_ARG_TYPE_VOLATILE;
        }

        // Type name
        MDString* pTypeName = dyn_cast<MDString>(pTypeNames->getOperand(i));
        assert(pTypeName && "pTypeName is not a valid MDString*");

        string szTypeName = pTypeName->getString().str();
        argInfo.typeName = new char[szTypeName.length() + 1];
        if (!argInfo.typeName)
        {
            delete pModule;
            return CL_OUT_OF_HOST_MEMORY;
        }
        STRCPY_S(argInfo.typeName, szTypeName.length() + 1, szTypeName.c_str());

        // Parameter name
        MDString* pArgName = dyn_cast<MDString>(pArgNames->getOperand(i));
        assert(pArgName && "pArgName is not a valid MDString*");

        string szArgName = pArgName->getString().str();
        argInfo.name = new char[szArgName.length() + 1];
        if (!argInfo.name)
        {
            delete pModule;
            return CL_OUT_OF_HOST_MEMORY;
        }
        STRCPY_S(argInfo.name, szArgName.length() + 1, szArgName.c_str());
    }

    delete pModule;
    return CL_SUCCESS;
    }
}

bool Intel::OpenCL::ClangFE::ClangFECompilerCheckCompileOptions(const char*  szOptions,
                                                                char**       szUnrecognizedOptions,
                                                                const Intel::OpenCL::Utils::BasicCLConfigWrapper& config)
{
    return ParseCompileOptions(szOptions, szUnrecognizedOptions, config);
}

bool Intel::OpenCL::ClangFE::ClangFECompilerCheckLinkOptions(const char* szOptions, char** szUnrecognizedOptions)
{
    return ParseLinkOptions(szOptions, szUnrecognizedOptions);
}

bool Intel::OpenCL::ClangFE::ParseCompileOptions(const char*  szOptions,
                                                 char**       szUnrecognizedOptions,
                                                 const Intel::OpenCL::Utils::BasicCLConfigWrapper& config,
                                                 ArgListType* pList,
                                                 ArgListType* pBEArgList,
                                                 int*         piCLStdSet,
                                                 bool*        pbOptDebugInfo,
                                                 bool*        pbOptProfiling,
                                                 bool*        pbOptDisable,
                                                 bool*        pbDenormsAreZeros,
                                                 bool*        pbFastRelaxedMath,
                                                 std::string* pszFileName,
                                                 std::string* pszTriple)
{
    // Reset options
    int  iCLStdSet = 0;
    bool bOptDebugInfo = false;
    bool bOptProfiling = false;
    bool bOptDisable = false;
    bool bDenormsAreZeros = false;
    bool bFastRelaxedMath = false;
    std::string szFileName = "";
    std::string szTriple = "";
    
    if (!szOptions)
        szOptions = "";

    bool res = true;

    ArgListType UnrecognizedArgs;
    ArgListType RecognizedArgs;
    size_t UnrecognizedArgsLength = 0;

    if (NULL == pList)
    {
        pList = &RecognizedArgs;
    }

    if (NULL == pBEArgList)
    {
        pBEArgList = &RecognizedArgs;
    }

    // Parse the build options - handle the ones we understand and pass the
    // rest into 'ignored'. Use " quoting to accept token with whitespace, but
    // don't use escaping (since we only need path tokens).
    //    
    vector<string> opts = quoted_tokenize(szOptions, " \t", '"', '\x00');
    vector<string>::const_iterator opt_i = opts.begin();
    while (opt_i != opts.end()) {
        if (*opt_i == "-g") {
            pList->push_back(*opt_i);
            bOptDebugInfo = true;
        }
        else if (*opt_i == "-profiling")  {
            // Pass -g on to clang to make it generate debug info
            pList->push_back("-g");
            bOptProfiling = true;
        }
        else if (*opt_i == "-w") {
            pList->push_back(*opt_i);
        }
        else if (opt_i->find("-D") == 0 || opt_i->find("-I") == 0) {
            if (opt_i->length() == 2) {
                // Definition is separated from the flag, so grab it from the
                // next token
                //
                string flag = *opt_i;
                if (++opt_i != opts.end()) {
                    pList->push_back(flag);
                    pList->push_back(*opt_i);
                }
                else {
                    // Check compile options should prevent this case
                    UnrecognizedArgs.push_back(flag);
                    UnrecognizedArgsLength += flag.length() + 1;
                    res = false;
                    continue;
                }
            }
            else {
                // Definition is attached to the flag, so pass it as is
                //
                pList->push_back(*opt_i);
            }
        }
        else if (opt_i->find("-dump-opt-llvm=") == 0) 
        {
            // Dump file must be attached to the flag, but we ignore it for now
        }
        else if (opt_i->find("-dump-opt-asm=") == 0)
        {
            // Dump file must be attached to the flag, but we ignore it for now
        }
        else if (opt_i->find(APFLevelOptionName) == 0)
        {
            const size_t AfpLen = APFLevelOptionName.size();
            // expecting only one digit after option name
            if (opt_i->length() != AfpLen + 1) {
              res = false;
              continue;
            }
            // set auto-prefetching level. should be one digit number.
            int val = opt_i->at(AfpLen) - '0';
            if (val < APFLEVEL_MIN || val > APFLEVEL_MAX) {
                string flag = *opt_i;
                UnrecognizedArgs.push_back(flag);
                UnrecognizedArgsLength += flag.length() + 1;
                res = false;
            } else {
               pBEArgList->push_back(*opt_i); 
            }
        }
        else if (*opt_i == "-s") {
            // Expect the file name as the next token
            //
            string flag = *opt_i;
            if (++opt_i != opts.end()) {
                szFileName = *opt_i;
                // Normalize path to contain forward slashes
                replace(
                    szFileName.begin(), 
                    szFileName.end(), 
                    '\\', '/');

                // On Windows only, normalize the filename to lowercase, since
                // LLVM saves buffer names in a case-sensitive manner, while
                // other Windows tools don't.
                //
#ifdef _WIN32
                transform(
                    szFileName.begin(),
                    szFileName.end(),
                    szFileName.begin(),
                    ::tolower);
#endif

                pList->push_back("-main-file-name");
                pList->push_back(szFileName);
            }
            else {
                UnrecognizedArgs.push_back(flag);
                UnrecognizedArgsLength += flag.length() + 1;
                res = false;
                continue;
            }
        }
        else if (*opt_i == "-Werror") {
            pList->push_back(*opt_i);
        }
        else if (*opt_i == "-cl-single-precision-constant") {
            pList->push_back("-cl-single-precision-constant");
        }
        else if (*opt_i == "-cl-denorms-are-zero") {
            pList->push_back("-cl-denorms-are-zero");
            bDenormsAreZeros = true;
        }
        else if (*opt_i == "-cl-fp32-correctly-rounded-divide-sqrt") {
            pList->push_back("-cl-fp32-correctly-rounded-divide-sqrt");
        }
        else if (*opt_i == "-cl-opt-disable") {
            pList->push_back("-cl-opt-disable");
            bOptDisable = true;
        }
        else if (*opt_i == "-cl-mad-enable") {
            pList->push_back("-cl-mad-enable");
        }
        else if (*opt_i == "-cl-no-signed-zeros") {
            pList->push_back("-cl-no-signed-zeros");
        }
        else if (*opt_i == "-cl-unsafe-math-optimizations") {
            pList->push_back("-cl-unsafe-math-optimizations");
        }
        else if (*opt_i == "-cl-finite-math-only") {
            pList->push_back("-cl-finite-math-only");
                  pList->push_back("-D");
            pList->push_back("__FINITE_MATH_ONLY__=1");
        }
        else if (*opt_i == "-cl-fast-relaxed-math") {
            pList->push_back("-cl-fast-relaxed-math");
                  pList->push_back("-D");
            pList->push_back("__FAST_RELAXED_MATH__=1");
            pBEArgList->push_back("-cl-fast-relaxed-math");
            bFastRelaxedMath = true;
        }
        else if (*opt_i == "-cl-kernel-arg-info") {
            pList->push_back("-cl-kernel-arg-info");
        }
        else if (*opt_i == "-cl-std=CL1.1") {
            iCLStdSet = 110;
            pList->push_back("-cl-std=CL1.1");
            pList->push_back("-D");
            pList->push_back("__OPENCL_C_VERSION__=110");
            pBEArgList->push_back("-cl-std=CL1.1");
        }
        else if (*opt_i == "-cl-std=CL1.2") {
            iCLStdSet = 120;
            pList->push_back("-cl-std=CL1.2");
            pList->push_back("-D");
            pList->push_back("__OPENCL_C_VERSION__=120");
            pBEArgList->push_back("-cl-std=CL1.2");
        }
        else if (config.GetOpenCLVersion() == OPENCL_VERSION_2_0 && *opt_i == "-cl-std=CL2.0") {
            iCLStdSet = 200;
            pList->push_back("-cl-std=CL2.0");
            pList->push_back("-D");
            pList->push_back("__OPENCL_C_VERSION__=200");
            pBEArgList->push_back("-cl-std=CL2.0");
        }
        else if (*opt_i == "-triple") {
            // Expect the target triple as the next token
            //
            if (++opt_i != opts.end()) {
                szTriple = *opt_i;
            }
        }
        else if (*opt_i == "-target-triple") {
            if (++opt_i != opts.end()) {
              // We use 'target-triple' instead of 'triple', since the triple is
              // SPIR by-definition.
              pBEArgList->push_back("-target-triple");
              pBEArgList->push_back(*opt_i);
            } else {
                res = false;
            }
        }
        else if ((*opt_i == "-std-spir=1.0") || 
                 (*opt_i == "-std-spir=1.2")) {
            //ignore SPIR version flag for now
        }
        else if ((*opt_i == "-x")) {
            if (++opt_i != opts.end()) {
                if (*opt_i != "spir") {
                    UnrecognizedArgs.push_back("-x");
                    UnrecognizedArgsLength += 3;
                    UnrecognizedArgs.push_back(*opt_i);
                    UnrecognizedArgsLength += opt_i->length() + 1;
                }
            } else {
                UnrecognizedArgs.push_back("-x");
                UnrecognizedArgsLength += 3;
            }
        }
        else {
            UnrecognizedArgs.push_back(*opt_i);
            UnrecognizedArgsLength += opt_i->length() + 1;
            res = false;
        }

        ++opt_i;
    }

    if (szUnrecognizedOptions)
    {
        if ( !UnrecognizedArgs.empty() )
        {
            if ( *szUnrecognizedOptions != NULL )
            {
                *szUnrecognizedOptions[0] = '\0';

                STRCAT_S(*szUnrecognizedOptions, UnrecognizedArgsLength, UnrecognizedArgs.front().c_str());
                UnrecognizedArgs.pop_front();

                while(!UnrecognizedArgs.empty())
                {
                    STRCAT_S(*szUnrecognizedOptions, UnrecognizedArgsLength, " ");
                    STRCAT_S(*szUnrecognizedOptions, UnrecognizedArgsLength, UnrecognizedArgs.front().c_str());
                    UnrecognizedArgs.pop_front();
                }
            }
        }
    }

    // Handle DebugInfo and profiling flags
    // Hope this covers all the cases
    if (bOptDebugInfo && szFileName.empty())
    {
        // if we have -g but we didn't get a -s create a unique name
        Twine t(g_uiProgID++);
            
        pList->push_back("-main-file-name");
        pList->push_back(t.str());
    }

    if (bOptProfiling && !bOptDebugInfo && szFileName.empty())
    {
        // We are in profiling mode without a given source name and we didn't 
        // already added a unique name
        Twine t(g_uiProgID++);
            
        pList->push_back("-main-file-name");
        pList->push_back(t.str());
    }

    if (piCLStdSet)
    {
        *piCLStdSet = iCLStdSet;
    }

    if (pbOptDebugInfo)
    {
        *pbOptDebugInfo = bOptDebugInfo;
    }

    if (pbOptProfiling)
    {
        *pbOptProfiling = bOptProfiling;
    }

    if (pbOptDisable)
    {
        *pbOptDisable = bOptDisable;
    }

    if (pbDenormsAreZeros)
    {
        *pbDenormsAreZeros = bDenormsAreZeros;
    }

    if (pbFastRelaxedMath)
    {
        *pbFastRelaxedMath = bFastRelaxedMath;
    }

    if (pszFileName && !szFileName.empty())
    {
        *pszFileName = szFileName;
    }

    if (pszTriple)
    {
        *pszTriple = szTriple;
    }


    return res;
}

bool Intel::OpenCL::ClangFE::ParseLinkOptions(const char* szOptions,
                                              char**      szUnrecognizedOptions,
                                              bool*       pbCreateLibrary,
                                              bool*       pbEnableLinkOptions,
                                              bool*       pbDenormsAreZero,
                                              bool*       pbNoSignedZeroes,
                                              bool*       pbUnsafeMath,
                                              bool*       pbFiniteMath,
                                              bool*       pbFastRelaxedMath)
{
    // Reset options
      bool bCreateLibrary = false;
    bool bEnableLinkOptions = false;
    bool bDenormsAreZero = false;
    bool bNoSignedZeroes = false;
    bool bUnsafeMath = false;
    bool bFiniteMath = false;
    bool bFastRelaxedMath = false;
    
    if (!szOptions)
        szOptions = "";

    bool res = true;

    std::list<std::string> UnrecognizedArgs;
    size_t UnrecognizedArgsLength = 0;

    // Parse the build options - handle the ones we understand and ignore the rest.
    //
    vector<string> opts = quoted_tokenize(szOptions, " \t", '"', '\x00');
    vector<string>::const_iterator opt_i = opts.begin();

    while (opt_i != opts.end()) {
        if (*opt_i == "-s") 
        {
            // Expect the file name as the next token and discard it
            //
            string flag = *opt_i;
            if (++opt_i == opts.end()) 
            {
              UnrecognizedArgs.push_back(flag);
              UnrecognizedArgsLength += flag.length() + 1;
              res = false;
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
        else if (opt_i->find("-dump-opt-llvm=") == 0) 
        {
            // Dump file must be attached to the flag, but we ignore it for now
        }
        else
        {
            UnrecognizedArgs.push_back(*opt_i);
            UnrecognizedArgsLength += opt_i->length() + 1;
            res = false;
        }

        ++opt_i;
    }

    if (szUnrecognizedOptions)
    {
        if ( !UnrecognizedArgs.empty() )
        {
            *szUnrecognizedOptions = new char[UnrecognizedArgsLength];
            *szUnrecognizedOptions[0] = '\0';

            if ( *szUnrecognizedOptions != NULL )
            {
                STRCAT_S(*szUnrecognizedOptions, UnrecognizedArgsLength, UnrecognizedArgs.front().c_str());
                UnrecognizedArgs.pop_front();

                while(!UnrecognizedArgs.empty())
                {
                    STRCAT_S(*szUnrecognizedOptions, UnrecognizedArgsLength, " ");
                    STRCAT_S(*szUnrecognizedOptions, UnrecognizedArgsLength, UnrecognizedArgs.front().c_str());
                    UnrecognizedArgs.pop_front();
                }
            }
        }
    }

    if (pbCreateLibrary)
    {
        *pbCreateLibrary = bCreateLibrary;
    }

    if (pbEnableLinkOptions)
    {
        *pbEnableLinkOptions = bEnableLinkOptions;
    }

    if (pbDenormsAreZero)
    {
        *pbDenormsAreZero = bDenormsAreZero;
    }

    if (pbNoSignedZeroes)
    {
        *pbNoSignedZeroes = bNoSignedZeroes;
    }

    if (pbUnsafeMath)
    {
        *pbUnsafeMath = bUnsafeMath;
    }

    if (pbFiniteMath)
    {
        *pbFiniteMath = bFiniteMath;
    }

    if (pbFastRelaxedMath)
    {
        *pbFastRelaxedMath = bFastRelaxedMath;
    }

    return res;
}
