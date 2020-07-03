//===- X86ModRMFilters.cpp - Disassembler ModR/M filterss -------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "X86ModRMFilters.h"

using namespace llvm::X86Disassembler;

void ModRMFilter::anchor() { }

void DumbFilter::anchor() { }

void ModFilter::anchor() { }

void ExtendedFilter::anchor() { }

<<<<<<< HEAD
#if INTEL_CUSTOMIZATION
#if INTEL_FEATURE_ISA_AMX
void ExtendedRMFilter::anchor() { }
#endif // INTEL_FEATURE_ISA_AMX
#endif // INTEL_CUSTOMIZATION
=======
void ExtendedRMFilter::anchor() { }
>>>>>>> aded4f0cc070fcef6763c9a3c2ba764d652b692e

void ExactFilter::anchor() { }
