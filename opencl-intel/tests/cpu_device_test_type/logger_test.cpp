// Copyright (c) 2006 Intel Corporation
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

#include "logger_test.h"
#include "CL/cl.h"
#include "cl_device_api.h"
#include "cl_sys_defines.h"
#include "cpu_dev_test.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CLIENT_NUM 10

static char *pLoggerClient[MAX_CLIENT_NUM];

static cl_int iLastClientId = 0;

bool InitLoggerTest() {
  // initialize the logger:
  iLastClientId = 0;
  for (int i = 0; i < MAX_CLIENT_NUM; i++) {
    pLoggerClient[i] = NULL;
  }
  return true;
}
// Create logger callback
cl_int CPUTestLogger::clLogCreateClient(cl_int device_id,
                                        const char *client_name,
                                        cl_int *client_id) {
  if (NULL == client_id) {
    return CL_INVALID_VALUE;
  }

  if (iLastClientId >= MAX_CLIENT_NUM) {
    return CL_INVALID_VALUE;
  }
  pLoggerClient[iLastClientId] = STRDUP(client_name);
  *client_id = iLastClientId++;

  return CL_SUCCESS;
}

// Release logger callback
cl_int CPUTestLogger::clLogReleaseClient(cl_int client_id) {
  if (client_id < iLastClientId && NULL != pLoggerClient[client_id]) {
    delete pLoggerClient[client_id];
    pLoggerClient[client_id] = NULL;
  }

  return CL_SUCCESS;
}

// Add Line logger callback
cl_int CPUTestLogger::clLogAddLine(cl_int client_id, cl_int log_level,
                                   const char *IN source_file,
                                   const char *IN function_name,
                                   cl_int line_num, ...) {
  if (client_id < iLastClientId && NULL != pLoggerClient[client_id]) {
    va_list va;
    va_start(va, line_num);
    const char *message = va_arg(va, char *);
    assert(message && "Printf-style format string in CPUINFOLOG is NULL");
    // Printing to stdout brokes lit.googletest runner
    fprintf(stderr, "Source file name is %s function_name %s line num %d ",
            source_file, function_name, line_num);
    vfprintf(stderr, message, va);
    fprintf(stderr, "\n");

    va_end(va);
  }
  return CL_SUCCESS;
}
