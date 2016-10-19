//===- lib/MC/LPUAsmStreamer.cpp - Text Assembly Output --------------------===//
//
// **NOTE**: LPUAsmStreamer is quasi-retired. It is compiled but not used.
//
// This is file implements the streamer that writes the LPU assembly file.
// THIS IS A HACK!!!
//
// We write LPU assembly as a series of strings in a data section of an x86
// assembly file.
//
//===----------------------------------------------------------------------===//

#include "llvm/MC/MCStreamer.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/Twine.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCFixupKindInfo.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstPrinter.h"
#include "llvm/MC/MCObjectFileInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSectionCOFF.h"
#include "llvm/MC/MCSectionMachO.h"
#include "llvm/MC/MCSymbolELF.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/Path.h"
#include <cctype>
using namespace llvm;

namespace {

class LPUAsmStreamer : public MCStreamer {
protected:

  std::unique_ptr<formatted_raw_ostream> OSOwner;
  formatted_raw_ostream &OS;
  const MCAsmInfo *MAI;
private:
  std::unique_ptr<MCInstPrinter> InstPrinter;
  std::unique_ptr<MCCodeEmitter> Emitter;
  std::unique_ptr<MCAsmBackend> AsmBackend;

  SmallString<128> CommentToEmit;
  raw_svector_ostream CommentStream;

  unsigned IsVerboseAsm : 1;
  unsigned ShowInst : 1;
  unsigned UseDwarfDirectory : 1;

  void EmitRegisterName(int64_t Register);
  void EmitCFIStartProcImpl(MCDwarfFrameInfo &Frame) override;
  void EmitCFIEndProcImpl(MCDwarfFrameInfo &Frame) override;

public:
  LPUAsmStreamer(MCContext &Context, std::unique_ptr<formatted_raw_ostream> os,
                bool isVerboseAsm, bool useDwarfDirectory,
                MCInstPrinter *printer, MCCodeEmitter *emitter,
                MCAsmBackend *asmbackend, bool showInst)
      : MCStreamer(Context), OSOwner(std::move(os)), OS(*OSOwner),
        MAI(Context.getAsmInfo()), InstPrinter(printer), Emitter(emitter),
        AsmBackend(asmbackend), CommentStream(CommentToEmit),
        IsVerboseAsm(isVerboseAsm), ShowInst(showInst),
        UseDwarfDirectory(useDwarfDirectory) {
    if (InstPrinter && IsVerboseAsm)
      InstPrinter->setCommentStream(CommentStream);
  }

  inline void EmitEOL() {
    // If we don't have any comments, just emit a \n.
    if (!IsVerboseAsm) {
      OS << '\n';
      return;
    }
    EmitCommentsAndEOL();
  }

  void EmitCommentsAndEOL();

  void InitSections(bool NoExecStack) override;

  /// isVerboseAsm - Return true if this streamer supports verbose assembly at
  /// all.
  bool isVerboseAsm() const override { return IsVerboseAsm; }

  /// hasRawTextSupport - We support EmitRawText.
  bool hasRawTextSupport() const override { return true; }

  /// AddComment - Add a comment that can be emitted to the generated .s
  /// file if applicable as a QoI issue to make the output of the compiler
  /// more readable.  This only affects the LPUAsmStreamer, and only when
  /// verbose assembly output is enabled.
  void AddComment(const Twine &T) override;

  /// AddEncodingComment - Add a comment showing the encoding of an instruction.
  void AddEncodingComment(const MCInst &Inst, const MCSubtargetInfo &);

  /// GetCommentOS - Return a raw_ostream that comments can be written to.
  /// Unlike AddComment, you are required to terminate comments with \n if you
  /// use this method.
  raw_ostream &GetCommentOS() override {
    if (!IsVerboseAsm)
      return nulls();  // Discard comments unless in verbose asm mode.
    return CommentStream;
  }

  void emitRawComment(const Twine &T, bool TabPrefix = true) override;

  /// AddBlankLine - Emit a blank line to a .s file to pretty it up.
  void AddBlankLine() override {
    EmitEOL();
  }

  /// @name MCStreamer Interface
  /// @{

  void ChangeSection(MCSection *Section, const MCExpr *Subsection) override;

  void EmitLOHDirective(MCLOHType Kind, const MCLOHArgs &Args) override;
  void EmitLabel(MCSymbol *Symbol) override;

  void EmitAssemblerFlag(MCAssemblerFlag Flag) override;
  void EmitLinkerOptions(ArrayRef<std::string> Options) override;
  void EmitDataRegion(MCDataRegionType Kind) override;
  void EmitVersionMin(MCVersionMinType Kind, unsigned Major, unsigned Minor,
                      unsigned Update) override;
  void EmitThumbFunc(MCSymbol *Func) override;

  void EmitAssignment(MCSymbol *Symbol, const MCExpr *Value) override;
  void EmitWeakReference(MCSymbol *Alias, const MCSymbol *Symbol) override;
  bool EmitSymbolAttribute(MCSymbol *Symbol, MCSymbolAttr Attribute) override;

  void EmitSymbolDesc(MCSymbol *Symbol, unsigned DescValue) override;
  void BeginCOFFSymbolDef(const MCSymbol *Symbol) override;
  void EmitCOFFSymbolStorageClass(int StorageClass) override;
  void EmitCOFFSymbolType(int Type) override;
  void EndCOFFSymbolDef() override;
  void EmitCOFFSectionIndex(MCSymbol const *Symbol) override;
  void EmitCOFFSecRel32(MCSymbol const *Symbol) override;
  void emitELFSize(MCSymbolELF *Symbol, const MCExpr *Value) override;
  void EmitCommonSymbol(MCSymbol *Symbol, uint64_t Size,
                        unsigned ByteAlignment) override;

  /// EmitLocalCommonSymbol - Emit a local common (.lcomm) symbol.
  ///
  /// @param Symbol - The common symbol to emit.
  /// @param Size - The size of the common symbol.
  /// @param ByteAlignment - The alignment of the common symbol in bytes.
  void EmitLocalCommonSymbol(MCSymbol *Symbol, uint64_t Size,
                             unsigned ByteAlignment) override;

  void EmitZerofill(MCSection *Section, MCSymbol *Symbol = nullptr,
                    uint64_t Size = 0, unsigned ByteAlignment = 0) override;

  void EmitTBSSSymbol (MCSection *Section, MCSymbol *Symbol,
                       uint64_t Size, unsigned ByteAlignment = 0) override;

  void EmitBytes(StringRef Data) override;

  void EmitValueImpl(const MCExpr *Value, unsigned Size,
                     const SMLoc &Loc = SMLoc()) override;
  void EmitIntValue(uint64_t Value, unsigned Size) override;

