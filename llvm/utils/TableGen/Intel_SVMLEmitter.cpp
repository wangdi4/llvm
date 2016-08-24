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
/// This tablegen backend is responsible for emitting the set of SVML variants.
///
///  External interfaces:
///      void EmitSVMLVariants(RecordKeeper &RK, raw_ostream &OS);
///
// ===--------------------------------------------------------------------=== //

#include "CodeGenTarget.h"
#include "llvm/Support/Format.h"
#include "llvm/TableGen/Error.h"
#include "llvm/TableGen/Record.h"
#include "llvm/TableGen/TableGenBackend.h"

using namespace llvm;

#define DEBUG_TYPE "SVMLVariants"
#include "llvm/Support/Debug.h"

namespace {

class SVMLVariantsEmitter {

  RecordKeeper &Records;
  unsigned MaxVL;

private:
  void emitSVMLVariants(raw_ostream &OS);

public:
  SVMLVariantsEmitter(RecordKeeper &R) : Records(R) {}

  // Output the directives and clauses to an enum and enum to string map.
  void run(raw_ostream &OS);
};
} // End anonymous namespace

/// \brief Emit the set of SVML variant function names.
void SVMLVariantsEmitter::emitSVMLVariants(raw_ostream &OS) {

  MaxVL = 32;

  Record *SvmlVariantClass = Records.getClass("SvmlVariant");
  assert(SvmlVariantClass &&
         "SvmlVariant class not found in target description file!");

  OS << "#ifdef GET_SVML_VARIANTS\n";

  for (const auto &S : Records.getDefs()) {
    if (S.second->isSubClassOf(SvmlVariantClass)) {
      std::string SvmlVariantNameStr = S.first;

      // Emit double precision variants.
      for (unsigned VL = 2; VL <= MaxVL; VL *= 2) {
        OS << "{\"" << SvmlVariantNameStr << "\", ";
        OS << "\"" << "__svml_" << SvmlVariantNameStr << VL << "\", "
           << VL << "},\n";
      }

      // Emit single precision variants.
      for (unsigned VL = 2; VL <= MaxVL; VL *= 2) {
        OS << "{\"" << SvmlVariantNameStr << "f" << "\", ";
        OS << "\"" << "__svml_" << SvmlVariantNameStr << "f" << VL << "\", "
           << VL << "},\n";
      }

      // Some functions can be in scalar intrinsic form, so a mapping between
      // the intrinsic and svml variants is needed.
      if (SvmlVariantNameStr == "pow") {
        for (unsigned VL = 2; VL <= MaxVL; VL *= 2) {
          OS << "{\"" << "llvm." << SvmlVariantNameStr << ".f64" << "\", ";
          OS << "\"" << "__svml_" << SvmlVariantNameStr << VL << "\", " << VL
             << "},\n";
        }
        for (unsigned VL = 2; VL <= MaxVL; VL *= 2) {
          OS << "{\"" << "llvm." << SvmlVariantNameStr << ".f32" << "\", ";
          OS << "\"" << "__svml_" << SvmlVariantNameStr << "f" << VL << "\", "
             << VL << "},\n";
        }
      }
    }
  }

  OS << "#endif // GET_SVML_VARIANTS\n\n";
}

void SVMLVariantsEmitter::run(raw_ostream &OS) {
  emitSVMLVariants(OS);
}

namespace llvm {

void EmitSVMLVariants(RecordKeeper &RK, raw_ostream &OS) {
  SVMLVariantsEmitter(RK).run(OS);
}

} // End llvm namespace
