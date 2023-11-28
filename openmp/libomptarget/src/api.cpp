//===----------- api.cpp - Target independent OpenMP target RTL -----------===//
/* INTEL_CUSTOMIZATION */
/*
 * INTEL CONFIDENTIAL
 *
 * Modifications, Copyright (C) 2022 Intel Corporation
 *
 * This software and the related documents are Intel copyrighted materials, and
 * your use of them is governed by the express license under which they were
 * provided to you ("License"). Unless the License provides otherwise, you may not
 * use, modify, copy, publish, distribute, disclose or transmit this software or
 * the related documents without Intel's prior written permission.
 *
 * This software and the related documents are provided as is, with no express
 * or implied warranties, other than those that are expressly stated in the
 * License.
 */
/* end INTEL_CUSTOMIZATION */
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Implementation of OpenMP API interface functions.
//
//===----------------------------------------------------------------------===//

#include "device.h"
#include "omptarget.h"
#include "private.h"
#include "rtl.h"

#include "llvm/ADT/SmallVector.h"

#include <climits>
#include <cstdlib>
#include <cstring>
#include <mutex>

#if INTEL_CUSTOMIZATION
/// API functions that can access host-to-target data map need to load device
/// image to get correct mapping information for global data. This macro is
/// called in such functions.
#define CHECK_DEVICE_AND_CTORS_RET(ID, RetVal)                                 \
  do {                                                                         \
    int64_t DeviceID = ID;                                                     \
    if (checkDeviceAndCtors(DeviceID, nullptr) != OFFLOAD_SUCCESS)             \
      return RetVal;                                                           \
  } while(0)
#endif // INTEL_CUSTOMIZATION

EXTERN int omp_get_num_devices(void) {
  TIMESCOPE();
  PM->RTLsMtx.lock();
  size_t DevicesSize = PM->Devices.size();
  PM->RTLsMtx.unlock();

  DP("Call to omp_get_num_devices returning %zd\n", DevicesSize);

  return DevicesSize;
}

EXTERN int omp_get_device_num(void) {
  TIMESCOPE();
  int HostDevice = omp_get_initial_device();

  DP("Call to omp_get_device_num returning %d\n", HostDevice);

  return HostDevice;
}

EXTERN int omp_get_initial_device(void) {
  TIMESCOPE();
  int HostDevice = omp_get_num_devices();
  DP("Call to omp_get_initial_device returning %d\n", HostDevice);
  return HostDevice;
}

EXTERN void *omp_target_alloc(size_t Size, int DeviceNum) {
  return targetAllocExplicit(Size, DeviceNum, TARGET_ALLOC_DEFAULT, __func__);
}

EXTERN void *llvm_omp_target_alloc_device(size_t Size, int DeviceNum) {
  return targetAllocExplicit(Size, DeviceNum, TARGET_ALLOC_DEVICE, __func__);
}

EXTERN void *llvm_omp_target_alloc_host(size_t Size, int DeviceNum) {
  return targetAllocExplicit(Size, DeviceNum, TARGET_ALLOC_HOST, __func__);
}

EXTERN void *llvm_omp_target_alloc_shared(size_t Size, int DeviceNum) {
  return targetAllocExplicit(Size, DeviceNum, TARGET_ALLOC_SHARED, __func__);
}

EXTERN void omp_target_free(void *Ptr, int DeviceNum) {
  return targetFreeExplicit(Ptr, DeviceNum, TARGET_ALLOC_DEFAULT, __func__);
}

EXTERN void llvm_omp_target_free_device(void *Ptr, int DeviceNum) {
  return targetFreeExplicit(Ptr, DeviceNum, TARGET_ALLOC_DEVICE, __func__);
}

EXTERN void llvm_omp_target_free_host(void *Ptr, int DeviceNum) {
  return targetFreeExplicit(Ptr, DeviceNum, TARGET_ALLOC_HOST, __func__);
}

EXTERN void llvm_omp_target_free_shared(void *Ptre, int DeviceNum) {
  return targetFreeExplicit(Ptre, DeviceNum, TARGET_ALLOC_SHARED, __func__);
}

EXTERN void *llvm_omp_target_dynamic_shared_alloc() { return nullptr; }
EXTERN void *llvm_omp_get_dynamic_shared() { return nullptr; }

EXTERN [[nodiscard]] void *llvm_omp_target_lock_mem(void *Ptr, size_t Size,
                                                    int DeviceNum) {
  return targetLockExplicit(Ptr, Size, DeviceNum, __func__);
}

EXTERN void llvm_omp_target_unlock_mem(void *Ptr, int DeviceNum) {
  targetUnlockExplicit(Ptr, DeviceNum, __func__);
}

EXTERN int omp_target_is_present(const void *Ptr, int DeviceNum) {
  TIMESCOPE();
  DP("Call to omp_target_is_present for device %d and address " DPxMOD "\n",
     DeviceNum, DPxPTR(Ptr));

  if (!Ptr) {
    DP("Call to omp_target_is_present with NULL ptr, returning false\n");
    return false;
  }

  if (DeviceNum == omp_get_initial_device()) {
    DP("Call to omp_target_is_present on host, returning true\n");
    return true;
  }

  PM->RTLsMtx.lock();
  size_t DevicesSize = PM->Devices.size();
  PM->RTLsMtx.unlock();
  if (DevicesSize <= (size_t)DeviceNum) {
    DP("Call to omp_target_is_present with invalid device ID, returning "
       "false\n");
    return false;
  }
#if INTEL_CUSTOMIZATION
  CHECK_DEVICE_AND_CTORS_RET(DeviceNum, false);
#endif // INTEL_CUSTOMIZATION

  DeviceTy &Device = *PM->Devices[DeviceNum];
  // omp_target_is_present tests whether a host pointer refers to storage that
  // is mapped to a given device. However, due to the lack of the storage size,
  // only check 1 byte. Cannot set size 0 which checks whether the pointer (zero
  // lengh array) is mapped instead of the referred storage.
  TargetPointerResultTy TPR = Device.getTgtPtrBegin(const_cast<void *>(Ptr), 1,
                                                    /*UpdateRefCount=*/false,
                                                    /*UseHoldRefCount=*/false);
  int Rc = TPR.isPresent();
  DP("Call to omp_target_is_present returns %d\n", Rc);
  return Rc;
}

