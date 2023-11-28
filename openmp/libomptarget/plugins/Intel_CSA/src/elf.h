//===- plugins/Intel_CSA/src/elf.h - Target RTLs implementation -*- C++ -*-===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// Utility classes for parsing ELF.
//
//===----------------------------------------------------------------------===//

#ifndef _RTLS_INTEL_CSA_ELF_H_
#define _RTLS_INTEL_CSA_ELF_H_

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <forward_list>
#include <string>
#include <vector>

#include <elf.h>

namespace {

// Couple utility templates for copying bytes to/from structures.
template<typename T>
static inline void copyData(const char *Src, T *Dst, size_t Size = sizeof(T)) {
  std::copy_n(Src, Size, reinterpret_cast<char*>(Dst));
}

// Base class for Elf::Relocation object.
template <typename ElfRelTy>
class RelocBase {
public:
  using AddrTy = decltype(ElfRelTy::r_offset);
  using TypeTy = decltype(ElfRelTy::r_info);
  using SymTy = decltype(ElfRelTy::r_info);

  // Relocation entry kind - with or without explicit addend.
  enum class KindTy {
    Rel,
    Rela
  };
  static const KindTy Kind;

public:
  RelocBase(ElfRelTy &R) : RawRel(R) {}
  RelocBase(const RelocBase &R) : RawRel(R.RawRel) {}

  // Location at which relocation is to be applied.
  AddrTy getOffset() const {
    return RawRel.r_offset;
  }

protected:
  // Returns ELF relocation type. Relocation types are usually described in
  // target system ABI.
  TypeTy getType() const;

  // Returns symbol index with respect to which relocation is to be done.
  SymTy getSym() const;

  // Reference to the ELF relocation entry object
  ElfRelTy &RawRel;
};

// RelocBase specializations for standard ELF relocation types
// Elf32_Rel
template<>
const RelocBase<Elf32_Rel>::KindTy RelocBase<Elf32_Rel>::Kind =
  RelocBase<Elf32_Rel>::KindTy::Rel;

template<>
inline RelocBase<Elf32_Rel>::TypeTy RelocBase<Elf32_Rel>::getType() const {
  return ELF32_R_TYPE(RawRel.r_info);
}

template<>
inline RelocBase<Elf32_Rel>::SymTy RelocBase<Elf32_Rel>::getSym() const {
  return ELF32_R_SYM(RawRel.r_info);
}

// Elf32_Rela
template<>
const RelocBase<Elf32_Rela>::KindTy RelocBase<Elf32_Rela>::Kind =
  RelocBase<Elf32_Rela>::KindTy::Rela;

template<>
inline RelocBase<Elf32_Rela>::TypeTy RelocBase<Elf32_Rela>::getType() const {
  return ELF32_R_TYPE(RawRel.r_info);
}

template<>
inline RelocBase<Elf32_Rela>::SymTy RelocBase<Elf32_Rela>::getSym() const {
  return ELF32_R_SYM(RawRel.r_info);
}

// Elf64_Rel
template<>
const RelocBase<Elf64_Rel>::KindTy RelocBase<Elf64_Rel>::Kind =
  RelocBase<Elf64_Rel>::KindTy::Rel;

template<>
inline RelocBase<Elf64_Rel>::TypeTy RelocBase<Elf64_Rel>::getType() const {
  return ELF64_R_TYPE(RawRel.r_info);
}

template<>
inline RelocBase<Elf64_Rel>::SymTy RelocBase<Elf64_Rel>::getSym() const {
  return ELF64_R_SYM(RawRel.r_info);
}

// Elf64_Rela
template<>
const RelocBase<Elf64_Rela>::KindTy RelocBase<Elf64_Rela>::Kind =
  RelocBase<Elf64_Rela>::KindTy::Rela;

template<>
inline RelocBase<Elf64_Rela>::TypeTy RelocBase<Elf64_Rela>::getType() const {
  return ELF64_R_TYPE(RawRel.r_info);
}

template<>
inline RelocBase<Elf64_Rela>::SymTy RelocBase<Elf64_Rela>::getSym() const {
  return ELF64_R_SYM(RawRel.r_info);
}

// Template class which represents an executable ELF file.
// Machine - an ELF machine value
// EhdrTy - ELF header type (one of Elf32_Ehdr or Elf64_Ehdr)
// PhdrTy - ELF program header type (one of Elf32_Phdr or Elf64_Phdr)
// ShdrTy - ELF section header type (one of Elf32_Shdr or Elf64_Shdr)
// RelTy - ELF relocation entry type (one of Elf32_Rela, Elf64_Rela,
//       Elf32_Rel, Rel64_Rel)
// ShdrTy - ELF symbol entry type (one of Elf32_Sym or Elf64_Sym)
template <int Machine, typename EhdrTy, typename PhdrTy, typename ShdrTy,
          typename RelTy, typename SymTy>
class Elf {
public:
  // Address and size types for this Elf object
  using AddrTy = decltype(ShdrTy::sh_addr);
  using SizeTy = decltype(ShdrTy::sh_size);
  using OffTy  = decltype(ShdrTy::sh_offset);

