/*****************************************************************************\

Copyright (c) Intel Corporation (2011).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  NEATALU.cpp

Utility for NEATPlugin validation. It is command line utility for NEAT plugin validation.
It compares data from reference file with output data provided via standard input.
Reference file has comments starting with ";CHECKNEAT:" that contains NEAT data separated by spaces.
NEATValue has following format: %NEATVAL% = “%type% [%data1] [%data2]”
%type%= { “ACCURATE”, “ANY”, “UNKNOWN”, “UNWRITTEN”, “INTERVAL” };
Example: “ACCUATE 3.2434”, “INTERVAL 322.34 234.4”


\*****************************************************************************/
#include <iostream>
#include <fstream>
#include <sstream>
#include "llvm/Support/CommandLine.h"
#include "Exception.h"
#include "VectorWidth.h"
#include "DataType.h"
#include "BufferContainerList.h"
#include "XMLDataReader.h"
#include "XMLDataWriter.h"
#include "FloatOperations.h"
#include <cassert>
#include "IMemoryObject.h"
#include "Buffer.h"

using namespace std;
using namespace llvm;
using namespace Validation;

cl::opt<string> 
ReferenceFilename(cl::Required, "r", cl::desc("Specify reference pattern filename"), 
                  cl::value_desc("filename"));

cl::opt<string>
ActualOutFilename(cl::Optional, "a", cl::desc("Specify actual ouptut filename. It should contain buffers in DataManager format"), 
                  cl::value_desc("filename"), cl::init("-"));

cl::opt<unsigned>
Tolerance(cl::Optional, "t",  cl::desc("Specify comparison tolerance. If not specified, precise comparison is used."), cl::init(1000000000),
                  cl::value_desc("tolerance"));

enum MismatchType {MT_STATUS, MT_VALUE, MT_UNKNOWN_TYPE};

template <typename DstType, typename SrcType>
DstType BitCast(const SrcType &src)
{
    assert(sizeof(SrcType) == sizeof(DstType));
    DstType dst;
    memcpy(&dst, &src, sizeof(DstType));
    return dst;
}

int64_t ulpsDiff(double a, double b)
{
    int64_t aInt = BitCast<int64_t, double>(a);
    if (aInt < 0)
        aInt = 0x8000000000000000 - aInt;
    int64_t bInt = BitCast<int64_t, double>(b);
    if (bInt < 0)
        bInt = 0x8000000000000000 - bInt;
    int64_t ulpsDiff = aInt - bInt;
    if(ulpsDiff < 0) 
        ulpsDiff = -ulpsDiff;
    return ulpsDiff;
}

bool Equal(double a, double b)
{
    return (ulpsDiff(a,b) <= Tolerance);
}

void ReportError(const string& outputStr, const string& refStr, 
                 MismatchType type, int refLine)
{
    switch(type)
    {
    case MT_STATUS:
        cout << "Output status doesn't match reference!! Output status: ";
        cout << outputStr;
        cout << " Reference status: ";
        cout << refStr;
        cout << ", See reference file, Line #" << refLine << endl;
        break;
    case MT_VALUE:
        cout << "Output value doesn't match reference!! Output value: ";
        cout << outputStr;
        cout << " Reference value: ";
        cout << refStr;
        cout << ", See reference file, Line # " << refLine << endl;
        break;
    case MT_UNKNOWN_TYPE:
        cout << "Unknown type of status detected. Both reference and output statuses are equal to ";
        cout << outputStr;
        cout << ", Line # ";
        cout << refLine << endl;
        break;
    }
    /// exit with error
    exit(1);
}

void SkipDelimiters(stringstream& inout_Stream)
{
    // const char* delimiters = " \t";
    int byte = inout_Stream.peek();
    while(inout_Stream.good() && ((byte == ' ') || (byte == '\t')))
    {
        inout_Stream.read((char*)&byte, 1);
        byte = inout_Stream.peek();
    }
}

