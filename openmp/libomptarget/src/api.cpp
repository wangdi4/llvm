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

#include <climits>
#include <cstdlib>
#include <cstring>

#if INTEL_COLLAB
/// API functions that can access host-to-target data map need to load device
/// image to get correct mapping information for global data. This macro is
/// called in such functions.
#define CHECK_DEVICE_AND_CTORS_RET(ID, RetVal)                                 \
  do {                                                                         \
    int64_t DeviceID = ID;                                                     \
    if (checkDeviceAndCtors(DeviceID, nullptr) != OFFLOAD_SUCCESS)             \
      return RetVal;                                                           \
  } while(0)
#endif // INTEL_COLLAB

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

EXTERN void *llvm_omp_target_dynamic_shared_alloc() { return nullptr; }
EXTERN void *llvm_omp_get_dynamic_shared() { return nullptr; }

EXTERN void omp_target_free(void *DevicePtr, int DeviceNum) {
  TIMESCOPE();
  DP("Call to omp_target_free for device %d and address " DPxMOD "\n",
     DeviceNum, DPxPTR(DevicePtr));

  if (!DevicePtr) {
    DP("Call to omp_target_free with NULL ptr\n");
    return;
  }

  if (DeviceNum == omp_get_initial_device()) {
    free(DevicePtr);
    DP("omp_target_free deallocated host ptr\n");
    return;
  }

  if (!deviceIsReady(DeviceNum)) {
    DP("omp_target_free returns, nothing to do\n");
    return;
  }

  PM->Devices[DeviceNum]->deleteData(DevicePtr);
  DP("omp_target_free deallocated device ptr\n");
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
#if INTEL_COLLAB
  CHECK_DEVICE_AND_CTORS_RET(DeviceNum, false);
#endif // INTEL_COLLAB

  DeviceTy &Device = *PM->Devices[DeviceNum];
  bool IsLast; // not used
  bool IsHostPtr;
  // omp_target_is_present tests whether a host pointer refers to storage that
  // is mapped to a given device. However, due to the lack of the storage size,
  // only check 1 byte. Cannot set size 0 which checks whether the pointer (zero
  // lengh array) is mapped instead of the referred storage.
  TargetPointerResultTy TPR =
      Device.getTgtPtrBegin(const_cast<void *>(Ptr), 1, IsLast,
                            /*UpdateRefCount=*/false,
                            /*UseHoldRefCount=*/false, IsHostPtr);
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
      AsyncInfoTy AsyncInfo(SrcDev);
      Rc = DstDev.submitData(DstAddr, Buffer, Length, AsyncInfo);
    }
    free(Buffer);
  }

  DP("omp_target_memcpy returns %d\n", Rc);
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

#if INTEL_COLLAB
  int Rc = OFFLOAD_SUCCESS;
#else // INTEL_COLLAB
  int Rc;
#endif // INTEL_COLLAB
  if (NumDims == 1) {
    Rc = omp_target_memcpy(Dst, Src, ElementSize * Volume[0],
                           ElementSize * DstOffsets[0],
                           ElementSize * SrcOffsets[0], DstDevice, SrcDevice);
  } else {
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
#if INTEL_COLLAB
  CHECK_DEVICE_AND_CTORS_RET(DeviceNum, OFFLOAD_FAIL);
#endif // INTEL_COLLAB

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
#if INTEL_COLLAB
  CHECK_DEVICE_AND_CTORS_RET(DeviceNum, OFFLOAD_FAIL);
#endif // INTEL_COLLAB

  DeviceTy &Device = *PM->Devices[DeviceNum];
  int Rc = Device.disassociatePtr(const_cast<void *>(HostPtr));
  DP("omp_target_disassociate_ptr returns %d\n", Rc);
  return Rc;
}
#if INTEL_COLLAB
EXTERN void *omp_get_mapped_ptr(void *HostPtr, int DeviceNum) {
  DP("Call to omp_get_mapped_ptr with host pointer " DPxMOD
     ", device number %d\n", DPxPTR(HostPtr), DeviceNum);

  if (!HostPtr) {
    DP("Call to omp_get_mapped_ptr with invalid host pointer\n");
    return NULL;
  }

  if (DeviceNum == omp_get_initial_device()) {
    DP("omp_get_mapped_ptr: Mapped pointer is same as host\n");
    return HostPtr;
  }

  if (!deviceIsReady(DeviceNum)) {
    DP("omp_get_mapped_ptr: returns NULL\n");
    return NULL;
  }
  CHECK_DEVICE_AND_CTORS_RET(DeviceNum, NULL);

  DeviceTy& Device = *PM->Devices[DeviceNum];
  bool IsLast, IsHostPtr;
  TargetPointerResultTy TPR = Device.getTgtPtrBegin(HostPtr, 1, IsLast, false,
                                                    false, IsHostPtr);
  void *TgtPtr = TPR.TargetPointer;
  if (TgtPtr == NULL)
    DP("omp_get_mapped_ptr: cannot find device pointer\n");
  DP("omp_get_mapped_ptr returns " DPxMOD "\n", DPxPTR(TgtPtr));
  return TgtPtr;
}

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

#if INTEL_CUSTOMIZATION
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
#endif // INTEL_CUSTOMIZATION

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
#endif  // INTEL_COLLAB

