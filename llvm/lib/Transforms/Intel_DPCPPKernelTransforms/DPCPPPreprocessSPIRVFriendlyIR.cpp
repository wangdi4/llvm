//===- DPCPPPreprocessSPIRVFriendlyIR.cpp - DPC++ preprocessor on SPV-IR --===//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPPreprocessSPIRVFriendlyIR.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/CompilationUtils.h"

using namespace llvm;

#define DEBUG_TYPE "dpcpp-kernel-preprocess-spv-ir"

// Add "opencl.ocl.version" named metadata to SYCL module.
// SYCL (OpenCL CPP) implies OpenCL 2.0 implemententation for CPU backend.
// e.g.
// !opencl.ocl.version = !{!6}
// !6 = !{i32 2, i32 0}
static bool insertOpenCLVersionMetadata(Module &M) {
  if (!CompilationUtils::isGeneratedFromOCLCPP(M))
    return false;

  const char OCLVer[] = "opencl.ocl.version";
  if (M.getNamedMetadata(OCLVer))
    return false;

  auto *OCLVerMD = M.getOrInsertNamedMetadata(OCLVer);
  auto *Int32Ty = llvm::IntegerType::getInt32Ty(M.getContext());
  llvm::Metadata *OCLVerElts[] = {
      llvm::ConstantAsMetadata::get(llvm::ConstantInt::get(Int32Ty, 2)),
      llvm::ConstantAsMetadata::get(llvm::ConstantInt::get(Int32Ty, 0))};
  OCLVerMD->addOperand(llvm::MDNode::get(M.getContext(), OCLVerElts));
  return true;
}

// Translate OCL image types (refer to
// clang/include/clang/Basic/OpenCLImageTypes.def) to SPIRV image postfix
// representation:
// _{SampledType}_{Dim}_{Depth}_{Arrayed}_{MS}_{Sampled}_{Format}_{Access}
static bool
translateOCLImageTypeToSPIRVImagePostfix(StringRef OCLTypeName,
                                         std::string &SPIRVImagePostfix) {
  unsigned Dim = StringSwitch<unsigned>(OCLTypeName)
                     .StartsWith("image1d", 1)
                     .StartsWith("image2d", 2)
                     .StartsWith("image3d", 3)
                     .Default(0);
  if (Dim == 0)
    return false;
  OCLTypeName = OCLTypeName.drop_front(strlen("image*d"));
  // Arrayed must be one of the following indicated values:
  // 0 indicates non-arrayed content
  // 1 indicates arrayed content
  unsigned Arrayed = static_cast<unsigned>(OCLTypeName.consume_front("_array"));
  // Sampled indicates whether or not this image is accessed in combination with
  // a sampler, and must be one of the following values:
  // 0 indicates this is only known at run time, not at compile time
  // 1 indicates used with sampler
  // 2 indicates used without a sampler (a storage image)
  unsigned Sampled = OCLTypeName.consume_front("_buffer") ? 2 : 1;
  // MS must be one of the following indicated values:
  // 0 indicates single-sampled content
  // 1 indicates multisampled content
  unsigned MS = static_cast<unsigned>(OCLTypeName.consume_front("_msaa"));
  // Depth is whether or not this image is a depth image. (Note that whether or
  // not depth comparisons are actually done is a property of the sampling
  // opcode, not of this type declaration.)
  // 0 indicates not a depth image
  // 1 indicates a depth image
  // 2 means no indication as to whether this is a depth or non-depth image
  unsigned Depth = static_cast<unsigned>(OCLTypeName.consume_front("_depth"));
  // Access defines the access permissions:
  // 0 indicates ReadOnly
  // 1 indicates WriteOnly
  // 2 indicates ReadWrite
  const unsigned Invalid = 42;
  unsigned Access = StringSwitch<unsigned>(OCLTypeName)
                        .Case("_ro_t", 0)
                        .Case("_wo_t", 1)
                        .Case("_rw_t", 2)
                        .Default(Invalid);
  if (Access == Invalid)
    return false;

  // SampledType is always unknown (void)
  const char SampledType[] = "void";
  // Format is always unknown (0)
  const unsigned Format = 0;
  SPIRVImagePostfix =
      ("_" + Twine(SampledType) + "_" + Twine(Dim) + "_" + Twine(Depth) + "_" +
       Twine(Arrayed) + "_" + Twine(MS) + "_" + Twine(Sampled) + "_" +
       Twine(Format) + "_" + Twine(Access))
          .str();
  return true;
}

