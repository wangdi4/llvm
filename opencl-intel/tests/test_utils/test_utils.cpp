// INTEL CONFIDENTIAL
//
// Copyright 2012-2021 Intel Corporation.
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

#include "test_utils.h"
#include "CL/cl.h"
#include "cl_sys_info.h"
#include "cl_types.h"
#include "cl_utils.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include <algorithm>
#include <cstdio>
#include <cstring>

#include "gtest_wrapper.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/resource.h>
#endif

using namespace llvm;
using namespace std;

bool CheckCondition(const char * name, bool condition)
{
	if (condition)
	{
		printf("SUCCESS: %s\n",name);
	}
	else
	{
		printf("FAIL: %s\n",name);
	}
	return condition;
}

bool SilentCheckCondition(const char * name, bool condition)
{
	if (!condition)
	{
		printf("FAIL: %s\n",name);
	}
	return condition;
}

bool Check(const char * name, cl_int expected, cl_int result)
{
	if (expected == result)
	{
		printf("SUCCESS: %s\n",name);
	}
	else
	{
		printf("FAIL: %s\n",name);
	}
	printf("\t\texpected = %s, result = %s\n", ClErrTxt(expected), ClErrTxt(result));
	return (expected == result);
}

bool SilentCheck(const char* name, cl_int expected, cl_int result)
{
	if (expected == result)
	{
		return true;
	}
	printf("FAIL: %s\n",name);
	printf("\t\texpected = %s, result = %s\n", ClErrTxt(expected), ClErrTxt(result));
	return false;
}

bool CheckStr(const char * name, const char * expected, char * result)
{
	int iRes = strcmp(expected, result);
	bool bRes = (iRes == 0);

	if (bRes)
	{
		printf("SUCCESS: %s\n",name);
	}
	else
	{
		printf("FAIL: %s\n",name);
	}
	printf("\t\texpected = %s, result = %s\n", expected, result);
	return bRes;
}

bool SilentCheckStr(const char * name, char * expected, char * result)
{
	int iRes = strcmp(expected, result);
	bool bRes = (iRes == 0);

	if (!bRes)
	{
		printf("FAIL: %s\n",name);
		printf("\t\texpected = %s, result = %s\n", expected, result);
	}
	return bRes;
}

bool CheckInt(const char * name, cl_long expected, cl_long result)
{
	bool bRes = (expected == result);

	if (bRes)
	{
		printf("SUCCESS: %s\n",name);
	}
	else
	{
		printf("FAIL: %s\n",name);
	}
#if defined(_WIN32)
        printf("\t\texpected = %lld, result = %lld\n", expected, result);
#else
        printf("\t\texpected = %ld, result = %ld\n", expected, result);
#endif
        return bRes;
}

bool SilentCheckInt(const char * name, cl_long expected, cl_long result)
{
	bool bRes = (expected == result);

	if (!bRes)
	{
		printf("FAIL: %s\n",name);
#if defined(_WIN32)
                printf("\t\texpected = %lld, result = %lld\n", expected,
                       result);
#else
                printf("\t\texpected = %ld, result = %ld\n", expected,
                       result);
#endif
        }
	return bRes;
}

bool CheckSize(const char * name, size_t expected, size_t result)
{
	bool bRes = (expected == result);

	if (bRes)
	{
		printf("SUCCESS: %s\n",name);
	}
	else
	{
		printf("FAIL: %s\n",name);
	}
#if defined(_WIN32)
        printf("\t\texpected = %lld, result = %lld\n", expected, result);
#else
        printf("\t\texpected = %ld, result = %ld\n", expected, result);
#endif
        return bRes;
}

bool CheckHandle(const char * name, cl_platform_id expected, cl_platform_id result)
{
	bool bRes = (expected == result);
	return CheckHandleImpl(name, (void*)expected, (void*)result, bRes);
}

