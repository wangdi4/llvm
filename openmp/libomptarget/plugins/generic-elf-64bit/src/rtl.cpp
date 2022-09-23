//===-RTLs/generic-64bit/src/rtl.cpp - Target RTLs Implementation - C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// RTL for generic 64-bit machine
//
//===----------------------------------------------------------------------===//

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ffi.h>
#if INTEL_COLLAB
#include <limits>
#include <unordered_set>
#include <mutex>
#endif // INTEL_COLLAB
#include <link.h>
#include <list>
#include <string>
#include <vector>

#include "Debug.h"
#include "omptargetplugin.h"

#ifndef TARGET_NAME
#define TARGET_NAME Generic ELF - 64bit
#endif
#define DEBUG_PREFIX "TARGET " GETNAME(TARGET_NAME) " RTL"

#ifndef TARGET_ELF_ID
#define TARGET_ELF_ID 0
#endif

#include "elf_common.h"

#define NUMBER_OF_DEVICES 4
#define OFFLOAD_SECTION_NAME "omp_offloading_entries"

/// Array of Dynamic libraries loaded for this target.
struct DynLibTy {
  std::string FileName;
  void* Handle;
};

/// Keep entries table per device.
struct FuncOrGblEntryTy {
  __tgt_target_table Table;
};

/// Class containing all the device information.
class RTLDeviceInfoTy {
  std::vector<std::list<FuncOrGblEntryTy>> FuncGblEntries;

public:
  std::list<DynLibTy> DynLibs;
#if INTEL_COLLAB
  std::unordered_set<void *> DevicePtrs;
  std::mutex Mtx;
#endif // INTEL_COLLAB

  // Record entry point associated with device.
  void createOffloadTable(int32_t DeviceId, __tgt_offload_entry *Begin,
                          __tgt_offload_entry *End) {
    assert(DeviceId < (int32_t)FuncGblEntries.size() &&
           "Unexpected device id!");
    FuncGblEntries[DeviceId].emplace_back();
    FuncOrGblEntryTy &E = FuncGblEntries[DeviceId].back();

    E.Table.EntriesBegin = Begin;
    E.Table.EntriesEnd = End;
  }

  // Return true if the entry is associated with device.
  bool findOffloadEntry(int32_t DeviceId, void *Addr) {
    assert(DeviceId < (int32_t)FuncGblEntries.size() &&
           "Unexpected device id!");
    FuncOrGblEntryTy &E = FuncGblEntries[DeviceId].back();

    for (__tgt_offload_entry *I = E.Table.EntriesBegin,
                             *End = E.Table.EntriesEnd;
         I < End; ++I) {
      if (I->addr == Addr)
        return true;
    }

    return false;
  }

  // Return the pointer to the target entries table.
  __tgt_target_table *getOffloadEntriesTable(int32_t DeviceId) {
    assert(DeviceId < (int32_t)FuncGblEntries.size() &&
           "Unexpected device id!");
    FuncOrGblEntryTy &E = FuncGblEntries[DeviceId].back();

    return &E.Table;
  }

  RTLDeviceInfoTy(int32_t NumDevices) { FuncGblEntries.resize(NumDevices); }

  ~RTLDeviceInfoTy() {
    // Close dynamic libraries
    for (auto &Lib : DynLibs) {
      if (Lib.Handle) {
        dlclose(Lib.Handle);
        remove(Lib.FileName.c_str());
      }
    }
  }
};

static RTLDeviceInfoTy DeviceInfo(NUMBER_OF_DEVICES);

