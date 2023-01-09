// REQUIRES: windows

// RUN: mkdir -p %t.d
// RUN: %clang_profgen -c -O2 -o %t.d/main.obj %s
// RUN: %clang_profgen -c -O2 -o %t.d/instr-dump-all-dyn1.obj %S/Inputs/instr-dump-all-dyn1.c
// RUN: %clang_profgen -Wl,-dll -o %t.d/instr-dump-all-dyn1.dll %t.d/instr-dump-all-dyn1.obj
// RUN: %clang_profgen -o %t.d/main.exe %t.d/main.obj
// RUN: env LLVM_PROFILE_FILE=%t.d/instr-dump-all.profraw %run %t.d/main.exe %t.d\\instr-dump-all-dyn1.dll
// RUN: llvm-profdata merge -o %t.d/instr-dump-all.profdata %t.d/instr-dump-all.profraw
// RUN: llvm-profdata show --all-functions %t.d/instr-dump-all.profdata | FileCheck %s

// This test checks whether the compiler API function that allows the
// user to force dumping profiling data dumps the data from a DLL that
// is loaded at runtime. In this case, the function 'dos1_func1'
// is coming from a DLL, and the other two functions come from the main
// module.

// CHECK: Counters:
// CHECK-DAG: main
// CHECK-DAG: foo
// CHECK-DAG: dso1_func1

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

void _PGOPTI_Prof_Dump_All();
void dso1_func1(int x);

int foo(int);

typedef void (*DLLFnPtr)(int);

int main(int argc, const char *argv[]) {
  int result;
  const char *lib;
  HMODULE handle;

  if (argc < 2) {
    fprintf(stderr, "Missing argument\n");
    return 1;
  }

  fprintf(stderr, "Loading shared library\n");

  // Dynamically load the shared object library
  lib = argv[1];
  handle = LoadLibraryA(lib);
  if (!handle) {
    fprintf(stderr, "Error during LoadLibraryExA(%s)\n", lib);
    return 1;
  }

  DLLFnPtr fnPtr = (DLLFnPtr)GetProcAddress(handle, "dso1_func1");
  if (!fnPtr) {
    fprintf(stderr, "Did not find dso1_func1\n");
    return 1;
  }

  // Execute a function in the main module
  result = foo(0);

  // Execute a function that is from a shared object
  fnPtr(result);

  // Force the profile data to be written for this module and the
  // loaded shared objects.
  _PGOPTI_Prof_Dump_All();

  // Use '_exit' to terminate without calling the atexit() registrations
  // that normally dump the profile data.
  _exit(0);
}

__attribute__((noinline)) int foo(int X) { return X <= 0 ? -X : X; }
