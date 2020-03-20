//===-- pi.cpp - PI utilities implementation -------------------*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#include <CL/sycl/detail/common.hpp>
#include <CL/sycl/detail/pi.hpp>
#include <detail/plugin.hpp>

#include <bitset>
#include <cstdarg>
#include <cstring>
#include <iostream>
#include <map>
#include <stddef.h>
#include <string>
#include <sstream>

#ifdef XPTI_ENABLE_INSTRUMENTATION
// Include the headers necessary for emitting
// traces using the trace framework
#include "xpti_trace_framework.h"
#endif

__SYCL_INLINE_NAMESPACE(cl) {
namespace sycl {
namespace detail {
#ifdef XPTI_ENABLE_INSTRUMENTATION
// Stream name being used for traces generated from the SYCL runtime
constexpr const char *PICALL_STREAM_NAME = "sycl.pi";
// Global (to the SYCL runtime) graph handle that all command groups are a
// child of
///< Event to be used by graph related activities
xpti_td *GSYCLGraphEvent = nullptr;
///< Event to be used by PI layer related activities
xpti_td *GPICallEvent = nullptr;
///< Constansts being used as placeholder until one is able to reliably get the
///< version of the SYCL runtime
constexpr uint32_t GMajVer = 1;
constexpr uint32_t GMinVer = 0;
constexpr const char *GVerStr = "sycl 1.0";
#endif

namespace pi {

bool XPTIInitDone = false;

std::string platformInfoToString(pi_platform_info info) {
  switch (info) {
  case PI_PLATFORM_INFO_PROFILE:
    return "PI_PLATFORM_INFO_PROFILE";
  case PI_PLATFORM_INFO_VERSION:
    return "PI_PLATFORM_INFO_VERSION";
  case PI_PLATFORM_INFO_NAME:
    return "PI_PLATFORM_INFO_NAME";
  case PI_PLATFORM_INFO_VENDOR:
    return "PI_PLATFORM_INFO_VENDOR";
  case PI_PLATFORM_INFO_EXTENSIONS:
    return "PI_PLATFORM_INFO_EXTENSIONS";
  default:
    pi::die("Unknown pi_platform_info value passed to "
            "cl::sycl::detail::pi::platformInfoToString");
  }
}

std::string memFlagToString(pi_mem_flags Flag) {
  assertion(((Flag == 0u) || ((Flag & (Flag - 1)) == 0)) &&
            "More than one bit set");

  std::stringstream Sstream;

  switch (Flag) {
  case pi_mem_flags{0}:
    Sstream << "pi_mem_flags(0)";
    break;
  case PI_MEM_FLAGS_ACCESS_RW:
    Sstream << "PI_MEM_FLAGS_ACCESS_RW";
    break;
  case PI_MEM_FLAGS_HOST_PTR_USE:
    Sstream << "PI_MEM_FLAGS_HOST_PTR_USE";
    break;
  case PI_MEM_FLAGS_HOST_PTR_COPY:
    Sstream << "PI_MEM_FLAGS_HOST_PTR_COPY";
    break;
  default:
    Sstream << "unknown pi_mem_flags bit == " << Flag;
  }

  return Sstream.str();
}

std::string memFlagsToString(pi_mem_flags Flags) {
  std::stringstream Sstream;
  bool FoundFlag = false;

  auto FlagSeparator = [](bool FoundFlag) { return FoundFlag ? "|" : ""; };

  pi_mem_flags ValidFlags[] = {PI_MEM_FLAGS_ACCESS_RW,
                               PI_MEM_FLAGS_HOST_PTR_USE,
                               PI_MEM_FLAGS_HOST_PTR_COPY};

  if (Flags == 0u) {
    Sstream << "pi_mem_flags(0)";
  } else {
    for (const auto Flag : ValidFlags) {
      if (Flag & Flags) {
        Sstream << FlagSeparator(FoundFlag) << memFlagToString(Flag);
        FoundFlag = true;
      }
    }

    std::bitset<64> UnkownBits(Flags & ~(PI_MEM_FLAGS_ACCESS_RW |
                                         PI_MEM_FLAGS_HOST_PTR_USE |
                                         PI_MEM_FLAGS_HOST_PTR_COPY));
    if (UnkownBits.any()) {
      Sstream << FlagSeparator(FoundFlag)
              << "unknown pi_mem_flags bits == " << UnkownBits;
    }
  }

  return Sstream.str();
}

#if INTEL_CUSTOMIZATION
// Check for selected BE for EnvVariable at run-time.
// If not set, the DefaultBE is returned.
// The BEEnvVar can be SYCL_BE or SYCL_INTEROP_BE
static Backend getBackend(const std::string &BEEnvVar, Backend DefaultBE) {
  // TODO: make it cached somehow
  const char *GetEnv = std::getenv(BEEnvVar.c_str());
  // Current default backend as SYCL_BE_PI_OPENCL
  // Valid values of GetEnv are "PI_OPENCL", "PI_CUDA", "PI_LEVEL0" and "PI_OTHER"
  // TODO: Currently PI_OTHER maps to SYCL_BE_PI_LEVEL0.
  if (!GetEnv)
    return DefaultBE;
  const std::map<std::string, Backend> SyclBeMap{
      {"PI_OTHER", SYCL_BE_PI_LEVEL0},
      {"PI_LEVEL0", SYCL_BE_PI_LEVEL0},
      // {"PI_CUDA", SYCL_BE_PI_CUDA}, // INTEL
      {"PI_OPENCL", SYCL_BE_PI_OPENCL}};
  auto It = SyclBeMap.find(std::string(GetEnv));
  if (It == SyclBeMap.end())
    pi::die("Invalid SYCL_BE/SYCL_INTEROP_BE. Valid values are PI_LEVEL0/PI_OPENCL");
  static const Backend Use = It->second;
  return Use;
}

// Checks if the BE given by SYCL_BE is the same as parameter TheBackend.
// This API is mostly used to check if a plugin/device are of the preferred BE.
bool preferredBackend(Backend TheBackend) {
  // TODO: Current default is PI_LEVEL0. Change if default needs to change.
  return TheBackend == getBackend("SYCL_BE", SYCL_BE_PI_LEVEL0);
}

#endif // INTEL_CUSTOMIZATION

// GlobalPlugin is a global Plugin used with Interoperability constructors that
// use OpenCL objects to construct SYCL class objects.
std::shared_ptr<plugin> GlobalPlugin;

// Find the plugin at the appropriate location and return the location.
#if INTEL_CUSTOMIZATION
bool findPlugins(vector_class<std::pair<std::string, Backend>> &PluginNames) {
  // TODO: Based on final design discussions, change the location where the
  // plugin must be searched; how to identify the plugins etc. Currently the
  // search is done for libpi_opencl.so/pi_opencl.dll file in LD_LIBRARY_PATH
  // env only.
  PluginNames.push_back(std::make_pair<std::string, Backend>(
      OPENCL_PLUGIN_NAME, SYCL_BE_PI_OPENCL));
  PluginNames.push_back(std::make_pair<std::string, Backend>(
      LEVEL0_PLUGIN_NAME, SYCL_BE_PI_LEVEL0));
#if 0
  // Disabling use of CUDA plugin.
   PluginNames.push_back(std::make_pair<std::string, Backend>(
      CUDA_PLUGIN_NAME, SYCL_BE_PI_CUDA));
#endif
  return true;
}
#endif // INTEL_CUSTOMIZATION

// Load the Plugin by calling the OS dependent library loading call.
// Return the handle to the Library.
void *loadPlugin(const std::string &PluginPath) {
  return loadOsLibrary(PluginPath);
}

// Binds all the PI Interface APIs to Plugin Library Function Addresses.
// TODO: Remove the 'OclPtr' extension to PI_API.
// TODO: Change the functionality such that a single getOsLibraryFuncAddress
// call is done to get all Interface API mapping. The plugin interface also
// needs to setup infrastructure to route PI_CALLs to the appropriate plugins.
// Currently, we bind to a singe plugin.
bool bindPlugin(void *Library, PiPlugin *PluginInformation) {

  decltype(::piPluginInit) *PluginInitializeFunction = (decltype(
      &::piPluginInit))(getOsLibraryFuncAddress(Library, "piPluginInit"));
  if (PluginInitializeFunction == nullptr)
    return false;

  int Err = PluginInitializeFunction(PluginInformation);

  // TODO: Compare Supported versions and check for backward compatibility.
  // Make sure err is PI_SUCCESS.
  assert((Err == PI_SUCCESS) && "Unexpected error when binding to Plugin.");
  (void)Err;

  // TODO: Return a more meaningful value/enum.
  return true;
}
#if INTEL_CUSTOMIZATION
// TODO: open-source
// Return true if we want to trace PI related activities.
bool trace(TraceLevel Level) {
  static auto TraceLevelCString = std::getenv("SYCL_PI_TRACE");
  static int TraceLevelMask = TraceLevelCString ? std::atoi(TraceLevelCString) : 0;
  return (TraceLevelMask & Level) == Level;
}

// Initializes all available Plugins.
vector_class<plugin> initialize() {
  vector_class<plugin> Plugins;
  vector_class<std::pair<std::string, Backend>> PluginNames;
  findPlugins(PluginNames);

  if (PluginNames.empty() && trace())
    std::cerr << "SYCL_PI_TRACE[-1]: No Plugins Found." << std::endl;

  PiPlugin PluginInformation;
  for (unsigned int I = 0; I < PluginNames.size(); I++) {
    void *Library = loadPlugin(PluginNames[I].first);
    if (!Library && trace()) {
      std::cerr << "SYCL_PI_TRACE[-1]: Check if plugin is present. "
                << "Failed to load plugin: "
                << PluginNames[I].first << std::endl;
    }
    if (!bindPlugin(Library, &PluginInformation) && trace()) {
      std::cerr << "SYCL_PI_TRACE[-1]: Failed to bind PI APIs to the plugin: "
                << PluginNames[I].first << std::endl;
    }
    // Set as Global Plugin if SYCL_INTEROP_BE is set.
    if (getBackend("SYCL_INTEROP_BE", SYCL_BE_PI_OPENCL /*default*/) ==
        PluginNames[I].second) {
      GlobalPlugin =
          std::make_shared<plugin>(PluginInformation, PluginNames[I].second);
    }
    Plugins.push_back(plugin(PluginInformation, PluginNames[I].second));
    if (trace(TraceLevel::PI_TRACE_BASIC))
      std::cerr << "SYCL_PI_TRACE[1]: Plugin found and successfully loaded: "
                << PluginNames[I].first << std::endl;
  }

#ifdef XPTI_ENABLE_INSTRUMENTATION
  if (!(xptiTraceEnabled() && !XPTIInitDone))
    return Plugins;
  // Not sure this is the best place to initialize the framework; SYCL runtime
  // team needs to advise on the right place, until then we piggy-back on the
  // initialization of the PI layer.

  // Initialize the global events just once, in the case pi::initialize() is
  // called multiple times
  XPTIInitDone = true;
  // Registers a new stream for 'sycl' and any plugin that wants to listen to
  // this stream will register itself using this string or stream ID for this
  // string.
  uint8_t StreamID = xptiRegisterStream(SYCL_STREAM_NAME);
  //  Let all tool plugins know that a stream by the name of 'sycl' has been
  //  initialized and will be generating the trace stream.
  //
  //                                           +--- Minor version #
  //            Major version # ------+        |   Version string
  //                                  |        |       |
  //                                  v        v       v
  xptiInitialize(SYCL_STREAM_NAME, GMajVer, GMinVer, GVerStr);
  // Create a tracepoint to indicate the graph creation
  xpti::payload_t GraphPayload("application_graph");
  uint64_t GraphInstanceNo;
  GSYCLGraphEvent =
      xptiMakeEvent("application_graph", &GraphPayload, xpti::trace_graph_event,
                    xpti_at::active, &GraphInstanceNo);
  if (GSYCLGraphEvent) {
    // The graph event is a global event and will be used as the parent for
    // all nodes (command groups)
    xptiNotifySubscribers(StreamID, xpti::trace_graph_create, nullptr,
                          GSYCLGraphEvent, GraphInstanceNo, nullptr);
  }

  xpti::payload_t PIPayload("Plugin Interface Layer");
  uint64_t PiInstanceNo;
  GPICallEvent =
      xptiMakeEvent("PI Layer", &PIPayload, xpti::trace_algorithm_event,
                    xpti_at::active, &PiInstanceNo);
#endif

  return Plugins;
}
#endif // INTEL_CUSTOMIZATION

// Report error and no return (keeps compiler from printing warnings).
// TODO: Probably change that to throw a catchable exception,
//       but for now it is useful to see every failure.
//
[[noreturn]] void die(const char *Message) {
  std::cerr << "pi_die: " << Message << std::endl;
  std::terminate();
}

void assertion(bool Condition, const char *Message) {
  if (!Condition)
    die(Message);
}

} // namespace pi
} // namespace detail
} // namespace sycl
} // __SYCL_INLINE_NAMESPACE(cl)