  void EmitULEB128Value(const MCExpr *Value) override;

  void EmitSLEB128Value(const MCExpr *Value) override;

  void EmitGPRel64Value(const MCExpr *Value) override;

  void EmitGPRel32Value(const MCExpr *Value) override;


  void EmitFill(uint64_t NumBytes, uint8_t FillValue) override;

  void EmitValueToAlignment(unsigned ByteAlignment, int64_t Value = 0,
                            unsigned ValueSize = 1,
                            unsigned MaxBytesToEmit = 0) override;

  void EmitCodeAlignment(unsigned ByteAlignment,
                         unsigned MaxBytesToEmit = 0) override;

  bool EmitValueToOffset(const MCExpr *Offset,
                         unsigned char Value = 0) override;

  void EmitFileDirective(StringRef Filename) override;
  unsigned EmitDwarfFileDirective(unsigned FileNo, StringRef Directory,
                                  StringRef Filename,
                                  unsigned CUID = 0) override;
  void EmitDwarfLocDirective(unsigned FileNo, unsigned Line,
                             unsigned Column, unsigned Flags,
                             unsigned Isa, unsigned Discriminator,
                             StringRef FileName) override;
  MCSymbol *getDwarfLineTableSymbol(unsigned CUID) override;

  void EmitIdent(StringRef IdentString) override;
  void EmitCFISections(bool EH, bool Debug) override;
  void EmitCFIDefCfa(int64_t Register, int64_t Offset) override;
  void EmitCFIDefCfaOffset(int64_t Offset) override;
  void EmitCFIDefCfaRegister(int64_t Register) override;
  void EmitCFIOffset(int64_t Register, int64_t Offset) override;
  void EmitCFIPersonality(const MCSymbol *Sym, unsigned Encoding) override;
  void EmitCFILsda(const MCSymbol *Sym, unsigned Encoding) override;
  void EmitCFIRememberState() override;
  void EmitCFIRestoreState() override;
  void EmitCFISameValue(int64_t Register) override;
  void EmitCFIRelOffset(int64_t Register, int64_t Offset) override;
  void EmitCFIAdjustCfaOffset(int64_t Adjustment) override;
  void EmitCFISignalFrame() override;
  void EmitCFIUndefined(int64_t Register) override;
  void EmitCFIRegister(int64_t Register1, int64_t Register2) override;
  void EmitCFIWindowSave() override;

  void EmitWinCFIStartProc(const MCSymbol *Symbol) override;
  void EmitWinCFIEndProc() override;
  void EmitWinCFIStartChained() override;
  void EmitWinCFIEndChained() override;
  void EmitWinCFIPushReg(unsigned Register) override;
  void EmitWinCFISetFrame(unsigned Register, unsigned Offset) override;
  void EmitWinCFIAllocStack(unsigned Size) override;
  void EmitWinCFISaveReg(unsigned Register, unsigned Offset) override;
  void EmitWinCFISaveXMM(unsigned Register, unsigned Offset) override;
  void EmitWinCFIPushFrame(bool Code) override;
  void EmitWinCFIEndProlog() override;

  void EmitWinEHHandler(const MCSymbol *Sym, bool Unwind, bool Except) override;
  void EmitWinEHHandlerData() override;

  void EmitInstruction(const MCInst &Inst, const MCSubtargetInfo &STI) override;

  void EmitBundleAlignMode(unsigned AlignPow2) override;
  void EmitBundleLock(bool AlignToEnd) override;
  void EmitBundleUnlock() override;

  /// EmitRawText - If this file is backed by an assembly streamer, this dumps
  /// the specified string in the output .s file.  This capability is
  /// indicated by the hasRawTextSupport() predicate.
  void EmitRawTextImpl(StringRef String) override;

  void FinishImpl() override;
};

} // end anonymous namespace.

// This is the first thing called on the streamer. We'll end up putting out
// the .text directive
void LPUAsmStreamer::InitSections(bool NoExecStack) {
  MCStreamer::InitSections(NoExecStack);

  // Add the global we'll lookup to find the CSA assembly
  OS << "\t.globl lpu_assembly\n\n";
  OS << "lpu_assembly:\n";
  OS << "\t.ascii \"\\t.text\\n\"\n";
}

/// AddComment - Add a comment that can be emitted to the generated .s
/// file if applicable as a QoI issue to make the output of the compiler
/// more readable.  This only affects the LPUAsmStreamer, and only when
/// verbose assembly output is enabled.
void LPUAsmStreamer::AddComment(const Twine &T) {
  if (!IsVerboseAsm) return;

  // Make sure that CommentStream is flushed.
  CommentStream.flush();

  T.toVector(CommentToEmit);
  // Each comment goes on its own line.
  CommentToEmit.push_back('\n');

  // Tell the comment stream that the vector changed underneath it.
  CommentStream.resync();
}

void LPUAsmStreamer::EmitCommentsAndEOL() {
  if (CommentToEmit.empty() && CommentStream.GetNumBytesInBuffer() == 0) {
    OS << '\n';
    return;
  }

  CommentStream.flush();
  StringRef Comments = CommentToEmit.str();

  assert(Comments.back() == '\n' &&
         "Comment array not newline terminated");
  do {
    // Emit a line of comments.
    OS.PadToColumn(MAI->getCommentColumn());
    size_t Position = Comments.find('\n');
    OS << MAI->getCommentString() << ' ' << Comments.substr(0, Position) <<'\n';

    Comments = Comments.substr(Position+1);
  } while (!Comments.empty());

  CommentToEmit.clear();
  // Tell the comment stream that the vector changed underneath it.
  CommentStream.resync();
}

static inline int64_t truncateToSize(int64_t Value, unsigned Bytes) {
  assert(Bytes && "Invalid size!");
  return Value & ((uint64_t) (int64_t) -1 >> (64 - Bytes * 8));
}

void LPUAsmStreamer::emitRawComment(const Twine &T, bool TabPrefix) {
  if (TabPrefix)
    OS << '\t';
  OS << MAI->getCommentString() << T;
  EmitEOL();
}

void LPUAsmStreamer::ChangeSection(MCSection *Section,
                                  const MCExpr *Subsection) {
  assert(Section && "Cannot switch to a null section!");
  Section->PrintSwitchToSection(*MAI, OS, Subsection);
}

void LPUAsmStreamer::EmitLabel(MCSymbol *Symbol) {
  assert(Symbol->isUndefined() && "Cannot define a symbol twice!");
  MCStreamer::EmitLabel(Symbol);

  OS << *Symbol << MAI->getLabelSuffix();
  EmitEOL();
}

