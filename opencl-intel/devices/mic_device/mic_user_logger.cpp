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


#include <stdexcept>
#include <cassert>
#include <cerrno>
#include <cstdio>
#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#ifndef _WIN32
#include <unistd.h>
#else
#include <io.h>
#include <fcntl.h>
#include <limits>
#endif
#include "mic_user_logger.h"
#include "cl_user_logger.h"

using namespace Intel::OpenCL::MICDevice;
using std::string;
using std::stringstream;
using std::ios_base;
using std::istream;
using std::vector;
using Intel::OpenCL::Utils::FrameworkUserLogger;
using Intel::OpenCL::Utils::g_pUserLogger;

static const string STDERR_LOG_PREFIX = "OPENCL_USER_LOGGER:";

// MicUserLogger methods:

MicUserLogger& MicUserLogger::Instance()
{
    static MicUserLogger instance;
    return instance;
}

MicUserLogger::MicUserLogger()
{
    int pipefd[2];
#ifdef _WIN32
    if (-1 == _pipe(pipefd, 512, _O_TEXT))  // I've taken this size from an example on the Internet
#else
    if (-1 == pipe(pipefd))
#endif    
    {
        throw IOError();
    }
    m_pipeReadEnd = pipefd[0];
    m_dupStderr = dup(fileno(stderr));
    if (-1 == m_dupStderr)
    {
        throw IOError();
    }
    // all prints to stderr will not be printed to it, but to the pipe
    if (dup2(pipefd[1], fileno(stderr)) == -1)
    {
        throw IOError();
    }
    close(pipefd[1]);   // after duplicating stderr to the pipe its write side doesn't need to be open
    m_stderrListenerThread.Init(this);
    m_stderrListenerThread.Start();
}

MicUserLogger::~MicUserLogger()
{
    m_stderrListenerThread.Terminate(nullptr);
    m_stderrListenerThread.Join();
    RestoreStderr();
}

void MicUserLogger::RestoreStderr()
{
    dup2(fileno(stderr), m_dupStderr);
    close(m_dupStderr);
}

void MicUserLogger::ListenerThreadError()
{
    RestoreStderr();
    g_pUserLogger->PrintError("MIC user logger has terminated unexpectedly");
}

MicUserLogger::FileWrapper& MicUserLogger::FileWrapper::operator=(int fileDesc)
{
    assert(-1 == m_fileDesc);
    m_fileDesc = fileDesc;
    return *this;
}

MicUserLogger::FileWrapper::~FileWrapper()
{
    if (-1 != m_fileDesc)
    {
        close(m_fileDesc);        
    }
}

void MicUserLogger::StderrListerenerThread::HandleLogMessage(FILE* pipeFile)
{
    size_t szMsgLen;
    const int iFscanfRet = fscanf(pipeFile, " %lu ", &szMsgLen);
    if (EOF == iFscanfRet)
    {
        m_pLogger->ListenerThreadError();
        return;
    }
    ASSERT_RET(1 == iFscanfRet, "fscanf didn't read just one item");

    vector<char> buf(szMsgLen + 1);
    const size_t szFreadRet = fread(&buf[0], 1, szMsgLen, pipeFile);
    if (szFreadRet < szMsgLen)
    {
        m_pLogger->ListenerThreadError();
        return;
    }
    buf[szMsgLen] = '\0';

    Intel::OpenCL::Utils::LogMessageWrapper wrapper(&buf[0]);
}

RETURN_TYPE_ENTRY_POINT MicUserLogger::StderrListerenerThread::Run()
{
    FILE* pipeFile = fdopen(m_pLogger->m_pipeReadEnd, "r");
    if (nullptr == pipeFile) { m_pLogger->ListenerThreadError(); return NULL; }

    const string prefix(STDERR_LOG_PREFIX);
    while (true)
    {
        int c = fgetc(pipeFile);
        if (EOF == c) { m_pLogger->ListenerThreadError(); return nullptr; }
        size_t i = 0;
        while (c == prefix[i])
        {
            assert(i < prefix.size());
            c = fgetc(pipeFile);
            if (EOF == c) { m_pLogger->ListenerThreadError(); return nullptr; }
            ++i;
            if (prefix.size() == i)
            {
                HandleLogMessage(pipeFile);
                break;
            }
        }
        if (prefix.size() == i)
        {
            continue;
        }
        const char ch = c;    // what if we have to support big-endian for some reason?!
        write(m_pLogger->m_dupStderr, &ch, 1);
    }
    return nullptr;
}

// IOError methods:

IOError::IOError() : std::runtime_error(sys_errlist[errno])
{
    assert(errno < sys_nerr);
}
