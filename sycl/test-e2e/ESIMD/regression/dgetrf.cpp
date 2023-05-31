//==-------------- dgetrf.cpp  - DPC++ ESIMD on-device test ----------------==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: esimd_emulator
// INTEL_CUSTOMIZATION
// this is a temporary fix for CMPLRTST-21037
// we pass -fno-inline-functions to sure it won't hang in host compile
// we don't do the same thing in intel/llvm since it will lead to runfail
// RUN: %{build} -I%S/.. -O2 -fno-inline-functions -o %t.out
// end INTEL_CUSTOMIZATION
// RUN: %{run} %t.out 3 2 1
//
// Failure is under investigation, tracked internally:
// UNSUPPORTED: gpu-intel-pvc
//
// This test checks the correctness of ESIMD program for batched LU
// decomposition without pivoting. The program contains multiple branches
// corresponding to LU input sizes; all internal functions are inlined.
//

#include "Inputs/dgetrf.hpp"
