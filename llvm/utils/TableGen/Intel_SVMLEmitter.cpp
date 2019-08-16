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
  // Community version only allows direct translation to legal svml calls. E.g.,
  // there is no such function as __svml_sinf32 and calls such as this must be
  // legalized by breaking them into two __svml_sinf16, or four __svml_sinf8
  // calls, etc. The legalization code does not currently exist in LLVM trunk.
  // Thus, these min/max values for single/double precision svml variants
  // specify the ranges of vector lengths for which the loop vectorizer can
  // emit svml calls.
  unsigned MinSinglePrecVL = 2;
  unsigned MaxSinglePrecVL = MaxVL;
  unsigned MinDoublePrecVL = 2;
  unsigned MaxDoublePrecVL = MaxVL;
#else
  unsigned MinSinglePrecVL = 4;
  unsigned MaxSinglePrecVL = 16;
  unsigned MinDoublePrecVL = 2;
  unsigned MaxDoublePrecVL = 8;
#endif // INTEL_CUSTOMIZATION

  Record *SvmlVariantsClass = Records.getClass("SvmlVariants");
  (void) SvmlVariantsClass;
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

  OS << "{\"_Z5floorf\", \"_Z5floorDv4_f\", 4, false},\n";
  OS << "{\"_Z5floorf\", \"_Z5floorDv8_f\", 8, false},\n";
  OS << "{\"_Z5floorf\", \"_Z5floorDv16_f\", 16, false},\n";

  OS << "{\"_Z5floorf\", \"_Z5floorDv4_f\", 4, true},\n";
  OS << "{\"_Z5floorf\", \"_Z5floorDv8_f\", 8, true},\n";
  OS << "{\"_Z5floorf\", \"_Z5floorDv16_f\", 16, true},\n";

  OS << "{\"_Z5hypotff\", \"_Z5hypotDv4_fS_\", 4, false},\n";
  OS << "{\"_Z5hypotff\", \"_Z5hypotDv8_fS_\", 8, false},\n";
  OS << "{\"_Z5hypotff\", \"_Z5hypotDv16_fS_\", 16, false},\n";

  OS << "{\"_Z5hypotff\", \"_Z5hypotDv4_fS_\", 4, true},\n";
  OS << "{\"_Z5hypotff\", \"_Z5hypotDv8_fS_\", 8, true},\n";
  OS << "{\"_Z5hypotff\", \"_Z5hypotDv16_fS_\", 16, true},\n";

  OS << "{\"_Z5atan2ff\", \"_Z5atan2Dv4_fS_\", 4, false},\n";
  OS << "{\"_Z5atan2ff\", \"_Z5atan2Dv8_fS_\", 8, false},\n";
  OS << "{\"_Z5atan2ff\", \"_Z5atan2Dv16_fS_\", 16, false},\n";

  OS << "{\"_Z5atan2ff\", \"_Z5atan2Dv4_fS_\", 4, true},\n";
  OS << "{\"_Z5atan2ff\", \"_Z5atan2Dv8_fS_\", 8, true},\n";
  OS << "{\"_Z5atan2ff\", \"_Z5atan2Dv16_fS_\", 16, true},\n";

  OS << "{\"_Z15convert_int_rtef\", \"_Z16convert_int4_rteDv4_f\", 4, false},\n";
  OS << "{\"_Z15convert_int_rtef\", \"_Z16convert_int8_rteDv8_f\", 8, false},\n";
  OS << "{\"_Z15convert_int_rtef\", \"_Z17convert_int16_rteDv16_f\", 16, false},\n";

  OS << "{\"_Z15convert_int_rtef\", \"_Z16convert_int4_rteDv4_f\", 4, true},\n";
  OS << "{\"_Z15convert_int_rtef\", \"_Z16convert_int8_rteDv8_f\", 8, true},\n";
  OS << "{\"_Z15convert_int_rtef\", \"_Z17convert_int16_rteDv16_f\", 16, true},\n";

  OS << "{\"_Z5clampiii\", \"_Z5clampDv4_iS_S_\", 4, false},\n";
  OS << "{\"_Z5clampiii\", \"_Z5clampDv8_iS_S_\", 8, false},\n";
  OS << "{\"_Z5clampiii\", \"_Z5clampDv16_iS_S_\", 16, false},\n";

  OS << "{\"_Z5clampiii\", \"_Z5clampDv4_iS_S_\", 4, true},\n";
  OS << "{\"_Z5clampiii\", \"_Z5clampDv8_iS_S_\", 8, true},\n";
  OS << "{\"_Z5clampiii\", \"_Z5clampDv16_iS_S_\", 16, true},\n";

  OS << "{\"_Z5clampfff\", \"_Z5clampDv4_fS_S_\", 4, false},\n";
  OS << "{\"_Z5clampfff\", \"_Z5clampDv8_fS_S_\", 8, false},\n";
  OS << "{\"_Z5clampfff\", \"_Z5clampDv16_fS_S_\", 16, false},\n";

  OS << "{\"_Z5clampfff\", \"_Z5clampDv4_fS_S_\", 4, true},\n";
  OS << "{\"_Z5clampfff\", \"_Z5clampDv8_fS_S_\", 8, true},\n";
  OS << "{\"_Z5clampfff\", \"_Z5clampDv16_fS_S_\", 16, true},\n";

  OS << "{\"_Z6selectffi\", \"_Z6selectDv4_fS_Dv4_i\", 4, false},\n";
  OS << "{\"_Z6selectffi\", \"_Z6selectDv8_fS_Dv8_i\", 8, false},\n";
  OS << "{\"_Z6selectffi\", \"_Z6selectDv16_fS_Dv16_i\", 16, false},\n";

  OS << "{\"_Z6selectffi\", \"_Z6selectDv4_fS_Dv4_i\", 4, true},\n";
  OS << "{\"_Z6selectffi\", \"_Z6selectDv8_fS_Dv8_i\", 8, true},\n";
  OS << "{\"_Z6selectffi\", \"_Z6selectDv16_fS_Dv16_i\", 16, true},\n";

  OS << "{\"_Z10native_sinf\", \"_Z10native_sinDv4_f\", 4, false},\n";
  OS << "{\"_Z10native_sinf\", \"_Z10native_sinDv8_f\", 8, false},\n";
  OS << "{\"_Z10native_sinf\", \"_Z10native_sinDv16_f\", 16, false},\n";

  OS << "{\"_Z10native_sinf\", \"_Z10native_sinDv4_f\", 4, true},\n";
  OS << "{\"_Z10native_sinf\", \"_Z10native_sinDv8_f\", 8, true},\n";
  OS << "{\"_Z10native_sinf\", \"_Z10native_sinDv16_f\", 16, true},\n";

  OS << "{\"_Z10native_cosf\", \"_Z10native_cosDv4_f\", 4, false},\n";
  OS << "{\"_Z10native_cosf\", \"_Z10native_cosDv8_f\", 8, false},\n";
  OS << "{\"_Z10native_cosf\", \"_Z10native_cosDv16_f\", 16, false},\n";

  OS << "{\"_Z10native_cosf\", \"_Z10native_cosDv4_f\", 4, true},\n";
  OS << "{\"_Z10native_cosf\", \"_Z10native_cosDv8_f\", 8, true},\n";
  OS << "{\"_Z10native_cosf\", \"_Z10native_cosDv16_f\", 16, true},\n";

  OS << "{\"_Z6sincosfPf\", \"_Z14sincos_ret2ptrDv2_fPS_S1_\", 2, false},\n";
  OS << "{\"_Z6sincosfPf\", \"_Z14sincos_ret2ptrDv3_fPS_S1_\", 3, false},\n";
  OS << "{\"_Z6sincosfPf\", \"_Z14sincos_ret2ptrDv4_fPS_S1_\", 4, false},\n";
  OS << "{\"_Z6sincosfPf\", \"_Z14sincos_ret2ptrDv8_fPS_S1_\", 8, false},\n";
  OS << "{\"_Z6sincosfPf\", \"_Z14sincos_ret2ptrDv16_fPS_S1_\", 16, false},\n";

  OS << "{\"_Z3minii\", \"_Z3minDv4_iS_\", 4, false},\n";
  OS << "{\"_Z3minii\", \"_Z3minDv8_iS_\", 8, false},\n";
  OS << "{\"_Z3minii\", \"_Z3minDv16_iS_\", 16, false},\n";

  OS << "{\"_Z3minii\", \"_Z3minDv4_iS_\", 4, true},\n";
  OS << "{\"_Z3minii\", \"_Z3minDv8_iS_\", 8, true},\n";
  OS << "{\"_Z3minii\", \"_Z3minDv16_iS_\", 16, true},\n";

  OS << "{\"_Z3minff\", \"_Z3minDv4_fS_\", 4, false},\n";
  OS << "{\"_Z3minff\", \"_Z3minDv8_fS_\", 8, false},\n";
  OS << "{\"_Z3minff\", \"_Z3minDv16_fS_\", 16, false},\n";

  OS << "{\"_Z3minff\", \"_Z3minDv4_fS_\", 4, true},\n";
  OS << "{\"_Z3minff\", \"_Z3minDv8_fS_\", 8, true},\n";
  OS << "{\"_Z3minff\", \"_Z3minDv16_fS_\", 16, true},\n";

  OS << "{\"_Z3maxii\", \"_Z3maxDv4_iS_\", 4, false},\n";
  OS << "{\"_Z3maxii\", \"_Z3maxDv8_iS_\", 8, false},\n";
  OS << "{\"_Z3maxii\", \"_Z3maxDv16_iS_\", 16, false},\n";

  OS << "{\"_Z3maxii\", \"_Z3maxDv4_iS_\", 4, true},\n";
  OS << "{\"_Z3maxii\", \"_Z3maxDv8_iS_\", 8, true},\n";
  OS << "{\"_Z3maxii\", \"_Z3maxDv16_iS_\", 16, true},\n";

  OS << "{\"_Z3maxff\", \"_Z3maxDv4_fS_\", 4, false},\n";
  OS << "{\"_Z3maxff\", \"_Z3maxDv8_fS_\", 8, false},\n";
  OS << "{\"_Z3maxff\", \"_Z3maxDv16_fS_\", 16, false},\n";

  OS << "{\"_Z3maxff\", \"_Z3maxDv4_fS_\", 4, true},\n";
  OS << "{\"_Z3maxff\", \"_Z3maxDv8_fS_\", 8, true},\n";
  OS << "{\"_Z3maxff\", \"_Z3maxDv16_fS_\", 16, true},\n";

  OS << "{\"_Z4fmaxff\", \"_Z4fmaxDv2_fS_\", 2, false},\n";
  OS << "{\"_Z4fmaxff\", \"_Z4fmaxDv3_fS_\", 3, false},\n";
  OS << "{\"_Z4fmaxff\", \"_Z4fmaxDv4_fS_\", 4, false},\n";
  OS << "{\"_Z4fmaxff\", \"_Z4fmaxDv8_fS_\", 8, false},\n";
  OS << "{\"_Z4fmaxff\", \"_Z4fmaxDv16_fS_\", 16, false},\n";

  OS << "{\"_Z3logf\", \"_Z3logDv2_f\", 2, false},\n";
  OS << "{\"_Z3logf\", \"_Z3logDv3_f\", 3, false},\n";
  OS << "{\"_Z3logf\", \"_Z3logDv4_f\", 4, false},\n";
  OS << "{\"_Z3logf\", \"_Z3logDv8_f\", 8, false},\n";
  OS << "{\"_Z3logf\", \"_Z3logDv16_f\", 16, false},\n";

  OS << "{\"_Z3expf\", \"_Z3expDv2_f\", 2, false},\n";
  OS << "{\"_Z3expf\", \"_Z3expDv3_f\", 3, false},\n";
  OS << "{\"_Z3expf\", \"_Z3expDv4_f\", 4, false},\n";
  OS << "{\"_Z3expf\", \"_Z3expDv8_f\", 8, false},\n";
  OS << "{\"_Z3expf\", \"_Z3expDv16_f\", 16, false},\n";

  OS << "{\"_Z3expf\", \"_Z3expDv2_f\", 2, true},\n";
  OS << "{\"_Z3expf\", \"_Z3expDv3_f\", 3, true},\n";
  OS << "{\"_Z3expf\", \"_Z3expDv4_f\", 4, true},\n";
  OS << "{\"_Z3expf\", \"_Z3expDv8_f\", 8, true},\n";
  OS << "{\"_Z3expf\", \"_Z3expDv16_f\", 16, true},\n";

  OS << "{\"_Z3expd\", \"_Z3expDv2_d\", 2, false},\n";
  OS << "{\"_Z3expd\", \"_Z3expDv3_d\", 3, false},\n";
  OS << "{\"_Z3expd\", \"_Z3expDv4_d\", 4, false},\n";
  OS << "{\"_Z3expd\", \"_Z3expDv8_d\", 8, false},\n";
  OS << "{\"_Z3expd\", \"_Z3expDv16_d\", 16, false},\n";

  OS << "{\"_Z3expd\", \"_Z3expDv2_d\", 2, true},\n";
  OS << "{\"_Z3expd\", \"_Z3expDv3_d\", 3, true},\n";
  OS << "{\"_Z3expd\", \"_Z3expDv4_d\", 4, true},\n";
  OS << "{\"_Z3expd\", \"_Z3expDv8_d\", 8, true},\n";
  OS << "{\"_Z3expd\", \"_Z3expDv16_d\", 16, true},\n";

  OS << "{\"_Z10native_expf\", \"_Z10native_expDv2_f\", 2, false},\n";
  OS << "{\"_Z10native_expf\", \"_Z10native_expDv3_f\", 3, false},\n";
  OS << "{\"_Z10native_expf\", \"_Z10native_expDv4_f\", 4, false},\n";
  OS << "{\"_Z10native_expf\", \"_Z10native_expDv8_f\", 8, false},\n";
  OS << "{\"_Z10native_expf\", \"_Z10native_expDv16_f\", 16, false},\n";

  OS << "{\"_Z10native_expf\", \"_Z10native_expDv2_f\", 2, true},\n";
  OS << "{\"_Z10native_expf\", \"_Z10native_expDv3_f\", 3, true},\n";
  OS << "{\"_Z10native_expf\", \"_Z10native_expDv4_f\", 4, true},\n";
  OS << "{\"_Z10native_expf\", \"_Z10native_expDv8_f\", 8, true},\n";
  OS << "{\"_Z10native_expf\", \"_Z10native_expDv16_f\", 16, true},\n";

  OS << "{\"_Z4sqrtf\", \"_Z4sqrtDv2_f\", 2, false},\n";
  OS << "{\"_Z4sqrtf\", \"_Z4sqrtDv3_f\", 3, false},\n";
  OS << "{\"_Z4sqrtf\", \"_Z4sqrtDv4_f\", 4, false},\n";
  OS << "{\"_Z4sqrtf\", \"_Z4sqrtDv8_f\", 8, false},\n";
  OS << "{\"_Z4sqrtf\", \"_Z4sqrtDv16_f\", 16, false},\n";

  OS << "{\"_Z4sqrtf\", \"_Z4sqrtDv2_f\", 2, true},\n";
  OS << "{\"_Z4sqrtf\", \"_Z4sqrtDv3_f\", 3, true},\n";
  OS << "{\"_Z4sqrtf\", \"_Z4sqrtDv4_f\", 4, true},\n";
  OS << "{\"_Z4sqrtf\", \"_Z4sqrtDv8_f\", 8, true},\n";
  OS << "{\"_Z4sqrtf\", \"_Z4sqrtDv16_f\", 16, true},\n";

  OS << "{\"_Z4sqrtd\", \"_Z4sqrtDv2_d\", 2, false},\n";
  OS << "{\"_Z4sqrtd\", \"_Z4sqrtDv3_d\", 3, false},\n";
  OS << "{\"_Z4sqrtd\", \"_Z4sqrtDv4_d\", 4, false},\n";
  OS << "{\"_Z4sqrtd\", \"_Z4sqrtDv8_d\", 8, false},\n";
  OS << "{\"_Z4sqrtd\", \"_Z4sqrtDv16_d\", 16, false},\n";

  OS << "{\"_Z4sqrtd\", \"_Z4sqrtDv2_d\", 2, true},\n";
  OS << "{\"_Z4sqrtd\", \"_Z4sqrtDv3_d\", 3, true},\n";
  OS << "{\"_Z4sqrtd\", \"_Z4sqrtDv4_d\", 4, true},\n";
  OS << "{\"_Z4sqrtd\", \"_Z4sqrtDv8_d\", 8, true},\n";
  OS << "{\"_Z4sqrtd\", \"_Z4sqrtDv16_d\", 16, true},\n";

  OS << "{\"_Z3madfff\", \"_Z3madDv2_fS_S_\", 2, false},\n";
  OS << "{\"_Z3madfff\", \"_Z3madDv3_fS_S_\", 3, false},\n";
  OS << "{\"_Z3madfff\", \"_Z3madDv4_fS_S_\", 4, false},\n";
  OS << "{\"_Z3madfff\", \"_Z3madDv8_fS_S_\", 8, false},\n";
  OS << "{\"_Z3madfff\", \"_Z3madDv16_fS_S_\", 16, false},\n";

  OS << "{\"_Z4udivjj\", \"_Z4udivDv2_jS_\", 2, false},\n";
  OS << "{\"_Z4udivjj\", \"_Z4udivDv3_jS_\", 3, false},\n";
  OS << "{\"_Z4udivjj\", \"_Z4udivDv4_jS_\", 4, false},\n";
  OS << "{\"_Z4udivjj\", \"_Z4udivDv8_jS_\", 8, false},\n";
  OS << "{\"_Z4udivjj\", \"_Z4udivDv16_jS_\", 16, false},\n";

  OS << "{\"_Z4udivjj\", \"_Z4udivDv2_jS_\", 2, true},\n";
  OS << "{\"_Z4udivjj\", \"_Z4udivDv3_jS_\", 3, true},\n";
  OS << "{\"_Z4udivjj\", \"_Z4udivDv4_jS_\", 4, true},\n";
  OS << "{\"_Z4udivjj\", \"_Z4udivDv8_jS_\", 8, true},\n";
  OS << "{\"_Z4udivjj\", \"_Z4udivDv16_jS_\", 16, true},\n";

  OS << "{\"_Z4idivii\", \"_Z4idivDv2_iS_\", 2, false},\n";
  OS << "{\"_Z4idivii\", \"_Z4idivDv3_iS_\", 3, false},\n";
  OS << "{\"_Z4idivii\", \"_Z4idivDv4_iS_\", 4, false},\n";
  OS << "{\"_Z4idivii\", \"_Z4idivDv8_iS_\", 8, false},\n";
  OS << "{\"_Z4idivii\", \"_Z4idivDv16_iS_\", 16, false},\n";

  OS << "{\"_Z4idivii\", \"_Z4idivDv2_iS_\", 2, true},\n";
  OS << "{\"_Z4idivii\", \"_Z4idivDv3_iS_\", 3, true},\n";
  OS << "{\"_Z4idivii\", \"_Z4idivDv4_iS_\", 4, true},\n";
  OS << "{\"_Z4idivii\", \"_Z4idivDv8_iS_\", 8, true},\n";
  OS << "{\"_Z4idivii\", \"_Z4idivDv16_iS_\", 16, true},\n";

  OS << "{\"_Z4uremjj\", \"_Z4uremDv2_jS_\", 2, false},\n";
  OS << "{\"_Z4uremjj\", \"_Z4uremDv2_jS_\", 3, false},\n";
  OS << "{\"_Z4uremjj\", \"_Z4uremDv4_jS_\", 4, false},\n";
  OS << "{\"_Z4uremjj\", \"_Z4uremDv8_jS_\", 8, false},\n";
  OS << "{\"_Z4uremjj\", \"_Z4uremDv16_jS_\", 16, false},\n";

  OS << "{\"_Z4uremjj\", \"_Z4uremDv2_jS_\", 2, true},\n";
  OS << "{\"_Z4uremjj\", \"_Z4uremDv2_jS_\", 3, true},\n";
  OS << "{\"_Z4uremjj\", \"_Z4uremDv4_jS_\", 4, true},\n";
  OS << "{\"_Z4uremjj\", \"_Z4uremDv8_jS_\", 8, true},\n";
  OS << "{\"_Z4uremjj\", \"_Z4uremDv16_jS_\", 16, true},\n";

  OS << "{\"_Z4iremii\", \"_Z4iremDv2_iS_\", 2, false},\n";
  OS << "{\"_Z4iremii\", \"_Z4iremDv3_iS_\", 3, false},\n";
  OS << "{\"_Z4iremii\", \"_Z4iremDv4_iS_\", 4, false},\n";
  OS << "{\"_Z4iremii\", \"_Z4iremDv8_iS_\", 8, false},\n";
  OS << "{\"_Z4iremii\", \"_Z4iremDv16_iS_\", 16, false},\n";

  OS << "{\"_Z4iremii\", \"_Z4iremDv2_iS_\", 2, true},\n";
  OS << "{\"_Z4iremii\", \"_Z4iremDv3_iS_\", 3, true},\n";
  OS << "{\"_Z4iremii\", \"_Z4iremDv4_iS_\", 4, true},\n";
  OS << "{\"_Z4iremii\", \"_Z4iremDv8_iS_\", 8, true},\n";
  OS << "{\"_Z4iremii\", \"_Z4iremDv16_iS_\", 16, true},\n";

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
