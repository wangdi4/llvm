#if INTEL_CUSTOMIZATION
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
//===-- elf_light.cpp - Basic ELF functionality -----------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "elf_light.h"
#include "Debug.h"
#include <assert.h>

#ifndef TARGET_NAME
#define TARGET_NAME ELF light
#endif
#define DEBUG_PREFIX "TARGET " GETNAME(TARGET_NAME)

#if MAY_USE_LIBELF

// Implementation based on libelf.
#include <gelf.h>
#include <libelf.h>

// Helper methods to align addresses.
template <typename T> inline T alignDown(T value, size_t alignment) {
  return (T)(value & ~(alignment - 1));
}

template <typename T> inline T *alignDown(T *value, size_t alignment) {
  return reinterpret_cast<T *>(alignDown((intptr_t)value, alignment));
}

template <typename T> inline T alignUp(T value, size_t alignment) {
  return alignDown((T)(value + alignment - 1), alignment);
}

template <typename T> inline T *alignUp(T *value, size_t alignment) {
  return reinterpret_cast<T *>(
      alignDown((intptr_t)(value + alignment - 1), alignment));
}

// FIXME: this is taken from openmp/libomptarget/plugins/amdgpu/impl/system.cpp,
//        but it may be incorrect for 64-bit ELF. Elf64_Nhdr and Elf32_Nhdr
//        have different representation. The alignment used for the name and
//        the descriptor is still 4 bytes. At the same time, it seems to work
//        for 64-bit ELFs produced by LLVM.
struct Elf_Note {
  uint32_t n_namesz; // Length of note's name.
  uint32_t n_descsz; // Length of note's value.
  uint32_t n_type;   // Type of note.
  // then name
  // then padding, optional
  // then desc, at 4 byte alignment (not 8, despite being elf64)
};

static const uint32_t NoteAlignment = 4;

// Implementation of the iterator for SHT_NOTE sections.
// The iterator allows processing all NOTEs in all SHT_NOTE sections
// in the ELF image provided during the iterator object construction.
class ElfLSectionNoteIteratorImpl {
  // A pointer to Elf object created by elf_memory() for
  // the ELF image we are going to iterate.
  Elf *EF;

  // A pointer to the current SHT_NOTE section.
  // In the initial state of the iterator object this will actually
  // point to the very first section in the ELF image, but it will be
  // adjusted right away either to point to the first SHT_NOTE section
  // or set to nullptr (if there are no SHT_NOTE sections).
  Elf_Scn *CurrentSection = nullptr;

  // A pointer to the current NOTE inside a SHT_NOTE section
  // pointed by CurrentSection. If it is nullptr, then this means
  // that the iterator object is an end() iterator.
  Elf_Note *NPtr = nullptr;

  uint64_t getNotesBeginAddr(const GElf_Shdr &Shdr) const {
    return reinterpret_cast<uint64_t>(elf_rawfile(EF, nullptr)) +
           Shdr.sh_offset;
  }

  uint64_t getNotesEndAddr(const GElf_Shdr &Shdr) const {
    return getNotesBeginAddr(Shdr) + Shdr.sh_size;
  }

  uint64_t getNoteSize(const Elf_Note &Note) const {
    return sizeof(Note) + alignUp(Note.n_namesz, NoteAlignment) +
           alignUp(Note.n_descsz, NoteAlignment);
  }

  // Given the current state of the iterator object, advances
  // the iterator forward to point to the next NOTE in the next
  // SHT_NOTE section.
  // If there is no such a NOTE, then it sets the iterator
  // object to the end() state.
  //
  // Note that this method does not change the iterator, if
  // NPtr is pointing to a valid note within CurrentSection.
  // The iterator advancement in this case is done via operator++.
  void autoAdvance(bool IsFirst = false) {
    // Cannot advance, if CurrentSection is NULL.
    if (!CurrentSection)
      return;

    // NPtr points to a valid NOTE in CurrentSection, thus,
    // no auto advancement.
    if (NPtr)
      return;

    GElf_Shdr Shdr;
    gelf_getshdr(CurrentSection, &Shdr);

    // CurrentSection is a valid section, and NPtr is an end() iterator.
    //
    // If IsFirst is true, then we just in the initial state, and
    // we need to set CurrentSection to the first SHT_NOTE section (if any),
    // and, then, NPtr to the first note in this section.
    //
    // If IsFirst is false, then we've reached the end of the current
    // SHT_NOTE section, and should find the next section with notes.
    if (!IsFirst || gelf_getshdr(CurrentSection, &Shdr)->sh_type != SHT_NOTE)
      CurrentSection = elf_nextscn(EF, CurrentSection);

    while (CurrentSection &&
           gelf_getshdr(CurrentSection, &Shdr)->sh_type != SHT_NOTE)
      CurrentSection = elf_nextscn(EF, CurrentSection);

    if (!CurrentSection) {
      // No more sections.
      // Note that NPtr is already nullptr indicating the end() iterator.
      return;
    }

    gelf_getshdr(CurrentSection, &Shdr);
    uint64_t NotesBegin = getNotesBeginAddr(Shdr);
    uint64_t NotesEnd = getNotesEndAddr(Shdr);
    if (NotesBegin >= NotesEnd) {
      // Something went wrong. Assume that we've reached
      // the end of all notes.
      CurrentSection = nullptr;
      NPtr = nullptr;
      return;
    }

    NPtr = reinterpret_cast<Elf_Note *>(NotesBegin);
    assert(NPtr && "Invalid SHT_NOTE section.");
  }

  bool operator!=(const ElfLSectionNoteIteratorImpl Other) const {
    return !(*this == Other);
  }

public:
  ElfLSectionNoteIteratorImpl(Elf *RawElf, bool IsEnd = false) : EF(RawElf) {
    assert(EF && "Trying to iterate invalid ELF.");

    if (IsEnd) {
      // NPtr equal to nullptr means end() iterator.
      return;
    }

    // Set CurrentSection to the very first section,
    // and let autoAdvance() find the first valid note (if any).
    CurrentSection = elf_getscn(EF, 0);
    autoAdvance(true);
  }

  bool operator==(const ElfLSectionNoteIteratorImpl Other) const {
    // They should be pointing to the same NOTE to be equal.
    return NPtr == Other.NPtr;
  }

  const Elf_Note *operator*() const {
    assert(*this != ElfLSectionNoteIteratorImpl(EF, true) &&
           "Dereferencing the end iterator.");
    return NPtr;
  }

  // Advance to the next NOTE in the CurrentSection.
  // If there is no next NOTE, then autoAdvance() to the next
  // SHT_NOTE section and its first NOTE.
  ElfLSectionNoteIteratorImpl &operator++() {
    assert(*this != ElfLSectionNoteIteratorImpl(EF, true) &&
           "Incrementing the end iterator.");

    GElf_Shdr Shdr;
    gelf_getshdr(CurrentSection, &Shdr);
    uint64_t NotesBegin = getNotesBeginAddr(Shdr);
    uint64_t NotesEnd = getNotesEndAddr(Shdr);
    assert(reinterpret_cast<uint64_t>(NPtr) >= NotesBegin &&
           reinterpret_cast<uint64_t>(NPtr) < NotesEnd &&
           "Invalid pointer to a note computed somewhere else.");
    (void)NotesBegin;

    uint64_t NoteSize = getNoteSize(*NPtr);
    NPtr =
        reinterpret_cast<Elf_Note *>(reinterpret_cast<char *>(NPtr) + NoteSize);
    if (reinterpret_cast<uint64_t>(NPtr) >= NotesEnd ||
        reinterpret_cast<uint64_t>(NPtr) + sizeof(*NPtr) >= NotesEnd) {
      // We've reached the end of the current NOTE section.
      NPtr = nullptr;
    }

    // Auto advance to the next section, if needed.
    autoAdvance();
    return *this;
  }
};

// Implementation of the iterator for PT_NOTE segments.
// The iterator allows processing all NOTEs in all PT_NOTE segments
// in the ELF image provided during the iterator object construction.
class ElfLSegmentNoteIteratorImpl {
  // A pointer to Elf object created by elf_memory() for
  // the ELF image we are going to iterate.
  Elf *EF;

