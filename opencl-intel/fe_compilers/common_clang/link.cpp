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
#include "common_clang.h"
#include "binary_result.h"
#include "options.h"

#include "llvm/ADT/OwningPtr.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Constants.h"
#include "llvm/Linker.h"
#include "llvm/Support/raw_ostream.h"

using namespace Intel::OpenCL::ClangFE;

void CommonClangInitialize();

///
// Options parser for the Link function
//
class ClangLinkOptions
{
public:
    //
    // Parses the link options, ignoring the missing args
    //
    void processOptions(const char* pszOptions)
    {
        unsigned missingArgIndex, missingArgCount;
        m_pArgs.reset( m_optTbl.ParseArgs(pszOptions, missingArgIndex, missingArgCount));
    }

    //
    // Validates the user supplied OpenCL link options
    //
    bool checkOptions(const char* pszOptions, char* pszUnknownOptions, size_t uiUnknownOptionsSize)
    {
        // Parse the arguments.
        unsigned missingArgIndex, missingArgCount;
        m_pArgs.reset( m_optTbl.ParseArgs(pszOptions, missingArgIndex, missingArgCount));

        // check for options with missing argument error.
        if (missingArgCount)
        {
            std::fill_n(pszUnknownOptions, uiUnknownOptionsSize, '\0');
            std::string missingArg(m_pArgs->getArgString(missingArgIndex));
            missingArg.copy(pszUnknownOptions, uiUnknownOptionsSize-1);
            return false;
        }

        // check for unknown options
        std::string unknownOptions = m_pArgs->getFilteredArgs(OPT_LINK_UNKNOWN);
        if( !unknownOptions.empty())
        {
            std::fill_n(pszUnknownOptions, uiUnknownOptionsSize, '\0');
            unknownOptions.copy(pszUnknownOptions, uiUnknownOptionsSize-1);
            return false;
        }

        // we do not support input options
        std::string inputOptions = m_pArgs->getFilteredArgs(OPT_LINK_INPUT);
        if( !inputOptions.empty())
        {
            std::fill_n(pszUnknownOptions, uiUnknownOptionsSize, '\0');
            inputOptions.copy(pszUnknownOptions, uiUnknownOptionsSize-1);
            return false;
        }

        return true;
    }

    bool hasArg( int id ) const
    {
        return m_pArgs->hasArg(id);
    }
private:
    OpenCLLinkOptTable m_optTbl;
    llvm::OwningPtr<OpenCLArgList> m_pArgs;
};

//
// Wrapper around the 'opencl.compiler.options' metadata
// (we can't use the MetaData Utils since it would add additional dependency
//  on MetaData component which is not shared with VPG yet )
//
class SpirOptions
{
public:
    SpirOptions(llvm::Module* pModule):
        m_bDebugInfo(false),
        m_bProfiling(false),
        m_bDisableOpt(false),
        m_bFastRelaxedMath(false),
        m_bDenormsAreZero(false)
    {
        llvm::NamedMDNode* metadata = pModule->getNamedMetadata("opencl.compiler.options");

        if(NULL == metadata)
        {
            return;
        }

        if(metadata->getNumOperands() == 0)
        {
            return;
        }

        llvm::MDNode* flag = metadata->getOperand(0);
        for(uint32_t i =0; flag && (i < flag->getNumOperands()); ++i)
        {
            llvm::MDString* flagName = llvm::dyn_cast<llvm::MDString>(flag->getOperand(i));

            if(flagName->getString() == "-g")
                setDebugInfoFlag(true);
            else if(flagName->getString() == "-profiling")
                setProfiling(true);
            else if(flagName->getString() == "-cl-opt-disable")
                setDisableOpt(true);
            else if(flagName->getString() == "-cl-fast-relaxed-math")
                setFastRelaxedMath(true);
            else if(flagName->getString() == "-cl-denorms-are-zero")
                setDenormsAreZero(true);
            else
                addOption(flagName->getString());
        }
    }

    bool getDebugInfoFlag() const  { return m_bDebugInfo; }
    bool getProfiling() const      { return m_bProfiling; }
    bool getDisableOpt() const     { return m_bDisableOpt; }
    bool getFastRelaxedMath() const{ return m_bFastRelaxedMath; }
    bool getDenormsAreZero() const { return m_bDenormsAreZero; }

    void setDebugInfoFlag(bool value)
    {
        setBoolFlag(value, "-g", m_bDebugInfo);
    }

    void setProfiling(bool value)
    {
        setBoolFlag(value, "-profiling", m_bProfiling);
    }

    void setDisableOpt(bool value)
    {
        setBoolFlag(value, "-cl-opt-disable", m_bDisableOpt);
    }

    void setFastRelaxedMath(bool value)
    {
        setBoolFlag(value, "-cl-fast-relaxed-math", m_bFastRelaxedMath);
    }

