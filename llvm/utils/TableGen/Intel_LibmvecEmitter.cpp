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
/// This tablegen backend is responsible for emitting the set of Libmvec
/// variants.
///
///  External interfaces:
///      void EmitLibmvecVariants(RecordKeeper &RK, raw_ostream &OS);
///
// ===--------------------------------------------------------------------=== //

#include "CodeGenTarget.h"
#include "llvm/Support/Format.h"
#include "llvm/TableGen/Error.h"
#include "llvm/TableGen/Record.h"
#include "llvm/TableGen/TableGenBackend.h"
#include <map>
#include <vector>

using namespace llvm;

#define DEBUG_TYPE "LibmvecVariants"

namespace {

class LibmvecVariantsEmitter {

private:
  RecordKeeper &Records;
  void emitLibmvecVariants(raw_ostream &OS);

public:
  LibmvecVariantsEmitter(RecordKeeper &R) : Records(R) {}

  void run(raw_ostream &OS);
};
} // End anonymous namespace

/// \brief Emit the set of Libmvec variant function names.
void LibmvecVariantsEmitter::emitLibmvecVariants(raw_ostream &OS) {

  // This function reads entries from include/llvm/IR/Intel_Libmvec.td and
  // generates the scalar math call/intrinsic call to vector call mapping.
  // Variants will include single/double precision, masked/non-masked for
  // VF of 2-8 for double precision and VF of 4-16 for single precision.
  // The last column of true/false indicates whether the mapping is for a
  // masked or non-masked variant.
  //
  // def sin    : LibmvecVariant<[Vector]>;
  //
  // is translated to:
  //
  // {"sinf", "_ZGVbN4v_sin", 4, false},
  // {"sinf", "_ZGVbM4v_sin", 4, true},
  // {"llvm.sin.f32", "_ZGVbN4v_sin", 4, false},
  // {"llvm.sin.f32", "_ZGVbM4v_sin", 4, true},
  // {"sinf", "_ZGVcN8v_sin", 8, false},
  // {"sinf", "_ZGVcM8v_sin", 8, true},
  // {"llvm.sin.f32", "_ZGVcN8v_sin", 8, false},
  // {"llvm.sin.f32", "_ZGVcM8v_sin", 8, true},
  // {"sinf", "_ZGVdN8v_sin", 8, false},
  // {"sinf", "_ZGVdM8v_sin", 8, true},
  // {"llvm.sin.f32", "_ZGVdN8v_sin", 8, false},
  // {"llvm.sin.f32", "_ZGVdM8v_sin", 8, true},
  // {"sinf", "_ZGVeN16v_sin", 16, false},
  // {"sinf", "_ZGVeM16v_sin", 16, true},
  // {"llvm.sin.f32", "_ZGVeN16v_sin", 16, false},
  // {"llvm.sin.f32", "_ZGVeM16v_sin", 16, true},
  // {"sin", "_ZGVbN2v_sin", 2, false},
  // {"sin", "_ZGVbM2v_sin", 2, true},
  // {"llvm.sin.f64", "_ZGVbN2v_sin", 2, false},
  // {"llvm.sin.f64", "_ZGVbM2v_sin", 2, true},
  // {"sin", "_ZGVcN4v_sin", 4, false},
  // {"sin", "_ZGVcM4v_sin", 4, true},
  // {"llvm.sin.f64", "_ZGVcN4v_sin", 4, false},
  // {"llvm.sin.f64", "_ZGVcM4v_sin", 4, true},
  // {"sin", "_ZGVdN4v_sin", 4, false},
  // {"sin", "_ZGVdM4v_sin", 4, true},
  // {"llvm.sin.f64", "_ZGVdN4v_sin", 4, false},
  // {"llvm.sin.f64", "_ZGVdM4v_sin", 4, true},
  // {"sin", "_ZGVeN8v_sin", 8, false},
  // {"sin", "_ZGVeM8v_sin", 8, true},
  // {"llvm.sin.f64", "_ZGVeN8v_sin", 8, false},
  // {"llvm.sin.f64", "_ZGVeM8v_sin", 8, true},

  std::map<unsigned, std::vector<char>> VLToISAMapFloat;
  std::map<unsigned, std::vector<char>> VLToISAMapDouble;

  VLToISAMapFloat[4] = {'b'};
  VLToISAMapFloat[8] = {'c', 'd'};
  VLToISAMapFloat[16] = {'e'};

  VLToISAMapDouble[2] = {'b'};
  VLToISAMapDouble[4] = {'c', 'd'};
  VLToISAMapDouble[8] = {'e'};

  Record *LibmvecVariantClass = Records.getClass("LibmvecVariant");
  assert(LibmvecVariantClass &&
         "LibmvecVariant class not found in target description file!");

  std::vector<Record*> LibmvecVariants =
      Records.getAllDerivedDefinitions("LibmvecVariant");

  OS << "#ifdef GET_LIBMVEC_VARIANTS\n";

  for (auto LibmvecVariant : LibmvecVariants) {
    std::vector<Record*> ArgList =
      LibmvecVariant->getValueAsListOfDefs("ArgList");
    std::string LibmvecVariantNameStr = LibmvecVariant->getName();
    std::string ParmSignatureStr = "";
    for (unsigned i = 0; i < ArgList.size(); i++) {
      ParmSignatureStr += ArgList[i]->getValueAsString("Type");
    }

    // Emit single precision variants.
    for (unsigned VL = 4; VL <= 16; VL *= 2) {
      std::vector<char> IsaClasses = VLToISAMapFloat[VL];
      for (unsigned j = 0; j < IsaClasses.size(); j++) {
        // math library call to non-masked variant.
        OS << "{\"" << LibmvecVariantNameStr << "f" << "\", ";
        OS << "\"" << "_ZGV" << IsaClasses[j] << "N" << VL << ParmSignatureStr
           << "_" << LibmvecVariantNameStr << "\", " << VL << ", false},\n";

        // math library call to masked variant.
        OS << "{\"" << LibmvecVariantNameStr << "f" << "\", ";
        OS << "\"" << "_ZGV" << IsaClasses[j] << "M" << VL << ParmSignatureStr
           << "_" << LibmvecVariantNameStr << "\", " << VL << ", true},\n";

        // llvm intrinsic to non-masked variant.
        OS << "{\"" << "llvm." << LibmvecVariantNameStr << ".f32" << "\", ";
        OS << "\"" << "_ZGV" << IsaClasses[j] << "N" << VL << ParmSignatureStr
           << "_" << LibmvecVariantNameStr << "\", " << VL << ", false},\n";

        // llvm intrinsic to masked variant.
        OS << "{\"" << "llvm." << LibmvecVariantNameStr << ".f32" << "\", ";
        OS << "\"" << "_ZGV" << IsaClasses[j] << "M" << VL << ParmSignatureStr
           << "_" << LibmvecVariantNameStr << "\", " << VL << ", true},\n";
      }
    }

    // Emit double precision variants.
    for (unsigned VL = 2; VL <= 8; VL *= 2) {
      std::vector<char> IsaClasses = VLToISAMapDouble[VL];
      for (unsigned j = 0; j < IsaClasses.size(); j++) {
        // math library call to non-masked variant.
        OS << "{\"" << LibmvecVariantNameStr << "\", ";
        OS << "\"" << "_ZGV" << IsaClasses[j] << "N" << VL << ParmSignatureStr
           << "_" << LibmvecVariantNameStr << "\", " << VL << ", false},\n";

        // math library call to masked variant.
        OS << "{\"" << LibmvecVariantNameStr << "\", ";
        OS << "\"" << "_ZGV" << IsaClasses[j] << "M" << VL << ParmSignatureStr
           << "_" << LibmvecVariantNameStr << "\", " << VL << ", true},\n";

        // llvm intrinsic to non-masked variant.
        OS << "{\"" << "llvm." << LibmvecVariantNameStr << ".f64" << "\", ";
        OS << "\"" << "_ZGV" << IsaClasses[j] << "N" << VL << ParmSignatureStr
           << "_" << LibmvecVariantNameStr << "\", " << VL << ", false},\n";

        // llvm intrinsic to masked variant.
        OS << "{\"" << "llvm." << LibmvecVariantNameStr << ".f64" << "\", ";
        OS << "\"" << "_ZGV" << IsaClasses[j] << "M" << VL << ParmSignatureStr
           << "_" << LibmvecVariantNameStr << "\", " << VL << ", true},\n";
      }
    }
  }

  OS << "#endif // GET_LIBMVEC_VARIANTS\n\n";
}

void LibmvecVariantsEmitter::run(raw_ostream &OS) {
  emitLibmvecVariants(OS);
}

namespace llvm {

void EmitLibmvecVariants(RecordKeeper &RK, raw_ostream &OS) {
  LibmvecVariantsEmitter(RK).run(OS);
}

} // End llvm namespace