void LPUAsmStreamer::EmitLOHDirective(MCLOHType Kind, const MCLOHArgs &Args) {
  StringRef str = MCLOHIdToName(Kind);

#ifndef NDEBUG
  int NbArgs = MCLOHIdToNbArgs(Kind);
  assert(NbArgs != -1 && ((size_t)NbArgs) == Args.size() && "Malformed LOH!");
  assert(str != "" && "Invalid LOH name");
#endif

  OS << "\t" << MCLOHDirectiveName() << " " << str << "\t";
  bool IsFirst = true;
  for (MCLOHArgs::const_iterator It = Args.begin(), EndIt = Args.end();
       It != EndIt; ++It) {
    if (!IsFirst)
      OS << ", ";
    IsFirst = false;
    OS << **It;
  }
  EmitEOL();
}

void LPUAsmStreamer::EmitAssemblerFlag(MCAssemblerFlag Flag) {
  switch (Flag) {
  case MCAF_SyntaxUnified:         OS << "\t.syntax unified"; break;
  case MCAF_SubsectionsViaSymbols: OS << ".subsections_via_symbols"; break;
  case MCAF_Code16:                OS << '\t'<< MAI->getCode16Directive();break;
  case MCAF_Code32:                OS << '\t'<< MAI->getCode32Directive();break;
  case MCAF_Code64:                OS << '\t'<< MAI->getCode64Directive();break;
  }
  EmitEOL();
}

void LPUAsmStreamer::EmitLinkerOptions(ArrayRef<std::string> Options) {
  assert(!Options.empty() && "At least one option is required!");
  OS << "\t.linker_option \"" << Options[0] << '"';
  for (ArrayRef<std::string>::iterator it = Options.begin() + 1,
         ie = Options.end(); it != ie; ++it) {
    OS << ", " << '"' << *it << '"';
  }
  OS << "\n";
}

void LPUAsmStreamer::EmitDataRegion(MCDataRegionType Kind) {
  if (!MAI->doesSupportDataRegionDirectives())
    return;
  switch (Kind) {
  case MCDR_DataRegion:            OS << "\t.data_region"; break;
  case MCDR_DataRegionJT8:         OS << "\t.data_region jt8"; break;
  case MCDR_DataRegionJT16:        OS << "\t.data_region jt16"; break;
  case MCDR_DataRegionJT32:        OS << "\t.data_region jt32"; break;
  case MCDR_DataRegionEnd:         OS << "\t.end_data_region"; break;
  }
  EmitEOL();
}

void LPUAsmStreamer::EmitVersionMin(MCVersionMinType Kind, unsigned Major,
                                   unsigned Minor, unsigned Update) {
  switch (Kind) {
  case MCVM_IOSVersionMin:        OS << "\t.ios_version_min"; break;
  case MCVM_OSXVersionMin:        OS << "\t.macosx_version_min"; break;
  }
  OS << " " << Major << ", " << Minor;
  if (Update)
    OS << ", " << Update;
  EmitEOL();
}

void LPUAsmStreamer::EmitThumbFunc(MCSymbol *Func) {
  // This needs to emit to a temporary string to get properly quoted
  // MCSymbols when they have spaces in them.
  OS << "\t.thumb_func";
  // Only Mach-O hasSubsectionsViaSymbols()
  if (MAI->hasSubsectionsViaSymbols())
    OS << '\t' << *Func;
  EmitEOL();
}

void LPUAsmStreamer::EmitAssignment(MCSymbol *Symbol, const MCExpr *Value) {
  OS << *Symbol << " = " << *Value;
  EmitEOL();

  MCStreamer::EmitAssignment(Symbol, Value);
}

void LPUAsmStreamer::EmitWeakReference(MCSymbol *Alias, const MCSymbol *Symbol) {
  OS << ".weakref " << *Alias << ", " << *Symbol;
  EmitEOL();
}

bool LPUAsmStreamer::EmitSymbolAttribute(MCSymbol *Symbol,
                                        MCSymbolAttr Attribute) {
  switch (Attribute) {
  case MCSA_Invalid: llvm_unreachable("Invalid symbol attribute");
  case MCSA_ELF_TypeFunction:    /// .type _foo, STT_FUNC  # aka @function
  case MCSA_ELF_TypeIndFunction: /// .type _foo, STT_GNU_IFUNC
  case MCSA_ELF_TypeObject:      /// .type _foo, STT_OBJECT  # aka @object
  case MCSA_ELF_TypeTLS:         /// .type _foo, STT_TLS     # aka @tls_object
  case MCSA_ELF_TypeCommon:      /// .type _foo, STT_COMMON  # aka @common
  case MCSA_ELF_TypeNoType:      /// .type _foo, STT_NOTYPE  # aka @notype
  case MCSA_ELF_TypeGnuUniqueObject:  /// .type _foo, @gnu_unique_object
    if (!MAI->hasDotTypeDotSizeDirective())
      return false; // Symbol attribute not supported
    OS << "\t.ascii \"";
    OS << "\t.type\t" << *Symbol << ','
       << ((MAI->getCommentString()[0] != '@') ? '@' : '%');
    switch (Attribute) {
    default: return false;
    case MCSA_ELF_TypeFunction:    OS << "function"; break;
    case MCSA_ELF_TypeIndFunction: OS << "gnu_indirect_function"; break;
    case MCSA_ELF_TypeObject:      OS << "object"; break;
    case MCSA_ELF_TypeTLS:         OS << "tls_object"; break;
    case MCSA_ELF_TypeCommon:      OS << "common"; break;
    case MCSA_ELF_TypeNoType:      OS << "no_type"; break;
    case MCSA_ELF_TypeGnuUniqueObject: OS << "gnu_unique_object"; break;
    }
    OS << "\\n\"\n";
    return true;
  case MCSA_Global: // .globl/.global
    OS << "\t.ascii \"";
    OS << MAI->getGlobalDirective();
    break;
  case MCSA_Hidden:         OS << "\t.hidden\t";          break;
  case MCSA_IndirectSymbol: OS << "\t.indirect_symbol\t"; break;
  case MCSA_Internal:       OS << "\t.internal\t";        break;
  case MCSA_LazyReference:  OS << "\t.lazy_reference\t";  break;
  case MCSA_Local:          OS << "\t.local\t";           break;
  case MCSA_NoDeadStrip:
    if (!MAI->hasNoDeadStrip())
      return false;
    OS << "\t.ascii \"";
    OS << "\t.no_dead_strip\t";
    break;
  case MCSA_SymbolResolver: OS << "\t.symbol_resolver\t"; break;
  case MCSA_PrivateExtern:
    OS << "\t.ascii \"";
    OS << "\t.private_extern\t";
    break;
  case MCSA_Protected:      OS << "\t.protected\t";       break;
  case MCSA_Reference:      OS << "\t.reference\t";       break;
  case MCSA_Weak:           OS << MAI->getWeakDirective(); break;
  case MCSA_WeakDefinition:
    OS << "\t.ascii \"";
    OS << "\t.weak_definition\t";
    break;
      // .weak_reference
  case MCSA_WeakReference:  OS << MAI->getWeakRefDirective(); break;
  case MCSA_WeakDefAutoPrivate: OS << "\t.weak_def_can_be_hidden\t"; break;
  }

  OS << *Symbol;
  OS << "\\n\"\n";

  return true;
}