    void setDenormsAreZero(bool value)
    {
        setBoolFlag(value, "-cl-denorms-are-zero", m_bDenormsAreZero);
    }

    void addOption( const std::string& option )
    {
        if( std::find(m_options.begin(), m_options.end(), option) == m_options.end() )
        {
            m_options.push_back(option);
        }
    }

    std::list<std::string>::const_iterator beginOptions() const
    {
        return m_options.begin();
    }

    std::list<std::string>::const_iterator endOptions() const
    {
        return m_options.end();
    }

    void save( llvm::Module *pModule)
    {
        // Add build options
        llvm::NamedMDNode *pRoot = pModule->getOrInsertNamedMetadata("opencl.compiler.options");
        pRoot->dropAllReferences();

        llvm::SmallVector<llvm::Value*,5> values;
        for (std::list<std::string>::const_iterator it = m_options.begin(), e = m_options.end(); it != e ; ++it)
        {
            values.push_back(llvm::MDString::get(pModule->getContext(), *it));
        }
        pRoot->addOperand(llvm::MDNode::get(pModule->getContext(), values));
    }

private:
    void setBoolFlag( bool value, const char* flag, bool& dest )
    {
        if( value == dest)
            return;

        dest = value;

        if( !value )
            m_options.remove(std::string(flag));
        else
            m_options.push_back(std::string(flag));
    }

private:
    bool m_bDebugInfo;
    bool m_bProfiling;
    bool m_bDisableOpt;
    bool m_bFastRelaxedMath;
    bool m_bDenormsAreZero;
    std::list<std::string> m_options;
};

//
// Helper class for 'opencl.compiler.options' metadata conflict resolution during link
//
class MetadataLinker
{
public:
    MetadataLinker(const ClangLinkOptions& linkOptions, llvm::Module* pModule):
        m_effectiveOptions(pModule),
        m_bDenormsAreZeroLinkOpt(false),
        m_bNoSignedZerosLinkOpt(false),
        m_bUnsafeMathLinkOpt(false),
        m_bFiniteMathOnlyLinkOpt(false),
        m_bFastRelaxedMathLinkOpt(false)
    {
        // get the options specified during link
        m_bEnableLinkOptions      = linkOptions.hasArg(OPT_LINK_enable_link_options);
        m_bCreateLibrary          = linkOptions.hasArg(OPT_LINK_create_library);
        m_bDenormsAreZeroLinkOpt  = linkOptions.hasArg(OPT_LINK_cl_denorms_are_zero);
        m_bNoSignedZerosLinkOpt   = linkOptions.hasArg(OPT_LINK_cl_no_signed_zeroes);
        m_bUnsafeMathLinkOpt      = linkOptions.hasArg(OPT_LINK_cl_unsafe_math_optimizations);
        m_bFiniteMathOnlyLinkOpt  = linkOptions.hasArg(OPT_LINK_cl_finite_math_only);
        m_bFastRelaxedMathLinkOpt = linkOptions.hasArg(OPT_LINK_cl_fast_relaxed_math);
    }

    void Link(llvm::Module* pModule)
    {
        // do NOT take the link options overrides into account yet
        SpirOptions options(pModule);

        m_effectiveOptions.setDebugInfoFlag( m_effectiveOptions.getDebugInfoFlag() & options.getDebugInfoFlag());
        m_effectiveOptions.setProfiling( m_effectiveOptions.getProfiling() & options.getProfiling());
        m_effectiveOptions.setDisableOpt( m_effectiveOptions.getDisableOpt() | options.getDisableOpt());
        m_effectiveOptions.setFastRelaxedMath( m_effectiveOptions.getFastRelaxedMath() & options.getFastRelaxedMath());
        m_effectiveOptions.setDenormsAreZero( m_effectiveOptions.getDenormsAreZero() & options.getDenormsAreZero());

        for(std::list<std::string>::const_iterator it = options.beginOptions(), ie = options.endOptions(); it != ie; ++it)
        {
            m_effectiveOptions.addOption(*it);
        }
    }

    void Save(llvm::Module* pModule)
    {
        // overwrite the known options if such were specified during link
        if( m_bEnableLinkOptions && m_bCreateLibrary)
        {
            if( m_bDenormsAreZeroLinkOpt)
                m_effectiveOptions.setDenormsAreZero(true);
            if( m_bFastRelaxedMathLinkOpt)
                m_effectiveOptions.setFastRelaxedMath(true);
        }
        m_effectiveOptions.save(pModule);
    }

private:
    // effective flags stored in the module
    SpirOptions m_effectiveOptions;
    // flags supplied by link options
    bool m_bEnableLinkOptions;
    bool m_bCreateLibrary;
    // flags supplied by link options
    bool m_bDenormsAreZeroLinkOpt;
    bool m_bNoSignedZerosLinkOpt;
    bool m_bUnsafeMathLinkOpt;
    bool m_bFiniteMathOnlyLinkOpt;
    bool m_bFastRelaxedMathLinkOpt;
};

