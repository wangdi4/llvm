//===----------- rtl.cpp - Target independent OpenMP target RTL -----------===//
// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2021 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Functionality for handling RTL plugins.
//
//===----------------------------------------------------------------------===//

#include "llvm/Object/OffloadBinary.h"

#include "OmptCallback.h"
#include "device.h"
#include "private.h"
#include "rtl.h"

#include "Utilities.h"

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <string>
#if INTEL_CUSTOMIZATION
#ifdef _WIN32
#include <windows.h>
#endif
#include "xpti_registry.h"
#endif // INTEL_CUSTOMIZATION

using namespace llvm;
using namespace llvm::sys;
using namespace llvm::omp::target;

#if INTEL_CUSTOMIZATION
#if _WIN32
#define GET_RTL_NAME(Name) "omptarget.rtl." #Name ".dll"
#else
#define GET_RTL_NAME(Name) "libomptarget.rtl." #Name ".so"
#endif
#endif // INTEL_CUSTOMIZATION
// List of all plugins that can support offloading.
static const char *RTLNames[] = {
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
    /* CSA target           */ GET_RTL_NAME(csa),
#endif // INTEL_FEATURE_CSA
    /* Level Zero target    */ GET_RTL_NAME(level0),
#if !_WIN32
    // OpenCL plugin is excluded from the default list to avoid initialization
    // issue when dll contains device image.
    /* OpenCL target        */ GET_RTL_NAME(opencl),
#endif
    /* x86_64 target        */ GET_RTL_NAME(x86_64),
#else  // INTEL_CUSTOMIZATION
    /* PowerPC target       */ "libomptarget.rtl.ppc64",
    /* x86_64 target        */ "libomptarget.rtl.x86_64",
    /* CUDA target          */ "libomptarget.rtl.cuda",
    /* AArch64 target       */ "libomptarget.rtl.aarch64",
    /* AMDGPU target        */ "libomptarget.rtl.amdgpu",
#endif // INTEL_CUSTOMIZATION
};

PluginManager *PM;

#if INTEL_CUSTOMIZATION
#else  // INTEL_CUSTOMIZATION
static char *ProfileTraceFile = nullptr;
#endif // INTEL_CUSTOMIZATION

#ifdef OMPT_SUPPORT
extern void ompt::connectLibrary();
#endif

#if INTEL_CUSTOMIZATION
#ifdef _WIN32
#define __attribute__(x)
#endif
#endif // INTEL_CUSTOMIZATION
__attribute__((constructor(101))) void init() {
  DP("Init target library!\n");

  bool UseEventsForAtomicTransfers = true;
  if (const char *ForceAtomicMap = getenv("LIBOMPTARGET_MAP_FORCE_ATOMIC")) {
    std::string ForceAtomicMapStr(ForceAtomicMap);
    if (ForceAtomicMapStr == "false" || ForceAtomicMapStr == "FALSE")
      UseEventsForAtomicTransfers = false;
    else if (ForceAtomicMapStr != "true" && ForceAtomicMapStr != "TRUE")
      fprintf(stderr,
              "Warning: 'LIBOMPTARGET_MAP_FORCE_ATOMIC' accepts only "
              "'true'/'TRUE' or 'false'/'FALSE' as options, '%s' ignored\n",
              ForceAtomicMap);
  }

  PM = new PluginManager(UseEventsForAtomicTransfers);

#if INTEL_CUSTOMIZATION
  XPTIRegistry = new XPTIRegistryTy();
#endif // INTEL_CUSTOMIZATION

#if INTEL_CUSTOMIZATION
// LLVM tracing is disabled
#else  // INTEL_CUSTOMIZATION
  ProfileTraceFile = getenv("LIBOMPTARGET_PROFILE");
  // TODO: add a configuration option for time granularity
  if (ProfileTraceFile)
    timeTraceProfilerInitialize(500 /* us */, "libomptarget");
#endif // INTEL_CUSTOMIZATION

#if INTEL_CUSTOMIZATION
// OMPT initialization is delayed to support Windows.
#else // INTEL_CUSTOMIZATION
#ifdef OMPT_SUPPORT
  // Initialize OMPT first
  ompt::connectLibrary();
#endif
#endif // INTEL_CUSTOMIZATION

#if INTEL_CUSTOMIZATION
#else // INTEL_CUSTOMIZATION
  PM->RTLs.loadRTLs();
  PM->registerDelayedLibraries();
#endif // INTEL_CUSTOMIZATION
}

#if INTEL_CUSTOMIZATION
__attribute__((destructor(101))) void deinit() {
  DP("Deinit target library!\n");
  delete PM;
  delete XPTIRegistry;
}
#else  // INTEL_CUSTOMIZATION
__attribute__((destructor(101))) void deinit() {
  DP("Deinit target library!\n");
  delete PM;

  if (ProfileTraceFile) {
    // TODO: add env var for file output
    if (auto E = timeTraceProfilerWrite(ProfileTraceFile, "-"))
      fprintf(stderr, "Error writing out the time trace\n");

    timeTraceProfilerCleanup();
  }
}
#endif // INTEL_CUSTOMIZATION

