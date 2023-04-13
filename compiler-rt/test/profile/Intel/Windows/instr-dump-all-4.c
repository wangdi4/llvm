// REQUIRES: windows

// RUN: mkdir -p %t.d
// RUN: %clang_profgen -c -O2 -o %t.d/main.obj %s
// RUN: %clang -c -O2 -o %t.d/instr-dump-all-dyn1.obj %S/Inputs/instr-dump-all-dyn1.c
// RUN: %clang_profgen -Wl,-dll -o %t.d/instr-dump-all-dyn1.dll %t.d/instr-dump-all-dyn1.obj
// RUN: %clang_profgen -o %t.d/main.exe %t.d/main.obj %t.d/instr-dump-all-dyn1.lib
// RUN: env LLVM_PROFILE_FILE=%t.d/instr-dump-all.profraw %run %t.d/main.exe
// RUN: llvm-profdata merge -o %t.d/instr-dump-all.profdata %t.d/instr-dump-all.profraw
// RUN: llvm-profdata show --all-functions %t.d/instr-dump-all.profdata | FileCheck %s

// This test checks the compiler API function that allows the user to force
// dumping profiling data dumps the data from a DLL for the case when the
// DLL is not compiled with instrumentation to verify that does not cause
// problems.

// CHECK: Counters:
// CHECK-DAG: main
// CHECK-DAG: foo
// CHECK-NOT: dso1_func1

#include <stdlib.h>
#include <windows.h>

void _PGOPTI_Prof_Dump_All();
void dso1_func1(int x);

int foo(int);

int main(int argc, const char *argv[]) {
  // Execute a function in the main module
  int result = foo(0);

  // Execute a function that is from a shared object
  dso1_func1(result);

  // Force the profile data to be written for this module and the
  // loaded shared objects.
  _PGOPTI_Prof_Dump_All();

  // Use '_exit' to terminate without calling the atexit() registrations
  // that normally dump the profile data.
  _exit(0);
}

__attribute__((noinline)) int foo(int X) { return X <= 0 ? -X : X; }
