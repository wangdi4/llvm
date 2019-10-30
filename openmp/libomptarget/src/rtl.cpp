//===----------- rtl.cpp - Target independent OpenMP target RTL -----------===//
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

#include "device.h"
#include "private.h"
#include "rtl.h"

#include <cassert>
#include <cstdlib>
#include <cstring>
#if INTEL_CUSTOMIZATION
// FIXME: temporary solution for LIBDL on Windows.
#ifdef _WIN32
#include <intel_win_dlfcn.h>
#else  // !_WIN32
#include <dlfcn.h>
#endif // !_WIN32
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
    /* OpenCL target  */ "omptarget.rtl.opencl.dll",
#else  // !_WIN32
    /* OpenCL target  */ "libomptarget.rtl.opencl.so",
#endif // !_WIN32
#endif // INTEL_COLLAB
    /* PowerPC target */ "libomptarget.rtl.ppc64.so",
#if INTEL_COLLAB
#if _WIN32
    /* x86_64 target  */ "omptarget.rtl.x86_64.dll",
#else  // !_WIN32
    /* x86_64 target  */ "libomptarget.rtl.x86_64.so",
#endif // !_WIN32
#else  // INTEL_COLLAB
    /* x86_64 target  */ "libomptarget.rtl.x86_64.so",
#endif  // INTEL_COLLAB
    /* CUDA target    */ "libomptarget.rtl.cuda.so",
    /* AArch64 target */ "libomptarget.rtl.aarch64.so"};

RTLsTy RTLs;
std::mutex RTLsMtx;

HostEntriesBeginToTransTableTy HostEntriesBeginToTransTable;
std::mutex TrlTblMtx;

HostPtrToTableMapTy HostPtrToTableMap;
std::mutex TblMapMtx;

void RTLsTy::LoadRTLs() {
#ifdef OMPTARGET_DEBUG
  if (char *envStr = getenv("LIBOMPTARGET_DEBUG")) {
    DebugLevel = std::stoi(envStr);
  }
#endif // OMPTARGET_DEBUG

  // Parse environment variable OMP_TARGET_OFFLOAD (if set)
  TargetOffloadPolicy = (kmp_target_offload_kind_t) __kmpc_get_target_offload();
  if (TargetOffloadPolicy == tgt_disabled) {
    return;
  }

  DP("Loading RTLs...\n");

  // Attempt to open all the plugins and, if they exist, check if the interface
  // is correct and if they are supporting any devices.
  for (auto *Name : RTLNames) {
    DP("Loading library '%s'...\n", Name);
    void *dynlib_handle = dlopen(Name, RTLD_NOW);

    if (!dynlib_handle) {
      // Library does not exist or cannot be found.
      DP("Unable to load library '%s': %s!\n", Name, dlerror());
      continue;
    }

    DP("Successfully loaded library '%s'!\n", Name);

    // Retrieve the RTL information from the runtime library.
    RTLInfoTy R;

    R.LibraryHandler = dynlib_handle;
    R.isUsed = false;

#ifdef OMPTARGET_DEBUG
    R.RTLName = Name;
#endif
#if INTEL_COLLAB
    R.RTLConstName = Name;
#endif  // INTEL_COLLAB

    if (!(*((void**) &R.is_valid_binary) = dlsym(
              dynlib_handle, "__tgt_rtl_is_valid_binary")))
      continue;
    if (!(*((void**) &R.number_of_devices) = dlsym(
              dynlib_handle, "__tgt_rtl_number_of_devices")))
      continue;
    if (!(*((void**) &R.init_device) = dlsym(
              dynlib_handle, "__tgt_rtl_init_device")))
      continue;
    if (!(*((void**) &R.load_binary) = dlsym(
              dynlib_handle, "__tgt_rtl_load_binary")))
      continue;
    if (!(*((void**) &R.data_alloc) = dlsym(
              dynlib_handle, "__tgt_rtl_data_alloc")))
      continue;
    if (!(*((void**) &R.data_submit) = dlsym(
              dynlib_handle, "__tgt_rtl_data_submit")))
      continue;
    if (!(*((void**) &R.data_retrieve) = dlsym(
              dynlib_handle, "__tgt_rtl_data_retrieve")))
      continue;
    if (!(*((void**) &R.data_delete) = dlsym(
              dynlib_handle, "__tgt_rtl_data_delete")))
      continue;
    if (!(*((void**) &R.run_region) = dlsym(
              dynlib_handle, "__tgt_rtl_run_target_region")))
      continue;
    if (!(*((void**) &R.run_team_region) = dlsym(
              dynlib_handle, "__tgt_rtl_run_target_team_region")))
      continue;
#if INTEL_COLLAB
    if ((*((void **)&R.data_submit_nowait) =
              dlsym(dynlib_handle, "__tgt_rtl_data_submit_nowait")))
      DP("Optional interface: __tgt_rtl_data_submit_nowait\n");
    if ((*((void **)&R.data_retrieve_nowait) =
              dlsym(dynlib_handle, "__tgt_rtl_data_retrieve_nowait")))
      DP("Optional interface: __tgt_rtl_data_retrieve_nowait\n");
    if ((*((void **)&R.manifest_data_for_region) =
              dlsym(dynlib_handle, "__tgt_rtl_manifest_data_for_region")))
      DP("Optional interface: __tgt_rtl_manifest_data_for_region\n");
    if ((*((void **)&R.data_alloc_base) =
              dlsym(dynlib_handle, "__tgt_rtl_data_alloc_base")))
      DP("Optional interface: __tgt_rtl_data_alloc_base\n");
    if ((*((void **)&R.data_alloc_user) =
              dlsym(dynlib_handle, "__tgt_rtl_data_alloc_user")))
      DP("Optional interface: __tgt_rtl_data_alloc_user\n");
    if ((*((void **)&R.create_buffer) =
              dlsym(dynlib_handle, "__tgt_rtl_create_buffer")))
      DP("Optional interface: __tgt_rtl_create_buffer\n");
    if ((*((void **)&R.get_device_name) =
              dlsym(dynlib_handle, "__tgt_rtl_get_device_name")))
      DP("Optional interface: __tgt_rtl_get_device_name\n");
    if ((*((void **)&R.release_buffer) =
              dlsym(dynlib_handle, "__tgt_rtl_release_buffer")))
      DP("Optional interface: __tgt_rtl_release_buffer\n");
    if ((*((void **)&R.run_team_nd_region) =
              dlsym(dynlib_handle, "__tgt_rtl_run_target_team_nd_region")))
      DP("Optional interface: __tgt_rtl_run_target_team_nd_region\n");
    if ((*((void **)&R.run_region_nowait) =
              dlsym(dynlib_handle, "__tgt_rtl_run_target_region_nowait")))
      DP("Optional interface: __tgt_rtl_run_target_region_nowait\n");
    if ((*((void **)&R.run_team_region_nowait) =
              dlsym(dynlib_handle, "__tgt_rtl_run_target_team_region_nowait")))
      DP("Optional interface: __tgt_rtl_run_target_team_region_nowait\n");
    if ((*((void **)&R.run_team_nd_region_nowait) = dlsym(
              dynlib_handle, "__tgt_rtl_run_target_team_nd_region_nowait")))
      DP("Optional interface: __tgt_rtl_run_target_team_nd_region_nowait\n");
#endif // INTEL_COLLAB

    // Optional functions
    *((void**) &R.init_requires) = dlsym(
        dynlib_handle, "__tgt_rtl_init_requires");

    // No devices are supported by this RTL?
    if (!(R.NumberOfDevices = R.number_of_devices())) {
      DP("No devices supported in this RTL\n");
      continue;
    }

    DP("Registering RTL %s supporting %d devices!\n",
        R.RTLName.c_str(), R.NumberOfDevices);

    // The RTL is valid! Will save the information in the RTLs list.
    AllRTLs.push_back(R);
  }

  DP("RTLs loaded!\n");

  return;
}