void LPUAsmStreamer::EmitSymbolDesc(MCSymbol *Symbol, unsigned DescValue) {
  OS << ".desc" << ' ' << *Symbol << ',' << DescValue;
  EmitEOL();
}

void LPUAsmStreamer::BeginCOFFSymbolDef(const MCSymbol *Symbol) {
  OS << "\t.def\t " << *Symbol << ';';
  EmitEOL();
}

void LPUAsmStreamer::EmitCOFFSymbolStorageClass (int StorageClass) {
  OS << "\t.scl\t" << StorageClass << ';';
  EmitEOL();
}

void LPUAsmStreamer::EmitCOFFSymbolType (int Type) {
  OS << "\t.type\t" << Type << ';';
  EmitEOL();
}

void LPUAsmStreamer::EndCOFFSymbolDef() {
  OS << "\t.endef";
  EmitEOL();
}

void LPUAsmStreamer::EmitCOFFSectionIndex(MCSymbol const *Symbol) {
  OS << "\t.secidx\t" << *Symbol;
  EmitEOL();
}

void LPUAsmStreamer::EmitCOFFSecRel32(MCSymbol const *Symbol) {
  OS << "\t.secrel32\t" << *Symbol;
  EmitEOL();
}

void LPUAsmStreamer::emitELFSize(MCSymbolELF *Symbol, const MCExpr *Value) {
  assert(MAI->hasDotTypeDotSizeDirective());
  OS << "\t.size\t" << *Symbol << ", " << *Value << '\n';
}

void LPUAsmStreamer::EmitCommonSymbol(MCSymbol *Symbol, uint64_t Size,
                                     unsigned ByteAlignment) {
  // Common symbols do not belong to any actual section.
  AssignSection(Symbol, nullptr);

  OS << "\t.comm\t" << *Symbol << ',' << Size;
  if (ByteAlignment != 0) {
    if (MAI->getCOMMDirectiveAlignmentIsInBytes())
      OS << ',' << ByteAlignment;
    else
      OS << ',' << Log2_32(ByteAlignment);
  }
  EmitEOL();
}

/// EmitLocalCommonSymbol - Emit a local common (.lcomm) symbol.
///
/// @param Symbol - The common symbol to emit.
/// @param Size - The size of the common symbol.
void LPUAsmStreamer::EmitLocalCommonSymbol(MCSymbol *Symbol, uint64_t Size,
                                          unsigned ByteAlign) {
  // Common symbols do not belong to any actual section.
  AssignSection(Symbol, nullptr);

  OS << "\t.lcomm\t" << *Symbol << ',' << Size;
  if (ByteAlign > 1) {
    switch (MAI->getLCOMMDirectiveAlignmentType()) {
    case LCOMM::NoAlignment:
      llvm_unreachable("alignment not supported on .lcomm!");
    case LCOMM::ByteAlignment:
      OS << ',' << ByteAlign;
      break;
    case LCOMM::Log2Alignment:
      assert(isPowerOf2_32(ByteAlign) && "alignment must be a power of 2");
      OS << ',' << Log2_32(ByteAlign);
      break;
    }
  }
  EmitEOL();
}

void LPUAsmStreamer::EmitZerofill(MCSection *Section, MCSymbol *Symbol,
                                 uint64_t Size, unsigned ByteAlignment) {
  if (Symbol)
    AssignSection(Symbol, Section);

  // Note: a .zerofill directive does not switch sections.
  OS << ".zerofill ";

  // This is a mach-o specific directive.
  const MCSectionMachO *MOSection = ((const MCSectionMachO*)Section);
  OS << MOSection->getSegmentName() << "," << MOSection->getSectionName();

  if (Symbol) {
    OS << ',' << *Symbol << ',' << Size;
    if (ByteAlignment != 0)
      OS << ',' << Log2_32(ByteAlignment);
  }
  EmitEOL();
}

// .tbss sym, size, align
// This depends that the symbol has already been mangled from the original,
// e.g. _a.
void LPUAsmStreamer::EmitTBSSSymbol(MCSection *Section, MCSymbol *Symbol,
                                   uint64_t Size, unsigned ByteAlignment) {
  AssignSection(Symbol, Section);

  assert(Symbol && "Symbol shouldn't be NULL!");
  // Instead of using the Section we'll just use the shortcut.
  // This is a mach-o specific directive and section.
  OS << ".tbss " << *Symbol << ", " << Size;

  // Output align if we have it.  We default to 1 so don't bother printing
  // that.
  if (ByteAlignment > 1) OS << ", " << Log2_32(ByteAlignment);

  EmitEOL();
}

static inline char toOctal(int X) { return (X&7)+'0'; }

static void PrintQuotedString(StringRef Data, raw_ostream &OS) {
  OS << '"';

  for (unsigned i = 0, e = Data.size(); i != e; ++i) {
    unsigned char C = Data[i];
    if (C == '"' || C == '\\') {
      OS << '\\' << (char)C;
      continue;
    }

    if (isprint((unsigned char)C)) {
      OS << (char)C;
      continue;
    }

    switch (C) {
      case '\b': OS << "\\b"; break;
      case '\f': OS << "\\f"; break;
      case '\n': OS << "\\n"; break;
      case '\r': OS << "\\r"; break;
      case '\t': OS << "\\t"; break;
      default:
        OS << '\\';
        OS << toOctal(C >> 6);
        OS << toOctal(C >> 3);
        OS << toOctal(C >> 0);
        break;
    }
  }

  OS << '"';
}

void LPUAsmStreamer::EmitBytes(StringRef Data) {
  assert(getCurrentSection().first &&
         "Cannot emit contents before setting section!");
  if (Data.empty()) return;

  if (Data.size() == 1) {
    OS << MAI->getData8bitsDirective();
    OS << (unsigned)(unsigned char)Data[0];
    EmitEOL();
    return;
  }

  // If the data ends with 0 and the target supports .asciz, use it, otherwise
  // use .ascii
  if (MAI->getAscizDirective() && Data.back() == 0) {
    OS << MAI->getAscizDirective();
    Data = Data.substr(0, Data.size()-1);
  } else {
    OS << MAI->getAsciiDirective();
  }

  PrintQuotedString(Data, OS);
  EmitEOL();
}

