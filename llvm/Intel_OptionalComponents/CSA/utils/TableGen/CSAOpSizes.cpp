//===- CSAOpSizes.cpp - Generate CSA instruction tables ---------*- C++ -*-===//
//
// Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
///===---------------------------------------------------------------------===//
/// \file
///
/// This tablegen backend is responsible for helping the CSA optimization passes
/// match between specific instructions and generic ops.
///
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

// TODO (vzakhari 10/8/2018): replace the tuple with a real struct,
//       to make the accesses to ReverseMap more readable.
typedef std::vector<
  std::tuple<size_t, unsigned, unsigned, const CodeGenInstruction *>
  > ReverseMapTy;

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
  void emitMIRMatcher(raw_ostream &OS, const ReverseMapTy &ReverseMap,
    const std::vector<Record *> &GenericOps);
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
  ReverseMapTy ReverseMap;

  OS << "static OpcGenericMap opcode_to_generic_map[] = {\n";
  for (auto &II : Target.getInstructionsByEnumValue()) {
    RecordVal *GenOpValue = II->TheDef->getValue("GenOp");
    Record *GenOp = nullptr, *opInfo = nullptr;
    if (GenOpValue && isa<DefInit>(GenOpValue->getValue())) {
      GenOp = cast<DefInit>(GenOpValue->getValue())->getDef();
      opInfo = II->TheDef->getValueAsDef("OpInfo");
    }
    OS << "  { " << Namespace << "::Generic::";
    if (GenOp) {
      unsigned Size = opInfo->getValueAsInt("OpBitSize");
      unsigned Classification;
      auto suffixStr = opInfo->getValueAsString("InstrSuffix");
      if (suffixStr == "f16x4")
        Classification = 5;
      else if (suffixStr == "f32x2")
        Classification = 4;
      else if (suffixStr[0] == 's')
        Classification = 2;
      else if (suffixStr[0] == 'u')
        Classification = 3;
      else if (suffixStr[0] == 'f')
        Classification = 1;
      else
        Classification = 0;
      ReverseMap.emplace_back(
        std::find(GenericOps.begin(), GenericOps.end(), GenOp) - GenericOps.begin(),
        Size, Classification, II);
      OS << GenOp->getName() << ", ";
      OS << Size << ", " << Classification;
    } else {
      OS << "INVALID_OP, 0, 0";
    }
    OS << " }, // " << II->TheDef->getName() << "\n";
  }
  OS << "};\n";

  // Sort the reverse map
  std::sort(ReverseMap.begin(), ReverseMap.end());
  OS << "\nstatic GenericOpcMap generic_to_opcode_map[] = {\n";
  for (auto &Val : ReverseMap) {
    OS << "  { " << Namespace << "::Generic::" <<
      GenericOps[std::get<0>(Val)]->getName();
    auto II = std::get<3>(Val);
    OS << ", " << II->Namespace << "::" << II->TheDef->getName();
    OS << ", " << std::get<1>(Val) << ", " << std::get<2>(Val) << " },\n";
  }
  // An invalid operation at the end to prevent reading off the end of the
  // array.
  OS << "  { " << Namespace << "::Generic::INVALID_OP, 0, 0, 0 }\n";
  OS << "};\n";

  // Emit an index map that indexes into the first value of the array.
  OS << "\nstatic size_t generic_index_map[] = {\n";
  OS << "  ~0U,\n"; // INVALID_OP, which is invalid
  unsigned Expected = ~0U;
  for (size_t I = 0; I < ReverseMap.size(); I++) {
    unsigned Index = std::get<0>(ReverseMap[I]);
    if (Index != Expected) {
      // This is the next index. Now, we should have everything in the reverse
      // map, but if we don't, then we need to emit extra entries.
      for (unsigned Val = Expected + 1; Val != Index; Val++)
        OS << "  ~0U, // " << GenericOps[Val]->getName() << "\n";
      OS << "  " << I << ", // " << GenericOps[Index]->getName() << "\n";
    }
    Expected = Index;
  }
  OS << "};\n";

  OS << "} // end llvm namespace\n";

  OS << "#endif // GET_OPC_GENERIC_MAP\n\n";

  emitMIRMatcher(OS, ReverseMap, GenericOps);
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

void CSAOpSizes::emitMIRMatcher(raw_ostream &OS, const ReverseMapTy &ReverseMap,
    const std::vector<Record *> &GenericOps) {
  OS << "#ifdef GET_MIRMATCHERS\n";
  OS << "#undef GET_MIRMATCHERS\n";

  OS << "namespace llvm {\n\n";

  CodeGenTarget Target(Records);

  StringRef Namespace = Target.getInstNamespace();

  if (Namespace.empty())
    PrintFatalError("No instructions defined!");

  OS << "namespace " << Namespace << "Match {\n";

  const auto &Insts = Target.getInstructionsByEnumValue();
  for (auto &II : Insts) {
    // Ignore target-independent opcodes
    if (II->Namespace == "TargetOpcode")
      continue;
    StringRef Name = II->TheDef->getName();
    OS << "  constexpr mirmatch::OpcodeMatcher<" << II->Namespace << "::"
      << Name << "> " << Name.lower() << "{};\n";
  }

  OS << "\n";

  // Add the reverse opcode map.
  unsigned Expected = ~0U;
  bool NeedToClose = false, NeedComma = false;
  for (size_t I = 0; I < ReverseMap.size(); I++) {
    unsigned Index = std::get<0>(ReverseMap[I]);
    if (Index != Expected) {
      if (NeedToClose) {
        OS << "> " << GenericOps[Expected]->getName().lower() << "_N{};\n";
      }
      OS << "  constexpr mirmatch::OpcodeGroupMatcher<";
      NeedToClose = true;
      NeedComma = false;
    }
    Expected = Index;

    if (NeedComma)
      OS << ", ";
    auto &II = std::get<3>(ReverseMap[I]);
    OS << II->Namespace << "::" << II->TheDef->getName();
    NeedComma = true;
  }
  if (NeedToClose) {
    OS << "> " << GenericOps[Expected]->getName().lower() << "_N{};\n";
  }
  OS << "} // end " << Namespace << "Match namespace\n";
  OS << "} // end llvm namespace\n";

  OS << "#endif // GET_MIRMATCHERS\n\n";
}

namespace llvm {

void EmitCSAOpTypes(RecordKeeper &RK, raw_ostream &OS) {
  CSAOpSizes(RK).run(OS);
  EmitMapTable(RK, OS);
}

} // end llvm namespace
