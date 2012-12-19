#include <CoreFoundation/CFData.h>
#include <mach/machine.h>
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <mach-o/reloc.h>
#include <mach-o/stab.h>
#include <mach-o/x86_64/reloc.h>
#include <sys/cdefs.h>
#include <sys/stat.h>
#include <uuid/uuid.h>
#include <fcntl.h>
#include <algorithm>
#include <cassert>
#include <dlfcn.h>
#include <iostream>
#include <map>
#include <string>
#include <string.h>
#include <vector>
#include "Trie.h"

#include "llvm/ADT/StringMap.h"
#include "llvm/Support/MathExtras.h"

// FIXME: some iterators would be nice.
// FIXME: convert to llvm data structures now that this is on llvm side.
// FIXME: use llvm_unreachable?
// FIXME: switch to the llvm linker once it exists

using llvm::RoundUpToAlignment;

enum RefKind {
  kindNone,
  kindSetTargetAddress,
  kindSubtractTargetAddress,
  kindAddAddend,
  kindSubtractAddend,
  
  kindStoreLittleEndian16,
  kindStoreLittleEndian32,
  kindStoreLittleEndian64,

  kindStoreX86BranchPCRel8,
  kindStoreX86PCRel16,
  kindStoreX86PCRel32, 
  kindStoreX86PCRel32_1, 
  kindStoreX86PCRel32_2, 
  kindStoreX86PCRel32_4, 
  kindStoreX86BranchPCRel32,
  kindStoreX86PCRel32GOTLoad,
 
  kindStoreTargetAddressLittleEndian32,
  kindStoreTargetAddressLittleEndian64,
  kindStoreTargetAddressX86PCRel32,
  kindStoreTargetAddressX86BranchPCRel32
};

// FIXME: used for validation, but we don't validate them.
enum RefCluster {
  k1of1,
  k1of2,
  k1of3,
  k2of2,
  k2of3,
  k3of3,
  k1of5,
  k2of5,
  k3of5,
  k4of5,
  k5of5
};

class SectionData; // forward ref.

class Reference {
public:
  unsigned oidx;      // obj file is associated with ref.
  RefCluster c;
  RefKind kind;

  uint32_t offset;
  
  unsigned tgt_oidx;  // obj file is associated with the target.
  SectionData *target;
  uint64_t addend;
  
  const char *name;
  int32_t symindex;
  
  Reference(unsigned idx, uint32_t off, RefCluster rc, RefKind k, uint64_t a) :
    oidx(idx), c(rc), kind(k), offset(off), tgt_oidx(idx), target(0), addend(a),
    name(0), symindex(-1) {}

  Reference(unsigned idx, uint32_t off, RefCluster rc, RefKind k,
            unsigned tgt_idx, SectionData *t, uint64_t a, int32_t sym) :
    oidx(idx), c(rc), kind(k), offset(off), tgt_oidx(tgt_idx), target(t),
    addend(a), name(0), symindex(sym) {}

  Reference(unsigned idx, uint32_t off, RefCluster rc, RefKind k,
            const char *n, int32_t s, uint64_t a) :
    oidx(idx), c(rc), kind(k), offset(off), tgt_oidx(idx), target(0), addend(a),
    name(n), symindex(s) {}

  void pointTo(SectionData *t, uint64_t a) {
    assert(symindex > 0 && "illegal point-to");
    symindex = -symindex;
    target = t;
    addend = a;
  }
};


// SectionObjectInfo
//   Multiple sections with the same name will map to the same Section.
//   The SectionObjectInfo tracks the mapping of the original object file
//   to the Section. This is useful to translate relocations and other
//   information from the file to the new binary.
struct SectionObjectInfo {
  uint32_t OIdx;       // objectFile index
  uint64_t Address;    // obj section virtual address
  uint64_t FileOffset; // obj section file offset
  unsigned reloff;     // obj relection offsets
  unsigned nrelocs;    // obj number of relocations
  unsigned DataStart;  // maps to the start of this obj to shared Section Data.
  unsigned DataEnd;    // maps to the end of this obj to shared Section Data.
  unsigned Size;       // size

  SectionObjectInfo(uint32_t objIdx, uint64_t address, uint64_t fileOffset,
                    unsigned _reloff, unsigned _nrelocs,
                    unsigned dataStart, unsigned dataEnd) :
  OIdx(objIdx), Address(address), FileOffset(fileOffset), reloff(_reloff),
  nrelocs(_nrelocs), DataStart(dataStart), DataEnd(dataEnd),
  Size(dataEnd - dataStart) {}
  
  SectionObjectInfo() :
  OIdx((uint32_t)-1), Address(0), FileOffset(0), reloff(0),
  nrelocs(0), DataStart(0), DataEnd(0),
  Size(0) {}
};

class SectionData {
  std::string Segment;
  std::string Section;

  // Update address and file offset in final executable.
  uint64_t Address;
  uint64_t FileOffset;

  std::vector<unsigned char> Data;

  unsigned Align;
  unsigned TypeAndAttributes;
  unsigned IndirectOff;

public:
  std::vector<SectionObjectInfo> ObjInfo;

  // FIXME: move private, export iterators.
  unsigned sectnum;
  bool AfterLayout;
  std::vector<Reference> Refs;

  SectionData(const char *, unsigned, const section_64 *, unsigned);
  SectionData(const char *, unsigned, const section *, unsigned);
  SectionData(const char *, const char *, unsigned flags = 0, unsigned align=2);
  
  // Used to add section data to the same section from a different object file.
  void addSectionData(const char *, unsigned, const section_64 *);
  void addSectionData(const char *, unsigned, const section *);
  
  std::vector<unsigned char> &getData() { return Data; }

  bool isVirtual() const { return Section[0] == '.'; }
  bool isSymbolStubs() const { 
    return (TypeAndAttributes & SECTION_TYPE) == S_SYMBOL_STUBS;
  }
  bool isLazyPointers() const { 
    return (TypeAndAttributes & SECTION_TYPE) == S_LAZY_SYMBOL_POINTERS;
  }
  bool isNonLazyPointers() const {
    return (TypeAndAttributes & SECTION_TYPE) == S_NON_LAZY_SYMBOL_POINTERS;
  }  

  uint64_t getAddress() const { assert(AfterLayout); return Address; }
  uint64_t getFileOffset() const { assert(AfterLayout); return FileOffset; }
  uint64_t getSize() const { return Data.size(); }
  void setAddress(uint64_t a) { AfterLayout = true; Address = a; }
  void setFileOffset(uint64_t o) { AfterLayout = true; FileOffset = o; }

  unsigned getAlignment() const { return Align; }
  unsigned getTypeAndAttributes() const { return TypeAndAttributes;}
  
  unsigned getIndirectOffset() const { return IndirectOff; }
  void setIndirectTableOffset(unsigned o) { IndirectOff = o; }

  unsigned getNumObjs() const { return ObjInfo.size(); }
  unsigned getOIdx(unsigned idx) const { return ObjInfo[idx].OIdx; }
  
  // Given an object file index, returns the original section information.
  uint64_t getObjAddress(unsigned oidx) const { return ObjInfo[oidx].Address; }
  uint64_t getObjFileOffset(unsigned oidx) const {return ObjInfo[oidx].FileOffset;} 
  unsigned getObjNrelocs(unsigned oidx) const { return ObjInfo[oidx].nrelocs; }
  unsigned getObjReloff(unsigned oidx) const { return ObjInfo[oidx].reloff; }

  // Indicates if an address in the original object file is included in this
  // section.
  bool isInSectionData(uintptr_t addr, unsigned oidx);
  
  unsigned getStubSize() const { return isSymbolStubs() ? getSize() : 0; }

  const std::string &getSegName() const { return Segment; } 
  const std::string &getSectName() const { return Section; }

  void addFixup(unsigned obj_idx, uint32_t offset, RefCluster c, RefKind k,
                unsigned tgt_idx, SectionData *target,
                uint64_t addend = 0, int32_t symindex = -1);
  void addFixup(unsigned obj_idx, uint32_t offset, RefCluster c, RefKind k,
                uint64_t addend = 0);
  void addFixup(unsigned obj_idx, uint32_t offset, RefCluster c, RefKind k,
                const char *name, int32_t symindex);

  // Returns the start address offset after layout for the executable for the
  // section in the oidx object file.
  uint64_t getAdjAddress(unsigned oidx) const;
  
  // Returns the adjust reference offset after layout for the reference in
  // this section.
  uint32_t getAdjRefOffset(const Reference &R) const;
};

SectionData::SectionData(const char *obj, uint32_t obj_idx,
                         const section *s, unsigned i) 
  : Segment(s->segname, 16), Section(s->sectname, 16),
    Data(obj+s->offset, obj+s->offset+s->size), Align(s->align),
    TypeAndAttributes(s->flags), IndirectOff(0), sectnum(i), AfterLayout(false)
{
  // We are processing files in object order so the obj_idx must be greater
  // than the ObjInfo size.
  assert(obj_idx >= ObjInfo.size() && "Processing objects in invalid order");
  if (obj_idx > ObjInfo.size())
    ObjInfo.resize(obj_idx);
  ObjInfo.push_back(SectionObjectInfo(obj_idx, s->addr, s->offset, s->reloff,
                                      s->nreloc, 0, s->size));
}

SectionData::SectionData(const char *obj, uint32_t obj_idx, const section_64 *s,
                         unsigned i) 
  : Segment(s->segname, 16), Section(s->sectname, 16),
    Data(obj+s->offset, obj+s->offset+s->size), Align(s->align),
    TypeAndAttributes(s->flags), IndirectOff(0), sectnum(i), AfterLayout(false)
{
  // We are processing files in object order so the obj_idx must be greater
  // than the ObjInfo size.
  assert(obj_idx >= ObjInfo.size() && "Processing objects in invalid order");
  if (obj_idx > ObjInfo.size())
    ObjInfo.resize(obj_idx);
  ObjInfo.push_back(SectionObjectInfo(obj_idx, s->addr, s->offset, s->reloff,
                                      s->nreloc, 0, s->size));
}

SectionData::SectionData(const char *seg, const char *sec,
                         unsigned flags, unsigned Alignment)
  : Segment(seg), Section(sec), Align(Alignment), TypeAndAttributes(flags),
    IndirectOff(0), sectnum(0), AfterLayout(false)
{
  ObjInfo.push_back(SectionObjectInfo(0, 0, 0, 0, 0, 0, 0));
}

void SectionData::addSectionData(const char* obj, uint32_t obj_idx,
                                 const section *s) {
  // We are processing files in object order so the obj_idx must be greater
  // than the ObjInfo size.
  assert(obj_idx >= ObjInfo.size() && "Processing objects in invalid order");
  if (obj_idx > ObjInfo.size())
    ObjInfo.resize(obj_idx);
  unsigned start = Data.size();
  Data.insert(Data.end(), obj+s->offset, obj + s->offset + s->size); 
  ObjInfo.push_back(SectionObjectInfo(obj_idx, s->addr, s->offset, s->reloff,
                                      s->nreloc, start, start + s->size));  
}

void SectionData::addSectionData(const char* obj, uint32_t obj_idx,
                                 const section_64 *s) {
  // We are processing files in object order so the obj_idx must be greater
  // than the ObjInfo size.
  assert(obj_idx >= ObjInfo.size() && "Processing objects in invalid order");
  if (obj_idx > ObjInfo.size())
    ObjInfo.resize(obj_idx);
  unsigned start = Data.size();
  Data.insert(Data.end(), obj+s->offset, obj + s->offset + s->size); 
  ObjInfo.push_back(SectionObjectInfo(obj_idx, s->addr, s->offset, s->reloff,
                                      s->nreloc, start, start + s->size));
}


void SectionData::addFixup(unsigned oidx, uint32_t offset, RefCluster c,
                           RefKind k,  unsigned tgt_oidx, SectionData *target,
                           uint64_t addend, int32_t sym) {
  Refs.push_back(Reference(oidx, offset, c, k, tgt_oidx, target, addend, sym));
}
void SectionData::addFixup(unsigned oidx, uint32_t offset, RefCluster c,
                           RefKind k, uint64_t addend) {
  Refs.push_back(Reference(oidx, offset, c, k, addend));
}
void SectionData::addFixup(unsigned oidx, uint32_t offset, RefCluster c,
                           RefKind k, const char *name, int32_t symindex) {
  Refs.push_back(Reference(oidx, offset, c, k, name, symindex, 0));
}

