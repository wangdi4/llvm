// Check that fopen(NULL, "r") is ok.
// RUN: %clang -O2 %s -o %t && %run %t

// INTEL_CUSTOMIZATION
// JIRA: CMPLRLLVM-42781
// We add `-D_FILE_OFFSET_BITS=64` to 32bit program, which makes fopen resolve to fopen64, but
// fopen64 interceptor in sanitizer_common cannot handle null filename yet.
// UNSUPPORTED: i386-target-arch
// end INTEL_CUSTOMIZATION

#include <stdio.h>
const char *fn = NULL;
FILE *f;
int main() { f = fopen(fn, "r"); }