#ifdef __cplusplus
extern "C" {
#endif

#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
int32_t __tgt_rtl_is_valid_binary(__tgt_device_image *Image) {
// If we don't have a valid ELF ID we can just fail.
#if TARGET_ELF_ID < 1
  return 0;
#else
  return elf_check_machine(Image, TARGET_ELF_ID);
#endif
}

#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
int32_t __tgt_rtl_number_of_devices() { return NUMBER_OF_DEVICES; }

#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
int32_t __tgt_rtl_init_device(int32_t DeviceId) { return OFFLOAD_SUCCESS; }

#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
__tgt_target_table *__tgt_rtl_load_binary(int32_t DeviceId,
                                          __tgt_device_image *Image) {

  DP("Dev %d: load binary from " DPxMOD " image\n", DeviceId,
     DPxPTR(Image->ImageStart));

  assert(DeviceId >= 0 && DeviceId < NUMBER_OF_DEVICES && "bad dev id");

  size_t ImageSize = (size_t)Image->ImageEnd - (size_t)Image->ImageStart;

  // Is the library version incompatible with the header file?
  if (elf_version(EV_CURRENT) == EV_NONE) {
    DP("Incompatible ELF library!\n");
    return NULL;
  }

  // Obtain elf handler
  Elf *E = elf_memory((char *)Image->ImageStart, ImageSize);
  if (!E) {
    DP("Unable to get ELF handle: %s!\n", elf_errmsg(-1));
    return NULL;
  }

  if (elf_kind(E) != ELF_K_ELF) {
    DP("Invalid Elf kind!\n");
    elf_end(E);
    return NULL;
  }

  // Find the entries section offset
  Elf_Scn *Section = 0;
  Elf64_Off EntriesOffset = 0;

  size_t Shstrndx;

  if (elf_getshdrstrndx(E, &Shstrndx)) {
    DP("Unable to get ELF strings index!\n");
    elf_end(E);
    return NULL;
  }

  while ((Section = elf_nextscn(E, Section))) {
    GElf_Shdr Hdr;
    gelf_getshdr(Section, &Hdr);

    if (!strcmp(elf_strptr(E, Shstrndx, Hdr.sh_name), OFFLOADSECTIONNAME)) {
      EntriesOffset = Hdr.sh_addr;
      break;
    }
  }

  if (!EntriesOffset) {
    DP("Entries Section Offset Not Found\n");
    elf_end(E);
    return NULL;
  }

  DP("Offset of entries section is (" DPxMOD ").\n", DPxPTR(EntriesOffset));

  // load dynamic library and get the entry points. We use the dl library
  // to do the loading of the library, but we could do it directly to avoid the
  // dump to the temporary file.
  //
  // 1) Create tmp file with the library contents.
  // 2) Use dlopen to load the file and dlsym to retrieve the symbols.
  char TmpName[] = "/tmp/tmpfile_XXXXXX";
  int TmpFd = mkstemp(TmpName);

  if (TmpFd == -1) {
    elf_end(E);
    return NULL;
  }

  FILE *Ftmp = fdopen(TmpFd, "wb");

  if (!Ftmp) {
    elf_end(E);
    return NULL;
  }

  fwrite(Image->ImageStart, ImageSize, 1, Ftmp);
  fclose(Ftmp);

  DynLibTy Lib = {TmpName, dlopen(TmpName, RTLD_LAZY)};

  if (!Lib.Handle) {
    DP("Target library loading error: %s\n", dlerror());
    elf_end(E);
    return NULL;
  }

  DeviceInfo.DynLibs.push_back(Lib);

  struct link_map *LibInfo = (struct link_map *)Lib.Handle;

  // The place where the entries info is loaded is the library base address
  // plus the offset determined from the ELF file.
  Elf64_Addr EntriesAddr = LibInfo->l_addr + EntriesOffset;

  DP("Pointer to first entry to be loaded is (" DPxMOD ").\n",
     DPxPTR(EntriesAddr));

  // Table of pointers to all the entries in the target.
  __tgt_offload_entry *EntriesTable = (__tgt_offload_entry *)EntriesAddr;

  __tgt_offload_entry *EntriesBegin = &EntriesTable[0];
  __tgt_offload_entry *EntriesEnd = EntriesBegin + NumEntries;

  if (!EntriesBegin) {
    DP("Can't obtain entries begin\n");
    elf_end(E);
    return NULL;
  }

  DP("Entries table range is (" DPxMOD ")->(" DPxMOD ")\n",
     DPxPTR(EntriesBegin), DPxPTR(EntriesEnd));
  DeviceInfo.createOffloadTable(DeviceId, EntriesBegin, EntriesEnd);

  elf_end(E);

  return DeviceInfo.getOffloadEntriesTable(DeviceId);
}

void __tgt_rtl_print_device_info(int32_t DeviceId) {
  printf("    This is a generic-elf-64bit device\n");
}

#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
// Sample implementation of explicit memory allocator. For this plugin all kinds
// are equivalent to each other.
void *__tgt_rtl_data_alloc(int32_t DeviceId, int64_t Size, void *HstPtr,
                           int32_t Kind) {
  void *Ptr = NULL;

  switch (Kind) {
  case TARGET_ALLOC_DEVICE:
  case TARGET_ALLOC_HOST:
  case TARGET_ALLOC_SHARED:
  case TARGET_ALLOC_DEFAULT:
    Ptr = malloc(Size);
    break;
  default:
    REPORT("Invalid target data allocation kind");
  }
#if INTEL_COLLAB
  if (Ptr) {
    std::lock_guard<std::mutex> Lock(DeviceInfo.Mtx);
    DeviceInfo.DevicePtrs.insert(Ptr);
  }
#endif // INTEL_COLLAB

  return Ptr;
}

#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
int32_t __tgt_rtl_data_submit(int32_t DeviceId, void *TgtPtr, void *HstPtr,
                              int64_t Size) {
  memcpy(TgtPtr, HstPtr, Size);
  return OFFLOAD_SUCCESS;
}

#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
int32_t __tgt_rtl_data_retrieve(int32_t DeviceId, void *HstPtr, void *TgtPtr,
                                int64_t Size) {
  memcpy(HstPtr, TgtPtr, Size);
  return OFFLOAD_SUCCESS;
}

#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
int32_t __tgt_rtl_data_delete(int32_t DeviceId, void *TgtPtr, int32_t) {
  free(TgtPtr);

#if INTEL_COLLAB
  std::lock_guard<std::mutex> Lock(DeviceInfo.Mtx);
  DeviceInfo.DevicePtrs.erase(TgtPtr);
#endif // INTEL_COLLAB

  return OFFLOAD_SUCCESS;
}

#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
int32_t __tgt_rtl_run_target_team_region(int32_t DeviceId, void *TgtEntryPtr,
                                         void **TgtArgs, ptrdiff_t *TgtOffsets,
                                         int32_t ArgNum, int32_t TeamNum,
                                         int32_t ThreadLimit,
                                         uint64_t LoopTripcount /*not used*/) {
  // ignore team num and thread limit.

  // Use libffi to launch execution.
  ffi_cif Cif;

  // All args are references.
  std::vector<ffi_type *> ArgsTypes(ArgNum, &ffi_type_pointer);
  std::vector<void *> Args(ArgNum);
  std::vector<void *> Ptrs(ArgNum);

  for (int32_t I = 0; I < ArgNum; ++I) {

#if INTEL_COLLAB
    ptrdiff_t offset = TgtOffsets[I];
    // Offset equal to MAX(ptrdiff_t) means that the argument
    // must be passed as literal, and the offset should be ignored.
    if (offset == (std::numeric_limits<ptrdiff_t>::max)())
      Ptrs[I] = TgtArgs[I];
    else
      Ptrs[I] = (void *)((intptr_t)TgtArgs[I] + offset);
#else // INTEL_COLLAB
    Ptrs[I] = (void *)((intptr_t)TgtArgs[I] + TgtOffsets[I]);
#endif // INTEL_COLLAB

    Args[I] = &Ptrs[I];
  }

#if INTEL_COLLAB
  ffi_status Status = ffi_prep_cif(&Cif, FFI_DEFAULT_ABI, ArgNum,
                                   &ffi_type_void, ArgsTypes.data());
#else // INTEL_COLLAB
  ffi_status Status = ffi_prep_cif(&Cif, FFI_DEFAULT_ABI, ArgNum,
                                   &ffi_type_void, &ArgsTypes[0]);
#endif // INTEL_COLLAB

  assert(Status == FFI_OK && "Unable to prepare target launch!");

  if (Status != FFI_OK)
    return OFFLOAD_FAIL;

  DP("Running entry point at " DPxMOD "...\n", DPxPTR(TgtEntryPtr));

  void (*Entry)(void);
  *((void **)&Entry) = TgtEntryPtr;

#if INTEL_COLLAB
  ffi_call(&Cif, Entry, NULL, Args.data());
#else // INTEL_COLLAB
  ffi_call(&Cif, Entry, NULL, &Args[0]);
#endif // INTEL_COLLAB

  return OFFLOAD_SUCCESS;
}

#if INTEL_COLLAB
EXTERN
#endif  // INTEL_COLLAB
int32_t __tgt_rtl_run_target_region(int32_t DeviceId, void *TgtEntryPtr,
                                    void **TgtArgs, ptrdiff_t *TgtOffsets,
                                    int32_t ArgNum) {
  // use one team and one thread.
  return __tgt_rtl_run_target_team_region(DeviceId, TgtEntryPtr, TgtArgs,
                                          TgtOffsets, ArgNum, 1, 1, 0);
}

#if INTEL_COLLAB
int32_t __tgt_rtl_requires_mapping(int32_t DeviceId, void *Ptr, int64_t Size) {
  int32_t Ret;
  std::lock_guard<std::mutex> Lock(DeviceInfo.Mtx);
  if (DeviceInfo.DevicePtrs.count(Ptr) > 0)
    Ret = 0;
  else
    Ret = 1;
  DP("Ptr " DPxMOD " %s mapping.\n", DPxPTR(Ptr),
     Ret ? "requires" : "does not require");
  return Ret;
}

int32_t __tgt_rtl_set_function_ptr_map(
    int32_t ID, uint64_t Size, const __omp_offloading_fptr_map_t *FnPtrs) {
  return OFFLOAD_SUCCESS;
}
#endif // INTEL_COLLAB
#ifdef __cplusplus
}
#endif