bool SectionData::isInSectionData(uintptr_t addr, unsigned oidx) {
  if (oidx < getNumObjs()) {
    if (getObjAddress(oidx) <= addr &&
        (getObjAddress(oidx) + ObjInfo[oidx].Size) > addr)
    return true;
  }
  return false;
}

uint64_t SectionData::getAdjAddress(unsigned oidx) const {
  // if oidx is beyond the number of objects, this must be for a stub.
  if (oidx > getNumObjs())
    oidx = 0;
  return getAddress() + ObjInfo[oidx].DataStart;
}

uint32_t SectionData::getAdjRefOffset(const Reference &R) const{
  assert(R.oidx < getNumObjs() && "Invalid Reference object number");
  return R.offset + ObjInfo[R.oidx].DataStart;
}

class Segment {
  std::string segname;
  
  std::vector<SectionData *> Sections;
  
  uint64_t BaseAddress;
  uint64_t VMSize;
  uint64_t FileOffset;
  uint64_t FileSize;
  
  vm_prot_t maxprot;
  vm_prot_t initprot;
  
public:
  Segment(const std::string &name);
  
  const std::string &getName() const { return segname; }
  
  uint64_t getBaseAddress() const { return BaseAddress; }
  uint64_t getFileOffset () const { return FileOffset; }
  uint64_t getFileSize() const { return FileSize; }
  uint64_t getVMSize() const { return VMSize; }
  
  void setFileOffset(uint64_t o) { FileOffset = o; }
  void setBaseAddress(uint64_t b) { BaseAddress = b; }
  void setVMSize(uint64_t s) { VMSize = s; }
  void setFileSize(uint64_t s) { FileSize = s; }
  
  vm_prot_t getMaxProt() const { return maxprot; }
  vm_prot_t getInitProt() const { return initprot; }
  
  // FIXME: when we get iterators, make this private.
  size_t getNumSections() const { return Sections.size(); }
  size_t getNumNonVirtualSections() const;
  
  SectionData *getSection(unsigned i) { return Sections[i]; }
  SectionData *getSection(const char *secname);

  void addSection(SectionData *S) { Sections.push_back(S); }
};

Segment::Segment(const std::string &name) 
  : segname(name), BaseAddress(0), VMSize(0), FileOffset(0), FileSize(0),
    maxprot(0), initprot(0) {
  
  if (0 == strcmp(name.c_str(), "__TEXT"))
    initprot = VM_PROT_READ | VM_PROT_EXECUTE;
  else if (0 == strcmp(name.c_str(), "__DATA"))
    initprot = VM_PROT_READ | VM_PROT_WRITE;
  else if (0 == strcmp(name.c_str(), "__LINKEDIT"))
    initprot = VM_PROT_READ;
  
  // FIXME: for iphone, set maxprot = initprot.
  maxprot = VM_PROT_READ | VM_PROT_WRITE | VM_PROT_EXECUTE;
}

size_t Segment::getNumNonVirtualSections() const { 
  size_t count = 0;
  for (unsigned i = 0, e = Sections.size(); i != e; ++i)
    if (!Sections[i]->isVirtual())
      ++count;
  return count;
}

SectionData *Segment::getSection(const char *secname) { 
  for (unsigned i = 0, e = Sections.size(); i != e; ++i)
    if (0 == strcmp(secname, Sections[i]->getSectName().c_str()))
      return Sections[i];
  return 0;
}

class Symbol {
  const char *Name;
  const SectionData *Sec;
  unsigned OIdx;
  
  uint32_t strx;
  uint8_t type;
  uint8_t ordinal;
  uint16_t desc;
  uint64_t value;
  
  Symbol* res_sym;  // resolved symbol

public:
  Symbol(const SectionData *s, unsigned oidx, uint8_t ty, uint8_t o, uint16_t d, uint64_t val,
         const char *str, uint32_t off = 0) 
  : Name(str+off), Sec(0), OIdx(0), strx(0), type(ty), ordinal(o), desc(d),
    value(val), res_sym(NULL) {
    if (isSect()) {
      Sec = s;
      OIdx = oidx;
      value -= Sec->getObjAddress(OIdx);
    }
  }
  const char *getName() const { return Name; }
  
  bool isExternal() const { return (type & N_EXT) != 0; }
  bool isPrivateExtern() const { return (type & N_PEXT) != 0; }
  bool isUndef() const { return getType() == N_UNDF; }
  bool isSect() const { return getType() == N_SECT; }
  
  uint8_t getType() const { return type & N_TYPE; }
  uint8_t getSect() const { return Sec ? Sec->sectnum : ordinal;  }
  uint16_t getDesc() const { return desc; }
  uint8_t getOrdinal() const { return GET_LIBRARY_ORDINAL(desc); }
  
  // Get value relative to the original object file.
  uint64_t getObjValue(unsigned oidx) const { 
    if (isSect() && !isPrivateExtern())
      return value + Sec->getObjAddress(oidx);
    return value;
  }
  
  // Get value relative to the combined location
  uint64_t getValue() const {
    if (isSect() && !isPrivateExtern())
      return value + Sec->getAdjAddress(OIdx);
    return value;
  }
  const SectionData *getSection() const { return Sec; }
  unsigned getOIdx() const { return OIdx; }

  void setDesc(uint16_t d) { desc = d; }
  void setStrx(size_t s) { 
    strx = s;
  }
  void setType(uint8_t t) {
    type = t;
  }

  // Get/set the resolved symbol for this undefined.  The resolved symbol is
  // defined in another file we are linking.  Return NULL if we haven't resolved
  // the symbol.
  Symbol* getResSym() const { return res_sym; }
  void setResSym(Symbol* sym) { res_sym = sym; }

  void writeData(std::vector<unsigned char> &out, bool lp64) {
    if (lp64) {
      struct nlist_64 tmp = { { strx }, type, getSect(), desc, getValue() };
      out.insert(out.end(), (unsigned char *)(&tmp), (unsigned char *)(&tmp+1));
    } else {
      struct nlist tmp = { { 0 }, type, getSect(), desc, getValue() };
      tmp.n_un.n_strx = strx;
      out.insert(out.end(), (unsigned char *)(&tmp), (unsigned char *)(&tmp+1));
    }
  }
  void writeName(std::vector<unsigned char> &out) {
    // Don't emit empty names, set string index to '1', the empty string.
    if (0 == strcmp(Name, "")) {
      strx = 1;
      return;
    }
    out.insert(out.end(), Name, Name+strlen(Name)+1);
  }
};

class MachOBundler {
  bool lp64;
  bool sawDWARF;
  
  const char **ObjFiles;
  size_t       NumObjFiles;
  const char *SrcPath;
  char SrcRealpath[PATH_MAX];
  
  std::vector<unsigned char> *OS;
  
  std::string &Log;
  
  std::vector<Segment> Segments;

  std::vector<Symbol> DebugNotes;
  std::vector<Symbol> LocalSymbols;
  std::vector<Symbol> ExternalSymbols;
  std::vector<Symbol> UndefSymbols;
  bool hasExternalUndefSymbols;

  // Records the start position in the vectors up based on the object file
  // index number we are linking in.
  std::vector<size_t> LocalSymbolsIdxs;
  std::vector<size_t> ExternalSymbolsIdxs;
  std::vector<size_t> UndefSymbolsIdxs;

  std::vector<const char *> Dylibs;
  
  void *libsys;
  
  CFDataRef infodict;

  unsigned NumLoadCommands;
  unsigned LoadCommandsSize;

  unsigned nHeaderCommands;
  unsigned HeaderSize;
  unsigned SegCmdSize;
  unsigned SecCmdSize;
  unsigned NlistSize;
  
  static const unsigned UUIDLoadCommandSize = sizeof(uuid_command);
  static const unsigned SymtabLoadCommandSize = sizeof(symtab_command);
  static const unsigned DysymtabLoadCommandSize = sizeof(dysymtab_command);
  static const unsigned DylibLoadCommandSize = sizeof(dylib_command);
  static const unsigned DyldInfoCommandSize = sizeof(dyld_info_command);
  static const unsigned RelocationInfoSize = 8;
  static const unsigned StubSize = 6;

  struct TargetDesc {
    uint32_t oidx;
    SectionData *atom;
    const char *name;
    int64_t addend;
    int32_t symindex;
  };
  struct SourceDesc {
    uint32_t oidx;
    SectionData *atom;
    uint32_t offset;
    
    SourceDesc(uint32_t o, SectionData *a, uint32_t off) :
      oidx(o), atom(a), offset(off) {}
    SourceDesc() : oidx(0), atom(0), offset(0) {}
  };
  
public:
  explicit MachOBundler(std::vector<std::string*>& objs, const char *p,
                        CFDataRef d, std::vector<unsigned char> &os,
                        std::string &l) 
  : lp64(false), sawDWARF(false), NumObjFiles(objs.size()), SrcPath(p),
    OS(&os), Log(l), hasExternalUndefSymbols(false), libsys(0), infodict(d) {

    // Convert strings into to char*.
    ObjFiles = (const char**) malloc(NumObjFiles * sizeof(char*));
    std::vector<std::string*>::iterator iter = objs.begin();
    std::vector<std::string*>::iterator end = objs.end();

    if (iter != end) {
      ObjFiles[0] = (*iter)->data();
      mach_header *mh = (mach_header *)ObjFiles[0];
      if (mh->magic == 0xfeedfacf)
        lp64 = true;
      ++iter;

      for (int i = 1; iter != end; ++iter, ++i) {
        ObjFiles[i] = (*iter)->data();
        mach_header *lmh = (mach_header *)ObjFiles[i];
        if (lmh->magic == 0xfeedfacf) {
          assert(lp64 == true && "All object files must be the same type");
        }
        assert(lmh->magic == 0xfeedface || lmh->magic == 0xfeedfacf && "not macho");
      }

      nHeaderCommands = mh->ncmds;
      if (lp64) {
        HeaderSize = sizeof(struct mach_header_64);
        SegCmdSize = sizeof(struct segment_command_64);
        SecCmdSize = sizeof(struct section_64);
        NlistSize = sizeof(struct nlist_64);
      } else {
        HeaderSize = sizeof(struct mach_header);
        SegCmdSize = sizeof(struct segment_command);
        SecCmdSize = sizeof(struct section);
        NlistSize = sizeof(struct nlist);
      }
    } else {
      // Assume lp64 if we only want to create an image with the kernel
      // annotation.
      lp64 = true;
      HeaderSize = sizeof(struct mach_header_64);
      SegCmdSize = sizeof(struct segment_command_64);
      SecCmdSize = sizeof(struct section_64);
      NlistSize = sizeof(struct nlist_64);
    }
    libsys = dlopen("/usr/lib/libSystem.B.dylib", RTLD_LAZY);
  }
  ~MachOBundler() { 
    if (libsys) 
      dlclose(libsys); 

    // Delete segments
    for (unsigned i = 0, e = Segments.size(); i != e; ++i) {
      Segment &S = Segments[i];
      for (unsigned si = 0, se = S.getNumSections(); si != se; ++si) {
        SectionData *Sec = S.getSection(si);
        delete Sec;
      }
    }

    // Free object files.
    free(ObjFiles);
  }

  int run();

private:
  Segment &getSegment(const char *segname);
  
  void parseObject();
  void parseSegment(load_command *, unsigned obj_idx);
  void parseSegment64(load_command *, unsigned obj_idx);
  void parseSymtab(load_command *, unsigned obj_idx);
  int addStubs();

  void adjustLoadCommandsAndPadding();
  void adjustSections(Segment &, uint64_t, uint64_t &, unsigned &);
  void processSegments();
  void synthesizeDebugNotes();

  void encodeCompressedRebaseInfo();
  void encodeCompressedBindingInfo();
  void encodeCompressedLazyBindingInfo(std::vector<uint32_t> &);
  void encodeCompressedExportInfo();
  
  void buildSymbolTable();
  void buildExecutableFixups();
  void adjustLinkEditSections();
  void writeHeaderSectionData();

  // Relocation crud
  const Symbol &symbolForIndex(unsigned oidx, uint32_t index) const;
  const char *nameForSymbolIndex(unsigned oidx, uint32_t index) const;
  
