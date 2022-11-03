// Copyright (c) 2006-2012 Intel Corporation
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
// common_methods.h

#include "common_methods.h"
#include "gtest_wrapper.h"
#include <fstream>
#include <iostream>
#include <malloc.h>
#include <stdlib.h>
#include <string>

#ifdef _WIN32
// for sleep function
#include <windows.h>
#else
// for sleep function
#include <unistd.h>
#endif

/**
 * Encapsulates common methods
 **/

// bineryFileToBuffer - returns (via return_val) content of file with sFileName
// name in current working directory caller is responsible for resources
// deallocation
void binaryFileToBuffer(const char **return_val, const char *sFileName,
                        size_t *return_length) {
  std::stringstream ss;
#if defined(_WIN32)
  // [QA]: correct hardcoded path
  // ss << "validation\\common_runtime_test_type\\" << sFileName;
  ss << "" << sFileName;
#else
  // [QA]: correct hardcoded path
  // ss << "validation/common_runtime_test_type/" << sFileName;
  ss << "" << sFileName;
#endif
  // try to open file
  std::ifstream file(ss.str().c_str(), std::fstream::in | std::fstream::binary |
                                           std::fstream::ate);
  // check if file was opened successfully
  ASSERT_FALSE(file.fail()) << "Could not open file " << ss.str();
  file.seekg(0, std::ios::end);
  size_t length = file.tellg();
  file.seekg(0, std::ios::beg);
  char *buffer = new char[length + 1];
  file.read(buffer, length);
  buffer[file.gcount()] = '\0';
  // close file
  file.close();
  // return buffer
  if (return_length != nullptr) {
    *return_length = length + 1;
  }
  *return_val = static_cast<const char *>(buffer);
}

// fileToBuffer - returns (via return_val) content of file with sFileName name
// in current working directory caller is responsible for resources deallocation
void fileToBuffer(const char **return_val, const char *sFileName,
                  size_t *return_length) {
  std::stringstream ss;
#if defined(_WIN32)
  // [QA]: correct hardcoded path
  // ss << "validation\\common_runtime_test_type\\" << sFileName;
  ss << "" << sFileName;
#else
  // [QA]: correct hardcoded path
  // ss << "validation/common_runtime_test_type/" << sFileName;
  ss << "" << sFileName;
#endif
  // try to open file
  std::ifstream file(ss.str().c_str());
  // check if file was opened successfully
  ASSERT_FALSE(file.fail()) << "Could not open file " << ss.str();
  file.seekg(0, std::ios::end);
  size_t length = file.tellg();
  file.seekg(0, std::ios::beg);
  char *buffer = new char[length + 1];
  file.read(buffer, length);
  buffer[file.gcount()] = '\0';
  // close file
  file.close();
  // return buffer
  if (return_length != nullptr) {
    *return_length = length + 1;
  }
  *return_val = static_cast<const char *>(buffer);
}

// getAllStrings - helper function - returns all strings in s separated by
// whitespace as vector of single strings
std::vector<std::string> getAllStrings(std::string s) {
  std::vector<std::string> vec;
  std::istringstream iss(s);
  int i = 0;
  do {
    std::string sub;
    iss >> sub;
    vec.push_back(sub);
  } while (iss);
  return vec;
}

// assertVectorInclusion - helper function - asserts that all elements of
// expected are in found
void assertVectorInclusion(std::vector<std::string> &expected,
                           std::vector<std::string> &found) {
  std::vector<std::string>::iterator itExp;
  std::vector<std::string>::iterator itVec;
  for (itExp = expected.begin(); itExp < expected.end(); itExp++) {
    // iterate over all expected extensions
    bool foundBool = false;
    for (itVec = found.begin(); itVec < found.end(); itVec++) {
      // iterate over all found extensions
      if (0 == itExp->compare(*itVec)) {
        // found this extension
        foundBool = true;
        continue;
      }
    }
    if (false == foundBool) {
      // did not find this extension
      EXPECT_TRUE(false) << "Extension " << *itExp << " was not returned";
    }
  }
}

//  sleepMS - will cause the running thread to sleep for milliseconds
void sleepMS(unsigned milliseconds) {
#ifdef _WIN32
  Sleep(milliseconds);
#else
  usleep(milliseconds * 1000);
#endif
}
