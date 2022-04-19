// CQ#364268
// RUN: %clang_cc1 -emit-llvm -fintel-compatibility -triple x86_64-unknown-linux-gnu -opaque-pointers -o - %s | FileCheck %s

// CHECK: define{{.*}}i32 @main(ptr noundef %arg1, i32 noundef %arg2)
int main(char **arg1, int arg2) {
}