  // Address range.
  class AddrRangeTy {
  public:
    explicit AddrRangeTy(AddrTy Start, SizeTy Len)
      : Addr(Start), Size(Len)
    {}

    // Return starting and ending addresses.
    AddrTy getStart() const { return Addr; }
    AddrTy getEnd() const { return Addr + Size; }

    // Returns interval length
    SizeTy getSize() const { return Size; }

    // Returns true if this interval overlaps with another interval.
    bool overlaps(const AddrRangeTy &Other) const {
      return getStart() < Other.getEnd() && getEnd() > Other.getStart();
    }

    // Returns true if this interval includes the other interval.
    bool contains(const AddrRangeTy &Other) const {
      return getStart() <= Other.getStart() && Other.getEnd() <= getEnd();
    }

    // Tests if address range contains given address.
    bool contains(AddrTy Addr) const {
      return Addr >= getStart() && Addr <= getEnd();
    }

  private:
    AddrTy Addr;
    SizeTy Size;
  };

  class Section;
  class Strtab;
  class Symtab;
  class Reltab;

  // Interface for an ELF symbol
  class Symbol {
  public:
    enum class Type {
      NoType,
      Object,
      Func,
      Section,
      File,
      Common,
      TLS,
      Reserved
    };

  public:
    Symbol(Symtab &ST, SymTy &S) : Container(ST), RawSym(S) {}
    Symbol(const Symbol &S) : Container(S.Container), RawSym(S.RawSym) {}

    // Returns symbol name
    std::string getName() const {
      return Container.getStrtab()->getString(RawSym.st_name);
    }

    // Symbol type
    Type getType() const {
      switch (auto ST = RawSym.st_info & 0xFu) {
        case STT_NOTYPE:
          return Type::NoType;
        case STT_OBJECT:
          return Type::Object;
        case STT_FUNC:
          return Type::Func;
        case STT_SECTION:
          return Type::Section;
        case STT_FILE:
          return Type::File;
        case STT_COMMON:
          return Type::Common;
        case STT_TLS:
          return Type::TLS;
        default:
          if ((ST >= STT_LOOS   && ST <= STT_HIOS) ||
              (ST >= STT_LOPROC && ST <= STT_HIPROC))
            return Type::Reserved;
      }
      assert(false && "Unknown symbol type");
      return Type::Reserved;
    }

    // Returns true if symbol is undefined or false otherwise
    bool isUndefined() const {
      return RawSym.st_shndx == SHN_UNDEF;
    }

    // Returns true if symbol has an absolute value that does not change as
    // a result of relocation.
    bool isAbs() const {
      return RawSym.st_shndx == SHN_ABS;
    }

    // Return section where the symbol is defined.
    Section* getDefiningSection() const {
      if (isUndefined() || isAbs())
        return nullptr;
      // TODO: need to add special handling for symbols with SHN_XINDEX index.
      return Container.getElf().getSection(RawSym.st_shndx);
    }

    // Symbol size
    SizeTy getSize() const {
      return RawSym.st_size;
    }

    // Symbol value
    AddrTy getValue() const {
      return RawSym.st_value;
    }

    // Returns symbol index in symtab
    size_t getIndex() const {
      return &RawSym - reinterpret_cast<SymTy*>(Container.getBits());
    }

    // Returns pointer to the symbol data if it exists (i.e. symbol should
    // be defined in a section which has data).
    char* getImage() const {
      if (isUndefined())
        return nullptr;

      // Section where the symbol is defined
      auto Sec = getDefiningSection();
      if (!Sec || !Sec->isAlloc() || !Sec->hasBits())
        return nullptr;

      // Symbol image within section data
      return Sec->getBits() + getValue() - Sec->getAddr();
    }