void LPUAsmStreamer::EmitIntValue(uint64_t Value, unsigned Size) {
  EmitValue(MCConstantExpr::create(Value, getContext()), Size);
}

void LPUAsmStreamer::EmitValueImpl(const MCExpr *Value, unsigned Size,
                                  const SMLoc &Loc) {
  assert(Size <= 8 && "Invalid size");
  assert(getCurrentSection().first &&
         "Cannot emit contents before setting section!");
  const char *Directive = nullptr;
  switch (Size) {
  default: break;
  case 1: Directive = MAI->getData8bitsDirective();  break;
  case 2: Directive = MAI->getData16bitsDirective(); break;
  case 4: Directive = MAI->getData32bitsDirective(); break;
  case 8: Directive = MAI->getData64bitsDirective(); break;
  }

  if (!Directive) {
    int64_t IntValue;
    if (!Value->evaluateAsAbsolute(IntValue))
      report_fatal_error("Don't know how to emit this value.");

    // We couldn't handle the requested integer size so we fallback by breaking
    // the request down into several, smaller, integers.  Since sizes greater
    // than eight are invalid and size equivalent to eight should have been
    // handled earlier, we use four bytes as our largest piece of granularity.
    bool IsLittleEndian = MAI->isLittleEndian();
    for (unsigned Emitted = 0; Emitted != Size;) {
      unsigned Remaining = Size - Emitted;
      // The size of our partial emission must be a power of two less than
      // eight.
      unsigned EmissionSize = PowerOf2Floor(Remaining);
      if (EmissionSize > 4)
        EmissionSize = 4;
      // Calculate the byte offset of our partial emission taking into account
      // the endianness of the target.
      unsigned ByteOffset =
          IsLittleEndian ? Emitted : (Remaining - EmissionSize);
      uint64_t ValueToEmit = IntValue >> (ByteOffset * 8);
      // We truncate our partial emission to fit within the bounds of the
      // emission domain.  This produces nicer output and silences potential
      // truncation warnings when round tripping through another assembler.
      uint64_t Shift = 64 - EmissionSize * 8;
      assert(Shift < static_cast<uint64_t>(
                         std::numeric_limits<unsigned long long>::digits) &&
             "undefined behavior");
      ValueToEmit &= ~0ULL >> Shift;
      EmitIntValue(ValueToEmit, EmissionSize);
      Emitted += EmissionSize;
    }
    return;
  }

  assert(Directive && "Invalid size for machine code value!");
  OS << Directive << *Value;
  EmitEOL();
}

void LPUAsmStreamer::EmitULEB128Value(const MCExpr *Value) {
  int64_t IntValue;
  if (Value->evaluateAsAbsolute(IntValue)) {
    EmitULEB128IntValue(IntValue);
    return;
  }
  OS << ".uleb128 " << *Value;
  EmitEOL();
}

void LPUAsmStreamer::EmitSLEB128Value(const MCExpr *Value) {
  int64_t IntValue;
  if (Value->evaluateAsAbsolute(IntValue)) {
    EmitSLEB128IntValue(IntValue);
    return;
  }
  OS << ".sleb128 " << *Value;
  EmitEOL();
}

void LPUAsmStreamer::EmitGPRel64Value(const MCExpr *Value) {
  assert(MAI->getGPRel64Directive() != nullptr);
  OS << MAI->getGPRel64Directive() << *Value;
  EmitEOL();
}

void LPUAsmStreamer::EmitGPRel32Value(const MCExpr *Value) {
  assert(MAI->getGPRel32Directive() != nullptr);
  OS << MAI->getGPRel32Directive() << *Value;
  EmitEOL();
}


/// EmitFill - Emit NumBytes bytes worth of the value specified by
/// FillValue.  This implements directives such as '.space'.
void LPUAsmStreamer::EmitFill(uint64_t NumBytes, uint8_t FillValue) {
  if (NumBytes == 0) return;

  if (const char *ZeroDirective = MAI->getZeroDirective()) {
    OS << ZeroDirective << NumBytes;
    if (FillValue != 0)
      OS << ',' << (int)FillValue;
    EmitEOL();
    return;
  }

  // Emit a byte at a time.
  MCStreamer::EmitFill(NumBytes, FillValue);
}

void LPUAsmStreamer::EmitValueToAlignment(unsigned ByteAlignment, int64_t Value,
                                         unsigned ValueSize,
                                         unsigned MaxBytesToEmit) {
  // Some assemblers don't support non-power of two alignments, so we always
  // emit alignments as a power of two if possible.
  if (isPowerOf2_32(ByteAlignment)) {
    switch (ValueSize) {
    default:
      llvm_unreachable("Invalid size for machine code value!");
    case 1:
      OS << "\t.align\t";
      break;
    case 2:
      OS << ".p2alignw ";
      break;
    case 4:
      OS << ".p2alignl ";
      break;
    case 8:
      llvm_unreachable("Unsupported alignment size!");
    }

    if (MAI->getAlignmentIsInBytes())
      OS << ByteAlignment;
    else
      OS << Log2_32(ByteAlignment);

    if (Value || MaxBytesToEmit) {
      OS << ", 0x";
      OS.write_hex(truncateToSize(Value, ValueSize));

      if (MaxBytesToEmit)
        OS << ", " << MaxBytesToEmit;
    }
    EmitEOL();
    return;
  }

  // Non-power of two alignment.  This is not widely supported by assemblers.
  // FIXME: Parameterize this based on MAI.
  switch (ValueSize) {
  default: llvm_unreachable("Invalid size for machine code value!");
  case 1: OS << ".balign";  break;
  case 2: OS << ".balignw"; break;
  case 4: OS << ".balignl"; break;
  case 8: llvm_unreachable("Unsupported alignment size!");
  }

  OS << ' ' << ByteAlignment;
  OS << ", " << truncateToSize(Value, ValueSize);
  if (MaxBytesToEmit)
    OS << ", " << MaxBytesToEmit;
  EmitEOL();
}

void LPUAsmStreamer::EmitCodeAlignment(unsigned ByteAlignment,
                                      unsigned MaxBytesToEmit) {
  // Emit with a text fill value.
  EmitValueToAlignment(ByteAlignment, MAI->getTextAlignFillValue(),
                       1, MaxBytesToEmit);
}

