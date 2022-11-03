// INTEL CONFIDENTIAL
//
// Copyright 2006-2021 Intel Corporation.
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

#include "cl_user_logger.h"
#include "cl_config.h"
#include "cl_sys_defines.h"
#include <CL/cl_ext.h>
#include <algorithm>
#include <cassert>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <unordered_set>
#ifdef _WIN32
#include <Windows.h>
#else
#include <pthread.h>
#include <sys/types.h>
#include <syscall.h>
#include <unistd.h>
#endif

using std::cerr;
using std::endl;
using std::ends;
using std::ios_base;
using std::ostringstream;
using std::string;
using std::vector;

namespace Intel {
namespace OpenCL {
namespace Utils {

#ifdef _WIN32
#if 0
// if we want to use command name in the future
static string GetCommandName()
{
    char pathCstr[256]; // maximum length of a filename in Windows
    const DWORD strLen = GetModuleFileName(nullptr, pathCstr, sizeof(pathCstr));
    assert(GetLastError() != ERROR_INSUFFICIENT_BUFFER);    
    
    const string path(pathCstr);
    const size_t index = path.find_last_of('\\');
    assert(index != string::npos);
    assert(index < path.size() - 1);

    const string filename = path.substr(index + 1);
    if (filename[filename.size() - 1] == '"')
    {
        return filename.substr(0, filename.size() - 1);
    }
    else
    {
        return filename;
    }
}
#endif
#endif

static tm GetLocalTime() {
  time_t time = std::time(nullptr);
  assert(time != (time_t)-1);

  tm *const lt = std::localtime(&time);
  assert(lt != nullptr);
  return *lt;
}

static string GetFormattedHour(bool bUseColonSeperator, const tm &lt) {
  ostringstream formattedHour;
  const string timeSeperator = bUseColonSeperator ? ":" : "";
  formattedHour << setfill('0') << lt.tm_hour << timeSeperator << setw(2)
                << lt.tm_min << timeSeperator << setw(2) << lt.tm_sec;
  return formattedHour.str();
}

static string GetFormattedTime() {
  const tm lt = GetLocalTime();
  ostringstream formattedTime;
  formattedTime << (lt.tm_mon + 1) << "." << lt.tm_mday << "."
                << (1900 + lt.tm_year) << "_" <<
#ifdef _WIN32
      GetFormattedHour(false, lt) // : isn't allowed in Windows filenames
#else
      GetFormattedHour(true, lt)
#endif
                << ".txt" << ends;
  return formattedTime.str();
}

// FrameworkUserLogger's methods

string FrameworkUserLogger::FormatLocalWorkSize(size_t ndim,
                                                const size_t *localWorkSize) {
  stringstream stream;
  stream << "[";
  for (size_t i = 0; i < ndim; ++i) {
    stream << localWorkSize[i];
    if (i < (ndim - 1))
      stream << ",";
  }
  stream << "]";
  return stream.str();
}

void FrameworkUserLogger::Setup(const string &filename, bool bLogErrors,
                                bool bLogApis) {
  if ("stdout" == filename) {
    m_pOutput = &std::cout;
  } else if ("stderr" == filename || filename.empty()) {
    m_pOutput = &cerr;
  } else {
    ostringstream finalFilename;
    finalFilename << filename << "_PID" <<
#ifdef _WIN32
        GetCurrentProcessId()
#else
        getpid()
#endif
                  << "_" << GetFormattedTime() << ends;
    m_logFile.open(finalFilename.str().c_str(), ios_base::out);
    if (!m_logFile.is_open()) {
      cerr << "cannot open log file " << finalFilename.str() << " for writing"
           << endl;
      return;
    } else {
      m_pOutput = &m_logFile;
    }
  }
  m_bLogErrors = bLogErrors;
  m_bLogApis = bLogApis;
}

FrameworkUserLogger::FrameworkUserLogger()
    : m_bLogErrors(false), m_bLogApis(false), m_pOutput(nullptr) {
  ConfigFile config(GetConfigFilePath());
  const string varName = "CL_CONFIG_USER_LOGGER";
  const string configStr = config.Read(varName, string(""));
  // parse configStr
  bool bLogErrs = true, bLogApsi = false; // the defaults
  const string::size_type indexOfComman = configStr.find(',');
  if (indexOfComman != string::npos) {
    const string enableStr = configStr.substr(0, indexOfComman);
    if ("I" == enableStr) {
      bLogErrs = false;
      bLogApsi = true;
    } else if ("EI" == enableStr || "IE" == enableStr) {
      bLogApsi = true;
    } else if ("E" != enableStr) {
      cerr << "\"" << configStr << "\" is an invalid value for " << varName
           << endl;
      return;
    }
  }

  const string filename = indexOfComman != string::npos
                              ? configStr.substr(indexOfComman + 1)
                              : configStr;
  if (!configStr.empty()) {
    Setup(filename, bLogErrs, bLogApsi);
  }
}

void ApiLogger::EndApiFuncInternal(cl_int retVal) {
  m_strStream << ") = " << ClErrTxt(retVal);
  m_iLastRetValue = retVal;
  EndApiFuncEpilog();
}

void ApiLogger::EndApiFuncInternal(const void *retPtr) {
  m_strStream << ") = 0x" << ios_base::hex << retPtr;
  if (nullptr != retPtr) {
    m_iLastRetValue = CL_SUCCESS;
  } else {
    m_iLastRetValue = CL_INVALID_VALUE;
  }
  EndApiFuncEpilog();
}

void ApiLogger::EndApiFuncInternal() {
  m_strStream << ")";
  m_iLastRetValue = CL_SUCCESS;
  EndApiFuncEpilog();
}

static std::unordered_set<unsigned> stringParams = {
    // clGetPlatformInfo
    CL_PLATFORM_PROFILE, CL_PLATFORM_VERSION, CL_PLATFORM_NAME,
    CL_PLATFORM_VENDOR, CL_PLATFORM_EXTENSIONS, CL_PLATFORM_ICD_SUFFIX_KHR,
    // clGetDeviceInfo
    CL_DEVICE_IL_VERSION, CL_DEVICE_BUILT_IN_KERNELS, CL_DEVICE_NAME,
    CL_DEVICE_VENDOR, CL_DRIVER_VERSION, CL_DEVICE_PROFILE, CL_DEVICE_VERSION,
    CL_DEVICE_OPENCL_C_VERSION, CL_DEVICE_EXTENSIONS, CL_DEVICE_IL_VERSION_KHR,
    // clGetProgramInfo
    CL_PROGRAM_SOURCE, CL_PROGRAM_IL, CL_PROGRAM_KERNEL_NAMES,
    CL_PROGRAM_IL_KHR,
    // clGetProgramBuildInfo
    CL_PROGRAM_BUILD_OPTIONS, CL_PROGRAM_BUILD_LOG,
    // clGetKernelInfo
    CL_KERNEL_FUNCTION_NAME, CL_KERNEL_ATTRIBUTES,
    // clGetKernelArgInfo
    CL_KERNEL_ARG_TYPE_NAME, CL_KERNEL_ARG_NAME};

void ApiLogger::PrintOutputParam(const string &name, unsigned paramName,
                                 const void *addr, size_t size, bool bIsPtr2Ptr,
                                 bool bIsUnsigned) {

  if (!m_bLogApis) {
    return;
  }
  m_stream << ", *" << name << " = ";
  if (bIsPtr2Ptr) {
    const void *const *pp = reinterpret_cast<const void *const *>(addr);
    if (nullptr != pp) {
      m_stream << hex << *pp;
    } else {
      m_stream << "NULL";
    }
  } else {
    if (!addr) {
      m_stream << "nil";
      return;
    }
    if (stringParams.count(paramName)) {
      m_stream << "\"" << (const char *)addr << "\"";
      return;
    }
    if (size > 8) {
      size_t sizeTrunc = std::min((size_t)1024, size & (~7));
      m_stream << "[";
      for (const char *I = (const char *)addr, *E = I + sizeTrunc; I != E;
           I += 8) {
        if (I != addr)
          m_stream << ", ";
        if (bIsUnsigned)
          PrintIntegerOutputParam<cl_ulong>(I);
        else
          PrintIntegerOutputParam<cl_long>(I);
      }
      if (sizeTrunc != size)
        m_stream << ", ...";
      m_stream << "]";
      return;
    }
    switch (size) {
    case 1:
      if (bIsUnsigned) {
        PrintIntegerOutputParam<cl_uchar>(addr);
      } else {
        PrintIntegerOutputParam<cl_char>(addr);
      }
      break;
    case 2:
      if (bIsUnsigned) {
        PrintIntegerOutputParam<cl_ushort>(addr);
      } else {
        PrintIntegerOutputParam<cl_short>(addr);
      }
      break;
    case 4:
      if (bIsUnsigned) {
        PrintIntegerOutputParam<cl_uint>(addr);
      } else {
        PrintIntegerOutputParam<cl_int>(addr);
      }
      break;
    case 8:
      if (bIsUnsigned) {
        PrintIntegerOutputParam<cl_ulong>(addr);
      } else {
        PrintIntegerOutputParam<cl_long>(addr);
      }
      break;
    default:
      assert(false && "unexpected size of type");
    }
  }
}

void FrameworkUserLogger::PrintError(const string &msg) {
  if (m_bLogErrors) {
    *m_pOutput << "ERROR: " << msg << endl;
  }
}

void FrameworkUserLogger::PrintStringInternal(const string &str) {
  OclAutoMutex mutex(&m_outputMutex);
  *m_pOutput << str;
}

void FrameworkUserLogger::SetWGSizeCount(cl_dev_cmd_id id, size_t ndim,
                                         const size_t *localSizeUniform,
                                         const size_t *localSizeNonUniform,
                                         const size_t *workgroupCount) {
  OclAutoMutex mutex(&m_outputMutex);
  *m_pOutput
      << "Internally calculated WG info for NDRangeKernel command with ID "
      << (size_t)id << ": work dimension = " << ndim
      << ", uniform work group size = "
      << FormatLocalWorkSize(ndim, localSizeUniform)
      << ", non-uniform work group size = "
      << FormatLocalWorkSize(ndim, localSizeNonUniform)
      << ", work group count = " << FormatLocalWorkSize(ndim, workgroupCount)
      << endl;
}

// LogMessageWrapper methods:

void LogMessageWrapper::Serialize() {
  stringstream stream;
  stream << m_id << " " << m_msg << ends;
  m_rawStr = stream.str();
}

void LogMessageWrapper::Unserialize() {
  stringstream stream(m_rawStr);
  stream >> m_id;

  stream.seekg(1, ios_base::cur); // skip the space

  vector<char> buf(100);
  stream.getline(&buf[0], buf.size(), '\0');
  m_msg = &buf[0];

  assert(stream.eof());
}

// ApiLogger methods:

void ApiLogger::StartApiFuncInternal(const string &funcName) {
  m_strStream << funcName << "(";
  m_bFirstApiFuncArg = true;
  m_timer.Start();
}

ApiLogger &ApiLogger::operator<<(const cl_uint &val) {
  if (m_bLogApis) {
    m_strStream << val;

    if (m_isNumEvents) {
      if (val > 0)
        m_numEvents = val;
      else
        m_isNumEvents = false;
    }
  }
  return *this;
}

ApiLogger &ApiLogger::operator<<(const cl_event *val) {
  if (m_bLogApis) {
    if (m_numEvents > 0) {
      cl_event *const *v = reinterpret_cast<cl_event *const *>(val);
      PrintArray(m_numEvents, v);

      // Reset event variables.
      m_isNumEvents = false;
      m_numEvents = 0;
    } else
      m_strStream << val;
  }
  return *this;
}

ApiLogger &ApiLogger::PrintMacroCode(cl_ulong value) {
  m_strStream << std::hex << "0x" << std::setfill('0') << std::setw(4)
              << std::uppercase << value
              << std::resetiosflags(std::ios_base::basefield);
  return *this;
}

void ApiLogger::PrintPtrValue(size_t size, const void *value) {
  if (!value)
    return;
  // Limitations due to that we don't know param type of the kernel:
  //   * we can't differentiate cl_uint3 and cl_uint4 because both of them
  //     have size of 16 bytes.
  //   * we can't differentiate cl_ushort16, cl_uint8 and cl_ulong4 because
  //     all of them have size of 32 bytes.
  m_strStream << " [" << hex;
  if (size == sizeof(void *))
    m_strStream << *reinterpret_cast<const void *const *>(value);
  else if (size == sizeof(cl_uchar))
    m_strStream << "0x"
                << (cl_uint) * reinterpret_cast<const cl_uchar *>(value);
  else if (size == sizeof(cl_ushort))
    m_strStream << "0x" << *reinterpret_cast<const cl_ushort *>(value);
  else if (size == sizeof(cl_uint))
    m_strStream << "0x" << *reinterpret_cast<const cl_uint *>(value);
  else if (size == sizeof(cl_ulong))
    m_strStream << "0x" << *reinterpret_cast<const cl_ulong *>(value);
  else if (size == sizeof(cl_uint4)) {
    const cl_uint4 *v = reinterpret_cast<const cl_uint4 *>(value);
    m_strStream << "as_uint4: X = 0x" << v->s[0] << ", Y = 0x" << v->s[1]
                << ", Z = 0x" << v->s[2] << ", W = 0x" << v->s[3];
  } else if (size == sizeof(cl_ulong4)) {
    const cl_ulong4 *v = reinterpret_cast<const cl_ulong4 *>(value);
    m_strStream << "as_ulong4: X = 0x" << v->s[0] << ", Y = 0x" << v->s[1]
                << ", Z = 0x" << v->s[2] << ", W = 0x" << v->s[3];
  } else if (size == sizeof(cl_ulong8)) {
    const cl_ulong8 *v = reinterpret_cast<const cl_ulong8 *>(value);
    m_strStream << "as_ulong8: s0 = 0x" << v->s[0] << ", s1 = 0x" << v->s[1]
                << ", s2 = 0x" << v->s[2] << ", s3 = 0x" << v->s[3]
                << ", s4 = 0x" << v->s[4] << ", s5 = 0x" << v->s[5]
                << ", s6 = 0x" << v->s[6] << ", s7 = 0x" << v->s[7];
  } else if (size == sizeof(cl_ulong16)) {
    const cl_ulong16 *v = reinterpret_cast<const cl_ulong16 *>(value);
    m_strStream << "as_ulong16: s0 = 0x" << v->s[0] << ", s1 = 0x" << v->s[1]
                << ", s2 = 0x" << v->s[2] << ", s3 = 0x" << v->s[3]
                << ", s4 = 0x" << v->s[4] << ", s5 = 0x" << v->s[5]
                << ", s6 = 0x" << v->s[6] << ", s7 = 0x" << v->s[7]
                << ", s8 = 0x" << v->s[8] << ", s9 = 0x" << v->s[9]
                << ", sa = 0x" << v->s[10] << ", sb = 0x" << v->s[11]
                << ", sc = 0x" << v->s[12] << ", sd = 0x" << v->s[13]
                << ", se = 0x" << v->s[14] << ", sf = 0x" << v->s[15];
  }
  m_strStream << "]";
}

void ApiLogger::PrintParamTypeAndName(const char *sParamTypeAndName) {
  if (!m_bFirstApiFuncArg) {
    m_strStream << ", ";
  } else {
    m_bFirstApiFuncArg = false;
  }
  if (!sParamTypeAndName)
    return;
  m_strStream << sParamTypeAndName << " = ";

  // Check if param name is num_events_in_wait_list or num_events.
  static const char *numEvents[2] = {"num_events_in_wait_list", "num_events"};
  static const size_t numEventsSize[2] = {strlen(numEvents[0]),
                                          strlen(numEvents[1])};
  std::string param(sParamTypeAndName);
  size_t paramSize = param.size();
  for (size_t i = 0; i < sizeof(numEvents) / sizeof(numEvents[0]); ++i) {
    if (paramSize > numEventsSize[i] &&
        param.compare(paramSize - numEventsSize[i], numEventsSize[i],
                      numEvents[i]) == 0) {
      m_isNumEvents = true;
      break;
    }
  }
}

void ApiLogger::PrintCStringValInternal(const char *sVal) {
  if (nullptr != sVal) {
    m_strStream << sVal;
  } else {
    m_strStream << "NULL";
  }
}

void ApiLogger::EndApiFuncEpilog() {
  m_timer.Stop();
  // thread ID
  std::right(m_stream);
  m_stream << "TID " << setfill(' ') << setw(9) << dec <<
#ifdef _WIN32
      GetCurrentThreadId()
#else
      syscall(SYS_gettid)
#endif
      ;
  const unsigned long long ulStartTime = RDTSC();
  // start time in clock ticks
  m_stream << "    START TIME 0x" << setfill('0') << setw(16) << hex
           << ulStartTime;
  // duration
  m_stream << "    DURATION 0x" << setw(16) << m_timer.GetTotalUsecs();
  std::left(m_stream);
  if (m_iCmdId != -1) {
    m_stream << "    CMD ID " << setfill(' ') << dec << setw(10) << m_iCmdId;
  } else {
    m_stream << "                     ";
  }
  // API call itself
  m_stream << "    " << m_strStream.str();
}

} // namespace Utils
} // namespace OpenCL
} // namespace Intel
