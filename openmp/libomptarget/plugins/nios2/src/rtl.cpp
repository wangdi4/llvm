//======= plugins/nios2/rtl.cpp - Target RTLs implementation -*- C++ -*-======//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.txt for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Plugin RTL for Nios(R) II.
///
//===----------------------------------------------------------------------===//

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <forward_list>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <vector>

#include <elf.h>
#include <msof.h>

#include "omptargetplugin.h"

// Define Nios2 machine and relocations if elf.h does not have these
// definitions.
#ifndef EM_ALTERA_NIOS2
// Nios2 machine value
#define EM_ALTERA_NIOS2         113

// Nios2 relocations
#define R_NIOS2_NONE            0
#define R_NIOS2_S16             1
#define R_NIOS2_U16             2
#define R_NIOS2_PCREL16         3
#define R_NIOS2_CALL26          4
#define R_NIOS2_CALL26_NOAT     41
#define R_NIOS2_IMM5            5
#define R_NIOS2_CACHE_OPX       6
#define R_NIOS2_IMM6            7
#define R_NIOS2_IMM8            8
#define R_NIOS2_HI16            9
#define R_NIOS2_LO16            10
#define R_NIOS2_HIADJ16         11
#define R_NIOS2_BFD_RELOC_32    12
#define R_NIOS2_BFD_RELOC_16    13
#define R_NIOS2_BFD_RELOC_8     14
#define R_NIOS2_GPREL           15
#define R_NIOS2_GNU_VTINHERIT   16
#define R_NIOS2_GNU_VTENTRY     17
#define R_NIOS2_UJMP            18
#define R_NIOS2_CJMP            19
#define R_NIOS2_CALLR           20
#define R_NIOS2_ALIGN           21
#define R_NIOS2_GOT16           22
#define R_NIOS2_CALL16          23
#define R_NIOS2_GOTOFF_LO       24
#define R_NIOS2_GOTOFF_HA       25
#define R_NIOS2_PCREL_LO        26
#define R_NIOS2_PCREL_HA        27
#define R_NIOS2_TLS_GD16        28
#define R_NIOS2_TLS_LDM16       29
#define R_NIOS2_TLS_LDO16       30
#define R_NIOS2_TLS_IE16        31
#define R_NIOS2_TLS_LE16        32
#define R_NIOS2_TLS_DTPMOD      33
#define R_NIOS2_TLS_DTPREL      34
#define R_NIOS2_TLS_TPREL       35
#define R_NIOS2_COPY            36
#define R_NIOS2_GLOB_DAT        37
#define R_NIOS2_JUMP_SLOT       38
#define R_NIOS2_RELATIVE        39
#define R_NIOS2_GOTOFF          40
#define R_NIOS2_GOT_LO          42
#define R_NIOS2_GOT_HA          43
#define R_NIOS2_CALL_LO         44
#define R_NIOS2_CALL_HA         45

#endif // EM_ALTERA_NIOS2

#ifdef OMPTARGET_DEBUG
#define DP(...)                                                            \
  {                                                                        \
    fprintf(stderr, "Nios2 (HOST) --> ");                                  \
    fprintf(stderr, __VA_ARGS__);                                          \
    fflush(nullptr);                                                       \
  }
#else
#define DP(...)                                                            \
  {}
#endif

// Elf Relocator
namespace {

// Couple utility templates for copying bytes to/from structures.
template<typename T>
inline void copyData(const char *Src, T *Dst, size_t Size = sizeof(T)) {
  std::copy_n(Src, Size, reinterpret_cast<char*>(Dst));
}

template<typename T>
inline void copyData(const T *Src, char *Dst, size_t Size = sizeof(T)) {
  std::copy_n(reinterpret_cast<const char*>(Src), Size, Dst);
}

template<typename PtrTy, typename SizeTy>
class Interval {
public:
  Interval(PtrTy Start, SizeTy Len) : Addr(Start), Size(Len) {}

  // Returns starting and ending addresses of the interval.
  PtrTy getStart() const { return Addr; }
  PtrTy getEnd() const { return Addr + Size; }

  // Returns interval length
  SizeTy getSize() const { return Size; }

  // Returns true if this interval overlaps with another interval.
  bool overlaps(const Interval &Other) const {
    return getStart() < Other.getEnd() && getEnd() > Other.getStart();
  }

  // Returns true if this interval includes the other interval.
  bool contains(const Interval &Other) const {
    return getStart() <= Other.getStart() && Other.getEnd() <= getEnd();
  }