bool LPUAsmStreamer::EmitValueToOffset(const MCExpr *Offset,
                                      unsigned char Value) {
  // FIXME: Verify that Offset is associated with the current section.
  OS << ".org " << *Offset << ", " << (unsigned) Value;
  EmitEOL();
  return false;
}


void LPUAsmStreamer::EmitFileDirective(StringRef Filename) {
  assert(MAI->hasSingleParameterDotFile());
  OS << "\t.file\t";
  PrintQuotedString(Filename, OS);
  EmitEOL();
}

unsigned LPUAsmStreamer::EmitDwarfFileDirective(unsigned FileNo,
                                               StringRef Directory,
                                               StringRef Filename,
                                               unsigned CUID) {
  assert(CUID == 0);

  MCDwarfLineTable &Table = getContext().getMCDwarfLineTable(CUID);
  unsigned NumFiles = Table.getMCDwarfFiles().size();
  FileNo = Table.getFile(Directory, Filename, FileNo);
  if (FileNo == 0)
    return 0;
  if (NumFiles == Table.getMCDwarfFiles().size())
    return FileNo;

  SmallString<128> FullPathName;

  if (!UseDwarfDirectory && !Directory.empty()) {
    if (sys::path::is_absolute(Filename))
      Directory = "";
    else {
      FullPathName = Directory;
      sys::path::append(FullPathName, Filename);
      Directory = "";
      Filename = FullPathName;
    }
  }

  OS << "\t.file\t" << FileNo << ' ';
  if (!Directory.empty()) {
    PrintQuotedString(Directory, OS);
    OS << ' ';
  }
  PrintQuotedString(Filename, OS);
  EmitEOL();

  return FileNo;
}

void LPUAsmStreamer::EmitDwarfLocDirective(unsigned FileNo, unsigned Line,
                                          unsigned Column, unsigned Flags,
                                          unsigned Isa,
                                          unsigned Discriminator,
                                          StringRef FileName) {
  OS << "\t.loc\t" << FileNo << " " << Line << " " << Column;
  if (Flags & DWARF2_FLAG_BASIC_BLOCK)
    OS << " basic_block";
  if (Flags & DWARF2_FLAG_PROLOGUE_END)
    OS << " prologue_end";
  if (Flags & DWARF2_FLAG_EPILOGUE_BEGIN)
    OS << " epilogue_begin";

  unsigned OldFlags = getContext().getCurrentDwarfLoc().getFlags();
  if ((Flags & DWARF2_FLAG_IS_STMT) != (OldFlags & DWARF2_FLAG_IS_STMT)) {
    OS << " is_stmt ";

    if (Flags & DWARF2_FLAG_IS_STMT)
      OS << "1";
    else
      OS << "0";
  }

  if (Isa)
    OS << " isa " << Isa;
  if (Discriminator)
    OS << " discriminator " << Discriminator;

  if (IsVerboseAsm) {
    OS.PadToColumn(MAI->getCommentColumn());
    OS << MAI->getCommentString() << ' ' << FileName << ':'
       << Line << ':' << Column;
  }
  EmitEOL();
  this->MCStreamer::EmitDwarfLocDirective(FileNo, Line, Column, Flags,
                                          Isa, Discriminator, FileName);
}

MCSymbol *LPUAsmStreamer::getDwarfLineTableSymbol(unsigned CUID) {
  // Always use the zeroth line table, since asm syntax only supports one line
  // table for now.
  return MCStreamer::getDwarfLineTableSymbol(0);
}

void LPUAsmStreamer::EmitIdent(StringRef IdentString) {
  // This is one of the last things emitted for the module, so
  // terminate our strings
  OS << "\t.asciz \"\\n\"\n\n";
  assert(MAI->hasIdentDirective() && ".ident directive not supported");
  OS << "\t.ident\t";
  PrintQuotedString(IdentString, OS);
  EmitEOL();
}

void LPUAsmStreamer::EmitCFISections(bool EH, bool Debug) {
  MCStreamer::EmitCFISections(EH, Debug);
  OS << "\t.cfi_sections ";
  if (EH) {
    OS << ".eh_frame";
    if (Debug)
      OS << ", .debug_frame";
  } else if (Debug) {
    OS << ".debug_frame";
  }

  EmitEOL();
}

void LPUAsmStreamer::EmitCFIStartProcImpl(MCDwarfFrameInfo &Frame) {
  OS << "\t.cfi_startproc";
  if (Frame.IsSimple)
    OS << " simple";
  EmitEOL();
}

void LPUAsmStreamer::EmitCFIEndProcImpl(MCDwarfFrameInfo &Frame) {
  MCStreamer::EmitCFIEndProcImpl(Frame);
  OS << "\t.cfi_endproc";
  EmitEOL();
}

void LPUAsmStreamer::EmitRegisterName(int64_t Register) {
  if (InstPrinter && !MAI->useDwarfRegNumForCFI()) {
    const MCRegisterInfo *MRI = getContext().getRegisterInfo();
    unsigned LLVMRegister = MRI->getLLVMRegNum(Register, true);
    InstPrinter->printRegName(OS, LLVMRegister);
  } else {
    OS << Register;
  }
}

void LPUAsmStreamer::EmitCFIDefCfa(int64_t Register, int64_t Offset) {
  MCStreamer::EmitCFIDefCfa(Register, Offset);
  OS << "\t.cfi_def_cfa ";
  EmitRegisterName(Register);
  OS << ", " << Offset;
  EmitEOL();
}

void LPUAsmStreamer::EmitCFIDefCfaOffset(int64_t Offset) {
  MCStreamer::EmitCFIDefCfaOffset(Offset);
  OS << "\t.cfi_def_cfa_offset " << Offset;
  EmitEOL();
}

void LPUAsmStreamer::EmitCFIDefCfaRegister(int64_t Register) {
  MCStreamer::EmitCFIDefCfaRegister(Register);
  OS << "\t.cfi_def_cfa_register ";
  EmitRegisterName(Register);
  EmitEOL();
}

void LPUAsmStreamer::EmitCFIOffset(int64_t Register, int64_t Offset) {
  this->MCStreamer::EmitCFIOffset(Register, Offset);
  OS << "\t.cfi_offset ";
  EmitRegisterName(Register);
  OS << ", " << Offset;
  EmitEOL();
}

void LPUAsmStreamer::EmitCFIPersonality(const MCSymbol *Sym,
                                       unsigned Encoding) {
  MCStreamer::EmitCFIPersonality(Sym, Encoding);
  OS << "\t.cfi_personality " << Encoding << ", " << *Sym;
  EmitEOL();
}

