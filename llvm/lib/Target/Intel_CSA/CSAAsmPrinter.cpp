//===-- CSAAsmPrinter.cpp - CSA LLVM assembly writer ----------------------===//
//
// Copyright (C) 2017-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file contains a printer that converts from our internal representation
// of machine-dependent LLVM code to the CSA assembly language.
//
//===----------------------------------------------------------------------===//

#include "CSA.h"
#include "CSAAsmWrapOstream.h"
#include "CSAInstrInfo.h"
#include "CSAMCInstLower.h"
#include "CSATargetMachine.h"
#include "CSAUtils.h"
#include "InstPrinter/CSAInstPrinter.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Mangler.h"
#include "llvm/IR/Module.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCSectionELF.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetLoweringObjectFile.h"
#include <fstream>
#include <sstream>
#include <string>
#include "llvm/ADT/SmallSet.h"

using namespace llvm;

#define DEBUG_TYPE "asm-printer"

static cl::opt<bool>
  EmitLineNumbers("csa-emit-line-numbers", cl::Hidden,
                  cl::desc("CSA Specific: Emit Line numbers even without -G"),
                  cl::init(true));

static cl::opt<bool>
  InterleaveSrc("csa-emit-src", cl::ZeroOrMore, cl::Hidden,
                cl::desc("CSA Specific: Emit source line in asm file"),
                cl::init(false));

static cl::opt<bool>
  StrictTermination("csa-strict-term", cl::Hidden,
                    cl::desc("CSA Specific: Turn on strict termination mode"),
                    cl::init(false));

static cl::opt<bool>
  ImplicitLicDefs("csa-implicit-lics", cl::Hidden,
                  cl::desc("CSA Specific: Define LICs implicitly"),
                  cl::init(false));

static cl::opt<bool>
  EmitRegNames("csa-print-lic-names", cl::Hidden,
               cl::desc("CSA Specific: Print pretty names for LICs"),
               cl::init(false));

static cl::opt<bool>
  EmitExperimental("csa-experimental-annotations", cl::Hidden,
      cl::desc("CSA Specific: Print experimental late tools annotations"),
      cl::init(true));

namespace {
class LineReader {
private:
  unsigned theCurLine;
  std::ifstream fstr;
  char buff[512];
  std::string theFileName;
  SmallVector<unsigned, 32> lineOffset;

public:
  LineReader(std::string filename) {
    theCurLine = 0;
    fstr.open(filename.c_str());
    theFileName = filename;
  }
  std::string fileName() { return theFileName; }
  ~LineReader() { fstr.close(); }
  std::string readLine(unsigned lineNum) {
    if (lineNum < theCurLine) {
      theCurLine = 0;
      fstr.seekg(0, std::ios::beg);
    }
    while (theCurLine < lineNum) {
      fstr.getline(buff, 500);
      theCurLine++;
    }
    return buff;
  }
};

class CSAAsmPrinter : public AsmPrinter {
  const Function *F;
  const MachineRegisterInfo *MRI;
  const CSAMachineFunctionInfo *LMFI;
  DebugLoc prevDebugLoc;
  bool ignoreLoc(const MachineInstr &);
  LineReader *reader;
  bool isFirstFunc; // flag to specify if first function is being compiled
  bool doInitialization(Module &M) override;
  bool doFinalization(Module &M) override;
  void emitLineNumberAsDotLoc(const MachineInstr &);
  void emitSrcInText(StringRef filename, unsigned line);
  LineReader *getReader(std::string);
  void emitParamList(const Function *);
  void emitReturnVal(const Function *);
  void emitParamList();
  void emitReturnVal();
  void setLICNames();
  void EmitCallInstruction(const MachineInstr *);
  void EmitContinueInstruction(const MachineInstr *);
  void EmitAll0(const MachineInstr *);
  void EmitSimpleEntryInstruction(MachineFunction *MF);
  void EmitParamsResultsDecl(MachineInstr *, MachineInstr *);
  void EmitTrampolineMarkers(const MachineInstr *);
  void EmitCSAOperands(const MachineInstr *, raw_ostream &, int, int);
  unsigned resultReg;
  void writeSmallFountain(const MachineInstr *MI);

  void EmitLicGroup(CSALicGroup &group);

public:
  CSAAsmPrinter(TargetMachine &TM, std::unique_ptr<MCStreamer> Streamer)
      : AsmPrinter(TM, std::move(Streamer)), reader() {
        resultReg = 0;
        isFirstFunc = true;
      }

  StringRef getPassName() const override { return "CSA: Assembly Printer"; }

  void printOperand(const MachineInstr *MI, int OpNum, raw_ostream &O);
  bool PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
                       const char *ExtraCode, raw_ostream &O) override;

  void EmitStartOfAsmFile(Module &) override;
  void EmitEndOfAsmFile(Module &) override;

  bool runOnMachineFunction(MachineFunction &F) override;
  void EmitFunctionEntryLabel() override;
  void EmitFunctionBodyStart() override;
  void EmitFunctionBodyEnd() override;
  void EmitInstruction(const MachineInstr *MI) override;
  void EmitConstantPool() override;
  void EmitGlobalVariable(const GlobalVariable *GV) override;

  void EmitCsaCodeSection();
  void EmitScratchpad(MCSymbol *Symbol, bool isConstant, const Constant *Init);
};
} // end of anonymous namespace