  private:
    // Containing symbol table
    Symtab &Container;

    // Reference to the ELF's symbol structure in the symbol table data
    SymTy &RawSym;
  };

  // Represents a relocation entry in relocation table.
  class Reloc final : public RelocBase<RelTy> {
  public:
    Reloc(Reltab &ST, RelTy &S) : RelocBase<RelTy>(S), Container(ST) {}
    Reloc(const Reloc &R) : RelocBase<RelTy>(R), Container(R.Container) {}

    // Symbol with respect to which relocation is to be made.
    Symbol getSymbol() const {
      return Container.getSymtab()->getSymbol(this->getSym());
    }

    char* getTarget() const {
      // For executable and shared object files relocation offset is a
      // virtual address of the location where relocation is applied.
      auto Offset = this->getOffset() - Container.getTarget()->getAddr();
      return Container.getTarget()->getBits() + Offset;
    }

  private:
    // Containing relocation table
    Reltab &Container;
  };

  // Represents a generic section in the ELF image
  class Section {
    // Section type
    enum class Type {
      Generic,
      Symtab,
      Strtab,
      Reltab
    };

    // Returns section type
    virtual Type getType() const {
      return Type::Generic;
    }

  public:
    virtual ~Section() {}

    Symtab* toSymtab() {
      return getType() == Type::Symtab ? static_cast<Symtab*>(this) : nullptr;
    }

    Strtab* toStrtab() {
      return getType() == Type::Strtab ? static_cast<Strtab*>(this) : nullptr;
    }

    Reltab* toReltab() {
      return getType() == Type::Reltab ? static_cast<Reltab*>(this) : nullptr;
    }

    Elf& getElf() {
      return Container;
    }

    const Elf& getElf() const {
      return Container;
    }

    std::string getName() const {
      auto StrTab = Container.getSectionsStrtab();
      assert(StrTab && "No section string table");
      return StrTab->getString(Header.sh_name);
    }

    AddrTy getAddr() const {
      return Header.sh_addr;
    }

    OffTy getOffset() const {
      return Header.sh_offset;
    }

    SizeTy getSize() const {
      return Header.sh_size;
    }

    AddrRangeTy getAddrRange() const {
      assert(isAlloc() && "Should be an allocatable section");
      return AddrRangeTy(getAddr(), getSize());
    }

    // Returns true if section has data
    bool hasBits() const {
      return Header.sh_type != SHT_NULL &&
             Header.sh_type != SHT_NOBITS;
    }

    // Returns true if section is allocatable
    bool isAlloc() const {
      return (Header.sh_flags & SHF_ALLOC) != 0;
    }

    // Access to raw section data
    const char* getBits() const {
      return hasBits() ? Bits.data() : nullptr;
    }

  private:
    // ELF image this section belongs to.
    Elf &Container;

    // Section header
    ShdrTy Header;

    // Section data
    std::vector<char> Bits;

  private:
    Section(Elf &E, const ShdrTy &H, const char *I) : Container(E), Header(H) {
      // copy bits if section has data
      if (hasBits()) {
        Bits.resize(getSize());
        std::copy_n(I + getOffset(), getSize(), Bits.data());
      }
    }

    friend class Elf;
  };

  // String table section.
  class Strtab final : public Section {
    typename Section::Type getType() const override {
      return Section::Type::Strtab;
    }

  public:
    std::string getString(size_t Idx) const {
      assert(Idx < this->getSize() && "Out of range string index");
      return std::string(this->getBits() + Idx);
    }

  private:
    Strtab(Elf &E, const ShdrTy &H, const char *I) : Section(E, H, I) {}

    friend class Elf;
  };

  // Symbol table section.
  class Symtab final : public Section {
    typename Section::Type getType() const override {
      return Section::Type::Symtab;
    }

  public:
    // Iterator for symbol table
    class Iterator {
    public:
      Iterator(Symtab &C, size_t I) : Container(C), Idx(I) {}
      Iterator(const Iterator &It) : Container(It.Container), Idx(It.Idx) {}

      Iterator& operator++() {
        ++Idx;
        return *this;
      }
      Iterator operator++(int) {
        Iterator Tmp(*this);
        operator++();
        return Tmp;
      }
      bool operator==(const Iterator &It) const {
        return &Container == &It.Container && Idx == It.Idx;
      }
      bool operator!=(const Iterator &It) const {
        return &Container != &It.Container || Idx != It.Idx;
      }
      Symbol operator*() const {
        return Container.getSymbol(Idx);
      }

