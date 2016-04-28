//
//      Copyright (c) 2016 Intel Corporation.
//      All rights reserved.
//
//        INTEL CORPORATION PROPRIETARY INFORMATION
//
// This software is supplied under the terms of a license
// agreement or nondisclosure agreement with Intel Corp.
// and may not be copied or disclosed except in accordance
// with the terms of that agreement.
//
// ===--------------------------------------------------------------------=== //
///
/// \file
/// This tablegen backend is responsible for emitting the enums and tables for
/// the directives and clauses that support parallelization/vectorization.
/// These tables are used to look up strings for use as arguments to intrinsics
/// that define vector/parallel regions and constructs.
///
///  External interfaces:
///      void EmitDirectives(RecordKeeper &RK, raw_ostream &OS);
///
// ===--------------------------------------------------------------------=== //

#include "CodeGenTarget.h"
#include "llvm/Support/Format.h"
#include "llvm/TableGen/Error.h"
#include "llvm/TableGen/Record.h"
#include "llvm/TableGen/TableGenBackend.h"

using namespace llvm;

#define DEBUG_TYPE "IntelDirectives"
#include "llvm/Support/Debug.h"

namespace {

class DirectivesEmitter {

  RecordKeeper &Records;

private:
  void emitDirectivesEnums(raw_ostream &OS);
  void emitClausesEnums(raw_ostream &OS);
  void emitDirectivesStringsTable(raw_ostream &OS);
  void emitClausesStringsTable(raw_ostream &OS);
  void emitDirectivesIdsTable(raw_ostream &OS);
  void emitClausesIdsTable(raw_ostream &OS);

public:
  DirectivesEmitter(RecordKeeper &R) : Records(R) {}

  // Output the directives and clauses to an enum and enum to string map.
  void run(raw_ostream &OS);
};
} // End anonymous namespace

/// \brief Emit the enumerations for parallel/vector directives.
/// E.g., things like the begin/end of a simd loop, parallel region, etc.
void DirectivesEmitter::emitDirectivesEnums(raw_ostream &OS) {

  Record *DirectiveClass = Records.getClass("Directive");
  assert(DirectiveClass &&
         "Directive class not found in target description file!");

  OS << "#ifdef GET_DIRECTIVES_ENUM_VALUES\n";

  for (const auto &D : Records.getDefs()) {
    if (D.second->isSubClassOf(DirectiveClass)) {
      std::string DirectiveName = D.first;
      StringRef DirectiveEnum = StringRef(DirectiveName);
      OS << "  " << DirectiveEnum << ",\n";
    }
  }

  OS << "#endif // GET_DIRECTIVES_ENUM_VALUES\n\n";
}

/// \brief Emit the enumerations for parallel/vector clauses.
/// E.g., things like the shared clause for OpenMP.
void DirectivesEmitter::emitClausesEnums(raw_ostream &OS) {

  Record *ClauseClass = Records.getClass("Clause");
  assert(ClauseClass && "Clause class not found in target description file!");

  OS << "#ifdef GET_CLAUSES_ENUM_VALUES\n";

  for (const auto &D : Records.getDefs()) {
    if (D.second->isSubClassOf(ClauseClass)) {
      std::string ClauseName = D.first;
      StringRef ClauseEnum = StringRef(ClauseName);
      OS << "  " << ClauseEnum << ",\n";
    }
  }

  OS << "#endif // GET_CLAUSES_ENUM_VALUES\n\n";
}

/// \brief emit the enum to string mapping for the vector/parallel directives.
void DirectivesEmitter::emitDirectivesStringsTable(raw_ostream &OS) {

  Record *DirectiveClass = Records.getClass("Directive");
  assert(DirectiveClass &&
         "Directive class not found in target description file!");

  OS << "#ifdef GET_DIRECTIVES_STRINGS_TABLE\n";

  for (const auto &D : Records.getDefs()) {
    if (D.second->isSubClassOf(DirectiveClass)) {
      std::string DirectiveName = D.first;
      StringRef DirectiveEnum = StringRef(DirectiveName);
      SmallVector<StringRef, 4> StringParts;
      DirectiveEnum.split(StringParts, '_');
      std::string DirectiveString = StringParts[0];
      for (unsigned Idx = 1; Idx < StringParts.size(); Idx++) {
        DirectiveString = DirectiveString + "." + StringParts[Idx].str();
      }
      OS << "  { " << DirectiveEnum << ",\n";
      OS << "    \"" << DirectiveString << "\" },\n";
    }
  }

  OS << "#endif // GET_DIRECTIVES_STRINGS_TABLE\n\n";
}

