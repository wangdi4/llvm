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

#include "rtl.h"
#include "device.h"
#include "private.h"
#if INTEL_CUSTOMIZATION
#include "omptarget-tools.h"
#endif // INTEL_CUSTOMIZATION

#include <cassert>
#include <cstdlib>
#include <cstring>
#if INTEL_CUSTOMIZATION
// FIXME: temporary solution for LIBDL on Windows.
#ifdef _WIN32
#include <windows.h>
#include <intel_win_dlfcn.h>
#else  // !_WIN32
#include <dlfcn.h>
#endif // !_WIN32
#include "xpti_registry.h"
#else  // INTEL_CUSTOMIZATION
#include <dlfcn.h>
#endif  // INTEL_CUSTOMIZATION
#include <mutex>
#include <string>

// List of all plugins that can support offloading.
static const char *RTLNames[] = {
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_CSA
    /* CSA target     */ "libomptarget.rtl.csa.so",
#endif  // INTEL_FEATURE_CSA
#endif // INTEL_CUSTOMIZATION
#if INTEL_COLLAB
#if _WIN32
#if INTEL_CUSTOMIZATION
    /* Level0 target  */ "omptarget.rtl.level0.dll",
#endif // INTEL_CUSTOMIZATION
    /* OpenCL target  */ "omptarget.rtl.opencl.dll",
#else  // !_WIN32
#if INTEL_CUSTOMIZATION
    /* Level0 target  */ "libomptarget.rtl.level0.so",
#endif // INTEL_CUSTOMIZATION
    /* OpenCL target  */ "libomptarget.rtl.opencl.so",
#endif // !_WIN32
#endif // INTEL_COLLAB
    /* PowerPC target       */ "libomptarget.rtl.ppc64.so",
#if INTEL_COLLAB
#if _WIN32
    /* x86_64 target        */ "omptarget.rtl.x86_64.dll",
#else  // !_WIN32
    /* x86_64 target        */ "libomptarget.rtl.x86_64.so",
#endif // !_WIN32
#else  // INTEL_COLLAB
    /* x86_64 target        */ "libomptarget.rtl.x86_64.so",
#endif  // INTEL_COLLAB
    /* CUDA target          */ "libomptarget.rtl.cuda.so",
    /* AArch64 target       */ "libomptarget.rtl.aarch64.so",
    /* SX-Aurora VE target  */ "libomptarget.rtl.ve.so",
    /* AMDGPU target        */ "libomptarget.rtl.amdgpu.so",
    /* Remote target        */ "libomptarget.rtl.rpc.so",
};

PluginManager *PM;

#if INTEL_CUSTOMIZATION
OmptGlobalTy *OmptGlobal;
#endif // INTEL_CUSTOMIZATION

#ifdef INTEL_CUSTOMIZATION
#ifdef _WIN32
#define __ATTRIBUTE__(X)
#else
#define __ATTRIBUTE__(X)  __attribute__((X))
#endif
#endif // INTEL_CUSTOMIZATION

#if OMPTARGET_PROFILE_ENABLED
static char *ProfileTraceFile = nullptr;
#endif

__ATTRIBUTE__(constructor(101)) void init() { // INTEL
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
  OmptGlobal = new OmptGlobalTy();
  XPTIRegistry = new XPTIRegistryTy();
#endif // INTEL_CUSTOMIZATION

#ifdef OMPTARGET_PROFILE_ENABLED
  ProfileTraceFile = getenv("LIBOMPTARGET_PROFILE");
  // TODO: add a configuration option for time granularity
  if (ProfileTraceFile)
    llvm::timeTraceProfilerInitialize(500 /* us */, "libomptarget");
#endif
}

__ATTRIBUTE__(destructor(101)) void deinit() { // INTEL
  DP("Deinit target library!\n");
  delete PM;

#if INTEL_CUSTOMIZATION
  delete OmptGlobal;
  delete XPTIRegistry;
#endif // INTEL_CUSTOMIZATION

#ifdef OMPTARGET_PROFILE_ENABLED
  if (ProfileTraceFile) {
    // TODO: add env var for file output
    if (auto E = llvm::timeTraceProfilerWrite(ProfileTraceFile, "-"))
      fprintf(stderr, "Error writing out the time trace\n");

    llvm::timeTraceProfilerCleanup();
  }
#endif
}

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
#endif // INTEL_CUSTOMIZATION

