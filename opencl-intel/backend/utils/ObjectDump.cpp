// INTEL CONFIDENTIAL
//
// Copyright 2020 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

// This file is ported from llvm-objdump.cpp
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "ObjectDump.h"
#include "llvm/ADT/StringSet.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Demangle/Demangle.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCObjectFileInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCTargetOptions.h"
#include "llvm/Object/ELFObjectFile.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/StringSaver.h"
#include "llvm/Support/TargetSelect.h"

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {
namespace Utils {

using namespace llvm;
using namespace llvm::object;

namespace {

bool isRelocAddressLess(RelocationRef A, RelocationRef B) {
  return A.getOffset() < B.getOffset();
}

uint8_t getElfSymbolType(const ObjectFile *Obj, const SymbolRef &Sym) {
  assert(Obj->isELF());
  if (auto *Elf32LEObj = dyn_cast<ELF32LEObjectFile>(Obj))
    return Elf32LEObj->getSymbol(Sym.getRawDataRefImpl())->getType();
  if (auto *Elf64LEObj = dyn_cast<ELF64LEObjectFile>(Obj))
    return Elf64LEObj->getSymbol(Sym.getRawDataRefImpl())->getType();
  if (auto *Elf32BEObj = dyn_cast<ELF32BEObjectFile>(Obj))
    return Elf32BEObj->getSymbol(Sym.getRawDataRefImpl())->getType();
  if (auto *Elf64BEObj = cast<ELF64BEObjectFile>(Obj))
    return Elf64BEObj->getSymbol(Sym.getRawDataRefImpl())->getType();
  llvm_unreachable("Unsupported binary format");
}

template <class ELFT>
Error addDynamicElfSymbols(const ELFObjectFile<ELFT> *Obj,
                           std::map<SectionRef, SectionSymbolsTy> &AllSymbols) {
  for (auto Symbol : Obj->getDynamicSymbolIterators()) {
    uint8_t SymbolType = Symbol.getELFType();
    if (SymbolType == ELF::STT_SECTION)
      continue;

    Expected<uint64_t> AddressOrErr = Symbol.getAddress();
    if (!AddressOrErr)
      return AddressOrErr.takeError();
    uint64_t &Address = AddressOrErr.get();
    // ELFSymbolRef::getAddress() returns size instead of value for common
    // symbols which is not desirable for disassembly output. Overriding.
    if (SymbolType == ELF::STT_COMMON)
      Address = Obj->getSymbol(Symbol.getRawDataRefImpl())->st_value;

    Expected<StringRef> NameOrErr = Symbol.getName();
    if (!NameOrErr)
      return NameOrErr.takeError();
    StringRef &Name = NameOrErr.get();
    if (Name.empty())
      continue;

    Expected<section_iterator> SecIOrErr = Symbol.getSection();
    if (!SecIOrErr)
      return SecIOrErr.takeError();
    section_iterator &SecI = SecIOrErr.get();
    if (SecI == Obj->section_end())
      continue;

    AllSymbols[*SecI].emplace_back(Address, Name, SymbolType);
  }
  return Error::success();
}

Error addDynamicElfSymbols(const ObjectFile *Obj,
                           std::map<SectionRef, SectionSymbolsTy> &AllSymbols) {
  assert(Obj->isELF());
  if (auto *Elf32LEObj = dyn_cast<ELF32LEObjectFile>(Obj))
    return addDynamicElfSymbols(Elf32LEObj, AllSymbols);
  else if (auto *Elf64LEObj = dyn_cast<ELF64LEObjectFile>(Obj))
    return addDynamicElfSymbols(Elf64LEObj, AllSymbols);
  else if (auto *Elf32BEObj = dyn_cast<ELF32BEObjectFile>(Obj))
    return addDynamicElfSymbols(Elf32BEObj, AllSymbols);
  else if (auto *Elf64BEObj = cast<ELF64BEObjectFile>(Obj))
    return addDynamicElfSymbols(Elf64BEObj, AllSymbols);
  else
    llvm_unreachable("Unsupported binary format");
}

Error addPltEntries(const ObjectFile *Obj,
                    std::map<SectionRef, SectionSymbolsTy> &AllSymbols,
                    StringSaver &Saver) {
  Optional<SectionRef> Plt = None;
  for (const SectionRef &Section : Obj->sections()) {
    Expected<StringRef> SecNameOrErr = Section.getName();
    if (!SecNameOrErr) {
      consumeError(SecNameOrErr.takeError());
      continue;
    }
    if (*SecNameOrErr == ".plt")
      Plt = Section;
  }
  if (Plt) {
    if (auto *ElfObj = dyn_cast<ELFObjectFileBase>(Obj)) {
      for (auto PltEntry : ElfObj->getPltAddresses()) {
        SymbolRef Symbol(PltEntry.first, ElfObj);
        uint8_t SymbolType = getElfSymbolType(Obj, Symbol);

        Expected<StringRef> NameOrErr = Symbol.getName();
        if (!NameOrErr)
          return NameOrErr.takeError();
        StringRef &Name = NameOrErr.get();
        if (!Name.empty())
          AllSymbols[*Plt].emplace_back(
              PltEntry.second, Saver.save((Name + "@plt").str()), SymbolType);
      }
    }
  }
  return Error::success();
}

// Normally the disassembly output will skip blocks of zeroes. This function
// returns the number of zero bytes that can be skipped when dumping the
// disassembly of the instructions in Buf.
size_t countSkippableZeroBytes(ArrayRef<uint8_t> Buf) {
  // Find the number of leading zeroes.
  size_t N = 0;
  while (N < Buf.size() && !Buf[N])
    ++N;

  // We may want to skip blocks of zero bytes, but unless we see
  // at least 8 of them in a row.
  if (N < 8)
    return 0;

  // We skip zeroes in multiples of 4 because do not want to truncate an
  // instruction if it starts with a zero byte.
  return N & ~0x3;
}

// Returns a map from sections to their relocations.
Expected<std::map<SectionRef, std::vector<RelocationRef>>>
getRelocsMap(object::ObjectFile const &Obj) {
  std::map<SectionRef, std::vector<RelocationRef>> Ret;
  uint64_t I = (uint64_t)-1;
  for (SectionRef Sec : Obj.sections()) {
    ++I;
    Expected<section_iterator> RelocatedOrErr = Sec.getRelocatedSection();
    if (!RelocatedOrErr)
      return RelocatedOrErr.takeError();

    section_iterator Relocated = *RelocatedOrErr;
    if (Relocated == Obj.section_end())
      continue;
    std::vector<RelocationRef> &V = Ret[*Relocated];
    for (const RelocationRef &R : Sec.relocations())
      V.push_back(R);
    // Sort relocations by address.
    llvm::stable_sort(V, isRelocAddressLess);
  }
  return Ret;
}

} // namespace

void PrettyPrinter::printInst(MCInstPrinter &IP, const MCInst *MI,
                              ArrayRef<uint8_t> Bytes,
                              object::SectionedAddress Address,
                              raw_fd_ostream &OS, MCSubtargetInfo const &STI,
                              bool NoLeadingAddr, bool NoShowRawInsn) {
  size_t Start = OS.tell();
  if (!NoLeadingAddr)
    OS << format("%8" PRIx64 ":", Address.Address);
  if (!NoShowRawInsn) {
    OS << ' ';
    dumpBytes(Bytes, OS);
  }

  // The output of printInst starts with a tab. Print some spaces so that
  // the tab has 1 column and advances to the target tab stop.
  unsigned TabStop = NoShowRawInsn ? 16 : 40;
  unsigned Column = OS.tell() - Start;
  OS.indent(Column < TabStop - 1 ? TabStop - 1 - Column : 7 - Column % 8);

  if (MI)
    IP.printInst(MI, Address.Address, "", STI, OS);
  else
    OS << "\t<unknown>";
}

ObjectDump::ObjectDump()
    : Demangle(false), DisassembleZeroes(false), InlineRelocs(false),
      NoLeadingAddr(false), NoShowRawInsn(true), PrintImmHex(false),
      StartAddress(0), StopAddress(UINT64_MAX) {
  // Initialize targets and assembly printers/parsers.
  InitializeAllTargetInfos();
  InitializeAllTargetMCs();
  InitializeAllDisassemblers();

  char argv0[] = "ObjectDump";
  char argv1[] = "--x86-asm-syntax=intel";
  std::vector<char *> argv = {argv0, argv1};
  cl::ParseCommandLineOptions((int)argv.size(), &argv[0]);
}

Error ObjectDump::disassembleAll(const MemoryBuffer *ObjBuffer,
                                 raw_fd_ostream &Out) {
  std::error_code EC = object_error::parse_failed;

  Expected<std::unique_ptr<ObjectFile>> ObjOrErr =
      ObjectFile::createObjectFile(*ObjBuffer);
  if (!ObjOrErr)
    return ObjOrErr.takeError();
  ObjectFile *Obj = (*ObjOrErr).get();

  Out << "Format " << Obj->getFileFormatName() << "\n\n";

  // Figure out the target triple.
  Triple TheTriple = Obj->makeTriple();

  // Get the target specific parser.
  std::string Err;
  const Target *TheTarget = TargetRegistry::lookupTarget("", TheTriple, Err);
  if (!TheTarget)
    return make_error<StringError>("can't find target: " + Err, EC);

  // Update the triple name and return the found target.
  std::string TripleName = TheTriple.getTriple();

  // Package up features to be passed to target/subtarget
  SubtargetFeatures Features = Obj->getFeatures();

  std::unique_ptr<const MCRegisterInfo> MRI(
      TheTarget->createMCRegInfo(TripleName));
  if (!MRI)
    return make_error<StringError>("no register info for target " + TripleName,
                                   EC);

  // Set up disassembler.
  MCTargetOptions MCOptions;
  std::unique_ptr<const MCAsmInfo> AsmInfo(
      TheTarget->createMCAsmInfo(*MRI, TripleName, MCOptions));
  if (!AsmInfo)
    return make_error<StringError>("no assembly info for target " + TripleName,
                                   EC);
  std::unique_ptr<const MCSubtargetInfo> STI(
      TheTarget->createMCSubtargetInfo(TripleName, "", Features.getString()));
  if (!STI)
    return make_error<StringError>("no subtarget info for target " + TripleName,
                                   EC);
  std::unique_ptr<const MCInstrInfo> MII(TheTarget->createMCInstrInfo());
  if (!MII)
    return make_error<StringError>(
        "no instruction info for target " + TripleName, EC);
  MCObjectFileInfo MOFI;
  MCContext Ctx(AsmInfo.get(), MRI.get(), &MOFI);
  // FIXME: for now initialize MCObjectFileInfo with default values
  MOFI.InitMCObjectFileInfo(Triple(TripleName), false, Ctx);

  std::unique_ptr<MCDisassembler> DisAsm(
      TheTarget->createMCDisassembler(*STI, Ctx));
  if (!DisAsm)
    return make_error<StringError>("no disassembler for target " + TripleName,
                                   EC);

  std::unique_ptr<const MCInstrAnalysis> MIA(
      TheTarget->createMCInstrAnalysis(MII.get()));

  int AsmPrinterVariant = AsmInfo->getAssemblerDialect();
  std::unique_ptr<MCInstPrinter> IP(TheTarget->createMCInstPrinter(
      Triple(TripleName), AsmPrinterVariant, *AsmInfo, *MII, *MRI));
  if (!IP)
    return make_error<StringError>(
        "no instruction printer for target " + TripleName, EC);
  IP->setPrintImmHex(PrintImmHex);

  return disassembleObject(Out, TheTarget, Obj, Ctx, DisAsm.get(), MIA.get(),
                           IP.get(), STI.get(), InlineRelocs);
}

Error ObjectDump::disassembleObject(
    raw_fd_ostream &Out, const Target *TheTarget, const ObjectFile *Obj,
    MCContext &Ctx, MCDisassembler *DisAsm, const MCInstrAnalysis *MIA,
    MCInstPrinter *IP, const MCSubtargetInfo *STI, bool InlineRelocs) {

  std::map<SectionRef, std::vector<RelocationRef>> RelocMap;
  if (InlineRelocs) {
    auto RelocMapOrErr = getRelocsMap(*Obj);
    if (!RelocMapOrErr)
      return RelocMapOrErr.takeError();
    RelocMap = std::move(*RelocMapOrErr);
  }
  bool Is64Bits = Obj->getBytesInAddress() > 4;

  // Create a mapping from virtual address to symbol name.  This is used to
  // pretty print the symbols while disassembling.
  std::map<SectionRef, SectionSymbolsTy> AllSymbols;
  SectionSymbolsTy AbsoluteSymbols;
  const StringRef FileName = Obj->getFileName();
  for (const SymbolRef &Symbol : Obj->symbols()) {
    Expected<uint64_t> AddressOrErr = Symbol.getAddress();
    if (!AddressOrErr)
      return AddressOrErr.takeError();
    uint64_t &Address = AddressOrErr.get();

    Expected<StringRef> NameOrErr = Symbol.getName();
    if (!NameOrErr)
      return NameOrErr.takeError();
    StringRef &Name = NameOrErr.get();
    if (Name.empty())
      continue;

    uint8_t SymbolType = ELF::STT_NOTYPE;
    if (Obj->isELF()) {
      SymbolType = getElfSymbolType(Obj, Symbol);
      if (SymbolType == ELF::STT_SECTION)
        continue;
    }

    Expected<section_iterator> SecIOrErr = Symbol.getSection();
    if (!SecIOrErr)
      return SecIOrErr.takeError();
    section_iterator &SecI = SecIOrErr.get();
    if (SecI != Obj->section_end())
      AllSymbols[*SecI].emplace_back(Address, Name, SymbolType);
    else
      AbsoluteSymbols.emplace_back(Address, Name, SymbolType);
  }
  if (AllSymbols.empty() && Obj->isELF()) {
    if (Error Err = addDynamicElfSymbols(Obj, AllSymbols))
      return Err;
  }

  BumpPtrAllocator A;
  StringSaver Saver(A);
  if (Error Err = addPltEntries(Obj, AllSymbols, Saver))
    return Err;

  // Create a mapping from virtual address to section.
  std::vector<std::pair<uint64_t, SectionRef>> SectionAddresses;
  for (SectionRef Sec : Obj->sections())
    SectionAddresses.emplace_back(Sec.getAddress(), Sec);
  array_pod_sort(SectionAddresses.begin(), SectionAddresses.end());

  // Sort all the symbols, this allows us to use a simple binary search to find
  // a symbol near an address.
  StringSet<> FoundDisasmFuncsSet;
  for (std::pair<const SectionRef, SectionSymbolsTy> &SecSyms : AllSymbols)
    array_pod_sort(SecSyms.second.begin(), SecSyms.second.end());
  array_pod_sort(AbsoluteSymbols.begin(), AbsoluteSymbols.end());

  for (const SectionRef &Section : Obj->sections()) {
    uint64_t SectionAddr = Section.getAddress();
    uint64_t SectSize = Section.getSize();
    if (!SectSize)
      continue;

    // Get the list of all the symbols in this section.
    SectionSymbolsTy &Symbols = AllSymbols[Section];

    StringRef SegmentName = "";

    Expected<StringRef> SectionNameOrErr = Section.getName();
    if (!SectionNameOrErr)
      return SectionNameOrErr.takeError();
    StringRef &SectionName = SectionNameOrErr.get();
    // If the section has no symbol at the start, just insert a dummy one.
    if (Symbols.empty() || Symbols[0].Addr != 0) {
      Symbols.insert(
          Symbols.begin(),
          SymbolInfoTy(SectionAddr, SectionName,
                       Section.isText() ? ELF::STT_FUNC : ELF::STT_OBJECT));
    }

    SmallString<40> Comments;
    raw_svector_ostream CommentStream(Comments);

    Expected<StringRef> ContentsOrErr = Section.getContents();
    if (!ContentsOrErr)
      return ContentsOrErr.takeError();
    ArrayRef<uint8_t> Bytes = arrayRefFromStringRef(ContentsOrErr.get());

    uint64_t Size;
    uint64_t Index;
    bool PrintedSection = false;
    std::vector<RelocationRef> Rels = RelocMap[Section];
    std::vector<RelocationRef>::const_iterator RelCur = Rels.begin();
    std::vector<RelocationRef>::const_iterator RelEnd = Rels.end();
    // Disassemble symbol by symbol.
    for (unsigned SI = 0, SE = Symbols.size(); SI != SE; ++SI) {
      std::string SymbolName = Symbols[SI].Name.str();
      if (Demangle)
        SymbolName = demangle(SymbolName);

      uint64_t Start = Symbols[SI].Addr;
      if (Start < SectionAddr || StopAddress <= Start)
        continue;
      else
        FoundDisasmFuncsSet.insert(SymbolName);

      // The end is the section end, the beginning of the next symbol, or
      // --stop-address.
      uint64_t End = std::min<uint64_t>(SectionAddr + SectSize, StopAddress);
      if (SI + 1 < SE)
        End = std::min(End, Symbols[SI + 1].Addr);
      if (Start >= End || End <= StartAddress)
        continue;
      Start -= SectionAddr;
      End -= SectionAddr;

      if (!PrintedSection) {
        PrintedSection = true;
        Out << "\nDisassembly of section ";
        if (!SegmentName.empty())
          Out << SegmentName << ",";
        Out << SectionName << ":\n";
      }

      Out << '\n';
      if (!NoLeadingAddr)
        Out << format(Is64Bits ? "%016" PRIx64 " " : "%08" PRIx64 " ",
                      SectionAddr + Start);

      Out << SymbolName << ":\n";

      // Don't print raw contents of a virtual section. A virtual section
      // doesn't have any contents in the file.
      if (Section.isVirtual()) {
        Out << "...\n";
        continue;
      }

      // Some targets (like WebAssembly) have a special prelude at the start
      // of each symbol.
      DisAsm->onSymbolStart(SymbolName, Size, Bytes.slice(Start, End - Start),
                            SectionAddr + Start, CommentStream);
      Start += Size;

      Index = Start;
      if (SectionAddr < StartAddress)
        Index = std::max<uint64_t>(Index, StartAddress - SectionAddr);

      while (Index < End) {
        // When -z or --disassemble-zeroes are given we always dissasemble
        // them. Otherwise we might want to skip zero bytes we see.
        if (!DisassembleZeroes) {
          uint64_t MaxOffset = End - Index;
          // For -reloc: print zero blocks patched by relocations, so that
          // relocations can be shown in the dump.
          if (RelCur != RelEnd)
            MaxOffset = RelCur->getOffset() - Index;

          if (size_t N =
                  countSkippableZeroBytes(Bytes.slice(Index, MaxOffset))) {
            Out << "\t\t..." << '\n';
            Index += N;
            continue;
          }
        }

        // Disassemble a real instruction or a data when disassemble all is
        // provided
        MCInst Inst;
        bool Disassembled = DisAsm->getInstruction(
            Inst, Size, Bytes.slice(Index), SectionAddr + Index, CommentStream);
        if (Size == 0)
          Size = 1;

        PIP.printInst(*IP, Disassembled ? &Inst : nullptr,
                      Bytes.slice(Index, Size),
                      {SectionAddr + Index, Section.getIndex()}, Out, *STI,
                      NoLeadingAddr, NoShowRawInsn);
        Out << CommentStream.str();
        Comments.clear();

        // If disassembly has failed, continue with the next instruction, to
        // avoid analysing invalid/incomplete instruction information.
        if (!Disassembled) {
          Out << "\n";
          Index += Size;
          continue;
        }

        // Try to resolve the target of a call, tail call, etc. to a specific
        // symbol.
        if (MIA && (MIA->isCall(Inst) || MIA->isUnconditionalBranch(Inst) ||
                    MIA->isConditionalBranch(Inst))) {
          uint64_t Target;
          if (MIA->evaluateBranch(Inst, SectionAddr + Index, Size, Target)) {
            // In a relocatable object, the target's section must reside in
            // the same section as the call instruction or it is accessed
            // through a relocation.
            //
            // In a non-relocatable object, the target may be in any section.
            //
            // N.B. We don't walk the relocations in the relocatable case yet.
            auto *TargetSectionSymbols = &Symbols;
            if (!Obj->isRelocatableObject()) {
              auto It = partition_point(
                  SectionAddresses,
                  [=](const std::pair<uint64_t, SectionRef> &O) {
                    return O.first <= Target;
                  });
              if (It != SectionAddresses.begin()) {
                --It;
                TargetSectionSymbols = &AllSymbols[It->second];
              } else {
                TargetSectionSymbols = &AbsoluteSymbols;
              }
            }

            // Find the last symbol in the section whose offset is less than
            // or equal to the target. If there isn't a section that contains
            // the target, find the nearest preceding absolute symbol.
            auto TargetSym = partition_point(
                *TargetSectionSymbols,
                [=](const SymbolInfoTy &O) { return O.Addr <= Target; });
            if (TargetSym == TargetSectionSymbols->begin()) {
              TargetSectionSymbols = &AbsoluteSymbols;
              TargetSym =
                  partition_point(AbsoluteSymbols, [=](const SymbolInfoTy &O) {
                    return O.Addr <= Target;
                  });
            }
            if (TargetSym != TargetSectionSymbols->begin()) {
              --TargetSym;
              uint64_t TargetAddress = TargetSym->Addr;
              StringRef TargetName = TargetSym->Name;
              Out << " <" << TargetName;
              uint64_t Disp = Target - TargetAddress;
              if (Disp)
                Out << "+0x" << Twine::utohexstr(Disp);
              Out << '>';
            }
          }
        }
        Out << "\n";

        Index += Size;
      }
    }
  }

  return Error::success();
}

} // namespace Utils
} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
