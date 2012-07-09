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

#include <string>
#include <vector>
#include "NumericListParser.h"
#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <cl_utils.h>
#include <ToolsUtils.h>

using namespace std;
using namespace Intel;
using namespace OpenCL;
using namespace Tools;
using namespace Utils;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////////////////////////
// C'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
NumericListParser::NumericListParser(bool bSigned) : 
mbValid(false),
mbSigned(bSigned)
{
}

NumericListParser::NumericListParser(const std::string& sList, bool bSigned) 
{
    Init(sList, bSigned);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Init
///////////////////////////////////////////////////////////////////////////////////////////////////
void NumericListParser::Init(const std::string& sList, bool bSigned) 
{
    mInputList = TrimString(sList);
    mbValid = false;
    mbSigned = bSigned;
    msError = "";
    mvNumbers.clear();
    AddRequiredCommas();
    Parse();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Valid
///////////////////////////////////////////////////////////////////////////////////////////////////
bool NumericListParser::Valid() const 
{
    return mbValid;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// CheckBoundaries
///////////////////////////////////////////////////////////////////////////////////////////////////
void NumericListParser::CheckBoundaries(int MaxValid, int MinValid) const
{
    try
    {
        // Verify that the vector contains numbers within range
        const vector<int>& viNumbers = GetVector();
        for (vector<int>::const_iterator It = viNumbers.begin();
            It != viNumbers.end();
            ++It) 
        {
            if (*It > MaxValid || *It < MinValid) 
            {
                string sErr("Number ");
                sErr += boost::lexical_cast<string>(*It);
                sErr += " is out of valid range.";
                throw sErr;
            }
        }
    } catch (string sError) {
        throw (sError);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// GetError
///////////////////////////////////////////////////////////////////////////////////////////////////
const std::string& NumericListParser::GetError() const 
{
    return msError;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// GetVector
///////////////////////////////////////////////////////////////////////////////////////////////////
const std::vector<int>& NumericListParser::GetVector() const 
{
    if (!mbValid) 
    {
        throw std::string("There is no valid data");
    }
    return mvNumbers;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// GetString
///////////////////////////////////////////////////////////////////////////////////////////////////
const std::string& NumericListParser::GetString() const 
{
    return mInputList;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Parse
///////////////////////////////////////////////////////////////////////////////////////////////////
void NumericListParser::Parse() 
{
    vector<string> vTokens;
    char_separator<char> Separators(", ");
    tokenizer<char_separator<char> > tokens(mInputList, Separators);
    for (tokenizer<char_separator<char> >::iterator iToken = tokens.begin();
        iToken != tokens.end();
        ++iToken)
    {
        vTokens.push_back(*iToken);
    }
    if (vTokens.empty())
    {
        msError = "List is empty";
        return;
    }
    for(vector<string>::const_iterator Iter = vTokens.begin();Iter<vTokens.end();Iter++) 
    {
        if (!ParseToken(*Iter)) 
        {
            mbValid = false;
            return;
        }
    }
    mbValid = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// ParseToken
///////////////////////////////////////////////////////////////////////////////////////////////////
bool NumericListParser::ParseToken(const string& sToken)
{
    if (!mbSigned)
    {
        return ParseUnsignedToken(sToken);
    }
    else
    {
        return ParseSignedToken(sToken);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// ParseUnsignedToken
///////////////////////////////////////////////////////////////////////////////////////////////////
bool NumericListParser::ParseUnsignedToken(const string& sToken)
{
    try
    {
        vector<string> vItems;
        char_separator<char> Separators("-");
        tokenizer<char_separator<char> > tokens(sToken, Separators);
        for (tokenizer<char_separator<char> >::iterator iToken = tokens.begin();
            iToken != tokens.end();
            ++iToken)
        {
            vItems.push_back(*iToken);
        }    
        // one item
        if (vItems.size() == 1)
        {
            int Value;
            if (!IsOfType<int>(sToken, Value))
            {
                msError = sToken + " is not a number";
                return false;
            }
            mvNumbers.push_back(Value);
            return true;
        }

        if (vItems.size() > 2)
        {
            msError="Erroneous range";
            return false;
        }

        // range
        int Start, End;
        if (!IsOfType<int>(vItems[0], Start)) 
        {
            msError = vItems[0] + " is not a number";
            return false;
        }
        // step
        vector<string> vWithStep;
        char_separator<char> Separators2(":");
        tokenizer<char_separator<char> > tokens2(vItems[1], Separators2);
        for (tokenizer<char_separator<char> >::iterator iToken = tokens2.begin();
            iToken != tokens.end();
            ++iToken)
        {
            vWithStep.push_back(*iToken);
        }
        int Step = 1; // 1 is the default step
        // check for no step
        if (vWithStep.size() > 2)
        {
            msError="Erroneous range with step";
            return false;
        }
        // step
        if (vWithStep.size() == 1)
        {
            if (!IsOfType<int>(vWithStep[0], End))
            {
                msError = vWithStep[1] + " is not a number";
                return false;
            }
        }
        if (vWithStep.size() == 2)
        {
            if (!IsOfType<int>(vWithStep[0], End))
            {
                msError = vWithStep[1] + " is not a number";
                return false;
            }
            if (!IsOfType<int>(vWithStep[1], Step))
            {
                msError = vWithStep[1] + " is not a number";
                return false;
            }
            if (Step <= 0)
            {
                msError = "Erroneous step";
                return false;
            }
        }
        // check for start < end
        if (Start == End) 
        {
            msError = "Start==End in range";
            return false;
        }

        if (Start > End) 
        {
            Start = -Start;
            End = -End;
        }


        for (int i = Start; i <= End; i += Step)
        {
            mvNumbers.push_back(abs(i));
        }
    }
    catch (string sError)
    {
        msError = sError;
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// GetStringRepresentation
///////////////////////////////////////////////////////////////////////////////////////////////////
string NumericListParser::GetStringRepresentation () const
{
    try
    {
        if (!mbValid)
        {
            string sError("Initializtion problem for ");
            sError += mInputList + ": ";
            sError += msError;
            throw sError;
        }
        string sNumbers = boost::lexical_cast<string>(mvNumbers[0]);
        for (size_t i = 1; i < mvNumbers.size(); i++) 
        {
            sNumbers += "," + boost::lexical_cast<string>(mvNumbers[i]);
        }
        return sNumbers;

    } 
    catch (string sError)
    {
        throw (sError);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// add required commas where space is used as delimiter
///////////////////////////////////////////////////////////////////////////////////////////////////
void NumericListParser::AddRequiredCommas()
{
    bool bInRange = false;
    bool bFoundComma = false;

    for (size_t i = 0; i < mInputList.size(); ++i)
    {
        if ((mInputList[i] == '-' && !mbSigned) || mInputList[i] == ':')
        {
            bInRange = true;
            continue;    
        }

        if (mInputList[i] != ' ') 
        {
            bInRange = false;	
            continue;
        }

        if (i+1 < mInputList.size() && 
            ((mInputList[i+1] == '-' && !mbSigned) || mInputList[i+1] == ' ' || mInputList[i+1]==':'))
        {
            continue;
        }

        if (mInputList[i+1] == ',')
        {
            bInRange=false;
            bFoundComma=true;
            continue;
        }

        if (!bInRange && i+1 != mInputList.size() && !bFoundComma)
        {
            bFoundComma = false;
            mInputList[i] = ',';
            continue;
        }

        bFoundComma = false;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// ParseSignedToken
///////////////////////////////////////////////////////////////////////////////////////////////////
bool NumericListParser::ParseSignedToken(const string& sToken)
{
    if (sToken.find_first_not_of("0123456789:- ") != string::npos)
    {
        msError = "Found illegal character - cannot parse";
        return false;
    }
    vector<string> vItems;
    char_separator<char> Separators(":");
    tokenizer<char_separator<char> > tokens(sToken, Separators);
    for (tokenizer<char_separator<char> >::iterator iToken = tokens.begin();
        iToken != tokens.end();
        ++iToken)
    {
        vItems.push_back(*iToken);
    }

    // one item
    if (vItems.size()==1)
    {
        mvNumbers.push_back(atoi(sToken.c_str()));
        return true;
    }

    if (vItems.size() > 3)
    {
        msError = "Erroneous range";
        return false;
    }

    // range
    int Start = atoi(vItems[0].c_str());
    int End = atoi(vItems[1].c_str());
    // check for start<end
    if (Start >= End)
    {
        msError = "Start>=End in range";
        return false;
    }

    // step
    int Step = 1; // 1 is the default step
    if (vItems.size() == 3)
    {
        Step = atoi(vItems[2].c_str());
        if (Step <= 0)
        {
            msError = "Erroneous step";
            return false;
        }
    }

    for(int i = Start; i <= End; i += Step)
    {
        mvNumbers.push_back(i);
    }

    return true;
}
