// REQUIRES: linux

// RUN: mkdir -p %t.d
// RUN: %clang_profgen -c -fPIC -O2 -o %t.d/main.o %s
// RUN: %clang -c -fPIC -O2 -o %t.d/instr-dump-all-so1.o %S/Inputs/instr-dump-all-so1.c
// RUN: %clang -shared -o %t.d/libinstr-dump-all-so1.so %t.d/instr-dump-all-so1.o
// RUN: %clang_profgen -rpath %t.d -o %t.d/main.exe %t.d/main.o -L%t.d -l instr-dump-all-so1
// RUN: env LLVM_PROFILE_FILE=%t.d/instr-dump-all.profraw %run %t.d/main.exe
// RUN: llvm-profdata merge -o %t.d/instr-dump-all.profdata %t.d/instr-dump-all.profraw
// RUN: llvm-profdata show --all-functions %t.d/instr-dump-all.profdata | FileCheck %s

// This test checks that the compiler _PGOPTI_Prof_Dump_All API function does
// not have issues when the main module is instrumented, but the shared library
// is NOT instrumented.

// CHECK: Counters:
// CHECK-DAG: main
// CHECK-NOT: dso1_func1

#include <dlfcn.h>
#include <unistd.h>

void _PGOPTI_Prof_Dump_All();
void dso1_func1(int x);

int foo(int);

int main(int argc, const char *argv[]) {
  // Execute a function in the main module
  int Result = foo(0);

  // Execute a function that is from a shared object
  dso1_func1(Result);

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
