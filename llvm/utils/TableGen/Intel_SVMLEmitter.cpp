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
#include <set>

using namespace llvm;

#define DEBUG_TYPE "SVMLVariants"
#include "llvm/Support/Debug.h"

namespace {

class SVMLVariantsEmitter {

  RecordKeeper &Records;

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

  // These math functions may appear in intrinsic form in LLVM IR and can
  // be translated to SVML.
  std::set<std::string> Intrinsics = { "pow", "exp", "log" };

  // largest logical vector length that is supported for svml translation.
  // Can increase if necessary.
  unsigned MaxVL = 32;

#if INTEL_CUSTOMIZATION
  unsigned MinSinglePrecVL = 2;
  unsigned MaxSinglePrecVL = MaxVL;
  unsigned MinDoublePrecVL = 2;
  unsigned MaxDoublePrecVL = MaxVL;
#else
  // Community version only allows direct translation to legal svml calls. E.g.,
  // there is no such function as __svml_sinf32 and calls such as this must be
  // legalized by breaking them into two __svml_sinf16, or four __svml_sinf8
  // calls, etc. The legalization code does not currently exist in LLVM trunk.
  // Thus, these min/max values for single/double precision svml variants
  // specify the ranges of vector lengths for which the loop vectorizer can
  // emit svml calls.
  unsigned MinSinglePrecVL = 4;
  unsigned MaxSinglePrecVL = 16;
  unsigned MinDoublePrecVL = 2;
  unsigned MaxDoublePrecVL = 8;
#endif // INTEL_CUSTOMIZATION

  Record *SvmlVariantClass = Records.getClass("SvmlVariant");
  assert(SvmlVariantClass &&
         "SvmlVariant class not found in target description file!");

  OS << "#ifdef GET_SVML_VARIANTS\n";

  for (const auto &S : Records.getDefs()) {
    if (S.second->isSubClassOf(SvmlVariantClass)) {
      std::string SvmlVariantNameStr = S.first;

      // Emit double precision variants.
      for (unsigned VL = MinDoublePrecVL; VL <= MaxDoublePrecVL; VL *= 2) {
        OS << "{\"" << SvmlVariantNameStr << "\", ";
        OS << "\"" << "__svml_" << SvmlVariantNameStr << VL << "\", "
           << VL << "},\n";
      }

      // Emit single precision variants.
      for (unsigned VL = MinSinglePrecVL; VL <= MaxSinglePrecVL; VL *= 2) {
        OS << "{\"" << SvmlVariantNameStr << "f" << "\", ";
        OS << "\"" << "__svml_" << SvmlVariantNameStr << "f" << VL << "\", "
           << VL << "},\n";
      }

      // Some functions can be in scalar intrinsic form, so a mapping between
      // the intrinsic and svml variants is needed.
      std::set<std::string>::iterator It = Intrinsics.find(SvmlVariantNameStr);
      if (It != Intrinsics.end()) {
        for (unsigned VL = MinDoublePrecVL; VL <= MaxDoublePrecVL; VL *= 2) {
          OS << "{\"" << "llvm." << SvmlVariantNameStr << ".f64" << "\", ";
          OS << "\"" << "__svml_" << SvmlVariantNameStr << VL << "\", " << VL
             << "},\n";
        }
        for (unsigned VL = MinSinglePrecVL; VL <= MaxSinglePrecVL; VL *= 2) {
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
