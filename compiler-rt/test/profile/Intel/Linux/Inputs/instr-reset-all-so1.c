// Input file for testing instr-reset-all*.c test case
// dumping of shared objects.

#include <stdio.h>

void _PGOPTI_Prof_Reset_All();

void (*func)() = NULL;

void dso_sub() { printf("DSO 1. dso_sub()\n"); }

void dso_add() { printf("DSO 1. dso_add()\n"); }

void dso1_init(int x) {
  printf("DSO 1. Func1(%d)\n", x);
  if (x < 0) {
    func = &dso_sub;
  } else {
    func = &dso_add;
  }
  return;
}

void dso1_exec() { func(); }

void dso1_reset() { _PGOPTI_Prof_Reset_All(); }