  SectionData *sectionForAddress(uintptr_t addr, unsigned oidx);
  SectionData *sectionForSectNum(unsigned sectNum);
  void findTargetFromAddressAndSection(intptr_t addr, unsigned sect, 
                                       unsigned oidx, TargetDesc &target);
  
  void addFixups(const SourceDesc &src, RefKind k, const TargetDesc& target);

  void parseRelocations();
  void parseRelocations_x86(const SectionData *);
  void parseRelocations_x86_64(const SectionData *);
  
  // Return the index, starting at 1, of the dylib we need to load.
  uint8_t getOrdinalForSymbol(const char *symbol) {
    static const char *eng = "/System/Library/Frameworks/OpenCL.framework"
                             "/Libraries/libcldcpuengine.dylib";
    static const char *sys = "/usr/lib/libSystem.B.dylib";

    bool isEngine = (strcmp(symbol, "___ci_gamma") == 0) || 
                    (strcmp(symbol, "___ci_gamma_scalar") == 0) ||
                    (strcmp(symbol, "___printf_cl") == 0);
    if (Dylibs.empty())
      Dylibs.push_back(sys);

    // dyld symbols are in libsystem, but aren't external.
    if (*symbol == 'd')
      return 1;
    
    if (!isEngine) {
      if (0 == dlsym(libsys, ++symbol)) {
        Log += "kernel referenced external symbol \'";
        Log += symbol;
        Log += "\' which could not be found.\n";
        return 0;
      }
      return 1;
    }
    
    if (Dylibs.size() == 1)
      Dylibs.push_back(eng);
    
    return 2;
  }

  void AddSymbol(const Symbol &S) {
    if (S.isUndef()) {
      UndefSymbols.push_back(S);
      if (S.isExternal()) {
        hasExternalUndefSymbols = true;
      }
    } else if (S.isExternal())
      ExternalSymbols.push_back(S);
    else 
      LocalSymbols.push_back(S);
  }

  const SectionData *getSection(uint8_t nsect) {
    if (nsect == 0)
      return 0;
    
    // FIXME: nsect -> section map.
    for (unsigned i = 0, e = Segments.size(); i != e; ++i) {
      Segment &S = Segments[i];
      for (unsigned si = 0, se = S.getNumSections(); si != se; ++si) {
        SectionData *Sec = S.getSection(si);
        if (Sec->sectnum == nsect)
          return Sec;
      }
    }
    return 0;
  }
  
  void Write8(uint8_t Value) {
    OS->push_back(Value);
  }
  
  void Write16(uint16_t Value) {
    Write8(uint8_t(Value >> 0));
    Write8(uint8_t(Value >> 8));
  }
  
  void Write32(uint32_t Value) {
    Write16(uint16_t(Value >> 0));
    Write16(uint16_t(Value >> 16));
  }

  void WriteL(uint64_t Value) {
    if (lp64) {
      Write32(uint32_t(Value >> 0));
      Write32(uint32_t(Value >> 32));
    } else {
      Write32(uint32_t(Value >> 0));
    }
  }
  
  void WriteZeros(unsigned N) {
    const char Zeros[16] = { 0 };

    for (unsigned i = 0, e = N / 16; i != e; ++i)
      OS->insert(OS->end(), Zeros, Zeros + 16);

    OS->insert(OS->end(), Zeros, Zeros + (N % 16));
  }
  
  void WriteString(std::string Str, unsigned ZeroFillSize = 0) {
    OS->insert(OS->end(), Str.begin(), Str.end());
    if (ZeroFillSize)
      WriteZeros(ZeroFillSize - Str.size());
  }
  
  void writeObjectFile();
  
  void WriteHeader();
  void WriteSegmentLoadCommand(Segment &S);
  void WriteSection(const SectionData &SD);
  void WriteUUIDCommand();
  void WriteDyldInfoOnlyCommand();
  void WriteSymtabLoadCommand();
  void WriteDysymtabLoadCommand();
  void WriteLoadDylibCommand(const char *L);
  
  // Returns true if we have an undef symbols which have not been resolved.
  bool hasUndefSymbols() const {
    bool hasUndef = false;
    for (std::vector<Symbol>::const_iterator I = UndefSymbols.begin(), 
         E = UndefSymbols.end(); I != E; ++I) {
      if (I->getResSym() == NULL) {
        hasUndef = true;
        break;
      }
    }
    return hasUndef;
  }
};

Segment &MachOBundler::getSegment(const char *segname) {
  for (unsigned i = 0, e = Segments.size(); i != e; ++i)
    if (0 == strcmp(segname, Segments[i].getName().c_str()))
      return Segments[i];

  Segments.push_back(Segment(segname));
  return Segments.back();
}

int MachOBundler::run() {
  // The general process of supporting multiple files is to process
  // each files together and compute offset, relocations, and fixups
  // for each object file.  Afterward, compute the full layout and
  // translate the offset from the object file to the new executable.
  
  // parse the header, verifying it for each object file, and setting
  // up the load commands and section data.  Note that we will merge
  // all sections with the same name to be one (e.g., __text or __const).
  parseObject();
  
  // Parse relocations now that all sections have been discovered in
  // the object file space.
  parseRelocations();
  
  // For each reference, determine if we need to emit a stub for it.
  if (addStubs() != 0)
    return -1;
  
  if (infodict) {
    static const char *textseg = "__TEXT";
    static const char *infosec = "__opencl";

    Segment &Text = getSegment(textseg);
    Text.addSection(new SectionData(textseg, infosec, 0, 1));
    SectionData *IS = Text.getSection(infosec);
    std::vector<unsigned char> &Info = IS->getData();
    
    const UInt8 *iptr = CFDataGetBytePtr(infodict);
    Info.insert(Info.end(), iptr, iptr + CFDataGetLength(infodict));
  }
  
  // Pad out the mach header and load commands to the beginning of the text.
  adjustLoadCommandsAndPadding();

  // Set the segment permissions and sizes, now that they are all extant.
  // After this is done, we can map each offset for relocations and other
  // things to the new location in the executable.
  processSegments();
  
  // If we're debugging, generate the debug notes.
  synthesizeDebugNotes();
  
  // Generate the symbol table.
  buildSymbolTable();
  
  // Fix section fixups and encode compressed dyld info.
  buildExecutableFixups();
  
  // Fix up linkedit section offsets.
  adjustLinkEditSections();
  
  // Generate mach header
  writeHeaderSectionData();
  
  // Write the sections to the object file.
  writeObjectFile();
  
  return 0;
}

void MachOBundler::parseObject() {
  // Add the first text section, a load commands padding section.
  static const char *padseg = "__TEXT";
  static const char *hdrsec = "._mach_header";
  static const char *padsec = "._load_cmds_pad";
  
  Segment &S = getSegment(padseg);
  S.addSection(new SectionData(padseg, hdrsec));
  S.addSection(new SectionData(padseg, padsec));

  // Process the load commands, initializing our section objects and symbol
  // table.
  for (unsigned j = 0; j < NumObjFiles; ++j) {    
    char *cmd = (char *)ObjFiles[j];
    mach_header *lmh = (mach_header *)cmd;
    unsigned lNHeaderCommands = lmh->ncmds;
    cmd += HeaderSize;

    // Remember the symbols of each type before processing another file.
    LocalSymbolsIdxs.push_back(LocalSymbols.size());
    ExternalSymbolsIdxs.push_back(ExternalSymbols.size());
    UndefSymbolsIdxs.push_back(UndefSymbols.size());

    for (unsigned i = 0; i != lNHeaderCommands; ++i) { 
      load_command *lc = (load_command *)cmd;

      switch (lc->cmd) { 
        case LC_SEGMENT:      parseSegment(lc, j);   break;
        case LC_SEGMENT_64:   parseSegment64(lc, j); break;
        case LC_SYMTAB:       parseSymtab(lc, j);    break;
        default: 
          break;
      }
      cmd += lc->cmdsize;
    }
  }

  if (hasExternalUndefSymbols) {
    // Now that we know all the symbols, check if any of the external undefined
    // symbols were defined by another object file.  If so, associate the
    // undef symbol with the definition symbol.
    llvm::StringMap<Symbol*> ExtMap;
    for (std::vector<Symbol>::iterator I = ExternalSymbols.begin(), 
         E = ExternalSymbols.end(); I != E; ++I) {
      Symbol& Sym = *I;
      ExtMap[Sym.getName()] = &Sym;
    }

    for (std::vector<Symbol>::iterator I = UndefSymbols.begin(),
         E = UndefSymbols.end(); I != E; ++I) {
      Symbol& Sym = *I;
      if (Sym.isExternal()) {
        llvm::StringMap<Symbol*>::const_iterator Iter = ExtMap.find(Sym.getName());
        if (Iter != ExtMap.end()) 
          Sym.setResSym(Iter->second);
      }
    }
  }
}

void MachOBundler::parseSegment(load_command *lc, unsigned obj_idx) {
  segment_command *sc = (segment_command *)lc;
  const section *sec = (const section *)((char *)lc + sizeof *sc);
  std::string SegName(sec->segname, 16); 
  Segment &S = getSegment(SegName.c_str());
  
  std::vector<SectionData *> tmp;

  for (unsigned i = 0; i != sc->nsects; ++i, ++sec) {
    if (sec->size == 0)
      continue;
    if (0 == strcmp(sec->segname, "__DWARF")) {
      sawDWARF = true;
      continue;
    }
    std::string Name(sec->sectname, 16); 
    SectionData* Data = S.getSection(Name.c_str());
    if (Data) {
      Data->addSectionData(ObjFiles[obj_idx], obj_idx, sec);
    } else {
      Data = new SectionData(ObjFiles[obj_idx], obj_idx, sec, i+1);
      tmp.push_back(Data);
    }
  }
  
  // FIXME: assert that there are no empty segments in the object file.
  for (unsigned i = 0, e = tmp.size(); i != e; ++i)
    S.addSection(tmp[i]);
}

void MachOBundler::parseSegment64(load_command *lc, unsigned obj_idx) {
  segment_command_64 *sc = (segment_command_64 *)lc;
  const section_64 *sec = (const section_64 *)((char *)lc + sizeof *sc);
  std::string SegName(sec->segname, 16); 
  Segment &S = getSegment(SegName.c_str());

  std::vector<SectionData *> tmp;
  
  for (unsigned i = 0; i != sc->nsects; ++i, ++sec) {
    if (sec->size == 0)
      continue;
    if (0 == strcmp(sec->segname, "__DWARF")) {
      sawDWARF = true;
      continue;
    }
    std::string Name(sec->sectname, 16); 
    SectionData* Data = S.getSection(Name.c_str());
    if (Data) {
      Data->addSectionData(ObjFiles[obj_idx], obj_idx, sec);
    } else {
      Data = new SectionData(ObjFiles[obj_idx], obj_idx, sec, i+1);
      tmp.push_back(Data);
    }
  }
  
  // FIXME: assert that there are no empty segments in the object file.
  for (unsigned i = 0, e = tmp.size(); i != e; ++i)
    S.addSection(tmp[i]);
}

void MachOBundler::parseSymtab(load_command *lc, unsigned obj_idx) {
  symtab_command *sc = (symtab_command *)lc;
  const char *strings = (const char *)ObjFiles[obj_idx] + sc->stroff;
  const char *symbols = (const char *)ObjFiles[obj_idx] + sc->symoff;

  // __mh_bundle_header is always at address zero of the first section.
  if (obj_idx == 0) {
    LocalSymbols.push_back(Symbol(getSection(1), obj_idx, N_PEXT|N_SECT, 0, 0, 0,
                           "__mh_bundle_header"));
  }

  for (unsigned i = 0, e = sc->nsyms; i != e; ++i) {
    uint32_t strx = ((nlist_64 *)symbols)->n_un.n_strx;
    uint8_t type =  ((nlist_64 *)symbols)->n_type;
    uint8_t sect =  ((nlist_64 *)symbols)->n_sect;
    uint16_t desc = ((nlist_64 *)symbols)->n_desc;

    uint64_t value = 0;
    if (lp64)
      value = ((nlist_64 *)symbols)->n_value;
    else
      value = ((struct nlist *)symbols)->n_value;

    AddSymbol(Symbol(getSection(sect), obj_idx, type, sect, desc, value, strings, strx));
    symbols += NlistSize;
  }
}