bool CheckHandle(const char * name, cl_device_id expected, cl_device_id result)
{
	bool bRes = (expected == result);
	return CheckHandleImpl(name, (void*)expected, (void*)result, bRes);
}

bool CheckHandle(const char * name, cl_context expected, cl_context result)
{
	bool bRes = (expected == result);
	return CheckHandleImpl(name, (void*)expected, (void*)result, bRes);
}

bool CheckHandle(const char * name, cl_command_queue expected, cl_command_queue result)
{
	bool bRes = (expected == result);
	return CheckHandleImpl(name, (void*)expected, (void*)result, bRes);
}

bool CheckHandle(const char * name, cl_mem expected, cl_mem result)
{
	bool bRes = (expected == result);
	return CheckHandleImpl(name, (void*)expected, (void*)result, bRes);
}

bool CheckHandle(const char * name, cl_program expected, cl_program result)
{
	bool bRes = (expected == result);
	return CheckHandleImpl(name, (void*)expected, (void*)result, bRes);
}

bool CheckHandle(const char * name, cl_kernel expected, cl_kernel result)
{
	bool bRes = (expected == result);
	return CheckHandleImpl(name, (void*)expected, (void*)result, bRes);
}

bool CheckHandle(const char * name, cl_event expected, cl_event result)
{
	bool bRes = (expected == result);
	return CheckHandleImpl(name, (void*)expected, (void*)result, bRes);
}

bool CheckHandle(const char * name, cl_sampler expected, cl_sampler result)
{
	bool bRes = (expected == result);
	return CheckHandleImpl(name, (void*)expected, (void*)result, bRes);
}

bool CheckHandleImpl(const char * name, void* expected, void* result, bool bRes)
{
	if (bRes)
	{
		printf("SUCCESS: %s\n",name);
	}
	else
	{
		printf("FAIL: %s\n",name);
	}
	printf("\t\texpected = %p, result = %p\n", expected, result);
	return bRes;
}

bool SilentCheckSize(const char * name, size_t expected, size_t result)
{
	bool bRes = (expected == result);

	if (!bRes)
	{
		printf("FAIL: %s\n",name);
#if defined(_WIN32)
                printf("\t\texpected = %lld, result = %lld\n", expected,
                       result);
#else
                printf("\t\texpected = %ld, result = %ld\n", expected,
                       result);
#endif
        }
	return bRes;
}

bool CheckBuildStatus(const char * name, cl_build_status expected, cl_build_status result)
{
	bool bRes = (expected == result);

	if (bRes)
	{
		printf("SUCCESS: %s\n",name);
	}
	else
	{
		printf("FAIL: %s\n",name);
	}
	printf("\t\texpected = ");
	switch(expected)
	{
	case CL_BUILD_IN_PROGRESS:
		printf("CL_BUILD_IN_PROGRESS");
		break;
	case CL_BUILD_SUCCESS:
		printf("CL_BUILD_SUCCESS");
		break;
	case CL_BUILD_ERROR:
		printf("CL_BUILD_ERROR");
		break;
	case CL_BUILD_NONE:
	default:
		printf("CL_BUILD_NONE");
		break;
	}
	printf(" result = ");
	switch(result)
	{
	case CL_BUILD_IN_PROGRESS:
		printf("CL_BUILD_IN_PROGRESS");
		break;
	case CL_BUILD_SUCCESS:
		printf("CL_BUILD_SUCCESS");
		break;
	case CL_BUILD_ERROR:
		printf("CL_BUILD_ERROR");
		break;
	case CL_BUILD_NONE:
	default:
		printf("CL_BUILD_NONE");
		break;
	}
	printf("\n");
	return bRes;
}