EXTERN int omp_target_memcpy(void *Dst, const void *Src, size_t Length,
                             size_t DstOffset, size_t SrcOffset, int DstDevice,
                             int SrcDevice) {
  TIMESCOPE();
  DP("Call to omp_target_memcpy, dst device %d, src device %d, "
     "dst addr " DPxMOD ", src addr " DPxMOD ", dst offset %zu, "
     "src offset %zu, length %zu\n",
     DstDevice, SrcDevice, DPxPTR(Dst), DPxPTR(Src), DstOffset, SrcOffset,
     Length);

  if (!Dst || !Src || Length <= 0) {
    if (Length == 0) {
      DP("Call to omp_target_memcpy with zero length, nothing to do\n");
      return OFFLOAD_SUCCESS;
    }

    REPORT("Call to omp_target_memcpy with invalid arguments\n");
    return OFFLOAD_FAIL;
  }

  if (SrcDevice != omp_get_initial_device() && !deviceIsReady(SrcDevice)) {
    REPORT("omp_target_memcpy returns OFFLOAD_FAIL\n");
    return OFFLOAD_FAIL;
  }

  if (DstDevice != omp_get_initial_device() && !deviceIsReady(DstDevice)) {
    REPORT("omp_target_memcpy returns OFFLOAD_FAIL\n");
    return OFFLOAD_FAIL;
  }

  int Rc = OFFLOAD_SUCCESS;
  void *SrcAddr = (char *)const_cast<void *>(Src) + SrcOffset;
  void *DstAddr = (char *)Dst + DstOffset;

  if (SrcDevice == omp_get_initial_device() &&
      DstDevice == omp_get_initial_device()) {
    DP("copy from host to host\n");
    const void *P = memcpy(DstAddr, SrcAddr, Length);
    if (P == NULL)
      Rc = OFFLOAD_FAIL;
  } else if (SrcDevice == omp_get_initial_device()) {
    DP("copy from host to device\n");
    DeviceTy &DstDev = *PM->Devices[DstDevice];
    AsyncInfoTy AsyncInfo(DstDev);
    Rc = DstDev.submitData(DstAddr, SrcAddr, Length, AsyncInfo);
  } else if (DstDevice == omp_get_initial_device()) {
    DP("copy from device to host\n");
    DeviceTy &SrcDev = *PM->Devices[SrcDevice];
    AsyncInfoTy AsyncInfo(SrcDev);
    Rc = SrcDev.retrieveData(DstAddr, SrcAddr, Length, AsyncInfo);
  } else {
    DP("copy from device to device\n");
    DeviceTy &SrcDev = *PM->Devices[SrcDevice];
    DeviceTy &DstDev = *PM->Devices[DstDevice];
    // First try to use D2D memcpy which is more efficient. If fails, fall back
    // to unefficient way.
    if (SrcDev.isDataExchangable(DstDev)) {
      AsyncInfoTy AsyncInfo(SrcDev);
      Rc = SrcDev.dataExchange(SrcAddr, DstDev, DstAddr, Length, AsyncInfo);
      if (Rc == OFFLOAD_SUCCESS)
        return OFFLOAD_SUCCESS;
    }

    void *Buffer = malloc(Length);
    {
      AsyncInfoTy AsyncInfo(SrcDev);
      Rc = SrcDev.retrieveData(Buffer, SrcAddr, Length, AsyncInfo);
    }
    if (Rc == OFFLOAD_SUCCESS) {
      AsyncInfoTy AsyncInfo(DstDev);
      Rc = DstDev.submitData(DstAddr, Buffer, Length, AsyncInfo);
    }
    free(Buffer);
  }

  DP("omp_target_memcpy returns %d\n", Rc);
  return Rc;
}

#if INTEL_CUSTOMIZATION
/// Use optimized copy methods if device supports.
static int memcpyRect3D(void *Dst, const void *Src, size_t ElementSize,
                        int NumDims, const size_t *Volume,
                        const size_t *DstOffsets, const size_t *SrcOffsets,
                        const size_t *DstDims, const size_t *SrcDims,
                        int DstDevice, int SrcDevice) {
  // Only 2D or 3D volume is expected here
  if (NumDims != 2 && NumDims != 3)
    return OFFLOAD_FAIL;
  if (!Dst || !Src || ElementSize == 0 || !Volume || !DstOffsets ||
      !SrcOffsets || !DstDims || !SrcDims)
    return OFFLOAD_FAIL;

  int InitDevice = omp_get_initial_device();
  int DeviceNum = InitDevice;
  if (DstDevice != InitDevice) {
    DeviceNum = DstDevice;
  } else if (SrcDevice != InitDevice) {
    DeviceNum = SrcDevice;
  } else {
    return OFFLOAD_FAIL; // Let omp_target_memcpy_rect() handle this case
  }

  if (!deviceIsReady(DeviceNum)) {
    REPORT("Device %" PRId32 " is not ready.\n", DeviceNum);
    return OFFLOAD_FAIL;
  }

  DeviceTy &Device = *PM->Devices[DeviceNum];

  return Device.memcpyRect3D(Dst, Src, ElementSize, NumDims, Volume, DstOffsets,
                             SrcOffsets, DstDims, SrcDims);
}
#endif // INTEL_CUSTOMIZATION

// The helper function that calls omp_target_memcpy or omp_target_memcpy_rect
static int libomp_target_memcpy_async_task(kmp_int32 Gtid, kmp_task_t *Task) {
  if (Task == nullptr)
    return OFFLOAD_FAIL;

  TargetMemcpyArgsTy *Args = (TargetMemcpyArgsTy *)Task->shareds;

  if (Args == nullptr)
    return OFFLOAD_FAIL;

  // Call blocked version
  int Rc = OFFLOAD_SUCCESS;
  if (Args->IsRectMemcpy) {
#if INTEL_CUSTOMIZATION
    Rc = omp_target_memcpy_rect(
        Args->Dst, Args->Src, Args->ElementSize, Args->NumDims,
        Args->getVolume(), Args->getDstOffsets(), Args->getSrcOffsets(),
        Args->getDstDimensions(), Args->getSrcDimensions(), Args->DstDevice,
        Args->SrcDevice);
#else  // INTEL_CUSTOMIZATION
    Rc = omp_target_memcpy_rect(
        Args->Dst, Args->Src, Args->ElementSize, Args->NumDims, Args->Volume,
        Args->DstOffsets, Args->SrcOffsets, Args->DstDimensions,
        Args->SrcDimensions, Args->DstDevice, Args->SrcDevice);
#endif // INTEL_CUSTOMIZATION
    DP("omp_target_memcpy_rect returns %d\n", Rc);
  } else {
    Rc = omp_target_memcpy(Args->Dst, Args->Src, Args->Length, Args->DstOffset,
                           Args->SrcOffset, Args->DstDevice, Args->SrcDevice);

    DP("omp_target_memcpy returns %d\n", Rc);
  }

  // Release the arguments object
  delete Args;

  return Rc;
}