OCLFEBinaryResult* LinkInternal(const void**    pInputBinaries,
                                unsigned int    uiNumBinaries,
                                const size_t*   puiBinariesSizes,
                                const char*     pszOptions)
{
    // Lazy initialization
    CommonClangInitialize();

    llvm::OwningPtr<OCLFEBinaryResult> pResult;

    try
    {
        pResult.reset(new OCLFEBinaryResult());

        if (0 == uiNumBinaries)
        {
            return pResult.take();
        }

        // Parse options
        ClangLinkOptions optionsParser;
        optionsParser.processOptions(pszOptions);
        IR_TYPE binaryType =
            optionsParser.hasArg(OPT_LINK_create_library)? IR_TYPE_LIBRARY : IR_TYPE_EXECUTABLE;

        // Prepare the LLVM Context
        std::string sError;
        llvm::OwningPtr<llvm::LLVMContext> context( new llvm::LLVMContext() );

        // Initialize the module with the first binary
        llvm::StringRef strInputBuff((const char*)pInputBinaries[0], puiBinariesSizes[0]);
        llvm::OwningPtr<llvm::MemoryBuffer> pBinBuff( llvm::MemoryBuffer::getMemBuffer( strInputBuff, "" ,false));
        llvm::OwningPtr<llvm::Module> composite(ParseBitcodeFile(pBinBuff.get(), *context, &sError));

        if (!composite)
        {
            throw sError;
        }

        // Now go over the rest of the binaries and link them.
        MetadataLinker mdlinker(optionsParser, composite.get());
        for (unsigned int i = 1; i < uiNumBinaries; ++i)
        {
            llvm::OwningPtr<llvm::MemoryBuffer> pBinBuff(llvm::MemoryBuffer::getMemBuffer(llvm::StringRef((const char*)pInputBinaries[i], puiBinariesSizes[i]),"", false));
            llvm::OwningPtr<llvm::Module> module(ParseBitcodeFile(pBinBuff.get(), *context, &sError));
            mdlinker.Link(module.get());

            if (!module)
            {
                throw sError;
            }

            if( llvm::Linker::LinkModules(composite.get(), module.get(), llvm::Linker::DestroySource, &sError))
            {
                // apparently LinkModules returns true on failure and false on success
                throw sError;
            }
        }
        mdlinker.Save(composite.get());
        pResult->setIRType(binaryType);

        llvm::raw_svector_ostream ir_ostream(pResult->getIRBufferRef());
        llvm::WriteBitcodeToFile(composite.get(), ir_ostream);
        ir_ostream.flush();

        return pResult.take();
    }
    catch( std::bad_alloc& e)
    {
        if( pResult )
        {
            pResult->getLogRef()="Internal error";
            pResult->setResult(CL_OUT_OF_HOST_MEMORY);
            return pResult.take();
        }
        throw e;
    }
    catch( std::string &err)
    {
        pResult->setLog(err);
        pResult->setResult(CL_LINK_PROGRAM_FAILURE);
        return pResult.take();
    }
}

extern "C" CC_DLL_EXPORT int Link(const void**    pInputBinaries,
                                  unsigned int    uiNumBinaries,
                                  const size_t*   puiBinariesSizes,
                                  const char*     pszOptions,
                                  IOCLFEBinaryResult** pBinaryResult )
{
    llvm::OwningPtr<OCLFEBinaryResult> pResult;
    int resultCode = CL_SUCCESS;
    try{
        pResult.reset( LinkInternal(pInputBinaries,
            uiNumBinaries,
            puiBinariesSizes,
            pszOptions));
        resultCode = pResult->getResult();
    }
    catch( std::bad_alloc& )
    {
        resultCode = CL_OUT_OF_HOST_MEMORY;
    }

    if (pBinaryResult)
    {
        *pBinaryResult = pResult.take();
    }
    return resultCode;
}

extern "C" CC_DLL_EXPORT bool CheckLinkOptions(const char* pszOptions,
                                               char* pszUnknownOptions,
                                               size_t uiUnknownOptionsSize)
{
    try
    {
        ClangLinkOptions optionsParser;
        return optionsParser.checkOptions(pszOptions, pszUnknownOptions, uiUnknownOptionsSize);
    }
    catch( std::bad_alloc& )
    {
        if( pszUnknownOptions && uiUnknownOptionsSize > 0 )
        {
            std::fill_n(pszUnknownOptions, uiUnknownOptionsSize, '\0');
        }
        return false;
    }
}
