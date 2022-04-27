 // RUN: %clang_cc1 %s -triple spir-unknown-unknown-intelfpga -cl-std=CL1.2 -verify -pedantic -fsyntax-only
 // RUN: %clang_cc1 %s -triple spir-unknown-unknown-intelfpga -cl-std=CL1.2 -verify -Wno-ivdep-usage -fsyntax-only -DNOWARN
 // RUN: %clang_cc1 %s -triple spir-unknown-unknown-unknown -cl-std=CL1.2 -verify -pedantic -fsyntax-only -DNOWARN
__kernel void set_zero(void) {
  int array[10];
#pragma ivdep
#ifdef NOWARN
// expected-no-diagnostics
#else
// expected-warning@-4 {{'#pragma ivdep' is used in FPGA emulator environment\n NOTE: incorrect use of the ivdep pragma can result in functional issues that will not be replicated in the emulator flow}}
#endif // NOWARN
  for (int i=0; i<10; i++)
    array[i] = 0;
}