static int libomp_target_memset_async_task(kmp_int32 Gtid, kmp_task_t *Task) {
  if (!Task)
    return OFFLOAD_FAIL;

  auto *Args = reinterpret_cast<TargetMemsetArgsTy *>(Task->shareds);
  if (!Args)
    return OFFLOAD_FAIL;

  // call omp_target_memset()
  omp_target_memset(Args->Ptr, Args->C, Args->N, Args->DeviceNum);

  delete Args;

  return OFFLOAD_SUCCESS;
}

static inline void
convertDepObjVector(llvm::SmallVector<kmp_depend_info_t> &Vec, int DepObjCount,
                    omp_depend_t *DepObjList) {
  for (int i = 0; i < DepObjCount; ++i) {
    omp_depend_t DepObj = DepObjList[i];
    Vec.push_back(*((kmp_depend_info_t *)DepObj));
  }
}

template <class T>
static inline int
libomp_helper_task_creation(T *Args, int (*Fn)(kmp_int32, kmp_task_t *),
                            int DepObjCount, omp_depend_t *DepObjList) {
  // Create global thread ID
  int Gtid = __kmpc_global_thread_num(nullptr);

  // Setup the hidden helper flags
  kmp_int32 Flags = 0;
#if INTEL_CUSTOMIZATION
// This flag is set by __kmpc_omp_target_task_alloc when helper task is
// supported.
#else  // INTEL_CUSTOMIZATION
  kmp_tasking_flags_t *InputFlags = (kmp_tasking_flags_t *)&Flags;
  InputFlags->hidden_helper = 1;
#endif // INTEL_CUSTOMIZATION

  // Alloc the helper task
  kmp_task_t *Task = __kmpc_omp_target_task_alloc(
      nullptr, Gtid, Flags, sizeof(kmp_task_t), 0, Fn, -1);
  if (!Task) {
    delete Args;
    return OFFLOAD_FAIL;
  }

  // Setup the arguments for the helper task
  Task->shareds = Args;

  // Convert types of depend objects
  llvm::SmallVector<kmp_depend_info_t> DepObjs;
  convertDepObjVector(DepObjs, DepObjCount, DepObjList);

  // Launch the helper task
  int Rc = __kmpc_omp_task_with_deps(nullptr, Gtid, Task, DepObjCount,
                                     DepObjs.data(), 0, nullptr);

  return Rc;
}

EXTERN void *omp_target_memset(void *Ptr, int ByteVal, size_t NumBytes,
                               int DeviceNum) {
  TIMESCOPE();
  DP("Call to omp_target_memset, device %d, device pointer %p, size %zu\n",
     DeviceNum, Ptr, NumBytes);

  // Behave as a no-op if N==0 or if Ptr is nullptr (as a useful implementation
  // of unspecified behavior, see OpenMP spec).
  if (!Ptr || NumBytes == 0) {
    return Ptr;
  }

  if (DeviceNum == omp_get_initial_device()) {
    DP("filling memory on host via memset");
    memset(Ptr, ByteVal, NumBytes); // ignore return value, memset() cannot fail
  } else {
    // TODO: replace the omp_target_memset() slow path with the fast path.
    // That will require the ability to execute a kernel from within
    // libomptarget.so (which we do not have at the moment).

    // This is a very slow path: create a filled array on the host and upload
    // it to the GPU device.
    int InitialDevice = omp_get_initial_device();
    void *Shadow = omp_target_alloc(NumBytes, InitialDevice);
    if (Shadow) {
      (void)memset(Shadow, ByteVal, NumBytes);
      (void)omp_target_memcpy(Ptr, Shadow, NumBytes, 0, 0, DeviceNum,
                              InitialDevice);
      (void)omp_target_free(Shadow, InitialDevice);
    } else {
      // If the omp_target_alloc has failed, let's just not do anything.
      // omp_target_memset does not have any good way to fail, so we
      // simply avoid a catastrophic failure of the process for now.
      DP("omp_target_memset failed to fill memory due to error with "
         "omp_target_alloc");
    }
  }

  DP("omp_target_memset returns %p\n", Ptr);
  return Ptr;
}

EXTERN void *omp_target_memset_async(void *Ptr, int ByteVal, size_t NumBytes,
                                     int DeviceNum, int DepObjCount,
                                     omp_depend_t *DepObjList) {
  DP("Call to omp_target_memset_async, device %d, device pointer %p, size %zu",
     DeviceNum, Ptr, NumBytes);

  // Behave as a no-op if N==0 or if Ptr is nullptr (as a useful implementation
  // of unspecified behavior, see OpenMP spec).
  if (!Ptr || NumBytes == 0)
    return Ptr;

  // Create the task object to deal with the async invocation
  auto *Args = new TargetMemsetArgsTy{Ptr, ByteVal, NumBytes, DeviceNum};

  // omp_target_memset_async() cannot fail via a return code, so ignore the
  // return code of the helper function
  (void)libomp_helper_task_creation(Args, &libomp_target_memset_async_task,
                                    DepObjCount, DepObjList);

  return Ptr;
}

EXTERN int omp_target_memcpy_async(void *Dst, const void *Src, size_t Length,
                                   size_t DstOffset, size_t SrcOffset,
                                   int DstDevice, int SrcDevice,
                                   int DepObjCount, omp_depend_t *DepObjList) {
  TIMESCOPE();
  DP("Call to omp_target_memcpy_async, dst device %d, src device %d, "
     "dst addr " DPxMOD ", src addr " DPxMOD ", dst offset %zu, "
     "src offset %zu, length %zu\n",
     DstDevice, SrcDevice, DPxPTR(Dst), DPxPTR(Src), DstOffset, SrcOffset,
     Length);

  // Check the source and dest address
  if (Dst == nullptr || Src == nullptr)
    return OFFLOAD_FAIL;

  // Create task object
  TargetMemcpyArgsTy *Args = new TargetMemcpyArgsTy(
      Dst, Src, Length, DstOffset, SrcOffset, DstDevice, SrcDevice);

  // Create and launch helper task
  int Rc = libomp_helper_task_creation(Args, &libomp_target_memcpy_async_task,
                                       DepObjCount, DepObjList);

  DP("omp_target_memcpy_async returns %d\n", Rc);
  return Rc;
}

