//===- ArchiveWriter.h - ar archive file format writer ----------*- C++ -*-===//
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
//
// Declares the writeArchive function for writing an archive file.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_OBJECT_ARCHIVEWRITER_H
#define LLVM_OBJECT_ARCHIVEWRITER_H

#include "llvm/Object/Archive.h"

namespace llvm {

struct NewArchiveMember {
  std::unique_ptr<MemoryBuffer> Buf;
  StringRef MemberName;
  sys::TimePoint<std::chrono::seconds> ModTime;
  unsigned UID = 0, GID = 0, Perms = 0644;

  NewArchiveMember() = default;
  NewArchiveMember(MemoryBufferRef BufRef);

  // Detect the archive format from the object or bitcode file. This helps
  // assume the archive format when creating or editing archives in the case
  // one isn't explicitly set.
  object::Archive::Kind detectKindFromObject() const;

  static Expected<NewArchiveMember>
  getOldMember(const object::Archive::Child &OldMember, bool Deterministic);

  static Expected<NewArchiveMember> getFile(StringRef FileName,
                                            bool Deterministic);

#ifdef INTEL_CUSTOMIZATION
  void setOpaquePointers() { OpaquePointers = true; }
  bool isOpaque() const { return OpaquePointers; }

private:
  bool OpaquePointers = false;
#endif // INTEL_CUSTOMIZATION
};

Expected<std::string> computeArchiveRelativePath(StringRef From, StringRef To);

Error writeArchive(StringRef ArcName, ArrayRef<NewArchiveMember> NewMembers,
                   bool WriteSymtab, object::Archive::Kind Kind,
                   bool Deterministic, bool Thin,
                   std::unique_ptr<MemoryBuffer> OldArchiveBuf = nullptr,
                   bool IsEC = false);

// writeArchiveToBuffer is similar to writeArchive but returns the Archive in a
// buffer instead of writing it out to a file.
Expected<std::unique_ptr<MemoryBuffer>>
writeArchiveToBuffer(ArrayRef<NewArchiveMember> NewMembers, bool WriteSymtab,
                     object::Archive::Kind Kind, bool Deterministic, bool Thin);
}

#endif
