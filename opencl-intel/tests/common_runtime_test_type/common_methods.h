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

#ifndef COMMON_METHODS_GTEST_
#define COMMON_METHODS_GTEST_

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <malloc.h>
#include <gtest/gtest.h>
#include <CL/cl.h>
/**
 * Encapsulates common methods
 **/

// fileToBuffer - returns (via return_val) content of file with sFileName name in current working directory
// caller is responsible for resources deallocation
void fileToBuffer(const char** return_val, const char* sFileName);

//	sleepMS - will cause the running thread to sleep for milliseconds
void sleepMS(unsigned milliseconds);

// getAllStrings - helper function - returns all strings in s separated by whitespace as vector of single strings
//
std::vector<std::string> getAllStrings(std::string s);

// getAllStrings - helper function - asserts that all elements of expected are in found
void assertVectorInclusion(
	std::vector<std::string>& expected, std::vector<std::string>& found);

#endif /* COMMON_METHODS_GTEST_ */