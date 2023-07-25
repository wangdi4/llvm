// REQUIRES: linux

// Test the usage of the "profile/pgouser.h" header file to
// get access to the user accessible API of the PGO runtime.

// RUN: mkdir -p %t.d
// RUN: %clang_profgen -c -O2 -o %t.d/main.o %s
// RUN: %clang_profgen -o %t.d/main.exe %t.d/main.o
// RUN: env LLVM_PROFILE_FILE=%t.d/instr-dump-all.profraw %run %t.d/main.exe
// RUN: llvm-profdata merge -o %t.d/instr-dump-all.profdata %t.d/instr-dump-all.profraw
// RUN: llvm-profdata show --all-functions %t.d/instr-dump-all.profdata | FileCheck %s

#include <unistd.h>

#define _PGO_INSTRUMENT
#include "profile/pgouser.h"

int foo(int);

int main(int argc, const char *argv[]) {
  // Execute a function in the main module
  int Result = foo(0);

  _PGOPTI_Prof_Reset_All();

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

// CHECK: Counters:
// CHECK: main
