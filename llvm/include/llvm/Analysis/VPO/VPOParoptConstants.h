#if INTEL_COLLAB // -*- C++ -*-
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2022 Intel Corporation
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
//===-- VPOParoptConstants.h - Paropt specific Enums/Constants --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// Defines some common constants and enums to be used across VPO/Paropt passes.
///
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_ANALYSIS_VPO_VPOPAROPTCONSTANTS_H
#define LLVM_ANALYSIS_VPO_VPOPAROPTCONSTANTS_H

#include <stdint.h>

namespace llvm {
namespace vpo {

enum TgtMapTypes : uint64_t {
  TGT_MAP_TO = 0x01,
  // instructs the runtime to copy the host data to the device.
  TGT_MAP_FROM = 0x02,
  // instructs the runtime to copy the device data to the host.
  TGT_MAP_ALWAYS = 0x04,
  // forces the copying regardless of the reference
  // count associated with the map.
  TGT_MAP_DELETE = 0x08,
  // forces the unmapping of the object in a target data.
  TGT_MAP_PTR_AND_OBJ = 0x10,
  // forces the runtime to map the pointer variable as
  // well as the pointee variable.
  TGT_MAP_TARGET_PARAM = 0x20,
  // instructs the runtime that it is the first
  // occurrence of this mapped variable within this construct.
  TGT_MAP_RETURN_PARAM = 0x40,
  // instructs the runtime to return the base
  // device address of the mapped variable.
  TGT_MAP_PRIVATE = 0x80,
  // informs the runtime that the variable is a private variable.
  TGT_MAP_LITERAL = 0x100,
  // instructs the runtime to forward the value to target construct.
  TGT_MAP_IMPLICIT = 0x200,
  TGT_MAP_CLOSE = 0x400,
  // The close map-type-modifier is a hint to the runtime to
  // allocate memory close to the target device.
  TGT_MAP_ND_DESC = 0x800,
  // runtime error if not already allocated
  TGT_MAP_PRESENT = 0x1000,
  // use a separate reference counter so that the data cannot be unmapped within
  // the structured region
  // This is an OpenMP extension for the sake of OpenACC support.
  TGT_MAP_OMPX_HOLD = 0x2000,
  // descriptor for non-contiguous target-update
  TGT_MAP_NON_CONTIG = 0x100000000000,
  // member of struct, member given by [16 MSBs] - 1
  TGT_MAP_MEMBER_OF = 0xffff000000000000
};

} // end namespace vpo
} // end namespace llvm
#endif // LLVM_ANALYSIS_VPO_VPOPAROPTCONSTANTS_H
#endif // INTEL_COLLAB
