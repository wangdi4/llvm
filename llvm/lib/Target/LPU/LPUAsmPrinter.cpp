//===-- LPUAsmPrinter.cpp - LPU LLVM assembly writer ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains a printer that converts from our internal representation
// of machine-dependent LLVM code to the LPU assembly language.
//
//===----------------------------------------------------------------------===//

#include "LPU.h"
#include "InstPrinter/LPUInstPrinter.h"
#include "LPUInstrInfo.h"
#include "LPUMCInstLower.h"
#include "LPUTargetMachine.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Mangler.h"
#include "llvm/IR/Module.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/ELF.h"
#include <fstream>
#include <sstream>
using namespace llvm;

#define DEBUG_TYPE "asm-printer"

static cl::opt<bool>
EmitLineNumbers("lpu-emit-line-numbers", cl::Hidden,
                cl::desc("LPU Specific: Emit Line numbers even without -G"),
                cl::init(true));

static cl::opt<bool>
InterleaveSrc("lpu-emit-src", cl::ZeroOrMore, cl::Hidden,
              cl::desc("LPU Specific: Emit source line in asm file"),
              cl::init(false));

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

  class LPUAsmPrinter : public AsmPrinter {
    const Function *F;
    const MachineRegisterInfo *MRI;
    DebugLoc prevDebugLoc;
    bool ignoreLoc(const MachineInstr &);
    LineReader* reader;
    // To record filename to ID mapping
    std::map<std::string, unsigned> filenameMap;
    void recordAndEmitFilenames(Module &);
    bool doInitialization(Module &M) override;
    void emitLineNumberAsDotLoc(const MachineInstr &);
    void emitSrcInText(StringRef filename, unsigned line);
    LineReader* getReader(std::string);
    void emitParamList(const Function *);
    void emitReturnVal(const Function*);

    void writeAsmLine(const char *);

  public:
    LPUAsmPrinter(TargetMachine &TM,
            std::unique_ptr<MCStreamer> Streamer)
      : AsmPrinter(TM, std::move(Streamer)), reader() {}

    const char *getPassName() const override {
      return "LPU Assembly Printer";
    }

    void EmitStartOfAsmFile(Module &) override;
    void EmitEndOfAsmFile(Module &) override;

    void EmitFunctionEntryLabel() override;
    void EmitFunctionBodyStart() override;
    void EmitFunctionBodyEnd() override;
    void EmitInstruction(const MachineInstr *MI) override;
  };
} // end of anonymous namespace

bool LPUAsmPrinter::ignoreLoc(const MachineInstr &MI) {
  switch (MI.getOpcode()) {
  default:
    return false;
    // May be desirable to avoid LPU-specific MachineInstrs
  }
}

void LPUAsmPrinter::recordAndEmitFilenames(Module &M) {
  DebugInfoFinder DbgFinder;
  DbgFinder.processModule(M);

  unsigned i = 1;
  for (const DICompileUnit *DIUnit : DbgFinder.compile_units()) {
    StringRef Filename(DIUnit->getFilename());
    StringRef Dirname(DIUnit->getDirectory());
    SmallString<128> FullPathName = Dirname;
    if (!Dirname.empty() && !sys::path::is_absolute(Filename)) {
      sys::path::append(FullPathName, Filename);
      Filename = FullPathName.str();
    }
    if (filenameMap.find(Filename.str()) != filenameMap.end())
      continue;
    filenameMap[Filename.str()] = i;
    OutStreamer->EmitDwarfFileDirective(i, "", Filename.str());
    ++i;
  }

  for (const DISubprogram *SP : DbgFinder.subprograms()) {
    StringRef Filename(SP->getFilename());
    StringRef Dirname(SP->getDirectory());
    SmallString<128> FullPathName = Dirname;
    if (!Dirname.empty() && !sys::path::is_absolute(Filename)) {
      sys::path::append(FullPathName, Filename);
      Filename = FullPathName.str();
    }
    if (filenameMap.find(Filename.str()) != filenameMap.end())
      continue;
    filenameMap[Filename.str()] = i;
    ++i;
  }
}

bool LPUAsmPrinter::doInitialization(Module &M) {
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

  recordAndEmitFilenames(M);

  return result;
}

void LPUAsmPrinter::emitLineNumberAsDotLoc(const MachineInstr &MI) {
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
  SmallString<128> FullPathName = dirName;
  if (!dirName.empty() && !sys::path::is_absolute(fileName)) {
    sys::path::append(FullPathName, fileName);
    fileName = FullPathName.str();
  }

  if (filenameMap.find(fileName.str()) == filenameMap.end())
    return;

  // Emit the line from the source file.
  if (InterleaveSrc)
    this->emitSrcInText(fileName.str(), curLoc.getLine());

  std::stringstream temp;
  temp << "\t.loc " << filenameMap[fileName.str()] << " " << curLoc.getLine()
       << " " << curLoc.getCol();
  OutStreamer->EmitRawText(Twine(temp.str().c_str()));
}

