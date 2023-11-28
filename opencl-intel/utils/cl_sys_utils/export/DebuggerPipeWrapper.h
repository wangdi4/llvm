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

#ifndef _DebuggerPipeWrapper_h_
#define _DebuggerPipeWrapper_h_

#ifdef _WIN32

#include <string>
#define NOMINMAX
#include <windows.h>

class DebuggerPipeWrapper {
public:
  DebuggerPipeWrapper();
  bool init(std::string pipeName);
  bool isDebuggingEnabled() const;
  unsigned int getDebuggingPort() const;

private:
  bool m_isDeubggingEnabled;
  unsigned int m_debuggingPort;

  bool readPipeContent(HANDLE pipeHanle, DWORD bytesToRead,
                       std::string &content) const;
};

#endif // _WIN32

#endif // _DebuggerPipeWrapper_h_
