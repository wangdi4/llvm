// INTEL CONFIDENTIAL
//
// Copyright 2012 Intel Corporation.
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

#ifdef _WIN32

#include "test_pipe_thread.h"
#include "cl_env.h"
#include "host_program_common.h"

NamedPipeThread::NamedPipeThread() {
  pipe = NULL;

  // Read Port string
  if (!Intel::OpenCL::Utils::getEnvVar(portNumber, "CL_CONFIG_DBG_PORT_NUMBER"))
    DTT_LOG("failed to read the port number from the environment variable, "
            "the test will probably fail");

  std::wstring pipeNameString(L"\\\\.\\pipe\\INTEL_OCL_DBG_PIPE" +
                              std::to_wstring(GetCurrentProcessId()));

  // Create a pipe to send the data
  pipe = CreateNamedPipeW(pipeNameString.c_str(), // name of the pipe
                          PIPE_ACCESS_OUTBOUND,   // 1-way pipe -- send only
                          PIPE_TYPE_BYTE,         // send data as a byte stream
                          1,   // only allow 1 instance of this pipe
                          0,   // no outbound buffer
                          0,   // no inbound buffer
                          0,   // use default wait time
                          NULL // use default security attributes
  );

  if (pipe == NULL || pipe == INVALID_HANDLE_VALUE) {
    DTT_LOG("Failed to create outbound pipe instance");
    // look up error code here using GetLastError()
  }
}

RETURN_TYPE_ENTRY_POINT NamedPipeThread::Run() {
  // This call blocks until a client process connects to the pipe
  BOOL result = ConnectNamedPipe(pipe, NULL);
  if (!result) {
    DTT_LOG("Failed to make connection on named pipe");
    // Look up error code here using GetLastError()
    // Close the pipe
    CloseHandle(pipe);
    return 1;
  }

  // Create the data string
  std::string dataString = "1;" + portNumber;

  // This call blocks until a client process reads all the data
  DWORD numBytesWritten = 0;
  result = WriteFile(pipe,                // handle to our outbound pipe
                     dataString.c_str(),  // data to send
                     dataString.length(), // length of data to send (bytes)
                     &numBytesWritten, // will store actual amount of data sent
                     NULL              // not using overlapped IO
  );

  if (result) {
    DTT_LOG("WriteFile succeeded. Number of bytes sent" +
            stringify(numBytesWritten));
  } else {
    DTT_LOG("WriteFile failed");
    // look up error code here using GetLastError()
  }

  // Close the pipe (automatically disconnects client too)
  CloseHandle(pipe);

  return 0;
}

#endif // _WIN32
