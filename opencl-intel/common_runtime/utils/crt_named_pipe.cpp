// Copyright (c) 2014 Intel Corporation
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
//
///////////////////////////////////////////////////////////

#include "crt_named_pipe.h"

#ifdef _WIN32
#include <sstream>

#define CONTENT_SIZE 7

using namespace OCLCRT::Utils;

ApiDebuggerPipeReader::ApiDebuggerPipeReader(): m_isDeubggingEnabled( false )
{
}

const bool ApiDebuggerPipeReader::init( std::string pipeName )
{
    HANDLE hPipe = nullptr;

    // Wait for available pipe instance for connection.
    BOOL res = WaitNamedPipeA( pipeName.c_str(), NMPWAIT_WAIT_FOREVER );
    if( !res )
    {
        return false;
    }

    // The hPipe handle (pipe client), is returned in byte-read mode.
    hPipe = CreateFileA(
        pipeName.c_str(),   // pipe name
        GENERIC_READ,       // read access
        0,                  // no sharing
        nullptr,               // default security attributes
        OPEN_EXISTING,      // opens existing pipe
        0,                  // default attributes
        nullptr );             // no template file

    // Exit if the pipe handle is not valid
    if( hPipe == INVALID_HANDLE_VALUE )
    {
        return false;
    }

    std::string pipeContent;
    bool isValid = readPipeContent( hPipe, CONTENT_SIZE, pipeContent );
    CloseHandle( hPipe );

    if( !isValid )
    {
        return false;
    }

    // syntax: "<0/1>;<5 digits port number>"
    std::size_t found = pipeContent.find( ";" );
    if( found == std::string::npos )
    {
        // Something is wrong with the string read
        return false;
    }
    else
    {
        std::stringstream enabledStringstream( pipeContent.substr( 0, found ) );
        enabledStringstream >> m_isDeubggingEnabled;
    }

    return true;
}

const bool ApiDebuggerPipeReader::isAPIDebuggingEnabled() const
{
    return m_isDeubggingEnabled;
}

const bool ApiDebuggerPipeReader::readPipeContent( HANDLE pipeHandle,
                                                   DWORD bytesToRead,
                                                   std::string &content ) const
{
    DWORD bytesRead = 0;
    DWORD offset = 0;
    DWORD lastError = 0;
    BOOL fSuccess = false;
    char chBuf[ CONTENT_SIZE+1 ] = {'\0'};

    // Data is read from the pipe as a stream of bytes,
    // So we might need several iterations to complete the whole read.
    while( offset < bytesToRead )
    {
        fSuccess = ReadFile(
            pipeHandle,                 // pipe handle
            ((char *)chBuf) + offset,   // buffer to receive reply
            CONTENT_SIZE - offset,      // buffer size
            &bytesRead,                 // number of bytes read
            nullptr );                     // not overlapped

        // A read operation is completed successfully when:
        //  all available bytes in the pipe are read, or
        //  when the specified number of bytes is read.
        if( !fSuccess )
        {
            break;
        }

        offset += bytesRead;
    }

    if( offset != bytesToRead )
    {
        // Failed to read the message
        return false;
    }

    content = chBuf;
    return true;
}

#endif // _WIN32

bool OCLCRT::Utils::isAPIDebuggingEnabled()
{
#ifdef _WIN32
    ApiDebuggerPipeReader pipeReader;
    std::stringstream pidString;

    pidString << GetCurrentProcessId();

    // The pipe name format is: \\servername\pipe\pipename
    // the "." means that the pipe is local.
    bool res = pipeReader.init( "\\\\.\\pipe\\INTEL_API_DBG_PIPE_" + pidString.str() );
    if( res && pipeReader.isAPIDebuggingEnabled() )
    {
        return true;
    }
#endif

    return false;
}
