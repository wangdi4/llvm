// REQUIRES: windows

// RUN: mkdir -p %t.d
// RUN: %clang_profgen -mllvm -enable-value-profiling -c -O2 -o %t.d/main.obj %s
// RUN: %clang_profgen -mllvm -enable-value-profiling -c -O2 -o %t.d/instr-reset-all-dyn1.obj %S/Inputs/instr-reset-all-dyn1.c
// RUN: %clang_profgen -Wl,-dll -o %t.d/instr-reset-all-dyn1.dll %t.d/instr-reset-all-dyn1.obj
// RUN: %clang_profgen -o %t.d/main.exe %t.d/main.obj %t.d/instr-reset-all-dyn1.lib
// RUN: env LLVM_PROFILE_FILE=%t.d/instr-reset-all.profraw %run %t.d/main.exe
// RUN: llvm-profdata merge -o %t.d/instr-reset-all.profdata %t.d/*.profraw

// llvm-profdata does not currently report functions sorted by name, so for now,
// use separate check blocks to check the counters before and after the use of
// the reset function.
// RUN: llvm-profdata show --all-functions %t.d/instr-reset-all.profdata | FileCheck --check-prefix=CHECK-RESET %s
// RUN: llvm-profdata show --all-functions %t.d/instr-reset-all.profdata | FileCheck --check-prefix=CHECK-NORMAL %s

// Verify the indirect call data is reset.
// RUN: llvm-profdata show --all-functions --ic-targets %t.d/instr-reset-all.profdata | FileCheck --check-prefix=CHECK-INDIRECTS %s

// This test checks whether the compiler API function that allows the
// user to force dumping profiling data dumps the data from a DLL that
// is linked into the application. In this case, the function 'dos1_func1'
// is coming from a DLL, and the other two functions come from the main
// module.

// CHECK-RESET: Counters:
// CHECK-RESET:  dso_sub:
// CHECK-RESET-NEXT:    Hash: 0x0000000000000000
// CHECK-RESET-NEXT:    Counters: 1
// CHECK-RESET-NEXT:    Function count: 0

// CHECK-NORMAL: Counters:
// CHECK-NORMAL:  dso_add:
// CHECK-NORMAL-NEXT:    Hash: 0x0000000000000000
// CHECK-NORMAL-NEXT:    Counters: 1
// CHECK-NORMAL-NEXT:    Function count: 1

// CHECK-INDIRECTS: Counters:
// CHECK-INDIRECTS:  dso1_exec:
// CHECK-INDIRECTS-NEXT:    Hash: 0x0000000000000000
// CHECK-INDIRECTS-NEXT:    Counters: 1
// CHECK-INDIRECTS-NEXT:    Function count: 1
// CHECK-INDIRECTS-NEXT:    Indirect Call Site Count: 1
// CHECK-INDIRECTS-NEXT:    Indirect Target Results:
// CHECK-INDIRECTS-NEXT:        [  0, dso_add,          1 ]
// CHECK-INDIRECTS-NEXT:        [  0, dso_sub,          0 ]

#include <stdlib.h>
#include <windows.h>

void _PGOPTI_Prof_Reset_All();
void dso1_init(int x);
void dso1_exec();

int main(int argc, const char *argv[]) {
  // Invoke the shared object function to generate some counts,
  // and collect some value profiling information.
  dso1_init(-1);
  dso1_exec();

  // Clear the data collected so far on the main module and DLL.
  _PGOPTI_Prof_Reset_All();

  // Invoke the shared object function to generate some counts,
  // and collect different value profiling information.
  dso1_init(1);
  dso1_exec();

  return 0;
}
