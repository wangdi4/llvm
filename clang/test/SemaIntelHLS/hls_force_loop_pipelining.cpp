// RUN: %clang_cc1 -fhls -fsyntax-only -ast-dump -verify -pedantic %s | FileCheck %s

__attribute__((hls_force_loop_pipelining("on"))) void foo1() {}
// CHECK: FunctionDecl{{.*}}foo1
// CHECK: ForceLoopPipeliningAttr{{.*}}"on"

__attribute__((hls_force_loop_pipelining("off"))) void foo2() {}
// CHECK: FunctionDecl{{.*}}foo2
// CHECK: ForceLoopPipeliningAttr{{.*}}"off"

__attribute__((hls_force_loop_pipelining("on")))
__attribute__((ihc_component)) void
foo3() {}
// CHECK: FunctionDecl{{.*}}foo3
// CHECK-DAG: ComponentAttr
// CHECK-DAG: ForceLoopPipeliningAttr{{.*}}"on"

__attribute__((ihc_component))
__attribute__((hls_force_loop_pipelining("on"))) void
foo4() {}
// CHECK: FunctionDecl{{.*}}foo4
// CHECK-DAG: ComponentAttr
// CHECK-DAG: ForceLoopPipeliningAttr{{.*}}"on"

__attribute__((hls_force_loop_pipelining("off")))
__attribute__((ihc_component)) void
foo5() {}
// CHECK: FunctionDecl{{.*}}foo5
// CHECK-DAG: ComponentAttr
// CHECK-DAG: ForceLoopPipeliningAttr{{.*}}"off"

__attribute__((ihc_component))
__attribute__((hls_force_loop_pipelining("off"))) void
foo6() {}
// CHECK: FunctionDecl{{.*}}foo6
// CHECK-DAG: ComponentAttr
// CHECK-DAG: ForceLoopPipeliningAttr{{.*}}"off"
//
//expected-error@+1{{'hls_force_loop_pipelining' attribute argument must be 'on' or 'off'}}
__attribute__((hls_force_loop_pipelining("abc"))) void foo7() {}

//expected-error@+1{{expected string literal as argument of 'hls_force_loop_pipelining' attribute}}
__attribute__((hls_force_loop_pipelining(1))) void foo8() {}

__attribute__((hls_ii(1)))
__attribute__((hls_force_loop_pipelining("on"))) void
foo9() {}

__attribute__((hls_min_ii(1)))
__attribute__((hls_force_loop_pipelining("on"))) void
foo10() {}

__attribute__((hls_max_ii(1)))
__attribute__((hls_force_loop_pipelining("on"))) void
foo11() {}

__attribute__((hls_max_invocation_delay(1)))
__attribute__((hls_force_loop_pipelining("on"))) void
foo12() {}

//expected-error@+3{{'hls_force_loop_pipelining' and 'hls_ii' attributes are not compatible}}
//expected-note@+1{{conflicting attribute is here}}
__attribute__((hls_ii(1)))
__attribute__((hls_force_loop_pipelining("off"))) void
foo13() {}

//expected-error@+3{{'hls_force_loop_pipelining' and 'hls_min_ii' attributes are not compatible}}
//expected-note@+1{{conflicting attribute is here}}
__attribute__((hls_min_ii(1)))
__attribute__((hls_force_loop_pipelining("off"))) void
foo14() {}

//expected-error@+3{{'hls_force_loop_pipelining' and 'hls_max_ii' attributes are not compatible}}
//expected-note@+1{{conflicting attribute is here}}
__attribute__((hls_max_ii(1)))
__attribute__((hls_force_loop_pipelining("off"))) void
foo15() {}

//expected-error@+3{{'hls_force_loop_pipelining' and 'hls_max_invocation_delay' attributes are not compatible}}
//expected-note@+1{{conflicting attribute is here}}
__attribute__((hls_max_invocation_delay(1)))
__attribute__((hls_force_loop_pipelining("off"))) void
foo16() {}