bool SilentCheckBuildStatus(const char * name, cl_build_status expected, cl_build_status result)
{
	bool bRes = (expected == result);

	if (!bRes)
	{
		printf("FAIL: %s\n",name);
		printf("\t\texpected = ");
		switch(expected)
		{
		case CL_BUILD_IN_PROGRESS:
			printf("CL_BUILD_IN_PROGRESS");
			break;
		case CL_BUILD_SUCCESS:
			printf("CL_BUILD_SUCCESS");
			break;
		case CL_BUILD_ERROR:
			printf("CL_BUILD_ERROR");
			break;
		case CL_BUILD_NONE:
		default:
			printf("CL_BUILD_NONE");
			break;
		}
		printf(" result = ");
		switch(result)
		{
		case CL_BUILD_IN_PROGRESS:
			printf("CL_BUILD_IN_PROGRESS");
			break;
		case CL_BUILD_SUCCESS:
			printf("CL_BUILD_SUCCESS");
			break;
		case CL_BUILD_ERROR:
			printf("CL_BUILD_ERROR");
			break;
		case CL_BUILD_NONE:
		default:
			printf("CL_BUILD_NONE");
			break;
		}
		printf("\n");
	}
	return bRes;
}
bool BuildProgramSynch(cl_context	        context,
					   cl_uint              count,
					   const char **        strings,
					   const size_t *       lengths,
					   const char *         options,
					   cl_program *         program_ret)
{
	bool bRes = true;
	cl_int iRes = CL_SUCCESS;
	cl_program program = 0;

	printf("--------------------------------------------------------------\n");
	printf("Create and build new program (from source) for the context %p:\n", (void*)context);
	printf("--------------------------------------------------------------\n");
	//print source
	printf("building the following source code:\n");
	for (unsigned int i=0; i<count; ++i)
	{
		printf("%s",strings[i]);
	}

	printf("\n get devices of the context\n");
	size_t szNumDevices = 0, szNumDevicesRet = 0;
	iRes = clGetContextInfo(context, CL_CONTEXT_DEVICES, 0, NULL, &szNumDevicesRet);
	bRes = SilentCheck("clGetContextInfo (CL_CONTEXT_DEVICES) - size only", CL_SUCCESS, iRes);
	if (!bRes)
	{
		return bRes;
	}
	szNumDevices = szNumDevicesRet / sizeof(cl_device_id);
	bRes = SilentCheckCondition("there must be at least 1 device (szNumDevices > 0)", szNumDevices > 0);
	if (!bRes)
	{
		return bRes;
	}
	cl_device_id * devices = new cl_device_id[szNumDevices];
	iRes = clGetContextInfo(context, CL_CONTEXT_DEVICES, sizeof(cl_device_id) * szNumDevices, devices, &szNumDevicesRet);
	bRes = SilentCheckSize("clGetContextInfo (CL_CONTEXT_DEVICES)", CL_SUCCESS, iRes);
	if (!bRes)
	{
		delete[] devices;
		return bRes;
	}
	bRes = SilentCheckSize("clGetContextInfo (CL_CONTEXT_DEVICES) - check returned size", sizeof(cl_device_id) * szNumDevices, szNumDevicesRet);
	if (!bRes)
	{
		delete[] devices;
		return bRes;
	}
	printf("the devices are: ");
	for (unsigned int i=0; i<szNumDevices; ++i)
	{
		printf("%p", (void*)devices[i]);
		if (i < szNumDevices-1)
		{
			printf(", ");
		}
	}
	printf("\n");

	program = clCreateProgramWithSource(context, count, strings, lengths, &iRes);
	bRes = SilentCheck("clCreateProgramWithSource", CL_SUCCESS, iRes);
	if (false == bRes)
	{
		delete[] devices;
		return bRes;
	}
	printf("program id = %p\n", (void*)program);
	printf("check new program\n");

	cl_context program_context = 0;
	cl_uint uiProgramNumDevices = 0;
	iRes = clGetProgramInfo(program, CL_PROGRAM_CONTEXT, sizeof(cl_context), &program_context, NULL);
	bRes = SilentCheck("clGetProgramInfo (CL_PROGRAM_CONTEXT)", CL_SUCCESS, iRes);
	bRes = CheckHandle("check program's context", context, program_context);
	if (false == bRes)
	{
		delete[] devices;
		return bRes;
	}

	iRes = clGetProgramInfo(program, CL_PROGRAM_NUM_DEVICES, sizeof(cl_uint), &uiProgramNumDevices, NULL);
	bRes = SilentCheck("clGetProgramInfo (CL_PROGRAM_NUM_DEVICES)", CL_SUCCESS, iRes);
	bRes = SilentCheckInt("check program's devices number", (cl_int)szNumDevices, (cl_int)uiProgramNumDevices);
	if (false == bRes)
	{
		delete[] devices;
		return bRes;
	}

	iRes = clBuildProgram(program, (cl_uint)szNumDevices, devices, options, NULL, NULL);
	bRes = SilentCheck("clBuildProgram (Synch)", CL_SUCCESS, iRes);

	if (!bRes)
	{
		for (unsigned int i=0; i<szNumDevices; ++i)
		{
			size_t szLogLenth = 0;
			printf("build failed, error log for device %p:\n", (void*)devices[i]);
			iRes = clGetProgramBuildInfo(program, devices[i], CL_PROGRAM_BUILD_LOG, 0, NULL, &szLogLenth);
			bRes = SilentCheck("clGetProgramBuildInfo (CL_PROGRAM_BUILD_LOG) - size only", CL_SUCCESS, iRes);
			if (!bRes)
			{
				delete[] devices;
				return bRes;
			}
			char * pLog = new char[szLogLenth];
			iRes = clGetProgramBuildInfo(program, devices[i], CL_PROGRAM_BUILD_LOG, szLogLenth, pLog, NULL);
			bRes = SilentCheck("clGetProgramBuildInfo (CL_PROGRAM_BUILD_LOG)", CL_SUCCESS, iRes);
			if (!bRes)
			{
				delete[] devices;
				delete[] pLog;
				return bRes;
			}
			printf("%s",pLog);
			delete[] pLog;

			cl_build_status clBuildStatus = CL_BUILD_NONE;
			size_t szBuildStatusRet = 0;
			iRes = clGetProgramBuildInfo(program, devices[i], CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status), &clBuildStatus, &szBuildStatusRet);
			bRes = SilentCheck("clGetProgramBuildInfo (CL_PROGRAM_BUILD_STATUS)", CL_SUCCESS, iRes);
			if (!bRes)
			{
				delete[] devices;
				return bRes;
			}
			bRes = SilentCheckSize("clGetProgramBuildInfo (CL_PROGRAM_BUILD_STATUS) - check returned size", sizeof(cl_build_status), szBuildStatusRet);
			if (!bRes)
			{
				delete[] devices;
				return bRes;
			}
			bRes = SilentCheckBuildStatus("check build status", CL_BUILD_ERROR, clBuildStatus);
			if (!bRes)
			{
				delete[] devices;
				return bRes;
			}
		}
	}
	printf("build succeeded!!!\n");
	for (unsigned int i=0; i<szNumDevices; ++i)
	{
		cl_build_status clBuildStatus = CL_BUILD_NONE;
		size_t szBuildStatusRet = 0;

		printf("check build for device %p:\n", (void*)devices[i]);
		iRes = clGetProgramBuildInfo(program, devices[i], CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status), &clBuildStatus, &szBuildStatusRet);
		bRes = SilentCheck("clGetProgramBuildInfo (CL_PROGRAM_BUILD_STATUS)", CL_SUCCESS, iRes);
		if (!bRes)
		{
			delete[] devices;
			return bRes;
		}

		bRes = SilentCheckSize("clGetProgramBuildInfo (CL_PROGRAM_BUILD_STATUS) - check returned size", sizeof(cl_build_status), szBuildStatusRet);
		if (!bRes)
		{
			delete[] devices;
			return bRes;
		}

		bRes = SilentCheckBuildStatus("check build status", CL_BUILD_SUCCESS, clBuildStatus);
		if (!bRes)
		{
			delete[] devices;
			return bRes;
		}
	}
	printf("get binaries and try to build new program with these binaries \n");

	size_t szBinarySizesRet = 0;
	size_t pBinarySizes[4096] = {0};
	unsigned char * binaries[4096] = {NULL};
	iRes = clGetProgramInfo(program, CL_PROGRAM_BINARY_SIZES, 4096 * sizeof(size_t), pBinarySizes, &szBinarySizesRet);
	bRes = SilentCheck("clGetProgramInfo (CL_PROGRAM_BINARY_SIZES)", CL_SUCCESS, iRes);
	if (!bRes)
	{
		delete[] devices;
		return bRes;
	}

	for (unsigned int i=0; i<szNumDevices; ++i)
	{
		binaries[i] = new unsigned char[pBinarySizes[i]];
	}

	iRes = clGetProgramInfo(program, CL_PROGRAM_BINARIES, 4096 * sizeof(unsigned char*), binaries, NULL);
	bRes = SilentCheck("clGetProgramInfo (CL_PROGRAM_BINARIES)", CL_SUCCESS, iRes);
	if (!bRes)
	{
		for (unsigned int i=0; i<szNumDevices; ++i)
		{
			delete[] binaries[i];
		}
		delete[] devices;
		return bRes;
        }

        cl_program new_program = 0;
        new_program = clCreateProgramWithBinary(
            context, (cl_uint)szNumDevices, devices, pBinarySizes,
            const_cast<const unsigned char **>(binaries), NULL, &iRes);
        bRes = SilentCheck("clCreateProgramWithBinary (new program)",
                           CL_SUCCESS, iRes);
        if (!bRes) {
          for (unsigned int i = 0; i < szNumDevices; ++i) {
            delete[] binaries[i];
          }
          delete[] devices;
          return bRes;
        }

        iRes = clBuildProgram(new_program, (cl_uint)szNumDevices, devices, options, NULL, NULL);
	bRes = SilentCheck("clBuildProgram (new program)", CL_SUCCESS, iRes);
	if (!bRes)
	{
		for (unsigned int i=0; i<szNumDevices; ++i)
		{
			delete[] binaries[i];
		}
		delete[] devices;
		return bRes;
		clReleaseProgram(new_program);
	}

	for (unsigned int i=0; i<szNumDevices; ++i)
	{
		delete[] binaries[i];
	}
	delete[] devices;
	*program_ret = program;
	clReleaseProgram(new_program);
	return bRes;
}

