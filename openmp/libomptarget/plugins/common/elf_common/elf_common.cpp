#if INTEL_COLLAB
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
#include "elf_light.h"

#ifndef TARGET_NAME
#define TARGET_NAME Common ELF
#endif
#define DEBUG_PREFIX "TARGET " GETNAME(TARGET_NAME)

#ifdef ELFMAG
// Subtle verification that libelf APIs are not used explicitly here.
// Code here must use libelf independent APIs from elf_light.h.
#error "LIBELF.H cannot be used explicitly here."
#endif // ELFMAG

EXTERN int32_t elf_check_machine(__tgt_device_image *image,
                                 uint16_t target_id) {
  char *img_begin = reinterpret_cast<char *>(image->ImageStart);
  char *img_end = reinterpret_cast<char *>(image->ImageEnd);
  size_t img_size = img_end - img_begin;
  ElfL E(img_begin, img_size);
  if (!E.isValidElf()) {
    DP("Unable to get ELF handle: %s!\n", E.getErrmsg(-1));
    return 0;
  }

  if (getDebugLevel() > 0) {
    auto PrintOutNote = [](const ElfLNote &Note) {
      if (Note.getNameSize() == 0)
        return;

      // Note that the NameStr below does not include the null
      // terminator.
      std::string NameStr(Note.getName(), Note.getNameSize());
      if (NameStr != "LLVMOMPOFFLOAD")
        return;

      uint64_t Type = Note.getType();
      switch (Type) {
      default:
        DP("LLVMOMPOFFLOAD ELF note with unknown type %" PRIu64 ".\n", Type);
        break;
      case NT_LLVM_OPENMP_OFFLOAD_VERSION:
      case NT_LLVM_OPENMP_OFFLOAD_PRODUCER:
      case NT_LLVM_OPENMP_OFFLOAD_PRODUCER_VERSION: {
        std::string DescStr(reinterpret_cast<const char *>(Note.getDesc()),
                            Note.getDescSize());
        DP("LLVMOMPOFFLOAD ELF note %s with value: '%s'\n",
           OMPNoteTypeNames[Type], DescStr.c_str());
        break;
      }
      }
    };

    for (auto I = E.section_notes_begin(), IE = E.section_notes_end(); I != IE;
         ++I)
      PrintOutNote(*I);

    for (auto I = E.segment_notes_begin(), IE = E.segment_notes_end(); I != IE;
         ++I)
      PrintOutNote(*I);
  }
  uint16_t MachineID = E.getEMachine();
  return MachineID == target_id;
}

EXTERN int32_t elf_is_dynamic(__tgt_device_image *image) {
  char *img_begin = reinterpret_cast<char *>(image->ImageStart);
  char *img_end = reinterpret_cast<char *>(image->ImageEnd);
  size_t img_size = img_end - img_begin;
  ElfL E(img_begin, img_size);
  if (!E.isValidElf()) {
    DP("Unable to get ELF handle: %s!\n", E.getErrmsg(-1));
    return 0;
  }

  uint16_t Type = E.getEType();
  DP("ELF Type: %" PRIu16 "\n", Type);
  return ElfL::isDynType(Type);
}
#endif // INTEL_COLLAB
