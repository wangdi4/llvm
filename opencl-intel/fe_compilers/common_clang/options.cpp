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

#include "clang/Driver/OptTable.h"
#include "clang/Driver/Option.h"
#include "clang/Driver/ArgList.h"
#include "clang/Driver/Arg.h"
#include "llvm/ADT/OwningPtr.h"
#include "options.h"
#include <iterator>
#include <sstream>

using namespace clang;
using namespace clang::driver;
using namespace clang::driver::options;

OpenCLArgList::OpenCLArgList(const char* pszOptions)
{
    std::back_insert_iterator<ArgsVector> it( std::back_inserter(m_synthesizedStrings));
    quoted_tokenize(it, pszOptions, " \t", '"', '\x00');

    // need to be careful about the reallocation that could happen in m_synthesizedStrings upon push_back
    for( ArgsVector::const_iterator it = m_synthesizedStrings.begin(), end = m_synthesizedStrings.end(); it != end; ++it )
    {
        m_argStrings.push_back(it->c_str());
    }
    m_uiOriginalArgsCount = m_argStrings.size();
}

OpenCLArgList::~OpenCLArgList()
{
    // An OpenCLArgList always owns its arguments.
    for (iterator it = begin(), ie = end(); it != ie; ++it)
    {
        delete *it;
    }
}

unsigned OpenCLArgList::MakeIndex(llvm::StringRef str) const
{
    unsigned index = m_argStrings.size();

    // Tuck away so we have a reliable const char *.
    m_synthesizedStrings.push_back(str);
    m_argStrings.push_back(m_synthesizedStrings.back().c_str());

    return index;
}

unsigned OpenCLArgList::MakeIndex(llvm::StringRef str0, llvm::StringRef str1) const
{
    unsigned index0 = MakeIndex(str0);
    unsigned index1 = MakeIndex(str1);
    assert(index0 + 1 == index1 && "Unexpected non-consecutive indices!");
    (void) index1;
    return index0;
}

const char *OpenCLArgList::MakeArgString(llvm::StringRef str) const
{
    return getArgString(MakeIndex(str));
}

std::string OpenCLArgList::getFilteredArgs(int id) const
{
    std::stringstream ss;
    for (clang::driver::arg_iterator it = filtered_begin(id), ie = filtered_end(); it != ie; ++it)
    {
        ss << (*it)->getAsString(*this) << ' ';
    }
    return ss.str();
}

OpenCLArgList * OpenCLOptTable::ParseArgs(const char* szOptions,
                                          unsigned &missingArgIndex,
                                          unsigned &missingArgCount) const
{
    llvm::OwningPtr<OpenCLArgList> pArgs(new OpenCLArgList(szOptions));

    // FIXME: Handle '@' args (or at least error on them).

    missingArgIndex = missingArgCount = 0;
    unsigned index = 0, argsCount = pArgs->getNumInputArgStrings();
    while (index < argsCount)
    {
        // Ignore empty arguments (other things may still take them as arguments).
        if (pArgs->getArgString(index)[0] == '\0')
        {
            ++index;
            continue;
        }

        unsigned prev = index;
        Arg *pArg = ParseOneArg(*pArgs, index);
        assert(index > prev && "Parser failed to consume argument.");

        // Check for missing argument error.
        if (!pArg) {
            assert(index >= argsCount && "Unexpected parser error.");
            assert(index - prev - 1 && "No missing arguments!");
            missingArgIndex = prev;
            missingArgCount = index - prev - 1;
            break;
        }

        pArgs->append(pArg);
    }
    return pArgs.take();
}