  // A pointer to the current PT_NOTE segment.
  // In the initial state of the iterator object this will actually
  // point to the very first segment in the ELF image, but it will be
  // adjusted right away either to point to the first PT_NOTE segment
  // or set to nullptr (if there are no PT_NOTE segments).
  size_t NumberOfPhdrs = (std::numeric_limits<size_t>::max)();
  size_t CurrentSegment = (std::numeric_limits<size_t>::max)();

  // A pointer to the current NOTE inside a PT_NOTE segment
  // pointed by CurrentSegment. If it is nullptr, then this means
  // that the iterator object is an end() iterator.
  Elf_Note *NPtr = nullptr;

  uint64_t getNotesBeginAddr(const GElf_Phdr &Phdr) const {
    return reinterpret_cast<uint64_t>(elf_rawfile(EF, nullptr)) + Phdr.p_offset;
  }

  uint64_t getNotesEndAddr(const GElf_Phdr &Phdr) const {
    return getNotesBeginAddr(Phdr) + Phdr.p_filesz;
  }

  uint64_t getNoteSize(const Elf_Note &Note) const {
    return sizeof(Note) + alignUp(Note.n_namesz, NoteAlignment) +
           alignUp(Note.n_descsz, NoteAlignment);
  }

  // Given the current state of the iterator object, advances
  // the iterator forward to point to the next NOTE in the next
  // PT_NOTE segment.
  // If there is no such a NOTE, then it sets the iterator
  // object to the end() state.
  //
  // Note that this method does not change the iterator, if
  // NPtr is pointing to a valid note within CurrentSegment.
  // The iterator advancement in this case is done via operator++.
  void autoAdvance(bool IsFirst = false) {
    // Cannot advance, if CurrentSegment is invalid.
    if (CurrentSegment >= NumberOfPhdrs)
      return;

    // NPtr points to a valid NOTE in CurrentSegment, thus,
    // no auto advancement.
    if (NPtr)
      return;

    GElf_Phdr Phdr;
    gelf_getphdr(EF, CurrentSegment, &Phdr);

    // CurrentSegment is a valid segment, and NPtr is an end() iterator.
    //
    // If IsFirst is true, then we just in the initial state, and
    // we need to set CurrentSegment to the first PT_NOTE segment (if any),
    // and, then, NPtr to the first note in this segment.
    //
    // If IsFirst is false, then we've reached the end of the current
    // PT_NOTE segment, and should find the next segment with notes.
    if (!IsFirst || Phdr.p_type != PT_NOTE)
      ++CurrentSegment;

    while (CurrentSegment < NumberOfPhdrs) {
      if (gelf_getphdr(EF, CurrentSegment, &Phdr) != &Phdr)
        continue;

      if (Phdr.p_type == PT_NOTE)
        break;

      ++CurrentSegment;
    }

    if (CurrentSegment >= NumberOfPhdrs) {
      // No more segments.
      // Note that NPtr is already nullptr indicating the end() iterator.
      return;
    }

    if (gelf_getphdr(EF, CurrentSegment, &Phdr) != &Phdr)
      assert(false && "Invalid program header selected above.");

    uint64_t NotesBegin = getNotesBeginAddr(Phdr);
    uint64_t NotesEnd = getNotesEndAddr(Phdr);
    if (NotesBegin >= NotesEnd) {
      // Something went wrong. Assume that we've reached
      // the end of all notes.
      CurrentSegment = NumberOfPhdrs;
      NPtr = nullptr;
      return;
    }

    NPtr = reinterpret_cast<Elf_Note *>(NotesBegin);
    assert(NPtr && "Invalid PT_NOTE segment.");
  }

  bool operator!=(const ElfLSegmentNoteIteratorImpl Other) const {
    return !(*this == Other);
  }

public:
  ElfLSegmentNoteIteratorImpl(Elf *RawElf, bool IsEnd = false) : EF(RawElf) {
    assert(EF && "Trying to iterate invalid ELF.");

    if (IsEnd) {
      // NPtr equal to nullptr means end() iterator.
      return;
    }

    // Set CurrentSegment to the very first segment,
    // and let autoAdvance() find the first valid note (if any).
    CurrentSegment = 0;

    // Set NumberOfPhdrs to 0, if we cannot query it.
    if (elf_getphdrnum(EF, &NumberOfPhdrs) != 0)
      NumberOfPhdrs = 0;
    autoAdvance(true);
  }

  bool operator==(const ElfLSegmentNoteIteratorImpl Other) const {
    // They should be pointing to the same NOTE to be equal.
    return NPtr == Other.NPtr;
  }

  const Elf_Note *operator*() const {
    assert(*this != ElfLSegmentNoteIteratorImpl(EF, true) &&
           "Dereferencing the end iterator.");
    return NPtr;
  }

  // Advance to the next NOTE in the CurrentSegment.
  // If there is no next NOTE, then autoAdvance() to the next
  // PT_NOTE segment and its first NOTE.
  ElfLSegmentNoteIteratorImpl &operator++() {
    assert(*this != ElfLSegmentNoteIteratorImpl(EF, true) &&
           "Incrementing the end iterator.");

    GElf_Phdr Phdr;
    gelf_getphdr(EF, CurrentSegment, &Phdr);
    uint64_t NotesBegin = getNotesBeginAddr(Phdr);
    uint64_t NotesEnd = getNotesEndAddr(Phdr);
    assert(reinterpret_cast<uint64_t>(NPtr) >= NotesBegin &&
           reinterpret_cast<uint64_t>(NPtr) < NotesEnd &&
           "Invalid pointer to a note computed somewhere else.");
    (void)NotesBegin;

    uint64_t NoteSize = getNoteSize(*NPtr);
    NPtr =
        reinterpret_cast<Elf_Note *>(reinterpret_cast<char *>(NPtr) + NoteSize);
    if (reinterpret_cast<uint64_t>(NPtr) >= NotesEnd ||
        reinterpret_cast<uint64_t>(NPtr) + sizeof(*NPtr) >= NotesEnd) {
      // We've reached the end of the current NOTE section.
      NPtr = nullptr;
    }

    // Auto advance to the next section, if needed.
    autoAdvance();
    return *this;
  }
};

class ElfLSectionImpl {
  Elf *EF = nullptr;
  Elf_Scn *Section = nullptr;

public:
  ElfLSectionImpl(Elf *EF, Elf_Scn *Section) : EF(EF), Section(Section) {}

  const char *getName() const {
    size_t SHStrNdx;
    if (elf_getshdrstrndx(EF, &SHStrNdx) != 0)
      return "";

    GElf_Shdr Shdr;
    gelf_getshdr(Section, &Shdr);
    char *Name = elf_strptr(EF, SHStrNdx, static_cast<size_t>(Shdr.sh_name));
    return Name ? Name : "";
  }

  uint64_t getSize() const {
    Elf_Data *Desc = elf_rawdata(Section, nullptr);
    if (!Desc)
      return 0;

    return Desc->d_size;
  }

  const uint8_t *getContents() const {
    Elf_Data *Desc = elf_rawdata(Section, nullptr);
    if (!Desc)
      return 0;

    return reinterpret_cast<const uint8_t *>(Desc->d_buf);
  }
};

class ElfLSectionIteratorImpl {
  // A pointer to Elf object created by elf_memory() for
  // the ELF image we are going to iterate.
  Elf *EF = nullptr;

  // A pointer to the current section.
  Elf_Scn *CurrentSection = nullptr;

  bool operator!=(const ElfLSectionIteratorImpl Other) const {
    return !(*this == Other);
  }

public:
  ElfLSectionIteratorImpl(Elf *RawElf, bool IsEnd = false) : EF(RawElf) {
    assert(EF && "Trying to iterate invalid ELF.");
    if (IsEnd)
      return;

    CurrentSection = elf_getscn(EF, 0);
  }

  bool operator==(const ElfLSectionIteratorImpl Other) const {
    return CurrentSection == Other.CurrentSection;
  }