EXTERN int
omp_target_memcpy_rect(void *Dst, const void *Src, size_t ElementSize,
                       int NumDims, const size_t *Volume,
                       const size_t *DstOffsets, const size_t *SrcOffsets,
                       const size_t *DstDimensions, const size_t *SrcDimensions,
                       int DstDevice, int SrcDevice) {
  TIMESCOPE();
  DP("Call to omp_target_memcpy_rect, dst device %d, src device %d, "
     "dst addr " DPxMOD ", src addr " DPxMOD ", dst offsets " DPxMOD ", "
     "src offsets " DPxMOD ", dst dims " DPxMOD ", src dims " DPxMOD ", "
     "volume " DPxMOD ", element size %zu, num_dims %d\n",
     DstDevice, SrcDevice, DPxPTR(Dst), DPxPTR(Src), DPxPTR(DstOffsets),
     DPxPTR(SrcOffsets), DPxPTR(DstDimensions), DPxPTR(SrcDimensions),
     DPxPTR(Volume), ElementSize, NumDims);

  if (!(Dst || Src)) {
    DP("Call to omp_target_memcpy_rect returns max supported dimensions %d\n",
       INT_MAX);
    return INT_MAX;
  }

  if (!Dst || !Src || ElementSize < 1 || NumDims < 1 || !Volume ||
      !DstOffsets || !SrcOffsets || !DstDimensions || !SrcDimensions) {
    REPORT("Call to omp_target_memcpy_rect with invalid arguments\n");
    return OFFLOAD_FAIL;
  }

#if INTEL_CUSTOMIZATION
  int Rc = OFFLOAD_SUCCESS;
#else  // INTEL_CUSTOMIZATION
  int Rc;
#endif // INTEL_CUSTOMIZATION
  if (NumDims == 1) {
    Rc = omp_target_memcpy(Dst, Src, ElementSize * Volume[0],
                           ElementSize * DstOffsets[0],
                           ElementSize * SrcOffsets[0], DstDevice, SrcDevice);
  } else {
#if INTEL_CUSTOMIZATION
    if (NumDims <= 3) {
      Rc = memcpyRect3D(Dst, Src, ElementSize, NumDims, Volume, DstOffsets,
                        SrcOffsets, DstDimensions, SrcDimensions, DstDevice,
                        SrcDevice);
      if (Rc == OFFLOAD_SUCCESS)
        return Rc;
    }
#endif // INTEL_CUSTOMIZATION
    size_t DstSliceSize = ElementSize;
    size_t SrcSliceSize = ElementSize;
    for (int I = 1; I < NumDims; ++I) {
      DstSliceSize *= DstDimensions[I];
      SrcSliceSize *= SrcDimensions[I];
    }

    size_t DstOff = DstOffsets[0] * DstSliceSize;
    size_t SrcOff = SrcOffsets[0] * SrcSliceSize;
    for (size_t I = 0; I < Volume[0]; ++I) {
      Rc = omp_target_memcpy_rect(
          (char *)Dst + DstOff + DstSliceSize * I,
          (char *)const_cast<void *>(Src) + SrcOff + SrcSliceSize * I,
          ElementSize, NumDims - 1, Volume + 1, DstOffsets + 1, SrcOffsets + 1,
          DstDimensions + 1, SrcDimensions + 1, DstDevice, SrcDevice);

      if (Rc) {
        DP("Recursive call to omp_target_memcpy_rect returns unsuccessfully\n");
        return Rc;
      }
    }
  }

  DP("omp_target_memcpy_rect returns %d\n", Rc);
  return Rc;
}

EXTERN int omp_target_memcpy_rect_async(
    void *Dst, const void *Src, size_t ElementSize, int NumDims,
    const size_t *Volume, const size_t *DstOffsets, const size_t *SrcOffsets,
    const size_t *DstDimensions, const size_t *SrcDimensions, int DstDevice,
    int SrcDevice, int DepObjCount, omp_depend_t *DepObjList) {
  TIMESCOPE();
  DP("Call to omp_target_memcpy_rect_async, dst device %d, src device %d, "
     "dst addr " DPxMOD ", src addr " DPxMOD ", dst offsets " DPxMOD ", "
     "src offsets " DPxMOD ", dst dims " DPxMOD ", src dims " DPxMOD ", "
     "volume " DPxMOD ", element size %zu, num_dims %d\n",
     DstDevice, SrcDevice, DPxPTR(Dst), DPxPTR(Src), DPxPTR(DstOffsets),
     DPxPTR(SrcOffsets), DPxPTR(DstDimensions), DPxPTR(SrcDimensions),
     DPxPTR(Volume), ElementSize, NumDims);

  // Need to check this first to not return OFFLOAD_FAIL instead
  if (!Dst && !Src) {
    DP("Call to omp_target_memcpy_rect returns max supported dimensions %d\n",
       INT_MAX);
    return INT_MAX;
  }

#if INTEL_CUSTOMIZATION
  // Use the same check as in synchronous version to fail fast.
  if (!Dst || !Src || ElementSize < 1 || NumDims < 1 || !Volume ||
      !DstOffsets || !SrcOffsets || !DstDimensions || !SrcDimensions) {
    REPORT("Call to omp_target_memcpy_rect_async with invalid arguments\n");
    return OFFLOAD_FAIL;
  }
#else  // INTEL_CUSTOMIZATION
  // Check the source and dest address
  if (Dst == nullptr || Src == nullptr)
    return OFFLOAD_FAIL;
#endif // INTEL_CUSTOMIZATION

  // Create task object
  TargetMemcpyArgsTy *Args = new TargetMemcpyArgsTy(
      Dst, Src, ElementSize, NumDims, Volume, DstOffsets, SrcOffsets,
      DstDimensions, SrcDimensions, DstDevice, SrcDevice);

  // Create and launch helper task
  int Rc = libomp_helper_task_creation(Args, &libomp_target_memcpy_async_task,
                                       DepObjCount, DepObjList);

  DP("omp_target_memcpy_rect_async returns %d\n", Rc);
  return Rc;
}

