#ifdef _WIN32

#include "DebuggerPipeWrapper.h"
#include <sstream>

#define CONTENT_SIZE 7

DebuggerPipeWrapper::DebuggerPipeWrapper(): m_isDeubggingEnabled(false), m_debuggingPort(0)
{
}

const bool DebuggerPipeWrapper::init(std::string pipeName)
{
    HANDLE hPipe = nullptr;

    // Wait for available pipe instance for connection.
    BOOL res = WaitNamedPipeA(pipeName.c_str(), NMPWAIT_WAIT_FOREVER);
    if (!res)
    {
        return false;
    }

    // The hPipe handle (pipe client), is returned in byte-read mode.
    hPipe = CreateFileA( 
        pipeName.c_str(), // pipe name 
        GENERIC_READ,     // read access 
        0,                // no sharing 
        nullptr,             // default security attributes
        OPEN_EXISTING,    // opens existing pipe 
        0,                // default attributes 
        nullptr);            // no template file 
 
    // Exit if the pipe handle is not valid
    if (hPipe == INVALID_HANDLE_VALUE) 
    {
        return false; 
    }

    std::string pipeContent;
    bool isValid = readPipeContent(hPipe, CONTENT_SIZE, pipeContent); 
    CloseHandle(hPipe);
    
    if (!isValid)
    {
        return false;
    }

    // syntax: "<0/1>;<5 digits port number>"
    std::size_t found = pipeContent.find(";");
    if (found == std::string::npos)
    {
        // Something is worng with the string read
        return false;
    }
    else
    {
        std::stringstream enabledStringstream(pipeContent.substr(0, found));
        std::stringstream portStringstream(pipeContent.substr(found+1));
        enabledStringstream >> m_isDeubggingEnabled;
        portStringstream >> m_debuggingPort;
    }

    return true;
}

const bool DebuggerPipeWrapper::isDebuggingEnabled() const
{
    return m_isDeubggingEnabled;
}

const unsigned int DebuggerPipeWrapper::getDebuggingPort() const
{
    return m_debuggingPort;
}

const bool DebuggerPipeWrapper::readPipeContent(HANDLE pipeHandle, DWORD bytesToRead, std::string &content) const
{
    DWORD bytesRead = 0;
    DWORD offset = 0;
    DWORD lastError = 0;
    BOOL fSuccess = false;
    char chBuf[CONTENT_SIZE+1] = {'\0'};

    // Data is read from the pipe as a stream of bytes,
    // So we might need several iterations to complete the whole read.
    while (offset < bytesToRead)
    {
        fSuccess = ReadFile( 
            pipeHandle,							   // pipe handle 
            chBuf + offset,			               // buffer to receive reply 
            CONTENT_SIZE - offset,                 // buffer size
            &bytesRead,							   // number of bytes read 
            nullptr);                                 // not overlapped 

        // A read operation is completed successfully when:
        //  all available bytes in the pipe are read, or
        //  when the specified number of bytes is read.
        if ( ! fSuccess ) {
            break;
        }

        offset += bytesRead;
    }

    if (offset != bytesToRead)
    {
        // Failed to read the message
        return false;
    }

    content = chBuf;
    return true;
}

#endif // _WIN32