void RTLsTy::LoadRTLs() {
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
  OmptGlobal->init();
  XPTIRegistry->initializeFrameworkOnce();
#endif // INTEL_CUSTOMIZATION

  DP("Loading RTLs...\n");

#if INTEL_COLLAB
  // Only check a single plugin if specified by user
  std::vector<const char *> RTLChecked;
  if (char *envStr = getenv("LIBOMPTARGET_PLUGIN")) {
    std::string pluginName(envStr);
    if (pluginName == "OPENCL" || pluginName == "opencl") {
#if _WIN32
      RTLChecked.push_back("omptarget.rtl.opencl.dll");
#else
      RTLChecked.push_back("libomptarget.rtl.opencl.so");
#endif
#if INTEL_CUSTOMIZATION
    } else if (pluginName == "LEVEL0" || pluginName == "level0" ||
               pluginName == "LEVEL_ZERO" || pluginName == "level_zero") {
#if _WIN32
      RTLChecked.push_back("omptarget.rtl.level0.dll");
#else
      RTLChecked.push_back("libomptarget.rtl.level0.so");
#endif
#endif // INTEL_CUSTOMIZATION
    } else if (pluginName == "CUDA" || pluginName == "cuda") {
      RTLChecked.push_back("libomptarget.rtl.cuda.so");
    } else if (pluginName == "X86_64" || pluginName == "x86_64") {
#if _WIN32
      RTLChecked.push_back("omptarget.rtl.x86_64.dll");
#else
      RTLChecked.push_back("libomptarget.rtl.x86_64.so");
#endif
    } else if (pluginName == "NIOS2" || pluginName == "nios2") {
      RTLChecked.push_back("libomptarget.rtl.nios2.so");
    // TODO: any other plugins to be listed here?
    } else {
      DP("Unknown plugin name '%s'\n", envStr);
    }
  }
  // Use the whole list by default
  if (RTLChecked.empty()) {
    RTLChecked.insert(RTLChecked.begin(), RTLNames,
                      RTLNames + sizeof(RTLNames) / sizeof(const char *));
  } else {
    DP("Checking user-specified plugin '%s'...\n", RTLChecked[0]);
  }

  for (auto *Name : RTLChecked) {
#else // INTEL_COLLAB

  // Attempt to open all the plugins and, if they exist, check if the interface
  // is correct and if they are supporting any devices.
  for (auto *Name : RTLNames) {
#endif // INTEL_COLLAB
    DP("Loading library '%s'...\n", Name);
    void *dynlib_handle = dlopen(Name, RTLD_NOW);

    if (!dynlib_handle) {
      // Library does not exist or cannot be found.
      DP("Unable to load library '%s': %s!\n", Name, dlerror());
      continue;
    }

    DP("Successfully loaded library '%s'!\n", Name);

    AllRTLs.emplace_back();

    // Retrieve the RTL information from the runtime library.
    RTLInfoTy &R = AllRTLs.back();

    bool ValidPlugin = true;

    if (!(*((void **)&R.is_valid_binary) =
              dlsym(dynlib_handle, "__tgt_rtl_is_valid_binary")))
      ValidPlugin = false;
    if (!(*((void **)&R.number_of_devices) =
              dlsym(dynlib_handle, "__tgt_rtl_number_of_devices")))
      ValidPlugin = false;
    if (!(*((void **)&R.init_device) =
              dlsym(dynlib_handle, "__tgt_rtl_init_device")))
      ValidPlugin = false;
    if (!(*((void **)&R.load_binary) =
              dlsym(dynlib_handle, "__tgt_rtl_load_binary")))
      ValidPlugin = false;
    if (!(*((void **)&R.data_alloc) =
              dlsym(dynlib_handle, "__tgt_rtl_data_alloc")))
      ValidPlugin = false;
    if (!(*((void **)&R.data_submit) =
              dlsym(dynlib_handle, "__tgt_rtl_data_submit")))
      ValidPlugin = false;
    if (!(*((void **)&R.data_retrieve) =
              dlsym(dynlib_handle, "__tgt_rtl_data_retrieve")))
      ValidPlugin = false;
    if (!(*((void **)&R.data_delete) =
              dlsym(dynlib_handle, "__tgt_rtl_data_delete")))
      ValidPlugin = false;
    if (!(*((void **)&R.run_region) =
              dlsym(dynlib_handle, "__tgt_rtl_run_target_region")))
      ValidPlugin = false;
    if (!(*((void **)&R.run_team_region) =
              dlsym(dynlib_handle, "__tgt_rtl_run_target_team_region")))
      ValidPlugin = false;

    // Invalid plugin
    if (!ValidPlugin) {
      DP("Invalid plugin as necessary interface is not found.\n");
      AllRTLs.pop_back();
      continue;
    }

    // No devices are supported by this RTL?
    if (!(R.NumberOfDevices = R.number_of_devices())) {
      // The RTL is invalid! Will pop the object from the RTLs list.
      DP("No devices supported in this RTL\n");
      AllRTLs.pop_back();
      continue;
    }

    R.LibraryHandler = dynlib_handle;

#ifdef OMPTARGET_DEBUG
    R.RTLName = Name;
#endif
#if INTEL_COLLAB
    R.RTLConstName = Name;
#endif  // INTEL_COLLAB

    DP("Registering RTL %s supporting %d devices!\n", R.RTLName.c_str(),
       R.NumberOfDevices);

    // Optional functions
    *((void **)&R.deinit_device) =
        dlsym(dynlib_handle, "__tgt_rtl_deinit_device");
    *((void **)&R.init_requires) =
        dlsym(dynlib_handle, "__tgt_rtl_init_requires");
    *((void **)&R.data_submit_async) =
        dlsym(dynlib_handle, "__tgt_rtl_data_submit_async");
    *((void **)&R.data_retrieve_async) =
        dlsym(dynlib_handle, "__tgt_rtl_data_retrieve_async");
    *((void **)&R.run_region_async) =
        dlsym(dynlib_handle, "__tgt_rtl_run_target_region_async");
    *((void **)&R.run_team_region_async) =
        dlsym(dynlib_handle, "__tgt_rtl_run_target_team_region_async");
    *((void **)&R.synchronize) = dlsym(dynlib_handle, "__tgt_rtl_synchronize");
    *((void **)&R.data_exchange) =
        dlsym(dynlib_handle, "__tgt_rtl_data_exchange");
    *((void **)&R.data_exchange_async) =
        dlsym(dynlib_handle, "__tgt_rtl_data_exchange_async");
    *((void **)&R.is_data_exchangable) =
        dlsym(dynlib_handle, "__tgt_rtl_is_data_exchangable");

#if INTEL_COLLAB
    #define SET_OPTIONAL_INTERFACE(entry, name)                                \
      do {                                                                     \
        if ((*((void **)&R.entry) = dlsym(dynlib_handle, "__tgt_rtl_" #name))) \
          DP("Optional interface: __tgt_rtl_" #name "\n");                     \
      } while (0)
    #define SET_OPTIONAL_INTERFACE_FN(name) SET_OPTIONAL_INTERFACE(name, name)
    SET_OPTIONAL_INTERFACE_FN(data_alloc_base);
    SET_OPTIONAL_INTERFACE_FN(data_alloc_managed);
    SET_OPTIONAL_INTERFACE_FN(data_realloc);
    SET_OPTIONAL_INTERFACE_FN(data_aligned_alloc);
    SET_OPTIONAL_INTERFACE_FN(get_offload_queue);
    SET_OPTIONAL_INTERFACE_FN(release_offload_queue);
    SET_OPTIONAL_INTERFACE_FN(get_device_name);
    SET_OPTIONAL_INTERFACE_FN(get_platform_handle);
    SET_OPTIONAL_INTERFACE_FN(set_device_handle);
    SET_OPTIONAL_INTERFACE_FN(get_context_handle);
    SET_OPTIONAL_INTERFACE_FN(get_data_alloc_info);
#if INTEL_CUSTOMIZATION
    SET_OPTIONAL_INTERFACE_FN(init_ompt);
#endif // INTEL_CUSTOMIZATION
    SET_OPTIONAL_INTERFACE_FN(requires_mapping);
    SET_OPTIONAL_INTERFACE_FN(manifest_data_for_region);
    SET_OPTIONAL_INTERFACE_FN(push_subdevice);
    SET_OPTIONAL_INTERFACE_FN(pop_subdevice);
    SET_OPTIONAL_INTERFACE_FN(add_build_options);
    SET_OPTIONAL_INTERFACE_FN(is_supported_device);
    SET_OPTIONAL_INTERFACE_FN(deinit);
    SET_OPTIONAL_INTERFACE_FN(create_interop);
    SET_OPTIONAL_INTERFACE_FN(release_interop);
    SET_OPTIONAL_INTERFACE_FN(use_interop);
    SET_OPTIONAL_INTERFACE_FN(get_num_interop_properties);
    SET_OPTIONAL_INTERFACE_FN(get_interop_property_value);
    SET_OPTIONAL_INTERFACE_FN(get_interop_property_info);
    SET_OPTIONAL_INTERFACE_FN(get_interop_rc_desc);
    SET_OPTIONAL_INTERFACE_FN(get_num_sub_devices);
    SET_OPTIONAL_INTERFACE_FN(is_accessible_addr_range);
#if INTEL_CUSTOMIZATION
    SET_OPTIONAL_INTERFACE_FN(notify_indirect_access);
#endif // INTEL_CUSTOMIZATION
    SET_OPTIONAL_INTERFACE_FN(is_private_arg_on_host);
    SET_OPTIONAL_INTERFACE_FN(command_batch_begin);
    SET_OPTIONAL_INTERFACE_FN(command_batch_end);
    SET_OPTIONAL_INTERFACE_FN(kernel_batch_begin);
    SET_OPTIONAL_INTERFACE_FN(kernel_batch_end);
    SET_OPTIONAL_INTERFACE_FN(set_function_ptr_map);
    SET_OPTIONAL_INTERFACE_FN(alloc_per_hw_thread_scratch);
    SET_OPTIONAL_INTERFACE_FN(free_per_hw_thread_scratch);
    SET_OPTIONAL_INTERFACE(run_team_nd_region, run_target_team_nd_region);
    #undef SET_OPTIONAL_INTERFACE
    #undef SET_OPTIONAL_INTERFACE_FN

#if INTEL_CUSTOMIZATION
    // Initialize RTL's OMPT data
    if (R.init_ompt)
      R.init_ompt(OmptGlobal);
#endif // INTEL_CUSTOMIZATION
#endif // INTEL_COLLAB

    *((void **)&R.register_lib) =
        dlsym(dynlib_handle, "__tgt_rtl_register_lib");
    *((void **)&R.unregister_lib) =
        dlsym(dynlib_handle, "__tgt_rtl_unregister_lib");
    *((void **)&R.supports_empty_images) =
        dlsym(dynlib_handle, "__tgt_rtl_supports_empty_images");
    *((void **)&R.set_info_flag) =
        dlsym(dynlib_handle, "__tgt_rtl_set_info_flag");
    *((void **)&R.print_device_info) =
        dlsym(dynlib_handle, "__tgt_rtl_print_device_info");
    *((void **)&R.create_event) =
        dlsym(dynlib_handle, "__tgt_rtl_create_event");
    *((void **)&R.record_event) =
        dlsym(dynlib_handle, "__tgt_rtl_record_event");
    *((void **)&R.wait_event) = dlsym(dynlib_handle, "__tgt_rtl_wait_event");
    *((void **)&R.sync_event) = dlsym(dynlib_handle, "__tgt_rtl_sync_event");
    *((void **)&R.destroy_event) =
        dlsym(dynlib_handle, "__tgt_rtl_destroy_event");
    *((void **)&R.release_async_info) =
        dlsym(dynlib_handle, "__tgt_rtl_release_async_info");
    *((void **)&R.init_async_info) =
        dlsym(dynlib_handle, "__tgt_rtl_init_async_info");
    *((void **)&R.init_device_info) =
        dlsym(dynlib_handle, "__tgt_rtl_init_device_info");
  }

  DP("RTLs loaded!\n");

  return;
}

////////////////////////////////////////////////////////////////////////////////
// Functionality for registering libs

static void RegisterImageIntoTranslationTable(TranslationTable &TT,
                                              RTLInfoTy &RTL,
                                              __tgt_device_image *image) {

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
  for (int32_t i = 0; i < RTL.NumberOfDevices; ++i) {
    // If we are changing the image we are also invalidating the target table.
    if (TT.TargetsImages[RTL.Idx + i] != image) {
      TT.TargetsImages[RTL.Idx + i] = image;
      TT.TargetsTable[RTL.Idx + i] = 0; // lazy initialization of target table.
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// Functionality for registering Ctors/Dtors

static void RegisterGlobalCtorsDtorsForImage(__tgt_bin_desc *desc,
                                             __tgt_device_image *img,
                                             RTLInfoTy *RTL) {

  for (int32_t i = 0; i < RTL->NumberOfDevices; ++i) {
    DeviceTy &Device = *PM->Devices[RTL->Idx + i];
    Device.PendingGlobalsMtx.lock();
    Device.HasPendingGlobals = true;
    for (__tgt_offload_entry *entry = img->EntriesBegin;
#if INTEL_COLLAB
         // Due to paddings potentially inserted by a linker
         // (e.g. due to MSVC incremental linking),
         // the EntriesEnd may be unaligned to the multiple
         // of the entry size. Potentially we may access memory
         // beyond the entries section - we should probably
         // setup the alignment for the entries begin/end
         // symbols and the entries themselves.
         // Another potential issue is that we rely on the gaps
         // inserted by the linker being zeroes.
         entry < img->EntriesEnd; ++entry) {
#else  // INTEL_COLLAB
         entry != img->EntriesEnd; ++entry) {
#endif  // INTEL_COLLAB
      if (entry->flags & OMP_DECLARE_TARGET_CTOR) {
        DP("Adding ctor " DPxMOD " to the pending list.\n",
           DPxPTR(entry->addr));
        Device.PendingCtorsDtors[desc].PendingCtors.push_back(entry->addr);
      } else if (entry->flags & OMP_DECLARE_TARGET_DTOR) {
        // Dtors are pushed in reverse order so they are executed from end
        // to beginning when unregistering the library!
        DP("Adding dtor " DPxMOD " to the pending list.\n",
           DPxPTR(entry->addr));
        Device.PendingCtorsDtors[desc].PendingDtors.push_front(entry->addr);
      }

      if (entry->flags & OMP_DECLARE_TARGET_LINK) {
        DP("The \"link\" attribute is not yet supported!\n");
      }
    }
    Device.PendingGlobalsMtx.unlock();
  }
}

void RTLsTy::RegisterRequires(int64_t flags) {
  // TODO: add more elaborate check.
  // Minimal check: only set requires flags if previous value
  // is undefined. This ensures that only the first call to this
  // function will set the requires flags. All subsequent calls
  // will be checked for compatibility.
  assert(flags != OMP_REQ_UNDEFINED &&
         "illegal undefined flag for requires directive!");
  if (RequiresFlags == OMP_REQ_UNDEFINED) {
    RequiresFlags = flags;
    return;
  }

  // If multiple compilation units are present enforce
  // consistency across all of them for require clauses:
  //  - reverse_offload
  //  - unified_address
  //  - unified_shared_memory
  if ((RequiresFlags & OMP_REQ_REVERSE_OFFLOAD) !=
      (flags & OMP_REQ_REVERSE_OFFLOAD)) {
    FATAL_MESSAGE0(
        1, "'#pragma omp requires reverse_offload' not used consistently!");
  }
  if ((RequiresFlags & OMP_REQ_UNIFIED_ADDRESS) !=
      (flags & OMP_REQ_UNIFIED_ADDRESS)) {
    FATAL_MESSAGE0(
        1, "'#pragma omp requires unified_address' not used consistently!");
  }
  if ((RequiresFlags & OMP_REQ_UNIFIED_SHARED_MEMORY) !=
      (flags & OMP_REQ_UNIFIED_SHARED_MEMORY)) {
    FATAL_MESSAGE0(
        1,
        "'#pragma omp requires unified_shared_memory' not used consistently!");
  }

  // TODO: insert any other missing checks

  DP("New requires flags %" PRId64 " compatible with existing %" PRId64 "!\n",
     flags, RequiresFlags);
}

void RTLsTy::initRTLonce(RTLInfoTy &R) {
  // If this RTL is not already in use, initialize it.
  if (!R.isUsed && R.NumberOfDevices != 0) {
    // Initialize the device information for the RTL we are about to use.
    const size_t Start = PM->Devices.size();
    PM->Devices.reserve(Start + R.NumberOfDevices);
    for (int32_t device_id = 0; device_id < R.NumberOfDevices; device_id++) {
      PM->Devices.push_back(std::make_unique<DeviceTy>(&R));
      // global device ID
      PM->Devices[Start + device_id]->DeviceID = Start + device_id;
      // RTL local device ID
      PM->Devices[Start + device_id]->RTLDeviceID = device_id;
    }

    // Initialize the index of this RTL and save it in the used RTLs.
    R.Idx = (UsedRTLs.empty())
                ? 0
                : UsedRTLs.back()->Idx + UsedRTLs.back()->NumberOfDevices;
    assert((size_t)R.Idx == Start &&
           "RTL index should equal the number of devices used so far.");
    R.isUsed = true;
    UsedRTLs.push_back(&R);

    DP("RTL " DPxMOD " has index %d!\n", DPxPTR(R.LibraryHandler), R.Idx);
  }
}

void RTLsTy::initAllRTLs() {
  for (auto &R : AllRTLs)
    initRTLonce(R);
}

void RTLsTy::RegisterLib(__tgt_bin_desc *desc) {
  PM->RTLsMtx.lock();
  // Register the images with the RTLs that understand them, if any.
  for (int32_t i = 0; i < desc->NumDeviceImages; ++i) {
    // Obtain the image.
    __tgt_device_image *img = &desc->DeviceImages[i];

    RTLInfoTy *FoundRTL = nullptr;

    // Scan the RTLs that have associated images until we find one that supports
    // the current image.
    for (auto &R : AllRTLs) {
      if (!R.is_valid_binary(img)) {
        DP("Image " DPxMOD " is NOT compatible with RTL %s!\n",
           DPxPTR(img->ImageStart), R.RTLName.c_str());
        continue;
      }

      DP("Image " DPxMOD " is compatible with RTL %s!\n",
         DPxPTR(img->ImageStart), R.RTLName.c_str());

      initRTLonce(R);

      // Initialize (if necessary) translation table for this library.
      PM->TrlTblMtx.lock();
      if (!PM->HostEntriesBeginToTransTable.count(desc->HostEntriesBegin)) {
        PM->HostEntriesBeginRegistrationOrder.push_back(desc->HostEntriesBegin);
        TranslationTable &TransTable =
            (PM->HostEntriesBeginToTransTable)[desc->HostEntriesBegin];
        TransTable.HostTable.EntriesBegin = desc->HostEntriesBegin;
        TransTable.HostTable.EntriesEnd = desc->HostEntriesEnd;
      }

      // Retrieve translation table for this library.
      TranslationTable &TransTable =
          (PM->HostEntriesBeginToTransTable)[desc->HostEntriesBegin];

      DP("Registering image " DPxMOD " with RTL %s!\n", DPxPTR(img->ImageStart),
         R.RTLName.c_str());
      RegisterImageIntoTranslationTable(TransTable, R, img);
      PM->TrlTblMtx.unlock();
      FoundRTL = &R;

      // Load ctors/dtors for static objects
      RegisterGlobalCtorsDtorsForImage(desc, img, FoundRTL);

      // if an RTL was found we are done - proceed to register the next image
      break;
    }

    if (!FoundRTL) {
      DP("No RTL found for image " DPxMOD "!\n", DPxPTR(img->ImageStart));
    }
  }
  PM->RTLsMtx.unlock();

  DP("Done registering entries!\n");
}

void RTLsTy::UnregisterLib(__tgt_bin_desc *desc) {
  DP("Unloading target library!\n");

  PM->RTLsMtx.lock();
  // Find which RTL understands each image, if any.
  for (int32_t i = 0; i < desc->NumDeviceImages; ++i) {
    // Obtain the image.
    __tgt_device_image *img = &desc->DeviceImages[i];

    RTLInfoTy *FoundRTL = NULL;

    // Scan the RTLs that have associated images until we find one that supports
    // the current image. We only need to scan RTLs that are already being used.
    for (auto *R : UsedRTLs) {

      assert(R->isUsed && "Expecting used RTLs.");

      if (!R->is_valid_binary(img)) {
        DP("Image " DPxMOD " is NOT compatible with RTL " DPxMOD "!\n",
           DPxPTR(img->ImageStart), DPxPTR(R->LibraryHandler));
        continue;
      }

      DP("Image " DPxMOD " is compatible with RTL " DPxMOD "!\n",
         DPxPTR(img->ImageStart), DPxPTR(R->LibraryHandler));

      FoundRTL = R;

      // Execute dtors for static objects if the device has been used, i.e.
      // if its PendingCtors list has been emptied.
      for (int32_t i = 0; i < FoundRTL->NumberOfDevices; ++i) {
        DeviceTy &Device = *PM->Devices[FoundRTL->Idx + i];
        Device.PendingGlobalsMtx.lock();
        if (Device.PendingCtorsDtors[desc].PendingCtors.empty()) {
          AsyncInfoTy AsyncInfo(Device);
          for (auto &dtor : Device.PendingCtorsDtors[desc].PendingDtors) {
            int rc = target(nullptr, Device, dtor, 0, nullptr, nullptr, nullptr,
                            nullptr, nullptr, nullptr, 1, 1, true /*team*/,
                            AsyncInfo);
            if (rc != OFFLOAD_SUCCESS) {
              DP("Running destructor " DPxMOD " failed.\n", DPxPTR(dtor));
            }
          }
          // Remove this library's entry from PendingCtorsDtors
          Device.PendingCtorsDtors.erase(desc);
          // All constructors have been issued, wait for them now.
          if (AsyncInfo.synchronize() != OFFLOAD_SUCCESS)
            DP("Failed synchronizing destructors kernels.\n");
        }
        Device.PendingGlobalsMtx.unlock();
      }

      DP("Unregistered image " DPxMOD " from RTL " DPxMOD "!\n",
         DPxPTR(img->ImageStart), DPxPTR(R->LibraryHandler));

      break;
    }

    // if no RTL was found proceed to unregister the next image
    if (!FoundRTL) {
      DP("No RTLs in use support the image " DPxMOD "!\n",
         DPxPTR(img->ImageStart));
    }
  }
  PM->RTLsMtx.unlock();
  DP("Done unregistering images!\n");

  // Remove entries from PM->HostPtrToTableMap
  PM->TblMapMtx.lock();
  for (__tgt_offload_entry *cur = desc->HostEntriesBegin;
       cur < desc->HostEntriesEnd; ++cur) {
    PM->HostPtrToTableMap.erase(cur->addr);
  }

  // Remove translation table for this descriptor.
  auto TransTable =
      PM->HostEntriesBeginToTransTable.find(desc->HostEntriesBegin);
  if (TransTable != PM->HostEntriesBeginToTransTable.end()) {
    DP("Removing translation table for descriptor " DPxMOD "\n",
       DPxPTR(desc->HostEntriesBegin));
    PM->HostEntriesBeginToTransTable.erase(TransTable);
  } else {
    DP("Translation table for descriptor " DPxMOD " cannot be found, probably "
       "it has been already removed.\n",
       DPxPTR(desc->HostEntriesBegin));
  }

  PM->TblMapMtx.unlock();
#if INTEL_COLLAB
  // Force RTL deinit if applicable
  for (auto *R : UsedRTLs)
    if (R->deinit)
      R->deinit();
#endif // INTEL_COLLAB

  // TODO: Remove RTL and the devices it manages if it's not used anymore?
  // TODO: Write some RTL->unload_image(...) function?

  DP("Done unregistering library!\n");
}
