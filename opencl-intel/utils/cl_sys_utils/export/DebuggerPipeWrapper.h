#ifndef _DebuggerPipeWrapper_h_
#define _DebuggerPipeWrapper_h_

#ifdef _WIN32

#include <string>
#include <windows.h> 

class DebuggerPipeWrapper {
public:
    DebuggerPipeWrapper();
    ~DebuggerPipeWrapper();
    const bool init(std::string pipeName);
    const bool isDebuggingEnabled() const;
    const unsigned int getDebuggingPort() const;
private:
    bool m_isDeubggingEnabled;
    unsigned int m_debuggingPort;

const bool readPipeContent(HANDLE pipeHanle, DWORD bytesToRead, std::string &content) const;
};

#endif // _WIN32

#endif // _DebuggerPipeWrapper_h_


