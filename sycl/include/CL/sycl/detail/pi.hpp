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
#include <sstream>

#include <cassert>
#include <string>

#ifdef XPTI_ENABLE_INSTRUMENTATION
// Forward declarations
namespace xpti {
struct trace_event_data_t;
}
#endif

__SYCL_INLINE_NAMESPACE(cl) {
namespace sycl {
namespace detail {

enum class PiApiKind {
#define _PI_API(api) api,
#include <CL/sycl/detail/pi.def>
};
class plugin;
namespace pi {

/* INTEL_CUSTOMIZATION */
// TODO: open-source
// The SYCL_PI_TRACE sets what we will trace.
// This is a bit-mask of various things we'd want to trace.
enum TraceLevel {
  PI_TRACE_BASIC  = 0x1,
  PI_TRACE_CALLS  = 0x2,
  PI_TRACE_ALL    = -1
};

// Return true if we want to trace PI related activities.
bool trace(TraceLevel level = PI_TRACE_ALL);
/* end INTEL_CUSTOMIZATION */

#ifdef SYCL_RT_OS_WINDOWS
#define OPENCL_PLUGIN_NAME "pi_opencl.dll"
#define LEVEL0_PLUGIN_NAME "pi_level0.dll"
#define CUDA_PLUGIN_NAME "pi_cuda.dll"
#else
#define OPENCL_PLUGIN_NAME "libpi_opencl.so"
#define LEVEL0_PLUGIN_NAME "libpi_level0.so"
#define CUDA_PLUGIN_NAME "libpi_cuda.so"
#endif

// Report error and no return (keeps compiler happy about no return statements).
[[noreturn]] void die(const char *Message);

void assertion(bool Condition, const char *Message = nullptr);

template <typename T>
void handleUnknownParamName(const char *functionName, T parameter) {
  std::stringstream stream;
  stream << "Unknown parameter " << parameter << " passed to " << functionName
         << "\n";
  auto str = stream.str();
  auto msg = str.c_str();
  die(msg);
}

// This macro is used to report invalid enumerators being passed to PI API
// GetInfo functions. It will print the name of the function that invoked it
// and the value of the unknown enumerator.
#define PI_HANDLE_UNKNOWN_PARAM_NAME(parameter)                                \
  { cl::sycl::detail::pi::handleUnknownParamName(__func__, parameter); }

using PiPlugin = ::pi_plugin;
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

/* INTEL_CUSTOMIZATION */
// TODO: open-source
// For selection of SYCL RT back-end, now manually through the "SYCL_BE"
// environment variable.
enum Backend {
  SYCL_BE_PI_OPENCL,
  SYCL_BE_PI_LEVEL0,
  SYCL_BE_PI_CUDA,
  SYCL_BE_PI_OTHER /*TODO: Remove it since LEVEL0 string is added. Add specific
                      strings for new backends*/
};

// Get the preferred BE (selected with SYCL_BE).
Backend getPreferredBE();
/* end INTEL_CUSTOMIZATION */

// Get a string representing a _pi_platform_info enum
std::string platformInfoToString(pi_platform_info info);

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
} // namespace pi

namespace RT = cl::sycl::detail::pi;

// Want all the needed casts be explicit, do not define conversion
// operators.
template <class To, class From> To inline pi::cast(From value) {
  // TODO: see if more sanity checks are possible.
  RT::assertion((sizeof(From) == sizeof(To)), "assert: cast failed size check");
  return (To)(value);
}

// These conversions should use PI interop API.
template <> pi::PiProgram inline pi::cast(cl_program interop) {
  RT::assertion(false, "pi::cast -> use piextProgramConvert");
  return {};
}

template <> pi::PiDevice inline pi::cast(cl_device_id interop) {
  RT::assertion(false, "pi::cast -> use piextDeviceConvert");
  return {};
}

} // namespace detail

// For shortness of using PI from the top-level sycl files.
namespace RT = cl::sycl::detail::pi;

} // namespace sycl
} // __SYCL_INLINE_NAMESPACE(cl)
