// RUN: %clang_cc1 -x cl -triple spir-unknown-unknown-intelfpga -fsyntax-only -ast-dump -verify -pedantic %s | FileCheck %s
// RUN: %clang_cc1 -x cl -triple x86_64-unknown-unknown-intelfpga -fsyntax-only -ast-dump -verify -pedantic %s | FileCheck %s

__attribute__((stall_latency_enabled))
__kernel
void kfoo1(void) {}
// CHECK: FunctionDecl{{.*}}kfoo1
// CHECK: StallLatencyAttr{{.*}}stall_latency_enabled

__kernel
__attribute__((stall_latency_enabled))
void kfoo2(void) {}
// CHECK: FunctionDecl{{.*}}kfoo2
// CHECK: StallLatencyAttr{{.*}}stall_latency_enabled

__attribute__((stall_latency_disabled))
__kernel
void kfoo3(void) {}
// CHECK: FunctionDecl{{.*}}kfoo3
// CHECK: StallLatencyAttr{{.*}}stall_latency_disabled

__kernel
__attribute__((stall_latency_disabled))
void kfoo4(void) {}
// CHECK: FunctionDecl{{.*}}kfoo4
// CHECK: StallLatencyAttr{{.*}}stall_latency_disabled

//expected-error@+1{{'stall_latency_enabled' attribute takes no arguments}}
void __kernel __attribute__((stall_latency_enabled(1)))
bar1(void) {}

//expected-error@+1{{'stall_latency_disabled' attribute takes no arguments}}
void __kernel __attribute__((stall_latency_disabled(1)))
bar2(void) {}

//expected-error@+1{{'stall_latency_enabled' can only be applied to an OpenCL kernel function}}
void __attribute__((stall_latency_enabled)) bar3(void) {}

//expected-error@+1{{'stall_latency_disabled' can only be applied to an OpenCL kernel function}}
void __attribute__((stall_latency_disabled)) bar4(void) {}

__kernel
__attribute__((stall_latency_enabled))
__attribute__((stall_latency_disabled))
//expected-error@-1{{attributes are not compatible}}
//expected-note@-3{{conflicting attribute is here}}
void bar5(void) {}

__kernel
__attribute__((stall_latency_disabled))
__attribute__((stall_latency_enabled))
//expected-error@-1{{attributes are not compatible}}
//expected-note@-3{{conflicting attribute is here}}
void bar6(void) {}

__kernel
__attribute__((stall_latency_enabled))
__attribute__((stall_latency_enabled))
//expected-warning@-1{{'stall_latency_enabled' is already applied}}
void bar7(void) {}

__kernel
__attribute__((stall_latency_disabled))
__attribute__((stall_latency_disabled))
//expected-warning@-1{{'stall_latency_disabled' is already applied}}
void bar8(void) {}
