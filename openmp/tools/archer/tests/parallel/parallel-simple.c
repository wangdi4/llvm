/*
 * parallel-simple.c -- Archer testcase
 */

//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
//
// See tools/archer/LICENSE.txt for details.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//


// RUN: %libarcher-compile && env OMP_TOOL_VERBOSE_INIT=stderr %libarcher-run 2>&1 | FileCheck %s --check-prefixes CHECK,TSAN_ON
// INTEL_CUSTOMIZATION
// RUN: %clang-archer %openmp_flags %flags %s -o %t && env OMP_TOOL_LIBRARIES="libarcher.so" OMP_TOOL_VERBOSE_INIT=stderr %t 2>&1 | FileCheck %s --check-prefixes CHECK,TSAN_OFF
// end INTEL_CUSTOMIZATION
// REQUIRES: tsan
#include <omp.h>
#include <stdio.h>

// TSAN_ON: ----- START LOGGING OF TOOL REGISTRATION -----
// TSAN_ON-NEXT: Search for OMP tool in current address space... Failed.
// INTEL_CUSTOMIZATION
// Unlike libomp, libiomp doesn't use archer if no other tool is found.
// Therefore, we do need to define OMP_TOOL_LIBRARIES="libarcher.so"
// to enable these tests. Eventually we should ask libiomp to mimic
// libomp's behavior in handling archer.
// TSAN_ON-NEXT: Searching tool libraries...
// TSAN_ON-NEXT: OMP_TOOL_LIBRARIES = libarcher.so
// end INTEL_CUSTOMIZATION
// TSAN_ON-NEXT: Opening libarcher.so... Success.
// TSAN_ON-NEXT: Searching for ompt_start_tool in libarcher.so... Success.
// TSAN_ON-NEXT: Tool was started and is using the OMPT interface.
// TSAN_ON-NEXT: ----- END LOGGING OF TOOL REGISTRATION -----

// TSAN_OFF: ----- START LOGGING OF TOOL REGISTRATION -----
// TSAN_OFF-NEXT: Search for OMP tool in current address space... Failed.
// INTEL_CUSTOMIZATION
// Unlike libomp, libiomp doesn't use archer if no other tool is found.
// Therefore, we do need to define OMP_TOOL_LIBRARIES="libarcher.so"
// to enable these tests. Eventually we should ask libiomp to mimic
// libomp's behavior in handling archer.
// TSAN_OFF-NEXT: Searching tool libraries...
// TSAN_OFF-NEXT: OMP_TOOL_LIBRARIES = libarcher.so
// TSAN_OFF-NEXT: Opening libarcher.so... Success.
// TSAN_OFF-NEXT: Searching for ompt_start_tool in libarcher.so... Found but not using the OMPT interface.
// TSAN_OFF-NEXT: Continuing search...
// end INTEL_CUSTOMIZATION
// TSAN_OFF-NEXT: No OMP tool loaded.
// TSAN_OFF-NEXT: ----- END LOGGING OF TOOL REGISTRATION -----


int main(int argc, char *argv[]) {
  int var = 0;

#pragma omp parallel num_threads(2) shared(var)
  {
    if (omp_get_thread_num() == 1) {
      var++;
    }
  } // implicit barrier

  var++;

  fprintf(stderr, "DONE\n");
  int error = (var != 2);
  return error;
}

// CHECK-NOT: ThreadSanitizer: data race
// CHECK-NOT: ThreadSanitizer: reported
// CHECK-NOT: Warning: please export TSAN_OPTIONS
// CHECK: DONE
