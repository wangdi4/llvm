// INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2023 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// end INTEL_CUSTOMIZATION
//===- OmptConnector.h - Target independent OpenMP target RTL -- C++ ------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Support used by OMPT implementation to establish communication between
// various OpenMP runtime libraries: host openmp library, target-independent
// runtime library, and device-dependent runtime libraries.
//
//===----------------------------------------------------------------------===//

#ifndef _OMPTCONNECTOR_H
#define _OMPTCONNECTOR_H

#ifdef OMPT_SUPPORT

#include "llvm/Support/DynamicLibrary.h"

#include <memory>
#include <string>

#include "omp-tools.h"

#include "Debug.h"
#include "omptarget.h"

#pragma push_macro("DEBUG_PREFIX")
#undef DEBUG_PREFIX
#define DEBUG_PREFIX "OMPT"

/// Type for the function to be invoked for connecting two libraries.
typedef void (*OmptConnectRtnTy)(ompt_start_tool_result_t *result);

#if INTEL_CUSTOMIZATION
/// Type to support data exchange between omptarget and plugin
using OmptUpdateDataTy = void (*)(int32_t Kind, size_t Size, void *Value);
#endif // INTEL_CUSTOMIZATION

/// Establish connection between openmp runtime libraries
///
/// This class is used to communicate between an OMPT implementation in
/// libomptarget and libomp. It is also used to communicate between an
/// OMPT implementation in a device-specific plugin and
/// libomptarget. The decision whether OMPT is enabled or not needs to
/// be made when the library is loaded before any functions in the
/// library are invoked. For that reason, an instance of this class is
/// intended to be defined in the constructor for libomptarget or a
/// plugin so that the decision about whether OMPT is supposed to be
/// enabled is known before any interface function in the library is
/// invoked.
class OmptLibraryConnectorTy {
public:
  /// Use \p LibName as the prefix of the global function used for connecting
  /// two libraries, the source indicated by \p LibName and the destination
  /// being the one that creates this object.
  OmptLibraryConnectorTy(const char *Ident) {
    LibIdent.append(Ident);
    IsInitialized = false;
#if INTEL_CUSTOMIZATION
    LibConnHandle = nullptr;
#endif // INTEL_CUSTOMIZATION
  }
  OmptLibraryConnectorTy() = delete;
  /// Use \p OmptResult init to connect the two libraries denoted by this
  /// object. The init function of \p OmptResult will be used during connection
  /// and the fini function of \p OmptResult will be used during teardown.
  void connect(ompt_start_tool_result_t *OmptResult) {
    initialize();
    if (!LibConnHandle)
      return;
    // Call the function provided by the source library for connect
    LibConnHandle(OmptResult);
  }
#if INTEL_CUSTOMIZATION
  /// Support customized data channel between omptarget and plugin
  void getUpdateDataHandles(OmptUpdateDataTy *SetFn, OmptUpdateDataTy *GetFn) {
    if (!IsInitialized)
      return;
    if (SetFn)
      *SetFn = SetOmptData;
    if (GetFn)
      *GetFn = GetOmptData;
  }
#endif // INTEL_CUSTOMIZATION

private:
  void initialize() {
    if (IsInitialized)
      return;

    std::string ErrMsg;
    std::string LibName = LibIdent;
    LibName += ".so";
#if INTEL_CUSTOMIZATION
    // Support runtime names in the product.
#ifdef _WIN32
    if (LibIdent == "libomp")
      LibName = "libiomp5md.dll";
    else if (LibIdent == "libomptarget")
      LibName = "omptarget.dll";
#else  // _WIN32
    if (LibIdent == "libomp")
      LibName = "libiomp5.so";
#endif // _WIN32
#endif // INTEL_CUSTOMIZATION

    DP("OMPT: Trying to load library %s\n", LibName.c_str());
    auto DynLibHandle = std::make_unique<llvm::sys::DynamicLibrary>(
        llvm::sys::DynamicLibrary::getPermanentLibrary(LibName.c_str(),
                                                       &ErrMsg));
    if (!DynLibHandle->isValid()) {
      // The upper layer will bail out if the handle is null.
      LibConnHandle = nullptr;
    } else {
      auto LibConnRtn = "ompt_" + LibIdent + "_connect";
      DP("OMPT: Trying to get address of connection routine %s\n",
         LibConnRtn.c_str());
      LibConnHandle = reinterpret_cast<OmptConnectRtnTy>(
          DynLibHandle->getAddressOfSymbol(LibConnRtn.c_str()));
#if INTEL_CUSTOMIZATION
      /// We could do this separately, but it is better to set these handles
      /// since we already loaded module here.
      SetOmptData = reinterpret_cast<OmptUpdateDataTy>(
          DynLibHandle->getAddressOfSymbol("ompt_oneapi_set_data"));
      GetOmptData = reinterpret_cast<OmptUpdateDataTy>(
          DynLibHandle->getAddressOfSymbol("ompt_oneapi_get_data"));
#endif // INTEL_CUSTOMIZATION
    }
    DP("OMPT: Library connection handle = %p\n", LibConnHandle);
    IsInitialized = true;
  }

  /// Ensure initialization occurs only once
  bool IsInitialized;
  /// Handle of connect routine provided by source library
  OmptConnectRtnTy LibConnHandle;
  /// Name of connect routine provided by source library
  std::string LibIdent;
#if INTEL_CUSTOMIZATION
  /// Handle to set OMPT interface data
  OmptUpdateDataTy SetOmptData = nullptr;
  /// Handle to get OMPT interface data
  OmptUpdateDataTy GetOmptData = nullptr;
#endif // INTEL_CUSTOMIZATION
};

#endif // OMPT_SUPPORT

#pragma pop_macro("DEBUG_PREFIX")

#endif // _OMPTCONNECTOR_H
