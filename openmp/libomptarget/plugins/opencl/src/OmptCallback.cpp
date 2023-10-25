#if INTEL_CUSTOMIZATION
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
//===---------- OmptCallback.cpp - Generic OMPT callbacks --------- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// OMPT support for Level Zero devices
//
//===----------------------------------------------------------------------===//
#ifdef OMPT_SUPPORT

#include "llvm/Support/DynamicLibrary.h"

#include <cstdlib>
#include <cstring>
#include <memory>

#include "Debug.h"
#include "OmptCallback.h"
#include "OmptConnector.h"

#define TARGET_NAME OPENCL
#define DEBUG_PREFIX "Target " GETNAME(TARGET_NAME) " RTL"

using namespace llvm::omp::target::ompt;

bool llvm::omp::target::ompt::Initialized = false;

ompt_get_callback_t llvm::omp::target::ompt::lookupCallbackByCode = nullptr;
ompt_function_lookup_t llvm::omp::target::ompt::lookupCallbackByName = nullptr;

OmptUpdateDataTy OmptSetData = nullptr, OmptGetData = nullptr;

int llvm::omp::target::ompt::initializeLibrary(ompt_function_lookup_t lookup,
                                               int initial_device_num,
                                               ompt_data_t *tool_data) {
  DP("OMPT: Executing initializeLibrary (libomptarget)\n");
#define bindOmptFunctionName(OmptFunction, DestinationFunction)                \
  if (lookup)                                                                  \
    DestinationFunction = (OmptFunction##_t)lookup(#OmptFunction);             \
  DP("OMPT: initializeLibrary (libomptarget) bound %s=%p\n",                   \
     #DestinationFunction, ((void *)(uint64_t)DestinationFunction));

  bindOmptFunctionName(ompt_get_callback, lookupCallbackByCode);
#undef bindOmptFunctionName

  // Store pointer of 'ompt_libomp_target_fn_lookup' for use by the plugin
  lookupCallbackByName = lookup;

  Initialized = true;

  return 0;
}

void llvm::omp::target::ompt::finalizeLibrary(ompt_data_t *tool_data) {
  DP("OMPT: Executing finalizeLibrary (libomptarget)\n");
}

void llvm::omp::target::ompt::connectLibrary() {
  DP("OMPT: Entering connectLibrary (libomptarget)\n");
  /// Connect plugin instance with libomptarget
  OmptLibraryConnectorTy LibomptargetConnector("libomptarget");
  ompt_start_tool_result_t OmptResult;

  // Initialize OmptResult with the init and fini functions that will be
  // called by the connector
  OmptResult.initialize = ompt::initializeLibrary;
  OmptResult.finalize = ompt::finalizeLibrary;
  OmptResult.tool_data.value = 0;

  // Now call connect that causes the above init/fini functions to be called
  LibomptargetConnector.connect(&OmptResult);
  LibomptargetConnector.getUpdateDataHandles(&OmptSetData, &OmptGetData);
  DP("OMPT: Exiting connectLibrary (libomptarget)\n");
}

/// Set data in the current interface
bool setOmptData(OmptExtDataTy Type, size_t Size, void *Data) {
  if (!OmptSetData)
    return false;
  OmptSetData(Type, Size, Data);
  return true;
}

/// Get data in the current interface
bool getOmptData(OmptExtDataTy Type, size_t Size, void *Data) {
  if (!OmptGetData)
    return false;
  OmptGetData(Type, Size, Data);
  return true;
}

///
/// OMPT entry points provided by plugin
///

/// Return the number of assigned number of teams and team sizes
static void ompt_oneapi_get_team_size(int *NumTeams, int *TeamSize) {
  if (!NumTeams || !TeamSize)
    return;
  if (!getOmptData(OmptExtDataNumTeams, sizeof(int), NumTeams))
    *NumTeams = 0;
  if (!getOmptData(OmptExtDataTeamSize, sizeof(int), TeamSize))
    *TeamSize = 0;
}

/// Return the code location string associated with the return address
static const char *ompt_oneapi_get_code_location(void) {
  const char *Ret = nullptr;
  (void)getOmptData(OmptExtDataCodeLocation, sizeof(Ret), &Ret);
  return Ret;
}

ompt_interface_fn_t lookupOmptEntry(const char *EntryName) {
  ompt_interface_fn_t NotFound = nullptr;
  if (!EntryName)
    return NotFound;

  std::string FnName(EntryName);
#define CHECK_OMPT_ENTRY(Name)                                                 \
  do {                                                                         \
    if (FnName == #Name)                                                       \
      return (ompt_interface_fn_t)Name;                                        \
  } while (0)

  CHECK_OMPT_ENTRY(ompt_oneapi_get_code_location);
  CHECK_OMPT_ENTRY(ompt_oneapi_get_team_size);

  return NotFound;
}

#endif // OMPT_SUPPORT

#endif // INTEL_CUSTOMIZATION
