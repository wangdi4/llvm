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
///      void EmitSVMLVariants(RecordKeeper &RK, raw_ostream &OS, bool
///      IsDevice);
///
// ===--------------------------------------------------------------------=== //

#include "llvm/ADT/StringSet.h"
#include "llvm/Support/Format.h"
#include "llvm/TableGen/Error.h"
#include "llvm/TableGen/Record.h"
#include "llvm/TableGen/TableGenBackend.h"
#include <sstream>
#include <vector>

using namespace llvm;

#define DEBUG_TYPE "SVMLVariants"

namespace {

class SVMLVariantsEmitter {

  RecordKeeper &Records;

private:
  void emitSVMLVariants(raw_ostream &OS);
  void emitSVMLDeviceVariants(raw_ostream &OS);

public:
  SVMLVariantsEmitter(RecordKeeper &R) : Records(R) {}

  void run(raw_ostream &OS, bool IsDevice);
};
} // End anonymous namespace

/// \brief Calculates the integer value representing the BitsInit object.
static inline uint64_t getValueFromBitsInit(const BitsInit *B) {
  assert(B->getNumBits() <= sizeof(uint64_t) * 8 && "BitInits' too long!");

  uint64_t Value = 0;
  for (unsigned i = 0, e = B->getNumBits(); i != e; ++i) {
    BitInit *Bit = cast<BitInit>(B->getBit(i));
    Value |= uint64_t(Bit->getValue()) << i;
  }
  return Value;
}

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
  unsigned MinSinglePrecVL = 1;
  unsigned MaxSinglePrecVL = MaxVL;
  unsigned MinDoublePrecVL = 1;
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
  OS << "#define FIXED(NL) ElementCount::getFixed(NL)\n";

  for (auto SvmlVariant : SvmlVariants) {
    std::vector<Record*> VList = SvmlVariant->getValueAsListOfDefs("VList");
    std::string SvmlVariantNameStr = std::string(SvmlVariant->getName());
    int64_t NumArgs = SvmlVariant->getValueAsInt("NumArgs");
    for (unsigned i = 0; i < VList.size(); i++) {
      bool isMasked = VList[i]->getValueAsBit("isMasked");
      bool hasSingle = VList[i]->getValueAsBit("hasSingle");
      bool hasDouble = VList[i]->getValueAsBit("hasDouble");
      bool hasIntrinsic = VList[i]->getValueAsBit("hasIntrinsic");
      BitsInit *extraAttrs = VList[i]->getValueAsBitsInit("extraAttrs");
      uint64_t extraAttrsInt = getValueFromBitsInit(extraAttrs);
      std::string VFABIPrefixInit = "\"_ZGV";
      VFABIPrefixInit += "_LLVM_"; // ISA is identified by VFABI::_LLVM_
      // Encode number of args for the vector function into vector function ABI
      // prefix.
      std::string VFABIPrefixArgs = "";
      for (unsigned I = 0; I < NumArgs; ++I)
        VFABIPrefixArgs += "v";
      VFABIPrefixArgs += "\"";

      if (hasSingle) {
        for (unsigned VL = MinSinglePrecVL; VL <= MaxSinglePrecVL; VL *= 2) {
          // Generate vector ABI prefix for both masked and unmasked versions.
          std::string VFABIPrefixUnmasked =
              VFABIPrefixInit + "N" + std::to_string(VL) + VFABIPrefixArgs;
          std::string VFABIPrefixMasked =
              VFABIPrefixInit + "M" + std::to_string(VL) + VFABIPrefixArgs;
          OS << "{\"" << SvmlVariantNameStr << "f" << "\", ";
          OS << "\""
             << "__svml_" << SvmlVariantNameStr << "f" << VL << "\", "
             << "FIXED(" << VL << ")"
             << ", false, " << VFABIPrefixUnmasked << "},\n";
          if (isMasked) {
            OS << "{\"" << SvmlVariantNameStr << "f" << "\", ";
            OS << "\""
               << "__svml_" << SvmlVariantNameStr << "f" << VL << "_mask"
               << "\", "
               << "FIXED(" << VL << ")"
               << ", true, " << VFABIPrefixMasked << "},\n";
          }
          if (hasIntrinsic) {
            OS << "{\"" << "llvm." << SvmlVariantNameStr << ".f32" << "\", ";
            OS << "\""
               << "__svml_" << SvmlVariantNameStr << "f" << VL << "\", "
               << "FIXED(" << VL << ")"
               << ", false, " << VFABIPrefixUnmasked << "},\n";
            if (isMasked) {
              OS << "{\"" << "llvm." << SvmlVariantNameStr << ".f32"
                 << "\", ";
              OS << "\""
                 << "__svml_" << SvmlVariantNameStr << "f" << VL << "_mask"
                 << "\", "
                 << "FIXED(" << VL << ")"
                 << ", true, " << VFABIPrefixMasked << "},\n";
            }
          }
        }
      }
      if (hasDouble) {
        for (unsigned VL = MinDoublePrecVL; VL <= MaxDoublePrecVL; VL *= 2) {
          // Generate vector ABI prefix for both masked and unmasked versions.
          std::string VFABIPrefixUnmasked =
              VFABIPrefixInit + "N" + std::to_string(VL) + VFABIPrefixArgs;
          std::string VFABIPrefixMasked =
              VFABIPrefixInit + "M" + std::to_string(VL) + VFABIPrefixArgs;
          // For now, we are interested in the extra attributes only for double
          // precision non-intrinsic versions of library calls.
          OS << "{\"" << SvmlVariantNameStr << "\", ";
          OS << "\""
             << "__svml_" << SvmlVariantNameStr << VL << "\", "
             << "FIXED(" << VL << ")"
             << ", false, " << VFABIPrefixUnmasked << ", " << extraAttrsInt
             << "},\n";
          if (isMasked) {
            OS << "{\"" << SvmlVariantNameStr << "\", ";
            OS << "\""
               << "__svml_" << SvmlVariantNameStr << VL << "_mask"
               << "\", "
               << "FIXED(" << VL << ")"
               << ", true, " << VFABIPrefixMasked << ", " << extraAttrsInt
               << "},\n";
          }
          if (hasIntrinsic) {
            OS << "{\"" << "llvm." << SvmlVariantNameStr << ".f64" << "\", ";
            OS << "\""
               << "__svml_" << SvmlVariantNameStr << VL << "\", "
               << "FIXED(" << VL << ")"
               << ", false, " << VFABIPrefixUnmasked << "},\n";
            if (isMasked) {
              OS << "{\"" << "llvm." << SvmlVariantNameStr << ".f64"
                 << "\", ";
              OS << "\""
                 << "__svml_" << SvmlVariantNameStr << VL << "_mask"
                 << "\", "
                 << "FIXED(" << VL << ")"
                 << ", true, " << VFABIPrefixMasked << "},\n";
            }
          }
        }
      }
    }
  }

  // Because ldexp has two arguments, the names of its intrinsics doesn't follow
  // the same pattern as the other functions. Thus the logic for intrinsics
  // above can't be reused. We just emit their mappings separately.
  OS << "{\"llvm.ldexp.f32.i32\", \"__svml_ldexpf1\", FIXED(1), false, \"_ZGV_LLVM_N1vv\"},\n";
  OS << "{\"llvm.ldexp.f32.i32\", \"__svml_ldexpf1_mask\", FIXED(1), true, \"_ZGV_LLVM_M1vv\"},\n";
  OS << "{\"llvm.ldexp.f32.i32\", \"__svml_ldexpf2\", FIXED(2), false, \"_ZGV_LLVM_N2vv\"},\n";
  OS << "{\"llvm.ldexp.f32.i32\", \"__svml_ldexpf2_mask\", FIXED(2), true, \"_ZGV_LLVM_M2vv\"},\n";
  OS << "{\"llvm.ldexp.f32.i32\", \"__svml_ldexpf4\", FIXED(4), false, \"_ZGV_LLVM_N4vv\"},\n";
  OS << "{\"llvm.ldexp.f32.i32\", \"__svml_ldexpf4_mask\", FIXED(4), true, \"_ZGV_LLVM_M4vv\"},\n";
  OS << "{\"llvm.ldexp.f32.i32\", \"__svml_ldexpf8\", FIXED(8), false, \"_ZGV_LLVM_N8vv\"},\n";
  OS << "{\"llvm.ldexp.f32.i32\", \"__svml_ldexpf8_mask\", FIXED(8), true, \"_ZGV_LLVM_M8vv\"},\n";
  OS << "{\"llvm.ldexp.f32.i32\", \"__svml_ldexpf16\", FIXED(16), false, \"_ZGV_LLVM_N16vv\"},\n";
  OS << "{\"llvm.ldexp.f32.i32\", \"__svml_ldexpf16_mask\", FIXED(16), true, \"_ZGV_LLVM_M16vv\"},\n";
  OS << "{\"llvm.ldexp.f32.i32\", \"__svml_ldexpf32\", FIXED(32), false, \"_ZGV_LLVM_N32vv\"},\n";
  OS << "{\"llvm.ldexp.f32.i32\", \"__svml_ldexpf32_mask\", FIXED(32), true, \"_ZGV_LLVM_M32vv\"},\n";
  OS << "{\"llvm.ldexp.f32.i32\", \"__svml_ldexpf64\", FIXED(64), false, \"_ZGV_LLVM_N64vv\"},\n";
  OS << "{\"llvm.ldexp.f32.i32\", \"__svml_ldexpf64_mask\", FIXED(64), true, \"_ZGV_LLVM_M64vv\"},\n";
  OS << "{\"llvm.ldexp.f64.i32\", \"__svml_ldexp1\", FIXED(1), false, \"_ZGV_LLVM_N1vv\"},\n";
  OS << "{\"llvm.ldexp.f64.i32\", \"__svml_ldexp1_mask\", FIXED(1), true, \"_ZGV_LLVM_M1vv\"},\n";
  OS << "{\"llvm.ldexp.f64.i32\", \"__svml_ldexp2\", FIXED(2), false, \"_ZGV_LLVM_N2vv\"},\n";
  OS << "{\"llvm.ldexp.f64.i32\", \"__svml_ldexp2_mask\", FIXED(2), true, \"_ZGV_LLVM_M2vv\"},\n";
  OS << "{\"llvm.ldexp.f64.i32\", \"__svml_ldexp4\", FIXED(4), false, \"_ZGV_LLVM_N4vv\"},\n";
  OS << "{\"llvm.ldexp.f64.i32\", \"__svml_ldexp4_mask\", FIXED(4), true, \"_ZGV_LLVM_M4vv\"},\n";
  OS << "{\"llvm.ldexp.f64.i32\", \"__svml_ldexp8\", FIXED(8), false, \"_ZGV_LLVM_N8vv\"},\n";
  OS << "{\"llvm.ldexp.f64.i32\", \"__svml_ldexp8_mask\", FIXED(8), true, \"_ZGV_LLVM_M8vv\"},\n";
  OS << "{\"llvm.ldexp.f64.i32\", \"__svml_ldexp16\", FIXED(16), false, \"_ZGV_LLVM_N16vv\"},\n";
  OS << "{\"llvm.ldexp.f64.i32\", \"__svml_ldexp16_mask\", FIXED(16), true, \"_ZGV_LLVM_M16vv\"},\n";
  OS << "{\"llvm.ldexp.f64.i32\", \"__svml_ldexp32\", FIXED(32), false, \"_ZGV_LLVM_N32vv\"},\n";
  OS << "{\"llvm.ldexp.f64.i32\", \"__svml_ldexp32_mask\", FIXED(32), true, \"_ZGV_LLVM_M32vv\"},\n";
  OS << "{\"llvm.ldexp.f64.i32\", \"__svml_ldexp64\", FIXED(64), false, \"_ZGV_LLVM_N64vv\"},\n";
  OS << "{\"llvm.ldexp.f64.i32\", \"__svml_ldexp64_mask\", FIXED(64), true, \"_ZGV_LLVM_M64vv\"},\n";

  // TODO: Should we use VecDescAttrs::IsOCLFn to set this?
  unsigned IsOCLFnAttr = 1;
  unsigned IsFortranOnlyAttr = 4;
  OS << "{\"_Z5floorf\", \"_Z5floorDv4_f\", FIXED(4), false, \"_ZGV_LLVM_N4v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z5floorf\", \"_Z5floorDv8_f\", FIXED(8), false, \"_ZGV_LLVM_N8v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z5floorf\", \"_Z5floorDv16_f\", FIXED(16), false, \"_ZGV_LLVM_N16v\", " << IsOCLFnAttr << "},\n";

  OS << "{\"_Z5floorf\", \"_Z5floorDv4_f\", FIXED(4), true, \"_ZGV_LLVM_M4v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z5floorf\", \"_Z5floorDv8_f\", FIXED(8), true, \"_ZGV_LLVM_M8v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z5floorf\", \"_Z5floorDv16_f\", FIXED(16), true, \"_ZGV_LLVM_M16v\", " << IsOCLFnAttr << "},\n";

  OS << "{\"_Z5hypotff\", \"_Z5hypotDv4_fS_\", FIXED(4), false, \"_ZGV_LLVM_N4v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z5hypotff\", \"_Z5hypotDv8_fS_\", FIXED(8), false, \"_ZGV_LLVM_N8v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z5hypotff\", \"_Z5hypotDv16_fS_\", FIXED(16), false, \"_ZGV_LLVM_N16v\", " << IsOCLFnAttr << "},\n";

  OS << "{\"_Z5hypotff\", \"_Z5hypotDv4_fS_\", FIXED(4), true, \"_ZGV_LLVM_M4v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z5hypotff\", \"_Z5hypotDv8_fS_\", FIXED(8), true, \"_ZGV_LLVM_M8v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z5hypotff\", \"_Z5hypotDv16_fS_\", FIXED(16), true, \"_ZGV_LLVM_M16v\", " << IsOCLFnAttr << "},\n";

  OS << "{\"_Z5atan2ff\", \"_Z5atan2Dv4_fS_\", FIXED(4), false, \"_ZGV_LLVM_N4v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z5atan2ff\", \"_Z5atan2Dv8_fS_\", FIXED(8), false, \"_ZGV_LLVM_N8v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z5atan2ff\", \"_Z5atan2Dv16_fS_\", FIXED(16), false, \"_ZGV_LLVM_N16v\", " << IsOCLFnAttr << "},\n";

  OS << "{\"_Z5atan2ff\", \"_Z5atan2Dv4_fS_\", FIXED(4), true, \"_ZGV_LLVM_M4v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z5atan2ff\", \"_Z5atan2Dv8_fS_\", FIXED(8), true, \"_ZGV_LLVM_M8v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z5atan2ff\", \"_Z5atan2Dv16_fS_\", FIXED(16), true, \"_ZGV_LLVM_M16v\", " << IsOCLFnAttr << "},\n";

  OS << "{\"_Z15convert_int_rtef\", \"_Z16convert_int4_rteDv4_f\", FIXED(4), false, \"_ZGV_LLVM_N4v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z15convert_int_rtef\", \"_Z16convert_int8_rteDv8_f\", FIXED(8), false, \"_ZGV_LLVM_N8v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z15convert_int_rtef\", \"_Z17convert_int16_rteDv16_f\", FIXED(16), false, \"_ZGV_LLVM_N16v\", " << IsOCLFnAttr << "},\n";

  OS << "{\"_Z15convert_int_rtef\", \"_Z16convert_int4_rteDv4_f\", FIXED(4), true, \"_ZGV_LLVM_M4v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z15convert_int_rtef\", \"_Z16convert_int8_rteDv8_f\", FIXED(8), true, \"_ZGV_LLVM_M8v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z15convert_int_rtef\", \"_Z17convert_int16_rteDv16_f\", FIXED(16), true, \"_ZGV_LLVM_M16v\", " << IsOCLFnAttr << "},\n";

  OS << "{\"_Z5clampiii\", \"_Z5clampDv4_iS_S_\", FIXED(4), false, \"_ZGV_LLVM_N4v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z5clampiii\", \"_Z5clampDv8_iS_S_\", FIXED(8), false, \"_ZGV_LLVM_N8v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z5clampiii\", \"_Z5clampDv16_iS_S_\", FIXED(16), false, \"_ZGV_LLVM_N16v\", " << IsOCLFnAttr << "},\n";

  OS << "{\"_Z5clampiii\", \"_Z5clampDv4_iS_S_\", FIXED(4), true, \"_ZGV_LLVM_M4v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z5clampiii\", \"_Z5clampDv8_iS_S_\", FIXED(8), true, \"_ZGV_LLVM_M8v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z5clampiii\", \"_Z5clampDv16_iS_S_\", FIXED(16), true, \"_ZGV_LLVM_M16v\", " << IsOCLFnAttr << "},\n";

  OS << "{\"_Z5clampfff\", \"_Z5clampDv4_fS_S_\", FIXED(4), false, \"_ZGV_LLVM_N4v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z5clampfff\", \"_Z5clampDv8_fS_S_\", FIXED(8), false, \"_ZGV_LLVM_N8v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z5clampfff\", \"_Z5clampDv16_fS_S_\", FIXED(16), false, \"_ZGV_LLVM_N16v\", " << IsOCLFnAttr << "},\n";

  OS << "{\"_Z5clampfff\", \"_Z5clampDv4_fS_S_\", FIXED(4), true, \"_ZGV_LLVM_M4v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z5clampfff\", \"_Z5clampDv8_fS_S_\", FIXED(8), true, \"_ZGV_LLVM_M8v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z5clampfff\", \"_Z5clampDv16_fS_S_\", FIXED(16), true, \"_ZGV_LLVM_M16v\", " << IsOCLFnAttr << "},\n";

  OS << "{\"_Z6selectffi\", \"_Z6selectDv4_fS_Dv4_i\", FIXED(4), false, \"_ZGV_LLVM_N4v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z6selectffi\", \"_Z6selectDv8_fS_Dv8_i\", FIXED(8), false, \"_ZGV_LLVM_N8v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z6selectffi\", \"_Z6selectDv16_fS_Dv16_i\", FIXED(16), false, \"_ZGV_LLVM_N16v\", " << IsOCLFnAttr << "},\n";

  OS << "{\"_Z6selectffi\", \"_Z6selectDv4_fS_Dv4_i\", FIXED(4), true, \"_ZGV_LLVM_M4v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z6selectffi\", \"_Z6selectDv8_fS_Dv8_i\", FIXED(8), true, \"_ZGV_LLVM_M8v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z6selectffi\", \"_Z6selectDv16_fS_Dv16_i\", FIXED(16), true, \"_ZGV_LLVM_M16v\", " << IsOCLFnAttr << "},\n";

  OS << "{\"_Z10native_sinf\", \"_Z10native_sinDv4_f\", FIXED(4), false, \"_ZGV_LLVM_N4v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z10native_sinf\", \"_Z10native_sinDv8_f\", FIXED(8), false, \"_ZGV_LLVM_N8v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z10native_sinf\", \"_Z10native_sinDv16_f\", FIXED(16), false, \"_ZGV_LLVM_N16v\", " << IsOCLFnAttr << "},\n";

  OS << "{\"_Z10native_sinf\", \"_Z10native_sinDv4_f\", FIXED(4), true, \"_ZGV_LLVM_M4v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z10native_sinf\", \"_Z10native_sinDv8_f\", FIXED(8), true, \"_ZGV_LLVM_M8v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z10native_sinf\", \"_Z10native_sinDv16_f\", FIXED(16), true, \"_ZGV_LLVM_M16v\", " << IsOCLFnAttr << "},\n";

  OS << "{\"_Z10native_cosf\", \"_Z10native_cosDv4_f\", FIXED(4), false, \"_ZGV_LLVM_N4v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z10native_cosf\", \"_Z10native_cosDv8_f\", FIXED(8), false, \"_ZGV_LLVM_N8v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z10native_cosf\", \"_Z10native_cosDv16_f\", FIXED(16), false, \"_ZGV_LLVM_N16v\", " << IsOCLFnAttr << "},\n";

  OS << "{\"_Z10native_cosf\", \"_Z10native_cosDv4_f\", FIXED(4), true, \"_ZGV_LLVM_M4v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z10native_cosf\", \"_Z10native_cosDv8_f\", FIXED(8), true, \"_ZGV_LLVM_M8v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z10native_cosf\", \"_Z10native_cosDv16_f\", FIXED(16), true, \"_ZGV_LLVM_M16v\", " << IsOCLFnAttr << "},\n";

  OS << "{\"_Z6sincosfPf\", \"_Z6sincosDv2_fPS_\", FIXED(2), false, \"_ZGV_LLVM_N2v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z6sincosfPf\", \"_Z6sincosDv3_fPS_\", FIXED(3), false, \"_ZGV_LLVM_N3v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z6sincosfPf\", \"_Z6sincosDv4_fPS_\", FIXED(4), false, \"_ZGV_LLVM_N4v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z6sincosfPf\", \"_Z6sincosDv8_fPS_\", FIXED(8), false, \"_ZGV_LLVM_N8v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z6sincosfPf\", \"_Z6sincosDv16_fPS_\", FIXED(16), false, \"_ZGV_LLVM_N16v\", " << IsOCLFnAttr << "},\n";

  OS << "{\"_Z3minii\", \"_Z3minDv4_iS_\", FIXED(4), false, \"_ZGV_LLVM_N4v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z3minii\", \"_Z3minDv8_iS_\", FIXED(8), false, \"_ZGV_LLVM_N8v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z3minii\", \"_Z3minDv16_iS_\", FIXED(16), false, \"_ZGV_LLVM_N16v\", " << IsOCLFnAttr << "},\n";

  OS << "{\"_Z3minii\", \"_Z3minDv4_iS_\", FIXED(4), true, \"_ZGV_LLVM_M4v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z3minii\", \"_Z3minDv8_iS_\", FIXED(8), true, \"_ZGV_LLVM_M8v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z3minii\", \"_Z3minDv16_iS_\", FIXED(16), true, \"_ZGV_LLVM_M16v\", " << IsOCLFnAttr << "},\n";

  OS << "{\"_Z3minff\", \"_Z3minDv4_fS_\", FIXED(4), false, \"_ZGV_LLVM_N4v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z3minff\", \"_Z3minDv8_fS_\", FIXED(8), false, \"_ZGV_LLVM_N8v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z3minff\", \"_Z3minDv16_fS_\", FIXED(16), false, \"_ZGV_LLVM_N16v\", " << IsOCLFnAttr << "},\n";

  OS << "{\"_Z3minff\", \"_Z3minDv4_fS_\", FIXED(4), true, \"_ZGV_LLVM_M4v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z3minff\", \"_Z3minDv8_fS_\", FIXED(8), true, \"_ZGV_LLVM_M8v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z3minff\", \"_Z3minDv16_fS_\", FIXED(16), true, \"_ZGV_LLVM_M16v\", " << IsOCLFnAttr << "},\n";

  OS << "{\"_Z3maxii\", \"_Z3maxDv4_iS_\", FIXED(4), false, \"_ZGV_LLVM_N4v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z3maxii\", \"_Z3maxDv8_iS_\", FIXED(8), false, \"_ZGV_LLVM_N8v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z3maxii\", \"_Z3maxDv16_iS_\", FIXED(16), false, \"_ZGV_LLVM_N16v\", " << IsOCLFnAttr << "},\n";

  OS << "{\"_Z3maxii\", \"_Z3maxDv4_iS_\", FIXED(4), true, \"_ZGV_LLVM_M4v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z3maxii\", \"_Z3maxDv8_iS_\", FIXED(8), true, \"_ZGV_LLVM_M8v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z3maxii\", \"_Z3maxDv16_iS_\", FIXED(16), true, \"_ZGV_LLVM_M16v\", " << IsOCLFnAttr << "},\n";

  OS << "{\"_Z3maxff\", \"_Z3maxDv4_fS_\", FIXED(4), false, \"_ZGV_LLVM_N4v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z3maxff\", \"_Z3maxDv8_fS_\", FIXED(8), false, \"_ZGV_LLVM_N8v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z3maxff\", \"_Z3maxDv16_fS_\", FIXED(16), false, \"_ZGV_LLVM_N16v\", " << IsOCLFnAttr << "},\n";

  OS << "{\"_Z3maxff\", \"_Z3maxDv4_fS_\", FIXED(4), true, \"_ZGV_LLVM_M4v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z3maxff\", \"_Z3maxDv8_fS_\", FIXED(8), true, \"_ZGV_LLVM_M8v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z3maxff\", \"_Z3maxDv16_fS_\", FIXED(16), true, \"_ZGV_LLVM_M16v\", " << IsOCLFnAttr << "},\n";

  OS << "{\"_Z4fmaxff\", \"_Z4fmaxDv2_fS_\", FIXED(2), false, \"_ZGV_LLVM_N2v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4fmaxff\", \"_Z4fmaxDv3_fS_\", FIXED(3), false, \"_ZGV_LLVM_N3v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4fmaxff\", \"_Z4fmaxDv4_fS_\", FIXED(4), false, \"_ZGV_LLVM_N4v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4fmaxff\", \"_Z4fmaxDv8_fS_\", FIXED(8), false, \"_ZGV_LLVM_N8v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4fmaxff\", \"_Z4fmaxDv16_fS_\", FIXED(16), false, \"_ZGV_LLVM_N16v\", " << IsOCLFnAttr << "},\n";

  OS << "{\"_Z3logf\", \"_Z3logDv2_f\", FIXED(2), false, \"_ZGV_LLVM_N2v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z3logf\", \"_Z3logDv3_f\", FIXED(3), false, \"_ZGV_LLVM_N3v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z3logf\", \"_Z3logDv4_f\", FIXED(4), false, \"_ZGV_LLVM_N4v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z3logf\", \"_Z3logDv8_f\", FIXED(8), false, \"_ZGV_LLVM_N8v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z3logf\", \"_Z3logDv16_f\", FIXED(16), false, \"_ZGV_LLVM_N16v\", " << IsOCLFnAttr << "},\n";

  OS << "{\"_Z3expf\", \"_Z3expDv2_f\", FIXED(2), false, \"_ZGV_LLVM_N2v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z3expf\", \"_Z3expDv3_f\", FIXED(3), false, \"_ZGV_LLVM_N3v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z3expf\", \"_Z3expDv4_f\", FIXED(4), false, \"_ZGV_LLVM_N4v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z3expf\", \"_Z3expDv8_f\", FIXED(8), false, \"_ZGV_LLVM_N8v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z3expf\", \"_Z3expDv16_f\", FIXED(16), false, \"_ZGV_LLVM_N16v\", " << IsOCLFnAttr << "},\n";

  OS << "{\"_Z3expf\", \"_Z3expDv2_f\", FIXED(2), true, \"_ZGV_LLVM_M2v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z3expf\", \"_Z3expDv3_f\", FIXED(3), true, \"_ZGV_LLVM_M3v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z3expf\", \"_Z3expDv4_f\", FIXED(4), true, \"_ZGV_LLVM_M4v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z3expf\", \"_Z3expDv8_f\", FIXED(8), true, \"_ZGV_LLVM_M8v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z3expf\", \"_Z3expDv16_f\", FIXED(16), true, \"_ZGV_LLVM_M16v\", " << IsOCLFnAttr << "},\n";

  OS << "{\"_Z3expd\", \"_Z3expDv2_d\", FIXED(2), false, \"_ZGV_LLVM_N2v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z3expd\", \"_Z3expDv3_d\", FIXED(3), false, \"_ZGV_LLVM_N3v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z3expd\", \"_Z3expDv4_d\", FIXED(4), false, \"_ZGV_LLVM_N4v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z3expd\", \"_Z3expDv8_d\", FIXED(8), false, \"_ZGV_LLVM_N8v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z3expd\", \"_Z3expDv16_d\", FIXED(16), false, \"_ZGV_LLVM_N16v\", " << IsOCLFnAttr << "},\n";

  OS << "{\"_Z3expd\", \"_Z3expDv2_d\", FIXED(2), true, \"_ZGV_LLVM_M2v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z3expd\", \"_Z3expDv3_d\", FIXED(3), true, \"_ZGV_LLVM_M3v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z3expd\", \"_Z3expDv4_d\", FIXED(4), true, \"_ZGV_LLVM_M4v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z3expd\", \"_Z3expDv8_d\", FIXED(8), true, \"_ZGV_LLVM_M8v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z3expd\", \"_Z3expDv16_d\", FIXED(16), true, \"_ZGV_LLVM_M16v\", " << IsOCLFnAttr << "},\n";

  OS << "{\"_Z10native_expf\", \"_Z10native_expDv2_f\", FIXED(2), false, \"_ZGV_LLVM_N2v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z10native_expf\", \"_Z10native_expDv3_f\", FIXED(3), false, \"_ZGV_LLVM_N3v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z10native_expf\", \"_Z10native_expDv4_f\", FIXED(4), false, \"_ZGV_LLVM_N4v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z10native_expf\", \"_Z10native_expDv8_f\", FIXED(8), false, \"_ZGV_LLVM_N8v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z10native_expf\", \"_Z10native_expDv16_f\", FIXED(16), false, \"_ZGV_LLVM_N16v\", " << IsOCLFnAttr << "},\n";

  OS << "{\"_Z10native_expf\", \"_Z10native_expDv2_f\", FIXED(2), true, \"_ZGV_LLVM_M2v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z10native_expf\", \"_Z10native_expDv3_f\", FIXED(3), true, \"_ZGV_LLVM_M3v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z10native_expf\", \"_Z10native_expDv4_f\", FIXED(4), true, \"_ZGV_LLVM_M4v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z10native_expf\", \"_Z10native_expDv8_f\", FIXED(8), true, \"_ZGV_LLVM_M8v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z10native_expf\", \"_Z10native_expDv16_f\", FIXED(16), true, \"_ZGV_LLVM_M16v\", " << IsOCLFnAttr << "},\n";

  OS << "{\"_Z4sqrtf\", \"_Z4sqrtDv2_f\", FIXED(2), false, \"_ZGV_LLVM_N2v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4sqrtf\", \"_Z4sqrtDv3_f\", FIXED(3), false, \"_ZGV_LLVM_N3v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4sqrtf\", \"_Z4sqrtDv4_f\", FIXED(4), false, \"_ZGV_LLVM_N4v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4sqrtf\", \"_Z4sqrtDv8_f\", FIXED(8), false, \"_ZGV_LLVM_N8v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4sqrtf\", \"_Z4sqrtDv16_f\", FIXED(16), false, \"_ZGV_LLVM_N16v\", " << IsOCLFnAttr << "},\n";

  OS << "{\"_Z4sqrtf\", \"_Z4sqrtDv2_f\", FIXED(2), true, \"_ZGV_LLVM_M2v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4sqrtf\", \"_Z4sqrtDv3_f\", FIXED(3), true, \"_ZGV_LLVM_M3v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4sqrtf\", \"_Z4sqrtDv4_f\", FIXED(4), true, \"_ZGV_LLVM_M4v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4sqrtf\", \"_Z4sqrtDv8_f\", FIXED(8), true, \"_ZGV_LLVM_M8v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4sqrtf\", \"_Z4sqrtDv16_f\", FIXED(16), true, \"_ZGV_LLVM_M16v\", " << IsOCLFnAttr << "},\n";

  OS << "{\"_Z4sqrtd\", \"_Z4sqrtDv2_d\", FIXED(2), false, \"_ZGV_LLVM_N2v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4sqrtd\", \"_Z4sqrtDv3_d\", FIXED(3), false, \"_ZGV_LLVM_N3v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4sqrtd\", \"_Z4sqrtDv4_d\", FIXED(4), false, \"_ZGV_LLVM_N4v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4sqrtd\", \"_Z4sqrtDv8_d\", FIXED(8), false, \"_ZGV_LLVM_N8v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4sqrtd\", \"_Z4sqrtDv16_d\", FIXED(16), false, \"_ZGV_LLVM_N16v\", " << IsOCLFnAttr << "},\n";

  OS << "{\"_Z4sqrtd\", \"_Z4sqrtDv2_d\", FIXED(2), true, \"_ZGV_LLVM_M2v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4sqrtd\", \"_Z4sqrtDv3_d\", FIXED(3), true, \"_ZGV_LLVM_M3v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4sqrtd\", \"_Z4sqrtDv4_d\", FIXED(4), true, \"_ZGV_LLVM_M4v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4sqrtd\", \"_Z4sqrtDv8_d\", FIXED(8), true, \"_ZGV_LLVM_M8v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4sqrtd\", \"_Z4sqrtDv16_d\", FIXED(16), true, \"_ZGV_LLVM_M16v\", " << IsOCLFnAttr << "},\n";

  OS << "{\"_Z3madfff\", \"_Z3madDv2_fS_S_\", FIXED(2), false, \"_ZGV_LLVM_N2v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z3madfff\", \"_Z3madDv3_fS_S_\", FIXED(3), false, \"_ZGV_LLVM_N3v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z3madfff\", \"_Z3madDv4_fS_S_\", FIXED(4), false, \"_ZGV_LLVM_N4v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z3madfff\", \"_Z3madDv8_fS_S_\", FIXED(8), false, \"_ZGV_LLVM_N8v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z3madfff\", \"_Z3madDv16_fS_S_\", FIXED(16), false, \"_ZGV_LLVM_N16v\", " << IsOCLFnAttr << "},\n";

  OS << "{\"_Z4udivjj\", \"_Z4udivDv2_jS_\", FIXED(2), false, \"_ZGV_LLVM_N2v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4udivjj\", \"_Z4udivDv3_jS_\", FIXED(3), false, \"_ZGV_LLVM_N3v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4udivjj\", \"_Z4udivDv4_jS_\", FIXED(4), false, \"_ZGV_LLVM_N4v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4udivjj\", \"_Z4udivDv8_jS_\", FIXED(8), false, \"_ZGV_LLVM_N8v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4udivjj\", \"_Z4udivDv16_jS_\", FIXED(16), false, \"_ZGV_LLVM_N16v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4udivjj\", \"_Z4udivDv32_jS_\", FIXED(32), false, \"_ZGV_LLVM_N32v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4udivjj\", \"_Z4udivDv64_jS_\", FIXED(64), false, \"_ZGV_LLVM_N64v\", " << IsOCLFnAttr << "},\n";

  OS << "{\"_Z4udivjj\", \"_Z4udivDv2_jS_\", FIXED(2), true, \"_ZGV_LLVM_M2v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4udivjj\", \"_Z4udivDv3_jS_\", FIXED(3), true, \"_ZGV_LLVM_M3v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4udivjj\", \"_Z4udivDv4_jS_\", FIXED(4), true, \"_ZGV_LLVM_M4v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4udivjj\", \"_Z4udivDv8_jS_\", FIXED(8), true, \"_ZGV_LLVM_M8v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4udivjj\", \"_Z4udivDv16_jS_\", FIXED(16), true, \"_ZGV_LLVM_M16v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4udivjj\", \"_Z4udivDv32_jS_\", FIXED(32), true, \"_ZGV_LLVM_M32v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4udivjj\", \"_Z4udivDv64_jS_\", FIXED(64), true, \"_ZGV_LLVM_M64v\", " << IsOCLFnAttr << "},\n";

  OS << "{\"_Z4idivii\", \"_Z4idivDv2_iS_\", FIXED(2), false, \"_ZGV_LLVM_N2v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4idivii\", \"_Z4idivDv3_iS_\", FIXED(3), false, \"_ZGV_LLVM_N3v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4idivii\", \"_Z4idivDv4_iS_\", FIXED(4), false, \"_ZGV_LLVM_N4v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4idivii\", \"_Z4idivDv8_iS_\", FIXED(8), false, \"_ZGV_LLVM_N8v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4idivii\", \"_Z4idivDv16_iS_\", FIXED(16), false, \"_ZGV_LLVM_N16v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4idivii\", \"_Z4idivDv32_iS_\", FIXED(32), false, \"_ZGV_LLVM_N32v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4idivii\", \"_Z4idivDv64_iS_\", FIXED(64), false, \"_ZGV_LLVM_N64v\", " << IsOCLFnAttr << "},\n";

  OS << "{\"_Z4idivii\", \"_Z4idivDv2_iS_\", FIXED(2), true, \"_ZGV_LLVM_M2v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4idivii\", \"_Z4idivDv3_iS_\", FIXED(3), true, \"_ZGV_LLVM_M3v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4idivii\", \"_Z4idivDv4_iS_\", FIXED(4), true, \"_ZGV_LLVM_M4v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4idivii\", \"_Z4idivDv8_iS_\", FIXED(8), true, \"_ZGV_LLVM_M8v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4idivii\", \"_Z4idivDv16_iS_\", FIXED(16), true, \"_ZGV_LLVM_M16v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4idivii\", \"_Z4idivDv32_iS_\", FIXED(32), true, \"_ZGV_LLVM_M32v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4idivii\", \"_Z4idivDv64_iS_\", FIXED(64), true, \"_ZGV_LLVM_M64v\", " << IsOCLFnAttr << "},\n";

  OS << "{\"_Z4uremjj\", \"_Z4uremDv2_jS_\", FIXED(2), false, \"_ZGV_LLVM_N2v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4uremjj\", \"_Z4uremDv2_jS_\", FIXED(3), false, \"_ZGV_LLVM_N3v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4uremjj\", \"_Z4uremDv4_jS_\", FIXED(4), false, \"_ZGV_LLVM_N4v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4uremjj\", \"_Z4uremDv8_jS_\", FIXED(8), false, \"_ZGV_LLVM_N8v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4uremjj\", \"_Z4uremDv16_jS_\", FIXED(16), false, \"_ZGV_LLVM_N16v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4uremjj\", \"_Z4uremDv32_jS_\", FIXED(32), false, \"_ZGV_LLVM_N32v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4uremjj\", \"_Z4uremDv64_jS_\", FIXED(64), false, \"_ZGV_LLVM_N64v\", " << IsOCLFnAttr << "},\n";

  OS << "{\"_Z4uremjj\", \"_Z4uremDv2_jS_\", FIXED(2), true, \"_ZGV_LLVM_M2v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4uremjj\", \"_Z4uremDv2_jS_\", FIXED(3), true, \"_ZGV_LLVM_M3v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4uremjj\", \"_Z4uremDv4_jS_\", FIXED(4), true, \"_ZGV_LLVM_M4v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4uremjj\", \"_Z4uremDv8_jS_\", FIXED(8), true, \"_ZGV_LLVM_M8v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4uremjj\", \"_Z4uremDv16_jS_\", FIXED(16), true, \"_ZGV_LLVM_M16v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4uremjj\", \"_Z4uremDv32_jS_\", FIXED(32), true, \"_ZGV_LLVM_M32v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4uremjj\", \"_Z4uremDv64_jS_\", FIXED(64), true, \"_ZGV_LLVM_M64v\", " << IsOCLFnAttr << "},\n";

  OS << "{\"_Z4iremii\", \"_Z4iremDv2_iS_\", FIXED(2), false, \"_ZGV_LLVM_N2v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4iremii\", \"_Z4iremDv3_iS_\", FIXED(3), false, \"_ZGV_LLVM_N3v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4iremii\", \"_Z4iremDv4_iS_\", FIXED(4), false, \"_ZGV_LLVM_N4v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4iremii\", \"_Z4iremDv8_iS_\", FIXED(8), false, \"_ZGV_LLVM_N8v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4iremii\", \"_Z4iremDv16_iS_\", FIXED(16), false, \"_ZGV_LLVM_N16v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4iremii\", \"_Z4iremDv32_iS_\", FIXED(32), false, \"_ZGV_LLVM_N32v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4iremii\", \"_Z4iremDv64_iS_\", FIXED(64), false, \"_ZGV_LLVM_N64v\", " << IsOCLFnAttr << "},\n";

  OS << "{\"_Z4iremii\", \"_Z4iremDv2_iS_\", FIXED(2), true, \"_ZGV_LLVM_M2v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4iremii\", \"_Z4iremDv3_iS_\", FIXED(3), true, \"_ZGV_LLVM_M3v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4iremii\", \"_Z4iremDv4_iS_\", FIXED(4), true, \"_ZGV_LLVM_M4v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4iremii\", \"_Z4iremDv8_iS_\", FIXED(8), true, \"_ZGV_LLVM_M8v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4iremii\", \"_Z4iremDv16_iS_\", FIXED(16), true, \"_ZGV_LLVM_M16v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4iremii\", \"_Z4iremDv32_iS_\", FIXED(32), true, \"_ZGV_LLVM_M32v\", " << IsOCLFnAttr << "},\n";
  OS << "{\"_Z4iremii\", \"_Z4iremDv64_iS_\", FIXED(64), true, \"_ZGV_LLVM_M64v\", " << IsOCLFnAttr << "},\n";

  OS << "{\"for_random_number\", \"for_simd_random_number\", FIXED(2), false, \"_ZGV_LLVM_N2v\", " << IsFortranOnlyAttr << "},\n";
  OS << "{\"for_random_number\", \"for_simd_random_number_mask\", FIXED(2), true, \"_ZGV_LLVM_M2v\", " << IsFortranOnlyAttr << "},\n";
  OS << "{\"for_random_number\", \"for_simd_random_number_avx\", FIXED(4), false, \"_ZGVcN4v\", " << IsFortranOnlyAttr << "},\n";
  OS << "{\"for_random_number\", \"for_simd_random_number_avx_mask\", FIXED(4), true, \"_ZGVcM4v\", " << IsFortranOnlyAttr << "},\n";

  OS << "{\"for_random_number_single\", \"for_simd_random_number_single\", FIXED(4), false, \"_ZGV_LLVM_N4v\", " << IsFortranOnlyAttr << "},\n";
  OS << "{\"for_random_number_single\", \"for_simd_random_number_single_mask\", FIXED(4), true, \"_ZGV_LLVM_M4v\", " << IsFortranOnlyAttr << "},\n";
  OS << "{\"for_random_number_single\", \"for_simd_random_number_single_avx\", FIXED(8), false, \"_ZGVcN8v\", " << IsFortranOnlyAttr << "},\n";
  OS << "{\"for_random_number_single\", \"for_simd_random_number_single_avx_mask\", FIXED(8), true, \"_ZGVcM8v\", " << IsFortranOnlyAttr << "},\n";

  OS << "#endif // GET_SVML_VARIANTS\n\n";
}

