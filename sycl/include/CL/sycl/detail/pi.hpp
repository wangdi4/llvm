//==---------- pi.hpp - Plugin Interface for SYCL RT -----------------------==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// C++ wrapper of extern "C" PI interfaces
//
#pragma once

#include <CL/sycl/detail/common.hpp>
#include <CL/sycl/detail/os_util.hpp>
#include <CL/sycl/detail/pi.h>

#include <cassert>
#include <string>

__SYCL_INLINE namespace cl {
namespace sycl {
namespace detail {

enum class PiApiKind {
#define _PI_API(api) api,
#include <CL/sycl/detail/pi.def>
};
class plugin;
namespace pi {

<<<<<<< HEAD
// Check for manually selected BE at run-time.
bool useBackend(Backend Backend);

=======
#ifdef SYCL_RT_OS_WINDOWS
#define PLUGIN_NAME "pi_opencl.dll"
#else
#define PLUGIN_NAME "libpi_opencl.so"
#endif

using PiPlugin = ::pi_plugin;
>>>>>>> 95652d4642b858ada012e55b820a584acb9adca0
using PiResult = ::pi_result;
using PiPlatform = ::pi_platform;
using PiDevice = ::pi_device;
using PiDeviceType = ::pi_device_type;
using PiDeviceInfo = ::pi_device_info;
using PiProgramInfo = ::pi_program_info;
using PiDeviceBinaryType = ::pi_device_binary_type;
using PiContext = ::pi_context;
using PiProgram = ::pi_program;
using PiKernel = ::pi_kernel;
using PiQueue = ::pi_queue;
using PiQueueProperties = ::pi_queue_properties;
using PiMem = ::pi_mem;
using PiMemFlags = ::pi_mem_flags;
using PiEvent = ::pi_event;
using PiSampler = ::pi_sampler;
using PiSamplerInfo = ::pi_sampler_info;
using PiSamplerProperties = ::pi_sampler_properties;
using PiSamplerAddressingMode = ::pi_sampler_addressing_mode;
using PiSamplerFilterMode = ::pi_sampler_filter_mode;
using PiMemImageFormat = ::pi_image_format;
using PiMemImageDesc = ::pi_image_desc;
using PiMemImageInfo = ::pi_image_info;
using PiMemObjectType = ::pi_mem_type;
using PiMemImageChannelOrder = ::pi_image_channel_order;
using PiMemImageChannelType = ::pi_image_channel_type;

// Function to load the shared library
// Implementation is OS dependent.
void *loadOsLibrary(const std::string &Library);

// Function to get Address of a symbol defined in the shared
// library, implementation is OS dependent.
void *getOsLibraryFuncAddress(void *Library, const std::string &FunctionName);

// For selection of SYCL RT back-end, now manually through the "SYCL_BE"
// environment variable.
enum Backend { SYCL_BE_PI_OPENCL, SYCL_BE_PI_OTHER };

// Check for manually selected BE at run-time.
bool useBackend(Backend Backend);

// Get a string representing a _pi_platform_info enum
std::string platformInfoToString(pi_platform_info info);

// Report error and no return (keeps compiler happy about no return statements).
[[noreturn]] void die(const char *Message);

void assertion(bool Condition, const char *Message = nullptr);

// Want all the needed casts be explicit, do not define conversion operators.
template <class To, class From> To cast(From value);

// Holds the PluginInformation for the plugin that is bound.
// Currently a global varaible is used to store OpenCL plugin information to be
// used with SYCL Interoperability Constructors.
extern std::shared_ptr<plugin> GlobalPlugin;

// Performs PI one-time initialization.
vector_class<plugin> initialize();

// Utility Functions to get Function Name for a PI Api.
template <PiApiKind PiApiOffset> struct PiFuncInfo {};

#define _PI_API(api)                                                           \
  template <> struct PiFuncInfo<PiApiKind::api> {                              \
    inline std::string getFuncName() { return #api; }                          \
    inline decltype(&::api) getFuncPtr(PiPlugin MPlugin) {                     \
      return MPlugin.PiFunctionTable.api;                                      \
    }                                                                          \
  };
#include <CL/sycl/detail/pi.def>

// Helper utilities for PI Tracing
// The run-time tracing of PI calls.
// Print functions used by Trace class.
template <typename T> inline void print(T val) {
  std::cout << "<unknown> : " << val << std::endl;
}

template <> inline void print<>(PiPlatform val) {
  std::cout << "pi_platform : " << val << std::endl;
}

/* INTEL_CUSTOMIZATION */
template <> inline void print<>(PiEvent val) {
  std::cout << "pi_event : " << val;
}

template <> inline void print<>(PiMem val) {
  std::cout << "pi_mem : " << val;
}

template <> inline void print<>(PiEvent *val) {
  std::cout << "pi_event * : " << val;
  if (val) {
    std::cout << "[ " << *val << " ... ]";
  }
}

template <> inline void print<>(const PiEvent *val) {
  std::cout << "const pi_event * : " << val;
  if (val) {
    std::cout << "[ " << *val << " ... ]";
  }
}
/* end INTEL_CUSTOMIZATION */

template <> inline void print<>(PiResult val) {
  std::cout << "pi_result : ";
  if (val == PI_SUCCESS)
    std::cout << "PI_SUCCESS" << std::endl;
  else
    std::cout << val << std::endl;
}

// cout does not resolve a nullptr.
template <> inline void print<>(std::nullptr_t val) { print<void *>(val); }

inline void printArgs(void) {}
template <typename Arg0, typename... Args>
void printArgs(Arg0 arg0, Args... args) {
  std::cout << "       ";
  print(arg0);
  printArgs(std::forward<Args>(args)...);
}
<<<<<<< HEAD

/* INTEL_CUSTOMIZATION */
template <typename T>
struct printOut { printOut(T val) { } }; // Do nothing

template<> struct printOut<PiEvent *> {
  printOut(PiEvent *val) {
  std::cout << std::endl << "       ";
  std::cout << "[out]pi_event * : " << val;
  if (val) {
    std::cout << "[ " << *val << " ... ]";
  }
}};

template<> struct printOut<PiMem *> {
  printOut(PiMem *val) {
  std::cout << std::endl << "       ";
  std::cout << "[out]pi_mem * : " << val;
  if (val) {
    std::cout << "[ " << *val << " ... ]";
  }
}};

template<> struct printOut<void *> {
  printOut(void *val) {
  std::cout << std::endl << "       ";
  std::cout << "[out]void * : " << val;
}};

template<typename T> struct printOut<T **> {
  printOut(T **val) {
  std::cout << std::endl << "       ";
  std::cout << "[out]<unknown> ** : " << val;
  if (val) {
    std::cout << "[ " << *val << " ... ]";
  }
}};

inline void printOuts(void) {}
template <typename Arg0, typename... Args>
void printOuts(Arg0 arg0, Args... args) {
  using T = decltype(arg0);
  printOut<T> a(arg0);
  printOuts(std::forward<Args>(args)...);
}
/* end INTEL_CUSTOMIZATION */

// Utility function to check return from pi calls.
// Throws if pi_result is not a PI_SUCCESS.
template <typename Exception = cl::sycl::runtime_error>
inline void checkPiResult(PiResult pi_result) {
  CHECK_OCL_CODE_THROW(pi_result, Exception);
}

// Class to call PI API, trace and get the result.
// To Trace : Set SYCL_PI_TRACE environment variable.
// Template Arguments:
//    FnType  - Type of Function pointer to the PI API.
//    FnOffset- Offset to the Function Pointer in the piPlugin::FunctionPointers
//    structure. Used to differentiate between APIs with same pointer type,
//    E.g.: piDeviceRelease and piDeviceRetain. Differentiation needed to avoid
//    redefinition error during explicit specialization of class in pi.cpp.
// Members: Initialized in default constructor in Class Template Specialization.
// Usage:
// Operator() - Call, Trace and Get result
// Use Macro PI_CALL_NOCHECK call the constructor directly.
template <typename FnType, size_t FnOffset> class CallPi {
private:
  FnType MFnPtr;
  std::string MFnName;
  static bool MEnableTrace;

public:
  CallPi();
  template <typename... Args> PiResult operator()(Args... args) {
    if (MEnableTrace) {
      std::cout << "---> " << MFnName << "(";
      printArgs(args...);
      std::cout << std::flush;  // INTEL
    }

    PiResult r = MFnPtr(args...);

    if (MEnableTrace) {
      std::cout << ") ---> ";
      std::cout << (print(r), "") << std::endl;
      printOuts(args...);       // INTEL
      std::cout << std::endl;   // INTEL
      std::cout << std::flush;  // INTEL
    }
    return r;
  }
};

template <typename FnType, size_t FnOffset>
bool CallPi<FnType, FnOffset>::MEnableTrace = (std::getenv("SYCL_PI_TRACE") !=
                                               nullptr);

// Class to call PI API, trace, check the return result and throw Exception.
// To Trace : Set SYCL_PI_TRACE environment variable.
// Template Arguments:
//    FnType, FnOffset - for CallPi Class.
//    Exception - The type of exception to throw if PiResult of a call is not
//    PI_SUCCESS. Default value is cl::sycl::runtime_error.
// Usage:
// Operator() - Call, Trace, check Result and Throw Exception.
// Use Macro PI_CALL and PI_CALL_THROW to call the constructor directly.
template <typename FnType, size_t FnOffset,
          typename Exception = cl::sycl::runtime_error>
class CallPiAndCheck : private CallPi<FnType, FnOffset> {
public:
  CallPiAndCheck() : CallPi<FnType, FnOffset>(){};

  template <typename... Args> void operator()(Args... args) {
    PiResult Err = (CallPi<FnType, FnOffset>::operator()(args...));
    checkPiResult<Exception>(Err);
  }
};

// Explicit specialization declarations for Trace class for every FnType.
// The offsetof is used as a template argument to uniquely identify every
// api.
#define _PI_API(api)                                                           \
  template <>                                                                  \
  CallPi<decltype(&::api),                                                     \
         (offsetof(pi_plugin::FunctionPointers, api))>::CallPi();

#include <CL/sycl/detail/pi.def>

=======
>>>>>>> 95652d4642b858ada012e55b820a584acb9adca0
} // namespace pi

namespace RT = cl::sycl::detail::pi;

// Want all the needed casts be explicit, do not define conversion
// operators.
template <class To, class From> To pi::cast(From value) {
  // TODO: see if more sanity checks are possible.
  RT::assertion((sizeof(From) == sizeof(To)), "assert: cast failed size check");
  return (To)(value);
}

} // namespace detail

// For shortness of using PI from the top-level sycl files.
namespace RT = cl::sycl::detail::pi;

} // namespace sycl
} // namespace cl