void MachOBundler::parseRelocations() {
  // For each segment, calculate the vm size, vm address, and where to start
  // writing the data to the file.
  for (unsigned i = 0, e = Segments.size(); i != e; ++i) {
    Segment &S = Segments[i];
    for (unsigned si = 0, se = S.getNumSections(); si != se; ++si) {
      SectionData *Sec = S.getSection(si);
      if (lp64)
        parseRelocations_x86_64(Sec);
      else
        parseRelocations_x86(Sec);
    }
  }
}

void MachOBundler::parseRelocations_x86(const SectionData *sect) {
  uint32_t srcAddr;
  uint32_t* fixUpPtr;
  uint32_t contentValue = 0;
  RefKind kind = kindNone;
  TargetDesc target;
  SourceDesc src;

  for (unsigned oidx = 0; oidx < sect->getNumObjs(); ++oidx) {
    const relocation_info *r =
     (const relocation_info *)(ObjFiles[oidx] + sect->getObjReloff(oidx));

    for (unsigned i = 0, n = sect->getObjNrelocs(oidx); i != n; ++i) {
      const relocation_info &ri = r[i];

      if ((ri.r_address & R_SCATTERED) == 0) {
        assert(ri.r_type == GENERIC_RELOC_VANILLA && "illegal relocation type");

        srcAddr = sect->getObjAddress(oidx) + ri.r_address;
        src.oidx = oidx;
        src.atom = sectionForAddress(srcAddr, oidx);
        src.offset = srcAddr - src.atom->getObjAddress(oidx);
        fixUpPtr = (uint32_t*)(ObjFiles[oidx] + sect->getObjFileOffset(oidx) + ri.r_address);

        switch(ri.r_length) {
        case 0:
          contentValue = *(int8_t *)fixUpPtr;
          if (ri.r_pcrel) {
            kind = kindStoreX86BranchPCRel8;
            contentValue += srcAddr + sizeof(uint8_t);
          } else {
            assert(0 && "r_length=0 and r_pcrel=0 not supported");
          }
          break;
        case 1:
          contentValue = *(int16_t *)fixUpPtr;
          if (ri.r_pcrel) {
            kind = kindStoreX86PCRel16;
            contentValue += srcAddr + sizeof(uint16_t);
          } else {
            kind = kindStoreLittleEndian16;
          }
          break;
        case 2:
          contentValue = *fixUpPtr;
          if (ri.r_pcrel) {
            kind = kindStoreX86BranchPCRel32;
            contentValue += srcAddr + sizeof(uint32_t);
          } else {
            kind = kindStoreLittleEndian32;
          }
          break;
        case 3:
          assert(0 && "bad pc-rel vanilla relocation length");
          abort();
        }
        if (ri.r_extern) {
          // Undef symbol resolved with a symbol in another file.
          const Symbol &S = symbolForIndex(oidx, ri.r_symbolnum);
          if (const Symbol* RS = S.getResSym()) {
            findTargetFromAddressAndSection(RS->getObjValue(RS->getOIdx()),
                                            RS->getSect(), RS->getOIdx(),target);
            target.addend += contentValue;
          } else {
            target.oidx = oidx;
            target.atom = NULL;
            target.name = S.getName();
            target.addend = contentValue;
            target.symindex = ri.r_symbolnum;
          }
        } else {
          findTargetFromAddressAndSection(contentValue, ri.r_symbolnum, oidx, target);
        }
        addFixups(src, kind, target);
        continue;
      }
      const scattered_relocation_info *s = (const scattered_relocation_info *)r;
      const scattered_relocation_info &si = s[i];
      const scattered_relocation_info &nsi = s[i+1];
      const relocation_info &nri = r[i+1];
      bool nextRelocIsPair = false;
    
      srcAddr = sect->getObjAddress(oidx) + si.r_address;
      src.oidx = oidx;
      src.atom = sectionForAddress(srcAddr, oidx);
      src.offset = srcAddr - src.atom->getObjAddress(oidx);

      fixUpPtr = (uint32_t*)(ObjFiles[oidx] + sect->getObjFileOffset(oidx) + si.r_address);
  
      // uint32_t nextRelocAddress = 0;
      uint32_t nextRelocValue = 0;
      if ((nri.r_address & R_SCATTERED) == 0) {
        if (nri.r_type == GENERIC_RELOC_PAIR) {
          nextRelocIsPair = true;
          // nextRelocAddress = nri.r_address;
          ++i; // iterator should skip next reloc, since we've consumed it here
        }
      }
      else if (nsi.r_type == GENERIC_RELOC_PAIR) {
        nextRelocIsPair = true;
        // nextRelocAddress = nsi.r_address;
        nextRelocValue = nsi.r_value;
        ++i;
      }

      switch(si.r_type) {
      case GENERIC_RELOC_SECTDIFF:
      case GENERIC_RELOC_LOCAL_SECTDIFF:
      {
        assert(nextRelocIsPair && "SECTDIFF missing following pair");
        
        switch (si.r_length) {
          case 0: 
          case 3:
            assert(0 && "bad length for SECTDIFF");
            abort();
          case 1:
            contentValue = *(int16_t *)fixUpPtr;
            kind = kindStoreLittleEndian16;
            break;
          case 2:
            contentValue = *fixUpPtr;
            kind = kindStoreLittleEndian32;
            break;
        }
        SectionData *from = sectionForAddress(nextRelocValue, oidx);
        SectionData *to   = sectionForAddress(si.r_value, oidx);
        
        uint32_t fromOff = nextRelocValue - from->getObjAddress(oidx);
        uint32_t toOff   = si.r_value - to->getObjAddress(oidx);
        
        int32_t addend = contentValue - (si.r_value - nextRelocValue);
        if (addend < 0) {
          src.atom->addFixup(src.oidx, src.offset, k1of5, kindSetTargetAddress, oidx, to);
          src.atom->addFixup(src.oidx, src.offset, k2of5, kindAddAddend, toOff);
          src.atom->addFixup(src.oidx, src.offset, k3of5, kindSubtractTargetAddress, oidx, from);
          src.atom->addFixup(src.oidx, src.offset, k4of5, kindSubtractAddend, fromOff-addend);
          src.atom->addFixup(src.oidx, src.offset, k5of5, kind);
        } else {
          src.atom->addFixup(src.oidx, src.offset, k1of5, kindSetTargetAddress, oidx, to);
          src.atom->addFixup(src.oidx, src.offset, k2of5, kindAddAddend, toOff+addend);
          src.atom->addFixup(src.oidx, src.offset, k3of5, kindSubtractTargetAddress, oidx, from);
          src.atom->addFixup(src.oidx, src.offset, k4of5, kindSubtractAddend, fromOff);
          src.atom->addFixup(src.oidx, src.offset, k5of5, kind);
        }
        break;
      }
      default:
        assert(0 && "illegal relocation type!");
        abort();
      }
    }
  }
}

static uint32_t x86_64pcreloff(uint8_t r_type) {
  switch (r_type) {
    case X86_64_RELOC_UNSIGNED: // 2326 hack
    case X86_64_RELOC_SIGNED:
      return 4;
    case X86_64_RELOC_SIGNED_1:
      return 5;
    case X86_64_RELOC_SIGNED_2:
      return 6;
    case X86_64_RELOC_SIGNED_4:
      return 8;
  }
  return 0;
}

void MachOBundler::parseRelocations_x86_64(const SectionData *sect) {
  uint8_t *fixUpPtr;
  TargetDesc target;

  for (unsigned oidx = 0; oidx < sect->getNumObjs(); ++oidx) {
    const relocation_info *r =
      (const relocation_info *)(ObjFiles[oidx] + sect->getObjReloff(oidx));

    for (unsigned i = 0, n = sect->getObjNrelocs(oidx); i != n; ++i) {
      const relocation_info &ri = r[i];

      fixUpPtr = (uint8_t*)(ObjFiles[oidx] + sect->getObjFileOffset(oidx) + ri.r_address);
      uint64_t srcAddr = sect->getObjAddress(oidx) + ri.r_address;
      uint64_t contentValue = 0;

      SourceDesc src;
      src.oidx = oidx;
      src.atom = sectionForAddress(srcAddr, oidx);
      src.offset = srcAddr - src.atom->getObjAddress(oidx);
      switch (ri.r_length) {
      case 0:
        contentValue = *fixUpPtr;
        break;
      case 1:
        contentValue = *(int16_t *)fixUpPtr;
        break;
      case 2:
        contentValue = *(int32_t *)fixUpPtr;
        break;
      case 3:
        contentValue = *(uint64_t *)fixUpPtr;
        break;
      }
      target.oidx = oidx;
      target.atom = 0;
      target.name = 0;
      target.addend = 0;
      target.symindex = 0;

      if (ri.r_extern) {
        const Symbol &S = symbolForIndex(oidx, ri.r_symbolnum);
        if ( S.isSect() && (!S.isExternal() || (S.getName()[0] == 'L')) ) {
          // Local symbol to update address.
          findTargetFromAddressAndSection(S.getObjValue(oidx), S.getSect(), oidx, target);
          target.addend += contentValue;
        } else if (const Symbol* RS = S.getResSym()) {
          // Undef symbol resolved with a symbol in another file.
          findTargetFromAddressAndSection(RS->getObjValue(RS->getOIdx()), RS->getSect(),
                                          RS->getOIdx(), target);
          target.addend += contentValue;
        } else {
          target.name = S.getName();
          target.addend = contentValue;
          target.symindex = ri.r_symbolnum;
        }
      } else {
        if (ri.r_pcrel)
          contentValue += srcAddr + x86_64pcreloff(ri.r_type);
        findTargetFromAddressAndSection(contentValue, ri.r_symbolnum, oidx, target);
      }
      switch (ri.r_type) {
      case X86_64_RELOC_UNSIGNED:
        assert(!ri.r_pcrel && "pcrel & RELOC_UNSIGNED not supported");
        switch (ri.r_length) {
          case 0:
          case 1:
            assert(0 && "length < 2 & RELOC_UNSIGNED not supported");
            abort();
            break;
          case 2:
            addFixups(src, kindStoreLittleEndian32, target);
            break;
          case 3:
            addFixups(src, kindStoreLittleEndian64, target);
            break;
        }
        break;
      case X86_64_RELOC_SIGNED:
      case X86_64_RELOC_SIGNED_1:
      case X86_64_RELOC_SIGNED_2:
      case X86_64_RELOC_SIGNED_4:
        assert(ri.r_pcrel && "!pcrel & RELOC_SIGNED not supported");
        assert(ri.r_length == 2);
        switch (ri.r_type) {
          case X86_64_RELOC_SIGNED:
            addFixups(src, kindStoreX86PCRel32, target);
            break;
          case X86_64_RELOC_SIGNED_1:
            if (ri.r_extern)
              target.addend += 1;
            addFixups(src, kindStoreX86PCRel32_1, target);
            break;
          case X86_64_RELOC_SIGNED_2:
            if (ri.r_extern)
              target.addend += 2;
            addFixups(src, kindStoreX86PCRel32_2, target);
            break;
          case X86_64_RELOC_SIGNED_4:
            if (ri.r_extern)
              target.addend += 4;
            addFixups(src, kindStoreX86PCRel32_4, target);
            break;
        }
        break;
      case X86_64_RELOC_BRANCH:
        assert(ri.r_pcrel && "!pcrel & RELOC_BRANCH not supported");
        switch (ri.r_length) {
          case 0:
            addFixups(src, kindStoreX86BranchPCRel8, target);
            break;
          case 2:
            addFixups(src, kindStoreX86BranchPCRel32, target);
            break;
          case 1:
          case 3:
            assert(0 && "odd length & RELOC_BRANCH not supported");
            abort();
            break;
        }
        break;
      case X86_64_RELOC_GOT_LOAD:
        assert(ri.r_extern && "!extern & X86_64_RELOC_GOT_LOAD not supported");
        assert(ri.r_pcrel && "!pcrel & RELOC_BRANCH not supported");
        assert(ri.r_length == 2 &&
               "length!=2 & X86_64_RELOC_GOT_LOAD not supported");
        addFixups(src, kindStoreX86PCRel32GOTLoad, target);
         break;
      case X86_64_RELOC_GOT:
      case X86_64_RELOC_SUBTRACTOR:
// X86_64_RELOC_TLV was introduced in Barolo11A156
#if CL_MAJOR_BUILD_VERSION >= 11
      case X86_64_RELOC_TLV:
#endif
      default:
        assert(0 && "illegal relocation type!");
        abort();
      }
    }
  }
}