#if INTEL_CUSTOMIZATION
#if _WIN32
extern "C" BOOL WINAPI
DllMain(HINSTANCE const instance, // handle to DLL module
        DWORD const reason,       // reason for calling function
        LPVOID const reserved)    // reserved
{
  // Perform actions based on the reason for calling.
  switch (reason) {
  case DLL_PROCESS_ATTACH:
    // Initialize once for each new process.
    // Return FALSE to fail DLL load.
    init();
    break;

  case DLL_THREAD_ATTACH:
    // Do thread-specific initialization.
    break;

  case DLL_THREAD_DETACH:
    // Do thread-specific cleanup.
    break;

  case DLL_PROCESS_DETACH:
    // Perform any necessary cleanup.
    deinit();
    break;
  }
  return TRUE; // Successful DLL_PROCESS_ATTACH.
}
#endif // _WIN32

std::vector<std::string_view> tokenize(const std::string_view &Filter,
                                       const std::string &Delim) {
  std::vector<std::string_view> Tokens;
  size_t Pos = 0;
  size_t LastPos = 0;

  while ((Pos = Filter.find(Delim, LastPos)) != std::string::npos) {
    std::string_view Tok(Filter.data() + LastPos, (Pos - LastPos));

    if (!Tok.empty()) {
      Tokens.push_back(Tok);
    }
    // move the search starting index
    LastPos = Pos + 1;
  }

  // Add remainder if any
  if (LastPos < Filter.size()) {
    std::string_view Tok(Filter.data() + LastPos, Filter.size() - LastPos);
    Tokens.push_back(Tok);
  }
  return Tokens;
}

void getPlugInNameFromEnv(std::vector<const char *> &RTLChecked) {
  //  Process ONEAPI_DEVICE_SELECTOR first and then LIBOMPTARGET_PLUGIN
  //  which will  be obsoleted
  if (char *Env = getenv("ONEAPI_DEVICE_SELECTOR")) {
    // Empty string select level0
    if (Env[0] == '\0') {
      RTLChecked.push_back(GET_RTL_NAME(level0));
      return;
    }

    bool done_l0 = false;
    bool done_opencl = false;
    bool done_x86_64 = false;
    bool done_ur = false;
    std::string EnvStr(Env);
    std::transform(EnvStr.begin(), EnvStr.end(), EnvStr.begin(),
                   [](unsigned char C) { return std::tolower(C); });
    std::vector<std::string_view> OdsStr = tokenize(EnvStr, ";");
    for (const auto &Filter : OdsStr) {
      std::vector<std::string_view> Backend = tokenize(Filter, ":");
      if (Backend.empty() || (Backend.size() == 1)) {
        continue;
      }
      auto Plugin = Backend.front();
      if (!strncmp(Plugin.data(), "*", Plugin.length())) {
        RTLChecked.insert(RTLChecked.begin(), RTLNames,
                          RTLNames + sizeof(RTLNames) / sizeof(const char *));
        return;
      } else if (!strncmp(Plugin.data(), "level_zero", Plugin.length()) &&
                 !done_l0) {
        RTLChecked.push_back(GET_RTL_NAME(level0));
        done_l0 = true;
      } else if (!strncmp(Plugin.data(), "opencl", Plugin.length()) &&
                 !done_opencl) {
        RTLChecked.push_back(GET_RTL_NAME(opencl));
        done_opencl = true;
      } else if (!strncmp(Plugin.data(), "unified_runtime", Plugin.length()) &&
                 !done_ur) {
        RTLChecked.push_back(GET_RTL_NAME(unified_runtime));
        done_ur = true;
      } else if (!strncmp(Plugin.data(), "x86_64", Plugin.length()) &&
                 !done_x86_64) {
        RTLChecked.push_back(GET_RTL_NAME(x86_64));
        done_x86_64 = true;
      } else if (Plugin[0] == '!') {
        DP("Negative filter ignored currently not supported '%.*s'\n",
           static_cast<int>(Plugin.length()), Plugin.data());
      } else {
        DP("Unknown  plugin specified with ONEAPI_DEVICE_SELECTOR '%.*s'\n",
           static_cast<int>(Plugin.length()), Plugin.data());
      }
    }
    if (char *EnvStr = getenv("LIBOMPTARGET_PLUGIN"))
      DP("Ignoring LIBOMPTARGET_PLUGIN  since ONEAPI_DEVICE_SELECTOR is  "
         "set\n");
    return;
  }

  if (char *EnvStr = getenv("LIBOMPTARGET_PLUGIN")) {
    std::string PlugInName(EnvStr);
    if (PlugInName == "OPENCL" || PlugInName == "opencl") {
      RTLChecked.push_back(GET_RTL_NAME(opencl));
    } else if (PlugInName == "LEVEL0" || PlugInName == "level0" ||
               PlugInName == "LEVEL_ZERO" || PlugInName == "level_zero") {
      RTLChecked.push_back(GET_RTL_NAME(level0));
#if OMPTARGET_UNIFIED_RUNTIME_BUILD
    } else if (PlugInName == "UNIFIED_RUNTIME" ||
               PlugInName == "unified_runtime") {
      RTLChecked.push_back(GET_RTL_NAME(unified_runtime));
#endif // OMPTARGET_UNIFIED_RUNTIME_BUILD
    } else if (PlugInName == "X86_64" || PlugInName == "x86_64") {
      RTLChecked.push_back(GET_RTL_NAME(x86_64));
    } else {
      DP("Unknown plugin name '%s'\n", EnvStr);
    }
  }
  return;
}
#endif // INTEL_CUSTOMIZATION