/// @brief Compares reference and output strings. Reference is given as 
///        string from reference file and offset of NEAT data. Output is
///        provided as a stringstream.
///
/// @param outputStr    Output string for validation. 
///                     It contains only NEATValue descriptions
/// @param refStr       Reference string from pattern file. 
///                     It contains NEATValue description starting with 
///                     refLineOffset index in the refStr.
/// @param lineNum      Line number of refStr in pattern file. 
///                     Used for error reporting.
/// @return             substring of output Str that is left unprocessed 
void ProcessString(stringstream& outStream, const string& refStr, 
                   int refLineOffset, int lineNum) 
{
    stringstream refStream;
    refStream << refStr.substr(refLineOffset, refStr.size() - refLineOffset);
    string outStatus, refStatus; 
    // const string& delimiters = " \t";
    string referenceString = refStream.str();
    refStream>>skipws;
    while(refStream.good())
    {
        if(!outStream.good())
        {
            cerr << "Reference has more data than the output" << endl;
            exit(1);
        }
        refStream>>refStatus;
        outStream>>outStatus;
        if(outStatus != refStatus)
        {
            ReportError(outStatus, refStatus, MT_STATUS, lineNum);
        }
        else
        {
            if(outStatus == "ACCURATE")
            {
                string outValStr,refValStr;
                outStream>>outValStr;
                refStream>>refValStr;
                double outVal = atof(outValStr.c_str());
                double refVal = atof(refValStr.c_str());
                if(!Equal(refVal, outVal))
                    ReportError(outValStr, refValStr, MT_VALUE, lineNum);
            } else if(outStatus == "INTERVAL")
            {
                string outMinStr,outMaxStr,refMinStr,refMaxStr;
                outStream>>outMinStr;
                outStream>>outMaxStr;
                refStream>>refMinStr;
                refStream>>refMaxStr;
                double outMin = atof(outMinStr.c_str());
                double outMax = atof(outMaxStr.c_str());
                double refMin = atof(refMinStr.c_str());
                double refMax = atof(refMaxStr.c_str());
                if(!Equal(outMin, refMin))
                {
                    ReportError(outMinStr, refMinStr, MT_VALUE, lineNum);
                }
                if(!Equal(outMax, refMax))
                {
                    ReportError(outMaxStr, refMaxStr, MT_VALUE, lineNum);
                }
            } else if (( outStatus != "UNKNOWN") && (outStatus != "UNWRITTEN") 
                        && (outStatus != "ANY"))
            {
                ReportError(outStatus, outStatus, MT_UNKNOWN_TYPE, lineNum);
            }
            SkipDelimiters(refStream);
            SkipDelimiters(outStream);
        }
        //string referenceString = refStream.str();
    }
}

template<typename T>
void StringifyTypedBuffer(stringstream& inout_Stream, const IMemoryObject* in_pBuf)
{
    BufferDesc desc = GetBufferDescription(in_pBuf->GetMemoryObjectDesc());
    /// Skip those buffers that don't contain NEAT Values
    if(desc.IsNEAT())
    {
        BufferAccessor<NEATValue> BufAcc(*in_pBuf);
        /// For each vector in the buffer
        for(uint32_t iV = 0; iV<desc.NumOfElements(); iV++)
        {
            /// For each element in vector
            for(uint32_t off=0; off<desc.SizeOfVector(); off++)
            {
                NEATValue& NeatOut  = BufAcc.GetElem(iV, off);
                NEATValue::Status st = NeatOut.GetStatus();
                switch(st)
                {
                    case NEATValue::ANY:
                        inout_Stream<<" ANY ";
                        break;
                    case NEATValue::UNKNOWN:
                        inout_Stream<<" UNKNOWN ";
                        break;
                    case NEATValue::UNWRITTEN:
                        inout_Stream<<" UNWRITTEN ";
                        break;
                    case NEATValue::ACCURATE:
                        inout_Stream<<" ACCURATE "<<*NeatOut.GetAcc<T>()<<" ";
                        break;
                    case NEATValue::INTERVAL:
                        inout_Stream<<" INTERVAL "<<*NeatOut.GetMin<T>()<<" "<<*NeatOut.GetMax<T>()<<" ";
                        break;
                }
            }
        }
    }
}