EXTERN int omp_target_associate_ptr(const void *HostPtr, const void *DevicePtr,
                                    size_t Size, size_t DeviceOffset,
                                    int DeviceNum) {
  TIMESCOPE();
  DP("Call to omp_target_associate_ptr with host_ptr " DPxMOD ", "
     "device_ptr " DPxMOD ", size %zu, device_offset %zu, device_num %d\n",
     DPxPTR(HostPtr), DPxPTR(DevicePtr), Size, DeviceOffset, DeviceNum);

  if (!HostPtr || !DevicePtr || Size <= 0) {
    REPORT("Call to omp_target_associate_ptr with invalid arguments\n");
    return OFFLOAD_FAIL;
  }

  if (DeviceNum == omp_get_initial_device()) {
    REPORT("omp_target_associate_ptr: no association possible on the host\n");
    return OFFLOAD_FAIL;
  }

  if (!deviceIsReady(DeviceNum)) {
    REPORT("omp_target_associate_ptr returns OFFLOAD_FAIL\n");
    return OFFLOAD_FAIL;
  }
#if INTEL_CUSTOMIZATION
  CHECK_DEVICE_AND_CTORS_RET(DeviceNum, OFFLOAD_FAIL);
#endif // INTEL_CUSTOMIZATION

  DeviceTy &Device = *PM->Devices[DeviceNum];
  void *DeviceAddr = (void *)((uint64_t)DevicePtr + (uint64_t)DeviceOffset);
  int Rc = Device.associatePtr(const_cast<void *>(HostPtr),
                               const_cast<void *>(DeviceAddr), Size);
  DP("omp_target_associate_ptr returns %d\n", Rc);
  return Rc;
}

EXTERN int omp_target_disassociate_ptr(const void *HostPtr, int DeviceNum) {
  TIMESCOPE();
  DP("Call to omp_target_disassociate_ptr with host_ptr " DPxMOD ", "
     "device_num %d\n",
     DPxPTR(HostPtr), DeviceNum);

  if (!HostPtr) {
    REPORT("Call to omp_target_associate_ptr with invalid host_ptr\n");
    return OFFLOAD_FAIL;
  }

  if (DeviceNum == omp_get_initial_device()) {
    REPORT(
        "omp_target_disassociate_ptr: no association possible on the host\n");
    return OFFLOAD_FAIL;
  }

  if (!deviceIsReady(DeviceNum)) {
    REPORT("omp_target_disassociate_ptr returns OFFLOAD_FAIL\n");
    return OFFLOAD_FAIL;
  }
#if INTEL_CUSTOMIZATION
  CHECK_DEVICE_AND_CTORS_RET(DeviceNum, OFFLOAD_FAIL);
#endif // INTEL_CUSTOMIZATION

  DeviceTy &Device = *PM->Devices[DeviceNum];
  int Rc = Device.disassociatePtr(const_cast<void *>(HostPtr));
  DP("omp_target_disassociate_ptr returns %d\n", Rc);
  return Rc;
}

#if INTEL_CUSTOMIZATION
EXTERN int omp_target_is_accessible(const void *Ptr, size_t Size,
                                    int DeviceNum) {
  TIMESCOPE();
  DP("Call to omp_target_is_accessible with ptr " DPxMOD ", size %zu, "
     "device number %" PRId32 "\n", DPxPTR(Ptr), Size, DeviceNum);

  if (!Ptr) {
    DP("Call to omp_target_is_accessible with invalid pointer returns 0\n");
    return 0;
  }

  if (Size == 0) {
    DP("Call to omp_target_is_accessible with size 0 returns 0\n");
    return 0;
  }

  if (DeviceNum == omp_get_initial_device()) {
    DP("Call to omp_target_is_accessible with initial device returns 1\n");
    return 1;
  }

  if (!deviceIsReady(DeviceNum)) {
    REPORT("omp_target_is_accessible returns 0 due to device failure\n");
    return 0;
  }

  DeviceTy &Device = *PM->Devices[DeviceNum];
  int Ret = Device.isAccessibleAddrRange(Ptr, Size);
  DP("omp_target_is_accessible returns %" PRId32 "\n", Ret);
  return Ret;
}

static int32_t checkInteropCall(const omp_interop_t Interop,
                                const char *FnName) {
  if (!Interop) {
    DP("Call to %s with invalid interop\n", FnName);
    return omp_irc_empty;
  }

  int64_t DeviceNum = static_cast<__tgt_interop *>(Interop)->DeviceNum;

  if (!deviceIsReady(DeviceNum)) {
    DP("Device %" PRId64 " is not ready in %s\n", DeviceNum, FnName);
    return omp_irc_other;
  }

  return omp_irc_success;
}

EXTERN int omp_get_num_interop_properties(const omp_interop_t Interop) {
  DP("Call to %s with interop " DPxMOD "\n", __func__, DPxPTR(Interop));

  int32_t Rc = checkInteropCall(Interop, __func__);
  if (Rc != omp_irc_success)
    return 0;

  int64_t DeviceNum = static_cast<__tgt_interop *>(Interop)->DeviceNum;
  DeviceTy &Device = *PM->Devices[DeviceNum];
  return Device.getNumInteropProperties();
}

/// Return interop property value for the given value type
static int32_t getInteropValue(const omp_interop_t Interop, int32_t Ipr,
                               int32_t ValueType, size_t Size, void *Value) {

  __tgt_interop *TgtInterop = static_cast<__tgt_interop *>(Interop);
  int32_t Rc = omp_irc_success;

  switch (Ipr) {
  case omp_ipr_fr_id:
  case omp_ipr_vendor:
  case omp_ipr_device_num:
    if (ValueType != OMP_IPR_VALUE_INT)
      Rc = omp_irc_type_int;
    else if (Ipr == omp_ipr_fr_id)
      *static_cast<intptr_t *>(Value) = TgtInterop->FrId;
    else if (Ipr == omp_ipr_vendor)
      *static_cast<intptr_t *>(Value) = TgtInterop->Vendor;
    else
      *static_cast<intptr_t *>(Value) = TgtInterop->DeviceNum;
    break;

  case omp_ipr_fr_name:
  case omp_ipr_vendor_name:
    if (ValueType != OMP_IPR_VALUE_STR)
      Rc = omp_irc_type_str;
    else if (Ipr == omp_ipr_fr_name)
      *static_cast<const char **>(Value) = TgtInterop->FrName;
    else
      *static_cast<const char **>(Value) = TgtInterop->VendorName;
    break;

  case omp_ipr_platform:
  case omp_ipr_device:
  case omp_ipr_device_context:
  case omp_ipr_targetsync:
    if (ValueType != OMP_IPR_VALUE_PTR)
      Rc = omp_irc_type_ptr;
    else if (Ipr == omp_ipr_platform)
      *static_cast<void **>(Value) = TgtInterop->Platform;
    else if (Ipr == omp_ipr_device)
      *static_cast<void **>(Value) = TgtInterop->Device;
    else if (Ipr == omp_ipr_device_context)
      *static_cast<void **>(Value) = TgtInterop->DeviceContext;
    else
      *static_cast<void **>(Value) = TgtInterop->TargetSync;
    break;

  default: {
    // Get implementation-defined property value
    DeviceTy &Device = *PM->Devices[TgtInterop->DeviceNum];
    Rc = Device.getInteropPropertyValue(TgtInterop, Ipr, ValueType, Size,
                                        Value);
  }
  }

  return Rc;
}

