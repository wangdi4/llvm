// REQUIRES: windows

// RUN: mkdir -p %t.d
// RUN: %clang_profgen -g -c -O2 -o %t.d/main.obj %s
// RUN: %clang_profgen -g -c -O2 -o %t.d/instr-dump-all-dyn1.obj %S/Inputs/instr-dump-all-dyn1.c
// RUN: %clang_profgen -g -Wl,-dll -o %t.d/instr-dump-all-dyn1.dll %t.d/instr-dump-all-dyn1.obj
// RUN: %clang_profgen -g -o %t.d/main.exe %t.d/main.obj %t.d/instr-dump-all-dyn1.lib
// RUN: env LLVM_PROFILE_FILE=%t.d/instr-dump-all.profraw %run %t.d/main.exe
// RUN: llvm-profdata merge -o %t.d/instr-dump-all.profdata %t.d/instr-dump-all.profraw
// RUN: llvm-profdata show --all-functions %t.d/instr-dump-all.profdata | FileCheck %s

// This test checks whether the compiler API function that allows the
// user to force dumping profiling data dumps the data from a DLL that
// is linked into the application when the API function is called from
// the DLL. In this case, the function 'dos1_func1' is coming from a DLL,
// and the other two functions come from the main module.

// CHECK: Counters:
// CHECK-DAG: main
// CHECK-DAG: foo
// CHECK-DAG: dso1_func1

#include <stdlib.h>
#include <windows.h>

void dso1_func1(int x);
void dso1_dump();

int foo(int);

int main(int argc, const char *argv[]) {
  // Execute a function in the main module
  int result = foo(0);

  // Execute a function that is from a shared object
  dso1_func1(result);

  // Force the profile data to be written for this module and the
  // loaded shared objects, with a call to the dump routine made
  // by a shared object library.
  dso1_dump();

  // Use '_exit' to terminate without calling the atexit() registrations
  // that normally dump the profile data.
  _exit(0);
}

__attribute__((noinline)) int foo(int X) { return X <= 0 ? -X : X; }