  ElfLSectionImpl *operator*() const {
    assert(*this != ElfLSectionIteratorImpl(EF, true) &&
           "Dereferencing the end iterator.");
    return new ElfLSectionImpl(EF, CurrentSection);
  }

  ElfLSectionIteratorImpl &operator++() {
    assert(*this != ElfLSectionIteratorImpl(EF, true) &&
           "Dereferencing the end iterator.");

    CurrentSection = elf_nextscn(EF, CurrentSection);
    return *this;
  }
};

// Actual implementation of ElfL via libelf.
// It is constructed from an ELF image defined by its
// starting pointer in memory and a length in bytes.
class ElfLImpl {
  // A pointer to Elf object created by elf_memory() for
  // the ELF image.
  Elf *EF = nullptr;

  // Class of the ELF image.
  unsigned ElfClass = ELFCLASSNONE;

  // A pointer to the ELF image's header.
  // Depending on the class it may be either 'Elf32_Ehdr *'
  // or 'Elf64_Ehdr '.
  const void *Header = nullptr;

  // Let the owning object access this.
  friend class ElfL;

public:
  ElfLImpl(Elf *RawElf, unsigned ElfClass, const void *Header)
      : EF(RawElf), ElfClass(ElfClass), Header(Header) {}

  // Allocates and constructs a new iterator for NOTEs in
  // SHT_NOTE sections of the ELF image.
  ElfLSectionNoteIteratorImpl *
  createSectionNoteIteratorImpl(bool IsEnd) const {
    return new ElfLSectionNoteIteratorImpl(EF, IsEnd);
  }

  // Allocates and constructs a new iterator for NOTEs in
  // PT_NOTE segments of the ELF image.
  ElfLSegmentNoteIteratorImpl *
  createSegmentNoteIteratorImpl(bool IsEnd) const {
    return new ElfLSegmentNoteIteratorImpl(EF, IsEnd);
  }

  ElfLSectionIteratorImpl *
  createSectionIteratorImpl(bool IsEnd) const {
    return new ElfLSectionIteratorImpl(EF, IsEnd);
  }
};

ElfL::ElfL(char *Begin, size_t Size) {
  Elf *ElfHandle = elf_memory(Begin, Size);
  if (!ElfHandle) {
    elf_end(ElfHandle);
    return;
  }

  const Elf32_Ehdr *Header32 = elf32_getehdr(ElfHandle);
  const Elf64_Ehdr *Header64 = elf64_getehdr(ElfHandle);

  if (!Header32 == !Header64) {
    // Ambiguous ELF header or unrecognized ELF image.
    elf_end(ElfHandle);
    return;
  }

  const void *Header = nullptr;
  unsigned ElfClass = ELFCLASSNONE;

  if (Header32) {
    ElfClass = ELFCLASS32;
    Header = reinterpret_cast<const void *>(Header32);
  } else {
    ElfClass = ELFCLASS64;
    Header = reinterpret_cast<const void *>(Header64);
  }

  Impl = reinterpret_cast<void *>(new ElfLImpl(ElfHandle, ElfClass, Header));
}

ElfL::~ElfL() {
  if (Impl) {
    ElfLImpl *EImpl = reinterpret_cast<ElfLImpl *>(Impl);
    elf_end(EImpl->EF);
    delete EImpl;
  }
}

bool ElfL::isValidElf() const {
  ElfLImpl *EImpl = reinterpret_cast<ElfLImpl *>(Impl);
  return Impl && EImpl->Header && EImpl->ElfClass != ELFCLASSNONE;
}

const char *ElfL::getErrmsg(int N) const { return elf_errmsg(-1); }

uint16_t ElfL::getEMachine() const {
  assert(isValidElf() && "Invalid ELF.");
  ElfLImpl *EImpl = reinterpret_cast<ElfLImpl *>(Impl);
  if (EImpl->ElfClass == ELFCLASS32)
    return reinterpret_cast<const Elf32_Ehdr *>(EImpl->Header)->e_machine;
  else if (EImpl->ElfClass == ELFCLASS64)
    return reinterpret_cast<const Elf64_Ehdr *>(EImpl->Header)->e_machine;
  else
    assert(false && "Unsupported ELF class.");

  return EM_NONE;
}

uint16_t ElfL::getEType() const {
  assert(isValidElf() && "Invalid ELF.");
  ElfLImpl *EImpl = reinterpret_cast<ElfLImpl *>(Impl);
  if (EImpl->ElfClass == ELFCLASS32)
    return reinterpret_cast<const Elf32_Ehdr *>(EImpl->Header)->e_type;
  else if (EImpl->ElfClass == ELFCLASS64)
    return reinterpret_cast<const Elf64_Ehdr *>(EImpl->Header)->e_type;
  else
    assert(false && "Unsupported ELF class.");

  return ET_NONE;
}

bool ElfL::isDynType(uint16_t Ty) { return Ty == ET_DYN; }

ElfLSectionNoteIterator ElfL::section_notes_begin() const {
  return ElfLSectionNoteIterator(reinterpret_cast<const ElfLImpl *>(Impl));
}

ElfLSectionNoteIterator ElfL::section_notes_end() const {
  return ElfLSectionNoteIterator(reinterpret_cast<const ElfLImpl *>(Impl),
                                 true);
}

ElfLSectionNoteIterator::ElfLSectionNoteIterator(const void *I, bool IsEnd) {
  const ElfLImpl *EImpl = reinterpret_cast<const ElfLImpl *>(I);
  Impl = EImpl->createSectionNoteIteratorImpl(IsEnd);
}

ElfLSectionNoteIterator::ElfLSectionNoteIterator(
    const ElfLSectionNoteIterator &Other) {
  ElfLSectionNoteIteratorImpl *IImpl =
      reinterpret_cast<ElfLSectionNoteIteratorImpl *>(Other.Impl);
  Impl = new ElfLSectionNoteIteratorImpl(*IImpl);
}

ElfLSectionNoteIterator::~ElfLSectionNoteIterator() {
  assert(Impl && "Invalid ElfLSectionNoteIterator object.");
  ElfLSectionNoteIteratorImpl *IImpl =
      reinterpret_cast<ElfLSectionNoteIteratorImpl *>(Impl);
  delete IImpl;
}

bool ElfLSectionNoteIterator::operator==(
    const ElfLSectionNoteIterator Other) const {
  const ElfLSectionNoteIteratorImpl *Lhs =
      reinterpret_cast<const ElfLSectionNoteIteratorImpl *>(Impl);
  const ElfLSectionNoteIteratorImpl *Rhs =
      reinterpret_cast<const ElfLSectionNoteIteratorImpl *>(Other.Impl);
  return (*Lhs == *Rhs);
}

bool ElfLSectionNoteIterator::operator!=(
    const ElfLSectionNoteIterator Other) const {
  return !(*this == Other);
}

ElfLSectionNoteIterator &ElfLSectionNoteIterator::operator++() {
  ElfLSectionNoteIteratorImpl *IImpl =
      reinterpret_cast<ElfLSectionNoteIteratorImpl *>(Impl);
  ++(*IImpl);
  return *this;
}

ElfLNote ElfLSectionNoteIterator::operator*() const {
  ElfLSectionNoteIteratorImpl *IImpl =
      reinterpret_cast<ElfLSectionNoteIteratorImpl *>(Impl);
  return ElfLNote(**IImpl);
}

ElfLSegmentNoteIterator ElfL::segment_notes_begin() const {
  return ElfLSegmentNoteIterator(reinterpret_cast<const ElfLImpl *>(Impl));
}

ElfLSegmentNoteIterator ElfL::segment_notes_end() const {
  return ElfLSegmentNoteIterator(reinterpret_cast<const ElfLImpl *>(Impl),
                                 true);
}

ElfLSegmentNoteIterator::ElfLSegmentNoteIterator(const void *I, bool IsEnd) {
  const ElfLImpl *EImpl = reinterpret_cast<const ElfLImpl *>(I);
  Impl = EImpl->createSegmentNoteIteratorImpl(IsEnd);
}

