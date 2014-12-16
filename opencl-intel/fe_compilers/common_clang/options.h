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

//
// Defines the common structures for both compile and link options parsing
//

#ifndef COMMON_CLANG_OPTIONS_H
#define COMMON_CLANG_OPTIONS_H

#include <list>
#include "clang/Driver/OptTable.h"
#include "clang/Driver/ArgList.h"
#include "clang/Basic/LangOptions.h"
#include "llvm/ADT/StringRef.h"

enum COMPILE_OPT_ID {
    OPT_COMPILE_INVALID = 0, // This is not an option ID.
#define PREFIX(NAME, VALUE)
#define OPTION(PREFIX, NAME, ID, KIND, GROUP, ALIAS, FLAGS, PARAM, HELPTEXT, METAVAR) OPT_COMPILE_##ID,
#include "opencl_clang_options.inc"
    OPT_COMPILE_LAST_OPTION
#undef OPTION
#undef PREFIX
};

enum LINK_OPT_ID {
    OPT_LINK_INVALID = 0, // This is not an option ID.
#define PREFIX(NAME, VALUE)
#define OPTION(PREFIX, NAME, ID, KIND, GROUP, ALIAS, FLAGS, PARAM, HELPTEXT, METAVAR) OPT_LINK_##ID,
#include "opencl_link_options.inc"
    OPT_LINK_LAST_OPTION
#undef OPTION
#undef PREFIX
};

using namespace clang::driver;
typedef std::list<std::string> ArgsVector;

//
// Extend the clang ArgList to allow the
// argument parsing from single string.
//
// Originally clang::ArgList allowed only parsing
// or array of options string
//
class OpenCLArgList : public ArgList
{
public:
    OpenCLArgList(const char* pszOptions);
    ~OpenCLArgList();

    virtual const char *getArgString(unsigned index) const
    {
        return m_argStrings[index];
    }

    virtual unsigned getNumInputArgStrings() const
    {
        return m_uiOriginalArgsCount;
    }

    std::string getFilteredArgs(int id) const;

public:
    /// MakeIndex - Get an index for the given string(s).
    unsigned MakeIndex(llvm::StringRef str) const;
    unsigned MakeIndex(llvm::StringRef str0, llvm::StringRef str1) const;

    virtual const char *MakeArgString(llvm::StringRef str) const;

private:
    /// List of argument strings used by the contained Args.
    ///
    /// This is mutable since we treat the ArgList as being the list
    /// of Args, and allow routines to add new strings (to have a
    /// convenient place to store the memory) via MakeIndex.
    mutable ArgStringList m_argStrings;

    /// Strings for synthesized arguments.
    ///
    /// This is mutable since we treat the ArgList as being the list
    /// of Args, and allow routines to add new strings (to have a
    /// convenient place to store the memory) via MakeIndex.
    mutable std::list<std::string> m_synthesizedStrings;

    /// The number of original input argument strings.
    unsigned m_uiOriginalArgsCount;
};

//
// OpenCL specific OptTable
//
class OpenCLOptTable : public OptTable
{
public:
    OpenCLOptTable(const Info *pOptionInfos, unsigned uiNumOptionInfos)
        :OptTable(pOptionInfos, uiNumOptionInfos)
    {}

    OpenCLArgList * ParseArgs(const char* szOptions,
        unsigned &missingArgIndex,
        unsigned &missingArgCount) const;
};

// OpenCL OptTable for compile options
class OpenCLCompileOptTable: public OpenCLOptTable
{
public:
    OpenCLCompileOptTable();
};

// OpenCL OptTable for link options
class OpenCLLinkOptTable: public OpenCLOptTable
{
public:
    OpenCLLinkOptTable();
};

///
// Options filter that validates the opencl used options
//
class EffectiveOptionsFilter
{
public:
    EffectiveOptionsFilter(const char* pszOpenCLVer):
        m_opencl_ver(pszOpenCLVer)
    {
        assert( pszOpenCLVer != NULL);
    }

