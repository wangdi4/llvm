//===-- elf_common.cpp - Common ELF functionality -------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Common ELF functionality for target plugins.
//
//===----------------------------------------------------------------------===//
#include "elf_common.h"
#include "Debug.h"

#include "llvm/BinaryFormat/Magic.h"
#include "llvm/Object/Binary.h"
#include "llvm/Object/ELFObjectFile.h"
#include "llvm/Object/ELFTypes.h"
#include "llvm/Object/ObjectFile.h"
#include "llvm/Support/MemoryBuffer.h"

#ifndef TARGET_NAME
#define TARGET_NAME ELF Common
#endif
#define DEBUG_PREFIX "TARGET " GETNAME(TARGET_NAME)

using namespace llvm;
using namespace llvm::ELF;
using namespace llvm::object;
#if INTEL_COLLAB
static const char *getOffloadNoteTypeName(uint64_t Type) {
  struct NoteType {
    uint64_t ID;
    const char *Name;
  };

  static constexpr const NoteType LLVMOMPOFFLOADNoteTypes[] = {
      {NT_LLVM_OPENMP_OFFLOAD_VERSION, "NT_LLVM_OPENMP_OFFLOAD_VERSION"},
      {NT_LLVM_OPENMP_OFFLOAD_PRODUCER, "NT_LLVM_OPENMP_OFFLOAD_PRODUCER"},
      {NT_LLVM_OPENMP_OFFLOAD_PRODUCER_VERSION,
       "NT_LLVM_OPENMP_OFFLOAD_PRODUCER_VERSION"},
  };

  for (const NoteType &N : LLVMOMPOFFLOADNoteTypes) {
    if (N.ID == Type)
      return N.Name;
  }

  return "";
}

template <typename ELFT>
static void printELFNotes(const ELFObjectFile<ELFT> *Object) {
  auto &ELFFile = Object->getELFFile();
  using Elf_Shdr_Range = typename ELFT::ShdrRange;
  Expected<Elf_Shdr_Range> Sections = ELFFile.sections();
  if (!Sections)
    return;

  for (auto SB = Sections->begin(), SE = Sections->end(); SB != SE; ++SB) {
    if (SB->sh_type != ELF::SHT_NOTE)
      continue;
    Error Err = Error::success();
    for (auto Note : ELFFile.notes(*SB, Err)) {
      StringRef Name = Note.getName();
      if (!Name.equals("LLVMOMPOFFLOAD"))
        continue;

      uint64_t Type = static_cast<uint64_t>(Note.getType());
      switch (Type) {
      default:
        DP("LLVMOMPOFFLOAD ELF note with unknown type %" PRIu64 ".\n", Type);
        break;
      case NT_LLVM_OPENMP_OFFLOAD_VERSION:
      case NT_LLVM_OPENMP_OFFLOAD_PRODUCER:
      case NT_LLVM_OPENMP_OFFLOAD_PRODUCER_VERSION: {
        StringRef Desc = Note.getDescAsStringRef();
        DP("LLVMOMPOFFLOAD ELF note %s with value: '%s'\n",
           getOffloadNoteTypeName(Type), Desc.str().c_str());
        break;
      }
      }
    }
  }
}
#endif // INTEL_COLLAB

/// If the given range of bytes [\p BytesBegin, \p BytesEnd) represents
/// a valid ELF, then invoke \p Callback on the ELFObjectFileBase
/// created from this range, otherwise, return 0.
/// If \p Callback is invoked, then return whatever value \p Callback returns.
template <typename F>
static int32_t withBytesAsElf(char *BytesBegin, char *BytesEnd, F Callback) {
  size_t Size = BytesEnd - BytesBegin;
  StringRef StrBuf(BytesBegin, Size);

  auto Magic = identify_magic(StrBuf);
  if (Magic != file_magic::elf && Magic != file_magic::elf_relocatable &&
      Magic != file_magic::elf_executable &&
      Magic != file_magic::elf_shared_object && Magic != file_magic::elf_core) {
    DP("Not an ELF image!\n");
    return 0;
  }

  std::unique_ptr<MemoryBuffer> MemBuf =
      MemoryBuffer::getMemBuffer(StrBuf, "", false);
  Expected<std::unique_ptr<ObjectFile>> BinOrErr =
      ObjectFile::createELFObjectFile(MemBuf->getMemBufferRef(),
                                      /*InitContent=*/false);
  if (!BinOrErr) {
    DP("Unable to get ELF handle: %s!\n",
       toString(BinOrErr.takeError()).c_str());
    return 0;
  }

  auto *Object = dyn_cast<const ELFObjectFileBase>(BinOrErr->get());

  if (!Object) {
    DP("Unknown ELF format!\n");
    return 0;
  }

  return Callback(Object);
}

// Check whether an image is valid for execution on target_id
int32_t elf_check_machine(__tgt_device_image *Image, uint16_t TargetId) {
  auto CheckMachine = [TargetId](const ELFObjectFileBase *Object) {
    return TargetId == Object->getEMachine();
  };
<<<<<<< HEAD
#if INTEL_COLLAB
  if (getDebugLevel() > 0) {
    auto PrintELFNotes = [](const ELFObjectFileBase *Object) {
      if (auto *Obj = dyn_cast<ELF64LEObjectFile>(Object))
        printELFNotes(Obj);
      else if (auto *Obj = dyn_cast<ELF64BEObjectFile>(Object))
        printELFNotes(Obj);
      else if (auto *Obj = dyn_cast<ELF32LEObjectFile>(Object))
        printELFNotes(Obj);
      else if (auto *Obj = dyn_cast<ELF32BEObjectFile>(Object))
        printELFNotes(Obj);
      return 0;
    };

    (void)withBytesAsElf(reinterpret_cast<char *>(image->ImageStart),
                         reinterpret_cast<char *>(image->ImageEnd),
                         PrintELFNotes);
  }
#endif // INTEL_COLLAB
  return withBytesAsElf(reinterpret_cast<char *>(image->ImageStart),
                        reinterpret_cast<char *>(image->ImageEnd),
=======
  return withBytesAsElf(reinterpret_cast<char *>(Image->ImageStart),
                        reinterpret_cast<char *>(Image->ImageEnd),
>>>>>>> d27d0a673c64068c5f3a1981c428e0ef5cff8062
                        CheckMachine);
}

int32_t elf_is_dynamic(__tgt_device_image *Image) {
  auto CheckDynType = [](const ELFObjectFileBase *Object) {
    uint16_t Type = Object->getEType();
    DP("ELF Type: %d\n", Type);
    return Type == ET_DYN;
  };
  return withBytesAsElf(reinterpret_cast<char *>(Image->ImageStart),
                        reinterpret_cast<char *>(Image->ImageEnd),
                        CheckDynType);
}
