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

///////////////////////////////////////////////////////////
// logger_test.cpp
///////////////////////////////////////////////////////////

#include "CL/cl.h"
#include "cl_device_api.h"
#include "cpu_dev_test.h"

#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#ifndef _WIN32
#include <string.h>
#endif

#include "logger_test.h"
#include "cl_sys_defines.h"
#define MAX_CLIENT_NUM 10

static char * pLoggerClient[MAX_CLIENT_NUM];

static cl_int iLastClientId = 0;

bool InitLoggerTest()
{
	//initialize the logger:
	iLastClientId = 0;
	for (int i=0;i<MAX_CLIENT_NUM;i++)
	{
		pLoggerClient[i] = NULL;
	}
	return true;
}
//Create logger callback
cl_int CPUTestLogger::clLogCreateClient(cl_int device_id, const wchar_t* client_name, cl_int * client_id)
{
	if (NULL == client_id)
	{
		return CL_INVALID_VALUE;
	}

	if(iLastClientId >= MAX_CLIENT_NUM)
	{
		return CL_INVALID_VALUE;
	}
	pLoggerClient[iLastClientId] = STRDUP((char*)client_name);
	if (NULL == pLoggerClient)
	{
		return CL_INVALID_VALUE;
	}
	*client_id = iLastClientId++;

	return CL_SUCCESS;
}

//Release logger callback
cl_int CPUTestLogger::clLogReleaseClient(cl_int client_id)
{
	if (client_id < iLastClientId && NULL != pLoggerClient[client_id])
	{
		delete pLoggerClient[client_id];
		pLoggerClient[client_id] = NULL;
	}

	return CL_SUCCESS;
}

//Add Line logger callback
cl_int CPUTestLogger::clLogAddLine(cl_int client_id, cl_int log_level, 
	const wchar_t* IN source_file, 
	const wchar_t* IN function_name, 
	cl_int line_num, 
	const wchar_t* IN message, ...)
{
	if (client_id < iLastClientId && NULL != pLoggerClient[client_id])
	{
		va_list va;
		va_start(va, message);

		printf("Source file name is %ws function_name %ws line num %d %ws\n", source_file, function_name, line_num, message, va);

		va_end(va);

	}
	return CL_SUCCESS;
}

cl_int CPUTestLogger::clLogAddLine(cl_int client_id, cl_int log_level, 
	const char* IN source_file, 
	const char* IN function_name, 
	cl_int line_num, 
	const wchar_t* IN message, ...)
{
	if (client_id < iLastClientId && NULL != pLoggerClient[client_id])
	{
		int err = 0;
		size_t sourceFileSize = 0;
		err = MULTIBYTE_TO_WIDE_CHARACTER_S(&sourceFileSize, NULL, 0, source_file, strlen(source_file));
		if (err != 0)
		{
		    return -1;
		}
		wchar_t* wSourceFile = (wchar_t*)malloc(sizeof(wchar_t) * sourceFileSize);
		err = MULTIBYTE_TO_WIDE_CHARACTER_S(&sourceFileSize, wSourceFile, sourceFileSize, source_file, sourceFileSize - 1);
		if (err != 0)
		{
		    free(wSourceFile);
		    return -1;
		}
		size_t functionNameSize = 0;
		err = MULTIBYTE_TO_WIDE_CHARACTER_S(&functionNameSize, NULL, 0, function_name, strlen(function_name));
		if (err != 0)
		{
		    return -1;
		}
		wchar_t* wFunctionName = (wchar_t*)malloc(sizeof(wchar_t) * functionNameSize);
		err = MULTIBYTE_TO_WIDE_CHARACTER_S(&functionNameSize, wFunctionName, functionNameSize, function_name, functionNameSize - 1);
		if (err != 0)
		{
		    free(wSourceFile);
		    free(wFunctionName);
		    return -1;
		}

		va_list va;
		va_start(va, message);

		printf("Source file name is %ws function_name %ws line num %d %ws\n", wSourceFile, wFunctionName, line_num, message, va);

		va_end(va);
    	
		free(wSourceFile);
		free(wFunctionName);
	}
	return CL_SUCCESS;
}