SectionData *MachOBundler::sectionForAddress(uintptr_t addr, unsigned oidx) {
  // For each segment, calculate the vm size, vm address, and where to start
  // writing the data to the file.
  for (unsigned i = 0, e = Segments.size(); i != e; ++i) {
    Segment &S = Segments[i];
    for (unsigned si = 0, se = S.getNumSections(); si != se; ++si) {
      SectionData *Sec = S.getSection(si);
      if (Sec->isInSectionData(addr, oidx))
        return Sec;
     }
  }
  abort();
}

SectionData *MachOBundler::sectionForSectNum(unsigned sectNum) {
  // For each segment, calculate the vm size, vm address, and where to start
  // writing the data to the file.
  for (unsigned i = 0, e = Segments.size(); i != e; ++i) {
    Segment &S = Segments[i];
    for (unsigned si = 0, se = S.getNumSections(); si != se; ++si) {
      SectionData *Sec = S.getSection(si);
      if (Sec->sectnum == sectNum)
        return Sec;
    }
  }
  abort();
}

void MachOBundler::addFixups(const SourceDesc &src, RefKind setKind,
                             const TargetDesc& target) {
  // some fixup pairs can be combined
  RefCluster cl = k1of3;
  RefKind firstKind = kindSetTargetAddress;
  bool combined = false;
  if ( target.addend == 0 ) {
    cl = k1of1;
    combined = true;
    switch ( setKind ) {
      case kindStoreLittleEndian32:
        firstKind = kindStoreTargetAddressLittleEndian32;
        break;
      case kindStoreLittleEndian64:
        firstKind = kindStoreTargetAddressLittleEndian64;
        break;
      case kindStoreX86BranchPCRel32:
        firstKind = kindStoreTargetAddressX86BranchPCRel32;
        break;
      case kindStoreX86PCRel32:
        firstKind = kindStoreTargetAddressX86PCRel32;
        break;
      default:
        combined = false;
        cl = k1of2;
        break;
    }
  }
  
  if ( target.atom != NULL ) {
    src.atom->addFixup(src.oidx, src.offset, cl, firstKind, target.oidx, target.atom);
  }
  else {
    src.atom->addFixup(src.oidx, src.offset, cl, firstKind, target.name, target.symindex);
  }
  if ( target.addend == 0 ) {
    if ( ! combined )
      src.atom->addFixup(src.oidx, src.offset, k2of2, setKind);
  }
  else {
    src.atom->addFixup(src.oidx, src.offset, k2of3, kindAddAddend, target.addend);
    src.atom->addFixup(src.oidx, src.offset, k3of3, setKind);
  }
}

const Symbol& MachOBundler::symbolForIndex(unsigned oidx, uint32_t index) const {
  bool isLast = (NumObjFiles - 1) == oidx;
  if (oidx == 0)
    index += 1; // local symbols we've added.

  // The Symbol vectors are built across all object file while the index
  // is relative to one object file.  First build the Num[Type]Symbols based
  // on the object file id.
  unsigned FirstLocalSymbol = 0;
  unsigned NumLocalSymbols = ((isLast) ? LocalSymbols.size() :
                                         LocalSymbolsIdxs[oidx+1]) -
                             LocalSymbolsIdxs[oidx];
  unsigned FirstExternalSymbol = FirstLocalSymbol + NumLocalSymbols;
  unsigned NumExternalSymbols = ((isLast) ? ExternalSymbols.size() :
                                            ExternalSymbolsIdxs[oidx+1]) -
                                ExternalSymbolsIdxs[oidx];
  unsigned FirstUndefinedSymbol = FirstExternalSymbol + NumExternalSymbols;
  unsigned NumUndefinedSymbols = ((isLast) ? UndefSymbols.size() :
                                            UndefSymbolsIdxs[oidx+1]) -
                                 UndefSymbolsIdxs[oidx];

  assert((index < (FirstUndefinedSymbol + NumUndefinedSymbols)) &&
         "index out of range");

  // Returns Symbol based on which range it is in.
  if (index >= FirstUndefinedSymbol)
    return UndefSymbols[index - FirstUndefinedSymbol + UndefSymbolsIdxs[oidx]];
  if (index >= FirstExternalSymbol && NumExternalSymbols > 0)
    return ExternalSymbols[index-FirstExternalSymbol+ExternalSymbolsIdxs[oidx]];
  assert(index < NumLocalSymbols && "oops");

  return LocalSymbols[index + LocalSymbolsIdxs[oidx]];
}

const char *MachOBundler::nameForSymbolIndex(unsigned oidx, uint32_t index) const {
  return symbolForIndex(oidx, index).getName();
}

void MachOBundler::findTargetFromAddressAndSection(intptr_t addr, unsigned sect, 
                                                   unsigned oidx, TargetDesc &target) {
  assert(sect != R_ABS && "R_ABS not handled");
  target.oidx = oidx;
  target.atom = sectionForSectNum(sect);
  target.addend = addr - target.atom->getObjAddress(oidx);
  if (target.addend > 1000000)
    abort();
  target.name = 0;
  target.symindex = 0;
}

struct RefCmp {
public:
  bool operator()(const Reference *lhs, const Reference *rhs) {
    return strcmp(lhs->name, rhs->name) < 0;
  }
};

int MachOBundler::addStubs() {
  if (!hasUndefSymbols())
    return 0;

  static const char *textseg = "__TEXT";
  static const char *dataseg = "__DATA"; 

  static const char *stubsec32 = "__symbol_stub";
  static const char *stubsec64 = "__symbol_stub1";
  static const char *stubhlp = "__stub_helper";
  
  static const char *nlzyptr = "__nl_symbol_ptr";
  static const char *lazyptr = "__la_symbol_ptr";
  
  const char *stubsec = lp64 ? stubsec64 : stubsec32;

  // create stub sections.
  uint32_t flags = S_SYMBOL_STUBS;
  uint32_t attrs = S_ATTR_PURE_INSTRUCTIONS | S_ATTR_SOME_INSTRUCTIONS;
  if (!lp64)
    attrs |= S_ATTR_LOC_RELOC;
  
  Segment &Text = getSegment(textseg);
  Text.addSection(new SectionData(textseg, stubsec, flags | attrs, 1));
  Text.addSection(new SectionData(textseg, stubhlp, attrs, 0));

  // FIXME: getSegment() may invalidate the other references. replace all this
  //        byref copying in the final version.
  Segment &Data = getSegment(dataseg);
  Data.addSection(new SectionData(dataseg, nlzyptr, S_NON_LAZY_SYMBOL_POINTERS));
  Data.addSection(new SectionData(dataseg, lazyptr, S_LAZY_SYMBOL_POINTERS));

  // Gather stubable references from sections
  // FIXME: seems like there ought to be a better way to do this.
  std::vector<Reference *> References;
  for (unsigned i = 0, e = Segments.size(); i != e; ++i) {
    Segment &S = Segments[i];
    for (unsigned si = 0, se = S.getNumSections(); si != se; ++si) {
      std::vector<Reference> &Refs = S.getSection(si)->Refs;
      for (unsigned ri = 0, re = Refs.size(); ri != re; ++ri) {
        if (Refs[ri].kind == kindStoreTargetAddressX86BranchPCRel32 &&
            Refs[ri].name != NULL)
          References.push_back(&Refs[ri]);
      }
    }
  }
  
  // Sort by name.
  std::sort(References.begin(), References.end(), RefCmp());
  
  Segment &TS = getSegment(textseg);
  Segment &DS = getSegment(dataseg);
  SectionData *SS = TS.getSection(stubsec);
  SectionData *HS = TS.getSection(stubhlp);
  SectionData *NS = DS.getSection(nlzyptr);
  SectionData *LS = DS.getSection(lazyptr);
  std::vector<unsigned char> &Stubs = SS->getData();
  std::vector<unsigned char> &Help = HS->getData();
  std::vector<unsigned char> &Lazy = LS->getData();
  
  unsigned char HelperHelper64[] = {
    0x4C, 0x8D, 0x1D, 0x00, 0x00, 0x00, 0x00, // leaq dyld_ImageLoaderCache(%rip), %r11
    0x41, 0x53,                               // pushq %r11
    0xFF, 0x25, 0x00, 0x00, 0x00, 0x00,       // jmp *_fast_lazy_bind(%rip)
    0x90                                      // nop
  };
  unsigned char HelperHelper[] = {
    0x68, 0x00, 0x00, 0x00, 0x00,             // pushl %dyld_ImageLoaderCache
    0xFF, 0x25, 0x00, 0x00, 0x00, 0x00,       // jmp *_fast_lazy_bind
    0x90                                      // nop
  };
  unsigned char Helper[] = { 
    0x68, 0x00, 0x00, 0x00, 0x00, // push $lazy-info-offset
    0xE9, 0x00, 0x00, 0x00, 0x00  // jmp helperhelper
  };
  unsigned char Stub[] = { 
    0xFF, 0x25, 0x00, 0x00, 0x00, 0x00  // jmp *foo$lazy_pointer(%rip)
  };

  // Emit the FastStubHelperHelper, and its references to the GOT non-lazy ptr
  // and new non-lazy ptr.
  unsigned char *hh = lp64 ? HelperHelper64 : HelperHelper;
  size_t len = lp64 ? sizeof HelperHelper64 : sizeof HelperHelper;
  Help.insert(Help.end(), hh, hh+len);

  RefKind kind = lp64 ? kindStoreTargetAddressX86PCRel32 : kindStoreTargetAddressLittleEndian32;
  RefKind ptrk = lp64 ? kindStoreTargetAddressLittleEndian64 : kindStoreTargetAddressLittleEndian32;

  uint32_t cic_offset = lp64 ? 3 : 1; // compressed image cache
  HS->addFixup(0, cic_offset, k1of1, kind, 0, NS, lp64 ? 8 : 4);

  uint32_t cfb_offset = lp64 ? 11 : 7; // compressed fast binder
  uint32_t cfb_addend = 0;
  HS->addFixup(0, cfb_offset, k1of1, kind, 0, NS, cfb_addend);

  // Emit the 2 non-lazy pointers required by the helperhelper to the nonlazy
  // pointers section.
  NS->getData().insert(NS->getData().end(), lp64 ? 16 : 8, 0);
  NS->addFixup(0, cfb_addend, k1of1, ptrk, "dyld_stub_binder", 1);
  
  // FIXME: std::map is terrible, but use it for prototyping.
  std::map<int, unsigned> StubMap;
  
  for (unsigned i = 0, e = References.size(); i != e; ++i) {
    Reference *R = References[i];

    // If map hit, update reference to point to stub offset.
    std::map<int, unsigned>::iterator entry;
    entry = StubMap.find(R->symindex);
    if (entry != StubMap.end()) {
      R->pointTo(SS, entry->second);
      continue;
    }
    
    // Create new FastStubHelper, and add a pointer reference to the
    // helperhelper.
    int32_t FastStubHelperOffset = Help.size();
    HS->addFixup(0, FastStubHelperOffset + 6, k1of1, kindStoreTargetAddressX86BranchPCRel32, 0, HS);
    Help.insert(Help.end(), Helper, Helper + sizeof Helper);
                    
    // Create new lazy pointer and reference the helper.
    int32_t LazyPointerOffset = Lazy.size();
    LS->addFixup(0, LazyPointerOffset, k1of1, ptrk, 0, HS, FastStubHelperOffset, -R->symindex);
    Lazy.insert(Lazy.end(), lp64 ? 8 : 4, 0);
    
    // Create new stub and reference lazy pointer.
    unsigned StubOffset = Stubs.size();
    SS->addFixup(0, StubOffset + 2, k1of1, kind, 0, LS, LazyPointerOffset);
    Stubs.insert(Stubs.end(), Stub, Stub + sizeof Stub);
    
    // update undef sec index -> stub map and point reference at new stub.
    StubMap[R->symindex] = StubOffset;
    R->pointTo(SS, StubOffset);
  }
  
  // Add the symbol "dyld_stub_binder", imported from libsystem, which is 
  // used by the non-lazy symbol stuff.
  UndefSymbols.push_back(Symbol(0, 0, N_UNDF|N_EXT, 1, 0, 0, "dyld_stub_binder"));
  LocalSymbols.push_back(Symbol(HS, 0, N_PEXT|N_SECT, 0, 0, 0, " stub helpers"));
  
  // for each undef symbol, set library ordinal.
  int err = 0;
  for (unsigned i = 0, e = UndefSymbols.size(); i != e; ++i) {
    Symbol &U = UndefSymbols[i];
    if (U.getResSym()) {
      uint16_t desc = REFERENCE_FLAG_PRIVATE_DEFINED;
      U.setDesc(desc);
    } else {
      uint8_t ordinal = getOrdinalForSymbol(U.getName());

      // We failed to find the symbol in libsystem or libcldcpuengine.
      if (ordinal == 0) {
        err = -1;
        continue;
      }
      uint16_t desc = REFERENCE_FLAG_UNDEFINED_LAZY;
      U.setDesc(SET_LIBRARY_ORDINAL(desc, ordinal));
    }
  }
  return err;
}

