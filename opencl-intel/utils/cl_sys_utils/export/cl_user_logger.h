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
class FrameworkUserLogger
{
public:    

    /**
     * Constructor
     */
    static FrameworkUserLogger& Instance();    

    /**
     * Destructor
     */
    ~FrameworkUserLogger()
    {
        if (m_logFile.is_open())
        {
            m_logFile.close();
        }
    }           
    
    /**
     * @return whether API logging is enabled
     */
    bool IsApiLoggingEnabled() const { return m_bLogApis; }

    /**
     * @return whether error logging is enabled
     */
    bool IsErrorLoggingEnabled() const { return m_bLogErrors; }    

    /**
     * Print a string to the log
     * @param str the string to print
     */
    void PrintString(const std::string& str)
    {
        if (m_bLogApis)
        {
            PrintStringInternal(str);
        }
    }

    /**
     * Print an error to the log
     * @param               the error message
     */
    void PrintError(const std::string& msg);

    /**
     * Set the local work size calculated by BE for a specific NDRange command identified by cl_dev_cmd_id
     * @param id the cl_dev_cmd_id identifying the NDRange command
     * @param localWorkSize the local work size calculated by BE
     */
    void SetLocalWorkSize4ArgValues(cl_dev_cmd_id id, const std::vector<size_t>& localWorkSize);

private:    

    FrameworkUserLogger();

    void Setup(const std::string& filename, bool bLogErrors, bool bLogApis);        

    void PrintStringInternal(const std::string& str);    

    std::string FormatLocalWorkSize(const std::vector<size_t>& localWorkSize);

    // do not implement
    FrameworkUserLogger(const FrameworkUserLogger&);
    FrameworkUserLogger& operator=(const FrameworkUserLogger&);

    bool m_bLogErrors;
    bool m_bLogApis;
    std::ofstream m_logFile;
    std::ostream* m_pOutput;
    mutable OclSpinMutex m_outputMutex; // Synchronize writing to m_pOutput    
};

extern FrameworkUserLogger* g_pUserLogger;   // a global pointer to the logger, which be defined in each shared library (so LOG_ERROR can use the user logger)

/**
 * This class is responsible for collecting logging data from a signle API call and printing it to the user log at its destruction
 */
class ApiLogger
{
public:

    /**
     * Constructor
     * @param apiCallName name of the API call
     */
    ApiLogger(const std::string& apiCallName) : m_iLastRetValue(CL_SUCCESS), m_bLogApis(g_pUserLogger->IsApiLoggingEnabled()), m_bFirstApiFuncArg(false), m_iCmdId(-1)
    {
        if (!m_bLogApis)
        {
            return;
        }
        StartApiFuncInternal(apiCallName);
    }

    /**
     * Destructor
     */
    ~ApiLogger()
    {
        m_stream << endl;
        g_pUserLogger->PrintString(m_stream.str());        
    }    

    /**
     * @return the return value from the last API function (if the return value of the function is a pointer, then this method returns CL_SUCCESS if it is not NULL; if there is
     *         no return value, then this method always returns CL_SUCCESS)
     */
    cl_int GetLastRetVal() const
    { 
        return m_iLastRetValue;
    }

    /**
     * Start logging of API function
     * @param funcName the name of the function
     */
    void StartApiFunc(const std::string& funcName);

    /**
     * Print a value in an API call into the log
     * @param val the value to be printed
     * @return this FrameworkUserLogger
     */
    template<typename T>
    ApiLogger& operator<<(const T& val)
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
     * @return this FrameworkUserLogger
     */
    ApiLogger& operator<<(const char* sParamTypeAndName);

