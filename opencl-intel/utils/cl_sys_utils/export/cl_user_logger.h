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

#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <utility>
#include "cl_synch_objects.h"
#include "cl_timer.h"

namespace Intel { namespace OpenCL { namespace Utils {

/**
 * This class represents a user-oriented logger - it is a singleton
 */
class UserLogger
{
public:    

    /**
     * Constructor
     */
    UserLogger();

    /**
     * Destructor
     */
    ~UserLogger()
    {
        if (m_logFile.is_open())
        {
            m_logFile.close();
        }
    }    

    /**
     * Print a value in an API call into the log
     * @param val the value to be printed
     * @return this UserLogger
     */
    template<typename T>
    UserLogger& operator<<(const T& val)
    {
        if (m_bLogApis)
        {
            m_strStream << val;
        }
        return *this;
    }

    /**
     * Print the type and name of a parameter of an API call
     * @param sParamTypeAndName type followed by the parameter's name
     * @return this UserLogger
     */
    UserLogger& operator<<(const char* sParamTypeAndName);

    /**
     * Print a C-string value in an API call(operator<< cannot be used in this case, because it assumes that the string is a parameter type and name)
     * @param sVal the C-string value
     * @return this UserLogger
     */
    UserLogger& PrintCStringVal(const char* sVal);
    
    /**
     * @return whether API logging is enabled
     */
    bool IsApiLoggingEnabled() const { return m_bLogApis; }

    /**
     * @return whether error logging is enabled
     */
    bool IsErrorLoggingEnabled() const { return m_bLogErrors; }

    /**
     * Start logging of API function
     * @param funcName the name of the function
     */
    void StartApiFunc(const std::string& funcName);

    /**
     * End logging of an API function with a pointer return value
     * @param retVal the return value
     */
    void EndApiFunc(const void* retVal);

    /**
     * End logging of an API function with a cl_int return value, which will be printed with the macro's name
     * @param retVal the returned pointer
     */
    void EndApiFunc(cl_int retPtr);

    /**
     * End logging of an API function without a return value
     */
    void EndApiFunc();

    /**
     * Set whether to expect output parameter in the current API function (this means that a new-line will not be printed after the return value)
     * @param bExpect whether to expect output parameter in the current API function
     */
    void SetExpectOutputParams(bool bExpect) { m_bExpectOutputParams = bExpect; }

    /**
     * Print the value of an output parameter after the function returns
     * @param name          the name of the parameter
     * @param addr          the address of the parameter
     * @param size          the size of the type the output parameter points to
     * @param bIsPtr2Ptr    whether the parameter's type is a pointer to pointer
     * @param bIsUnsigned   whether the parameter is an unsigned integer type (only relevant when bIsPtr2Ptr is false)
     */
    void PrintOutputParam(const std::string& name, const void* addr, size_t size, bool bIsPtr2Ptr, bool bIsUnsigned = false);

    /**
     * Print a string directly to the log (not as a part of an API call)
     * @param str   the string to print
     * @param bLock whether to lock the logger's stream (regular printing by client class should use the default to prevent interleaving of prints from different threads)
     */
    void PrintString(const std::string& str, bool bLock = true)
    {
        if (m_bLogApis)
        {
            PrintStringInternal(str, bLock);
        }
    }

    /**
     * @return the return value from the last API function (if the return value of the function is a pointer, then this method returns CL_SUCCESS if it is not NULL; if there is
     *         no return value, then this method always returns CL_SUCCESS)
     */
    cl_int GetLastRetVal() const { return m_iLastRetValue; }

    /**
     * Print an error to the log
     * @param               the error message
     */
    void PrintError(const std::string& msg);

private:    

    void Setup(const std::string& filename, bool bLogErrors, bool bLogApis);

    void EndApiFuncEpilog();

    template<typename T>
    void PrintIntegerOutputParam(const void* ptr)
    {
        if (NULL != ptr)
        {
            *m_pOutput << *reinterpret_cast<const T*>(ptr);
        }
        else
        {
            *m_pOutput << "NULL";
        }
    }

    void PrintParamTypeAndName(const char* sParamTypeAndName);

    void PrintCStringValInternal(const char* sVal);

    void StartApiFuncInternal(const string& funcName);

    void EndApiFuncInternal(cl_int retVal);

    void EndApiFuncInternal(const void* retPtr);

    void EndApiFuncInternal();

    void PrintStringInternal(const std::string& str, bool bLock);

    // do not implement
    UserLogger(const UserLogger&);
    UserLogger& operator=(const UserLogger&);
    
    std::ofstream m_logFile;
    std::ostream* m_pOutput;
    Timer m_timer;
    /* synchronize calls to API functions, so that logging from different threads won't be interleaved (it is reentrant to allow printing from the same thread that locked the
        mutex) */
    OclSpinMutex m_mutex;
    bool m_bFirstApiFuncArg;    
    // we use this to collect all data about the API function call, so that when it ends, we'll be able to put its duration before the log of the call itself
    std::ostringstream m_strStream;
    std::ostringstream m_retValStream;
    bool m_bExpectOutputParams;
    cl_int m_iLastRetValue;
    bool m_bLogErrors;
    bool m_bLogApis;
};

// inline methods (I want to save the function call in case no logging is done)

inline UserLogger& UserLogger::operator<<(const char* sParamTypeAndName)
{
    if (m_bLogApis)
    {
        PrintParamTypeAndName(sParamTypeAndName);
    }
    return *this;
}

inline UserLogger& UserLogger::PrintCStringVal(const char* sVal)
{
    if (m_bLogApis)
    {
        PrintCStringValInternal(sVal);
    }
    return *this;
}

inline void UserLogger::StartApiFunc(const string& funcName)
{
    if (!m_bLogApis)
    {
        return;
    }
    StartApiFuncInternal(funcName);
}

inline void UserLogger::EndApiFunc(cl_int retVal)
{
    if (!m_bLogApis)
    {
        return;
    }
    EndApiFuncInternal(retVal);
}

inline void UserLogger::EndApiFunc(const void* retPtr)
{
    if (!m_bLogApis)
    {
        return;
    }
    EndApiFuncInternal(retPtr);
}

inline void UserLogger::EndApiFunc()
{
    if (!m_bLogApis)
    {
        return;
    }
    EndApiFuncInternal();
}

}}}
