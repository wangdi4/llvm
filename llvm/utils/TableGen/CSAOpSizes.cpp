//===- CSAOpSizes.cpp - Generate CSA instruction tables ---------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This tablegen backend is responsible for helping the CSA optimization passes
// match between specific instructions and generic ops.
//
//===----------------------------------------------------------------------===//

#include "CodeGenDAGPatterns.h"
#include "CodeGenInstruction.h"
#include "CodeGenSchedule.h"
#include "CodeGenTarget.h"
#include "SequenceToOffsetTable.h"
#include "TableGenBackends.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/TableGen/Error.h"
#include "llvm/TableGen/Record.h"
#include "llvm/TableGen/TableGenBackend.h"
#include <cassert>
#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>

using namespace llvm;

namespace {

class CSAOpSizes {
  RecordKeeper &Records;
  CodeGenDAGPatterns CDP;

public:
  CSAOpSizes(RecordKeeper &R):
    Records(R), CDP(R) {}

  // run - Output the instruction set description.
  void run(raw_ostream &OS);

private:
  void emitEnums(raw_ostream &OS);
};

} // end anonymous namespace

//===----------------------------------------------------------------------===//
// Main Output.
//===----------------------------------------------------------------------===//

// run - Emit the main instruction description records for the target...
void CSAOpSizes::run(raw_ostream &OS) {
  emitSourceFileHeader("CSA generic opcode mapping tables", OS);
  emitEnums(OS);

  OS << "#ifdef GET_OPC_GENERIC_MAP\n";
  OS << "#undef GET_OPC_GENERIC_MAP\n";

  OS << "namespace llvm {\n\n";

  CodeGenTarget &Target = CDP.getTargetInfo();
  auto Namespace = Target.getInstNamespace();

  OS << "static OpcGenericMap opcode_to_generic_map[] = {\n";
  for (auto &II : Target.getInstructionsByEnumValue()) {
    RecordVal *genOpValue = II->TheDef->getValue("GenOp");
    Record *genOp = nullptr, *opInfo = nullptr;
    if (genOpValue && isa<DefInit>(genOpValue->getValue())) {
      genOp = cast<DefInit>(genOpValue->getValue())->getDef();
      opInfo = II->TheDef->getValueAsDef("OpInfo");
    }
    OS << "  { " << Namespace << "::Generic::";
    if (genOp) {
      OS << genOp->getName() << ", ";
      OS << opInfo->getValueAsInt("OpBitSize") << ", ";
      auto suffixStr = opInfo->getValueAsString("InstrSuffix");
      if (suffixStr[0] == 's')
        OS << "2";
      else if (suffixStr[0] == 'u')
        OS << "3";
      else if (suffixStr[0] == 'f')
        OS << "1";
      else
        OS << "0";
    } else {
      OS << "INVALID_OP, 0, 0";
    }
    OS << " }, // " << II->TheDef->getName() << "\n";
  }
  OS << "};\n";

  OS << "} // end llvm namespace\n";

  OS << "#endif // GET_OPC_GENERIC_MAP\n\n";
}

// emitEnums - Print out enum values for all of the instructions.
void CSAOpSizes::emitEnums(raw_ostream &OS) {
  OS << "#ifdef GET_CSAOPGENERIC_ENUM\n";
  OS << "#undef GET_CSAOPGENERIC_ENUM\n";

  OS << "namespace llvm {\n\n";

  CodeGenTarget Target(Records);
  auto GenericOps = Records.getAllDerivedDefinitions("GenericOp");

  // We must emit the PHI opcode first...
  StringRef Namespace = Target.getInstNamespace();

  if (Namespace.empty())
    PrintFatalError("No instructions defined!");

  OS << "namespace " << Namespace << " {\n";
  OS << "  enum class Generic {\n";
  OS << "    INVALID_OP\t= 0,\n";
  unsigned Num = 1;
  for (auto &OpInfo : GenericOps) {
    OS << "    " << OpInfo->getName() << "\t= " << Num++ << ",\n";
  }
  OS << "  };\n\n";
  OS << "  constexpr unsigned NUM_GENERIC_OPS = " << Num << ";\n";
  OS << "} // end " << Namespace << " namespace\n";
  OS << "} // end llvm namespace\n";

  OS << "#endif // GET_CSAOPGENERIC_ENUM\n\n";
}

namespace llvm {

void EmitCSAOpTypes(RecordKeeper &RK, raw_ostream &OS) {
  CSAOpSizes(RK).run(OS);
  EmitMapTable(RK, OS);
}

} // end llvm namespace