void LPUAsmStreamer::EmitCFILsda(const MCSymbol *Sym, unsigned Encoding) {
  MCStreamer::EmitCFILsda(Sym, Encoding);
  OS << "\t.cfi_lsda " << Encoding << ", " << *Sym;
  EmitEOL();
}

void LPUAsmStreamer::EmitCFIRememberState() {
  MCStreamer::EmitCFIRememberState();
  OS << "\t.cfi_remember_state";
  EmitEOL();
}

void LPUAsmStreamer::EmitCFIRestoreState() {
  MCStreamer::EmitCFIRestoreState();
  OS << "\t.cfi_restore_state";
  EmitEOL();
}

void LPUAsmStreamer::EmitCFISameValue(int64_t Register) {
  MCStreamer::EmitCFISameValue(Register);
  OS << "\t.cfi_same_value ";
  EmitRegisterName(Register);
  EmitEOL();
}

void LPUAsmStreamer::EmitCFIRelOffset(int64_t Register, int64_t Offset) {
  MCStreamer::EmitCFIRelOffset(Register, Offset);
  OS << "\t.cfi_rel_offset ";
  EmitRegisterName(Register);
  OS << ", " << Offset;
  EmitEOL();
}

void LPUAsmStreamer::EmitCFIAdjustCfaOffset(int64_t Adjustment) {
  MCStreamer::EmitCFIAdjustCfaOffset(Adjustment);
  OS << "\t.cfi_adjust_cfa_offset " << Adjustment;
  EmitEOL();
}

void LPUAsmStreamer::EmitCFISignalFrame() {
  MCStreamer::EmitCFISignalFrame();
  OS << "\t.cfi_signal_frame";
  EmitEOL();
}

void LPUAsmStreamer::EmitCFIUndefined(int64_t Register) {
  MCStreamer::EmitCFIUndefined(Register);
  OS << "\t.cfi_undefined " << Register;
  EmitEOL();
}

void LPUAsmStreamer::EmitCFIRegister(int64_t Register1, int64_t Register2) {
  MCStreamer::EmitCFIRegister(Register1, Register2);
  OS << "\t.cfi_register " << Register1 << ", " << Register2;
  EmitEOL();
}

void LPUAsmStreamer::EmitCFIWindowSave() {
  MCStreamer::EmitCFIWindowSave();
  OS << "\t.cfi_window_save";
  EmitEOL();
}

void LPUAsmStreamer::EmitWinCFIStartProc(const MCSymbol *Symbol) {
  MCStreamer::EmitWinCFIStartProc(Symbol);

  OS << ".seh_proc " << *Symbol;
  EmitEOL();
}

void LPUAsmStreamer::EmitWinCFIEndProc() {
  MCStreamer::EmitWinCFIEndProc();

  OS << "\t.seh_endproc";
  EmitEOL();
}

void LPUAsmStreamer::EmitWinCFIStartChained() {
  MCStreamer::EmitWinCFIStartChained();

  OS << "\t.seh_startchained";
  EmitEOL();
}

void LPUAsmStreamer::EmitWinCFIEndChained() {
  MCStreamer::EmitWinCFIEndChained();

  OS << "\t.seh_endchained";
  EmitEOL();
}

void LPUAsmStreamer::EmitWinEHHandler(const MCSymbol *Sym, bool Unwind,
                                      bool Except) {
  MCStreamer::EmitWinEHHandler(Sym, Unwind, Except);

  OS << "\t.seh_handler " << *Sym;
  if (Unwind)
    OS << ", @unwind";
  if (Except)
    OS << ", @except";
  EmitEOL();
}

void LPUAsmStreamer::EmitWinEHHandlerData() {
  MCStreamer::EmitWinEHHandlerData();

  // Switch sections. Don't call SwitchSection directly, because that will
  // cause the section switch to be visible in the emitted assembly.
  // We only do this so the section switch that terminates the handler
  // data block is visible.
  WinEH::FrameInfo *CurFrame = getCurrentWinFrameInfo();
  if (MCSection *XData = WinEH::UnwindEmitter::getXDataSection(
          CurFrame->Function, getContext()))
    SwitchSectionNoChange(XData);

  OS << "\t.seh_handlerdata";
  EmitEOL();
}

void LPUAsmStreamer::EmitWinCFIPushReg(unsigned Register) {
  MCStreamer::EmitWinCFIPushReg(Register);

  OS << "\t.seh_pushreg " << Register;
  EmitEOL();
}

void LPUAsmStreamer::EmitWinCFISetFrame(unsigned Register, unsigned Offset) {
  MCStreamer::EmitWinCFISetFrame(Register, Offset);

  OS << "\t.seh_setframe " << Register << ", " << Offset;
  EmitEOL();
}

void LPUAsmStreamer::EmitWinCFIAllocStack(unsigned Size) {
  MCStreamer::EmitWinCFIAllocStack(Size);

  OS << "\t.seh_stackalloc " << Size;
  EmitEOL();
}

void LPUAsmStreamer::EmitWinCFISaveReg(unsigned Register, unsigned Offset) {
  MCStreamer::EmitWinCFISaveReg(Register, Offset);

  OS << "\t.seh_savereg " << Register << ", " << Offset;
  EmitEOL();
}

void LPUAsmStreamer::EmitWinCFISaveXMM(unsigned Register, unsigned Offset) {
  MCStreamer::EmitWinCFISaveXMM(Register, Offset);

  OS << "\t.seh_savexmm " << Register << ", " << Offset;
  EmitEOL();
}

void LPUAsmStreamer::EmitWinCFIPushFrame(bool Code) {
  MCStreamer::EmitWinCFIPushFrame(Code);

  OS << "\t.seh_pushframe";
  if (Code)
    OS << " @code";
  EmitEOL();
}

void LPUAsmStreamer::EmitWinCFIEndProlog(void) {
  MCStreamer::EmitWinCFIEndProlog();

  OS << "\t.seh_endprologue";
  EmitEOL();
}

