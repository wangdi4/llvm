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
  void emitEnums(raw_ostream &OS, const std::vector<Record *> &GenericOps);
};

} // end anonymous namespace

//===----------------------------------------------------------------------===//
// Main Output.
//===----------------------------------------------------------------------===//

// run - Emit the main instruction description records for the target...
void CSAOpSizes::run(raw_ostream &OS) {
  auto GenericOps = Records.getAllDerivedDefinitions("GenericOp");

  emitSourceFileHeader("CSA generic opcode mapping tables", OS);
  emitEnums(OS, GenericOps);

  OS << "#ifdef GET_OPC_GENERIC_MAP\n";
  OS << "#undef GET_OPC_GENERIC_MAP\n";

  OS << "namespace llvm {\n\n";

  CodeGenTarget &Target = CDP.getTargetInfo();
  auto Namespace = Target.getInstNamespace();
  typedef std::tuple<size_t, unsigned, unsigned, const CodeGenInstruction *> ReverseMapTy;
  std::vector<ReverseMapTy> ReverseMap;

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
      unsigned size = opInfo->getValueAsInt("OpBitSize");
      unsigned classification;
      auto suffixStr = opInfo->getValueAsString("InstrSuffix");
      if (suffixStr[0] == 's')
        classification = 2;
      else if (suffixStr[0] == 'u')
        classification = 3;
      else if (suffixStr[0] == 'f')
        classification = 1;
      else
        classification = 0;
      ReverseMap.emplace_back(
        std::find(GenericOps.begin(), GenericOps.end(), genOp) - GenericOps.begin(),
        size, classification, II);
      OS << genOp->getName() << ", ";
      OS << size << ", " << classification;
    } else {
      OS << "INVALID_OP, 0, 0";
    }
    OS << " }, // " << II->TheDef->getName() << "\n";
  }
  OS << "};\n";

  // Sort the reverse map
  std::sort(ReverseMap.begin(), ReverseMap.end());
  OS << "\nstatic GenericOpcMap generic_to_opcode_map[] = {\n";
  for (auto &val : ReverseMap) {
    OS << "  { " << Namespace << "::Generic::" <<
      GenericOps[std::get<0>(val)]->getName();
    auto II = std::get<3>(val);
    OS << ", " << II->Namespace << "::" << II->TheDef->getName();
    OS << ", " << std::get<1>(val) << ", " << std::get<2>(val) << " },\n";
  }
  // An invalid operation at the end to prevent reading off the end of the
  // array.
  OS << "  { " << Namespace << "::Generic::INVALID_OP, 0, 0, 0 }\n";
  OS << "};\n";

  // Emit an index map that indexes into the first value of the array.
  OS << "\nstatic size_t generic_index_map[] = {\n";
  OS << "  ~0U,\n"; // INVALID_OP, which is invalid
  unsigned expected = ~0U;
  for (size_t i = 0; i < ReverseMap.size(); i++) {
    unsigned index = std::get<0>(ReverseMap[i]);
    if (index != expected) {
      // This is the next index. Now, we should have everything in the reverse
      // map, but if we don't, then we need to emit extra entries.
      for (unsigned val = expected + 1; val != index; val++)
        OS << "  ~0U, // " << GenericOps[val]->getName() << "\n";
      OS << "  " << i << ", // " << GenericOps[index]->getName() << "\n";
    }
    expected = index;
  }
  OS << "};\n";

  OS << "} // end llvm namespace\n";

  OS << "#endif // GET_OPC_GENERIC_MAP\n\n";
}

// emitEnums - Print out enum values for all of the instructions.
void CSAOpSizes::emitEnums(raw_ostream &OS,
    const std::vector<Record *> &GenericOps) {
  OS << "#ifdef GET_CSAOPGENERIC_ENUM\n";
  OS << "#undef GET_CSAOPGENERIC_ENUM\n";

  OS << "namespace llvm {\n\n";

  CodeGenTarget Target(Records);

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