/// \brief emit the string to enum mapping for the vector/parallel directives.
void DirectivesEmitter::emitDirectivesIdsTable(raw_ostream &OS) {

  Record *DirectiveClass = Records.getClass("Directive");
  assert(DirectiveClass &&
         "Directive class not found in target description file!");

  OS << "#ifdef GET_DIRECTIVES_IDS_TABLE\n";

  for (const auto &D : Records.getDefs()) {
    if (D.second->isSubClassOf(DirectiveClass)) {
      std::string DirectiveName = D.first;
      StringRef DirectiveEnum = StringRef(DirectiveName);
      SmallVector<StringRef, 4> StringParts;
      DirectiveEnum.split(StringParts, '_');
      std::string DirectiveString = StringParts[0];
      for (unsigned Idx = 1; Idx < StringParts.size(); Idx++) {
        DirectiveString = DirectiveString + "." + StringParts[Idx].str();
      }
      OS << "  { \"" << DirectiveString << "\",\n";
      OS << "    " << DirectiveEnum << " },\n";
    }
  }

  OS << "#endif // GET_DIRECTIVES_IDS_TABLE\n\n";
}

/// \brief emit the enum to string mapping for the vector/parallel clauses.
void DirectivesEmitter::emitClausesStringsTable(raw_ostream &OS) {

  Record *ClauseClass = Records.getClass("Clause");
  assert(ClauseClass && "Clause class not found in target description file!");

  OS << "#ifdef GET_CLAUSES_STRINGS_TABLE\n";

  for (const auto &D : Records.getDefs()) {
    if (D.second->isSubClassOf(ClauseClass)) {
      std::string ClauseName = D.first;
      StringRef ClauseEnum = StringRef(ClauseName);
      SmallVector<StringRef, 4> StringParts;
      ClauseEnum.split(StringParts, '_');
      std::string ClauseString = StringParts[0];
      for (unsigned Idx = 1; Idx < StringParts.size(); Idx++) {
        ClauseString = ClauseString + "." + StringParts[Idx].str();
      }
      OS << "  { " << ClauseEnum << ",\n";
      OS << "    \"" << ClauseString << "\" },\n";
    }
  }

  OS << "#endif // GET_CLAUSES_STRINGS_TABLE\n\n";
}

/// \brief emit the string to enum mapping for the vector/parallel clauses.
void DirectivesEmitter::emitClausesIdsTable(raw_ostream &OS) {

  Record *ClauseClass = Records.getClass("Clause");
  assert(ClauseClass && "Clause class not found in target description file!");

  OS << "#ifdef GET_CLAUSES_IDS_TABLE\n";

  for (const auto &D : Records.getDefs()) {
    if (D.second->isSubClassOf(ClauseClass)) {
      std::string ClauseName = D.first;
      StringRef ClauseEnum = StringRef(ClauseName);
      SmallVector<StringRef, 4> StringParts;
      ClauseEnum.split(StringParts, '_');
      std::string ClauseString = StringParts[0];
      for (unsigned Idx = 1; Idx < StringParts.size(); Idx++) {
        ClauseString = ClauseString + "." + StringParts[Idx].str();
      }
      OS << "  { \"" << ClauseString << "\",\n";
      OS << "    " << ClauseEnum << " },\n";
    }
  }

  OS << "#endif // GET_CLAUSES_IDS_TABLE\n\n";
}

void DirectivesEmitter::run(raw_ostream &OS) {
  emitDirectivesEnums(OS);
  emitClausesEnums(OS);
  emitDirectivesStringsTable(OS);
  emitClausesStringsTable(OS);
  emitDirectivesIdsTable(OS);
  emitClausesIdsTable(OS);
}

namespace llvm {

void EmitDirectives(RecordKeeper &RK, raw_ostream &OS) {
  DirectivesEmitter(RK).run(OS);
}

} // End llvm namespace
