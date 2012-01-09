// Copyright (c) 2006-2007 Intel Corporation
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

#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <malloc.h>
#include <gtest/gtest.h>
#include "common_methods.h"

/**
 * Encapsulates common methods
 **/

// fileToBuffer - returns (via return_val) content of file with sFileName name in current working directory
// caller is responsible for resources deallocation
void fileToBuffer(const char** return_val, const char* sFileName)
{
	//	try to open file
	std::ifstream file(sFileName);
	//	check if file was opened successfully
	ASSERT_FALSE(file.fail());
	file.seekg(0, std::ios::end);
	size_t length = file.tellg();
	file.seekg(0, std::ios::beg);
	char* buffer = new char[length + 1];
	file.read(buffer, length);
	buffer[file.gcount()] = '\0';
	//	close file
	file.close();
	//	return buffer
	*return_val = static_cast<const char*> (buffer);
}





