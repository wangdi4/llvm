// RUN: %clang_cc1 -fhls -fsyntax-only -ast-dump -verify -pedantic %s | FileCheck %s

__attribute__((hls_max_ii(123))) void foo1() {}
// CHECK: FunctionDecl{{.*}}foo1
// CHECK: HLSMaxIIAttr
// CHECK-NEXT: ConstantExpr
// CHECK-NEXT: value: Int 123
// CHECK-NEXT: IntegerLiteral{{.*}}123{{$}}

__attribute__((hls_max_ii(123)))
__attribute__((ihc_component)) void
foo2() {}
// CHECK: FunctionDecl{{.*}}foo2
// CHECK: HLSMaxIIAttr
// CHECK-NEXT: ConstantExpr
// CHECK-NEXT: value: Int 123
// CHECK-NEXT: IntegerLiteral{{.*}}123{{$}}
// CHECK: ComponentAttr

__attribute__((ihc_component))
__attribute__((hls_max_ii(123))) void
foo3() {}
// CHECK: FunctionDecl{{.*}}foo3
// CHECK: ComponentAttr
// CHECK: HLSMaxIIAttr
// CHECK-NEXT: ConstantExpr
// CHECK-NEXT: value: Int 123
// CHECK-NEXT: IntegerLiteral{{.*}}123{{$}}

template <int tvalue>
void __attribute__((hls_max_ii(tvalue))) tfoo() {}

void call() {
  tfoo<8>();
  // CHECK: FunctionDecl{{.*}}tfoo
  // CHECK: HLSMaxIIAttr
  // CHECK-NEXT: DeclRefExpr{{.*}}NonTypeTemplateParm{{.*}}tvalue
  // CHECK: FunctionDecl{{.*}}tfoo
  // CHECK-NEXT: TemplateArgument integral 8
  // CHECK: HLSMaxIIAttr
  // CHECK-NEXT: ConstantExpr
  // CHECK-NEXT: value: Int 8
  // CHECK-NEXT: SubstNonTypeTemplateParmExpr
  // CHECK-NEXT: NonTypeTemplateParmDecl
  // CHECK-NEXT: IntegerLiteral{{.*}}8{{$}}
}

//expected-error@+1{{'hls_max_ii' attribute takes one argument}}
__attribute__((hls_max_ii(1, 2))) void foo4() {}

//expected-error@+1{{integral constant expression must have integral or unscoped enumeration type, not 'double'}}
__attribute__((hls_max_ii(1.0))) void foo5() {}

//expected-error@+1{{'hls_max_ii' attribute requires integer constant between 1 and 2147483647 inclusive}}
__attribute__((hls_max_ii(0))) void foo6() {}

//expected-error@+1{{'hls_max_ii' attribute requires integer constant between 1 and 2147483647 inclusive}}
__attribute__((hls_max_ii(-1))) void foo7() {}

//expected-error@+1{{'hls_max_ii' attribute requires integer constant between 1 and 2147483647 inclusive}}
__attribute__((hls_max_ii(2147483648))) void foo12() {}

//expected-error@+3{{'hls_max_ii' and 'hls_ii' attributes are not compatible}}
//expected-note@+1{{conflicting attribute is here}}
__attribute__((hls_ii(1)))
__attribute__((hls_max_ii(1))) void
foo8() {}

//expected-error@+3{{'hls_max_ii' and 'hls_min_ii' attributes are not compatible}}
//expected-note@+1{{conflicting attribute is here}}
__attribute__((hls_min_ii(1)))
__attribute__((hls_max_ii(1))) void
foo9() {}

//expected-error@+3{{'hls_max_ii' and 'hls_force_loop_pipelining(off)' attributes are not compatible}}
//expected-note@+1{{conflicting attribute is here}}
__attribute__((hls_force_loop_pipelining("off")))
__attribute__((hls_max_ii(1))) void
foo10() {}

__attribute__((hls_force_loop_pipelining("on")))
__attribute__((hls_max_ii(1))) void
foo11() {}
