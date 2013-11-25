#ifdef _WIN32

#include "DebuggerPipeWrapper.h"
#include <sstream>

#define CONTENT_SIZE 7

DebuggerPipeWrapper::DebuggerPipeWrapper(): m_isDeubggingEnabled(false), m_debuggingPort(0)
{
}

DebuggerPipeWrapper::~DebuggerPipeWrapper()
{
}

const bool DebuggerPipeWrapper::init(std::string pipeName)
{
    HANDLE hPipe;
    BOOL fSuccess = false;
    DWORD lastError = 0;
    TCHAR *pipeNameConv = new TCHAR[pipeName.size()+1];
    LPTSTR lpszPipename;

    wsprintf(pipeNameConv, TEXT("%hs"), pipeName.c_str());
    lpszPipename = pipeNameConv;

    hPipe = CreateFile( 
        lpszPipename,   // pipe name 
        GENERIC_READ,   // read and write access 
        0,              // no sharing 
        NULL,           // default security attributes
        OPEN_EXISTING,  // opens existing pipe 
        0,              // default attributes 
        NULL);          // no template file 
 
    // Exit if the pipe handle is not valid
    if (hPipe == INVALID_HANDLE_VALUE) 
    {
        delete[] pipeNameConv;
        return false; 
    }

    std::string pipeContent;
    bool isValid = readPipeContent(hPipe, CONTENT_SIZE*sizeof(wchar_t), pipeContent); 
    
    if (!isValid)
    {
        delete[] pipeNameConv;
        return false;
    }

    std::size_t found = pipeContent.find(";");
    if (found == std::string::npos)
    {
        // Something is worng with the string read
        delete[] pipeNameConv;
        return false;
    }
    else
    {
        std::stringstream enabledStringstream(pipeContent.substr(0, found));
        std::stringstream portStringstream(pipeContent.substr(found+1));
        enabledStringstream >> m_isDeubggingEnabled;
        portStringstream >> m_debuggingPort;
    }

    CloseHandle(hPipe);
    delete[] pipeNameConv;
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

const bool DebuggerPipeWrapper::readPipeContent(HANDLE pipeHanle, DWORD bytesToRead, std::string &content) const
{
    DWORD bytesRead = 0;
    DWORD offset = 0;
    DWORD lastError = 0;
    BOOL fSuccess = false;
    wchar_t  chBuf[CONTENT_SIZE+1] = {L'\0'};

    while (bytesRead < bytesToRead)
    {
        fSuccess = ReadFile( 
            pipeHanle,                    // pipe handle 
            chBuf + offset,               // buffer to receive reply 
            CONTENT_SIZE*sizeof(wchar_t), // buffer size
            &bytesRead,                   // number of bytes read 
            NULL);                        // not overlapped 

        // Break if all bytes were read
        offset += bytesRead;
        if (offset == bytesToRead) 
        {
            break;
        }

        // Break if some error occured during the pipe read
        lastError = GetLastError();
        if ( ! fSuccess && lastError != ERROR_MORE_DATA ) {
            break;
        }
    }

    if (offset != bytesToRead)
    {
        // Failed to read the message
        return false;
    }
    else
    {
        std::stringstream contentStream;

        for (int i = 0 ; i < bytesToRead / sizeof(wchar_t) ; ++i )
        {
            int     bChar = 0;
            wchar_t  wChar = 0;

            bChar = wctob( chBuf[i] );
            if (bChar == WEOF)
            {
                return false;
            }
            contentStream << (char)bChar;
        }
        content = contentStream.str();
        return true;
    }
}

#endif // _WIN32
