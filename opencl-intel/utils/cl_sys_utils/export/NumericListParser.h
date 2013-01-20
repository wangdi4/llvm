// Copyright (c) 2006-2007 Intel Corporation
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

#ifndef _NumericListParser_h_
#define _NumericListParser_h_

#include <string>
#include <vector>

/* NumericListParser parses numeric lists with ranges and steps support:
 * List of numbers (comma separated): 3,5,7
 * Or ranges: 5-7
 * And combinations of them: 3,7,12-15
 * Step value can also be specified (step value of 1 is implicit if not stated):
 * 3,7,15-20:2,57
 * (The last example will give the std::vector 3,7,15,17,19 and 57).
 *
 * Delimiters can be commas or spaces. Spaces can be used as either delimiters
 * But can also be used as white spaces for example 5 - 7 9 will properly
 * create the list: 5,6,7,9
 *
 * Supports also negative values (only when specifically requesting).
 * When using negative values, range should be specified with : (e.g. -5:-1:2)
*/

//#include <cl_utils.h>

namespace Intel { namespace OpenCL { namespace Tools {
 
class NumericListParser {
public:
    NumericListParser(bool bSigned = false);
    NumericListParser(const std::string& sList, bool bSigned = false);
    void Init(const std::string& sList, bool bSigned = false);

    bool Valid() const;
    void CheckBoundaries(int MaxValid, int MinValid) const;
    const std::string& GetError() const;
    const std::vector<int>& GetVector() const;

    const std::string& GetString() const;

    // return string with all numbers, ranged already parsed, and comma separated
    std::string GetStringRepresentation () const;

private: // methods

    void Parse();
    bool ParseToken(const std::string& sToken);
    bool ParseSignedToken(const std::string& sToken);
    bool ParseUnsignedToken(const std::string& sToken);

    void AddRequiredCommas();

private: // members
    std::string mInputList;
    bool mbValid;
    std::string msError;
    std::vector<int> mvNumbers;
    bool mbSigned;
};

}}}
#endif // _NumericListParser_h_