////////////////////////////////////////////////////////////////////////////////
// Functionality for registering libs

static void RegisterImageIntoTranslationTable(TranslationTable &TT,
    RTLInfoTy &RTL, __tgt_device_image *image) {

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
    __tgt_device_image *img, RTLInfoTy *RTL) {

  for (int32_t i = 0; i < RTL->NumberOfDevices; ++i) {
    DeviceTy &Device = Devices[RTL->Idx + i];
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
    FATAL_MESSAGE0(1,
        "'#pragma omp requires reverse_offload' not used consistently!");
  }
  if ((RequiresFlags & OMP_REQ_UNIFIED_ADDRESS) !=
          (flags & OMP_REQ_UNIFIED_ADDRESS)) {
    FATAL_MESSAGE0(1,
        "'#pragma omp requires unified_address' not used consistently!");
  }
  if ((RequiresFlags & OMP_REQ_UNIFIED_SHARED_MEMORY) !=
          (flags & OMP_REQ_UNIFIED_SHARED_MEMORY)) {
    FATAL_MESSAGE0(1,
        "'#pragma omp requires unified_shared_memory' not used consistently!");
  }

  // TODO: insert any other missing checks

  DP("New requires flags %" PRId64 " compatible with existing %" PRId64 "!\n",
     flags, RequiresFlags);
}