bool CSAAsmPrinter::runOnMachineFunction(MachineFunction &MF) {
  CSAMachineFunctionInfo *LMFI = MF.getInfo<CSAMachineFunctionInfo>();
  if (LMFI && LMFI->getDoNotEmitAsm() && csa_utils::isAlwaysDataFlowLinkageSet())
    return false;
  SmallString<128> Str;
  raw_svector_ostream O(Str);
  if (csa_utils::createSCG()) {
    O << ".module __mod_" << MF.getName() << "\n";
    O << "\t.version 0,6,0\n";
    // This should probably be replaced by code to handle externs
    O << "\t.set implicitextern\n";
    if (not StrictTermination)
      O << "\t.set relaxed\n";
    if (ImplicitLicDefs)
      O << "\t.set implicit\n";
    if (csa_utils::isAlwaysDataFlowLinkageSet())
      O << "\t.unit\n";
    else
      O << "\t.unit sxu\n";
    OutStreamer->EmitRawText(O.str());

    // If we have any scratchpads, emit them before the machine function.
    const Module &M = *MF.getFunction().getParent();
    for (const auto &GV: M.globals()) {
      if (isScratchpadAddressSpace(GV.getAddressSpace())) {
        EmitScratchpad(getSymbol(&GV), GV.isConstant(), GV.getInitializer());
      }
    }
  }
  AsmPrinter::runOnMachineFunction(MF);

  if (csa_utils::createSCG()) {
    OutStreamer->EmitRawText(".endmodule");
    isFirstFunc = false;
  }
  return false;
}

void CSAAsmPrinter::printOperand(const MachineInstr *MI, int OpNum,
                                 raw_ostream &O) {
  const MachineOperand &MO = MI->getOperand(OpNum);

  switch (MO.getType()) {
  case MachineOperand::MO_Register:
    O << "%" << CSAInstPrinter::getRegisterName(MO.getReg());
    break;

  case MachineOperand::MO_Immediate:
    O << MO.getImm();
    break;

  case MachineOperand::MO_MachineBasicBlock:
    O << *MO.getMBB()->getSymbol();
    break;

  case MachineOperand::MO_GlobalAddress:
    O << *getSymbol(MO.getGlobal());
    break;

  case MachineOperand::MO_BlockAddress: {
    MCSymbol *BA = GetBlockAddressSymbol(MO.getBlockAddress());
    O << BA->getName();
    break;
  }

  case MachineOperand::MO_ExternalSymbol:
    O << *GetExternalSymbolSymbol(MO.getSymbolName());
    break;

  case MachineOperand::MO_JumpTableIndex:
    O << MAI->getPrivateGlobalPrefix() << "JTI" << getFunctionNumber() << '_'
      << MO.getIndex();
    break;

  case MachineOperand::MO_ConstantPoolIndex:
    O << MAI->getPrivateGlobalPrefix() << "CPI" << getFunctionNumber() << '_'
      << MO.getIndex();
    return;

  default:
    llvm_unreachable("<unknown operand type>");
  }
}

// PrintAsmOperand - Print out an operand for an inline asm expression.
bool CSAAsmPrinter::PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
                                    const char *ExtraCode, raw_ostream &O) {
  // Does this asm operand have a single letter operand modifier?
  if (ExtraCode && ExtraCode[0]) {
    if (ExtraCode[1])
      return true; // Unknown modifier.

    switch (ExtraCode[0]) {
    default:
      return true; // Unknown modifier.
    }
  }
  printOperand(MI, OpNo, O);
  return false;
}

bool CSAAsmPrinter::ignoreLoc(const MachineInstr &MI) {
  switch (MI.getOpcode()) {
  default:
    return false;
    // May be desirable to avoid CSA-specific MachineInstrs
  }
}

bool CSAAsmPrinter::doInitialization(Module &M) {
  bool result = AsmPrinter::doInitialization(M);

  // Emit module-level inline asm if it exists.
  if (!M.getModuleInlineAsm().empty()) {
    OutStreamer->AddComment("Start of file scope inline assembly");
    OutStreamer->AddBlankLine();
    OutStreamer->EmitRawText(StringRef(M.getModuleInlineAsm()));
    OutStreamer->AddBlankLine();
    OutStreamer->AddComment("End of file scope inline assembly");
    OutStreamer->AddBlankLine();
  }

  return result;
}

static bool isMathFunc(Function *F, const Module &M) {
  if (F == M.getFunction("cos")) return true;
  if (F == M.getFunction("exp")) return true;
  if (F == M.getFunction("exp2")) return true;
  if (F == M.getFunction("floor")) return true;
  if (F == M.getFunction("log")) return true;
  if (F == M.getFunction("log2")) return true;
  if (F == M.getFunction("log10")) return true;
  if (F == M.getFunction("pow")) return true;
  if (F == M.getFunction("round")) return true;
  if (F == M.getFunction("sin")) return true;
  if (F == M.getFunction("sincos")) return true;
  if (F == M.getFunction("trunc")) return true;
  if (F == M.getFunction("cosf")) return true;
  if (F == M.getFunction("expf")) return true;
  if (F == M.getFunction("exp2f")) return true;
  if (F == M.getFunction("floorf")) return true;
  if (F == M.getFunction("logf")) return true;
  if (F == M.getFunction("log2f")) return true;
  if (F == M.getFunction("log10f")) return true;
  if (F == M.getFunction("powf")) return true;
  if (F == M.getFunction("roundf")) return true;
  if (F == M.getFunction("sinf")) return true;
  if (F == M.getFunction("sincosf")) return true;
  if (F == M.getFunction("truncf")) return true;
  return false;
}