void LPUAsmStreamer::AddEncodingComment(const MCInst &Inst,
                                       const MCSubtargetInfo &STI) {
  raw_ostream &OS = GetCommentOS();
  SmallString<256> Code;
  SmallVector<MCFixup, 4> Fixups;
  raw_svector_ostream VecOS(Code);
  Emitter->encodeInstruction(Inst, VecOS, Fixups, STI);
  VecOS.flush();

  // If we are showing fixups, create symbolic markers in the encoded
  // representation. We do this by making a per-bit map to the fixup item index,
  // then trying to display it as nicely as possible.
  SmallVector<uint8_t, 64> FixupMap;
  FixupMap.resize(Code.size() * 8);
  for (unsigned i = 0, e = Code.size() * 8; i != e; ++i)
    FixupMap[i] = 0;

  for (unsigned i = 0, e = Fixups.size(); i != e; ++i) {
    MCFixup &F = Fixups[i];
    const MCFixupKindInfo &Info = AsmBackend->getFixupKindInfo(F.getKind());
    for (unsigned j = 0; j != Info.TargetSize; ++j) {
      unsigned Index = F.getOffset() * 8 + Info.TargetOffset + j;
      assert(Index < Code.size() * 8 && "Invalid offset in fixup!");
      FixupMap[Index] = 1 + i;
    }
  }

  // FIXME: Note the fixup comments for Thumb2 are completely bogus since the
  // high order halfword of a 32-bit Thumb2 instruction is emitted first.
  OS << "encoding: [";
  for (unsigned i = 0, e = Code.size(); i != e; ++i) {
    if (i)
      OS << ',';

    // See if all bits are the same map entry.
    uint8_t MapEntry = FixupMap[i * 8 + 0];
    for (unsigned j = 1; j != 8; ++j) {
      if (FixupMap[i * 8 + j] == MapEntry)
        continue;

      MapEntry = uint8_t(~0U);
      break;
    }

    if (MapEntry != uint8_t(~0U)) {
      if (MapEntry == 0) {
        OS << format("0x%02x", uint8_t(Code[i]));
      } else {
        if (Code[i]) {
          // FIXME: Some of the 8 bits require fix up.
          OS << format("0x%02x", uint8_t(Code[i])) << '\''
             << char('A' + MapEntry - 1) << '\'';
        } else
          OS << char('A' + MapEntry - 1);
      }
    } else {
      // Otherwise, write out in binary.
      OS << "0b";
      for (unsigned j = 8; j--;) {
        unsigned Bit = (Code[i] >> j) & 1;

        unsigned FixupBit;
        if (MAI->isLittleEndian())
          FixupBit = i * 8 + j;
        else
          FixupBit = i * 8 + (7-j);

        if (uint8_t MapEntry = FixupMap[FixupBit]) {
          assert(Bit == 0 && "Encoder wrote into fixed up bit!");
          OS << char('A' + MapEntry - 1);
        } else
          OS << Bit;
      }
    }
  }
  OS << "]\n";

  for (unsigned i = 0, e = Fixups.size(); i != e; ++i) {
    MCFixup &F = Fixups[i];
    const MCFixupKindInfo &Info = AsmBackend->getFixupKindInfo(F.getKind());
    OS << "  fixup " << char('A' + i) << " - " << "offset: " << F.getOffset()
       << ", value: " << *F.getValue() << ", kind: " << Info.Name << "\n";
  }
}

void LPUAsmStreamer::EmitInstruction(const MCInst &Inst, const MCSubtargetInfo &STI) {
  assert(getCurrentSection().first &&
         "Cannot emit contents before setting section!");

  // Show the encoding in a comment if we have a code emitter.
  if (Emitter)
    AddEncodingComment(Inst, STI);

  // Show the MCInst if enabled.
  if (ShowInst) {
    Inst.dump_pretty(GetCommentOS(), InstPrinter.get(), "\n ");
    GetCommentOS() << "\n";
  }

  // If we have an AsmPrinter, use that to print, otherwise print the MCInst.
  OS << "\t.ascii \"";
  if (InstPrinter)
    InstPrinter->printInst(&Inst, OS, "", STI);
  else
    Inst.print(OS);
  OS << "\\n\"\n";
}

void LPUAsmStreamer::EmitBundleAlignMode(unsigned AlignPow2) {
  OS << "\t.bundle_align_mode " << AlignPow2;
  EmitEOL();
}

void LPUAsmStreamer::EmitBundleLock(bool AlignToEnd) {
  OS << "\t.bundle_lock";
  if (AlignToEnd)
    OS << " align_to_end";
  EmitEOL();
}

void LPUAsmStreamer::EmitBundleUnlock() {
  OS << "\t.bundle_unlock";
  EmitEOL();
}

/// EmitRawText - If this file is backed by an assembly streamer, this dumps
/// the specified string in the output .s file.  This capability is
/// indicated by the hasRawTextSupport() predicate.
void LPUAsmStreamer::EmitRawTextImpl(StringRef String) {
  OS << "\t.ascii ";
  PrintQuotedString(String, OS);
  EmitEOL();
  OS << "\t.ascii \"\\n\"\n";
}

void LPUAsmStreamer::FinishImpl() {
  // If we are generating dwarf for assembly source files dump out the sections.
  if (getContext().getGenDwarfForAssembly())
    MCGenDwarfInfo::Emit(this);

  // Emit the label for the line table, if requested - since the rest of the
  // line table will be defined by .loc/.file directives, and not emitted
  // directly, the label is the only work required here.
  auto &Tables = getContext().getMCDwarfLineTables();
  if (!Tables.empty()) {
    assert(Tables.size() == 1 && "asm output only supports one line table");
    if (auto *Label = Tables.begin()->second.getLabel()) {
      SwitchSection(getContext().getObjectFileInfo()->getDwarfLineSection());
      EmitLabel(Label);
    }
  }
}

namespace llvm {

static cl::opt<bool>
WrapAssembly("lpu-wrap-assembly",
             cl::desc("LPU Specific: wrap LPU assembly statements in ascii strings which can be compiled into an object"),
             cl::init(false), cl::ReallyHidden);

MCStreamer *createLpuAsmStreamer(MCContext &Context,
                                 std::unique_ptr<formatted_raw_ostream> OS,
                                 bool isVerboseAsm,
                                 bool useDwarfDirectory,
                                 MCInstPrinter *IP,
                                 MCCodeEmitter *CE,
                                 MCAsmBackend *MAB,
                                 bool ShowInst) {

  // Are we using our streamer or the standard one? Our streamer will
  // wrap all of the LPU assembly statements as .ascii strings.
  //
  // Note that we cannot make this determination in LLVMInitializeLPUTargetInfo()
  // because that's called before the options have been parsed.
  if (WrapAssembly) {

    // Use LpuAsmStreamer - our hacked version of MCAsmStreamer which
    // will put all of the LPU assembly statements in .ascii statements
    // so the compiler can generate an object file.
    return new LPUAsmStreamer(Context, std::move(OS), isVerboseAsm,
                              useDwarfDirectory, IP, CE, MAB, ShowInst);
  } else {

    // Use MCAsmStream to generat a raw LPU assembly file
    return llvm::createAsmStreamer(Context, std::move(OS), isVerboseAsm, useDwarfDirectory,
                                   IP, CE, MAB, ShowInst);
  }
}

} // namespace llvm
