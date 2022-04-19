// CQ#364268
// RUN: %clang_cc1 -emit-llvm -fintel-compatibility -triple x86_64-unknown-linux-gnu -opaque-pointers -o - %s | FileCheck %s

typedef struct S {
    unsigned char a;
    double *b;
    unsigned short c;
} SA;

// CHECK: define{{.*}}i32 @main(double {{.*}}, ptr {{.+}}, ...)
int main(double arg1, SA arg2, ...) {
}