    private:
      Symtab &Container;
      size_t Idx;
    };

    // Begin/end iterators for the table. Using non-conforming (for LLVM)
    // naming for these methods to enable range-based loops for symbol tables.
    Iterator begin() {
      return Iterator(*this, 0);
    }
    Iterator end() {
      return Iterator(*this, getSize());
    }

  public:
    // Returns string table associated with this symbol table
    const Strtab* getStrtab() const {
      return this->Container.getSection(this->Header.sh_link)->toStrtab();
    }

    // Returns number of symbols in the symbol table
    size_t getSize() const {
      assert(this->Header.sh_entsize == sizeof(SymTy) &&
             "Unsupported entry size");
      return this->Header.sh_size / this->Header.sh_entsize;
    }

    // Access to symbols in the table
    Symbol getSymbol(size_t Idx) {
      assert(Idx < this->getSize() && "Out of range symbol index");
      return Symbol(*this, reinterpret_cast<SymTy*>(this->getBits())[Idx]);
    }

  private:
    Symtab(Elf &E, const ShdrTy &H, const char *I) : Section(E, H, I) {}
    friend class Elf;
  };

  // Relocation table section
  class Reltab : public Section {
    typename Section::Type getType() const override {
      return Section::Type::Reltab;
    }

  public:
    // Iterator for relocation table entries
    class Iterator {
    public:
      Iterator(Reltab &C, size_t I) : Container(C), Idx(I) {}
      Iterator(const Iterator &It) : Container(It.Container), Idx(It.Idx) {}

      Iterator& operator++() {
        ++Idx;
        return *this;
      }
      Iterator operator++(int) {
        Iterator Tmp(*this);
        operator++();
        return Tmp;
      }
      bool operator==(const Iterator &It) const {
        return &Container == &It.Container && Idx == It.Idx;
      }
      bool operator!=(const Iterator &It) const {
        return &Container != &It.Container || Idx != It.Idx;
      }
      Reloc operator*() const {
        return Container.getReloc(Idx);
      }

    private:
      Reltab &Container;
      size_t Idx;
    };

    // Begin/end iterators for the table. Using non-conforming (for LLVM)
    // naming for these methods to enable range-based loops for relocation
    // tables.
    Iterator begin() {
      return Iterator(*this, 0);
    }
    Iterator end() {
      return Iterator(*this, getSize());
    }

    // Returns target section for this relocation section
    Section* getTarget() const {
      return this->Container.getSection(this->Header.sh_info);
    }

    // Returns symbol table associated with the relocation section
    Symtab* getSymtab() const {
      return this->Container.getSection(this->Header.sh_link)->toSymtab();
    }

    // Returns number of relocations in this section
    size_t getSize() const {
      assert(this->Header.sh_entsize == sizeof(RelTy) &&
             "Unsupported entry size");
      return this->Header.sh_size / this->Header.sh_entsize;
    }

    // Access to symbols in the table
    Reloc getReloc(size_t Idx) {
      assert(Idx < this->getSize() && "Out of range relocation index");
      return Reloc(*this, reinterpret_cast<RelTy*>(this->getBits())[Idx]);
    }

  public:
    Reltab(Elf &E, const ShdrTy &H, const char *I) : Section(E, H, I) {}
    friend class Elf;
  };

  // Program segment
  class Segment {
  public:
    Segment(Elf &E, const char *H) : Container(E) {
      // Read segment header
      copyData(H, &Header);

      // Build list of sections which belong to this segment
      auto SegAddr = getAddrRange();
      for (const auto &Sec : Container.getSections())
        if (Sec->isAlloc() && SegAddr.contains(Sec->getAddrRange()))
          Sections.insert(Sec);
    }

    // Return address range occupied by this segment
    AddrRangeTy getAddrRange() const {
      return AddrRangeTy(getVAddr(), getSize());
    }


  private:
    AddrTy getVAddr() const {
      return Header.p_vaddr;
    }

    AddrTy getPAddr() const {
      return Header.p_paddr;
    }

    SizeTy getAlign() const {
      return Header.p_align;
    }

    SizeTy getSize() const {
      return Header.p_memsz;
    }

    // Returns true if this segment includes given section.
    bool contains(const Section *Sec) const {
      return Sections.find(Sec) != Sections.end();
    }

  private:
    Elf &Container;

    // Segment header
    PhdrTy Header;