void SVMLVariantsEmitter::emitSVMLDeviceVariants(raw_ostream &OS) {
  assert(Records.getClass("SvmlVariants") &&
         "SvmlVariants class not found in target description file!");

  std::vector<Record *> SvmlVariants =
      Records.getAllDerivedDefinitions("SvmlVariants");

  OS << "#ifdef GET_SVML_VARIANTS\n";
  OS << "#define FIXED(NL) ElementCount::getFixed(NL)\n";

  for (auto SvmlVariant : SvmlVariants) {
    // TODO: enable SIMD32 once it's supported by SPIR-V
    unsigned SIMDSizes[] = {
        8, 16,
        // 32
    };

    for (auto SIMDSize : SIMDSizes) {
      auto FuncName = SvmlVariant->getName();
      auto ArgsNum = SvmlVariant->getValueAsInt("Args");
      bool HasIntrin = SvmlVariant->getValueAsBit("hasIntrinsic");

      auto EmitVariant = [ArgsNum, SIMDSize, &OS](const StringRef Name,
                                                  bool IsIntrinsic,
                                                  bool Is64Bit, bool IsMasked) {
        OS << "{\"";
        if (IsIntrinsic)
          OS << "llvm.";
        OS << Name;
        if (IsIntrinsic)
          OS << (Is64Bit ? ".f64" : ".f32");
        else if (!Is64Bit)
          OS << "f";
        OS << "\", ";
        std::stringstream VABIPrefix;
        VABIPrefix << "\"_ZGV_unknown_" << (IsMasked ? "M" : "N") << SIMDSize;
        for (int i = 0; i < ArgsNum; ++i) {
          // TODO: support other argument types when necessary
          VABIPrefix << 'v';
        }
        OS << VABIPrefix.str();
        OS << "___svml_device_" << Name;
        if (!Is64Bit)
          OS << "f";
        OS << "\", FIXED(" << SIMDSize << "), " << (IsMasked ? "true" : "false")
           << ", " << VABIPrefix.str() << "\"},\n";
      };

      for (bool Is64Bit : {true, false}) {
        for (bool IsMasked : {true, false}) {
          EmitVariant(FuncName, false, Is64Bit, IsMasked);
          if (HasIntrin)
            EmitVariant(FuncName, true, Is64Bit, IsMasked);
        }
      }
    }
  }
  OS << "#endif // GET_SVML_VARIANTS\n\n";
}

void SVMLVariantsEmitter::run(raw_ostream &OS, bool IsDevice) {
  if (IsDevice)
    emitSVMLDeviceVariants(OS);
  else
    emitSVMLVariants(OS);
}

static void EmitSVMLVariants(RecordKeeper &RK, raw_ostream &OS) {
  SVMLVariantsEmitter(RK).run(OS, /*IsDevice=*/false);
}

static TableGen::Emitter::Opt X("gen-svml", EmitSVMLVariants,
                                "Generate SVML variant function names");

static void EmitSVMLDeviceVariants(RecordKeeper &RK, raw_ostream &OS) {
  SVMLVariantsEmitter(RK).run(OS, /*IsDevice=*/true);
}

static TableGen::Emitter::Opt
    Y("gen-svml-device", EmitSVMLDeviceVariants,
      "Generate OMP SIMD versions of SVML variant function names");