// Mark all globals from surviving math lib functions as external
void markMathLibGlobalsAsExtern(Module &M) {
  for (auto GVI = M.global_begin(), E = M.global_end(); GVI != E; GVI++) {
    GlobalVariable *GV = &*GVI;
    StringRef Name = GV->getName();
    if (Name.startswith("llvm."))
      continue;
    for(Value::use_iterator UI = GVI->use_begin(), UE = GVI->use_end(); UI!=UE; ++UI) {
      Use &U = *UI;
      if (Instruction *I = dyn_cast<Instruction>(U.getUser())) {
        Function *TmpF = I->getParent()->getParent();
        if (isMathFunc(TmpF,M))
          GV->setLinkage(llvm::Function::ExternalLinkage);
      }
    }
  }
}

bool CSAAsmPrinter::doFinalization(Module &M) {
  markMathLibGlobalsAsExtern(M);

  // If we have scratchpads, emit them now.
  if (!csa_utils::createSCG()) {
    for (const auto &GV: M.globals()) {
      if (isScratchpadAddressSpace(GV.getAddressSpace())) {
        EmitScratchpad(getSymbol(&GV), GV.isConstant(), GV.getInitializer());
      }
    }
  }
  if (CSAInstPrinter::WrapCsaAsm()) {
    OutStreamer->AddBlankLine();
    if (!csa_utils::createSCG())
      OutStreamer->EmitRawText(".endmodule\n");
  } else {
    if (!csa_utils::createSCG())
      OutStreamer->EmitRawText(".endmodule\n");
  }
  bool result = AsmPrinter::doFinalization(M);
  return result;
}
// Copied from lib/MC/MCAsmStreamer.cpp
static inline char toOctal(int X) { return (X&7)+'0'; }