ElfLSegmentNoteIterator::ElfLSegmentNoteIterator(
    const ElfLSegmentNoteIterator &Other) {
  ElfLSegmentNoteIteratorImpl *IImpl =
      reinterpret_cast<ElfLSegmentNoteIteratorImpl *>(Other.Impl);
  Impl = new ElfLSegmentNoteIteratorImpl(*IImpl);
}

ElfLSegmentNoteIterator::~ElfLSegmentNoteIterator() {
  assert(Impl && "Invalid ElfLSegmentNoteIterator object.");
  ElfLSegmentNoteIteratorImpl *IImpl =
      reinterpret_cast<ElfLSegmentNoteIteratorImpl *>(Impl);
  delete IImpl;
}

bool ElfLSegmentNoteIterator::operator==(
    const ElfLSegmentNoteIterator Other) const {
  const ElfLSegmentNoteIteratorImpl *Lhs =
      reinterpret_cast<const ElfLSegmentNoteIteratorImpl *>(Impl);
  const ElfLSegmentNoteIteratorImpl *Rhs =
      reinterpret_cast<const ElfLSegmentNoteIteratorImpl *>(Other.Impl);
  return (*Lhs == *Rhs);
}

bool ElfLSegmentNoteIterator::operator!=(
    const ElfLSegmentNoteIterator Other) const {
  return !(*this == Other);
}

ElfLSegmentNoteIterator &ElfLSegmentNoteIterator::operator++() {
  ElfLSegmentNoteIteratorImpl *IImpl =
      reinterpret_cast<ElfLSegmentNoteIteratorImpl *>(Impl);
  ++(*IImpl);
  return *this;
}

ElfLNote ElfLSegmentNoteIterator::operator*() const {
  ElfLSegmentNoteIteratorImpl *IImpl =
      reinterpret_cast<ElfLSegmentNoteIteratorImpl *>(Impl);
  return ElfLNote(**IImpl);
}

ElfLNote::ElfLNote(const void *I) {
  // ElfLNote::Impl is a pointer to Elf_Note in this implementation.
  // A pointer to Elf_Note is returned by
  // ElfLSectionNoteIteratorImpl::operator*().
  Impl = I;
}

ElfLNote::ElfLNote(const ElfLNote &Other) { Impl = Other.Impl; }

ElfLNote::~ElfLNote() {}

uint64_t ElfLNote::getNameSize() const {
  const Elf_Note *Note = reinterpret_cast<const Elf_Note *>(Impl);
  if (Note->n_namesz == 0)
    return 0;
  // libelf returns name size that accounts for the null terminator.
  // ELF light interface returns the size ignoring it.
  return Note->n_namesz - 1;
}

const char *ElfLNote::getName() const {
  const Elf_Note *Note = reinterpret_cast<const Elf_Note *>(Impl);
  return reinterpret_cast<const char *>(Note) + sizeof(*Note);
}

uint64_t ElfLNote::getDescSize() const {
  const Elf_Note *Note = reinterpret_cast<const Elf_Note *>(Impl);
  return Note->n_descsz;
}

const uint8_t *ElfLNote::getDesc() const {
  const Elf_Note *Note = reinterpret_cast<const Elf_Note *>(Impl);
  return reinterpret_cast<const uint8_t *>(Note) + sizeof(*Note) +
         alignUp(getNameSize(), NoteAlignment);
}

uint64_t ElfLNote::getType() const {
  const Elf_Note *Note = reinterpret_cast<const Elf_Note *>(Impl);
  return Note->n_type;
}

ElfLSection::ElfLSection(const void *I) {
  Impl = I;
}

ElfLSection::ElfLSection(const ElfLSection &Other) {
  const ElfLSectionImpl *SImpl =
      reinterpret_cast<const ElfLSectionImpl *>(Other.Impl);
  Impl = new ElfLSectionImpl(*SImpl);
}

ElfLSection::~ElfLSection() {
  const ElfLSectionImpl *SImpl =
      reinterpret_cast<const ElfLSectionImpl *>(Impl);
  delete SImpl;
}

const char *ElfLSection::getName() const {
  const ElfLSectionImpl *SImpl =
      reinterpret_cast<const ElfLSectionImpl *>(Impl);
  return SImpl->getName();
}

uint64_t ElfLSection::getSize() const {
  const ElfLSectionImpl *SImpl =
      reinterpret_cast<const ElfLSectionImpl *>(Impl);
  return SImpl->getSize();
}

const uint8_t *ElfLSection::getContents() const {
  const ElfLSectionImpl *SImpl =
      reinterpret_cast<const ElfLSectionImpl *>(Impl);
  return SImpl->getContents();
}

ElfLSectionIterator ElfL::sections_begin() const {
  return ElfLSectionIterator(Impl);
}

ElfLSectionIterator ElfL::sections_end() const {
  return ElfLSectionIterator(Impl, true);
}

ElfLSectionIterator::ElfLSectionIterator(const void *I, bool IsEnd) {
  const ElfLImpl *EImpl = reinterpret_cast<const ElfLImpl *>(I);
  Impl = EImpl->createSectionIteratorImpl(IsEnd);
}

ElfLSectionIterator::ElfLSectionIterator(
    const ElfLSectionIterator &Other) {
  ElfLSectionIteratorImpl *IImpl =
      reinterpret_cast<ElfLSectionIteratorImpl *>(Other.Impl);
  Impl = new ElfLSectionIteratorImpl(*IImpl);
}

ElfLSectionIterator::~ElfLSectionIterator() {
  assert(Impl && "Invalid ElfLSectionIterator object.");
  ElfLSectionIteratorImpl *IImpl =
      reinterpret_cast<ElfLSectionIteratorImpl *>(Impl);
  delete IImpl;
}

bool ElfLSectionIterator::operator==(
    const ElfLSectionIterator Other) const {
  const ElfLSectionIteratorImpl *Lhs =
      reinterpret_cast<const ElfLSectionIteratorImpl *>(Impl);
  const ElfLSectionIteratorImpl *Rhs =
      reinterpret_cast<const ElfLSectionIteratorImpl *>(Other.Impl);
  return (*Lhs == *Rhs);
}

bool ElfLSectionIterator::operator!=(
    const ElfLSectionIterator Other) const {
  return !(*this == Other);
}

ElfLSectionIterator &ElfLSectionIterator::operator++() {
  ElfLSectionIteratorImpl *IImpl =
      reinterpret_cast<ElfLSectionIteratorImpl *>(Impl);
  ++(*IImpl);
  return *this;
}

ElfLSection ElfLSectionIterator::operator*() const {
  ElfLSectionIteratorImpl *IImpl =
      reinterpret_cast<ElfLSectionIteratorImpl *>(Impl);
  return ElfLSection(**IImpl);
}
#else // !MAY_USE_LIBELF

// Implementation based on LLVM ELF binary format.
#include "llvm/Object/Binary.h"
#include "llvm/Object/ELFObjectFile.h"
#include "llvm/Object/ELFTypes.h"
#include "llvm/Object/ObjectFile.h"
#include "llvm/Support/MemoryBuffer.h"

using namespace llvm;
using namespace llvm::ELF;
using namespace llvm::object;

class ElfLNoteImplBase {
public:
  virtual ~ElfLNoteImplBase() = default;
  virtual ElfLNoteImplBase *clone() const = 0;
  virtual size_t getNameSize() const = 0;
  virtual const char *getName() const = 0;
  virtual size_t getDescSize() const = 0;
  virtual const uint8_t *getDesc() const = 0;
  virtual uint32_t getType() const = 0;
};

template <class ELFT> class ElfLNoteImpl : public ElfLNoteImplBase {
  using Elf_Note = typename ELFT::Note;
  const Elf_Note Note;
  // FIXME: Using 1 as default alignment.
  size_t Align = 1;

public:
  ElfLNoteImpl(const Elf_Note Note) : Note(Note) {}
  ElfLNoteImpl(const Elf_Note Note, size_t Align_) : Note(Note) {
    // FIXME: 0 alignment will hit assertion when calling getDesc().
    if (Align_ > 0)
      Align = Align_;
  }
  ElfLNoteImpl(const ElfLNoteImpl &) = default;
  ElfLNoteImplBase *clone() const override { return new ElfLNoteImpl(*this); }
  ~ElfLNoteImpl() = default;
  size_t getNameSize() const override { return Note.getName().size(); }
  const char *getName() const override { return Note.getName().data(); }
  size_t getDescSize() const override { return Note.getDesc(Align).size(); }
  const uint8_t *getDesc() const override {
    // FIXME: For some reason, the returned descriptor array is off by one
    // while the base address has '\0'.
    return Note.getDesc(Align).data() + 1;
  }
  uint32_t getType() const override { return Note.getType(); }
};

