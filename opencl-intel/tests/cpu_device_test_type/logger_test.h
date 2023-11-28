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
//  logger_test.h
//  Include the logger test API's
///////////////////////////////////////////////////////////

#pragma once
#include "cl_device_api.h"

bool InitLoggerTest();

class CPUTestLogger : public IOCLDevLogDescriptor {
public:
  // Create logger callback
  cl_int clLogCreateClient(cl_int device_id, const char *client_name,
                           cl_int *client_id);

  // Release logger callback
  cl_int clLogReleaseClient(cl_int client_id);

  // Add Line logger callback
  cl_int clLogAddLine(cl_int client_id, cl_int log_level,
                      const char *IN source_file, const char *IN function_name,
                      cl_int line_num, ...);
};
