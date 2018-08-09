// INTEL CONFIDENTIAL
//
// Copyright 2006-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include <iostream>
#include <cassert>
#include <iomanip>
#include <ctime>
#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <syscall.h>
#endif
#include "cl_user_logger.h"
#include "cl_sys_defines.h"
#include "cl_config.h"

using std::ostringstream;
using std::ends;
using std::cerr;
using std::ios_base;
using std::endl;
using std::string;
using std::vector;

namespace Intel { namespace OpenCL { namespace Utils {

#ifdef _WIN32
#if 0
// if we want to use command name in the future
static string GetCommandName()
{
    char pathCstr[256]; // maximum length of a filename in Windows
    const DWORD strLen = GetModuleFileName(nullptr, pathCstr, sizeof(pathCstr));
    assert(GetLastError() != ERROR_INSUFFICIENT_BUFFER);    
    
    const string path(pathCstr);
    const size_t index = path.find_last_of('\\');
    assert(index != string::npos);
    assert(index < path.size() - 1);

    const string filename = path.substr(index + 1);
    if (filename[filename.size() - 1] == '"')
    {
        return filename.substr(0, filename.size() - 1);
    }
    else
    {
        return filename;
    }
}
#endif
#endif

static tm GetLocalTime()
{
    time_t time = std::time(nullptr);
    assert(time != (time_t)-1);

    tm* const lt = std::localtime(&time);
    assert(lt != nullptr);
    return *lt;
}

static string GetFormattedHour(bool bUseColonSeperator, const tm& lt)
{
    ostringstream formattedHour;
    const string timeSeperator = bUseColonSeperator ? ":" : "";
    formattedHour << setfill('0') << lt.tm_hour << timeSeperator << setw(2) << lt.tm_min << timeSeperator << setw(2) << lt.tm_sec;
    return formattedHour.str();
}

static string GetFormattedTime()
{
    const tm lt = GetLocalTime();
    ostringstream formattedTime;
    formattedTime << (lt.tm_mon + 1) << "." << lt.tm_mday << "." << (1900 + lt.tm_year) << "_" <<
#ifdef _WIN32
        GetFormattedHour(false, lt) // : isn't allowed in Windows filenames
#else
        GetFormattedHour(true, lt)
#endif
        << ".txt" << ends;
    return formattedTime.str();
}

// FrameworkUserLogger's methods

string FrameworkUserLogger::FormatLocalWorkSize(const vector<size_t>& localWorkSize)
{
    stringstream stream;
    stream  << "(";    
    for (vector<size_t>::size_type i = 0; i < localWorkSize.size(); ++i)
    {
        stream << localWorkSize[i];
        if (i < localWorkSize.size() - 1)
        {
            stream << ",";
        }
    }
    stream << ")";
    return stream.str();
}

void FrameworkUserLogger::Setup(const string& filename, bool bLogErrors, bool bLogApis)
{
    if ("stdout" == filename)
    {
        m_pOutput = &std::cout;
    }
    else if ("stderr" == filename || filename.empty())
    {
        m_pOutput = &cerr;
    }
    else
    {
        ostringstream finalFilename;
        finalFilename << filename << "_PID" <<
#ifdef _WIN32
        GetCurrentProcessId()
#else
        getpid()
#endif
        << "_" << GetFormattedTime() << ends;    
        m_logFile.open(finalFilename.str().c_str(), ios_base::out);
        if (!m_logFile.is_open())
        {
            cerr << "cannot open log file " << finalFilename.str() << " for writing" << endl;
            return;
        }
        else
        {
            m_pOutput = &m_logFile;
        }
    }
    m_bLogErrors = bLogErrors;
    m_bLogApis = bLogApis;
}

FrameworkUserLogger::FrameworkUserLogger() : m_bLogErrors(false), m_bLogApis(false), m_pOutput(nullptr)
{
    ConfigFile config(GetConfigFilePath());
    const string varName = "CL_CONFIG_USER_LOGGER";
    const string configStr = config.Read(varName, string(""));
    // parse configStr
    bool bLogErrs = true, bLogApsi = false; // the defaults
    const string::size_type indexOfComman = configStr.find(',');
    if (indexOfComman != string::npos)
    {
        const string enableStr = configStr.substr(0, indexOfComman);
        if ("I" == enableStr)
        {
            bLogErrs = false;
            bLogApsi = true;
        }
        else if ("EI" == enableStr || "IE" == enableStr)
        {
            bLogApsi = true;
        }
        else if ("E" != enableStr)
        {
            cerr << "\"" << configStr << "\" is an invalid value for " << varName << endl;
            return;
        }
    }

    const string filename = indexOfComman != string::npos ? configStr.substr(indexOfComman + 1) : configStr;
    if (!configStr.empty())
    {
        Setup(filename, bLogErrs, bLogApsi);
    }
}

void ApiLogger::EndApiFuncInternal(cl_int retVal)
{
    m_strStream << ") = " << ClErrTxt(retVal);
    m_iLastRetValue = retVal;
    EndApiFuncEpilog();
}

void ApiLogger::EndApiFuncInternal(const void* retPtr)
{
    m_strStream << ") = 0x" << ios_base::hex << retPtr;
    if (nullptr != retPtr)
    {
        m_iLastRetValue = CL_SUCCESS;
    }
    else
    {
        m_iLastRetValue = CL_INVALID_VALUE;
    }
    EndApiFuncEpilog();
}

void ApiLogger::EndApiFuncInternal()
{
    m_strStream << ")";
    m_iLastRetValue = CL_SUCCESS;
    EndApiFuncEpilog();
}

void ApiLogger::PrintOutputParam(const string& name, const void* addr, size_t size, bool bIsPtr2Ptr, bool bIsUnsigned)
{
    if (!m_bLogApis)
    {
        return;
    }
    m_stream << ", *" << name << " = ";
    if (bIsPtr2Ptr)
    {
        const void* const* pp = reinterpret_cast<const void* const*>(addr);
        if (nullptr != pp)
        {
            m_stream << "0x" << hex << setfill('0') << setw(sizeof(void*) * 2) << *pp;
        }
        else
        {
            m_stream << "NULL";
        }
    }
    else
    {
        m_stream << dec;
        switch (size)
        {
        case 1:
            if (bIsUnsigned)
            {
                PrintIntegerOutputParam<cl_uchar>(addr);
            }
            else
            {
                PrintIntegerOutputParam<cl_char>(addr);
            }
            break;
        case 2:
            if (bIsUnsigned)
            {
                PrintIntegerOutputParam<cl_ushort>(addr);
            }
            else
            {
                PrintIntegerOutputParam<cl_short>(addr);
            }
            break;
        case 4:
            if (bIsUnsigned)
            {
                PrintIntegerOutputParam<cl_uint>(addr);
            }
            else
            {
                PrintIntegerOutputParam<cl_int>(addr);
            }
            break;
        case 8:
            if (bIsUnsigned)
            {
                PrintIntegerOutputParam<cl_ulong>(addr);
            }
            else
            {
                PrintIntegerOutputParam<cl_long>(addr);
            }
            break;
        default:
            assert(false && "unexpected size of type");
        }
    }
}

void FrameworkUserLogger::PrintError(const string& msg)
{
    if (m_bLogErrors)
    {
        *m_pOutput << "ERROR: " << msg << endl;
    }
}

void FrameworkUserLogger::PrintStringInternal(const string& str)
{
    OclAutoMutex mutex(&m_outputMutex);
    *m_pOutput << str;
}

void FrameworkUserLogger::SetLocalWorkSize4ArgValues(cl_dev_cmd_id id, const vector<size_t>& localWorkSize)
{
    OclAutoMutex mutex(&m_outputMutex);
    *m_pOutput << "Internally calculated local_work_size with for NDRangeKernel command with ID " << (size_t)id << ": " << FormatLocalWorkSize(localWorkSize) << endl;
}

// LogMessageWrapper methods:

void LogMessageWrapper::Serialize()
{
    stringstream stream;
    stream << m_id << " " << m_msg << ends;
    m_rawStr = stream.str();
}

void LogMessageWrapper::Unserialize()
{
    stringstream stream(m_rawStr);
    stream >> m_id;

    stream.seekg(1, ios_base::cur); // skip the space

    vector<char> buf(100);
    stream.getline(&buf[0], buf.size(), '\0');
    m_msg = &buf[0];

    assert(stream.eof());
}

// ApiLogger methods:

void ApiLogger::StartApiFuncInternal(const string& funcName)
{
    m_strStream << funcName << "(";
    m_bFirstApiFuncArg = true;
    m_timer.Start();
}

void ApiLogger::PrintParamTypeAndName(const char* sParamTypeAndName)
{
    if (!m_bFirstApiFuncArg)
    {
        m_strStream << ", ";        
    }
    else
    {
        m_bFirstApiFuncArg = false;
    }
    m_strStream << sParamTypeAndName << " = ";
}

void ApiLogger::PrintCStringValInternal(const char* sVal)
{
    if (nullptr != sVal)
    {
        m_strStream << sVal;
    }
    else
    {
        m_strStream << "NULL";
    }
}

void ApiLogger::EndApiFuncEpilog()
{
    m_timer.Stop();    
    // thread ID    
    std::right(m_stream);
    m_stream << "TID " << setfill(' ') << setw(9) << dec << 
#ifdef _WIN32
        GetCurrentThreadId()
#else
        syscall(SYS_gettid)
#endif
        ;
    const unsigned long long ulStartTime = RDTSC();
    // start time in clock ticks
    m_stream << "    START TIME 0x" << setfill('0') << setw(16) << hex << ulStartTime;
    // duration
    m_stream << "    DURATION 0x" << setw(16) << m_timer.GetTotalUsecs();
    std::left(m_stream);
    if (m_iCmdId != -1)
    {
        m_stream << "    CMD ID " << setfill(' ') << dec << setw(10) << m_iCmdId;
    }
    else
    {
        m_stream << "                     ";
    }
    // API call itself
    m_stream << "    " << m_strStream.str();
}

}}}
