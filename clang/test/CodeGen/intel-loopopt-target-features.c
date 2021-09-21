// RUN: %clang_cc1 -triple x86_64-linux-gnu -target-cpu x86-64 -emit-llvm %s -o - | FileCheck %s --check-prefix NO-LOOPOPT
// NO-LOOPOPT-NOT: "loopopt-pipeline"=

// RUN: %clang_cc1 -floopopt-pipeline=light -triple x86_64-linux-gnu -target-cpu x86-64 -emit-llvm %s -o - | FileCheck %s --check-prefix LOOPOPT-LIGHT
// LOOPOPT-LIGHT: "loopopt-pipeline"="light"

// RUN: %clang_cc1 -floopopt-pipeline=full -triple x86_64-linux-gnu -target-cpu x86-64 -emit-llvm %s -o - | FileCheck %s --check-prefix LOOPOPT-FULL
// LOOPOPT-FULL: "loopopt-pipeline"="full"
int foo() { return 42; }
