// Input file for testing instr-reset-all-dyn*.c Windows test cases
// for dumping PGO data from DLLs.

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>

#include <stdio.h>
#include <stdlib.h>

void _PGOPTI_Prof_Dump_All();

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call,
                      LPVOID lpReserved) {
  switch (ul_reason_for_call) {
  case DLL_PROCESS_ATTACH:
  case DLL_THREAD_ATTACH:
  case DLL_THREAD_DETACH:
    break;
  case DLL_PROCESS_DETACH:
    break;
  }
  return TRUE;
}

__declspec(dllexport) void dso1_func1(int x) {
  printf("DSO 1. Func1(%d)\n", x);
  return;
}

__declspec(dllexport) void dso1_dump() { _PGOPTI_Prof_Dump_All(); }

void _PGOPTI_Prof_Reset_All();

void (*func)() = NULL;

void dso_sub() { printf("DSO 1. dso_sub()\n"); }

void dso_add() { printf("DSO 1. dso_add()\n"); }

__declspec(dllexport) void dso1_init(int x) {
  printf("DSO 1. Func1(%d)\n", x);
  if (x < 0) {
    func = &dso_sub;
  } else {
    func = &dso_add;
  }
  return;
}

__declspec(dllexport) void dso1_exec() { func(); }

__declspec(dllexport) void dso1_reset() { _PGOPTI_Prof_Reset_All(); }