class ElfLNoteIteratorImplBase {
protected:
  const endianness TargetEndianness;
  const bool Is64Bits;
  const bool IsSectionIterator;

  ElfLNoteIteratorImplBase(endianness TargetEndianness, bool Is64Bits,
                           bool IsSectionIterator)
      : TargetEndianness(TargetEndianness), Is64Bits(Is64Bits),
        IsSectionIterator(IsSectionIterator) {}

public:
  ElfLNoteIteratorImplBase(const ElfLNoteIteratorImplBase &) = default;
  virtual ~ElfLNoteIteratorImplBase() = default;
  virtual ElfLNoteIteratorImplBase *clone() const = 0;
  virtual ElfLNoteIteratorImplBase &operator++() = 0;
  virtual bool operator==(const ElfLNoteIteratorImplBase &) const = 0;
  virtual ElfLNoteImplBase *operator*() const = 0;

  endianness getEndianness() const { return TargetEndianness; }

  bool is64Bits() const { return Is64Bits; }

  bool isSectionIterator() const { return IsSectionIterator; }
};

template <class ELFT>
class ElfLNoteIteratorImpl : public ElfLNoteIteratorImplBase {
protected:
  using NoteIterator = typename ELFT::NoteIterator;

  const ELFFile<ELFT> &EF;
  NoteIterator NotesIt;
  Error &Err;

  explicit ElfLNoteIteratorImpl(const ELFFile<ELFT> &EF, Error &Err,
                                bool IsSectionIterator)
      : ElfLNoteIteratorImplBase(ELFT::TargetEndianness, ELFT::Is64Bits,
                                 IsSectionIterator),
        EF(EF), NotesIt(EF.notes_end()), Err(Err) {}

public:
  ElfLNoteIteratorImpl(const ElfLNoteIteratorImpl &) = default;
  virtual ~ElfLNoteIteratorImpl() = default;

  static bool classof(const ElfLNoteIteratorImplBase *B) {
    return (B->getEndianness() == ELFT::TargetEndianness &&
            B->is64Bits() == ELFT::Is64Bits);
  }
};

template <class ELFT>
class ElfLSectionNoteIteratorImpl : public ElfLNoteIteratorImpl<ELFT> {
  using Elf_Shdr = typename ELFT::Shdr;
  using Elf_Shdr_Range = typename ELFT::ShdrRange;
  using NoteIterator = typename ElfLNoteIteratorImpl<ELFT>::NoteIterator;
  using SectionsIteratorTy = typename Elf_Shdr_Range::iterator;

  SectionsIteratorTy SectionsIt;

  const ELFFile<ELFT> &getEF() const { return this->EF; }
  const NoteIterator &getNotesIt() const { return this->NotesIt; }
  Error &getErr() const { return this->Err; }
  NoteIterator &getNotesIt() { return this->NotesIt; }
  SectionsIteratorTy section_begin() const {
    Expected<Elf_Shdr_Range> Sections = getEF().sections();
    if (!Sections)
      return SectionsIteratorTy();

    return Sections->begin();
  }

  SectionsIteratorTy section_end() const {
    Expected<Elf_Shdr_Range> Sections = getEF().sections();
    if (!Sections)
      return SectionsIteratorTy();

    return Sections->end();
  }

  bool isEqual(const ElfLSectionNoteIteratorImpl &Lhs,
               const ElfLSectionNoteIteratorImpl &Rhs) const {
    // Check for end() iterators, first.
    if (Lhs.SectionsIt == section_end() && Rhs.SectionsIt == section_end())
      return true;

    if (Lhs.SectionsIt != Rhs.SectionsIt)
      return false;

    return Lhs.getNotesIt() == Rhs.getNotesIt();
  }

  void autoAdvance(bool IsFirst = false) {
    if (SectionsIt == section_end())
      return;

    if (getNotesIt() != getEF().notes_end())
      return;

    // SectionsIt is not an end iterator, and NotesIt is an end()
    // iterator.
    //
    // If IsFirst is true, then we just in the initial state, and
    // we need to set SectionsIt to the first SHT_NOTE section (if any),
    // and, then, NotesIt to the first note in this section.
    //
    // If IsFirst is false, then we've reached the end of the current
    // SHT_NOTE section, and should find the next section with notes.
    if (!IsFirst || SectionsIt->sh_type != ELF::SHT_NOTE)
      ++SectionsIt;

    while (SectionsIt != section_end() &&
           SectionsIt->sh_type != ELF::SHT_NOTE) {
      ++SectionsIt;
    }

    if (SectionsIt == section_end()) {
      // No more sections.
      return;
    }

    const Elf_Shdr &Section = *SectionsIt;
    getNotesIt() = getEF().notes_begin(Section, getErr());

    // Auto advance the iterator, if the NOTE section
    // does not contain any notes (e.g. some error happened
    // during the note parsing).
    autoAdvance();
  }

  bool operator!=(const ElfLSectionNoteIteratorImpl &Other) const {
    return !(*this == Other);
  }

public:
  ElfLSectionNoteIteratorImpl(const ELFFile<ELFT> &EF, Error &Err,
                              bool IsEnd = false)
      : ElfLNoteIteratorImpl<ELFT>(EF, Err, true) {
    if (IsEnd) {
      SectionsIt = section_end();
      // It is an end() iterator, if SectionsIt is an end() iterator.
      return;
    }

    SectionsIt = section_begin();
    autoAdvance(true);
  }

  ElfLSectionNoteIteratorImpl(const ElfLSectionNoteIteratorImpl &Copy) =
      default;

  ElfLNoteIteratorImplBase *clone() const override {
    return new ElfLSectionNoteIteratorImpl(*this);
  }

  bool operator==(const ElfLNoteIteratorImplBase &Other) const override {
    if (const ElfLSectionNoteIteratorImpl *OPtr =
            dyn_cast<const ElfLSectionNoteIteratorImpl>(&Other)) {
      return isEqual(*this, *OPtr);
    }
    return false;
  }

  ElfLSectionNoteIteratorImpl &operator++() override {
    assert(*this != ElfLSectionNoteIteratorImpl(getEF(), getErr(), true) &&
           "Incrementing the end iterator.");
    // Move the notes iterator within the current section.
    ++getNotesIt();
    autoAdvance();

    return *this;
  }

  ElfLNoteImplBase *operator*() const override {
    assert(*this != ElfLSectionNoteIteratorImpl(getEF(), getErr(), true) &&
           "Dereferencing the end iterator.");
    return new ElfLNoteImpl<ELFT>(*getNotesIt(), SectionsIt->sh_addralign);
  }

  static bool classof(const ElfLNoteIteratorImplBase *B) {
    return (ElfLNoteIteratorImpl<ELFT>::classof(B) &&
            B->isSectionIterator() == true);
  }
};

template <class ELFT>
class ElfLSegmentNoteIteratorImpl : public ElfLNoteIteratorImpl<ELFT> {
  using Elf_Phdr = typename ELFT::Phdr;
  using Elf_Phdr_Range = typename ELFT::PhdrRange;
  using NoteIterator = typename ElfLNoteIteratorImpl<ELFT>::NoteIterator;
  using SegmentIteratorTy = typename Elf_Phdr_Range::iterator;

  SegmentIteratorTy SegmentsIt;

  const ELFFile<ELFT> &getEF() const { return this->EF; }
  const NoteIterator &getNotesIt() const { return this->NotesIt; }
  Error &getErr() const { return this->Err; }
  NoteIterator &getNotesIt() { return this->NotesIt; }
  SegmentIteratorTy segment_begin() const {
    Expected<Elf_Phdr_Range> Segments = getEF().program_headers();
    if (!Segments)
      return SegmentIteratorTy();

    return Segments->begin();
  }