EXTERN omp_intptr_t omp_get_interop_int(
    const omp_interop_t Interop, omp_interop_property_t PropertyId,
    int *RetCode) {
  DP("Call to %s with interop " DPxMOD ", property ID %" PRId32 "\n", __func__,
     DPxPTR(Interop), PropertyId);

  omp_intptr_t Ret = 0;
  int32_t Rc = checkInteropCall(Interop, __func__);

  if (Rc == omp_irc_success)
    Rc = getInteropValue(Interop, (int32_t)PropertyId, OMP_IPR_VALUE_INT,
                         sizeof(Ret), &Ret);
  if (RetCode)
    *RetCode = Rc;

  return Ret;
}

EXTERN void *omp_get_interop_ptr(
    const omp_interop_t Interop, omp_interop_property_t PropertyId,
    int *RetCode) {
  DP("Call to %s with interop " DPxMOD ", property ID %" PRId32 "\n", __func__,
     DPxPTR(Interop), PropertyId);

  void *Ret = NULL;
  int32_t Rc = checkInteropCall(Interop, __func__);

  if (Rc == omp_irc_success)
    Rc = getInteropValue(Interop, (int32_t)PropertyId, OMP_IPR_VALUE_PTR,
                         sizeof(Ret), &Ret);
  if (RetCode)
    *RetCode = Rc;

  return Ret;
}

EXTERN const char *omp_get_interop_str(
    const omp_interop_t Interop, omp_interop_property_t PropertyId,
    int *RetCode) {
  DP("Call to %s with interop " DPxMOD ", property ID %" PRId32 "\n", __func__,
     DPxPTR(Interop), PropertyId);

  const char *Ret = NULL;
  int32_t Rc = checkInteropCall(Interop, __func__);

  if (Rc == omp_irc_success)
    Rc = getInteropValue(Interop, (int32_t)PropertyId, OMP_IPR_VALUE_STR,
                         sizeof(Ret), &Ret);
  if (RetCode)
    *RetCode = Rc;

  return Ret;
}

EXTERN const char *omp_get_interop_name(
    const omp_interop_t Interop, omp_interop_property_t PropertyId) {
  DP("Call to %s with interop " DPxMOD ", property ID %" PRId32 "\n", __func__,
     DPxPTR(Interop), PropertyId);

  if (checkInteropCall(Interop, __func__) != omp_irc_success)
    return NULL;

  int64_t DeviceNum = static_cast<__tgt_interop *>(Interop)->DeviceNum;
  DeviceTy &Device = *PM->Devices[DeviceNum];

  return Device.getInteropPropertyInfo(PropertyId, OMP_IPR_INFO_NAME);
}

EXTERN const char *omp_get_interop_type_desc(
    const omp_interop_t Interop, omp_interop_property_t PropertyId) {
  DP("Call to %s with interop " DPxMOD ", property ID %" PRId32 "\n", __func__,
     DPxPTR(Interop), PropertyId);

  if (checkInteropCall(Interop, __func__) != omp_irc_success)
    return NULL;

  int64_t DeviceNum = static_cast<__tgt_interop *>(Interop)->DeviceNum;
  DeviceTy &Device = *PM->Devices[DeviceNum];

  return Device.getInteropPropertyInfo(PropertyId, OMP_IPR_INFO_TYPE_DESC);
}

EXTERN const char *omp_get_interop_rc_desc(
    const omp_interop_t Interop, omp_interop_rc_t RetCode) {
  DP("Call to %s with interop " DPxMOD ", return code %" PRId32 "\n", __func__,
     DPxPTR(Interop), RetCode);

  if (checkInteropCall(Interop, __func__) != omp_irc_success)
    return NULL;

  int64_t DeviceNum = static_cast<__tgt_interop *>(Interop)->DeviceNum;
  DeviceTy &Device = *PM->Devices[DeviceNum];

  return Device.getInteropRcDesc(RetCode);
}

EXTERN void *omp_target_alloc_device(size_t Size, int DeviceNum) {
  return targetAllocExplicit(Size, DeviceNum, TARGET_ALLOC_DEVICE, __func__);
}

EXTERN void *omp_target_alloc_host(size_t Size, int DeviceNum) {
  return targetAllocExplicit(Size, DeviceNum, TARGET_ALLOC_HOST, __func__);
}

EXTERN void *omp_target_alloc_shared(size_t Size, int DeviceNum) {
  return targetAllocExplicit(Size, DeviceNum, TARGET_ALLOC_SHARED, __func__);
}

static void *targetRealloc(void *Ptr, size_t Size, int DeviceNum, int Kind,
                           const char *Name) {
  TIMESCOPE();
  DP("Call to %s for device %d requesting %zu bytes (Ptr: " DPxMOD ")\n",
     Name, DeviceNum, Size, DPxPTR(Ptr));

  if (Size <= 0) {
    DP("Call to %s with non-positive length\n", Name);
    return NULL;
  }

  void *Ret = NULL;

  if (DeviceNum == omp_get_initial_device()) {
    if (Ptr)
      Ret = realloc(Ptr, Size);
    else
      Ret = malloc(Size);
    DP("%s returns host ptr " DPxMOD "\n", Name, DPxPTR(Ret));
    return Ret;
  }

  if (!deviceIsReady(DeviceNum)) {
    DP("%s returns NULL ptr\n", Name);
    return NULL;
  }

  DeviceTy &Device = *PM->Devices[DeviceNum];
  Ret = Device.dataRealloc(Ptr, Size, Kind);
  DP("%s returns target ptr " DPxMOD "\n", Name, DPxPTR(Ret));

  return Ret;
}

