// RUN: %clang_cc1 -triple=x86_64-unknown-linux-gnu -O0 -emit-llvm %s -o - \
// RUN:   | FileCheck %s

// CHECK: !llvm.ident = !{[[I:![0-9]+]]}
// CHECK: [[I]] = {{.*}}icx