void MachOBundler::adjustLoadCommandsAndPadding() {
  // FIXME: find a better place for this.
  static const char *leseg = "__LINKEDIT";
  static const char *rebas = "._rebase info";
  // NOTE: weak not yet supported.
  static const char *binds = "._binding info";
  static const char *lzybd = "._lzbinding info";
  static const char *exprt = "._export info";
  static const char *lesym = "._symbol_table"; 
  static const char *leind = "._indirect_syms";
  static const char *lestr = "._string_pool";
  
  Segment &S = getSegment(leseg);
  S.addSection(new SectionData(leseg, rebas));
  S.addSection(new SectionData(leseg, binds));
  S.addSection(new SectionData(leseg, lzybd));
  S.addSection(new SectionData(leseg, exprt));
  S.addSection(new SectionData(leseg, lesym));
  S.addSection(new SectionData(leseg, leind));
  S.addSection(new SectionData(leseg, lestr));

  // One load command per segment, one per dylib import, uuid, dyld_info, 
  // symtab, and dysymtab.
  NumLoadCommands = Segments.size() + Dylibs.size() + 4;
  unsigned nonVirtualSections = 0;
  for (unsigned i = 0, e = Segments.size(); i != e; ++i)
    nonVirtualSections += Segments[i].getNumNonVirtualSections();
  
  uint64_t Offset = 0;
  Offset += Segments.size() * SegCmdSize;
  Offset += nonVirtualSections * SecCmdSize;
  Offset += UUIDLoadCommandSize;
  Offset += DyldInfoCommandSize;
  Offset += SymtabLoadCommandSize + DysymtabLoadCommandSize;
  
  for (unsigned i = 0, e = Dylibs.size(); i != e; ++i) {
    uint64_t DylibSize = DylibLoadCommandSize + strlen(Dylibs[i]) + 1;
    Offset += RoundUpToAlignment(DylibSize, lp64 ? 8 : 4);
  }
  
  LoadCommandsSize = Offset;
  uint64_t sizeOfLoadCommandsPlusHeader = Offset + HeaderSize;
  
  // work backwards from end of segment and lay out sections so that extra
  // room goes to padding atom.
  uint64_t Addr = 0;
  uint64_t PaddingSize = 0;
  Segment &Text = getSegment("__TEXT");
  
  // FIXME: get a section iterator.
  for (unsigned i = 0, e = Text.getNumSections(); i != e; ++i) {
    SectionData *Sec = Text.getSection(e - (i + 1));
    if (Sec->getSectName() == "._load_cmds_pad") {
      Addr -= sizeOfLoadCommandsPlusHeader;
      PaddingSize = Addr % 4096;
      break;
    }
    Addr -= Sec->getSize();
    Addr &= (0 - (1 << Sec->getAlignment()));
  }
  
  unsigned MinPad = 32;
  if (PaddingSize < MinPad) {
    int extraPages = (MinPad - PaddingSize + 4095) / 4096;
    PaddingSize += extraPages * 4096;
  }
  
  // Adjust padding size and update section size.
  SectionData *Hdr = Text.getSection("._mach_header");
  SectionData *Pad = Text.getSection("._load_cmds_pad");
  Hdr->getData().resize(sizeOfLoadCommandsPlusHeader);
  Pad->getData().resize(PaddingSize);
}

void MachOBundler::adjustSections(Segment &S, uint64_t Address, 
                                  uint64_t &FileOffset, unsigned &SectNum) {
  SectionData *PrevSec = 0;
  
  for (unsigned si = 0, se = S.getNumSections(); si != se; ++si) {
    SectionData *Sec = S.getSection(si);
    uint64_t Alignment = Sec->getAlignment();
    Address = RoundUpToAlignment(Address, 1 << Alignment);
    
    if (PrevSec)
      FileOffset = Address - PrevSec->getAddress() + PrevSec->getFileOffset();
    
    Sec->setFileOffset(FileOffset);
    Sec->setAddress(Address);
    Sec->sectnum = Sec->isVirtual() ? 0 : SectNum++;
    
    // Even virtual sections occupy address space, since we are not creating
    // an object file.
    Address += Sec->getSize();
    FileOffset += Sec->getSize();
    PrevSec = Sec;
  }
}

void MachOBundler::processSegments() {
  uint64_t FileOffset = 0;
  uint64_t NextContiguousAddress = 0;
  unsigned SectNum = 1;
  
  for (unsigned i = 0, e = Segments.size(); i != e; ++i) {
    Segment &Seg = Segments[i];

    FileOffset = RoundUpToAlignment(FileOffset, 4096);
    Seg.setFileOffset(FileOffset);

    // Set the segment address, and then process each section in this segment
    // to set its address and file offset.
    Seg.setBaseAddress(RoundUpToAlignment(NextContiguousAddress, 4096));
    adjustSections(Seg, Seg.getBaseAddress(), FileOffset, SectNum);
    
    Seg.setFileSize(RoundUpToAlignment(FileOffset - Seg.getFileOffset(), 4096));
    Seg.setVMSize(RoundUpToAlignment(Seg.getFileSize(), 4096));

    if (Seg.getBaseAddress() >= NextContiguousAddress) {
      uint64_t NewAddress = Seg.getBaseAddress() + Seg.getVMSize();
      NextContiguousAddress = RoundUpToAlignment(NewAddress, 4096);
    }
  }
}

struct DebugNoteSorter {
  bool operator()(const Symbol* left, const Symbol *right) {
    return left->getValue() < right->getValue();
  }
};

void MachOBundler::synthesizeDebugNotes() {
  // FIXME: See if this correct when linking multiple object files.
  if (!sawDWARF || !SrcPath)
    return;
  
  char *rpath = realpath(SrcPath, SrcRealpath);
  if (!rpath)
    return;
  
  // change suffix from .c -> .o
  rpath[strlen(rpath)-1] = 'o';
  
  /*
  struct stat st;
  if (0 != stat(rpath, &st))
    return;
  */

  DebugNotes.push_back(Symbol(0, 0, N_SO, 0, 0, 0, "/tmp/"));
  DebugNotes.push_back(Symbol(0, 0, N_SO, 0, 0, 0, "foo.o"));
  DebugNotes.push_back(Symbol(0, 0, N_OSO, 0, 1, 0 /*st.st_mtime*/, rpath));

  // gather symbols
  std::vector<Symbol*> ObjSyms;
  for (unsigned i = 0, e = LocalSymbols.size(); i != e; ++i)
    if (LocalSymbols[i].getSect() != 0 && !LocalSymbols[i].isPrivateExtern())
      ObjSyms.push_back(&LocalSymbols[i]);
  for (unsigned i = 0, e = ExternalSymbols.size(); i != e; ++i)
    ObjSyms.push_back(&ExternalSymbols[i]);
  
  // sort by address
  std::sort(ObjSyms.begin(), ObjSyms.end(), DebugNoteSorter());
  
  bool emittedSOL = false;
  for (unsigned i = 0, e = ObjSyms.size(); i != e; ++i) {
    Symbol *S = ObjSyms[i];
    const SectionData *Sec = S->getSection();
    unsigned Address = S->getValue();

    if (0 == strcmp(Sec->getSectName().c_str(), "__text")) {
      unsigned SymSize = Sec->getAdjAddress(S->getOIdx()) + Sec->getSize() - S->getValue();
      if ((i + 1) != e) {
        unsigned NextAddress = ObjSyms[i+1]->getValue();
        SymSize = std::min(SymSize, NextAddress-Address);
      }
      DebugNotes.push_back(Symbol(Sec, 0, N_BNSYM, 0, 0, Address, ""));
      DebugNotes.push_back(Symbol(Sec, 0, N_FUN, 0, 0, Address, S->getName()));
      if (!emittedSOL) {
        DebugNotes.push_back(Symbol(0, 0, N_SOL, 0, 0, 0, SrcPath));
        emittedSOL = true;
      }
      DebugNotes.push_back(Symbol(0, 0, N_FUN, 0, 0, SymSize, ""));
      DebugNotes.push_back(Symbol(Sec, 0, N_ENSYM, 0, 0, SymSize, ""));
    } else {
      DebugNotes.push_back(Symbol(Sec, 0, N_STSYM, 0, 0, Address, S->getName()));
    }
  }
  DebugNotes.push_back(Symbol(0, 0, N_SO, 0, 0, 0, ""));
}

void MachOBundler::buildSymbolTable() {
  // Symbol Section
  Segment &LE = Segments.back();
  SectionData *SymSec = LE.getSection("._symbol_table");
  SectionData *StrSec = LE.getSection("._string_pool");
  
  std::vector<unsigned char> SymTmp;
  
  std::vector<unsigned char> &SymTab = SymSec->getData();
  std::vector<unsigned char> &StrTab = StrSec->getData();
  
  // burn first byte of string pool (so zero is never a valid string offset)
  StrTab.push_back(' ');
  // make offset 1 always point to an empty string
  StrTab.push_back('\0');
  
  // Write local symbols to string table and set their indices.
  for (std::vector<Symbol>::iterator I = LocalSymbols.begin(), 
       E = LocalSymbols.end(); I != E; ++I) {
    I->setStrx(StrTab.size());

    const char *begin = I->getName();

    StrTab.insert(StrTab.end(), begin, begin+strlen(begin)+1);
    I->writeData(SymTmp, lp64);
  }
  
  // External & Undef symbols already sorted in .o, don't need to re-sort
  // FIXME: assert this.
  for (std::vector<Symbol>::iterator I = ExternalSymbols.begin(), 
       E = ExternalSymbols.end(); I != E; ++I) {
    I->setStrx(StrTab.size());
    I->setType(N_SECT|N_EXT);
    
    I->writeName(StrTab);
    I->writeData(SymTmp, lp64);
  }
  
  for (std::vector<Symbol>::iterator I = UndefSymbols.begin(), 
       E = UndefSymbols.end(); I != E; ++I) {
    if (I->getResSym() == NULL) {
      I->setStrx(StrTab.size());
      I->setType(N_UNDF|N_EXT);

      I->writeName(StrTab);
      I->writeData(SymTmp, lp64);
    }
  }

  for (std::vector<Symbol>::iterator I = DebugNotes.begin(),
       E = DebugNotes.end(); I != E; ++I) {
    I->setStrx(StrTab.size());

    I->writeName(StrTab);
    I->writeData(SymTab, lp64);
  }
  
  // Debug notes appear in symbol table first, but their strings are last.
  SymTab.insert(SymTab.end(), SymTmp.begin(), SymTmp.end());
  
  // The string table is padded to a multiple of 4.
  while (StrTab.size() % 4)
    StrTab.push_back('\0');
}

static void append_uleb128(std::vector<unsigned char> &out, uint64_t value) {
  uint8_t byte;
  do {
    byte = value & 0x7F;
    value &= ~0x7F;
    if ( value != 0 )
      byte |= 0x80;
    out.push_back(byte);
    value = value >> 7;
  } while( byte >= 0x80 );
}

