// RUN: %clang_cc1 -x cl -cl-std=CL2.0 -triple spir-unknown-unknown-intelfpga %s -verify

__kernel void k1(__local  __attribute__((local_mem_size(1042))) int *a ) {
}

__kernel void k2(__global __attribute__((buffer_location("DDR"))) int * a ) {
}

__kernel void k3(__local __attribute__((buffer_location("DDR"))) int *a ) { // expected-warning{{'buffer_location' attribute can be aplied only for parameters that reside in global address space, attribute ignored}}
}

__kernel void k4(__global __attribute__((local_mem_size(1024))) int * a ) { // expected-warning{{'local_mem_size' attribute can be aplied only for parameters that reside in local address space, attribute ignored}}
}
