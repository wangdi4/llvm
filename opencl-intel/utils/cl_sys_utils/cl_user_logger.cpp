// Copyright (c) 2006-2014 Intel Corporation
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

#include <iostream>
#include <cassert>
#include <iomanip>
#include <ctime>
#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/types.h>
#include <unistd.h>
#ifndef __ANDROID__
#include <pthread.h>
#include <syscall.h>
#endif
#endif
#include "cl_user_logger.h"
#include "cl_sys_defines.h"
#include "cl_config.h"

namespace Intel { namespace OpenCL { namespace Utils {

#ifdef _WIN32
#if 0
// if we want to use command name in the future
static string GetCommandName()
{
    char pathCstr[256]; // maximum length of a filename in Windows
    const DWORD strLen = GetModuleFileName(NULL, pathCstr, sizeof(pathCstr));
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

static std::tm GetLocalTime()
{
    std::time_t time = std::time(NULL);
    assert(time != (std::time_t)-1);

    std::tm* const lt = std::localtime(&time);
    assert(lt != NULL);
    return *lt;
}

static string GetFormattedHour(bool bUseColonSeperator, const std::tm& lt)
{
    std::ostringstream formattedHour;
    const string timeSeperator = bUseColonSeperator ? ":" : "";
    formattedHour << std::setfill('0') << lt.tm_hour << timeSeperator << std::setw(2) << lt.tm_min << timeSeperator << std::setw(2) << lt.tm_sec;
    return formattedHour.str();
}

static string GetFormattedTime()
{
    const std::tm lt = GetLocalTime();
    std::ostringstream formattedTime;
    formattedTime << (lt.tm_mon + 1) << "." << lt.tm_mday << "." << (1900 + lt.tm_year) << "_" <<
#ifdef _WIN32
        GetFormattedHour(false, lt) // : isn't allowed in Windows filenames
#else
        GetFormattedHour(true, lt)
#endif
        << ".txt" << std::ends;
    return formattedTime.str();
}

// UserLogger's methods

void UserLogger::Setup(const string& filename, bool bLogErrors, bool bLogApis)
{
    assert(!filename.empty());    
    if ("stdout" == filename)
    {
        m_pOutput = &std::cout;
    }
    else if ("stderr" == filename)
    {
        m_pOutput = &std::cerr;
    }
    else
    {
        std::ostringstream finalFilename;
        finalFilename << filename << "_PID" <<
#ifdef _WIN32
        GetCurrentProcessId()
#else
        getpid()
#endif
        << "_" << GetFormattedTime() << std::ends;    
        m_logFile.open(finalFilename.str().c_str(), std::ios_base::out);
        if (!m_logFile.is_open())
        {
            std::cerr << "cannot open log file " << finalFilename.str() << " for writing" << std::endl;
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

UserLogger::UserLogger() : m_pOutput(NULL), m_bFirstApiFuncArg(false), m_bExpectOutputParams(false), m_iLastRetValue(CL_SUCCESS), m_bLogErrors(false), m_bLogApis(false)
{
    ConfigFile config(GetConfigFilePath());
    const string varName = "CL_CONFIG_USER_LOGGER";
    const std::string configStr = config.Read(varName, string(""));
    // parse configStr
    bool bLogErrs = true, bLogApsi = false; // the defaults
    const std::string::size_type indexOfComman = configStr.find(',');
    if (indexOfComman != std::string::npos)
    {
        const std::string enableStr = configStr.substr(0, indexOfComman);
        if ("I" == enableStr)
        {
            bLogErrs = false;
            bLogApsi = true;
        }
        else if ("E|I" == enableStr || "I|E" == enableStr)
        {
            bLogApsi = true;
        }
        else if ("E" != enableStr)
        {
            std::cerr << "\"" << configStr << "\" is an invalid value for " << varName << std::endl;
            return;
        }
    }

    const std::string filename = indexOfComman != std::string::npos ? configStr.substr(indexOfComman + 1) : configStr;
    if (!configStr.empty())
    {
        Setup(filename, bLogErrs, bLogApsi);
    }
}

void UserLogger::PrintParamTypeAndName(const char* sParamTypeAndName)
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

void UserLogger::PrintCStringValInternal(const char* sVal)
{
    if (NULL != sVal)
    {
        m_strStream << sVal;
    }
    else
    {
        m_strStream << "NULL";
    }
}

void UserLogger::StartApiFuncInternal(const string& funcName)
{
    m_mutex.Lock();
    m_strStream << funcName << "(";
    m_bFirstApiFuncArg = true;
    m_timer.Start();
}

void UserLogger::EndApiFuncInternal(cl_int retVal)
{
    m_strStream << ")";
    m_retValStream << " = " << ClErrTxt(retVal);
    m_iLastRetValue = retVal;
    EndApiFuncEpilog();
}

void UserLogger::EndApiFuncInternal(const void* retPtr)
{
    m_strStream << ")";
    m_retValStream << " = 0x" << std::ios_base::hex << retPtr;
    if (NULL != retPtr)
    {
        m_iLastRetValue = CL_SUCCESS;
    }
    else
    {
        m_iLastRetValue = CL_INVALID_VALUE;
    }
    EndApiFuncEpilog();
}

void UserLogger::EndApiFuncInternal()
{
    m_strStream << ")";
    m_iLastRetValue = CL_SUCCESS;
    EndApiFuncEpilog();
}

void UserLogger::EndApiFuncEpilog()
{
    m_timer.Stop();    
    if (!m_bExpectOutputParams)
    {
        m_retValStream << std::endl;
    }
    // thread ID
    std::right(*m_pOutput);
    *m_pOutput << std::setfill(' ') << std::setw(5) << std::dec << 
#ifdef _WIN32
        GetCurrentThreadId()
#else
#ifdef __ANDROID__
        gettid()
#else
        syscall(SYS_gettid)
#endif
#endif
        << " ";
    // start time in clock ticks
    *m_pOutput << RDTSC() << " ";
    // duration
    *m_pOutput << m_timer.GetTotalUsecs() << " ";
    // API call - I've taken this format from ltrace Linux program
    *m_pOutput << m_strStream.str() << m_retValStream.str();
    
    // clear all necessary attributes
    m_timer.Reset();
    m_strStream.str("");
    m_retValStream.str("");
    m_mutex.Unlock();
}

void UserLogger::PrintOutputParam(const std::string& name, const void* addr, size_t size, bool bIsPtr2Ptr, bool bIsUnsigned)
{
    if (!m_bLogApis)
    {
        return;
    }
    *m_pOutput << ", *" << name << " = ";
    if (bIsPtr2Ptr)
    {
        const void* const* pp = reinterpret_cast<const void* const*>(addr);
        if (NULL != pp)
        {
            *m_pOutput << "0x" << std::hex << std::setfill('0') << std::setw(sizeof(void*) * 2) << *pp;
        }
        else
        {
            *m_pOutput << "NULL";
        }
    }
    else
    {
        *m_pOutput << std::dec;
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

void UserLogger::PrintError(const std::string& msg)
{
    if (m_bLogErrors)
    {
        *m_pOutput << "ERROR: " << msg << std::endl;
    }
}

void UserLogger::PrintStringInternal(const std::string& str, bool bLock)
{
    if (bLock)
    {
        m_mutex.Lock();    
    }
    *m_pOutput << str;
    if (bLock)
    {
        m_mutex.Unlock();
    }
}

}}}
