// RUN: %clang_cc1 %s -triple spir64-unknown-unknown-intelfpga -cl-std=CL2.0 -verify -pedantic -fsyntax-only
// RUN: %clang_cc1 %s -triple spir64-unknown-unknown-intelfpga -verify -pedantic -fsyntax-only
// RUN: %clang_cc1 %s -triple x86_64-unknown-unknown-intelfpga -cl-std=CL2.0 -verify -pedantic -fsyntax-only
// RUN: %clang_cc1 %s -triple x86_64-unknown-unknown-intelfpga -verify -pedantic -fsyntax-only

#ifndef cl_intel_fpga_host_pipe
#error "cl_intel_fpga_host_pipe is not defined."
#endif

#pragma OPENCL EXTENSION cl_intel_fpga_host_pipe : enable

kernel void k1(read_only pipe int p1 __attribute__((intel_host_accessible)),
               write_only pipe float p2 __attribute__((intel_host_accessible))) {

}

kernel void k2(int i __attribute__((intel_host_accessible))) {
  // expected-warning@-1{{'intel_host_accessible' attribute only applies to OpenCL pipes}}
}

__attribute__((intel_host_accessible))
kernel void k3(int i) {
  // expected-warning@-2{{'intel_host_accessible' attribute only applies to variables}}
}

__kernel void k4(read_only pipe int pin __attribute__((intel_host_accessible)),
                       write_only pipe int pout __attribute__((intel_host_accessible)),
                       int iters) {
    for (int i = 0; i < iters; ++i) {
      int val = 0;
      while (read_pipe(pin, &val)) {}
      while (write_pipe(pout, &val)) {}
    }
  }

#pragma OPENCL EXTENSION cl_intel_fpga_host_pipe : disable

kernel void k5(read_only pipe int p1 __attribute__((intel_host_accessible)),
  // expected-error@-1{{'intel_host_accessible' attribute requires cl_intel_fpga_host_pipe extension to be enabled}}
               write_only pipe float p2 __attribute__((intel_host_accessible))) {
  // expected-error@-1{{'intel_host_accessible' attribute requires cl_intel_fpga_host_pipe extension to be enabled}}
}
