//
//      Copyright (c) 2016-2017 Intel Corporation.
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
#include <vector>

using namespace llvm;

#define DEBUG_TYPE "SVMLVariants"

namespace {

class SVMLVariantsEmitter {

  RecordKeeper &Records;

private:
  void emitSVMLVariants(raw_ostream &OS);

public:
  SVMLVariantsEmitter(RecordKeeper &R) : Records(R) {}

  void run(raw_ostream &OS);
};
} // End anonymous namespace

/// \brief Emit the set of SVML variant function names.
void SVMLVariantsEmitter::emitSVMLVariants(raw_ostream &OS) {

  // largest logical vector length that is supported for svml translation.
  // Can increase if necessary.
  unsigned MaxVL = 64;

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

  Record *SvmlVariantsClass = Records.getClass("SvmlVariants");
  assert(SvmlVariantsClass &&
         "SvmlVariants class not found in target description file!");

  std::vector<Record*> SvmlVariants =
      Records.getAllDerivedDefinitions("SvmlVariants");

  OS << "#ifdef GET_SVML_VARIANTS\n";

  for (auto SvmlVariant : SvmlVariants) {
    std::vector<Record*> VList = SvmlVariant->getValueAsListOfDefs("VList");
    std::string SvmlVariantNameStr = SvmlVariant->getName();
    for (unsigned i = 0; i < VList.size(); i++) {
      bool isMasked = VList[i]->getValueAsBit("isMasked");
      bool hasSingle = VList[i]->getValueAsBit("hasSingle");
      bool hasDouble = VList[i]->getValueAsBit("hasDouble");
      bool hasIntrinsic = VList[i]->getValueAsBit("hasIntrinsic");
      if (hasSingle) {
        for (unsigned VL = MinSinglePrecVL; VL <= MaxSinglePrecVL; VL *= 2) {
          OS << "{\"" << SvmlVariantNameStr << "f" << "\", ";
          OS << "\"" << "__svml_" << SvmlVariantNameStr << "f" << VL << "\", "
             << VL << ", false},\n";
          if (isMasked) {
            OS << "{\"" << SvmlVariantNameStr << "f" << "\", ";
            OS << "\"" << "__svml_" << SvmlVariantNameStr << "f" << VL
               << "_mask" << "\", " << VL << ", true},\n";
          }
          if (hasIntrinsic) {
            OS << "{\"" << "llvm." << SvmlVariantNameStr << ".f32" << "\", ";
            OS << "\"" << "__svml_" << SvmlVariantNameStr << "f" << VL
               << "\", " << VL << ", false},\n";
            if (isMasked) {
              OS << "{\"" << "llvm." << SvmlVariantNameStr << ".f32"
                 << "\", ";
              OS << "\"" << "__svml_" << SvmlVariantNameStr << "f" << VL
                 << "_mask" << "\", " << VL << ", true},\n";
            }
          }
        }
      }
      if (hasDouble) {
        for (unsigned VL = MinDoublePrecVL; VL <= MaxDoublePrecVL; VL *= 2) {
          OS << "{\"" << SvmlVariantNameStr << "\", ";
          OS << "\"" << "__svml_" << SvmlVariantNameStr << VL << "\", "
             << VL << ", false},\n";
          if (isMasked) {
            OS << "{\"" << SvmlVariantNameStr << "\", ";
            OS << "\"" << "__svml_" << SvmlVariantNameStr << VL << "_mask"
               << "\", " << VL << ", true},\n";
          }
          if (hasIntrinsic) {
            OS << "{\"" << "llvm." << SvmlVariantNameStr << ".f64" << "\", ";
            OS << "\"" << "__svml_" << SvmlVariantNameStr << VL << "\", "
               << VL << ", false},\n";
            if (isMasked) {
              OS << "{\"" << "llvm." << SvmlVariantNameStr << ".f64"
                 << "\", ";
              OS << "\"" << "__svml_" << SvmlVariantNameStr << VL << "_mask"
                 << "\", " << VL << ", true},\n";
            }
          }
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