// Materialize SampledImage opaque type name coming from SYCL frontend,
// to the form conforming with SPIR-V Friendly IR SPEC:
// https://github.com/KhronosGroup/SPIRV-LLVM-Translator/blob/master/docs/SPIRVRepresentationInLLVM.rst#optypeimage
//
// e.g.
// %spirv.SampledImage.image2d_ro_t = type opaque
// will be converted as
// %spirv.SampledImage._void_2_0_0_0_1_0_0 = type opaque
// The postfix format:
// _{SampledType}_{Dim}_{Depth}_{Arrayed}_{MS}_{Sampled}_{Format}_{Access}
// where the encoding of each field is defined in SPIR-V SPEC:
// https://www.khronos.org/registry/SPIR-V/specs/unified1/SPIRV.html#OpTypeImage
//
// Note: this method is a workaround as SYCL-frontend-emitted IR is not fully
// aligned with the SPV-IR SPEC.
static bool materializeSampledImageOpaqueTypeName(Module &M) {
  bool Changed = false;
  const char SPIRVSampledImage[] = "spirv.SampledImage.";
  // A replace map from OCL image type name (e.g. image1d_ro) to SPV-IR image
  // postfix (e.g. _void_1_0_0_0_1_0_0).
  StringMap<std::string> ReplaceMap;
  for (auto *ST : M.getIdentifiedStructTypes()) {
    if (!ST->hasName())
      continue;
    StringRef Name = ST->getName();
    if (!Name.consume_front(SPIRVSampledImage))
      continue;
    // Dropped "spirv.SampledImage." prefix.
    StringRef OCLImageTypeName = Name;
    LLVM_DEBUG(dbgs() << "Materializing Image opaque type name: "
                      << ST->getName() << '\n');
    std::string Postfix;
    if (!translateOCLImageTypeToSPIRVImagePostfix(Name, Postfix))
      continue;
    // Store OCL image type name --> SPV-IR postfix mapping.
    // "_t" is dropped for SampledImage in Itanium mangling.
    if (!OCLImageTypeName.consume_back("_t"))
      continue;
    ReplaceMap[OCLImageTypeName] = Postfix;
    // Rename the StructType.
    ST->setName(SPIRVSampledImage + Postfix);
    LLVM_DEBUG(dbgs() << "  --> " << ST->getName() << '\n');
    Changed = true;
  }

  // Recovering the function name mangling.
  // Clang FrontEnd mangling (clang/lib/AST/ItaniumMangle.cpp:3027):
  // ```cpp
  // #define IMAGE_TYPE(ImgType, Id, SingletonId, Access, Suffix)             \
  //   case BuiltinType::Sampled##Id:                                         \
  //     type_name = "__spirv_SampledImage__" #ImgType "_" #Suffix;           \
  //     Out << type_name.size() << type_name;                                \
  //     break;
  // #define IMAGE_WRITE_TYPE(Type, Id, Ext)
  // #define IMAGE_READ_WRITE_TYPE(Type, Id, Ext)
  // #include "clang/Basic/OpenCLImageTypes.def"
  // ```
  // The desired mangling defined by SPV-IR:
  // __spirv_SampledImage__{SampledType}_{Dim}_{Depth}_{Arrayed}_{MS}_{Sampled}_{Format}_{Access}
  StringRef ManglePrefix("__spirv_SampledImage_");
  for (auto &Entry : ReplaceMap) {
    std::string OriginalMangled = (ManglePrefix + "_" + Entry.getKey()).str();
    OriginalMangled = std::to_string(OriginalMangled.size()) + OriginalMangled;

    std::string ReplaceMangled = ManglePrefix.str() + Entry.getValue();
    ReplaceMangled = std::to_string(ReplaceMangled.size()) + ReplaceMangled;

    for (auto &F : M) {
      StringRef FuncName = F.getName();
      if (!FuncName.contains(OriginalMangled))
        continue;
      // Replace all occurrences of OriginalMangled with ReplaceMangled.
      SmallVector<StringRef, 2> Splits;
      FuncName.split(Splits, OriginalMangled);
      LLVM_DEBUG(dbgs() << "Remangling function: " << F.getName() << " --> ");
      F.setName(join(Splits, ReplaceMangled));
      LLVM_DEBUG(dbgs() << F.getName() << '\n');
    }
  }
  return Changed;
}

bool DPCPPPreprocessSPIRVFriendlyIRPass::runImpl(Module &M) {
  bool Changed = false;
  Changed |= insertOpenCLVersionMetadata(M);
  Changed |= materializeSampledImageOpaqueTypeName(M);
  return Changed;
}

PreservedAnalyses
DPCPPPreprocessSPIRVFriendlyIRPass::run(Module &M, ModuleAnalysisManager &) {
  if (!runImpl(M))
    return PreservedAnalyses::all();
  PreservedAnalyses PA;
  PA.preserveSet<CFGAnalyses>();
  PA.preserve<CallGraphAnalysis>();
  return PA;
}
