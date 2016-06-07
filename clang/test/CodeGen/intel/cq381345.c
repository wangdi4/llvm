// RUN: %clang_cc1 -triple=x86_64-unknown-linux-gnu -fintel-compatibility -O0 -emit-llvm %s -o - | FileCheck %s

char ic; short is; int i; long il; float f; double d; long double ld; _Complex float cf; _Complex double cd; _Complex long double cl;
int foo1() {
    return __generic(i,ic,is, 4, 5, 6, 7, 8, 9);
}
// CHECK: ret i32 4
int foo2() {
    return __generic(f,ic,is, 4, 5, 6, 7, 8, 9);
}
// CHECK: ret i32 4
int foo3() {
    return __generic(f,f,f, 4, 5, 6, 7, 8, 9);
}
// CHECK: ret i32 5
int foo4() {
    return __generic(f,ic,ld, 4, 5, 6, 7, 8, 9);
}
// CHECK: ret i32 6
int foo5() {
    return __generic(cf,d, , 4, 5, 6, 7, 8, 9);
}
// CHECK: ret i32 7
int foo6() {
    return __generic(d,cf, , 4, 5, 6, 7, 8, 9);
}
// CHECK: ret i32 7
int foo7() {
    return __generic(cf,f, , 4, 5, 6, 7, 8, 9);
}
// CHECK: ret i32 8
int foo8() {
    return __generic(cf,cl, , 4, 5, 6, 7, 8, 9);
}
// CHECK: ret i32 9
int foo9() {
    return __generic(i, , , 4, 5, 6, 7, 8, 9);
}
// CHECK: ret i32 4
int foo10() {
    return __generic(d, , , 4, 5, 6, 7, 8, 9);
}
// CHECK: ret i32 4

