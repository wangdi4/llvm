// REQUIRES: linux

// RUN: mkdir -p %t.d
// RUN: %clang_profgen -c -fPIC -O2 -o %t.d/main.o %s
// RUN: %clang_profgen -c -fPIC -O2 -o %t.d/instr-dump-all-so1.o %S/Inputs/instr-dump-all-so1.c
// RUN: %clang_profgen -shared -o %t.d/libinstr-dump-all-so1.so %t.d/instr-dump-all-so1.o
// RUN: %clang_profgen -o %t.d/main.exe %t.d/main.o
// RUN: env LLVM_PROFILE_FILE=%t.d/instr-dump-all.profraw %run %t.d/main.exe %t.d/libinstr-dump-all-so1.so
// RUN: llvm-profdata merge -o %t.d/instr-dump-all.profdata %t.d/instr-dump-all.profraw
// RUN: llvm-profdata show --all-functions %t.d/instr-dump-all.profdata | FileCheck %s

// This test checks whether the compiler API function that allows the
// user to force dumping profiling data dumps the data from a shared
// object that is loaded at runtime. In this case, the function
// 'dos1_func1' is coming from a shared object.

// CHECK: Counters:
// CHECK-DAG: main
// CHECK-DAG: foo
// CHECK-DAG: dso1_func1

#include <dlfcn.h>
#include <stdio.h>
#include <unistd.h>

void _PGOPTI_Prof_Dump_All();
void dso1_func1(int x);

int foo(int);

int main(int argc, const char *argv[]) {
  int result;
  const char *lib;
  void (*dso_func)(int);
  char *error;
  void* lh;

  if (argc < 2) {
    fprintf(stderr, "Missing argument\n");
    return 1;
  }

  fprintf(stderr, "Loading shared library\n");

  // Dynamically load the shared object library
  lib = argv[1];
  lh = dlopen(lib, RTLD_LAZY);
  if (!lh) {
    fprintf(stderr, "Error during dlopen(): %s\n", dlerror());
    return 1;
  }

  // Execute a function in the main module
  result = foo(0);

  // Execute a function that is from a shared object
  dso_func = (void (*)(int)) dlsym(lh, "dso1_func1");
  if ((error = dlerror()) != NULL)  {
    fprintf (stderr, "%s\n", error);
    return 1;
  }
  (*dso_func)(result);

  // Force the profile data to be written for this module and the
  // loaded shared objects.
  _PGOPTI_Prof_Dump_All();

  // Use '_exit' to terminate without calling the atexit() registrations
  // that normally dump the profile data.
  _exit(0);
}

__attribute__((noinline)) int foo(int X) {
	return X <= 0 ? -X : X;
}