// Copied from lib/MC/MCAsmStreamer.cpp
static void PrintQuotedString(StringRef Data, raw_ostream &OS) {
  OS << '"';

  for (unsigned i = 0, e = Data.size(); i != e; ++i) {
    unsigned char C = Data[i];
    if (C == '"' || C == '\\') {
      OS << '\\' << (char)C;
      continue;
    }

    if (isPrint((unsigned char)C)) {
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

// Copied from lib/MC/MCAsmStreamer.cpp
static void printDwarfFileDirective(unsigned FileNo, StringRef Directory,
                                    StringRef Filename,
                                    raw_svector_ostream &OS) {
  SmallString<128> FullPathName;

  if (!Directory.empty()) {
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
}

void CSAAsmPrinter::emitLineNumberAsDotLoc(const MachineInstr &MI) {
  if (!EmitLineNumbers)
    return;
  if (ignoreLoc(MI))
    return;

  DebugLoc curLoc = MI.getDebugLoc();

  if (!prevDebugLoc && !curLoc)
    return;

  if (prevDebugLoc == curLoc)
    return;

  prevDebugLoc = curLoc;

  if (!curLoc)
    return;

  auto *Scope = cast_or_null<DIScope>(curLoc.getScope());
  if (!Scope)
    return;

  StringRef fileName(Scope->getFilename());
  StringRef dirName(Scope->getDirectory());

  // Emit the line from the source file.
  if (InterleaveSrc)
    this->emitSrcInText(fileName.str(), curLoc.getLine());

  std::stringstream temp;

  //
  // EmitDwarfFileDirective() returns the file ID for the given
  // file path.  It will only emit the file directive once
  // for each file.
  //
  unsigned FileNo = OutStreamer->EmitDwarfFileDirective(0, dirName, fileName);

  if (FileNo == 0)
    return;

  temp << "\t.loc " << FileNo << " " << curLoc.getLine()
       << " " << curLoc.getCol();
  OutStreamer->EmitRawText(Twine(temp.str().c_str()));
}

void CSAAsmPrinter::emitSrcInText(StringRef filename, unsigned line) {
  std::stringstream temp;
  LineReader *reader = this->getReader(filename.str());
  temp << "\n#";
  temp << filename.str();
  temp << ":";
  temp << line;
  temp << " ";
  temp << reader->readLine(line);
  temp << "\n";
  this->OutStreamer->EmitRawText(Twine(temp.str()));
}

LineReader *CSAAsmPrinter::getReader(std::string filename) {
  if (!reader) {
    reader = new LineReader(filename);
  }

  if (reader->fileName() != filename) {
    delete reader;
    reader = new LineReader(filename);
  }

  return reader;
}

void CSAAsmPrinter::emitParamList(const Function *F) {
  SmallString<128> Str;
  raw_svector_ostream O(Str);
  const TargetLowering *TLI =
    MF->getSubtarget<CSASubtarget>().getTargetLowering();
  Function::const_arg_iterator I, E;
  MVT thePointerTy = TLI->getPointerTy(MF->getDataLayout());

  // Stride through parameters, putting out a .param {type} .reg %r{num}
  // This is a hack mostly taken from NVPTX.  This assumes successive
  // parameters go to successive registers, starting with the initial
  // value of paramReg.  This may be too simplistic for longer term.
  int paramReg = 2;  // Params start in R2 - see CSACallingConv.td
  int lastReg  = 17; // Params end (inclusive) in R17 - see CSACallingConv.td
  bool first   = true;
  for (I = F->arg_begin(), E = F->arg_end(); I != E && paramReg <= lastReg;
       ++I, paramReg++) {
    Type *Ty            = I->getType();
    unsigned sz         = 0;
    std::string typeStr = ".i";
    if (isa<IntegerType>(Ty)) {
      sz = cast<IntegerType>(Ty)->getBitWidth();
    } else if (Ty->isFloatingPointTy()) {
      sz = Ty->getPrimitiveSizeInBits();
    } else if (isa<PointerType>(Ty)) {
      sz = thePointerTy.getSizeInBits();
    } else {
      sz = Ty->getPrimitiveSizeInBits();
    }
    if (!first) {
      O << '\n';
    }
    O << "\t.param .reg " << typeStr << sz << " %r" << paramReg;
    first = false;
  }
  if (!first)
    OutStreamer->EmitRawText(O.str());
}

void CSAAsmPrinter::emitReturnVal(const Function *F) {
  SmallString<128> Str;
  raw_svector_ostream O(Str);
  const TargetLowering *TLI =
    MF->getSubtarget<CSASubtarget>().getTargetLowering();

  Type *Ty = F->getReturnType();

  if (Ty->getTypeID() == Type::VoidTyID)
    return;

  O << "\t.result .reg";

  if (Ty->isFloatingPointTy() || Ty->isIntegerTy()) {
    unsigned size = 0;
    if (const IntegerType *ITy = dyn_cast<IntegerType>(Ty)) {
      size = ITy->getBitWidth();
      O << " .i" << size;
    } else {
      assert(Ty->isFloatingPointTy() && "Floating point type expected here");
      size = Ty->getPrimitiveSizeInBits();
      O << " .i" << size;
    }

  } else if (isa<PointerType>(Ty)) {
    O << " .i" << TLI->getPointerTy(MF->getDataLayout()).getSizeInBits();
  } else if ((Ty->getTypeID() == Type::StructTyID) || isa<VectorType>(Ty)) {
    llvm_unreachable("NYI: aggregate result");
  } else
    llvm_unreachable("Unknown return type");

  // Hack: For now, we simply go with the standard return register.
  // (Should really use the allocation.)
  O << " %r0";

  OutStreamer->EmitRawText(O.str());
}

void CSAAsmPrinter::EmitCsaCodeSection() {
  // The .section directive for an ELF object as a name and 3 optional,
  // comma separated parts as detailed at
  // https://sourceware.org/binutils/docs/as/Section.html
  //
  // The CSA code section uses the following:
  //
  // Name: ".csa.code". I may want to append the module name.
  //
  // Flag values:
  // - ELF::SHF_ALLOC - Section is allocatable - Which tells us very little. The
  //       ELF docs expand this to explain that SHF_ALLOC means that the section
  //       occupies memory during process execution
  // - ELF::SHF_STRINGS - Section contains zero terminated strings
  //
  // Type: ELF::SHT_PROGBITS - section contains data
  MCSection *const CsaSec = OutContext.getELFSection(
    ".csa.code", ELF::SHT_PROGBITS, ELF::SHF_ALLOC | ELF::SHF_STRINGS);
  OutStreamer->PushSection();
  OutStreamer->SwitchSection(CsaSec);
}

void CSAAsmPrinter::EmitStartOfAsmFile(Module &M) {

  /* Disabled 2016/3/31.  Long term, we should only put this out if it
   * is not autounit.  The theory is if the compiler has done tailoring
   * for a specific target, that should be reflected in the file.
   */
  SmallString<128> Str;
  raw_svector_ostream O(Str);
  if (CSAInstPrinter::WrapCsaAsm()) {
    EmitCsaCodeSection();

    // Emit a symbol for each of the functions pointing to the CSA code
    // block.
    {
      SmallString<128> OutStr;
      raw_svector_ostream OO(OutStr);
      for (const Function &F : M) {
        if (!F.isDeclaration())
          OO << "\t.set " << *getSymbol(&F) << ", .csa.code.start\n";
      }
      OutStreamer->EmitRawText(OO.str());
    }

    // Start the CSA code block.
    OutStreamer->EmitRawText(".csa.code.start:");
    OutStreamer->EmitRawText("\t.ascii ");
    startCSAAsmString(*OutStreamer);
    O << "\t.text\n";
  } 
  if (!csa_utils::createSCG()) {
    O << ".module __mod_top\n";
    O << "\t.version 0,6,0\n";
    // This should probably be replaced by code to handle externs
    O << "\t.set implicitextern\n";
    if (not StrictTermination)
      O << "\t.set relaxed\n";
    if (ImplicitLicDefs)
      O << "\t.set implicit\n";
    if (csa_utils::isAlwaysDataFlowLinkageSet())
      O << "\t.unit\n";
    else
      O << "\t.unit sxu\n";
  }
  OutStreamer->EmitRawText(O.str());

  // If we have any scratchpads, emit them before the machine function.
  if (!csa_utils::createSCG()) {
    for (const auto &GV: M.globals()) {
      if (isScratchpadAddressSpace(GV.getAddressSpace())) {
        EmitScratchpad(getSymbol(&GV), GV.isConstant(), GV.getInitializer());
      }
    }
  }
}

void CSAAsmPrinter::EmitEndOfAsmFile(Module &M) {
  if (CSAInstPrinter::WrapCsaAsm()) {
    OutStreamer->AddBlankLine();
    endCSAAsmString(*OutStreamer);
    OutStreamer->AddBlankLine();
    // Add the terminating null for the .csa section.
    OutStreamer->EmitRawText("\t.asciz \"\"");
    OutStreamer->PopSection();
  }
}

void CSAAsmPrinter::EmitFunctionEntryLabel() {
  SmallString<128> Str;
  raw_svector_ostream O(Str);

  // Set up
  MRI = &MF->getRegInfo();
  F   = &MF->getFunction();

  //
  // CMPLRS-49165: set compilation directory DWARF emission.
  //
  // With -fdwarf-directory-asm (default in ICX) and unset compilation
  // directory EmitDwarfFileDirective will use new syntax for assembly
  // .file directory:
  //     .file 1 "directory" "file"
  //
  // Neither standard 'as' nor CSA simulator can handle this.
  //
  // If we set the compilation directory, and the file being compiled
  // is located in the compilation folder, then the old syntax will be used.
  // At the same time, even if we set the compilation directory,
  // the new syntax will be used in cases, when the file is not
  // in the compilation directory.  So the general fix is to use
  // -fno-dwarf-directory-asm - see CMPLRS-49173.
  //
  // I think setting the compilation directory is the right thing to do
  // anyway.
  //
  auto *SubProgram = MF->getFunction().getSubprogram();
  if (SubProgram &&
      SubProgram->getUnit()->getEmissionKind() != DICompileUnit::NoDebug) {
#if RAVI
    MCDwarfLineTable &Table = OutStreamer->getContext().getMCDwarfLineTable(0);
    Table.setCompilationDir(SubProgram->getUnit()->getDirectory());
#endif
  }

  if (csa_utils::isAlwaysDataFlowLinkageSet()) {
    setLICNames();
    return;
  }
  O << "\t.entry\t" << *CurrentFnSym << "\n";
  // For now, assume control flow (sequential) entry
  O << *CurrentFnSym << ":\n";

  // Start a scope for this routine to localize the LIC names
  // For now, this includes parameters and results
  O << "{";

  OutStreamer->EmitRawText(O.str());

  emitReturnVal(F);

  emitParamList(F);
}

void CSAAsmPrinter::setLICNames(void) {
  MRI                                = &MF->getRegInfo();
  const CSAMachineFunctionInfo *LMFI = MF->getInfo<CSAMachineFunctionInfo>();
  if (not ImplicitLicDefs) {
    for (unsigned index = 0, e = MRI->getNumVirtRegs(); index != e; ++index) {
      unsigned vreg = Register::index2VirtReg(index);
      if (!MRI->reg_empty(vreg)) {
        StringRef name = LMFI->getLICName(vreg);
        if ((!EmitRegNames && !(csa_utils::isAlwaysDataFlowLinkageSet()))
            || name.empty()) {
          LMFI->setLICName(vreg, Twine("cv") + Twine(LMFI->getLICSize(vreg)) +
                                   "_" + Twine(index));
        }
      }
    }
  }
}

void CSAAsmPrinter::EmitLicGroup(CSALicGroup &licGroup) {
  // All lic group annotations are still experimental.
  if (!EmitExperimental)
    return;

  SmallString<128> Str;
  raw_svector_ostream O(Str);
  O << "\t.attrib csasim_frequency=";
  auto freq = licGroup.executionFrequency;
  freq.print(O);
  if (licGroup.LoopId) {
    O << ", csasim_loop_id=" << licGroup.LoopId;
  }
  OutStreamer->EmitRawText(O.str());
}

void CSAAsmPrinter::EmitFunctionBodyStart() {
  MRI  = &MF->getRegInfo();
  LMFI = MF->getInfo<CSAMachineFunctionInfo>();
  if (csa_utils::isAlwaysDataFlowLinkageSet()) {
    // Emit code for all entry points
    for (unsigned i = 0; i < LMFI->getNumCSAEntryPoints(); ++i) {
      const CSAEntryPoint &CSAEP = LMFI->getCSAEntryPoint(i);
      EmitSimpleEntryInstruction(CSAEP.MF);
      EmitParamsResultsDecl(CSAEP.EntryMI,CSAEP.ReturnMI);
    }
  }
  if (not ImplicitLicDefs) {
    auto printRegisterAttribs = [&](unsigned reg) {
      for (StringRef k : LMFI->getLICAttributes(reg)) {
        OutStreamer->EmitRawText("\t.attrib " + k + " " +
                                 LMFI->getLICAttribute(reg, k));
      }
    };

    auto printRegister = [&](unsigned reg, StringRef name) {
      printRegisterAttribs(reg);

      SmallString<128> Str;
      raw_svector_ostream O(Str);

      if (auto group = LMFI->getLICGroup(reg))
        EmitLicGroup(*group);

      O << "\t.lic";
      if (Register::isVirtualRegister(reg)) {
        if (unsigned depth = LMFI->getLICDepth(reg)) {
          O << "@" << depth;
        }
      }
      if (Register::isVirtualRegister(reg))
        O << " .i" << LMFI->getLICSize(reg) << " ";
      else
        O << " .i64 ";
      O << "%" << name;
      OutStreamer->EmitRawText(O.str());
    };

    // Generate declarations for each LIC by looping over the LIC classes,
    // and over each lic in the class, outputting a decl if needed.
    // Note: If we start allowing parameters and results in LICs for
    // HybridDataFlow, this may need to be revisited to make sure they
    // are in order.
    for (TargetRegisterClass::iterator ri = CSA::ANYCRegClass.begin();
         ri != CSA::ANYCRegClass.end(); ++ri) {
      MCPhysReg reg = *ri;
      bool isParam = false;
      Function::const_arg_iterator I, E;
      unsigned Arg;
      for (I = F->arg_begin(), E = F->arg_end(), Arg= CSA::P64_2;
            I != E; ++I, ++Arg) {
        if (Arg == reg) {
          isParam = true;
          break;
        }
      }
      if (isParam || (resultReg == reg)) continue;
      // A decl is needed if we allocated this LIC and it has using/defining
      // instruction. (Sometimes all such instructions are cleaned up by DIE.)
      if (reg != CSA::IGN && reg != CSA::NA && !MRI->reg_empty(reg)) {
        StringRef name = "";
        if (Register::isVirtualRegister(reg))
          name = LMFI->getLICName(reg);
        else
          name = CSAInstPrinter::getRegisterName(reg);
        printRegister(reg, name);
      }
    }
    for (unsigned index = 0, e = MRI->getNumVirtRegs(); index != e; ++index) {
      unsigned vreg = Register::index2VirtReg(index);
      if (!MRI->reg_empty(vreg) && LMFI->getIsDeclared(vreg)) {
        if (csa_utils::isAlwaysDataFlowLinkageSet()) {
          if (LMFI->getNumCallSites() == 0) {
            bool isParamOrResult = false;
            for (auto UI = MRI->use_begin(vreg); UI != MRI->use_end(); ++UI) {
              MachineInstr *MI = UI->getParent();
              if (MI->getOpcode() == CSA::CSA_RETURN) {
                isParamOrResult = true;
                break;
              }
            }
            MachineInstr *DefMI = MRI->getUniqueVRegDef(vreg);
            if (DefMI && DefMI->getOpcode() == CSA::CSA_ENTRY)
                isParamOrResult = true;
            if (isParamOrResult) continue;
          }
        }
        if (!csa_utils::isAlwaysDataFlowLinkageSet())
          assert(!MRI->use_nodbg_empty(vreg) && "LIC without consumers");
        StringRef name = LMFI->getLICName(vreg);
        if ((!EmitRegNames &&
              !(csa_utils::isAlwaysDataFlowLinkageSet())) || name.empty()) {
          LMFI->setLICName(vreg, Twine("cv") + Twine(LMFI->getLICSize(vreg)) +
                                   "_" + Twine(index));
        }
        printRegister(vreg, LMFI->getLICName(vreg));
      }
    }
  }
  // For each CSA module, we forcibly emit the .file directive
  // Each .file directive (one for each source file) is added
  // exactly once at the beginning of the body of the CSA module
  if (csa_utils::isAlwaysDataFlowLinkageSet()) {
    OutStreamer->EmitRawText("{");
    DebugLoc curLoc;
    // Set used to hold FileNos to avoid .file beign emitted twice
    SmallSet<unsigned, 32> FileNos;
    // We have already emitted the .file directives for the first file
    if (!isFirstFunc) {
      for (auto &MBB : *MF) {
        for (auto &MI : MBB) {
          curLoc = MI.getDebugLoc();
          if (!curLoc)
            continue;
          auto *Scope = cast_or_null<DIScope>(curLoc.getScope());
          if (!Scope)
            continue;
          StringRef fileName(Scope->getFilename());
          StringRef dirName(Scope->getDirectory());
          unsigned FileNo = OutStreamer->EmitDwarfFileDirective(0, dirName, fileName);
          if (FileNos.count(FileNo) == 0) {
            SmallString<128> Str;
            raw_svector_ostream O(Str);
            printDwarfFileDirective(FileNo, dirName, fileName, O);
            OutStreamer->EmitRawText(O.str());
            FileNos.insert(FileNo);
          }
        }
      }
    }
  }
}

void CSAAsmPrinter::EmitFunctionBodyEnd() {
  OutStreamer->EmitRawText("}");
}

void CSAAsmPrinter::EmitCSAOperands(const MachineInstr *MI, raw_ostream &O,
  int startindex, int numopds) {
  const CSAMachineFunctionInfo *LMFI = MF->getInfo<CSAMachineFunctionInfo>();
  for (int i=startindex; i<numopds; ++i) {
    unsigned reg = MI->getOperand(i).getReg();
    StringRef name = "";
    if (reg != CSA::IGN && reg != CSA::NA) {
      if (Register::isVirtualRegister(reg))
        name = LMFI->getLICName(reg);
      else
        name = CSAInstPrinter::getRegisterName(reg);
    } else
      name = CSAInstPrinter::getRegisterName(reg);
    O << "%" << name;
    if (i != numopds-1) O << ", ";
  }
}

void CSAAsmPrinter::EmitSimpleEntryInstruction(MachineFunction *CalleeMF) {
  StringRef Linkage("dataflow");
  const Function &F = CalleeMF->getFunction();
  if (F.hasFnAttribute("__csa_attr_initializer"))
    Linkage = "initializer";
  OutStreamer->EmitRawText("\t.entry\t" + CalleeMF->getFunction().getName() +
                           ", " + Linkage);
}

void CSAAsmPrinter::EmitParamsResultsDecl(
  MachineInstr *entryMI, MachineInstr *returnMI) {
  SmallString<128> Str;
  raw_svector_ostream O(Str);
  // Emit CSA parameters
  if (returnMI)
    for (unsigned i = 0; i < returnMI->getNumOperands(); ++i) {
      unsigned reg = returnMI->getOperand(i).getReg();
      O << "\t.result .lic .i" << LMFI->getLICSize(reg) << " %"
        << LMFI->getLICName(reg) << "\n";
      if (i == 1) resultReg = reg;
    }
  if (entryMI) {
    unsigned reg = entryMI->getOperand(0).getReg();
    O << "\t.param .lic .i" << LMFI->getLICSize(reg) << " %"
      << LMFI->getLICName(reg) << "\n";
    for (unsigned i = 1; i < entryMI->getNumOperands(); ++i) {
      unsigned reg = entryMI->getOperand(i).getReg();
        O << "\t.param .lic .i" << LMFI->getLICSize(reg) << " %"
          << LMFI->getLICName(reg) << "\n";
    }
  }
  OutStreamer->EmitRawText(O.str());
}

void CSAAsmPrinter::EmitCallInstruction(const MachineInstr *MI) {
  SmallString<128> Str;
  raw_svector_ostream O(Str);
  O << "\t#.call\t";
  const MachineOperand &MO = MI->getOperand(0);
  if (MO.isGlobal()) {
    const Function *F = dyn_cast<Function>(MO.getGlobal());
    O << F->getName();
  } else if (MO.isSymbol())
    O << MI->getOperand(0).getSymbolName();
  O << ", ";
  EmitCSAOperands(MI,O,2,MI->getNumOperands());
  O << "\n";
  OutStreamer->EmitRawText(O.str());
}

void CSAAsmPrinter::EmitContinueInstruction(const MachineInstr *MI) {
  SmallString<128> Str;
  raw_svector_ostream O(Str);
  O << "\t#.continue\t";
  EmitCSAOperands(MI,O,0,MI->getNumOperands());
  O << "\n";
  OutStreamer->EmitRawText(O.str());
}

void CSAAsmPrinter::EmitAll0(const MachineInstr *MI) {
  const CSAMachineFunctionInfo *LMFI = MF->getInfo<CSAMachineFunctionInfo>();
  SmallString<128> Str;
  raw_svector_ostream O(Str);
  O << "\tall0\t";
  const int OpndCount = MI->getNumOperands();
  bool first = true;
  for (int i=0; i<OpndCount; ++i) {

    // Non-register all0 inputs are no-ops, so don't bother emitting them.
    if (not MI->getOperand(i).isReg()) continue;

    if (not first) O << ", ";
    first = false;

    unsigned reg = MI->getOperand(i).getReg();
    StringRef name = "";
    if (reg != CSA::IGN && reg != CSA::NA) {
      if (Register::isVirtualRegister(reg))
        name = LMFI->getLICName(reg);
      else
        name = CSAInstPrinter::getRegisterName(reg);
    } else
      name = CSAInstPrinter::getRegisterName(reg);
    O << "%" << name;
  }
  O << "\n";
  OutStreamer->EmitRawText(O.str());
}

void CSAAsmPrinter::EmitTrampolineMarkers(const MachineInstr *MI) {
  SmallString<128> Str;
  raw_svector_ostream O(Str);
  if (MI->getOpcode() == CSA::TRAMPOLINE_START)
    O << "\t#.trampoline_start\t";
  if (MI->getOpcode() == CSA::TRAMPOLINE_END)
    O << "\t#.trampoline_end\t";
  O << "\n";
  OutStreamer->EmitRawText(O.str());
}

void CSAAsmPrinter::EmitInstruction(const MachineInstr *MI) {
  if (MI->getOpcode() == CSA::CSA_ENTRY) { return; }
  if (MI->getOpcode() == CSA::CSA_RETURN) return;
  if (MI->getOpcode() == CSA::CSA_CALL) { EmitCallInstruction(MI); return; }
  if (MI->getOpcode() == CSA::CSA_CONTINUE) {
    EmitContinueInstruction(MI);
    return;
  }
  if (MI->getOpcode() == CSA::ALL0) { EmitAll0(MI); return; }
  if (MI->getOpcode() == CSA::TRAMPOLINE_START ||
      MI->getOpcode() == CSA::TRAMPOLINE_END) {
    EmitTrampolineMarkers(MI);
    return;
  }
  if (MI->getFlag(MachineInstr::RasReplayable)) {
      OutStreamer->EmitRawText("\t.attrib ras_replayable=true\n");
  }
  CSAMCInstLower MCInstLowering(OutContext, *this);
  emitLineNumberAsDotLoc(*MI);
  MCInst TmpInst;
  MCInstLowering.Lower(MI, TmpInst);
  EmitToStreamer(*OutStreamer, TmpInst);
}

void CSAAsmPrinter::EmitConstantPool() {
  const MachineConstantPool *MCP                  = MF->getConstantPool();
  const std::vector<MachineConstantPoolEntry> &CP = MCP->getConstants();
  if (CP.empty())
    return;

  // Just emit each constant pool entry in its own scratchpad.
  for (unsigned i = 0, e = CP.size(); i != e; ++i) {
    const MachineConstantPoolEntry &CPE = CP[i];

    const Constant *C = nullptr;
    if (!CPE.isMachineConstantPoolEntry())
      C = CPE.Val.ConstVal;
    assert(C && "Should have a constant for CSA machine constant pools");

    MCSymbol *Sym = GetCPISymbol(i);
    if (!Sym->isUndefined())
      continue;

    EmitScratchpad(Sym, true, C);
  }
}

void CSAAsmPrinter::EmitGlobalVariable(const GlobalVariable *GV) {
  // Handle scratchpads differently from regular global variables.
  if (isScratchpadAddressSpace(GV->getAddressSpace())) {
    // These were emitted elsewhere.
    return;
  }

  // If the global's section name starts with .csa. or if its linkage type is
  // private, it belongs on the CSA and needs to go in the target code.
  // Otherwise, it is a normal global which should go on the host and be pulled
  // in implicitly by the target code. However, if we aren't wrapping assembly
  // nothing should go on the host.
  const bool PutOnHost =
    CSAInstPrinter::WrapCsaAsm() and
    not(GV->getSection().startswith(".csa.") or GV->hasPrivateLinkage());
  if (PutOnHost) {
    OutStreamer->AddBlankLine();
    endCSAAsmString(*OutStreamer);
    OutStreamer->AddBlankLine();
    OutStreamer->PopSection();
  }

  AsmPrinter::EmitGlobalVariable(GV);

  if (PutOnHost) {
    EmitCsaCodeSection();
    OutStreamer->EmitRawText("\t.ascii ");
    startCSAAsmString(*OutStreamer);
  }
}

template <typename Func>
static void printValues(MCStreamer &Streamer, uint64_t Count, Func GetElement) {
  for (uint64_t i = 0; i < Count; i++) {
    SmallString<128> Str;
    raw_svector_ostream O(Str);
    O << "\t.value " << format_hex(GetElement(i).getLimitedValue(), 10) << "\n";
    Streamer.EmitRawText(O.str());
  }
}

void CSAAsmPrinter::EmitScratchpad(MCSymbol *Sym, bool IsConstant,
    const Constant *CV) {
  Sym->redefineIfPossible();
  const DataLayout &DL = getDataLayout();
  uint64_t Size = DL.getTypeAllocSize(CV->getType());

  // Compute the type of the scratchpad.
  Type *ValueTy = CV->getType();
  if (isa<ConstantAggregateZero>(CV)) {
    // All 0's -- use i64 if possible, else i8.
    if (Size % 8 == 0)
      ValueTy = Type::getInt64Ty(ValueTy->getContext());
    else
      ValueTy = Type::getInt8Ty(ValueTy->getContext());
  } else if (auto SeqTy = dyn_cast<SequentialType>(ValueTy)) {
    // Arrays and vectors: look through the array type.
    ValueTy = SeqTy->getElementType();
  }

  if (ValueTy->isPointerTy()) {
    // Convert pointers to integers.
    ValueTy = DL.getIntPtrType(ValueTy);
  } else if (!ValueTy->isIntegerTy() && !ValueTy->isPointerTy() &&
      !ValueTy->isFloatingPointTy()) {
    // Any other type? It's an i8 type (for byte emission).
    ValueTy = Type::getInt8Ty(ValueTy->getContext());
  } else if (ValueTy->getPrimitiveSizeInBits() > 64) {
    // Oversize integers are emitted with byte emission.
    ValueTy = Type::getInt8Ty(ValueTy->getContext());
  }

  // Compute the number of entries.
  uint64_t ValueSize = DL.getTypeAllocSize(ValueTy);
  if (Size % ValueSize != 0) {
    ValueTy = Type::getInt8Ty(ValueTy->getContext());
    ValueSize = 1;
  }
  uint64_t EntryCount = Size / ValueSize;

  // Emit the .spad directive.
  SmallString<128> Str;
  raw_svector_ostream O(Str);
  O << ".text\n"; // The simulator requires this...
  O << (IsConstant ? ".rom " : ".spad ");
  if (ValueTy->isFloatTy())
    O << ".f32";
  else if (ValueTy->isDoubleTy())
    O << ".f64";
  else {
    assert(ValueTy->isIntegerTy() && "Scratchpad isn't int or float?");
    O << ".i" << (ValueSize * 8);
  }
  O << " " << Sym->getName() << "[" << EntryCount << "]\n";
  OutStreamer->EmitRawText(O.str());

  // If there are values to emit, emit them now.
  if (isa<ConstantAggregateZero>(CV)) {
    // Do nothing--initialized to 0 by default.
  } else if (auto CDS = dyn_cast<ConstantDataSequential>(CV)) {
    if (ValueTy->isFloatingPointTy())
      printValues(*OutStreamer, EntryCount,
          [&](int i) { return CDS->getElementAsAPFloat(i).bitcastToAPInt(); });
    else
      printValues(*OutStreamer, EntryCount,
          [&](int i) { return CDS->getElementAsAPInt(i); });
  } else {
    report_fatal_error("Scratchpad constant format unhandled");
  }
}

// Force static initialization.
extern "C" void LLVMInitializeCSAAsmPrinter() {
  RegisterAsmPrinter<CSAAsmPrinter> X(getTheCSATarget());
}