  SegmentIteratorTy segment_end() const {
    Expected<Elf_Phdr_Range> Segments = getEF().program_headers();
    if (!Segments)
      return SegmentIteratorTy();

    return Segments->end();
  }

  bool isEqual(const ElfLSegmentNoteIteratorImpl &Lhs,
               const ElfLSegmentNoteIteratorImpl &Rhs) const {
    // Check for end() iterators, first.
    if (Lhs.SegmentsIt == segment_end() && Rhs.SegmentsIt == segment_end())
      return true;

    if (Lhs.SegmentsIt != Rhs.SegmentsIt)
      return false;

    return Lhs.getNotesIt() == Rhs.getNotesIt();
  }

  void autoAdvance(bool IsFirst = false) {
    if (SegmentsIt == segment_end())
      return;

    if (getNotesIt() != getEF().notes_end())
      return;

    // SegmentsIt is not an end iterator, and NotesIt is an end()
    // iterator.
    //
    // If IsFirst is true, then we just in the initial state, and
    // we need to set SegmentsIt to the first PT_NOTE segment (if any),
    // and, then, NotesIt to the first note in this segment.
    //
    // If IsFirst is false, then we've reached the end of the current
    // PT_NOTE segment, and should find the next segment with notes.
    if (!IsFirst || SegmentsIt->p_type != ELF::SHT_NOTE)
      ++SegmentsIt;

    while (SegmentsIt != segment_end() && SegmentsIt->p_type != ELF::PT_NOTE) {
      ++SegmentsIt;
    }

    if (SegmentsIt == segment_end()) {
      // No more segments.
      return;
    }

    const Elf_Phdr &Segment = *SegmentsIt;
    getNotesIt() = getEF().notes_begin(Segment, getErr());

    // Auto advance the iterator, if the NOTE segment
    // does not contain any notes (e.g. some error happened
    // during the note parsing).
    autoAdvance();
  }

  bool operator!=(const ElfLSegmentNoteIteratorImpl &Other) const {
    return !(*this == Other);
  }

public:
  ElfLSegmentNoteIteratorImpl(const ELFFile<ELFT> &EF, Error &Err,
                              bool IsEnd = false)
      : ElfLNoteIteratorImpl<ELFT>(EF, Err, false) {
    if (IsEnd) {
      SegmentsIt = segment_end();
      // It is an end() iterator, if SegmentsIt is an end() iterator.
      return;
    }

    SegmentsIt = segment_begin();
    autoAdvance(true);
  }

  ElfLSegmentNoteIteratorImpl(const ElfLSegmentNoteIteratorImpl &Copy) =
      default;

  ElfLNoteIteratorImplBase *clone() const override {
    return new ElfLSegmentNoteIteratorImpl(*this);
  }

  bool operator==(const ElfLNoteIteratorImplBase &Other) const override {
    if (const ElfLSegmentNoteIteratorImpl *OPtr =
            dyn_cast<const ElfLSegmentNoteIteratorImpl>(&Other)) {
      return isEqual(*this, *OPtr);
    }
    return false;
  }

  ElfLSegmentNoteIteratorImpl &operator++() override {
    assert(*this != ElfLSegmentNoteIteratorImpl(getEF(), getErr(), true) &&
           "Incrementing the end iterator.");
    // Move the notes iterator within the current segment.
    ++getNotesIt();
    autoAdvance();

    return *this;
  }

  ElfLNoteImplBase *operator*() const override {
    assert(*this != ElfLSegmentNoteIteratorImpl(getEF(), getErr(), true) &&
           "Dereferencing the end iterator.");
    return new ElfLNoteImpl<ELFT>(*getNotesIt(), SegmentsIt->p_align);
  }

  static bool classof(const ElfLNoteIteratorImplBase *B) {
    return (ElfLNoteIteratorImpl<ELFT>::classof(B) &&
            B->isSectionIterator() == false);
  }
};

class ElfLSectionImplBase {
public:
  virtual ~ElfLSectionImplBase() = default;
  virtual ElfLSectionImplBase *clone() const = 0;
  virtual const char *getName() const = 0;
  virtual uint64_t getSize() const = 0;
  virtual const uint8_t *getContents() const = 0;
};

template <class ELFT> class ElfLSectionImpl : public ElfLSectionImplBase {
  using Elf_Shdr = typename ELFT::Shdr;

  const ELFFile<ELFT> &EF;
  const Elf_Shdr &Section;

public:
  ElfLSectionImpl(const ELFFile<ELFT> &EF, const Elf_Shdr &Section)
    : EF(EF), Section(Section) {}
  ElfLSectionImpl(const ElfLSectionImpl &) = default;
  ElfLSectionImpl *clone() const override { return new ElfLSectionImpl(*this); }
  ~ElfLSectionImpl() = default;

  const char *getName() const override {
    Expected<StringRef> NameOrErr = EF.getSectionName(Section);
    if (!NameOrErr) {
      consumeError(NameOrErr.takeError());
      return "";
    }
    return NameOrErr->data();
  }

  uint64_t getSize() const override {
    Expected<ArrayRef<uint8_t>> ContentsOrErr = EF.getSectionContents(Section);
    if (!ContentsOrErr) {
      consumeError(ContentsOrErr.takeError());
      return 0;
    }
    return ContentsOrErr->size();
  }

  const uint8_t *getContents() const override {
    Expected<ArrayRef<uint8_t>> ContentsOrErr = EF.getSectionContents(Section);
    if (!ContentsOrErr) {
      consumeError(ContentsOrErr.takeError());
      return 0;
    }
    return ContentsOrErr->data();
  }
};

class ElfLSectionIteratorImplBase {
protected:
  const endianness TargetEndianness;
  const bool Is64Bits;

  ElfLSectionIteratorImplBase(endianness TargetEndianness, bool Is64Bits)
    : TargetEndianness(TargetEndianness), Is64Bits(Is64Bits) {}

public:
  ElfLSectionIteratorImplBase(const ElfLSectionIteratorImplBase &) = default;
  virtual ~ElfLSectionIteratorImplBase() = default;
  virtual ElfLSectionIteratorImplBase *clone() const = 0;
  virtual ElfLSectionIteratorImplBase &operator++() = 0;
  virtual bool operator==(const ElfLSectionIteratorImplBase &) const = 0;
  virtual ElfLSectionImplBase *operator*() const = 0;

  endianness getEndianness() const { return TargetEndianness; }

  bool is64Bits() const { return Is64Bits; }
};

template <class ELFT>
class ElfLSectionIteratorImpl : public ElfLSectionIteratorImplBase {
  using Elf_Shdr = typename ELFT::Shdr;
  using Elf_Shdr_Range = typename ELFT::ShdrRange;
  using SectionsIteratorTy = typename Elf_Shdr_Range::iterator;

  const ELFFile<ELFT> &EF;
  SectionsIteratorTy SectionsIt;

  const ELFFile<ELFT> &getEF() const { return EF; }

  SectionsIteratorTy section_begin() const {
    Expected<Elf_Shdr_Range> Sections = getEF().sections();
    if (!Sections)
      return SectionsIteratorTy();

    return Sections->begin();
  }

  SectionsIteratorTy section_end() const {
    Expected<Elf_Shdr_Range> Sections = getEF().sections();
    if (!Sections)
      return SectionsIteratorTy();

    return Sections->end();
  }

  bool isEqual(const ElfLSectionIteratorImpl &Lhs,
               const ElfLSectionIteratorImpl &Rhs) const {
    return Lhs.SectionsIt == Rhs.SectionsIt;
  }

  bool operator!=(const ElfLSectionIteratorImpl Other) const {
    return !(*this == Other);
  }

public:
  ElfLSectionIteratorImpl(const ELFFile<ELFT> &EF, bool IsEnd = false)
    : ElfLSectionIteratorImplBase(ELFT::TargetEndianness, ELFT::Is64Bits),
      EF(EF) {
    if (IsEnd) {
      SectionsIt = section_end();
      return;
    }

    SectionsIt = section_begin();
  }