    std::string processOptions(const OpenCLArgList& args, const char* pszOptionsEx, ArgsVector& effectiveArgs );

private:
    std::string m_opencl_ver;
    static int s_progID;
};

///
// Options filter that is responsible for building the '-cl-spir-compile-options' value
//
class SPIROptionsFilter
{
public:
    void processOptions(const OpenCLArgList& args, ArgsVector& effectiveArgs);
};

///
// Options filter that gathers information of supported opencl extensions
//
class SupportedPragmasFilter
{
public:
    SupportedPragmasFilter(const char* pszDeviceSuportedExtentions);

    void processOptions(const OpenCLArgList& args, ArgsVector& effectiveArgs );

    void getSupportedPragmas( clang::OpenCLOptions& options )
    {
        options = m_options;
    }

private:
    clang::OpenCLOptions m_options;
};

///
// Options parser for the Compile function
//
class CompileOptionsParser
{
public:
    CompileOptionsParser(const char* pszDeviceSupportedExtensions, const char* pszOpenCLVersion):
        m_pragmasFilter(pszDeviceSupportedExtensions),
        m_commonFilter(pszOpenCLVersion)
    {
    }

    //
    // Validates and prepares the effective options to pass to clang upon compilation
    //
    void processOptions(const char* pszOptions, const char* pszOptionsEx);

    //
    // Just validates the user supplied OpenCL compile options
    //
    bool checkOptions(const char* pszOptions, char* pszUnknownOptions, size_t uiUnknownOptionsSize);

    //
    // Fills the supplied OpenCLOptions structure (called after options has been processed)
    //
    void getSupportedPragmas(clang::OpenCLOptions& options)
    {
        m_pragmasFilter.getSupportedPragmas(options);
    }

    //
    // Returns the calculated source name for the input source
    //
    std::string getSourceName() const
    {
        return m_sourceName;
    }

    const char* const* beginArgs() const
    {
        return m_effectiveArgsRaw.data();
    }

    const char* const* endArgs() const
    {
        return beginArgs() + m_effectiveArgsRaw.size();
    }

    std::string getEffectiveOptionsAsString() const;

private:
    OpenCLCompileOptTable  m_optTbl;
    SupportedPragmasFilter m_pragmasFilter;
    EffectiveOptionsFilter m_commonFilter;
    SPIROptionsFilter      m_spirFilter;
    ArgsVector             m_effectiveArgs;
    llvm::SmallVector<const char*, 16> m_effectiveArgsRaw;
    std::string            m_sourceName;
};

// Tokenize a string into tokens separated by any char in 'delims'.
// Support quoting to allow some tokens to contain delimiters, with possible
// escape characters to support quotes inside quotes.
// To disable quoting or escaping, set relevant chars to '\x00'.
//
template<class OutIt>
void quoted_tokenize(OutIt dest, llvm::StringRef str, llvm::StringRef delims, char quote, char escape)
{
    llvm::StringRef::size_type ptr = str.find_first_not_of(delims);

    if (ptr == llvm::StringRef::npos)
        return;

    // pArg state machine, with the following state vars:
    //
    // ptr        - points to the current char in the string
    // is_escaped - is the current char escaped (i.e. was the
    //              previous char = escape, inside a quote)
    // in_quote   - are we in a quote now (i.e. a quote character
    //              appeared without a matching closing quote)
    // tok        - accumulates the current token. once an unquoted
    //              delimiter or end of string is encountered, tok
    //              is added to the return vector and re-initialized
    //
    bool is_escaped = false;
    bool in_quote = false;
    std::string tok;

    do
    {
        char c = str[ptr];

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
        else if (delims.find(c) != llvm::StringRef::npos)
        {
            if (in_quote)
                tok += c;
            else
            {
                *(dest++) = tok;
                tok.clear();
                ptr = str.find_first_not_of(delims, ptr);

                if (ptr == llvm::StringRef::npos)
                    break;
                else
                    --ptr; // will be increased at end of iteration
            }

            is_escaped = false;
        }
        else
            tok += c;

        if (ptr == str.size() - 1)
            *(dest++) = tok;
    }
    while (++ptr < str.size());
}

#endif
