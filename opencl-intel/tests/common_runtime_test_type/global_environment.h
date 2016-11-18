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
// global_environment.h


#ifndef COMMON_RUNTIME_INVORONMENT_GTEST_
#define COMMON_RUNTIME_INVORONMENT_GTEST_

#include "ocl_wrapper.h"

#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <malloc.h>
#include <CL/cl.h>
#include <gtest/gtest.h>

/**
 * EnvironemntCommonRuntimeTestType - global setup of the tests environment
 * Determines environment properties and "decides" which tests to exclude.
 **/
class EnvironemntCommonRuntimeTestType : public ::testing::Environment  
{
protected:
	// SetUp - global set up (called once before all test are run)
	virtual void SetUp() 
	{
	}

	// TearDown - global tear down (called once after all test are run)
	virtual void TearDown() 
	{
	}
	
public:
	
};

#endif /* COMMON_RUNTIME_INVORONMENT_GTEST_ */