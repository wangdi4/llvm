// RUN: %clang_cc1 -fhls -fsyntax-only -ast-dump -verify -pedantic %s | FileCheck %s

__attribute__((ihc_component))
__attribute__((stall_latency_enabled))
void foo1() {}
// CHECK: FunctionDecl{{.*}}foo1
// CHECK-DAG: ComponentAttr
// CHECK-DAG: StallLatencyAttr{{.*}}stall_latency_enabled

__attribute__((stall_latency_enabled))
__attribute__((ihc_component))
void foo2() {}
// CHECK: FunctionDecl{{.*}}foo2
// CHECK-DAG: ComponentAttr
// CHECK-DAG: StallLatencyAttr{{.*}}stall_latency_enabled

__attribute__((ihc_component))
__attribute__((stall_latency_disabled))
void foo3() {}
// CHECK: FunctionDecl{{.*}}foo3
// CHECK-DAG: ComponentAttr
// CHECK-DAG: StallLatencyAttr{{.*}}stall_latency_disabled

__attribute__((stall_latency_disabled))
__attribute__((ihc_component))
void foo4() {}
// CHECK: FunctionDecl{{.*}}foo4
// CHECK-DAG: ComponentAttr
// CHECK-DAG: StallLatencyAttr{{.*}}stall_latency_disabled


//expected-error@+1{{'stall_latency_enabled' attribute takes no arguments}}
void __attribute__((ihc_component)) __attribute__((stall_latency_enabled(1)))
bar1() {}

//expected-error@+1{{'stall_latency_disabled' attribute takes no arguments}}
void __attribute__((ihc_component)) __attribute__((stall_latency_disabled(1)))
bar2() {}

//expected-error@+1{{'stall_latency_enabled' can only be applied to a component function}}
void __attribute__((stall_latency_enabled)) bar3() {}

//expected-error@+1{{'stall_latency_disabled' can only be applied to a component function}}
void __attribute__((stall_latency_disabled)) bar4() {}

__attribute__((ihc_component))
__attribute__((stall_latency_enabled))
__attribute__((stall_latency_disabled))
//expected-error@-1{{attributes are not compatible}}
//expected-note@-3{{conflicting attribute is here}}
void bar5() {}

__attribute__((ihc_component))
__attribute__((stall_latency_disabled))
__attribute__((stall_latency_enabled))
//expected-error@-1{{attributes are not compatible}}
//expected-note@-3{{conflicting attribute is here}}
void bar6() {}

__attribute__((ihc_component))
__attribute__((stall_latency_enabled))
__attribute__((stall_latency_enabled))
//expected-warning@-1{{'stall_latency_enabled' is already applied}}
void bar7() {}

__attribute__((ihc_component))
__attribute__((stall_latency_disabled))
__attribute__((stall_latency_disabled))
//expected-warning@-1{{'stall_latency_disabled' is already applied}}
void bar8() {}
