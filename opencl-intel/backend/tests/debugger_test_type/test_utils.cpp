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

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#ifdef _WIN32
#include <io.h>
#include <sys/stat.h>
#include <windows.h>
#else
#include <stdlib.h>
#include <unistd.h>
#endif

using namespace std;

string read_file_contents(string filename) {
  ifstream stream(filename.c_str());
  if (stream.fail()) {
    return "";
  }

  stringstream sstr;
  sstr << stream.rdbuf();
  return sstr.str();
}

vector<string> tokenize(const string &str, const string &delims) {
  string::size_type start_index, end_index;
  vector<string> ret;

  // Skip leading delimiters, to get to the first token
  start_index = str.find_first_not_of(delims);

  // While found a beginning of a new token
  //
  while (start_index != string::npos) {
    // Find the end of this token
    end_index = str.find_first_of(delims, start_index);

    // If this is the end of the string
    if (end_index == string::npos)
      end_index = str.length();

    ret.push_back(str.substr(start_index, end_index - start_index));

    // Find beginning of the next token
    start_index = str.find_first_not_of(delims, end_index);
  }

  return ret;
}

// Disable Microsoft deprecation warnings for POSIX functions called from
// this class (creat, dup, dup2, and close)
//
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996)
#endif // _MSC_VER

// Object that captures an output stream (stdout/stderr)
//
class CapturedStream {
public:
  // The ctor redirects the stream to a temporary file.
  CapturedStream(int fd) : fd_(fd), uncaptured_fd_(dup(fd)) {
#ifdef _WIN32
    char temp_dir_path[MAX_PATH + 1] = {'\0'};
    char temp_file_path[MAX_PATH + 1] = {'\0'};

    ::GetTempPathA(sizeof(temp_dir_path), temp_dir_path);
    const UINT success = ::GetTempFileNameA(temp_dir_path, "redir",
                                            0, // Generate unique file name.
                                            temp_file_path);
    if (success > 0) {
      const int captured_fd = creat(temp_file_path, _S_IREAD | _S_IWRITE);
      filename_ = temp_file_path;
      fflush(NULL);
      dup2(captured_fd, fd_);
      close(captured_fd);
    }
#else
    char filename_template[] = "/tmp/redirXXXXXX";
    const int captured_fd = mkstemp(filename_template);
    filename_ = filename_template;
    fflush(NULL);
    dup2(captured_fd, fd_);
    close(captured_fd);
#endif
  }

  ~CapturedStream() { remove(filename_.c_str()); }

  string GetCapturedString() {
    if (uncaptured_fd_ != -1) {
      // Restores the original stream.
      fflush(NULL);
      dup2(uncaptured_fd_, fd_);
      close(uncaptured_fd_);
      uncaptured_fd_ = -1;
    }

    const string content = read_file_contents(filename_);
    return content;
  }

private:
  const int fd_; // A stream to capture.
  int uncaptured_fd_;
  // Name of the temporary file holding the output.
  string filename_;
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#if defined(_MSC_VER)
const int kStdOutFileno = 1;
const int kStdErrFileno = 2;
#else
const int kStdOutFileno = STDOUT_FILENO;
const int kStdErrFileno = STDERR_FILENO;
#endif // _MSC_VER

static CapturedStream *g_captured_stderr = NULL;
static CapturedStream *g_captured_stdout = NULL;

// Starts capturing an output stream (stdout/stderr).
static void CaptureStream(int fd, const char *stream_name,
                          CapturedStream **stream) {
  *stream = new CapturedStream(fd);
}

static string GetCapturedStream(CapturedStream **captured_stream) {
  const string content = (*captured_stream)->GetCapturedString();

  delete *captured_stream;
  *captured_stream = NULL;

  return content;
}

void CaptureStdout() {
  CaptureStream(kStdOutFileno, "stdout", &g_captured_stdout);
}

void CaptureStderr() {
  CaptureStream(kStdErrFileno, "stderr", &g_captured_stderr);
}

string GetCapturedStdout() { return GetCapturedStream(&g_captured_stdout); }

string GetCapturedStderr() { return GetCapturedStream(&g_captured_stderr); }