void RTLsTy::loadRTLs() {

#if INTEL_CUSTOMIZATION
#if !_WIN32
  // Turn on helper task
  if (!getenv("LIBOMP_USE_HIDDEN_HELPER_TASK"))
    kmp_set_defaults("LIBOMP_USE_HIDDEN_HELPER_TASK=1");
#endif // !_WIN32
#endif // INTEL_CUSTOMIZATION

  // Parse environment variable OMP_TARGET_OFFLOAD (if set)
  PM->TargetOffloadPolicy =
      (kmp_target_offload_kind_t)__kmpc_get_target_offload();
  if (PM->TargetOffloadPolicy == tgt_disabled) {
    return;
  }
#if INTEL_CUSTOMIZATION
  XPTIRegistry->initializeFrameworkOnce();
#ifdef OMPT_SUPPORT
  ompt::connectLibrary();
#endif
#endif // INTEL_CUSTOMIZATION

  DP("Loading RTLs...\n");

#if INTEL_CUSTOMIZATION
  // Only check a single plugin if specified by user
  std::vector<const char *> RTLChecked;
  getPlugInNameFromEnv(RTLChecked);

  // Use the whole list by default
  if (RTLChecked.empty()) {
    RTLChecked.insert(RTLChecked.begin(), RTLNames,
                      RTLNames + sizeof(RTLNames) / sizeof(const char *));
#if _WIN32
    // Add OpenCL RTL if CPU device type is requested.
    const char *EnvStr = getenv("LIBOMPTARGET_DEVICETYPE");
    if (EnvStr) {
      std::string DeviceType(EnvStr);
      if (DeviceType == "CPU" || DeviceType == "cpu")
        RTLChecked.push_back("omptarget.rtl.opencl.dll");
    }
#endif // _WIN32
  } else {
    DP("Checking user-specified plugin '%s'...\n", RTLChecked[0]);
  }

  for (auto *Name : RTLChecked) {
    AllRTLs.emplace_back();
    RTLInfoTy &RTL = AllRTLs.back();
    const std::string BaseRTLName(Name);
#ifdef OMPTARGET_DEBUG
    RTL.RTLName = Name;
#endif
    RTL.RTLConstName = Name;

    if (!attemptLoadRTL(BaseRTLName, RTL))
#else  // INTEL_CUSTOMIZATION
  // Attempt to open all the plugins and, if they exist, check if the interface
  // is correct and if they are supporting any devices.
  for (const char *Name : RTLNames) {
    AllRTLs.emplace_back();

    RTLInfoTy &RTL = AllRTLs.back();

    const std::string BaseRTLName(Name);
    if (!attemptLoadRTL(BaseRTLName + ".so", RTL))
#endif // INTEL_CUSTOMIZATION
      AllRTLs.pop_back();
  }

  DP("RTLs loaded!\n");
}

bool RTLsTy::attemptLoadRTL(const std::string &RTLName, RTLInfoTy &RTL) {
  const char *Name = RTLName.c_str();

  DP("Loading library '%s'...\n", Name);

  std::string ErrMsg;
  auto DynLibrary = std::make_unique<sys::DynamicLibrary>(
      sys::DynamicLibrary::getPermanentLibrary(Name, &ErrMsg));

  if (!DynLibrary->isValid()) {
    // Library does not exist or cannot be found.
    DP("Unable to load library '%s': %s!\n", Name, ErrMsg.c_str());
    return false;
  }

  DP("Successfully loaded library '%s'!\n", Name);

  // Remove plugin on failure to call optional init_plugin
  *((void **)&RTL.init_plugin) =
      DynLibrary->getAddressOfSymbol("__tgt_rtl_init_plugin");
  if (RTL.init_plugin) {
    int32_t Rc = RTL.init_plugin();
    if (Rc != OFFLOAD_SUCCESS) {
      DP("Unable to initialize library '%s': %u!\n", Name, Rc);
      return false;
    }
  }

  bool ValidPlugin = true;

  if (!(*((void **)&RTL.is_valid_binary) =
            DynLibrary->getAddressOfSymbol("__tgt_rtl_is_valid_binary")))
    ValidPlugin = false;
  if (!(*((void **)&RTL.number_of_devices) =
            DynLibrary->getAddressOfSymbol("__tgt_rtl_number_of_devices")))
    ValidPlugin = false;
  if (!(*((void **)&RTL.init_device) =
            DynLibrary->getAddressOfSymbol("__tgt_rtl_init_device")))
    ValidPlugin = false;
  if (!(*((void **)&RTL.load_binary) =
            DynLibrary->getAddressOfSymbol("__tgt_rtl_load_binary")))
    ValidPlugin = false;
  if (!(*((void **)&RTL.data_alloc) =
            DynLibrary->getAddressOfSymbol("__tgt_rtl_data_alloc")))
    ValidPlugin = false;
  if (!(*((void **)&RTL.data_submit) =
            DynLibrary->getAddressOfSymbol("__tgt_rtl_data_submit")))
    ValidPlugin = false;
  if (!(*((void **)&RTL.data_retrieve) =
            DynLibrary->getAddressOfSymbol("__tgt_rtl_data_retrieve")))
    ValidPlugin = false;
  if (!(*((void **)&RTL.data_delete) =
            DynLibrary->getAddressOfSymbol("__tgt_rtl_data_delete")))
    ValidPlugin = false;
  if (!(*((void **)&RTL.launch_kernel) =
            DynLibrary->getAddressOfSymbol("__tgt_rtl_launch_kernel")))
    ValidPlugin = false;

  // Invalid plugin
  if (!ValidPlugin) {
    DP("Invalid plugin as necessary interface is not found.\n");
    return false;
  }

  // No devices are supported by this RTL?
  if (!(RTL.NumberOfDevices = RTL.number_of_devices())) {
    // The RTL is invalid! Will pop the object from the RTLs list.
    DP("No devices supported in this RTL\n");
    return false;
  }

#ifdef OMPTARGET_DEBUG
  RTL.RTLName = Name;
#endif

  DP("Registering RTL %s supporting %d devices!\n", Name, RTL.NumberOfDevices);

  // Optional functions
  *((void **)&RTL.deinit_plugin) =
      DynLibrary->getAddressOfSymbol("__tgt_rtl_deinit_plugin");
  *((void **)&RTL.is_valid_binary_info) =
      DynLibrary->getAddressOfSymbol("__tgt_rtl_is_valid_binary_info");
  *((void **)&RTL.deinit_device) =
      DynLibrary->getAddressOfSymbol("__tgt_rtl_deinit_device");
  *((void **)&RTL.init_requires) =
      DynLibrary->getAddressOfSymbol("__tgt_rtl_init_requires");
  *((void **)&RTL.data_submit_async) =
      DynLibrary->getAddressOfSymbol("__tgt_rtl_data_submit_async");
  *((void **)&RTL.data_retrieve_async) =
      DynLibrary->getAddressOfSymbol("__tgt_rtl_data_retrieve_async");
  *((void **)&RTL.synchronize) =
      DynLibrary->getAddressOfSymbol("__tgt_rtl_synchronize");
  *((void **)&RTL.query_async) =
      DynLibrary->getAddressOfSymbol("__tgt_rtl_query_async");
  *((void **)&RTL.data_exchange) =
      DynLibrary->getAddressOfSymbol("__tgt_rtl_data_exchange");
  *((void **)&RTL.data_exchange_async) =
      DynLibrary->getAddressOfSymbol("__tgt_rtl_data_exchange_async");
  *((void **)&RTL.is_data_exchangable) =
      DynLibrary->getAddressOfSymbol("__tgt_rtl_is_data_exchangable");
  *((void **)&RTL.register_lib) =
      DynLibrary->getAddressOfSymbol("__tgt_rtl_register_lib");
  *((void **)&RTL.unregister_lib) =
      DynLibrary->getAddressOfSymbol("__tgt_rtl_unregister_lib");
  *((void **)&RTL.supports_empty_images) =
      DynLibrary->getAddressOfSymbol("__tgt_rtl_supports_empty_images");
  *((void **)&RTL.set_info_flag) =
      DynLibrary->getAddressOfSymbol("__tgt_rtl_set_info_flag");
  *((void **)&RTL.print_device_info) =
      DynLibrary->getAddressOfSymbol("__tgt_rtl_print_device_info");
  *((void **)&RTL.create_event) =
      DynLibrary->getAddressOfSymbol("__tgt_rtl_create_event");
  *((void **)&RTL.record_event) =
      DynLibrary->getAddressOfSymbol("__tgt_rtl_record_event");
  *((void **)&RTL.wait_event) =
      DynLibrary->getAddressOfSymbol("__tgt_rtl_wait_event");
  *((void **)&RTL.sync_event) =
      DynLibrary->getAddressOfSymbol("__tgt_rtl_sync_event");
  *((void **)&RTL.destroy_event) =
      DynLibrary->getAddressOfSymbol("__tgt_rtl_destroy_event");
  *((void **)&RTL.release_async_info) =
      DynLibrary->getAddressOfSymbol("__tgt_rtl_release_async_info");
  *((void **)&RTL.init_async_info) =
      DynLibrary->getAddressOfSymbol("__tgt_rtl_init_async_info");
  *((void **)&RTL.init_device_info) =
      DynLibrary->getAddressOfSymbol("__tgt_rtl_init_device_info");
#if INTEL_CUSTOMIZATION
#define SET_OPTIONAL_INTERFACE(Entry, Name)                                    \
  do {                                                                         \
    if ((*((void **)&RTL.Entry) =                                              \
             DynLibrary->getAddressOfSymbol("__tgt_rtl_" #Name)))              \
      DP("Optional interface: __tgt_rtl_" #Name "\n");                         \
  } while (0)
#define SET_OPTIONAL_INTERFACE_FN(Name) SET_OPTIONAL_INTERFACE(Name, Name)
  SET_OPTIONAL_INTERFACE_FN(data_alloc_base);
  SET_OPTIONAL_INTERFACE_FN(data_realloc);
  SET_OPTIONAL_INTERFACE_FN(data_aligned_alloc);
  SET_OPTIONAL_INTERFACE_FN(register_host_pointer);
  SET_OPTIONAL_INTERFACE_FN(unregister_host_pointer);
  SET_OPTIONAL_INTERFACE_FN(get_device_name);
  SET_OPTIONAL_INTERFACE_FN(get_context_handle);
  SET_OPTIONAL_INTERFACE_FN(get_data_alloc_info);
  SET_OPTIONAL_INTERFACE_FN(requires_mapping);
  SET_OPTIONAL_INTERFACE_FN(manifest_data_for_region);
  SET_OPTIONAL_INTERFACE_FN(push_subdevice);
  SET_OPTIONAL_INTERFACE_FN(pop_subdevice);
  SET_OPTIONAL_INTERFACE_FN(is_supported_device);
  SET_OPTIONAL_INTERFACE_FN(create_interop);
  SET_OPTIONAL_INTERFACE_FN(release_interop);
  SET_OPTIONAL_INTERFACE_FN(use_interop);
  SET_OPTIONAL_INTERFACE_FN(get_num_interop_properties);
  SET_OPTIONAL_INTERFACE_FN(get_interop_property_value);
  SET_OPTIONAL_INTERFACE_FN(get_interop_property_info);
  SET_OPTIONAL_INTERFACE_FN(get_interop_rc_desc);
  SET_OPTIONAL_INTERFACE_FN(get_num_sub_devices);
  SET_OPTIONAL_INTERFACE_FN(is_accessible_addr_range);
  SET_OPTIONAL_INTERFACE_FN(notify_indirect_access);
  SET_OPTIONAL_INTERFACE_FN(is_private_arg_on_host);
  SET_OPTIONAL_INTERFACE_FN(command_batch_begin);
  SET_OPTIONAL_INTERFACE_FN(command_batch_end);
  SET_OPTIONAL_INTERFACE_FN(kernel_batch_begin);
  SET_OPTIONAL_INTERFACE_FN(kernel_batch_end);
  SET_OPTIONAL_INTERFACE_FN(set_function_ptr_map);
  SET_OPTIONAL_INTERFACE(run_team_nd_region, run_target_team_nd_region);
  SET_OPTIONAL_INTERFACE_FN(get_device_info);
  SET_OPTIONAL_INTERFACE_FN(data_aligned_alloc_shared);
  SET_OPTIONAL_INTERFACE_FN(prefetch_shared_mem);
  SET_OPTIONAL_INTERFACE_FN(get_device_from_ptr);
  SET_OPTIONAL_INTERFACE_FN(flush_queue);
  SET_OPTIONAL_INTERFACE_FN(sync_barrier);
  SET_OPTIONAL_INTERFACE_FN(async_barrier);
  SET_OPTIONAL_INTERFACE_FN(memcpy_rect_3d);
  SET_OPTIONAL_INTERFACE_FN(get_groups_shape);
  SET_OPTIONAL_INTERFACE_FN(get_mem_resources);
  SET_OPTIONAL_INTERFACE_FN(omp_alloc);
  SET_OPTIONAL_INTERFACE_FN(omp_free);
  SET_OPTIONAL_INTERFACE_FN(notify_legacy_offload);
#undef SET_OPTIONAL_INTERFACE
#undef SET_OPTIONAL_INTERFACE_FN

#endif // INTEL_CUSTOMIZATION
  *((void **)&RTL.data_lock) =
      DynLibrary->getAddressOfSymbol("__tgt_rtl_data_lock");
  *((void **)&RTL.data_unlock) =
      DynLibrary->getAddressOfSymbol("__tgt_rtl_data_unlock");
  *((void **)&RTL.data_notify_mapped) =
      DynLibrary->getAddressOfSymbol("__tgt_rtl_data_notify_mapped");
  *((void **)&RTL.data_notify_unmapped) =
      DynLibrary->getAddressOfSymbol("__tgt_rtl_data_notify_unmapped");
  *((void **)&RTL.set_device_offset) =
      DynLibrary->getAddressOfSymbol("__tgt_rtl_set_device_offset");

  // Record Replay RTL
  *((void **)&RTL.activate_record_replay) =
      DynLibrary->getAddressOfSymbol("__tgt_rtl_initialize_record_replay");

  RTL.LibraryHandler = std::move(DynLibrary);

  // Successfully loaded
  return true;
}

////////////////////////////////////////////////////////////////////////////////
// Functionality for registering libs

static void registerImageIntoTranslationTable(TranslationTable &TT,
                                              RTLInfoTy &RTL,
                                              __tgt_device_image *Image) {

  // same size, as when we increase one, we also increase the other.
  assert(TT.TargetsTable.size() == TT.TargetsImages.size() &&
         "We should have as many images as we have tables!");

  // Resize the Targets Table and Images to accommodate the new targets if
  // required
  unsigned TargetsTableMinimumSize = RTL.Idx + RTL.NumberOfDevices;

  if (TT.TargetsTable.size() < TargetsTableMinimumSize) {
    TT.TargetsImages.resize(TargetsTableMinimumSize, 0);
    TT.TargetsTable.resize(TargetsTableMinimumSize, 0);
  }

  // Register the image in all devices for this target type.
  for (int32_t I = 0; I < RTL.NumberOfDevices; ++I) {
    // If we are changing the image we are also invalidating the target table.
    if (TT.TargetsImages[RTL.Idx + I] != Image) {
      TT.TargetsImages[RTL.Idx + I] = Image;
      TT.TargetsTable[RTL.Idx + I] = 0; // lazy initialization of target table.
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// Functionality for registering Ctors/Dtors

static void registerGlobalCtorsDtorsForImage(__tgt_bin_desc *Desc,
                                             __tgt_device_image *Img,
                                             RTLInfoTy *RTL) {

  for (int32_t I = 0; I < RTL->NumberOfDevices; ++I) {
    DeviceTy &Device = *PM->Devices[RTL->Idx + I];
    Device.PendingGlobalsMtx.lock();
    Device.HasPendingGlobals = true;
    for (__tgt_offload_entry *Entry = Img->EntriesBegin;
#if INTEL_CUSTOMIZATION
         // Due to paddings potentially inserted by a linker
         // (e.g. due to MSVC incremental linking),
         // the EntriesEnd may be unaligned to the multiple
         // of the entry size. Potentially we may access memory
         // beyond the entries section - we should probably
         // setup the alignment for the entries begin/end
         // symbols and the entries themselves.
         // Another potential issue is that we rely on the gaps
         // inserted by the linker being zeroes.
         Entry < Img->EntriesEnd; ++Entry) {
#else  // INTEL_CUSTOMIZATION
         Entry != Img->EntriesEnd; ++Entry) {
#endif // INTEL_CUSTOMIZATION
      // Globals are not callable and use a different set of flags.
      if (Entry->size != 0)
        continue;

      if (Entry->flags & OMP_DECLARE_TARGET_CTOR) {
        DP("Adding ctor " DPxMOD " to the pending list.\n",
           DPxPTR(Entry->addr));
        Device.PendingCtorsDtors[Desc].PendingCtors.push_back(Entry->addr);
        MESSAGE("WARNING: Calling deprecated constructor for entry %s will be "
                "removed in a future release \n",
                Entry->name);
      } else if (Entry->flags & OMP_DECLARE_TARGET_DTOR) {
        // Dtors are pushed in reverse order so they are executed from end
        // to beginning when unregistering the library!
        DP("Adding dtor " DPxMOD " to the pending list.\n",
           DPxPTR(Entry->addr));
        Device.PendingCtorsDtors[Desc].PendingDtors.push_front(Entry->addr);
        MESSAGE("WARNING: Calling deprecated destructor for entry %s will be "
                "removed in a future release \n",
                Entry->name);
      }

      if (Entry->flags & OMP_DECLARE_TARGET_LINK) {
        DP("The \"link\" attribute is not yet supported!\n");
      }
    }
    Device.PendingGlobalsMtx.unlock();
  }
}

static __tgt_device_image getExecutableImage(__tgt_device_image *Image) {
  StringRef ImageStr(static_cast<char *>(Image->ImageStart),
                     static_cast<char *>(Image->ImageEnd) -
                         static_cast<char *>(Image->ImageStart));
  auto BinaryOrErr =
      object::OffloadBinary::create(MemoryBufferRef(ImageStr, ""));
  if (!BinaryOrErr) {
    consumeError(BinaryOrErr.takeError());
    return *Image;
  }

  void *Begin = const_cast<void *>(
      static_cast<const void *>((*BinaryOrErr)->getImage().bytes_begin()));
  void *End = const_cast<void *>(
      static_cast<const void *>((*BinaryOrErr)->getImage().bytes_end()));

  return {Begin, End, Image->EntriesBegin, Image->EntriesEnd};
}

static __tgt_image_info getImageInfo(__tgt_device_image *Image) {
  StringRef ImageStr(static_cast<char *>(Image->ImageStart),
                     static_cast<char *>(Image->ImageEnd) -
                         static_cast<char *>(Image->ImageStart));
  auto BinaryOrErr =
      object::OffloadBinary::create(MemoryBufferRef(ImageStr, ""));
  if (!BinaryOrErr) {
    consumeError(BinaryOrErr.takeError());
    return __tgt_image_info{};
  }

  return __tgt_image_info{(*BinaryOrErr)->getArch().data()};
}

void RTLsTy::registerRequires(int64_t Flags) {
  // TODO: add more elaborate check.
  // Minimal check: only set requires flags if previous value
  // is undefined. This ensures that only the first call to this
  // function will set the requires flags. All subsequent calls
  // will be checked for compatibility.
  assert(Flags != OMP_REQ_UNDEFINED &&
         "illegal undefined flag for requires directive!");
  if (RequiresFlags == OMP_REQ_UNDEFINED) {
    RequiresFlags = Flags;
    return;
  }

  // If multiple compilation units are present enforce
  // consistency across all of them for require clauses:
  //  - reverse_offload
  //  - unified_address
  //  - unified_shared_memory
  if ((RequiresFlags & OMP_REQ_REVERSE_OFFLOAD) !=
      (Flags & OMP_REQ_REVERSE_OFFLOAD)) {
    FATAL_MESSAGE0(
        1, "'#pragma omp requires reverse_offload' not used consistently!");
  }
  if ((RequiresFlags & OMP_REQ_UNIFIED_ADDRESS) !=
      (Flags & OMP_REQ_UNIFIED_ADDRESS)) {
    FATAL_MESSAGE0(
        1, "'#pragma omp requires unified_address' not used consistently!");
  }
  if ((RequiresFlags & OMP_REQ_UNIFIED_SHARED_MEMORY) !=
      (Flags & OMP_REQ_UNIFIED_SHARED_MEMORY)) {
    FATAL_MESSAGE0(
        1,
        "'#pragma omp requires unified_shared_memory' not used consistently!");
  }

  // TODO: insert any other missing checks

  DP("New requires flags %" PRId64 " compatible with existing %" PRId64 "!\n",
     Flags, RequiresFlags);
}

void RTLsTy::initRTLonce(RTLInfoTy &R) {
  // If this RTL is not already in use, initialize it.
  if (!R.IsUsed && R.NumberOfDevices != 0) {
    // Initialize the device information for the RTL we are about to use.
    const size_t Start = PM->Devices.size();
    PM->Devices.reserve(Start + R.NumberOfDevices);
    for (int32_t DeviceId = 0; DeviceId < R.NumberOfDevices; DeviceId++) {
      PM->Devices.push_back(std::make_unique<DeviceTy>(&R));
      // global device ID
      PM->Devices[Start + DeviceId]->DeviceID = Start + DeviceId;
      // RTL local device ID
      PM->Devices[Start + DeviceId]->RTLDeviceID = DeviceId;
    }

    // Initialize the index of this RTL and save it in the used RTLs.
    R.Idx = (UsedRTLs.empty())
                ? 0
                : UsedRTLs.back()->Idx + UsedRTLs.back()->NumberOfDevices;
    assert((size_t)R.Idx == Start &&
           "RTL index should equal the number of devices used so far.");
    R.IsUsed = true;
    UsedRTLs.push_back(&R);

    // If possible, set the device identifier offset
    if (R.set_device_offset)
      R.set_device_offset(Start);

    DP("RTL " DPxMOD " has index %d!\n", DPxPTR(R.LibraryHandler.get()), R.Idx);
  }
}

void RTLsTy::initAllRTLs() {
  for (auto &R : AllRTLs)
    initRTLonce(R);
}

void RTLsTy::registerLib(__tgt_bin_desc *Desc) {
  PM->RTLsMtx.lock();

  // Extract the exectuable image and extra information if availible.
  for (int32_t i = 0; i < Desc->NumDeviceImages; ++i)
    PM->Images.emplace_back(getExecutableImage(&Desc->DeviceImages[i]),
                            getImageInfo(&Desc->DeviceImages[i]));

  // Register the images with the RTLs that understand them, if any.
  for (auto &ImageAndInfo : PM->Images) {
    // Obtain the image and information that was previously extracted.
    __tgt_device_image *Img = &ImageAndInfo.first;
    __tgt_image_info *Info = &ImageAndInfo.second;

    RTLInfoTy *FoundRTL = nullptr;

    // Scan the RTLs that have associated images until we find one that supports
    // the current image.
    for (auto &R : AllRTLs) {
      if (R.is_valid_binary_info) {
        if (!R.is_valid_binary_info(Img, Info)) {
          DP("Image " DPxMOD " is NOT compatible with RTL %s!\n",
             DPxPTR(Img->ImageStart), R.RTLName.c_str());
          continue;
        }
      } else if (!R.is_valid_binary(Img)) {
        DP("Image " DPxMOD " is NOT compatible with RTL %s!\n",
           DPxPTR(Img->ImageStart), R.RTLName.c_str());
        continue;
      }

      DP("Image " DPxMOD " is compatible with RTL %s!\n",
         DPxPTR(Img->ImageStart), R.RTLName.c_str());

      initRTLonce(R);

      // Initialize (if necessary) translation table for this library.
      PM->TrlTblMtx.lock();
      if (!PM->HostEntriesBeginToTransTable.count(Desc->HostEntriesBegin)) {
        PM->HostEntriesBeginRegistrationOrder.push_back(Desc->HostEntriesBegin);
        TranslationTable &TransTable =
            (PM->HostEntriesBeginToTransTable)[Desc->HostEntriesBegin];
        TransTable.HostTable.EntriesBegin = Desc->HostEntriesBegin;
        TransTable.HostTable.EntriesEnd = Desc->HostEntriesEnd;
      }

      // Retrieve translation table for this library.
      TranslationTable &TransTable =
          (PM->HostEntriesBeginToTransTable)[Desc->HostEntriesBegin];

      DP("Registering image " DPxMOD " with RTL %s!\n", DPxPTR(Img->ImageStart),
         R.RTLName.c_str());
      registerImageIntoTranslationTable(TransTable, R, Img);
      R.UsedImages.insert(Img);

      PM->TrlTblMtx.unlock();
      FoundRTL = &R;

      // Load ctors/dtors for static objects
      registerGlobalCtorsDtorsForImage(Desc, Img, FoundRTL);

      // if an RTL was found we are done - proceed to register the next image
      break;
    }

    if (!FoundRTL) {
      DP("No RTL found for image " DPxMOD "!\n", DPxPTR(Img->ImageStart));
    }
  }
  PM->RTLsMtx.unlock();

  DP("Done registering entries!\n");
}

void RTLsTy::unregisterLib(__tgt_bin_desc *Desc) {
  DP("Unloading target library!\n");

  PM->RTLsMtx.lock();
#if INTEL_CUSTOMIZATION
  PM->InteropTbl.clear();
#endif // INTEL_CUSTOMIZATION
  // Find which RTL understands each image, if any.
  for (auto &ImageAndInfo : PM->Images) {
    // Obtain the image and information that was previously extracted.
    __tgt_device_image *Img = &ImageAndInfo.first;

    RTLInfoTy *FoundRTL = NULL;

    // Scan the RTLs that have associated images until we find one that supports
    // the current image. We only need to scan RTLs that are already being used.
    for (auto *R : UsedRTLs) {

      assert(R->IsUsed && "Expecting used RTLs.");

      // Ensure that we do not use any unused images associated with this RTL.
      if (!R->UsedImages.contains(Img))
        continue;

      FoundRTL = R;

      // Execute dtors for static objects if the device has been used, i.e.
      // if its PendingCtors list has been emptied.
      for (int32_t I = 0; I < FoundRTL->NumberOfDevices; ++I) {
        DeviceTy &Device = *PM->Devices[FoundRTL->Idx + I];
        Device.PendingGlobalsMtx.lock();
        if (Device.PendingCtorsDtors[Desc].PendingCtors.empty()) {
          AsyncInfoTy AsyncInfo(Device);
          for (auto &Dtor : Device.PendingCtorsDtors[Desc].PendingDtors) {
            int Rc =
                target(nullptr, Device, Dtor, CTorDTorKernelArgs, AsyncInfo);
            if (Rc != OFFLOAD_SUCCESS) {
              DP("Running destructor " DPxMOD " failed.\n", DPxPTR(Dtor));
            }
          }
          // Remove this library's entry from PendingCtorsDtors
          Device.PendingCtorsDtors.erase(Desc);
          // All constructors have been issued, wait for them now.
          if (AsyncInfo.synchronize() != OFFLOAD_SUCCESS)
            DP("Failed synchronizing destructors kernels.\n");
        }
        Device.PendingGlobalsMtx.unlock();
      }

      DP("Unregistered image " DPxMOD " from RTL " DPxMOD "!\n",
         DPxPTR(Img->ImageStart), DPxPTR(R->LibraryHandler.get()));

      break;
    }

    // if no RTL was found proceed to unregister the next image
    if (!FoundRTL) {
      DP("No RTLs in use support the image " DPxMOD "!\n",
         DPxPTR(Img->ImageStart));
    }
  }
  PM->RTLsMtx.unlock();
  DP("Done unregistering images!\n");

  // Remove entries from PM->HostPtrToTableMap
  PM->TblMapMtx.lock();
  for (__tgt_offload_entry *Cur = Desc->HostEntriesBegin;
       Cur < Desc->HostEntriesEnd; ++Cur) {
    PM->HostPtrToTableMap.erase(Cur->addr);
  }

  // Remove translation table for this descriptor.
  auto TransTable =
      PM->HostEntriesBeginToTransTable.find(Desc->HostEntriesBegin);
  if (TransTable != PM->HostEntriesBeginToTransTable.end()) {
    DP("Removing translation table for descriptor " DPxMOD "\n",
       DPxPTR(Desc->HostEntriesBegin));
    PM->HostEntriesBeginToTransTable.erase(TransTable);
  } else {
    DP("Translation table for descriptor " DPxMOD " cannot be found, probably "
       "it has been already removed.\n",
       DPxPTR(Desc->HostEntriesBegin));
  }

  PM->TblMapMtx.unlock();

  // TODO: Write some RTL->unload_image(...) function?
  for (auto *R : UsedRTLs) {
    if (R->deinit_plugin) {
      if (R->deinit_plugin() != OFFLOAD_SUCCESS) {
        DP("Failure deinitializing RTL %s!\n", R->RTLName.c_str());
      }
    }
  }

  DP("Done unregistering library!\n");
}
