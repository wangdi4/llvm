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

      Twine SvmlVariantNameDoublePrec = SvmlVariantNameStr;
      Twine SvmlVariantNameSinglePrec = Twine(SvmlVariantNameStr) + "f";

      // Emit double precision variants.
      for (unsigned VL = 2; VL <= MaxVL; VL *= 2) {
        Twine FullNameDoublePrec = "__svml_" + SvmlVariantNameDoublePrec +
                                   Twine(VL);
        OS << "{\"" << SvmlVariantNameDoublePrec << "\", ";
        OS << "\"" << FullNameDoublePrec << "\", " << VL << "},\n";
      }

      // Emit single precision variants.
      for (unsigned VL = 2; VL <= MaxVL; VL *= 2) {
        Twine FullNameSinglePrec = "__svml_" + SvmlVariantNameSinglePrec +
                                   Twine(VL);
        OS << "{\"" << SvmlVariantNameSinglePrec << "\", ";
        OS << "\"" << FullNameSinglePrec << "\", " << VL << "},\n";
      }

      // Some functions can be in scalar intrinsic form, so a mapping between
      // the intrinsic and svml variants is needed.
      if (SvmlVariantNameStr == "pow") {
        for (unsigned VL = 2; VL <= MaxVL; VL *= 2) {
          Twine IntrinsicName = "llvm." + Twine(SvmlVariantNameStr) + ".f64";
          OS << "{\"" << IntrinsicName << "\", ";
          Twine FullNameDoublePrec = "__svml_" + SvmlVariantNameDoublePrec +
                                     Twine(VL);
          OS << "\"" << FullNameDoublePrec << "\", " << VL << "},\n";
        }
        for (unsigned VL = 2; VL <= MaxVL; VL *= 2) {
          Twine IntrinsicName = "llvm." + Twine(SvmlVariantNameStr) + ".f32";
          OS << "{\"" << IntrinsicName << "\", ";
          Twine FullNameSinglePrec = "__svml_" + SvmlVariantNameSinglePrec +
                                     Twine(VL);
          OS << "\"" << FullNameSinglePrec << "\", " << VL << "},\n";
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
