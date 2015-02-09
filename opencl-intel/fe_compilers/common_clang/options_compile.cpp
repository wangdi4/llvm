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

#include "options.h"
#include "common_clang.h"
#include "llvm/ADT/OwningPtr.h"
#include "clang/Driver/OptTable.h"
#include "clang/Driver/Option.h"
#include "clang/Driver/ArgList.h"
#include "clang/Driver/Arg.h"
#include <sstream>

#define PREFIX(NAME, VALUE) const char *const NAME[] = VALUE;
#define OPTION(PREFIX, NAME, ID, KIND, GROUP, ALIAS, FLAGS, PARAM, \
    HELPTEXT, METAVAR)
#include "opencl_clang_options.inc"
#undef OPTION
#undef PREFIX

using namespace clang::driver::options;

static const OptTable::Info ClangOptionsInfoTable[] = {
#define PREFIX(NAME, VALUE)
#define OPTION(PREFIX, NAME, ID, KIND, GROUP, ALIAS, FLAGS, PARAM, HELPTEXT, METAVAR) \
    { PREFIX, NAME, HELPTEXT, METAVAR, OPT_COMPILE_##ID, clang::driver::Option::KIND##Class, PARAM, FLAGS, OPT_COMPILE_##GROUP, OPT_COMPILE_##ALIAS },
#include "opencl_clang_options.inc"
};

OpenCLCompileOptTable::OpenCLCompileOptTable():
    OpenCLOptTable(ClangOptionsInfoTable, sizeof(ClangOptionsInfoTable) / sizeof(ClangOptionsInfoTable[0]))
{
}

int EffectiveOptionsFilter::s_progID = 1;

///
// Options filter that validates the opencl used options
//
std::string EffectiveOptionsFilter::processOptions(const OpenCLArgList& args, const char* pszOptionsEx, ArgsVector& effectiveArgs )
{
    // Reset args
    int  iCLStdSet = 0;
    std::string szTriple;
    std::string sourceName(llvm::Twine(s_progID++).str());

    for (OpenCLArgList::const_iterator it = args.begin(), ie = args.end(); it != ie; ++it)
    {
        switch ((*it)->getOption().getID())
        {
        case OPT_COMPILE_w:
        case OPT_COMPILE_D:
        case OPT_COMPILE_I:
        case OPT_COMPILE_Werror:
        case OPT_COMPILE_cl_single_precision_constant:
        case OPT_COMPILE_cl_denorms_are_zero:
        case OPT_COMPILE_cl_fp32_correctly_rounded_divide_sqrt:
        case OPT_COMPILE_cl_opt_disable:
        case OPT_COMPILE_cl_mad_enable:
        case OPT_COMPILE_cl_no_signed_zeros:
        case OPT_COMPILE_cl_unsafe_math_optimizations:
        case OPT_COMPILE_g_Flag:
            effectiveArgs.push_back((*it)->getAsString(args));
            break;
        case OPT_COMPILE_profiling:
            effectiveArgs.push_back("-g");
            break;
        case OPT_COMPILE_s:
            sourceName = (*it)->getValue();
            // Normalize path to contain forward slashes
            replace(
                sourceName.begin(),
                sourceName.end(),
                '\\', '/');

#ifdef _WIN32
            // On Windows only, normalize the file name to lower case, since
            // LLVM saves buffer names in a case-sensitive manner, while
            // other Windows tools don't.
            //
            std::transform(
                sourceName.begin(),
                sourceName.end(),
                sourceName.begin(),
                ::tolower);
#endif
            effectiveArgs.push_back("-main-file-name");
            effectiveArgs.push_back(sourceName);
            break;
        case OPT_COMPILE_cl_finite_math_only:
            effectiveArgs.push_back((*it)->getAsString(args));
            effectiveArgs.push_back("-D");
            effectiveArgs.push_back("__FINITE_MATH_ONLY__=1");
            break;
        case OPT_COMPILE_cl_fast_relaxed_math:
            effectiveArgs.push_back((*it)->getAsString(args));
            effectiveArgs.push_back("-D");
            effectiveArgs.push_back("__FAST_RELAXED_MATH__=1");
            break;
        case OPT_COMPILE_cl_std_CL1_1:
            iCLStdSet = 110;
            effectiveArgs.push_back((*it)->getAsString(args));
            effectiveArgs.push_back("-D");
            effectiveArgs.push_back("__OPENCL_C_VERSION__=110");
            break;
        case OPT_COMPILE_cl_std_CL1_2:
            iCLStdSet = 120;
            effectiveArgs.push_back((*it)->getAsString(args));
            effectiveArgs.push_back("-D");
            effectiveArgs.push_back("__OPENCL_C_VERSION__=120");
            break;
        case OPT_COMPILE_cl_std_CL2_0:
            iCLStdSet = 200;
            effectiveArgs.push_back((*it)->getAsString(args));
            effectiveArgs.push_back("-D");
            effectiveArgs.push_back("__OPENCL_C_VERSION__=200");
            break;
        case OPT_COMPILE_triple:
            szTriple = (*it)->getValue();
            break;
        case OPT_COMPILE_cl_uniform_work_group_size:
        case OPT_COMPILE_target_triple:
        case OPT_COMPILE_spir_std_1_0:
        case OPT_COMPILE_spir_std_1_2:  // ignore for now
        case OPT_COMPILE_cl_kernel_arg_info:  // For SPIR, we always create kernel arg info, so ignoring it here
        case OPT_COMPILE_dump_opt_asm:
        case OPT_COMPILE_dump_opt_llvm: // Dump file must be attached to the flag, but we ignore it for now
        case OPT_COMPILE_auto_prefetch_level0:
        case OPT_COMPILE_auto_prefetch_level1:
        case OPT_COMPILE_auto_prefetch_level2:
        case OPT_COMPILE_auto_prefetch_level3:
            break;
        case OPT_COMPILE_pch_cpu:
            effectiveArgs.push_back("-include");
            effectiveArgs.push_back("header_with_defs_cpu.h");
            effectiveArgs.push_back("-include-pch");
            effectiveArgs.push_back("PCH_CPU.pch");
            break;
        case OPT_COMPILE_pch_gpu:
            break;
        case OPT_COMPILE_x:
            // ensure that the value is spir
            assert((*it)->getValue() == std::string("spir"));
            //TODO: move the validation of the value to the check section of the option processing to be reported as an unknown option
            break;
            //Just ignore the unknown options ( they will be listed in the unknown list inside the ArgsList anyway)
            //The below assert is usable for manual debugging only
            //default:
            //assert(false && "some unknown argument");
        }
    }

    if((args.hasArg(OPT_COMPILE_g_Flag) || args.hasArg(OPT_COMPILE_profiling)) && sourceName.empty())
    {
        // Handle DebugInfo and profiling flags
        // Hope this covers all the cases
        // if we have -g but we didn't get a -s create a unique name
        effectiveArgs.push_back("-main-file-name");
        effectiveArgs.push_back(sourceName);
    }

    if(!iCLStdSet)
    {
        effectiveArgs.push_back("-cl-std=CL1.2");
        effectiveArgs.push_back("-D");
        effectiveArgs.push_back("__OPENCL_C_VERSION__=120");
        iCLStdSet = 120;
    }

    //add the external options verbatim
    std::back_insert_iterator<ArgsVector> it( std::back_inserter(effectiveArgs));
    quoted_tokenize(it, pszOptionsEx, " \t", '"', '\x00');

    effectiveArgs.push_back("-D");
    effectiveArgs.push_back("__OPENCL_VERSION__="+m_opencl_ver);
    effectiveArgs.push_back("-x");
    effectiveArgs.push_back("cl");
    effectiveArgs.push_back("-cl-kernel-arg-info");
    effectiveArgs.push_back("-O0"); // Don't optimize in the frontend
    effectiveArgs.push_back("-fno-validate-pch");

    if( !strstr(pszOptionsEx, "-emit" ) && !strstr(pszOptionsEx, "-S") )
    {
        effectiveArgs.push_back("-emit-llvm-bc");
    }

    effectiveArgs.push_back("-triple");
    if( szTriple.empty())
    {
#if defined(_WIN64) || defined(__x86_64__) || defined(_M_AMD64) || defined (_M_X64)
        effectiveArgs.push_back("spir64-unknown-unknown");
#elif defined(_WIN32) || defined(i386) || defined(__i386__) || defined(__x86__) || defined(__ANDROID__)
        effectiveArgs.push_back("spir-unknown-unknown");
#else
#error "Can't define target triple: unknown architecture."
#endif
    }
    else
    {
        effectiveArgs.push_back(szTriple);
    }

    return sourceName;
}

///
// Options filter that is responsible for building the '-cl-spir-compile-options' value
//
void SPIROptionsFilter::processOptions(const OpenCLArgList& args, ArgsVector& effectiveArgs)
{
    std::stringstream spirArgs;

    for (OpenCLArgList::const_iterator it = args.begin(), ie = args.end(); it != ie; ++it)
    {
        switch ((*it)->getOption().getID())
        {
        case OPT_COMPILE_w:
        case OPT_COMPILE_D:
        case OPT_COMPILE_I:
        case OPT_COMPILE_auto_prefetch_level0:
        case OPT_COMPILE_auto_prefetch_level1:
        case OPT_COMPILE_auto_prefetch_level2:
        case OPT_COMPILE_auto_prefetch_level3:
        case OPT_COMPILE_Werror:
        case OPT_COMPILE_cl_single_precision_constant:
        case OPT_COMPILE_cl_denorms_are_zero:
        case OPT_COMPILE_cl_fp32_correctly_rounded_divide_sqrt:
        case OPT_COMPILE_cl_opt_disable:
        case OPT_COMPILE_cl_mad_enable:
        case OPT_COMPILE_cl_no_signed_zeros:
        case OPT_COMPILE_cl_unsafe_math_optimizations:
        case OPT_COMPILE_g_Flag:
        case OPT_COMPILE_cl_finite_math_only:
        case OPT_COMPILE_cl_fast_relaxed_math:
        case OPT_COMPILE_cl_std_CL1_1:
        case OPT_COMPILE_cl_std_CL1_2:
        case OPT_COMPILE_cl_std_CL2_0:
        case OPT_COMPILE_cl_uniform_work_group_size:
        case OPT_COMPILE_spir_std_1_0:
        case OPT_COMPILE_spir_std_1_2:  // ignore for now
        case OPT_COMPILE_cl_kernel_arg_info:  // For SPIR, we always create kernel arg info, so ignoring it here
        case OPT_COMPILE_target_triple:
        case OPT_COMPILE_x:
            spirArgs << (*it)->getAsString(args) << ' ';
            break;
        case OPT_COMPILE_profiling:
            break;
            //Just ignore the unknown options. The below assert is used for manual debugging only.
            //default:
            //assert(false && "some unknown argument");
        }
    }

    std::string opt = spirArgs.str();
    if(!opt.empty())
    {
        effectiveArgs.push_back("-cl-spir-compile-options");
        effectiveArgs.push_back("\"" + opt + "\"");
    }
}

SupportedPragmasFilter::SupportedPragmasFilter(const char* pszDeviceSuportedExtentions)
{
    llvm::StringRef extStr(NULL == pszDeviceSuportedExtentions ? "" : pszDeviceSuportedExtentions);
    llvm::SmallVector<llvm::StringRef, 10> extVector;

    extStr.split(extVector, " ", -1, false);
    llvm::SmallVectorImpl<llvm::StringRef>::const_iterator it = extVector.begin(), e = extVector.end();

    for(; it != e; ++it)
    {
#undef OPENCLEXT_1_2
#define OPENCLEXT(nm) if( *it == #nm ) m_options.nm = 1;
#include "clang/Basic/OpenCLExtensions.def"
    }
}

void SupportedPragmasFilter::processOptions(const OpenCLArgList& args, ArgsVector& effectiveArgs )
{
#undef OPENCLEXT_1_2
#define OPENCLEXT(nm) if( m_options.nm == 1 ) effectiveArgs.push_back("-D"#nm);
#include "clang/Basic/OpenCLExtensions.def"
}

void CompileOptionsParser::processOptions(const char* pszOptions, const char* pszOptionsEx)
{
    //parse options
    unsigned missingArgIndex, missingArgCount;
    llvm::OwningPtr<OpenCLArgList> pArgs( m_optTbl.ParseArgs(pszOptions, missingArgIndex, missingArgCount));

    //post process logic
    m_sourceName = m_commonFilter.processOptions(*pArgs, pszOptionsEx, m_effectiveArgs);
    m_spirFilter.processOptions(*pArgs, m_effectiveArgs);
    m_pragmasFilter.processOptions(*pArgs, m_effectiveArgs);

    //build the raw options array
    for( ArgsVector::iterator it = m_effectiveArgs.begin(), end = m_effectiveArgs.end();
        it != end;
        ++it)
    {
        m_effectiveArgsRaw.push_back(it->c_str());
    }
}

bool CompileOptionsParser::checkOptions(const char* pszOptions, char* pszUnknownOptions, size_t uiUnknownOptionsSize)
{
    // Parse the arguments.
    unsigned missingArgIndex, missingArgCount;
    llvm::OwningPtr<OpenCLArgList> pArgs( m_optTbl.ParseArgs(pszOptions, missingArgIndex, missingArgCount));

    // Check for missing argument error.
    if (missingArgCount)
    {
        std::fill_n(pszUnknownOptions, uiUnknownOptionsSize, '\0');
        std::string missingArg(pArgs->getArgString(missingArgIndex));
        missingArg.copy(pszUnknownOptions, uiUnknownOptionsSize-1);
        return false;
    }

    std::string unknownOptions = pArgs->getFilteredArgs(OPT_COMPILE_UNKNOWN);
    if( !unknownOptions.empty())
    {
        std::fill_n(pszUnknownOptions, uiUnknownOptionsSize, '\0');
        unknownOptions.copy(pszUnknownOptions, uiUnknownOptionsSize-1);
        return false;
    }

    //we do not support input options
    std::string inputOptions = pArgs->getFilteredArgs(OPT_COMPILE_INPUT);
    if( !inputOptions.empty())
    {
        std::fill_n(pszUnknownOptions, uiUnknownOptionsSize, '\0');
        inputOptions.copy(pszUnknownOptions, uiUnknownOptionsSize-1);
        return false;
    }

    return true;
}

std::string CompileOptionsParser::getEffectiveOptionsAsString() const
{
    std::stringstream ss;

    const char* const* it = beginArgs();
    const char* const* ie = endArgs();

    for( ; it != ie; ++it )
    {
        ss << *it << " ";
    }
    return ss.str();
}

extern "C" CC_DLL_EXPORT bool CheckCompileOptions(const char* pszOptions,
                                                  char* pszUnknownOptions,
                                                  size_t uiUnknownOptionsSize)
{
    try
    {
        CompileOptionsParser optionsParser(NULL, "200");
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