void LPUAsmPrinter::emitSrcInText(StringRef filename, unsigned line) {
  std::stringstream temp;
  LineReader *reader = this->getReader(filename.str());
  temp << "\n//";
  temp << filename.str();
  temp << ":";
  temp << line;
  temp << " ";
  temp << reader->readLine(line);
  temp << "\n";
  this->OutStreamer->EmitRawText(Twine(temp.str()));
}

LineReader *LPUAsmPrinter::getReader(std::string filename) {
  if (!reader) {
    reader = new LineReader(filename);
  }

  if (reader->fileName() != filename) {
    delete reader;
    reader = new LineReader(filename);
  }

  return reader;
}

void LPUAsmPrinter::emitParamList(const Function *F) {
  SmallString<128> Str;
  raw_svector_ostream O(Str);
  const TargetLowering *TLI = MF->getSubtarget<LPUSubtarget>().getTargetLowering();
  Function::const_arg_iterator I, E;
  unsigned paramIndex = 0;
  MVT thePointerTy = TLI->getPointerTy();

  // Stride through parameters, putting out a .param {type} .reg %r{num}
  // This is a hack mostly taken from NVPTX.  This assumes successive
  // parameters go to successive registers, starting with the initial
  // value of paramReg.  This may be too simplistic for longer term.
  int paramReg = 2;  // Params start in R0 - see LPUCallingConv.td
  bool first = true;
  for (I = F->arg_begin(), E = F->arg_end(); I != E; ++I, paramIndex++) {
    Type *Ty = I->getType();
    unsigned sz = 0;
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
    O << LPUInstPrinter::WrapLpuAsmLinePrefix();
    O << "\t.param .reg " << typeStr << sz << " %r" << paramReg++;
    O << LPUInstPrinter::WrapLpuAsmLineSuffix();
    first = false;
  }
  if (!first)
    OutStreamer->EmitRawText(O.str());
}

void LPUAsmPrinter::emitReturnVal(const Function *F) {
  SmallString<128> Str;
  raw_svector_ostream O(Str);
  const TargetLowering *TLI = MF->getSubtarget<LPUSubtarget>().getTargetLowering();

  Type *Ty = F->getReturnType();

  if (Ty->getTypeID() == Type::VoidTyID)
    return;

  O << LPUInstPrinter::WrapLpuAsmLinePrefix();
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
    O << " .i" << TLI->getPointerTy().getSizeInBits();
  } else if ((Ty->getTypeID() == Type::StructTyID) || isa<VectorType>(Ty)) {
    llvm_unreachable("NYI: aggregate result");
  } else
    llvm_unreachable("Unknown return type");

  // Hack: For now, we simply go with the standard return register.
  // (Should really use the allocation.)
  O << " %r0";
  O << LPUInstPrinter::WrapLpuAsmLineSuffix();

  OutStreamer->EmitRawText(O.str());
}

void LPUAsmPrinter::writeAsmLine(const char *text) {
  SmallString<128> Str;
  raw_svector_ostream O(Str);

  O << LPUInstPrinter::WrapLpuAsmLinePrefix();
  O << text;
  O << LPUInstPrinter::WrapLpuAsmLineSuffix();
  OutStreamer->EmitRawText(O.str());
}

void LPUAsmPrinter::EmitStartOfAsmFile(Module &M) {

  if (LPUInstPrinter::WrapLpuAsm()) {
    // Put the code in the .lpu section. Note that we are NOT
    // using SwitchSection because then we'll fight with the
    // AmsPrinter::EmitFunctionHeader
    OutStreamer->EmitRawText("\t.section .lpu.code");
    writeAsmLine("\t.text");
  }
  
  /* Disabled 2016/3/31.  Long term, we should only put this out if it
   * is not autounit.  The theory is if the compiler has done tailoring
   * for a specific target, that should be reflected in the file.
   */
  SmallString<128> Str;
  raw_svector_ostream O(Str);
  const LPUTargetMachine *LPUTM = static_cast<const LPUTargetMachine*>(&TM);
  assert(LPUTM && LPUTM->getSubtargetImpl());
  O << LPUInstPrinter::WrapLpuAsmLinePrefix();
  O << "\t# .processor ";  // note - commented out...
  O << LPUTM->getSubtargetImpl()->lpuName();
  O << LPUInstPrinter::WrapLpuAsmLineSuffix();
  OutStreamer->EmitRawText(O.str());

  writeAsmLine("\t.version 0,6,0");
  writeAsmLine("\t.unit sxu");
}