    // Sections which belong to this segment.
    std::unordered_set<const Section*> Sections;

    friend Elf;
  };

private:
  // Creates an approptiate Section object for the given ELF section
  // header and adds it to the list of sections in this ELF object.
  Section* readSection(const char *Image, size_t ShdrOffset) {
    // Read section header
    ShdrTy SecHeader;
    copyData(Image + ShdrOffset, &SecHeader);

    // Create section specializations based on the section type
    switch (SecHeader.sh_type) {
      case SHT_SYMTAB:
        return new Symtab(*this, SecHeader, Image);
      case SHT_STRTAB:
        return new Strtab(*this, SecHeader, Image);
      case SHT_RELA:
      case SHT_REL: {
        // We assume that only one relocation entry kind is supported on a
        // particular platform. Thus make sure we get an expected relocation
        // section type based on the relocation's kind.
        decltype(SecHeader.sh_type) Type =
            Reloc::Kind == Reloc::KindTy::Rel ? SHT_REL : SHT_RELA;
        if (SecHeader.sh_type == Type)
          return new Reltab(*this, SecHeader, Image);
        assert(false && "Unexpected relocation section type");
        break;
      }
    }
    return new Section(*this, SecHeader, Image);
  }

public:
  Elf() = default;

  ~Elf() {
    for (auto *Sec : Sections)
      delete Sec;
    for (auto *Seg : Segments)
      delete Seg;
  }

  static bool isElf(const char *Image, size_t Size) {
    if (Size < sizeof(EhdrTy))
      return false;
    const auto *Ehdr = reinterpret_cast<const EhdrTy*>(Image);
    if (memcmp(Ehdr->e_ident, ELFMAG, SELFMAG) != 0 ||
        Ehdr->e_machine != Machine ||
        !(Ehdr->e_type == ET_EXEC ||
          Ehdr->e_type == ET_DYN) ||
        Ehdr->e_ehsize != sizeof(EhdrTy) ||
        Ehdr->e_shentsize != sizeof(ShdrTy) ||
        Ehdr->e_phentsize != sizeof(PhdrTy))
      return false;
    return true;
  }

  bool readFromMemory(const char *Image, size_t Size) {
    if (!isElf(Image, Size))
      return false;

    // copy ELF header
    copyData(Image, &Header);

    // Read sections
    if (Header.e_shoff) {
      if (Header.e_shnum)
        Sections.resize(Header.e_shnum);
      else {
        ShdrTy Shdr0;
        copyData(Image + Header.e_shoff, &Shdr0);
        Sections.resize(Shdr0.sh_size);
      }
      auto Offset = Header.e_shoff;
      for (auto *&Sec : Sections) {
        Sec = readSection(Image, Offset);
        Offset += sizeof(ShdrTy);
      }
    }
    else
      Sections.resize(0);

    // Read program segments
    if (Header.e_phoff && Header.e_phnum) {
      Segments.resize(Header.e_phnum);
      auto Offset = Header.e_phoff;
      for (auto *&Seg : Segments) {
        Seg = new Segment(*this, Image + Offset);
        Offset += sizeof(PhdrTy);
      }
    }
    else
      Segments.resize(0);
    return true;
  }

  // Returns ELF segments. Segments cannot be modified.
  const std::vector<Segment*>& getSegments() const {
    return Segments;
  }

  // Returns sections. Sections cannot be modified.
  const std::vector<Section*>& getSections() const {
    return Sections;
  }

  // Return section with given index.
  Section* getSection(size_t Idx) const {
    assert(Idx < Sections.size() && "Out of range section index");
    return Sections[Idx];
  }

  // Returns the first instance of a section which has specified name
  Section* findSection(const std::string &Name) const {
    for (auto *Sec : Sections)
      if (Sec->getName() == Name)
        return Sec;
    return nullptr;
  }

  // Returns string table which contains section names.
  const Strtab* getSectionsStrtab() const {
    if (Header.e_shstrndx == SHN_UNDEF)
      return nullptr;
    auto Idx = Header.e_shstrndx;
    if (Idx == SHN_XINDEX)
      Idx = getSection(0)->Header.sh_link;
    return getSection(Idx)->toStrtab();
  }

private:
  // ELF header
  EhdrTy Header;

  // Section table
  std::vector<Section*> Sections;

  // Segment table
  std::vector<Segment*> Segments;
};

} // anonymous namespace

#endif // _RTLS_INTEL_CSA_ELF_H_