  // Tests if interval contains given address.
  bool contains(PtrTy Addr) const {
    return Addr >= getStart() && Addr <= getEnd();
  }

private:
  PtrTy  Addr;
  SizeTy Size;
};

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
  void setOffset(AddrTy NewAddr) {
    RawRel.r_offset = NewAddr;
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

// RelocBase specializations for Elf32_Rela relocation type. Other
// specializations may also be added as needed.
template<>
const RelocBase<Elf32_Rela>::KindTy RelocBase<Elf32_Rela>::Kind =
  RelocBase<Elf32_Rela>::KindTy::Rela;

template<>
Elf32_Word RelocBase<Elf32_Rela>::getType() const {
  return ELF32_R_TYPE(RawRel.r_info);
}

template<>
Elf32_Word RelocBase<Elf32_Rela>::getSym() const {
  return ELF32_R_SYM(RawRel.r_info);
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
  using AddrRangeTy = Interval<AddrTy, SizeTy>;

  // Forward declarations.
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
              (ST >= STT_LOPROC && ST <= STT_HIPROC)) {
            return Type::Reserved;
          }
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
      if (isUndefined() || isAbs()) {
        return nullptr;
      }
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
    void setValue(AddrTy NewValue) {
      RawSym.st_value = NewValue;
    }

    // Returns symbol index in symtab
    size_t getIndex() const {
      return &RawSym - reinterpret_cast<SymTy*>(Container.getBits());
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

    // Returns true if relocation implicitly uses other symbols besides the
    // one explicitly referenced by relocation.
    bool hasImplicitSymbolRefs() const;

    char* getTarget() const {
      // For executable and shared object files relocation offset is a
      // virtual address of the location where relocation is applied.
      auto Offset = this->getOffset() - Container.getTarget()->getAddr();
      return Container.getTarget()->getBits() + Offset;
    }

    void apply() const;

  private:
    // Containing symbol table
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
      assert(StrTab != nullptr && "No section string table");
      return StrTab->getString(Header.sh_name);
    }

    AddrTy getAddr() const {
      return Header.sh_addr;
    }

    void setAddr(AddrTy NewAddr) {
      Header.sh_addr = NewAddr;
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
    char* getBits() {
      return hasBits() ? Bits.data() : nullptr;
    }
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
    typename Section::Type getType() const {
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
    typename Section::Type getType() const {
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
      assert(this->Header.sh_entsize == sizeof(SymTy) && "Unsupported entry size");
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
    typename Section::Type getType() const {
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
      assert(this->Header.sh_entsize == sizeof(RelTy) && "Unsupported entry size");
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
      for (const auto &Sec : Container.getSections()) {
        if (Sec->isAlloc() && SegAddr.contains(Sec->getAddrRange())) {
          Sections.insert(Sec);
        }
      }
    }

    // Return address range occupied by this segment
    AddrRangeTy getAddrRange() const {
      return AddrRangeTy(getVAddr(), getSize());
    }

    // Relocate segment, i.e. move it to a new address in memory and
    // update all references to objects defined in this segment.
    void relocate(AddrTy NewAddr) {
      // Current address range
      auto AddrRange = getAddrRange();

        // Displacement from current address - that is what will be added
      // to all references.
      auto Disp = NewAddr - getVAddr();

      // Start with segment address
      setVAddr(NewAddr);
      setPAddr(getPAddr() + Disp);

      // First pass over sections - update section and symbol addresses
      for (auto *Sec : Container.getSections()) {
        // Update section address if it is a part of this segment
        if (contains(Sec)) {
          Sec->setAddr(Sec->getAddr() + Disp);
        }
        if (auto *SymTab = Sec->toSymtab()) {
          // For symbol tables update addresses for symbols defined in this
          // segment.
          for (auto Sym : *SymTab) {
            if (auto *DefSec = Sym.getDefiningSection()) {
              if (contains(DefSec)) {
                Sym.setValue(Sym.getValue() + Disp);
              }
              continue;
            }
            if (Sym.isAbs() && Sym.getType() != Symbol::Type::File) {
              if (AddrRange.contains(Sym.getValue())) {
                Sym.setValue(Sym.getValue() + Disp);
              }
            }
          }
        }
      }

      // Second pass - apply relocations where nesessary
      for (auto *Sec : Container.getSections()) {
        if (auto *RelTab = Sec->toReltab()) {
          bool PatchOffset = contains(RelTab->getTarget());
          for (auto Rel : *RelTab) {
            // If relocation target (where relocation is aplied) belongs to
            // this segment, then we need to update location address.
            if (PatchOffset) {
              Rel.setOffset(Rel.getOffset() + Disp);
            }

            // Patch location if symbol associated with the relocation is
            // defined in this segment.
            if (auto *DefSec = Rel.getSymbol().getDefiningSection()) {
              if (contains(DefSec)) {
                Rel.apply();
                continue;
              }
            }

            // Unconditionally apply relocations which implicitly use
            // other symbols for calculating target address.
            if (Rel.hasImplicitSymbolRefs()) {
              Rel.apply();
              continue;
            }
          }
        }
      }
    }

  private:
    AddrTy getVAddr() const {
      return Header.p_vaddr;
    }

    void setVAddr(AddrTy NewAddr) {
      Header.p_vaddr = NewAddr;
    }

    AddrTy getPAddr() const {
      return Header.p_paddr;
    }

    void setPAddr(AddrTy NewAddr) {
      Header.p_paddr = NewAddr;
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
    std::set<const Section*> Sections;

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
        auto Type = Reloc::Kind == Reloc::KindTy::Rel ? SHT_REL : SHT_RELA;
        if (SecHeader.sh_type == Type) {
          return new Reltab(*this, SecHeader, Image);
        }
        assert(false && "Unexpected relocation section type");
        break;
      }
    }
    return new Section(*this, SecHeader, Image);
  }

public:
  ~Elf() {
    for (auto &Sec : Sections) {
      delete Sec;
    }
    for (auto &Seg : Segments) {
      delete Seg;
    }
  }

  bool readFromMemory(const char *Image, size_t Size) {
    // copy ELF header
    copyData(Image, &Header);

    // Make sure we are dealing with an ELF with expected properties:
    // - ELF machine matches expect value
    // - It is an executable file
    // - Header structures have expected sizes
    if (memcmp(Header.e_ident, ELFMAG, SELFMAG) != 0 ||
        Header.e_machine != Machine ||
        Header.e_type != ET_EXEC ||
        Header.e_ehsize != sizeof(EhdrTy) ||
        Header.e_shentsize != sizeof(ShdrTy) ||
        Header.e_phentsize != sizeof(PhdrTy)) {
      return false;
    }

    // Read sections
    if (Header.e_shoff != 0) {
      if (Header.e_shnum != 0) {
        Sections.resize(Header.e_shnum);
      }
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
    else {
      Sections.resize(0);
    }

    // Read program segments
    if (Header.e_phoff != 0 && Header.e_phnum != 0) {
      Segments.resize(Header.e_phnum);
      auto Offset = Header.e_phoff;
      for (auto *&Seg : Segments) {
        Seg = new Segment(*this, Image + Offset);
        Offset += sizeof(PhdrTy);
      }
    }
    else {
      Segments.resize(0);
    }
    return true;
  }

  void writeToMemory(char *Image, size_t Size) const {
    // Elf header
    copyData(&Header, Image);

    // write sections
    if (Header.e_shoff != 0) {
      auto Offset = Header.e_shoff;
      for (const auto *Sec : Sections) {
        copyData(&Sec->Header, Image + Offset);
        Offset += sizeof(ShdrTy);
        if (const auto *Bits = Sec->getBits()) {
          std::copy_n(Bits, Sec->getSize(), Image + Sec->getOffset());
        }
      }
    }

    // write program segments
    if (Header.e_phoff != 0 && Header.e_phnum != 0) {
      auto Offset = Header.e_phoff;
      for (const auto *Seg : Segments) {
        copyData(&Seg->Header, Image + Offset);
        Offset += sizeof(PhdrTy);
      }
    }
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
    for (auto *Sec : Sections) {
      if (Sec->getName() == Name) {
        return Sec;
      }
    }
    return nullptr;
  }

  // Returns string table which contains section names.
  const Strtab* getSectionsStrtab() const {
    if (Header.e_shstrndx == SHN_UNDEF) {
      return nullptr;
    }
    auto Idx = Header.e_shstrndx;
    if (Idx == SHN_XINDEX) {
      Idx = getSection(0)->Header.sh_link;
    }
    return getSection(Idx)->toStrtab();
  }

  // Find symbol with specified name.
  std::pair<Symtab*, size_t> findSymbol(const std::string &Name) const {
    for (auto *Sec : getSections()) {
      if (auto *SymTab = Sec->toSymtab()) {
        for (auto Sym : *SymTab) {
          if (Sym.getName() == Name) {
            return { SymTab, Sym.getIndex() };
          }
        }
      }
    }
    return { nullptr, 0 };
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

// Nios2 specific part of Elf implementation
namespace {

// Elf specialization for Nios2. Using 32-bit header types and relocation
// entry type with explicit addend.
using Nios2Elf = Elf<EM_ALTERA_NIOS2,
  Elf32_Ehdr, Elf32_Phdr, Elf32_Shdr, Elf32_Rela, Elf32_Sym>;

inline uint32_t Adj(uint32_t X) {
  return (((X >> 16u) & 0xFFFFu) + ((X >> 15u) & 0x1u)) & 0xFFFFu;
}

template<>
void Nios2Elf::Reloc::apply() const {
  AddrTy Addr = 0u;
  uint32_t Mask = 0u;
  uint32_t Shift = 0u;

  switch (this->getType()) {
    case R_NIOS2_NONE:
      return;
    case R_NIOS2_S16:
    case R_NIOS2_U16:
      // S + A
      Addr = getSymbol().getValue() + RawRel.r_addend;
      Mask = 0x003FFFC0u;
      Shift = 6u;
      break;
    case R_NIOS2_PCREL16:
      // ((S + A) - 4) - PC
      Addr = ((getSymbol().getValue() + RawRel.r_addend) - 4) - getOffset();
      Mask = 0x003FFFC0u;
      Shift = 6u;
      break;
    case R_NIOS2_CALL26:
    case R_NIOS2_CALL26_NOAT:
      // (S + A) >> 2
      Addr = (getSymbol().getValue() + RawRel.r_addend) >> 2;
      Mask = 0x003FFFC0u;
      Shift = 6u;
      break;
    case R_NIOS2_IMM5:
      // (S + A) & 0x1F
      Addr = (getSymbol().getValue() + RawRel.r_addend) & 0x1Fu;
      Mask = 0x000007C0u;
      Shift = 6u;
      break;
    case R_NIOS2_CACHE_OPX:
      // (S + A) & 0x1F
      Addr = (getSymbol().getValue() + RawRel.r_addend) & 0x1Fu;
      Mask = 0x07C00000u;
      Shift = 22u;
      break;
    case R_NIOS2_IMM6:
      // (S + A) & 0x3F
      Addr = (getSymbol().getValue() + RawRel.r_addend) & 0x3Fu;
      Mask = 0x00000FC0u;
      Shift = 6u;
      break;
    case R_NIOS2_IMM8:
      // (S + A) & 0xFF
      Addr = (getSymbol().getValue() + RawRel.r_addend) & 0xFFu;
      Mask = 0x00003FC0u;
      Shift = 6u;
      break;
    case R_NIOS2_HI16:
      // ((S + A) >> 16) & 0xFFFF
      Addr = ((getSymbol().getValue() + RawRel.r_addend) >> 16) & 0xFFFFu;
      Mask = 0x003FFFC0u;
      Shift = 6u;
      break;
    case R_NIOS2_LO16:
      // (S + A) & 0xFFFF
      Addr = (getSymbol().getValue() + RawRel.r_addend) & 0xFFFFu;
      Mask = 0x003FFFC0u;
      Shift = 6u;
      break;
    case R_NIOS2_HIADJ16:
      // Adj(S + A)
      Addr = Adj(getSymbol().getValue() + RawRel.r_addend);
      Mask = 0x003FFFC0u;
      Shift = 6u;
      break;
    case R_NIOS2_BFD_RELOC_32:
      // S + A
      Addr = getSymbol().getValue() + RawRel.r_addend;
      Mask = 0xFFFFFFFFu;
      Shift = 0u;
      break;
    case R_NIOS2_BFD_RELOC_16:
      // (S + A) & 0xFFFF
      Addr = (getSymbol().getValue() + RawRel.r_addend) & 0xFFFFu;
      Mask = 0xFFFFu;
      Shift = 0u;
      break;
    case R_NIOS2_BFD_RELOC_8:
      // (S + A) & 0xFF
      Addr = (getSymbol().getValue() + RawRel.r_addend) & 0xFFu;
      Mask = 0xFFu;
      Shift = 0u;
      break;
    case R_NIOS2_GPREL: {
      // (S + A - GP) & 0xFFFF
      auto Res = Container.getElf().findSymbol("_gp");
      assert(Res.first && "no global pointer symbol");
      auto GP = Res.first->getSymbol(Res.second);
      Addr = (getSymbol().getValue() + RawRel.r_addend - GP.getValue()) & 0xFFFFu;
      Mask = 0x003FFFC0u;
      Shift = 6u;
      break;
    }
    case R_NIOS2_GNU_VTINHERIT:
    case R_NIOS2_GNU_VTENTRY:
      return;
    case R_NIOS2_UJMP:
    case R_NIOS2_CJMP:
    case R_NIOS2_CALLR:
      // ((S + A) >> 16) & 0xFFFF, (S + A + 4) & 0xFFFF
      assert(false && "unsupported relocation type");
      break;
    case R_NIOS2_ALIGN:
      return;
    case R_NIOS2_GOT16:
    case R_NIOS2_CALL16:
    case R_NIOS2_GOTOFF_LO:
    case R_NIOS2_GOTOFF_HA:
    case R_NIOS2_PCREL_LO:
    case R_NIOS2_PCREL_HA:
    case R_NIOS2_TLS_GD16:
    case R_NIOS2_TLS_LDM16:
    case R_NIOS2_TLS_LDO16:
    case R_NIOS2_TLS_IE16:
    case R_NIOS2_TLS_LE16:
    case R_NIOS2_TLS_DTPMOD:
    case R_NIOS2_TLS_DTPREL:
    case R_NIOS2_TLS_TPREL:
    case R_NIOS2_COPY:
    case R_NIOS2_GLOB_DAT:
    case R_NIOS2_JUMP_SLOT:
    case R_NIOS2_RELATIVE:
    case R_NIOS2_GOTOFF:
    case R_NIOS2_GOT_LO:
    case R_NIOS2_GOT_HA:
    case R_NIOS2_CALL_LO:
    case R_NIOS2_CALL_HA:
      assert(false && "unsupported relocation type");
      break;
    default:
      assert(false && "Unknown relocation type");
  }

  auto *Loc = reinterpret_cast<uint32_t*>(getTarget());
  *Loc = (((Addr << Shift) & Mask) | (*Loc & ~Mask));
}

template<>
bool Nios2Elf::Reloc::hasImplicitSymbolRefs() const {
  switch (this->getType()) {
    case R_NIOS2_PCREL16:
    case R_NIOS2_GPREL:
      return true;
  }
  return false;
}

} // anonymous namespace

// Objects for target device managements.
namespace {

// Name of ELF section where compiler puts entry table
const char * const EntryTableSectionName = ".omp_offloading.entries";

// Memory type for allocation on device (default is L4). Can be overridden by
// environment variable OMP_TARGET_NIOS2_MEMORY=[L3|L4].
msof_mem_type_t DeviceMemType = MSOF_MEM_L4;

// FPGA device with instantiated Nios II R2 processors.
class DeviceTy {
public:
  using PtrTy = msof_device_ptr_t;
  using SizeTy = msof_device_size_t;

  const PtrTy NullPtr = 0u;

private:
  // Class representing an instance of a target program. We may have more
  // than one target program associated with the host porcess.
  class ProgramTy {
  public:
    // We can have multiple target programs, thus we need to keep program
    // pointer in addition to the entry address. Thus entry address in
    // target entry table is an address of a pair { ProgramTy*, PtrTy }
    using TableEntryTy = std::pair<ProgramTy*, PtrTy>;

  public:
    ProgramTy(DeviceTy &D) : Device(D) {}

    ~ProgramTy() {
      for (auto Ptr : Ptrs) {
        Device.freeMem(Ptr);
      }
    }

    __tgt_target_table* load(const __tgt_device_image *TgtImage) {
      Nios2Elf Obj;

      // Image start and end
      auto ImageStart = static_cast<const char*>(TgtImage->ImageStart);
      auto ImageSize = static_cast<const char*>(TgtImage->ImageEnd) - ImageStart;

      DP("Parsing device ELF %p...\n", ImageStart);
      if (!Obj.readFromMemory(ImageStart, ImageSize)) {
        return nullptr;
      }

      // Relocate program segments which occupy L3 and L4 memrory types. Find all
      // such program segments, allocate memory for them from appropriate space
      // and relocate them to new address.
      // TODO: optimize memory allocation if multiple segments occupy a contiguous
      // memory chunk.
      const auto &L3 = Device.getL3();
      const auto &L4 = Device.getL4();
      for (auto *Seg : Obj.getSegments()) {
        auto SegAddr = Seg->getAddrRange();
        if (SegAddr.overlaps(L3)) {
          assert(L3.contains(SegAddr) && "Partial overlap with L3");
          if (auto Ptr = Device.allocMem(MSOF_MEM_L3, SegAddr.getSize(), nullptr)) {
            Ptrs.push_front(Ptr);
            DP("Relocating target image segment from %x to %x...\n",
              Seg->getAddrRange().getStart(), Ptr);
            Seg->relocate(Ptr);
          }
          else {
            return nullptr;
          }
          continue;
        }
        if (SegAddr.overlaps(L4)) {
          assert(L4.contains(SegAddr) && "Partial overlap with L4");
          if (auto Ptr = Device.allocMem(MSOF_MEM_L4, SegAddr.getSize(), nullptr)) {
            Ptrs.push_front(Ptr);
            DP("Relocating target image segment from %x to %x...\n",
              Seg->getAddrRange().getStart(), Ptr);
            Seg->relocate(Ptr);
          }
          else {
            return nullptr;
          }
          continue;
        }
      }

      DP("Building target entry table for %p\n", ImageStart);
      if (auto Sec = Obj.findSection(EntryTableSectionName)) {
        // Nios is a 32-bit target, so its entry type differs from host
        struct TgtOffloadEntryTy {
          PtrTy Addr;
          PtrTy Name;
          SizeTy Size;
          int32_t Flags;
          int32_t Reserved;
        };

        assert((Sec->getSize() % sizeof(TgtOffloadEntryTy)) == 0 &&
          "Entry table section size should be a multiple of entry size");

        ptrdiff_t NumEntries = Sec->getSize() / sizeof(TgtOffloadEntryTy);
        assert(NumEntries == TgtImage->EntriesEnd - TgtImage->EntriesBegin &&
          "Host and target entry table size mismatch");

        auto TgtEntries = reinterpret_cast<TgtOffloadEntryTy*>(Sec->getBits());
        assert(TgtEntries != nullptr && "No data in entry table section");

        ET.resize(NumEntries);
        for (int II = 0; II < NumEntries; ++II) {
          // Copy host entry
          ET.Entries[II] = TgtImage->EntriesBegin[II];

          // We can have multiple target programs, thus we need to keep
          // program pointer in addition to the entry address.
          ET.Addrs[II] = { this, TgtEntries[II].Addr };
          ET.Entries[II].addr = &ET.Addrs[II];
        }
      }
      else {
        // No not expect to have target binary with no entry table section
        DP("No entry table section in the target image\n");
        return nullptr;
      }

      // And finally construct relocated ELF for MSOF loader.
      Image.resize(ImageSize);
      Obj.writeToMemory(Image.data(), ImageSize);

      return &ET.Table;
    }

    // Return relocated program image
    const std::vector<char>& getImage() const {
      return Image;
    }

  private:
    DeviceTy &Device;

    // Relocated program image
    std::vector<char> Image;

    // List of pointers that were allocated for L3 && L4 program segments.
    std::forward_list<PtrTy> Ptrs;

    // Target entry table
    struct {
      void resize(size_t Size) {
        Addrs.resize(Size);
        Entries.resize(Size);

        Table.EntriesBegin = Entries.data();
        Table.EntriesEnd = Table.EntriesBegin + Entries.size();
      }

      std::vector<TableEntryTy> Addrs;
      std::vector<__tgt_offload_entry> Entries;
      __tgt_target_table Table;
    } ET;
  };

public:
  bool init() {
    DP("Creating device\n");
    auto Res = msof_device_create(&Device);
    if (Res != MSOF_SUCCESS) {
      DP("Error creating device, error code %d\n", Res);
      return false;
    }

    DP("Reading device properties\n");
    uint32_t MCols, L3Start, L3Size, L4Start, L4Size;
    if (!getDeviceProperty(MSOF_MAX_COLUMNS, &MCols) ||
        !getDeviceProperty(MSOF_L3_START, &L3Start) ||
        !getDeviceProperty(MSOF_L3_SIZE, &L3Size) ||
        !getDeviceProperty(MSOF_L4_START, &L4Start) ||
        !getDeviceProperty(MSOF_L4_SIZE, &L4Size)) {
      return false;
    }

    DP("Max columns: %u\n", MCols);
    DP("L3: start 0x%x, size %u\n", L3Start, L3Size);
    DP("L4: start 0x%x, size %u\n", L4Start, L4Size);

    MaxColumns = MCols;
    L3 = Nios2Elf::AddrRangeTy(L3Start, L3Size);
    L4 = Nios2Elf::AddrRangeTy(L4Start, L4Size);

    return true;
  }

  void fini() {
    Programs.clear();
    if (Device != nullptr) {
      DP("Destroying device\n");
      auto Res = msof_device_destroy(Device);
      if (Res != MSOF_SUCCESS) {
        DP("Error destroying device, error code %d\n", Res);
      }
      Device = nullptr;
    }
  }

  uint32_t getMaxColumns() const {
    return MaxColumns;
  }

  const Nios2Elf::AddrRangeTy& getL3() const {
    return L3;
  }

  const Nios2Elf::AddrRangeTy& getL4() const {
    return L4;
  }

  __tgt_target_table* loadProgram(const __tgt_device_image *Image) {
    std::unique_ptr<ProgramTy> Program(new ProgramTy(*this));
    if (auto *Table = Program->load(Image)) {
      Programs.emplace_front(Program.release());
      return Table;
    }
    return nullptr;
  }

  PtrTy allocMem(msof_mem_type_t Type, SizeTy Size, void *HostPtr) const {
#ifdef OMPTARGET_DEBUG
    auto getTypeStr = [](msof_mem_type_t Type) -> const char* {
      switch (Type) {
        case MSOF_MEM_L2:
          return "L2";
        case MSOF_MEM_L3:
          return "L3";
        case MSOF_MEM_L4:
          return "L4";
        default:
          assert(false && "Unknown memory type");
      }
      return nullptr;
    };
#endif // OMPTARGET_DEBUG

    DP("Allocating %d bytes from %s\n", Size, getTypeStr(Type));
    auto Ptr = msof_mem_alloc(Device, Type, Size, 1, HostPtr);
    if (Ptr == NullPtr) {
      DP("Faled to allocate %d bytes from %s\n", Size, getTypeStr(Type));
    }
    return Ptr;
  }

  void freeMem(PtrTy Ptr) const {
    DP("Deallocating device pointer %x\n", Ptr);
    msof_mem_free(Device, Ptr);
  }

  bool writeMem(PtrTy Dst, const void *Src, SizeTy Size) const {
    DP("Copying %u bytes from host (%p) to device (%x)\n", Size, Src, Dst);
    auto Res = msof_memcpy_host_to_device(Device, Dst, Src, Size, nullptr);
    if (Res != MSOF_SUCCESS) {
      DP("Error writing to device memory, error code %d\n", Res);
      return false;
    }
    return true;
  }

  bool readMem(void *Dst, const PtrTy Src, SizeTy Size) const {
    DP("Copying %u bytes from device (%x) to host (%p)\n", Size, Src, Dst);
    auto Res = msof_memcpy_device_to_host(Device, Dst, Src, Size, nullptr);
    if (Res != MSOF_SUCCESS) {
      DP("Error reading from device memory, error code %d\n", Res);
      return false;
    }
    return true;
  }

  bool runFunction(const void *Func, const std::vector<PtrTy> &Args) const {
    auto Entry = static_cast<const ProgramTy::TableEntryTy*>(Func);

    class AutoContext {
    public:
      ~AutoContext() {
        if (Context != nullptr) {
          auto Res = msof_context_destroy(Context);
          if (Res != MSOF_SUCCESS) {
            DP("Error destroying context, error code %d\n", Res);
          }
          Context = nullptr;
        }
      }

      operator msof_context_t() {
        return Context;
      }
      
      msof_context_t Context = nullptr;
    } Context;

    DP("Creating context\n");
    auto Res = msof_context_create(Device, &Context.Context);
    if (Res != MSOF_SUCCESS) {
      DP("Error creating context, error code %d\n", Res);
      return false;
    }

    DP("Reserving columns\n");
    // TODO: Should update this place once we start supporting multiple columns
    size_t NumColumns = 1u; // MaxColumns;
    std::vector<msof_column_t> Columns(NumColumns);
    Res = msof_column_reserve(Context, MSOF_COLUMN_MAX, &NumColumns,
      Columns.data(), 0, nullptr, nullptr);
    if (Res != MSOF_SUCCESS) {
      DP("Error reserving columns, error code %d\n", Res);
      return false;
    }

    DP("Reserved %lu columns\n", NumColumns);
    if (NumColumns < 1) {
      return false;
    }

    DP("Loading program to context\n");
    const auto &Image = Entry->first->getImage();
    msof_program_t Program = nullptr;
    Res = msof_load_program(Context, Image.data(), Image.size(), &Program);
    if (Res != MSOF_SUCCESS) {
      DP("Error loading program to context, error code %d\n", Res);
      return false;
    }

    // Prepare arguments for run function.
    auto NumArgs = Args.size();
    std::vector<SizeTy> Sizes(NumArgs, sizeof(PtrTy));
    std::vector<const void*> Refs(NumArgs);
    for (size_t II = 0u; II < NumArgs; ++II) {
      Refs[II] = &Args[II];
    }

    DP("Executing function (%x) on device\n", Entry->second);
    Res = msof_run_function(Context, Program, Entry->second,
      NumArgs, Refs.data(), Sizes.data(), nullptr);
    if (Res != MSOF_SUCCESS) {
      DP("Error running function on device, error code %d\n", Res);
      return false;
    }
    return true;
  }

private:
  DeviceTy() {}

  bool getDeviceProperty(msof_device_info_t Prop, uint32_t *Info) const {
    auto Res = msof_get_device_info(Device, Prop, sizeof(uint32_t), Info);
    if (Res != MSOF_SUCCESS) {
      DP("Error getting device property, error code %d\n", Res);
      return false;
    }
    return true;
  }

private:
  // Device handle
  msof_device_t Device = nullptr;

  // Device's L3 and L4 address ranges.
  Nios2Elf::AddrRangeTy L3 = { NullPtr, 0u };
  Nios2Elf::AddrRangeTy L4 = { NullPtr, 0u };

  // Maximum number of columns
  uint32_t MaxColumns = 0u;

  // Target programs
  std::forward_list<std::unique_ptr<ProgramTy>> Programs;

  friend DeviceTy& getDevice();
};

// Single entry point for getting device. Performs one time initialization
// if necessary and returns device instance.
DeviceTy& getDevice() {
  static DeviceTy Device;
  static std::once_flag InitFlag;

  std::call_once(InitFlag, [&]() {
    if (const char *Str = getenv("OMP_TARGET_NIOS2_MEMORY")) {
      if (strcmp(Str, "L3") == 0) {
        DeviceMemType = MSOF_MEM_L3;
      }
      else if (strcmp(Str, "L4") == 0) {
        DeviceMemType = MSOF_MEM_L4;
      }
      else {
        DP("Ignoring unsupported device memory type %s. Should be "
           "either L3 or L4.\n", Str);
      }
    }
    if (Device.init()) {
      atexit([&]() {
        Device.fini();
      });
    }
  });
  return Device;
}

// Cast void* to device pointer type
DeviceTy::PtrTy castToDevicePtr(void *Ptr) {
  auto IntPtr = reinterpret_cast<intptr_t>(Ptr);
  return static_cast<DeviceTy::PtrTy>(IntPtr);
}

} // end anonymous namespace

// Plugin API implementation

int32_t __tgt_rtl_is_valid_binary(__tgt_device_image *Image) {
  auto Ehdr = reinterpret_cast<const Elf32_Ehdr*>(Image->ImageStart);
  if (memcmp(Ehdr->e_ident, ELFMAG, SELFMAG) != 0 ||
      Ehdr->e_machine != EM_ALTERA_NIOS2 ||
      Ehdr->e_type != ET_EXEC) {
    return 0;
  }
  return 1;
}

int32_t __tgt_rtl_number_of_devices() {
  return getDevice().getMaxColumns() > 0 ? 1 : 0;
}

int32_t __tgt_rtl_init_device(int32_t ID) {
  if (__tgt_rtl_number_of_devices() <= 0) {
    return OFFLOAD_FAIL;
  }
  return OFFLOAD_SUCCESS;
}

__tgt_target_table *__tgt_rtl_load_binary(
  int32_t ID,
  __tgt_device_image *Image
) {
  return getDevice().loadProgram(Image);
}

void* __tgt_rtl_data_alloc(int32_t ID, int64_t Size, void *HostPtr) {
  auto Ptr = getDevice().allocMem(DeviceMemType, Size, HostPtr);
  return reinterpret_cast<void*>(Ptr);
}

int32_t __tgt_rtl_data_submit(
  int32_t ID,
  void *TargetPtr,
  void *HostPtr,
  int64_t Size
) {
  if (!getDevice().writeMem(castToDevicePtr(TargetPtr), HostPtr, Size)) {
    return OFFLOAD_FAIL;
  }
  return OFFLOAD_SUCCESS;
}

int32_t __tgt_rtl_data_retrieve(
  int32_t ID,
  void *HostPtr,
  void *TargetPtr,
  int64_t Size
) {
  if (!getDevice().readMem(HostPtr, castToDevicePtr(TargetPtr), Size)) {
    return OFFLOAD_FAIL;
  }
  return OFFLOAD_SUCCESS;
}

int32_t __tgt_rtl_data_delete(int32_t ID, void *TargetPtr) {
  getDevice().freeMem(castToDevicePtr(TargetPtr));
  return OFFLOAD_SUCCESS;
}

int32_t __tgt_rtl_run_target_region(
  int32_t ID,
  void *Entry,
  void **Args,
  ptrdiff_t *Offsets,
  int32_t NumArgs
) {
  // Remove an extra nullptr that is unconditionally added by libomptarget
  // to the list of function arguments.
  if (NumArgs > 0) {
    assert(Args[NumArgs-1] == nullptr && "unexpected last argument");
    if (--NumArgs == 0) {
      Args = nullptr;
      Offsets = nullptr;
    }
  }

  // Cast arguments to device pointers.
  std::vector<DeviceTy::PtrTy> TgtArgs(NumArgs);
  for (int II = 0; II < NumArgs; ++II) {
    TgtArgs[II] = castToDevicePtr(Args[II]) + Offsets[II];
  }

  // And then call the device function.
  if (!getDevice().runFunction(Entry, TgtArgs)) {
    return OFFLOAD_FAIL;
  }
  return OFFLOAD_SUCCESS;
}

int32_t __tgt_rtl_run_target_team_region(
  int32_t ID,
  void *Entry,
  void **Args,
  ptrdiff_t *Offsets,
  int32_t NumArgs,
  int32_t NumTeams,
  int32_t ThreadLimit,
  uint64_t LoopTripcount
) {
  // TODO: target region is executed by one team. Changes might be needed
  // when we enable support for multiple columns.
  return __tgt_rtl_run_target_region(ID, Entry, Args, Offsets, NumArgs);
}