static void *targetAlignedAlloc(size_t Align, size_t Size, int DeviceNum,
                                int Kind, const char *Name) {
  TIMESCOPE();
  DP("Call to %s for device %d requesting %zu bytes (Align: %zu)\n", Name,
     DeviceNum, Size, Align);

  if (Size <= 0) {
    DP("Call to %s with non-positive length\n", Name);
    return NULL;
  }

  void *Ret = NULL;

  if (DeviceNum == omp_get_initial_device()) {
    Ret = malloc(Size);
    DP("%s returns host ptr " DPxMOD "\n", Name, DPxPTR(Ret));
    return Ret;
  }

  if (!deviceIsReady(DeviceNum)) {
    DP("%s returns NULL ptr\n", Name);
    return NULL;
  }

  DeviceTy &Device = *PM->Devices[DeviceNum];
  Ret = Device.dataAlignedAlloc(Align, Size, Kind);
  DP("%s returns target ptr " DPxMOD "\n", Name, DPxPTR(Ret));

  return Ret;
}

EXTERN void *ompx_target_realloc(void *Ptr, size_t Size, int DeviceNum) {
  return targetRealloc(Ptr, Size, DeviceNum, TARGET_ALLOC_DEFAULT, __func__);
}

EXTERN void *ompx_target_realloc_device(void *Ptr, size_t Size, int DeviceNum) {
  return targetRealloc(Ptr, Size, DeviceNum, TARGET_ALLOC_DEVICE, __func__);
}

EXTERN void *ompx_target_realloc_host(void *Ptr, size_t Size, int DeviceNum) {
  return targetRealloc(Ptr, Size, DeviceNum, TARGET_ALLOC_HOST, __func__);
}

EXTERN void *ompx_target_realloc_shared(void *Ptr, size_t Size, int DeviceNum) {
  return targetRealloc(Ptr, Size, DeviceNum, TARGET_ALLOC_SHARED, __func__);
}

EXTERN void *ompx_target_aligned_alloc(
    size_t Align, size_t Size, int DeviceNum) {
  return targetAlignedAlloc(Align, Size, DeviceNum, TARGET_ALLOC_DEFAULT,
                            __func__);
}

EXTERN void *ompx_target_aligned_alloc_device(
    size_t Align, size_t Size, int DeviceNum) {
  return targetAlignedAlloc(Align, Size, DeviceNum, TARGET_ALLOC_DEVICE,
                            __func__);
}

EXTERN void *ompx_target_aligned_alloc_host(
    size_t Align, size_t Size, int DeviceNum) {
  return targetAlignedAlloc(Align, Size, DeviceNum, TARGET_ALLOC_HOST,
                            __func__);
}

EXTERN void *ompx_target_aligned_alloc_shared(
    size_t Align, size_t Size, int DeviceNum) {
  return targetAlignedAlloc(Align, Size, DeviceNum, TARGET_ALLOC_SHARED,
                            __func__);
}

EXTERN int ompx_target_register_host_pointer(void *HostPtr, size_t Size,
                                             int DeviceNum) {
  DP("Call to %s for device %d requesting registering " DPxMOD
     " of %zu bytes\n", __func__, DeviceNum, DPxPTR(HostPtr),  Size);

  if (Size <= 0) {
    DP("Call to %s with non-positive length\n", __func__);
    return 0;
  }

  if (DeviceNum == omp_get_initial_device()) {
    DP("Cannot register host pointer " DPxMOD " with host device\n",
       DPxPTR(HostPtr));
    return 0;
  }

  if (!deviceIsReady(DeviceNum)) {
    DP("Cannot register host pointer as device is not ready\n");
    return 0;
  }

  DeviceTy &Device = *PM->Devices[DeviceNum];

  int Ret = Device.registerHostPointer(HostPtr, Size);
  if (!Ret)
    DP("Register host pointer failed\n");
  return Ret;
}

EXTERN void ompx_target_unregister_host_pointer(void *HostPtr, int DeviceNum) {
  DP("Call to %s for device %d requesting unregistering " DPxMOD " \n",
     __func__, DeviceNum, DPxPTR(HostPtr));
  DeviceTy &Device = *PM->Devices[DeviceNum];

  bool Ret = Device.unregisterHostPointer(HostPtr);
  if (!Ret)
    DP("UnRegister host pointer failed\n");
}

EXTERN void *omp_target_get_context(int DeviceNum) {
  if (DeviceNum == omp_get_initial_device()) {
    REPORT("%s returns null for the host device\n", __func__);
    return nullptr;
  }

  if (!deviceIsReady(DeviceNum)) {
    REPORT("%s returns null for device %d\n", __func__, DeviceNum);
    return nullptr;
  }

  void *Context = PM->Devices[DeviceNum]->getContextHandle();
  DP("%s returns " DPxMOD " for device %d\n", __func__, DPxPTR(Context),
     DeviceNum);
  return Context;
}

EXTERN int omp_set_sub_device(int DeviceNum, int Level) {
  if (DeviceNum == omp_get_initial_device()) {
    REPORT("%s returns 0 for the host device\n", __func__);
    return 0;
  }

  if (!deviceIsReady(DeviceNum)) {
    REPORT("%s returns 0 for device %d\n", __func__, DeviceNum);
    return 0;
  }

  return PM->Devices[DeviceNum]->setSubDevice(Level);
}

EXTERN void omp_unset_sub_device(int DeviceNum) {
  if (DeviceNum == omp_get_initial_device()) {
    REPORT("%s does nothing for the host device\n", __func__);
    return;
  }

  if (!deviceIsReady(DeviceNum)) {
    REPORT("%s does nothing for device %d\n", __func__, DeviceNum);
    return;
  }

  PM->Devices[DeviceNum]->unsetSubDevice();
}

EXTERN int ompx_get_num_subdevices(int DeviceNum, int Level) {
  if (DeviceNum == omp_get_initial_device()) {
    REPORT("%s returns 0 for the host device\n", __func__);
    return 0;
  }

  if (Level < 0 || Level > 1) {
    REPORT("%s returns 0 for invalid level %" PRId32 "\n", __func__, Level);
    return 0;
  }

  if (!deviceIsReady(DeviceNum)) {
    REPORT("%s returns 0 for device %d\n", __func__, DeviceNum);
    return 0;
  }

  // We return 1 if the device does not support subdevice (Ret == 0)
  int Ret = PM->Devices[DeviceNum]->getNumSubDevices(Level);
  if (Ret > 1)
    return Ret;
  else
    return 1;
}