void GetBuildLog(cl_device_id device, cl_program program, std::string &log) {
  size_t logSize = 0;
  cl_int err = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0,
                                     nullptr, &logSize);
  ASSERT_EQ(CL_SUCCESS, err) << "clGetProgramBuildInfo failed";
  log.resize(logSize);
  err = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
                              logSize, &log[0], nullptr);
  ASSERT_EQ(CL_SUCCESS, err) << "clGetProgramBuildInfo failed";
}

void CreateAndBuildProgramFromProgramBinaries(cl_context context,
                                              cl_device_id device,
                                              const std::string &buildOptions,
                                              cl_program srcProgram,
                                              cl_program &dstProgram) {
  cl_int err;
  size_t binarySize;
  err = clGetProgramInfo(srcProgram, CL_PROGRAM_BINARY_SIZES,
                         sizeof(binarySize), &binarySize, nullptr);
  ASSERT_EQ(CL_SUCCESS, err)
      << "clGetProgramInfo CL_PROGRAM_BINARY_SIZES failed";
  std::vector<unsigned char> binary(binarySize);
  const unsigned char *binaries[1] = {&binary[0]};
  err = clGetProgramInfo(srcProgram, CL_PROGRAM_BINARIES, sizeof(binaries),
                         &binaries, nullptr);
  ASSERT_EQ(CL_SUCCESS, err) << "clGetProgramInfo CL_PROGRAM_BINARIES failed";

  cl_int binaryStatus[1];
  dstProgram = clCreateProgramWithBinary(context, 1, &device, &binarySize,
                                         binaries, binaryStatus, &err);
  ASSERT_EQ(CL_SUCCESS, err) << "clCreateProgramWithBinary failed";
  ASSERT_EQ(CL_SUCCESS, binaryStatus[0]) << "clCreateProgramWithBinary failed";

  err = clBuildProgram(dstProgram, 1, &device, buildOptions.c_str(), nullptr,
                       nullptr);
  if (CL_SUCCESS != err) {
    std::string log;
    ASSERT_NO_FATAL_FAILURE(GetBuildLog(device, dstProgram, log));
    FAIL() << log;
  }
}

// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef _WIN32
#include <io.h>
#include <windows.h>
#else
#include <stdlib.h>
#include <fcntl.h>
#endif

using namespace std;


// Disable Microsoft deprecation warnings for POSIX functions called from
// this class (creat, dup, dup2, and close)
//
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4996)
#endif  // _MSC_VER

// Create temporary file, return filename and descriptor
static int CreateTemporaryFile(int fd, string& filename) {
#ifdef _WIN32
    char temp_dir_path[MAX_PATH + 1] = {'\0'};
    char temp_file_path[MAX_PATH + 1] = {'\0'};

    ::GetTempPathA(sizeof(temp_dir_path), temp_dir_path);
    ::GetTempFileNameA(temp_dir_path, "redir",
        0,  // Generate unique file name.
        temp_file_path);
    filename = temp_file_path;
    return creat(temp_file_path, _S_IREAD | _S_IWRITE);
#else
    char temp_file_path[] = "/tmp/redirXXXXXX";
    int tmp_fd = mkstemp(temp_file_path);
    filename = temp_file_path;
    return tmp_fd;
#endif
}


// Object that captures an output stream (stdout/stderr)
//
class CapturedStream
{
public:
    // The ctor redirects the stream to a temporary file.
    CapturedStream(int fd)
        : fd_(fd), uncaptured_fd_(dup(fd)), initialized(false) {}
    bool Initialize()
    {
        if (initialized)
            return true;
        int captured_fd = CreateTemporaryFile(fd_, filename_);
        if (captured_fd == -1) {
          cout << "ERROR: Can't create " << filename_.c_str() << "\n";
          return false;
        }
        fflush(NULL);
        dup2(captured_fd, fd_);
        close(captured_fd);
        initialized = true;
        return true;
    }

