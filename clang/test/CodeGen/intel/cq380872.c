// RUN: %clang_cc1 -triple x86_64-unknown-unknown -fintel-compatibility %s -emit-llvm -o - | FileCheck %s
// expected-no-diagnostics

int  foo(int [][]); // expected-warning {{array with incomplete (unknown) size element 'int []'}}
int  foo(int tokens[5][5])  // expected-note {{passing argument to parameter 'tokens' here}}
{
    return tokens[2][2];
};

// CHECK-LABEL: bar
int bar(){
int a;
int tokens2[7][7];
int tokens1[5][5];
    tokens1[2][2] = 5;
    tokens2[2][2] = 10;
    a = foo(tokens1);
    a += foo(tokens2);  // expected-warning {{incompatible pointer types passing 'int [7][7]' to parameter of type 'int (*)[5]'}}
    return a;
}

// CHECK: call {{.+}}foo{{.+}}5 x i
// CHECK: call {{.+}}foo{{.+}}5 x i