EXTERN void ompx_kernel_batch_begin(int DeviceNum, uint32_t MaxKernels) {
  if (DeviceNum == omp_get_initial_device()) {
    REPORT("%s does nothing for the host device\n", __func__);
    return;
  }

  if (!deviceIsReady(DeviceNum)) {
    REPORT("%s does nothing for device %d\n", __func__, DeviceNum);
    return;
  }

  PM->Devices[DeviceNum]->kernelBatchBegin(MaxKernels);
}

EXTERN void ompx_kernel_batch_end(int DeviceNum) {
  if (DeviceNum == omp_get_initial_device()) {
    REPORT("%s does nothing for the host device\n", __func__);
    return;
  }

  if (!deviceIsReady(DeviceNum)) {
    REPORT("%s does nothing for device %d\n", __func__, DeviceNum);
    return;
  }

  PM->Devices[DeviceNum]->kernelBatchEnd();
}

EXTERN int ompx_get_device_info(int DeviceNum, int InfoId, size_t InfoSize,
                                void *InfoValue, size_t *InfoSizeRet) {
  if (DeviceNum == omp_get_initial_device()) {
    REPORT("%s does nothing for the host device\n", __func__);
    return OFFLOAD_FAIL;
  }

  if (!deviceIsReady(DeviceNum)) {
    REPORT("%s does nothing for device %d\n", __func__, DeviceNum);
    return OFFLOAD_FAIL;
  }

  return PM->Devices[DeviceNum]->getDeviceInfo(InfoId, InfoSize, InfoValue,
                                               InfoSizeRet);
}

EXTERN void *ompx_target_aligned_alloc_shared_with_hint(
    size_t Align, size_t Size, int32_t AccessHint, int32_t DeviceNum) {
  DP("Call to %s for device %d requesting %zu bytes (Align: %zu, "
     "AccessHint: %d)\n", __func__, DeviceNum, Size, Align, AccessHint);

  void *Ret = nullptr;

  if (Size == 0)
    return Ret;

  if (DeviceNum == omp_get_initial_device()) {
    Ret = malloc(Size);
    DP("%s returns host ptr " DPxMOD "\n", __func__, DPxPTR(Ret));
    return Ret;
  }

  if (!deviceIsReady(DeviceNum)) {
    DP("%s returns NULL ptr\n", __func__);
    return Ret;
  }

  DeviceTy &Device = *PM->Devices[DeviceNum];
  Ret = Device.dataAlignedAllocShared(Align, Size, AccessHint);
  DP("%s returns target pointer " DPxMOD "\n", __func__, DPxPTR(Ret));

  return Ret;
}

EXTERN int ompx_target_prefetch_shared_mem(
    size_t NumPtrs, void **Ptrs, size_t *Sizes, int32_t DeviceNum) {
  DP("Call to %s for device %d with number of pointers %zu, "
     "pointer array " DPxMOD ", size array " DPxMOD "\n", __func__, DeviceNum,
     NumPtrs, DPxPTR(Ptrs), DPxPTR(Sizes));

  if (NumPtrs == 0 || DeviceNum == omp_get_initial_device())
    return OFFLOAD_SUCCESS; // Nothing to be done

  if (!Ptrs || !Sizes) {
    REPORT("Call to %s with invalid input\n", __func__);
    return OFFLOAD_FAIL;
  }

  if (!deviceIsReady(DeviceNum)) {
    DP("%s returns OFFLOAD_FAIL\n", __func__);
    return OFFLOAD_FAIL;
  }

  DeviceTy &Device = *PM->Devices[DeviceNum];
  int Ret = Device.prefetchSharedMem(NumPtrs, Ptrs, Sizes);
  if (Ret != OFFLOAD_SUCCESS)
    REPORT("%s returns OFFLOAD_FAIL\n", __func__);

  return Ret;
}

EXTERN int ompx_get_device_from_ptr(const void *Ptr) {
  int Ret = omp_get_initial_device();

  // Use the first device to access the RTL
  if (!Ptr || !deviceIsReady(0)) {
    DP("%s returns initial device for the pointer " DPxMOD "\n", __func__,
       DPxPTR(Ptr));
    return Ret;
  }

  DeviceTy &Device = *PM->Devices[0];
  if (!Device.RTL->get_device_from_ptr)
    return Ret;

  int DeviceNum = Device.RTL->get_device_from_ptr(Ptr);
  // Expect RTL returning negative value if Ptr does not belong to any device
  if (DeviceNum >= 0)
    Ret = DeviceNum;

  return Ret;
}
#endif // INTEL_CUSTOMIZATION

EXTERN void *omp_get_mapped_ptr(const void *Ptr, int DeviceNum) {
  TIMESCOPE();
  DP("Call to omp_get_mapped_ptr with ptr " DPxMOD ", device_num %d.\n",
     DPxPTR(Ptr), DeviceNum);

  if (!Ptr) {
    REPORT("Call to omp_get_mapped_ptr with nullptr.\n");
    return nullptr;
  }

  if (DeviceNum == omp_get_initial_device()) {
    REPORT("Device %d is initial device, returning Ptr " DPxMOD ".\n",
           DeviceNum, DPxPTR(Ptr));
    return const_cast<void *>(Ptr);
  }

  int DevicesSize = omp_get_initial_device();
  {
    std::lock_guard<std::mutex> LG(PM->RTLsMtx);
    DevicesSize = PM->Devices.size();
  }
  if (DevicesSize <= DeviceNum) {
    DP("DeviceNum %d is invalid, returning nullptr.\n", DeviceNum);
    return nullptr;
  }

  if (!deviceIsReady(DeviceNum)) {
    REPORT("Device %d is not ready, returning nullptr.\n", DeviceNum);
    return nullptr;
  }
#if INTEL_CUSTOMIZATION
  CHECK_DEVICE_AND_CTORS_RET(DeviceNum, nullptr);
#endif // INTEL_CUSTOMIZATION

  auto &Device = *PM->Devices[DeviceNum];
  TargetPointerResultTy TPR = Device.getTgtPtrBegin(const_cast<void *>(Ptr), 1,
                                                    /*UpdateRefCount=*/false,
                                                    /*UseHoldRefCount=*/false);
  if (!TPR.isPresent()) {
    DP("Ptr " DPxMOD "is not present on device %d, returning nullptr.\n",
       DPxPTR(Ptr), DeviceNum);
    return nullptr;
  }

  DP("omp_get_mapped_ptr returns " DPxMOD ".\n", DPxPTR(TPR.TargetPointer));

  return TPR.TargetPointer;
}