    ~CapturedStream()
    {
        if (initialized)
            remove(filename_.c_str());
    }

    string GetCapturedString()
    {
        if (uncaptured_fd_ != -1) {
            // Restores the original stream.
            fflush(NULL);
            dup2(uncaptured_fd_, fd_);
            close(uncaptured_fd_);
            uncaptured_fd_ = -1;
        }

        const string content = ReadFileContents(filename_);
        return content;
    }

private:
    const int fd_;  // A stream to capture.
    int uncaptured_fd_;
    // Name of the temporary file holding the output.
    string filename_;
    bool initialized;
};


#ifdef _MSC_VER
#pragma warning(pop)
#endif  // _MSC_VER

#if defined(_MSC_VER)
const int kStdOutFileno = 1;
const int kStdErrFileno = 2;
#else
const int kStdOutFileno = STDOUT_FILENO;
const int kStdErrFileno = STDERR_FILENO;
#endif  // _MSC_VER

static CapturedStream* g_captured_stderr = NULL;
static CapturedStream* g_captured_stdout = NULL;


// Starts capturing an output stream (stdout/stderr).
static bool CaptureStream(int fd, const char* stream_name, CapturedStream** stream)
{
    *stream = new CapturedStream(fd);
    return (*stream)->Initialize();
}