void LPUAsmPrinter::EmitEndOfAsmFile(Module &M) {

  if (LPUInstPrinter::WrapLpuAsm()) {
    // Add the terminating null for the .lpu section. Note
    // that we are NOT using SwitchSection because then we'll
    // fight with the AmsPrinter::EmitFunctionHeader
    OutStreamer->EmitRawText("\t.section .lpu.code");
    OutStreamer->EmitRawText("\t.asciz \"\"");
  }
}

void LPUAsmPrinter::EmitFunctionEntryLabel() {
  SmallString<128> Str;
  raw_svector_ostream O(Str);

  // Set up
  MRI = &MF->getRegInfo();
  F = MF->getFunction();

  // If we're wrapping the LPU assembly, the global symbol declaration
  // generated by MCAsmStreamer::EmitSymbolAttribute() will be commented
  // out, so we need to create our own
  if (LPUInstPrinter::WrapLpuAsm()) {
    O << LPUInstPrinter::WrapLpuAsmLinePrefix();
    O << "\t.globl\t" << *CurrentFnSym;
    O << LPUInstPrinter::WrapLpuAsmLineSuffix();
    O << "\n";
  }
  O << LPUInstPrinter::WrapLpuAsmLinePrefix();
  O << "\t.entry\t" << *CurrentFnSym;
  O << LPUInstPrinter::WrapLpuAsmLineSuffix();
  O << "\n";
  // For now, assume control flow (sequential) entry
  O << LPUInstPrinter::WrapLpuAsmLinePrefix();
  O << *CurrentFnSym << ":";
  O << LPUInstPrinter::WrapLpuAsmLineSuffix();
  OutStreamer->EmitRawText(O.str());

  // Start a scope for this routine to localize the LIC names
  // For now, this includes parameters and results
  writeAsmLine("{");

  emitReturnVal(F);

  emitParamList(F);
}


void LPUAsmPrinter::EmitFunctionBodyStart() {
  //  const MachineRegisterInfo *MRI;
  MRI = &MF->getRegInfo();
  const LPUMachineFunctionInfo *LMFI = MF->getInfo<LPUMachineFunctionInfo>();

  // Generate declarations for each LIC by looping over the LIC classes,
  // and over each lic in the class, outputting a decl if needed.
  // Note: If we start allowing parameters and results in LICs for
  // HybridDataFlow, this may need to be revisited to make sure they
  // are in order.
  for (TargetRegisterClass::iterator ri = LPU::ANYCRegClass.begin();
                                     ri != LPU::ANYCRegClass.end(); ++ri) {
    MCPhysReg reg = *ri;
    if (LMFI->isAllocated(reg)) {
      SmallString<128> Str;
      raw_svector_ostream O(Str);
      O << LPUInstPrinter::WrapLpuAsmLinePrefix();
      O << "\t";
      // LIC or register
      O << (LPU::ANYCRegClass.contains(reg) ? ".lic " : ".reg ");
      // Output type based on regclass
      if      (LPU::CI64RegClass.contains(reg)) O << ".i64";
      else if (LPU::CI32RegClass.contains(reg)) O << ".i32";
      else if (LPU::CI16RegClass.contains(reg)) O << ".i16";
      else if (LPU::CI8RegClass.contains(reg))  O << ".i8";
      else if (LPU::CI1RegClass.contains(reg))  O << ".i1";
      else if (LPU::CI0RegClass.contains(reg))  O << ".i0";
      O << " " << LPUInstPrinter::getRegisterName(reg);
      O << LPUInstPrinter::WrapLpuAsmLineSuffix();
      OutStreamer->EmitRawText(O.str());
    }
  }

}

void LPUAsmPrinter::EmitFunctionBodyEnd() {
  writeAsmLine("}");
#if 0
  SmallString<128> Str;
  raw_svector_ostream O(Str);

  if (LPUInstPrinter::WrapLpuAsm()) {
    O << "\t.asciz \"";
  }
  O << "}";
  O << LPUInstPrinter::WrapLpuAsmLineSuffix();
  OutStreamer->EmitRawText(O.str());
#endif
}

void LPUAsmPrinter::EmitInstruction(const MachineInstr *MI) {
  LPUMCInstLower MCInstLowering(OutContext, *this);
  emitLineNumberAsDotLoc(*MI);
  MCInst TmpInst;
  MCInstLowering.Lower(MI, TmpInst);
  EmitToStreamer(*OutStreamer, TmpInst);
}

// Force static initialization.
extern "C" void LLVMInitializeLPUAsmPrinter() {
  RegisterAsmPrinter<LPUAsmPrinter> X(TheLPUTarget);
}
