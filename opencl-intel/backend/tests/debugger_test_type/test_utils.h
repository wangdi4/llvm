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

#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <string>
#include <vector>

// Read the contents of a file into a string. If there's a problem opening the
// file return an empty string.
//
std::string read_file_contents(std::string filename);

// Split a string to tokens by the given delimiters
//
std::vector<std::string> tokenize(const std::string &str,
                                  const std::string &delims);

// Start capturing stdout
//
void CaptureStdout();

// Start capturing stderr
//
void CaptureStderr();

// Stop capturing stdout and return the captured string
//
std::string GetCapturedStdout();

// Stop capturing stderr and return the captured string
//
std::string GetCapturedStderr();

#endif // TEST_UTILS_H
