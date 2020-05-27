//==--- device.h - device definitions ------------------------*- C++ -*-----==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef __LIBDEVICE_DEVICE_H__
#define __LIBDEVICE_DEVICE_H__

#ifdef __cplusplus
#define EXTERN_C extern "C"
#else // __cplusplus
#define EXTERN_C
#endif // __cplusplus

#ifdef __SPIR__
#ifdef __SYCL_DEVICE_ONLY__
#define DEVICE_EXTERNAL SYCL_EXTERNAL __attribute__((weak))
#else // __SYCL_DEVICE_ONLY__
#define DEVICE_EXTERNAL __attribute__((weak))
#endif // __SYCL_DEVICE_ONLY__
<<<<<<< HEAD
#else  // CL_SYCL_LANGUAGE_VERSION
#if INTEL_COLLAB
#if OMP_LIBDEVICE
// __INTEL_OFFLOAD is undefined for the host compilation
// from source to LLVM BC, so we can use it to make
// all functions 'static' for the host compilation.
// This will cause an empty host binary eventually.
#if __INTEL_OFFLOAD
#define DEVICE_EXTERNAL __attribute__((weak))
#else  // !__INTEL_OFFLOAD
#define DEVICE_EXTERNAL static
#undef EXTERN_C
#define EXTERN_C
#endif  // !__INTEL_OFFLOAD
#else   // !OMP_LIBDEVICE
#error "Unsupported configuration of libdevice."
#endif  // !OMP_LIBDEVICE
#else // INTEL_COLLAB
#define DEVICE_EXTERNAL
#endif  // INTEL_COLLAB
#endif // CL_SYCL_LANGUAGE_VERSION
=======
>>>>>>> 857ee511bf4053f1f0cdc7f0d2b41fd6273926e0

#define DEVICE_EXTERN_C DEVICE_EXTERNAL EXTERN_C
#endif // __SPIR__

#endif // __LIBDEVICE_DEVICE_H__