void MachOBundler::encodeCompressedRebaseInfo() {
  if (!hasUndefSymbols())
    return;
  
  SectionData *Sec = getSegment("__LINKEDIT").getSection("._rebase info");
  std::vector<unsigned char> &bytes = Sec->getData();
  
  Segment &Data = getSegment("__DATA");
  SectionData *Ptrs = Data.getSection("__la_symbol_ptr");
  uint64_t lazy_address = Ptrs->getAddress() - Data.getBaseAddress();
  unsigned lazy_count = Ptrs->getSize() / (lp64 ? 8 : 4);
  
  // Technically we would want to sort these by address and type, if we were
  // actually recording rebase info entries from somewhere instead of hard 
  // coding lazy-pointers-only here.
  bytes.push_back(REBASE_OPCODE_SET_TYPE_IMM | REBASE_TYPE_POINTER);
  bytes.push_back(REBASE_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB | 1);
  append_uleb128(bytes, lazy_address);
  if (lazy_count < 15) {
    bytes.push_back(REBASE_OPCODE_DO_REBASE_IMM_TIMES | lazy_count);
  } else {
    bytes.push_back(REBASE_OPCODE_DO_REBASE_ULEB_TIMES);
    append_uleb128(bytes, lazy_count);
  }

  // Append REBASE_TYPE_TEXT_ABSOLUTE32 entries for stubs, helpers
  // on i386.
  if (!lp64) {
    SectionData *Stubs = getSegment("__TEXT").getSection("__symbol_stub");
    unsigned nstubs = Stubs->getSize() / 6;
    uint64_t stub_address = Stubs->getAddress() + 2; // offset to first abs32
    
    bytes.push_back(REBASE_OPCODE_SET_TYPE_IMM | REBASE_TYPE_TEXT_ABSOLUTE32);
    bytes.push_back(REBASE_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB | 0);
    append_uleb128(bytes, stub_address);

    // Skipping 2 bytes after each, rebase the first n-1 stubs.
    if (nstubs > 1) {
      bytes.push_back(REBASE_OPCODE_DO_REBASE_ULEB_TIMES_SKIPPING_ULEB);
      append_uleb128(bytes, nstubs - 1);
      append_uleb128(bytes, 2);
    }
    // Rebase the last stub, skipping 3 bytes, over the leading 0x68 of the
    // stub helper helper.
    bytes.push_back(REBASE_OPCODE_DO_REBASE_ADD_ADDR_ULEB);
    append_uleb128(bytes, 1);
    
    // Rebase the helper helper.
    bytes.push_back(REBASE_OPCODE_DO_REBASE_ADD_ADDR_ULEB);
    append_uleb128(bytes, 2);
    bytes.push_back(REBASE_OPCODE_DO_REBASE_IMM_TIMES | 1);
  }
  
  // pad to pointer size.
  while (bytes.size() % (lp64 ? 8 : 4))
    bytes.push_back('\0');
}

void MachOBundler::encodeCompressedBindingInfo() {
  if (!hasUndefSymbols())
    return;

  SectionData *Sec = getSegment("__LINKEDIT").getSection("._binding info");
  std::vector<unsigned char> &bytes = Sec->getData();
  
  Segment &Data = getSegment("__DATA");
  SectionData *Ptrs = Data.getSection("__nl_symbol_ptr");
  uint64_t address = Ptrs->getAddress() - Data.getBaseAddress();
  
  const char *binder = "dyld_stub_binder";
  bytes.push_back(BIND_OPCODE_SET_DYLIB_ORDINAL_IMM | 1);
  bytes.push_back(BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM | 0);
  bytes.insert(bytes.end(), binder, binder + strlen(binder) + 1);
  bytes.push_back(BIND_OPCODE_SET_TYPE_IMM | BIND_TYPE_POINTER);
  bytes.push_back(BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB | 1);
  append_uleb128(bytes, address);
  bytes.push_back(BIND_OPCODE_DO_BIND);

  // pad to pointer size.
  while (bytes.size() % (lp64 ? 8 : 4))
    bytes.push_back('\0');
}

void MachOBundler::encodeCompressedLazyBindingInfo(std::vector<uint32_t> &I) {
  if (!hasUndefSymbols())
    return;
  
  SectionData *Sec = getSegment("__LINKEDIT").getSection("._lzbinding info");
  std::vector<unsigned char> &bytes = Sec->getData();
  
  SectionData *Hlp = getSegment("__TEXT").getSection("__stub_helper");
  std::vector<unsigned char> &helper = Hlp->getData();
  unsigned char *off = &helper[(lp64 ? 16 : 12)];
  
  Segment &Data = getSegment("__DATA");
  SectionData *Ptrs = Data.getSection("__la_symbol_ptr");
  uint64_t lazy_address = Ptrs->getAddress() - Data.getBaseAddress();
  unsigned lazy_count = Ptrs->getSize() / (lp64 ? 8 : 4);
  
  for (unsigned i = 0; i != lazy_count; ++i) {
    // Write the lazy binding info offset into the stub helper.
    // All stub helpers are 10 bytes. The helper helper is 16/12 for 64/32b.
    unsigned LazyBindingInfoOffset = bytes.size();
    memcpy(off + (i * 10) + 1, &LazyBindingInfoOffset, 4);
    
    // Data is seg #1
    bytes.push_back(BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB | 1);
    append_uleb128(bytes, lazy_address + i * (lp64 ? 8 : 4));
    
    // lookup undef symbol
    uint32_t SymIdx = I[Ptrs->getIndirectOffset() + i];
    SymIdx -= LocalSymbols.size() + ExternalSymbols.size();
    assert(SymIdx < UndefSymbols.size() && "SymIdx is beyond range");
    Symbol &U = UndefSymbols[SymIdx];
    
    const char *symbol = U.getName();
    bytes.push_back(BIND_OPCODE_SET_DYLIB_ORDINAL_IMM | U.getOrdinal());
    bytes.push_back(BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM);
    bytes.insert(bytes.end(), symbol, symbol + strlen(symbol) + 1);
    bytes.push_back(BIND_OPCODE_DO_BIND);
    bytes.push_back(BIND_OPCODE_DONE);
  }
  
  // pad to pointer size.
  while (bytes.size() % (lp64 ? 8 : 4))
    bytes.push_back('\0');
}

struct TrieEntriesSorter {
  bool operator()(const mach_o::trie::Entry& left,
                  const mach_o::trie::Entry& right) {
    return (left.address < right.address);
  }
};

void MachOBundler::encodeCompressedExportInfo() {
  Segment &LE = Segments.back();
  SectionData *S = LE.getSection("._export info");

  std::vector<mach_o::trie::Entry> Entries;
  
  for (unsigned i = 0, e = ExternalSymbols.size(); i != e; ++i) {
    mach_o::trie::Entry entry;
    entry.name = ExternalSymbols[i].getName();
    entry.flags = 0;
    entry.address = ExternalSymbols[i].getValue();
    Entries.push_back(entry);
  }
  
  // sort
  std::sort(Entries.begin(), Entries.end(), TrieEntriesSorter());
  
  // make trie
  std::vector<unsigned char> &bytes = S->getData();
  mach_o::trie::makeTrie(Entries, bytes);
  
  // pad to pointer size.
  while (bytes.size() % (lp64 ? 8 : 4))
    bytes.push_back('\0');
}

void MachOBundler::buildExecutableFixups() {
  std::vector<uint32_t> Indirects;
  
  for (unsigned i = 0, e = Segments.size(); i != e; ++i) {
    Segment &S = Segments[i];
    for (unsigned si = 0, se = S.getNumSections(); si != se; ++si) {
      SectionData *Sec = S.getSection(si);
      std::vector<unsigned char> &bytes = Sec->getData();
      
      if (Sec->isNonLazyPointers() && (Sec->Refs.size() > 0)) {
        assert(Sec->Refs.size() == 1 && "Too many nonlazy references");
        Sec->setIndirectTableOffset(Indirects.size());
        Indirects.push_back(INDIRECT_SYMBOL_ABS);
        Indirects.push_back(INDIRECT_SYMBOL_ABS);
      }
      
      if (Sec->isLazyPointers() || Sec->isSymbolStubs())
        Sec->setIndirectTableOffset(Indirects.size());
      
      SectionData *toTarget;
      SectionData *fromTarget;
      int64_t delta;
      int64_t accumulator = 0;
      
      for (unsigned ri = 0, re = Sec->Refs.size(); ri != re; ++ri) {
        Reference &R = Sec->Refs[ri];
        // Leave pointers to external symbols with zero-fill data.
        if (R.symindex >= 0)
          continue;

        // FIXME: print names to make sure they all come out in alpha order
        // FIXME: document that '2' is the number of local symbols we've added.
        // FIXME: We could precompute some of these values but we should be
        // switching to lld in the future.
        if (Sec->isLazyPointers() || Sec->isSymbolStubs()) {
          // Adjust index to map to the undef symbols based on the object index.
          // The LocalSymbols and ExternalSymbols contains the symbols from all
          // files while the R.symindex is relative to a single file.
          uint32_t adj = 0;
          if (NumObjFiles > 1) {
            bool isLast = (NumObjFiles - 1) == R.oidx;
            if (isLast) {
              adj = LocalSymbolsIdxs[R.oidx] + ExternalSymbolsIdxs[R.oidx];
            } else {
              adj = LocalSymbols.size() + ExternalSymbols.size();
              adj -= LocalSymbolsIdxs[R.oidx+1] - LocalSymbolsIdxs[R.oidx];
              adj -= ExternalSymbolsIdxs[R.oidx+1]-ExternalSymbolsIdxs[R.oidx];
            }
            adj += UndefSymbolsIdxs[R.oidx];
          }
          Indirects.push_back(2 - R.symindex + adj);
        }
        uint8_t *fixUpLocation = &bytes[Sec->getAdjRefOffset(R)];

        switch (R.kind) {
          case kindNone:
            break;
          case kindSetTargetAddress:
            toTarget = R.target;
            accumulator = toTarget->getAdjAddress(R.tgt_oidx);
            break;
          case kindSubtractTargetAddress:
            fromTarget = R.target;
            delta = fromTarget->getAdjAddress(R.tgt_oidx);
            accumulator -= delta;
            break;
          case kindAddAddend:
            accumulator += R.addend;
            break;
          case kindSubtractAddend:
            accumulator -= R.addend;
            break;
          case kindStoreLittleEndian16:
            *(uint16_t *)fixUpLocation = accumulator;
            break;
          case kindStoreLittleEndian32:
            *(uint32_t *)fixUpLocation = accumulator;
            break;
          case kindStoreLittleEndian64:
            *(uint64_t *)fixUpLocation = accumulator;
            break;
          case kindStoreX86BranchPCRel8:
            delta = accumulator - (Sec->getAdjAddress(R.oidx) + R.offset + 1);
            assert((delta <= 127 && delta >= -128) && "out of range");
            *(uint8_t *)fixUpLocation = delta;
            break;
          case kindStoreX86PCRel16:
            delta = accumulator - (Sec->getAdjAddress(R.oidx) + R.offset + 2);
            assert((delta <= 0x7FFF && delta >= -0x7FFF) && "out of range");
            *(uint16_t *)fixUpLocation = delta;
            break;
          case kindStoreX86BranchPCRel32:
            delta = accumulator - (Sec->getAdjAddress(R.oidx) + R.offset + 4);
            assert((delta <= 0x7FFFFFFF && delta >= -0x7FFFFFFF) && "out of range");
            *(uint32_t *)fixUpLocation = delta;
            break;
          case kindStoreTargetAddressLittleEndian32:
            toTarget = R.target;
            accumulator = toTarget->getAdjAddress(R.tgt_oidx) + R.addend;
            *(uint32_t *)fixUpLocation = accumulator;
            break;
          case kindStoreTargetAddressLittleEndian64:
            toTarget = R.target;
            accumulator = toTarget->getAdjAddress(R.tgt_oidx) + R.addend;
            *(uint64_t *)fixUpLocation = accumulator;
            break;
          case kindStoreTargetAddressX86PCRel32: 
          case kindStoreTargetAddressX86BranchPCRel32:
            toTarget = R.target;
            accumulator = toTarget->getAdjAddress(R.tgt_oidx) + R.addend;
            delta = accumulator - (Sec->getAdjAddress(R.oidx) + R.offset + 4);
            assert((delta <= 0x7FFFFFFF && delta >= -0x7FFFFFFF) && "out of range");
            *(uint32_t *)fixUpLocation = delta;
            break;
          case kindStoreX86PCRel32GOTLoad:
          case kindStoreX86PCRel32:
            delta = accumulator - (Sec->getAdjAddress(R.oidx) + R.offset + 4);
            assert((delta <= 0x7FFFFFFF && delta >= -0x7FFFFFFF) && "out of range");
            *(uint32_t *)fixUpLocation = delta;
            break;
          case kindStoreX86PCRel32_1:
            delta = accumulator - (Sec->getAdjAddress(R.oidx) + R.offset + 5);
            assert((delta <= 0x7FFFFFFF && delta >= -0x7FFFFFFF) && "out of range");
            *(uint32_t *)fixUpLocation = delta;
            break;
          case kindStoreX86PCRel32_2:
            delta = accumulator - (Sec->getAdjAddress(R.oidx) + R.offset + 6);
            assert((delta <= 0x7FFFFFFF && delta >= -0x7FFFFFFF) && "out of range");
            *(uint32_t *)fixUpLocation = delta;
            break;
          case kindStoreX86PCRel32_4:
            delta = accumulator - (Sec->getAdjAddress(R.oidx) + R.offset + 8);
            assert((delta <= 0x7FFFFFFF && delta >= -0x7FFFFFFF) && "out of range");
            *(uint32_t *)fixUpLocation = delta;
            break;
        }
      }
    }
  }
  
  // Write the Indirect table data into the indirect table section.
  Segment &LE = Segments.back();
  SectionData *IndSec = LE.getSection("._indirect_syms");

  std::vector<unsigned char> &ID = IndSec->getData();
  unsigned char *begin = (unsigned char *)(&Indirects[0]);
  unsigned char *end = (unsigned char *)(&Indirects[Indirects.size()]);
  ID.insert(ID.end(), begin, end);
  
  encodeCompressedRebaseInfo();
  encodeCompressedBindingInfo();
  encodeCompressedLazyBindingInfo(Indirects);
  encodeCompressedExportInfo();
}

