//==-------------- dgetrf.cpp  - DPC++ ESIMD on-device test ----------------==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
<<<<<<< HEAD
// TODO: remove fno-fast-math option once the issue is investigated and the test
// is fixed.
// UNSUPPORTED: esimd_emulator
// DEFINE: %{mathflags} = %if cl_options %{/clang:-fno-fast-math%} %else %{-fno-fast-math%}
// INTEL_CUSTOMIZATION
// this is a temporary fix for CMPLRTST-21037
// we pass -fno-inline-functions to sure it won't hang in host compile
// we don't do the same thing in intel/llvm since it will lead to runfail
// DEFINE: %{noinlineflags} = %if cl_options %{/clang:-fno-inline-functions%} %else %{-fno-inline-functions%}
// RUN: %{build} %{mathflags} %{noinlineflags} -I%S/.. -O2 -o %t.out
// end INTEL_CUSTOMIZATION
=======
// RUN: %{build} -I%S/.. -o %t.out
>>>>>>> 9698767dfee6acde192d5e06d6f47b7abf97cea2
// RUN: %{run} %t.out 3 2 1
//
// This test checks the correctness of ESIMD program for batched LU
// decomposition without pivoting. The program contains multiple branches
// corresponding to LU input sizes; all internal functions are inlined.
//

#include "Inputs/dgetrf.hpp"
