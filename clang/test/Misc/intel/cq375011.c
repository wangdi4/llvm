// RUN: %clang_cc1 -fintel-compatibility -ast-dump %s | FileCheck %s

int *f(int a, unsigned b) __attribute__((__alloc_size__(1, 2)));
int *g(int a) __attribute__((__alloc_size__(1)));

// CHECK: FunctionDecl {{.*}} 'int *(int, unsigned int)'
// CHECK: |-ParmVarDecl {{.*}} 'int'
// CHECK: |-ParmVarDecl {{.*}} 'unsigned int'
// CHECK: `-AllocSizeAttr
// CHECK: |-IntegerLiteral {{.*}} 'int' 1
// CHECK: `-IntegerLiteral {{.*}} 'int' 2

// CHECK: FunctionDecl {{.*}} 'int *(int)'
// CHECK: |-ParmVarDecl {{.*}} 'int'
// CHECK: `-AllocSizeAttr
// CHECK: |-IntegerLiteral {{.*}} 'int' 1
// CHECK: `-<<<NULL>>>
