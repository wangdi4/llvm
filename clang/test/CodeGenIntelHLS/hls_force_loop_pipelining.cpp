//RUN: %clang_cc1 -fhls -emit-llvm -triple x86_64-unknown-linux-gnu -o - %s | FileCheck %s

__attribute__((hls_force_loop_pipelining("on"))) void foo1() {}
// CHECK: @_Z4foo1v{{.*}}!force_loop_pipelining [[CFOO1:![0-9]+]]

__attribute__((ihc_component))
__attribute__((hls_force_loop_pipelining("off"))) void
foo2() {}
// CHECK: @_Z4foo2v{{.*}}!force_loop_pipelining [[CFOO2:![0-9]+]]

//CHECK: [[CFOO1]] = !{!"on"}
//CHECK: [[CFOO2]] = !{!"off"}