static string GetCapturedStream(CapturedStream** captured_stream)
{
    const string content = (*captured_stream)->GetCapturedString();

    delete *captured_stream;
    *captured_stream = NULL;

    return content;
}


bool CaptureStdout()
{
  return CaptureStream(kStdOutFileno, "stdout", &g_captured_stdout);
}


bool CaptureStderr()
{
   return CaptureStream(kStdErrFileno, "stderr", &g_captured_stderr);
}

string GetCapturedStdout()
{
    return GetCapturedStream(&g_captured_stdout);
}

string GetCapturedStderr()
{
    return GetCapturedStream(&g_captured_stderr);
}


vector<string> tokenize(const string& str, const string& delims)
{
    string::size_type start_index, end_index;
    vector<string> ret;

    // Skip leading delimiters, to get to the first token
    start_index = str.find_first_not_of(delims);

    // While found a beginning of a new token
    //
    while (start_index != string::npos)
    {
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

template<>
bool Compare<const char*>(const char* x, const char* y)
{
    return strcmp(x, y) == 0;
}

template<>
void Print<cl_int>(std::ostream& os, const cl_int& x)
{
    os << ClErrTxt(x);
}

// Since kernel output may be in arbitrary order (we can't ensure which work
// item runs first), to compare it to an expected output we split the output to
// lines and sort them.
// Note: this assumes each work-item outputs one or more lines ending with '\n'
//
bool compare_kernel_output(const string& expected, const string& actual)
{
    vector<string> expected_vec = tokenize(expected, "\n\r");
    vector<string> actual_vec = tokenize(actual, "\n\r");

    std::sort(expected_vec.begin(), expected_vec.end());
    std::sort(actual_vec.begin(), actual_vec.end());

    return expected_vec == actual_vec;
}

cl_ulong trySetStackSize(cl_ulong size)
{
#ifdef _WIN32
    // on windows we cannot change stack size on runtime, so, just return
    // predefined value
    return size;
#else
    // another way to set stack size on Linux is to use `ulimit -s stack_size`
    rlimit tLimitStruct;
    tLimitStruct.rlim_cur = size;
    tLimitStruct.rlim_max = ULLONG_MAX;
    if (setrlimit(RLIMIT_STACK, &tLimitStruct) != 0)
    {
        printf("Failed to set stack size. Error code: %d\n", errno);
        return 0;
    }
    else
    {
        return tLimitStruct.rlim_cur;
    }
#endif
}

unsigned getMaxNumExternalThreads() {
  return std::min((unsigned)Intel::OpenCL::Utils::GetNumberOfProcessors(), 10u);
}

bool fileContains(const std::string &Filename,
                  const std::vector<std::string> &Patterns) {
  bool contains = true;
  std::ifstream IFS(Filename);
  if (!IFS)
        return false;
  std::stringstream SS;
  SS << IFS.rdbuf();
  std::string Buffer = SS.str();
  for (auto &Pattern : Patterns)
        contains = contains && Buffer.find(Pattern) != std::string::npos;
  return contains;
}

void checkFileContains(const std::string &Filename,
                       const std::vector<std::string> &Patterns) {
  std::ifstream IFS(Filename);
  ASSERT_TRUE(IFS) << "Failed to open file " << Filename;
  std::stringstream SS;
  SS << IFS.rdbuf();
  std::string Buffer = SS.str();
  for (auto &Pattern : Patterns)
    ASSERT_NE(Buffer.find(Pattern), std::string::npos)
        << "Pattern \"" << Pattern << "\" is not found in file " << Filename
        << "\n";
}

std::vector<std::string> findFilesInDir(const std::string &Dir,
                                        const Regex &R) {
  std::vector<std::string> FileNames;
  std::error_code EC;
  for (sys::fs::directory_iterator I(Dir, EC), E; I != E && !EC;
       I.increment(EC)) {
    StringRef FileName = sys::path::filename(I->path());
    if (R.match(FileName))
      FileNames.push_back(FileName.str());
  }
  return FileNames;
}
