//===------ PGOOptions.h -- PGO option tunables ----------------*- C++ -*--===//
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
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
/// \file
///
/// Define option tunables for PGO.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_SUPPORT_PGOOPTIONS_H
#define LLVM_SUPPORT_PGOOPTIONS_H

#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/Support/Error.h"

namespace llvm {

namespace vfs {
class FileSystem;
} // namespace vfs

/// A struct capturing PGO tunables.
struct PGOOptions {
  enum PGOAction { NoAction, IRInstr, IRUse, SampleUse, MLUse }; // INTEL
  enum CSPGOAction { NoCSAction, CSIRInstr, CSIRUse };
  PGOOptions(std::string ProfileFile, std::string CSProfileGenFile,
             std::string ProfileRemappingFile, std::string MemoryProfile,
             IntrusiveRefCntPtr<vfs::FileSystem> FS,
             PGOAction Action = NoAction, CSPGOAction CSAction = NoCSAction,
             bool DebugInfoForProfiling = false,
             bool PseudoProbeForProfiling = false,
             bool AtomicCounterUpdate = false);
  PGOOptions(const PGOOptions &);
  ~PGOOptions();
  PGOOptions &operator=(const PGOOptions &);

  std::string ProfileFile;
  std::string CSProfileGenFile;
  std::string ProfileRemappingFile;
  std::string MemoryProfile;
  PGOAction Action;
  CSPGOAction CSAction;
  bool DebugInfoForProfiling;
  bool PseudoProbeForProfiling;
  bool AtomicCounterUpdate;
  IntrusiveRefCntPtr<vfs::FileSystem> FS;
  bool IsCGPGO = false; // INTEL
};
} // namespace llvm

#endif
