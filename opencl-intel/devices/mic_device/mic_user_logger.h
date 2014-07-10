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

#include <stdexcept>
#include <iostream>
#include "cl_thread.h"



namespace Intel { namespace OpenCL { namespace MICDevice {

/**
 * This class is responsible for reading the stream coming from the device's stderr and extracting the BE
 * and error reports.
 */
class MicUserLogger
{
public:

    /**
     * @returns the singleton instance of MicUserLogger
     * @throws IOError if some failure in allocating resource occurs
     */
    static MicUserLogger& Instance();

    /**
     * Destructor
     */
    ~MicUserLogger();

private:

    class FileWrapper
    {
    public:

        FileWrapper() : m_fileDesc(-1) { }

        ~FileWrapper();

        FileWrapper& operator=(int fileDesc);

        operator int() { return m_fileDesc; }

    private:

        // don't implement
        FileWrapper(const FileWrapper&);
        FileWrapper& operator=(const FileWrapper&);

        int m_fileDesc;

    };

    class StderrListerenerThread : public Intel::OpenCL::Utils::OclThread
    {
    public:

        void Init(MicUserLogger* pLogger) { m_pLogger = pLogger; }

        RETURN_TYPE_ENTRY_POINT Run();

    private:

        MicUserLogger* m_pLogger;

        void HandleLogMessage(std::istream& pipeStream);

    };

    MicUserLogger();

    void RestoreStderr();

    void ListenerThreadError();

    StderrListerenerThread m_stderrListenerThread;

#ifndef _WIN32
    FileWrapper m_pipeReadEnd;
    FileWrapper m_dupStderr; // duplicate of stderr that saves the original destination of stderr
#else
#endif

};

/**
 * This class represents the wrapper around the BE log message as passed through stderr
 */
class LogMessageWrapper
{
public:

    /**
     * Constructor for use by device side
     * @param id    the ID of the NDRange command
     * @param beMsg the string holding the message from BE containing the calculated local work size
     */
    LogMessageWrapper(cl_dev_cmd_id id, const std::string& beMsg) : m_id(id), m_beMsg(beMsg) { Serialize(); }

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
     * @return the string holding the message from BE containing the calculated local work size
     */
    std::string GetBeMsg() const { return m_beMsg; }

    /**
     * the raw string from which the log message should be parsed
     */
    std::string GetRawString() const { return m_rawStr; }

private:

    void Serialize();

    void Unserialize();

    cl_dev_cmd_id m_id;
    std::string m_beMsg;
    std::string m_rawStr;

};

class IOError : public std::runtime_error
{
public:

    explicit IOError();

};

}}}
