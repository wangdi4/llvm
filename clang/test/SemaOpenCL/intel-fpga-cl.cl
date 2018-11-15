// RUN: %clang_cc1 %s -triple spir-unknown-unknown-intelfpga -cl-std=CL1.2 -verify -pedantic -fsyntax-only -DINTELFPGATRIPLE
// RUN: %clang_cc1 %s -triple spir-unknown-unknown-intelfpga -cl-std=CL2.0 -verify -pedantic -fsyntax-only -DINTELFPGATRIPLE
// RUN: %clang_cc1 %s -triple x86_64-unknown-unknown-intelfpga -cl-std=CL1.2 -verify -pedantic -fsyntax-only -DINTELFPGATRIPLE
// RUN: %clang_cc1 %s -triple spir-unknown-unknown -cl-std=CL1.2 -verify -pedantic -fsyntax-only
// RUN: %clang_cc1 %s -triple spir-unknown-unknown -cl-std=CL2.0 -verify -pedantic -fsyntax-only
// RUN: %clang_cc1 %s -triple x86_64-unknown-unknown -cl-std=CL1.2 -verify -pedantic -fsyntax-only

#ifndef INTELFPGA_CL
  #error "INTELFPGA_CL define is missed!"
#elif (INTELFPGA_CL != 191)
  #error "INTELFPGA_CL contains a wrong version"
#endif
#ifdef INTELFPGATRIPLE
// expected-no-diagnostics
#else
// expected-error@-7{{INTELFPGA_CL define is missed!}}
#endif // INTELFPGATRIPLE

void foo() {}
