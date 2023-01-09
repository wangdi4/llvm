// Input file for testing instr-dump-all.c test case
// dumping of shared objects.

#include <stdio.h>

void _PGOPTI_Prof_Dump_All();

void dso1_func1(int x) {
  printf("DSO 1. Func1(%d)\n", x);
  return;
}

void dso1_dump() {
  _PGOPTI_Prof_Dump_All();
}
