#if INTEL_COLLAB
//===-- elf_light.h - Basic ELF functionality -------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Light ELF implementation provides basic ELF reading functionality.
// It may be used in systems without libelf support, if the corresponding
// LLVM ELF implementation is available.
// The interface declared here must be independent of libelf.h/elf.h.
//
// NOTE: we can try to rely on https://github.com/WolfgangSt/libelf
//       on Windows, if this implementation gets more complex.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_OPENMP_LIBOMPTARGET_PLUGINS_COMMON_ELF_COMMON_ELF_LIGHT_H
#define LLVM_OPENMP_LIBOMPTARGET_PLUGINS_COMMON_ELF_COMMON_ELF_LIGHT_H

#include <cstddef>
#include <cstdint>
#include <iterator>

class ElfL;
class ElfLSegmentNoteIterator;
class ElfLSectionNoteIterator;
class ElfNote;

// Class representing NOTEs from PT_NOTE segments and SHT_NOTE sections.
class ElfLNote {
  const void *Impl = nullptr;

  friend class ElfLSegmentNoteIterator;
  friend class ElfLSectionNoteIterator;

  // Only ElfLSectionNoteIterator is allowed to create notes via its
  // operator*().
  explicit ElfLNote(const void *I);
  ElfLNote &operator=(const ElfLNote &) = delete;

public:
  // FIXME: add move copy constructor and assignment operator.
  ElfLNote(const ElfLNote &);
  ~ElfLNote();
  // Returns the note's name size not including the null terminator.
  // Note that it may be illegal to access the getName() pointer
  // beyond the returned size, i.e. the implementation may
  // not guarantee that there is '\0' after getNameSize()
  // characters of the name.
  uint64_t getNameSize() const;
  // Returns a pointer to the beginning of the note's name.
  const char *getName() const;
  // Returns the number of bytes in the descriptor.
  uint64_t getDescSize() const;
  // Returns a pointer to the beginning of the note's descriptor.
  // It is illegal to access more that getDescSize() bytes
  // via this pointer.
  const uint8_t *getDesc() const;
  uint64_t getType() const;
};

// Iterator over NOTEs in PT_NOTE segments.
class ElfLSegmentNoteIterator
    : std::iterator<std::forward_iterator_tag, ElfLNote> {

  void *Impl = nullptr;

  friend class ElfL;

  // Only ElfL is allowed to create iterators to itself.
  ElfLSegmentNoteIterator(const void *I, bool IsEnd = false);
  ElfLSectionNoteIterator &operator=(const ElfLSegmentNoteIterator &) = delete;

public:
  // FIXME: add move copy constructor and assignment operator.
  ElfLSegmentNoteIterator(const ElfLSegmentNoteIterator &Other);
  ~ElfLSegmentNoteIterator();
  ElfLSegmentNoteIterator &operator++();
  bool operator==(const ElfLSegmentNoteIterator Other) const;
  bool operator!=(const ElfLSegmentNoteIterator Other) const;
  ElfLNote operator*() const;
};

// Iterator over NOTEs in SHT_NOTE sections.
class ElfLSectionNoteIterator
    : std::iterator<std::forward_iterator_tag, ElfLNote> {

  void *Impl = nullptr;

  friend class ElfL;

  // Only ElfL is allowed to create iterators to itself.
  ElfLSectionNoteIterator(const void *I, bool IsEnd = false);
  ElfLSectionNoteIterator &operator=(const ElfLSectionNoteIterator &) = delete;

public:
  // FIXME: add move copy constructor and assignment operator.
  ElfLSectionNoteIterator(const ElfLSectionNoteIterator &Other);
  ~ElfLSectionNoteIterator();
  ElfLSectionNoteIterator &operator++();
  bool operator==(const ElfLSectionNoteIterator Other) const;
  bool operator!=(const ElfLSectionNoteIterator Other) const;
  ElfLNote operator*() const;
};

// Wrapper around the given ELF image.
class ElfL {
  // Opaque pointer to the actual implementation.
  void *Impl = nullptr;

  // FIXME: implement if needed.
  ElfL(const ElfL &) = delete;
  ElfL &operator=(const ElfL &) = delete;

public:
  ElfL(char *Begin, size_t Size);
  ~ElfL();
  bool isValidElf() const;
  const char *getErrmsg(int N) const;
  uint16_t getEMachine() const;
  uint16_t getEType() const;

  static bool isDynType(uint16_t Ty);

  ElfLSectionNoteIterator section_notes_begin() const;
  ElfLSectionNoteIterator section_notes_end() const;
  ElfLSegmentNoteIterator segment_notes_begin() const;
  ElfLSegmentNoteIterator segment_notes_end() const;
};

#endif // LLVM_OPENMP_LIBOMPTARGET_PLUGINS_COMMON_ELF_COMMON_ELF_LIGHT_H
#endif // INTEL_COLLAB