void MachOBundler::adjustLinkEditSections() {
  Segment &Seg = Segments.back();
  
  uint64_t FileOffset = Seg.getFileOffset();
  
  unsigned SectNum = 0x100; // something large
  adjustSections(Seg, Seg.getBaseAddress(), FileOffset, SectNum);

  Seg.setFileSize(FileOffset - Seg.getFileOffset());
  Seg.setVMSize(RoundUpToAlignment(Seg.getFileSize(), 4096));
}

void MachOBundler::writeHeaderSectionData() {
  Segment &Text = getSegment("__TEXT");
  SectionData *Hdr = Text.getSection("._mach_header");

  std::vector<unsigned char> *Tmp = OS;
  OS = &Hdr->getData();
  OS->resize(0);
  
  // Write the header, now that we know the size of the load commands.
  WriteHeader();
  
  // For each segment, calculate the vm size, vm address, and where to start
  // writing the data to the file.
  for (unsigned i = 0, e = Segments.size(); i != e; ++i) {
    Segment &S = Segments[i];
    WriteSegmentLoadCommand(S);
    for (unsigned si = 0, se = S.getNumSections(); si != se; ++si) {
      SectionData *Sec = S.getSection(si);
      if (!Sec->isVirtual())
        WriteSection(*Sec);
    }
  }
  
  // Write UUID
  WriteUUIDCommand();
  
  // Write DYLD_INFO_ONLY
  WriteDyldInfoOnlyCommand();
  
  // Write SYMTAB & DYSYMTAB
  WriteSymtabLoadCommand();
  WriteDysymtabLoadCommand();
  
  // Write LC_LOAD_DYLIB for libsystem & libcldcpuengine
  for (unsigned i = 0, e = Dylibs.size(); i != e; ++i)
    WriteLoadDylibCommand(Dylibs[i]);
  
  OS = Tmp;
}

void MachOBundler::WriteHeader() {
  uint64_t Start = OS->size();
  (void) Start;
  
  Write32(lp64 ? MH_MAGIC_64 : MH_MAGIC);
  Write32(lp64 ? CPU_TYPE_X86_64 : CPU_TYPE_I386);
  Write32(CPU_SUBTYPE_X86_ALL);
  Write32(MH_BUNDLE);
  Write32(NumLoadCommands);
  Write32(LoadCommandsSize);
  Write32(MH_NOUNDEFS | MH_DYLDLINK | MH_TWOLEVEL);

  if (lp64) Write32(0); // reserved
  
  assert(OS->size() - Start == HeaderSize);
}

void MachOBundler::WriteSegmentLoadCommand(Segment &S) {
  uint64_t Start = OS->size();
  (void) Start;
  
  Write32(lp64 ? LC_SEGMENT_64 : LC_SEGMENT);
  Write32(SegCmdSize + S.getNumNonVirtualSections() * SecCmdSize);
  
  WriteString(S.getName(), 16);
  WriteL(S.getBaseAddress()); // vmaddr
  WriteL(S.getVMSize()); // vmsize
  WriteL(S.getFileOffset()); // file offset
  WriteL(S.getFileSize()); // file size
  Write32(S.getMaxProt()); // maxprot
  Write32(S.getInitProt()); // initprot
  Write32(S.getNumNonVirtualSections());
  Write32(0); // flags
  
  assert(OS->size() - Start == SegCmdSize);
}

void MachOBundler::WriteSection(const SectionData &SD) {
  uint64_t Start = OS->size();
  (void) Start;
  
  WriteString(SD.getSectName(), 16);
  WriteString(SD.getSegName(), 16);
  WriteL(SD.getAddress()); // address
  WriteL(SD.getSize()); // size
  Write32(SD.getFileOffset()); // fileoff
  Write32(SD.getAlignment());
  Write32(0); // reloc off
  Write32(0); // num relocs
  Write32(SD.getTypeAndAttributes());
  Write32(SD.getIndirectOffset()); // reserved1
  Write32(SD.getStubSize()); // reserved2

  if (lp64) Write32(0); // reserved3
  
  assert(OS->size() - Start == SecCmdSize);
}

void MachOBundler::WriteUUIDCommand() {
  uint64_t Start = OS->size();
  (void) Start;
  
  Write32(LC_UUID);
  Write32(UUIDLoadCommandSize);
  
  uuid_t uuid;
  uuid_generate_random(uuid);
  
  WriteString(std::string(uuid, uuid+16), 16);
  
  assert(OS->size() - Start == UUIDLoadCommandSize);
}

void MachOBundler::WriteDyldInfoOnlyCommand() {
  uint64_t Start = OS->size();
  (void) Start;

  Segment &LE = Segments.back();
  SectionData *RebSec = LE.getSection("._rebase info");
  SectionData *BndSec = LE.getSection("._binding info");
  SectionData *LzySec = LE.getSection("._lzbinding info");
  SectionData *ExtSec = LE.getSection("._export info");

  Write32(LC_DYLD_INFO_ONLY);
  Write32(sizeof(struct dyld_info_command));
  Write32(RebSec->getFileOffset()); // rebase_off
  Write32(RebSec->getSize()); // rebase_size
  Write32(BndSec->getFileOffset()); // bind_off
  Write32(BndSec->getSize()); // bind_size
  Write32(0); // weak_bind_off
  Write32(0); // weak_bind_size
  Write32(LzySec->getFileOffset()); // lazy_bind_off
  Write32(LzySec->getSize()); // lazy_bind_size
  Write32(ExtSec->getFileOffset()); // export_off
  Write32(ExtSec->getSize()); // export_size
}

void MachOBundler::WriteSymtabLoadCommand() {
  uint64_t Start = OS->size();
  (void) Start;
  
  Segment &LE = Segments.back();
  SectionData *SymSec = LE.getSection("._symbol_table");
  SectionData *StrSec = LE.getSection("._string_pool");
  
  Write32(LC_SYMTAB);
  Write32(SymtabLoadCommandSize);
  Write32(SymSec->getFileOffset());
  Write32(SymSec->getSize() / NlistSize);
  Write32(StrSec->getFileOffset());
  Write32(StrSec->getSize());
  
  assert(OS->size() - Start == SymtabLoadCommandSize);
}

void MachOBundler::WriteDysymtabLoadCommand() {
  uint64_t Start = OS->size();
  (void) Start;
  
  Segment &LE = Segments.back();
  SectionData *IndSec = LE.getSection("._indirect_syms");
  
  // Compute and write symbol table and dysmtable
  unsigned FirstLocalSymbol = 0;
  unsigned NumLocalSymbols = DebugNotes.size() + LocalSymbols.size();
  unsigned FirstExternalSymbol = FirstLocalSymbol + NumLocalSymbols;
  unsigned NumExternalSymbols = ExternalSymbols.size();
  unsigned FirstUndefinedSymbol = FirstExternalSymbol + NumExternalSymbols;
  unsigned NumUndefinedSymbols = UndefSymbols.size();
  
  Write32(LC_DYSYMTAB);
  Write32(DysymtabLoadCommandSize);
  Write32(FirstLocalSymbol);
  Write32(NumLocalSymbols);
  Write32(FirstExternalSymbol);
  Write32(NumExternalSymbols);
  Write32(FirstUndefinedSymbol);
  Write32(NumUndefinedSymbols);
  Write32(0); // tocoff
  Write32(0); // ntoc
  Write32(0); // modtaboff
  Write32(0); // nmodtab
  Write32(0); // extrefsymoff
  Write32(0); // nextrefsyms
  Write32(IndSec->getFileOffset());
  Write32(IndSec->getSize() / sizeof(uint32_t));
  Write32(0); // extreloff
  Write32(0); // nextrel
  Write32(0); // locreloff
  Write32(0); // nlocrel
  
  assert(OS->size() - Start == DysymtabLoadCommandSize);
}

void MachOBundler::WriteLoadDylibCommand(const char *path) {
  uint64_t Start = OS->size();
  (void) Start;
  
  std::string P(path);

  unsigned Size = DylibLoadCommandSize + P.size() + 1;
  Size = RoundUpToAlignment(Size, lp64 ? 8 : 4);
  
  Write32(LC_LOAD_DYLIB);
  Write32(Size);
  Write32(DylibLoadCommandSize);
  Write32(0);
  Write32(0);
  Write32(0);
  WriteString(P, Size-DylibLoadCommandSize);
}

void MachOBundler::writeObjectFile() {
  for (unsigned i = 0, e = Segments.size(); i != e; ++i) {
    Segment &Seg = Segments[i];
    
    for (unsigned si = 0, se = Seg.getNumSections(); si != se; ++si) {
      SectionData *S = Seg.getSection(si);
      uint64_t FileOffset = S->getFileOffset();
      
      //std::cerr << "Write " << S->getSegName() << ", " << S->getSectName()
      //          << " to file @ :" << FileOffset << ", cur pos:" 
      //          << OS->size() << ", Size: " << S->getData().size() << '\n';
      
      if (FileOffset != OS->size()) {
        assert(FileOffset >= OS->size() && "File went backwards!");
        WriteZeros(FileOffset-OS->size());
      }
      WriteString(std::string(S->getData().begin(), S->getData().end()));
    }
  }
}

__BEGIN_DECLS

int Link(std::vector<std::string*>& objs, const char *path, CFDataRef dict,
         std::vector<unsigned char> &dylib, std::string &log);

__END_DECLS


int Link(std::vector<std::string*>& objs , const char *path, CFDataRef dict,
         std::vector<unsigned char> &dylib, std::string &log) {
  MachOBundler B(objs, path, dict, dylib, log);
  return B.run();
}

#ifdef STANDALONE_LINKER
#include <sys/stat.h>
#include <sys/mman.h>

int main(int argc, char **argv) {
  
  if (argc != 2)
    return -1;
    
  struct stat statbuf;
  int fd = open(argv[1], O_RDONLY);
  int ret = fstat(fd, &statbuf);
  if (ret)
    return ret;

  // mmap the input file.
  void *object = mmap(NULL, statbuf.st_size, PROT_READ, MAP_FILE|MAP_PRIVATE, fd, 0);
  if (object == (void *)(MAP_FAILED))
    return -2;

  std::vector<unsigned char> dylib;
  std::string log;
  
  argv[1][strlen(argv[1])-1] = 'c';
  MachOBundler B((const char *)object, argv[1], NULL, dylib, log);
  B.run();
  
  // write out the ouput 
  int fd2 = open("/tmp/ocl.dylib", O_CREAT | O_TRUNC | O_RDWR );
  write(fd2, &dylib[0], dylib.size());

  // clean up
  munmap(object, statbuf.st_size);
  close(fd);
  close(fd2);
  
  return 0;
}

#endif
