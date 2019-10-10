//RUN: %clang_cc1 -fhls -fsyntax-only -ast-dump -verify -pedantic %s | FileCheck %s

//CHECK: VarDecl{{.*}}gc1
//CHECK: MemoryAttr{{.*}}Implicit
//CHECK: MaxReplicatesAttr
//CHECK: IntegerLiteral{{.*}}2{{$}}

const int __attribute__((max_replicates(2))) gc1 = 0;

// expected-note@+2{{conflicting attribute is here}}
// expected-error@+1{{'max_replicates' and 'register' attributes are not compatible}}
const int __attribute__((register, max_replicates(2))) gc3 = 0;

//CHECK: VarDecl{{.*}}gc4
//CHECK: MemoryAttr{{.*}}Implicit
//CHECK: SimpleDualPortAttr
const int __attribute__((simple_dual_port)) gc4 = 0;

// expected-error@+2{{'simple_dual_port' and 'register' attributes are not compatible}}
// expected-note@+1{{conflicting attribute is here}}
const int __attribute__((register, simple_dual_port)) gc5 = 0;

//CHECK: VarDecl{{.*}}gc6
//CHECK: MemoryAttr{{.*}}Implicit
//CHECK: MaxReplicatesAttr
//CHECK: IntegerLiteral{{.*}}2{{$}}
//CHECK: SimpleDualPortAttr
const int __attribute__((max_replicates(2), simple_dual_port)) gc6 = 0;

const int __attribute__((max_replicates(2))) gc7 = 0;

// expected-error@+1{{'simple_dual_port' attribute takes no arguments}}
const int __attribute__((simple_dual_port(1))) gc10 = 0;

void foo() {
  //CHECK: VarDecl{{.*}}lc1
  //CHECK: MemoryAttr{{.*}}Implicit
  //CHECK: MaxReplicatesAttr
  //CHECK: IntegerLiteral{{.*}}2{{$}}
  int __attribute__((max_replicates(2))) lc1;

  // expected-error@+2{{'max_replicates' and 'register' attributes are not compatible}}
  // expected-note@+1{{conflicting attribute is here}}
  int __attribute__((register, max_replicates(2))) lc2;

  //CHECK: VarDecl{{.*}}lc4
  //CHECK: MemoryAttr{{.*}}Implicit
  //CHECK: SimpleDualPortAttr
  int __attribute__((simple_dual_port)) lc4 = 0;

  // expected-error@+2{{'simple_dual_port' and 'register' attributes are not compatible}}
  // expected-note@+1{{conflicting attribute is here}}
  int __attribute__((register, simple_dual_port)) lc5 = 0;

  //CHECK: VarDecl{{.*}}lc6
  //CHECK: MemoryAttr{{.*}}Implicit
  //CHECK: MaxReplicatesAttr
  //CHECK: IntegerLiteral{{.*}}2{{$}}
  //CHECK: SimpleDualPortAttr

  int __attribute__((max_replicates(2), simple_dual_port)) lc6;
  //expected-error@+1{{'max_replicates' attribute requires integer constant between 1 and 1048576 inclusive}}
  int __attribute__((max_replicates(0))) lc7;
  //expected-error@+1{{'max_replicates' attribute requires integer constant between 1 and 1048576 inclusive}}
  int __attribute__((max_replicates(-1))) lc8;
  //expected-error@+1{{'simple_dual_port' attribute takes no arguments}}
  int __attribute__((simple_dual_port(0))) lc12;

  // expected-warning@+1{{attribute 'max_replicates' is already applied}}
  int __attribute__((max_replicates(2), max_replicates(3))) lc13;

  //expected-warning@+1{{attribute 'simple_dual_port' is already applied}}
  int __attribute__((simple_dual_port, simple_dual_port)) lc14;
}

//CHECK: ComponentAttr
//CHECK: ComponentInterfaceAttr{{.*}}Implicit Streaming
//CHECK: FunctionDecl{{.*}}foo_one
//CHECK: TemplateArgument{{.*}}4{{$}}
//CHECK: TemplateArgument{{.*}}8{{$}}
//CHECK: TemplateArgument{{.*}}2{{$}}
//CHECK: TemplateArgument{{.*}}3{{$}}
//CHECK: TemplateArgument{{.*}}2{{$}}
//CHECK: TemplateArgument{{.*}}3{{$}}
//CHECK: TemplateArgument{{.*}}4{{$}}
//CHECK: TemplateArgument{{.*}}4{{$}}
//CHECK: TemplateArgument{{.*}}2{{$}}

//CHECK: VarDecl{{.*}}var_one
//CHECK: MaxReplicatesAttr
//CHECK_NEXT: ConstantExpr
//CHECK_NEXT: SubstNonTypeTemplateParmExpr
//CHECK_NEXT: IntegerLiteral{{.*}}2{{$}}

//CHECK: VarDecl{{.*}}var_two
//CHECK: MemoryAttr{{.*}}Implicit
//CHECK: BankWidthAttr
//CHECK-NEXT: ConstantExpr
//CHECK-NEXT: SubstNonTypeTemplateParmExpr
//CHECK-NEXT: IntegerLiteral{{.*}}4{{$}}
//CHECK: NumBanksAttr
//CHECK-NEXT: ConstantExpr
//CHECK-NEXT: SubstNonTypeTemplateParmExpr
//CHECK-NEXT: IntegerLiteral{{.*}}8{{$}}