    /**
     * Print a C-string value in an API call (operator<< cannot be used in this case, because it assumes that the string is a parameter type and name)
     * @param sVal the C-string value
     * @return this FrameworkUserLogger
     */
    ApiLogger& PrintCStringVal(const char* sVal);

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
     * Print the value of an output parameter after the function returns
     * @param name          the name of the parameter
     * @param addr          the address of the parameter
     * @param size          the size of the type the output parameter points to
     * @param bIsPtr2Ptr    whether the parameter's type is a pointer to pointer
     * @param bIsUnsigned   whether the parameter is an unsigned integer type (only relevant when bIsPtr2Ptr is false)
     */
    void PrintOutputParam(const std::string& name, const void* addr, size_t size, bool bIsPtr2Ptr, bool bIsUnsigned = false);

    /**
     * Print a string by OutputParamsValueProvider
     */
    void PrintOutputParamStr(const std::string& str) { m_stream << str; }

    /**
     * Set the command ID of a command
     */
    void SetCmdId(cl_int id) { m_iCmdId = id; }

private:

    void StartApiFuncInternal(const string& funcName);

    void PrintParamTypeAndName(const char* sParamTypeAndName);

    void PrintCStringValInternal(const char* sVal);    

    void EndApiFuncInternal(cl_int retVal);

    void EndApiFuncInternal(const void* retPtr);

    void EndApiFuncInternal();

    void EndApiFuncEpilog();

    template<typename T>
    void PrintIntegerOutputParam(const void* ptr)
    {
        if (nullptr != ptr)
        {
            m_stream << *reinterpret_cast<const T*>(ptr);
        }
        else
        {
            m_stream << "NULL";
        }
    }
        
    cl_int m_iLastRetValue;
    const bool m_bLogApis;
    // we use this to collect all data about the API function call, so that when it ends, we'll be able to put its duration before the log of the call itself
    std::ostringstream m_strStream;
    std::ostringstream m_stream;
    Timer m_timer;
    bool m_bFirstApiFuncArg;
    cl_int m_iCmdId;

};

// Wrapper for log error from device
class LogMessageWrapper
{
public:

    /**
     * Constructor for use by device side
     * @param id    the ID of the NDRange command
     * @param msg the string holding the log message
     */
    LogMessageWrapper(cl_dev_cmd_id id, const std::string& msg) : m_id(id), m_msg(msg) { Serialize(); }

    /**
     * Constructor for use by host side
     * @param rawStr the raw string from which the log message should be parsed
     */
    LogMessageWrapper(const char* rawStr) : m_rawStr(rawStr) { Unserialize(); }

    /**
     * @return the ID of the NDRange command
     */
    cl_dev_cmd_id GetId() const { return m_id; }

    /**
     * @return the string holding the log message
     */
    std::string GetMsg() const { return m_msg; }

    /**
     * the raw string from which the log message should be parsed
     */
    std::string GetRawString() const { return m_rawStr; }

private:

    void Serialize();

    void Unserialize();

    cl_dev_cmd_id m_id;
    std::string m_msg;
    std::string m_rawStr;

};


// inline methods (I want to save the function call in case no logging is done)

inline ApiLogger& ApiLogger::PrintCStringVal(const char* sVal)
{
    if (m_bLogApis)
    {
        PrintCStringValInternal(sVal);
    }
    return *this;
}

inline void ApiLogger::EndApiFunc(cl_int retVal)
{
    if (!m_bLogApis)
    {
        return;
    }
    EndApiFuncInternal(retVal);
}

inline void ApiLogger::EndApiFunc(const void* retPtr)
{
    if (!m_bLogApis)
    {
        return;
    }
    EndApiFuncInternal(retPtr);
}

inline void ApiLogger::EndApiFunc()
{
    if (!m_bLogApis)
    {
        return;
    }
    EndApiFuncInternal();
}

inline void ApiLogger::StartApiFunc(const string& funcName)
{
    if (!m_bLogApis)
    {
        return;
    }
    StartApiFuncInternal(funcName);
}

inline ApiLogger& ApiLogger::operator<<(const char* sParamTypeAndName)
{
    if (m_bLogApis)
    {
        PrintParamTypeAndName(sParamTypeAndName);
    }
    return *this;
}

}}}