void StringifyBuffer(stringstream& inout_Stream, IMemoryObject* in_pBuf, const BufferDesc& in_bufDesc)
{
    TypeVal elemType = (in_bufDesc.GetElementDescription().IsComposite() && !in_bufDesc.GetElementDescription().IsStruct()) ?
        in_bufDesc.GetElementDescription().GetSubTypeDesc(0).GetType() :
        in_bufDesc.GetElementDescription().GetType();
    switch(elemType)
    {
        /// Check integer types first
    case TCHAR:
        StringifyTypedBuffer<int8_t>(inout_Stream, in_pBuf);
        break;
    case TSHORT:
        StringifyTypedBuffer<int16_t>(inout_Stream, in_pBuf);
        break;
    case TINT:
        StringifyTypedBuffer<int32_t>(inout_Stream, in_pBuf);
        break;
    case TLONG:
        StringifyTypedBuffer<int64_t>(inout_Stream, in_pBuf);
        break;
    case TUCHAR:
        StringifyTypedBuffer<uint8_t>(inout_Stream, in_pBuf);
        break;
    case TUSHORT:
        StringifyTypedBuffer<uint16_t>(inout_Stream, in_pBuf);
        break;
    case TUINT:
        StringifyTypedBuffer<uint32_t>(inout_Stream, in_pBuf);
        break;
    case TULONG:
        StringifyTypedBuffer<uint64_t>(inout_Stream, in_pBuf);
        break;
    case THALF:
        StringifyTypedBuffer<CFloat16>(inout_Stream, in_pBuf);
        break;
    case TFLOAT:
        StringifyTypedBuffer<float>(inout_Stream, in_pBuf);
        break;
    case TDOUBLE:
        StringifyTypedBuffer<double>(inout_Stream, in_pBuf);
        break;
    case UNSPECIFIED_TYPE:
        throw Exception::InvalidArgument("Cannot compare buffers with unspecified data type.");
        break;
    case INVALID_TYPE:
        throw Exception::InvalidArgument("Invalid data type!");
        break;
    default:
        throw Exception::InvalidArgument("StringifyBuffer: Unsupported data type of buffer elements!");
        break;
    }
}

void DMtoStringStream(const string& in_filename, stringstream& inout_pSs)
{
    XMLBufferContainerListReader loader(in_filename);
    BufferContainerList inp;
    loader.Read(&inp);

    for(uint32_t BCIndex = 0; BCIndex < inp.GetBufferContainerCount(); BCIndex++)
    {
        IBufferContainer* pBC = inp.GetBufferContainer(BCIndex);
        uint32_t count = (uint32_t)pBC->GetMemoryObjectCount();
        /// go through buffers in the BufferContainter and put them to the stream
        for(uint32_t bufInd = 0; bufInd<count; bufInd++)
        {
            IMemoryObject* pBuf = pBC->GetMemoryObject(bufInd);
            BufferDesc bufDesc = GetBufferDescription(pBuf->GetMemoryObjectDesc());
            StringifyBuffer(inout_pSs, pBuf, bufDesc);
            //lineNum = CompareBuffers(pBuf, lineNum);
        }
    }
}

int main(int argc, char **argv)
{
    const int BUF_LENGTH = 100000;
    char buf[BUF_LENGTH];
    const string checkKeyword = ";CHECKNEAT:";
    string outStr;
    /// Parse command line arguments
    cl::ParseCommandLineOptions(argc, argv);
    if(ActualOutFilename == "-")
    {
        //pActStream = &cin;
        cin.getline(buf, BUF_LENGTH);
        outStr = string(buf);
    }   
    else
    {
        stringstream ss;
        DMtoStringStream(ActualOutFilename.c_str(), ss);
        outStr = ss.str();
    }
    /// Go through files and write to output
    std::ifstream Reference(ReferenceFilename.c_str());
    if(Reference.is_open())
    {
        stringstream outStream;
        outStream << outStr;
        string referenceString;
        size_t found;
        int lineNum = 0;
        while(Reference.good())
        {
            Reference.getline(buf, BUF_LENGTH);
            referenceString = string(buf);
            found = referenceString.find(checkKeyword);
            if(found != string::npos)
            {
                 ProcessString(outStream, referenceString, found + 
                   checkKeyword.size(), lineNum);
            }
            lineNum++;
        }
        if(outStream.good())
        {
            cerr << "output has more data than the reference" << endl;
            return 1;
        }
    }
    else
    {
        cerr << "Unable to open reference pattern file" << endl;
        return 1;
    }
    cout<<"Test passed"<<endl;
    return 0;
}

