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

    // TODO: deal with this thread in shut-down
    class StderrListerenerThread : public Intel::OpenCL::Utils::OclThread
    {
    public:

        void Init(MicUserLogger* pLogger) { m_pLogger = pLogger; }

        RETURN_TYPE_ENTRY_POINT Run();

    private:

        MicUserLogger* m_pLogger;

        void HandleLogMessage(FILE* pipeFile);

    };

    MicUserLogger();

    void RestoreStderr();

    void ListenerThreadError();

    StderrListerenerThread m_stderrListenerThread;

    FileWrapper m_pipeReadEnd;
    FileWrapper m_dupStderr; // duplicate of stderr that saves the original destination of stderr

};

class IOError : public std::runtime_error
{
public:

    explicit IOError();

};

}}}
