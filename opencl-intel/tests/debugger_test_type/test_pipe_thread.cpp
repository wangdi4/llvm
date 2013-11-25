#ifdef _WIN32

#include "test_pipe_thread.h"
#include "host_program_common.h"

NamedPipeThread::NamedPipeThread()
{
    pipe = NULL;
    
    // Read Port string 
    portNumber = getenv ("CL_CONFIG_DBG_PORT_NUMBER");
    if (portNumber == NULL)
    {
        DTT_LOG("failed to read the port number from the environment variable, the test will probably fail");
    }
    
    string pipeNameString("\\\\.\\pipe\\INTEL_OCL_DBG_PIPE" + stringify(GetCurrentProcessId()));

    // Create a pipe to send the data
    pipe = CreateNamedPipeA(
        pipeNameString.c_str(), // name of the pipe
        PIPE_ACCESS_OUTBOUND,   // 1-way pipe -- send only
        PIPE_TYPE_BYTE,         // send data as a byte stream
        1,                      // only allow 1 instance of this pipe
        0,                      // no outbound buffer
        0,                      // no inbound buffer
        0,                      // use default wait time
        NULL                    // use default security attributes
    );

    if (pipe == NULL || pipe == INVALID_HANDLE_VALUE)
    {
        DTT_LOG("Failed to create outbound pipe instance");
        // look up error code here using GetLastError()
    }
}

RETURN_TYPE_ENTRY_POINT NamedPipeThread::Run()
{
    // This call blocks until a client process connects to the pipe
    BOOL result = ConnectNamedPipe(pipe, NULL);
    if (!result) 
    {
        DTT_LOG("Failed to make connection on named pipe");
        // Look up error code here using GetLastError()
        // Close the pipe
        CloseHandle(pipe); 
        return 1;
    }

    // Create the data string
    string portString(portNumber);
    wstring wPortString(portString.begin(), portString.end());
    wstring dataString = L"1;" + wPortString;

    // This call blocks until a client process reads all the data
    const wchar_t *data = dataString.c_str();

    DWORD numBytesWritten = 0;
    result = WriteFile(
        pipe,                           // handle to our outbound pipe
        data,                           // data to send
        wcslen(data) * sizeof(wchar_t), // length of data to send (bytes)
        &numBytesWritten,               // will store actual amount of data sent
        NULL                            // not using overlapped IO
    );

    if (result) 
    {
        DTT_LOG("WriteFile succeeded. Number of bytes sent" + stringify(numBytesWritten));
    } 
    else
    {
        DTT_LOG("WriteFile failed");
        // look up error code here using GetLastError()
    }

    // Close the pipe (automatically disconnects client too)
    CloseHandle(pipe);

    return 0;
}

#endif _WIN32