  ElfLSectionIteratorImpl *clone() const override {
    return new ElfLSectionIteratorImpl(*this);
  }

  bool operator==(const ElfLSectionIteratorImplBase &Other) const override {
    if (const ElfLSectionIteratorImpl *OPtr =
            dyn_cast<const ElfLSectionIteratorImpl>(&Other)) {
      return isEqual(*this, *OPtr);
    }
    return false;
  }

  ElfLSectionImplBase *operator*() const override {
    assert(*this != ElfLSectionIteratorImpl(EF, true) &&
           "Dereferencing the end iterator.");
    return new ElfLSectionImpl<ELFT>(EF, *SectionsIt);
  }

  ElfLSectionIteratorImpl &operator++() override {
    assert(*this != ElfLSectionIteratorImpl(EF, true) &&
           "Dereferencing the end iterator.");

    ++SectionsIt;
    return *this;
  }

  static bool classof(const ElfLSectionIteratorImplBase *B) {
    return (B->getEndianness() == ELFT::TargetEndianness &&
            B->is64Bits() == ELFT::Is64Bits);
  }
};

class ElfLImplBase {
public:
  ElfLImplBase() = default;
  ElfLImplBase(const ElfLImplBase &) = delete;
  ElfLImplBase &operator=(const ElfLImplBase &) = delete;
  virtual ~ElfLImplBase() = default;
  virtual uint16_t getEMachine() const = 0;
  virtual uint16_t getEType() const = 0;

  virtual ElfLNoteIteratorImplBase *
  createSectionNoteIteratorImpl(bool IsEnd) const = 0;
  virtual ElfLNoteIteratorImplBase *
  createSegmentNoteIteratorImpl(bool IsEnd) const = 0;
  virtual ElfLSectionIteratorImplBase *
  createSectionIteratorImpl(bool IsEnd) const = 0;
};

template <class ELFT> class ElfLImpl : public ElfLImplBase {
  std::unique_ptr<ELFObjectFile<ELFT>> File;
  Error *Err = nullptr;

  friend class ElfL;

public:
  ElfLImpl(std::unique_ptr<ObjectFile> F) {
    ObjectFile *FPtr = F.release();
    if (auto *Obj = dyn_cast<ELFObjectFile<ELFT>>(FPtr))
      File = std::unique_ptr<ELFObjectFile<ELFT>>(Obj);
    else
      assert(false && "Not an ELF object file, or ELF class is wrong.");

    Err = new Error(std::move(Error::success()));
  }
  ElfLImpl(const ElfLImpl &) = delete;
  ElfLImpl &operator=(const ElfLImpl &) = delete;
  virtual ~ElfLImpl() {
    if (!Err)
      return;

    if (*Err) {
      auto ErrorString = toString(std::move(*Err));
      DP("Destroying ELF object parsed with errors: %s\n", ErrorString.c_str());
    } else {
      delete Err;
    }
    Err = nullptr;
  }
  uint16_t getEMachine() const override {
    return cast<ELFObjectFileBase>(File.get())->getEMachine();
  }
  uint16_t getEType() const override {
    return cast<ELFObjectFileBase>(File.get())->getEType();
  }

  ElfLNoteIteratorImplBase *
  createSectionNoteIteratorImpl(bool IsEnd) const override {
    return new ElfLSectionNoteIteratorImpl<ELFT>(File->getELFFile(), *Err,
                                                 IsEnd);
  }

  ElfLNoteIteratorImplBase *
  createSegmentNoteIteratorImpl(bool IsEnd) const override {
    return new ElfLSegmentNoteIteratorImpl<ELFT>(File->getELFFile(), *Err,
                                                 IsEnd);
  }

  ElfLSectionIteratorImplBase *
  createSectionIteratorImpl(bool IsEnd) const override {
    return new ElfLSectionIteratorImpl<ELFT>(File->getELFFile(), IsEnd);
  }
};

ElfL::ElfL(char *Begin, size_t Size) {
  StringRef StrBuf(Begin, Size);
  std::unique_ptr<MemoryBuffer> MemBuf =
      MemoryBuffer::getMemBuffer(StrBuf, "", false);
  Expected<std::unique_ptr<ObjectFile>> BinOrErr =
      ObjectFile::createELFObjectFile(MemBuf->getMemBufferRef(),
                                      /*InitContent=*/false);
  if (!BinOrErr) {
    consumeError(BinOrErr.takeError());
    return;
  }

  if (isa<ELF64LEObjectFile>(BinOrErr->get())) {
    Impl =
        reinterpret_cast<void *>(new ElfLImpl<ELF64LE>(std::move(*BinOrErr)));
  } else if (isa<ELF32LEObjectFile>(BinOrErr->get()))
    Impl =
        reinterpret_cast<void *>(new ElfLImpl<ELF32LE>(std::move(*BinOrErr)));
  else if (isa<ELF32BEObjectFile>(BinOrErr->get()))
    Impl =
        reinterpret_cast<void *>(new ElfLImpl<ELF32BE>(std::move(*BinOrErr)));
  else if (isa<ELF64BEObjectFile>(BinOrErr->get()))
    Impl =
        reinterpret_cast<void *>(new ElfLImpl<ELF64BE>(std::move(*BinOrErr)));
}

ElfL::~ElfL() {
  ElfLImplBase *EImpl = reinterpret_cast<ElfLImplBase *>(Impl);
  delete EImpl;
}

bool ElfL::isValidElf() const { return Impl; }

const char *ElfL::getErrmsg(int N) const {
  // TODO: return text representation for the latest Error.
  return "LLVM ELF error";
}

uint16_t ElfL::getEMachine() const {
  assert(isValidElf() && "Invalid ELF.");
  ElfLImplBase *EImpl = reinterpret_cast<ElfLImplBase *>(Impl);
  return EImpl->getEMachine();
}

uint16_t ElfL::getEType() const {
  assert(isValidElf() && "Invalid ELF.");
  ElfLImplBase *EImpl = reinterpret_cast<ElfLImplBase *>(Impl);
  return EImpl->getEType();
}

bool ElfL::isDynType(uint16_t Ty) { return Ty == ET_DYN; }

ElfLSectionNoteIterator::ElfLSectionNoteIterator(const void *I, bool IsEnd) {
  const ElfLImplBase *EImpl = reinterpret_cast<const ElfLImplBase *>(I);
  // Create new ElfLSectionNoteIteratorImpl<ELFT> object.
  Impl = EImpl->createSectionNoteIteratorImpl(IsEnd);
}

ElfLSectionNoteIterator::~ElfLSectionNoteIterator() {
  const ElfLNoteIteratorImplBase *IImpl =
      reinterpret_cast<const ElfLNoteIteratorImplBase *>(Impl);
  delete IImpl;
}

ElfLSectionNoteIterator::ElfLSectionNoteIterator(
    const ElfLSectionNoteIterator &Other) {
  const ElfLNoteIteratorImplBase *IImpl =
      reinterpret_cast<const ElfLNoteIteratorImplBase *>(Other.Impl);
  Impl = IImpl->clone();
}

bool ElfLSectionNoteIterator::operator==(
    const ElfLSectionNoteIterator Other) const {
  const ElfLNoteIteratorImplBase *Lhs =
      reinterpret_cast<const ElfLNoteIteratorImplBase *>(Impl);
  const ElfLNoteIteratorImplBase *Rhs =
      reinterpret_cast<const ElfLNoteIteratorImplBase *>(Other.Impl);
  return (*Lhs == *Rhs);
}

bool ElfLSectionNoteIterator::operator!=(
    const ElfLSectionNoteIterator Other) const {
  return !(*this == Other);
}

ElfLSectionNoteIterator &ElfLSectionNoteIterator::operator++() {
  ElfLNoteIteratorImplBase *EImpl =
      reinterpret_cast<ElfLNoteIteratorImplBase *>(Impl);
  ++(*EImpl);
  return *this;
}

ElfLNote ElfLSectionNoteIterator::operator*() const { return ElfLNote(Impl); }

