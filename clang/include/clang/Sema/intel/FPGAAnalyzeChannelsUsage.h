//==--- FPGAAnalyzeChannelsUsage.h -                            *- C++ -*---==//
//
// Copyright (C) 2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef LLVM_CLANG_LIB_SEMA_INTEL_FPGAANALYZECHANNELSUSAGE_H
#define LLVM_CLANG_LIB_SEMA_INTEL_FPGAANALYZECHANNELSUSAGE_H

namespace clang {

class Decl;
class Sema;

void launchOCLFPGAFeaturesAnalysis(const Decl *, Sema &);

} // namespace clang

#endif // LLVM_CLANG_LIB_SEMA_INTEL_FPGAANALYZECHANNELSUSAGE_H
