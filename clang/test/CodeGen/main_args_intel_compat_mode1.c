// CQ#364268
// RUN: %clang_cc1 -emit-llvm -fintel-compatibility -o - %s | FileCheck %s

// CHECK: define i32 @main(i32 %arg1, i32* %arg2)
int main(int arg1, int *arg2) {
}