ElfLSectionNoteIterator ElfL::section_notes_begin() const {
  assert(isValidElf() && "Invalid ELF.");
  return ElfLSectionNoteIterator(reinterpret_cast<const ElfLImplBase *>(Impl));
}

ElfLSectionNoteIterator ElfL::section_notes_end() const {
  assert(isValidElf() && "Invalid ELF.");
  return ElfLSectionNoteIterator(reinterpret_cast<const ElfLImplBase *>(Impl),
                                 true);
}

ElfLSegmentNoteIterator::ElfLSegmentNoteIterator(const void *I, bool IsEnd) {
  const ElfLImplBase *EImpl = reinterpret_cast<const ElfLImplBase *>(I);
  // Create new ElfLSegmentNoteIteratorImpl<ELFT> object.
  Impl = EImpl->createSegmentNoteIteratorImpl(IsEnd);
}

ElfLSegmentNoteIterator::~ElfLSegmentNoteIterator() {
  const ElfLNoteIteratorImplBase *IImpl =
      reinterpret_cast<const ElfLNoteIteratorImplBase *>(Impl);
  delete IImpl;
}

ElfLSegmentNoteIterator::ElfLSegmentNoteIterator(
    const ElfLSegmentNoteIterator &Other) {
  const ElfLNoteIteratorImplBase *IImpl =
      reinterpret_cast<const ElfLNoteIteratorImplBase *>(Other.Impl);
  Impl = IImpl->clone();
}

bool ElfLSegmentNoteIterator::operator==(
    const ElfLSegmentNoteIterator Other) const {
  const ElfLNoteIteratorImplBase *Lhs =
      reinterpret_cast<const ElfLNoteIteratorImplBase *>(Impl);
  const ElfLNoteIteratorImplBase *Rhs =
      reinterpret_cast<const ElfLNoteIteratorImplBase *>(Other.Impl);
  return (*Lhs == *Rhs);
}

bool ElfLSegmentNoteIterator::operator!=(
    const ElfLSegmentNoteIterator Other) const {
  return !(*this == Other);
}

ElfLSegmentNoteIterator &ElfLSegmentNoteIterator::operator++() {
  ElfLNoteIteratorImplBase *EImpl =
      reinterpret_cast<ElfLNoteIteratorImplBase *>(Impl);
  ++(*EImpl);
  return *this;
}

ElfLNote ElfLSegmentNoteIterator::operator*() const { return ElfLNote(Impl); }

ElfLSegmentNoteIterator ElfL::segment_notes_begin() const {
  assert(isValidElf() && "Invalid ELF.");
  return ElfLSegmentNoteIterator(reinterpret_cast<const ElfLImplBase *>(Impl));
}

ElfLSegmentNoteIterator ElfL::segment_notes_end() const {
  assert(isValidElf() && "Invalid ELF.");
  return ElfLSegmentNoteIterator(reinterpret_cast<const ElfLImplBase *>(Impl),
                                 true);
}

ElfLNote::ElfLNote(const void *IteratorImpl) {
  const ElfLNoteIteratorImplBase *IImpl =
      reinterpret_cast<const ElfLNoteIteratorImplBase *>(IteratorImpl);
  Impl = **IImpl;
}

ElfLNote::ElfLNote(const ElfLNote &Other) {
  const ElfLNoteImplBase *NImpl =
      reinterpret_cast<const ElfLNoteImplBase *>(Impl);
  if (NImpl)
    Impl = NImpl->clone();
}

ElfLNote::~ElfLNote() {
  const ElfLNoteImplBase *NImpl =
      reinterpret_cast<const ElfLNoteImplBase *>(Impl);
  delete NImpl;
}

uint64_t ElfLNote::getNameSize() const {
  const ElfLNoteImplBase *NImpl =
      reinterpret_cast<const ElfLNoteImplBase *>(Impl);
  return NImpl->getNameSize();
}

const char *ElfLNote::getName() const {
  const ElfLNoteImplBase *NImpl =
      reinterpret_cast<const ElfLNoteImplBase *>(Impl);
  return NImpl->getName();
}

uint64_t ElfLNote::getDescSize() const {
  const ElfLNoteImplBase *NImpl =
      reinterpret_cast<const ElfLNoteImplBase *>(Impl);
  return NImpl->getDescSize();
}

const uint8_t *ElfLNote::getDesc() const {
  const ElfLNoteImplBase *NImpl =
      reinterpret_cast<const ElfLNoteImplBase *>(Impl);
  return NImpl->getDesc();
}

uint64_t ElfLNote::getType() const {
  const ElfLNoteImplBase *NImpl =
      reinterpret_cast<const ElfLNoteImplBase *>(Impl);
  return NImpl->getType();
}

ElfLSection::ElfLSection(const void *I) {
  Impl = I;
}

ElfLSection::ElfLSection(const ElfLSection &Other) {
  const ElfLSectionImplBase *SImpl =
      reinterpret_cast<const ElfLSectionImplBase *>(Other.Impl);
  Impl = SImpl->clone();
}

ElfLSection::~ElfLSection() {
  const ElfLSectionImplBase *SImpl =
      reinterpret_cast<const ElfLSectionImplBase *>(Impl);
  delete SImpl;
}

const char *ElfLSection::getName() const {
  const ElfLSectionImplBase *SImpl =
      reinterpret_cast<const ElfLSectionImplBase *>(Impl);
  return SImpl->getName();
}

uint64_t ElfLSection::getSize() const {
  const ElfLSectionImplBase *SImpl =
      reinterpret_cast<const ElfLSectionImplBase *>(Impl);
  return SImpl->getSize();
}

const uint8_t *ElfLSection::getContents() const {
  const ElfLSectionImplBase *SImpl =
      reinterpret_cast<const ElfLSectionImplBase *>(Impl);
  return SImpl->getContents();
}

ElfLSectionIterator ElfL::sections_begin() const {
  assert(isValidElf() && "Invalid ELF.");
  return ElfLSectionIterator(reinterpret_cast<const ElfLImplBase *>(Impl));
}

ElfLSectionIterator ElfL::sections_end() const {
  assert(isValidElf() && "Invalid ELF.");
  return ElfLSectionIterator(reinterpret_cast<const ElfLImplBase *>(Impl),
                             true);
}

ElfLSectionIterator::ElfLSectionIterator(const void *I, bool IsEnd) {
  const ElfLImplBase *EImpl = reinterpret_cast<const ElfLImplBase *>(I);
  Impl = EImpl->createSectionIteratorImpl(IsEnd);
}

ElfLSectionIterator::ElfLSectionIterator(
    const ElfLSectionIterator &Other) {
  ElfLSectionIteratorImplBase *IImpl =
      reinterpret_cast<ElfLSectionIteratorImplBase *>(Other.Impl);
  Impl = IImpl->clone();
}

ElfLSectionIterator::~ElfLSectionIterator() {
  ElfLSectionIteratorImplBase *IImpl =
      reinterpret_cast<ElfLSectionIteratorImplBase *>(Impl);
  delete IImpl;
}

bool ElfLSectionIterator::operator==(
    const ElfLSectionIterator Other) const {
  const ElfLSectionIteratorImplBase *Lhs =
      reinterpret_cast<const ElfLSectionIteratorImplBase *>(Impl);
  const ElfLSectionIteratorImplBase *Rhs =
      reinterpret_cast<const ElfLSectionIteratorImplBase *>(Other.Impl);
  return (*Lhs == *Rhs);
}

bool ElfLSectionIterator::operator!=(
    const ElfLSectionIterator Other) const {
  return !(*this == Other);
}

ElfLSectionIterator &ElfLSectionIterator::operator++() {
  ElfLSectionIteratorImplBase *IImpl =
      reinterpret_cast<ElfLSectionIteratorImplBase *>(Impl);
  ++(*IImpl);
  return *this;
}

ElfLSection ElfLSectionIterator::operator*() const {
  ElfLSectionIteratorImplBase *IImpl =
      reinterpret_cast<ElfLSectionIteratorImplBase *>(Impl);
  return ElfLSection(**IImpl);
}
#endif // !MAY_USE_LIBELF
#endif // INTEL_CUSTOMIZATION
