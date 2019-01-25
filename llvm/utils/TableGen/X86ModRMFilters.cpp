//===- X86ModRMFilters.cpp - Disassembler ModR/M filterss -------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "X86ModRMFilters.h"

using namespace llvm::X86Disassembler;

void ModRMFilter::anchor() { }

void DumbFilter::anchor() { }

void ModFilter::anchor() { }

void ExtendedFilter::anchor() { }

#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_ISA_AMX
void ExtendedRMFilter::anchor() { }
#endif // INTEL_FEATURE_ISA_AMX
#endif // INTEL_CUSTOMIZATION

void ExactFilter::anchor() { }