void RTLsTy::RegisterLib(__tgt_bin_desc *desc) {
  // Attempt to load all plugins available in the system.
  std::call_once(initFlag, &RTLsTy::LoadRTLs, this);

  RTLsMtx.lock();
  // Register the images with the RTLs that understand them, if any.
  for (int32_t i = 0; i < desc->NumDeviceImages; ++i) {
    // Obtain the image.
    __tgt_device_image *img = &desc->DeviceImages[i];

    RTLInfoTy *FoundRTL = NULL;

    // Scan the RTLs that have associated images until we find one that supports
    // the current image.
    for (auto &R : RTLs.AllRTLs) {
      if (!R.is_valid_binary(img)) {
        DP("Image " DPxMOD " is NOT compatible with RTL %s!\n",
            DPxPTR(img->ImageStart), R.RTLName.c_str());
        continue;
      }

      DP("Image " DPxMOD " is compatible with RTL %s!\n",
          DPxPTR(img->ImageStart), R.RTLName.c_str());

      // If this RTL is not already in use, initialize it.
      if (!R.isUsed) {
        // Initialize the device information for the RTL we are about to use.
        DeviceTy device(&R);
        size_t start = Devices.size();
        Devices.resize(start + R.NumberOfDevices, device);
        for (int32_t device_id = 0; device_id < R.NumberOfDevices;
            device_id++) {
          // global device ID
          Devices[start + device_id].DeviceID = start + device_id;
          // RTL local device ID
          Devices[start + device_id].RTLDeviceID = device_id;
        }

        // Initialize the index of this RTL and save it in the used RTLs.
        R.Idx = (RTLs.UsedRTLs.empty())
                    ? 0
                    : RTLs.UsedRTLs.back()->Idx +
                          RTLs.UsedRTLs.back()->NumberOfDevices;
        assert((size_t) R.Idx == start &&
            "RTL index should equal the number of devices used so far.");
        R.isUsed = true;
        RTLs.UsedRTLs.push_back(&R);

        DP("RTL " DPxMOD " has index %d!\n", DPxPTR(R.LibraryHandler), R.Idx);
      }

      // Initialize (if necessary) translation table for this library.
      TrlTblMtx.lock();
      if(!HostEntriesBeginToTransTable.count(desc->HostEntriesBegin)){
        TranslationTable &tt =
            HostEntriesBeginToTransTable[desc->HostEntriesBegin];
        tt.HostTable.EntriesBegin = desc->HostEntriesBegin;
        tt.HostTable.EntriesEnd = desc->HostEntriesEnd;
      }

      // Retrieve translation table for this library.
      TranslationTable &TransTable =
          HostEntriesBeginToTransTable[desc->HostEntriesBegin];

      DP("Registering image " DPxMOD " with RTL %s!\n",
          DPxPTR(img->ImageStart), R.RTLName.c_str());
      RegisterImageIntoTranslationTable(TransTable, R, img);
      TrlTblMtx.unlock();
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
  RTLsMtx.unlock();


  DP("Done registering entries!\n");
}

void RTLsTy::UnregisterLib(__tgt_bin_desc *desc) {
  DP("Unloading target library!\n");

  RTLsMtx.lock();
  // Find which RTL understands each image, if any.
  for (int32_t i = 0; i < desc->NumDeviceImages; ++i) {
    // Obtain the image.
    __tgt_device_image *img = &desc->DeviceImages[i];

    RTLInfoTy *FoundRTL = NULL;

    // Scan the RTLs that have associated images until we find one that supports
    // the current image. We only need to scan RTLs that are already being used.
    for (auto *R : RTLs.UsedRTLs) {

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
        DeviceTy &Device = Devices[FoundRTL->Idx + i];
        Device.PendingGlobalsMtx.lock();
        if (Device.PendingCtorsDtors[desc].PendingCtors.empty()) {
          for (auto &dtor : Device.PendingCtorsDtors[desc].PendingDtors) {
            int rc = target(Device.DeviceID, dtor, 0, NULL, NULL, NULL, NULL, 1,
                1, true /*team*/);
            if (rc != OFFLOAD_SUCCESS) {
              DP("Running destructor " DPxMOD " failed.\n", DPxPTR(dtor));
            }
          }
          // Remove this library's entry from PendingCtorsDtors
          Device.PendingCtorsDtors.erase(desc);
        }
        Device.PendingGlobalsMtx.unlock();
      }

      DP("Unregistered image " DPxMOD " from RTL " DPxMOD "!\n",
          DPxPTR(img->ImageStart), DPxPTR(R->LibraryHandler));

      break;
    }

    // if no RTL was found proceed to unregister the next image
    if (!FoundRTL){
      DP("No RTLs in use support the image " DPxMOD "!\n",
          DPxPTR(img->ImageStart));
    }
  }
  RTLsMtx.unlock();
  DP("Done unregistering images!\n");

  // Remove entries from HostPtrToTableMap
  TblMapMtx.lock();
  for (__tgt_offload_entry *cur = desc->HostEntriesBegin;
      cur < desc->HostEntriesEnd; ++cur) {
    HostPtrToTableMap.erase(cur->addr);
  }

  // Remove translation table for this descriptor.
  auto tt = HostEntriesBeginToTransTable.find(desc->HostEntriesBegin);
  if (tt != HostEntriesBeginToTransTable.end()) {
    DP("Removing translation table for descriptor " DPxMOD "\n",
        DPxPTR(desc->HostEntriesBegin));
    HostEntriesBeginToTransTable.erase(tt);
  } else {
    DP("Translation table for descriptor " DPxMOD " cannot be found, probably "
        "it has been already removed.\n", DPxPTR(desc->HostEntriesBegin));
  }

  TblMapMtx.unlock();

  // TODO: Remove RTL and the devices it manages if it's not used anymore?
  // TODO: Write some RTL->unload_image(...) function?

  DP("Done unregistering library!\n");
}