//CHECK: VarDecl{{.*}}var_three
//CHECK: MemoryAttr{{.*}}Implicit
//CHECK: BankWidthAttr
//CHECK-NEXT: ConstantExpr
//CHECK-NEXT: SubstNonTypeTemplateParmExpr
//CHECK-NEXT: IntegerLiteral{{.*}}4{{$}}
//CHECK: NumBanksAttr
//CHECK-NEXT: ConstantExpr
//CHECK-NEXT: SubstNonTypeTemplateParmExpr
//CHECK-NEXT: IntegerLiteral{{.*}}8{{$}}
//CHECK: SimpleDualPortAttr

//CHECK: VarDecl{{.*}}var_four
//CHECK: MemoryAttr{{.*}}Implicit
//CHECK: SimpleDualPortAttr

//CHECK: VarDecl{{.*}}var_five
//CHECK: MemoryAttr{{.*}}Implicit
//CHECK: SimpleDualPortAttr
//CHECK: MaxReplicatesAttr
//CHECK-NEXT: ConstantExpr
//CHECK-NEXT: SubstNonTypeTemplateParmExpr
//CHECK-NEXT: IntegerLiteral{{.*}}2{{$}}

template <int bankwidth, int numbanks, int readports, int writeports,
          int bit1, int bit2, int bit3, int max_concurrency, int num_replicates>
__attribute__((ihc_component)) void foo_one() {
  __attribute__((bankwidth(bankwidth), numbanks(numbanks),
                 max_replicates(num_replicates))) int var_one;

  __attribute__((bankwidth(bankwidth), numbanks(numbanks),
                 max_replicates(num_replicates))) int var_two;

  __attribute__((bankwidth(bankwidth), numbanks(numbanks), simple_dual_port)) int var_three;

  // expected-error@+2{{'register' and 'simple_dual_port' attributes are not compatible}}
  // expected-note@+1{{conflicting attribute is here}}
  __attribute__((simple_dual_port, register)) int var_four;
  __attribute__((simple_dual_port, max_replicates(num_replicates))) int var_five;
}

void call() {
  foo_one<4, 8, 2, 3, 2, 3, 4, 4, 2>();
}

//CHECK: FunctionDecl {{.*}}call
//CHECK: CXXRecordDecl {{.*}}struct foo_two
//CHECK: FieldDecl {{.*}} f1
//CHECK-NEXT: RegisterAttr
//CHECK: FieldDecl {{.*}} f2
//CHECK-NEXT: RegisterAttr
//CHECK: FieldDecl {{.*}} f3
//CHECK-NEXT: MemoryAttr
//CHECK-NEXT: MaxReplicatesAttr
//CHECK-NEXT: ConstantExpr
//CHECK-NEXT: IntegerLiteral{{.*}}2{{$}}
//CHECK: FieldDecl {{.*}} f4
//CHECK-NEXT: MemoryAttr
//CHECK-NEXT: SimpleDualPortAttr
//CHECK: FieldDecl {{.*}} f5
//CHECK-NEXT: MemoryAttr
//CHECK-NEXT: MaxReplicatesAttr
//CHECK-NEXT: ConstantExpr
//CHECK-NEXT: IntegerLiteral{{.*}}2{{$}}
//CHECK-NEXT: SimpleDualPortAttr

struct foo_two {
  // expected-note@+2{{conflicting attribute is here}}
  // expected-error@+1{{'simple_dual_port' and 'register' attributes are not compatible}}
  int __attribute__((register, simple_dual_port)) f1;
  // expected-note@+2{{conflicting attribute is here}}
  // expected-error@+1{{'max_replicates' and 'register' attributes are not compatible}}
  int __attribute__((register, max_replicates(3))) f2;
  int __attribute__((max_replicates(2))) f3;
  int __attribute__((simple_dual_port)) f4;
  int __attribute__((max_replicates(2), simple_dual_port)) f5;
  //expected-error@+1{{'simple_dual_port' attribute takes no arguments}}
  int __attribute__((simple_dual_port(3))) f6;
};

static foo_two s1;

void bar1() {
  s1.f1 = 0;
  s1.f2 = 0;
  s1.f3 = 0;
  s1.f4 = 0;
  s1.f5 = 0;
}

//CHECK: FunctionDecl {{.*}}bar1

struct foo_five {
  char __attribute__((max_replicates(2))) f1;
  char __attribute__((max_replicates(2))) f2[2];
  char __attribute__((simple_dual_port)) f3;
  char __attribute__((simple_dual_port)) f4[2];
};
//CHECK: CXXRecordDecl {{.*}}struct foo_five
//CHECK: FieldDecl{{.*}} f1
//CHECK: MemoryAttr
//CHECK: MaxReplicatesAttr
//CHECK-NEXT: ConstantExpr
//CHECK-NEXT: IntegerLiteral{{.*}}2{{$}}
//CHECK: FieldDecl{{.*}} f2
//CHECK-NEXT: MemoryAttr
//CHECK_NEXT: MaxReplicatesAttr
//CHECK: ConstantExpr
//CHECK-NEXT: IntegerLiteral{{.*}}2{{$}}
//CHECK: FieldDecl{{.*}} f3
//CHECK-NEXT: MemoryAttr
//CHECK-NEXT: SimpleDualPortAttr
//CHECK: FieldDecl{{.*}} f4
//CHECK-NEXT: MemoryAttr
//CHECK-NEXT: SimpleDualPortAttr
